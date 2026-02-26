

typedef struct Ring {
    int id;
    int *atom_ids;
    int size;
    int nof_unique_fused_ring;
    int parent_id; // Added: ID of the larger ring containing this one
    int *child_ids;
    int child_count;
} Ring;

typedef struct {
    Ring* rings;
    int count;
} RingResult;


RingResult *find_rings(inp_ATOM* atoms, int num_atoms);

int get_number_of_common_rings(const inp_ATOM* atoms,
                               int num_atoms,
                               int atom1,
                               int atom2);

void print_ring_result(const RingResult *rr);

void free_ring_result(RingResult *rr);
