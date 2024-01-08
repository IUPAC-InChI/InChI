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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#ifdef _WIN32
#include "windows.h"
#endif

#include "../../../../INCHI_BASE/src/inchi_api.h"
#include "../../../../INCHI_BASE/src/util.h"

#include "moreitil.h"
#include "test_ixa.h"



/****************************************************************************
 Check for errors and warnings in the status object and output them, then clear
 the status object. The errors and warnings have been dealt with and are
 no longer needed.
****************************************************************************/
static int CheckStatus( IXA_STATUS_HANDLE hStatus, long nrecord )
{
    int result = 1;

    int count = IXA_STATUS_GetCount( hStatus );
    int index;
    for (index = 0; index < count; index++)
    {
        switch (IXA_STATUS_GetSeverity( hStatus, index ))
        {
            case IXA_STATUS_ERROR:
                result = 0;
                fprintf( stderr, "\nERROR: %s : structure %-ld", IXA_STATUS_GetMessage( hStatus, index ), nrecord );
                break;
            case IXA_STATUS_WARNING:
                fprintf( stderr, "\nWARNING: %s : structure %-ld", IXA_STATUS_GetMessage( hStatus, index ), nrecord );
                break;
        }
    }

    IXA_STATUS_Clear( hStatus );

    return result;
}


/****************************************************************************
 Test whether the command line argument matches the given option name.
 This is little more than a case insensitive compare.
****************************************************************************/
static int OptionCompare( const char *pArg,
                          const char *pOption )
{
    if (( *pArg != '-' ) && ( *pArg != '/' ))
    {
        return 0;
    }

    pArg++;
    while (*pArg)
    {
        if (( *pArg != *pOption ) && ( tolower( (int) *pArg ) != tolower( (int) *pOption ) ))
        {
            return 0;
        }

        pArg++;
        pOption++;
    }

    if (*pOption)
    {
        return 0;
    }

    return 1;
}


