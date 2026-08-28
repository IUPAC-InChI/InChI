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

#include <stdlib.h>
#include <string.h>

#include "mode.h"
#include "aromaticity.h"
#include "ichinorm.h"  /* declares mark_alt_bonds_and_taut_groups + engine types */
#include "util.h"      /* is_in_the_list */
#include "ichierr.h"

#include "bcf_s.h"


/****************************************************************************/
int fix_aromatic_oxygen_and_sulfur( inp_ATOM *atom )
{
    if (!atom->elname[1] &&
         (atom->elname[0] == 'O' || atom->elname[0] == 'S') &&
         atom->valence == 2 && !atom->charge && !atom->radical &&
         atom->bond_type[0] + atom->bond_type[1] == 3)
    {
        atom->charge = 1;
        return 1; /* fixed */
    }

    return 0;
}


/****************************************************************************/
int huckel_pi_contribution( inp_ATOM *at, int i )
{
    if (at[i].charge > 0)
    {
        return 0; /* cationic center: empty p orbital */
    }
    if (at[i].charge < 0)
    {
        return 2; /* anionic center: in-ring lone pair */
    }
    if (at[i].radical == RADICAL_DOUBLET)
    {
        return 1; /* radical center: singly occupied MO */
    }
    return 1;     /* ordinary conjugated atom: one ring double bond */
}


/****************************************************************************/
/* Number of atom i's neighbors that are metal atoms. */
static int count_metal_neighbors( inp_ATOM *at, int i )
{
    int k, n = 0;
    for (k = 0; k < at[i].valence; k++)
    {
        if (is_el_a_metal( at[at[i].neighbor[k]].el_number ))
        {
            n++;
        }
    }
    return n;
}

/****************************************************************************/
int is_aromatic_electron_source( inp_ATOM *at, int i )
{
    int ring_coordination = (int) at[i].valence - count_metal_neighbors( at, i );

    return ring_coordination == 2 &&
           at[i].num_H == 0 &&
           at[i].chem_bonds_valence > at[i].valence &&
           ( at[i].charge != 0 || at[i].radical == RADICAL_DOUBLET );
}


/****************************************************************************/
int relax_aromatic_electron_sources( inp_ATOM *at, int num_atoms )
{
    int i, num_relaxed = 0;
    for (i = 0; i < num_atoms; i++)
    {
        if (is_aromatic_electron_source( at, i ))
        {
            at[i].chem_bonds_valence--;
            at[i].num_H++;
            num_relaxed++;
        }
    }
    return num_relaxed;
}


/****************************************************************************/
int check_arom_chain( inp_ATOM *at,
                      int cur /* first*/,
                      int from,
                      int last,
                      int len )
{
    int i, num;
    num = 0;
    do
    {
        /* check this on all except at[last], which is typically different */
        if (at[cur].el_number != EL_NUMBER_C ||
             at[cur].valence != 2 ||
             at[cur].chem_bonds_valence != 3 ||
             at[cur].num_H != 1)
        {
            goto check_next_derivative;
        }
        /* bond to the next atom - check on all, cur..last, atoms */
        i = ( at[cur].neighbor[0] == from ); /* index of a bond to the next atom */
        if (at[cur].bond_type[i] != BOND_ALTERN)
        {
            goto check_next_derivative;
        }
        num++; /* checks are complete */
               /* prepare for the next atom */
        from = cur;
        cur = at[cur].neighbor[i];
    } while (cur != last && num < len);

    return ( cur == last && ++num == len );

check_next_derivative:

    return 0;
}


#if ( RING2CHAIN == 1 || UNDERIVATIZE == 1 )
/****************************************************************************/
int mark_arom_bonds( struct tagINCHI_CLOCK *ic, struct tagCANON_GLOBALS *pCG, inp_ATOM *at, int num_atoms )
{
    INCHI_MODE bTautFlags = 0, bTautFlagsDone = 0;
    inp_ATOM *at_fixed_bonds_out = NULL;
    T_GROUP_INFO *t_group_info = NULL;
    int ret;

    ret = mark_alt_bonds_and_taut_groups( ic, pCG, at, at_fixed_bonds_out, num_atoms,
                                          NULL,
                                          t_group_info, &bTautFlags, &bTautFlagsDone, 0, NULL );

    return ret;
}


