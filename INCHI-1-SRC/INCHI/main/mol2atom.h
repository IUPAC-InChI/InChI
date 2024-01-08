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


#ifndef __MOL2ATOM_H__
#define __MOL2ATOM_H__

#include "readmol.h"

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

void calculate_valences (MOL_DATA* mol_data, inp_ATOM* at, int *num_atoms, int bDoNotAddH, int *err, char *pStrErr);
/* void WriteCoord( char *str, double x );*/

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif  /* __MOL2ATOM_H__ */
