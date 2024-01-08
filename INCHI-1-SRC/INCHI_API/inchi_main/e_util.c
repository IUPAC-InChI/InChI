/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.03
 * May 9, 2010
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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "e_mode.h"
#include "e_ichisize.h"
#include "inchi_api.h"


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

#ifdef INCHI_ANSI_ONLY
/*************************************************************************/
/*************          non-ANSI functions                ****************/
/*************************************************************************/
#define __MYTOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') : (c) )

#if ( defined(ADD_NON_ANSI_FUNCTIONS) || defined(__STDC__) && __STDC__ == 1 )
/* support (VC++ Language extensions) = OFF && defined(INCHI_ANSI_ONLY) */
#ifdef INCHI_LINK_AS_DLL
/* the following code is enabled if linked as dll (ANSI C)                      */
/* because the InChI library dll does not export it                             */
int memicmp ( const void * p1, const void * p2, size_t length )
{
    const U_CHAR *s1 = (const U_CHAR*)p1;
    const U_CHAR *s2  = (const U_CHAR*)p2;
    while ( length-- ) {
        if ( *s1 == *s2 ||
              __MYTOLOWER( (int)*s1 ) == __MYTOLOWER( (int)*s2 )) {
            s1 ++;
            s2  ++;
        } else {
            return __MYTOLOWER( (int)*s1 ) - __MYTOLOWER( (int)*s2 );
        }
    }
    return 0;
}
/*************************************************************************/
int stricmp( const char *s1, const char *s2 )
{
    while ( *s1 ) {
        if ( *s1 == *s2 ||
              __MYTOLOWER( (int)*s1 ) == __MYTOLOWER( (int)*s2 )) {
            s1 ++;
            s2 ++;
        } else {
            return __MYTOLOWER( (int)*s1 ) - __MYTOLOWER( (int)*s2 );
        }
    }
    if ( *s2 )
        return -1;
    return 0;
}
/*************************************************************************/
char *_strnset( char *s, int val, size_t length )
{
    char *ps = s;
    while (length-- && *ps)
        *ps++ = (char)val;
    return s;
}
/*************************************************************************/
char *_strdup( const char *string )
{
    char *p = NULL;
    if ( string ) {
        size_t length = strlen( string );
        p = (char *)e_inchi_malloc( length + 1 );
        if ( p ) {
            strcpy( p, string );
        }
    }
    return p;
}
#endif /* INCHI_LINK_AS_DLL */
#endif /* !defined(_MSC_VER) || defined(__STDC__) && __STDC__ == 1 */
#endif /* INCHI_ANSI_ONLY */

