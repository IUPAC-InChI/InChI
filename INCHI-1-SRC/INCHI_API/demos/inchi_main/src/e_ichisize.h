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


#ifndef ___INCHISIZE_H__
#define ___INCHISIZE_H__


typedef unsigned short AT_NUMB;
typedef unsigned short AT_RANK;
#define AT_RANK_MASK   ((AT_RANK)~0)

typedef signed short NUM_H;
#define MAX_ATOMS  32766
#define NORMALLY_ALLOWED_INP_MAX_ATOMS 1024

#define CHAR_MASK  0xFF

typedef unsigned long INCHI_MODE;

#define LEN_COORD 10
#define NUM_COORD 3
typedef char MOL_COORD[LEN_COORD*NUM_COORD + NUM_COORD - 1]; /*copied 30 bytes from MOLfile */

typedef enum tagInputType { INPUT_NONE = 0, INPUT_MOLFILE = 1, INPUT_SDFILE = 2, INPUT_INCHI_XML = 3, INPUT_INCHI_PLAIN = 4, INPUT_CMLFILE = 5, INPUT_INCHI = 6, INPUT_MAX } INPUT_TYPE;

/* other types */

#define UCINT  (int)(unsigned char)
#ifndef INCHI_US_CHAR_DEF
typedef signed char   S_CHAR;
typedef unsigned char U_CHAR;
#define INCHI_US_CHAR_DEF
#endif



#endif /* ___INCHISIZE_H__ */
