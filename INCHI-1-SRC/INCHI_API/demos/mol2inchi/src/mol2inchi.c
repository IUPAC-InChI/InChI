/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.05
 * January 27, 2017
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
 * Copyright (C) IUPAC and InChI Trust Limited
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
 * Licence No. 1.0 with this library; if not, please write to:
 *
 * The InChI Trust
 * 8 Cavendish Avenue
 * Cambridge CB1 7US
 * UK
 *
 * or e-mail to alan@inchi-trust.org
 *
 */


#pragma warning( disable : 4996 )

#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

#include <time.h>

#include <omp.h>

#include "../../../../INCHI_BASE/src/inchi_api.h"
#include "mol2inchi.h"
#include "moreitil.h"

#ifdef BUILD_WITH_ENG_OPTIONS
#include "shuffler.h"
/* local functions */
int extract_MOL_counts_and_version(char *str, M2I_NUM *num_at, M2I_NUM *num_bo);
int extract_counts_from_MOL_V3000(char *str, M2I_NUM *num_at, M2I_NUM *num_bo);
int extract_name_from_MOL(char *str, char *name, size_t max_symbols);
double getCPUTime(void);
#endif





/****************************************************************************/
int main(int argc, char *argv[])
{
int retcode=0, result=0, nerrs=0, k;
char *fname=NULL;
FILE *f=NULL;
char *text=NULL;

char *option1;
char *out=NULL, *log=NULL;
struct WorkerDetails wd;

time_t begin;

#if defined(_WIN32)
const char *platform="Windows";
#else
const char *platform="Linux";
#endif

    char banner[255];
    sprintf( banner, "%s\n%-s Build of %-s %-s%s\n",
              APP_DESCRIPTION,
              platform, __DATE__, __TIME__,
              RELEASE_IS_FINAL?"":" *** pre-release, for evaluation only ***");

    /* Parse command line
        assume that the first item is input filename,
        then mol2ichi own switches, then  all the others
        which should be passed to InChI calculation algorithm
    */

    memset( &wd, 0, sizeof(wd) );

    fprintf( stderr, "%-s\n", banner);
    if ( argc < 2 )
    {
        print_help();
        retcode = 1;
        goto finish;
    }

    fprintf( stderr, "Started at ");
    print_time();
    ctime(&begin);

    /* Set worker details */
    wd.get_inchikey = 0;
    wd.do_not_print_inchi = 0;
    wd.output_error_inchi = 0;
    wd.do_benchmark = 0;
    wd.nperms = 0;
    wd.tmax_shuffle = 0;
    wd.dtmax_shuffle = 0.0;
    wd.verbose = 1;
    wd.out = NULL;
    wd.log = NULL;
    wd.rstart = 1;
    wd.rend = 999999999;
    wd.nmol = 0;
    wd.n_inchi_err = 0;

    /* limit calculations by 60 sec */
#ifdef _WIN32
    strcpy(wd.options, "/W60");
#else
    strcpy(wd.options, "-W60");
#endif

    fname = argv[1];
    for(k=2; k<argc; k++)
    {
        option1 =  argv[k]+1;
        /* Treat mol2inchi own switches */
        if ( !own_stricmp(option1, "NOINCHI") )
        {
            wd.do_not_print_inchi = 1;
        }
#ifdef BUILD_WITH_ENG_OPTIONS
        else if ( !own_stricmp(option1, "BENCHMARK") )
        {
            wd.do_benchmark = 1;
        }
        else if ( !own_memicmp( option1, "NSHUFFLE:", 9 ) )
        {
            wd.nperms = strtol(option1+9, NULL, 10);
            if ( wd.nperms < 0 )
                wd.nperms = 0;
        }
        else if ( !own_memicmp( option1, "TSHUFFLE:", 9 ) )
        {
            wd.tmax_shuffle = strtol(option1+9, NULL, 10);
            if ( wd.tmax_shuffle < 0 )
                wd.tmax_shuffle = 0; /* no time limit for shuffling */
            wd.dtmax_shuffle = (double) wd.tmax_shuffle;
        }
        else if ( !own_memicmp( option1, "VERBOSE:", 8 ) )
        {
            wd.verbose = strtol(option1+8, NULL, 10);
            if ( wd.verbose < 0 )
                wd.verbose = 0;
        }
#endif
        else if ( !own_memicmp( option1, "START:", 6 ) )
        {
            wd.rstart = strtol(option1+6, NULL, 10);
            if ( wd.rstart < 1 )
                wd.rstart = 1;
        }
        else if ( !own_memicmp( option1, "END:", 4 ) )
        {
            wd.rend = strtol(option1+4, NULL, 10);
            if ( wd.rend < 1 )
                wd.rend = 1;
            if ( wd.rend < wd.rstart )
                wd.rend = wd.rstart;
        }
        else if ( !own_memicmp( option1, "RECORD:", 7 ) )
        {
            int num = strtol(option1+7, NULL, 10);
            wd.rstart = wd.rend = num;
        }
        else
        {
            if ( !own_stricmp(option1, "KEY") )
            {
                wd.get_inchikey = 1;
            }
            /* other switches to be passed to inchi-calc worker */
            strcat( wd.options, " " );
            strcat( wd.options, argv[k] );
        }
    }

    fprintf( stderr, "by the following command line:\n\"");
    for(k=0; k<argc-1; k++)
        fprintf( stderr, "%-s ",argv[k]);
    fprintf( stderr, "%-s\"\n", argv[argc-1]);

    f = fopen(fname,"rb");
    if ( !f )
    {
        retcode = 2;
        goto finish;
    }

    /* Warning: in this demo, we assume that molfile's size does not    */
    /* exceed some large MOLBUFSIZE; be (more) careful for production.    */
    text = (char *) calloc( MOLBUFSIZE, sizeof(char) );
    if ( !text )
    {
        retcode = 3;
        goto finish;
    }
    fprintf( stderr, "\n" );


    /* Main loop */

    while ( get_next_molfile_as_text( f, text, MOLBUFSIZE ) > 0 )
    {
        wd.nmol++;
        if ( wd.nmol < wd.rstart )
            continue;
        if ( wd.nmol > wd.rend )
            break;
        nerrs = work_on_molfile_text ( text, &wd );
    }


finish:

    if ( retcode==0 )
        fprintf( stderr, "\nFinished OK at ");
    else
        fprintf( stderr, "\nFinished with exit code %-d at ", retcode);

    print_time();

    fprintf( stderr, "Processed %lu structure(s) with %-ld error(s).\n",
                        wd.nmol, wd.n_inchi_err );
    if ( text )
        free( text );

    return retcode;
}



