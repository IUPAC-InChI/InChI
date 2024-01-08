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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "e_mode.h"
#include "e_ctl_data.h"

#include "inchi_api.h"
#include "e_ichi_io.h"
#include "e_ichi_parms.h"
#include "e_util.h"
#include "e_ichicomp.h"

#define DetectInputINChIFileType e_DetectInputINChIFileType
#define ReadCommandLineParms     e_ReadCommandLineParms
#define mystrncpy                e_mystrncpy


#define LtrimRtrim               e_LtrimRtrim
#define PrintInputParms          e_PrintInputParms
#define OpenFiles                e_OpenFiles
#define PrintFileName            e_PrintFileName

/*^^^ */
#define HelpCommandLineParms     e_HelpCommandLineParms
#define HelpCommandLineParmsReduced    e_HelpCommandLineParmsReduced
/*^^^ */



#include "../inchi_dll/ichiparm.h"

