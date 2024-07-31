/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.07
 * April 30, 2024
 *
 * MIT License
 *
 * Copyright (c) 2024 IUPAC and InChI Trust
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * info@inchi-trust.org
 *
 */

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
#define APP_DESCRIPTION "InChI version 1, Software v. 1.061 (test_ixa - Library call example, IXA API v. 1.061)"

/*#define RELEASE_IS_FINAL  0*/ /* 1=> pre-release version; comment out to disable */
#ifndef RELEASE_IS_FINAL
#define RELEASE_IS_FINAL  1    /* final release */
#endif

/* Print program usage instructions */
void print_help( void );

#endif /* _TEST_IXA_H_ */
