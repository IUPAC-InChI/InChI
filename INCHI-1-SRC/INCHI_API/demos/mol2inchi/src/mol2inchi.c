/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
 */

#pragma warning( disable : 4996 )

#include <locale.h>

#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>

#if( defined( _MSC_VER ) && _MSC_VER > 1200 )
/* not VC6 or earlier */
#include <omp.h>
#endif
#else
#include <pthread.h>
#endif

#include <time.h>

#include "moreutil.h"
#include "mol2inchi.h"

#include "../../../../INCHI_BASE/src/inchi_api.h"


/****************************************************************************/
int main( int argc, char *argv[] )
{
    int retcode = 0, ret = 0, result = 0, at_eof = 0;
    int nmol = 0, curr_task = 0, curr_worker = 0, i, worker_done = 0, workers_done = 0, nread = 0;
    unsigned int tick_start, tick_current, tick_stop;
    char *fname = NULL;
    FILE *f = NULL;
    THREAD_PTR *pthreads = NULL;
    m2i_WorkDetails wd;
    m2i_WorkPool    pool;


    fprintf( stderr, "%s %-s\n%-s Build (%-s%-s) of %s %s %-s\n\n",
             APP_DESCRIPTION, INCHI_SRC_REV,
             INCHI_BUILD_PLATFORM, INCHI_BUILD_COMPILER, INCHI_BUILD_DEBUG, __DATE__, __TIME__,
             RELEASE_IS_FINAL ? "" : " *** pre-release, for evaluation only ***" );

#if (USE_TBB_MALLOC==1) && defined(_WIN32) 
#if 1 /*if defined(DEBUG_MOL2INCHI)*/
    {
        char **func_replacement_log;
        int func_replacement_status = TBB_malloc_replacement_log(&func_replacement_log);        
        if (func_replacement_status==0)
        {
            fprintf(stderr, "Intel tbbmalloc_proxy library loaded\n");
        }
        /*
        else
        {
            fprintf(stderr, "Intel TBB scalable allocator lib load failed: tbbmalloc_proxy cannot replace memory allocation routines\n");
            for (char** log_string = func_replacement_log; *log_string != 0; log_string++)
            {
                fprintf(stderr, "%s\n", *log_string);
            }
        }
        */

    }
#endif
#endif

#if (USE_TBB_MALLOC==1) 
    /* Dirty hack to if TBB malloc library loaded (call it to clear cache which is empty yet) */
    scalable_allocation_command(TBBMALLOC_CLEAN_ALL_BUFFERS, 0);
    /* ... if not failed, then the library is in place */
    fprintf(stderr, "Intel TBB scalable allocator replaced memory allocation routines\n");
#endif


    /* Parse command line.                                              */

    /* The first item is input filename, then mol2ichi own  switches,   */
    /* then  all the others (which are passed to InChI algorithm).      */

    if (argc < 2)
    {
        print_help( );
        return M2I_ERR_COMMAND_LINE;
    }

    tick_start = get_msec_timer( );

    fname = argv[1];
    result = m2i_WorkDetails_init( &wd, argc, argv );
    if (!result)
    {
        fprintf(stderr, "\nFATAL ERROR M2I_ERR_SET_JOB_OPTS\n");
        return M2I_ERR_SET_JOB_OPTS;
    }

    fprintf(stderr, "\nStarted at "); 
    print_time();
    fprintf( stderr, "by the following command line:\n\"" );
    for (i = 0; i < argc - 1; i++)
    {
        fprintf(stderr, "%-s ", argv[i]);
    }
    fprintf( stderr, "%-s\"\n", argv[argc - 1] );

    f = fopen( fname, "rb" );
    if (!f)
    {
        fprintf(stderr, "\nFATAL ERROR M2I_ERR_OPEN_INPUT\n");
        return M2I_ERR_OPEN_INPUT;
    }

    result = m2i_WorkPool_init( &pool, &wd );
    if (!result)
    {
        fprintf(stderr, "\nFATAL ERROR M2I_ERR_WORKPOOL_INIT\n");
        return M2I_ERR_WORKPOOL_INIT;
    }

    fprintf( stderr, "\n" );
    fflush( NULL );

    pthreads = (THREAD_PTR *) calloc( wd.n_workers, sizeof( THREAD_PTR ) );
    if (!pthreads)
    {
        fprintf(stderr, "\nFATAL ERROR M2I_ERR_PTHREADS_START\n"); 
        return M2I_ERR_PTHREADS_START;
    }

    at_eof = 0;
    nmol = 0;

    curr_worker = 0;
    curr_task = 0;
    while ( GrowStr_reset( &( pool.workers[curr_worker].tasks[curr_task].in_buf ) )
            && (ret=get_next_molfile_as_growing_str_buffer( f, &( pool.workers[curr_worker].tasks[curr_task].in_buf ), &at_eof ) ) == 1 )
    {
        nread++;
        if (nmol + 1 < wd.rstart)
        {
            continue;
        }
        if (nmol > wd.rend)
        {
            nmol--;
            at_eof = 1;
            break;
        }
        if (!at_eof || nread == 1)
        {
            ++nmol;
        }

        pool.workers[curr_worker].tasks[curr_task].nmol = nmol;
        wd.nmol = nmol;
        if (0 == wd.nmol % 25000)
        {
            tick_current = get_msec_timer( );
            fprintf( stderr, "Conversion speed %.1f mols/sec, structure %-ld\n",
                (double) wd.nmol *1000.0 / ( tick_current - tick_start ), wd.nmol );
            fflush( NULL );
        }

        if (!at_eof || nread == 1)
        {
            /* the current block of records/tasks */
            worker_done = 0;
            workers_done = 0;
            if ((curr_task + 1) == pool.workers[curr_worker].n_total_tasks)
            {
                pool.workers[curr_worker].n_ready_tasks = curr_task + 1;
#ifdef _WIN32
                pthreads[curr_worker] = (THREAD_PTR) _beginthreadex( NULL, 0, m2i_Worker_run, &pool.workers[curr_worker], 0, NULL );
#endif
#if ( !defined(_WIN32) && defined(__linux__) )
                pthread_create( &pthreads[curr_worker], NULL, m2i_Worker_run, &( pool.workers[curr_worker] ) );
#endif

#if defined(DEBUG_MOL2INCHI)
                fprintf( stderr, "Launched new thread: pthreads[%-ld]=%-p\n", curr_worker, pthreads[curr_worker]  );
                /*
                {
                    for (long t=0; t<pool.workers[curr_worker].n_total_tasks; t++)
                    {
                        fprintf( stderr, "\nTASK%-d={%s}\n", t, pool.workers[curr_worker].tasks[t].in_buf.ps );
                    }
                }
                */
                fflush( NULL );
#endif

                worker_done = 1;
                /*pool.workers[curr_worker].n_ready_tasks = 0; */

                if ( (curr_worker + 1)==pool.n_total_workers)
                {
                    pool.n_ready_workers = curr_worker + 1;
                    m2i_WorkPool_wait_and_print_all( &pool, pthreads );

                    workers_done = 1;
                    /* reset pool data */
#ifdef _WIN32
                    {
                        for (long w = 0; w < pool.n_ready_workers; w++)
                        {
                            CloseHandle( pthreads[w] );
                        }
                    }
#endif
                    m2i_WorkPool_reset( &pool );
                    curr_worker = 0;
                    curr_task = 0;
                    if (at_eof)
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            if (at_eof)
            {
                break;
            }

            ++curr_task;
            if (curr_task == pool.workers[curr_worker].n_total_tasks)
            {
                ++curr_worker;
                curr_task = 0;
            }
            if (curr_worker == pool.n_total_workers)
            {
                curr_worker = 0;
                curr_task = 0;
            }
        }
        else
        {
            /* the remaining records/tasks */
            if (!worker_done)
            {
                pool.workers[curr_worker].n_ready_tasks = curr_task;
#ifdef _WIN32
                pthreads[curr_worker] = (THREAD_PTR) _beginthreadex( NULL, 0, m2i_Worker_run, &pool.workers[curr_worker], 0, NULL );
#endif
#if ( !defined(_WIN32) && defined(__linux__) )
                pthread_create( &pthreads[curr_worker], NULL, m2i_Worker_run, &( pool.workers[curr_worker] ) );
#endif
            }
            if (!workers_done)
            {
                if (!curr_task)
                {
                    curr_worker--;
                }
                pool.n_ready_workers = curr_worker + 1;
                m2i_WorkPool_wait_and_print_all( &pool, pthreads );
            }
            break;
        }

    }

    if (!result)
    {
        fprintf( stderr, "\nReading record next to #%-ld: read/store error", wd.nmol );
    }

    /* Destroy pool data */
    m2i_WorkPool_close( &pool );
#ifdef _WIN32
    free( pthreads );
#endif
#if ( !defined(_WIN32) && defined(__linux__) )
    free( pthreads );
#endif

    /* Final printouts */
    tick_stop = get_msec_timer();
    fprintf( stderr, "\nProcessed %lu structure(s) with %-ld error(s).\n", wd.nmol, wd.n_inchi_err );
    fprintf(stderr, "Conversion speed %.1f mols/sec, structure %-ld\n", (double)wd.nmol *1000.0 / (tick_stop - tick_start), wd.nmol);   
    fprintf( stderr, "Finished at " );
    print_time( );
    fprintf( stderr, "Elapsed wall time: %d msec (", tick_stop - tick_start );

    if (wd.n_workers == 1)
    {
        fprintf( stderr, "single thread" );
    }
    else
    {
        fprintf( stderr, "%-d threads", wd.n_workers );
    }
    if (wd.n_tasks_per_worker != 1)
    {
        fprintf( stderr, " by %-d mols each)\n", wd.n_tasks_per_worker );
    }
    else
    {
        fprintf( stderr, ")\n" );
    }

    return retcode;
}


/****************************************************************************/
int m2i_WorkPool_wait_and_print_all( m2i_WorkPool *pool, THREAD_PTR *pthreads )
{
    int wait_res = 0, ret = 1;
    long w, t;
    m2i_WorkDetails *wd = pool->workers->tasks[0].wd;

#ifdef _WIN32
    wait_res = WaitForMultipleObjects( pool->n_ready_workers, pthreads, TRUE, INFINITE );
#endif
#if ( !defined(_WIN32) && defined(__linux__) )
    for (w = 0; w < pool->n_ready_workers; w++)
        pthread_join( pthreads[w], NULL );
#endif
    
    for (w = 0; w < pool->n_ready_workers; w++)
    {
        for (t = 0; t < pool->workers[w].n_ready_tasks; t++)
        {
            if (pool->workers[w].tasks[t].errstr.used > 0)
            {
                if (wd->verbose > 0)
                {
                    fprintf( stderr, "%-s\n", pool->workers[w].tasks[t].errstr.ps );
                }
            }
            if (pool->workers[w].tasks[t].output.used > 0)
            {
                fprintf( stdout, "%-s", pool->workers[w].tasks[t].output.ps );
                fprintf( stdout, "\n" );
            }
        }
        fflush( NULL );
    }

#if defined(DEBUG_MOL2INCHI)
    fprintf( stderr, "All threads finished, exiting from m2i_WorkPool_wait_and_print_all()\n");
    fflush( NULL );
#endif

    return ret;
}


/****************************************************************************
 Run a single worker thread
****************************************************************************/
M2I_THREADFUNC m2i_Worker_run( void *arg )
{
    int res = 0;
    long t;
    unsigned errs = 0;
    const char *delim = "\t"; /*"\n";*/
    inchi_Output iout, *piout = &iout;
    m2i_Worker      *w = (m2i_Worker *) arg;
    m2i_WorkDetails *wd = w->tasks[0].wd;

    /* For all prepared tasks */
    for (t = 0; t < w->n_ready_tasks; t++)
    {
        memset( piout, 0, sizeof( *piout ) );

        /* call API function to calculate InChI */
        res = MakeINCHIFromMolfileText( w->tasks[t].in_buf.ps, wd->options, piout );

        if ( res == mol2inchi_Ret_ERROR     ||
             res == mol2inchi_Ret_ERROR_get ||
             res == mol2inchi_Ret_ERROR_comp )
        {
            errs++;
            wd->n_inchi_err++;
            /* sprintf( errstr, "Error " ); */
            /* GrowStr_printf( errstr, "Error " ); */
        }
        else if (res == mol2inchi_Ret_WARNING)
        {
            ;
            /* sprintf( errstr, "Warning " ); */
            /* GrowStr_printf( errstr, "Warning " ); */
        }
        if (piout->szLog && piout->szLog[0])
        {
            GrowStr_printf( &( w->tasks[t].errstr ), "%-s structure #%-lu", piout->szLog, w->tasks[t].nmol );
        }

        /* Print InChI and other stuff if not empty */
        if (piout->szInChI && piout->szInChI[0])
        {
            /* But first, calculate InChIKey if requested */
            if (wd->get_inchikey)
            {
                int ik_ret = 0;           /* InChIKey-calc result code */
                int xhash1, xhash2;
                xhash1 = xhash2 = 0;

                ik_ret = GetINCHIKeyFromINCHI( piout->szInChI,
                                               xhash1,
                                               xhash2,
                                               w->tasks[t].ikey,
                                               w->tasks[t].szXtra1,
                                               w->tasks[t].szXtra2 );
                if (ik_ret != INCHIKEY_OK)
                {
                    w->tasks[t].ikey[0] = '\0';
                }
            }

            GrowStr_printf( &( w->tasks[t].output ), "%-lu%s%-s%s%-s%s%-s",
                             w->tasks[t].nmol,
                             wd->do_not_print_inchi ? "" : delim,
                             wd->do_not_print_inchi ? "" : piout->szInChI,
                             w->tasks[t].ikey[0] ? delim : "",
                             w->tasks[t].ikey[0] ? w->tasks[t].ikey : "",
                             piout->szAuxInfo && piout->szAuxInfo[0] ? delim : "",
                             piout->szAuxInfo && piout->szAuxInfo[0] ? piout->szAuxInfo : "" );
        }
        /* Reset output data structure */
        FreeINCHI( piout );
    }

#ifdef _WIN32
    /* _endthreadex( 0 ); */

    return errs;
#endif
}


/* Print program usage instructions */
void print_help( void )
{
    fprintf( stderr, "Usage: \n" );
    fprintf( stderr, "mol2inchi inputfilename [options]\n" );
    fprintf( stderr, "Options:\n" );
    fprintf( stderr, "\tSTART:k     - start from SDF record #k\n" );
    fprintf( stderr, "\tEND:k       - end at SDF record #k\n" );
    fprintf( stderr, "\tRECORD:k    - process only SDF record #k\n" );
    fprintf( stderr, "\tTHREADS:n   - run n threads\n" );
    fprintf( stderr, "\tPERTHREAD:m - run m tasks per thread (each task treats one record)\n" );
    fprintf( stderr, "\tNOINCHI     - do not print InChI itself\n" );
    fprintf( stderr, "\tKEY         - calc and print InChIKey\n" );
    fprintf( stderr, "\tPOLYMERS    - treat polymers (experimental)\n" );
    fprintf( stderr, "\tVERBOSE:n   - > 0 means more output\n" );
    fprintf( stderr, "\t[common InChI API options]\n" );
}


/*******************/
/* m2i_WorkDetails */
/*******************/

/****************************************************************************/
int m2i_WorkDetails_init( m2i_WorkDetails *wd, int argc, char *argv[] )
{
    int i;
    char *option1 = NULL;

    if (!wd)
    {
        return 0;
    }

    memset( wd, 0, sizeof( *wd ) );

    wd->n_workers = 1;
    wd->n_tasks_per_worker = 1;
    wd->verbose = 1;
    wd->rstart = 1;
    wd->rend = 999999999;

    /* Limit calculations by 60 sec */
#ifdef _WIN32
    strcpy( wd->options, "/W60" );
#else
    strcpy( wd->options, "-W60" );
#endif

    for (i = 2; i < argc; i++)
    {
        option1 = argv[i] + 1;
        /* mol2inchi own switches */
        if (!own_stricmp( option1, "NOINCHI" ))
        {
            wd->do_not_print_inchi = 1;
        }
        else if (!own_memicmp( option1, "VERBOSE:", 8 ))
        {
            wd->verbose = strtol( option1 + 8, NULL, 10 );
            if (wd->verbose < 0)
            {
                wd->verbose = 0;
            }
        }
        else if (!own_memicmp( option1, "START:", 6 ))
        {
            wd->rstart = strtol( option1 + 6, NULL, 10 );
            if (wd->rstart < 1)
            {
                wd->rstart = 1;
            }
        }
        else if (!own_memicmp( option1, "THREADS:", 8 ))
        {
            wd->n_workers = strtol( option1 + 8, NULL, 10 );
            if (wd->n_workers < 1)
            {
                wd->n_workers = 1;
            }
            if (wd->n_workers > MAX_WORKERS)
            {
                wd->n_workers = MAX_WORKERS;
                fprintf(stderr, "Too high number of threads requested, use %-d\n", MAX_WORKERS);
            }
        }
        else if (!own_memicmp( option1, "PERTHREAD:", 10 ))
        {
            wd->n_tasks_per_worker = strtol( option1 + 10, NULL, 10 );
            if (wd->n_tasks_per_worker < 1)
            {
                wd->n_tasks_per_worker = 1;
            }
            if (wd->n_tasks_per_worker > MAX_SLOTS)
            {
                wd->n_tasks_per_worker = MAX_SLOTS;
                fprintf(stderr, "Too high number of slots per thread requested, use %-d\n", MAX_SLOTS);
            }
        }
        else if (!own_memicmp( option1, "END:", 4 ))
        {
            wd->rend = strtol( option1 + 4, NULL, 10 );
            if (wd->rend < 1)
            {
                wd->rend = 1;
            }
            if (wd->rend < wd->rstart)
            {
                wd->rend = wd->rstart;
            }
        }
        else if (!own_memicmp( option1, "RECORD:", 7 ))
        {
            int num = strtol( option1 + 7, NULL, 10 );
            wd->rstart = wd->rend = num;
        }
        else
        {
            if (!own_stricmp( option1, "KEY" ))
            {
                wd->get_inchikey = 1;
            }
            /* other switches to be passed to inchi-calc worker */
            strcat( wd->options, " " );
            strcat( wd->options, argv[i] );
            if (!own_stricmp( option1, "LARGEMOLECULES" ))
            {
                wd->large_mols = 1;
            }
        }
    }

    return 1;
}


/***********************/
/* m2i_WorkPool	object */
/***********************/

/****************************************************************************/
int m2i_WorkPool_init( m2i_WorkPool *pool, m2i_WorkDetails *wd )
{
    int ret = 1;
    long w;

    memset( pool, 0, sizeof( *pool ) );
    pool->n_total_workers = wd->n_workers;
    pool->workers = (m2i_Worker *) calloc( pool->n_total_workers, sizeof( m2i_Worker ) );

    if (!pool->workers)
    {
        fprintf(stderr, "Memory alloc fail %-s#%-d\n", __FILE__, __LINE__);
        return 0;
    }

    for (w = 0; w < pool->n_total_workers; w++)
    {
        ret = m2i_Worker_init( &pool->workers[w], wd );
        if (!ret)
        {
            fprintf(stderr, "m2i_Worker_init() returned error %-s#%-d\n", __FILE__, __LINE__);
            return 0;
        }
    }

    return 1;
}


/****************************************************************************/
void m2i_WorkPool_close( m2i_WorkPool *pool )
{
    long w;
    for (w = 0; w < pool->n_total_workers; w++)
    {
        m2i_Worker_close( &pool->workers[w] );
    }
    free( pool->workers );
}


/****************************************************************************/
void m2i_WorkPool_reset( m2i_WorkPool *pool )   /* do not touch already allocd pointers to workers */
{
    long w;
    for (w = 0; w < pool->n_total_workers; w++)
    {
        m2i_Worker_reset( &pool->workers[w] );
    }
    pool->n_ready_workers = 0;
}


/*********************/
/* m2i_Worker object */
/*********************/

/****************************************************************************/
int m2i_Worker_init( m2i_Worker *w, m2i_WorkDetails *wd )
{
    int ret = 1;
    long t;

    memset( w, 0, sizeof( *w ) );
    w->n_total_tasks = wd->n_tasks_per_worker;
    w->n_ready_tasks = 0;
    w->tasks = (m2i_Task *) calloc( w->n_total_tasks, sizeof( m2i_Task ) );
    if (!w->tasks)
    {
        fprintf(stderr, "Memory alloc fail %-s#%-d\n", __FILE__, __LINE__);
        return 0;
    }
    for (t = 0; t < w->n_total_tasks; t++)
    {
        ret = m2i_Task_init( &( w->tasks[t] ), wd );
        if (!ret)
        {
            return 0;
        }
    }

    return 1;
}


/****************************************************************************/
void m2i_Worker_reset( m2i_Worker *w )  /* do not touch already allocd pointers to tasks */
{
    long t;
    for (t = 0; t < w->n_total_tasks; t++)
    {
        m2i_Task_reset( &( w->tasks[t] ) );
    }
    w->n_ready_tasks = 0;
}


/****************************************************************************/
void m2i_Worker_close( m2i_Worker *w )
{
    long t;
    for (t = 0; t < w->n_total_tasks; t++)
    {
        m2i_Task_close( &( w->tasks[t] ) );
    }
    free( w->tasks );
}


/*******************/
/* m2i_Task object */
/*******************/


/****************************************************************************/
void m2i_Task_reset( m2i_Task *task )
{
    GrowStr_reset( &task->in_buf );
    GrowStr_reset( &task->output );
    GrowStr_reset( &task->errstr );
}


/****************************************************************************/
void m2i_Task_close( m2i_Task *task )
{
    GrowStr_close( &task->in_buf );
    GrowStr_close( &task->output );
    GrowStr_close( &task->errstr );
}


/****************************************************************************/
int  m2i_Task_init( m2i_Task *task, m2i_WorkDetails *wd )
{
    int ret;
    int bufsize0 = 4096; /*16384*/
    int bufsize1 = 1024;
    int bufsize2 = 128;

    memset( task, 0, sizeof( *task ) );

    task->index = -1;
    task->retcode = 0;
    task->nmol = -1;
    task->wd = wd;

    if (wd->large_mols)
    {
        bufsize0 = 32768; /*262144*/
        bufsize1 = 4096;
    }
    ret = GrowStr_init( &task->in_buf, bufsize0, bufsize0 );
    if (!ret || !task->in_buf.ps || !task->in_buf.allocated)
    {
        return 0;
    }
    ret = GrowStr_init( &task->output, bufsize1, bufsize1 );
    if (!ret || !task->output.ps || !task->output.allocated)
    {
        return 0;
    }
    ret = GrowStr_init( &task->errstr, bufsize2, bufsize2 );
    if (!ret || !task->errstr.ps || !task->errstr.allocated)
    {
        return 0;
    }

    return 1;
}
