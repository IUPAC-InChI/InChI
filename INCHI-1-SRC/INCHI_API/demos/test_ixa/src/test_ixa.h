/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
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