/*
    Process one MOLFile text

    Optionally organize benchmarking and
    repeat calculations with shuffled atoms
*/
int work_on_molfile_text( const char* text, struct WorkerDetails * wd)
{
int res=0, nerrs=0, shuffle_atoms=0;
int lenname=0, max_mname_symbols=79;
long irepeat=0;
double startTime=0.0;
double run_duration=0.0;
int nrepeats=0;
int have_problems=0;

#ifdef BUILD_WITH_ENG_OPTIONS
M2I_NUM numbers[PERMAXATOMS];
M2I_NUM nat = -1, nbonds = -1;
char mname[80];
MFLines *mfdata=NULL;
long i;
int version;
#endif

/* Warning: in this demo, we assume that printout (InChI, etc.)    */
/* does not exceed some large INCHIPRINTOUTSIZE                    */
char output0[INCHIPRINTOUTSIZE], errstr0[4096];
char ikey0[28];

    ikey0[0] = output0[0] = errstr0[0] = '\0';

#ifdef BUILD_WITH_ENG_OPTIONS
    /* Get info for printout */
    lenname = extract_name_from_MOL( (char *) text, mname, max_mname_symbols);
    if ( !lenname )
        strcpy(mname, "");
    if ( wd->do_benchmark )
    {
        startTime = getCPUTime( );
    }
#endif

    /* Perform a single run */
    nerrs = run_once_on_molfile_text( text, wd, ikey0, output0, errstr0);

    if ( errstr0[0] )
        fprintf( stderr, "%-s\n", errstr0);

#ifndef BUILD_WITH_ENG_OPTIONS
    fprintf( stdout, "%-s", output0 );
    fprintf( stdout, "\n");
    fflush(NULL);
#else
    /* have_problems = extract_counts_from_MOL_V3000( (char *) text, &nat, &nbonds);                 */
    version = extract_MOL_counts_and_version( (char *) text, &nat, &nbonds);
    have_problems = (version!=2000) && (version!=3000);

    if ( wd->do_benchmark )
    {
        run_duration = getCPUTime( ) - startTime;
        if ( wd->nperms > 0 )
        {
            /* Permutations requested */
            nrepeats = wd->nperms;

            if ( nrepeats > 0 )
            {
                if ( version!=3000 || nat<1 || nat>PERMAXATOMS )
                {
                    nrepeats = 0;
                    if ( wd->verbose >= 1 )
                        fprintf( stderr,
                                 "No shuffle for  struct #%-ld: not V3000 data file or too many atoms (code %-d).\n",
                                 wd->nmol, have_problems );
                }
            }

            if ( nrepeats > 0 && wd->tmax_shuffle > 0 )
            {
                /* Check single-run timing... */
                if ( run_duration >= wd->dtmax_shuffle )
                {
                    /* Single-run duration exceeds treshold, do not repeat */
                    nrepeats = 0;
                    if ( wd->verbose >= 1 )
                        fprintf( stderr, "No shuffle for  struct #%-ld: too long run.\n", wd->nmol );
                }
            }
        }
    }    /* if ( wd->do_benchmark ) */

    fprintf( stdout, "%-s\t%-s\t", output0, mname );

    if ( wd->do_benchmark )
    {
        fprintf( stdout, "#%-ld\t%-d\t%-d\t%-lf",
                    irepeat, nat, nbonds,run_duration );
        if ( ikey0[0] )
            fprintf( stdout, "%-s", (nrepeats < 1)?"\t---":"\t***" );
    }
    fprintf( stdout, "\n");
    fflush(NULL);


    if ( nerrs > 0 )
    {
        /* Failed, nothing to do here anymore */
        return nerrs;
    }

    if ( nrepeats < 1 )
        return 0;

    /* Repeat nrepeat times */

    /* Prepare data structs for repeated calulations */
    for (i=0; i<nat; i++)
        numbers[i] = i;

    mfdata = new_mflines( (char *) text, nat, nbonds );

/*#pragma omp parallel num_threads(4)*/
#ifdef _OPENMP
    {
#ifdef WIN32
        int maxcores=3;    /* laptop */
#else
        int maxcores=5;    /* server */
#endif
        int nthreads = ( nrepeats <= maxcores ) ? nrepeats : maxcores;
        omp_set_num_threads( nthreads );
        fprintf( stderr, "Using OpenMP with %-d threads\n", ( nrepeats <= maxcores ) ? nrepeats : maxcores );
    }
#endif


#pragma omp parallel for firstprivate( numbers ) private(startTime)
    for ( irepeat=1; irepeat<nrepeats+1; irepeat++)
    {
        int res1;
        char *mftext=NULL;
        char ikey[28];
        /* Warning: in this demo, we just assume that printout (InChI, etc.)    */
        /* does not exceed some large INCHIPRINTOUTSIZE; be (more) careful.                        */
        char output[INCHIPRINTOUTSIZE], errstr[4096];
        output[0] = errstr[0] = ikey[0] = '\0';

#ifdef _OPENMP
        /* Make sure that different threads go with different rand */
        srand( (unsigned int) (time(NULL)) ^ omp_get_thread_num());
#else
        srand( (unsigned int) (time(NULL)) );
#endif

        /* Re-order the atoms and make new 'Molfile as text' */
        shuffle( (void *) numbers, (M2I_NUM) nat, sizeof(M2I_NUM) );
        mftext = shuffled_mflines( mfdata, numbers );
        if ( NULL==mftext )
            continue;

        /* debug printout */
        if ( wd->verbose >= 1 )
        {
            if ( wd->verbose >= 2 )
#pragma omp critical
            {
                fprintf( stderr, "%-s perm#%-.4ld { %-d", mname, irepeat, numbers[0]);
                for (i=1; i<nat-1; i++) fprintf( stderr, ", %-d", numbers[i]);
                    fprintf( stderr, ", %-d }\n", numbers[nat-1]);
                if ( wd->verbose >= 10 )
                {
                    fprintf( stderr, "\n\n%-s\n\n", mftext);
                }
            }
        }

        if ( wd->do_benchmark )
            startTime = getCPUTime( );


        res1 = run_once_on_molfile_text( (const char *) mftext, wd, ikey, output, errstr);


        if ( wd->do_benchmark )
        {
            run_duration = getCPUTime( ) - startTime;
        }
        if ( errstr[0] )
#pragma omp critical
        {
            if ( wd->verbose >= 1 )
#ifdef _OPENMP
                fprintf( stderr, "(%-s) [thread %-d] \n", errstr, omp_get_thread_num());
#else
                fprintf( stderr, "(%-s)\n", errstr );
#endif
        }

        if ( wd->do_benchmark )
#pragma omp critical
        {
            {
                fprintf( stdout, "%-s\t%-s\t#%-ld\t%-d\t%-d\t%-lf",
                    output, mname, irepeat, nat, nbonds,run_duration );
                if ( ikey0[0] && ikey[0] )
                    fprintf( stdout, "%-s",
                                    strcmp(ikey,ikey0)?"\t!!!":"\t..." );
                fprintf( stdout, "\n");
                fflush(NULL);
            }
        }

        if ( mftext )
            free( mftext );
    }


    if ( mfdata )
        del_mflines( mfdata );

#endif

    return 0;
}


