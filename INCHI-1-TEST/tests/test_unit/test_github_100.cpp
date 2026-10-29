#include <gtest/gtest.h>

#include <clocale>
#include <cmath>
#include <cstring>
#include <functional>
#include <string>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/util.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inpdef.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichimain.h"
}

// GH #100: InChI/AuxInfo must not depend on the caller's LC_NUMERIC.
// Each test computes a result under "C" and under a comma-decimal locale.

// Comma-decimal locale names: glibc/macOS spellings, then MSVC ones
static const char *const COMMA_LOCALES[] = {"de_DE.UTF-8", "de_DE.utf8", "de_DE", "de-DE",
                                            "German_Germany.1252"};

// Locales whose decimal point is the multi-byte U+066B (Arabic decimal separator)
static const char *const MULTIBYTE_LOCALES[] = {"ps_AF.UTF-8", "ps_AF.utf8", "ps_AF"};

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

// First name in names that loads and really uses dp as decimal point, or nullptr.
// musl accepts any name but keeps '.', so the name alone proves nothing.
// Restores the previous locale.
template <size_t N>
static const char *FirstLocale(const char *const (&names)[N], const char *dp)
{
    std::string saved = setlocale(LC_ALL, nullptr);
    for (const char *name : names)
    {
        if (!setlocale(LC_ALL, name))
        {
            continue;
        }
        bool usable = !strcmp(localeconv()->decimal_point, dp);
        setlocale(LC_ALL, saved.c_str());
        if (usable)
        {
            return name;
        }
    }
    return nullptr;
}

static const char *CommaLocale()
{
    return FirstLocale(COMMA_LOCALES, ",");
}

static const char *MultibyteLocale()
{
    return FirstLocale(MULTIBYTE_LOCALES, "\xD9\xAB"); // U+066B in UTF-8
}

// glibc can install any locale (see INCHI-1-TEST/install_build_dependencies.sh),
// so a missing one is a setup error there. musl and others lack locale support.
#ifdef __GLIBC__
#define MISSING_LOCALE(msg) FAIL() << msg << ", run INCHI-1-TEST/install_build_dependencies.sh"
#else
#define MISSING_LOCALE(msg) GTEST_SKIP() << msg
#endif

#define REQUIRE_COMMA_LOCALE()                                       \
    do                                                               \
    {                                                                \
        if (!CommaLocale())                                          \
        {                                                            \
            MISSING_LOCALE("no comma-decimal locale installed");     \
        }                                                            \
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
    REQUIRE_COMMA_LOCALE();

    auto run = [] { return MolfileToInchiAux(EPOXYBUTANE_V2000); };
    std::string expected = RunInLocale("C", run);

    EXPECT_NE(expected.find("/rC:-1.127,-.5635,0;"), std::string::npos) << expected;
    EXPECT_EQ(RunInLocale(CommaLocale(), run), expected);
}

TEST(test_github_100, molfile_v3000_aux_info_is_locale_independent)
{
    REQUIRE_COMMA_LOCALE();

    auto run = [] { return MolfileToInchiAux(EPOXYBUTANE_V3000); };
    std::string expected = RunInLocale("C", run);

    EXPECT_NE(expected.find("/rC:-1.127,-.5635,0;"), std::string::npos) << expected;
    EXPECT_EQ(RunInLocale(CommaLocale(), run), expected);
}

TEST(test_github_100, api_atoms_aux_info_is_locale_independent)
{
    REQUIRE_COMMA_LOCALE();

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
    EXPECT_EQ(RunInLocale(CommaLocale(), run), expected);
}

TEST(test_github_100, aux_info_coordinates_parse_in_comma_locale)
{
    REQUIRE_COMMA_LOCALE();

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
    EXPECT_EQ(RunInLocale(CommaLocale(), run), expected);
}

// Polyethyleneoxide from GH #146, with SRU bracket coordinates (SDI)
static const char PEO_V2000[] =
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

TEST(test_github_100, polymer_bracket_coordinates_are_locale_independent)
{
    REQUIRE_COMMA_LOCALE();

    auto run = []
    {
        inchi_Output output = {};
        char options[] = "-Polymers";

        MakeINCHIFromMolfileText(PEO_V2000, options, &output);
        std::string result = output.szAuxInfo ? output.szAuxInfo : "";
        FreeINCHI(&output);
        return result;
    };
    std::string expected = RunInLocale("C", run);

    EXPECT_NE(expected.find("[5.042700,-8.014200,5.042700,-6.832800]"), std::string::npos) << expected;
    EXPECT_EQ(RunInLocale(CommaLocale(), run), expected);
}

// A numeric token longer than any fixed copy buffer must still parse completely.
TEST(test_github_100, strtod_reads_long_token_in_comma_locale)
{
    REQUIRE_COMMA_LOCALE();

    std::string token = std::string(100, '0') + "1.5";
    std::string text = token + "x";
    double val = 0;
    const char *end = nullptr;
    RunInLocale(CommaLocale(), [&]
    {
        val = inchi_strtod(text.c_str(), &end);
        return "";
    });

    EXPECT_EQ(val, 1.5);
    EXPECT_EQ(end - text.c_str(), (long) token.size());
}

// Multi-byte decimal point: parse stops at the right byte, formatting keeps the field width.
TEST(test_github_100, multibyte_decimal_point_is_handled)
{
    if (!MultibyteLocale())
    {
        MISSING_LOCALE("no multi-byte decimal-point locale installed");
    }

    double val = 0;
    const char *text = "1.5x";
    const char *end = nullptr;
    char buf[32] = "";
    RunInLocale(MultibyteLocale(), [&]
    {
        val = inchi_strtod(text, &end);
        snprintf(buf, sizeof(buf), "%10.4f%10.4f", 1.5, -2.25);
        inchi_dot_decimal(buf);
        return "";
    });

    EXPECT_EQ(val, 1.5);
    EXPECT_EQ(end - text, 3);
    EXPECT_STREQ(buf, "    1.5000   -2.2500");
}

// The SDF writer must print coordinates with '.', so its output reads back anywhere.
static std::string MolfileToSdf(const char *molfile)
{
    INCHI_IOSTREAM input{}, output{};
    ORIG_ATOM_DATA atom_data{};
    INCHI_MODE flags = 0;
    int err = 0;

    inchi_ios_init(&input, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input, "%s", molfile);
    CreateOrigInpDataFromMolfile(&input, &atom_data, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr,
                                 nullptr, &flags, &err, nullptr, 0);

    inchi_ios_init(&output, INCHI_IOS_TYPE_STRING, nullptr);
    OrigAtData_WriteToSDfile(&atom_data, &output, nullptr, nullptr, 0, 0, nullptr, nullptr);
    std::string result = output.s.pStr ? output.s.pStr : "";

    inchi_ios_free_str(&input);
    inchi_ios_free_str(&output);
    FreeOrigAtData(&atom_data);
    return result;
}

TEST(test_github_100, sdf_writer_is_locale_independent)
{
    REQUIRE_COMMA_LOCALE();

    auto run = [] { return MolfileToSdf(EPOXYBUTANE_V2000); };
    std::string expected = RunInLocale("C", run);

    EXPECT_NE(expected.find("   -1.1270   -0.5635    0.0000 C"), std::string::npos) << expected;
    EXPECT_EQ(RunInLocale(CommaLocale(), run), expected);
}
