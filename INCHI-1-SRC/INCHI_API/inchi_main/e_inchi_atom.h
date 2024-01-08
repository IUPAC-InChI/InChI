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


#ifndef __INCHI_ATOM_H__
#define __INCHI_ATOM_H__

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

void           e_FreeInchi_Atom( inchi_Atom **at );
void           e_FreeInchi_Stereo0D( inchi_Stereo0D **stereo0D );
inchi_Atom     *e_CreateInchi_Atom( int num_atoms );
inchi_Stereo0D *e_CreateInchi_Stereo0D( int num_stereo0D );
void           e_FreeInchi_Input( inchi_Input *inp_at_data );
int            e_RemoveRedundantNeighbors( inchi_Input *inp_at_data );

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __INCHI_ATOM_H__ */