/****************************************************************************/
int is_C_unsat_not_arom( inp_ATOM *at, int i )
{
    int j, neigh, num_arom, num_DB;
    if (at[i].el_number != EL_NUMBER_C ||
         at[i].valence == at[i].chem_bonds_valence || /* no double/triple bonds */
         at[i].valence + 1 < at[i].chem_bonds_valence || /* >1 double bond or >=1 triple bond */
         at[i].chem_bonds_valence + at[i].num_H != 4 || /* C has wrong valence */
         at[i].charge || at[i].radical)
        return 0;
    num_arom = num_DB = 0;
    for (j = 0; j < at[i].valence; j++)
    {
        neigh = at[i].neighbor[j];
        num_arom += at[i].bond_type[j] == BOND_TYPE_ALTERN;
        if (( at[neigh].el_number == EL_NUMBER_O ||
              at[neigh].el_number == EL_NUMBER_S ) &&
             !at[neigh].num_H && 1 == at[neigh].valence &&
             2 == at[neigh].chem_bonds_valence)
        {
            continue; /* do not count double bonds to terminal =O or =S */
        }
        num_DB += at[i].bond_type[j] == BOND_TYPE_DOUBLE;
    }

    return num_DB && !num_arom;
}


/****************************************************************************/
int is_Aryl( inp_ATOM *at, int outside_point, int attachment_pont )
{
    int i, num_arom_bonds, neigh;
    if (at[attachment_pont].el_number == EL_NUMBER_C &&
         at[attachment_pont].valence == 3 && at[attachment_pont].chem_bonds_valence == 4 &&
         !at[attachment_pont].num_H && !at[attachment_pont].charge && !at[attachment_pont].radical)
    {
        for (i = 0, num_arom_bonds = 0; i < at[attachment_pont].valence; i++)
        {
            neigh = at[attachment_pont].neighbor[i];
            if (neigh != outside_point)
            {
                num_arom_bonds += ( at[attachment_pont].bond_type[i] == BOND_ALTERN &&
                    ( at[neigh].el_number == EL_NUMBER_C || at[neigh].el_number == EL_NUMBER_N ) );
            }
        }
        return ( num_arom_bonds == 2 );
    }

    return 0;
}
#endif /* ( RING2CHAIN == 1 || UNDERIVATIZE == 1 ) */


#ifdef FIX_UNDERIV_TO_SDF
/****************************************************************************/
int replace_arom_bonds( inp_ATOM *at,
                        int num_atoms,
                        inp_ATOM *at2,
                        int num_atoms2 )
{
    int i, j, num_err = 0;

    for (i = 0; i < num_atoms; i++)
    {
        for (j = 0; j < at[i].valence; j++)
        {
            if (at[i].bond_type[j] > BOND_TRIPLE)
            {
                /* find pairs of atoms using orig. atom numbers */
                int i1, i2;
                char bSuccess = 0;
                int neigh = at[i].neighbor[j];
                AT_NUMB orig_no1 = at[i].orig_at_number;
                AT_NUMB orig_no2 = at[neigh].orig_at_number;
                for (i1 = 0; i1 < num_atoms2 && at2[i1].orig_at_number != orig_no1; i1++)
                {
                    ;
                }
                for (i2 = 0; i2 < num_atoms2 && at2[i2].orig_at_number != orig_no2; i2++)
                {
                    ;
                }
                if (i1 < num_atoms2 && i2 < num_atoms2)
                {
                    AT_NUMB *p1 = is_in_the_list( at2[i1].neighbor, (AT_NUMB) i2, at2[i1].valence );
                    AT_NUMB *pneigh = is_in_the_list( at[neigh].neighbor, (AT_NUMB) i, at[neigh].valence );
                    if (p1 && pneigh)
                    {
                        int n1 = p1 - at2[i1].neighbor;
                        int nneigh = pneigh - at[neigh].neighbor;
                        at[i].bond_type[j] = at[neigh].bond_type[nneigh] = at2[i1].bond_type[n1];
                        bSuccess = 1;
                    }
                }
                if (!bSuccess)
                {
#ifdef _DEBUG
                    int stop_here = 1;
#endif
                    num_err++;
                }
            }
        }
    }

    return num_err;
}
#endif /* FIX_UNDERIV_TO_SDF */
