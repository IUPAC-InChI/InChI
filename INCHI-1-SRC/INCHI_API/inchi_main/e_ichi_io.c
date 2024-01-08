/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.03
 *
 * Originally developed at NIST
 * Modifications and additions by IUPAC and the InChI Trust
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC);
 * you can redistribute this software and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software 
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-2.1.php
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>

#include "e_mode.h"
#include "e_ctl_data.h"

#include "inchi_api.h"

#include "ichitime.h"

#include "e_comdef.h"
#include "e_ichicomp.h"
#include "e_util.h"
#include "e_ichi_io.h"
#include "e_ichi_parms.h"


#define LtrimRtrim e_LtrimRtrim

/**********************************************
 * output " L=V" or " L missing" or ""
 * The fprintf format string must contain %s%s%s%s
 */
char e_gsMissing[] = "is missing";
char e_gsEmpty[]   = "";
char e_gsSpace[]   = " ";
char e_gsEqual[]   = "=";






/*******************************************************************/
void e_PrintFileName( const char *fmt, FILE *output_file, const char *szFname )
{
    fprintf( output_file, fmt, szFname );
}





#ifdef INCHI_ANSI_ONLY

static clock_t InchiClock(void);

#ifdef INCHI_USETIMES
static clock_t InchiClock(void)
{
    struct tms buf;
    clock_t c = times( &buf );
    if ( c != (clock_t)-1 ) {
        return buf.tms_utime;
    }
    return 0;
}
#else
static clock_t InchiClock(void)
{
    clock_t c = clock();
    if ( c != (clock_t)-1 ) {
        return c;
    }
    return 0;
}
#endif

#define INCHI_MSEC(X)    (long)((1000.0/(double)CLOCKS_PER_SEC)*(X))
#define INCHI_CLOCK_T(X) (clock_t)( (double)(X) / 1000.0 * (double)CLOCKS_PER_SEC )
const clock_t FullMaxClock = (clock_t)(-1);
const clock_t HalfMaxClock = (clock_t)(-1) / 2;
clock_t MaxPositiveClock = 0;
clock_t MinNegativeClock = 0;
clock_t HalfMaxPositiveClock = 0;
clock_t HalfMinNegativeClock = 0;

static void FillMaxMinClock(void); /* keep compiler happy */

static void FillMaxMinClock(void)
{ /* assume clock_t is a signed integral value */
    if ( !MaxPositiveClock ) {
        clock_t valPos=0, val1 = 1;
        while ( 0 < ((val1 <<= 1), (val1 |= 1)) ) {
            valPos = val1;
        }
        MaxPositiveClock =  valPos;
        MinNegativeClock = -valPos;
        HalfMaxPositiveClock = MaxPositiveClock / 2;
        HalfMinNegativeClock = MinNegativeClock / 2;
    }
}


