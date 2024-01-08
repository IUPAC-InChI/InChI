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


#if( defined( WIN32 ) && defined( _CONSOLE ) && defined(_MSC_VER) && _MSC_VER >= 800 )
#define ADD_WIN_CTLC   /* detect Ctrl-C under Win-32 and Microsoft Visual C ++ */
#endif

#if( defined( WIN32 ) && defined( _CONSOLE ) && defined(_MSC_VER) && _MSC_VER >= 800 && defined(ADD_WIN_CTLC) && !(defined(__STDC__) && __STDC__ == 1) )
#include <windows.h>
#endif


/* #define CREATE_0D_PARITIES */      /* uncomment to replace coordinates and 2D-parirties with 0D-parities */
/* in case of CREATE_0D_PARITIES, the hardcoded bFixSp3Bug = 1 fixes sp3 bugs in original InChI v. 1.00  */
/* in case of CREATE_0D_PARITIES, the Phosphine and Arsine sp3 stereo options are not supported */

/* #define MAKE_INCHI_FROM_AUXINFO */  /* uncomment to create the second InChI out of AuxInfo and compare */
#define INCHI_TO_STRUCTURE            /* uncomment to convert Struct->InChI->Struct->InChI in case of /Inchi2Struct option */
/* #define NEIGH_ONLY_ONCE */              /* comment out to include each bond in both neighboring atoms adjacency lists */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

#include "e_mode.h"

/* uncomment to test /InChI2InChI option */
#define INCHI_TO_INCHI                


#ifndef CREATE_INCHI_STEP_BY_STEP

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Use old (classic) library interface (see main() below)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#include "inchi_api.h"
#include "e_ichi_io.h"

#include "e_inchi_atom.h"
#include "e_ctl_data.h"
#include "e_ichi_parms.h"
#include "e_util.h"
#include "e_ichierr.h"
#include "e_readstru.h"
#include "e_ichicomp.h"
#ifdef CREATE_0D_PARITIES
#include "e_0dstereo.h"
#endif



int e_MakeOutputHeader( char *pSdfLabel, char *pSdfValue, long lSdfId, long num_inp, char *pStr1, char *pStr2 );
char *e_GetChiralFlagString( int bChiralFlagOn );
static int e_bEnableCmdlineOption( char *szCmdLine, const char *szOption, int bEnable );

/*  console-specific */
/*****************************************************************
 *
 *  Ctrl+C trap; works under Win32 + MS VC++
 *
 *****************************************************************/
#ifndef BUILD_LIB_FOR_WINCHI

int e_bInterrupted = 0;
#if( defined( _WIN32 ) && defined( _CONSOLE ) )

#if( defined(_MSC_VER) && _MSC_VER >= 800 && defined(ADD_WIN_CTLC) && !(defined(__STDC__) && __STDC__ == 1) )
BOOL WINAPI MyHandlerRoutine(
  DWORD dwCtrlType   /*   control signal type */
  ) {
    if ( dwCtrlType == CTRL_C_EVENT     ||
         dwCtrlType == CTRL_BREAK_EVENT ||
         dwCtrlType == CTRL_CLOSE_EVENT ||
         dwCtrlType == CTRL_LOGOFF_EVENT ) {
        e_bInterrupted = 1;
        return TRUE;
    }
    return FALSE;
}
#endif
int e_WasInterrupted(void) {
#ifdef _DEBUG            
    if ( e_bInterrupted ) {
        int stop=1;  /*  for debug only <BRKPT> */
    }
#endif
    return e_bInterrupted;
}

#endif

