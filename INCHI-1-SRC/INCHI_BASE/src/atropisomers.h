
#include "inpdef.h"
#include "ring_detection.h"

#ifndef ATROP_MIN_ROTATABLE_RING
/* Axis bonds trapped in a ring of size <= this are rotation-locked and are NOT
   candidate axes. Rings >= this+1 (macrocycles, bridged biaryls) still qualify.
   This is the single tunable knob of the detector. */
#define ATROP_MIN_ROTATABLE_RING 6
#endif

/* Deterministic predicate: is the single bond a1-a2 a candidate atropisomer axis?
   Returns 1 if bond is single, both ends have valence >= 3, and the bond is not
   part of a ring of size <= ATROP_MIN_ROTATABLE_RING. */
int is_candidate_atrop_axis(const inp_ATOM *at,
                            int num_atoms,
                            const RingSystems *rs,
                            int a1, int a2);

int find_atropisomeric_atoms_and_bonds(inp_ATOM* out_at,
                                       int num_atoms,
                                       RingSystems *ring_result,
                                       ORIG_ATOM_DATA *orig_inp_data);

/* Geometric axial parity for candidate axis a1-a2, via the allene stereo
   primitives (half_stereo_bond_parity + triple_prod_char). Returns
   AB_PARITY_ODD / AB_PARITY_EVEN / AB_PARITY_UNDF. */
int atrop_axis_parity(inp_ATOM *at, int a1, int a2,
                      S_CHAR z_dir1[3], S_CHAR z_dir2[3]);
