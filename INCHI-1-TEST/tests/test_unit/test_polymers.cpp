#include <gtest/gtest.h>
#include <cstring>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}

/*
* (NaumanUllahKhan :: @nnuk)
* helper function to run the molfiles below as unit tests.
*/
static void ExpectedPolymersInChI(const char* molblock,
                                  const char* expected_inchi,
                                  int expected_retcode)
{
    char options[] = "-Polymers -NPZz";

    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    int ret = MakeINCHIFromMolfileText(molblock, options, poutput);

    EXPECT_EQ(ret, expected_retcode);
    EXPECT_NE(poutput->szInChI, nullptr);

    if (poutput->szInChI)
    {
        EXPECT_STREQ(poutput->szInChI, expected_inchi);
    }

    FreeINCHI(poutput);
}

/* GitHub issue # 252 */
TEST(test_polymers, test_GHI_252)
{
    const char* molblock = R"(
  ACCLDraw08102613492D

  5  4  0  0  0  0  0  0  0  0999 V2000
    5.9688   -4.0938    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    6.9916   -3.5032    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0148   -4.0939    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0380   -3.5032    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   10.0611   -4.0939    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT
M  SAL   1  3   3   2   4
M  SBL   1  2   1   4
M  SDI   1  4    6.4802   -4.3892    6.4802   -3.2078
M  SDI   1  4    9.5495   -3.2079    9.5495   -4.3892
M  SMT   1 n
M  END
)";

    ExpectedPolymersInChI(molblock, "InChI=1B/C2H6O2/c3-1-2-4-5/h3H,1-2H2/z101-1-2,4(5-4,3-1)", 0);
}


/* GitHub issue #253: Polymer with defined and undefined end groups */
TEST(test_polymers, test_GHI_253)
{
    const char* molblock = R"(
  ACCLDraw11172613002D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 15 15 1 0 0
M  V30 BEGIN ATOM
M  V30 1 N 6.2533 -6.4213 0 0
M  V30 2 C 7.2759 -3.4694 0 0
M  V30 3 C 8.2984 -4.0598 0 0
M  V30 4 C 7.2759 -5.8309 0 0
M  V30 5 C 8.2984 -6.4213 0 0
M  V30 6 * 11.366 -5.8309 0 0
M  V30 7 C 10.3435 -5.2406 0 0
M  V30 8 C 8.2984 -5.2406 0 0 CFG=3
M  V30 9 C 9.3209 -5.8309 0 0 CFG=3
M  V30 10 C 8.2984 -8.7828 0 0
M  V30 11 C 9.3209 -9.3731 0 0
M  V30 12 C 10.3435 -8.7828 0 0
M  V30 13 C 10.3435 -7.6021 0 0
M  V30 14 C 9.3209 -7.0116 0 0
M  V30 15 C 8.2984 -7.6021 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 3 4 1
M  V30 2 1 3 2
M  V30 3 1 8 3
M  V30 4 1 8 4
M  V30 5 1 8 5
M  V30 6 1 7 6
M  V30 7 1 9 7
M  V30 8 1 9 8
M  V30 9 1 14 9
M  V30 10 1 15 10
M  V30 11 2 10 11
M  V30 12 1 11 12
M  V30 13 2 12 13
M  V30 14 1 13 14
M  V30 15 2 14 15
M  V30 END BOND
M  V30 BEGIN SGROUP
M  V30 1 SRU 1 ATOMS=(8 15 14 13 12 11 10 9 7) XBONDS=(2 8 6) BRKXYZ=(9 8.81 -
M  V30 -6.13 0 8.81 -4.95 0 0 0 0) BRKXYZ=(9 10.85 -4.95 0 10.85 -6.13 -
M  V30 0 0 0 0) CONNECT=HT LABEL=n
M  V30 END SGROUP
M  V30 END CTAB
M  END
)";

    ExpectedPolymersInChI(molblock, "InChI=1B/C13H16NZz/c1-3-13(2,10-14)12(9-15)11-7-5-4-6-8-11/h4-8,12H,3,9H2,1-2H3/z101-4-9,11-12(13-12,15-9)", 1);
}