#define EXIT_ON_ERR
#define REPEAT_ALL  0
/************** case insensitive strstr() ****************************/
static char *e_stristr ( const char * string1, const char * string2 )
{
    const char *str_target = string1;
    const char *cur_string1_ptr, *cur_string2_ptr;
    if ( !*string2 ) {
        return (char *)string1;
    }
    while ( *str_target ) {
        for ( cur_string1_ptr = str_target, cur_string2_ptr = string2;
                  *cur_string1_ptr && *cur_string2_ptr &&
                   toupper(UCINT *cur_string1_ptr) == toupper(UCINT *cur_string2_ptr);
                        cur_string1_ptr++, cur_string2_ptr++ )
            ;
        if ( !*cur_string2_ptr ) {
            return (char *)str_target;
        }
        str_target++;
    }
    return NULL;
}
/********************************************************************/
static int e_bEnableCmdlineOption( char *szCmdLine, const char *szOption, int bEnable )
{
    int   len  = strlen(szOption)+1, num = 0;
    char *pOpt = (char *)e_inchi_malloc(len+1);
    char *p = szCmdLine, *q, *r;
    if ( !pOpt )
        return -1;
    strcpy( pOpt+1, szOption );
    pOpt[0] = INCHI_OPTION_PREFX;
    while ( q = e_stristr(p, pOpt) ) {
        r = q + len;
        if ( bEnable > 0 && r[0] == '$' ) {
            memmove( r, r+1, strlen(r+1)+1 );
            num ++;
        } else
        if ( 0 == bEnable && (!r[0] || r[0] == ' ' || r[0] == '\t') ||
            -1 == bEnable && (!r[0] || r[0] == ' ' || r[0] == '\t' || r[0] == ':' ) ) {
            memmove( r+1, r, strlen(r)+1 );
            r[0] = '$';
            num ++;
        }
        p = q+1;
    }
    e_inchi_free( pOpt );
    return num;
}
/********************************************************************/
int main( int argc, char *argv[ ] )
{

    STRUCT_DATA struct_data;
    STRUCT_DATA *sd = &struct_data;

    long num_inp, num_err, num_output;
    char        szSdfDataValue[MAX_SDF_VALUE+1];
    const char *p1, *p2;

    unsigned long  ulDisplTime = 0;    /*  infinite, milliseconds */
    unsigned long  ulTotalProcessingTime = 0;
    INPUT_PARMS inp_parms;
    INPUT_PARMS *ip = &inp_parms;
    char        szInchiCmdLine[512];

    inchi_Input  inchi_inp, *pInp = &inchi_inp;
    inchi_Output inchi_out, *pOut = &inchi_out;


    int             bReleaseVersion = bRELEASE_VERSION;
#define nStrLen 256
    char  pStrInchiId[nStrLen], pStrLogId[nStrLen];
    int   nRet = 0, nRet1, i, k, tot_len;
    int   inp_index, out_index;
    long  lSdfId;
    int   nStructNo;
#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
    INPUT_TYPE  nActualInputType = INPUT_NONE;
    int   bActualReadInChIOptions=0;
#endif

#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    int  num_repeat = REPEAT_ALL;
#endif

/*^^^ New: using InChIKey API */
char ik_string[256];    /*^^^ Resulting InChIkey string */
int ik_ret=0;           /*^^^ InChIKey calculation result code */
int xhash1, xhash2;
char szXtra1[256], szXtra2[256];
/*^^^ */

/*^^^ Post-1.02b - moved here from below */
int bTabbed=0; 

/*^^^ Post-1.02b */
INCHI_IOSTREAM outputstr, logstr, prbstr, instr;
INCHI_IOSTREAM *out_stream=&outputstr, *log_stream=&logstr, *prb_stream=&prbstr, *inp_stream=&instr;



#if( TRACE_MEMORY_LEAKS == 1 )
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
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
    _CrtSetReportMode(_CRT_WARN | _CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
   /* turn on floating point exceptions */
#if ( !defined(__STDC__) || __STDC__ != 1 )
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
#endif

/*****************************************************************
 *
 *  Set Ctrl+C trap; works under Win32 + MS VC++
 *
 *****************************************************************/
#if( defined( _WIN32 ) && defined( _CONSOLE ) && defined(_MSC_VER) && _MSC_VER >= 800 && defined(ADD_WIN_CTLC) && !(defined(__STDC__) && __STDC__ == 1))
    if ( SetConsoleCtrlHandler( MyHandlerRoutine, 1 ) ) {
        ; /*ConsoleQuit = WasInterrupted*/;
    }
#endif



#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
repeat:

    inchi_ios_close(inp_stream);
    inchi_ios_close(log_stream);
    inchi_ios_close(out_stream);
    inchi_ios_close(prb_stream);


#endif

    /*^^^ Initialize I/O streams as files */
    inchi_ios_init(log_stream, INCHI_IOSTREAM_FILE, stderr);
    inchi_ios_init(inp_stream, INCHI_IOSTREAM_FILE, NULL);
    inchi_ios_init(out_stream, INCHI_IOSTREAM_FILE, NULL);
    inchi_ios_init(prb_stream, INCHI_IOSTREAM_FILE, NULL);

    num_inp    = 0;
    num_err    = 0;
    num_output = 0;

    if ( argc < 2 || argc==2 && ( argv[1][0]==INCHI_OPTION_PREFX ) &&
        (!strcmp(argv[1]+1, "?") || !stricmp(argv[1]+1, "help") ) ) {
        e_HelpCommandLineParms(log_stream);
        return 0;
    }
    /*  original input structure */
    memset( pInp, 0, sizeof(*pInp) );
    memset( pOut, 0, sizeof(*pOut) );

    memset( szSdfDataValue    , 0, sizeof( szSdfDataValue    ) );
    
    /* explicitly cast to (const char **) to avoid a warning about "incompatible pointer type":*/
    if ( 0 > e_ReadCommandLineParms( argc, (const char **)argv, ip, szSdfDataValue, &ulDisplTime, bReleaseVersion, log_stream) ) {
        goto exit_function;
    }
    if ( !e_OpenFiles( &(inp_stream->f), &(out_stream->f), &(log_stream->f), &(prb_stream->f), ip ) ) {
        goto exit_function;
    }
    if ( ip->bNoStructLabels ) {
        ip->pSdfLabel = NULL;
        ip->pSdfValue = NULL;
    } else
    if ( ip->nInputType == INPUT_INCHI_PLAIN  ||
         ip->nInputType == INPUT_INCHI_XML    ||
         ip->nInputType == INPUT_CMLFILE      ) {
        /* the input may contain both the header and the label of the structure */
        if ( !ip->pSdfLabel ) 
            ip->pSdfLabel  = ip->szSdfDataHeader;
        if ( !ip->pSdfValue )
            ip->pSdfValue  = szSdfDataValue;
    }
    e_PrintInputParms( log_stream, ip );
    pStrInchiId[0] = '\0';
    /******************************************************************/
    /*  Main cycle */
    /*  read input structures and create their InChI */
    ulTotalProcessingTime = 0;

    out_index = 0;
    /****************************** main cycle *************************/
    while ( !e_bInterrupted ) {
        int bHasTimeout = 0;
        if ( ip->last_struct_number && num_inp >= ip->last_struct_number ) {
            nRet = _IS_EOF; /*  simulate end of file */
            goto exit_function;
        }
        /* create command line */
        szInchiCmdLine[0] = '\0';
        for ( i = 1; i < argc; i ++ ) {
            if ( argv[i] && INCHI_OPTION_PREFX == argv[i][0] && argv[i][1] ) {
                /* omit certain options */
                if ( !memicmp( argv[i]+1, "start:", 6) ||
                     !memicmp( argv[i]+1, "end:",   4) ||
                     !stricmp( argv[i]+1, "Tabbed" )
                     ) {
                    continue;
                }
                if ( !stricmp( argv[i]+1, "Inchi2Inchi" ) ) {
#ifdef INCHI_TO_INCHI
                    nActualInputType        = INPUT_INCHI;
                    bActualReadInChIOptions = READ_INCHI_OUTPUT_INCHI;
#endif
                    continue;
                }
                if ( !stricmp( argv[i]+1, "Inchi2Struct" ) ) {
#ifdef INCHI_TO_STRUCTURE
                    nActualInputType        = INPUT_INCHI;
                    bActualReadInChIOptions = READ_INCHI_TO_STRUCTURE;
#endif
                    continue;
                }
                if ( !memicmp( argv[i]+1, "w", 1 ) && isdigit( UCINT argv[i][2] ) ) {
                    bHasTimeout = 1;
                }
                /* add option to szInchiCmdLine */
                if ( strlen(szInchiCmdLine)+strlen(argv[i]) + 4 < sizeof(szInchiCmdLine) ) {
                    if ( szInchiCmdLine[0] )
                        strcat( szInchiCmdLine, " " );
                    k = ( !(k=strcspn( argv[i], " \t" )) || argv[i][k] ); /* k means enclose in "" */
                    if ( k )
                        strcat( szInchiCmdLine, "\"" );
                    strcat( szInchiCmdLine, argv[i] );
                    if ( k )
                        strcat( szInchiCmdLine, "\"" );
                } else {
                    inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", argv[i] );
                }
            }
        }
#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
            if ( nActualInputType == INPUT_INCHI ) {
                /* disable other input types */
                e_bEnableCmdlineOption( szInchiCmdLine, "sdf", -1 );
                e_bEnableCmdlineOption( szInchiCmdLine, "mol", 0 );
                e_bEnableCmdlineOption( szInchiCmdLine, "cml", 0 );
            }
#endif
        if ( !bHasTimeout ) {
            /* add default timeout option -W60: 60 seconds */
            char szW60[] = " ?W60";
            szW60[1] = INCHI_OPTION_PREFX;
            if ( strlen(szInchiCmdLine) + strlen( szW60 ) < sizeof(szInchiCmdLine) ) {
                strcat( szInchiCmdLine, szW60 );
            } else {
                inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", szW60 );
            }
        }
        /************************* skip input cycle **************************/
        while(!e_bInterrupted) {
            inp_index = out_index;
            /*  read one structure from input */
            e_FreeInchi_Input( pInp );
            if ( num_inp > ip->first_struct_number ) {
                inchi_fprintf( stderr, "\rStructure: %d.................\r", num_inp+1 );
            }
#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
            if ( nActualInputType == INPUT_INCHI ) {
                /* make e_ReadStructure() read the structures */
                ip->nInputType  = INPUT_SDFILE;
            }
#endif
            nRet = e_ReadStructure( sd, ip, inp_stream, log_stream, out_stream, prb_stream, pInp, num_inp+1,
                          /* for CML:*/ inp_index, &out_index );
#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
            if ( nActualInputType != INPUT_NONE ) {
                ip->nInputType  = nActualInputType;
            }
#endif
            if ( _IS_SKIP != nRet ) {
                lSdfId    = ( ip->bGetSdfileId )? ip->lSdfId : 0; /* if requested then CAS r.n. otherwise struct. number*/
                nStructNo = ( ip->lMolfileNumber > 0 )? ip->lMolfileNumber : num_inp+1;
                e_MakeOutputHeader( ip->pSdfLabel, ip->pSdfValue, lSdfId, nStructNo, pStrInchiId, pStrLogId );
                /*
                if ( sd->pStrErrStruct && sd->pStrErrStruct[0] ) {
                    p1 = "; ";
                    p2 = sd->pStrErrStruct;
                } else {
                    p1 = p2 = "";
                }
                */
            }
            /* e_ReadStructure() outputs the error/warning messages, so we do not need to re-output them here */
            switch ( nRet ) {
            case _IS_FATAL:
                num_inp ++;
                num_err ++;
                goto exit_function;
            case _IS_EOF:
                inchi_ios_eprint( log_stream, "\rStructure %d could not be read: Detected end of file. \r", num_inp+1 );
                goto exit_function;
            case _IS_ERROR:
                num_inp ++;
                num_err ++;
                continue;
            case _IS_SKIP:
                num_inp ++;
                continue;
            }
            break;
        }
        if ( e_bInterrupted ) {
            inchi_ios_eprint( log_stream, "\nStructure %d could not be read: User Quit.\n", num_inp+1 );
            num_err ++;
            goto exit_function;
        }
        if ( nRet != _IS_WARNING ) {
            inchi_fprintf( stderr, "\r%s   \r", pStrLogId );
        }
        /* chiral flag */
        /* *****************************************************************************
         * Chiral flags are set in: 
         * - RunICHI.c #1610 -- ReadTheStructure()     -- cInChI, wInChI
         * - e_IchiMain.c #273 -- main()               -- C example of calling InChI dll (here)
         * - inchi_dll.c  #1662 -- ExtractOneStructure -- InChI dll code 
         *******************************************************************************/   

        p1 = NULL;

#ifndef USE_STDINCHI_API
        if ( (ip->nMode & REQ_MODE_CHIR_FLG_STEREO) && (ip->nMode & REQ_MODE_STEREO) &&
             ( ip->bChiralFlag & (FLAG_SET_INP_AT_CHIRAL | FLAG_SET_INP_AT_NONCHIRAL) ) ) {
                ; /* cmd line has priority over the chiral flag in Molfile */
        } else
        if ( sd->bChiralFlag & FLAG_INP_AT_CHIRAL ) {
            p1 =  e_GetChiralFlagString( 1 );  /* input file has chiral flag */
        } else
        if ( (ip->nMode & REQ_MODE_CHIR_FLG_STEREO) && (ip->nMode & REQ_MODE_STEREO) ||
             (sd->bChiralFlag & FLAG_INP_AT_NONCHIRAL) ) { /* fix 04/05/2005 D.T.*/
            /* chiral flag requested (/SUCF) or input has non-chiral flag */
            p1 =  e_GetChiralFlagString( 0 );
        }
#endif
        if ( p1 ) {
            if ( strlen(szInchiCmdLine) + strlen( p1 ) < sizeof(szInchiCmdLine) ) {
                strcat( szInchiCmdLine, p1 );
            } else {
                inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", p1 );
            }
        }
        /* create INChI for each connected component of the structure and optionally display them */
        /* output INChI for the whole structure */
#ifndef USE_STDINCHI_API
        FreeINCHI ( pOut );
#else
        FreeStdINCHI ( pOut );
#endif
        pInp->szOptions = szInchiCmdLine;
#ifdef CREATE_0D_PARITIES
        if ( !pInp->stereo0D && !pInp->num_stereo0D ) {
            int bPointedEdgeStereo = (0 != (TG_FLAG_POINTED_EDGE_STEREO & ip->bTautFlags));
            set_0D_stereo_parities( pInp, bPointedEdgeStereo );
            Clear3D2Dstereo(pInp);
        }
#endif
#ifdef NEIGH_ONLY_ONCE
        e_RemoveRedundantNeighbors( pInp );
#endif
        /********************************************
         *
         *  CREATE  INCHI
         *
         ********************************************/

#ifndef USE_STDINCHI_API
        nRet1 = GetINCHI( pInp, pOut );
#else
        nRet1 = GetStdINCHI( pInp, pOut );
#endif

        ulTotalProcessingTime += sd->ulStructTime;
        nRet = nRet1;
#ifdef INCHI_TO_INCHI
        /* do not include -Inch2Inchi and =Inci2Struct options in command line options */
        if ( nActualInputType == INPUT_INCHI && (bActualReadInChIOptions & READ_INCHI_OUTPUT_INCHI) &&
             (nRet1 == inchi_Ret_OKAY || nRet1 == inchi_Ret_WARNING) ) {
            inchi_InputINCHI    inpInchi;
            inchi_Output        inchi_out2;
            int nRet3;
            inpInchi.szInChI = pOut->szInChI;
            inpInchi.szOptions = pInp->szOptions;
/*            e_bEnableCmdlineOption( szInchiCmdLine, "Inchi2Inchi", 1 );*/
            nRet3 = GetINCHIfromINCHI( &inpInchi, &inchi_out2 );
/*            e_bEnableCmdlineOption( szInchiCmdLine, "Inchi2Inchi", 0 );*/
            if (nRet3 == inchi_Ret_OKAY || nRet3 == inchi_Ret_WARNING) {
#ifndef USE_STDINCHI_API
                FreeINCHI ( pOut );
#else
                FreeStdINCHI ( pOut );
#endif
                *pOut = inchi_out2;
                nRet = nRet3;
            } else {
                /* same output in case of error */
#ifndef USE_STDINCHI_API
                FreeINCHI ( pOut );
#else
                FreeStdINCHI ( pOut );
#endif
                *pOut = inchi_out2;
                nRet = nRet3;
            }
        }
#endif
#ifdef INCHI_TO_STRUCTURE
        /* How to convert InChI back to structure */
        if (nActualInputType == INPUT_INCHI && (bActualReadInChIOptions & READ_INCHI_TO_STRUCTURE) &&
           (nRet1 == inchi_Ret_OKAY || nRet1 == inchi_Ret_WARNING) ) {
            
            inchi_InputINCHI    inpInchi;
            inchi_OutputStruct  outStructure;
            inchi_Input         inchi_inp2;
            inchi_Output        inchi_out2;
            int nRet3, nRet4;

            memset( &inpInchi, 0, sizeof(inpInchi) );
            memset( &outStructure, 0, sizeof(outStructure) );
            memset( &inchi_inp2,   0, sizeof(inchi_inp2) );
            memset( &inchi_out2,   0, sizeof(inchi_out2) );

            
            /* InChI -> Structure */
            inpInchi.szInChI   = pOut->szInChI; /* use the original InChI as input */
            inpInchi.szOptions = szInchiCmdLine;

/*            e_bEnableCmdlineOption( szInchiCmdLine, "Inchi2Struct", 1 );*/
            nRet3 = GetStructFromINCHI( &inpInchi, &outStructure );
/*            e_bEnableCmdlineOption( szInchiCmdLine, "Inchi2Struct", 0 );*/
            
            FreeINCHI( pOut ); /* we do not need the original InChI anymore */
            
            if ( nRet3 == inchi_Ret_OKAY || nRet3 == inchi_Ret_WARNING ) {
                /* convert structure back to InChI */
                inpInchi.szInChI = NULL; /* do not keep invalid pointers */
                /* prepare structure input: make input structure out of the reconstructed one */
                inchi_inp2.atom         = outStructure.atom;
                inchi_inp2.num_atoms    = outStructure.num_atoms;
                inchi_inp2.stereo0D     = outStructure.stereo0D;
                inchi_inp2.num_stereo0D = outStructure.num_stereo0D;
                inchi_inp2.szOptions    = szInchiCmdLine;
                /* create InChI out of the reconstructed structure */
                nRet4 = GetINCHI( &inchi_inp2, &inchi_out2 );
                if ( nRet4 == inchi_Ret_OKAY || nRet4 == inchi_Ret_WARNING ) {
                    /* InChI of the reconstructed structure has been created */
                    /* get rid of Struct->InChI message in inchi_out2 */
                    /* and replace it with warning message from GetStructFromINCHI() */ 
                    if ( inchi_out2.szMessage && inchi_out2.szMessage[0] ) {
                        inchi_out2.szMessage[0] = '\0';
                    }
                    if ( nRet3 == inchi_Ret_WARNING ) {
                        /* prepare InChI->Structure warning for displaying: */
                        /* exchange outStructure message pointer with that of inchi_out2 */
                        char *p = inchi_out2.szMessage;
                        inchi_out2.szMessage = outStructure.szMessage;
                        outStructure.szMessage = p;
                    }
                    /* copy to pOut to use same output source code  */
                    /* as for Structure->InChI conversion output */
                    *pOut = inchi_out2;
                    nRet1 = nRet4; /* InChI->Struct->InChI return value */
                    nRet  = nRet3; /* InChI->Struct return value */
                    memset( &inchi_out2, 0, sizeof( inchi_out2) ); /* do not keep duplicated pointers */
                } else {
                    /* the last step in Structure->InChI->Structure->InChI failed */
                    *pOut = inchi_out2; /* save the error output */
                    nRet = nRet4;       /* Reconstructed Struct->InChI error code */
                    memset( &inchi_out2, 0, sizeof( inchi_out2) ); /* do not keep duplicated pointers */
                }
                memset( &inchi_inp2,   0, sizeof(inchi_inp2) ); /* do not keep invalid pointers */
            } else {
                /* error: Could not create Structure */
                inchi_ios_eprint( log_stream, "GetStructFromINCHI failed (%s)(%-d) %s\n", outStructure.szMessage? outStructure.szMessage:"???", nRet3, pStrLogId );
            }
            FreeStructFromINCHI( &outStructure );
        }
#endif

#ifdef MAKE_INCHI_FROM_AUXINFO
        /**************************************************
         *
         *  CREATE one more INCHI FROM AuxInfo and compare
         *
         *  it to to the first INCHI
         *
         *  Note: This should double the elapsed CPU time
         *
         **************************************************/
        if ( nRet1 == inchi_Ret_OKAY || nRet1 == inchi_Ret_WARNING ) {
            char *szInchiAuxInfo = pOut->szAuxInfo, *p1 /* shadowing previous definition */;
            int   bDoNotAddH = ip->bDoNotAddH;
            int   nRet2;
            inchi_Input  inchi_inp2, *pInp2 = &inchi_inp2;
            inchi_Output inchi_out2, *pOut2 = &inchi_out2;
            InchiInpData idat;
            /* setup input for Get_inchi_Input_FromAuxInfo */
            idat.pInp = pInp2;
            pInp2->szOptions = NULL; /* not needed */
            
            /* Make InChI input out of AuxInfo */
            nRet2 = Get_inchi_Input_FromAuxInfo( szInchiAuxInfo, bDoNotAddH, &idat );
            
            if ( inchi_Ret_OKAY == nRet2 || inchi_Ret_WARNING == nRet2 ) {
                /* set chiral flag */
                p1 = NULL;
                if ( (ip->nMode & REQ_MODE_CHIR_FLG_STEREO) && (ip->nMode & REQ_MODE_STEREO) &&
                     ( ip->bChiralFlag & (FLAG_SET_INP_AT_CHIRAL | FLAG_SET_INP_AT_NONCHIRAL) ) ) {
                        ; /* cmd line has priority over the chiral flag in Molfile */
                } else
                if ( idat.bChiral & FLAG_INP_AT_CHIRAL ) { /* fix 04/05/2005 D.T.*/
                    p1 = e_GetChiralFlagString( 1 );  /* input file has chiral flag */
                } else
                if ( (ip->nMode & REQ_MODE_CHIR_FLG_STEREO) && (ip->nMode & REQ_MODE_STEREO) ||
                     (idat.bChiral & FLAG_INP_AT_NONCHIRAL) ) { /* fix 04/05/2005 D.T.*/
                    /* chiral flag requested (/SUCF) or input has non-chiral flag */
                    p1 = e_GetChiralFlagString( 0 );
                }
                if ( p1 ) {
                    if ( strlen(szInchiCmdLine) + strlen( p1 ) < sizeof(szInchiCmdLine) ) {
                        strcat( szInchiCmdLine, p1 );
                    } else {
                        inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", p1 );
                    }
                }

                /* Make InChI out of InChI input out of AuxInfo */
                pInp2->szOptions = pInp->szOptions;
                /* pInp2->szOptions =" /OutputSDF"; */
                memset( pOut2, 0, sizeof(*pOut2) );
                nRet2 = GetINCHI( pInp2, pOut2 );
                /* Compare the two InChI */
                if ( nRet2 == inchi_Ret_OKAY || nRet2 == inchi_Ret_WARNING ) {
                    if ( !pOut2->szInChI || strcmp( pOut->szInChI, pOut2->szInChI ) ) {
                        inchi_fprintf( stderr, "InChI from AuxInfo Is Different!\n" );
                    }
                } else {
                    inchi_fprintf( stderr, "InChI from AuxInfo could not be produced: \"%s\"\n", pOut2->szMessage );
                }
                FreeINCHI( pOut2 );
            }
            Free_inchi_Input( pInp2 );
        }
#endif

        /*****************************/
        /*     output err/warn       */
        /*****************************/
        if ( pOut->szMessage && pOut->szMessage[0] ) {
            p1 = "; ";
            p2 = pOut->szMessage;
        } else {
            p1 = p2 = "";
        }
        switch ( nRet ) {
        case inchi_Ret_UNKNOWN:
        case inchi_Ret_FATAL: /* fatal processing error -- typically, memory allocation error */
            num_inp ++;
            num_err ++;
#if( defined(EXIT_ON_ERR) && defined(REPEAT_ALL) && REPEAT_ALL > 0 )
            num_repeat = 0;
#endif
            inchi_ios_eprint( log_stream, "Fatal Error (No INChI%s%s) %s\n", p1, p2, pStrLogId );
            inchi_ios_eprint( log_stream, "Log start:---------------------\n%s\nLog end--------------------\n", pOut->szLog? pOut->szLog : "Log is missing" );
            goto exit_function;
        case inchi_Ret_EOF: /* typically, no input structure provided or help requested */
            /* output previous structure number and message */
            inchi_ios_eprint( log_stream, "End of file detected after structure %d\n", num_inp );
            goto exit_function;
        case inchi_Ret_ERROR:
            num_inp ++;
            num_err ++;
            inchi_ios_eprint( log_stream, "Error (No INChI%s%s) %s\n", p1, p2, pStrLogId );
#if( defined(EXIT_ON_ERR) && defined(REPEAT_ALL) && REPEAT_ALL > 0 )
            num_repeat = 0;
            goto exit_function;
#endif
            continue;
        case inchi_Ret_SKIP:
            num_inp ++;
            inchi_ios_eprint( log_stream, "Skipped %s\n", pStrLogId );
            goto exit_function;
        case inchi_Ret_OKAY:
            break;
        case inchi_Ret_WARNING:
            if ( p2 && p2[0] ) {
                inchi_ios_eprint( log_stream, "Warning (%s) %s\n", p2, pStrLogId );
            }
            break; /* ok */
        }

        num_inp ++;
        tot_len = 0;

        /*^^^ Post-1.02b - moved from below */
        bTabbed  = 0 != ( ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT );
        
        if ( pOut->szInChI && pOut->szInChI[0] ) {
            if (ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY ) {
                /*****************************/
                /*       output SDfile       */
                /*****************************/
                char *start;
                unsigned len;
                int bAddLabel = 0;
                /*******************************************************************************/
                /* output a SDfile. pOut->szInChI contains Molfile ending with "$$$$\n" line.  */
                /* Replace the 1st line with the structure number                              */
                /* Replace the last line with the SDfile header, label, and new "$$$$\n" line  */
                /*******************************************************************************/
                /* 1. remove the 1st line (later replace it with the actual structure number) */
                if ( start = strchr( pOut->szInChI, '\n' ) ) {
                    inchi_ios_print( out_stream, "Structure #%ld", nStructNo );
                } else {
                    start = pOut->szInChI;
                }
                /* 2. SDfile header and data: write zero to the 1st byte of
                 *    the last line "$$$$\n" to remove this line with purpose to relpace it */
                if ( ip->pSdfLabel && ip->pSdfLabel[0] && ip->pSdfValue && ip->pSdfValue[0] &&
                     (len = strlen(start)) && len > 5 && '$' == start[len-5] && '\n' == start[len-6] ) {
                    start[len-5] = '\0';
                    bAddLabel = 1;
                }
                /* 3. Output the whole Molfile */
                inchi_ios_print( out_stream, "%s", start );
                if ( bAddLabel ) {
                    inchi_ios_print( out_stream, ">  <%s>\n%s\n\n$$$$\n", ip->pSdfLabel, ip->pSdfValue );
                }

            } else {
                /*****************************/
                /*       output InChI        */
                /*****************************/

                int bAuxInfo = !( ip->bINChIOutputOptions & INCHI_OUT_ONLY_AUX_INFO ) &&
                               pOut->szAuxInfo && pOut->szAuxInfo[0];
                /*^^^ Post-1.02b - correctly treat tabbed output with InChIKey */
                int bAuxOrKey = bAuxInfo || ( ip->bCalcInChIHash != INCHIHASH_NONE );

                const char *pLF  = "\n";
                const char *pTAB = bTabbed? "\t" : pLF;
                if ( !ip->bNoStructLabels ) {
                    /* or print a previously created label string */
                    inchi_ios_print( out_stream, "%s%s", pStrInchiId, pTAB );
                }
                /* output INChI Identifier */
                /*^^^ Post-1.02b */
                /* inchi_ios_print( out_stream, "%s%s", pOut->szInChI, bAuxInfo? pTAB : pLF );*/
                inchi_ios_print( out_stream, "%s%s", pOut->szInChI, bAuxOrKey? pTAB : pLF );
                /* output INChI Aux Info */
                if ( bAuxInfo ) {
                    inchi_ios_print(out_stream, "%s%s",pOut->szAuxInfo, ip->bCalcInChIHash? pTAB : pLF); 
                }

                /*^^^ Calculate InChIKey */
                if ( ip->bCalcInChIHash != INCHIHASH_NONE ) 
                {                      
                    xhash1 = xhash2 = 0;
                    if ( ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1 ) ||
                         ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1_XTRA2 ) )
                        xhash1 = 1;
                    if ( ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA2 ) ||
                         ( ip->bCalcInChIHash == INCHIHASH_KEY_XTRA1_XTRA2 ) )
                        xhash2 = 1;                
                    
                    ik_ret = GetINCHIKeyFromINCHI(pOut->szInChI, xhash1, xhash2, 
                                                  ik_string, szXtra1, szXtra2);
                    if (ik_ret==INCHIKEY_OK)    
                    {
                        /* NB: correctly treat tabbed output with InChIKey & hash extensions */                
                        char csep = '\n';
                        if ( ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT ) 
                            csep = '\t';
                        inchi_ios_print(out_stream, "InChIKey=%-s",ik_string);
                        if ( xhash1 )
                            inchi_ios_print(out_stream, "%cXHash1=%-s",csep,szXtra1);
                        if ( xhash2 )
                            inchi_ios_print(out_stream, "%cXHash2=%-s",csep,szXtra2);
                        inchi_ios_print(out_stream, "\n");
                    }
                    else    
                    {
                        /*^^^ Post-1.02b - add LF if output is tabbed and key generation failed */
                        if (bTabbed)
                            inchi_ios_print(out_stream, "\n");

                        inchi_ios_eprint(log_stream, "Warning: could not compute InChIKey for #%-d ", 
                                     num_inp );
                        switch(ik_ret)
                        {
                        case INCHIKEY_UNKNOWN_ERROR :
                                inchi_ios_eprint(log_stream, "(invalid key length requested)\n");
                                break;
                        case INCHIKEY_EMPTY_INPUT:
                                inchi_ios_eprint(log_stream, "(got an empty string)\n");
                                break;
                        case INCHIKEY_INVALID_INCHI_PREFIX:
                        case INCHIKEY_INVALID_INCHI:
                        case INCHIKEY_INVALID_STD_INCHI:
                                inchi_ios_eprint(log_stream, "(got non-InChI string)\n");
                                break;
                        case INCHIKEY_NOT_ENOUGH_MEMORY:
                                inchi_ios_eprint(log_stream, "(not enough memory to treat the string)\n");
                                break;
                        default:inchi_ios_eprint(log_stream, "(internal program error)\n");
                                break;
                        }
                    }

                }


            }
        }
    }
    if ( e_bInterrupted ) {
        inchi_ios_eprint( log_stream, "\nStructure %d could not be processed: User Quit.\n", num_inp+1 );
        num_err ++;
        goto exit_function;
    }

