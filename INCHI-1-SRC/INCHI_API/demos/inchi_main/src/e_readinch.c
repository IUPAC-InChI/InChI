/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
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


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <ctype.h>

/* for use in the main program */


#include "e_mode.h"

#include "../../../../INCHI_BASE/src/inchi_api.h"

#include "e_ichisize.h"
#include "e_ctl_data.h"
#include "e_inchi_atom.h"
#include "e_ichi_io.h"


#include "e_comdef.h"
#include "e_util.h"
#include "e_ichierr.h"
#include "e_ichicomp.h"
#include "e_readstru.h"
#include "e_inpdef.h"

/* rename to avoid duplicated entry points */

#define mystrncpy            e_mystrncpy
#define LtrimRtrim           e_LtrimRtrim
#define FreeInchi_Atom       e_FreeInchi_Atom
#define FreeInchi_Stereo0D   e_FreeInchi_Stereo0D
#define CreateInchi_Atom     e_CreateInchi_Atom
#define CreateInchi_Stereo0D e_CreateInchi_Stereo0D
#define FreeInchi_Input      e_FreeInchi_Input
#define AddMOLfileError      e_AddMOLfileError
#define is_in_the_slist      e_is_in_the_slist
#define is_element_a_metal   e_is_element_a_metal
#define INChIToInchi_Atom    e_INChIToInchi_Atom
#define INChIToInchi_Input   e_INChIToInchi_Input

#define IGNORE_HEADERS
#define STATIC

#include "aux2atom.h"
