/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __INCHI_ATOM_H__
#define __INCHI_ATOM_H__

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

    void           e_FreeInchi_Atom( inchi_Atom **at );
    void           e_FreeInchi_Stereo0D( inchi_Stereo0D **stereo0D );
    inchi_Atom     *e_CreateInchi_Atom( int num_atoms );
    inchi_Stereo0D *e_CreateInchi_Stereo0D( int num_stereo0D );
    void           e_FreeInchi_Input( inchi_InputEx *inp_at_data );
    int            e_RemoveRedundantNeighbors( inchi_Input *inp_at_data );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __INCHI_ATOM_H__ */
