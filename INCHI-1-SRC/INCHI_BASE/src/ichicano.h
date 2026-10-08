/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef _INCHICANO_H_
#define _INCHICANO_H_


#include "ichicant.h"

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


    int GetCanonLengths( int num_at,
                         sp_ATOM* at,
                         ATOM_SIZES *s,
                         T_GROUP_INFO *t_group_info );
    int AllocateCS( CANON_STAT *pCS,
                    int num_at,
                    int num_at_tg,
                    int nLenCT,
                    int nLenCTAtOnly,
                    int nLenLinearCTStereoDble,
                    int nLenLinearCTIsotopicStereoDble,
                    int nLenLinearCTStereoCarb,
                    int nLenLinearCTIsotopicStereoCarb,
                    int nLenLinearCTTautomer,
                    int nLenLinearCTIsotopicTautomer,
                    int nLenIsotopic,
                    INCHI_MODE nMode,
                    BCN *pBCN );
    int DeAllocateCS( CANON_STAT *pCS );
    void DeAllocBCN( BCN *pBCN );

    struct tagINCHI_CLOCK;

    int Canon_INChI( struct tagINCHI_CLOCK *ic,
                     int num_atoms,
                     int num_at_tg,
                     sp_ATOM* at,
                     CANON_STAT* pCS,
                     CANON_GLOBALS *pCG,
                     INCHI_MODE nMode,
                     int bTautFtcn );
    int GetBaseCanonRanking( struct tagINCHI_CLOCK *ic,
                             int num_atoms,
                             int num_at_tg,
                             sp_ATOM* at[],
                             T_GROUP_INFO *t_group_info,
                             ATOM_SIZES s[],
                             BCN *pBCN,
                             struct tagInchiTime *ulTimeOutTime,
                             CANON_GLOBALS *pCG,
                             int bFixIsoFixedH,
                             int LargeMolecules );
    int bCanonIsFinerThanEquitablePartition( int num_atoms,
                                             sp_ATOM* at,
                                             AT_RANK *nSymmRank );
    int UpdateFullLinearCT( int num_atoms,
                            int num_at_tg,
                            sp_ATOM* at,
                            AT_RANK *nRank,
                            AT_RANK *nAtomNumber,
                            CANON_STAT* pCS,
                            CANON_GLOBALS *pCG,
                            int bFirstTime );
    int FixCanonEquivalenceInfo( CANON_GLOBALS *pCG,
                                 int num_at_tg,
                                 AT_RANK *nSymmRank,
                                 AT_RANK *nCurrRank,
                                 AT_RANK *nTempRank,
                                 AT_NUMB *nAtomNumber,
                                 int *bChanged );
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* _INCHICANO_H_ */
