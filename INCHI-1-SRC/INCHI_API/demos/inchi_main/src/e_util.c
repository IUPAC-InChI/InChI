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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "e_mode.h"

#include "../../../../INCHI_BASE/src/inchi_api.h"

#include "e_ichitime.h"
#include "e_ichisize.h"
#include "e_comdef.h"
#include "e_util.h"
#include "e_ichicomp.h"

#define extract_ChargeRadical  e_extract_ChargeRadical
#define normalize_name         e_normalize_name
/******************************************************************************************************/
int extract_ChargeRadical( char *elname, int *pnRadical, int *pnCharge )
{
    char *q, *r, *p;
    int  nCharge=0, nRad = 0, charge_len = 0, k, nVal, nSign, nLastSign=1, len;

    p = elname;

    /*  extract radicals & charges */
    while ( q = strpbrk( p, "+-^" ) ) {
        switch ( *q ) {
        case '+':
        case '-':
            for ( k = 0, nVal=0; (nSign = ('+' == q[k])) || (nSign = -('-' == q[k])); k++ ) {
                nVal += (nLastSign = nSign);
                charge_len ++;
            }
            if ( nSign = (int)strtol( q+k, &r, 10 ) ) { /*  fixed 12-5-2001 */
                nVal += nLastSign * (nSign-1);
            }
            charge_len = r - q;
            nCharge += nVal;
            break;
        /* case '.': */ /*  singlet '.' may be confused with '.' in formulas like CaO.H2O */
        case '^':
            nRad = 1; /* doublet here is 1. See below */
            charge_len = 1;
            for ( k = 1; q[0] == q[k]; k++ ) {
                nRad ++;
                charge_len ++;
            }
            break;
        }
        memmove( q, q+charge_len, strlen(q+charge_len)+1 );
    }
    len = strlen(p);
    /*  radical */
    if ( (q = strrchr( p, ':' )) && !q[1]) {
        nRad = RADICAL_SINGLET;
        q[0] = '\0';
        len --;
    } else {
        while( (q = strrchr( p, '.' )) && !q[1] ) {
            nRad ++;
            q[0] = '\0';
            len --;
        }

        nRad = nRad == 1? RADICAL_DOUBLET :
               nRad == 2? RADICAL_TRIPLET : 0;
    }
    *pnRadical = nRad;
    *pnCharge  = nCharge;
    return ( nRad || nCharge );
}
/*****************************************************************/
int normalize_name( char* name )
{
    /* remove leading & trailing spaces; replace consecutive spaces with a single space */
    /* Treat non-printable characters (Greeks) as spaces. 11-23-99 DCh. */
    int i, len, n;
    len = (int)strlen(name);
    for ( i = 0, n = 0; i < len; i++ ) {
        if ( isspace( UCINT name[i] ) /*|| !isprint( UCINT name[i] )*/ ) {
            name[i] = ' '; /* exterminate tabs !!! */
            n++;
        } else {
            if ( n > 0 ) {
                memmove( (void*) &name[i-n], (void*) &name[i], len-i+1 );
                i   -= n;
                len -= n;
            }
            n = -1;
        }
    }
    if ( n == len ) /* empty line */
        name[len=0] = '\0';
    else
    if ( ++n && n <= len ) {
        len -= n;
        name[len] = '\0';
    }
    return len;
}
/************************************************/
#ifndef e_inchi_malloc
void *e_inchi_malloc(size_t c)
{
    return  malloc(c);
}
#endif
#ifndef e_inchi_calloc
void *e_inchi_calloc(size_t c, size_t n)
{
    return calloc(c,n);
}
#endif
#ifndef e_inchi_free
void e_inchi_free(void *p)
{
    if(p) {
        free(p); /*added check if zero*/
    }
}
#endif


/***************************************************************************/
/* Copies up to maxlen characters INCLUDING end null from source to target */
/* Fills out the rest of the target with null bytes */
int e_mystrncpy(char *target,const char *source,unsigned maxlen)
{   /*  protected from non-zero-terminated source and overlapped target/source. 7-9-99 DCh. */
    const char  *p;
    unsigned    len;

    if (target==NULL || maxlen == 0 || source == NULL)
        return 0;
    if ( p = (const char*)memchr(source, 0, maxlen) ) {
        len = p-source; /*  maxlen does not include the found zero termination */
    } else {
        len = maxlen-1; /*  reduced length does not include one more byte for zero termination */
    }
    if ( len )
        memmove( target, source, len );
    /* target[len] = '\0'; */
    memset( target+len, 0, maxlen-len); /*  zero termination */
    return 1;
}
/************************************************************************/
/* Remove leading and trailing white spaces                             */
char* e_LtrimRtrim( char *p, int* nLen )
{
    int i, len=0;
    if ( p &&  (len = strlen( p )) ) {
        for ( i = 0; i < len && __isascii( p[i] ) && isspace( p[i] ); i++ )
            ;
        if ( i )
            (memmove)( p, p+i, (len -= i)+1 );
        for ( ; 0 < len && __isascii( p[len-1] ) && isspace( p[len-1] ); len-- )
            ;
        p[len] = '\0';
    }
    if ( nLen )
        *nLen = len;
    return p;
}