/****************************************************************************
 Read the command line arguments and set options on the
 InChI builder accordingly.
****************************************************************************/
static int ReadOptions( int argc,
                        const char *argv[],
                        IXA_BOOL *pKey,
                        IXA_BOOL *pRoundTrip,
                        IXA_BOOL *pGenerateAuxinfo,
                        IXA_BOOL *pVerbose,
                        char *pOptions,
                        IXA_STATUS_HANDLE hStatus,
                        IXA_INCHIBUILDER_HANDLE hBuilder )
{
    int index;

    *pKey = IXA_FALSE;
    *pRoundTrip = IXA_FALSE;
    *pGenerateAuxinfo = IXA_TRUE;
    pOptions[0] = '\0';
    for (index = 3; index < argc; index++)
    {
        strcat( pOptions, " " );
        strcat( pOptions, argv[index] );

        if (OptionCompare( argv[index], "RTrip" ))
        {
            *pRoundTrip = IXA_TRUE;
        }
        else if (OptionCompare( argv[index], "Key" ))
        {
            *pKey = IXA_TRUE;
        }
        else if (OptionCompare( argv[index], "Verbose" ))
        {
            *pVerbose = IXA_TRUE;
        }
        else if (OptionCompare( argv[index], "SNon" ))
        {
            IXA_INCHIBUILDER_SetOption_Stereo( hStatus, hBuilder, IXA_INCHIBUILDER_STEREOOPTION_SNon );
        }
        else if (OptionCompare( argv[index], "SRel" ))
        {
            IXA_INCHIBUILDER_SetOption_Stereo( hStatus, hBuilder, IXA_INCHIBUILDER_STEREOOPTION_SRel );
        }
        else if (OptionCompare( argv[index], "SRac" ))
        {
            IXA_INCHIBUILDER_SetOption_Stereo( hStatus, hBuilder, IXA_INCHIBUILDER_STEREOOPTION_SRac );
        }
        else if (OptionCompare( argv[index], "SUCF" ))
        {
            IXA_INCHIBUILDER_SetOption_Stereo( hStatus, hBuilder, IXA_INCHIBUILDER_STEREOOPTION_SUCF );
        }
        else if (OptionCompare( argv[index], "NoLabels" ))
        {
            /* Do nothing. This option is accepted but has no effect on behaviour. */
        }
        else if (OptionCompare( argv[index], "NEWPSOFF" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_NewPsOff, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "DoNotAddH" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_DoNotAddH, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "SUU" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_SUU, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "SLUUD" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_SLUUD, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "FixedH" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_FixedH, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "RecMet" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_RecMet, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "KET" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_KET, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "15T" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_15T, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "AuxNone" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_AuxNone, IXA_TRUE );
            *pGenerateAuxinfo = IXA_FALSE;
        }
        else if (OptionCompare( argv[index], "WarnOnEmptyStructure" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_WarnOnEmptyStructure, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "LargeMolecules" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_LargeMolecules, IXA_TRUE );
        }
#ifdef  BUILD_WITH_ENG_OPTIONS
        else if (OptionCompare( argv[index], "DoDrv" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_DoDrv, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "DoDrvReport" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_DoDrvReport, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "DoR2C" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_DoR2C, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "DoneOnly" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_DoneOnly, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "OnlyRecSalt" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_OnlyRecSalt, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "OnlyExact" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_OnlyExact, IXA_TRUE );
        }
        else if (OptionCompare( argv[index], "OnlyRecMet" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_OnlyRecMet, IXA_TRUE );
        }
        else if (OptionCompare(argv[index], "Polymers105+"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_Polymers105Plus, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "FilterSS"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_FilterSS, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "InvFilterSS"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_InvFilterSS, IXA_TRUE);
        }
#endif

        else if (OptionCompare( argv[index], "SaveOpt" ))
        {
            IXA_INCHIBUILDER_SetOption( hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_SaveOpt, IXA_TRUE );
        }

        else if (OptionCompare(argv[index], "OutErrInChI"))
        {
        IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_OutErrInChI, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "LooseTSACheck"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_LooseTSACheck, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "NoWarnings"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_NoWarnings, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "Polymers"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_Polymers, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "Polymers105"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_Polymers105, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "NPZz"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_NPZZ, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "SAtZz"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_SATZZ, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "FoldSRU"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_FoldCRU, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "FoldCRU"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_FoldCRU, IXA_TRUE);
        }
        else if (OptionCompare(argv[index], "NoEdits"))
        {
            IXA_INCHIBUILDER_SetOption(hStatus, hBuilder, IXA_INCHIBUILDER_OPTION_NoEdits, IXA_TRUE);
        }
        else
        {
            int is_timeout_requested, timeout_value;
            /* TimeoutNone = 0, TimeoutSec = 1, TimeoutMSec = 2; */
            is_timeout_requested = timeout_value = 0;
            is_timeout_requested = ( ( argv[index][0] == '-' ) || ( argv[index][0] == '/' ) ) &&
                ( ( argv[index][1] == 'W' ) || ( argv[index][1] == 'w' ) );
            if (is_timeout_requested && argv[index][2] && (argv[index][2] == 'M' || argv[index][2] == 'm' ))
            {
                is_timeout_requested = 2;
            }
            if (is_timeout_requested)
            {
                char* p_extra;
                timeout_value = strtol( argv[index] + 1 + is_timeout_requested, &p_extra, 10 );
                if (*p_extra == '\0')
                {
                    if (is_timeout_requested == 1)
                    {
                        IXA_INCHIBUILDER_SetOption_Timeout( hStatus, hBuilder, timeout_value );
                    }
                    else if (is_timeout_requested == 2)
                    {
                        IXA_INCHIBUILDER_SetOption_Timeout_MilliSeconds( hStatus, hBuilder, timeout_value );
                    }
                }
                else
                {
                    fprintf( stderr, "Option \"%s\" is not recognised (invalid integer number was supplied?)\n", argv[index] );
                    return 0;
                }
            }

        }
    }

    return 1;
}


