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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

#ifndef COMPILE_ANSI_ONLY
#include <conio.h>
#ifndef TARGET_LIB_FOR_WINCHI
#include <windows.h>
#endif
#endif

#include "../../INCHI_BASE/src/mode.h"

#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
#include <malloc.h>
#include <io.h>
#endif

#include "../../INCHI_BASE/src/ichitime.h"
#include "../../INCHI_BASE/src/incomdef.h"
#include "../../INCHI_BASE/src/ichidrp.h"
#include "../../INCHI_BASE/src/inpdef.h"
#include "../../INCHI_BASE/src/ichi.h"
#include "../../INCHI_BASE/src/strutil.h"
#include "../../INCHI_BASE/src/util.h"
#include "../../INCHI_BASE/src/ichierr.h"
#include "../../INCHI_BASE/src/ichimain.h"
#include "../../INCHI_BASE/src/ichicomp.h"
#include "../../INCHI_BASE/src/ichi_io.h"
#ifdef TARGET_EXE_STANDALONE
#include "../../INCHI_BASE/src/inchi_api.h"
#endif

/*  console-specific */

#if !defined(TARGET_API_LIB) && !defined(COMPILE_ANSI_ONLY)
/* Use Windows additional features */

/*****************************************************************************/
int user_quit( struct tagINCHI_CLOCK *ic, const char *msg, unsigned long ulMaxTime )
{
#if defined(TARGET_LIB_FOR_WINCHI)
    return 0;
#endif

#if ( !defined(TARGET_LIB_FOR_WINCHI) && defined(_WIN32) )

    int quit, enter, ret;
    printf(msg);
    if ( ulMaxTime )
    {
        inchiTime  ulEndTime;
        InchiTimeGet( &ulEndTime );
        InchiTimeAddMsec( ic, &ulEndTime, ulMaxTime );
        while ( !_kbhit() ) {
            if ( bInchiTimeIsOver( ic, &ulEndTime ) )
            {
                printf("\n");
                return 0;
            }
            MySleep( 100 );
        }
    }
    while( 1 )
    {
        quit  = ( 'q' == (ret = _getch()) || 'Q'==ret || /*Esc*/ 27==ret );
        enter = ( '\r' == ret );
        if ( ret == 0xE0 )
            ret = _getch();
        else
            _putch(ret); /* echo */
        if ( quit || enter )
            break;
        printf( "\r" );
        printf( msg );
    }
    _putch('\n');
    return quit;

#else

    return 0;

#endif    /* #if ( defined(_WIN32) && !defined(TARGET_LIB_FOR_WINCHI) ) */
}


/*****************************************************************************/
void eat_keyboard_input( void )
{
#ifndef TARGET_LIB_FOR_WINCHI

    while ( _kbhit() )
    {
        if ( 0xE0 == _getch() )
            _getch();
    }

#endif
}

#endif /* end of !COMPILE_ANSI_ONLY */




#ifndef TARGET_LIB_FOR_WINCHI
                    /* COVERS THE CODE FROM HERE TO THE END OF FILE */



/* Enable/disable internal tests */

/*#define TEST_FPTRS*/    /* uncomment for INCHI_LIB testing only */
#define REPEAT_ALL  0    /* set to 1 for mapping tests */


/* Windows-console-mode specific */

int bInterrupted = 0;

#if ( defined( _WIN32 ) && defined( _CONSOLE ) )
#ifndef COMPILE_ANSI_ONLY
BOOL WINAPI MyHandlerRoutine(
  DWORD dwCtrlType   /*   control signal type */
  ) {
    if ( dwCtrlType == CTRL_C_EVENT     ||
         dwCtrlType == CTRL_BREAK_EVENT ||
         dwCtrlType == CTRL_CLOSE_EVENT ||
         dwCtrlType == CTRL_LOGOFF_EVENT ) {
        bInterrupted = 1;
        return TRUE;
    }
    return FALSE;
}
int WasInterrupted(void)
{
#ifdef _DEBUG
    if ( bInterrupted )
    {
        int stop=1;  /*  for debug only <BRKPT> */
    }
#endif
    return bInterrupted;
}

