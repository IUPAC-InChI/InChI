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

#include "../../../../INCHI_BASE/src/inchi_api.h"
#include "../../../../INCHI_BASE/src/bcf_s.h"

#include "e_ichi_io.h"
#include "e_inchi_atom.h"
#include "e_ctl_data.h"
#include "e_util.h"
#include "e_ichierr.h"
#include "e_readstru.h"
#include "e_ichicomp.h"
#ifdef CREATE_0D_PARITIES
#include "e_0dstereo.h"
#endif

#if (RINCHI_TEST)
#include "jhj.h"
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
void dump_inchi_Input( inchi_Input *pinp, INCHI_IOSTREAM *pout );
int e_MakeOutputHeader( char *pSdfLabel, char *pSdfValue, long lSdfId, long num_inp, char *pStr1, char *pStr2, int pStr1_size, int pStr2_size );
static int e_bEnableCmdlineOption( char *szCmdLine, const char *szOption, int bEnable );


/*  Console-specific */
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


/****************************************************************************/
int e_WasInterrupted( void )
{
#ifdef _DEBUG
    if (e_bInterrupted)
    {
        int stop = 1;  /*  for debug only <BRKPT> */
    }
#endif
    return e_bInterrupted;
}

#endif

#define EXIT_ON_ERR
#define REPEAT_ALL  0


/****************************************************************************
 Case insensitive strstr()
****************************************************************************/
static char *e_stristr( const char * string1, const char * string2 )
{
    const char *str_target = string1;
    const char *cur_string1_ptr, *cur_string2_ptr;
    if (!*string2)
    {
        return (char *) string1;
    }
    while (*str_target)
    {
        for (cur_string1_ptr = str_target, cur_string2_ptr = string2;
                  *cur_string1_ptr && *cur_string2_ptr &&
                   toupper( UCINT *cur_string1_ptr ) == toupper( UCINT *cur_string2_ptr );
                        cur_string1_ptr++, cur_string2_ptr++)
        {
            ;
        }
        if (!*cur_string2_ptr)
        {
            return (char *) str_target;
        }
        str_target++;
    }

    return NULL;
}


/****************************************************************************/
static int e_bEnableCmdlineOption( char *szCmdLine, const char *szOption, int bEnable )
{
    int   len = strlen( szOption ) + 1, num = 0;
    char *pOpt = (char *) e_inchi_malloc( (long long)len + 1 ); /* djb-rwth: cast operator added */
    char *p = szCmdLine, *q, *r;
    if (!pOpt)
    {
        return -1;
    }
    strcpy( pOpt + 1, szOption );
    pOpt[0] = INCHI_OPTION_PREFX;
    while ((q = e_stristr( p, pOpt ))) /* djb-rwth: addressing LLVM warning */
    {
        r = q + len;
        if (bEnable > 0 && r[0] == '$')
        {
            memmove( r, r + 1, strlen( r + 1 ) + 1 );
            num++;
        }
        else
        {
            if ((0 == bEnable && ( !r[0] || r[0] == ' ' || r[0] == '\t' )) ||
                ( - 1 == bEnable && (!r[0] || r[0] == ' ' || r[0] == '\t' || r[0] == ':'))) /* djb-rwth: addressing LLVM warnings */
            {
                memmove( r + 1, r, strlen( r ) + 1 );
                r[0] = '$';
                num++;
            }
        }
        p = q + 1;
    }
    e_inchi_free( pOpt );

    return num;
}


