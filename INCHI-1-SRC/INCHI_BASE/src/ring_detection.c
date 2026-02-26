
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

void determine_ring_hierarchy(RingResult* rr) {
    for (int i = 0; i < rr->count; i++) {
        rr->rings[i].parent_id = -1;

        int smallest_parent_size = 1e6;

        for (int j = 0; j < rr->count; j++) {
            if (i == j) {
                continue;
            }

            if (is_subset(&rr->rings[i], &rr->rings[j])) {
                if (rr->rings[j].size < smallest_parent_size) {
                    rr->rings[i].parent_id = rr->rings[j].id;
                    smallest_parent_size = rr->rings[j].size;
                    rr->rings[j].child_ids = (int*)inchi_realloc(rr->rings[j].child_ids, (rr->rings[j].child_count + 1) * sizeof(int));
                    rr->rings[j].child_ids[rr->rings[j].child_count] = rr->rings[i].id;
                    rr->rings[j].child_count++;
                }
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

int get_number_of_common_rings(const inp_ATOM *atoms,
                               int num_atoms,
                               int atom1,
                               int atom2) {

    if (atoms == NULL || num_atoms <= 0) {
        return 0; // Invalid input
    }

    if (atom1 >= num_atoms || atom2 >= num_atoms) {
        return 0; // Invalid atom indices
    }
    int count = 0;
    for (int i = 0; i < atoms[atom1].ring_count; i++) {
        int ring_id1 = atoms[atom1].ring_ids[i];
        for (int j = 0; j < atoms[atom2].ring_count; j++) {
            int ring_id2 = atoms[atom2].ring_ids[j];
            if (ring_id1 == ring_id2) {
                count++;
            }
        }
    }
    return count;
}

void print_ring_result(const RingResult *rr) {

    printf("Number of rings: %d\n", rr->count);

    for (int i = 0; i < rr->count; i++) {
        const Ring *r = &rr->rings[i];
        printf("Ring ID: %2d, Size: %2d, nof fused ring %2d, parent %2d, Atoms: ",
            r->id, r->size, r->nof_unique_fused_ring, r->parent_id);
        for (int j = 0; j < r->size; j++) {
            printf("%d ", r->atom_ids[j]);
        }
        printf("\n");
        printf("  Child Ring IDs: ");
        for(int j = 0; j < r->child_count; j++) {
            printf(" %2d ", r->child_ids[j]);
        }
        printf("\n");
    }
}

void free_ring_result(RingResult *rr) {

    if (rr == NULL) {
        return;
    }

    for (int i = 0; i < rr->count; i++) {
        inchi_free(rr->rings[i].atom_ids);
        inchi_free(rr->rings[i].child_ids);
    }
    inchi_free(rr->rings);
    inchi_free(rr);
}

void determine_fused_rings(RingResult* rr) {

    for (int nof_rings = 0; nof_rings < 100; nof_rings++) {

        for (int i = 0; i < rr->count; i++) {
            Ring *r1 = &rr->rings[i];

            if (nof_rings == r1->child_count) {
                if (r1->child_count == 0) {
                    r1->nof_unique_fused_ring = 1;
                }
                if (r1->parent_id != -1) {
                    Ring *parent = &rr->rings[r1->parent_id];
                    parent->nof_unique_fused_ring+=r1->nof_unique_fused_ring;
                }
            }
        }
    }
}

void *create_new_ring(RingResult *rr,
                      inp_ATOM *atoms,
                      int *path,
                      int path_len) {

    // Ring *r = &rr->rings[rr->count]; //(Ring*)inchi_calloc(1, sizeof(Ring));
    rr->rings = (Ring*)inchi_realloc(rr->rings, (rr->count + 1) * sizeof(Ring));
    Ring *r = &rr->rings[rr->count];

    r->id = rr->count;
    r->size = path_len;
    r->parent_id = -1;
    r->child_count = 0;
    r->child_ids = NULL;
    r->nof_unique_fused_ring = 0;
    r->atom_ids = (int*)inchi_calloc(path_len, sizeof(int));
    for (int i = 0; i < path_len; i++) {
        r->atom_ids[i] = path[i];
        atoms[path[i]].ring_ids[atoms[path[i]].ring_count] = rr->count;
        atoms[path[i]].ring_count++;
    }

    rr->rings[rr->count] = *r;
    rr->count++;
}

int is_new_ring(RingResult *rr,
                int *path,
                int path_len) {

    for (int i = 0; i < rr->count; ++i) {
        if (rr->rings[i].size != path_len) continue;
        int match = 1;
        for (int j = 0; j < path_len; ++j) {
            int found = 0;
            for (int k = 0; k < path_len; ++k) {
                if (rr->rings[i].atom_ids[k] == path[j]) {
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

void dfs(RingResult *rr,
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
                if (is_new_ring(rr, path, path_len)) {

                    create_new_ring(rr, atoms, path, path_len);

                }
            } else if (!visited[i]) {
                // dfs(start, i, visited, path, path_len);
                dfs(rr, atoms, adj, num_atoms, start, i, visited, path, path_len);
            }
        }
    }
    visited[curr] = 0;


}

RingResult *find_rings(inp_ATOM* atoms,
                       int num_atoms) {

    if (atoms == NULL || num_atoms <= 0) {
        return NULL; // Invalid input
    }

    RingResult *rr = (RingResult*)inchi_calloc(1, sizeof(RingResult));
    rr->rings = NULL; //(Ring*)inchi_calloc(num_atoms * 10, sizeof(Ring));
    rr->count = 0;

    int visited[num_atoms];
    int path[num_atoms];
    int **adj = (int**)inchi_calloc(num_atoms, sizeof(int*));  //[num_atoms][num_atoms]; // Adjacency matrix

    for (int i = 0; i < num_atoms; ++i) {
        adj[i] = (int*)inchi_calloc(num_atoms, sizeof(int));
        visited[i] = 0;
        path[i] = -1;

        inp_ATOM *atom_i = &atoms[i];
        for (int j = 0; j < atom_i->valence; j++) {
            int neighbor = atom_i->neighbor[j];
            adj[i][neighbor] = 1;
            // adj[neighbor][i] = 1; // Undirected graph
        }
    }

    for (int i = 0; i < num_atoms; ++i) {
        dfs(rr, atoms, adj, num_atoms, i, i, visited, path, 0);
    }


    determine_ring_hierarchy(rr);

    determine_fused_rings(rr);

    for (int i = 0; i < num_atoms; ++i) {
        inchi_free(adj[i]);
    }
    inchi_free(adj);

    return rr;
}