exit_function:
    inchi_ios_eprint( log_stream, "\nProcessed %ld structure%s, %ld error%s.\n",
                 num_inp, (num_inp==1)?"":"s", num_err, (num_err==1)?"":"s" );


    e_FreeInchi_Input( pInp );
#ifndef USE_STDINCHI_API
    FreeINCHI ( pOut );
#else
    FreeStdINCHI ( pOut );
#endif

#if( ADD_CMLPP == 1 )
        /* BILLY 8/6/04 */
        /* free CML memory */
        FreeCml ();
        FreeCmlDoc( 1 );
#endif


    inchi_ios_close(inp_stream);
    inchi_ios_close(log_stream);
    inchi_ios_close(out_stream);
    inchi_ios_close(prb_stream);


    for ( i = 0; i < MAX_NUM_PATHS; i ++ ) {
        if ( ip->path[i] ) {
            e_inchi_free( (char*) ip->path[i] ); /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }

#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    if ( --num_repeat > 0 ) {
        goto repeat;
    }
#endif


    return 0;
}


#endif

/**********************************************************/
int e_MakeOutputHeader( char *pSdfLabel, char *pSdfValue, long lSdfId, long num_inp, char *pStr1, char *pStr2  )
{
    int tot_len1 = 0, tot_len2 = 0;
    pStr1[0] = '\0';
    if ( !(pSdfLabel && pSdfLabel[0]) && !(pSdfValue && pSdfValue[0]) ) {
        tot_len1 = sprintf( pStr1, "Structure: %ld", num_inp );
        tot_len2 = sprintf( pStr2, "structure #%ld", num_inp );
    } else {
        tot_len1 = sprintf( pStr1, "Structure: %ld.%s%s%s%s",
            num_inp,
            SDF_LBL_VAL(pSdfLabel, pSdfValue) );

        tot_len2 = sprintf( pStr2, "structure #%ld.%s%s%s%s",
            num_inp,
            SDF_LBL_VAL(pSdfLabel, pSdfValue) );
        if ( lSdfId ) {
            tot_len1 += sprintf( pStr1 + tot_len1, ":%ld", lSdfId );
            tot_len2 += sprintf( pStr2 + tot_len2, ":%ld", lSdfId );
        }
    }
    return tot_len1;
}
/************************************************************/
char *e_GetChiralFlagString( int bChiralFlagOn )
{
    static char szChiralFlag[64];
    szChiralFlag[0] = ' ';
    szChiralFlag[1] = INCHI_OPTION_PREFX;
    sprintf( szChiralFlag+2, "ChiralFlag%s", bChiralFlagOn? "On":"Off" );
    return szChiralFlag;
}


#endif /*^^^ CREATE_INCHI_STEP_BY_STEP */
