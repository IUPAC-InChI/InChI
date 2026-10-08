/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

 /**
  * Aromatic-bond classification utilities.
  *
  * Self-contained predicates and helpers that classify/transform bonds of
  * type BOND_TYPE_ALTERN ("aromatic") on inp_ATOM arrays. Extracted from
  * ichinorm.c (and a revived dead helper from strutil.c) so they can be
  * unit-tested independently of the normalization/underivatize code.
  *
  * NOT included here (kept in their own domains): the core aromatic-bond
  * perception mark_alt_bonds_and_taut_groups() (BNS engine, ichi_bns.c),
  * the stereo bond classifier (ichister.c), the tautomer-solver inline
  * BOND_ALTERN checks (ichitaut.c), the metal-disconnection aromatic-bond
  * counting (strutil.c), and parser bond-type mapping (mol2atom.c/readinch.c).
  */

#ifndef _AROMATICITY_H_
#define _AROMATICITY_H_

#include "mode.h"
#include "inpdef.h"

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

struct tagINCHI_CLOCK;
struct tagCANON_GLOBALS;

/**
 * @brief Charge a neutral, divalent aromatic oxygen or sulfur so it can be kekulized.
 *
 * Matches a two-coordinate, uncharged, non-radical O or S whose two bond
 * types sum to 3 (one single + one double bond -- the Kekule form of a
 * divalent -O-/-S- in an aromatic ring). Setting a formal +1 charge marks
 * such an atom so the surrounding ring can be kekulized.
 *
 * @param atom Atom to inspect; its @c charge is set to 1 in place on a match.
 * @return 1 if the atom matched and was modified; 0 otherwise (atom untouched).
 */
int fix_aromatic_oxygen_and_sulfur( inp_ATOM *atom );

/**
 * @brief Test whether an atom is an unsaturated but non-aromatic carbon.
 *
 * True when @c at[i] is a neutral, non-radical carbon of total valence 4 that
 * carries at least one genuine double bond (double bonds to a terminal =O or
 * =S are not counted) and no aromatic (BOND_TYPE_ALTERN) bonds.
 *
 * @param at Atom array.
 * @param i  Index of the atom to test.
 * @return Non-zero if the carbon is unsaturated and not aromatic; 0 otherwise.
 */
int is_C_unsat_not_arom( inp_ATOM *at, int i );

/**
 * @brief Test whether an atom is an aromatic aryl attachment carbon.
 *
 * True when @p attachment_pont is a neutral, non-radical, three-coordinate
 * carbon (chem_bonds_valence 4, no implicit H) whose two bonds other than the
 * one to @p outside_point are aromatic (BOND_TYPE_ALTERN) bonds to carbon or
 * nitrogen.
 *
 * @param at               Atom array.
 * @param outside_point    Index of the substituent-side neighbor (outside the ring).
 * @param attachment_pont  Index of the candidate aryl attachment carbon.
 * @return Non-zero if @p attachment_pont is an aromatic aryl carbon; 0 otherwise.
 */
int is_Aryl( inp_ATOM *at, int outside_point, int attachment_pont );

/**
 * @brief Verify a chain of aromatic CH carbons of a given length.
 *
 * Walks from @p cur toward @p last following aromatic (BOND_TYPE_ALTERN)
 * bonds. Every atom visited before @p last must be an aromatic CH carbon
 * (carbon, valence 2, chem_bonds_valence 3, one implicit H) joined to the
 * next atom by an aromatic bond. Succeeds only if the walk reaches @p last
 * with exactly @p len atoms validated.
 *
 * @param at   Atom array.
 * @param cur  Index of the first atom of the chain.
 * @param from Index of the atom preceding @p cur (fixes the walk direction).
 * @param last Index of the terminal atom the chain must reach.
 * @param len  Required chain length (number of validated atoms).
 * @return Non-zero if a valid aromatic chain of length @p len reaches @p last; 0 otherwise.
 */
int check_arom_chain( inp_ATOM *at, int cur, int from, int last, int len );

/**
 * @brief Restore resolved bond orders onto residual aromatic bonds from a reference.
 *
 * For every bond in @p at still typed above BOND_TRIPLE (an unresolved
 * type-4/aromatic bond), finds the same atom pair in reference array @p at2
 * by original atom number and copies the reference's concrete bond type onto
 * both endpoints in @p at.
 *
 * @param at         Atom array to fix up (modified in place).
 * @param num_atoms  Number of atoms in @p at.
 * @param at2        Reference atom array carrying resolved bond types.
 * @param num_atoms2 Number of atoms in @p at2.
 * @return Count of residual aromatic bonds that could not be resolved (0 on full success).
 */
int replace_arom_bonds( inp_ATOM *at, int num_atoms, inp_ATOM *at2, int num_atoms2 );

/**
 * @brief Mark alternating (aromatic) bonds via the balanced-network kekulizer.
 *
 * Thin convenience wrapper around mark_alt_bonds_and_taut_groups() invoked
 * with default (zeroed) tautomer flags and no fixed-bond/tautomer-group
 * output — i.e. normalize alternating bonds without collecting tautomer info.
 *
 * @param ic        InChI clock / timeout context.
 * @param pCG       Canonicalization globals.
 * @param at        Atom array to process (modified in place).
 * @param num_atoms Number of atoms in @p at.
 * @return Status from mark_alt_bonds_and_taut_groups() (0 on success, non-zero error code otherwise).
 */
int mark_arom_bonds( struct tagINCHI_CLOCK *ic, struct tagCANON_GLOBALS *pCG, inp_ATOM *at, int num_atoms );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* _AROMATICITY_H_ */