/* Actual InChI calculation worker    */
int run_once_on_molfile_text( const char* mftext,
                              struct WorkerDetails* wd,
                              char *ikey,
                              char *output,
                              char *errstr)
{
int res=0, errs=0;
inchi_Output iout, *result = &iout;
const char *delim = "\n";

    memset( result, 0, sizeof(*result) );

    /* Call API function to calculate InChI */
    res = MakeINCHIFromMolfileText( mftext, wd->options, result );

    if ( res==mol2inchi_Ret_ERROR        ||
         res==mol2inchi_Ret_ERROR_get    ||
         res==mol2inchi_Ret_ERROR_comp    )
    {
        errs++;
        wd->n_inchi_err++;
        sprintf( errstr, "Error " );
    }
    else if ( res==mol2inchi_Ret_WARNING )
    {
        sprintf( errstr, "Warning " );
    }
    if ( result->szLog && result->szLog[0] )
        sprintf( errstr,
                 "%-s structure #%-lu",
                 result->szLog,  wd->nmol );

    /* Print InChI and other stuff if InChI is not empty */
    if ( result->szInChI && result->szInChI[0] )
    {
        /* Calculate and print InChIKey */
        if ( wd->get_inchikey )
        {
            int ik_ret=0;           /* InChIKey-calc result code */
            int xhash1, xhash2;
            char szXtra1[256], szXtra2[256];
            xhash1 = xhash2 = 0;

            ik_ret = GetINCHIKeyFromINCHI( result->szInChI,
                                           xhash1,
                                           xhash2,
                                           ikey,
                                           szXtra1,
                                           szXtra2);
            if (ik_ret!=INCHIKEY_OK)
                ikey[0]='\0';
        }

        sprintf(output, "Structure: %-lu%s%-s%s%-s%s%-s",
                wd->nmol,
                wd->do_not_print_inchi ? "" : delim,
                wd->do_not_print_inchi ? "" : result->szInChI,
                ikey[0] ? delim : "",
                ikey[0] ? ikey : "",
                result->szAuxInfo && result->szAuxInfo[0] ? delim : "",
                result->szAuxInfo && result->szAuxInfo[0] ? result->szAuxInfo : ""
            );
    }

    /* Reset output data structure */
    FreeINCHI ( result );

    return errs;
}



