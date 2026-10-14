#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

TEST(test_inchi_dll_b, test_MakeINCHIFromMolfileText)
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

    char options[] = "";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1BB/C10H10Fe/c1-2-4-5-3(1)11(1,2,4,5)6-7(11)9(11)10(11)8(6)11/h1-10H/q-2";

    ASSERT_EQ(MakeINCHIFromMolfileText(ferrocene, options, poutput), 1);
    ASSERT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_inchi_dll_b, test_GetStructFromINCHI_rejects_negative_isotope_mass)
{
    char inchi[] = "InChI=1/T/q+1/i12-112T";
    inchi_InputINCHI input = {};
    input.szInChI = inchi;
    inchi_OutputStruct output = {};

    EXPECT_EQ(GetStructFromINCHI(&input, &output), inchi_Ret_ERROR);
    EXPECT_EQ(output.num_atoms, 0);

    FreeStructFromINCHI(&output);
}

TEST(test_inchi_dll_b, test_CheckINCHI_version_prefixes)
{
    EXPECT_EQ(CheckINCHI("InChI=1S/H2O/h1H2", 0), INCHI_VALID_STANDARD);
    EXPECT_EQ(CheckINCHI("InChI=1SB/H2O/h1H2", 0), INCHI_VALID_STANDARD);

    EXPECT_EQ(CheckINCHI("InChI=1B/H2O/h1H2", 0), INCHI_VALID_BETA);
    EXPECT_EQ(CheckINCHI("InChI=1BB/H2O/h1H2", 0), INCHI_VALID_BETA);

    EXPECT_EQ(CheckINCHI("InChI=1N/H2O/h1H2", 0), INCHI_VALID_NON_STANDARD);
    EXPECT_EQ(CheckINCHI("InChI=1NB/H2O/h1H2", 0), INCHI_VALID_NON_STANDARD);

    /* Historical non-standard prefix remains accepted as input. */
    EXPECT_EQ(CheckINCHI("InChI=1/H2O/h1H2", 0), INCHI_VALID_NON_STANDARD);
}

TEST(test_inchi_dll_b, test_CheckINCHIKey_version_flags)
{
    EXPECT_EQ(CheckINCHIKey("FAPWRFPIFSIZLT-UHFFFAOYSA-M"), INCHIKEY_VALID_STANDARD);
    EXPECT_EQ(CheckINCHIKey("FAPWRFPIFSIZLT-UHFFFAOYSB-M"), INCHIKEY_VALID_STANDARD);

    EXPECT_EQ(CheckINCHIKey("FAPWRFPIFSIZLT-UHFFFAOYBA-M"), INCHIKEY_VALID_NON_STANDARD);
    EXPECT_EQ(CheckINCHIKey("FAPWRFPIFSIZLT-UHFFFAOYBB-M"), INCHIKEY_VALID_NON_STANDARD);

    EXPECT_EQ(CheckINCHIKey("FAPWRFPIFSIZLT-UHFFFAOYNA-M"), INCHIKEY_VALID_NON_STANDARD);
    EXPECT_EQ(CheckINCHIKey("FAPWRFPIFSIZLT-UHFFFAOYNB-M"), INCHIKEY_VALID_NON_STANDARD);
}

TEST(test_inchi_dll_b, test_GetINCHIKeyFromINCHI_version_flags)
{
    struct TestCase
    {
        const char* inchi;
        char expected_class;
        char expected_subversion;
    };

    const TestCase cases[] =
    {
        { "InChI=1S/H2O/h1H2",  'S', 'A' },
        { "InChI=1SB/H2O/h1H2", 'S', 'B' },
        { "InChI=1B/H2O/h1H2",  'B', 'A' },
        { "InChI=1BB/H2O/h1H2", 'B', 'B' },
        { "InChI=1N/H2O/h1H2",  'N', 'A' },
        { "InChI=1NB/H2O/h1H2", 'N', 'B' }
    };

    for (const auto& test_case : cases)
    {
        char key[28] = { 0 };

        ASSERT_EQ(GetINCHIKeyFromINCHI(test_case.inchi, 0, 0, key, nullptr, nullptr), INCHIKEY_OK) << test_case.inchi;

        EXPECT_EQ(key[23], test_case.expected_class) << test_case.inchi;

        EXPECT_EQ(key[24], test_case.expected_subversion) << test_case.inchi;
    }
}

TEST(test_inchi_dll_b, test_MakeINCHIFromMolfileText_LegacyMetalHandling)
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

    char options[] = "-LegacyMetalHandling";
    inchi_Output output;
    inchi_Output *poutput = &output;

    const char expected_inchi[] = "InChI=1N/2C5H5.Fe/c2*1-2-4-5-3-1;/h2*1-5H;/q2*-1;";

    memset(poutput, 0, sizeof(*poutput));

    ASSERT_EQ(MakeINCHIFromMolfileText(ferrocene, options, poutput), 1);

    ASSERT_NE(poutput->szInChI, nullptr);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}
