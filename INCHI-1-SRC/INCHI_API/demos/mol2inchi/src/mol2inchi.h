/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.07
 * April 30, 2024
 *
 * MIT License
 *
 * Copyright (c) 2024 IUPAC and InChI Trust
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * info@inchi-trust.org
 *
 */

#ifndef _MOL2INCHI_H_
#define _MOL2INCHI_H_

/*#define BUILD_WITH_ENG_OPTIONS 1*/

/*#define USE_TBB_MALLOC 1*/
#ifdef USE_TBB_MALLOC
#include "../../../tbb/scalable_allocator.h"
#include "../../../tbb/tbbmalloc_proxy_for_inchi.h"
#endif


#ifdef APP_DESCRIPTION
#undef APP_DESCRIPTION
#endif
#define APP_DESCRIPTION "InChI version 1, Software v. 1.06 (mol2inchi - Library call example, API v. 1.06)"

/*#define RELEASE_IS_FINAL  0*/ /* 1=> pre-release version; comment out to disable */
#ifndef RELEASE_IS_FINAL
#define RELEASE_IS_FINAL    1           /* final release */
#endif


#ifndef INCHI_BUILD_PLATFORM

#if defined(_WIN32)

#if defined(_WIN64)
#define INCHI_BUILD_PLATFORM "Windows 64-bit"
#else
#define INCHI_BUILD_PLATFORM "Windows 32-bit"
#endif

#elif defined(__linux__)

#if defined(__x86_64__)||defined(__ppc64__)
#define INCHI_BUILD_PLATFORM "Linux 64-bit"
#else
#define INCHI_BUILD_PLATFORM "Linux 32-bit"
#endif

#elif defined(__APPLE__)
#define INCHI_BUILD_PLATFORM "OSX"

#else
#define INCHI_BUILD_PLATFORM ""
#endif
#endif

#ifndef INCHI_BUILD_DEBUG
#if defined(_DEBUG)
#define INCHI_BUILD_DEBUG " Debug"
#else
#define INCHI_BUILD_DEBUG ""
#endif
#endif

#if defined(_MSC_VER)

#if _MSC_VER > 1900
#define INCHI_BUILD_COMPILER "MS VS 2017 or later"
#elif _MSC_VER == 1900
#define INCHI_BUILD_COMPILER "MS VS 2015"
#elif _MSC_VER == 1800
#define INCHI_BUILD_COMPILER "MS VS 2013"
#elif _MSC_VER == 1700
#define INCHI_BUILD_COMPILER "MS VS 2012"
#elif _MSC_VER == 1600
#define INCHI_BUILD_COMPILER "MS VS 2010"
#elif _MSC_VER == 1500
#define INCHI_BUILD_COMPILER "MS VS 2008"
#elif _MSC_VER == 1400
#define INCHI_BUILD_COMPILER "MS VS 2005"
#elif _MSC_VER == 1310
#define INCHI_BUILD_COMPILER "MS VS 2003"
#elif _MSC_VER == 1300
#define INCHI_BUILD_COMPILER "MS VS 2002"
#elif _MSC_VER == 1200
#define INCHI_BUILD_COMPILER "MS VS 6.0"
#else
#define INCHI_BUILD_COMPILER "MS VC++ 5.0 or earlier"
#endif

#else

#if defined( __GNUC__)
#define INCHI_BUILD_COMPILER "gcc " __VERSION__ ""
#else
#define INCHI_BUILD_COMPILER ""
#endif
#endif


#define INCHI_SRC_REV ""
                                
/*#define DEBUG_MOL2INCHI 1*/



#define MAX_WORKERS       1024
#define MAX_SLOTS         4096 /*1024 */

#ifdef WIN32
#define THREAD_PTR HANDLE
#else
#define THREAD_PTR pthread_t
#endif

typedef enum M2I_ERROR
{
    M2I_ERR_COMMAND_LINE,
    M2I_ERR_SET_JOB_OPTS,
    M2I_ERR_OPEN_INPUT,
    M2I_ERR_WORKPOOL_INIT,
    M2I_ERR_PTHREADS_START
}
M2I_ERROR;


/*
    General layout
    --------------
    Whole job
             pool of worker threads
                                    worker thread 1
                                                   task 1 (process single mol/record)
                                                   ...
                                                   task m
                                   ...
                                   worker thread n
                                                   ...
*/


/* Whole-job-wide options */
typedef struct m2i_WorkDetails
{
    int    verbose;
    long   rstart;
    long   rend;
    long   nmol;
    long   n_inchi_err;
    /* threads */
    long int    n_workers;
    long int    n_tasks_per_worker;
    /* InChI related */
    int    do_not_print_inchi;
    int    get_inchikey;
    int    large_mols;
    int    output_error_inchi;
    char   options[1024];           /* common ('old') API options */
} m2i_WorkDetails;

int m2i_WorkDetails_init( m2i_WorkDetails *wd, int argc, char *argv[] );


/* Single-mol processing task */
typedef struct m2i_Task
{
    long            index;
    long            nmol;
    int             retcode;
    GrowStr         in_buf;
    GrowStr         output;
    GrowStr         errstr;
    char            ikey[28];
    char            szXtra1[256];
    char            szXtra2[256];
    m2i_WorkDetails *wd;
}
m2i_Task;

void m2i_Task_reset( m2i_Task *task );
void m2i_Task_close( m2i_Task *task );
int  m2i_Task_init( m2i_Task *task, m2i_WorkDetails *wd );


/* Single worker thread */
typedef struct m2i_Worker
{
    long n_total_tasks;
    long n_ready_tasks;
    m2i_Task *tasks;
}
m2i_Worker;

int m2i_Worker_init( m2i_Worker *w, m2i_WorkDetails *wd );
void m2i_Worker_close( m2i_Worker *w );
void m2i_Worker_reset( m2i_Worker *w );

/* Pool of worker threads */
typedef struct m2i_WorkPool
{
    long  n_total_workers;
    long n_ready_workers;
    m2i_Worker *workers;
}
m2i_WorkPool;

int  m2i_WorkPool_init( m2i_WorkPool *pool, m2i_WorkDetails *wd );
void m2i_WorkPool_close( m2i_WorkPool *pool );
void m2i_WorkPool_reset( m2i_WorkPool *pool );
int  m2i_WorkPool_wait_and_print_all( m2i_WorkPool *pool, THREAD_PTR *Thread );

#ifdef _WIN32
#define M2I_THREADFUNC unsigned __stdcall
#else
#define M2I_THREADFUNC void *
#endif

M2I_THREADFUNC m2i_Worker_run( void *arg );

void print_help( void );
int CalcPrintINCHIKEY( char *INCHI, char *ik_string );

#endif /* _MOL2INCHI_H_ */
