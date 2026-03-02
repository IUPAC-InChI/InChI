
#include "mode.h"
#include "inpdef.h"

#include "ring_detection.h"


int is_subset(Ring* child,
              Ring* potential_parent) {

    if (child->size >= potential_parent->size) {
        return 0;
    }

    for (int i = 0; i < child->size; i++) {
        int found = 0;
        for (int j = 0; j < potential_parent->size; j++) {
            if (child->atom_ids[i] == potential_parent->atom_ids[j]) {
                found = 1;
                break;
            }
        }
        if (!found) {
            return 0;
        }
    }
    return 1;
}

void determine_ring_hierarchy(RingSystems* rs) {
    for (int i = 0; i < rs->count; i++) {
        rs->rings[i].parent_id = -1;

        int smallest_parent_size = 1e6;

        for (int j = 0; j < rs->count; j++) {
            if (i == j) {
                continue;
            }

            if (is_subset(&rs->rings[i], &rs->rings[j])) {
                // if (rs->rings[j].size < smallest_parent_size) {
                rs->rings[i].parent_id = rs->rings[j].id;
                // smallest_parent_size = rs->rings[j].size;

                rs->rings[j].child_ids = (int*)inchi_realloc(rs->rings[j].child_ids, (rs->rings[j].child_count + 1) * sizeof(int));
                rs->rings[j].child_ids[rs->rings[j].child_count] = rs->rings[i].id;
                rs->rings[j].child_count++;
                // }
            }
        }
    }
}

int get_ring_atom_overlap(const Ring *r1, const Ring *r2) {
    int overlap = 0;
    for (int i = 0; i < r1->size; i++) {
        for (int j = 0; j < r2->size; j++) {
            if (r1->atom_ids[i] == r2->atom_ids[j]) {
                overlap++;
            }
        }
    }
    return overlap;
}

int get_number_of_overlapping_rings(const Ring *r1, const Ring *r2) {

    int count = 0;
    for (int i = 0; i < r1->child_count; i++) {
        for (int j = 0; j < r2->child_count; j++) {
            if (r1->child_ids[i] == r2->child_ids[j]) {
                count++;
            }
        }
    }
    return count;
}

int get_number_of_atomic_rings_from_atom(const RingSystems *rs,
                                         int atom_id) {

    if (atom_id < 0) {
        return 0; // Invalid input
    }

    int count = 0;
    for (int i = 0; i < rs->count; i++) {
        const Ring *cur_ring = &rs->rings[i];
        if (cur_ring->child_count == 0) {
            for (int j = 0; j < cur_ring->size; j++) {
                if (cur_ring->atom_ids[j] == atom_id) {
                    count++;
                }
            }
        }
    }
    return count;
}

int is_atom_in_ring(const Ring *r, int atom_id) {
    for (int i = 0; i < r->size; i++) {
        if (r->atom_ids[i] == atom_id) {
            return 1;
        }
    }
    return 0;
}

