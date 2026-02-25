
#include "mode.h"
#include "ring_detection.h"


    // int ring_ids[MAX_RINGS_PER_ATOM];
    // int ring_count;
    // int parent_id; // For BFS traversal
    // int distance;


// Helper: Check if all atoms in ring 'child' are present in ring 'potential_parent'
bool is_subset(Ring* child, Ring* potential_parent) {
    if (child->size >= potential_parent->size) return false;

    for (int i = 0; i < child->size; i++) {
        bool found = false;
        for (int j = 0; j < potential_parent->size; j++) {
            if (child->atom_ids[i] == potential_parent->atom_ids[j]) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

// Logic to assign parent_id based on containment
void determine_ring_hierarchy(RingResult* rr) {
    for (int i = 0; i < rr->count; i++) {
        rr->rings[i].parent_id = -1; // Default

        int smallest_parent_size = 1e6;

        for (int j = 0; j < rr->count; j++) {
            if (i == j) continue;

            if (is_subset(&rr->rings[i], &rr->rings[j])) {
                // We want the most immediate parent (the smallest ring that contains it)
                if (rr->rings[j].size < smallest_parent_size) {
                    rr->rings[i].parent_id = rr->rings[j].id;
                    smallest_parent_size = rr->rings[j].size;
                }
            }
        }
    }
}

Ring create_ring_struct(Atom* atoms, int u_id, int v_id, int ring_id) {
    int temp_path[MAX_NEIGHBORS * 5];
    int size = 0;
    temp_path[size++] = v_id;
    int curr = u_id;
    while (curr != -1) {
        temp_path[size++] = curr;
        if (curr == v_id) break;
        curr = atoms[curr].parent_id;
    }

    Ring r;
    r.id = ring_id;
    r.size = size;
    r.parent_id = -1;
    r.atom_ids = (int*)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        r.atom_ids[i] = temp_path[i];
        atoms[temp_path[i]].ring_ids[atoms[temp_path[i]].ring_count++] = ring_id;
    }
    return r;
}

RingResult find_unique_rings(Atom* atoms, int num_atoms) {
    RingResult rr;
    rr.rings = (Ring*)malloc(sizeof(Ring) * num_atoms);
    rr.count = 0;

    for (int i = 0; i < num_atoms; i++) {
        atoms[i].distance = -1;
        atoms[i].parent_id = -1;
    }

    int queue[MAX_NEIGHBORS * 10], head = 0, tail = 0;
    atoms[0].distance = 0;
    queue[tail++] = 0;

    while (head < tail) {
        Atom* u = &atoms[queue[head++]];
        for (int i = 0; i < u->neighbor_count; i++) {
            Atom* v = u->neighbors[i];
            if (v->distance == -1) {
                v->distance = u->distance + 1;
                v->parent_id = u->id;
                queue[tail++] = v->id;
            } else if (v->id != u->parent_id && u->distance >= v->distance) {
                rr.rings[rr.count] = create_ring_struct(atoms, u->id, v->id, rr.count);
                rr.count++;
            }
        }
    }

    determine_ring_hierarchy(&rr);
    return rr;
}
