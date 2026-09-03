/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __INCHI_DLL_B_H__
#define __INCHI_DLL_B_H__

#ifndef AB_PARITY_UNKN
#define AB_PARITY_UNKN   3  /* 3 => user marked as unknown parity */
#endif
#ifndef AB_PARITY_UNDF
#define AB_PARITY_UNDF   4  /* 4 => parity cannot be defined because of symmetry or not well defined geometry */
#endif


#define MOL2INCHI_NO_RAM    1001
#define MOL2INCHI_BAD_COMMAND_LINE 1002


void FreeInchi_Stereo0D( inchi_Stereo0D **stereo0D );
void FreeInchi_Atom( inchi_Atom **at );
inchi_Atom *CreateInchiAtom( int num_atoms );
inchi_Stereo0D *CreateInchi_Stereo0D( int num_stereo0D );
void FreeInchi_Input( inchi_Input *inp_at_data );
S_SHORT *is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen );
int is_element_a_metal( char szEl[] );

int InchiToInchiAtom( INCHI_IOSTREAM *inp_molfile,
                      inchi_Stereo0D **stereo0D,
                      int *num_stereo0D,
                      int bDoNotAddH,
                      int vABParityUnknown,
                      INPUT_TYPE nInputType,
                      inchi_Atom **at,
                      int max_num_at,
                      int *num_dimensions,
                      int *num_bonds,
                      char *pSdfLabel,
                      char *pSdfValue,
                      long *Id,
                      INCHI_MODE *pInpAtomFlags,
                      int *err,
                      char *pStrErr );


#endif /* __INCHI_DLL_B_H__ */