/*************************************************************************/
void e_remove_trailing_spaces( char* p )
{
    int   len;
    for( len = (int)strlen( p ) - 1; len >= 0 && isspace( UCINT p[len] ); len-- )
        ;
    p[++len] = '\0';
}
/*************************************************************************/
void e_remove_one_lf( char* p)
{
    size_t len;
    if ( p && 0 < (len = strlen(p)) && p[len-1] == '\n' ){
        p[len-1] = '\0';
        if ( len >= 2 && p[len-2] == '\r' )
            p[len-2] = '\0';
    }
}
/*************************************************************************/
S_SHORT *e_is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen )
{
    for ( ; nPathLen && *pathAtom != nNextAtom; nPathLen--,  pathAtom++ )
        ;
    return nPathLen? pathAtom : NULL;
}
/************************************************/
int e_is_element_a_metal( char szEl[] )
{
    static char szMetals[] = "K;V;Y;W;U;"
        "Li;Be;Na;Mg;Al;Ca;Sc;Ti;Cr;Mn;Fe;Co;Ni;Cu;Zn;Ga;Rb;Sr;Zr;"
        "Nb;Mo;Tc;Ru;Rh;Pd;Ag;Cd;In;Sn;Sb;Cs;Ba;La;Ce;Pr;Nd;Pm;Sm;"
        "Eu;Gd;Tb;Dy;Ho;Er;Tm;Yb;Lu;Hf;Ta;Re;Os;Ir;Pt;Au;Hg;Tl;Pb;"
        "Bi;Po;Fr;Ra;Ac;Th;Pa;Np;Pu;Am;Cm;Bk;Cf;Es;Fm;Md;No;Lr;Rf;";
    int len = strlen(szEl);
    char *p;

    if ( 0 < len && len <= 2 &&
         isalpha( UCINT szEl[0] ) && isupper( szEl[0] ) &&
         (p = strstr(szMetals, szEl) ) && p[len] == ';' ) {

            return 1; /*return AtType_Metal;*/
    }
    return 0;
}





#ifdef COMPILE_ANSI_ONLY

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


/******** returns difference TickEnd - TickStart in milliseconds **********/
long InchiTimeMsecDiff( e_inchiTime *TickEnd, e_inchiTime *TickStart )
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
long InchiTimeElapsed( e_inchiTime *TickStart )
{
    e_inchiTime TickEnd;
    if ( !TickStart )
        return 0;
    e_inchiTimeGet( &TickEnd );
    return InchiTimeMsecDiff( &TickEnd, TickStart );
}
/******************* add number of milliseconds to time *********************/
void InchiTimeAddMsec( e_inchiTime *TickEnd, unsigned long nNumMsec )
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
int bInchiTimeIsOver( e_inchiTime *TickStart )
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
void e_inchiTimeGet( e_inchiTime *TickEnd )
{
    if ( TickEnd ) {
        struct _timeb timeb;
        _ftime( &timeb );
        TickEnd->clockTime = (unsigned long)timeb.time;
        TickEnd->millitime = (long)timeb.millitm;
    }
}
/******** returns difference TickEnd - TickStart in milliseconds **********/
long InchiTimeMsecDiff( e_inchiTime *TickEnd, e_inchiTime *TickStart )
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
long InchiTimeElapsed( e_inchiTime *TickStart )
{
    e_inchiTime TickEnd;
    if ( !TickStart )
        return 0;
    e_inchiTimeGet( &TickEnd );
    return InchiTimeMsecDiff( &TickEnd, TickStart );
}
/******************* add number of milliseconds to time *********************/
void InchiTimeAddMsec( e_inchiTime *TickEnd, unsigned long nNumMsec )
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
int bInchiTimeIsOver( e_inchiTime *TickEnd )
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




/******** returns difference TickEnd - TickStart in milliseconds **********/
long e_inchiTimeMsecDiff( e_inchiTime *TickEnd, e_inchiTime *TickStart )
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
long e_inchiTimeElapsed( e_inchiTime *TickStart )
{
    e_inchiTime TickEnd;
    if ( !TickStart )
        return 0;
    e_inchiTimeGet( &TickEnd );
    return e_inchiTimeMsecDiff( &TickEnd, TickStart );
}
/******************* add number of milliseconds to time *********************/
void e_inchiTimeAddMsec( e_inchiTime *TickEnd, unsigned long nNumMsec )
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


/******** get current process time ****************************************/
void e_inchiTimeGet( e_inchiTime *TickEnd )
{
    TickEnd->clockTime = InchiClock();
}



/*
	For compatibility: local implementation of non-ANSI (MS-specific) functions, prefixed with "inchi_"
*/
#define __MYTOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') : (c) )
int e_inchi_memicmp( const void * p1, const void * p2, size_t length )
{
    const U_CHAR *s1 = (const U_CHAR*)p1;
    const U_CHAR *s2  = (const U_CHAR*)p2;
    while ( length-- )
    {
        if ( *s1 == *s2 ||
              __MYTOLOWER( (int)*s1 ) == __MYTOLOWER( (int)*s2 ))
        {
            s1 ++;
            s2  ++;
        }
        else
        {
            return
                __MYTOLOWER( (int)*s1 ) - __MYTOLOWER( (int)*s2 );
        }
    }

    return 0;
}
int e_inchi_stricmp( const char *s1, const char *s2 )
{
    while ( *s1 )
    {
        if ( *s1 == *s2 ||
              __MYTOLOWER( (int)*s1 ) == __MYTOLOWER( (int)*s2 ))
        {
            s1 ++;
            s2  ++;
        }
        else
        {
            return
                __MYTOLOWER( (int)*s1 ) - __MYTOLOWER( (int)*s2 );
        }
    }

    if ( *s2 )
        return -1;

    return 0;
}

#undef __MYTOLOWER
/*
	End of local implementation of non-ANSI (MS-specific) functions
*/