/******** get current process time ****************************************/
void InchiTimeGet( inchiTime *TickEnd )
{
    TickEnd->clockTime = InchiClock();
}
/******** returns difference TickEnd - TickStart in milliseconds **********/
long InchiTimeMsecDiff( inchiTime *TickEnd, inchiTime *TickStart )
{
    if ( FullMaxClock > 0 ) {
        clock_t delta;
        if ( !TickEnd || !TickStart )
            return 0;
        /* clock_t is unsigned */
        if ( TickEnd->clockTime > TickStart->clockTime ) {
            if ( TickEnd->clockTime > HalfMaxClock &&
                 TickEnd->clockTime - TickStart->clockTime > HalfMaxClock ) {
                /* overflow in TickStart->clockTime, actually TickStart->clockTime was later */
                delta = (FullMaxClock - TickEnd->clockTime) + TickStart->clockTime;
                return -INCHI_MSEC(delta);
            }
            delta = TickEnd->clockTime - TickStart->clockTime;
            return INCHI_MSEC(delta);
        } else
        if ( TickEnd->clockTime < TickStart->clockTime ) {
            if ( TickStart->clockTime > HalfMaxClock &&
                 TickStart->clockTime - TickEnd->clockTime > HalfMaxClock ) {
                /* overflow in TickEnd->clockTime, actually TickEnd->clockTime was later */
                delta = (FullMaxClock - TickStart->clockTime) + TickEnd->clockTime;
                return INCHI_MSEC(delta);
            }
            delta = TickStart->clockTime - TickEnd->clockTime;
            return -INCHI_MSEC(delta);
        }
        return 0; /* TickEnd->clockTime == TickStart->clockTime */
    } else {
        /* may happen under Win32 only where clock_t is SIGNED long */
        clock_t delta;
        FillMaxMinClock( );
        if ( !TickEnd || !TickStart )
            return 0;
        if ( TickEnd->clockTime >= 0 && TickStart->clockTime >= 0 ||
             TickEnd->clockTime <= 0 && TickStart->clockTime <= 0) {
            delta = TickEnd->clockTime - TickStart->clockTime;
        } else
        if ( TickEnd->clockTime >= HalfMaxPositiveClock &&
             TickStart->clockTime <= HalfMinNegativeClock ) {
            /* end is earlier than start */
            delta = (MaxPositiveClock - TickEnd->clockTime) + (TickStart->clockTime - MinNegativeClock);
            delta = -delta;
        } else
        if ( TickEnd->clockTime <= HalfMinNegativeClock &&
             TickStart->clockTime >= HalfMaxPositiveClock ) {
            /* start was earlier than end */
            delta = (MaxPositiveClock - TickStart->clockTime) + (TickEnd->clockTime - MinNegativeClock);
        } else {
            /* there was no overflow, clock passed zero */
            delta = TickEnd->clockTime - TickStart->clockTime;
        }
        return INCHI_MSEC(delta);
    }
}
/******************* get elapsed time from TickStart ************************/
long InchiTimeElapsed( inchiTime *TickStart )
{
    inchiTime TickEnd;
    if ( !TickStart )
        return 0;
    InchiTimeGet( &TickEnd );
    return InchiTimeMsecDiff( &TickEnd, TickStart );
}
/******************* add number of milliseconds to time *********************/
void InchiTimeAddMsec( inchiTime *TickEnd, unsigned long nNumMsec )
{
    clock_t delta;
    if ( !TickEnd )
        return;
    if ( FullMaxClock > 0 ) {
        /* clock_t is unsigned */
        delta = INCHI_CLOCK_T(nNumMsec);
        TickEnd->clockTime += delta;
    } else {
        /* may happen under Win32 only where clock_t is SIGNED long */
        /* clock_t is unsigned */
        FillMaxMinClock( );
        delta = INCHI_CLOCK_T(nNumMsec);
        TickEnd->clockTime += delta;
    }
}
/******************* check whether time has expired *********************/
int bInchiTimeIsOver( inchiTime *TickStart )
{
    if ( FullMaxClock > 0 ) {
        clock_t clockCurrTime;
        if ( !TickStart )
            return 0;
        clockCurrTime = InchiClock();
        /* clock_t is unsigned */
        if ( TickStart->clockTime > clockCurrTime ) {
            if ( TickStart->clockTime > HalfMaxClock &&
                 TickStart->clockTime - clockCurrTime > HalfMaxClock ) {
                /* overflow in clockCurrTime, actually clockCurrTime was later */
                return 1;
            }
            return 0;
        } else
        if ( TickStart->clockTime < clockCurrTime ) {
            if ( clockCurrTime > HalfMaxClock &&
                 clockCurrTime - TickStart->clockTime > HalfMaxClock ) {
                /* overflow in TickStart->clockTime, actually TickStart->clockTime was later */
                return 0;
            }
            return 1;
        }
        return 0; /* TickStart->clockTime == clockCurrTime */
    } else {
        /* may happen under Win32 only where clock_t is SIGNED long */
        clock_t clockCurrTime;
        FillMaxMinClock( );
        if ( !TickStart )
            return 0;
        clockCurrTime = InchiClock();
        if ( clockCurrTime >= 0 && TickStart->clockTime >= 0 ||
             clockCurrTime <= 0 && TickStart->clockTime <= 0) {
            return (clockCurrTime > TickStart->clockTime);
        } else
        if ( clockCurrTime >= HalfMaxPositiveClock &&
             TickStart->clockTime <= HalfMinNegativeClock ) {
            /* curr is earlier than start */
            return 0;
        } else
        if ( clockCurrTime <= HalfMinNegativeClock &&
             TickStart->clockTime >= HalfMaxPositiveClock ) {
            /* start was earlier than curr */
            return 1;
        } else {
            /* there was no overflow, clock passed zero */
            return (clockCurrTime > TickStart->clockTime);
        }
    }
}

#else

