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
 * @brief π-electron contribution of a ring atom to its conjugated system.
 *
 * Hückel counting for the electron-source cases this module handles: a cationic
 * center donates 0 (empty p orbital), an anionic center donates 2 (in-ring lone
 * pair), a doublet-radical center donates 1 (SOMO), and any other neutral atom
 * donates 1 (its share of a ring double bond). Heteroatom-specific lone-pair
 * rules (pyridine/pyrrole/furan) are deferred to a later increment.
 *
 * @param at Atom array.
 * @param i  Index of the ring atom.
 * @return Number of π electrons the atom donates (0, 1, or 2).
 */
int huckel_pi_contribution( inp_ATOM *at, int i );

/**
 * @brief Test whether a ring atom is a Hückel electron source needing relaxation.
 *
 * True when atom @p i cannot carry a localized ring double bond and therefore
 * blocks kekulization of an odd aromatic ring: its ring coordination
 * (@c valence minus the number of metal neighbors) is 2, it has no implicit H,
 * its @c chem_bonds_valence exceeds its @c valence (spare valence left by the
 * unresolved aromatic system), and it is either charged or a doublet radical.
 * Metal neighbors are counted, not excluded, so connected organometallic rings
 * (e.g. ferrocene Cp) qualify.
 *
 * @param at Atom array.
 * @param i  Index of the candidate ring atom.
 * @return 1 if @p i is an electron source to relax; 0 otherwise.
 */
int is_aromatic_electron_source( inp_ATOM *at, int i );

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
 * @brief Relax Hückel electron-source ring atoms so an odd aromatic ring kekulizes.
 *
 * For every atom for which @c is_aromatic_electron_source is true, moves one
 * valence unit from an assumed ring double bond to an implicit hydrogen
 * (@c chem_bonds_valence-- paired with @c num_H++). Total valence and formula
 * are preserved; charge and radical state are untouched. Intended to run only
 * after the balanced-network kekulizer returns @c BNS_ALTBOND_ERR; the caller
 * rebuilds the network and retries the conversion once.
 *
 * @param at        Atom array (modified in place).
 * @param num_atoms Number of atoms in @p at.
 * @return Number of atoms relaxed (0 if none matched).
 */
int relax_aromatic_electron_sources( inp_ATOM *at, int num_atoms );

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
