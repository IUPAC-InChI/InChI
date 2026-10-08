/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

/* djb-rwth: bounds-checking functions / C11 Annex K */

/* djb-rwth: adding guard idiom instead of #pragma once for code portabilty */
#ifndef BCF_S_H
#define BCF_S_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "inchi_version.h"

#define CURRENT_VER INCHI_SOFTWARE_VERSION

#ifdef __STDC_LIB_EXT1__
#if (__STDC_LIB_EXT1__ >= 201112L)
#define USE_BCF_GCC 1 /* djb-rwth: use bounds-checking functions / C11 Annex K */
#define __STDC_WANT_LIB_EXT1__ 1 /* Want the ext1 functions */
#endif
#endif

#ifdef __STDC_SECURE_LIB__
#define USE_BCF_MS 1 /* djb-rwth: use bounds-checking functions / C11 Annex K */
#endif

#if (USE_BCF_GCC || USE_BCF_MS)
/* djb-rwth: change the following parameter to 1 (or other unsigned integer) to use bounds-checking functions */
#define USE_BCF 0
#endif

/* djb-rwth: control macros */
#define RINCHI_TEST 0 /* djb-rwrh: RInChI testing */
/* #define GHI100_FIX */ /* djb-rwth: enabling any of the three GHI #100 fixes defined by SPRINTF_FLAG macro */
#define SPRINTF_FLAG 2 /* djb-rwth: 1 - stb_(v)s(n)pritnf, 2 - custom double to 2*int; any other value (different from 1 or 2) - standard sprintf w/ setlocale */

/* djb-rwth: custom functions */
int max_3(int a, int b, int c);
int memcpy_custom(char** dst, char* src, unsigned long long len);
int dbl2int(char* str, int fwidth, int ndecpl, char dbl_flag, double dblinp);

#endif
