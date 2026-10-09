#include <gtest/gtest.h>
#include <fstream>
#include <cstring>
#include <chrono>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/extr_ct.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/atropisomers.h"
}

// Shared molblock fixtures (also used by the recall_* tests below).
static const char *k_dummy1_molblock =
    "atropisomer test mol                                                                  \n"
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

static const char *k_dummy12_molblock =
    "atropisomer test mol                                                      \n"
    "  Ketcher  2272615132D 1   1.00000     0.00000     0                      \n"
    "                                                                          \n"
    " 15 17  0  0  1  0  0  0  0  0999 V2000                                   \n"
    "    5.4357   -3.2991    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    7.1745   -3.2986    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    6.3067   -2.7965    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    7.1745   -4.3043    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    5.4357   -4.3088    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    6.3090   -4.8062    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    5.4357   -6.6150    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    7.1745   -6.6145    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    6.3067   -6.1125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    7.1745   -7.6203    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    5.4357   -7.6248    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    6.3090   -8.1222    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    8.0449   -6.1125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    8.0452   -4.8059    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "    8.2722   -5.4088    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
    "  3  1  2  0     0  0                                                     \n"
    "  1  5  1  0     0  0                                                     \n"
    "  5  6  2  0     0  0                                                     \n"
    "  6  4  1  1     0  0                                                     \n"
    "  4  2  2  0     0  0                                                     \n"
    "  9  7  1  1     0  0                                                     \n"
    "  7 11  2  0     0  0                                                     \n"
    " 12 10  2  0     0  0                                                     \n"
    " 10  8  1  0     0  0                                                     \n"
    "  8  9  2  0     0  0                                                     \n"
    "  6  9  1  0     0  0                                                     \n"
    "  8 13  1  0     0  0                                                     \n"
    "  4 14  1  0     0  0                                                     \n"
    " 14 15  1  0     0  0                                                     \n"
    " 13 15  1  0     0  0                                                     \n"
    "  3  2  1  1     0  0                                                     \n"
    " 12 11  1  1     0  0                                                     \n"
    "M  END                                                                    \n";

// Runs a molblock through MakeINCHIFromMolfileText with -EnhancedStereochemistry and
// reports whether the InChI carries a /t layer (the axis on a stereo-free
// skeleton).
static bool inchi_has_atrop_flag(const char *molblock) {
    inchi_Output out;
    memset(&out, 0, sizeof(out));
    char opts[] = "-EnhancedStereochemistry";
    MakeINCHIFromMolfileText(molblock, opts, &out);
    bool flagged = out.szInChI && strstr(out.szInChI, "/t") != nullptr;
    FreeINCHI(&out);
    return flagged;
}

TEST(test_atropisomers, find_atropisomeric_atoms_and_bonds__null_parameters) {

    int ret = find_atropisomeric_atoms_and_bonds(nullptr,
                                                 0,
                                                 nullptr);


    EXPECT_EQ(ret, 0);

}

TEST(test_atropisomers, find_atropisomeric_atoms_and_bonds__atoms_below_min_valence) {

    const int num_atoms = 2;
    inp_ATOM atoms[2] = {};

    atoms[0].valence      = 1;
    atoms[0].neighbor[0]  = 1;
    atoms[0].bond_type[0] = 1;
    atoms[0].x = 0.0; atoms[0].y = 0.0; atoms[0].z = 0.0;

    atoms[1].valence      = 1;
    atoms[1].neighbor[0]  = 0;
    atoms[1].bond_type[0] = 1;
    atoms[1].x = 1.5; atoms[1].y = 0.0; atoms[1].z = 0.0;

    ORIG_ATOM_DATA orig_data = {};

    int ret = find_atropisomeric_atoms_and_bonds(atoms, num_atoms, &orig_data);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(atoms[0].bAtropisomeric, 0);
    EXPECT_EQ(atoms[1].bAtropisomeric, 0);
    EXPECT_EQ(orig_data.bAtropisomer, 0);
}

