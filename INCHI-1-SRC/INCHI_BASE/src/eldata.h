/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

#ifndef _ELDATA_H_
#define _ELDATA_H_

#include "util.h"      /* ELDATA struct definition */
#include "extr_ct.h"   /* NUM_CHEM_ELEMENTS */

/* Periodic-table element/valence data. Definition lives in eldata.c.
   ERR_ELEM and nElDataLen are declared extern in util.h. */
extern const ELDATA ElData[NUM_CHEM_ELEMENTS];

#endif /* _ELDATA_H_ */
