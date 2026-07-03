

#include "inpdef.h"
#include "ring_detection.h"
#include "ichister.h"


int find_atropisomeric_atoms_and_bonds(inp_ATOM* out_at,
                                       int num_atoms,
                                       RingSystems *ring_result,
                                       ORIG_ATOM_DATA *orig_inp_data) {

    int ret = 0;

    if (out_at == NULL || num_atoms <= 0) {
        return ret;
    }

    if (ring_result == NULL || orig_inp_data == NULL) {
        return ret;
    }

    int *fused_atom_partner = (int *)inchi_malloc(num_atoms * sizeof(int));
    if (fused_atom_partner == NULL) {
        return ret;
    }
    for (int i = 0; i < num_atoms; i++) {
        fused_atom_partner[i] = -1;
    }
    for (int i = 0; i < num_atoms; i++) {
        for (int j = i + 1; j < num_atoms; j++) {
            if(is_fused_ring_pivot(ring_result, out_at, i, j)) {
                fused_atom_partner[i] = j;
                fused_atom_partner[j] = i;
            }
        }
    }

    for (int i = 0; i < num_atoms; i++) {

        int atom_id1 = i;
        const inp_ATOM atom_i = out_at[atom_id1];

        int num_neighbors_i = atom_i.valence;

        const AT_NUMB *neighbors = atom_i.neighbor;

        double at_coord[4][3];
        at_coord[0][0] = atom_i.x;
        at_coord[0][1] = atom_i.y;
        at_coord[0][2] = atom_i.z;

        if (num_neighbors_i < 3) {
            continue;
        }

        for (int j = 0; j < num_neighbors_i; j++) {
            int score = 0;

            int atom_id2 = neighbors[j];

            if (atom_id1 >= atom_id2) {
                continue;
            }

            const inp_ATOM atom_j = out_at[atom_id2];

            int num_neighbors_j = atom_j.valence;

            if (num_neighbors_j < 3) {
                continue;
            }

            score += (num_neighbors_j == 3);

            score += (atom_i.bond_stereo[j] == 0);
            score -= ((atom_i.bond_stereo[j] == STEREO_SNGL_UP) ||
                      (atom_i.bond_stereo[j] == STEREO_SNGL_EITHER) ||
                      (atom_i.bond_stereo[j] == STEREO_SNGL_DOWN)) * 2;

            score += (atom_i.bond_type[j] == 1);
            score -= (atom_i.bond_type[j] != 1) * 2;

            score += (ring_result->atom_to_ring_mapping[atom_id1].ring_count > 0) * 3;
            score += (ring_result->atom_to_ring_mapping[atom_id2].ring_count > 0) * 3;


            int nof_wedge_bonds_i = 0;
            int has_double_bond_i = 0;
            int coord_count = 1;
            for (int k = 0; k < num_neighbors_i; k++) {
                if (atom_i.bond_stereo[k] == 1 ||
                    atom_i.bond_stereo[k] == 4 ||
                    atom_i.bond_stereo[k] == 6) {
                    nof_wedge_bonds_i++;
                }
                if (atom_i.bond_type[k] == 2) {
                    has_double_bond_i = 1;
                }
                if (atom_id2 != atom_i.neighbor[k]) {
                    if (coord_count <= 2) {
                        at_coord[coord_count][0] = out_at[atom_i.neighbor[k]].x;
                        at_coord[coord_count][1] = out_at[atom_i.neighbor[k]].y;
                        at_coord[coord_count][2] = out_at[atom_i.neighbor[k]].z;
                        coord_count++;
                    }
                }
            }
            int nof_wedge_bonds_j = 0;
            int has_double_bond_j = 0;
            for (int k = 0; k < num_neighbors_j; k++) {
                if (atom_j.bond_stereo[k] == 1 ||
                    atom_j.bond_stereo[k] == 4 ||
                    atom_j.bond_stereo[k] == 6) {
                    nof_wedge_bonds_j++;
                }
                if (atom_j.bond_type[k] == 2) {
                    has_double_bond_j = 1;
                }
                if (atom_id1 != atom_j.neighbor[k]) {
                    /* Use the first qualifying substituent so the planarity
                       test input is deterministic (was overwritten each pass,
                       keeping only the last neighbor). */
                    at_coord[3][0] = out_at[atom_j.neighbor[k]].x;
                    at_coord[3][1] = out_at[atom_j.neighbor[k]].y;
                    at_coord[3][2] = out_at[atom_j.neighbor[k]].z;
                    break;
                }
            }

            score += (nof_wedge_bonds_i > 0) * 4;
            score += (nof_wedge_bonds_j > 0) * 4;

            score -= (nof_wedge_bonds_i == 0) * 3;
            score -= (nof_wedge_bonds_j == 0) * 3;

            score += has_double_bond_i * 2;
            score += has_double_bond_j * 2;

            score += (fused_atom_partner[atom_id1] != atom_id2 && fused_atom_partner[atom_id2] != i);
            score += (fused_atom_partner[atom_id1] == -1 || fused_atom_partner[atom_id2] == -1);

            int both_atoms_in_same_small_ring = are_atoms_in_same_small_ring(out_at,
                                                                        num_atoms,
                                                                        ring_result,
                                                                        atom_id1, atom_id2,
                                                                        6);

            score += (both_atoms_in_same_small_ring == 0);
            score -= (both_atoms_in_same_small_ring == 1) * 20;

            int is_planar = are_4at_in_one_plane(at_coord, 0.03);

            /* Provisional heuristic: the weights above accumulate evidence that
               the i-j single bond is a hindered-rotation (atropisomeric) axis.
               A score above ATROP_SCORE_HIGH is accepted outright; a borderline
               score is accepted only when the axis environment is non-planar.
               These thresholds are empirical and will be revisited together with
               true axis-parity determination. */
            const int ATROP_SCORE_HIGH = 10;
            const int ATROP_SCORE_BORDERLINE = 9;

            int is_atropisomer = 0;

            if (score > ATROP_SCORE_HIGH) {
                is_atropisomer = 1;
                ret = 1;
            } else if (score > ATROP_SCORE_BORDERLINE) {
                if (is_planar == 0) {
                    is_atropisomer = 1;
                    ret = 1;
                }
            }

            if (is_atropisomer == 1) {
                orig_inp_data->bAtropisomer = 1;

                out_at[atom_id1].bAtropisomeric = 1;
                out_at[atom_id2].bAtropisomeric = 1;
            }
        }
    }

    inchi_free(fused_atom_partner);
    return ret;
}