#if ( BUILD_WITH_AMI == 1 )
#define CTRL_STOP_EVENT 101
#endif
#endif /* ifndef COMPILE_ANSI_ONLY */
#endif /* if( defined( _WIN32 ) && defined( _CONSOLE ) ) */





/*****************************************************************************/
int main( int argc, char *argv[ ] )
{

/*************************/
#if ( BUILD_WITH_AMI == 1 )
/*************************/

                                /* if in AMI mode, main() starts here and   */
                                /* captures command line to pass to actual  */
                                /* workers - ProcessSingleInputFile()       */
                                /*           ProcessMultipleInputFiles()    */
int i, ret=0, ami=0;

    /* Check if multiple inputs expected */
    for (i=1; i < argc; i++)
    {
        if (argv[i][0] == INCHI_OPTION_PREFX)
        {
            if ( !inchi_stricmp(argv[i]+1, "AMI") )
            {
                ami = 1;
                break;
            }
        }
    }

    if ( ami )
        ret = ProcessMultipleInputFiles( argc, argv);
    else
        ret = ProcessSingleInputFile(argc,argv);

    return 0;
}



/*******************************************************/
int ProcessMultipleInputFiles( int argc, char *argv[ ] )
/*******************************************************/
{
    int i, ret=0, nfn_ins=0,
        AMIOutStd=0, AMILogStd=0, AMIPrbNone=0;
    char *fn_out, *fn_log, *fn_prb;
    char pNUL[] = "NUL";
    char **fn_ins=NULL, **targv=NULL;


#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
    struct _finddata_t file_info;
    intptr_t hFile=-1;
    int   retFile, lenPath;
    char *pName, *pOutPath=NULL;
    char pathname[_MAX_PATH];
    char szBlank[] = "";
    int  numFiles=0; /* counts processed files */
    #else
    int p;
#endif

    fn_ins = (char**) inchi_calloc( argc, sizeof(char *) );
    if ( !fn_ins )
    {
        fprintf(stderr, "Not enough memory.\n");
        goto exit_ami;
    }

    /* Check for other options and collect inputs. */
    for (i=1; i < argc; i++)
    {
        if (argv[i][0] == INCHI_OPTION_PREFX)
        {
            if ( !inchi_stricmp(argv[i]+1, "STDIO") )
            {
                fprintf( stderr, "Options AMI and STDIO are not compatible.\n" );
                goto exit_ami;
            }
            else if ( !inchi_stricmp(argv[i]+1, "AMIOutStd") )
            {
                AMIOutStd = 1;
            }
            else if ( !inchi_stricmp(argv[i]+1, "AMILogStd") )
            {
                AMILogStd = 1;
            }
            else if ( !inchi_stricmp(argv[i]+1, "AMIPrbNone") )
            {
                AMIPrbNone = 1;
            }
#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
            else if ( !inchi_memicmp(argv[i]+1, "OP:", 3) )
            {
                pOutPath = argv[i]+4; /* output path */
            }
#endif
        }
        else
        {
            fn_ins[nfn_ins] = argv[i];
            nfn_ins++;
        }
    }

    if ( !nfn_ins )
    {
        fprintf(stderr, "At least one input file is expected in AMI mode.\n");
        goto exit_ami;
    }


    targv = (char**) calloc( argc+3, sizeof(char *) );


    if ( !targv )
    {
        fprintf(stderr, "Not enough memory.\n");
        goto exit_ami;
    }

#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
    if ( pName = strrchr( fn_ins[0], INCHI_PATH_DELIM ) ) {
        pName ++;
        lenPath = pName - fn_ins[0];
    } else {
        pName = fn_ins[0];
        lenPath = 0;
    }
    for ( hFile = _findfirst(fn_ins[0], &file_info), retFile=0;
            !retFile&&-1 != hFile;
                retFile = _findnext(hFile, &file_info), numFiles ++ )
#else
    for ( p=0; p < nfn_ins; p++ )
#endif
    {
        int targc;
#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
        const char *fn_in;
        int inlen = lenPath+strlen(file_info.name);
        if ( !file_info.size || (file_info.attrib & _A_SUBDIR) || inlen >= _MAX_PATH )
            continue;
        memcpy( pathname, fn_ins[0], lenPath );
        strcpy( pathname+lenPath, file_info.name );
        fn_in = pathname;
        if ( 0==numFiles%5000 )
            _heapmin(); /* reduce heap fragmentation */
#else
        const char *fn_in = fn_ins[p];
        int inlen = strlen(fn_in);
#endif
        fn_out=fn_log=fn_prb=NULL;

        targv[0] = argv[0];
        targv[1] = (char *) fn_in;
        targc = 1;

        if ( AMIOutStd )
        {
            targv[++targc] = pNUL;
        }
#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 && ALLOW_EMPTY_PATHS == 1 )
        else if ( pOutPath )
        {
            targv[++targc]=szBlank; /* output name will be created in process_single_input(...) */
        }
#endif
        else
        {
            /* make output name as input name plus ext. */
            fn_out = (char*) inchi_calloc( inlen+6, sizeof(char ) );
            if ( fn_out )
            {
                strcpy( fn_out, fn_in );
                strcat( fn_out, ".txt" );
            }
            targv[++targc] = fn_out;
        }

        if ( AMILogStd )
        {
        targv[++targc] = pNUL;
        }
#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 && ALLOW_EMPTY_PATHS == 1 )
        else if ( pOutPath )
        {
            targv[++targc]=szBlank; /* output name will be created in process_single_input(...) */
        }
#endif
        else
        {
            /* make log name as input name plus ext. */
            fn_log = (char*) inchi_calloc( inlen+6, sizeof(char ) );
            if ( fn_log )
            {
                strcpy( fn_log, fn_in );
                strcat( fn_log, ".log" );
            }
            targv[++targc] = fn_log;
        }
        if ( AMIPrbNone )
        {
            targv[++targc] = pNUL;
        }
#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 && ALLOW_EMPTY_PATHS == 1 )
        else if ( pOutPath )
        {
            targv[++targc]=szBlank; /* output name will be created in process_single_input(...) */
        }
#endif
        else
        {
            /* make problem file name as input file name plus ext. */
            fn_prb = (char*) inchi_calloc( inlen+6, sizeof(char ) );
            if ( fn_prb )
            {
                strcpy( fn_prb, fn_in );
                strcat( fn_prb, ".prb" );
            }
            targv[++targc] = fn_prb;
        }

        for (i=1; i < argc; i++)
        {
            if (argv[i][0] == INCHI_OPTION_PREFX)
            {
                /* avoid strnicmp/strncasecmp */
                if ( (strlen(argv[i])>3)&&
                     (toupper(argv[i][1])=='A')&&(toupper(argv[i][2])=='M')&&(toupper(argv[i][3])=='I') )
                        continue;
                targv[++targc] = argv[i];
            }
        }

        targv[++targc] = NULL;

        ret = ProcessSingleInputFile(targc,targv); /* ProcessSingleInputFile() is a former main() */

        if ( fn_out )
            inchi_free( fn_out );
        if ( fn_log )
            inchi_free( fn_log );
        if ( fn_prb )
            inchi_free( fn_prb );

#if ( defined( _WIN32 ) && defined( _CONSOLE ) && !defined( COMPILE_ANSI_ONLY ) )
        if ( ret == CTRL_STOP_EVENT)
            goto exit_ami;
#endif
    }


exit_ami:
#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
    if ( hFile != -1 )
        _findclose( hFile );
#endif
    if ( targv )
        inchi_free( targv );
    if ( fn_ins )
        inchi_free( fn_ins );

    return 0;
}



