/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef _ICHISTER_H_
#define _ICHISTER_H_

#include "ichicomn.h"

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif
    int bCanAtomBeAStereoCenter( char *elname, S_CHAR charge, S_CHAR radical );
    int bCanInpAtomBeAStereoCenter( inp_ATOM *at, int cur_at, int bPointedEdgeStereo, int bStereoAtZz );
    int bCanAtomHaveAStereoBond( char *elname, S_CHAR charge, S_CHAR radical );
    int bCanAtomBeTerminalAllene( char *elname, S_CHAR charge, S_CHAR radical );
    int bCanAtomBeMiddleAllene( char *elname, S_CHAR charge, S_CHAR radical );
    int bAtomHasValence3( char *elname, S_CHAR charge, S_CHAR radical );

    double dot_prod3(const double a[], const double b[]);
    void* cross_prod3(const double a[], const double b[], double result[]);

    int are_4at_in_one_plane( double at_coord[][3], double min_sine );

    struct tagCANON_GLOBALS;
    int set_stereo_parity( struct tagCANON_GLOBALS *pCG,
                           inp_ATOM* at,
                           sp_ATOM* at_output,
                           int num_at,
                           int num_removed_H,
                           int *nMaxNumStereoAtoms,
                           int *nMaxNumStereoBonds,
                           INCHI_MODE nMode,
                           int bPointedEdgeStereo,
                           int vABParityUnknown,
                           int bLooseTSACheck,
                           int bStereoAtZz );

    int get_opposite_sb_atom( inp_ATOM *at, int cur_atom, int icur2nxt,
                              int *pnxt_atom, int *pinxt2cur, int *pinxt_sb_parity_ord );

    /* Allene/atropisomer stereo primitives, used by atropisomers.c to compute
       geometric axial parity. */
    int triple_prod_char( inp_ATOM *at, int at_1, int i_next_at_1, S_CHAR *z_dir1,
                          int at_2, int i_next_at_2, S_CHAR *z_dir2 );
    int half_stereo_bond_parity( inp_ATOM *at, int cur_at, inp_ATOM *at_removed_H, int num_removed_H,
                                 S_CHAR *z_dir, int bPointedEdgeStereo, int vABParityUnknown );

#define PES_BIT_POINT_EDGE_STEREO    1
#define PES_BIT_PHOSPHINE_STEREO     2
#define PES_BIT_ARSINE_STEREO        4
#define PES_BIT_FIX_SP3_BUG          8
#define PES_BIT_ALLENE_ONE_WEDGE    16

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif    /* _ICHISTER_H_ */
