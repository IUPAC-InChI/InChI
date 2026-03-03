
#ifndef _RING_DETECTION_H_
#define _RING_DETECTION_H_

#define RS_MAX_RINGS_PER_ATOM 30

typedef struct Ring {
    int id;
    int *atom_ids;
    int size;
    int nof_atomic_rings;
    int parent_id; // Added: ID of the larger ring containing this one
    int *child_ids;
    int child_count;
    int is_fused_ring;
} Ring;

typedef struct Atom2RingMapping {
    int atom_id;
    int *ring_ids;
    int ring_count;
} Atom2RingMapping;

typedef struct {
    Ring* rings;
    int count;
    Atom2RingMapping* atom_to_ring_mapping;
    int num_atoms;
} RingSystems;

RingSystems *find_rings(inp_ATOM* atoms, int num_atoms);

int is_fused_ring_pivot(const RingSystems *rs,
                        const inp_ATOM * atoms,
                        int atom_id1, int atom_id2);

void print_ring_result(const RingSystems *rs);

void free_ring_system(RingSystems *rs);

int are_atoms_in_same_small_ring(const inp_ATOM* atoms,
                                 int num_atoms,
                                 const RingSystems *rs,
                                 int atom_id1, int atom_id2,
                                 int max_ring_size);


#endif
