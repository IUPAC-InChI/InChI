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
#include <stdlib.h>
#include <string.h>

#include "e_mode.h"

#include "../../../../INCHI_BASE/src/inchi_api.h"

#include "e_inchi_atom.h"
#include "e_ichisize.h"
#include "e_util.h"



/******************************************************************************************************/
void e_FreeInchi_Atom( inchi_Atom **at )
{
    if ( at && *at ) {
        e_inchi_free( *at );
        *at = NULL;
    }
}
/******************************************************************************************************/
void e_FreeInchi_Stereo0D( inchi_Stereo0D **stereo0D )
{
    if ( stereo0D && *stereo0D ) {
        e_inchi_free( *stereo0D );
        *stereo0D = NULL;
    }
}
/******************************************************************************************************/
inchi_Atom *e_CreateInchi_Atom( int num_atoms )
{
   inchi_Atom *p = (inchi_Atom* ) e_inchi_calloc(num_atoms, sizeof(inchi_Atom) );
   return p;
}
/******************************************************************************************************/
inchi_Stereo0D *e_CreateInchi_Stereo0D( int num_stereo0D )
{
   return (inchi_Stereo0D* ) e_inchi_calloc(num_stereo0D, sizeof(inchi_Stereo0D) );
}



/******************************************************************************************************/
void e_FreeInchi_Input( inchi_Input *inp_at_data )
{
    e_FreeInchi_Atom( &inp_at_data->atom );
    e_FreeInchi_Stereo0D( &inp_at_data->stereo0D );
    memset( inp_at_data, 0, sizeof(*inp_at_data) );
}



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* 	e_RemoveRedundantNeighbors: after calling it, the 'neighbor' in atomic
    data for any atom 'i' will contain only atoms whose numbers < i				*/
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
int e_RemoveRedundantNeighbors( inchi_Input *inp_at_data )
{
    int i, j, k;
    inchi_Atom *at        = inp_at_data->atom;
    int         num_atoms = inp_at_data->num_atoms;
    for ( i = 0; i < num_atoms; i ++ ) {
        for ( j = k = 0; j < at[i].num_bonds; j ++ ) {
            if (  at[i].neighbor[j] < i ) {
                at[i].neighbor[k]    = at[i].neighbor[j];
                at[i].bond_type[k]   = at[i].bond_type[j];
                at[i].bond_stereo[k] = at[i].bond_stereo[j];
                k ++;
            }
        }
        for ( j = k; j < at[i].num_bonds; j ++ ) {
            at[i].neighbor[j]    = 0;
            at[i].bond_type[j]   = 0;
            at[i].bond_stereo[j] = 0;
        }
        at[i].num_bonds = k;
    }
    return 0;
}