/******** get current process time ****************************************/
void InchiTimeGet( inchiTime *TickEnd )
{
    if ( TickEnd ) {
        struct _timeb timeb;
        _ftime( &timeb );
        TickEnd->clockTime = (unsigned long)timeb.time;
        TickEnd->millitime = (long)timeb.millitm;
    }
}
/******** returns difference TickEnd - TickStart in milliseconds **********/
long InchiTimeMsecDiff( inchiTime *TickEnd, inchiTime *TickStart )
{
    long delta;
    if ( !TickEnd || !TickStart ) {
        return 0;
    }
    if ( TickEnd->clockTime >= TickStart->clockTime ) {
        delta = (long)(TickEnd->clockTime - TickStart->clockTime);
        delta *= 1000;
        delta += TickEnd->millitime - TickStart->millitime;
    } else {
        delta = -(long)(TickStart->clockTime - TickEnd->clockTime);
        delta *= 1000;
        delta += TickEnd->millitime - TickStart->millitime;
    }
    return delta;
}
/******************* get elapsed time from TickStart ************************/
long InchiTimeElapsed( inchiTime *TickStart )
{
    inchiTime TickEnd;
    if ( !TickStart )
        return 0;
    InchiTimeGet( &TickEnd );
    return InchiTimeMsecDiff( &TickEnd, TickStart );
}
/******************* add number of milliseconds to time *********************/
void InchiTimeAddMsec( inchiTime *TickEnd, unsigned long nNumMsec )
{
    long delta;
    if ( !TickEnd )
        return;
    TickEnd->clockTime += nNumMsec / 1000;
    delta = nNumMsec % 1000 + TickEnd->millitime;
    TickEnd->clockTime += delta / 1000;
    TickEnd->millitime = delta % 1000;
}
/******************* check whether time has expired *********************/
int bInchiTimeIsOver( inchiTime *TickEnd )
{
    struct _timeb timeb;
    if ( !TickEnd )
        return 0;
    _ftime( &timeb );
    if ( TickEnd->clockTime > (unsigned long)timeb.time )
        return 0;
    if ( TickEnd->clockTime < (unsigned long)timeb.time ||
         TickEnd->millitime < (long)timeb.millitm ) {
        return 1;
    }
    return 0;
}
#endif





#ifndef INCHI_LIBRARY


#ifndef INCHI_ADD_STR_LEN
#define INCHI_ADD_STR_LEN   32768
#endif


/*^^^ Internal functions */

int inchi_ios_str_getc( INCHI_IOSTREAM *ios );
char *inchi_ios_str_gets( char *szLine, int len, INCHI_IOSTREAM *ios );
char *inchi_ios_str_getsTab( char *szLine, int len, INCHI_IOSTREAM *ios );
int GetMaxPrintfLength( const char *lpszFormat, va_list argList);
char *inchi_fgetsTab( char *szLine, int len, FILE *f ); 
int inchi_vfprintf( FILE* f, const char* lpszFormat, va_list argList ); 

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


                                    INCHI_IOSTREAM OPERATIONS 


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    Init INCHI_IOSTREAM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void inchi_ios_init(INCHI_IOSTREAM* ios, int io_type, FILE *f)
{
    memset( ios, 0, sizeof(*ios) );
    switch (io_type)
    {
        case INCHI_IOSTREAM_FILE:  ios->type = INCHI_IOSTREAM_FILE; 
                                    break;
        case INCHI_IOSTREAM_STRING:
        default:                    ios->type = INCHI_IOSTREAM_STRING;
                                    break;
    }
    ios->f = f; 
    return;
}



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    If INCHI_IOSTREAM type is INCHI_IOSTREAM_STRING,
        flush INCHI_IOSTREAM string buffer to file (if non-NULL); then free buffer.
    If INCHI_IOSTREAM type is INCHI_IOSTREAM_FILE, just flush the file.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void inchi_ios_flush(INCHI_IOSTREAM* ios)
{
    if (ios->type == INCHI_IOSTREAM_STRING) 
    {
        if (ios->s.pStr)
        {
            if (ios->s.nUsedLength > 0)
            {       
                if (ios->f)
                {
                    fprintf(ios->f,"%-s", ios->s.pStr); 
                    fflush(ios->f);
                }
                inchi_free(ios->s.pStr );
                ios->s.pStr = NULL; 
                ios->s.nUsedLength = ios->s.nAllocatedLength = ios->s.nPtr = 0;
            }       
        }
    }
    else if (ios->type == INCHI_IOSTREAM_FILE)
    {
        /* output to plain file: just flush it. */
        fflush(ios->f);
    }
    return;
}



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    If INCHI_IOSTREAM type is INCHI_IOSTREAM_STRING,
        flush INCHI_IOSTREAM string buffer to file (if non-NULL) and
        another file f2 supplied as parameter (typically, it will be stderr); then free buffer.
    If INCHI_IOSTREAM type is INCHI_IOSTREAM_FILE, just flush the both files.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void inchi_ios_flush2(INCHI_IOSTREAM* ios, FILE *f2)
{
    if (ios->type == INCHI_IOSTREAM_STRING) 
    {
        if (ios->s.pStr)
        {
            if (ios->s.nUsedLength > 0)
            {       
                if (ios->f)
                {
                    fprintf(ios->f,"%-s", ios->s.pStr); 
                    fflush(ios->f);
                }
                if (f2!=ios->f)
                    fprintf(f2,"%-s", ios->s.pStr); 
                
                inchi_free(ios->s.pStr );
                ios->s.pStr = NULL; 
                ios->s.nUsedLength = ios->s.nAllocatedLength = ios->s.nPtr = 0;
            }       
        }
    }
    else if (ios->type == INCHI_IOSTREAM_FILE)
    {
        /* output to plain file: just flush it. */
        if ( (ios->f) && (ios->f!=stderr) && (ios->f!=stdout) )
            fflush(ios->f);
        if ( f2 && f2!=stderr && f2!=stdout)
            fflush(f2);


    }

    return;
}



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    Close INCHI_IOSTREAM: free string buffer and close the file.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void inchi_ios_close(INCHI_IOSTREAM* ios)
{
    if (ios->s.pStr)
        inchi_free(ios->s.pStr);
    ios->s.pStr = NULL; 
    ios->s.nUsedLength = ios->s.nAllocatedLength = ios->s.nPtr = 0;
    if ( (ios->f) && (ios->f!=stderr) && (ios->f!=stdout) && (ios->f!=stdin))
        fclose(ios->f);
    return;
}



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    Reset INCHI_IOSTREAM: set string buffer ptr to NULL (but do _not_ free memory)and close the file.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
void inchi_ios_reset(INCHI_IOSTREAM* ios)
{
    ios->s.pStr = NULL; 
    ios->s.nUsedLength = ios->s.nAllocatedLength = ios->s.nPtr = 0;
    if ( (ios->f) && (ios->f!=stderr) && (ios->f!=stdout) && (ios->f!=stdin))
        fclose(ios->f);
    return;
}