/****************************************************************************/
int main( int argc, char *argv[] )
{
    STRUCT_DATA struct_data;
    STRUCT_DATA *sd = &struct_data;

    long num_inp, num_err; /* djb-rwth: removing redundant variables */
    char        szSdfDataValue[MAX_SDF_VALUE + 1];
    const char *p1, *p2;

    unsigned long  ulDisplTime = 0;    /*  infinite, milliseconds */
    /* djb-rwth: removing redundant variables */
    INPUT_PARMS inp_parms;
    INPUT_PARMS *ip = &inp_parms;
    char        szInchiCmdLine[512];

    inchi_InputEx inchi_inp, *pInp = &inchi_inp;

    inchi_Output inchi_out, *pOut = &inchi_out;


    int             bReleaseVersion = bRELEASE_VERSION;
#define nStrLen 256
    char  pStrInchiId[nStrLen], pStrLogId[nStrLen];
    int   nRet = 0, nRet1, i, k; /* djb-rwth: removing redundant variables */
    int   inp_index, out_index;
    long  lSdfId;
    int   nStructNo;
#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
    INPUT_TYPE  nActualInputType = INPUT_NONE;
    int   bActualReadInChIOptions = 0;
#endif

#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    int  num_repeat = REPEAT_ALL;
#endif

/*^^^ New: using InChIKey API */
    char ik_string[256];    /*^^^ Resulting InChIkey string */
    int ik_ret = 0;           /*^^^ InChIKey calculation result code */
    int xhash1, xhash2;
    char szXtra1[256], szXtra2[256];
    /*^^^ */

    /*^^^ Post-1.02b - moved here from below */
    int bTabbed = 0;

    /*^^^ Post-1.02b */
    INCHI_IOSTREAM outputstr, logstr, prbstr, instr;
    INCHI_IOSTREAM *out_stream = &outputstr, *log_stream = &logstr, *prb_stream = &prbstr, *inp_stream = &instr;

#if (RINCHI_TEST)
    printf("\nv.1.07 - JHJ BLOCK:\n");
    jhj_main();
    printf("\n");
#endif

#if( TRACE_MEMORY_LEAKS == 1 )
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
#endif

/* Set Ctrl+C trap; works under Win32 + MS VC++ */
#if( defined( _WIN32 ) && defined( _CONSOLE ) && defined(_MSC_VER) && _MSC_VER >= 800 && defined(ADD_WIN_CTLC) && !(defined(__STDC__) && __STDC__ == 1))
    if (SetConsoleCtrlHandler( MyHandlerRoutine, 1 ))
    {
        ; /*ConsoleQuit = WasInterrupted*/;
    }
#endif

#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    repeat:

    inchi_ios_close( inp_stream );
    inchi_ios_close( log_stream );
    inchi_ios_close( out_stream );
    inchi_ios_close( prb_stream );
#endif

    /*^^^ Initialize I/O streams as files */
    inchi_ios_init( log_stream, INCHI_IOSTREAM_TYPE_FILE, stderr );
    inchi_ios_init( inp_stream, INCHI_IOSTREAM_TYPE_FILE, NULL );
    inchi_ios_init( out_stream, INCHI_IOSTREAM_TYPE_FILE, NULL );
    inchi_ios_init( prb_stream, INCHI_IOSTREAM_TYPE_FILE, NULL );

    num_inp = 0;
    num_err = 0;
    /* djb-rwth: removing redundant code */

    if (argc < 2 || (argc == 2 && ( argv[1][0] == INCHI_OPTION_PREFX ) &&
        ( !strcmp( argv[1] + 1, "?" ) || !e_inchi_stricmp( argv[1] + 1, "help" ) ))) /* djb-rwth: addressing LLVM warning */
    {
        e_HelpCommandLineParms( log_stream );
        return 0;
    }

    /* djb-rwth: printing out InChI version */
    if (argc == 2 && ((argv[1][0] == INCHI_OPTION_PREFX)) && (!strcmp(argv[1] + 1, "v") || !strcmp(argv[1] + 1, "V")))
    {
        printf("%s\n", APP_DESCRIPTION);
        return 0;
    }

    /*  original input structure */
    memset( pInp, 0, sizeof( *pInp ) ); /* djb-rwth: memset_s C11/Annex K variant? */
    memset( pOut, 0, sizeof( *pOut ) ); /* djb-rwth: memset_s C11/Annex K variant? */

    memset( szSdfDataValue, 0, sizeof( szSdfDataValue ) ); /* djb-rwth: memset_s C11/Annex K variant? */

    /* explicitly cast to (const char **) to avoid a warning about "incompatible pointer type":*/
    if (0 > e_ReadCommandLineParms( argc, (const char **) argv, ip, szSdfDataValue, &ulDisplTime, bReleaseVersion, log_stream ))
    {
        goto exit_function;
    }
    if (!e_OpenFiles( &( inp_stream->f ), &( out_stream->f ), &( log_stream->f ), &( prb_stream->f ), ip ))
    {
        goto exit_function;
    }
    if (ip->bNoStructLabels)
    {
        ip->pSdfLabel = NULL;
        ip->pSdfValue = NULL;
    }
    else if (ip->nInputType == INPUT_INCHI_PLAIN ||
            ip->nInputType == INPUT_INCHI_XML    ||
            ip->nInputType == INPUT_CMLFILE       )
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
    pStrInchiId[0] = '\0';

    inchi_ios_flush( log_stream );

    /* Main loop */

    out_index = 0;
    /* djb-rwth: removing redundant code */

    while (!e_bInterrupted)
    {

        /* Read input structures and create their InChI */
        int bHasTimeout = 0;
        if (ip->last_struct_number && num_inp >= ip->last_struct_number)
        {
            nRet = _IS_EOF; /*  simulate end of file */ /* djb-rwth: ignoring LLVM warning: value used */
            goto exit_function;
        }

        /* Create command line */
        szInchiCmdLine[0] = '\0';
        for (i = 1; i < argc; i++)
        {
            if (argv[i] && INCHI_OPTION_PREFX == argv[i][0] && argv[i][1])
            {
                /* Omit certain options */
                if (!e_inchi_memicmp( argv[i] + 1, "start:", 6 ) ||
                    !e_inchi_memicmp( argv[i] + 1, "end:", 4 ) ||
                    !e_inchi_stricmp( argv[i] + 1, "Tabbed" )
                    )
                {
                    continue;
                }
                if (!e_inchi_stricmp( argv[i] + 1, "Inchi2Inchi" ))
                {
#ifdef INCHI_TO_INCHI
                    nActualInputType = INPUT_INCHI;
                    bActualReadInChIOptions = READ_INCHI_OUTPUT_INCHI;
#endif
                    continue;
                }
                if (!e_inchi_stricmp( argv[i] + 1, "Inchi2Struct" ))
                {
#ifdef INCHI_TO_STRUCTURE
                    nActualInputType = INPUT_INCHI;
                    bActualReadInChIOptions = READ_INCHI_TO_STRUCTURE;
#endif
                    continue;
                }
                if (!e_inchi_memicmp( argv[i] + 1, "w", 1 ) && isdigit( UCINT argv[i][2] ))
                {
                    bHasTimeout = 1;
                }
                /* Add option to szInchiCmdLine */
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
        } /* eof Create command line */

#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
        if (nActualInputType == INPUT_INCHI)
        {
            /* Disable other input types */
            e_bEnableCmdlineOption( szInchiCmdLine, "sdf", -1 );
            e_bEnableCmdlineOption( szInchiCmdLine, "mol", 0 );
            e_bEnableCmdlineOption( szInchiCmdLine, "cml", 0 );
        }
#endif

        if (!bHasTimeout)
        {
            /* Add default timeout 60 sec */
            char szW60[] = " ?W60";
            szW60[1] = INCHI_OPTION_PREFX;
            if (strlen( szInchiCmdLine ) + strlen( szW60 ) < sizeof( szInchiCmdLine ))
            {
                strcat( szInchiCmdLine, szW60 );
            }
            else
            {
                inchi_fprintf( stderr, "Too many options. Option \"%s\" ignored\n", szW60 );
            }
        }

        /* Skip input cycle */
        while (!e_bInterrupted)
        {
            inp_index = out_index;
            /*  Read one structure from input */
            e_FreeInchi_Input( pInp );
            if (num_inp > ip->first_struct_number)
            {
                inchi_fprintf( stderr, "\rStructure: %d.................\r", num_inp + 1 );
            }
#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
            if (nActualInputType == INPUT_INCHI)
            {
                /* Make e_ReadStructure() read the structures */
                ip->nInputType = INPUT_SDFILE;
            }
#endif
            nRet = e_ReadStructure( sd, ip,
                                    inp_stream, log_stream, out_stream, prb_stream,
                                    pInp, num_inp + 1,
                                    /* for CML:*/
                                    inp_index, &out_index );
#if ( defined(INCHI_TO_STRUCTURE) || defined(INCHI_TO_INCHI) )
            if (nActualInputType != INPUT_NONE)
            {
                ip->nInputType = nActualInputType;
            }
#endif
            if (_IS_SKIP != nRet)
            {
                lSdfId = ( ip->bGetSdfileId ) ? ip->lSdfId : 0; /* if requested then CAS r.n. otherwise struct. number*/
                nStructNo = ( ip->lMolfileNumber > 0 ) ? ip->lMolfileNumber : num_inp + 1;
                e_MakeOutputHeader( ip->pSdfLabel, ip->pSdfValue, lSdfId, nStructNo, pStrInchiId, pStrLogId, sizeof(pStrInchiId), sizeof(pStrLogId) );
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
            switch (nRet)
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
                    if (ip->bINChIOutputOptions & INCHI_OUT_STDINCHI)
                    {
                        inchi_ios_print( out_stream, "InChI=1S//\n" );
                    }
                    else
                    {
                        inchi_ios_print( out_stream, "InChI=1//\n" );
                    }
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
            num_err++;
            goto exit_function;
        }
        if (nRet != _IS_WARNING)
        {
            inchi_fprintf( stderr, "\r%s   \r", pStrLogId );
        }

        /* Chiral flag */
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
        else if ( (( ip->nMode & REQ_MODE_CHIR_FLG_STEREO ) && ( ip->nMode & REQ_MODE_STEREO )) ||
                    ( sd->bChiralFlag & FLAG_INP_AT_NONCHIRAL )) /* djb-rwth: addressing LLVM warning */
        {
            /* Fix 04/05/2005 D.T.*/
            /* Chiral flag requested (/SUCF) or input has non-chiral flag */
            p1 = e_GetChiralFlagString( 0 );
        }
#endif

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

        /* Create INChI for each connected component of the structure and optionally display them;  */
        /* output INChI for the whole structure                                                     */
#ifndef USE_STDINCHI_API
        FreeINCHI( pOut );
#else
        FreeStdINCHI( pOut );
#endif
        pInp->szOptions = szInchiCmdLine;
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

        /* Create  InChI */

        nRet1 = GetINCHIEx(pInp, pOut);

        /* djb-rwth: removing redundant code */
        nRet = nRet1;

#ifdef INCHI_TO_INCHI
        if (   nActualInputType == INPUT_INCHI                          &&
             ( bActualReadInChIOptions & READ_INCHI_OUTPUT_INCHI )      &&
             ( nRet1 == inchi_Ret_OKAY || nRet1 == inchi_Ret_WARNING )   )
        {
            inchi_InputINCHI    inpInchi;
            inchi_Output        inchi_out2;
            int nRet3;
            inpInchi.szInChI = pOut->szInChI;
            inpInchi.szOptions = pInp->szOptions;

            nRet3 = GetINCHIfromINCHI( &inpInchi, &inchi_out2 );

            FreeINCHI( pOut );
            *pOut = inchi_out2;
            nRet = nRet3;
        }
#endif

#ifdef INCHI_TO_STRUCTURE
        /* Convert InChI back to structure */
        if (nActualInputType == INPUT_INCHI &&
            ( bActualReadInChIOptions & READ_INCHI_TO_STRUCTURE ) &&
                ( nRet1 == inchi_Ret_OKAY || nRet1 == inchi_Ret_WARNING )
            )
        {

            inchi_InputINCHI    inpInchi;
            inchi_OutputStruct  outStructure;

            inchi_InputEx		inchi_inp2;
            
            inchi_Output        inchi_out2;
            int nRet3, nRet4;

            memset( &inpInchi, 0, sizeof( inpInchi ) ); /* djb-rwth: memset_s C11/Annex K variant? */
            memset( &outStructure, 0, sizeof( outStructure ) ); /* djb-rwth: memset_s C11/Annex K variant? */
            memset( &inchi_inp2, 0, sizeof( inchi_inp2 ) ); /* djb-rwth: memset_s C11/Annex K variant? */
            memset( &inchi_out2, 0, sizeof( inchi_out2 ) ); /* djb-rwth: memset_s C11/Annex K variant? */

            /* Use the original InChI as input */
            inpInchi.szInChI = pOut->szInChI;
            inpInchi.szOptions = szInchiCmdLine;


            nRet3 = GetStructFromINCHI( &inpInchi, &outStructure );


            FreeINCHI( pOut );
                    /* We do not need the original InChI anymore */

            if (nRet3 == inchi_Ret_OKAY || nRet3 == inchi_Ret_WARNING)
            {
                /* Convert structure back to InChI */
                inpInchi.szInChI = NULL;

                /* do not keep invalid pointers */

                /* Prepare structure input: make input structure out of the reconstructed one */
                inchi_inp2.atom = outStructure.atom;
                inchi_inp2.num_atoms = outStructure.num_atoms;
                inchi_inp2.stereo0D = outStructure.stereo0D;
                inchi_inp2.num_stereo0D = outStructure.num_stereo0D;
                inchi_inp2.szOptions = szInchiCmdLine;

                /* create InChI out of the reconstructed structure */

                nRet4 = GetINCHIEx(&inchi_inp2, &inchi_out2);

                if (nRet4 == inchi_Ret_OKAY || nRet4 == inchi_Ret_WARNING)
                {
                    /* InChI of the reconstructed structure has been created */
                    /* get rid of Struct->InChI message in inchi_out2 */
                    /* and replace it with warning message from GetStructFromINCHI() */
                    if (inchi_out2.szMessage && inchi_out2.szMessage[0])
                    {
                            inchi_out2.szMessage[0] = '\0';
                    }
                    if (nRet3 == inchi_Ret_WARNING)
                    {
                        /* Prepare InChI->Structure warning for displaying: */
                        /* exchange outStructure message pointer with that of inchi_out2 */
                        char *p = inchi_out2.szMessage;
                        inchi_out2.szMessage = outStructure.szMessage;
                        outStructure.szMessage = p;
                    }
                    /* Copy to pOut to use same output source code  */
                    /* as for Structure->InChI conversion output */
                    *pOut = inchi_out2;
                    nRet1 = nRet4; /* InChI->Struct->InChI return value */ /* djb-rwth: ignoring LLVM warning: value used */
                    nRet = nRet3; /* InChI->Struct return value */
                    memset( &inchi_out2, 0, sizeof( inchi_out2 ) ); /* do not keep duplicated pointers */ /* djb-rwth: memset_s C11/Annex K variant? */
                }
                else
                {
                    /* The last step in Structure->InChI->Structure->InChI failed */
                    *pOut = inchi_out2; /* save the error output */
                    nRet = nRet4;       /* Reconstructed Struct->InChI error code */
                    memset( &inchi_out2, 0, sizeof( inchi_out2 ) ); /* do not keep duplicated pointers */ /* djb-rwth: memset_s C11/Annex K variant? */
                }
                memset( &inchi_inp2, 0, sizeof( inchi_inp2 ) ); /* do not keep invalid pointers */ /* djb-rwth: memset_s C11/Annex K variant? */
            }
            else
            {
                /* Error: Could not create Structure */
                inchi_ios_eprint( log_stream, "GetStructFromINCHI failed (%s)(%-d) %s\n", outStructure.szMessage ? outStructure.szMessage : "???", nRet3, pStrLogId );
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
        if (nRet1 == inchi_Ret_OKAY || nRet1 == inchi_Ret_WARNING)
        {
            char *szInchiAuxInfo = pOut->szAuxInfo, *p1 /* shadowing previous definition */;
            int   bDoNotAddH = ip->bDoNotAddH;
            int   nRet2;
            inchi_InputEx inchi_inp2, *pInp2 = &inchi_inp2;           
            inchi_Output inchi_out2, *pOut2 = &inchi_out2;
            InchiInpData idat;
            /* setup input for Get_inchi_Input_FromAuxInfo */
            idat.pInp = pInp2;
            pInp2->szOptions = NULL; /* not needed */

            /* Make InChI input out of AuxInfo */
            nRet2 = Get_inchi_Input_FromAuxInfo( szInchiAuxInfo, bDoNotAddH, &idat );

            if (inchi_Ret_OKAY == nRet2 || inchi_Ret_WARNING == nRet2)
            {
                /* set chiral flag */
                p1 = NULL;
                if (( ip->nMode & REQ_MODE_CHIR_FLG_STEREO )    &&
                    ( ip->nMode & REQ_MODE_STEREO )             &&
                    ( ip->bChiralFlag & ( FLAG_SET_INP_AT_CHIRAL | FLAG_SET_INP_AT_NONCHIRAL ) )    )
                {
                    ; /* cmd line has priority over the chiral flag in Molfile */
                }
                else if (idat.bChiral & FLAG_INP_AT_CHIRAL)
                {
                    /* Fix 04/05/2005 D.T.*/
                    p1 = e_GetChiralFlagString( 1 );  /* input file has chiral flag */
                }
                else if (( ip->nMode & REQ_MODE_CHIR_FLG_STEREO ) && ( ip->nMode & REQ_MODE_STEREO ) ||
                            ( idat.bChiral & FLAG_INP_AT_NONCHIRAL ))
                {
                    /* fix 04/05/2005 D.T.*/
                    /* chiral flag requested (/SUCF) or input has non-chiral flag */
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

                /* Make InChI out of InChI input out of AuxInfo */
                pInp2->szOptions = pInp->szOptions;
                /* pInp2->szOptions =" /OutputSDF"; */
                memset( pOut2, 0, sizeof( *pOut2 ) );
                
                nRet2 = GetINCHIEx(pInp2, pOut2);

                /* Compare the two InChI */
                if (nRet2 == inchi_Ret_OKAY || nRet2 == inchi_Ret_WARNING)
                {
                    if (!pOut2->szInChI || strcmp( pOut->szInChI, pOut2->szInChI ))
                    {
                        inchi_fprintf( stderr, "InChI from AuxInfo Is Different!\n" );
                    }
                }
                else
                {
                    inchi_fprintf( stderr, "InChI from AuxInfo could not be produced: \"%s\"\n", pOut2->szMessage );
                }
                FreeINCHI( pOut2 );
            }
            Free_inchi_Input( pInp2 );
        }
#endif

        /* Output errors/warnings */
        if (pOut->szMessage && pOut->szMessage[0])
        {
            p1 = "; ";
            p2 = pOut->szMessage;
        }
        else
        {
            p1 = p2 = "";
        }
        switch (nRet)
        {
            case inchi_Ret_UNKNOWN:
            case inchi_Ret_FATAL:
                /* fatal processing error -- typically, memory allocation error */
                num_inp++;
                num_err++;
#if( defined(EXIT_ON_ERR) && defined(REPEAT_ALL) && REPEAT_ALL > 0 )
                num_repeat = 0;
#endif
                inchi_ios_eprint( log_stream, "Fatal Error (No INChI%s%s) %s\n", p1, p2, pStrLogId );
                inchi_ios_eprint( log_stream, "Log start:---------------------\n%s\nLog end--------------------\n", pOut->szLog ? pOut->szLog : "Log is missing" );
                goto exit_function;
            case inchi_Ret_EOF:
                /* Typically, no input structure provided or help requested */
                /* output previous structure number and message */
                inchi_ios_eprint( log_stream, "End of file detected after structure %d\n", num_inp );
                goto exit_function;
            case inchi_Ret_ERROR:
                num_inp++;
                num_err++;
                inchi_ios_eprint( log_stream, "Error (No INChI%s%s) %s\n", p1, p2, pStrLogId );
                if (ip->bINChIOutputOptions & INCHI_OUT_STDINCHI)
                {
                    inchi_ios_print( out_stream, "InChI=1S//\n" );
                }
                else
                {
                    inchi_ios_print( out_stream, "InChI=1//\n" );
                }
#if( defined(EXIT_ON_ERR) && defined(REPEAT_ALL) && REPEAT_ALL > 0 )
                num_repeat = 0;
                goto exit_function;
#endif
                continue;
            case inchi_Ret_SKIP:
                num_inp++;
                inchi_ios_eprint( log_stream, "Skipped %s\n", pStrLogId );
                goto exit_function;
            case inchi_Ret_OKAY:
                break;
            case inchi_Ret_WARNING:
                if (p2 && p2[0])
                {
                    inchi_ios_eprint( log_stream, "Warning (%s) %s\n", p2, pStrLogId );
                }
            break; /* OK */
        }

        num_inp++;
        /* djb-rwth: removing redundant code */

        /*^^^ Post-1.02b - moved from below */
        bTabbed = 0 != ( ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT );
        if (pOut->szInChI && pOut->szInChI[0])
        {

            if (ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY)
            {
                /* Output SDfile. pOut->szInChI contains Molfile ending with "$$$$\n" line.  */
                /* Replace the 1st line with the structure number                              */
                /* Replace the last line with the SDfile header, label, and new "$$$$\n" line  */

                char *start;
                unsigned len;
                int bAddLabel = 0;

                /* 1. Remove the 1st line (later replace it with the actual structure number) */
                if ((start = strchr( pOut->szInChI, '\n' ))) /* djb-rwth: addressing LLVM warning */
                {
                    inchi_ios_print( out_stream, "Structure #%ld", nStructNo );
                }
                else
                {
                    start = pOut->szInChI;
                }

                /* 2. SDfile header and data: write zero to the 1st byte of the last    */
                /*    line "$$$$\n" to remove this line with purpose to relpace it      */
                if (ip->pSdfLabel && ip->pSdfLabel[0] && ip->pSdfValue && ip->pSdfValue[0] &&
                    ( len = strlen( start ) ) && len > 5 && '$' == start[len - 5] && '\n' == start[len - 6])
                {
                    start[len - 5] = '\0';
                    bAddLabel = 1;
                }

                /* 3. Output the whole Molfile */
                inchi_ios_print( out_stream, "%s", start );
                if (bAddLabel)
                {
                    inchi_ios_print( out_stream, ">  <%s>\n%s\n\n$$$$\n", ip->pSdfLabel, ip->pSdfValue );
                }
            }
            else
            {
                /* Output InChI */
                int bAuxInfo =  !( ip->bINChIOutputOptions & INCHI_OUT_ONLY_AUX_INFO ) &&
                                pOut->szAuxInfo &&
                                pOut->szAuxInfo[0];
                /*^^^ Post-1.02b - correctly treat tabbed output with InChIKey */
                int bAuxOrKey = bAuxInfo || ( ip->bCalcInChIHash != INCHIHASH_NONE );
                const char *pLF = "\n";
                const char *pTAB = bTabbed ? "\t" : pLF;
                if (!ip->bNoStructLabels)
                {
                    /* or print a previously created label string */
                    inchi_ios_print( out_stream, "%s%s", pStrInchiId, pTAB );
                }

                /* Output INChI Identifier */
                /*^^^ Post-1.02b */
                /* inchi_ios_print( out_stream, "%s%s", pOut->szInChI, bAuxInfo? pTAB : pLF );*/
                inchi_ios_print( out_stream, "%s%s", pOut->szInChI, bAuxOrKey ? pTAB : pLF );
                /* Output INChI Aux Info */
                if (bAuxInfo)
                {
                    inchi_ios_print( out_stream, "%s%s", pOut->szAuxInfo, ip->bCalcInChIHash ? pTAB : pLF );
                }

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

                    ik_ret = GetINCHIKeyFromINCHI( pOut->szInChI, xhash1, xhash2,
                                                ik_string, szXtra1, szXtra2 );
                    if (ik_ret == INCHIKEY_OK)
                    {
                        /* NB: correctly treat tabbed output with InChIKey & hash extensions */
                        char csep = '\n';
                        if (ip->bINChIOutputOptions & INCHI_OUT_TABBED_OUTPUT)
                            csep = '\t';
                        inchi_ios_print( out_stream, "InChIKey=%-s", ik_string );
                        if (xhash1)
                            inchi_ios_print( out_stream, "%cXHash1=%-s", csep, szXtra1 );
                        if (xhash2)
                            inchi_ios_print( out_stream, "%cXHash2=%-s", csep, szXtra2 );
                        inchi_ios_print( out_stream, "\n" );
                    }
                    else
                    {
                        /*^^^ Post-1.02b - add LF if output is tabbed and key generation failed */
                        if (bTabbed)
                        {
                            inchi_ios_print( out_stream, "\n" );
                        }
                        inchi_ios_eprint( log_stream, "Warning: could not compute InChIKey for #%-d ", num_inp );
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
                }
            }

        } /* eof if (pOut->szInChI && pOut->szInChI[0]) */

    }   /* eof Main loop */

    if (e_bInterrupted)
    {
        inchi_ios_eprint( log_stream, "\nStructure %d could not be processed: User Quit.\n", num_inp + 1 );
        num_err++;
        goto exit_function;
    }

exit_function:
    inchi_ios_eprint( log_stream, "\nProcessed %ld structure%s, %ld error%s.\n",
                num_inp, ( num_inp == 1 ) ? "" : "s", num_err, ( num_err == 1 ) ? "" : "s" );
    e_FreeInchi_Input( pInp );
#ifndef USE_STDINCHI_API
    FreeINCHI( pOut );
#else
    FreeStdINCHI( pOut );
#endif
    inchi_ios_close( inp_stream );
    inchi_ios_close( log_stream );
    inchi_ios_close( out_stream );
    inchi_ios_close( prb_stream );
    for (i = 0; i < MAX_NUM_PATHS; i++)
    {
        if (ip->path[i])
        {
            e_inchi_free( (char*) ip->path[i] ); /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }
#if ( defined(REPEAT_ALL) && REPEAT_ALL > 0 )
    if (--num_repeat > 0)
    {
        goto repeat;
    }
#endif

    return 0;
}

#endif


/****************************************************************************/
int e_MakeOutputHeader(char* pSdfLabel,
    char* pSdfValue,
    long lSdfId,
    long num_inp,
    char* pStr1,
    char* pStr2,
    int pStr1_size,
    int pStr2_size)
{
    int tot_len1 = 0, tot_len2 = 0;
    pStr1[0] = '\0';
    if (!(pSdfLabel && pSdfLabel[0]) && !(pSdfValue && pSdfValue[0]))
    {
        tot_len1 = sprintf(pStr1, "Structure: %ld", num_inp);
        tot_len2 = sprintf(pStr2, "structure #%ld", num_inp);
    }
    else
    {
        tot_len1 = sprintf(pStr1, "Structure: %ld.%s%s%s%s", num_inp, SDF_LBL_VAL(pSdfLabel, pSdfValue));
        tot_len2 = sprintf(pStr2, "structure #%ld.%s%s%s%s", num_inp, SDF_LBL_VAL(pSdfLabel, pSdfValue));
        if (lSdfId)
        {
            tot_len1 += sprintf(pStr1 + tot_len1, ":%ld", lSdfId);
            tot_len2 += sprintf(pStr2 + tot_len2, ":%ld", lSdfId);
        }
    }

    return tot_len1;
}

#endif /*#ifndef CREATE_INCHI_STEP_BY_STEP */
