/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef _E__ICHITIME_H_
#define _E__ICHITIME_H_

#ifdef COMPILE_ANSI_ONLY

#ifdef __FreeBSD__
#include <sys/time.h>
#endif

/* get times() */
#ifdef INCHI_USETIMES
#include <sys/times.h>
#endif

/*#include <sys/timeb.h>*/

#include <time.h>

typedef struct e_tagInchiTime {
    clock_t clockTime;
} e_inchiTime;

#else

/* Win32 _ftime(): */
#include <sys/timeb.h>

typedef struct e_tagInchiTime {
    unsigned long  clockTime; /* Time in seconds since midnight (00:00:00), January 1, 1970;
                                 signed long overflow expected in 2038 */
    long           millitime; /* milliseconds */
} e_inchiTime;

#endif


unsigned long e_ulMyGetTickCount( int bStart );
unsigned long e_ulMyTickCountDiff( unsigned long ulTickEnd, unsigned long ulTickStart );

void e_inchiTimeGet( e_inchiTime *TickEnd );
long e_inchiTimeMsecDiff( e_inchiTime *TickEnd, e_inchiTime *TickStart );
void e_inchiTimeAddMsec( e_inchiTime *TickEnd, unsigned long nNumMsec );
int  bInchiTimeIsOver( e_inchiTime *TickEnd );
long e_inchiTimeElapsed( e_inchiTime *TickStart );


#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif



#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif



#endif    /* _E__ICHITIME_H_ */
