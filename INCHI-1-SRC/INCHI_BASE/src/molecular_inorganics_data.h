/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

#ifndef _MOLECULAR_INORGANICS_DATA_H_
#define _MOLECULAR_INORGANICS_DATA_H_

#include "strutil.h"   /* MolecularInorganicsElData, ElementsMolecularInorganics, MolInOrg(n) */
#include "extr_ct.h"   /* NUM_CHEM_ELEMENTS */

#define NUM_ELEMENTS 118 /* Adjust according to the number of elements in the array */

extern const MolecularInorganicsElData MolecularInorganicsArray[NUM_CHEM_ELEMENTS];
extern const ElementsMolecularInorganics elementListMolecularInorganics[NUM_ELEMENTS];
extern const int binaryArrayMolecularInorganics[NUM_ELEMENTS][NUM_ELEMENTS];

#endif /* _MOLECULAR_INORGANICS_DATA_H_ */
