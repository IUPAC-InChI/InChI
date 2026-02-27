
typedef struct Ring {
    int id;
    int *atom_ids;
    int size;
    int nof_atomic_rings;
    int parent_id; // Added: ID of the larger ring containing this one
    int *child_ids;
    int child_count;
} Ring;

typedef struct {
    Ring* rings;
    int count;
} RingSystems;


RingSystems *find_rings(inp_ATOM* atoms, int num_atoms);

int is_fused_ring_pivot(const RingSystems *rs,
                        const inp_ATOM * atoms,
                        int atom_id1, int atom_id2);

void print_ring_result(const RingSystems *rs);

void free_ring_system(RingSystems *rs);

int are_atoms_in_same_small_ring(const inp_ATOM* atoms,
                                 const int *ring_id_to_size,
                                 int atom_id1, int atom_id2,
                                 int max_ring_size);
