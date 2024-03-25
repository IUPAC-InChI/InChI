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


#ifndef __UTIL_H__
#define __UTIL_H__

/* BILLY 8/6/04 */
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

    int   e_extract_ChargeRadical( char *elname, int *pnRadical, int *pnCharge );
    int   e_normalize_name( char* name );

    int   e_mystrncpy( char *target, const char *source, unsigned maxlen );
    char *e_LtrimRtrim( char *p, int* nLen );
    void  e_remove_trailing_spaces( char* p );
    void  e_remove_one_lf( char* p );
    int   e_is_element_a_metal( char szEl[] );

    AT_NUMB *e_is_in_the_list( AT_NUMB *pathAtom, AT_NUMB nNextAtom, int nPathLen );
    S_SHORT *e_is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen );
    void     e_SplitTime( unsigned long ulTotalTime, int *hours, int *minutes, int *seconds, int *mseconds );

    int e_inchi_memicmp( const void * p1, const void * p2, size_t length );
    int e_inchi_stricmp( const char *s1, const char *s2 );

    /* allocator */
#ifndef e_inchi_malloc
    void *e_inchi_malloc( size_t c );
#endif
#ifndef e_inchi_calloc
    void *e_inchi_calloc( size_t c, size_t n );
#endif
#ifndef e_inchi_free
    void e_inchi_free( void *p );
#endif


    extern char e_gsMissing[];
    extern char e_gsEmpty[];
    extern char e_gsSpace[];
    extern char e_gsEqual[];
    /* format string for SDF_LBL_VAL(L,V): %s%s%s%s (four strings) */
#define SDF_LBL_VAL(L,V)  ((L)&&(L)[0])?e_gsSpace:e_gsEmpty, ((L)&&(L)[0])?L:e_gsEmpty, ((L)&&(L)[0])? (((V)&&(V)[0])?e_gsEqual:e_gsSpace):e_gsEmpty, ((V)&&(V)[0])?V:((L)&&(L)[0])?e_gsMissing:e_gsEmpty

#ifndef INCHI_BUILD_PLATFORM

#if defined(_WIN32)

#if defined(_WIN64)
#define INCHI_BUILD_PLATFORM "Windows 64-bit"
#else
#define INCHI_BUILD_PLATFORM "Windows 32-bit"
#endif

#elif defined(__linux__)

#if defined(__x86_64__)||defined(__ppc64__)
#define INCHI_BUILD_PLATFORM "Linux 64-bit"
#else
#define INCHI_BUILD_PLATFORM "Linux 32-bit"
#endif

#elif defined(__APPLE__)
#define INCHI_BUILD_PLATFORM "OSX"

#else
#define INCHI_BUILD_PLATFORM ""
#endif
#endif

#ifndef INCHI_BUILD_DEBUG
#if defined(_DEBUG)
#define INCHI_BUILD_DEBUG " Debug"
#else
#define INCHI_BUILD_DEBUG ""
#endif
#endif

#ifndef INCHI_SRC_REV
#if defined(_DEBUG)
#define INCHI_SRC_REV "rev. 9b6f1414ebf3+"
#else
#define INCHI_SRC_REV ""
#endif
#endif

#ifndef INCHI_BUILD_COMPILER

#if defined(_MSC_VER)

#if _MSC_VER > 1900
#define INCHI_BUILD_COMPILER "MS VS 2017 or later"
#elif _MSC_VER == 1900
#define INCHI_BUILD_COMPILER "MS VS 2015"
#elif _MSC_VER == 1800
#define INCHI_BUILD_COMPILER "MS VS 2013"
#elif _MSC_VER == 1700
#define INCHI_BUILD_COMPILER "MS VS 2012"
#elif _MSC_VER == 1600
#define INCHI_BUILD_COMPILER "MS VS 2010"
#elif _MSC_VER == 1500
#define INCHI_BUILD_COMPILER "MS VS 2008"
#elif _MSC_VER == 1400
#define INCHI_BUILD_COMPILER "MS VS 2005"
#elif _MSC_VER == 1310
#define INCHI_BUILD_COMPILER "MS VS 2003"
#elif _MSC_VER == 1300
#define INCHI_BUILD_COMPILER "MS VS 2002"
#elif _MSC_VER == 1200
#define INCHI_BUILD_COMPILER "MS VS 6.0"
#else
#define INCHI_BUILD_COMPILER "MS VC++ 5.0 or earlier"
#endif

#else

#if defined( __GNUC__)
#define INCHI_BUILD_COMPILER "gcc " __VERSION__ ""
#else
#define INCHI_BUILD_COMPILER ""
#endif
#endif

#endif

/* BILLY 8/6/04 */
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* __UTIL_H__*/