/****************************************************************************
Main program.
****************************************************************************/
void main( int argc, const char *argv[] )
{
    FILE*       sdfile = NULL;
    FILE*       outfile = NULL;
    char*       buffer = NULL;
    long        index = 0, nerr = 0, nboom_rtrip = 0, nmismatch_rtrip = 0;
    IXA_BOOL    key;
    IXA_BOOL    generate_auxinfo = IXA_TRUE;
    IXA_BOOL    verbose = IXA_FALSE;
    const char* inchi;
    const char* auxinfo;
    const char* inchikey;
    IXA_BOOL    round_trip;
    char        options[256];
    char        *saved_inchi = NULL;
    char        *new_inchi = NULL;
#if ( defined(_WIN32) && defined(_MSC_VER) )
#if WINVER >= 0x0501 /* XP or newer */ /* 0x0600 Vista or newer */
    DWORD tick_ixa_start, tick_ixa_stop;
#endif
#else
    time_t      time_t_ixa_start, time_t_ixa_stop;
#endif

    const int RTRIP_COMPARE_AUX_ALSO = 0;

    IXA_STATUS_HANDLE          status = NULL;
    IXA_MOL_HANDLE             molecule = NULL;
    IXA_INCHIBUILDER_HANDLE    inchi_builder = NULL;
    IXA_INCHIKEYBUILDER_HANDLE key_builder = NULL;

#if defined(_WIN32)
#if defined(_DEBUG)
    const char *platform = "Windows Debug";
#else
    const char *platform = "Windows";
#endif
#else
    const char *platform = "Linux";
#endif
    char banner[255];

    sprintf( banner, "%s %-s\n%-s Build (%-s%-s) of %s %s %-s",
        APP_DESCRIPTION, INCHI_SRC_REV,
        INCHI_BUILD_PLATFORM, INCHI_BUILD_COMPILER, INCHI_BUILD_DEBUG, __DATE__, __TIME__,
        RELEASE_IS_FINAL ? "" : " *** pre-release, for evaluation only ***" );
    fprintf( stderr, "%-s\n", banner );

#ifdef IXA_USES_NON_EX_CORE_API
    fprintf( stderr, "Note: IXA API in current build uses non-extended core API calls\n" );
#else
    fprintf( stderr, "Note: IXA API in current build uses extended core API calls\n" );
#endif

    if (argc < 3)
    {
        /* Not enough command line arguments have been provided. Output some help
        information, then exit. */
        print_help( );
        return;
    }

#if ( defined(_WIN32) && defined(_MSC_VER) )
#if WINVER >= 0x0501 /* XP or newer */ /* 0x0600 Vista or newer */
    tick_ixa_start = GetTickCount( );
#endif
#else
    time( &time_t_ixa_start );
#endif

    /* Open input and output files. */
    sdfile = fopen( argv[1], "rb" );
    /* fopen_s(&sdfile, argv[1], "r"); */
    if (!sdfile)
    {
        fprintf( stderr, "Failed to open \"%s\" for reading\n", argv[1] );
        goto cleanup;
    }
    outfile = fopen( argv[2], "w" );
    /* fopen_s(&outfile, argv[2], "w"); */
    if (!outfile)
    {
        fprintf( stderr, "Failed to open \"%s\" for writing\n", argv[2] );
        goto cleanup;
    }

    /* Create status, molecule and InChI builder objects. */
    status = IXA_STATUS_Create( );
    molecule = IXA_MOL_Create( status );
    if (!CheckStatus( status, index ))
    {
        goto cleanup;
    }
    inchi_builder = IXA_INCHIBUILDER_Create( status );
    if (!CheckStatus( status, index ))
    {
        goto cleanup;
    }

    /* Read any command line options that may have been given and set up the InChI
       builder accordingly. */
    if (!ReadOptions( argc, argv, &key, &round_trip, &generate_auxinfo, &verbose, options, status, inchi_builder ))
    {
        fprintf( stderr, "Please check the command line parameters.\n" );
        goto cleanup;
    }

    /* If InChI keys have been requested, create an InChI key builder object. */
    if (key)
    {
        key_builder = IXA_INCHIKEYBUILDER_Create( status );
        if (!CheckStatus( status, index ))
        {
            goto cleanup;
        }
    }

    /* Warning: in this demo, we just assume that molfile's size does not    */
    /* exceed some (large) MOLBUFSIZE; no further checks are performed.        */
    buffer = (char *) calloc( MOLBUFSIZE, sizeof( char ) );
    if (!buffer)
    {
        fprintf( stderr, "Out of memory\n" );
        goto cleanup;
    }

    /* Iterate the molecules in the input SD file and create an InChI for each one. */
    index = 0;
    while (get_next_molfile_as_text( sdfile, buffer, MOLBUFSIZE ) > 0 &&
            !is_empty_text( buffer ))
    {

        fprintf( outfile, "%-ld\t", index + 1 );

        /* Set up the molecule object with atoms and bonds to represent the data that
            has just been read from the input file. */
        IXA_MOL_ReadMolfile( status, molecule, buffer );

        if (!CheckStatus( status, index + 1 ))
        {
            nerr++;
            if ( IXA_TRUE==IXA_INCHIBUILDER_CheckOption(status, inchi_builder, IXA_INCHIBUILDER_OPTION_OutErrInChI) )
            { 
                /* output 'empty InChI' and InChIKey, AuxInfo if applicable */
                fprintf(outfile, "%s", "InChI=1//\t");
                if (IXA_FALSE == IXA_INCHIBUILDER_CheckOption(status, inchi_builder, IXA_INCHIBUILDER_OPTION_AuxNone))
                {
                    fprintf(outfile, "\t"); /* empty AuxInfo */
                }
                if (key_builder)
                {
                    fprintf(outfile, "InChIKey=%s\t", "MOSFIJXAXDLOML-UHFFFAOYNA-N"); /* 'empty' InChIKey */
                }			
            }
            goto endloop;
        }

        /* Bind the molecule object to the InChI builder object. */
        IXA_INCHIBUILDER_SetMolecule( status, inchi_builder, molecule );
        if (!CheckStatus( status, index + 1 ))
        {
            nerr++;
            goto endloop;
        }

        /* Retrieve the molecule's InChI and write it to the output file. */
        inchi = IXA_INCHIBUILDER_GetInChI( status, inchi_builder );
        
        if (!CheckStatus( status, index + 1 ))
        {
            /* Error(s) occurred */
            nerr++;
            if (inchi) /* here it should be only empty InChI "InChI=1//" (and only if OutErrINCHI switch was supplied to generator) */
            {
                /* output empty InChI and empty InChIKey, AuxInfo if applicable */
                fprintf(outfile, "%s\t", inchi); 
                if (!strcmp(inchi, "InChI=1//"))
                {
                    if (IXA_FALSE == IXA_INCHIBUILDER_CheckOption(status, inchi_builder, IXA_INCHIBUILDER_OPTION_AuxNone))
                    {
                        /* AuxInfo is requested, output empty AuxInfo == skip that field */
                        fprintf(outfile, "\t"); 
                    }
                }
                if (key_builder)
                {
                    /* output 'empty InChI' Key */
                    if (!strcmp(inchi, "InChI=1//"))
                    {
                        fprintf(outfile, "InChIKey=%s\t", "MOSFIJXAXDLOML-UHFFFAOYNA-N"); 
                    }
                    else if (!strcmp(inchi, "InChI=1S//"))
                    {
                        fprintf(outfile, "InChIKey=%s\t", "MOSFIJXAXDLOML-UHFFFAOYSA-N"); 
                    }
                }

            }
            goto endloop;
        }

        /* No errors, proceed */   
        /* But account for that inchi may be NULL due to e.g. skip record for any reason */
        if (!inchi) 
        {
            goto endloop;
        }

        fprintf(outfile, "%s\t", inchi);
        if (generate_auxinfo)
        {
            /* AuxInfo has been requested. Retrieve the molecule's AuxInfo and write 
                it to the output file. */
            auxinfo = IXA_INCHIBUILDER_GetAuxInfo( status, inchi_builder );
            if (!CheckStatus( status, index + 1 ))
            {
                nerr++;
                goto endloop;
            }
            fprintf( outfile, "%s\t", auxinfo );
        }

        if (key_builder)
        {
            /* InChI keys have been requested. Retrieve the molecule's InChI key and
                write it to the output file. */
            IXA_INCHIKEYBUILDER_SetInChI( status, key_builder, inchi );
            if (!CheckStatus( status, index + 1 ))
            {
                nerr++;
                goto endloop;
            }

            inchikey = IXA_INCHIKEYBUILDER_GetInChIKey( status, key_builder );
            if (!CheckStatus( status, index + 1 ))
            {
                nerr++;
                goto endloop;
            }

            fprintf( outfile, "InChIKey=%s\t", inchikey );
        }

        if (round_trip)
        {
            /* Round trip tests have been requested. Turn the InChI that has just been
                generated into a molecule and use that molecule as the basis for a new
                InChI. The two InChIs should be identical. */

            saved_inchi = (char *) realloc( saved_inchi, strlen( inchi ) + 1 );
            strcpy( saved_inchi, inchi );
            if (generate_auxinfo && auxinfo && RTRIP_COMPARE_AUX_ALSO==1)
            {
                saved_inchi = (char *) realloc( saved_inchi, strlen( inchi ) + strlen( auxinfo ) + 2 );
                strcpy( saved_inchi, inchi );
                strcat( saved_inchi, "\n" );
                strcat( saved_inchi, auxinfo );
            }

            /*fprintf(stderr,"*** input=%s\n", saved_inchi);*/

            IXA_MOL_ReadInChI( status, molecule, saved_inchi );
            if (!CheckStatus( status, index + 1 ))
            {
                fprintf( stderr, "\n!!! Round trip failed: could not read InChI : structure %-ld\n", index + 1 );
                { nboom_rtrip++; goto endloop; }
            }
            IXA_INCHIBUILDER_SetMolecule( status, inchi_builder, molecule );
            if (!CheckStatus( status, index + 1 ))
            {
                fprintf( stderr, "\n!!! Round trip failed: could not set IXA mol from read InChI string : structure %-ld\n", index + 1 );
                { nboom_rtrip++; goto endloop; }
            }
            /* strcpy(inchi,"NOTHING"); */
            inchi = IXA_INCHIBUILDER_GetInChI( status, inchi_builder );
            if (!CheckStatus( status, index + 1 ))
            {
                fprintf( stderr, "\n!!! Round trip failed: could not generate InChI from IXA mol obtained from read InChI : structure %-ld\n", index + 1 );
                { nboom_rtrip++; goto endloop; }
            }

            if (generate_auxinfo)
            {
                /* AuxInfo has been requested. Retrieve the molecule's AuxInfo and write
                    it to the output file. */
                auxinfo = IXA_INCHIBUILDER_GetAuxInfo( status, inchi_builder );
                if (!CheckStatus( status, index + 1 ) && RTRIP_COMPARE_AUX_ALSO==1)
                {
                    fprintf( stderr, "\n!!! Round trip failed: could not generate AuxInfo from IXA mol obtained from read InChI : structure %-ld\n", index + 1 );
                    { nboom_rtrip++; goto endloop; }
                }
            }

            new_inchi = (char *) realloc( new_inchi, strlen( inchi ) + 1 );
            strcpy( new_inchi, inchi );
            if (generate_auxinfo && auxinfo && RTRIP_COMPARE_AUX_ALSO==1)
            {
                new_inchi = (char *) realloc( new_inchi, strlen( inchi ) + strlen( auxinfo ) + 2 );
                strcpy( new_inchi, inchi );
                strcat( new_inchi, "\n" );
                strcat( new_inchi, auxinfo );
            }
            /*fprintf(stderr,"*** output=%s\n", new_inchi );*/


            /* Check saved vs. new */
            if (strcmp( new_inchi, saved_inchi ) != 0)
            {
                fprintf( stderr, "\n!!! Round trip failed : structure %-ld\n", index + 1 );
                fprintf( stderr, "   OLD: %s\n", saved_inchi );
                fprintf( stderr, "   NEW: %s", new_inchi );
                nmismatch_rtrip++;
            }
            else if (verbose)
            {
                fprintf( stderr, "\nRound trip OK : structure %-ld\n", index + 1 );
                fprintf( stderr, "   OLD: %s\n", saved_inchi );
                fprintf( stderr, "   NEW: %s", new_inchi );
            }
        } /* Round trip ends here */


endloop:
        /* Every so often write a dot to stdout so the user knows that this program has
        not frozen. */
        fprintf( outfile, "\n" );
        if (index % 100 == 0)
        {
            fprintf( stderr, "." );
        }
        index++;
        fflush( NULL );
    } /* Main loop */

    fprintf( stderr, "\n" );

cleanup:
    /* All done. */
    fprintf( stderr, "\nFinished processing %-ld molecules.\nGeneration (struct->InChI) errors: %-ld",
                    index, nerr );
    if (round_trip)
    {
        fprintf( stderr,
            "\nRound trip (struct->InChI->struct->InChI) problems: %-ld (%-ld failures, %-ld mismatches)",
                 nboom_rtrip + nmismatch_rtrip, nboom_rtrip, nmismatch_rtrip );
    }
    fprintf( stderr, ".\n" );

#if ( defined(_WIN32) && defined(_MSC_VER) )
#if WINVER >= 0x0501 /* XP or newer */ /* 0x0600 Vista or newer */
    tick_ixa_stop = GetTickCount( );
    fprintf( stderr, "\nElapsed time: %d msec.\n", tick_ixa_stop - tick_ixa_start );
#endif
#else
    time( &time_t_ixa_stop );
    fprintf( stderr, "\nElapsed time: %g sec.\n", difftime( time_t_ixa_stop, time_t_ixa_start ) );
#endif

    /* Release resources prior to program exit. */
    if (saved_inchi)
    {
        free( saved_inchi );
    }
    if (new_inchi)
    {
        free( new_inchi );
    }
    if (buffer)
    {
        free( buffer );
    }
    if (sdfile)
    {
        fclose( sdfile );
    }
    if (outfile)
    {
        fclose( outfile );
    }

    IXA_INCHIKEYBUILDER_Destroy( NULL, key_builder );
    IXA_INCHIBUILDER_Destroy( NULL, inchi_builder );
    IXA_MOL_Destroy( NULL, molecule );
    IXA_STATUS_Destroy( status );
}


