#include <cstring>
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

/* Guard bytes after the counts array that must stay untouched */
static const size_t NUM_GUARD = 8;

struct AmmoniumResult
{
    int ret;
    int iO;
    int k;
    S_CHAR buf[NUM_H_ISOTOPES + 1 + NUM_GUARD];
};

/* H4N-O-C with one explicit H of the given iso_atw_diff; call
   bIsAmmoniumSalt on the N. */
static AmmoniumResult CallWithIsoAtwDiff(int iso_atw_diff)
{
    enum { N, O, C, H1, H2, H3, H4, NUM_AT };
    inp_ATOM *at = CreateInpAtom(NUM_AT);
    AmmoniumResult r;

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
    at[H4].iso_atw_diff = (S_CHAR)iso_atw_diff;

    memset(r.buf, GUARD, sizeof(r.buf));
    r.iO = r.k = -1;
    r.ret = bIsAmmoniumSalt(at, N, &r.iO, &r.k, r.buf);

    FreeInpAtom(&at);

    return r;
}

static void ExpectGuardIntact(const AmmoniumResult &r)
{
    for (size_t j = NUM_H_ISOTOPES + 1; j < sizeof(r.buf); j++)
    {
        EXPECT_EQ(r.buf[j], GUARD) << "write past num_explicit_H at " << j;
    }
}

TEST(test_github_289, out_of_range_iso_atw_diff_stays_in_bounds)
{
    /* mass difference 6 (iso_atw_diff 7), as in the fuzzer input */
    AmmoniumResult r = CallWithIsoAtwDiff(7);

    ExpectGuardIntact(r);

    /* No disconnection; outputs untouched */
    EXPECT_EQ(r.ret, 0);
    EXPECT_EQ(r.iO, -1);
    EXPECT_EQ(r.k, -1);
}

TEST(test_github_289, iso_atw_diff_boundaries)
{
    /* 0 (non-isotopic) to NUM_H_ISOTOPES (T) are counted at their index;
       the N-O-C salt is recognised */
    for (int iso = 0; iso <= NUM_H_ISOTOPES; iso++)
    {
        AmmoniumResult r = CallWithIsoAtwDiff(iso);

        ExpectGuardIntact(r);
        EXPECT_EQ(r.ret, 1) << "iso_atw_diff " << iso;
        EXPECT_EQ(r.iO, 1) << "iso_atw_diff " << iso;
        EXPECT_EQ(r.k, 4) << "iso_atw_diff " << iso;
        for (int j = 0; j <= NUM_H_ISOTOPES; j++)
        {
            /* H1-H3 are non-isotopic, H4 has the isotope under test */
            EXPECT_EQ(r.buf[j], (j == 0 ? 3 : 0) + (j == iso)) << "iso_atw_diff " << iso << " index " << j;
        }
    }

    /* First value past T and a negative value are rejected */
    for (int iso : {NUM_H_ISOTOPES + 1, -1})
    {
        AmmoniumResult r = CallWithIsoAtwDiff(iso);

        ExpectGuardIntact(r);
        EXPECT_EQ(r.ret, 0) << "iso_atw_diff " << iso;
        EXPECT_EQ(r.iO, -1) << "iso_atw_diff " << iso;
        EXPECT_EQ(r.k, -1) << "iso_atw_diff " << iso;
    }
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
