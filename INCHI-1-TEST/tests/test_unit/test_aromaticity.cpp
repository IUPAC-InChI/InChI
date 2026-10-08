#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inpdef.h"   // inp_ATOM, CreateInpAtom, FreeInpAtom
#include "../../../INCHI-1-SRC/INCHI_BASE/src/util.h"     // EL_NUMBER_C, EL_NUMBER_N, EL_NUMBER_O
#include "../../../INCHI-1-SRC/INCHI_BASE/src/aromaticity.h" // functions under test
}

// ---- fix_aromatic_oxygen_and_sulfur ---------------------------------------

TEST(test_aromaticity, fix_aromatic_oxygen_and_sulfur_charges_neutral_aromatic_O)
{
    inp_ATOM a{};
    a.elname[0] = 'O';
    a.elname[1] = '\0';
    a.valence = 2;
    a.charge = 0;
    a.radical = 0;
    a.bond_type[0] = BOND_TYPE_SINGLE; // 1
    a.bond_type[1] = BOND_TYPE_DOUBLE; // 2  -> sum == 3

    EXPECT_EQ(fix_aromatic_oxygen_and_sulfur(&a), 1);
    EXPECT_EQ(a.charge, 1);
}

TEST(test_aromaticity, fix_aromatic_oxygen_and_sulfur_ignores_charged_atom)
{
    inp_ATOM a{};
    a.elname[0] = 'S';
    a.elname[1] = '\0';
    a.valence = 2;
    a.charge = 1;   // already charged
    a.radical = 0;
    a.bond_type[0] = BOND_TYPE_SINGLE;
    a.bond_type[1] = BOND_TYPE_DOUBLE;

    EXPECT_EQ(fix_aromatic_oxygen_and_sulfur(&a), 0);
    EXPECT_EQ(a.charge, 1); // untouched
}

TEST(test_aromaticity, fix_aromatic_oxygen_and_sulfur_ignores_non_O_S)
{
    inp_ATOM a{};
    a.elname[0] = 'N';
    a.elname[1] = '\0';
    a.valence = 2;
    a.bond_type[0] = BOND_TYPE_SINGLE;
    a.bond_type[1] = BOND_TYPE_DOUBLE;

    EXPECT_EQ(fix_aromatic_oxygen_and_sulfur(&a), 0);
    EXPECT_EQ(a.charge, 0);
}

// ---- is_C_unsat_not_arom ----------------------------------------------

TEST(test_aromaticity, is_C_unsat_not_arom_true_for_C_with_double_bond)
{
    // C(=C)  -- one double bond, no aromatic bonds
    inp_ATOM *at = CreateInpAtom(2);
    at[0].el_number = EL_NUMBER_C;
    at[0].valence = 1;
    at[0].chem_bonds_valence = 2; // one double bond
    at[0].num_H = 2;              // chem_bonds_valence + num_H == 4
    at[0].charge = 0;
    at[0].radical = 0;
    at[0].neighbor[0] = 1;
    at[0].bond_type[0] = BOND_TYPE_DOUBLE;
    at[1].el_number = EL_NUMBER_C;

    EXPECT_EQ(is_C_unsat_not_arom(at, 0), 1);
    FreeInpAtom(&at);
}

TEST(test_aromaticity, is_C_unsat_not_arom_false_for_aromatic_C)
{
    // aromatic carbon: has a BOND_TYPE_ALTERN bond -> not counted as "not arom"
    inp_ATOM *at = CreateInpAtom(2);
    at[0].el_number = EL_NUMBER_C;
    at[0].valence = 1;
    at[0].chem_bonds_valence = 2;
    at[0].num_H = 2;
    at[0].neighbor[0] = 1;
    at[0].bond_type[0] = BOND_TYPE_ALTERN;
    at[1].el_number = EL_NUMBER_C;

    EXPECT_EQ(is_C_unsat_not_arom(at, 0), 0);
    FreeInpAtom(&at);
}

// ---- is_Aryl ----------------------------------------------------------

TEST(test_aromaticity, is_Aryl_true_for_aromatic_attachment_carbon)
{
    // attachment C (idx 0), outside point idx 3; two BOND_ALTERN bonds to C/N
    inp_ATOM *at = CreateInpAtom(4);
    at[0].el_number = EL_NUMBER_C;
    at[0].valence = 3;
    at[0].chem_bonds_valence = 4;
    at[0].num_H = 0;
    at[0].charge = 0;
    at[0].radical = 0;
    at[0].neighbor[0] = 1; at[0].bond_type[0] = BOND_TYPE_ALTERN;
    at[0].neighbor[1] = 2; at[0].bond_type[1] = BOND_TYPE_ALTERN;
    at[0].neighbor[2] = 3; at[0].bond_type[2] = BOND_TYPE_SINGLE; // to outside point
    at[1].el_number = EL_NUMBER_C;
    at[2].el_number = EL_NUMBER_N;
    at[3].el_number = EL_NUMBER_C;

    EXPECT_EQ(is_Aryl(at, /*outside_point*/ 3, /*attachment_pont*/ 0), 1);
    FreeInpAtom(&at);
}

