#include <gtest/gtest.h>
#include <fstream>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}

TEST(test_atropisomers, test_Atropisomers_molfile_v2)
{
    const char *molblock =
        "test_mol_2																  \n"
        "  Ketcher  1302610202D 1   1.00000     0.00000     0                     \n"
        "                                                                         \n"
        " 13 12  0  0  0  0  0  0  0  0999 V2000                                  \n"
        "    2.9420   -4.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    3.8080   -3.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    4.6740   -4.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.5401   -3.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.4061   -4.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.2721   -3.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    8.1381   -4.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.4061   -5.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.5401   -2.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.5401   -5.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.5401   -6.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.4061   -2.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.4061   -1.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "  1  2  1  0     0  0                                                    \n"
        "  2  3  1  0     0  0                                                    \n"
        "  3  4  1  0     0  0                                                    \n"
        "  4  5  1  0     0  0                                                    \n"
        "  5  6  1  0     0  0                                                    \n"
        "  6  7  1  0     0  0                                                    \n"
        "  5  8  1  0     0  0                                                    \n"
        "  4  9  1  0     0  0                                                    \n"
        "  8 10  1  0     0  0                                                    \n"
        " 10 11  1  0     0  0                                                    \n"
        "  9 12  1  0     0  0                                                    \n"
        " 12 13  1  0     0  0                                                    \n"
        "M  END                                                                   \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C13H28/c1-5-9-12(8-4)13(10-6-2)11-7-3/h12-13H,5-11H2,1-4H3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}
