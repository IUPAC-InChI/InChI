/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */




/* input/output format */
#ifndef __INPDEF_H__
#define __INPDEF_H__


/* BILLY 8/6/04 */
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

    int e_MolfileToInchi_Input( FILE *inp_molfile, inchi_InputEx *orig_at_data, int bMergeAllInputStructures,
                           int bDoNotAddH, int bAllowEmptyStructure,
                           const char *pSdfLabel, char *pSdfValue, long *lSdfId,
                           long *lMolfileNumber, INCHI_MODE *pInpAtomFlags, int *err, char *pStrErr );
    int e_INChIToInchi_Input( INCHI_IOSTREAM *inp_molfile, inchi_InputEx *orig_at_data, int bMergeAllInputStructures,
                           int bDoNotAddH, int vABParityUnknown, INPUT_TYPE nInputType,
                           char *pSdfLabel, char *pSdfValue, long *lSdfId, INCHI_MODE *pInpAtomFlags,
                           int *err, char *pStrErr );

    int e_CopyMOLfile( FILE *inp_file, long fPtrStart, long fPtrEnd, FILE *prb_file, long nNumb );


    /* BILLY 8/6/04 */
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif  /* __INPDEF_H__ */
