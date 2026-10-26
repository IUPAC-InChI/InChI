#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/strutil.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"

    int bIsAmmoniumSalt(inp_ATOM *at, int i, int *piO, int *pk, S_CHAR *num_explicit_H);
}

static const S_CHAR GUARD = 0x5A;

static void Bond(inp_ATOM *at, int a, int b)
{
    at[a].neighbor[at[a].valence++] = (AT_NUMB)b;
    at[b].neighbor[at[b].valence++] = (AT_NUMB)a;
}

TEST(test_github_289, out_of_range_iso_atw_diff_stays_in_bounds)
{
    /* H4N-O-C, one explicit H with mass difference 6 (iso_atw_diff 7) */
    enum { N, O, C, H1, H2, H3, H4, NUM_AT };
    const int bad_iso_atw_diff = 7;
    inp_ATOM *at = CreateInpAtom(NUM_AT);

    at[N].el_number = EL_NUMBER_N;
    at[O].el_number = EL_NUMBER_O;
    at[C].el_number = EL_NUMBER_C;
    for (int h = H1; h <= H4; h++)
    {
        at[h].el_number = EL_NUMBER_H;
        Bond(at, N, h);
    }
    Bond(at, N, O);
    Bond(at, O, C);
    at[H4].iso_atw_diff = bad_iso_atw_diff;

    /* Counts array followed by guard bytes that must stay untouched */
    S_CHAR buf[NUM_H_ISOTOPES + 1 + 8];
    memset(buf, GUARD, sizeof(buf));

    int iO = -1, k = -1;
    bIsAmmoniumSalt(at, N, &iO, &k, buf);

    for (size_t j = NUM_H_ISOTOPES + 1; j < sizeof(buf); j++)
    {
        EXPECT_EQ(buf[j], GUARD) << "write past num_explicit_H at " << j;
    }

    FreeInpAtom(&at);
}

TEST(test_github_289, fuzzer_molfile_does_not_overflow)
{
    /* Minimized fuzzer input from GH #289: N with an explicit H of mass difference 6 */
    const char molfile[] =
        "\n"
        "\n"
        "\n"
        "  42\n"
        " 3                            FF\n"
        "                              SB\n"
        "               \x0c              FH1 6\n"
        "                              FN\x0b\x0b\x0b             5\n"
        "1  4  1\n"
        "3  4  1";
    char options[] = "";
    inchi_Output output = {};

    MakeINCHIFromMolfileText(molfile, options, &output);

    FreeINCHI(&output);
}
