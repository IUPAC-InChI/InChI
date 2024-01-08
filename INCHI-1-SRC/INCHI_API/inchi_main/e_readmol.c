/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.04
 * September 9, 2011
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST. Modifications and additions by IUPAC 
 * and the InChI Trust.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the 
 * International Chemical Identifier (InChI) Software version 1.04
 * Copyright (C) IUPAC and InChI Trust Limited
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
 * or any later version.
 * 
 * Please note that this library is distributed WITHOUT ANY WARRANTIES 
 * whatsoever, whether expressed or implied.  See the IUPAC/InChI Trust 
 * Licence for the International Chemical Identifier (InChI) Software 
 * version 1.04, October 2011 ("IUPAC/InChI-Trust InChI Licence No.1.0") 
 * for more details.
 * 
 * You should have received a copy of the IUPAC/InChI Trust InChI 
 * Licence No. 1.0 with this library; if not, please write to:
 * 
 * The InChI Trust
 * c/o FIZ CHEMIE Berlin
 *
 * Franklinstrasse 11
 * 10587 Berlin
 * GERMANY
 *
 * or email to: ulrich@inchi-trust.org.
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <ctype.h>

#include "e_mode.h"

#include "inchi_api.h"

#include "e_ctl_data.h"

#include "e_ichisize.h"
#include "e_comdef.h"
#include "e_util.h"
#include "e_readmol.h"
#include "e_ichicomp.h"
#include "e_ichi_io.h"
#include "e_readstru.h"
#include "e_inpdef.h"

#define AddMOLfileError        e_AddMOLfileError
#define remove_one_lf          e_remove_one_lf
#define RemoveNonPrintable     e_RemoveNonPrintable
#define mystrncpy              e_mystrncpy
#define read_mol_file          e_read_mol_file
#define bypass_sdf_data_items  e_bypass_sdf_data_items
#define extract_ChargeRadical  e_extract_ChargeRadical
#define delete_mol_data        e_delete_mol_data
#define remove_trailing_spaces e_remove_trailing_spaces
#define normalize_name         e_normalize_name
#define read_sdfile_segment    e_read_sdfile_segment
#define CopyMOLfile            e_CopyMOLfile
#define LtrimRtrim             e_LtrimRtrim

#ifndef inchi_calloc
#define inchi_calloc            e_inchi_calloc
#endif

#ifndef inchi_free
#define inchi_free              e_inchi_free
#endif

#include "lreadmol.h"