/*
    Print program usage instructions
*/
void print_help(void)
{
    fprintf( stderr, "Usage: \n");
    fprintf( stderr, "mol2inchi inputfilename [options]\n");
    fprintf( stderr, "Options:\n");
    fprintf( stderr, "\tSTART:n - start from SDF record n\n");
    fprintf( stderr, "\tEND:n   - end when reached SDF record n\n");
    fprintf( stderr, "\tRECORD:n   - process only SDF record n\n");
    fprintf( stderr, "\tNOINCHI - do not print InChI itself\n");
    fprintf( stderr, "\tKEY     - calc and print InChIKey\n");
    fprintf( stderr, "\tPOLYMERS - treat polymers (experimental)\n");
#ifdef BUILD_WITH_ENG_OPTIONS
    fprintf( stderr, "\tBENCHMARK - collect timing\n");
    fprintf( stderr, "\tNSHUFFLE:n - reorder atoms n times and repeat\n");
    fprintf( stderr, "\tTSHUFFLE:t - do not repeat runs of t sec or longer\n");
    fprintf( stderr, "\tVERBOSE:n  - > 0 means more output\n");
#endif
    fprintf( stderr, "\t[common InChI API options]\n");
}


#ifdef BUILD_WITH_ENG_OPTIONS
/******************************************************************/
int extract_name_from_MOL(char *str, char *name, size_t max_symbols)
{
char *p;
size_t ncopied;
#define CNAMEL 4096
char buf[CNAMEL];

    if (str==NULL)
        return -1;
    if (strlen(str)<1)
        return -1;

    p = get_substr_in_between(str, "", "\n", buf, CNAMEL-2, &ncopied);

    if ( ncopied )
    {
        if ( ncopied > max_symbols )
            ncopied = max_symbols;
        memcpy( name, buf, ncopied );
        name[ncopied++]='\0';
    }

    return (int) ncopied;
}


