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
 * http://www.opensource.org/licenses/lgpl-license.php
 */


#ifndef __READ_CML_HPP__
#define __READ_CML_HPP__

#ifdef __cplusplus
#include <cstdio>
#else
#include "stdio.h"
#endif


#include "mode.h"
#include "inpdef.h"

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C"
{
#endif
#endif

int CmlfileToOrigAtom( FILE *inp_molfile, ORIG_ATOM_DATA *orig_at_data, int bMergeAllInputStructures,
                       int bGetOrigCoord, int bDoNotAddH, int inp_index, int *out_index,
                       char *pSdfLabel, char *pSdfValue, long *lSdfId, int *err, char *pStrErr );
int CmlfileToInpAtom( FILE *inp_cmlfile, int bDoNotAddH, inp_ATOM **at, MOL_COORD **szCoord,
                      int max_num_at, int *num_dimensions, int *num_bonds, int inp_index, int *out_index,
                      char *pSdfLabel, char *pSdfValue, long *Id, int *err, char *pStrErr );

void FreeCml ( void );
int FreeCmlDoc( int n );
int GetCmlStructIndex( void );
int SetCmlStructIndex( int index );

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif	/* __READ_CML_HPP__ */