TEST(test_atropisomers, test_dummy_1_atropisomer)
{
    const char *molblock = k_dummy1_molblock;

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_2_atropisomer)
{
    const char *molblock =
        "atropisomer test mol                                                     \n"
        "  Ketcher  2202610402D 1   1.00000     0.00000     0                     \n"
        "                                                                         \n"
        " 25 28  0  0  1  0  0  0  0  0999 V2000                                  \n"
        "    6.2229   -3.4324    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    8.0096   -3.6108    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.1896   -3.0101    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.8699   -4.5953    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.0956   -4.4735    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.9105   -5.0541    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.0587   -6.8139    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.8275   -6.8325    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.8777   -6.3069    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.9085   -7.7852    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.1259   -7.8397    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.0613   -8.3249    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    8.4672   -5.1980    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    8.4510   -6.2674    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    9.4062   -5.3893    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    9.4009   -6.0976    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.3049   -4.9122    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.2817   -6.5709    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.9561   -5.7358    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.5172   -3.9146    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.4810   -7.5491    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.4299   -2.8268    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    4.5119   -3.2022    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.2852   -8.3732    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    4.3939   -7.8951    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "  3  1  2  0     0  0                                                    \n"
        "  1  5  1  0     0  0                                                    \n"
        "  5  6  2  0     0  0                                                    \n"
        "  6  4  1  1     0  0                                                    \n"
        "  4  2  2  0     0  0                                                    \n"
        "  9  7  1  1     0  0                                                    \n"
        "  7 11  2  0     0  0                                                    \n"
        " 12 10  2  0     0  0                                                    \n"
        " 10  8  1  0     0  0                                                    \n"
        "  8  9  2  0     0  0                                                    \n"
        "  6  9  1  0     0  0                                                    \n"
        "  4 13  1  0     0  0                                                    \n"
        "  8 14  1  0     0  0                                                    \n"
        " 13 15  1  0     0  0                                                    \n"
        " 14 16  1  0     0  0                                                    \n"
        " 15 16  1  0     0  0                                                    \n"
        " 15 17  1  0     0  0                                                    \n"
        " 16 18  1  0     0  0                                                    \n"
        " 17 19  1  0     0  0                                                    \n"
        " 18 19  1  0     0  0                                                    \n"
        " 17 20  1  6     0  0                                                    \n"
        " 18 21  1  1     0  0                                                    \n"
        "  1 22  1  0     0  0                                                    \n"
        " 22 23  1  0     0  0                                                    \n"
        " 11 24  1  0     0  0                                                    \n"
        " 24 25  1  0     0  0                                                    \n"
        " 12 11  1  1     0  0                                                    \n"
        "  3  2  1  1     0  0                                                    \n"
        "M  END                                                                   \n";


    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C20H22O3/c1-21-17-5-3-13-7-15-11-23-12-16(15)8-14-4-6-18(22-2)10-20(14)19(13)9-17/h3-6,9-10,15-16H,7-8,11-12H2,1-2H3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_3_atropismer)
{
    const char *molblock =
        "atropisomer test mol                                                     \n"
        "  Ketcher  2202610482D 1   1.00000     0.00000     0                     \n"
        "                                                                         \n"
        " 22 25  0  0  1  0  0  0  0  0999 V2000                                  \n"
        "    2.8848   -2.2501    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    4.6152   -2.2496    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    3.7516   -1.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    4.6152   -3.2505    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    2.8848   -3.2550    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    3.7538   -3.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.4796   -1.7512    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.3468   -2.2515    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.4858   -3.7529    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.3490   -3.2475    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    2.8348   -5.3751    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    4.5652   -5.3746    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    3.7016   -4.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    4.5652   -6.3755    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    2.8348   -6.3800    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    3.7038   -6.8750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.4296   -4.8762    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.2968   -5.3765    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.4358   -6.8779    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.2990   -6.3725    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.2171   -3.7439    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.1622   -4.8755    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "  3  1  2  0     0  0                                                    \n"
        "  1  5  1  0     0  0                                                    \n"
        "  5  6  2  0     0  0                                                    \n"
        "  6  4  1  0     0  0                                                    \n"
        "  4  2  1  0     0  0                                                    \n"
        "  2  3  1  0     0  0                                                    \n"
        "  4  9  2  0     0  0                                                    \n"
        "  9 10  1  1     0  0                                                    \n"
        " 10  8  1  0     0  0                                                    \n"
        "  7  2  2  0     0  0                                                    \n"
        " 13 11  2  0     0  0                                                    \n"
        " 11 15  1  0     0  0                                                    \n"
        " 15 16  2  0     0  0                                                    \n"
        " 16 14  1  0     0  0                                                    \n"
        " 14 12  1  0     0  0                                                    \n"
        " 12 13  1  0     0  0                                                    \n"
        " 14 19  2  0     0  0                                                    \n"
        " 19 20  1  0     0  0                                                    \n"
        " 20 18  2  0     0  0                                                    \n"
        " 18 17  1  0     0  0                                                    \n"
        " 17 12  2  0     0  0                                                    \n"
        "  9 17  1  0     0  0                                                    \n"
        " 10 21  1  0     0  0                                                    \n"
        " 18 22  1  0     0  0                                                    \n"
        "  7  8  1  1     0  0                                                    \n"
        "M  END                                                                   \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C20H16O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-11,18,21-22H,12H2/t18?,19-/m0/s1";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_4_atypical_no_2_rings)
{
    const char *molblock =
        "atropisomer atypical no 2 rings                                                  \n"
        "  Ketcher  2272612 82D 1   1.00000     0.00000     0                             \n"
        "                                                                                 \n"
        " 24 25  0  0  1  0  0  0  0  0999 V2000                                          \n"
        "    6.7791   -6.8681    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    8.4871   -6.8676    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    7.6347   -6.3744    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    8.4871   -7.8557    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    6.7791   -7.8601    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    7.6369   -8.3488    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    8.8265   -3.8573    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    8.8265   -4.8444    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    9.6814   -5.3380    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "   10.5363   -4.8444    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "   10.5363   -3.8573    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    9.6814   -3.3637    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    7.6353   -5.3873    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    6.8053   -4.8531    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    6.8529   -3.8671    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    5.9276   -5.3048    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    5.0975   -4.7705    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    9.3422   -6.3745    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    5.9243   -6.3744    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    9.3422   -5.3874    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "   10.1971   -6.8680    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    5.4307   -7.2293    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    6.4179   -5.5196    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "    5.0694   -5.8809    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0            \n"
        "  3  1  1  1     0  0                                                            \n"
        "  1  5  2  0     0  0                                                            \n"
        "  5  6  1  0     0  0                                                            \n"
        "  6  4  2  0     0  0                                                            \n"
        "  4  2  1  0     0  0                                                            \n"
        "  2  3  2  0     0  0                                                            \n"
        " 12  7  1  0     0  0                                                            \n"
        "  7  8  1  0     0  0                                                            \n"
        "  8  9  1  0     0  0                                                            \n"
        "  9 10  1  0     0  0                                                            \n"
        " 10 11  1  0     0  0                                                            \n"
        " 11 12  1  0     0  0                                                            \n"
        "  3 13  1  0     0  0                                                            \n"
        " 13  8  1  0     0  0                                                            \n"
        " 13 14  1  6     0  0                                                            \n"
        " 14 15  2  0     0  0                                                            \n"
        " 14 16  1  0     0  0                                                            \n"
        " 16 17  2  0     0  0                                                            \n"
        "  2 18  1  0     0  0                                                            \n"
        "  1 19  1  0     0  0                                                            \n"
        " 18 20  2  0     0  0                                                            \n"
        " 18 21  1  0     0  0                                                            \n"
        " 22 19  1  0     0  0                                                            \n"
        " 19 23  1  0     0  0                                                            \n"
        " 19 24  1  0     0  0                                                            \n"
        "M  STY  1   1 SUP                                                                \n"
        "M  SLB  1   1   1                                                                \n"
        "M  SAP   1  1  18   0                                                            \n"
        "M  SAL   1  3  18  20  21                                                        \n"
        "M  SBL   1  1  19                                                                \n"
        "M  SMT   1 CONH2                                                                 \n"
        "M  STY  1   2 SUP                                                                \n"
        "M  SLB  1   2   2                                                                \n"
        "M  SAP   2  1  19   0                                                            \n"
        "M  SAL   2  4  19  22  23  24                                                    \n"
        "M  SBL   2  1  20                                                                \n"
        "M  SMT   2 tBu                                                                   \n"
        "M  END                                                                           \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C20H28N2O2/c1-5-17(23)22(14-10-7-6-8-11-14)18-15(19(21)24)12-9-13-16(18)20(2,3)4/h5,9,12-14H,1,6-8,10-11H2,2-4H3,(H2,21,24)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_5_no_atropisomer_no_wedge_bonds)
{
    const char *molblock =
        "atropisomer test mol                                                        \n"
        "  Ketcher  2202613492D 1   1.00000     0.00000     0                        \n"
        "                                                                            \n"
        " 33 37  0  0  0  0  0  0  0  0999 V2000                                     \n"
        "   -4.6637   -1.0934    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -3.9945   -1.8366    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -4.2024   -2.8147    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -3.3364   -3.3147    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -2.5933   -2.6456    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -3.0000   -1.7321    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -2.5000   -0.8660    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -3.0000   -0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -2.5000    0.8660    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -1.5000    0.8660    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -1.0000   -0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -1.5000   -0.8660    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    0.5000    0.8660    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    0.0000    1.7321    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -1.0000    1.7321    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    1.4781    0.6581    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    1.5827   -0.3364    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    2.4487   -0.8364    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    0.6691   -0.7431    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    0.6342   -1.7425    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -0.2487   -2.2120    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -0.2836   -3.2114    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    0.5644   -3.7413    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    1.4474   -3.2718    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    1.4823   -2.2725    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    2.3652   -1.8030    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    3.2133   -2.3329    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -1.0968   -1.6821    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    2.2213    1.3272    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -4.0000   -0.0000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -4.5000    0.8660    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "   -3.2319   -4.3092    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "  1  2  1  0     0  0                                                       \n"
        "  2  3  2  0     0  0                                                       \n"
        "  3  4  1  0     0  0                                                       \n"
        "  4  5  1  0     0  0                                                       \n"
        "  5  6  2  0     0  0                                                       \n"
        "  2  6  1  0     0  0                                                       \n"
        "  6  7  1  0     0  0                                                       \n"
        "  7  8  1  0     0  0                                                       \n"
        "  8  9  2  0     0  0                                                       \n"
        "  9 10  1  0     0  0                                                       \n"
        " 10 11  2  0     0  0                                                       \n"
        " 11 12  1  0     0  0                                                       \n"
        "  7 12  2  0     0  0                                                       \n"
        " 11 13  1  0     0  0                                                       \n"
        " 13 14  2  0     0  0                                                       \n"
        " 14 15  1  0     0  0                                                       \n"
        " 15 16  2  0     0  0                                                       \n"
        " 10 16  1  0     0  0                                                       \n"
        " 14 17  1  0     0  0                                                       \n"
        " 17 18  1  0     0  0                                                       \n"
        " 18 19  2  0     0  0                                                       \n"
        " 18 20  1  0     0  0                                                       \n"
        " 13 20  1  0     0  0                                                       \n"
        " 20 21  1  0     0  0                                                       \n"
        " 21 22  1  0     0  0                                                       \n"
        " 22 23  2  0     0  0                                                       \n"
        " 23 24  1  0     0  0                                                       \n"
        " 24 25  2  0     0  0                                                       \n"
        " 25 26  1  0     0  0                                                       \n"
        " 21 26  2  0     0  0                                                       \n"
        " 26 27  1  0     0  0                                                       \n"
        " 27 28  1  0     0  0                                                       \n"
        " 22 29  1  0     0  0                                                       \n"
        " 17 30  1  0     0  0                                                       \n"
        "  8 31  1  0     0  0                                                       \n"
        " 31 32  1  0     0  0                                                       \n"
        "  4 33  1  0     0  0                                                       \n"
        "M  END                                                                      \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C23H21FN6O3/c1-12-15(11-28(2)27-12)13-6-14-17(7-19(13)32-4)26-9-18-21(14)30(23(31)29(18)3)22-16(24)8-25-10-20(22)33-5/h6-11H,1-5H3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_6_two_atropisomer_bonds)
{
    const char *molblock =
        "test mol atropisomer                          \n"
        "  -INDIGO-02232612272D                        \n"
        "                                              \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000       \n"
        "M  V30 BEGIN CTAB                             \n"
        "M  V30 COUNTS 35 38 0 0 0                     \n"
        "M  V30 BEGIN ATOM                             \n"
        "M  V30 1 C 7.64682 -5.78999 0.0 0             \n"
        "M  V30 2 C 9.47015 -5.85956 0.0 0             \n"
        "M  V30 3 C 8.58333 -5.30104 0.0 0             \n"
        "M  V30 4 C 9.42738 -6.91249 0.0 0             \n"
        "M  V30 5 C 7.60313 -6.84054 0.0 0             \n"
        "M  V30 6 C 8.49613 -7.40423 0.0 0             \n"
        "M  V30 7 C 7.83065 -2.75985 0.0 0             \n"
        "M  V30 8 C 9.58077 -2.8408 0.0 0              \n"
        "M  V30 9 C 8.73115 -2.29671 0.0 0             \n"
        "M  V30 10 C 9.53681 -3.85648 0.0 0            \n"
        "M  V30 11 C 7.7836 -3.77279 0.0 0             \n"
        "M  V30 12 C 8.64006 -4.32505 0.0 0            \n"
        "M  V30 13 Br 6.90205 -7.15024 0.0 0           \n"
        "M  V30 14 Br 10.3254 -7.4481 0.0 0            \n"
        "M  V30 15 Br 6.93185 -4.22329 0.0 0           \n"
        "M  V30 16 Br 10.4302 -4.38621 0.0 0           \n"
        "M  V30 17 Cl 10.4134 -5.37898 0.0 0           \n"
        "M  V30 18 Cl 6.83923 -5.25149 0.0 0           \n"
        "M  V30 19 C 8.55343 -8.50701 0.0 0            \n"
        "M  V30 20 C 7.7261 -9.10432 0.0 0             \n"
        "M  V30 21 C 7.02177 -10.6261 0.0 0            \n"
        "M  V30 22 C 7.81179 -10.0708 0.0 0            \n"
        "M  V30 23 C 6.13959 -10.2141 0.0 0            \n"
        "M  V30 24 C 6.84915 -8.69665 0.0 0            \n"
        "M  V30 25 C 6.05278 -9.25201 0.0 0            \n"
        "M  V30 26 C 5.14001 -8.79074 0.0 0            \n"
        "M  V30 27 C 3.39085 -8.89047 0.0 0            \n"
        "M  V30 28 C 4.29687 -9.34501 0.0 0            \n"
        "M  V30 29 C 3.33353 -7.88488 0.0 0            \n"
        "M  V30 30 C 5.08172 -7.77981 0.0 0            \n"
        "M  V30 31 C 4.18147 -7.33019 0.0 0            \n"
        "M  V30 32 Br 5.94146 -7.18944 0.0 0           \n"
        "M  V30 33 Br 4.36521 -10.2925 0.0 0           \n"
        "M  V30 34 Cl 5.34468 -10.7571 0.0 0           \n"
        "M  V30 35 Cl 6.77442 -7.90166 0.0 0           \n"
        "M  V30 END ATOM                               \n"
        "M  V30 BEGIN BOND                             \n"
        "M  V30 1 1 3 1 CFG=1                          \n"
        "M  V30 2 2 1 5                                \n"
        "M  V30 3 1 5 6                                \n"
        "M  V30 4 2 6 4                                \n"
        "M  V30 5 1 4 2                                \n"
        "M  V30 6 2 2 3                                \n"
        "M  V30 7 2 9 7                                \n"
        "M  V30 8 1 7 11                               \n"
        "M  V30 9 2 11 12                              \n"
        "M  V30 10 1 12 10 CFG=1                       \n"
        "M  V30 11 2 10 8                              \n"
        "M  V30 12 1 8 9                               \n"
        "M  V30 13 1 12 3                              \n"
        "M  V30 14 1 5 13                              \n"
        "M  V30 15 1 4 14                              \n"
        "M  V30 16 1 11 15                             \n"
        "M  V30 17 1 10 16                             \n"
        "M  V30 18 1 2 17                              \n"
        "M  V30 19 1 1 18                              \n"
        "M  V30 20 1 6 19                              \n"
        "M  V30 21 1 19 20                             \n"
        "M  V30 22 2 22 20                             \n"
        "M  V30 23 1 20 24                             \n"
        "M  V30 24 2 24 25                             \n"
        "M  V30 25 1 25 23 CFG=1                       \n"
        "M  V30 26 2 23 21                             \n"
        "M  V30 27 1 21 22                             \n"
        "M  V30 28 1 25 26                             \n"
        "M  V30 29 2 28 26                             \n"
        "M  V30 30 1 26 30 CFG=1                       \n"
        "M  V30 31 2 30 31                             \n"
        "M  V30 32 1 31 29                             \n"
        "M  V30 33 2 29 27                             \n"
        "M  V30 34 1 27 28                             \n"
        "M  V30 35 1 30 32                             \n"
        "M  V30 36 1 28 33                             \n"
        "M  V30 37 1 23 34                             \n"
        "M  V30 38 1 24 35                             \n"
        "M  V30 END BOND                               \n"
        "M  V30 BEGIN COLLECTION                       \n"
        "M  V30 MDLV30/STEABS ATOMS=(4 3 12 25 26)     \n"
        "M  V30 END COLLECTION                         \n"
        "M  V30 END CTAB                               \n"
        "M  END                                        \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C25H10Br6Cl4/c26-12-3-1-4-13(27)17(12)19-16(32)8-7-10(23(19)33)9-11-21(30)24(34)20(25(35)22(11)31)18-14(28)5-2-6-15(18)29/h1-8H,9H2";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_7_no_atropisomer_1)
{
    const char *molblock =
        "non-atropisomer test mol                                                 \n"
        "  Ketcher  2202614 02D 1   1.00000     0.00000     0                     \n"
        "                                                                         \n"
        " 14 15  0  0  1  0  0  0  0  0999 V2000                                  \n"
        "   -0.6402   -0.4251    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.0902   -0.4246    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.2266    0.0750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.0902   -1.4255    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -0.6402   -1.4300    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.2288   -1.9250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.9546    0.0738    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    2.8218   -0.4265    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.9608   -1.9279    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    2.8240   -1.4225    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.9548    1.0738    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.2272    1.0750    0.0000 Br  0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.2327   -2.9250    0.0000 Br  0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.9640   -2.9279    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "  3  1  1  0     0  0                                                    \n"
        "  1  5  2  0     0  0                                                    \n"
        "  5  6  1  0     0  0                                                    \n"
        "  6  4  2  0     0  0                                                    \n"
        "  4  2  1  0     0  0                                                    \n"
        "  2  3  2  0     0  0                                                    \n"
        "  4  9  1  6     0  0                                                    \n"
        "  9 10  2  0     0  0                                                    \n"
        " 10  8  1  0     0  0                                                    \n"
        "  8  7  2  0     0  0                                                    \n"
        "  7 11  1  0     0  0                                                    \n"
        "  3 12  1  0     0  0                                                    \n"
        "  6 13  1  0     0  0                                                    \n"
        "  9 14  1  0     0  0                                                    \n"
        "  2  7  1  1     0  0                                                    \n"
        "M  END                                                                   \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C12H10Br2/c1-7-3-4-8(2)12-10(14)6-5-9(13)11(7)12/h3-6H,1-2H3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_8_no_atropisomer)
{
    const char *molblock =
        "no atropisomer                                                              \n"
        "  Ketcher  2272613152D 1   1.00000     0.00000     0                        \n"
        "                                                                            \n"
        " 19 22  0  0  1  0  0  0  0  0999 V2000                                     \n"
        "    1.7386   -5.4033    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    3.4728   -5.4028    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    2.6073   -4.9019    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    3.4728   -6.4059    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    1.7386   -6.4105    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    2.6095   -6.9066    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    4.3393   -4.9032    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    5.2084   -5.4047    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    4.3455   -6.9095    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    5.2106   -6.4028    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    6.0695   -4.9058    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    6.9354   -5.4029    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    6.0800   -6.9019    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    6.9398   -6.3960    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    4.3470   -3.9142    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    6.0676   -3.9065    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    5.2026   -3.4159    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    4.3487   -7.9117    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "    6.0854   -7.9042    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0       \n"
        "  3  1  2  0     0  0                                                       \n"
        "  1  5  1  0     0  0                                                       \n"
        "  5  6  2  0     0  0                                                       \n"
        "  6  4  1  0     0  0                                                       \n"
        "  4  2  2  0     0  0                                                       \n"
        "  2  3  1  0     0  0                                                       \n"
        "  4  9  1  0     0  0                                                       \n"
        "  9 10  2  0     0  0                                                       \n"
        " 10  8  1  0     0  0                                                       \n"
        "  8  7  2  0     0  0                                                       \n"
        "  7  2  1  0     0  0                                                       \n"
        " 10 13  1  1     0  0                                                       \n"
        " 13 14  2  0     0  0                                                       \n"
        " 14 12  1  0     0  0                                                       \n"
        " 12 11  2  0     0  0                                                       \n"
        " 11 16  1  0     0  0                                                       \n"
        " 16 17  2  0     0  0                                                       \n"
        " 17 15  1  0     0  0                                                       \n"
        " 15  7  1  0     0  0                                                       \n"
        "  9 18  1  0     0  0                                                       \n"
        " 13 19  1  0     0  0                                                       \n"
        "  8 11  1  6     0  0                                                       \n"
        "M  END                                                                      \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C18H14O/c1-11-7-8-13-9-10-19-18-15-6-4-3-5-14(15)12(2)16(11)17(13)18/h3-10H,1-2H3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_9_no_atropisomer)
{
    const char *molblock =
        "no atropisomer                                                            \n"
        "  Ketcher  2272613262D 1   1.00000     0.00000     0                      \n"
        "                                                                          \n"
        " 11 12  0  0  1  0  0  0  0  0999 V2000                                   \n"
        "   10.0575   -5.5286    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   11.7917   -5.5281    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   10.9262   -5.0272    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   11.7917   -6.5312    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   10.0575   -6.5357    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   10.9284   -7.0319    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   13.3281   -6.0349    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   12.7485   -5.2267    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   12.7368   -6.8411    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   10.9323   -8.0341    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   13.0431   -7.7954    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "  3  1  1  0     0  0                                                     \n"
        "  1  5  2  0     0  0                                                     \n"
        "  5  6  1  0     0  0                                                     \n"
        "  6  4  2  0     0  0                                                     \n"
        "  4  2  1  0     0  0                                                     \n"
        "  2  3  1  1     0  0                                                     \n"
        "  4  9  1  1     0  0                                                     \n"
        "  9  7  2  0     0  0                                                     \n"
        "  7  8  1  0     0  0                                                     \n"
        "  8  2  2  0     0  0                                                     \n"
        "  6 10  1  0     0  0                                                     \n"
        "  9 11  1  0     0  0                                                     \n"
        "M  END                                                                    \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C9H9NO/c1-6-3-4-11-9-8(6)7(2)5-10-9/h3-5H,1-2H3";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_10_no_atropisomer)
{
    const char *molblock =
        "no atropisomer                                                          \n"
        "  Ketcher  2272613352D 1   1.00000     0.00000     0                    \n"
        "                                                                        \n"
        " 13 15  0  0  1  0  0  0  0  0999 V2000                                 \n"
        "    6.1045   -3.7191    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    7.8417   -4.0800    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    7.0986   -3.3949    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    7.5866   -5.0469    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    5.8981   -4.7379    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    6.6718   -5.3556    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    5.9176   -6.9529    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    7.5935   -6.6481    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    6.6749   -6.3496    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    7.8562   -7.5835    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    6.1260   -7.9570    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    7.1229   -8.2742    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "    8.2186   -5.8385    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0   \n"
        "  3  1  2  0     0  0                                                   \n"
        "  1  5  1  0     0  0                                                   \n"
        "  5  6  2  0     0  0                                                   \n"
        "  6  4  1  1     0  0                                                   \n"
        "  4  2  2  0     0  0                                                   \n"
        "  9  7  1  1     0  0                                                   \n"
        "  7 11  2  0     0  0                                                   \n"
        " 12 10  2  0     0  0                                                   \n"
        " 10  8  1  0     0  0                                                   \n"
        "  8  9  2  0     0  0                                                   \n"
        "  6  9  1  0     0  0                                                   \n"
        "  4 13  1  0     0  0                                                   \n"
        " 13  8  1  0     0  0                                                   \n"
        "  3  2  1  1     0  0                                                   \n"
        " 12 11  1  1     0  0                                                   \n"
        "M  END                                                                  \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C13H10/c1-3-7-12-10(5-1)9-11-6-2-4-8-13(11)12/h1-8H,9H2";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_11_no_atropisomer_3_fragments)
{
    const char *molblock =
        "no atropisomers 3 fragments                                              \n"
        "  Ketcher  2272613322D 1   1.00000     0.00000     0                     \n"
        "                                                                         \n"
        " 43 50  0  0  1  0  0  0  0  0999 V2000                                  \n"
        "    6.1045   -3.7191    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.8417   -4.0800    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.0986   -3.3949    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.5866   -5.0469    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.8981   -4.7379    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.6718   -5.3556    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    5.9176   -6.9529    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.5935   -6.6481    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.6749   -6.3496    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.8562   -7.5835    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    6.1260   -7.9570    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    7.1229   -8.2742    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    8.2186   -5.8385    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    9.5037   -3.9296    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   11.2375   -3.9224    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.3782   -3.4263    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   11.2430   -4.9363    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    9.5000   -4.9492    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.3729   -5.4522    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    9.5486   -6.9885    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   11.2820   -6.9498    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.3915   -6.4693    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   11.3184   -7.9397    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    9.5860   -7.9925    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   10.4799   -8.4673    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   12.1187   -5.4213    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   12.1416   -6.4195    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   14.4400   -3.7870    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   16.1721   -3.9293    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   15.3591   -3.3591    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   16.0862   -4.9426    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   14.3481   -4.8068    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   15.1682   -5.3798    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   14.1926   -6.8106    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   15.9249   -6.9467    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   15.0897   -6.3821    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   15.8590   -7.9312    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   14.1208   -7.8014    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   14.9645   -8.3690    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   16.9116   -5.5047    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   16.8379   -6.5024    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   13.4463   -5.2285    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   13.3619   -6.2344    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "  3  1  2  0     0  0                                                    \n"
        "  1  5  1  0     0  0                                                    \n"
        "  5  6  2  0     0  0                                                    \n"
        "  6  4  1  1     0  0                                                    \n"
        "  4  2  2  0     0  0                                                    \n"
        "  9  7  1  1     0  0                                                    \n"
        "  7 11  2  0     0  0                                                    \n"
        " 12 10  2  0     0  0                                                    \n"
        " 10  8  1  0     0  0                                                    \n"
        "  8  9  2  0     0  0                                                    \n"
        "  6  9  1  0     0  0                                                    \n"
        "  4 13  1  0     0  0                                                    \n"
        " 13  8  1  0     0  0                                                    \n"
        " 16 14  2  0     0  0                                                    \n"
        " 14 18  1  0     0  0                                                    \n"
        " 18 19  2  0     0  0                                                    \n"
        " 19 17  1  1     0  0                                                    \n"
        " 17 15  2  0     0  0                                                    \n"
        " 22 20  1  1     0  0                                                    \n"
        " 20 24  2  0     0  0                                                    \n"
        " 25 23  2  0     0  0                                                    \n"
        " 23 21  1  0     0  0                                                    \n"
        " 21 22  2  0     0  0                                                    \n"
        " 19 22  1  0     0  0                                                    \n"
        " 17 26  1  0     0  0                                                    \n"
        " 26 27  2  0     0  0                                                    \n"
        " 27 21  1  0     0  0                                                    \n"
        " 30 28  2  0     0  0                                                    \n"
        " 28 32  1  0     0  0                                                    \n"
        " 32 33  2  0     0  0                                                    \n"
        " 33 31  1  1     0  0                                                    \n"
        " 31 29  2  0     0  0                                                    \n"
        " 36 34  1  1     0  0                                                    \n"
        " 34 38  2  0     0  0                                                    \n"
        " 39 37  2  0     0  0                                                    \n"
        " 37 35  1  0     0  0                                                    \n"
        " 35 36  2  0     0  0                                                    \n"
        " 33 36  1  0     0  0                                                    \n"
        " 31 40  1  0     0  0                                                    \n"
        " 35 41  1  0     0  0                                                    \n"
        " 41 40  2  0     0  0                                                    \n"
        " 32 42  1  0     0  0                                                    \n"
        " 42 43  2  0     0  0                                                    \n"
        " 43 34  1  0     0  0                                                    \n"
        "  3  2  1  1     0  0                                                    \n"
        " 12 11  1  1     0  0                                                    \n"
        " 16 15  1  1     0  0                                                    \n"
        " 25 24  1  1     0  0                                                    \n"
        " 30 29  1  1     0  0                                                    \n"
        " 39 38  1  1     0  0                                                    \n"
        "M  END                                                                   \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C16H10.C14H10.C13H10/c1-3-11-7-9-13-5-2-6-14-10-8-12(4-1)15(11)16(13)14;1-3-7-13-11(5-1)9-10-12-6-2-4-8-14(12)13;1-3-7-12-10(5-1)9-11-6-2-4-8-13(11)12/h1-10H;1-10H;1-8H,9H2";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_12_atropisomer)
{
    const char *molblock = k_dummy12_molblock;

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C15H14/c1-3-10-14-12(6-1)8-5-9-13-7-2-4-11-15(13)14/h1-4,6-7,10-11H,5,8-9H2";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, recall_biaryl_dummy1_is_flagged) {
    EXPECT_TRUE(inchi_has_atrop_flag(k_dummy1_molblock));
}

// Bridged biaryl without ortho substituents: ring inversion is fast, no axis.
TEST(test_atropisomers, bridged_bare_biaryl_dummy12_not_flagged) {
    EXPECT_FALSE(inchi_has_atrop_flag(k_dummy12_molblock));
}

TEST(test_atropisomers, test_dummy_13_atropisomer_Caryophyllene)
{
    const char *molblock =
        "5281515 Caryophyllene                                                      \n"
        "  -OEChem-03022602172D                                                     \n"
        "                                                                           \n"
        " 39 40  0     1  0  0  0  0  0999 V2000                                    \n"
        "    2.9665    1.2303    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.7095    0.2553    0.0000 C   0  0  1  0  0  0  0  0  0  0  0  0      \n"
        "    3.6754   -0.0036    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0      \n"
        "    3.9404    0.9693    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.4507   -0.7107    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.2233    2.1967    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.0000    1.4871    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.5415    0.4964    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.9507   -1.5767    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    5.4075   -0.0036    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.8167   -1.0767    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    5.5488   -1.0767    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.5415    1.4964    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.6827   -1.5767    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.8167   -0.0767    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    1.8880    0.4736    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.7864   -0.8463    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.5388    0.8072    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.1017    1.5679    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    1.9137   -0.4007    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.0123   -1.1491    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.8225    2.0375    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.3825    2.7959    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.6241    2.3559    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.1592    2.0863    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    1.4008    1.6463    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    1.8408    0.8879    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    2.4137   -1.8867    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.2607   -2.1136    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    6.0251   -0.0576    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    5.5939    0.5877    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    5.7860   -1.6495    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    6.1476   -0.9162    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    5.0784    1.8064    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.0045    1.8064    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.6827   -2.1967    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    4.4367   -0.0767    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.8167    0.5433    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "    3.1967   -0.0767    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0      \n"
        "  1  2  1  0  0  0  0                                                      \n"
        "  1  4  1  0  0  0  0                                                      \n"
        "  1  6  1  0  0  0  0                                                      \n"
        "  1  7  1  0  0  0  0                                                      \n"
        "  2  3  1  0  0  0  0                                                      \n"
        "  2  5  1  0  0  0  0                                                      \n"
        "  2 16  1  6  0  0  0                                                      \n"
        "  3  4  1  0  0  0  0                                                      \n"
        "  3  8  1  0  0  0  0                                                      \n"
        "  3 17  1  1  0  0  0                                                      \n"
        "  4 18  1  0  0  0  0                                                      \n"
        "  4 19  1  0  0  0  0                                                      \n"
        "  5  9  1  0  0  0  0                                                      \n"
        "  5 20  1  0  0  0  0                                                      \n"
        "  5 21  1  0  0  0  0                                                      \n"
        "  6 22  1  0  0  0  0                                                      \n"
        "  6 23  1  0  0  0  0                                                      \n"
        "  6 24  1  0  0  0  0                                                      \n"
        "  7 25  1  0  0  0  0                                                      \n"
        "  7 26  1  0  0  0  0                                                      \n"
        "  7 27  1  0  0  0  0                                                      \n"
        "  8 10  1  0  0  0  0                                                      \n"
        "  8 13  2  0  0  0  0                                                      \n"
        "  9 11  1  0  0  0  0                                                      \n"
        "  9 28  1  0  0  0  0                                                      \n"
        "  9 29  1  0  0  0  0                                                      \n"
        " 10 12  1  0  0  0  0                                                      \n"
        " 10 30  1  0  0  0  0                                                      \n"
        " 10 31  1  0  0  0  0                                                      \n"
        " 11 14  2  0  0  0  0                                                      \n"
        " 11 15  1  0  0  0  0                                                      \n"
        " 12 14  1  0  0  0  0                                                      \n"
        " 12 32  1  0  0  0  0                                                      \n"
        " 12 33  1  0  0  0  0                                                      \n"
        " 13 34  1  0  0  0  0                                                      \n"
        " 13 35  1  0  0  0  0                                                      \n"
        " 14 36  1  0  0  0  0                                                      \n"
        " 15 37  1  0  0  0  0                                                      \n"
        " 15 38  1  0  0  0  0                                                      \n"
        " 15 39  1  0  0  0  0                                                      \n"
        "M  END                                                                     \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C15H24/c1-11-6-5-7-12(2)13-10-15(3,4)14(13)9-8-11/h6,13-14H,2,5,7-10H2,1,3-4H3/b11-6+/t13-,14-/m1/s1";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 1);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_14_atropisomer)
{
    const char *molblock =
        "atropisomer test mol                                                     \n"
        "  ChemDraw03052610012D                                                   \n"
        "                                                                         \n"
        " 22 25  0  0  0  0  0  0  0  0999 V2000                                  \n"
        "   -0.0000    0.4125    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -0.6675    0.8974    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.6674    0.8975    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -0.4125    1.6820    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.4124    1.6822    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -1.4744    0.7259    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -2.0264    1.3390    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -0.9645    2.2951    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -1.7715    2.1236    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.4521    0.6426    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.0000   -0.4125    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.6675   -0.8974    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -0.6674   -0.8975    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.4125   -1.6820    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -0.4124   -1.6822    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.4744   -0.7259    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    2.0264   -1.3390    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    0.9645   -2.2951    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    1.7715   -2.1236    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -1.4521   -0.6426    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "    2.0652    1.1946    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "   -2.0652   -1.1946    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0    \n"
        "  1  2  1  1                                                             \n"
        "  1  3  1  6                                                             \n"
        "  1 11  1  0                                                             \n"
        "  2  6  1  0                                                             \n"
        "  2  4  2  0                                                             \n"
        "  3  5  2  0                                                             \n"
        "  3 10  1  0                                                             \n"
        "  4  5  1  0                                                             \n"
        "  4  8  1  0                                                             \n"
        "  6  7  2  0                                                             \n"
        "  7  9  1  0                                                             \n"
        "  8  9  2  0                                                             \n"
        " 10 21  1  0                                                             \n"
        " 11 12  1  1                                                             \n"
        " 11 13  1  6                                                             \n"
        " 12 16  1  0                                                             \n"
        " 12 14  2  0                                                             \n"
        " 13 15  2  0                                                             \n"
        " 13 20  1  0                                                             \n"
        " 14 15  1  0                                                             \n"
        " 14 18  1  0                                                             \n"
        " 16 17  2  0                                                             \n"
        " 17 19  1  0                                                             \n"
        " 18 19  2  0                                                             \n"
        " 20 22  1  0                                                             \n"
        "M  END                                                                   \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    memset(poutput, 0, sizeof(*poutput));
    const char expected_inchi[] = "InChI=1B/C20H20N2/c1-3-17-13-15-9-5-7-11-19(15)21(17)22-18(4-2)14-16-10-6-8-12-20(16)22/h5-14H,3-4H2,1-2H3";

    // N-N axis (2,2'-diethyl-1,1'-biindole): outside the v1 carbon-only scope
    // (AT-R11), so no stereo layer and no warning -> return code 0.
    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

TEST(test_atropisomers, test_dummy_15_test_file_1)
{

#ifndef FIXTURES_DIR
#define FIXTURES_DIR "../../../../../INCHI-1-TEST/tests/test_unit/fixtures"
#endif
    const char* inchi_filename = FIXTURES_DIR "/atropisomers_test_file_1_v2.sdf";

    std::ifstream file_inchi(inchi_filename, std::ios::binary);
    ASSERT_TRUE(file_inchi.is_open());

    // Read the whole file into a string
    std::stringstream buffer;
    buffer << file_inchi.rdbuf();
    std::string file_content = buffer.str();
    file_inchi.close();

    // Split on "$$$$"
    std::vector<std::string> molblocks;
    size_t pos = 0;
    size_t prev = 0;
    const std::string delimiter = "$$$$";
    while ((pos = file_content.find(delimiter, prev)) != std::string::npos) {
        std::string mol = file_content.substr(prev, pos - prev);
        // Optionally trim whitespace
        size_t first_non_ws = mol.find_first_not_of(" \t\r\n");
        if (first_non_ws != std::string::npos) {
            mol = mol.substr(first_non_ws);
            molblocks.push_back(mol);
        }
        prev = pos + delimiter.length();
    }
    // Add the last block if any
    std::string mol = file_content.substr(prev);
    size_t first_non_ws = mol.find_first_not_of(" \t\r\n");
    if (first_non_ws != std::string::npos) {
        mol = mol.substr(first_non_ws);
        molblocks.push_back(mol);
    }

    std::vector<std::string> list_expected_inchis = {
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m0/s1",
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m1/s1",
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m0/s1",
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m1/s1",
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m0/s1",
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m1/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H",
        "InChI=1B/C20H14O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-12,21-22H/t19-/m0/s1",
        "InChI=1B/C20H14O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-12,21-22H/t19-/m0/s1",
        "InChI=1B/C20H14O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-12,21-22H/t19-/m1/s1",
        "InChI=1B/C20H14O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-12,21-22H/t19-/m1/s1",
        "InChI=1B/C20H14O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-12,21-22H/t19-/m1/s1",
        "InChI=1B/C20H22O3/c1-21-17-5-3-13-7-15-11-23-12-16(15)8-14-4-6-18(22-2)10-20(14)19(13)9-17/h3-6,9-10,15-16H,7-8,11-12H2,1-2H3",
        "InChI=1B/C20H22O3/c1-21-17-5-3-13-7-15-11-23-12-16(15)8-14-4-6-18(22-2)10-20(14)19(13)9-17/h3-6,9-10,15-16H,7-8,11-12H2,1-2H3",
        "InChI=1B/C20H22O3/c1-21-17-5-3-13-7-15-11-23-12-16(15)8-14-4-6-18(22-2)10-20(14)19(13)9-17/h3-6,9-10,15-16H,7-8,11-12H2,1-2H3",
        "InChI=1B/C20H22O3/c1-21-17-5-3-13-7-15-11-23-12-16(15)8-14-4-6-18(22-2)10-20(14)19(13)9-17/h3-6,9-10,15-16H,7-8,11-12H2,1-2H3",
        "InChI=1B/C20H22O3/c1-21-17-5-3-13-7-15-11-23-12-16(15)8-14-4-6-18(22-2)10-20(14)19(13)9-17/h3-6,9-10,15-16H,7-8,11-12H2,1-2H3",
        "InChI=1B/C20H23N/c1-14-9-8-11-17(20(3,4)5)19(14)21-15(2)13-16-10-6-7-12-18(16)21/h6-13H,1-5H3",
        "InChI=1B/C20H23N/c1-14-9-8-11-17(20(3,4)5)19(14)21-15(2)13-16-10-6-7-12-18(16)21/h6-13H,1-5H3",
        "InChI=1B/C20H20N2/c1-3-17-13-15-9-5-7-11-19(15)21(17)22-18(4-2)14-16-10-6-8-12-20(16)22/h5-14H,3-4H2,1-2H3",
        "InChI=1B/C20H20N2/c1-3-17-13-15-9-5-7-11-19(15)21(17)22-18(4-2)14-16-10-6-8-12-20(16)22/h5-14H,3-4H2,1-2H3",
        "InChI=1B/C20H22N2/c1-3-17-13-15-9-5-7-11-19(15)21(17)22-18(4-2)14-16-10-6-8-12-20(16)22/h5-13,18H,3-4,14H2,1-2H3/t18-/m1/s1",
        "InChI=1B/C16H20N2/c1-4-15-10-9-12(2)17(15)18-13(3)11-14-7-5-6-8-16(14)18/h5-12,15H,4H2,1-3H3/t12-,15+/m0/s1",
        "InChI=1B/C14H15NO/c1-10(2)12-8-9-14(16)15(12)13-7-5-4-6-11(13)3/h4-9H,1-3H3",
        "InChI=1B/C10H8N2/c1-3-9(7-11-5-1)10-4-2-6-12-8-10/h1-8H",
        "InChI=1B/C10H8N2/c1-3-9(7-11-5-1)10-4-2-6-12-8-10/h1-8H",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m1/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1",
        "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m1/s1",
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m1/s1",
        "InChI=1B/C14H8N2O8/c17-13(18)7-3-1-5-9(15(21)22)11(7)12-8(14(19)20)4-2-6-10(12)16(23)24/h1-6H,(H,17,18)(H,19,20)/t11-/m0/s1",
        "InChI=1B/C20H14O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-12,21-22H/t19-/m0/s1",
        "InChI=1B/C20H14O2/c21-17-11-9-13-5-1-3-7-15(13)19(17)20-16-8-4-2-6-14(16)10-12-18(20)22/h1-12,21-22H/t19-/m1/s1",
        "InChI=1B/C14H26/c1-9(2)13(10(3)4)14(11(5)6)12(7)8/h9,11H,1-8H3/t13-/m1/s1",
        "InChI=1B/C12H17NO/c1-5-10-8-6-7-9(2)11(10)12(14)13(3)4/h6-8H,5H2,1-4H3/t11-/m1/s1",
        "InChI=1B/C15H22ClNO2/c1-5-13-8-6-7-11(2)15(13)17(14(18)9-16)12(3)10-19-4/h6-8,12H,5,9-10H2,1-4H3/t12-/m0/s1",
        "InChI=1B/C24H14N2O2/c1-4-8-20-15(5-1)23(17-9-11-26-14-22(17)28-20)24-16-6-2-3-7-19(16)27-21-10-12-25-13-18(21)24/h1-14H/b24-23-",
        "InChI=1B/C24H14N2O2/c1-4-8-20-15(5-1)23(17-9-11-26-14-22(17)28-20)24-16-6-2-3-7-19(16)27-21-10-12-25-13-18(21)24/h1-14H/b24-23-",
        "InChI=1B/C24H14N2O2/c1-4-8-20-15(5-1)23(17-9-11-26-14-22(17)28-20)24-16-6-2-3-7-19(16)27-21-10-12-25-13-18(21)24/h1-14H/b24-23+",
        "InChI=1B/C24H14N2O2/c1-4-8-20-15(5-1)23(17-9-11-26-14-22(17)28-20)24-16-6-2-3-7-19(16)27-21-10-12-25-13-18(21)24/h1-14H/b24-23+"
    };

    int nof_inchis = 49;

    // EXPECT_EQ(nof_inchis, molblocks.size());
    // EXPECT_EQ(nof_inchis, list_expected_inchis.size());

    char options[] = "-EnhancedStereochemistry";

    for (int i = 0; i < nof_inchis; ++i) {

        inchi_Output output;
        inchi_Output* poutput = &output;

        poutput->szLog = nullptr;
        poutput->szMessage = nullptr;
        poutput->szInChI = nullptr;

        printf("mol no %d\n", i + 1);

        int ret = MakeINCHIFromMolfileText(molblocks[i].c_str(), options, poutput);

        // EXPECT_LT(ret, 2);

        EXPECT_STREQ(poutput->szInChI, list_expected_inchis[i].c_str());

        if (poutput->szLog) {
            inchi_free(poutput->szLog);
            poutput->szLog = nullptr;
        }
        if (poutput->szMessage) {
            inchi_free(poutput->szMessage);
            poutput->szMessage = nullptr;
        }
        if (poutput->szInChI) {
            inchi_free(poutput->szInChI);
            poutput->szInChI = nullptr;
        }
    }
}

// Helper: link a single bond a<->b (0-based) with a bond type, appending to each
// atom's neighbor/bond_type arrays and bumping valence.
static void link_bond(inp_ATOM *at, int a, int b, int btype) {
    at[a].neighbor[at[a].valence] = (AT_NUMB)b;
    at[a].bond_type[at[a].valence] = (S_CHAR)btype;
    at[a].valence++;
    at[b].neighbor[at[b].valence] = (AT_NUMB)a;
    at[b].bond_type[at[b].valence] = (S_CHAR)btype;
    at[b].valence++;
}

TEST(test_atropisomers, predicate_acyclic_3plus3_single_bond_is_candidate) {
    // Atoms 0-1 central single bond; 0 also -> 2,3 ; 1 also -> 4,5 (all terminal).
    const int n = 6;
    inp_ATOM at[6] = {};
    for (int i = 0; i < n; i++) { at[i].x = (double)i; at[i].y = 0.0; at[i].z = 0.0; }
    link_bond(at, 0, 1, 1);
    link_bond(at, 0, 2, 1); link_bond(at, 0, 3, 1);
    link_bond(at, 1, 4, 1); link_bond(at, 1, 5, 1);

    ORIG_ATOM_DATA orig = {};
    int ret = find_atropisomeric_atoms_and_bonds(at, n, &orig);

    EXPECT_EQ(ret, 1);
    EXPECT_EQ(orig.bAtropisomer, 1);
    EXPECT_EQ(at[0].bAtropisomeric, 1);
    EXPECT_EQ(at[1].bAtropisomeric, 1);
    EXPECT_EQ(at[2].bAtropisomeric, 0); // terminal, valence 1

    inchi_free(orig.atrop_axes);
}

TEST(test_atropisomers, predicate_single_bond_in_small_ring_is_not_candidate) {
    // 6-membered ring of single bonds (0..5), each ring atom also gets one
    // terminal substituent so ring atoms reach valence 3. No bond should qualify:
    // every 3+3 single bond lies in the size-6 ring.
    const int n = 12;
    inp_ATOM at[12] = {};
    for (int i = 0; i < n; i++) { at[i].x = (double)i; at[i].y = 0.0; at[i].z = 0.0; }
    for (int i = 0; i < 6; i++) link_bond(at, i, (i + 1) % 6, 1); // ring
    for (int i = 0; i < 6; i++) link_bond(at, i, 6 + i, 1);       // substituents

    ORIG_ATOM_DATA orig = {};
    int ret = find_atropisomeric_atoms_and_bonds(at, n, &orig);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(orig.bAtropisomer, 0);
}

TEST(test_atropisomers, predicate_is_order_independent) {
    // Same graph as the acyclic test but atoms declared in a permuted order:
    // central bond is 4-5, substituents 0,1 on 4 and 2,3 on 5.
    const int n = 6;
    inp_ATOM at[6] = {};
    for (int i = 0; i < n; i++) { at[i].x = (double)i; at[i].y = 0.0; at[i].z = 0.0; }
    link_bond(at, 4, 5, 1);
    link_bond(at, 4, 0, 1); link_bond(at, 4, 1, 1);
    link_bond(at, 5, 2, 1); link_bond(at, 5, 3, 1);

    ORIG_ATOM_DATA orig = {};
    int ret = find_atropisomeric_atoms_and_bonds(at, n, &orig);

    EXPECT_EQ(ret, 1);
    EXPECT_EQ(at[4].bAtropisomeric, 1);
    EXPECT_EQ(at[5].bAtropisomeric, 1);

    inchi_free(orig.atrop_axes);
}

TEST(test_atropisomers, parity_flat_no_wedge_is_undefined) {
    // Planar (z=0) axis, no wedge bonds -> handedness undefined.
    const int n = 6;
    inp_ATOM at[6] = {};
    double xs[6] = {0, 1, -0.5, -0.5, 1.5, 1.5};
    double ys[6] = {0, 0,  0.9, -0.9, 0.9, -0.9};
    for (int i = 0; i < n; i++) { at[i].x = xs[i]; at[i].y = ys[i]; at[i].z = 0.0; }
    link_bond(at, 0, 1, 1);
    link_bond(at, 0, 2, 1); link_bond(at, 0, 3, 1);
    link_bond(at, 1, 4, 1); link_bond(at, 1, 5, 1);

    S_CHAR z1[3] = {}, z2[3] = {};
    int p = atrop_axis_parity(at, 0, 1, z1, z2);
    EXPECT_EQ(p, AB_PARITY_UNDF);
}

TEST(test_atropisomers, parity_wedged_axis_is_defined_and_flips_with_wedge) {
    // A twisted-biphenyl-like geometry: ring "A" substituents (2,3) sit in the
    // atom0-atom1 axis's xy-plane, ring "B" substituents (4,5) are rotated 90
    // degrees into the xz-plane. Mirroring which of (4,5) is above/below the
    // axis plane swaps the handedness of the twist.
    //
    // NB deviates from the brief's bond_stereo-wedge encoding: with the axis
    // otherwise flat, half_stereo_bond_parity() only feeds a wedge's synthetic
    // z into *that atom's own* half-bond-parity calc (see get_z_coord());
    // triple_prod_char()'s "axis" vector is built from literal (x,y,z)
    // coordinates and is blind to bond_stereo, and with this skeleton's
    // symmetry the wedge's contribution canceled out, collapsing both wedge
    // directions onto the same (UNDF) result. Giving ring B a literal
    // out-of-plane twist (verified experimentally to push the primitives'
    // dot product past MIN_DOT_PROD) is what actually exercises them.
    const int n = 6;
    double xs[6] = {0, 1, -0.5, -0.5, 1.5, 1.5};
    double ys[6] = {0, 0,  0.9, -0.9, 0,    0};

    auto build = [&](double z4, double z5) {
        static inp_ATOM at[6];
        memset(at, 0, sizeof(at));
        double zs[6] = {0, 0, 0, 0, z4, z5};
        for (int i = 0; i < n; i++) {
            at[i].x = xs[i]; at[i].y = ys[i]; at[i].z = zs[i];
            // half_stereo_bond_parity() screens the central atom's element via
            // bCanAtomHaveAStereoBond(), which only accepts C/Si/Ge/N; a zeroed
            // inp_ATOM has an empty elname and is rejected outright.
            strcpy(at[i].elname, "C");
        }
        link_bond(at, 0, 1, 1);
        link_bond(at, 0, 2, 1); link_bond(at, 0, 3, 1);
        link_bond(at, 1, 4, 1); link_bond(at, 1, 5, 1);
        return at;
    };

    S_CHAR z1[3] = {}, z2[3] = {};
    inp_ATOM *up   = build(0.9, -0.9);
    int pu = atrop_axis_parity(up, 0, 1, z1, z2);
    inp_ATOM *down = build(-0.9, 0.9);
    int pd = atrop_axis_parity(down, 0, 1, z1, z2);

    EXPECT_TRUE(pu == AB_PARITY_ODD || pu == AB_PARITY_EVEN);
    EXPECT_TRUE(pd == AB_PARITY_ODD || pd == AB_PARITY_EVEN);
    EXPECT_NE(pu, pd); // enantiomeric wedge -> opposite parity
}

TEST(test_atropisomers, detector_populates_axis_record) {
    const int n = 6;
    inp_ATOM at[6] = {};
    for (int i = 0; i < n; i++) { at[i].x = (double)i; at[i].y = 0.0; at[i].z = 0.0; }
    link_bond(at, 0, 1, 1);
    link_bond(at, 0, 2, 1); link_bond(at, 0, 3, 1);
    link_bond(at, 1, 4, 1); link_bond(at, 1, 5, 1);
    for (int i = 0; i < n; i++) at[i].orig_at_number = (AT_NUMB)(i + 1);

    ORIG_ATOM_DATA orig = {};
    find_atropisomeric_atoms_and_bonds(at, n, &orig);

    ASSERT_EQ(orig.num_atrop_axes, 1);
    ASSERT_NE(orig.atrop_axes, nullptr);
    EXPECT_EQ(orig.atrop_axes[0].at1, 0);
    EXPECT_EQ(orig.atrop_axes[0].at2, 1);

    if (orig.atrop_axes) inchi_free(orig.atrop_axes);
}

// ---------------------------------------------------------------------------
// Sub-project B gates: native /t + /m emission for a stereogenic single-bond
// axis (spec 4, AT-R5..R9, gates T6/T7).
// ---------------------------------------------------------------------------

static const char *k_dummy1_connectivity =
    "/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H";

// Runs a molblock and returns the InChI (empty on failure); *ret gets the code.
static std::string run_inchi(const std::string &molblock, const char *options, int *ret = nullptr) {
    inchi_Output out;
    memset(&out, 0, sizeof(out));
    std::string opts(options);
    int r = MakeINCHIFromMolfileText(molblock.c_str(), &opts[0], &out);
    if (ret) { *ret = r; }
    std::string s = out.szInChI ? out.szInChI : "";
    FreeINCHI(&out);
    return s;
}

// Replaces the first occurrence of `from` by `to` in a molblock copy.
static std::string edit_molblock(const char *molblock, const std::string &from, const std::string &to) {
    std::string s(molblock);
    size_t p = s.find(from);
    EXPECT_NE(p, std::string::npos) << "pattern not found: " << from;
    if (p != std::string::npos) { s.replace(p, from.size(), to); }
    return s;
}

// Mirror image of a 2D drawing: every wedge becomes a hash.
static std::string mirror_wedges(const char *molblock) {
    std::string s = edit_molblock(molblock, "  2  3  1  1", "  2  3  1  6");
    return edit_molblock(s.c_str(), "  8  7  1  1", "  8  7  1  6");
}

static const char *k_biaryl3d_p44 =
    "biaryl dihedral 44\n"
    "  gen3d\n"
    "\n"
    " 16 17  0  0  0  0  0  0  0  0999 V2000\n"
    "   -0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400    1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400    1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -3.5400    0.0000   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400   -1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400   -1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150    2.6096   -1.0544 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150   -2.6096    1.0544 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400    1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400    1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.5400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400   -1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400   -1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150    2.6096    1.0544 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150   -2.6096   -1.0544 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  2  0  0  0  0\n"
    "  2  3  1  0  0  0  0\n"
    "  3  4  2  0  0  0  0\n"
    "  4  5  1  0  0  0  0\n"
    "  5  6  2  0  0  0  0\n"
    "  6  1  1  0  0  0  0\n"
    "  2  7  1  0  0  0  0\n"
    "  6  8  1  0  0  0  0\n"
    "  9 10  2  0  0  0  0\n"
    " 10 11  1  0  0  0  0\n"
    " 11 12  2  0  0  0  0\n"
    " 12 13  1  0  0  0  0\n"
    " 13 14  2  0  0  0  0\n"
    " 14  9  1  0  0  0  0\n"
    " 10 15  1  0  0  0  0\n"
    " 14 16  1  0  0  0  0\n"
    "  1  9  1  0  0  0  0\n"
    "M  END\n"
    ;
static const char *k_biaryl3d_p90 =
    "biaryl dihedral 90\n"
    "  gen3d\n"
    "\n"
    " 16 17  0  0  0  0  0  0  0  0999 V2000\n"
    "   -0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400    0.8573   -0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400    0.8573   -0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -3.5400    0.0000   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400   -0.8573    0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400   -0.8573    0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150    1.9902   -1.9902 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150   -1.9902    1.9902 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400    0.8573    0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400    0.8573    0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.5400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400   -0.8573   -0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400   -0.8573   -0.8573 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150    1.9902    1.9902 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150   -1.9902   -1.9902 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  2  0  0  0  0\n"
    "  2  3  1  0  0  0  0\n"
    "  3  4  2  0  0  0  0\n"
    "  4  5  1  0  0  0  0\n"
    "  5  6  2  0  0  0  0\n"
    "  6  1  1  0  0  0  0\n"
    "  2  7  1  0  0  0  0\n"
    "  6  8  1  0  0  0  0\n"
    "  9 10  2  0  0  0  0\n"
    " 10 11  1  0  0  0  0\n"
    " 11 12  2  0  0  0  0\n"
    " 12 13  1  0  0  0  0\n"
    " 13 14  2  0  0  0  0\n"
    " 14  9  1  0  0  0  0\n"
    " 10 15  1  0  0  0  0\n"
    " 14 16  1  0  0  0  0\n"
    "  1  9  1  0  0  0  0\n"
    "M  END\n"
    ;
static const char *k_biaryl3d_p120 =
    "biaryl dihedral 120\n"
    "  gen3d\n"
    "\n"
    " 16 17  0  0  0  0  0  0  0  0999 V2000\n"
    "   -0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400    0.6062   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400    0.6062   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -3.5400    0.0000   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400   -0.6062    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400   -0.6062    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150    1.4073   -2.4375 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150   -1.4073    2.4375 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400    0.6062    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400    0.6062    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.5400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400   -0.6062   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400   -0.6062   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150    1.4073    2.4375 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150   -1.4073   -2.4375 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  2  0  0  0  0\n"
    "  2  3  1  0  0  0  0\n"
    "  3  4  2  0  0  0  0\n"
    "  4  5  1  0  0  0  0\n"
    "  5  6  2  0  0  0  0\n"
    "  6  1  1  0  0  0  0\n"
    "  2  7  1  0  0  0  0\n"
    "  6  8  1  0  0  0  0\n"
    "  9 10  2  0  0  0  0\n"
    " 10 11  1  0  0  0  0\n"
    " 11 12  2  0  0  0  0\n"
    " 12 13  1  0  0  0  0\n"
    " 13 14  2  0  0  0  0\n"
    " 14  9  1  0  0  0  0\n"
    " 10 15  1  0  0  0  0\n"
    " 14 16  1  0  0  0  0\n"
    "  1  9  1  0  0  0  0\n"
    "M  END\n"
    ;
static const char *k_biaryl3d_m44 =
    "biaryl dihedral -44\n"
    "  gen3d\n"
    "\n"
    " 16 17  0  0  0  0  0  0  0  0999 V2000\n"
    "   -0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400    1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400    1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -3.5400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8400   -1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.4400   -1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150    2.6096    1.0544 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5150   -2.6096   -1.0544 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.7400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400    1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400    1.1242   -0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.5400    0.0000   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8400   -1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.4400   -1.1242    0.4542 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150    2.6096   -1.0544 Br  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.5150   -2.6096    1.0544 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  2  0  0  0  0\n"
    "  2  3  1  0  0  0  0\n"
    "  3  4  2  0  0  0  0\n"
    "  4  5  1  0  0  0  0\n"
    "  5  6  2  0  0  0  0\n"
    "  6  1  1  0  0  0  0\n"
    "  2  7  1  0  0  0  0\n"
    "  6  8  1  0  0  0  0\n"
    "  9 10  2  0  0  0  0\n"
    " 10 11  1  0  0  0  0\n"
    " 11 12  2  0  0  0  0\n"
    " 12 13  1  0  0  0  0\n"
    " 13 14  2  0  0  0  0\n"
    " 14  9  1  0  0  0  0\n"
    " 10 15  1  0  0  0  0\n"
    " 14 16  1  0  0  0  0\n"
    "  1  9  1  0  0  0  0\n"
    "M  END\n"
    ;

// T7: the axis is cited once, on the lower-numbered axis atom, with /m and /s.
TEST(test_atropisomers, gate_t7_axis_cited_on_lower_atom) {
    std::string s = run_inchi(k_dummy1_molblock, "-EnhancedStereochemistry");
    EXPECT_EQ(s, std::string("InChI=1B") + k_dummy1_connectivity + "/t11-/m0/s1");
}

// T6: enantiomer pair shares skeleton and /t, differs only in /m.
TEST(test_atropisomers, gate_t6_enantiomer_flips_m) {
    std::string ra = run_inchi(k_dummy1_molblock, "-EnhancedStereochemistry");
    std::string sa = run_inchi(mirror_wedges(k_dummy1_molblock), "-EnhancedStereochemistry");
    EXPECT_EQ(ra, std::string("InChI=1B") + k_dummy1_connectivity + "/t11-/m0/s1");
    EXPECT_EQ(sa, std::string("InChI=1B") + k_dummy1_connectivity + "/t11-/m1/s1");
}

// T6: 3D rotamers (44, 90, 120 deg) of one enantiomer give one InChI; the
// -44 deg twist is the other enantiomer.
TEST(test_atropisomers, gate_t6_rotamers_one_inchi) {
    std::string p44 = run_inchi(k_biaryl3d_p44, "-EnhancedStereochemistry");
    std::string p90 = run_inchi(k_biaryl3d_p90, "-EnhancedStereochemistry");
    std::string p120 = run_inchi(k_biaryl3d_p120, "-EnhancedStereochemistry");
    std::string m44 = run_inchi(k_biaryl3d_m44, "-EnhancedStereochemistry");
    EXPECT_EQ(p44, p90);
    EXPECT_EQ(p44, p120);
    EXPECT_NE(p44, m44);
    EXPECT_EQ(p44.substr(0, p44.size() - 5), m44.substr(0, m44.size() - 5)); // differ in /mX/s1 only
    EXPECT_NE(p44.find("/t11-/m"), std::string::npos);
    EXPECT_NE(m44.find("/t11-/m"), std::string::npos);
}

// AT-R5: an end with two identical substituents (2,6-dichloro ring) is not
// stereogenic: the axis is pruned by canonical equivalence, nothing emitted.
TEST(test_atropisomers, dissymmetry_symmetric_end_pruned) {
    std::string sym = edit_molblock(k_dummy1_molblock,
        "    7.5821   -7.4750    0.0000 Br ", "    7.5821   -7.4750    0.0000 Cl ");
    std::string s = run_inchi(sym, "-EnhancedStereochemistry");
    EXPECT_EQ(s, "InChI=1B/C12H6BrCl3/c13-7-3-1-4-8(14)11(7)12-9(15)5-2-6-10(12)16/h1-6H");
}

// AT-R9: one wedge (IUPAC) defines the axis like two do.
TEST(test_atropisomers, single_wedge_defines_axis) {
    std::string one = edit_molblock(k_dummy1_molblock, "  8  7  1  1", "  8  7  1  0");
    EXPECT_EQ(run_inchi(one, "-EnhancedStereochemistry"), run_inchi(k_dummy1_molblock, "-EnhancedStereochemistry"));
}

// A flat drawing without wedges has undefined axial geometry: omitted (with
// the usual "omitted undefined stereo" warning), never guessed.
TEST(test_atropisomers, flat_axis_is_omitted) {
    std::string flat = edit_molblock(k_dummy1_molblock, "  2  3  1  1", "  2  3  1  0");
    flat = edit_molblock(flat.c_str(), "  8  7  1  1", "  8  7  1  0");
    int ret = 0;
    std::string s = run_inchi(flat, "-EnhancedStereochemistry", &ret);
    EXPECT_EQ(s, std::string("InChI=1B") + k_dummy1_connectivity);
    EXPECT_EQ(ret, 1);
}

// Backward compatibility: without -EnhancedStereochemistry nothing changes.
TEST(test_atropisomers, standard_output_unchanged) {
    EXPECT_EQ(run_inchi(k_dummy1_molblock, ""), std::string("InChI=1S") + k_dummy1_connectivity);
    EXPECT_EQ(run_inchi(k_biaryl3d_p44, ""), std::string("InChI=1S") + k_dummy1_connectivity);
}

// [ATROP] stability heuristic: fewer than three substituted ortho positions
// (3,3'-bipyridine, fixture mol 34, drawn with a wedge) is not an axis.
TEST(test_atropisomers, unhindered_axis_not_emitted) {
    const char *bipyridine =
        "3,3'-bipyridine\n"
        "  ChemDraw03052609262D\n"
        "\n"
        " 12 13  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.7145    1.6500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.7145    0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.0000    2.0625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.0000    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "   -0.7144    1.6500    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "   -0.7144    0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "   -0.0000   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.7145   -0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "   -0.7145   -0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.7144   -1.6501    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "   -0.7145   -1.6500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "   -0.0001   -2.0625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "  1  2  1  0\n"
        "  1  3  2  0\n"
        "  2  4  2  0\n"
        "  3  5  1  0\n"
        "  4  6  1  1\n"
        "  4  7  1  0\n"
        "  5  6  2  0\n"
        "  7  8  1  0\n"
        "  7  9  2  0\n"
        "  8 10  2  0\n"
        "  9 11  1  0\n"
        " 10 12  1  0\n"
        " 11 12  2  0\n"
        "M  END\n";
    EXPECT_EQ(run_inchi(bipyridine, "-EnhancedStereochemistry"),
              "InChI=1B/C10H8N2/c1-3-9(7-11-5-1)10-4-2-6-12-8-10/h1-8H");
}

// Candidate detection must stay polynomial: a 150-atom / 292-bond metal
// cluster (InChI_TestSet_ext.sdf record 126) stalled for >20 s in ring
// enumeration. Without -EnhancedStereochemistry it is instant.
TEST(test_atropisomers, dense_cluster_completes_quickly) {
    const double max_seconds = 5.0;
    std::ifstream f(FIXTURES_DIR "/atrop_dense_cluster.sdf", std::ios::binary);
    ASSERT_TRUE(f.is_open());
    std::stringstream buf;
    buf << f.rdbuf();
    std::string mol = buf.str();
    mol = mol.substr(0, mol.find("$$$$"));

    int ret = -1;
    auto t0 = std::chrono::steady_clock::now();
    std::string s = run_inchi(mol, "-EnhancedStereochemistry", &ret);
    double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    EXPECT_LT(secs, max_seconds);
    EXPECT_GE(ret, 0);
    EXPECT_EQ(s.compare(0, 7, "InChI=1"), 0);
}

// An axis end may own a second stereo bond: a C=C (alkene-aryl axis) or
// another axis (diaryl ketone). The axis then sits in stereo_bond slot [1];
// code that only read slot [0] raised STEREOCOUNT_ERR or printed the axis
// into /b.
static const char *k_ketone_two_axes_molblock =
    "bis(2-chloro-6-methylphenyl)methanone\n"
    "  gen3D\n"
    "\n"
    " 18 19  0  0  0  0  0  0  0  0999 V2000\n"
    "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000   -1.2300    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.2990    0.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.6021    1.6250   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.8146    2.3250   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.7239    2.1500   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.4208    1.2750    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.2084    0.5750    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.6279    1.8125   -2.1750 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.8836   -0.3625    2.1750 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.2990    0.7500   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.2084    0.5750    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -3.4208    1.2750    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -3.7239    2.1500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.8146    2.3250   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.6021    1.6250   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.8836   -0.3625    2.1750 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.6279    1.8125   -2.1750 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  2  0  0  0  0\n"
    "  3  4  2  0  0  0  0\n"
    "  4  5  1  0  0  0  0\n"
    "  5  6  2  0  0  0  0\n"
    "  6  7  1  0  0  0  0\n"
    "  7  8  2  0  0  0  0\n"
    "  8  3  1  0  0  0  0\n"
    "  1  3  1  0  0  0  0\n"
    "  4  9  1  0  0  0  0\n"
    "  8 10  1  0  0  0  0\n"
    " 11 12  2  0  0  0  0\n"
    " 12 13  1  0  0  0  0\n"
    " 13 14  2  0  0  0  0\n"
    " 14 15  1  0  0  0  0\n"
    " 15 16  2  0  0  0  0\n"
    " 16 11  1  0  0  0  0\n"
    "  1 11  1  0  0  0  0\n"
    " 12 17  1  0  0  0  0\n"
    " 16 18  1  0  0  0  0\n"
    "M  END\n";

static const char *k_alkene_axes_molblock =
    "(E)-2,3-bis(2-chloro-6-methylphenyl)but-2-ene\n"
    "  gen3D\n"
    "\n"
    " 20 21  0  0  0  0  0  0  0  0999 V2000\n"
    "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.3400    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.7500    1.3000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.0900   -1.3000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.7496   -1.2993    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -0.5743   -2.2085   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.2739   -3.4212   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.1488   -3.7246   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -2.3241   -2.8153    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.6245   -1.6027    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.3631   -1.8835   -2.1750 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.8123   -0.6284    2.1750 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.0896    1.2993    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.9143    2.2085   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.6139    3.4212   -1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.4888    3.7246   -0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.6641    2.8153    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.9645    1.6027    1.0500 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.9769    1.8835   -2.1750 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    3.1523    0.6284    2.1750 Cl  0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  2  0  0  0  0\n"
    "  1  3  1  0  0  0  0\n"
    "  2  4  1  0  0  0  0\n"
    "  5  6  2  0  0  0  0\n"
    "  6  7  1  0  0  0  0\n"
    "  7  8  2  0  0  0  0\n"
    "  8  9  1  0  0  0  0\n"
    "  9 10  2  0  0  0  0\n"
    " 10  5  1  0  0  0  0\n"
    "  1  5  1  0  0  0  0\n"
    "  6 11  1  0  0  0  0\n"
    " 10 12  1  0  0  0  0\n"
    " 13 14  2  0  0  0  0\n"
    " 14 15  1  0  0  0  0\n"
    " 15 16  2  0  0  0  0\n"
    " 16 17  1  0  0  0  0\n"
    " 17 18  2  0  0  0  0\n"
    " 18 13  1  0  0  0  0\n"
    "  2 13  1  0  0  0  0\n"
    " 14 19  1  0  0  0  0\n"
    " 18 20  1  0  0  0  0\n"
    "M  END\n";

TEST(test_atropisomers, axis_end_with_second_stereo_bond) {
    EXPECT_EQ(run_inchi(k_ketone_two_axes_molblock, "-EnhancedStereochemistry"),
              "InChI=1B/C15H12Cl2O/c1-9-5-3-7-11(16)13(9)15(18)14-10(2)6-4-8-12(14)17/h3-8H,1-2H3/t13-,14-/m0/s1");
    EXPECT_EQ(run_inchi(k_alkene_axes_molblock, "-EnhancedStereochemistry"),
              "InChI=1B/C18H18Cl2/c1-11-7-5-9-15(19)17(11)13(3)14(4)18-12(2)8-6-10-16(18)20/h5-10H,1-4H3/b14-13+/t13-,14-/m1/s1");
}
