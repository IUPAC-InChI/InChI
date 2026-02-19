#include <gtest/gtest.h>
#include <fstream>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}

TEST(test_atropisomers, test_dummy)
{
    const char *molblock =
        "test mol                                                                  \n"
        "  Ketcher  2192614182D 1   1.00000     0.00000     0                      \n"
        "                                                                          \n"
        " 16 17  0  0  1  0  0  0  0  0999 V2000                                   \n"
        "    6.7160   -7.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    5.8500   -7.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    4.9840   -7.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    4.9840   -8.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    5.8500   -9.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    6.7160   -8.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    6.7160   -5.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    5.8500   -6.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    4.9840   -5.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    4.9840   -4.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    6.7160   -4.9750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    5.8500   -4.4750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    7.5821   -6.4750    0.0000 Br  0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    4.1179   -6.4750    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    4.1179   -7.4749    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    7.5821   -7.4750    0.0000 Br  0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "  1  6  1  0  0  0  0                                                     \n"
        "  1  2  2  0  0  0  0                                                     \n"
        "  2  3  1  1  0  0  0                                                     \n"
        "  2  8  1  0  0  0  0                                                     \n"
        "  3  4  2  0  0  0  0                                                     \n"
        "  5  4  1  0  0  0  0                                                     \n"
        "  5  6  2  0  0  0  0                                                     \n"
        "  8  7  1  1  0  0  0                                                     \n"
        "  7 11  2  0  0  0  0                                                     \n"
        "  8  9  2  0  0  0  0                                                     \n"
        "  9 10  1  0  0  0  0                                                     \n"
        " 10 12  2  0  0  0  0                                                     \n"
        " 12 11  1  0  0  0  0                                                     \n"
        "  7 13  1  0  0  0  0                                                     \n"
        "  9 14  1  0  0  0  0                                                     \n"
        "  3 15  1  0  0  0  0                                                     \n"
        "  1 16  1  0  0  0  0                                                     \n"
        "M  END                                                                    \n";

    char options[] = "-Atropisomers";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}