/****************************************************************************
 Print program usage instructions
****************************************************************************/
void print_help( void )
{
    fprintf( stderr, "\nUsage: test_ixa <sd filename> <output filename> [/option[ /option...]]\n\n\n" );
    fprintf( stderr, "Options:\n" );
    fprintf( stderr, "  RTrip       Do a round trip test for each InChI generated\n" );
    fprintf( stderr, "  Key         Generate InChIKey\n" );
    fprintf( stderr, "  AuxNone     Omit auxiliary information (default: Include)\n");
    fprintf( stderr, "  WarnOnEmptyStructure Warn and produce empty InChI for empty structure\n");
    fprintf( stderr, "  NoWarnings  Suppress all warning messages(default: show)\n");
    fprintf( stderr, "  OutErrInChI On fail, print empty InChI (default: nothing)\n");
    fprintf( stderr, "  SaveOpt     Save custom InChI creation options (non-standard InChI)\n");
    fprintf( stderr, "  SNon        Exclude stereo (default: include absolute stereo)\n" );
    fprintf( stderr, "  SRel        Relative stereo\n" );
    fprintf( stderr, "  SRac        Racemic stereo\n" );
    fprintf( stderr, "  SUCF        Use Chiral Flag: On means Absolute stereo, Off - Relative\n" );
    fprintf( stderr, "  NEWPSOFF    Both ends of wedge point to stereocenters (default: a narrow end)\n" );
    fprintf( stderr, "  LooseTSACheck   Relax criteria of ambiguous drawing for in-ring tetrahedral stereo\n");
    fprintf( stderr, "  DoNotAddH   All H are explicit (default: add H according to usual valences)\n" );
    fprintf( stderr, "  SUU         Always include omitted unknown/undefined stereo\n" );
    fprintf( stderr, "  SLUUD       Make labels for unknown and undefined stereo different\n" );
    fprintf( stderr, "  FixedH      Include Fixed H layer\n" );
    fprintf( stderr, "  RecMet      Include reconnected metals results\n" );
    fprintf( stderr, "  KET         Account for keto-enol tautomerism (experimental)\n" );
    fprintf( stderr, "  15T         Account for 1,5-tautomerism (experimental)\n" );
    fprintf( stderr, "  Wnumber     Set time-out per structure in seconds; W0 means unlimited\n" );
    fprintf( stderr, "  WMnumber    Set time-out per structure in milliseconds (int); WM0 means unlimited\n");
    fprintf( stderr, "  LargeMolecules Treat molecules up to 32766 atoms (experimental)\n" );
    fprintf( stderr, "  Polymers     Allow processing of polymers (experimental)\n");
    fprintf( stderr, "  Polymers105  Allow processing of polymers (experimental, legacy mode of v. 1.05)\n");
    fprintf( stderr, "  FoldCRU      Remove repeats within constitutional repeating units (CRU/SRU)\n");
    fprintf( stderr, "  NPZz         Allow non-polymer-related Zz atoms (pseudo element placeholders)\n");
    fprintf( stderr, "  SAtZz        Allow stereo at atoms connected to Zz\n");
#ifdef  BUILD_WITH_ENG_OPTIONS
    fprintf(stderr, "Engineering options:\n");
    fprintf(stderr, "  DoDrv                                                    \n");
    fprintf(stderr, "  DoDrvReport                                              \n");
    fprintf(stderr, "  DoR2C                                                    \n");
    fprintf(stderr, "  DoneOnly                                                 \n");
    fprintf(stderr, "  OnlyRecSalt                                              \n");
    fprintf(stderr, "  OnlyExact                                                \n");
    fprintf(stderr, "  OnlyRecMet                                               \n");
    fprintf(stderr, "  Polymers105+                                             \n");
    fprintf(stderr, "  FilterSS                                                 \n");
    fprintf(stderr, "  InvFilterSS                                              \n");
#endif
}
