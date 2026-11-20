#include <string>

#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

static std::string GetOutputText(const inchi_Output &output)
{
    std::string text;
    if (output.szMessage)
    {
        text += output.szMessage;
    }
    if (output.szLog)
    {
        text += output.szLog;
    }
    return text;
}

TEST(test_isotope, rejects_hydrogen_isotope_above_tritium)
{
    const char *molfile = R"mol(
  -ClnMol-06180618052D

  8  8  0  0  0  0  0  0  0  0999 V2000
    7.1910  -10.9218    0.0000 H   0  0  0  0  0  0  0  0  0  0  0
    6.9205   -7.7208    0.0000 H   0  0  0  0  0  0  0  0  0  0  0
   15.9713  -11.1810    0.0000 H   0  0  0  0  0  0  0  0  0  0  0
   16.2418   -7.9913    0.0000 H   0  0  0  0  0  0  0  0  0  0  0
   11.1810  -11.4515    0.0000 H   0  0  0  0  0  2  0  0  0  0  0  0
   11.7108   -7.7208    0.0000 H   4  0  0  0  0  2  0  0  0  0  0  0
    9.0508   -9.3213    0.0000 B   0  0  0  0  0  4  0  0  0  0  0  0
   13.8410   -9.3213    0.0000 B   0  0  0  0  0  4  0  0  0  0  0  0
  6  7  1  0  0  0  0
  6  8  1  0  0  0  0
  5  7  1  0  0  0  0
  5  8  1  0  0  0  0
  4  8  1  0  0  0  0
  3  8  1  0  0  0  0
  2  7  1  0  0  0  0
  1  7  1  0  0  0  0
M  ISO  1   6   5
M  END
)mol";
    char options[] = "";
    inchi_Output output = {};

    const int ret = MakeINCHIFromMolfileText(molfile, options, &output);

    EXPECT_EQ(ret, inchi_Ret_ERROR);
    EXPECT_TRUE(output.szInChI == nullptr || output.szInChI[0] == '\0');
    EXPECT_NE(
        GetOutputText(output).find("Unacceptable isotope of hydrogen"),
        std::string::npos
    );

    FreeINCHI(&output);
}
