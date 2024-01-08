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


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Demo caller using modularized library interface (see main() below)

Activated if (CREATE_INCHI_STEP_BY_STEP==1) [e_mode.h]

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/




#if( defined( WIN32 ) && defined( _CONSOLE ) && defined(_MSC_VER) && _MSC_VER >= 800 )
#define ADD_WIN_CTLC   /* detect Ctrl-C under Win-32 and Microsoft Visual C ++ */
#endif

#if( defined( WIN32 ) && defined( _CONSOLE ) && defined(_MSC_VER) && _MSC_VER >= 800 && defined(ADD_WIN_CTLC) && !(defined(__STDC__) && __STDC__ == 1) )
#include <windows.h>
#endif


/* Uncomment to replace coordinates and 2D-parirties with 0D-parities */
/* #define CREATE_0D_PARITIES */
/*  in case of CREATE_0D_PARITIES, the hardcoded bFixSp3Bug = 1 fixes sp3 bugs in original InChI v. 1.00  */
/*  in case of CREATE_0D_PARITIES, the Phosphine and Arsine sp3 stereo options are not supported */

/* Comment out to include each bond in both neighboring atoms adjacency lists */
#define NEIGH_ONLY_ONCE


#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <time.h>

#include "e_mode.h"
#ifdef CREATE_INCHI_STEP_BY_STEP

#include "e_ctl_data.h"

#include "../../../../INCHI_BASE/src/inchi_api.h"


#include "e_inchi_atom.h"
#include "e_util.h"
#include "e_ichi_io.h"
#include "e_ichierr.h"
#include "e_readstru.h"
#include "e_ichicomp.h"
#ifdef CREATE_0D_PARITIES
#include "e_0dstereo.h"
#endif


/* External (from former e_ichi_parms.h) */
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif
    int e_ReadCommandLineParms( int argc, const char *argv[], INPUT_PARMS *ip,
        char *szSdfDataValue, unsigned long *ulDisplTime,
        int bReleaseVersion, INCHI_IOSTREAM *log_file );
    int e_PrintInputParms( INCHI_IOSTREAM *log_file, INPUT_PARMS *ip );
    int e_OpenFiles( FILE **inp_file, FILE **output_file, FILE **log_file, FILE **prb_file, INPUT_PARMS *ip );
    void e_HelpCommandLineParms( INCHI_IOSTREAM *f );
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

/* Local */
int e_MakeOutputHeader( char *pSdfLabel, char *pSdfValue, long lSdfId, long num_inp, char *pStr1, char *pStr2 );
int e_set_inchi_input_by_extended_inchi_input(inchi_Input *pInp, inchi_InputEx *pInpEx);



/*  Console-specific */

/*^^^ Ctrl+C trap; works under Win32 + MS VC++ */
int e_bInterrupted = 0;
int stop = 0;
#if( defined( _WIN32 ) && defined( _CONSOLE ) )
#if( defined(_MSC_VER) && _MSC_VER >= 800 && defined(ADD_WIN_CTLC) && !(defined(__STDC__) && __STDC__ == 1) )
BOOL WINAPI MyHandlerRoutine( DWORD dwCtrlType )   /*   control signal type */
{
    if (dwCtrlType == CTRL_C_EVENT ||
         dwCtrlType == CTRL_BREAK_EVENT ||
         dwCtrlType == CTRL_CLOSE_EVENT ||
         dwCtrlType == CTRL_LOGOFF_EVENT)
    {
        e_bInterrupted = 1;
        return TRUE;
    }
    return FALSE;
}
#endif
int e_WasInterrupted( void )
{
#ifdef _DEBUG
    if (e_bInterrupted)
    {
        stop = 1;  /*  for debug only <BRKPT> */
    }
#endif
    return e_bInterrupted;
}

#endif


#define EXIT_ON_ERR





/****************************************************************************

main(): caller program demonstrating 'another InChI library interface'
        and InChIKey calculation

****************************************************************************/
int main( int argc, char *argv[] )
{
#define nStrLen 256
    int bReleaseVersion = bRELEASE_VERSION;

    char    szSdfDataValue[MAX_SDF_VALUE + 1];
    char    szInchiCmdLine[512];
    char    pStrInchiId[nStrLen], pStrLogId[nStrLen];
    const   char *p1, *p2;


    int   retcode = 0, retcode1 = 0, i, k, tot_len;
    int   inp_index, out_index;
    int   nStructNo;

    long  lSdfId;
    long num_inp, num_err, num_output;

    INPUT_PARMS inp_parms, *ip = &inp_parms;
    STRUCT_DATA struct_data, *sd = &struct_data;


    INCHI_IOSTREAM outputstr, logstr, prbstr, instr;
    INCHI_IOSTREAM *out_stream = &outputstr, *log_stream = &logstr, *prb_stream = &prbstr, *inp_stream = &instr;

    inchi_Input inchi_inp, *pInp = &inchi_inp;

    inchi_Output genout, *pResults = &genout;
    INCHIGEN_DATA       inchi_gendata, *pGenData = &inchi_gendata;
    INCHIGEN_HANDLE  HGen = NULL;

    char *pinfo;
    int len_mess = 0, len_mess1, mshift = 0, was_print = 0;
    char ik_string[256];    /*^^^ Resulting InChIKey string */
    int ik_ret = 0;           /*^^^ InChIKey-calc result code */
    int xhash1, xhash2;
    char szXtra1[256], szXtra2[256];

    unsigned long  ulDisplTime = 0;    /*  infinite, milliseconds */
    time_t elapsed;
    /*^^^ Post-1.02b - moved from below */
    int bTabbed = 0;

    /* Modularized API does not support 1.05+ novel things */
    /* Use novelty-nullified version of extended input object */
    inchi_InputEx inchi_inpex, *pInpEx = &inchi_inpex;
    pInpEx->polymer = NULL;
    pInpEx->v3000 = NULL;

    /*^^^ Set debug output */

#if (TRACE_MEMORY_LEAKS == 1)

    _CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );
    /* for execution outside of the VC++ debugger uncomment one of the following two */
    /*#define MY_REPORT_FILE  _CRTDBG_FILE_STDERR */
    /*#define MY_REPORT_FILE  _CRTDBG_FILE_STDOUT */
