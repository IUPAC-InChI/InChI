/* djb-rwth: bounds-checking functions / C11 Annex K */

/* djb-rwth: adding guard idiom instead of #pragma once for code portabilty */
#ifndef BCF_S_H 
#define BCF_S_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

int max_3(int a, int b, int c);

#endif