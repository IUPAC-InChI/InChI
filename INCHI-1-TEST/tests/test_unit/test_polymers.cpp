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
  ACCLDraw08102614092D

 15 15  0  0  0  0  0  0  0  0999 V2000
    4.8260   -6.2294    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8489   -5.6388    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.8718   -6.2294    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.8718   -7.4105    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8489   -8.0010    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.8260   -7.4105    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8489   -4.4577    0.0000 C   0  0  3  0  0  0  0  0  0  0  0  0
    4.8260   -3.8672    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
    6.8718   -3.8672    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.8946   -4.4577    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
    4.8260   -5.0483    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.8032   -4.4577    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.8260   -2.6861    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.8032   -2.0955    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.7803   -5.0483    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
  2  1  2  0  0  0  0
  3  2  1  0  0  0  0
  4  3  2  0  0  0  0
  5  4  1  0  0  0  0
  6  5  2  0  0  0  0
  1  6  1  0  0  0  0
  2  7  1  0  0  0  0
  7  8  1  0  0  0  0
  7  9  1  0  0  0  0
  9 10  1  0  0  0  0
  8 11  1  0  0  0  0
  8 12  1  0  0  0  0
  8 13  1  0  0  0  0
 13 14  1  0  0  0  0
 12 15  3  0  0  0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT
M  SAL   1  8   1   2   3   4   5   6   7   9
M  SBL   1  2   8  10
M  SDI   1  4    5.3375   -4.7530    5.3375   -3.5719
M  SDI   1  4    7.3832   -3.5719    7.3832   -4.7530
M  SMT   1 n
M  END
)";

    ExpectedPolymersInChI(molblock, "InChI=1B/C12H16NZz2/c1-3-15(2,10-13)12(9-14)11-7-5-4-6-8-11/h4-8,12H,3,9H2,1-2H3/z101-4-9,11-12(15-12,14-9)", 1);
}


/* GHI #253: Reject multivalent pseudoatoms that are not polymer caps. */
TEST(test_polymers, test_GHI_253_invalid_pseudoatom)
{
    const char* molblock = R"(
  GHI253 Negative Test

  8  6  0  0  0  0  0  0  0  0999 V2000
    5.9688   -4.0938    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    6.9916   -3.5032    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0148   -4.0939    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0380   -3.5032    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   10.0611   -4.0939    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   12.0000   -3.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.0000   -3.0000    0.0000 *   0  0  0  0  0  0  0  0  0  0  0  0
   14.0000   -3.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  6  7  1  0  0  0  0
  7  8  1  0  0  0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SCN  1   1 HT
M  SAL   1  3   3   2   4
M  SBL   1  2   1   4
M  SMT   1 n
M  END
)";

    char options[] = "-Polymers -NPZz";

    inchi_Output output = {};
    int ret = MakeINCHIFromMolfileText(molblock, options, &output);

    /* The unrelated multivalent pseudoatom must be rejected. */
    EXPECT_NE(ret, inchi_Ret_OKAY);
    EXPECT_NE(ret, inchi_Ret_WARNING);

    const char* expected_error = "Invalid pseudo element(s) bonding";

    bool has_expected_error =
        (output.szMessage && std::strstr(output.szMessage, expected_error) != nullptr) ||
        (output.szLog && std::strstr(output.szLog, expected_error) != nullptr);

    EXPECT_TRUE(has_expected_error)
        << "Return code: " << ret
        << "\nMessage: " << (output.szMessage ? output.szMessage : "(null)")
        << "\nLog: " << (output.szLog ? output.szLog : "(null)");

    FreeINCHI(&output);
}