TEST(test_aromaticity, is_Aryl_false_for_wrong_valence)
{
    inp_ATOM *at = CreateInpAtom(2);
    at[0].el_number = EL_NUMBER_C;
    at[0].valence = 2;            // not 3
    at[0].chem_bonds_valence = 4;
    at[0].neighbor[0] = 1;

    EXPECT_EQ(is_Aryl(at, 1, 0), 0);
    FreeInpAtom(&at);
}

// ---- check_arom_chain -------------------------------------------------

TEST(test_aromaticity, check_arom_chain_true_for_altern_CH_chain)
{
    // chain: 0(start) -1- 2(last). Atoms 0,1 are aromatic CH (valence 2,
    // chem_bonds_valence 3, num_H 1) joined by BOND_ALTERN.
    // check_arom_chain's `num` only counts the atoms actually walked
    // through the do-while (0, then 1); reaching `last` (2) contributes
    // the final "++num" in the return statement. So len must equal the
    // total chain length INCLUDING `last`, i.e. 3 (atoms 0, 1, 2), not 2.
    inp_ATOM *at = CreateInpAtom(3);
    for (int k = 0; k < 2; k++)
    {
        at[k].el_number = EL_NUMBER_C;
        at[k].valence = 2;
        at[k].chem_bonds_valence = 3;
        at[k].num_H = 1;
    }
    // 0: neighbor[0]=from(2), neighbor[1]=next(1) -> i=(neighbor[0]==from)=1
    at[0].neighbor[0] = 2; at[0].bond_type[0] = BOND_TYPE_ALTERN;
    at[0].neighbor[1] = 1; at[0].bond_type[1] = BOND_TYPE_ALTERN;
    // 1: neighbor[0]=from(0), neighbor[1]=next(2) -> i=1
    at[1].neighbor[0] = 0; at[1].bond_type[0] = BOND_TYPE_ALTERN;
    at[1].neighbor[1] = 2; at[1].bond_type[1] = BOND_TYPE_ALTERN;
    at[2].el_number = EL_NUMBER_C; // 'last' - not checked for CH pattern

    EXPECT_EQ(check_arom_chain(at, /*cur*/ 0, /*from*/ 2, /*last*/ 2, /*len*/ 3), 1);
    FreeInpAtom(&at);
}

TEST(test_aromaticity, check_arom_chain_false_when_bond_not_altern)
{
    inp_ATOM *at = CreateInpAtom(3);
    for (int k = 0; k < 2; k++)
    {
        at[k].el_number = EL_NUMBER_C;
        at[k].valence = 2;
        at[k].chem_bonds_valence = 3;
        at[k].num_H = 1;
    }
    at[0].neighbor[0] = 2; at[0].bond_type[0] = BOND_TYPE_SINGLE; // wrong
    at[0].neighbor[1] = 1; at[0].bond_type[1] = BOND_TYPE_SINGLE;
    at[1].neighbor[0] = 0; at[1].bond_type[0] = BOND_TYPE_ALTERN;
    at[1].neighbor[1] = 2; at[1].bond_type[1] = BOND_TYPE_ALTERN;
    at[2].el_number = EL_NUMBER_C;

    // Same chain shape/len semantics as the true case (len == 3), but the
    // very first hop is a non-ALTERN bond, so check_arom_chain must bail
    // out immediately regardless of len.
    EXPECT_EQ(check_arom_chain(at, 0, 2, 2, 3), 0);
    FreeInpAtom(&at);
}

// ---- replace_arom_bonds -----------------------------------------------

