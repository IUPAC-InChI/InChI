#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}

/*
 * Whether InChI fills a bare atom's normal valence with implicit hydrogens is
 * governed by the per-element bSkipAddingH flag in the default valence tables
 * (ElData in util.c, mirrored by MolecularInorganicsArray in strutil.c).
 *
 * Only the non-metals, metalloids and noble gases (H/D/T, He, B, C, N, O, F,
 * Ne, Si, P, S, Cl, Ar, Ge, As, Se, Br, Kr, Sb, Te, I, Xe, At, Rn) carry
 * bSkipAddingH == 0 and therefore receive implicit H; every metal -- main
 * group as well as transition -- is flagged to skip H addition, so a bare
 * metal atom yields just the element with no attached hydrogens.
 *
 * These tests pin that behaviour down end-to-end by generating the standard
 * InChI for a single, bond-free atom of each element.
 */

namespace
{

struct SingleAtomCase
{
    const char* element;        /* atom symbol as written in the molfile      */
    const char* expected_inchi; /* expected standard InChI for the lone atom  */
    int expected_ret = 0;       /* MakeINCHIFromMolfileText return code        */
                                /* (1 when the saturated hydride disconnects)  */
};

/* Generate the standard InChI for one bond-free atom of `element` and assert
   it equals `expected_inchi`. The molfile mirrors the minimal single-atom
   V2000 layout used elsewhere in the suite. */
void ExpectSingleAtomInChI(const char* element, const char* expected_inchi,
                           int expected_ret = 0)
{
    char molblock[256];
    snprintf(molblock, sizeof(molblock),
             "\n"
             "  test\n"
             "\n"
             "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
             "    0.0000    0.0000    0.0000 %-3s 0  0  0  0  0  0  0  0  0  0  0  0\n"
             "M  END\n",
             element);

    char options[] = "";
    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), expected_ret)
        << "element: " << element;
    EXPECT_STREQ(poutput->szInChI, expected_inchi)
        << "element: " << element;

    FreeINCHI(poutput);
}

void ExpectSingleAtomInChI(const SingleAtomCase& c)
{
    ExpectSingleAtomInChI(c.element, c.expected_inchi, c.expected_ret);
}

}  // namespace

/* Main-group metals: now flagged bSkipAddingH, so no implicit H is added.
   Previously these received their normal-valence H (e.g. Na -> "Na.H"). */
TEST(test_implicitHydrogen, mainGroupMetals_skipImplicitH)
{
    const SingleAtomCase cases[] = {
        /* alkali */          {"Li", "InChI=1S/Li"}, {"Na", "InChI=1S/Na"},
                              {"K", "InChI=1S/K"},   {"Rb", "InChI=1S/Rb"},
                              {"Cs", "InChI=1S/Cs"}, {"Fr", "InChI=1S/Fr"},
        /* alkaline earth */  {"Be", "InChI=1S/Be"}, {"Mg", "InChI=1S/Mg"},
                              {"Ca", "InChI=1S/Ca"}, {"Sr", "InChI=1S/Sr"},
                              {"Ba", "InChI=1S/Ba"}, {"Ra", "InChI=1S/Ra"},
        /* group 13 */        {"Al", "InChI=1S/Al"}, {"Ga", "InChI=1S/Ga"},
                              {"In", "InChI=1S/In"}, {"Tl", "InChI=1S/Tl"},
        /* heavier p-block */ {"Sn", "InChI=1S/Sn"}, {"Pb", "InChI=1S/Pb"},
                              {"Bi", "InChI=1S/Bi"}, {"Po", "InChI=1S/Po"},
    };

    for (const auto& c : cases)
    {
        ExpectSingleAtomInChI(c);
    }
}

/* Transition / noble metals were always flagged to skip H addition; included
   here as a control that their behaviour is unchanged. */
TEST(test_implicitHydrogen, transitionMetals_skipImplicitH)
{
    const SingleAtomCase cases[] = {
        {"Fe", "InChI=1S/Fe"}, {"Cu", "InChI=1S/Cu"}, {"Zn", "InChI=1S/Zn"},
        {"Pt", "InChI=1S/Pt"}, {"Au", "InChI=1S/Au"},
    };

    for (const auto& c : cases)
    {
        ExpectSingleAtomInChI(c);
    }
}

/* Non-metals and metalloids keep bSkipAddingH == 0, so a bare atom is still
   saturated to its normal valence with implicit H. Note Sb (valence 3) is
   reported as the disconnected "Sb.3H". */
TEST(test_implicitHydrogen, nonmetalsAndMetalloids_addImplicitH)
{
    const SingleAtomCase cases[] = {
        {"B", "InChI=1S/BH3/h1H3"},   {"C", "InChI=1S/CH4/h1H4"},
        {"N", "InChI=1S/H3N/h1H3"},   {"O", "InChI=1S/H2O/h1H2"},
        {"F", "InChI=1S/FH/h1H"},     {"Si", "InChI=1S/H4Si/h1H4"},
        {"P", "InChI=1S/H3P/h1H3"},   {"S", "InChI=1S/H2S/h1H2"},
        {"Cl", "InChI=1S/ClH/h1H"},   {"Ge", "InChI=1S/GeH4/h1H4"},
        {"As", "InChI=1S/AsH3/h1H3"}, {"Se", "InChI=1S/H2Se/h1H2"},
        {"Br", "InChI=1S/BrH/h1H"},   {"Sb", "InChI=1S/Sb.3H", 1},
        {"Te", "InChI=1S/H2Te/h1H2"}, {"I", "InChI=1S/HI/h1H"},
        {"At", "InChI=1S/AtH/h1H"},
    };

    for (const auto& c : cases)
    {
        ExpectSingleAtomInChI(c);
    }
}

/* Noble gases carry bSkipAddingH == 0 but have a zero normal valence, so no H
   is added regardless -- confirming the flag never forces unwanted hydrogens. */
TEST(test_implicitHydrogen, nobleGases_noImplicitH)
{
    const SingleAtomCase cases[] = {
        {"He", "InChI=1S/He"}, {"Ne", "InChI=1S/Ne"}, {"Ar", "InChI=1S/Ar"},
        {"Kr", "InChI=1S/Kr"}, {"Xe", "InChI=1S/Xe"}, {"Rn", "InChI=1S/Rn"},
    };

    for (const auto& c : cases)
    {
        ExpectSingleAtomInChI(c);
    }
}
