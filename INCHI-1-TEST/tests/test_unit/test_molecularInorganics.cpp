#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}


/* (NaumanUllahKhan :: @nnuk)
*  below are some tests with MI (MolecularInorganics) tags specific molfiles that go through the MI code pipeline.
*/
TEST(test_molecularInorganics, test_MI_1_VOF3)
{
    const char* molblock = R"(
  ACCLDraw12222500412D

  5  4  0  0  0  0  0  0  0  0999 V2000
    6.6078   -5.5530    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
    7.6307   -4.9625    0.0000 V   0  0  0  0  0  0  0  0  0  0  0  0
    7.6307   -3.7814    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    8.4658   -5.7976    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
    8.7715   -5.2681    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  2  0  0  0  0
  2  4  1  0  0  0  0
  2  5  1  0  0  0  0
M  END
)";

    char options[] = "-MolecularInorganics";
    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    const char expected_inchi[] = "InChI=1B/F3OV/c1-5(2,3)4";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_molecularInorganics, test_MI_2_FeF4)
{
    const char* molblock = R"(
  ACCLDraw11252412092D

  5  4  0  0  0  0  0  0  0  0999 V2000
   19.0241  -15.5577    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   17.0325  -15.0985    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   17.9929  -14.5328    0.0000 Fe  0  0  0  0  0  0  0  0  0  0  0  0
   19.3285  -14.6295    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   17.9928  -13.3513    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  3  1  1  0  0  0  0
  3  2  1  0  0  0  0
  3  4  1  0  0  0  0
  3  5  1  0  0  0  0
M  END
)";

    char options[] = "-MolecularInorganics";
    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    const char expected_inchi[] = "InChI=1B/F4Fe/c1-5(2,3)4";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_molecularInorganics, test_MI_3_FeF3)
{
    const char* molblock = R"(
  ACCLDraw11252412082D

  4  3  0  0  0  0  0  0  0  0999 V2000
   17.9929  -15.7140    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   16.9700  -13.9423    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   17.9929  -14.5328    0.0000 Fe  0  0  0  0  0  0  0  0  0  0  0  0
   19.0160  -13.9420    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  3  1  1  0  0  0  0
  3  2  1  0  0  0  0
  3  4  1  0  0  0  0
M  END
)";

    char options[] = "-MolecularInorganics";
    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    const char expected_inchi[] = "InChI=1B/3FH.Fe/h3*1H;/q;;;+3/p-3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_molecularInorganics, test_MI_4_FeCl3)
{
    const char* molblock = R"(
  ACCLDraw11212415402D

  4  3  0  0  0  0  0  0  0  0999 V2000
    5.3438   -5.7436    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0
    4.3209   -3.9719    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0
    6.3666   -3.9719    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0
    5.3438   -4.5625    0.0000 Fe  0  0  0  0  0  0  0  0  0  0  0  0
  4  1  1  0  0  0  0
  4  2  1  0  0  0  0
  4  3  1  0  0  0  0
M  END
)";

    char options[] = "-MolecularInorganics";
    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    const char expected_inchi[] = "InChI=1B/Cl3Fe/c1-4(2)3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_molecularInorganics, test_MI_5_hydrido_dimethyl_iron)
{
    const char* molblock = R"(hydrido(dimethyl)iron
  ACCLDraw11182517382D

  4  3  0  0  0  0  0  0  0  0999 V2000
    6.0938   -6.6250    0.0000 Fe  0  0  0  0  0  0  0  0  0  0  0  0
    7.1166   -6.0344    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    5.0709   -6.0344    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.0938   -7.8061    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  1  3  1  0  0  0  0
  1  4  1  0  0  0  0
M  END
)";

    char options[] = "-MolecularInorganics";
    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    const char expected_inchi[] = "InChI=1B/C2H7Fe/c1-3(2)4/h1-2H3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_molecularInorganics, test_MI_6_Pt_haptic)
{
    const char* molblock = R"(
  ACCLDraw04282514232D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 7 5 0 0 0
M  V30 BEGIN ATOM
M  V30 1 Cl 7.4847 -4.7161 0 0
M  V30 2 Cl 8.8623 -5.8977 0 0
M  V30 3 Cl 7.5377 -7.0884 0 0
M  V30 4 Pt 7.5222 -5.8893 0 0 CHG=-1
M  V30 5 * 5.9997 -5.9174 0 0
M  V30 6 C 5.9061 -5.327 0 0
M  V30 7 C 5.9061 -6.5078 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 4 1
M  V30 2 1 4 2
M  V30 3 1 4 3
M  V30 4 9 4 5 ENDPTS=(2 6 7) ATTACH=ALL
M  V30 5 2 7 6
M  V30 END BOND
M  V30 END CTAB
M  END
)";

    char options[] = "-MolecularInorganics";
    inchi_Output output;
    inchi_Output* poutput = &output;
    memset(poutput, 0, sizeof(*poutput));

    const char expected_inchi[] = "InChI=1B/C2H4Cl3Pt/c3-6(4,5)1-2-6/h1-2H2/q-1";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}
