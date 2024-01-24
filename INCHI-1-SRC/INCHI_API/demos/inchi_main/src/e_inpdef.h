/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.07
 * 20/11/2023
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
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