/*******************************************************************/
int inchi_ios_str_getc(INCHI_IOSTREAM *ios)
{
    if (ios->type==INCHI_IOSTREAM_STRING)
    {
        if ( ios->s.nPtr < ios->s.nUsedLength ) 
        {
            return (int)ios->s.pStr[ios->s.nPtr++];
        }
        return EOF;
    }
    else if (ios->type==INCHI_IOSTREAM_FILE)
    {
        return fgetc( ios->f );
    }
    
    /* error */
    return EOF;
}



/*******************************************************************/
char *inchi_ios_str_gets(char *szLine, int len, INCHI_IOSTREAM *f)
{
    int  length=0, c=0;
    if ( -- len < 0 ) 
    {
        return NULL;
    }
    while ( length < len && EOF != (c = inchi_ios_str_getc( f )) ) 
    {
        szLine[length++] = (char)c;
        if ( c == '\n' )
            break;
    }
    if ( !length && EOF == c ) 
    {
        return NULL;
    }
    szLine[length] = '\0';
    return szLine;
}



/********************************************************************************/
/* read up to len or tab or LF; if empty read next until finds non-empty line   */
/* remove leading and trailing white spaces; keep zero termination              */
/********************************************************************************/
char *inchi_ios_str_getsTab( char *szLine, int len, INCHI_IOSTREAM *f )
{
    int  length=0, c=0;
    if ( --len < 0 ) 
    {
        return NULL;
    }
    while ( length < len && EOF != (c = inchi_ios_str_getc(f)) ) 
    {
        if ( c == '\t' )
            c = '\n';
        szLine[length++] = (char)c;
        if ( c == '\n' )
            break;
    }
    if ( !length && EOF == c ) 
    {
        return NULL;
    }
    szLine[length] = '\0';
    return szLine;
}


/*******************************************************************/
int inchi_ios_gets( char *szLine, int len, INCHI_IOSTREAM *f, int *bTooLongLine )
{
    int  length;
    char *p;
    do 
    {
        p = inchi_ios_str_gets( szLine, len-1, f );
        if ( !p ) 
        {
            *bTooLongLine = 0;
            return -1; /* end of file or cannot read */
        }
        szLine[len-1] = '\0';
        /*
        *bTooLongLine = !strchr( szLine, '\n' );
        */
        p = strchr( szLine, '\n' );
        *bTooLongLine = ( !p && ((int)strlen(szLine)) == len-2 );
        LtrimRtrim( szLine, &length );
    } while ( !length );

    return length;
}


/*******************************************************************/
/* read up to len or tab or LF; if empty read next until finds non-empty line   */
/* remove leading and trailing white spaces; keep zero termination */
/*******************************************************************/
int inchi_ios_getsTab( char *szLine, int len, INCHI_IOSTREAM *f, int *bTooLongLine )
{
    int  length;
    char *p;
    do 
    {
        p = inchi_ios_str_getsTab( szLine, len-1, f );
        if ( !p ) 
        {
            *bTooLongLine = 0;
            return -1; /* end of file or cannot read */
        }
        szLine[len-1] = '\0';
        /*
        *bTooLongLine = !strchr( szLine, '\n' );
        */
        p = strchr( szLine, '\n' );
        *bTooLongLine = ( !p && ((int)strlen(szLine)) == len-2 );
        LtrimRtrim( szLine, &length );
    } while ( !length );
    return length;
}

