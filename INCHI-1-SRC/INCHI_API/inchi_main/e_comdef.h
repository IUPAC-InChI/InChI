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


/* common definitions -- do not change */
#ifndef __COMDEF_H__
#define __COMDEF_H__

/* input bond type definition */
#define MIN_INPUT_BOND_TYPE INCHI_BOND_TYPE_SINGLE
#define MAX_INPUT_BOND_TYPE INCHI_BOND_TYPE_ALTERN

/* MOlfile */
#define INPUT_STEREO_SNGL_UP       1
#define INPUT_STEREO_SNGL_EITHER   4
#define INPUT_STEREO_SNGL_DOWN     6
#define INPUT_STEREO_DBLE_EITHER   3

/* radical definitions */
#define RADICAL_SINGLET 1
#define RADICAL_DOUBLET 2
#define RADICAL_TRIPLET 3


/* BILLY 8/6/04 */
#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif



int e_AddMOLfileError( char *pStrErr, const char *szMsg );

#define MOLFILE_ERR_FIN(err, new_err, err_fin, msg) \
        if ( !(err) && (new_err) ) { (err) = (new_err);} e_AddMOLfileError(pStrErr, (msg)); goto err_fin
#define MOLFILE_ERR_SET(err, new_err, msg) \
        if ( !(err) && (new_err) ) { (err) = (new_err);} e_AddMOLfileError(pStrErr, (msg))




/* BILLY 8/6/04 */
#ifndef INCHI_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif  /* __COMDEF_H__ */

