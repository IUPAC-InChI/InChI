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