#ifdef MY_REPORT_FILE
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, MY_REPORT_FILE );
#else
    _CrtSetReportMode( _CRT_WARN | _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
   /* turn on floating point exceptions */
#if ( !defined(__STDC__) || __STDC__ != 1 )
    {
        /* Get the default control word. */
        int cw = _controlfp( 0, 0 );

        /* Set the exception masks OFF, turn exceptions on. */
        /*cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_INEXACT|EM_ZERODIVIDE|EM_DENORMAL);*/
        cw &= ~( EM_OVERFLOW | EM_UNDERFLOW | EM_ZERODIVIDE | EM_DENORMAL );

        /* Set the control word. */
        _controlfp( cw, MCW_EM );
    }
#endif

#endif /* #if (TRACE_MEMORY_LEAKS == 1) */




/*^^^ Set Ctrl+C trap under Win32 + MS VC++ */

#if( defined( _WIN32 ) && defined( _CONSOLE ) && defined(_MSC_VER) && _MSC_VER >= 800 && defined(ADD_WIN_CTLC) && !(defined(__STDC__) && __STDC__ == 1))
    if (SetConsoleCtrlHandler( MyHandlerRoutine, 1 ))
    {
        ; /*ConsoleQuit = WasInterrupted*/;
    }
#endif

    num_inp = 0;
    num_err = 0;
    num_output = 0;

    elapsed = time( NULL );

    /*^^^ Set original input structure */
    memset( pInp, 0, sizeof( *pInp ) );
    memset(pInpEx, 0, sizeof(*pInpEx));
    memset( pResults, 0, sizeof( *pResults ) );
    memset( pGenData, 0, sizeof( *pGenData ) );
    memset( szSdfDataValue, 0, sizeof( szSdfDataValue ) );

    /*^^^  Initialize I/O streams as string buffers so that they may be filled within dll;
           also associate files with the streams and use inchi_ios_flush to flush buffer content */

    inchi_ios_init( log_stream, INCHI_IOSTREAM_TYPE_STRING, NULL );
    inchi_ios_init( inp_stream, INCHI_IOSTREAM_TYPE_STRING, NULL );
    inchi_ios_init( out_stream, INCHI_IOSTREAM_TYPE_STRING, NULL );
    inchi_ios_init( prb_stream, INCHI_IOSTREAM_TYPE_STRING, NULL );


    /*^^^ Check command line */
    if (argc < 2 || argc == 2 && ( argv[1][0] == INCHI_OPTION_PREFX ) &&
        ( !strcmp( argv[1] + 1, "?" ) || !e_inchi_stricmp( argv[1] + 1, "help" ) ))
    {
        /* e_HelpCommandLineParms(stdout); */
        e_HelpCommandLineParmsReduced( log_stream );
        log_stream->f = stderr;
        inchi_ios_flush( log_stream );
        return 0;
    }

    if (0 > e_ReadCommandLineParms( argc, (const char **) argv, ip, szSdfDataValue, &ulDisplTime, bReleaseVersion, log_stream ))
                                    /*^^^ Explicitly cast to (const char **) to avoid a
                                          warning about "incompatible pointer type":*/
    {

        goto exit_function;
    }


    /*^^^ Open files associated with I/O streams. */
    if (!e_OpenFiles( &( inp_stream->f ), &( out_stream->f ), &( log_stream->f ), &( prb_stream->f ), ip ))
    {
        goto exit_function;
    }



    /*^^^ Create InChI generator object. */
#ifndef USE_STDINCHI_API
    HGen = INCHIGEN_Create( );
#else
    HGen = STDINCHIGEN_Create( );
#endif

    if (NULL == HGen)
    {
        /*^^^ Probably, out of RAM. Could do nothing. */
        inchi_fprintf( stderr, "Could not create InChI generator (out of RAM?)\n" );
        goto exit_function;
    }




    /*^^^ Set input labelling opts */
    if (ip->bNoStructLabels)
    {
        ip->pSdfLabel = NULL;
        ip->pSdfValue = NULL;
    }
    else
        if (ip->nInputType == INPUT_INCHI_PLAIN ||
             ip->nInputType == INPUT_INCHI_XML ||
             ip->nInputType == INPUT_CMLFILE)
        {
            /* the input may contain both the header and the label of the structure */
            if (!ip->pSdfLabel)
            {
                ip->pSdfLabel = ip->szSdfDataHeader;
            }
            if (!ip->pSdfValue)
            {
                ip->pSdfValue = szSdfDataValue;
            }
        }
    e_PrintInputParms( log_stream, ip );
    inchi_ios_flush2( log_stream, stdout );
    pStrInchiId[0] = '\0';




    /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                Main cycle:  read input structures and create their InChI
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    out_index = 0;


    while (!e_bInterrupted)
    {

        int bHasTimeout = 0;
        if (ip->last_struct_number && num_inp >= ip->last_struct_number)
        {
            retcode = _IS_EOF; /*  simulate end of file */
            goto exit_function;
        }

        /*^^^ Create command line */
        szInchiCmdLine[0] = '\0';
        for (i = 1; i < argc; i++)
        {
            if (argv[i] && INCHI_OPTION_PREFX == argv[i][0] && argv[i][1])
            {
                /*^^^ Omit certain options */
                if (!e_inchi_memicmp( argv[i] + 1, "start:", 6 ) ||
                     !e_inchi_memicmp( argv[i] + 1, "end:", 4 ) ||
                     !e_inchi_stricmp( argv[i] + 1, "Tabbed" )
                     )
                {
                    continue;
                }

                if (!e_inchi_stricmp( argv[i] + 1, "Inchi2Inchi" ))
                {
                    inchi_ios_eprint( log_stream, "\nOption Inchi2Inchi is not supported (use classic interface). \n" );
                    goto exit_function;
                }
                else if (!e_inchi_stricmp( argv[i] + 1, "Inchi2Struct" ))
                {
                    inchi_ios_eprint( log_stream, "\nOption Inchi2Struct is not supported (use classic interface). \n" );
                    goto exit_function;
                }


                if (!e_inchi_memicmp( argv[i] + 1, "w", 1 ) && isdigit( UCINT argv[i][2] ))
                {
                    bHasTimeout = 1;
                }
                /*^^^ Add option to szInchiCmdLine */
                if (strlen( szInchiCmdLine ) + strlen( argv[i] ) + 4 < sizeof( szInchiCmdLine ))
                {
                    if (szInchiCmdLine[0])
                    {
                        strcat( szInchiCmdLine, " " );
                    }
                    k = ( !( k = strcspn( argv[i], " \t" ) ) || argv[i][k] ); /* k means enclose in "" */
                    if (k)
                    {
                        strcat( szInchiCmdLine, "\"" );
                    }
                    strcat( szInchiCmdLine, argv[i] );
                    if (k)
                    {
                        strcat( szInchiCmdLine, "\"" );
                    }
                }
                else
                {
                    inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", argv[i] );
                }
            }
        }
        if (!bHasTimeout)
        {
            /*^^^ Add default timeout option -W60: 60 seconds */
            char szW60[] = " ?W60";
            szW60[1] = INCHI_OPTION_PREFX;
            if (strlen( szInchiCmdLine ) + strlen( szW60 ) < sizeof( szInchiCmdLine ))
            {
                strcat( szInchiCmdLine, szW60 );
            }
            else
                inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", szW60 );
        }
        /*^^^ End of command line deal */


        if (ip->nInputType == INPUT_INCHI_PLAIN)
        {
            inchi_ios_eprint( log_stream, "\nRestoring InChI from AuxInfo is not supported (use classic interface). \n" );
            goto exit_function;
        }


        /*^^^ Skip input cycle */
        while (!e_bInterrupted)
        {
            inp_index = out_index;


            /*^^^ Read one structure from input */
            e_FreeInchi_Input( pInpEx );

            retcode = e_ReadStructure(  sd, ip, inp_stream, log_stream, out_stream, prb_stream, 
                                        pInpEx, 
                                        num_inp + 1, inp_index, &out_index );
            e_set_inchi_input_by_extended_inchi_input(pInp, pInpEx);

            inchi_ios_flush2( log_stream, stdout );

            if (_IS_SKIP != retcode)
            {
                lSdfId = ( ip->bGetSdfileId ) ? ip->lSdfId : 0; /* if requested then CAS r.n. otherwise struct. number*/
                nStructNo = ( ip->lMolfileNumber > 0 ) ? ip->lMolfileNumber : num_inp + 1;
                e_MakeOutputHeader( ip->pSdfLabel, ip->pSdfValue, lSdfId, nStructNo, pStrInchiId, pStrLogId );
            }

            /* e_ReadStructure() outputs the error/warning messages, so we do not need to re-output them here */
            switch (retcode)
            {
                case _IS_FATAL:
                    num_inp++;
                    num_err++;
                    goto exit_function;
                case _IS_EOF:
                    inchi_ios_eprint( log_stream, "\rStructure %d could not be read: Detected end of file. \r", num_inp + 1 );
                    goto exit_function;
                case _IS_ERROR:
                    num_inp++;
                    num_err++;
                    continue;
                case _IS_SKIP:
                    num_inp++;
                    continue;
            }
            break;
        }
        if (e_bInterrupted)
        {
            inchi_ios_eprint( log_stream, "\nStructure %d could not be read: User Quit.\n", num_inp + 1 );
            inchi_ios_flush2( log_stream, stdout );
            num_err++;
            goto exit_function;
        }


        /*^^^ Analyze the chiral flag */

        p1 = NULL;
#ifndef USE_STDINCHI_API
        if (( ip->nMode & REQ_MODE_CHIR_FLG_STEREO ) && ( ip->nMode & REQ_MODE_STEREO ) &&
            ( ip->bChiralFlag & ( FLAG_SET_INP_AT_CHIRAL | FLAG_SET_INP_AT_NONCHIRAL ) ))
        {
            ; /* cmd line has priority over the chiral flag in Molfile */
        }
        else if (sd->bChiralFlag & FLAG_INP_AT_CHIRAL)
        {
            p1 = e_GetChiralFlagString( 1 );  /* input file has chiral flag */
        }
        else if (( ip->nMode & REQ_MODE_CHIR_FLG_STEREO ) && ( ip->nMode & REQ_MODE_STEREO ) ||
            ( sd->bChiralFlag & FLAG_INP_AT_NONCHIRAL ))  /* fix 04/05/2005 D.T.*/
        {  /* chiral flag requested (/SUCF) or input has non-chiral flag */
            p1 = e_GetChiralFlagString( 0 );
        }
        if (p1)
        {
            if (strlen( szInchiCmdLine ) + strlen( p1 ) < sizeof( szInchiCmdLine ))
            {
                strcat( szInchiCmdLine, p1 );
            }
            else
            {
                inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", p1 );
            }
        }
#endif
        pInp->szOptions = szInchiCmdLine;


        /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        create INChI for each connected component of the structure and optionally
        display them; output INChI for the whole structure.
        Use compartmentalized library interface.
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#ifdef CREATE_0D_PARITIES
        if (!pInp->stereo0D && !pInp->num_stereo0D)
        {
            int bPointedEdgeStereo = ( 0 != ( TG_FLAG_POINTED_EDGE_STEREO & ip->bTautFlags ) );
            set_0D_stereo_parities( pInp, bPointedEdgeStereo );
            Clear3D2Dstereo( pInp );
        }
#endif
#ifdef NEIGH_ONLY_ONCE
        e_RemoveRedundantNeighbors( pInp );
#endif

        len_mess = len_mess1 = 0;
        pinfo = ( char* ) &( pGenData->pStrErrStruct );
        was_print = 0;



        /*^^^ Set up */

#ifndef USE_STDINCHI_API
        retcode = INCHIGEN_Setup( HGen, pGenData, pInp );
#else
        retcode = STDINCHIGEN_Setup( HGen, pGenData, pInp );
#endif

        len_mess1 = strlen( pinfo );
        if (len_mess1 > len_mess)
        {
            mshift = len_mess ? 1 : 0;
            inchi_ios_eprint( log_stream, "*** %-s [Initialization] %-s\n", pStrLogId, pinfo + len_mess + mshift );
            inchi_ios_flush2( log_stream, stdout );
            len_mess = len_mess1;
            was_print = 1;
        }



        /*^^^ Normalization step */
        if (( retcode == inchi_Ret_OKAY ) || ( retcode == inchi_Ret_WARNING ))
        {

#ifndef USE_STDINCHI_API
            retcode = INCHIGEN_DoNormalization( HGen, pGenData );
#else
            retcode = STDINCHIGEN_DoNormalization( HGen, pGenData );
#endif

            len_mess1 = strlen( pinfo );
            if (len_mess1 > len_mess)
            {
                mshift = len_mess ? 1 : 0;
                inchi_ios_eprint( log_stream, "*** %-s [Normalization] %-s\n", pStrLogId, pinfo + len_mess + mshift );
                inchi_ios_flush2( log_stream, stdout );
                len_mess = len_mess1;
                was_print = 1;
            }

#ifdef OUTPUT_NORMALIZATION_DATA
            {
                int ic, istruct, itaut, ia, ib, nc[2];
                NORM_ATOMS *inp_norm_data[TAUT_NUM]; /*  = { &NormAtomsNontaut, &NormAtomsTaut}; */
                NORM_ATOM *atom;
                nc[0] = pGenData->num_components[0];
                nc[1] = pGenData->num_components[1];
                inchi_ios_eprint( log_stream, "=== %-s Intermediate normalization data follow", pStrLogId );
                for (istruct = 0; istruct < 2; istruct++)
                {
                    /*^^^ Print results for 1) original/disconnected structure and
                                            2)reconnected structure */
                    if (nc[istruct] > 0)
                    {
                        if (istruct == 0)
                            inchi_ios_eprint( log_stream, "\n\tOriginal/disconnected structure: " );
                        else
                            inchi_ios_eprint( log_stream, "\n\tReconnected structure: " );
                        inchi_ios_eprint( log_stream, "%-d component(s)\n", nc[istruct] );
                        inchi_ios_eprint( log_stream, "\t===============================================\n" );

                        for (ic = 0; ic < nc[istruct]; ic++)
                        {
                            /* Print results for each component of the structure */
                            inp_norm_data[0] = &( pGenData->NormAtomsNontaut[istruct][ic] );
                            inp_norm_data[1] = &( pGenData->NormAtomsTaut[istruct][ic] );
                            for (itaut = 0; itaut < 2; itaut++)
                            {
                                /* Print results for 1) non-tautomeric and
                                                     2) tautomeric  version (if present)
                                                     of the component                   */
                                if (NULL != inp_norm_data[itaut])
                                {

                                    if (inp_norm_data[itaut]->num_at > 0)
                                    {
                                        if (itaut == 0)
                                        {
                                            inchi_ios_eprint( log_stream, "\tComponent %-d, non-tautomeric:", ic + 1 );
                                        }
                                        else
                                            inchi_ios_eprint( log_stream, "\tComponent %-d, tautomeric:", ic + 1 );

                                        inchi_ios_eprint( log_stream, "\t%-d atom(s)\n", inp_norm_data[itaut]->num_at );

                                        for (ia = 0; ia < inp_norm_data[itaut]->num_at; ia++)
                                        {
                                            /*^^^ Print data for each atom */
                                            if (inp_norm_data[itaut]->at != NULL)
                                            {
                                                atom = &( inp_norm_data[itaut]->at[ia] );
                                                if (NULL != atom)
                                                {
                                                    /*^^^ Print: element, number, original number, no. of Hs,
                                                                 charge, coordination number, valence */
                                                    inchi_ios_eprint( log_stream, "\t\tatom %-s%-d (orig.%-s%-d) [H%-d] charge=%-+d  CN=%-d val=%-d ",
                                                        atom->elname, ia + 1, atom->elname, atom->orig_at_number, atom->num_H, atom->charge,
                                                        atom->valence, atom->chem_bonds_valence );
                                                    if (atom->valence > 0)
                                                    {
                                                        /*^^^ Neighbors */
                                                        inchi_ios_eprint( log_stream, "nbrs { " );
                                                        for (ib = 0; ib < atom->valence; ib++)
                                                        {
                                                            inchi_ios_eprint( log_stream, "%-d ", atom->neighbor[ib] + 1 );
                                                        }
                                                        inchi_ios_eprint( log_stream, "}" );
                                                    }
                                                    /* Indicate if atom shares Hs with others */
                                                    if (atom->endpoint > 0)
                                                    {
                                                        inchi_ios_eprint( log_stream, "\n\t\t(in taut. group: %-d)", atom->endpoint );
                                                    }
                                                    inchi_ios_eprint( log_stream, "\n" );
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            inchi_ios_flush( log_stream );
#endif
        }

        /*^^^ Canonicalization step */
        if (( retcode == inchi_Ret_OKAY ) || ( retcode == inchi_Ret_WARNING ))
        {
#ifndef USE_STDINCHI_API
            retcode = INCHIGEN_DoCanonicalization( HGen, pGenData );
#else
            retcode = STDINCHIGEN_DoCanonicalization( HGen, pGenData );
#endif
            len_mess1 = strlen( pinfo );
            if (len_mess1 > len_mess)
            {
                mshift = len_mess ? 1 : 0;
                inchi_ios_eprint( log_stream, "*** %-s [Canonicalization] %-s\n", pStrLogId, pinfo + len_mess + mshift );
                inchi_ios_flush2( log_stream, stdout );
                len_mess = len_mess1;
                was_print = 1;
            }
        }

        /*^^^ Serialization (final) step */
        if (( retcode == inchi_Ret_OKAY ) || ( retcode == inchi_Ret_WARNING ))
        {
#ifndef USE_STDINCHI_API
            retcode1 = INCHIGEN_DoSerialization( HGen, pGenData, pResults );
#else
            retcode1 = STDINCHIGEN_DoSerialization( HGen, pGenData, pResults );
#endif
            retcode = inchi_max( retcode, retcode1 );

            len_mess1 = strlen( pinfo );
            if (len_mess1 > len_mess)
            {
                mshift = len_mess ? 1 : 0;
                inchi_ios_eprint( log_stream, "*** %-s [Serialization] %-s\n", pStrLogId, pinfo + len_mess + mshift );
                inchi_ios_flush2( log_stream, stdout );
                was_print = 1;
            }
        }

/*
        if (!was_print)
        {
            inchi_ios_eprint(log_stream, "*** %-s        \r", pStrLogId);
        }
*/


        /*^^^ Output err/warn */
        if (pResults->szMessage && pResults->szMessage[0])
        {
            p1 = "; ";
            p2 = pResults->szMessage;
        }
        else
        {
            p1 = p2 = "";
        }


        switch (retcode)
        {
            case inchi_Ret_UNKNOWN:
            case inchi_Ret_FATAL: /* fatal processing error -- typically, memory allocation error */
                num_inp++;
                num_err++;
                inchi_ios_eprint( log_stream, "Fatal Error (No INChI%s%s) %s\n", p1, p2, pStrLogId );
                inchi_ios_eprint( log_stream, "Log start:---------------------\n%s\nLog end--------------------\n", pResults->szLog ? pResults->szLog : "Log is missing" );
                /*^^^ Free InChI library memories */
#ifndef USE_STDINCHI_API
                INCHIGEN_Reset( HGen, pGenData, pResults );
#else
                STDINCHIGEN_Reset( HGen, pGenData, pResults );
#endif
                goto exit_function;
            case inchi_Ret_EOF: /* typically, no input structure provided or help requested */
                /* output previous structure number and message */
                inchi_ios_eprint( log_stream, "End of file detected after structure %d\n", num_inp );
                goto exit_function;
            case inchi_Ret_ERROR:
                num_inp++;
                num_err++;
                inchi_ios_eprint( log_stream, "Error (No INChI%s%s) %s\n", p1, p2, pStrLogId );
                inchi_ios_flush2( log_stream, stdout );
                /*^^^ Free InChI library memories */
#ifndef USE_STDINCHI_API
                INCHIGEN_Reset( HGen, pGenData, pResults );
#else
                STDINCHIGEN_Reset( HGen, pGenData, pResults );
#endif
                continue;
            case inchi_Ret_SKIP:
                num_inp++;
                inchi_ios_eprint( log_stream, "Skipped %s\n", pStrLogId );
                goto exit_function;
            case inchi_Ret_OKAY:
                break;
            case inchi_Ret_WARNING:
                /*^^^ Suppress warnings, we display them step by steps */
                /*^^^
                if ( p2 && p2[0] )
                {
                    inchi_ios_eprint( log_stream, "Warning (%s) %s\n", p2, pStrLogId );
                }
                */
                break; /* ok */
        }

        num_inp++;
        tot_len = 0;

        /*^^^ Post-1.02b - here from below */
        bTabbed = 0 != ( ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT );

        if (pResults->szInChI && pResults->szInChI[0])
        {
            if (ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY)
            {
                /*^^^ output SDfile */
                char *start;
                unsigned len;
                int bAddLabel = 0;
                /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                output a SDfile. pResults->szInChI contains Molfile ending with "$$$$\n" line.
                Replace the 1st line with the structure number
                Replace the last line with the SDfile header, label, and new "$$$$\n" line
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


                start = pResults->szInChI;

                /*^^^ 2. SDfile header and data: write zero to the 1st byte of
                         the last line "$$$$\n" to remove this line with purpose to relpace it */
                if (ip->pSdfLabel && ip->pSdfLabel[0] && ip->pSdfValue && ip->pSdfValue[0] &&
                    ( len = strlen( start ) ) && len > 5 && '$' == start[len - 5] && '\n' == start[len - 6])
                {
                    start[len - 5] = '\0';
                    bAddLabel = 1;
                }

                /*^^^ 3. Output the whole Molfile */
                inchi_ios_print( out_stream, "%s", start );
                if (bAddLabel)
                {
                    inchi_ios_print( out_stream, ">  <%s>\n%s\n\n$$$$\n", ip->pSdfLabel, ip->pSdfValue );
                }

                inchi_ios_flush( out_stream );
            } /* if (ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY ) */

            else

            {
                /*^^^ Print InChI */

                int bAuxInfo = !( ip->bINChIOutputOptions & INCHI_OUT_ONLY_AUX_INFO ) &&
                    pResults->szAuxInfo && pResults->szAuxInfo[0];

     /*^^^ Post-1.02b - correctly treat tabbed output with InChIKey */
                int bAuxOrKey = bAuxInfo || ( ip->bCalcInChIHash != INCHIHASH_NONE );

                const char *pLF = "\n";
                const char *pTAB = bTabbed ? "\t" : pLF;
                if (!ip->bNoStructLabels)
                {
                    /* Print a previously created label string */
                    inchi_ios_print( out_stream, "%s%s", pStrInchiId, pTAB );
                }

                /*^^^ Print INChI Identifier */

                /*^^^ Post-1.02b */
                inchi_ios_print( out_stream, "%s%s", pResults->szInChI, bAuxOrKey ? pTAB : pLF );
                /*^^^ Print INChI Aux Info */
                if (bAuxInfo)
                {
                    inchi_ios_print( out_stream, "%s%s", pResults->szAuxInfo, ip->bCalcInChIHash ? pTAB : pLF );
                }

                inchi_ios_flush( out_stream );
            }
        } /* if ( pResults->szInChI && pResults->szInChI[0] )  */


        /*^^^ Calculate InChIKey */
        if (ip->bCalcInChIHash != INCHIHASH_NONE)
        {
            xhash1 = xhash2 = 0;
            if (( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1 ) ||
                ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1_XTRA2 ))
                xhash1 = 1;
            if (( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA2 ) ||
                ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1_XTRA2 ))
                xhash2 = 1;

            ik_ret = GetINCHIKeyFromINCHI( pResults->szInChI, xhash1, xhash2,
                                                  ik_string, szXtra1, szXtra2 );
            if (ik_ret == INCHIKEY_OK)
            {
                /* NB: correctly treat tabbed output with InChIKey & hash extensions */
                char csep = '\n';
                if (ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT)
                {
                    csep = '\t';
                }
                inchi_ios_print( out_stream, "InChIKey=%-s", ik_string );
                if (xhash1)
                {
                    inchi_ios_print( out_stream, "%cXHash1=%-s", csep, szXtra1 );
                }
                if (xhash2)
                {
                    inchi_ios_print( out_stream, "%cXHash2=%-s", csep, szXtra2 );
                }
                inchi_ios_print( out_stream, "\n" );
            }
            else
            {
                /*^^^ Post-1.02b - add LF if output is tabbed and key generation failed */
                if (bTabbed)
                {
                    inchi_ios_print( out_stream, "\n" );
                }
                inchi_ios_eprint( log_stream, "Warning: could not compute InChIKey for #%-d ",
                                         num_inp );
                switch (ik_ret)
                {
                    case INCHIKEY_UNKNOWN_ERROR:
                        inchi_ios_eprint( log_stream, "(invalid key length requested)\n" );
                        break;
                    case INCHIKEY_EMPTY_INPUT:
                        inchi_ios_eprint( log_stream, "(got an empty string)\n" );
                        break;
                    case INCHIKEY_INVALID_INCHI_PREFIX:
                    case INCHIKEY_INVALID_INCHI:
                    case INCHIKEY_INVALID_STD_INCHI:
                        inchi_ios_eprint( log_stream, "(got non-InChI string)\n" );
                        break;
                    case INCHIKEY_NOT_ENOUGH_MEMORY:
                        inchi_ios_eprint( log_stream, "(not enough memory to treat the string)\n" );
                        break;
                    default:inchi_ios_eprint( log_stream, "(internal program error)\n" );
                        break;
                }
            }
            inchi_ios_flush( out_stream );
            inchi_ios_flush2( log_stream, stdout );
        }



        /*^^^ Free InChI library memories */
#ifndef USE_STDINCHI_API
        INCHIGEN_Reset( HGen, pGenData, pResults );
#else
        STDINCHIGEN_Reset( HGen, pGenData, pResults );
#endif
    } /* while ( !e_bInterrupted ) */




    if (e_bInterrupted)
    {
        inchi_ios_eprint( log_stream, "\nStructure %d could not be processed: User Quit.\n", num_inp + 1 );
        num_err++;
        goto exit_function;
    }



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
exit_function:

    inchi_ios_eprint( log_stream, "\nProcessed %ld structure%s, %ld error%s.\n",
                 num_inp, ( num_inp == 1 ) ? "" : "s", num_err, ( num_err == 1 ) ? "" : "s" );

    elapsed -= time( NULL );
    inchi_ios_eprint( log_stream, "\nElapsed time: %ld\"\n", -elapsed );

    inchi_ios_flush2( log_stream, stdout );



    /*^^^ Freeing/closing */
    e_FreeInchi_Input( pInpEx );

#ifndef USE_STDINCHI_API
    INCHIGEN_Destroy( HGen );
#else
    STDINCHIGEN_Destroy( HGen );
#endif

    for (i = 0; i < MAX_NUM_PATHS; i++)
    {
        if (ip->path[i])
        {
            e_inchi_free( (char*) ip->path[i] ); /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }

    inchi_ios_close( inp_stream );
    inchi_ios_close( log_stream );
    inchi_ios_close( out_stream );
    inchi_ios_close( prb_stream );

    return 0;
}




/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Local helper.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
int e_MakeOutputHeader( char *pSdfLabel, char *pSdfValue, long lSdfId, long num_inp, char *pStr1, char *pStr2 )
{
    int tot_len1 = 0, tot_len2 = 0;
    pStr1[0] = '\0';
    if (!( pSdfLabel && pSdfLabel[0] ) && !( pSdfValue && pSdfValue[0] ))
    {
        tot_len1 = sprintf( pStr1, "Structure: %ld", num_inp );
        tot_len2 = sprintf( pStr2, "structure #%ld", num_inp );
    }
    else
    {
        tot_len1 = sprintf( pStr1, "Structure: %ld.%s%s%s%s",
            num_inp, SDF_LBL_VAL( pSdfLabel, pSdfValue ) );

        tot_len2 = sprintf( pStr2, "structure #%ld.%s%s%s%s",
            num_inp, SDF_LBL_VAL( pSdfLabel, pSdfValue ) );
        if (lSdfId)
        {
            tot_len1 += sprintf( pStr1 + tot_len1, ":%ld", lSdfId );
            tot_len2 += sprintf( pStr2 + tot_len2, ":%ld", lSdfId );
        }
    }
    return tot_len1;
}


/************************************************************************************/
/* Abridged version of HelpCommandLineParms,for use in the example of InChI DLL call*/
void e_HelpCommandLineParmsReduced( INCHI_IOSTREAM *f )
{
    if (!f)
    {
        return;
    }

#if ( bRELEASE_VERSION == 1 )


    inchi_ios_print_nodisplay( f,

#ifdef TARGET_EXE_USING_API
        "%s %s\n%-s Build (%-s%-s) of %s %s%s\n\nUsage:\ninchi_main inputFile [outputFile [logFile [problemFile]]] [%coption[ %coption...]]\n",
        APP_DESCRIPTION, INCHI_SRC_REV,
        INCHI_BUILD_PLATFORM, INCHI_BUILD_COMPILER, INCHI_BUILD_DEBUG, __DATE__, __TIME__,
        RELEASE_IS_FINAL ? "" : " *** pre-release, for evaluation only ***",
        INCHI_OPTION_PREFX, INCHI_OPTION_PREFX );
#else
        "%s %-s\n%-s Build (%-s%-s) of %s %s%s\n\nUsage:\ninchi-1 inputFile [outputFile [logFile [problemFile]]] [%coption[ %coption...]]\n",
        APP_DESCRIPTION, INCHI_SRC_REV,
        INCHI_BUILD_PLATFORM, INCHI_BUILD_COMPILER, INCHI_BUILD_DEBUG, __DATE__, __TIME__,
        RELEASE_IS_FINAL ? "" : " *** pre-release, for evaluation only ***",
        INCHI_OPTION_PREFX, INCHI_OPTION_PREFX );
#endif

    inchi_ios_print_nodisplay( f, "\nOptions:\n" );


    inchi_ios_print_nodisplay( f, "\nInput\n" );
    inchi_ios_print_nodisplay( f, "  STDIO       Use standard input/output streams\n" );
    inchi_ios_print_nodisplay( f, "  InpAux      Input structures in %s default aux. info format\n              (for use with STDIO)\n", INCHI_NAME );
    inchi_ios_print_nodisplay( f, "  SDF:DataHeader Read from the input SDfile the ID under this DataHeader\n" );

    /*
    inchi_ios_print_nodisplay( f, "  START:n     Skip structures up to n-th one\n");
    inchi_ios_print_nodisplay( f, "  END:m       Skip structures after m-th one\n");
    */

#if ( BUILD_WITH_AMI == 1 )
    inchi_ios_print_nodisplay( f, "  AMI         Allow multiple input files (wildcards supported)\n" );
#endif

    inchi_ios_print_nodisplay( f, "Output\n" );
    inchi_ios_print_nodisplay( f, "  NoWarnings  Suppress all warning messages(default: show)\n" );
    inchi_ios_print_nodisplay( f, "  AuxNone     Omit auxiliary information (default: Include)\n" );
    inchi_ios_print_nodisplay( f, "  SaveOpt     Save custom InChI creation options (non-standard InChI)\n" );
    inchi_ios_print_nodisplay( f, "  NoLabels    Omit structure number, DataHeader and ID from %s output\n", INCHI_NAME );
    inchi_ios_print_nodisplay( f, "  Tabbed      Separate structure number, %s, and AuxInfo with tabs\n", INCHI_NAME );
    /* inchi_ios_print_nodisplay( f, "  Compress    Compressed output\n"); */

#if ( defined(_WIN32) && defined(_MSC_VER) && !defined(COMPILE_ANSI_ONLY) && !defined(TARGET_API_LIB) )
    inchi_ios_print_nodisplay( f, "  D           Display the structures\n" );
    inchi_ios_print_nodisplay( f, "  EQU         Display sets of identical components\n" );
    inchi_ios_print_nodisplay( f, "  Fnumber     Set display Font size in number of points\n" );
#endif

    inchi_ios_print_nodisplay( f, "  OutputSDF   Convert %s created with default aux. info to SDfile\n", INCHI_NAME );

#if ( SDF_OUTPUT_DT == 1 )
    inchi_ios_print_nodisplay( f, "  SdfAtomsDT  Output Hydrogen Isotopes to SDfile as Atoms D and T\n" );
#endif

#if ( BUILD_WITH_AMI == 1 )
    inchi_ios_print_nodisplay( f, "  AMIOutStd   Write output to stdout (in AMI mode)\n" );
    inchi_ios_print_nodisplay( f, "  AMILogStd   Write log to stderr (in AMI mode)\n" );
    inchi_ios_print_nodisplay( f, "  AMIPrbNone  Suppress creation of problem files (in AMI mode)\n" );
#endif
    inchi_ios_print_nodisplay( f, "Structure perception\n" );
    inchi_ios_print_nodisplay( f, "  SNon        Exclude stereo (default: include absolute stereo)\n" );
    inchi_ios_print_nodisplay( f, "  NEWPSOFF    Both ends of wedge point to stereocenters (default: a narrow end)\n" );
    inchi_ios_print_nodisplay( f, "  LooseTSACheck   Relax criteria of ambiguous drawing for in-ring tetrahedral stereo\n" );
    inchi_ios_print_nodisplay( f, "  DoNotAddH   All H are explicit (default: add H according to usual valences)\n" );

#ifndef USE_STDINCHI_API
    inchi_ios_print_nodisplay( f, "Stereo perception modifiers (non-standard InChI)\n" );
    inchi_ios_print_nodisplay( f, "  SRel        Relative stereo\n" );
    inchi_ios_print_nodisplay( f, "  SRac        Racemic stereo\n" );
    inchi_ios_print_nodisplay( f, "  SUCF        Use Chiral Flag: On means Absolute stereo, Off - Relative\n" );

    inchi_ios_print_nodisplay( f, "Customizing InChI creation (non-standard InChI)\n" );
    inchi_ios_print_nodisplay( f, "  SUU         Always include omitted unknown/undefined stereo\n" );
    inchi_ios_print_nodisplay( f, "  SLUUD       Make labels for unknown and undefined stereo different\n" );
    inchi_ios_print_nodisplay( f, "  RecMet      Include reconnected metals results\n" );
    inchi_ios_print_nodisplay( f, "  FixedH      Include Fixed H layer\n" );
    inchi_ios_print_nodisplay( f, "  KET         Account for keto-enol tautomerism (experimental)\n" );
    inchi_ios_print_nodisplay( f, "  15T         Account for 1,5-tautomerism (experimental)\n" );
#endif

    inchi_ios_print_nodisplay( f, "Generation\n" );
    inchi_ios_print_nodisplay( f, "  Wnumber     Set time-out per structure in seconds; W0 means unlimited\n" );
    inchi_ios_print_nodisplay( f, "  WMnumber    Set time-out per structure in milliseconds (int); WM0 means unlimited\n" );
    inchi_ios_print_nodisplay( f, "  ExpoZz      Expose Zz atoms if present (experimental)\n" );
    inchi_ios_print_nodisplay( f, "  Polymers     Allow processing of polymers (experimental)\n" );
    inchi_ios_print_nodisplay( f, "  FoldCRU      Remove repeats within constitutional repeating units (CRU/SRU)\n" );
    inchi_ios_print_nodisplay( f, "  NoFrameShift Turn OFF polymer frame shift\n" );
#if ( USE_ZZ == ZZ_YES )
    inchi_ios_print_nodisplay( f, "  StereoZZ     Do not ignore stereo at centers adjacent to Zz atoms\n" );
#endif
    inchi_ios_print_nodisplay( f, "  LargeMolecules Treat molecules up to 32766 atoms (experimental)\n" );
    inchi_ios_print_nodisplay( f, "  WarnOnEmptyStructure Warn and produce empty %s for empty structure\n", INCHI_NAME );
    /* inchi_ios_print_nodisplay( f, "  MismatchIsError Treat problem/mismatch on inchi2struct conversion as error\n"); */
    inchi_ios_print_nodisplay( f, "  Key         Generate InChIKey\n" );
    inchi_ios_print_nodisplay( f, "  XHash1      Generate hash extension (to 256 bits) for 1st block of InChIKey\n" );
    inchi_ios_print_nodisplay( f, "  XHash2      Generate hash extension (to 256 bits) for 2nd block of InChIKey\n" );


#endif /* #if ( bRELEASE_VERSION == 1 ) */
}


/************************************************************************************/
int e_set_inchi_input_by_extended_inchi_input(inchi_Input *pInp, inchi_InputEx *pInpEx)
{
    pInp->atom = pInpEx->atom;
    pInp->stereo0D = pInpEx->stereo0D;
    pInp->szOptions = pInpEx->szOptions;
    pInp->num_atoms = pInpEx->num_atoms;
    pInp->num_stereo0D = pInpEx->num_stereo0D;
    return 0;
}


#endif /* CREATE_INCHI_STEP_BY_STEP */
