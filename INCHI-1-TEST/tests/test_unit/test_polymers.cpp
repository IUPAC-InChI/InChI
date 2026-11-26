#include <gtest/gtest.h>

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

/* GitHub issue # 254 */
TEST(test_polymers, test_GHI_254)
{
    const char* molblock = R"(
  ACCLDraw08102614222D

 16 15  0  0  0  0  0  0  0  0999 V2000
    2.7188   -5.4688    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
    3.7419   -6.0595    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    4.7651   -5.4688    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.7883   -6.0595    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.8114   -5.4688    0.0000 C   0  0  3  0  0  0  0  0  0  0  0  0
    7.8346   -6.0595    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.8578   -5.4688    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.8809   -6.0595    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   10.9041   -5.4688    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
    4.7651   -4.2873    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    6.8114   -4.2873    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.8114   -6.6502    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    5.7883   -3.6966    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    7.8346   -3.6966    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    7.8346   -2.5151    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
    8.8578   -4.2873    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  1  0  0  0  0
  7  8  1  0  0  0  0
  8  9  1  0  0  0  0
  3 10  2  0  0  0  0
  5 11  1  0  0  0  0
  5 12  1  0  0  0  0
 11 13  2  0  0  0  0
 11 14  1  0  0  0  0
 14 15  1  0  0  0  0
  7 16  2  0  0  0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT
M  SAL   1 13   3   2   4   5   6   7   8  16  11  13  14  12  10
M  SBL   1  3   1  14   8
M  SDI   1  4    3.2303   -6.3548    3.2303   -5.1734
M  SDI   1  4    7.2439   -3.1058    8.4253   -3.1058
M  SDI   1  4   10.3925   -5.1734   10.3925   -6.3548
M  SMT   1 n
M  END
)";

    ExpectedPolymersInChI(molblock, "InChI=1B/C6H5O7Zz3/c7-3(11-14)1-6(10,5(9)13-16)2-4(8)12-15/h10H,1-2H2/z101-1-13(14-11,15-12,16-13)", 0);
}

/* GitHub issue # 254 - Four crossing bonds (example 01) */
TEST(test_polymers, test_GHI_254_FourCrossingBonds_01)
{
    const char* molblock = R"(
  ACCLDraw11252614342D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 12 11 1 0 0
M  V30 BEGIN ATOM
M  V30 1 O 6.2813 -5.5313 0 0
M  V30 2 C 7.3041 -4.9407 0 0 CFG=3
M  V30 3 O 7.3041 -3.7592 0 0
M  V30 4 C 8.3273 -5.5314 0 0 CFG=3
M  V30 5 C 9.3505 -4.9407 0 0 CFG=3
M  V30 6 O 8.3273 -6.7129 0 0
M  V30 7 C 9.3505 -3.7592 0 0
M  V30 8 O 10.3736 -5.5314 0 0
M  V30 9 * 5.2581 -4.9405 0 0
M  V30 10 * 11.3968 -4.9407 0 0
M  V30 11 * 9.3505 -7.3036 0 0
M  V30 12 * 6.2809 -3.1685 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 2 4
M  V30 4 1 4 5
M  V30 5 1 4 6
M  V30 6 1 5 7
M  V30 7 1 5 8
M  V30 8 1 1 9
M  V30 9 1 8 10
M  V30 10 1 6 11
M  V30 11 1 3 12
M  V30 END BOND
M  V30 BEGIN SGROUP
M  V30 1 SRU 1 ATOMS=(8 2 1 3 4 5 7 8 6) XBONDS=(4 8 11 10 9) BRKXYZ=(9 5.77 -
M  V30 -5.83 0 5.77 -4.65 0 0 0 0) BRKXYZ=(9 6.79 -4.05 0 6.79 -2.87 0 -
M  V30 0 0 0) BRKXYZ=(9 8.84 -6.42 0 8.84 -7.6 0 0 0 0) BRKXYZ=(9 -
M  V30 10.89 -4.65 0 10.89 -5.83 0 0 0 0) CONNECT=HT LABEL=n
M  V30 END SGROUP
M  V30 END CTAB
M  END
)";

    ExpectedPolymersInChI(molblock, "InChI=1B/C4H6O4Zz4/c1-2(5-9)3(6-10)4(7-11)8-12/h2-4H,1H3/z101-1-8(9-5,10-6,11-7,12-8)", 1);
}

/* GitHub issue # 254 - Four crossing bonds (example 02) */
TEST(test_polymers, test_GHI_254_FourCrossingBonds_02)
{
    const char* molblock = R"(
  ACCLDraw11252614402D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 12 11 1 0 0
M  V30 BEGIN ATOM
M  V30 1 O 4.2595 -5.8957 0 0
M  V30 2 C 5.2824 -5.3052 0 0 CFG=3
M  V30 3 * 3.2367 -5.3052 0 0
M  V30 4 C 6.3052 -5.8957 0 0 CFG=3
M  V30 5 C 7.3281 -5.3052 0 0 CFG=3
M  V30 6 O 8.351 -5.8957 0 0
M  V30 7 * 9.3738 -5.3052 0 0
M  V30 8 O 5.2824 -4.1241 0 0
M  V30 9 C 6.3052 -7.0769 0 0
M  V30 10 O 7.3281 -4.1241 0 0
M  V30 11 * 4.2595 -3.5335 0 0
M  V30 12 * 8.351 -3.5335 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 1 3
M  V30 3 1 2 4
M  V30 4 1 4 5
M  V30 5 1 5 6
M  V30 6 1 6 7
M  V30 7 1 2 8
M  V30 8 1 4 9
M  V30 9 1 5 10
M  V30 10 1 8 11
M  V30 11 1 10 12
M  V30 END BOND
M  V30 BEGIN SGROUP
M  V30 1 SRU 1 ATOMS=(8 2 8 4 9 5 10 6 1) XBONDS=(4 2 10 11 6) BRKXYZ=(9 3.75 -
M  V30 -6.19 0 3.75 -5.01 0 0 0 0) BRKXYZ=(9 4.77 -4.42 0 4.77 -3.24 0 -
M  V30 0 0 0) BRKXYZ=(9 7.84 -3.24 0 7.84 -4.42 0 0 0 0) BRKXYZ=(9 -
M  V30 8.86 -5.01 0 8.86 -6.19 0 0 0 0) CONNECT=HT LABEL=n
M  V30 END SGROUP
M  V30 END CTAB
M  END
)";

    ExpectedPolymersInChI(molblock, "InChI=1B/C4H6O4Zz4/c1-2(3(5-9)6-10)4(7-11)8-12/h2-4H,1H3/z101-1-8(9-5,10-6,11-7,12-8)", 0);
}