/**************************************************/
int ProcessSingleInputFile( int argc, char *argv[ ] )
/**************************************************/
{

/**************************************/
#endif /* #if ( BUILD_WITH_AMI == 1 ) */
/**************************************/

    /* if not in AMI mode, main() starts here */

    int bReleaseVersion = bRELEASE_VERSION;
    const int nStrLen = INCHI_SEGM_BUFLEN;
    int   nRet = 0, nRet1;
    int    i;
    long num_err, num_output, num_inp;
    /* long rcPict[4] = {0,0,0,0}; */
    unsigned long  ulDisplTime = 0;    /*  infinite, milliseconds */
    unsigned long  ulTotalProcessingTime = 0;

    CANON_GLOBALS CG;
    INCHI_CLOCK ic;

    char szTitle[MAX_SDF_HEADER+MAX_SDF_VALUE+256];
    char szSdfDataValue[MAX_SDF_VALUE+1];

    INPUT_PARMS inp_parms;
    INPUT_PARMS *ip = &inp_parms;

    STRUCT_DATA struct_data;
    STRUCT_DATA *sd = &struct_data;

    ORIG_ATOM_DATA OrigAtData; /* 0=> disconnected, 1=> original */
    ORIG_ATOM_DATA *orig_inp_data = &OrigAtData;
    ORIG_ATOM_DATA PrepAtData[2]; /* 0=> disconnected, 1=> original */
    ORIG_ATOM_DATA *prep_inp_data = PrepAtData;

    PINChI2     *pINChI[INCHI_NUM];
    PINChI_Aux2 *pINChI_Aux[INCHI_NUM];

    char *pLF, *pTAB;
    INCHI_IOSTREAM_STRING temp_string_container;
    INCHI_IOSTREAM_STRING *strbuf = &temp_string_container;
    INCHI_IOSTREAM outputstr, logstr, prbstr, instr;
    INCHI_IOSTREAM *pout=&outputstr, *plog = &logstr, *pprb = &prbstr, *inp_file = &instr;
#ifdef TARGET_EXE_STANDALONE
    char ik_string[256];    /*^^^ Resulting InChIKey string */
    int ik_ret=0;           /*^^^ InChIKey-calc result code */
    int xhash1, xhash2;
    char szXtra1[65], szXtra2[65];
    int inchi_ios_type = INCHI_IOSTREAM_TYPE_STRING;
#else
    int inchi_ios_type = INCHI_IOSTREAM_TYPE_FILE;
#endif

int have_err_in_GetOneStructure = 0;
int output_error_inchi = 0;


    /* internal tests --- */
#ifndef TEST_FPTRS
    STRUCT_FPTRS *pStructPtrs = NULL;
#else
    STRUCT_FPTRS struct_fptrs, *pStructPtrs =&struct_fptrs; /* INCHI_LIB debug only */
#endif
#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    int  num_repeat = REPEAT_ALL;
#endif


#if ( TRACE_MEMORY_LEAKS == 1 )
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
/* for execution outside the VC++ debugger uncomment one of the following two */
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
    _CrtSetReportMode(_CRT_WARN | _CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
   /* turn on floating point exceptions */
    {
        /* Get the default control word. */
        int cw = _controlfp( 0,0 );

        /* Set the exception masks OFF, turn exceptions on. */
        /*cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_INEXACT|EM_ZERODIVIDE|EM_DENORMAL);*/
        cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_ZERODIVIDE|EM_DENORMAL);

        /* Set the control word. */
        _controlfp( cw, MCW_EM );
    }
#endif

#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
repeat:
    inchi_ios_close(inp_file);
    inchi_ios_close(pout);
    inchi_ios_close(plog);
    inchi_ios_close(pprb);
    pStr = NULL;
#endif

/* --- internal tests */

    sd->bUserQuit  = 0;

#if ( defined( _WIN32 ) && defined( _CONSOLE ) && !defined( COMPILE_ANSI_ONLY ) )
    if ( SetConsoleCtrlHandler( MyHandlerRoutine, 1 ) )
        ConsoleQuit = WasInterrupted;
#endif

    num_inp    = 0;
    num_err    = 0;
    num_output = 0;

    inchi_ios_init(inp_file, INCHI_IOSTREAM_TYPE_FILE, NULL);
    inchi_ios_init(pout, inchi_ios_type, NULL);
    inchi_ios_init(plog, inchi_ios_type, stdout);
    inchi_ios_init(pprb, inchi_ios_type, NULL);
    memset( strbuf, 0, sizeof(strbuf) );



    if ( argc == 1 || argc==2 && ( argv[1] [0]==INCHI_OPTION_PREFX ) &&
        (!strcmp(argv[1]+1, "?") || !inchi_stricmp(argv[1]+1, "help") ) )
    {
        HelpCommandLineParms(plog);
        inchi_ios_flush(plog);
        return 0;
    }

    /*  original input structure */
    memset( orig_inp_data     , 0,   sizeof( *orig_inp_data  ) );
    memset( prep_inp_data     , 0, 2*sizeof( *prep_inp_data  ) );
    memset( pINChI,     0, sizeof(pINChI    ) );
    memset( pINChI_Aux, 0, sizeof(pINChI_Aux) );
    memset( szSdfDataValue    , 0, sizeof( szSdfDataValue    ) );

    memset( &CG, 0, sizeof(CG));
    memset( &ic, 0, sizeof(ic));

    plog->f = stderr;

    if ( 0 > ReadCommandLineParms( argc, (const char **)argv,
                                   ip,
                                   szSdfDataValue,
                                   &ulDisplTime,
                                   bReleaseVersion,
                                   plog) )
    {
        goto exit_function;
    }


    if ( !OpenFiles( &(inp_file->f), &(pout->f), &(plog->f), &(pprb->f), ip ) )
    {
        goto exit_function;
    }


    if ( ip->bNoStructLabels )
    {
        ip->pSdfLabel = NULL;
        ip->pSdfValue = NULL;
    }
    else if ( ip->nInputType == INPUT_INCHI_PLAIN  ||
              ip->nInputType == INPUT_INCHI  )
    {
        /* the input may contain both the header and the label of the structure */
        if ( !ip->pSdfLabel )
            ip->pSdfLabel  = ip->szSdfDataHeader;
        if ( !ip->pSdfValue )
            ip->pSdfValue  = szSdfDataValue;
    }

    set_line_separators( ip->bINChIOutputOptions, &pLF, &pTAB );

    save_command_line( argc, argv, plog);


    PrintInputParms(plog,ip);

    inchi_ios_flush2(plog, stderr);

    if ( 0>=inchi_strbuf_init( strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT ) )
    {
        inchi_ios_eprint( plog, "Cannot allocate internal string buffer. Terminating\n");
        inchi_ios_flush2(plog, stderr);
        goto exit_function;
    }


#if ( READ_INCHI_STRING == 1 )

    if ( ip->nInputType == INPUT_INCHI )
    {
        const int bInChI2Structure = 0 != (ip->bReadInChIOptions & READ_INCHI_TO_STRUCTURE);
        memset( sd, 0, sizeof(*sd) );

        if ( bInChI2Structure )
        {
            /* loop through file lines here */
            INCHI_IOSTREAM tmpinpustream;
            INCHI_IOSTREAM_STRING *pTmpIn= &tmpinpustream.s;
            int crlf2lf = 0, preserve_lf = 1, read_result = 0;
            inchi_ios_init( &tmpinpustream, INCHI_IOSTREAM_TYPE_STRING, NULL );
            while ( 1 )
            {
                char *p, *pi;

                read_result = inchi_strbuf_getline( pTmpIn, inp_file->f, crlf2lf, preserve_lf );

                if ( read_result == -1 )
                    break;    /* EOF or read error */

                p = pTmpIn->pStr;
                if ( !p )
                    continue;
                pi = strstr( p, "InChI=1" );
                if ( pi != p )
                    continue;

                num_inp++;
                ip->lMolfileNumber = num_inp;

                ReadWriteInChI( &tmpinpustream, pout, plog, ip,  sd, NULL, 0, 0, NULL, NULL, NULL, 0, NULL, &ic, &CG);

                /*fprintf( stderr, "%ld", num_inp );*/
                inchi_strbuf_reset( pTmpIn );
                inchi_ios_flush2(plog, stderr);
            }
            fprintf( stderr, "\r" );
            inchi_strbuf_close( pTmpIn);
        }
        else
        {
            /* loop through file lines within ReadWriteInChI */
            ReadWriteInChI( inp_file, pout, plog, ip,  sd, NULL, 0, 0, NULL, NULL, NULL, 0, NULL, &ic, &CG);

            num_inp = sd->fPtrStart;
            num_err = sd->fPtrEnd;
        }

        inchi_ios_flush2(plog, stderr);
        ulTotalProcessingTime = sd->ulStructTime;
        /*num_inp               = sd->fPtrStart;
        num_err               = sd->fPtrEnd;
        */
        goto exit_function;
    }

#endif


    ulTotalProcessingTime = 0;
    if ( pStructPtrs )
    {
        memset ( pStructPtrs, 0, sizeof(pStructPtrs[0]) );
    }

#ifdef ERR_INCHI_STRING_ALLOWED
    output_error_inchi = ip->bINChIOutputOptions2 & INCHI_OUT_INCHI_GEN_ERROR;
#endif

    /*************************************************************/
    /*  Main cycle : read input structures and create their INChI                                  */
    /*************************************************************/
    while ( !sd->bUserQuit && !bInterrupted )
    {

        if ( ip->last_struct_number && num_inp >= ip->last_struct_number )
        {
            nRet = _IS_EOF; /*  simulate end of file */
            goto exit_function;
        }

        /*  Get (the next) one structure from input stream */
        nRet = GetOneStructure( &ic, sd, ip,
                                szTitle, inp_file,
                                plog, pout,  pprb,
                                orig_inp_data, &num_inp,
                                pStructPtrs );

        inchi_ios_flush2(plog, stderr);

        if ( pStructPtrs )
            pStructPtrs->cur_fptr ++;

        if ( sd->bUserQuit )
            break;

        have_err_in_GetOneStructure = 0;
        switch ( nRet )
        {
            case _IS_FATAL:
                num_err ++;
                if ( output_error_inchi == 0 )
                    goto exit_function;
                else
                    sd->pStrErrStruct[0] = '\0'; /* depress re-appearance of error as warning in ProcessOneStr */
                break;
            case _IS_EOF:
                    goto exit_function;
            case _IS_ERROR:
                num_err ++;
                have_err_in_GetOneStructure = 1;
                if ( output_error_inchi == 0 )
                    continue;
                else
                    sd->pStrErrStruct[0] = '\0'; /* depress re-appearance of error as warning in ProcessOneStr */
                break;
            case _IS_SKIP:
                continue;
            default:
                sd->pStrErrStruct[0] = '\0'; /* depress re-appearance of error as warning in ProcessOneStr */
        }


        /*
            Create INChI for each connected component of the structure;
            optionally display them;
            output INChI for the whole structure
        */

        if ( orig_inp_data->polymer )
        {
            if ( ip->bPolymers )
            {
                orig_inp_data->polymer->valid = 1;
            }
            else
            {
                inchi_ios_eprint( plog, "Ignore polymer data" );
                orig_inp_data->polymer->valid = 0;
            }
        }

        nRet1 = ProcessOneStructureEx( &ic, &CG, sd, ip, szTitle,
                                       pINChI,pINChI_Aux,
                                       inp_file, plog, pout, pprb,
                                       orig_inp_data, prep_inp_data,
                                       num_inp, strbuf,
                                       0 /* save_opt_bits */);

        inchi_ios_flush2(plog, stderr);

#ifdef TARGET_EXE_STANDALONE
        /* correctly treat tabbed output with InChIKey */
        if ( ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT )
            if ( ip->bCalcInChIHash != INCHIHASH_NONE )
                if (pout->s.pStr)
                    if (pout->s.nUsedLength>0)
                        if (pout->s.pStr[pout->s.nUsedLength-1]=='\n')
                            /* replace LF with TAB */
                            pout->s.pStr[pout->s.nUsedLength-1] = '\t';
#endif

        /*  free INChI memory */
        FreeAllINChIArrays( pINChI, pINChI_Aux, sd->num_components );

        /* free structure data */
        FreeOrigAtData( orig_inp_data );
        FreeOrigAtData( prep_inp_data );
        FreeOrigAtData( prep_inp_data+1 );

        ulTotalProcessingTime += sd->ulStructTime;

        if (nRet1 == _IS_SKIP )
            continue;

        nRet = inchi_max(nRet, nRet1);
        switch ( nRet )
        {

            case _IS_FATAL:
                if ( !have_err_in_GetOneStructure )
                /* increment error counter only if error is not appear earlier */
                    num_err ++;
                if ( output_error_inchi )
                    emit_error_inchi_text( ip,num_inp, pLF, pTAB, pout);
                else
                    goto exit_function;
                break;
            case _IS_ERROR:
                if ( !have_err_in_GetOneStructure )
                /* increment error counter only if error is not appear earlier */
                    num_err ++;
                if ( output_error_inchi )
                    emit_error_inchi_text( ip,num_inp, pLF, pTAB, pout);
                else
                    continue;
                break;
            case _IS_SKIP:
                continue;
        }

#ifdef TARGET_EXE_STANDALONE
        if ( ip->bCalcInChIHash != INCHIHASH_NONE )
        {
            char *buf = NULL;
            size_t slen = pout->s.nUsedLength;

            extract_inchi_substring(&buf, pout->s.pStr, slen);

            if (NULL==buf)
            {
                ik_ret = INCHIKEY_NOT_ENOUGH_MEMORY;
            }
            else
            {
                xhash1 = xhash2 = 0;
                if ( ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1 ) ||
                     ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1_XTRA2 ) )
                     xhash1 = 1;
                if ( ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA2 ) ||
                     ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1_XTRA2 ) )
                     xhash2 = 1;
                ik_ret = GetINCHIKeyFromINCHI( buf,
                                               xhash1,
                                               xhash2,
                                               ik_string,
                                               szXtra1,
                                               szXtra2);
                inchi_free(buf);
            }

            if (ik_ret==INCHIKEY_OK)
            {
                /* NB: correctly treat tabbed output with InChIKey & hash extensions */
                char csep = '\n';

#ifdef TARGET_EXE_STANDALONE
                if ( ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT )
                    csep = '\t';
#endif

                inchi_ios_print(pout, "InChIKey=%-s",ik_string);
                if ( xhash1 )
                    inchi_ios_print(pout, "%cXHash1=%-s",csep,szXtra1);
                if ( xhash2 )
                    inchi_ios_print(pout, "%cXHash2=%-s",csep,szXtra2);
                inchi_ios_print(pout, "\n");
            }
            else
            {
                inchi_ios_print(plog, "Warning (Could not compute InChIKey: ", num_inp);
                switch(ik_ret)
                {
                    case INCHIKEY_UNKNOWN_ERROR:
                        inchi_ios_print(plog, "unresolved error)");
                        break;
                    case INCHIKEY_EMPTY_INPUT:
                        inchi_ios_print(plog,  "got an empty string)");
                        break;
                    case INCHIKEY_INVALID_INCHI_PREFIX:
                    case INCHIKEY_INVALID_INCHI:
                    case INCHIKEY_INVALID_STD_INCHI:
                        inchi_ios_print(plog, "got non-InChI string)");
                        break;
                    case INCHIKEY_NOT_ENOUGH_MEMORY:
                        inchi_ios_print(plog, "not enough memory to treat the string)");
                        break;
                    default:inchi_ios_print(plog, "internal program error)");
                        break;
                }
                inchi_ios_print(plog, " structure #%-lu.\n", num_inp);
                if ( ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT )
                    inchi_ios_print(pout, "\n");
            }
            inchi_ios_flush(pout);
            inchi_ios_flush2(plog, stderr);
        }

        else
            inchi_ios_flush(pout);

