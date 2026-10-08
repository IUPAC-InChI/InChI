/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

#ifndef _PERMUTATION_UTIL_H_
#define _PERMUTATION_UTIL_H_

/* Renumbering atoms stuff */
#define PERMAXATOMS 32767
#define BYTE(X) ((unsigned char *)(X))
int rrand( int m );
void shuffle( void *obj, size_t nmemb, size_t size );
void OrigAtData_Permute( ORIG_ATOM_DATA *permuted, ORIG_ATOM_DATA *saved, int *numbers );
EXPIMP_TEMPLATE INCHI_API int INCHI_DECL PermuteMolfileText(
    const char *moltext,
    char *permuted_moltext,
    size_t permuted_moltext_len);

#endif // _PERMUTATION_UTIL_H_