/*******************************************************************/
int inchi_ios_getsTab1( char *szLine, int len, INCHI_IOSTREAM *f, int *bTooLongLine )
{
    int  length;
    char *p;
    /*do {*/
        p = inchi_ios_str_getsTab( szLine, len-1, f );
        if ( !p ) 
        {
            *bTooLongLine = 0;
            return -1; /* end of file or cannot read */
        }
        szLine[len-1] = '\0';
        /*
        *bTooLongLine = !strchr( szLine, '\n' );
        */
        p = strchr( szLine, '\n' );
        *bTooLongLine = ( !p && ((int)strlen(szLine)) == len-2 );
        LtrimRtrim( szLine, &length );
    /*} while ( !length );*/
    return length;
}






/*****************************************************************/
int inchi_ios_print( INCHI_IOSTREAM * ios, const char* lpszFormat, ... )
{
    if (!ios) return -1;
    
    if (ios->type == INCHI_IOSTREAM_STRING) 
    {
    
        /* output to string buffer */
        
        int ret=0, max_len;
        va_list argList;
        my_va_start( argList, lpszFormat );
        max_len = GetMaxPrintfLength( lpszFormat, argList);
        va_end( argList );

        if ( max_len >= 0 ) 
        {
            if ( ios->s.nAllocatedLength - ios->s.nUsedLength <= max_len ) 
            {
                /* enlarge output string */
                int  nAddLength = inchi_max( INCHI_ADD_STR_LEN, max_len );
                char *new_str = 
                    (char *)inchi_calloc( ios->s.nAllocatedLength + nAddLength, sizeof(new_str[0]) );
                if ( new_str ) 
                {
                    if ( ios->s.pStr ) 
                    {
                        if ( ios->s.nUsedLength > 0 ) 
                            memcpy( new_str, ios->s.pStr, sizeof(new_str[0])* ios->s.nUsedLength );
                        inchi_free( ios->s.pStr );
                    }
                    ios->s.pStr              = new_str;
                    ios->s.nAllocatedLength += nAddLength;
                } 
                else return -1; /* failed */
            }
            /* output */
            my_va_start( argList, lpszFormat );
            ret = vsprintf( ios->s.pStr + ios->s.nUsedLength, lpszFormat, argList );
            va_end(argList);
            if ( ret >= 0 ) 
                ios->s.nUsedLength += ret;
            return ret;
        }
        return -1;
    }
    
    else if (ios->type == INCHI_IOSTREAM_FILE)
    {
        /* output to file */
        int ret=0, ret2=0;
        va_list argList;
        if (ios->f) 
        {
            my_va_start( argList, lpszFormat );
            ret = vfprintf( ios->f, lpszFormat, argList );
            va_end( argList );
        } 
        else 
        {
            my_va_start( argList, lpszFormat );
            ret2 = vfprintf( stdout, lpszFormat, argList );
            va_end( argList );
        }
#ifdef INCHI_LIB
        if( FWPRINT )
        {
            my_va_start( argList, lpszFormat );
            FWPRINT( lpszFormat, argList );
            va_end( argList );
        }
#endif
        return ret? ret : ret2;
    }


    /* no output */
    return 0;
}




/**********************************************************************/
/* This function's output should not be displayed in the output pane  */
/**********************************************************************/
int inchi_ios_print_nodisplay( INCHI_IOSTREAM * ios, const char* lpszFormat, ... )
{
    if (!ios) return -1;
    
    if (ios->type == INCHI_IOSTREAM_STRING) 
    {
        /* output to string buffer */
        int ret=0, max_len;
        va_list argList;
        my_va_start( argList, lpszFormat );
        max_len = GetMaxPrintfLength( lpszFormat, argList);
        va_end( argList );

        if ( max_len >= 0 ) 
        {
            if ( ios->s.nAllocatedLength - ios->s.nUsedLength <= max_len ) 
            {
                /* enlarge output string */
                int  nAddLength = inchi_max( INCHI_ADD_STR_LEN, max_len );
                char *new_str = (char *)inchi_calloc( ios->s.nAllocatedLength + nAddLength, sizeof(new_str[0]) );
                if ( new_str ) 
                {
                    if ( ios->s.pStr ) 
                    {
                        if ( ios->s.nUsedLength > 0 ) 
                        {
                            memcpy( new_str, ios->s.pStr, sizeof(new_str[0])*ios->s.nUsedLength );
                        }
                        inchi_free( ios->s.pStr );
                    }
                    ios->s.pStr              = new_str;
                    ios->s.nAllocatedLength += nAddLength;
                } else 
                {
                    return -1; /* failed */
                }
            }
            /* output */
            my_va_start( argList, lpszFormat );
            ret = vsprintf( ios->s.pStr + ios->s.nUsedLength, lpszFormat, argList );
            va_end(argList);
            if ( ret >= 0 ) 
            {
                ios->s.nUsedLength += ret;
            }
            return ret;
        }
        return -1;
    }

    else if (ios->type == INCHI_IOSTREAM_FILE)
    {
        /* output to file */
        int ret=0, ret2=0;
        va_list argList;
        if (ios->f) 
        {
            my_va_start( argList, lpszFormat );
            ret = vfprintf( ios->f, lpszFormat, argList );
            va_end( argList );
        } 
        else 
        {
            my_va_start( argList, lpszFormat );
            ret2 = vfprintf( stdout, lpszFormat, argList );
            va_end( argList );
        }
        return ret? ret : ret2;
    }

    /* no output */
    return 0;
}




