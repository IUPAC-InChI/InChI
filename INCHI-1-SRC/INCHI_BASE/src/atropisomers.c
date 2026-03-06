

#include "inpdef.h"
#include "ring_detection.h"
#include "ichister.h"



void find_atropisomeric_atoms_and_bonds_old(inp_ATOM* out_at,
                                        int num_atoms,
                                        RingSystems *ring_result,
                                        ORIG_ATOM_DATA *orig_inp_data,
                                        int *fused_atom_partner) {


    for (int i = 0; i < num_atoms; i++) {

        int atom_id1 = i;
        const inp_ATOM atom_i = out_at[atom_id1];

        int num_neighbors_i = atom_i.valence;

        const AT_NUMB *neighbors = atom_i.neighbor;

        if (num_neighbors_i == 3) {
            for (int j = 0; j < num_neighbors_i; j++) {

                int atom_id2 = neighbors[j];

                if (atom_id1 >= atom_id2) {
                    continue;
                }

                const inp_ATOM atom_j = out_at[atom_id2];

                int num_neighbors_j = atom_j.valence;

                if (num_neighbors_j == 3 &&
                    atom_i.bond_stereo[j] == 0 &&
                    atom_i.bond_type[j] == 1) {

                    if (ring_result->atom_to_ring_mapping[atom_id1].ring_count > 0 ||
                        ring_result->atom_to_ring_mapping[atom_id2].ring_count > 0) {

                        int nof_wedge_bonds_i = 0;
                        int has_double_bond_i = 0;
                        for (int k = 0; k < num_neighbors_i; k++) {
                            if (atom_i.bond_stereo[k] == 1 ||
                                atom_i.bond_stereo[k] == 4 ||
                                atom_i.bond_stereo[k] == 6) {
                                nof_wedge_bonds_i++;
                            }
                            if (atom_i.bond_type[k] == 2) {
                                has_double_bond_i = 1;
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
                        }

                        if (nof_wedge_bonds_i > 0 ||
                            nof_wedge_bonds_j > 0) {

                            // if ((has_double_bond_i || has_double_bond_j) || 1==1) {
                                if ((fused_atom_partner[atom_id1] != j &&
                                     fused_atom_partner[atom_id2] != i) ||
                                    (fused_atom_partner[atom_id1] == -1 ||
                                     fused_atom_partner[atom_id2] == -1)) {

                                    if (are_atoms_in_same_small_ring(out_at,
                                                                     num_atoms,
                                                                     ring_result,
                                                                     atom_id1, atom_id2,
                                                                     6) == 0) {
                                        printf(">>> is atropisomer\n");
                                        printf("infos: atom %d with atom %d; bond type %d; bond stereo %d\n",
                                                atom_id1, atom_id2, atom_i.bond_type[j], atom_i.bond_stereo[j]);
                                        // printf("atom type %d %d\n", atom_i.el_number, atom_j.el_number);
                                        // printf("has double bond %d %d\n", has_double_bond_i, has_double_bond_j);
                                        // printf("fused pivot atoms %d %d\n", fused_atom_partner[atom_id1], fused_atom_partner[atom_id2]);

                                        //TODO set atoms with atropisomeric bonds in out_at
                                        orig_inp_data->is_atropisomer = 1;
                                    }
                                }
                            // }
                        } else {
                            // printf(">>> not atropisomer because no wedge bonds: atom %d with atom %d; bond type %d; bond stereo %d\n",
                            //         atom_id1, atom_id2, atom_i.bond_type[j], atom_i.bond_stereo[j]);
                        }

                    }
                }
            }
        }
    }

    for (int i = 0; i < ring_result->count; i++) {
        const Ring *cur_ring = &ring_result->rings[i];
        if (cur_ring->size >= 8) {
            int count_single_bonds = 0;
            int count_double_bonds = 0;
            for (int j = 0; j < cur_ring->size; j++) {
                int atom_id1 = cur_ring->atom_ids[j];
                int atom_id2 = cur_ring->atom_ids[(j + 1) % cur_ring->size];
                const inp_ATOM atom1 = out_at[atom_id1];
                for (int k = 0; k < atom1.valence; k++) {
                    if (atom1.neighbor[k] == atom_id2) {
                        if (atom1.bond_type[k] == 1) {
                            count_single_bonds++;
                        } else if (atom1.bond_type[k] == 2) {
                            count_double_bonds++;
                        }
                    }
                }
            }

            if (cur_ring->is_fused_ring == 0) {
                printf(">>> ring id %d size %d nof single bonds %d nof double bonds %d fused ring %d\n",
                    cur_ring->id, cur_ring->size,
                    count_single_bonds, count_double_bonds,
                    cur_ring->is_fused_ring);
            }
        }
    }
}

void find_atropisomeric_atoms_and_bonds(inp_ATOM* out_at,
                                        int num_atoms,
                                        RingSystems *ring_result,
                                        ORIG_ATOM_DATA *orig_inp_data,
                                        int *fused_atom_partner) {


    for (int i = 0; i < num_atoms; i++) {

        int atom_id1 = i;
        const inp_ATOM atom_i = out_at[atom_id1];

        int num_neighbors_i = atom_i.valence;

        const AT_NUMB *neighbors = atom_i.neighbor;

        double at_coord[4][3];
        at_coord[0][0] = out_at[atom_id1].x;
        at_coord[0][1] = out_at[atom_id1].y;
        at_coord[0][2] = out_at[atom_id1].z;

        if (num_neighbors_i == 3) {
            for (int j = 0; j < num_neighbors_i; j++) {
                int score = 0;


                int atom_id2 = neighbors[j];

                if (atom_id1 >= atom_id2) {
                    continue;
                }

                const inp_ATOM atom_j = out_at[atom_id2];

                int num_neighbors_j = atom_j.valence;

                score += (num_neighbors_j == 3);
                score += (atom_i.bond_stereo[j] == 0);

                score -= ((atom_i.bond_stereo[j] == 1) || (atom_i.bond_stereo[j] == 4) || (atom_i.bond_stereo[j] == 6)) * 2;

                score += (atom_i.bond_type[j] == 1);

                score -= (atom_i.bond_type[j] != 1) * 2;

                score += (ring_result->atom_to_ring_mapping[atom_id1].ring_count > 0) * 3;
                score += (ring_result->atom_to_ring_mapping[atom_id2].ring_count > 0) * 3;


                int nof_wedge_bonds_i = 0;
                int has_double_bond_i = 0;
                for (int k = 0; k < num_neighbors_i; k++) {
                    if (atom_i.bond_stereo[k] == 1 ||
                        atom_i.bond_stereo[k] == 4 ||
                        atom_i.bond_stereo[k] == 6) {
                        nof_wedge_bonds_i++;
                    }
                    if (atom_i.bond_type[k] == 2) {
                        has_double_bond_i = 1;
                    }
                    if (atom_id2 != out_at[atom_id1].neighbor[k]) {
                        at_coord[k + 1][0] = out_at[atom_i.neighbor[k]].x;
                        at_coord[k + 1][1] = out_at[atom_i.neighbor[k]].y;
                        at_coord[k + 1][2] = out_at[atom_i.neighbor[k]].z;
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
                    if (atom_id1 != out_at[atom_id2].neighbor[k]) {
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

                score += (fused_atom_partner[atom_id1] != j && fused_atom_partner[atom_id2] != i);
                score += (fused_atom_partner[atom_id1] == -1 || fused_atom_partner[atom_id2] == -1);

                int atom_in_same_small_ring = are_atoms_in_same_small_ring(out_at,
                                                                           num_atoms,
                                                                           ring_result,
                                                                           atom_id1, atom_id2,
                                                                           6);

                score += (atom_in_same_small_ring == 0);

                // printf("are atom in small ring? %d %d\n", atom_in_same_small_ring, (atom_in_same_small_ring == 1) * 10);
                score -= (atom_in_same_small_ring == 1) * 20;

                int planar_check = are_4at_in_one_plane(at_coord, 0.03);

                if (score > 10) {
                    printf(">>> FOUND atropisomer: atom id %2d atom id %2d planar check %d --> score %2d (%d)\n", atom_id1, atom_id2, planar_check, score, atom_in_same_small_ring);
                    orig_inp_data->is_atropisomer = 1;
                    orig_inp_data->is_diasteroisomeric_atropisomer = ((nof_wedge_bonds_i > 0) && (nof_wedge_bonds_j > 0));

                    out_at[atom_id1].bAtropisomeric = 1;
                    out_at[atom_id2].bAtropisomeric = 1;

                } else if (score > 9) {
                    if (planar_check == 0) {
                        printf(">>> FOUND atropisomer (higher score): atom id %2d atom id %2d  planar check %d  --> score %2d (%d)\n", atom_id1, atom_id2, planar_check, score, atom_in_same_small_ring);
                        orig_inp_data->is_atropisomer = 1;
                        orig_inp_data->is_diasteroisomeric_atropisomer = ((nof_wedge_bonds_i > 0) && (nof_wedge_bonds_j > 0));

                        out_at[atom_id1].bAtropisomeric = 1;
                        out_at[atom_id2].bAtropisomeric = 1;
                    } else {
                        // printf(">>> potential atropisomer (higher score): atom id %2d atom id %2d  planar check %d  --> score %2d (%d)\n", atom_id1, atom_id2, planar_check, score, atom_in_same_small_ring);
                    }
                }

                // printf(">>> is atropisomer\n");
                // printf("infos: atom %d with atom %d; bond type %d; bond stereo %d\n",
                //         atom_id1, atom_id2, atom_i.bond_type[j], atom_i.bond_stereo[j]);
                // printf("atom type %d %d\n", atom_i.el_number, atom_j.el_number);
                // printf("has double bond %d %d\n", has_double_bond_i, has_double_bond_j);
                // printf("fused pivot atoms %d %d\n", fused_atom_partner[atom_id1], fused_atom_partner[atom_id2]);
            }
        }
    }

    // for (int i = 0; i < ring_result->count; i++) {
    //     const Ring *cur_ring = &ring_result->rings[i];
    //     if (cur_ring->size >= 8) {
    //         int count_single_bonds = 0;
    //         int count_double_bonds = 0;
    //         for (int j = 0; j < cur_ring->size; j++) {
    //             int atom_id1 = cur_ring->atom_ids[j];
    //             int atom_id2 = cur_ring->atom_ids[(j + 1) % cur_ring->size];
    //             const inp_ATOM atom1 = out_at[atom_id1];
    //             for (int k = 0; k < atom1.valence; k++) {
    //                 if (atom1.neighbor[k] == atom_id2) {
    //                     if (atom1.bond_type[k] == 1) {
    //                         count_single_bonds++;
    //                     } else if (atom1.bond_type[k] == 2) {
    //                         count_double_bonds++;
    //                     }
    //                 }
    //             }
    //         }

    //         if (cur_ring->is_fused_ring == 0) {
    //             printf(">>> ring id %d size %d nof single bonds %d nof double bonds %d fused ring %d\n",
    //                 cur_ring->id, cur_ring->size,
    //                 count_single_bonds, count_double_bonds,
    //                 cur_ring->is_fused_ring);
    //         }
    //     }
    // }
}