int are_atoms_in_same_small_ring(const inp_ATOM* atoms,
                                 const RingSystems *rs,
                                 int atom_id1, int atom_id2,
                                 int max_ring_size) {

    inp_ATOM atom1 = atoms[atom_id1];
    inp_ATOM atom2 = atoms[atom_id2];

    for (int i = 0; i < rs->atom_to_ring_mapping[atom_id1].ring_count; i++) {
        int ring_id1 = rs->atom_to_ring_mapping[atom_id1].ring_ids[i];
        if (rs->rings[ring_id1].size <= max_ring_size) {
            for (int j = 0; j < rs->atom_to_ring_mapping[atom_id2].ring_count; j++) {
                int ring_id2 = rs->atom_to_ring_mapping[atom_id2].ring_ids[j];
                if (ring_id1 == ring_id2) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void print_ring(const Ring *r) {
    printf("Ring ID: %d, Size: %d, nof fused ring %d, parent %d, Atoms: ",
        r->id, r->size, r->nof_atomic_rings, r->parent_id);
    for (int i = 0; i < r->size; i++) {
        printf("%d ", r->atom_ids[i]);
    }
    printf("\n");
    printf("  Child Ring IDs: ");
    for(int i = 0; i < r->child_count; i++) {
        printf(" %d ", r->child_ids[i]);
    }
    printf("\n");
}

void print_ring_result(const RingSystems *rs) {

    printf("Number of rings: %d\n", rs->count);

    for (int i = 0; i < rs->count; i++) {
        const Ring *r = &rs->rings[i];
        print_ring(r);
    }
}

void free_ring_system(RingSystems *rs) {

    if (rs == NULL) {
        return;
    }

    for (int i = 0; i < rs->count; i++) {
        inchi_free(rs->rings[i].atom_ids);
        inchi_free(rs->rings[i].child_ids);
    }
    inchi_free(rs->rings);

    for (int i = 0; i < rs->num_atoms; i++) {
        inchi_free(rs->atom_to_ring_mapping[i].ring_ids);
    }
    inchi_free(rs->atom_to_ring_mapping);
    inchi_free(rs);
}

int sub_ring_counter(RingSystems* rs, const Ring *r, int *ring_counter) {
    if (r->child_count == 0) {
        ring_counter[r->id] = 1;
    } else {
        for (int i = 0; i < r->child_count; i++) {
            sub_ring_counter(rs, &rs->rings[r->child_ids[i]], ring_counter);
        }
    }
}

void determine_fused_rings(RingSystems* rs) {

    for (int i = 0; i < rs->count; i++) {
        Ring *cur_ring = &rs->rings[i];

        int ring_counter[rs->count];
        for (int j = 0; j < rs->count; j++) {
            ring_counter[j] = 0;
        }
        sub_ring_counter(rs, cur_ring, ring_counter);

        int count = 0;
        for (int j = 0; j < rs->count; j++) {
            if (ring_counter[j] > 0) {
                count++;
            }
        }

        cur_ring->nof_atomic_rings = count;
    }
}

void *create_new_ring(RingSystems *rs,
                      inp_ATOM *atoms,
                      int *path,
                      int path_len) {

    // Ring *r = &rs->rings[rs->count]; //(Ring*)inchi_calloc(1, sizeof(Ring));
    rs->rings = (Ring*)inchi_realloc(rs->rings, (rs->count + 1) * sizeof(Ring));
    Ring *r = &rs->rings[rs->count];

    r->id = rs->count;
    r->size = path_len;
    r->parent_id = -1;
    r->child_count = 0;
    r->child_ids = NULL;
    r->nof_atomic_rings = 0;
    r->is_fused_ring = 0;
    r->atom_ids = (int*)inchi_calloc(path_len, sizeof(int));
    for (int i = 0; i < path_len; i++) {
        r->atom_ids[i] = path[i];

        rs->atom_to_ring_mapping[path[i]].atom_id = path[i];
        rs->atom_to_ring_mapping[path[i]].ring_ids = (int*)inchi_realloc(rs->atom_to_ring_mapping[path[i]].ring_ids,
            (rs->atom_to_ring_mapping[path[i]].ring_count + 1) * sizeof(int));
        rs->atom_to_ring_mapping[path[i]].ring_ids[rs->atom_to_ring_mapping[path[i]].ring_count] = r->id;
        rs->atom_to_ring_mapping[path[i]].ring_count++;

        inp_ATOM atom = atoms[path[i]];
        for (int j = i + 1; j < path_len; j++) {
            // prev, i, next
            int prev_atom_id = path[i - 1 < 0 ? path_len - 1 : i - 1];
            int cur_atom_id = path[i];
            int next_atom_id = path[i + 1 >= path_len ? 0 : i + 1];
            int other_atom_id = path[j];
            for (int k = 0; k < atom.valence; k++) {
                if (atom.neighbor[k] == prev_atom_id || atom.neighbor[k] == next_atom_id) {
                    continue;
                } else if (atom.neighbor[k] == other_atom_id)
                {
                    r->is_fused_ring = 1;
                }
            }
        }
    }

    rs->rings[rs->count] = *r;
    rs->count++;
}

int is_new_ring(RingSystems *rs,
                int *path,
                int path_len) {

    for (int i = 0; i < rs->count; ++i) {
        if (rs->rings[i].size != path_len) continue;
        int match = 1;
        for (int j = 0; j < path_len; ++j) {
            int found = 0;
            for (int k = 0; k < path_len; ++k) {
                if (rs->rings[i].atom_ids[k] == path[j]) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                match = 0;
                break;
            }
        }
        if (match) return 0;
    }
    return 1;
}

void dfs(RingSystems *rs,
         inp_ATOM* atoms,
         int **adj,
         int num_atoms,
         int start,
         int curr,
         int *visited,
         int *path,
         int path_len) {

    visited[curr] = 1;
    path[path_len] = curr;
    path_len++;

    for (int i = 0; i < num_atoms; ++i) {
        if (adj[curr][i]) {
            if (i == start && path_len > 2) {
                if (is_new_ring(rs, path, path_len)) {

                    create_new_ring(rs, atoms, path, path_len);

                }
            } else if (!visited[i]) {
                // dfs(start, i, visited, path, path_len);
                dfs(rs, atoms, adj, num_atoms, start, i, visited, path, path_len);
            }
        }
    }
    visited[curr] = 0;
}

int is_fused_ring_pivot(const RingSystems *rs,
                        const inp_ATOM * atoms,
                        int atom_id1, int atom_id2) {

    if (atoms == NULL || rs == NULL || atom_id1 < 0 || atom_id2 < 0) {
        return 0; // Invalid input
    }

    int are_neighbours = 0;

    const inp_ATOM *atom1 = &atoms[atom_id1];
    const inp_ATOM *atom2 = &atoms[atom_id2];

    if (atom1 == NULL || atom2 == NULL) {
        return 0; // Invalid input
    }

    if (atom1->valence <= 2 || atom2->valence <= 2) {
        return 0;
    }

    for (int i = 0; i < atom1->valence; i++) {
        if (atom1->neighbor[i] == atom_id2) {
            are_neighbours = 1;
            break;
        }
    }
    if (get_number_of_atomic_rings_from_atom(rs, atom_id1) < 2 &&
        get_number_of_atomic_rings_from_atom(rs, atom_id2) < 2) {
        return 0;
    }

    if (are_neighbours) {
        for (int i = 0; i < rs->count; i++) {
            const Ring *r = &rs->rings[i];
            int found_atom1 = 0;
            int found_atom2 = 0;
            for (int j = 0; j < r->size; j++) {
                if (r->atom_ids[j] == atom_id1) {
                    found_atom1 = 1;
                }
                if (r->atom_ids[j] == atom_id2) {
                    found_atom2 = 1;
                }
            }
            if (found_atom1 && found_atom2) {
                if (r->nof_atomic_rings == 2) {
                    // printf("atom1 %d atom2 %d ring id %d nof atomic rings %d\n",
                    //     atom_id1, atom_id2, r->id, r->nof_atomic_rings);
                    // print_ring(r);
                    int count = 0;
                    for (int j = 0; j < r->child_count; j++) {
                        const Ring *child_ring = &rs->rings[r->child_ids[j]];
                        if (is_atom_in_ring(child_ring, atom_id1) && is_atom_in_ring(child_ring, atom_id2)) {
                            count++;
                        }
                    }
                    if (count == r->child_count) {
                        // printf("atom1 %d atom2 %d ring id %d nof atomic rings %d\n",
                        //     atom_id1, atom_id2, r->id, r->nof_atomic_rings);
                        // print_ring(r);
                        // printf(">>> Found pivot atom pair: %d, %d\n", atom_id1, atom_id2);
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

RingSystems *find_rings(inp_ATOM* atoms,
                        int num_atoms) {

    if (atoms == NULL || num_atoms <= 0) {
        return NULL; // Invalid input
    }

    RingSystems *rs = (RingSystems*)inchi_calloc(1, sizeof(RingSystems));
    rs->rings = NULL; //(Ring*)inchi_calloc(num_atoms * 10, sizeof(Ring));
    rs->count = 0;
    rs->num_atoms = num_atoms;
    rs->atom_to_ring_mapping = (Atom2RingMapping*)inchi_calloc(num_atoms, sizeof(Atom2RingMapping));

    int visited[num_atoms];
    int path[num_atoms];
    int **adj = (int**)inchi_calloc(num_atoms, sizeof(int*));  //[num_atoms][num_atoms]; // Adjacency matrix

    for (int i = 0; i < num_atoms; ++i) {
        adj[i] = (int*)inchi_calloc(num_atoms, sizeof(int));
        visited[i] = 0;
        path[i] = -1;

        const inp_ATOM *atom_i = &atoms[i];
        for (int j = 0; j < atom_i->valence; j++) {
            int neighbor = atom_i->neighbor[j];
            adj[i][neighbor] = 1;
            // adj[neighbor][i] = 1; // Undirected graph
        }
    }

    for (int i = 0; i < num_atoms; ++i) {
        dfs(rs, atoms, adj, num_atoms, i, i, visited, path, 0);
    }


    determine_ring_hierarchy(rs);

    determine_fused_rings(rs);

    for (int i = 0; i < num_atoms; ++i) {
        inchi_free(adj[i]);
    }
    inchi_free(adj);

    return rs;
}


