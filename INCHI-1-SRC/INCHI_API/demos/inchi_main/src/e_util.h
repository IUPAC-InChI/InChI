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

int   e_mystrncpy(char *target,const char *source,unsigned maxlen);
char *e_LtrimRtrim( char *p, int* nLen );
void  e_remove_trailing_spaces( char* p );
void  e_remove_one_lf( char* p);
int   e_is_element_a_metal( char szEl[] );

AT_NUMB *e_is_in_the_list( AT_NUMB *pathAtom, AT_NUMB nNextAtom, int nPathLen );
S_SHORT *e_is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen );
void     e_SplitTime( unsigned long ulTotalTime, int *hours, int *minutes, int *seconds, int *mseconds );

int e_inchi_memicmp( const void * p1, const void * p2, size_t length );
int e_inchi_stricmp( const char *s1, const char *s2 );

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
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* __UTIL_H__*/
