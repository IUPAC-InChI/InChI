/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __ICHITIME_H__
#define __ICHITIME_H__

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

typedef struct tagInchiTime {
    clock_t clockTime;
} inchiTime;

#else

/* Win32 _ftime(): */
#include <time.h>

typedef struct tagInchiTime {
    unsigned long  clockTime; /* Time in seconds since midnight (00:00:00), January 1, 1970;
                                 signed long overflow expected in 2038 */
    long           millitime; /* milliseconds */
} inchiTime;

#endif

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


    typedef struct tagINCHI_CLOCK
    {
        clock_t m_MaxPositiveClock;
        clock_t m_MinNegativeClock;
        clock_t m_HalfMaxPositiveClock;
        clock_t m_HalfMinNegativeClock;
    } INCHI_CLOCK;

    void InchiTimeGet( inchiTime *TickEnd );

    long InchiTimeMsecDiff( INCHI_CLOCK *ic, inchiTime *TickEnd, inchiTime *TickStart );
    void InchiTimeAddMsec( INCHI_CLOCK *ic, inchiTime *TickEnd, unsigned long nNumMsec );
    int  bInchiTimeIsOver( INCHI_CLOCK *ic, inchiTime *TickEnd );
    long InchiTimeElapsed( INCHI_CLOCK *ic, inchiTime *TickStart );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __ICHITIME_H__ */