/***************************************************************************/
int extract_counts_from_MOL_V3000(char *str, M2I_NUM *num_at, M2I_NUM *num_bo)
{
size_t n, ncopied;
char *p, *pp, *next;
#define CBUFL 16
char buf[CBUFL];


    *num_at = *num_bo = -1;

    if (str==NULL)
        return -1;
    if (strlen(str)<1)
        return -2;

    p = str;

    n = 5;
    while (n--)
        while ((++p)[0]!='\n') ;
    ++p;

    next = get_substr_in_between(p, "M  V30 COUNTS ", " ", buf, CBUFL-2, &ncopied);

    if ( !ncopied )
        return -3;
    if ( !strlen(buf) )
        return -4;

    *num_at = (M2I_NUM ) strtol(buf, NULL, 10);
    if ( *num_at < 1)
        return -5;

    pp = p + strlen("M  V30 COUNTS ") + strlen(buf) +1;
    buf[0] = '\0';
    next = get_substr_in_between(pp, "", " ", buf, CBUFL-2, &ncopied);
    if ( ncopied )
        if ( strlen(buf) )
            *num_bo = (M2I_NUM ) strtol(buf, NULL, 10);

    return 0;
}



/***************************************************************************/
int extract_MOL_counts_and_version(char *str, M2I_NUM *num_at, M2I_NUM *num_bo)
{
size_t n, ncopied;
char *p, *pp, *next;
#define CBUFL 16
char buf[CBUFL];
int version;
char *p2000=NULL;

    version = *num_at = *num_bo = -1;

    if (str==NULL)
        return -1;
    if (strlen(str)<1)
        return -2;

    p = str;

    n = 3;
    while (n--)
        while ((++p)[0]!='\n') ;
    ++p;

    p2000 = strstr(p, "V2000");
    if ( p2000)
    {
        int k;
        char sna[4], snb[4];
        if ( strlen(p) < 6 )
            return version;
        for (k=0; k<3; k++)
        {
            sna[k] = p[k];
            snb[k] = p[k+3];
        }
        sna[3] = snb[3] = '\0';
        *num_at = (M2I_NUM ) strtol(sna, NULL, 10);
        if ( *num_at < 1)
            return -5;
        *num_bo = (M2I_NUM ) strtol(snb, NULL, 10);
        version = 2000;
        return version;
    }

    version = 3000;
    p = str;
    n = 5;
    while (n--)
        while ((++p)[0]!='\n') ;
    ++p;

    next = get_substr_in_between(p, "M  V30 COUNTS ", " ", buf, CBUFL-2, &ncopied);

    if ( !ncopied )
        return -3;
    if ( !strlen(buf) )
        return -4;

    *num_at = (M2I_NUM ) strtol(buf, NULL, 10);
    if ( *num_at < 1)
        return -5;

    pp = p + strlen("M  V30 COUNTS ") + strlen(buf) +1;
    buf[0] = '\0';
    next = get_substr_in_between(pp, "", " ", buf, CBUFL-2, &ncopied);
    if ( ncopied )
        if ( strlen(buf) )
            *num_bo = (M2I_NUM ) strtol(buf, NULL, 10);

    return version;
}
#endif
