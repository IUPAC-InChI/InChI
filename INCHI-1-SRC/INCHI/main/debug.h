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
 * http://www.opensource.org/licenses/lgpl-license.php
 */


#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>

#include "mode.h"

#ifdef CML_DEBUG
#include "inpdef.h"


#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C"
{
#endif
#endif

void PrintInpAtom (FILE *f_p, const inp_ATOM * const atom_p, const unsigned int num_atoms);

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* CML_DEBUG */

#endif  /*  __DEBUG_H__ */

