/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.04
 * September 9, 2011
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST. Modifications and additions by IUPAC 
 * and the InChI Trust.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the 
 * International Chemical Identifier (InChI) Software version 1.04
 * Copyright (C) IUPAC and InChI Trust Limited
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
 * or any later version.
 * 
 * Please note that this library is distributed WITHOUT ANY WARRANTIES 
 * whatsoever, whether expressed or implied.  See the IUPAC/InChI Trust 
 * Licence for the International Chemical Identifier (InChI) Software 
 * version 1.04, October 2011 ("IUPAC/InChI-Trust InChI Licence No.1.0") 
 * for more details.
 * 
 * You should have received a copy of the IUPAC/InChI Trust InChI 
 * Licence No. 1.0 with this library; if not, please write to:
 * 
 * The InChI Trust
 * c/o FIZ CHEMIE Berlin
 *
 * Franklinstrasse 11
 * 10587 Berlin
 * GERMANY
 *
 * or email to: ulrich@inchi-trust.org.
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

#include "mode.h"

#include "ichitime.h"
#include "inpdef.h"
#include "ichi.h"
#include "strutil.h"
#include "util.h"
#include "ichidrp.h"
#include "ichierr.h"
#include "ichimain.h"
#include "ichicomp.h"
#include "ichi_io.h"
#ifdef TARGET_EXE_STANDALONE
#include "inchi_api.h"
#endif
#if ( ADD_CMLPP == 1 )
#include "readcml.hpp"
#endif



/*  console-specific */

#ifdef COMPILE_ANSI_ONLY
/* Force strict ANSI C */

/*****************************************************************************/
int user_quit( const char *msg, unsigned long ulMaxTime )
{
    return 0;
}
/*****************************************************************************/
void eat_keyboard_input( void )
{
}
#endif

#ifndef COMPILE_ANSI_ONLY

/* Use Windows additional features */


/*****************************************************************************/
int user_quit( const char *msg, unsigned long ulMaxTime )
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
        InchiTimeAddMsec( &ulEndTime, ulMaxTime );
        while ( !_kbhit() ) {
            if ( bInchiTimeIsOver( &ulEndTime ) ) 
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
#endif	/* #if ( defined(_WIN32) && !defined(TARGET_LIB_FOR_WINCHI) ) */
}


/*****************************************************************************/
void eat_keyboard_input( void )
{
#ifndef TARGET_LIB_FOR_WINCHI
    while ( _kbhit() ) {
        if ( 0xE0 == _getch() )
            _getch();
    }
#endif
}

#endif /* end of !COMPILE_ANSI_ONLY */





#ifndef TARGET_LIB_FOR_WINCHI 
                    /* COVERS THE CODE FROM HERE TO THE END OF FILE */