/*****************************************************************/
/* Print to string buffer or to file+stderr */
int inchi_ios_eprint( INCHI_IOSTREAM * ios, const char* lpszFormat, ... )
{
int ret=0, ret2=0;
va_list argList;

    if (!ios) 
        return -1;

    if (ios->type == INCHI_IOSTREAM_STRING) /* was #if ( defined(INCHI_LIBRARY) || defined(BUILD_CINCHI_WITH_INCHIKEY) ) */
    {
        /* output to string buffer */
        int max_len, nAddLength = 0;
        char *new_str = NULL;

        my_va_start( argList, lpszFormat );
        max_len = GetMaxPrintfLength( lpszFormat, argList);
        va_end( argList );

        if ( max_len >= 0 ) 
        {
            if ( ios->s.nAllocatedLength - ios->s.nUsedLength <= max_len ) 
            {
                /* enlarge output string */
                nAddLength = inchi_max( INCHI_ADD_STR_LEN, max_len );
                new_str = (char *)inchi_calloc( ios->s.nAllocatedLength + nAddLength, sizeof(new_str[0]) );
                if ( new_str ) 
                {
                    if ( ios->s.pStr ) 
                    {
                        if ( ios->s.nUsedLength > 0 ) 
                        {
                            memcpy( new_str, ios->s.pStr, sizeof(new_str[0])* ios->s.nUsedLength );
                        }
                        inchi_free( ios->s.pStr );
                    }
                    ios->s.pStr              = new_str;
                    ios->s.nAllocatedLength += nAddLength;
                } 
                else 
                {
                    return -1; /* failed */
                }
            }
         
            /* output */
            my_va_start( argList, lpszFormat );
            ret = vsprintf( ios->s.pStr + ios->s.nUsedLength, lpszFormat, argList );
            va_end(argList);
            if ( ret >= 0 ) 
            {
                ios->s.nUsedLength += ret;
            }
            return ret;
        }
        return -1;
    }

    else if (ios->type == INCHI_IOSTREAM_FILE)
    {
        if ( ios->f) 
        {
            /* output to plain file */
            my_va_start( argList, lpszFormat );
            ret = inchi_vfprintf( ios->f, lpszFormat, argList ); 
            va_end( argList );
/*^^^  No output to stderr from within dll or GUI program */
#if ( !defined(INCHI_LIBRARY) && !defined(INCHI_LIB) )
            if ( ios->f != stderr ) 
            { 
                my_va_start( argList, lpszFormat );
                ret2 = vfprintf( stderr, lpszFormat, argList );
                va_end( argList );
            }
#endif
            return ret? ret : ret2;
        }
    } 

    /* no output */
    return 0;
}









/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


                                    PLAIN FILE OPERATIONS 


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* Print to file, echoing to stderr */
int inchi_fprintf( FILE* f, const char* lpszFormat, ... )
{
int ret=0, ret2=0;
va_list argList;
    if (f) 
    {
        my_va_start( argList, lpszFormat );
        ret = inchi_vfprintf( f, lpszFormat, argList ); 
        va_end( argList );
/*^^^  No output to stderr from within dll or GUI program */
#if ( !defined(INCHI_LIBRARY) && !defined(INCHI_LIB) )
        if ( f != stderr ) 
        { 
            my_va_start( argList, lpszFormat );
            ret2 = vfprintf( stderr, lpszFormat, argList );
            va_end( argList );
        }
#endif
        return ret? ret : ret2;
    }
    return 0;
}




/* Print to file */
int inchi_vfprintf( FILE* f, const char* lpszFormat, va_list argList )
{
int ret=0;

    if ( f == stderr && lpszFormat && lpszFormat[0] && '\r' == lpszFormat[strlen(lpszFormat)-1] ) 
    {
#define CONSOLE_LINE_LEN 80
#ifndef INCHI_ANSI_ONLY
        char szLine[CONSOLE_LINE_LEN];
        ret = _vsnprintf( szLine, CONSOLE_LINE_LEN-1, lpszFormat, argList );
        if ( ret < 0 ) 
        {
            /*  output is longer than the console line */
            /*^^^ Fixed bug: (CONSOLE_LINE_LEN-4) --> (CONSOLE_LINE_LEN-4-1) 11-22-08 IPl */
            strcpy(szLine+CONSOLE_LINE_LEN-5, "...\r");

        }
        fputs( szLine, f );
#else
        ret = vfprintf( f, lpszFormat, argList );
#endif
#undef CONSOLE_LINE_LEN
    } 
    else 
    {
        ret = vfprintf( f, lpszFormat, argList );
    }
    return ret;
}


