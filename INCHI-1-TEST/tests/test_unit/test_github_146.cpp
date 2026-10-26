#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

/* GitHub issue #146: V3000 polymer molfile must give the V2000 InChI */

static const char EXPECTED_INCHI[] =
    "InChI=1B/C2H4OZz2/c4-2-1-3-5/h1-2H2/z101-1-3(5-3,4-2)";

static void ExpectInChI(const char *molblock)
{
    char options[] = "-Polymers";
    inchi_Output output = {};

    int ret = MakeINCHIFromMolfileText(molblock, options, &output);

    EXPECT_EQ(ret, inchi_Ret_OKAY);
    ASSERT_NE(output.szInChI, nullptr) << (output.szMessage ? output.szMessage : "");
    EXPECT_STREQ(output.szInChI, EXPECTED_INCHI);

    FreeINCHI(&output);
}

TEST(test_github_146, polyethyleneoxide_v2000)
{
    const char *molblock =
        "\n"
        "  ACCLDraw07292516342D\n"
        "\n"
        "  5  4  0  0  0  0  0  0  0  0999 V2000\n"
        "    4.5313   -7.7188    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    5.5541   -7.1282    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    6.5773   -7.7189    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    7.6005   -7.1282    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    8.6236   -7.7189    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "  1  2  1  0  0  0  0\n"
        "  2  3  1  0  0  0  0\n"
        "  3  4  1  0  0  0  0\n"
        "  4  5  1  0  0  0  0\n"
        "M  STY  1   1 SRU\n"
        "M  SLB  1   1   1\n"
        "M  SCN  1   1 HT \n"
        "M  SAL   1  3   3   2   4\n"
        "M  SBL   1  2   1   4\n"
        "M  SDI   1  4    5.0427   -8.0142    5.0427   -6.8328\n"
        "M  SDI   1  4    8.1120   -6.8329    8.1120   -8.0142\n"
        "M  SMT   1 n\n"
        "M  END\n";

    ExpectInChI(molblock);
}

TEST(test_github_146, polyethyleneoxide_v3000)
{
    const char *molblock =
        "\n"
        "  ACCLDraw07292516342D\n"
        "\n"
        "  0  0  0     0  0            999 V3000\n"
        "M  V30 BEGIN CTAB\n"
        "M  V30 COUNTS 5 4 1 0 0\n"
        "M  V30 BEGIN ATOM\n"
        "M  V30 1 * 4.5313 -7.7188 0 0 \n"
        "M  V30 2 C 5.5541 -7.1282 0 0 \n"
        "M  V30 3 C 6.5773 -7.7189 0 0 \n"
        "M  V30 4 O 7.6005 -7.1282 0 0 \n"
        "M  V30 5 * 8.6236 -7.7189 0 0 \n"
        "M  V30 END ATOM\n"
        "M  V30 BEGIN BOND\n"
        "M  V30 1 1 1 2 \n"
        "M  V30 2 1 2 3 \n"
        "M  V30 3 1 3 4 \n"
        "M  V30 4 1 4 5 \n"
        "M  V30 END BOND\n"
        "M  V30 BEGIN SGROUP\n"
        "M  V30 1 SRU 1 ATOMS=(3 3 2 4) XBONDS=(2 1 4) BRKXYZ=(9 5.04 -8.01 0 5.04 -\n"
        "M  V30 -6.83 0 0 0 0) BRKXYZ=(9 8.11 -6.83 0 8.11 -8.01 0 0 0 0) -\n"
        "M  V30 CONNECT=HT LABEL=n \n"
        "M  V30 END SGROUP\n"
        "M  V30 END CTAB\n"
        "M  END\n";

    ExpectInChI(molblock);
}