/* Enable/disable internal tests */
/*#define TEST_FPTRS*/	/* uncomment for INCHI_LIB testing only */
#define REPEAT_ALL  0	/* set to 1 for mapping tests */


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
/*****************************************************************************/
int main( int argc, char *argv[ ] )
{
/*************************/
#if ( BUILD_WITH_AMI == 1 )
/*************************/
                                /* if in AMI mode, main() starts here and   */
                                /* captures command line to pass to actual  */
                                /* worker - process_single_input()          */
                                /* [ which previously was known as main() ] */
int i, p, ret=0, ami=0, AMIOutStd=0, AMILogStd=0, AMIPrbNone=0, nfn_ins=0;
char *fn_out, *fn_log, *fn_prb;
char **fn_ins=NULL, **targv=NULL;
char pNUL[] = "NUL";

    /* Check if multiple inputs expected */
    for (i=1; i < argc; i++)
    {
        if (argv[i][0] == INCHI_OPTION_PREFX)
        {
            if ( !stricmp(argv[i]+1, "AMI") )
            {
                ami = 1;
                break;
            }
        }
    }


    /**********************/
    /* Single input file. */
    /**********************/
    if ( !ami )
    {
        ret = process_single_input(argc,argv);
        goto exit_ami_main;
    }
    

    /**********************************/
    /* Multiple input files expected. */
    /**********************************/

    fn_ins = (char**) calloc( argc, sizeof(char *) );
    if ( !fn_ins )
    {
        fprintf(stderr, "Not enough memory.\n");
        goto exit_ami_main;
    }

    /* Check for other options and collect inputs. */
    for (i=1; i < argc; i++)
    {
        if (argv[i][0] == INCHI_OPTION_PREFX)
        {
            if ( !stricmp(argv[i]+1, "STDIO") )
            {
                fprintf( stderr, "Options AMI and STDIO are not compatible.\n" );
                goto exit_ami_main;
            }
            else if ( !stricmp(argv[i]+1, "AMIOutStd") )
            {
                AMIOutStd = 1;
            }
            else if ( !stricmp(argv[i]+1, "AMILogStd") )
            {
                AMILogStd = 1;
            }
            else if ( !stricmp(argv[i]+1, "AMIPrbNone") )
            {
                AMIPrbNone = 1;
            }
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
        goto exit_ami_main;
    }

	targv = (char**) calloc( argc+3, sizeof(char *) );
    if ( !targv )
    {
        fprintf(stderr, "Not enough memory.\n");
        goto exit_ami_main;
    }

    for ( p=0; p < nfn_ins; p++ )
    {
        int targc;       
        const char *fn_in = fn_ins[p];
        int inlen = strlen(fn_in);
        fn_out=fn_log=fn_prb=NULL;

        targv[0] = argv[0];
        targv[1] = (char *) fn_in;
        targc = 1;

        if ( AMIOutStd )
        {
            targv[++targc] = pNUL;
        }
        else
        {
            /* make output name as input name plus ext. */
            fn_out = (char*) calloc( inlen+6, sizeof(char ) );
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
        else
        {
            /* make log name as input name plus ext. */
            fn_log = (char*) calloc( inlen+6, sizeof(char ) );
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
        else
        {
            /* make problem file name as input file name plus ext. */
            fn_prb = (char*) calloc( inlen+6, sizeof(char ) );
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

        
        ret = process_single_input(targc,targv); /* process_single_input() is a former main() */

        if ( fn_out ) 
            free( fn_out );
        if ( fn_log ) 
            free( fn_log );
        if ( fn_prb ) 
            free( fn_prb );


#if ( defined( _WIN32 ) && defined( _CONSOLE ) && !defined( COMPILE_ANSI_ONLY ) )
    if ( ret == CTRL_STOP_EVENT)
        goto exit_ami_main;
#endif

    }
    
exit_ami_main:
    if ( targv )  
		free( targv );
    if ( fn_ins ) 
		free( fn_ins );
    return 0;
}




/**************************************************/
int process_single_input( int argc, char *argv[ ] )
/**************************************************/
{

/**************************************/
#endif /* #if ( BUILD_WITH_AMI == 1 ) */
/**************************************/
                                /* if not in AMI mode, main() starts here */
                                
    
    int bReleaseVersion = bRELEASE_VERSION;
    const int nStrLen = INCHI_SEGM_BUFLEN;
    int   nRet = 0, nRet1;
    int	i, k;
    long num_err, num_output, num_inp;
    /* long rcPict[4] = {0,0,0,0}; */
    unsigned long  ulDisplTime = 0;    /*  infinite, milliseconds */
    unsigned long  ulTotalProcessingTime = 0;

    char szTitle[MAX_SDF_HEADER+MAX_SDF_VALUE+256];
    char szSdfDataValue[MAX_SDF_VALUE+1];
    char *pStr = NULL;

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

    INCHI_IOSTREAM outputstr, logstr, prbstr, instr;
    INCHI_IOSTREAM *pout=&outputstr, *plog = &logstr, *pprb = &prbstr, *inp_file = &instr;
#ifdef TARGET_EXE_STANDALONE
    char ik_string[256];    /*^^^ Resulting InChIKey string */
    int ik_ret=0;           /*^^^ InChIKey-calc result code */
    int xhash1, xhash2;
    char szXtra1[65], szXtra2[65];
    int inchi_ios_type = INCHI_IOSTREAM_STRING;
#else
    int inchi_ios_type = INCHI_IOSTREAM_FILE;
#endif



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
    
    inchi_ios_init(inp_file, INCHI_IOSTREAM_FILE, NULL);
    inchi_ios_init(pout, inchi_ios_type, NULL);
    inchi_ios_init(plog, inchi_ios_type, stdout);
    inchi_ios_init(pprb, inchi_ios_type, NULL);

    
    if ( argc == 1 || argc==2 && ( argv[1][0]==INCHI_OPTION_PREFX ) &&
        (!strcmp(argv[1]+1, "?") || !stricmp(argv[1]+1, "help") ) ) 
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
    

    plog->f = stderr;
    if ( 0 > ReadCommandLineParms( argc, (const char **)argv, ip, szSdfDataValue, &ulDisplTime, bReleaseVersion, plog) ) 
                                         /* explicitly cast to (const char **) to avoid a warning about "incompatible pointer type":*/    
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
    else if ( ip->nInputType == INPUT_INCHI_XML || ip->nInputType == INPUT_INCHI_PLAIN  ||
         ip->nInputType == INPUT_CMLFILE   || ip->nInputType == INPUT_INCHI  ) 
    {
        /* the input may contain both the header and the label of the structure */
        if ( !ip->pSdfLabel ) 
            ip->pSdfLabel  = ip->szSdfDataHeader;
        if ( !ip->pSdfValue )
            ip->pSdfValue  = szSdfDataValue;
    }


    inchi_ios_eprint( plog, "The command line used:\n\"");
    for(k=0; k<argc-1; k++)
        inchi_ios_eprint( plog, "%-s ",argv[k]);
    inchi_ios_eprint( plog, "%-s\"\n", argv[argc-1]);


    PrintInputParms(plog,ip);
    inchi_ios_flush2(plog, stderr);


    if ( !( pStr = (char*) inchi_malloc(nStrLen) ) ) 
    {
        inchi_ios_eprint( plog, "Cannot allocate output buffer. Terminating\n"); 
        inchi_ios_flush2(plog, stderr);
        goto exit_function;
    }
    pStr[0] = '\0';


#if ( READ_INCHI_STRING == 1 )
    if ( ip->nInputType == INPUT_INCHI ) {
        memset( sd, 0, sizeof(*sd) );
        ReadWriteInChI( inp_file, pout, plog, ip,  sd, NULL, NULL, NULL, 0, NULL);
        inchi_ios_flush2(plog, stderr);
        ulTotalProcessingTime = sd->ulStructTime;
        num_inp               = sd->fPtrStart;
        num_err               = sd->fPtrEnd;
        goto exit_function;
    }
#endif


    ulTotalProcessingTime = 0;
    if ( pStructPtrs ) 
    {
        memset ( pStructPtrs, 0, sizeof(pStructPtrs[0]) );
        /* debug: set CML reading sequence
        pStructPtrs->fptr = (INCHI_FPTR *)inchi_calloc(16, sizeof(INCHI_FPTR));
        for ( i = 0; i < 15; i ++ )
            pStructPtrs->fptr[i] = 15-i;
        pStructPtrs->cur_fptr = 7;
        pStructPtrs->len_fptr = 16;
        pStructPtrs->max_fptr = 14;
        */
    }


    /**********************************************************************************************/
    /*  Main cycle : read input structures and create their INChI								  */
    /**********************************************************************************************/
    while ( !sd->bUserQuit && !bInterrupted ) 
    {
    
        if ( ip->last_struct_number && num_inp >= ip->last_struct_number ) 
        {
            nRet = _IS_EOF; /*  simulate end of file */
            goto exit_function;
        }

        /*  read one structure from input and display optionally it */

        nRet = GetOneStructure( sd, ip, szTitle, inp_file, plog, pout, pprb,
                                orig_inp_data, &num_inp, pStr, nStrLen, pStructPtrs );
        inchi_ios_flush2(plog, stderr);


        if ( pStructPtrs ) 
            pStructPtrs->cur_fptr ++;

        if ( sd->bUserQuit ) 
            break;

        switch ( nRet ) 
        {
            case _IS_FATAL:
                num_err ++;
            case _IS_EOF:
                goto exit_function;
            case _IS_ERROR:
                num_err ++;
            case _IS_SKIP:
                continue;
        }

        /* create INChI for each connected component of the structure and optionally display them */
        /* output INChI for the whole structure */
        
        nRet1 = ProcessOneStructure( sd, ip, szTitle, pINChI, pINChI_Aux,
                                     inp_file, plog, pout, pprb,
                                     orig_inp_data, prep_inp_data,
                                     num_inp, pStr, nStrLen,
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

        nRet = inchi_max(nRet, nRet1);
        switch ( nRet ) 
        {
            case _IS_FATAL:
                num_err ++;
                goto exit_function;
            case _IS_ERROR:
                num_err ++;
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
                ik_ret = GetINCHIKeyFromINCHI(buf, xhash1, xhash2, 
                                              ik_string, szXtra1, szXtra2); 
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
    if ( (ip->bINChIOutputOptions & INCHI_OUT_XML) && sd->bXmlStructStarted > 0 ) 
    {
            if ( !OutputINChIXmlStructEndTag( pout,  pStr, nStrLen, 1 ) ) 
            {
                inchi_ios_eprint( plog, "Cannot create end xml tag for structure #%ld.%s%s%s%s Terminating.\n", num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue) );
                inchi_ios_flush2(plog, stderr);
                sd->bXmlStructStarted = -1; /*  do not repeat same message */
        }
    }
    if ( (ip->bINChIOutputOptions & INCHI_OUT_XML) && ip->bXmlStarted ) 
    {
        OutputINChIXmlRootEndTag( pout ); 
        inchi_ios_flush(pout);
        ip->bXmlStarted = 0;
    }

    /* avoid memory leaks in case of fatal error */
    if ( pStructPtrs && pStructPtrs->fptr ) {
        inchi_free( pStructPtrs->fptr );
    }

    /*  free INChI memory */
    FreeAllINChIArrays( pINChI, pINChI_Aux, sd->num_components );
    /* free structure data */
    FreeOrigAtData( orig_inp_data );
    FreeOrigAtData( prep_inp_data );
    FreeOrigAtData( prep_inp_data+1 );
#if ( ADD_CMLPP == 1 )
    /* BILLY 8/6/04 */
    /* free CML memory */
   FreeCml ();
   FreeCmlDoc( 1 );
#endif

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
    inchi_ios_close(plog);
    if ( pStr )                             
        inchi_free( pStr );


    /* frees */
    for ( i = 0; i < MAX_NUM_PATHS; i ++ ) 
    {
        if ( ip->path[i] ) 
        {
            free( (void*) ip->path[i] ); /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }
    SetBitFree( );

/* internal tests --- */
#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    if ( num_repeat-- > 0 ) 
        goto repeat;
#endif
/* --- internal tests */

#if ( bRELEASE_VERSION != 1 && defined(_DEBUG) )
    if ( inp_file->f && inp_file->f != stdin ) 
    {
        user_quit("Press Enter to exit ?", ulDisplTime);
    }
#endif

#if ( ( BUILD_WITH_AMI==1 ) && defined( _WIN32 ) && defined( _CONSOLE ) && !defined( COMPILE_ANSI_ONLY ) )
    if ( bInterrupted ) 
        return CTRL_STOP_EVENT;
#endif
	return 0;
}



/*****************************************************************/
#endif  /* ifndef TARGET_LIB_FOR_WINCHI */
/*****************************************************************/
