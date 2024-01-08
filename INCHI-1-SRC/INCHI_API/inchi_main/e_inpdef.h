/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.03
 * May 9, 2010
 *
 * Originally developed at NIST
 * Modifications and additions by IUPAC and the InChI Trust
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC);
 * you can redistribute this software and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software 
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-2.1.php
 */


/* input/output format */
#ifndef __INPDEF_H__
#define __INPDEF_H__


/* BILLY 8/6/04 */
#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

int e_MolfileToInchi_Input( FILE *inp_molfile, inchi_Input *orig_at_data, int bMergeAllInputStructures,
                       int bDoNotAddH, int bAllowEmptyStructure,
                       const char *pSdfLabel, char *pSdfValue, long *lSdfId,
                       long *lMolfileNumber, INCHI_MODE *pInpAtomFlags, int *err, char *pStrErr );
int e_INChIToInchi_Input( INCHI_IOSTREAM *inp_molfile, inchi_Input *orig_at_data, int bMergeAllInputStructures,
                       int bDoNotAddH, int vABParityUnknown, INPUT_TYPE nInputType,
                       char *pSdfLabel, char *pSdfValue, long *lSdfId, INCHI_MODE *pInpAtomFlags,
                       int *err, char *pStrErr );

int e_CopyMOLfile(FILE *inp_file, long fPtrStart, long fPtrEnd, FILE *prb_file, long nNumb);


/* BILLY 8/6/04 */
#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif  /* __INPDEF_H__ */
