#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

TEST(inchi_dll_b_testing, test_MakeINCHIFromMolfileText)
{
    const char *ferrocene =
        "Ferrocene\n"
        "  ACCLDraw12122514483D\n"
        "\n"
        "  0  0  0     0  0            999 V3000\n"
        "M  V30 BEGIN CTAB\n"
        "M  V30 COUNTS 13 12 0 0 0\n"
        "M  V30 BEGIN ATOM\n"
        "M  V30 1 C 19.44 -22.4142 -0.1542 0 \n"
        "M  V30 2 C 19.8251 -23.417 0.6958 0 \n"
        "M  V30 3 C 21.204 -23.4477 0.7167 0 \n"
        "M  V30 4 C 21.6716 -22.4638 -0.0917 0 \n"
        "M  V30 5 C 20.5725 -21.854 -0.7083 0 CHG=-1 CFG=3 \n"
        "M  V30 6 * 20.4688 -22.625 0 0 \n"
        "M  V30 7 Fe 20.4134 -20.7234 0 0 \n"
        "M  V30 8 * 20.5 -19.1875 0 0 \n"
        "M  V30 9 C 20.5767 -18.3331 -0.7083 0 CHG=-1 CFG=3 \n"
        "M  V30 10 C 19.44 -18.8933 -0.1542 0 \n"
        "M  V30 11 C 19.8251 -19.8962 0.6958 0 \n"
        "M  V30 12 C 21.6716 -18.943 -0.0917 0 \n"
        "M  V30 13 C 21.204 -19.9269 0.7167 0 \n"
        "M  V30 END ATOM\n"
        "M  V30 BEGIN BOND\n"
        "M  V30 1 2 1 2 \n"
        "M  V30 2 1 2 3 \n"
        "M  V30 3 2 3 4 \n"
        "M  V30 4 1 4 5 \n"
        "M  V30 5 1 5 1 \n"
        "M  V30 6 9 7 6 ENDPTS=(5 5 1 2 4 3) ATTACH=ALL\n"
        "M  V30 7 9 7 8 ENDPTS=(5 9 10 11 12 13) ATTACH=ALL\n"
        "M  V30 8 1 9 10 \n"
        "M  V30 9 2 10 11 \n"
        "M  V30 10 1 12 9 \n"
        "M  V30 11 1 11 13 \n"
        "M  V30 12 2 13 12 \n"
        "M  V30 END BOND\n"
        "M  V30 END CTAB\n"
        "M  END";

    const char *ethanol =
        "Ethanol\n"
        " OpenBabel02172217462D\n"
        "\n"
        "  0  0  0     0  0            999 V3000\n"
        "M  V30 BEGIN CTAB\n"
        "M  V30 COUNTS 9 8 0 0 0\n"
        "M  V30 BEGIN ATOM\n"
        "M  V30 1 C -0.3811 1.01968 0 0\n"
        "M  V30 2 C 0.4439 0.194681 0 0\n"
        "M  V30 3 H -0.964464 1.60305 0 0\n"
        "M  V30 4 H -0.065386 1.78188 0 0\n"
        "M  V30 5 H 1.02726 -0.388681 0 0\n"
        "M  V30 6 H 1.2061 0.510396 0 0\n"
        "M  V30 7 H -1.2061 1.01968 0 0\n"
        "M  V30 8 O -1.20037 -1.36938 0 0\n"
        "M  V30 9 H -0.485901 -1.78188 0 0\n"
        "M  V30 END ATOM\n"
        "M  V30 BEGIN BOND\n"
        "M  V30 1 1 1 3\n"
        "M  V30 2 1 1 4\n"
        "M  V30 3 1 1 2\n"
        "M  V30 4 1 1 7\n"
        "M  V30 5 1 2 5\n"
        "M  V30 6 1 2 6\n"
        "M  V30 7 1 8 2\n"
        "M  V30 8 1 8 9\n"
        "M  V30 END BOND\n"
        "M  V30 END CTAB\n"
        "M  END\n";


    char options[] = "";
    inchi_Output output;
    inchi_Output *poutput = &output;

    // We match any error because the test crashes with different error across runs (i.e., flaky test).
    // ASSERT_DEATH(MakeINCHIFromMolfileText(ferrocene, options, poutput), ".*");
    ASSERT_EQ(MakeINCHIFromMolfileText(ferrocene, options, poutput), 0);

    // Works with other V3000 molfile.
    ASSERT_EQ(MakeINCHIFromMolfileText(ethanol, options, poutput), 0);
}
