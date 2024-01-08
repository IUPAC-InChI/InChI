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


#ifndef __UTIL_H__
#define __UTIL_H__

/* BILLY 8/6/04 */
#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

int   e_extract_ChargeRadical( char *elname, int *pnRadical, int *pnCharge );
int   e_normalize_name( char* name );

int   e_mystrncpy(char *target,const char *source,unsigned maxlen);
char *e_LtrimRtrim( char *p, int* nLen );
void  e_remove_trailing_spaces( char* p );
void  e_remove_one_lf( char* p);
int   e_is_element_a_metal( char szEl[] );

AT_NUMB *e_is_in_the_list( AT_NUMB *pathAtom, AT_NUMB nNextAtom, int nPathLen );
S_SHORT *e_is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen );
void     e_SplitTime( unsigned long ulTotalTime, int *hours, int *minutes, int *seconds, int *mseconds );

/* allocator */
#ifndef e_inchi_malloc
void *e_inchi_malloc(size_t c);
#endif
#ifndef e_inchi_calloc
void *e_inchi_calloc(size_t c, size_t n);
#endif
#ifndef e_inchi_free
void e_inchi_free(void *p);
#endif


extern char e_gsMissing[];
extern char e_gsEmpty[];
extern char e_gsSpace[];
extern char e_gsEqual[];
/* format string for SDF_LBL_VAL(L,V): %s%s%s%s (four strings) */
#define SDF_LBL_VAL(L,V)  ((L)&&(L)[0])?e_gsSpace:e_gsEmpty, ((L)&&(L)[0])?L:e_gsEmpty, ((L)&&(L)[0])? (((V)&&(V)[0])?e_gsEqual:e_gsSpace):e_gsEmpty, ((V)&&(V)[0])?V:((L)&&(L)[0])?e_gsMissing:e_gsEmpty


/* BILLY 8/6/04 */
#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* __UTIL_H__*/

