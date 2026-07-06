

#include "inpdef.h"
#include "ring_detection.h"
#include "ichister.h"
#include "atropisomers.h"


int is_candidate_atrop_axis(const inp_ATOM *at,
                            int num_atoms,
                            const RingSystems *rs,
                            int a1, int a2)
{
    int k;
    /* bond a1-a2 must be a single bond */
    int found = 0, btype = 0;
    for (k = 0; k < at[a1].valence; k++) {
        if ((int)at[a1].neighbor[k] == a2) { found = 1; btype = at[a1].bond_type[k]; break; }
    }
    if (!found || btype != 1) {
        return 0;
    }
    /* both ends need >= 2 substituents besides the axis (valence >= 3) */
    if (at[a1].valence < 3 || at[a2].valence < 3) {
        return 0;
    }
    /* NEGATIVE filter: reject if the bond sits in a small (rotation-locked) ring.
       Never require a ring - acyclic axes are valid. */
    if (are_atoms_in_same_small_ring(at, num_atoms, rs, a1, a2,
                                     ATROP_MIN_ROTATABLE_RING)) {
        return 0;
    }
    return 1;
}

int find_atropisomeric_atoms_and_bonds(inp_ATOM* out_at,
                                       int num_atoms,
                                       RingSystems *ring_result,
                                       ORIG_ATOM_DATA *orig_inp_data) {

    int ret = 0;
    int i, j;

    if (out_at == NULL || num_atoms <= 0) {
        return ret;
    }

    if (ring_result == NULL || orig_inp_data == NULL) {
        return ret;
    }

    for (i = 0; i < num_atoms; i++) {
        for (j = 0; j < out_at[i].valence; j++) {
            int nb = (int)out_at[i].neighbor[j];
            if (i >= nb) {
                continue; /* consider each bond once */
            }
            if (is_candidate_atrop_axis(out_at, num_atoms, ring_result, i, nb)) {
                orig_inp_data->bAtropisomer = 1;
                out_at[i].bAtropisomeric = 1;
                out_at[nb].bAtropisomeric = 1;
                ret = 1;
            }
        }
    }

    return ret;
}