#if ( FIX_READ_LONG_LINE_BUG == 1 )
/********************************************************************/
int inchi_fgetsLfTab( char *szLine, int len, FILE *f )
{
    int  length;
    char *p;
    char szSkip[256];
    int  bTooLongLine = 0;
    do {
        p = inchi_fgetsTab( szLine, len, f );
        if ( !p ) {
            return -1; /* end of file or cannot read */
        }
        bTooLongLine = ( (int)strlen(szLine) == len-1 && szLine[len-2] != '\n' );
        LtrimRtrim( szLine, &length );
    } while ( !length );
    if ( bTooLongLine ) {
        while ( p = inchi_fgetsTab( szSkip, sizeof(szSkip)-1, f ) ) {
            if ( strchr( szSkip, '\n' ) )
                break;
        }
    }
    return length;
}
#else
/********************************************************************/
int inchi_fgetsLfTab( char *szLine, int len, FILE *f )
{
    int  length;
    char *p;
    char szSkip[256];
    int  bTooLongLine = 0;
    do {
        p = inchi_fgetsTab( szLine, len-1, f );
        if ( !p ) {
            return -1; /* end of file or cannot read */
        }
        szLine[len-1] = '\0';
        /*
        bTooLongLine = !strchr( szLine, '\n' );
        */
        bTooLongLine = ( !p && ((int)strlen(szLine)) == len-2 );
        LtrimRtrim( szLine, &length );
    } while ( !length );
    if ( bTooLongLine ) {
        while ( p = inchi_fgetsTab( szSkip, sizeof(szSkip)-1, f ) ) {
            szSkip[sizeof(szSkip)-1] = '\0';
            if ( strchr( szSkip, '\n' ) )
                break;
        }
    }
    return length;
}
#endif


/*******************************************************************/
/* read up to len or tab or LF; if empty read next until finds non-empty line   */
/* remove leading and trailing white spaces; keep zero termination */
/*******************************************************************/
char *inchi_fgetsTab( char *szLine, int len, FILE *f )
{
    int  length=0, c=0;
    len --;
    while ( length < len && EOF != (c = fgetc( f )) ) {
        if ( c == '\t' )
            c = '\n';
        szLine[length++] = (char)c;
        if ( c == '\n' )
            break;
    }
    if ( !length && EOF == c ) {
        return NULL;
    }
    szLine[length] = '\0';
    return szLine;
}



/******************************************************************/
/* read not more than line_len bytes from an lf-terminated line   */
/* if input line is too long quietly ignore the rest of the line  */
char* inchi_fgetsLf( char* line, int line_len, FILE* inp )
{
    char *p, *q;
    memset( line, 0, line_len );
    if ( NULL != (p = fgets( line, line_len, inp ) ) && NULL == strchr(p, '\n' ) ){
        char temp[64]; /* bypass up to '\n' or up to end of file whichever comes first*/
        while ( NULL != fgets( temp, sizeof(temp), inp ) && NULL == strchr(temp,'\n') )
            ;
    }
    if ( p && (q = strchr(line, '\r')) ) { /*  fix CR CR LF line terminator. */
        q[0] = '\n';
        q[1] = '\0';
    }    
    return p;
}










/*****************************************************************
 *
 *  Estimate printf string length
 *  
 *  The code is based on Microsoft Knowledge Base article Q127038:
 *  "FIX: CString::Format Gives Assertion Failed, Access Violation"
 *  (Related to Microsoft Visual C++, 32-bit Editions, versions 2.0, 2.1)
 *
 *****************************************************************/

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