#endif

   /*   --- debug only ---
        if ( pStructPtrs->cur_fptr > 5 ) {
            pStructPtrs->cur_fptr = 5;
        }
   */
    } /* end of main cycle - while ( !sd->bUserQuit && !bInterrupted ) */



exit_function:

    /* avoid memory leaks in case of fatal error */
    if ( pStructPtrs && pStructPtrs->fptr )
{
        inchi_free( pStructPtrs->fptr );
    }

    /*  free INChI memory */
    FreeAllINChIArrays( pINChI, pINChI_Aux, sd->num_components );
    /* free structure data */
    FreeOrigAtData( orig_inp_data );
    FreeOrigAtData( prep_inp_data );
    FreeOrigAtData( prep_inp_data+1 );


    /* close output(s) */
    inchi_ios_close(inp_file);
    inchi_ios_close(pout);
    inchi_ios_close(pprb);


    {
        int hours, minutes, seconds, mseconds;

        SplitTime( ulTotalProcessingTime, &hours, &minutes, &seconds, &mseconds );

        inchi_ios_eprint( plog, "Finished processing %ld structure%s: %ld error%s, processing time %d:%02d:%02d.%02d\n",
                                num_inp, num_inp==1?"":"s",
                                num_err, num_err==1?"":"s",
                                hours, minutes, seconds,mseconds/10);
        inchi_ios_flush2(plog, stderr);
    }


    /* cleanup */
    inchi_ios_close( plog );
    inchi_strbuf_close( strbuf );
    for ( i = 0; i < MAX_NUM_PATHS; i ++ )
    {
        if ( ip->path[i] )
        {
            inchi_free( (void*) ip->path[i] ); /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }
    SetBitFree( &CG );

/* internal tests --- */
#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    if ( num_repeat-- > 0 )
        goto repeat;
#endif

#if ( ( BUILD_WITH_AMI==1 ) && defined( _WIN32 ) && defined( _CONSOLE ) && !defined( COMPILE_ANSI_ONLY ) )
    if ( bInterrupted )
        return CTRL_STOP_EVENT;
#endif

    return 0;
}





