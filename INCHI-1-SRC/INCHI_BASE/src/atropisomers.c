

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

    int fused_atom_partner[num_atoms];
    for (int i = 0; i < num_atoms; i++) {
        // out_at[i].fused_partner_atom_id = -1;
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
                    at_coord[3][0] = out_at[atom_j.neighbor[k]].x;
                    at_coord[3][1] = out_at[atom_j.neighbor[k]].y;
                    at_coord[3][2] = out_at[atom_j.neighbor[k]].z;
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

            // printf("are atom in small ring? %d %d\n", atom_in_same_small_ring, (atom_in_same_small_ring == 1) * 10);
            score -= (both_atoms_in_same_small_ring == 1) * 20;

            int is_planar = are_4at_in_one_plane(at_coord, 0.03);

            int is_atropisomer = 0;

            if (score > 10) {
                is_atropisomer = 1;
                ret = 1;
            } else if (score > 9) {
                if (is_planar == 0) {
                    is_atropisomer = 1;
                    ret = 1;
                } else {
                    // TODO are more tests/rules needed??
                }
            }

            if (is_atropisomer == 1) {
                printf(">>> FOUND atropisomer (higher score): atom id %2d atom id %2d  is planar %d  --> score %2d (%d)\n", atom_id1, atom_id2, is_planar, score, both_atoms_in_same_small_ring);
                orig_inp_data->bAtropisomer = 1;

                out_at[atom_id1].bAtropisomeric = 1;
                out_at[atom_id2].bAtropisomeric = 1;
            }

            // printf(">>> is atropisomer\n");
            // printf("infos: atom %d with atom %d; bond type %d; bond stereo %d\n",
            //         atom_id1, atom_id2, atom_i.bond_type[j], atom_i.bond_stereo[j]);
            // printf("atom type %d %d\n", atom_i.el_number, atom_j.el_number);
            // printf("has double bond %d %d\n", has_double_bond_i, has_double_bond_j);
            // printf("fused pivot atoms %d %d\n", fused_atom_partner[atom_id1], fused_atom_partner[atom_id2]);
        }
    }
    return ret;
}
