/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_mode.h"

#include "../../../../INCHI_BASE/src/inchi_api.h"
#include "../../../../INCHI_BASE/src/bcf_s.h"

#include "e_inchi_atom.h"
#include "e_ichisize.h"
#include "e_util.h"



/******************************************************************************************************/
void e_FreeInchi_Atom( inchi_Atom **at )
{
    if (at && *at)
    {
        e_inchi_free( *at );
        *at = NULL;
    }
}
/******************************************************************************************************/
void e_FreeInchi_Stereo0D( inchi_Stereo0D **stereo0D )
{
    if (stereo0D && *stereo0D)
    {
        e_inchi_free( *stereo0D );
        *stereo0D = NULL;
    }
}
/******************************************************************************************************/
inchi_Atom *e_CreateInchi_Atom( int num_atoms )
{
    inchi_Atom *p = (inchi_Atom*) e_inchi_calloc( num_atoms, sizeof( inchi_Atom ) );
    return p;
}
/******************************************************************************************************/
inchi_Stereo0D *e_CreateInchi_Stereo0D( int num_stereo0D )
{
    return (inchi_Stereo0D*) e_inchi_calloc( num_stereo0D, sizeof( inchi_Stereo0D ) );
}



/******************************************************************************************************/
void e_FreeInchi_Input( inchi_InputEx *inp_at_data )
{
    inchi_Input_Polymer *polymer = inp_at_data->polymer;
    inchi_Input_V3000 *v3000 = inp_at_data->v3000;
    int k;

    e_FreeInchi_Atom( &inp_at_data->atom );

    e_FreeInchi_Stereo0D( &inp_at_data->stereo0D );

    if (polymer)
    {
        if (polymer->n && polymer->units)
        {
            for (k = 0; k < polymer->n; k++)
            {
                if (polymer->units[k])
                {
                    if (polymer->units[k]->alist)
                    {
                        inchi_free(polymer->units[k]->alist);  polymer->units[k]->alist = NULL;
                    }
                    if (polymer->units[k]->blist)
                    {
                        inchi_free(polymer->units[k]->blist);  polymer->units[k]->blist = NULL;
                    }
                }
                inchi_free(polymer->units[k]);
            }
            inchi_free(polymer->units);
            polymer->units = NULL;
            inchi_free(polymer);
        }
    }

    if (v3000)
    {
        if (v3000->atom_index_orig)
        {
            inchi_free(v3000->atom_index_orig);
            v3000->atom_index_orig = NULL;
        }
        if (v3000->atom_index_fin)
        {
            inchi_free(v3000->atom_index_fin);
            v3000->atom_index_fin = NULL;
        }
        if (v3000->n_haptic_bonds && v3000->lists_haptic_bonds)
        {
            for (k = 0; k < v3000->n_haptic_bonds; k++)
            {
                if (v3000->lists_haptic_bonds[k])
                {
                    inchi_free(v3000->lists_haptic_bonds[k]);
                    v3000->lists_haptic_bonds[k] = NULL;
                }
            }
            inchi_free(v3000->lists_haptic_bonds);
            v3000->lists_haptic_bonds = NULL;
        }
        if (v3000->n_steabs && v3000->lists_steabs)
        {
            for (k = 0; k < v3000->n_steabs; k++)
            {
                if (v3000->lists_steabs[k])
                {
                    inchi_free(v3000->lists_steabs[k]);
                    v3000->lists_steabs[k] = NULL;
                }
            }
            inchi_free(v3000->lists_steabs);
            v3000->lists_steabs = NULL;
        }
        if (v3000->n_sterel && v3000->lists_sterel)
        {
            for (k = 0; k < v3000->n_sterel; k++)
            {
                if (v3000->lists_sterel[k])
                {
                    inchi_free(v3000->lists_sterel[k]);
                    v3000->lists_sterel[k] = NULL;
                }
            }
            inchi_free(v3000->lists_sterel);
            v3000->lists_sterel = NULL;
        }
        if (v3000->n_sterac && v3000->lists_sterac)
        {
            for (k = 0; k < v3000->n_sterac; k++)
            {
                if (v3000->lists_sterac[k])
                {
                    inchi_free(v3000->lists_sterac[k]);
                    v3000->lists_sterac[k] = NULL;
                }
            }
            inchi_free(v3000->lists_sterac);
            v3000->lists_sterac = NULL;
        }
        inchi_free(v3000);
    }

    memset( inp_at_data, 0, sizeof( *inp_at_data ) ); /* djb-rwth: memset_s C11/Annex K variant? */
}


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* 	e_RemoveRedundantNeighbors: after calling it, the 'neighbor' in atomic
    data for any atom 'i' will contain only atoms whose numbers < i				*/
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
int e_RemoveRedundantNeighbors( inchi_Input *inp_at_data )
{
    int i, j, k;
    inchi_Atom *at = inp_at_data->atom;
    int         num_atoms = inp_at_data->num_atoms;
    for (i = 0; i < num_atoms; i++)
    {
        for (j = k = 0; j < at[i].num_bonds; j++)
        {
            if (at[i].neighbor[j] < i)
            {
                at[i].neighbor[k] = at[i].neighbor[j];
                at[i].bond_type[k] = at[i].bond_type[j];
                at[i].bond_stereo[k] = at[i].bond_stereo[j];
                k++;
            }
        }
        for (j = k; j < at[i].num_bonds; j++)
        {
            at[i].neighbor[j] = 0;
            at[i].bond_type[j] = 0;
            at[i].bond_stereo[j] = 0;
        }
        at[i].num_bonds = k;
    }
    return 0;
}
