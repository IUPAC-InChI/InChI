/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


 /**
  * Underivatization, ring-chain tautomerism, OriGAtData edits, etc.
  */

#ifndef _ICHINORM_H_
#define _ICHINORM_H_

#include "mode.h"
#include "ichitaut.h"

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

/* main normalization procedure */
int mark_alt_bonds_and_taut_groups( struct tagINCHI_CLOCK *ic,
                                    struct tagCANON_GLOBALS *pCG,
                                    inp_ATOM *at,
                                    inp_ATOM *at_fixed_bonds_out,
                                    int num_atoms,
                                    struct tagInchiTime *ulTimeOutTime,
                                    T_GROUP_INFO *t_group_info,
                                    INCHI_MODE *inpbTautFlags,
                                    INCHI_MODE *inpbTautFlagsDone,
                                    int nebend, int *ebend );

int MarkTautomerGroups( struct tagCANON_GLOBALS *pCG, inp_ATOM *at,
                        int num_atoms,
                        T_GROUP_INFO *t_group_info,
                        C_GROUP_INFO *c_group_info,
                        struct BalancedNetworkStructure *pBNS,
                        struct BalancedNetworkData *pBD );

int MarkChargeGroups( struct tagCANON_GLOBALS *pCG,
                      inp_ATOM *at,
                      int num_atoms,
                      C_GROUP_INFO *c_group_info,
                      T_GROUP_INFO *t_group_info,
                      struct BalancedNetworkStructure *pBNS,
                      struct BalancedNetworkData *pBD );
int MarkSaltChargeGroups( struct tagCANON_GLOBALS *pCG,
                          inp_ATOM *at,
                          int num_atoms,
                          S_GROUP_INFO *s_group_info,
                          T_GROUP_INFO *t_group_info,
                          C_GROUP_INFO *c_group_info,
                          struct BalancedNetworkStructure *pBNS,
                          struct BalancedNetworkData *pBD );
int MarkSaltChargeGroups2( struct tagCANON_GLOBALS *pCG,
                           inp_ATOM *at,
                           int num_atoms,
                           S_GROUP_INFO *s_group_info,
                           T_GROUP_INFO *t_group_info,
                           C_GROUP_INFO *c_group_info,
                           struct BalancedNetworkStructure *pBNS,
                           struct BalancedNetworkData *pBD );
int MergeSaltTautGroups( struct tagCANON_GLOBALS *pCG,
                         inp_ATOM *at, int num_atoms,
                         S_GROUP_INFO *s_group_info,
                         T_GROUP_INFO *t_group_info,
                         C_GROUP_INFO *c_group_info,
                         struct BalancedNetworkStructure *pBNS );
int MakeIsotopicHGroup( inp_ATOM *at,
                        int num_atoms,
                        S_GROUP_INFO *s_group_info,
                        T_GROUP_INFO *t_group_info );

int remove_terminal_HDT( int num_atoms,
                         inp_ATOM *at,
                         int bFixTermHChrg,
                         const unsigned char* keep_explicit_HDT);
int RemoveExcessiveImplicitH( int num_atoms,
                              int num_removed_H,
                              inp_ATOM *at );
int add_DT_to_num_H( int num_atoms,
                     inp_ATOM *at );
int MarkRingSystemsInp( inp_ATOM *at,
                        int num_atoms,
                        int start );
int free_t_group_info( T_GROUP_INFO *t_group_info );
int make_a_copy_of_t_group_info( T_GROUP_INFO *t_group_info,
                                 T_GROUP_INFO *t_group_info_orig );
int set_tautomer_iso_sort_keys( T_GROUP_INFO *t_group_info );
int CountTautomerGroups( sp_ATOM *at,
                         int num_atoms,
                         T_GROUP_INFO *t_group_info );
int CountTautomerGroupsInpAt( inp_ATOM *at,
                              int num_atoms,
                              T_GROUP_INFO *t_group_info );
int SortTautomerGroupsAndEndpoints( struct tagCANON_GLOBALS *pCG,
                                    T_GROUP_INFO *t_group_info,
                                    int num_atoms,
                                    int num_at_tg,
                                    AT_RANK *nRank );
int FillIsotopicAtLinearCT( int num_atoms,
                            sp_ATOM* at,
                            const AT_RANK *nAtomNumber,
                            AT_ISOTOPIC *LinearCTIsotopic,
                            int nMaxLenLinearCTIsotopic,
                            int *pnLenLinearCTIsotopic );
int FillTautLinearCT2( struct tagCANON_GLOBALS *pCG,
                       int num_atoms,
                       int num_at_tg,
                       int bIsoTaut,
                       const AT_RANK *nRank,
                       const AT_RANK *nAtomNumber,
                       const AT_RANK *nSymmRank,
                       const AT_RANK *nRankIso,
                       const AT_RANK *nAtomNumberIso,
                       const AT_RANK *nSymmRankIso,
                       AT_TAUTOMER   *LinearCTTautomer,
                       int nMaxLenLinearCTTautomer,
                       int *pnLenLinearCTTautomer,
                       AT_ISO_TGROUP *LinearCTIsotopicTautomer,
                       int nMaxLenLinearCTIsotopicTautomer,
                       int *pnLenLinearCTIsotopicTautomer,
                       T_GROUP_INFO *t_group_info );


#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif



#endif    /* _ICHINORM_H_ */
