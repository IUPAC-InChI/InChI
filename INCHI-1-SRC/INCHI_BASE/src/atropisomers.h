
#include "inpdef.h"
#include "ring_detection.h"

void find_atropisomeric_atoms_and_bonds(inp_ATOM* out_at,
                                        int num_atoms,
                                        RingSystems *ring_result,
                                        ORIG_ATOM_DATA *orig_inp_data,
                                        int *fused_atom_partner);
