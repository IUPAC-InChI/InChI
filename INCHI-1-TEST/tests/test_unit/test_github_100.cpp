#include <gtest/gtest.h>

#include <clocale>
#include <cmath>
#include <cstring>
#include <functional>
#include <string>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

// GH #100: InChI/AuxInfo must not depend on the caller's LC_NUMERIC.
// Each test computes a result under "C" and under a comma-decimal locale.

static const char COMMA_LOCALE[] = "de_DE.UTF-8";

// 2,3-epoxybutane from the GH #100 report, with one wedge
static const char EPOXYBUTANE_V2000[] =
    "\n"
    "  test\n"
    "\n"
    "  5  5  0  0  0  0  0  0  0  0999 V2000\n"
    "   -1.1270   -0.5635    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.4125   -0.1510    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.4125   -0.1510    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.1270   -0.5635    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000    0.5635    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  2  1  1  1\n"
    "  2  3  1  0\n"
    "  3  4  1  0\n"
    "  2  5  1  0\n"
    "  3  5  1  0\n"
    "M  END\n";

static const char EPOXYBUTANE_V3000[] =
    "\n"
    "  test\n"
    "\n"
    "  0  0  0     0  0            999 V3000\n"
    "M  V30 BEGIN CTAB\n"
    "M  V30 COUNTS 5 5 0 0 0\n"
    "M  V30 BEGIN ATOM\n"
    "M  V30 1 C -1.127 -0.5635 0 0\n"
    "M  V30 2 C -0.4125 -0.151 0 0\n"
    "M  V30 3 C 0.4125 -0.151 0 0\n"
    "M  V30 4 C 1.127 -0.5635 0 0\n"
    "M  V30 5 O 0 0.5635 0 0\n"
    "M  V30 END ATOM\n"
    "M  V30 BEGIN BOND\n"
    "M  V30 1 1 2 1 CFG=1\n"
    "M  V30 2 1 2 3\n"
    "M  V30 3 1 3 4\n"
    "M  V30 4 1 2 5\n"
    "M  V30 5 1 3 5\n"
    "M  V30 END BOND\n"
    "M  V30 END CTAB\n"
    "M  END\n";

// Runs fn under the given locale and restores the previous one.
static std::string RunInLocale(const char *name, const std::function<std::string()> &fn)
{
    std::string saved = setlocale(LC_ALL, nullptr);
    if (!setlocale(LC_ALL, name))
    {
        return "";
    }

    std::string result = fn();
    setlocale(LC_ALL, saved.c_str());
    return result;
}

#define SKIP_WITHOUT_COMMA_LOCALE()                                   \
    do                                                                \
    {                                                                 \
        std::string saved = setlocale(LC_ALL, nullptr);               \
        if (!setlocale(LC_ALL, COMMA_LOCALE))                         \
        {                                                             \
            GTEST_SKIP() << COMMA_LOCALE << " locale not installed";  \
        }                                                             \
        setlocale(LC_ALL, saved.c_str());                             \
    } while (0)

static std::string MolfileToInchiAux(const char *molfile)
{
    inchi_Output output = {};
    char options[] = "";

    MakeINCHIFromMolfileText(molfile, options, &output);
    std::string result = std::string(output.szInChI ? output.szInChI : "") + "\n" +
                         (output.szAuxInfo ? output.szAuxInfo : "");
    FreeINCHI(&output);
    return result;
}

TEST(test_github_100, molfile_v2000_aux_info_is_locale_independent)
{
    SKIP_WITHOUT_COMMA_LOCALE();

    auto run = [] { return MolfileToInchiAux(EPOXYBUTANE_V2000); };
    std::string expected = RunInLocale("C", run);

    EXPECT_NE(expected.find("/rC:-1.127,-.5635,0;"), std::string::npos) << expected;
    EXPECT_EQ(RunInLocale(COMMA_LOCALE, run), expected);
}

TEST(test_github_100, molfile_v3000_aux_info_is_locale_independent)
{
    SKIP_WITHOUT_COMMA_LOCALE();

    auto run = [] { return MolfileToInchiAux(EPOXYBUTANE_V3000); };
    std::string expected = RunInLocale("C", run);

    EXPECT_NE(expected.find("/rC:-1.127,-.5635,0;"), std::string::npos) << expected;
    EXPECT_EQ(RunInLocale(COMMA_LOCALE, run), expected);
}

TEST(test_github_100, api_atoms_aux_info_is_locale_independent)
{
    SKIP_WITHOUT_COMMA_LOCALE();

    // GetINCHI path: coordinates arrive as doubles (CDK / jna-inchi)
    auto run = []
    {
        inchi_Atom atoms[2] = {};
        inchi_Input input = {};
        inchi_Output output = {};
        char options[] = "";

        strcpy(atoms[0].elname, "C");
        atoms[0].x = -1.127;
        atoms[0].y = -0.5635;
        atoms[0].num_bonds = 1;
        atoms[0].neighbor[0] = 1;
        atoms[0].bond_type[0] = INCHI_BOND_TYPE_SINGLE;
        atoms[0].num_iso_H[0] = -1;

        strcpy(atoms[1].elname, "O");
        atoms[1].x = 0.4125;
        atoms[1].y = 0.151;
        atoms[1].num_iso_H[0] = -1;

        input.atom = atoms;
        input.num_atoms = 2;
        input.szOptions = options;

        GetINCHI(&input, &output);
        std::string result = output.szAuxInfo ? output.szAuxInfo : "";
        FreeINCHI(&output);
        return result;
    };
    std::string expected = RunInLocale("C", run);

    EXPECT_NE(expected.find("/rC:-1.127,-.5635,0;.4125,.151,0;"), std::string::npos) << expected;
    EXPECT_EQ(RunInLocale(COMMA_LOCALE, run), expected);
}

TEST(test_github_100, aux_info_coordinates_parse_in_comma_locale)
{
    SKIP_WITHOUT_COMMA_LOCALE();

    // Get_inchi_Input_FromAuxInfo path: AuxInfo /rC: is read back into doubles
    auto run = []
    {
        char aux[] = "AuxInfo=1/0/N:1,2,3/rA:3nCCO/rB:s1;s2;/rC:-1.127,-.5635,0;.4125,.151,0;1.5,2.25,0;";
        inchi_Input input = {};
        InchiInpData data = {};

        data.pInp = &input;
        Get_inchi_Input_FromAuxInfo(aux, 0, 0, &data);
        std::string result;
        for (int i = 0; i < input.num_atoms; i++)
        {
            result += std::to_string(std::llround(input.atom[i].x * 10000)) + "," +
                      std::to_string(std::llround(input.atom[i].y * 10000)) + ";";
        }
        Free_inchi_Input(&input);
        return result;
    };
    std::string expected = RunInLocale("C", run);

    EXPECT_EQ(expected, "-11270,-5635;4125,1510;15000,22500;");
    EXPECT_EQ(RunInLocale(COMMA_LOCALE, run), expected);
}