/*****************************************************************************/
void save_command_line( int argc, char *argv[ ], INCHI_IOSTREAM *plog)
{
int k;

    inchi_ios_eprint( plog, "The command line used:\n\"");
    for(k=0; k<argc-1; k++)
#if( ALLOW_EMPTY_PATHS == 1 )
        inchi_ios_eprint( plog, "%-s ", argv[k][0]? argv[k] : "\"\""  );
#else
        inchi_ios_eprint( plog, "%-s ",argv[k]);
#endif
    inchi_ios_eprint( plog, "%-s\"\n", argv[argc-1]);

    return;
}



/*****************************************************************************/
void emit_error_inchi_text( INPUT_PARMS *ip,
                            long num_inp,
                            char *pLF,
                            char *pTAB,
                            INCHI_IOSTREAM *pout)
                {
                    if ( !ip->bNoStructLabels )
                    {
                        if ( !(ip->pSdfLabel&&ip->pSdfLabel[0]) && !(ip->pSdfValue&&ip->pSdfValue[0]) )
                        {
                            inchi_ios_print( pout, "%sStructure: %ld%s",
                                             pLF, num_inp, pTAB );
                        }
                        else
                        {
                            inchi_ios_print( pout, "%sStructure: %ld.%s%s%s%s%s",
                                             pLF,
                                             num_inp,
                                             SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue),
                                             pTAB );
                        }
                    }
                    if ( ip->bINChIOutputOptions & INCHI_OUT_STDINCHI )
                        inchi_ios_eprint( pout, "InChI=1S//\n"); /* emit err string */
                    else
                        inchi_ios_eprint( pout, "InChI=1//\n"); /* emit err string */
}

#endif  /* ifndef TARGET_LIB_FOR_WINCHI */