/* formatting (using wsprintf style formatting)*/
int GetMaxPrintfLength( const char *lpszFormat, va_list argList)
{
     /*ASSERT(AfxIsValidString(lpszFormat, FALSE));*/
     const char * lpsz;
     int nMaxLen, nWidth, nPrecision, nModifier, nItemLen;

     nMaxLen = 0;
     /* make a guess at the maximum length of the resulting string */
     for ( lpsz = lpszFormat; *lpsz; lpsz ++ )
     {
          /* handle '%' character, but watch out for '%%' */
          if (*lpsz != '%' || *( ++ lpsz ) == '%')
          {
               nMaxLen += 1;
               continue;
          }

          nItemLen = 0;

          /*  handle '%' character with format */
          nWidth = 0;
          for (; *lpsz; lpsz ++ )
          {
               /* check for valid flags */
               if (*lpsz == '#')
                    nMaxLen += 2;   /* for '0x' */
               else if (*lpsz == '*')
                    nWidth = va_arg(argList, int);
               else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0'
                        || *lpsz == ' ')
                           ;
               else /* hit non-flag character */
                          break;
          }
          /* get width and skip it */
          if (nWidth == 0)
          {
               /* width indicated by */
               nWidth = atoi(lpsz);
               for (; *lpsz && isdigit(*lpsz); lpsz ++ )
                     ;
          }
          /*ASSERT(nWidth >= 0);*/
          if ( nWidth < 0 )
              goto exit_error; /* instead of exception */

          nPrecision = 0;
          if (*lpsz == '.')
          {
               /* skip past '.' separator (width.precision)*/
               lpsz ++;

               /* get precision and skip it*/
               if (*lpsz == '*')
               {
                    nPrecision = va_arg(argList, int);
                    lpsz ++;
               }
               else
               {
                    nPrecision = atoi(lpsz);
                    for (; *lpsz && isdigit(*lpsz); lpsz ++)
                            ;
               }
              if ( nPrecision < 0 )
                  goto exit_error; /* instead of exception */
          }

          /* should be on type modifier or specifier */
          nModifier = 0;
          switch (*lpsz)
          {
          /* modifiers that affect size */
          case 'h':
               switch ( lpsz[1] ) {
               case 'd':
               case 'i':
               case 'o':
               case 'x':
               case 'X':
               case 'u':
                   /* short unsigned, short double, etc. -- added to the original MS example */
                   /* ignore the fact that these modifiers do affect size */
                   lpsz ++;
                   break;
               default:
                   nModifier = FORCE_ANSI;
                   lpsz ++;
                   break;
               }
               break;
          case 'l':
               switch ( lpsz[1] ) {
               case 'd':
               case 'i':
               case 'o':
               case 'x':
               case 'X':
               case 'u':
               case 'f': /* long float -- post ANSI C */
                   /* long unsigned, long double, etc. -- added to the original MS example */
                   /* ignore the fact that these modifiers do affect size */
                   lpsz ++;
                   break;
               default:
                   /*
                   nModifier = FORCE_UNICODE;
                   lpsz ++;
                   break;
                   */
                   goto exit_error;  /* no UNICODE, please */
               }
               break;
          /* modifiers that do not affect size */
          case 'F':
          case 'N':
          case 'L':
               lpsz ++;
               break;
          }

          /* now should be on specifier */
          switch (*lpsz | nModifier)
          {
          /* single characters*/
          case 'c':
          case 'C':
               nItemLen = 2;
               va_arg(argList, int);
               break;
          case 'c'|FORCE_ANSI:
          case 'C'|FORCE_ANSI:
               nItemLen = 2;
               va_arg(argList, int);
               break;
          case 'c'|FORCE_UNICODE:
          case 'C'|FORCE_UNICODE:
               goto exit_error;  /* no UNICODE, please */
               /*
               nItemLen = 2;
               va_arg(argList, wchar_t);
               break;
               */

          /* strings*/
          case 's':
          case 'S':
               nItemLen = strlen(va_arg(argList, char*));
               nItemLen = inchi_max(1, nItemLen);
               break;
          case 's'|FORCE_ANSI:
          case 'S'|FORCE_ANSI:
               nItemLen = strlen(va_arg(argList, char*));
               nItemLen = inchi_max(1, nItemLen);
               break;

          case 's'|FORCE_UNICODE:
          case 'S'|FORCE_UNICODE:
               goto exit_error;  /* no UNICODE, please */
               /*
               nItemLen = wcslen(va_arg(argList, wchar_t*));
               nItemLen = inchi_max(1, nItemLen);
               break;
               */

          }

          /* adjust nItemLen for strings */
          if (nItemLen != 0)
          {
               nItemLen = inchi_max(nItemLen, nWidth);
               if (nPrecision != 0)
                    nItemLen = inchi_min(nItemLen, nPrecision);
          }
          else
          {
               switch (*lpsz)
               {
               /* integers */
               case 'd':
               case 'i':
               case 'u':
               case 'x':
               case 'X':
               case 'o':
                    va_arg(argList, int);
                    nItemLen = 32;
                    nItemLen = inchi_max(nItemLen, nWidth+nPrecision);
                    break;

               case 'e':
               case 'f':
               case 'g':
               case 'G':
                    va_arg(argList, double);
                    nItemLen = 32;
                    nItemLen = inchi_max(nItemLen, nWidth+nPrecision);
                    break;

               case 'p':
                    va_arg(argList, void*);
                    nItemLen = 32;
                    nItemLen = inchi_max(nItemLen, nWidth+nPrecision);
                    break;

               /* no output */
               case 'n':
                    va_arg(argList, int*);
                    break;

               default:
                   /*ASSERT(FALSE);*/  /* unknown formatting option*/
                   goto exit_error; /* instead of exception */
               }
          }

          /* adjust nMaxLen for output nItemLen */
          nMaxLen += nItemLen;
     }
     return nMaxLen;

exit_error:
     return -1; /* wrong format */
} 



#endif
