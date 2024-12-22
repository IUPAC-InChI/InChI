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

#include "../../../../INCHI_BASE/src/bcf_s.h"

#ifndef _TEST_IXA_H_
#define _TEST_IXA_H_

/*#define BUILD_TEST_IXA_WITH_ENG_OPTIONS 1*/

#ifdef _MSC_VER
/*
========== disable MS VC++ 6.0 Level 4 compiler warnings: ==============
 C4706: assignment within conditional expression
 C4127: conditional expression is constant
 C4244: '=' : conversion from 'int ' to '???', possible loss of data
 C4267: '=' : conversion from 'size_t' to 'int', possible loss of data
 C4701: local variable '???' may be used without having been initialized (removed)
 C4514: unreferenced inline/local function has been removed (C++)
 C4100: 'identifier' : unreferenced formal parameter
 C4786: 'identifier' : identifier was truncated to 'number' characters in the debug information
 C4090: 'function' : different 'const' qualifiers
 C4996: 'identifier' was declared deprecated
========================================================================
*/
#pragma warning( disable : 4706 4127 4514 4100 4786 4090 4996 4244 4267 )
#endif

/* supposed maxsize of Molfile (within an SD File) */
#define MOLBUFSIZE 16777216 /* 16 MB */

#ifdef APP_DESCRIPTION
#undef APP_DESCRIPTION
#endif
#define APP_DESCRIPTION "InChI version 1, Software v. " CURRENT_VER " (test_ixa - Library call example, IXA API v. " CURRENT_VER

/*#define RELEASE_IS_FINAL  0*/ /* 1=> pre-release version; comment out to disable */
#ifndef RELEASE_IS_FINAL
#define RELEASE_IS_FINAL  1    /* final release */
#endif

/* Print program usage instructions */
void print_help( void );

#endif /* _TEST_IXA_H_ */
