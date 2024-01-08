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


#ifndef __0DSTEREO_H__
#define __0DSTEREO_H__


#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


int Clear3D2Dstereo(inchi_Input *pInp);
int set_0D_stereo_parities( inchi_Input *pInp, int bPointedEdgeStereo );

#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __0DSTEREO_H__ */
