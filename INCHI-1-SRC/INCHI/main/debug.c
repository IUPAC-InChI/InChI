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


#include "mode.h"

#ifdef CML_DEBUG

#include "debug.h"

void PrintInpAtom (FILE *file_p, const inp_ATOM * const at_p, const unsigned int num_atoms)
{
    unsigned int i, j;
    const AT_NUMB *at_num_p;
    const U_CHAR *u_char_p;
    const S_CHAR *s_char_p;
    const char *char_p;

    const inp_ATOM *atom_p  = at_p;

    for (j = 0; j < num_atoms; ++ j, ++ atom_p)
        {

            fprintf (file_p, "\n**********************************\n");

            if (!atom_p)
                {
                    fprintf (file_p, "inp_Atom is NULL.");
                    return;
                }

            fprintf (file_p, "elname: %s\n", atom_p -> elname);
            fprintf (file_p, "el_number: %d\n", (int) (atom_p -> el_number));

            fprintf (file_p, "neighbor:\n");
            at_num_p = atom_p -> neighbor;

            for (i = 0; i < MAXVAL; ++ i, ++ at_num_p)
                {
                    fprintf (file_p, "%d ", (int) (*at_num_p));
                }

            fprintf (file_p, "\norig_at_number: %d\n", (int) (atom_p -> orig_at_number));
            fprintf (file_p, "orig_compt_at_numb: %d\n", (int) (atom_p -> orig_compt_at_numb));

            fprintf (file_p, "bond_stereo:\n");
            s_char_p = atom_p -> bond_stereo;
            for (i = 0; i < MAXVAL; ++ i, ++ s_char_p)
                {
                    fprintf (file_p, "%d ", (int) (*s_char_p));
                }

            fprintf (file_p, "\nbond_type:\n");
            u_char_p = atom_p -> bond_type;
            for (i = 0; i < MAXVAL; ++ i, ++ u_char_p)
                {
                    fprintf (file_p, "%d ", (int) (*u_char_p));
                }

            fprintf (file_p, "\nvalence: %d\n", (int) (atom_p -> valence));
            fprintf (file_p, "chem_bonds_valence: %d\n", (int) (atom_p -> chem_bonds_valence));
            fprintf (file_p, "num_H: %d\n", (int) (atom_p -> num_H));

            fprintf (file_p, "num_iso_H:\n");
            char_p = atom_p -> num_iso_H;
            for (i = 0; i < NUM_H_ISOTOPES; ++ i, ++ char_p)
                {
                    fprintf (file_p, "%d ", (int) (*char_p));
                }

            fprintf (file_p, "\niso_atw_diff: %d\n", (int) (atom_p -> iso_atw_diff));
            fprintf (file_p, "charge: %d\n", (int) (atom_p -> charge));
            fprintf (file_p, "radical: %d\n", (int) (atom_p -> radical));
            fprintf (file_p, "bAmbiguousStereo: %d\n", (int) (atom_p -> bAmbiguousStereo));
            fprintf (file_p, "cFlags: %d\n", (int) (atom_p -> cFlags));
            fprintf (file_p, "cInputParity: %d\n", (int) (atom_p -> p_parity));
            fprintf (file_p, "at_type: %d\n", (int) (atom_p -> at_type));
            fprintf (file_p, "component: %d\n", (int) (atom_p -> component));
            fprintf (file_p, "endpoint: %d\n", (int) (atom_p -> endpoint));
            fprintf (file_p, "c_point: %d\n", (int) (atom_p -> c_point));

            fprintf (file_p, "x: %lf\n", (atom_p -> x));
            fprintf (file_p, "y: %lf\n", (atom_p -> y));
            fprintf (file_p, "z: %lf\n", (atom_p -> z));

        #if( FIND_RING_SYSTEMS == 1 )
            fprintf (file_p, "bCutVertex: %d\n", (int) (atom_p -> bCutVertex));
            fprintf (file_p, "nRingSystem: %d\n", (int) (atom_p -> nRingSystem));
            fprintf (file_p, "nNumAtInRingSystem: %d\n", (int) (atom_p -> nNumAtInRingSystem));
            fprintf (file_p, "nBlockSystem: %d\n", (int) (atom_p -> nBlockSystem));
        #endif

        #if( FIND_RINS_SYSTEMS_DISTANCES == 1 )
            fprintf (file_p, "nDistanceFromTerminal: %d\n", (int) (atom_p -> nDistanceFromTerminal));
        #endif

        }
}

#else
int cml_debug; /* keep compiler happy */
#endif /* CML_DEBUG */