TEST(test_aromaticity, replace_arom_bonds_copies_resolved_type_from_reference)
{
    // `at`: two atoms with a residual aromatic bond (type 4 > BOND_TRIPLE).
    inp_ATOM *at = CreateInpAtom(2);
    at[0].orig_at_number = 10; at[0].valence = 1;
    at[0].neighbor[0] = 1; at[0].bond_type[0] = BOND_TYPE_ALTERN; // > BOND_TRIPLE
    at[1].orig_at_number = 20; at[1].valence = 1;
    at[1].neighbor[0] = 0; at[1].bond_type[0] = BOND_TYPE_ALTERN;

    // `at2`: reference with the SAME atoms (matched by orig_at_number) where
    // the bond is resolved to DOUBLE.
    inp_ATOM *at2 = CreateInpAtom(2);
    at2[0].orig_at_number = 10; at2[0].valence = 1;
    at2[0].neighbor[0] = 1; at2[0].bond_type[0] = BOND_TYPE_DOUBLE;
    at2[1].orig_at_number = 20; at2[1].valence = 1;
    at2[1].neighbor[0] = 0; at2[1].bond_type[0] = BOND_TYPE_DOUBLE;

    EXPECT_EQ(replace_arom_bonds(at, 2, at2, 2), 0); // no errors
    EXPECT_EQ(at[0].bond_type[0], BOND_TYPE_DOUBLE); // copied over
    EXPECT_EQ(at[1].bond_type[0], BOND_TYPE_DOUBLE);

    FreeInpAtom(&at);
    FreeInpAtom(&at2);
}

TEST(test_aromaticity, replace_arom_bonds_counts_error_when_unmatched)
{
    inp_ATOM *at = CreateInpAtom(2);
    at[0].orig_at_number = 10; at[0].valence = 1;
    at[0].neighbor[0] = 1; at[0].bond_type[0] = BOND_TYPE_ALTERN;
    at[1].orig_at_number = 20; at[1].valence = 1;
    at[1].neighbor[0] = 0; at[1].bond_type[0] = BOND_TYPE_ALTERN;

    // reference has no atom with orig_at_number 20 -> cannot resolve
    inp_ATOM *at2 = CreateInpAtom(2);
    at2[0].orig_at_number = 10; at2[0].valence = 0;
    at2[1].orig_at_number = 99; at2[1].valence = 0;

    EXPECT_GT(replace_arom_bonds(at, 2, at2, 2), 0); // at least one error

    FreeInpAtom(&at);
    FreeInpAtom(&at2);
}

TEST(test_aromaticity, replace_arom_bonds_resolves_when_reference_indexing_differs)
{
    // replace_arom_bonds matches atoms between `at` and `at2` by
    // orig_at_number, so at[i1] and at2[i1] are generally DIFFERENT atoms.
    // The scan of at2[i1]'s neighbor list must therefore be bounded by
    // at2[i1].valence, not at[i1].valence. This case makes the two diverge:
    // in `at` each atom has one aromatic bond (valence 1), but the matching
    // reference atoms in `at2` have valence 2 with the cross-bond neighbor at
    // index 1 -- beyond at[i1].valence. With the wrong bound the neighbor is
    // never found and both bonds are left unresolved.

    inp_ATOM *at = CreateInpAtom(2);
    at[0].orig_at_number = 10; at[0].valence = 1;
    at[0].neighbor[0] = 1; at[0].bond_type[0] = BOND_TYPE_ALTERN; // > BOND_TRIPLE
    at[1].orig_at_number = 20; at[1].valence = 1;
    at[1].neighbor[0] = 0; at[1].bond_type[0] = BOND_TYPE_ALTERN;

    // Reference with different indexing/valence. i1 for orig 10 is index 0,
    // i1 for orig 20 is index 1; the cross-bond neighbor sits at position 1
    // in each (>= at[i1].valence == 1), and the resolved order there is DOUBLE.
    inp_ATOM *at2 = CreateInpAtom(3);
    at2[0].orig_at_number = 10; at2[0].valence = 2;
    at2[0].neighbor[0] = 2; at2[0].bond_type[0] = BOND_TYPE_SINGLE;
    at2[0].neighbor[1] = 1; at2[0].bond_type[1] = BOND_TYPE_DOUBLE; // bond to orig 20
    at2[1].orig_at_number = 20; at2[1].valence = 2;
    at2[1].neighbor[0] = 2; at2[1].bond_type[0] = BOND_TYPE_SINGLE;
    at2[1].neighbor[1] = 0; at2[1].bond_type[1] = BOND_TYPE_DOUBLE; // bond to orig 10
    at2[2].orig_at_number = 30; at2[2].valence = 2;
    at2[2].neighbor[0] = 0; at2[2].bond_type[0] = BOND_TYPE_SINGLE;
    at2[2].neighbor[1] = 1; at2[2].bond_type[1] = BOND_TYPE_SINGLE;

    EXPECT_EQ(replace_arom_bonds(at, 2, at2, 3), 0);  // both bonds resolved, no errors
    EXPECT_EQ(at[0].bond_type[0], BOND_TYPE_DOUBLE);  // restored from reference
    EXPECT_EQ(at[1].bond_type[0], BOND_TYPE_DOUBLE);

    FreeInpAtom(&at);
    FreeInpAtom(&at2);
}
