

typedef struct Ring {
    int id;
    int* atom_ids;
    int size;
    int parent_id; // Added: ID of the larger ring containing this one
} Ring;

typedef struct {
    Ring* rings;
    int count;
} RingResult;
