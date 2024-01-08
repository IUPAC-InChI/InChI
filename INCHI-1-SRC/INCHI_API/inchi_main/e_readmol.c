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

