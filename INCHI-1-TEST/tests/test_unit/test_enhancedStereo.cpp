#include <gtest/gtest.h>
#include <fstream>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_molfile_v2)
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

TEST(test_enhancedStereo, test_EnhancedStereochemistry_1)
{
    const char *molblock =
        "enhanc_stereo1									   \n"
        "  ACD/LABS08242216132D                            \n"
        "												   \n"
        "  0  0  0  0  0  0  0  0  0  0999 V3000           \n"
        "M  V30 BEGIN CTAB                                 \n"
        "M  V30 COUNTS 18 17 0 0 1                         \n"
        "M  V30 BEGIN ATOM                                 \n"
        "M  V30 1 C 3424.1946 -1936.7935 0 0               \n"
        "M  V30 2 C 3352.3145 -1895.2935 0 0               \n"
        "M  V30 3 C 3280.4346 -1936.7935 0 0               \n"
        "M  V30 4 C 3208.5542 -1895.2935 0 0               \n"
        "M  V30 5 C 3136.6743 -1936.7935 0 0               \n"
        "M  V30 6 C 3064.7944 -1895.2935 0 0               \n"
        "M  V30 7 Br 3136.6743 -2019.7935 0 0              \n"
        "M  V30 8 Cl 3208.5542 -1812.2935 0 0              \n"
        "M  V30 9 Cl 3280.4346 -2019.7935 0 0              \n"
        "M  V30 10 Cl 3352.3145 -1812.2935 0 0             \n"
        "M  V30 11 Cl 3424.1946 -2019.7935 0 0             \n"
        "M  V30 12 C 3496.075 -1895.2935 0 0               \n"
        "M  V30 13 C 3567.9548 -1936.7942 0 0              \n"
        "M  V30 14 C 3639.835 -1895.2944 0 0               \n"
        "M  V30 15 C 3711.7148 -1936.7942 0 0              \n"
        "M  V30 16 Cl 3639.835 -1812.2944 0 0              \n"
        "M  V30 17 Cl 3567.9548 -2019.7942 0 0             \n"
        "M  V30 18 Cl 3496.075 -1812.2937 0 0              \n"
        "M  V30 END ATOM                                   \n"
        "M  V30 BEGIN BOND                                 \n"
        "M  V30 1 1 1 2                                    \n"
        "M  V30 2 1 1 11 CFG=3                             \n"
        "M  V30 3 1 1 12                                   \n"
        "M  V30 4 1 2 3                                    \n"
        "M  V30 5 1 2 10 CFG=1                             \n"
        "M  V30 6 1 3 4                                    \n"
        "M  V30 7 1 3 9 CFG=1                              \n"
        "M  V30 8 1 4 5                                    \n"
        "M  V30 9 1 4 8 CFG=1                              \n"
        "M  V30 10 1 5 6                                   \n"
        "M  V30 11 1 5 7 CFG=1                             \n"
        "M  V30 12 1 12 13                                 \n"
        "M  V30 13 1 12 18 CFG=3                           \n"
        "M  V30 14 1 13 14                                 \n"
        "M  V30 15 1 13 17 CFG=1                           \n"
        "M  V30 16 1 14 15                                 \n"
        "M  V30 17 1 14 16 CFG=1                           \n"
        "M  V30 END BOND                                   \n"
        "M  V30 BEGIN COLLECTION                           \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 1)                 \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(2 2 3)               \n"
        "M  V30 MDLV30/STEABS ATOMS=(2 4 5)                \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 12 13)             \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(1 14)                \n"
        "M  V30 END COLLECTION                             \n"
        "M  V30 END CTAB                                   \n"
        "M  END                                            \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C10H14BrCl7/c1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h3-10H,1-2H3/t3-,4-,5+,6-,7-,8-,9+,10-/m0/s1(3,5)2(4)(6,8)3(7,9)(10)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

/* Gate G6 / ES-R11: an explicit STEABS-only collection must reduce to the bare
   standard /s1. The whole string, /t and /m included, is byte-identical to the
   standard InChI for this molfile apart from the 1B prefix (SPEC 1 §2, §5.C). */
TEST(test_enhancedStereo, test_EnhancedStereochemistry_abs_only_reduces_to_s1)
{
    const char *molblock =
        "enhanc_stereo_abs_only                            \n"
        "  ACD/LABS08242216132D                            \n"
        "                                                  \n"
        "  0  0  0  0  0  0  0  0  0  0999 V3000           \n"
        "M  V30 BEGIN CTAB                                 \n"
        "M  V30 COUNTS 18 17 0 0 1                         \n"
        "M  V30 BEGIN ATOM                                 \n"
        "M  V30 1 C 3424.1946 -1936.7935 0 0               \n"
        "M  V30 2 C 3352.3145 -1895.2935 0 0               \n"
        "M  V30 3 C 3280.4346 -1936.7935 0 0               \n"
        "M  V30 4 C 3208.5542 -1895.2935 0 0               \n"
        "M  V30 5 C 3136.6743 -1936.7935 0 0               \n"
        "M  V30 6 C 3064.7944 -1895.2935 0 0               \n"
        "M  V30 7 Br 3136.6743 -2019.7935 0 0              \n"
        "M  V30 8 Cl 3208.5542 -1812.2935 0 0              \n"
        "M  V30 9 Cl 3280.4346 -2019.7935 0 0              \n"
        "M  V30 10 Cl 3352.3145 -1812.2935 0 0             \n"
        "M  V30 11 Cl 3424.1946 -2019.7935 0 0             \n"
        "M  V30 12 C 3496.075 -1895.2935 0 0               \n"
        "M  V30 13 C 3567.9548 -1936.7942 0 0              \n"
        "M  V30 14 C 3639.835 -1895.2944 0 0               \n"
        "M  V30 15 C 3711.7148 -1936.7942 0 0              \n"
        "M  V30 16 Cl 3639.835 -1812.2944 0 0              \n"
        "M  V30 17 Cl 3567.9548 -2019.7942 0 0             \n"
        "M  V30 18 Cl 3496.075 -1812.2937 0 0              \n"
        "M  V30 END ATOM                                   \n"
        "M  V30 BEGIN BOND                                 \n"
        "M  V30 1 1 1 2                                    \n"
        "M  V30 2 1 1 11 CFG=3                             \n"
        "M  V30 3 1 1 12                                   \n"
        "M  V30 4 1 2 3                                    \n"
        "M  V30 5 1 2 10 CFG=1                             \n"
        "M  V30 6 1 3 4                                    \n"
        "M  V30 7 1 3 9 CFG=1                              \n"
        "M  V30 8 1 4 5                                    \n"
        "M  V30 9 1 4 8 CFG=1                              \n"
        "M  V30 10 1 5 6                                   \n"
        "M  V30 11 1 5 7 CFG=1                             \n"
        "M  V30 12 1 12 13                                 \n"
        "M  V30 13 1 12 18 CFG=3                           \n"
        "M  V30 14 1 13 14                                 \n"
        "M  V30 15 1 13 17 CFG=1                           \n"
        "M  V30 16 1 14 15                                 \n"
        "M  V30 17 1 14 16 CFG=1                           \n"
        "M  V30 END BOND                                   \n"
        "M  V30 BEGIN COLLECTION                           \n"
        "M  V30 MDLV30/STEABS ATOMS=(8 1 2 3 4 5 12 13 14) \n"
        "M  V30 END COLLECTION                             \n"
        "M  V30 END CTAB                                   \n"
        "M  END                                            \n";

    inchi_Output output;
    inchi_Output *poutput = &output;

    // Standard InChI for the same molfile, i.e. with the option switched off
    char options_std[] = "";
    const char expected_std[] = "InChI=1S/C10H14BrCl7/c1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h3-10H,1-2H3/t3-,4-,5+,6+,7-,8+,9+,10-/m0/s1";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options_std, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_std);
    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;
    FreeINCHI(poutput);

    char options_enh[] = "-EnhancedStereochemistry";
    const char expected_enh[] = "InChI=1B/C10H14BrCl7/c1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h3-10H,1-2H3/t3-,4-,5+,6+,7-,8+,9+,10-/m0/s1";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options_enh, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_enh);
    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;
    FreeINCHI(poutput);
}

/* SPEC 1 gate G7 / ES-R18: a single OR (resp. AND) collection covering every
   stereocentre reduces to the bare /s2 (resp. /s3) and carries no /m at all —
   such a component has no absolute reference for /m to point at. Apart from the
   version prefix the whole string is then byte-identical to the corresponding
   -SRel / -SRac output, which is the [STRING-ALG] "only OR" / "only AND" edge
   table. Before the ES-R18 fix a spurious /m0 was emitted here. */
TEST(test_enhancedStereo, test_EnhancedStereochemistry_or_and_only_omit_m_layer)
{
    const std::string molblock_head =
        "enhanc_stereo_or_and_only                         \n"
        "  ACD/LABS08242216132D                            \n"
        "                                                  \n"
        "  0  0  0  0  0  0  0  0  0  0999 V3000           \n"
        "M  V30 BEGIN CTAB                                 \n"
        "M  V30 COUNTS 18 17 0 0 1                         \n"
        "M  V30 BEGIN ATOM                                 \n"
        "M  V30 1 C 3424.1946 -1936.7935 0 0               \n"
        "M  V30 2 C 3352.3145 -1895.2935 0 0               \n"
        "M  V30 3 C 3280.4346 -1936.7935 0 0               \n"
        "M  V30 4 C 3208.5542 -1895.2935 0 0               \n"
        "M  V30 5 C 3136.6743 -1936.7935 0 0               \n"
        "M  V30 6 C 3064.7944 -1895.2935 0 0               \n"
        "M  V30 7 Br 3136.6743 -2019.7935 0 0              \n"
        "M  V30 8 Cl 3208.5542 -1812.2935 0 0              \n"
        "M  V30 9 Cl 3280.4346 -2019.7935 0 0              \n"
        "M  V30 10 Cl 3352.3145 -1812.2935 0 0             \n"
        "M  V30 11 Cl 3424.1946 -2019.7935 0 0             \n"
        "M  V30 12 C 3496.075 -1895.2935 0 0               \n"
        "M  V30 13 C 3567.9548 -1936.7942 0 0              \n"
        "M  V30 14 C 3639.835 -1895.2944 0 0               \n"
        "M  V30 15 C 3711.7148 -1936.7942 0 0              \n"
        "M  V30 16 Cl 3639.835 -1812.2944 0 0              \n"
        "M  V30 17 Cl 3567.9548 -2019.7942 0 0             \n"
        "M  V30 18 Cl 3496.075 -1812.2937 0 0              \n"
        "M  V30 END ATOM                                   \n"
        "M  V30 BEGIN BOND                                 \n"
        "M  V30 1 1 1 2                                    \n"
        "M  V30 2 1 1 11 CFG=3                             \n"
        "M  V30 3 1 1 12                                   \n"
        "M  V30 4 1 2 3                                    \n"
        "M  V30 5 1 2 10 CFG=1                             \n"
        "M  V30 6 1 3 4                                    \n"
        "M  V30 7 1 3 9 CFG=1                              \n"
        "M  V30 8 1 4 5                                    \n"
        "M  V30 9 1 4 8 CFG=1                              \n"
        "M  V30 10 1 5 6                                   \n"
        "M  V30 11 1 5 7 CFG=1                             \n"
        "M  V30 12 1 12 13                                 \n"
        "M  V30 13 1 12 18 CFG=3                           \n"
        "M  V30 14 1 13 14                                 \n"
        "M  V30 15 1 13 17 CFG=1                           \n"
        "M  V30 16 1 14 15                                 \n"
        "M  V30 17 1 14 16 CFG=1                           \n"
        "M  V30 END BOND                                   \n"
        "M  V30 BEGIN COLLECTION                           \n";
    const std::string molblock_tail =
        "M  V30 END COLLECTION                             \n"
        "M  V30 END CTAB                                   \n"
        "M  END                                            \n";

    const char *skeleton = "InChI=1B/C10H14BrCl7/c1-3(11)5(13)7(15)9(17)10(18)8(16)"
                           "6(14)4(2)12/h3-10H,1-2H3/t3-,4-,5+,6+,7-,8+,9+,10-";

    struct EnhStereoOnlyCase
    {
        const char *collection;
        const char *ref_option;
        const char *expected_s_layer;
    };
    const EnhStereoOnlyCase cases[] = {
        { "M  V30 MDLV30/STEREL1 ATOMS=(8 1 2 3 4 5 12 13 14)\n", "-SRel", "/s2" },
        { "M  V30 MDLV30/STERAC1 ATOMS=(8 1 2 3 4 5 12 13 14)\n", "-SRac", "/s3" },
    };

    for (const EnhStereoOnlyCase &c : cases)
    {
        const std::string molblock = molblock_head + c.collection + molblock_tail;
        const std::string expected_enh = std::string(skeleton) + c.expected_s_layer;

        inchi_Output output;
        inchi_Output *poutput = &output;

        char options_enh[] = "-EnhancedStereochemistry";
        ASSERT_EQ(MakeINCHIFromMolfileText(molblock.c_str(), options_enh, poutput), 0);
        EXPECT_STREQ(poutput->szInChI, expected_enh.c_str());
        /* ES-R18: no /m segment anywhere in the string */
        EXPECT_EQ(strstr(poutput->szInChI, "/m"), nullptr);
        const std::string enh = poutput->szInChI;
        poutput->szLog = nullptr;
        poutput->szMessage = nullptr;
        FreeINCHI(poutput);

        char options_ref[16];
        snprintf(options_ref, sizeof(options_ref), "%s", c.ref_option);
        ASSERT_EQ(MakeINCHIFromMolfileText(molblock.c_str(), options_ref, poutput), 0);
        const std::string ref = poutput->szInChI;
        poutput->szLog = nullptr;
        poutput->szMessage = nullptr;
        FreeINCHI(poutput);

        /* identical apart from the version prefix: 1B vs the plain non-standard 1 */
        EXPECT_EQ(enh.substr(strlen("InChI=1B/")), ref.substr(strlen("InChI=1/")));
    }
}

/* ES-R18, per component: the collection lists are structure-wide, so an ABS
   collection on one component must not keep /m alive on an OR/AND-only sibling.
   Two disconnected fragments, STEABS on one and STEREL1 on the other, must give
   the '.' placeholder for the OR-only component and a digit for the ABS one.
   Before the per-component fix this emitted /m11 for both. */
TEST(test_enhancedStereo, test_EnhancedStereochemistry_m_layer_is_per_component)
{
    const char *molblock_head =
        "mixed_abs_plus_rel_components                \n"
        "  Hand-built: comp A = STEABS/STERAC1, comp B = STEREL1\n"
        "                                             \n"
        "  0  0  0     0  0            999 V3000      \n"
        "M  V30 BEGIN CTAB                            \n"
        "M  V30 COUNTS 12 10 0 0 0                    \n"
        "M  V30 BEGIN ATOM                            \n"
        "M  V30 1 C 0.0000 0.0000 0 0                 \n"
        "M  V30 2 C 0.8660 0.5000 0 0 CFG=1           \n"
        "M  V30 3 C 1.7320 0.0000 0 0 CFG=1           \n"
        "M  V30 4 C 2.5980 0.5000 0 0                 \n"
        "M  V30 5 Cl 0.8660 1.5000 0 0                \n"
        "M  V30 6 Cl 1.7320 -1.0000 0 0               \n"
        "M  V30 7 C 0.0000 5.0000 0 0                 \n"
        "M  V30 8 C 0.8660 5.5000 0 0 CFG=1           \n"
        "M  V30 9 C 1.7320 5.0000 0 0 CFG=1           \n"
        "M  V30 10 C 2.5980 5.5000 0 0                \n"
        "M  V30 11 Br 0.8660 6.5000 0 0               \n"
        "M  V30 12 Br 1.7320 4.0000 0 0               \n"
        "M  V30 END ATOM                              \n"
        "M  V30 BEGIN BOND                            \n"
        "M  V30 1 1 1 2                               \n"
        "M  V30 2 1 2 3                               \n"
        "M  V30 3 1 3 4                               \n"
        "M  V30 4 1 2 5 CFG=1                         \n"
        "M  V30 5 1 3 6 CFG=1                         \n"
        "M  V30 6 1 7 8                               \n"
        "M  V30 7 1 8 9                               \n"
        "M  V30 8 1 9 10                              \n"
        "M  V30 9 1 8 11 CFG=1                        \n"
        "M  V30 10 1 9 12 CFG=1                       \n"
        "M  V30 END BOND                              \n"
        "M  V30 BEGIN COLLECTION                      \n";
    const char *molblock_tail =
        "M  V30 MDLV30/STEREL1 ATOMS=(2 8 9)          \n"
        "M  V30 END COLLECTION                        \n"
        "M  V30 END CTAB                              \n"
        "M  END                                       \n";

    /* component order is C4H8Br2 (the STEREL1 one) then C4H8Cl2 (the other) */
    struct PerComponentCase
    {
        const char *collection_of_second_component;
        const char *expected_enh;
    };
    const PerComponentCase cases[] = {
        /* ABS sibling: /m survives, but only as a digit for the ABS component */
        { "M  V30 MDLV30/STEABS ATOMS=(2 2 3)           \n",
          "InChI=1B/C4H8Br2.C4H8Cl2/c2*1-3(5)4(2)6/h2*3-4H,1-2H3/t2*3-,4-/m.1/s2;1" },
        /* no ABS anywhere: the whole /m segment disappears */
        { "M  V30 MDLV30/STERAC1 ATOMS=(2 2 3)          \n",
          "InChI=1B/C4H8Br2.C4H8Cl2/c2*1-3(5)4(2)6/h2*3-4H,1-2H3/t2*3-,4-/s2;3" },
    };

    for (const PerComponentCase &c : cases)
    {
        const std::string molblock =
            std::string(molblock_head) + c.collection_of_second_component + molblock_tail;

        inchi_Output output;
        inchi_Output *poutput = &output;

        char options_enh[] = "-EnhancedStereochemistry";
        ASSERT_EQ(MakeINCHIFromMolfileText(molblock.c_str(), options_enh, poutput), 0);
        EXPECT_STREQ(poutput->szInChI, c.expected_enh);
        poutput->szLog = nullptr;
        poutput->szMessage = nullptr;
        FreeINCHI(poutput);
    }
}

/* ES-R18 across the Mobile-H/Fixed-H split: an OR-only component with a mobile-H
   group must drop /m from the main layer and keep the /f sublayer intact, staying
   byte-identical to -SRel -FixedH apart from the version prefix. */
TEST(test_enhancedStereo, test_EnhancedStereochemistry_or_only_omits_m_with_mobile_h)
{
    const char *molblock =
        "mobileh_or_only                              \n"
        "  Hand-built: 2-Cl-3-Br-butanoic acid        \n"
        "                                             \n"
        "  0  0  0     0  0            999 V3000      \n"
        "M  V30 BEGIN CTAB                            \n"
        "M  V30 COUNTS 8 7 0 0 0                      \n"
        "M  V30 BEGIN ATOM                            \n"
        "M  V30 1 C 0.0000 0.0000 0 0                 \n"
        "M  V30 2 C 0.8660 0.5000 0 0 CFG=1           \n"
        "M  V30 3 C 1.7320 0.0000 0 0 CFG=1           \n"
        "M  V30 4 C 2.5980 0.5000 0 0                 \n"
        "M  V30 5 O 3.4640 0.0000 0 0                 \n"
        "M  V30 6 O 2.5980 1.5000 0 0                 \n"
        "M  V30 7 Cl 0.8660 1.5000 0 0                \n"
        "M  V30 8 Br 1.7320 -1.0000 0 0               \n"
        "M  V30 END ATOM                              \n"
        "M  V30 BEGIN BOND                            \n"
        "M  V30 1 1 1 2                               \n"
        "M  V30 2 1 2 3                               \n"
        "M  V30 3 1 3 4                               \n"
        "M  V30 4 2 4 5                               \n"
        "M  V30 5 1 4 6                               \n"
        "M  V30 6 1 2 7 CFG=1                         \n"
        "M  V30 7 1 3 8 CFG=1                         \n"
        "M  V30 END BOND                              \n"
        "M  V30 BEGIN COLLECTION                      \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 2 3)          \n"
        "M  V30 END COLLECTION                        \n"
        "M  V30 END CTAB                              \n"
        "M  END                                       \n";

    inchi_Output output;
    inchi_Output *poutput = &output;

    const char expected_enh[] = "InChI=1B/C4H6BrClO2/c1-2(6)3(5)4(7)8/h2-3H,1H3,(H,7,8)/t2-,3+/s2/f/h7H";

    char options_enh[] = "-EnhancedStereochemistry -FixedH";
    ASSERT_EQ(MakeINCHIFromMolfileText(molblock, options_enh, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_enh);
    EXPECT_EQ(strstr(poutput->szInChI, "/m"), nullptr);
    const std::string enh = poutput->szInChI;
    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;
    FreeINCHI(poutput);

    char options_ref[] = "-SRel -FixedH";
    ASSERT_EQ(MakeINCHIFromMolfileText(molblock, options_ref, poutput), 0);
    const std::string ref = poutput->szInChI;
    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;
    FreeINCHI(poutput);

    EXPECT_EQ(enh.substr(strlen("InChI=1B/")), ref.substr(strlen("InChI=1/")));
}

/* SPEC 1 gate G9: the [BIOVIA] white-paper Appendix B / fig. 22 reference molfile, located in
   the wild as MDL_white_paper_fig22.v3000.sdf and persisted as datasets/biovia_white_paper_fig22.mol.
   29 atoms mixing all five collection types in one structure, so it cross-checks the whole
   grouping pipeline against a third-party authority at once: ABS (15) from STEABS(17), two OR
   groups (13,14) and (21,23), two AND groups (17) and (18), /m1 present because an ABS
   collection exists (the ES-R18 negative control), neither the two OR nor the two AND groups
   reduced (ES-R17 over-reduction guard), and every group's lowest centre normalised to '-'.
   Molblock inline so the gate does not depend on the fragile fixture path (ES-R15).
   NB: the upstream file terminates with "M END"; it must be "M  END" or InChI returns error 64. */
TEST(test_enhancedStereo, test_EnhancedStereochemistry_biovia_appendix_b)
{
    const char *molblock =
        "\n"
        "\n"
        "MDL-Draw12200218542D\n"
        "  0  0  0  0  0  0              0 V3000\n"
        "M  V30 BEGIN CTAB\n"
        "M  V30 COUNTS 29 32 0 0 0\n"
        "M  V30 BEGIN ATOM\n"
        "M  V30 1 C -0.0544 -0.428 0 0\n"
        "M  V30 2 C -0.0544 -1.278 0 0 CFG=2\n"
        "M  V30 3 C -0.7962 -1.6974 0 0\n"
        "M  V30 4 C -0.7962 -2.544 0 0\n"
        "M  V30 5 C -0.0703 -2.9817 0 0\n"
        "M  V30 6 C 0.6716 -2.5622 0 0 CFG=1\n"
        "M  V30 7 C 0.6716 -1.7051 0 0 CFG=2\n"
        "M  V30 8 C 1.4208 -1.2852 0 0\n"
        "M  V30 9 C 1.4093 -2.9942 0 0\n"
        "M  V30 10 C -1.2907 1.3481 0 0\n"
        "M  V30 11 C -2.0326 0.9232 0 0\n"
        "M  V30 12 C -2.0326 0.0734 0 0\n"
        "M  V30 13 C -1.293 -0.3579 0 0\n"
        "M  V30 14 C -0.5534 0.9274 0 0\n"
        "M  V30 15 C -0.5534 0.074 0 0\n"
        "M  V30 16 N 0.77 -0.3022 0 0\n"
        "M  V30 17 C 1.1707 0.466 0 0 CFG=1\n"
        "M  V30 18 C 0.8061 1.2704 0 0\n"
        "M  V30 19 N -0.0142 1.4289 0 0\n"
        "M  V30 20 O 1.3356 1.938 0 0\n"
        "M  V30 21 C -2.7699 1.3485 0 0 CFG=2\n"
        "M  V30 22 C -2.7699 2.2058 0 0 CFG=2\n"
        "M  V30 23 C -3.4976 2.6309 0 0 CFG=2\n"
        "M  V30 24 C -4.2412 2.2104 0 0\n"
        "M  V30 25 C -4.2412 1.355 0 0\n"
        "M  V30 26 O -3.4976 3.4805 0 0\n"
        "M  V30 27 O -2.0178 2.624 0 0\n"
        "M  V30 28 C 2.0249 0.466 0 0\n"
        "M  V30 29 C -3.4995 0.9299 0 0\n"
        "M  V30 END ATOM\n"
        "M  V30 BEGIN BOND\n"
        "M  V30 1 1 2 1 CFG=1\n"
        "M  V30 2 1 7 8 CFG=1\n"
        "M  V30 3 1 6 9 CFG=1\n"
        "M  V30 4 1 2 3\n"
        "M  V30 5 1 2 7\n"
        "M  V30 6 1 3 4\n"
        "M  V30 7 1 4 5\n"
        "M  V30 8 1 5 6\n"
        "M  V30 9 1 6 7\n"
        "M  V30 10 2 1 16\n"
        "M  V30 11 1 16 17\n"
        "M  V30 12 1 17 18\n"
        "M  V30 13 1 19 14\n"
        "M  V30 14 1 18 19\n"
        "M  V30 15 2 18 20\n"
        "M  V30 16 1 21 11 CFG=1\n"
        "M  V30 17 1 21 22\n"
        "M  V30 18 1 22 23\n"
        "M  V30 19 1 23 24\n"
        "M  V30 20 1 24 25\n"
        "M  V30 21 1 23 26 CFG=3\n"
        "M  V30 22 1 22 27 CFG=3\n"
        "M  V30 23 1 17 28 CFG=1\n"
        "M  V30 24 1 10 11\n"
        "M  V30 25 2 10 14\n"
        "M  V30 26 2 11 12\n"
        "M  V30 27 1 12 13\n"
        "M  V30 28 2 13 15\n"
        "M  V30 29 1 14 15\n"
        "M  V30 30 1 15 1\n"
        "M  V30 31 1 25 29\n"
        "M  V30 32 1 29 21\n"
        "M  V30 END BOND\n"
        "M  V30 BEGIN COLLECTION\n"
        "M  V30 MDLV30/STEABS ATOMS=(1 17)\n"
        "M  V30 MDLV30/STEREL2 ATOMS=(2 6 7)\n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 22 23)\n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 2)\n"
        "M  V30 MDLV30/STERAC1 ATOMS=(1 21)\n"
        "M  V30 END COLLECTION\n"
        "M  V30 END CTAB\n"
        "M  END\n"
;

    inchi_Output output;
    inchi_Output *poutput = &output;

    const char expected_enh[] =
        "InChI=1B/C24H34N2O3/c1-13-6-4-7-17(14(13)2)22-19-11-10-16(18-8-5-9-21(27)23(18)28)"
        "12-20(19)26-24(29)15(3)25-22/h10-15,17-18,21,23,27-28H,4-9H2,1-3H3,(H,26,29)/"
        "t13-,14-,15-,17-,18-,21-,23+/m1/s1(15)2(13,14)(21,23)3(17)(18)";

    char options_enh[] = "-EnhancedStereochemistry";
    ASSERT_EQ(MakeINCHIFromMolfileText(molblock, options_enh, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_enh);
    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;
    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_2_atropisomer)
{
    const char *molblock =
        "test_mol_atropisomer_1                        \n"
        "  -INDIGO-02052611532D                       \n"
        "                                             \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000      \n"
        "M  V30 BEGIN CTAB                            \n"
        "M  V30 COUNTS 16 17 0 0 0                    \n"
        "M  V30 BEGIN ATOM                            \n"
        "M  V30 1 C 6.11597 -6.27499 0.0 0            \n"
        "M  V30 2 C 5.24996 -5.775 0.0 0              \n"
        "M  V30 3 C 4.38395 -6.27499 0.0 0            \n"
        "M  V30 4 C 4.38395 -7.27499 0.0 0            \n"
        "M  V30 5 C 5.24996 -7.77498 0.0 0            \n"
        "M  V30 6 C 6.11597 -7.27499 0.0 0            \n"
        "M  V30 7 C 6.11605 -4.27501 0.0 0            \n"
        "M  V30 8 C 5.24996 -4.775 0.0 0              \n"
        "M  V30 9 C 4.38395 -4.27501 0.0 0            \n"
        "M  V30 10 C 4.38395 -3.27501 0.0 0           \n"
        "M  V30 11 C 6.11605 -3.27501 0.0 0           \n"
        "M  V30 12 C 5.25004 -2.77502 0.0 0           \n"
        "M  V30 13 Br 6.98205 -4.775 0.0 0            \n"
        "M  V30 14 Cl 3.51795 -4.775 0.0 0            \n"
        "M  V30 15 Cl 3.51795 -5.77492 0.0 0          \n"
        "M  V30 16 Br 6.98205 -5.775 0.0 0            \n"
        "M  V30 END ATOM                              \n"
        "M  V30 BEGIN BOND                            \n"
        "M  V30 1 1 1 6                               \n"
        "M  V30 2 2 1 2                               \n"
        "M  V30 3 1 2 3 CFG=1                         \n"
        "M  V30 4 1 2 8                               \n"
        "M  V30 5 2 3 4                               \n"
        "M  V30 6 1 5 4                               \n"
        "M  V30 7 2 5 6                               \n"
        "M  V30 8 1 8 7 CFG=1                         \n"
        "M  V30 9 2 7 11                              \n"
        "M  V30 10 2 8 9                              \n"
        "M  V30 11 1 9 10                             \n"
        "M  V30 12 2 10 12                            \n"
        "M  V30 13 1 12 11                            \n"
        "M  V30 14 1 7 13                             \n"
        "M  V30 15 1 9 14                             \n"
        "M  V30 16 1 3 15                             \n"
        "M  V30 17 1 1 16                             \n"
        "M  V30 END BOND                              \n"
        "M  V30 BEGIN COLLECTION                      \n"
        "M  V30 MDLV30/STEABS ATOMS=(2 2 8)           \n"
        "M  V30 END COLLECTION                        \n"
        "M  V30 END CTAB                              \n"
        "M  END                                       \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_3_empty_collection_info)
{
    const char *molblock =
        "broken_mol?                                \n"
        "  -INDIGO-02062608442D                     \n"
        "                                           \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000    \n"
        "M  V30 BEGIN CTAB                          \n"
        "M  V30 COUNTS 9 8 0 0 0                    \n"
        "M  V30 BEGIN ATOM                          \n"
        "M  V30 1 C 4.64199 -4.9 0.0 0              \n"
        "M  V30 2 C 5.50801 -4.4 0.0 0 CFG=2        \n"
        "M  V30 3 C 6.37404 -4.9 0.0 0 CFG=2        \n"
        "M  V30 4 C 7.24006 -4.4 0.0 0 CFG=2        \n"
        "M  V30 5 C 8.10609 -4.9 0.0 0              \n"
        "M  V30 6 C 6.37404 -5.9 0.0 0              \n"
        "M  V30 7 C 5.50801 -3.4 0.0 0              \n"
        "M  V30 8 C 8.97211 -4.4 0.0 0              \n"
        "M  V30 9 C 7.24006 -3.4 0.0 0              \n"
        "M  V30 END ATOM                            \n"
        "M  V30 BEGIN BOND                          \n"
        "M  V30 1 1 1 2                             \n"
        "M  V30 2 1 2 3                             \n"
        "M  V30 3 1 3 4                             \n"
        "M  V30 4 1 4 5                             \n"
        "M  V30 5 1 3 6 CFG=3                       \n"
        "M  V30 6 1 2 7 CFG=1                       \n"
        "M  V30 7 1 5 8                             \n"
        "M  V30 8 1 4 9 CFG=1                       \n"
        "M  V30 END BOND                            \n"
        "M  V30 BEGIN COLLECTION                    \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(1 )           \n"
        "M  V30 MDLV30/STEABS ATOMS=(2  )           \n"
        "M  V30 END COLLECTION                      \n"
        "M  V30 END CTAB                            \n"
        "M  END                                     \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C9H20/c1-6-8(4)9(5)7(2)3/h7-9H,6H2,1-5H3/t8-,9+/m0";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_3_wrong_atoms_in_collection)
{
    const char *molblock =
        "broken_mol?                                \n"
        "  -INDIGO-02062608442D                     \n"
        "                                           \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000    \n"
        "M  V30 BEGIN CTAB                          \n"
        "M  V30 COUNTS 9 8 0 0 0                    \n"
        "M  V30 BEGIN ATOM                          \n"
        "M  V30 1 C 4.64199 -4.9 0.0 0              \n"
        "M  V30 2 C 5.50801 -4.4 0.0 0 CFG=2        \n"
        "M  V30 3 C 6.37404 -4.9 0.0 0 CFG=2        \n"
        "M  V30 4 C 7.24006 -4.4 0.0 0 CFG=2        \n"
        "M  V30 5 C 8.10609 -4.9 0.0 0              \n"
        "M  V30 6 C 6.37404 -5.9 0.0 0              \n"
        "M  V30 7 C 5.50801 -3.4 0.0 0              \n"
        "M  V30 8 C 8.97211 -4.4 0.0 0              \n"
        "M  V30 9 C 7.24006 -3.4 0.0 0              \n"
        "M  V30 END ATOM                            \n"
        "M  V30 BEGIN BOND                          \n"
        "M  V30 1 1 1 2                             \n"
        "M  V30 2 1 2 3                             \n"
        "M  V30 3 1 3 4                             \n"
        "M  V30 4 1 4 5                             \n"
        "M  V30 5 1 3 6 CFG=3                       \n"
        "M  V30 6 1 2 7 CFG=1                       \n"
        "M  V30 7 1 5 8                             \n"
        "M  V30 8 1 4 9 CFG=1                       \n"
        "M  V30 END BOND                            \n"
        "M  V30 BEGIN COLLECTION                    \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(1 -2)         \n"
        "M  V30 MDLV30/STEABS ATOMS=(2 13 43)       \n"
        "M  V30 END COLLECTION                      \n"
        "M  V30 END CTAB                            \n"
        "M  END                                     \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C9H20/c1-6-8(4)9(5)7(2)3/h7-9H,6H2,1-5H3/t8-,9+/m0";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_2_mols)
{
    const char *molblock =
        "test_mol                                           \n"
        "  -INDIGO-01232613552D                             \n"
        "                                                   \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000            \n"
        "M  V30 BEGIN CTAB                                  \n"
        "M  V30 COUNTS 36 34 0 0 0                          \n"
        "M  V30 BEGIN ATOM                                  \n"
        "M  V30 1 C 5.83301 -3.275 0.0 0 CFG=1              \n"
        "M  V30 2 C 4.96699 -2.775 0.0 0 CFG=1              \n"
        "M  V30 3 C 4.10096 -3.275 0.0 0 CFG=2              \n"
        "M  V30 4 C 3.23493 -2.775 0.0 0 CFG=1              \n"
        "M  V30 5 C 2.36891 -3.275 0.0 0 CFG=2              \n"
        "M  V30 6 C 1.50289 -2.775 0.0 0                    \n"
        "M  V30 7 Br 2.36891 -4.275 0.0 0                   \n"
        "M  V30 8 Cl 3.23493 -1.775 0.0 0                   \n"
        "M  V30 9 Cl 4.10096 -4.275 0.0 0                   \n"
        "M  V30 10 Cl 4.96699 -1.775 0.0 0                  \n"
        "M  V30 11 Cl 5.83301 -4.275 0.0 0                  \n"
        "M  V30 12 C 6.69904 -2.775 0.0 0 CFG=1             \n"
        "M  V30 13 C 7.56506 -3.275 0.0 0 CFG=1             \n"
        "M  V30 14 C 8.43109 -2.77501 0.0 0 CFG=2           \n"
        "M  V30 15 C 9.29711 -3.275 0.0 0                   \n"
        "M  V30 16 Cl 8.43109 -1.77501 0.0 0                \n"
        "M  V30 17 Cl 7.56506 -4.275 0.0 0                  \n"
        "M  V30 18 Cl 6.69904 -1.775 0.0 0                  \n"
        "M  V30 19 C 6.13302 -7.62502 0.0 0 CFG=2           \n"
        "M  V30 20 C 5.26698 -7.12498 0.0 0 CFG=1           \n"
        "M  V30 21 C 4.40101 -7.62502 0.0 0 CFG=2           \n"
        "M  V30 22 C 3.53496 -7.12498 0.0 0 CFG=1           \n"
        "M  V30 23 C 2.66891 -7.62502 0.0 0 CFG=2           \n"
        "M  V30 24 C 6.99899 -7.12498 0.0 0 CFG=2           \n"
        "M  V30 25 C 7.86504 -7.62502 0.0 0 CFG=2           \n"
        "M  V30 26 C 8.73109 -7.12498 0.0 0 CFG=2           \n"
        "M  V30 27 C 1.80295 -7.12498 0.0 0                 \n"
        "M  V30 28 Br 2.66891 -8.62501 0.0 0                \n"
        "M  V30 29 Cl 3.53496 -6.12499 0.0 0                \n"
        "M  V30 30 Cl 4.40101 -8.62501 0.0 0                \n"
        "M  V30 31 Cl 5.26698 -6.12499 0.0 0                \n"
        "M  V30 32 Cl 6.13302 -8.62501 0.0 0                \n"
        "M  V30 33 C 9.59705 -7.62502 0.0 0                 \n"
        "M  V30 34 Cl 8.73109 -6.12499 0.0 0                \n"
        "M  V30 35 Cl 7.86504 -8.62501 0.0 0                \n"
        "M  V30 36 Cl 6.99899 -6.12499 0.0 0                \n"
        "M  V30 END ATOM                                    \n"
        "M  V30 BEGIN BOND                                  \n"
        "M  V30 1 1 1 2                                     \n"
        "M  V30 2 1 1 11 CFG=3                              \n"
        "M  V30 3 1 1 12                                    \n"
        "M  V30 4 1 2 3                                     \n"
        "M  V30 5 1 2 10 CFG=1                              \n"
        "M  V30 6 1 3 4                                     \n"
        "M  V30 7 1 3 9 CFG=1                               \n"
        "M  V30 8 1 4 5                                     \n"
        "M  V30 9 1 4 8 CFG=1                               \n"
        "M  V30 10 1 5 6                                    \n"
        "M  V30 11 1 5 7 CFG=1                              \n"
        "M  V30 12 1 12 13                                  \n"
        "M  V30 13 1 12 18 CFG=3                            \n"
        "M  V30 14 1 13 14                                  \n"
        "M  V30 15 1 13 17 CFG=1                            \n"
        "M  V30 16 1 14 15                                  \n"
        "M  V30 17 1 14 16 CFG=1                            \n"
        "M  V30 18 1 19 20                                  \n"
        "M  V30 19 1 20 21                                  \n"
        "M  V30 20 1 21 22                                  \n"
        "M  V30 21 1 22 23                                  \n"
        "M  V30 22 1 19 24                                  \n"
        "M  V30 23 1 24 25                                  \n"
        "M  V30 24 1 25 26                                  \n"
        "M  V30 25 1 23 27                                  \n"
        "M  V30 26 1 23 28 CFG=1                            \n"
        "M  V30 27 1 22 29 CFG=1                            \n"
        "M  V30 28 1 21 30 CFG=1                            \n"
        "M  V30 29 1 20 31 CFG=1                            \n"
        "M  V30 30 1 19 32 CFG=3                            \n"
        "M  V30 31 1 26 33                                  \n"
        "M  V30 32 1 26 34 CFG=1                            \n"
        "M  V30 33 1 25 35 CFG=3                            \n"
        "M  V30 34 1 24 36 CFG=1                            \n"
        "M  V30 END BOND                                    \n"
        "M  V30 BEGIN COLLECTION                            \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 1)                  \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(2 2 3)                \n"
        "M  V30 MDLV30/STEABS ATOMS=(4 4 5 22 23)           \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 12 13)              \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(1 14)                 \n"
        "M  V30 MDLV30/STERAC4 ATOMS=(1 19)                 \n"
        "M  V30 MDLV30/STERAC3 ATOMS=(2 20 21)              \n"
        "M  V30 MDLV30/STEREL3 ATOMS=(2 24 25)              \n"
        "M  V30 MDLV30/STEREL4 ATOMS=(1 26)                 \n"
        "M  V30 END COLLECTION                              \n"
        "M  V30 END CTAB                                    \n"
        "M  END                                             \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/2C10H14BrCl7/c2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h2*3-10H,1-2H3/t2*3-,4-,5+,6-,7-,8-,9+,10-/m00/s2*1(3,5)2(4)(6,8)3(7,9)(10)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_3_mols)
{
    const char *molblock =
        "test mol                                            \n"
        "  -INDIGO-01272609172D                              \n"
        "													 \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000             \n"
        "M  V30 BEGIN CTAB                                   \n"
        "M  V30 COUNTS 44 41 0 0 0                           \n"
        "M  V30 BEGIN ATOM                                   \n"
        "M  V30 1 C 5.48304 -3.77499 0.0 0 CFG=1             \n"
        "M  V30 2 C 4.61702 -3.27499 0.0 0 CFG=1             \n"
        "M  V30 3 C 3.75099 -3.77499 0.0 0 CFG=2             \n"
        "M  V30 4 C 2.88496 -3.27499 0.0 0 CFG=1             \n"
        "M  V30 5 C 2.01894 -3.77499 0.0 0 CFG=2             \n"
        "M  V30 6 C 1.15292 -3.27499 0.0 0                   \n"
        "M  V30 7 Br 2.01894 -4.77499 0.0 0                  \n"
        "M  V30 8 Cl 2.88496 -2.27499 0.0 0                  \n"
        "M  V30 9 Cl 3.75099 -4.77499 0.0 0                  \n"
        "M  V30 10 Cl 4.61702 -2.27499 0.0 0                 \n"
        "M  V30 11 Cl 5.48304 -4.77499 0.0 0                 \n"
        "M  V30 12 C 6.34907 -3.27499 0.0 0 CFG=1            \n"
        "M  V30 13 C 7.21509 -3.77499 0.0 0 CFG=1            \n"
        "M  V30 14 C 8.08112 -3.275 0.0 0 CFG=2              \n"
        "M  V30 15 C 8.94714 -3.77499 0.0 0                  \n"
        "M  V30 16 Cl 8.08112 -2.275 0.0 0                   \n"
        "M  V30 17 Cl 7.21509 -4.77499 0.0 0                 \n"
        "M  V30 18 Cl 6.34907 -2.27499 0.0 0                 \n"
        "M  V30 19 C 5.78305 -8.12502 0.0 0 CFG=2            \n"
        "M  V30 20 C 4.91701 -7.62498 0.0 0 CFG=1            \n"
        "M  V30 21 C 4.05104 -8.12502 0.0 0 CFG=2            \n"
        "M  V30 22 C 3.18499 -7.62498 0.0 0 CFG=1            \n"
        "M  V30 23 C 2.31894 -8.12502 0.0 0 CFG=2            \n"
        "M  V30 24 C 6.64902 -7.62498 0.0 0 CFG=2            \n"
        "M  V30 25 C 7.51507 -8.12502 0.0 0 CFG=2            \n"
        "M  V30 26 C 8.38112 -7.62498 0.0 0 CFG=2            \n"
        "M  V30 27 C 1.45298 -7.62498 0.0 0                  \n"
        "M  V30 28 Br 2.31894 -9.12501 0.0 0                 \n"
        "M  V30 29 Cl 3.18499 -6.62499 0.0 0                 \n"
        "M  V30 30 Cl 4.05104 -9.12501 0.0 0                 \n"
        "M  V30 31 Cl 4.91701 -6.62499 0.0 0                 \n"
        "M  V30 32 Cl 5.78305 -9.12501 0.0 0                 \n"
        "M  V30 33 C 9.24708 -8.12502 0.0 0                  \n"
        "M  V30 34 Cl 8.38112 -6.62499 0.0 0                 \n"
        "M  V30 35 Cl 7.51507 -9.12501 0.0 0                 \n"
        "M  V30 36 Cl 6.64902 -6.62499 0.0 0                 \n"
        "M  V30 37 C 14.367 -4.45 0.0 0 CFG=1                \n"
        "M  V30 38 C 15.233 -3.95 0.0 0 CFG=2                \n"
        "M  V30 39 C 16.099 -4.45 0.0 0                      \n"
        "M  V30 40 C 15.233 -2.95 0.0 0                      \n"
        "M  V30 41 C 13.501 -3.95 0.0 0 CFG=1                \n"
        "M  V30 42 C 12.6349 -4.45 0.0 0                     \n"
        "M  V30 43 C 13.501 -2.95 0.0 0                      \n"
        "M  V30 44 C 14.367 -5.45 0.0 0                      \n"
        "M  V30 END ATOM                                     \n"
        "M  V30 BEGIN BOND                                   \n"
        "M  V30 1 1 1 2                                      \n"
        "M  V30 2 1 1 11 CFG=3                               \n"
        "M  V30 3 1 1 12                                     \n"
        "M  V30 4 1 2 3                                      \n"
        "M  V30 5 1 2 10 CFG=1                               \n"
        "M  V30 6 1 3 4                                      \n"
        "M  V30 7 1 3 9 CFG=1                                \n"
        "M  V30 8 1 4 5                                      \n"
        "M  V30 9 1 4 8 CFG=1                                \n"
        "M  V30 10 1 5 6                                     \n"
        "M  V30 11 1 5 7 CFG=1                               \n"
        "M  V30 12 1 12 13                                   \n"
        "M  V30 13 1 12 18 CFG=3                             \n"
        "M  V30 14 1 13 14                                   \n"
        "M  V30 15 1 13 17 CFG=1                             \n"
        "M  V30 16 1 14 15                                   \n"
        "M  V30 17 1 14 16 CFG=1                             \n"
        "M  V30 18 1 19 20                                   \n"
        "M  V30 19 1 20 21                                   \n"
        "M  V30 20 1 21 22                                   \n"
        "M  V30 21 1 22 23                                   \n"
        "M  V30 22 1 19 24                                   \n"
        "M  V30 23 1 24 25                                   \n"
        "M  V30 24 1 25 26                                   \n"
        "M  V30 25 1 23 27                                   \n"
        "M  V30 26 1 23 28 CFG=1                             \n"
        "M  V30 27 1 22 29 CFG=1                             \n"
        "M  V30 28 1 21 30 CFG=1                             \n"
        "M  V30 29 1 20 31 CFG=1                             \n"
        "M  V30 30 1 19 32 CFG=3                             \n"
        "M  V30 31 1 26 33                                   \n"
        "M  V30 32 1 26 34 CFG=1                             \n"
        "M  V30 33 1 25 35 CFG=3                             \n"
        "M  V30 34 1 24 36 CFG=1                             \n"
        "M  V30 35 1 37 38                                   \n"
        "M  V30 36 1 38 39                                   \n"
        "M  V30 37 1 38 40 CFG=1                             \n"
        "M  V30 38 1 37 41                                   \n"
        "M  V30 39 1 41 42                                   \n"
        "M  V30 40 1 41 43 CFG=1                             \n"
        "M  V30 41 1 37 44 CFG=3                             \n"
        "M  V30 END BOND                                     \n"
        "M  V30 BEGIN COLLECTION                             \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 1)                   \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(2 2 3)                 \n"
        "M  V30 MDLV30/STEABS ATOMS=(5 4 5 22 23 38)         \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 12 13)               \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(1 14)                  \n"
        "M  V30 MDLV30/STERAC4 ATOMS=(1 19)                  \n"
        "M  V30 MDLV30/STERAC3 ATOMS=(2 20 21)               \n"
        "M  V30 MDLV30/STEREL3 ATOMS=(2 24 25)               \n"
        "M  V30 MDLV30/STEREL4 ATOMS=(1 26)                  \n"
        "M  V30 MDLV30/STERAC5 ATOMS=(2 37 41)               \n"
        "M  V30 END COLLECTION                               \n"
        "M  V30 END CTAB                                     \n"
        "M  END                                              \n";


    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/2C10H14BrCl7.C8H18/c2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12;1-6(2)8(5)7(3)4/h2*3-10H,1-2H3;6-8H,1-5H3/t2*3-,4-,5+,6-,7-,8-,9+,10-;/m00./s2*1(3,5)2(4)(6,8)3(7,9)(10);1(6)3(7,8)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_4_mols)
{
    const char *molblock =
        "test_mol                                                              \n"
        "  -INDIGO-01272609502D                                                \n"
        "                                                                      \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000                               \n"
        "M  V30 BEGIN CTAB                                                     \n"
        "M  V30 COUNTS 57 53 0 0 0                                             \n"
        "M  V30 BEGIN ATOM                                                     \n"
        "M  V30 1 C 5.48304 -3.77499 0.0 0 CFG=1                               \n"
        "M  V30 2 C 4.61702 -3.27499 0.0 0 CFG=1                               \n"
        "M  V30 3 C 3.75099 -3.77499 0.0 0 CFG=2                               \n"
        "M  V30 4 C 2.88496 -3.27499 0.0 0 CFG=1                               \n"
        "M  V30 5 C 2.01894 -3.77499 0.0 0 CFG=2                               \n"
        "M  V30 6 C 1.15292 -3.27499 0.0 0                                     \n"
        "M  V30 7 Br 2.01894 -4.77499 0.0 0                                    \n"
        "M  V30 8 Cl 2.88496 -2.27499 0.0 0                                    \n"
        "M  V30 9 Cl 3.75099 -4.77499 0.0 0                                    \n"
        "M  V30 10 Cl 4.61702 -2.27499 0.0 0                                   \n"
        "M  V30 11 Cl 5.48304 -4.77499 0.0 0                                   \n"
        "M  V30 12 C 6.34907 -3.27499 0.0 0 CFG=1                              \n"
        "M  V30 13 C 7.21509 -3.77499 0.0 0 CFG=1                              \n"
        "M  V30 14 C 8.08112 -3.275 0.0 0 CFG=2                                \n"
        "M  V30 15 C 8.94714 -3.77499 0.0 0                                    \n"
        "M  V30 16 Cl 8.08112 -2.275 0.0 0                                     \n"
        "M  V30 17 Cl 7.21509 -4.77499 0.0 0                                   \n"
        "M  V30 18 Cl 6.34907 -2.27499 0.0 0                                   \n"
        "M  V30 19 C 5.78305 -8.12502 0.0 0 CFG=2                              \n"
        "M  V30 20 C 4.91701 -7.62498 0.0 0 CFG=1                              \n"
        "M  V30 21 C 4.05104 -8.12502 0.0 0 CFG=2                              \n"
        "M  V30 22 C 3.18499 -7.62498 0.0 0 CFG=1                              \n"
        "M  V30 23 C 2.31894 -8.12502 0.0 0 CFG=2                              \n"
        "M  V30 24 C 6.64902 -7.62498 0.0 0 CFG=2                              \n"
        "M  V30 25 C 7.51507 -8.12502 0.0 0 CFG=2                              \n"
        "M  V30 26 C 8.38112 -7.62498 0.0 0 CFG=2                              \n"
        "M  V30 27 C 1.45298 -7.62498 0.0 0                                    \n"
        "M  V30 28 Br 2.31894 -9.12501 0.0 0                                   \n"
        "M  V30 29 Cl 3.18499 -6.62499 0.0 0                                   \n"
        "M  V30 30 Cl 4.05104 -9.12501 0.0 0                                   \n"
        "M  V30 31 Cl 4.91701 -6.62499 0.0 0                                   \n"
        "M  V30 32 Cl 5.78305 -9.12501 0.0 0                                   \n"
        "M  V30 33 C 9.24708 -8.12502 0.0 0                                    \n"
        "M  V30 34 Cl 8.38112 -6.62499 0.0 0                                   \n"
        "M  V30 35 Cl 7.51507 -9.12501 0.0 0                                   \n"
        "M  V30 36 Cl 6.64902 -6.62499 0.0 0                                   \n"
        "M  V30 37 C 14.367 -4.45 0.0 0 CFG=1                                  \n"
        "M  V30 38 C 15.233 -3.95 0.0 0 CFG=2                                  \n"
        "M  V30 39 C 16.099 -4.45 0.0 0                                        \n"
        "M  V30 40 C 15.233 -2.95 0.0 0                                        \n"
        "M  V30 41 C 13.501 -3.95 0.0 0 CFG=1                                  \n"
        "M  V30 42 C 12.6349 -4.45 0.0 0                                       \n"
        "M  V30 43 C 13.501 -2.95 0.0 0                                        \n"
        "M  V30 44 C 14.367 -5.45 0.0 0                                        \n"
        "M  V30 45 C 11.7689 -3.95 0.0 0                                       \n"
        "M  V30 46 C 11.417 -7.75 0.0 0                                        \n"
        "M  V30 47 C 12.283 -7.25 0.0 0 CFG=2                                  \n"
        "M  V30 48 C 13.149 -7.75 0.0 0                                        \n"
        "M  V30 49 C 14.0151 -7.25 0.0 0 CFG=2                                 \n"
        "M  V30 50 C 14.8811 -7.75 0.0 0                                       \n"
        "M  V30 51 C 13.149 -8.75 0.0 0 CFG=2                                  \n"
        "M  V30 52 C 12.283 -9.25 0.0 0                                        \n"
        "M  V30 53 C 14.0151 -9.25 0.0 0 CFG=2                                 \n"
        "M  V30 54 C 14.0151 -10.25 0.0 0                                      \n"
        "M  V30 55 C 14.8811 -8.75 0.0 0                                       \n"
        "M  V30 56 C 14.0151 -6.25 0.0 0                                       \n"
        "M  V30 57 C 12.283 -6.25 0.0 0                                        \n"
        "M  V30 END ATOM                                                       \n"
        "M  V30 BEGIN BOND                                                     \n"
        "M  V30 1 1 1 2                                                        \n"
        "M  V30 2 1 1 11 CFG=3                                                 \n"
        "M  V30 3 1 1 12                                                       \n"
        "M  V30 4 1 2 3                                                        \n"
        "M  V30 5 1 2 10 CFG=1                                                 \n"
        "M  V30 6 1 3 4                                                        \n"
        "M  V30 7 1 3 9 CFG=1                                                  \n"
        "M  V30 8 1 4 5                                                        \n"
        "M  V30 9 1 4 8 CFG=1                                                  \n"
        "M  V30 10 1 5 6                                                       \n"
        "M  V30 11 1 5 7 CFG=1                                                 \n"
        "M  V30 12 1 12 13                                                     \n"
        "M  V30 13 1 12 18 CFG=3                                               \n"
        "M  V30 14 1 13 14                                                     \n"
        "M  V30 15 1 13 17 CFG=1                                               \n"
        "M  V30 16 1 14 15                                                     \n"
        "M  V30 17 1 14 16 CFG=1                                               \n"
        "M  V30 18 1 19 20                                                     \n"
        "M  V30 19 1 20 21                                                     \n"
        "M  V30 20 1 21 22                                                     \n"
        "M  V30 21 1 22 23                                                     \n"
        "M  V30 22 1 19 24                                                     \n"
        "M  V30 23 1 24 25                                                     \n"
        "M  V30 24 1 25 26                                                     \n"
        "M  V30 25 1 23 27                                                     \n"
        "M  V30 26 1 23 28 CFG=1                                               \n"
        "M  V30 27 1 22 29 CFG=1                                               \n"
        "M  V30 28 1 21 30 CFG=1                                               \n"
        "M  V30 29 1 20 31 CFG=1                                               \n"
        "M  V30 30 1 19 32 CFG=3                                               \n"
        "M  V30 31 1 26 33                                                     \n"
        "M  V30 32 1 26 34 CFG=1                                               \n"
        "M  V30 33 1 25 35 CFG=3                                               \n"
        "M  V30 34 1 24 36 CFG=1                                               \n"
        "M  V30 35 1 37 38                                                     \n"
        "M  V30 36 1 38 39                                                     \n"
        "M  V30 37 1 38 40 CFG=1                                               \n"
        "M  V30 38 1 37 41                                                     \n"
        "M  V30 39 1 41 42                                                     \n"
        "M  V30 40 1 41 43 CFG=1                                               \n"
        "M  V30 41 1 37 44 CFG=3                                               \n"
        "M  V30 42 1 42 45                                                     \n"
        "M  V30 43 1 46 47                                                     \n"
        "M  V30 44 1 47 48                                                     \n"
        "M  V30 45 1 48 49                                                     \n"
        "M  V30 46 1 49 50                                                     \n"
        "M  V30 47 1 48 51                                                     \n"
        "M  V30 48 1 51 52 CFG=1                                               \n"
        "M  V30 49 1 51 53                                                     \n"
        "M  V30 50 1 53 54                                                     \n"
        "M  V30 51 1 53 55 CFG=1                                               \n"
        "M  V30 52 1 49 56 CFG=1                                               \n"
        "M  V30 53 1 47 57 CFG=1                                               \n"
        "M  V30 END BOND                                                       \n"
        "M  V30 BEGIN COLLECTION                                               \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 1)                                     \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(2 2 3)                                   \n"
        "M  V30 MDLV30/STEABS ATOMS=(9 4 5 22 23 38 47 49 51 53)               \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(2 12 13)                                 \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(1 14)                                    \n"
        "M  V30 MDLV30/STERAC4 ATOMS=(1 19)                                    \n"
        "M  V30 MDLV30/STERAC3 ATOMS=(2 20 21)                                 \n"
        "M  V30 MDLV30/STEREL3 ATOMS=(2 24 25)                                 \n"
        "M  V30 MDLV30/STEREL4 ATOMS=(1 26)                                    \n"
        "M  V30 MDLV30/STERAC5 ATOMS=(2 37 41)                                 \n"
        "M  V30 END COLLECTION                                                 \n"
        "M  V30 END CTAB                                                       \n"
        "M  END                                                                \n";


    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C12H26.2C10H14BrCl7.C9H20/c1-8(2)11(7)12(9(3)4)10(5)6;2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12;1-6-8(4)9(5)7(2)3/h8-12H,1-7H3;2*3-10H,1-2H3;7-9H,6H2,1-5H3/t11-;2*3-,4-,5+,6-,7-,8-,9+,10-;8-,9+/m1001/s1;2*1(3,5)2(4)(6,8)3(7,9)(10);1(7)3(8,9)";
    // First component contributes ABS atoms only, so its /s reduces to a bare
    // "1" (ES-R11); the other components keep their groups.

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_2_mols_inter_enhstereo_grps_1)
{
    const char *molblock =
        "my_test_mol                                      \n"
        "  -INDIGO-01232613442D                           \n"
        "                                                 \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000          \n"
        "M  V30 BEGIN CTAB                                \n"
        "M  V30 COUNTS 36 34 0 0 0                        \n"
        "M  V30 BEGIN ATOM                                \n"
        "M  V30 1 C 5.83301 -3.275 0.0 0 CFG=1            \n"
        "M  V30 2 C 4.96699 -2.775 0.0 0 CFG=1            \n"
        "M  V30 3 C 4.10096 -3.275 0.0 0 CFG=2            \n"
        "M  V30 4 C 3.23493 -2.775 0.0 0 CFG=1            \n"
        "M  V30 5 C 2.36891 -3.275 0.0 0 CFG=2            \n"
        "M  V30 6 C 1.50289 -2.775 0.0 0                  \n"
        "M  V30 7 Br 2.36891 -4.275 0.0 0                 \n"
        "M  V30 8 Cl 3.23493 -1.775 0.0 0                 \n"
        "M  V30 9 Cl 4.10096 -4.275 0.0 0                 \n"
        "M  V30 10 Cl 4.96699 -1.775 0.0 0                \n"
        "M  V30 11 Cl 5.83301 -4.275 0.0 0                \n"
        "M  V30 12 C 6.69904 -2.775 0.0 0 CFG=1           \n"
        "M  V30 13 C 7.56506 -3.275 0.0 0 CFG=1           \n"
        "M  V30 14 C 8.43109 -2.77501 0.0 0 CFG=2         \n"
        "M  V30 15 C 9.29711 -3.275 0.0 0                 \n"
        "M  V30 16 Cl 8.43109 -1.77501 0.0 0              \n"
        "M  V30 17 Cl 7.56506 -4.275 0.0 0                \n"
        "M  V30 18 Cl 6.69904 -1.775 0.0 0                \n"
        "M  V30 19 C 6.13302 -7.62502 0.0 0 CFG=2         \n"
        "M  V30 20 C 5.26698 -7.12498 0.0 0 CFG=1         \n"
        "M  V30 21 C 4.40101 -7.62502 0.0 0 CFG=2         \n"
        "M  V30 22 C 3.53496 -7.12498 0.0 0 CFG=1         \n"
        "M  V30 23 C 2.66891 -7.62502 0.0 0 CFG=2         \n"
        "M  V30 24 C 6.99899 -7.12498 0.0 0 CFG=2         \n"
        "M  V30 25 C 7.86504 -7.62502 0.0 0 CFG=2         \n"
        "M  V30 26 C 8.73109 -7.12498 0.0 0 CFG=2         \n"
        "M  V30 27 C 1.80295 -7.12498 0.0 0               \n"
        "M  V30 28 Br 2.66891 -8.62501 0.0 0              \n"
        "M  V30 29 Cl 3.53496 -6.12499 0.0 0              \n"
        "M  V30 30 Cl 4.40101 -8.62501 0.0 0              \n"
        "M  V30 31 Cl 5.26698 -6.12499 0.0 0              \n"
        "M  V30 32 Cl 6.13302 -8.62501 0.0 0              \n"
        "M  V30 33 C 9.59705 -7.62502 0.0 0               \n"
        "M  V30 34 Cl 8.73109 -6.12499 0.0 0              \n"
        "M  V30 35 Cl 7.86504 -8.62501 0.0 0              \n"
        "M  V30 36 Cl 6.99899 -6.12499 0.0 0              \n"
        "M  V30 END ATOM                                  \n"
        "M  V30 BEGIN BOND                                \n"
        "M  V30 1 1 1 2                                   \n"
        "M  V30 2 1 1 11 CFG=3                            \n"
        "M  V30 3 1 1 12                                  \n"
        "M  V30 4 1 2 3                                   \n"
        "M  V30 5 1 2 10 CFG=1                            \n"
        "M  V30 6 1 3 4                                   \n"
        "M  V30 7 1 3 9 CFG=1                             \n"
        "M  V30 8 1 4 5                                   \n"
        "M  V30 9 1 4 8 CFG=1                             \n"
        "M  V30 10 1 5 6                                  \n"
        "M  V30 11 1 5 7 CFG=1                            \n"
        "M  V30 12 1 12 13                                \n"
        "M  V30 13 1 12 18 CFG=3                          \n"
        "M  V30 14 1 13 14                                \n"
        "M  V30 15 1 13 17 CFG=1                          \n"
        "M  V30 16 1 14 15                                \n"
        "M  V30 17 1 14 16 CFG=1                          \n"
        "M  V30 18 1 19 20                                \n"
        "M  V30 19 1 20 21                                \n"
        "M  V30 20 1 21 22                                \n"
        "M  V30 21 1 22 23                                \n"
        "M  V30 22 1 19 24                                \n"
        "M  V30 23 1 24 25                                \n"
        "M  V30 24 1 25 26                                \n"
        "M  V30 25 1 23 27                                \n"
        "M  V30 26 1 23 28 CFG=1                          \n"
        "M  V30 27 1 22 29 CFG=1                          \n"
        "M  V30 28 1 21 30 CFG=1                          \n"
        "M  V30 29 1 20 31 CFG=1                          \n"
        "M  V30 30 1 19 32 CFG=3                          \n"
        "M  V30 31 1 26 33                                \n"
        "M  V30 32 1 26 34 CFG=1                          \n"
        "M  V30 33 1 25 35 CFG=3                          \n"
        "M  V30 34 1 24 36 CFG=1                          \n"
        "M  V30 END BOND                                  \n"
        "M  V30 BEGIN COLLECTION                          \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(2 1 19)             \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(4 2 3 20 21)        \n"
        "M  V30 MDLV30/STEABS ATOMS=(4 4 5 22 23)         \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(4 12 13 24 25)      \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(2 14 26)            \n"
        "M  V30 END COLLECTION                            \n"
        "M  V30 END CTAB                                  \n"
        "M  END                                           \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/2C10H14BrCl7/c2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h2*3-10H,1-2H3/t2*3-,4-,5+,6-,7-,8-,9+,10-/m00/s2*1(3,5)2(4)(6,8)3(7,9)(10)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_2_mols_inter_enhstereo_grps_2)
{
    const char *molblock =
        "test_mol                                              \n"
        "  -INDIGO-02042609012D                                \n"
        "                                                      \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000               \n"
        "M  V30 BEGIN CTAB                                     \n"
        "M  V30 COUNTS 18 16 0 0 0                             \n"
        "M  V30 BEGIN ATOM                                     \n"
        "M  V30 1 C 3.06699 -3.575 0.0 0                       \n"
        "M  V30 2 C 3.93301 -3.075 0.0 0                       \n"
        "M  V30 3 C 4.79904 -3.575 0.0 0 CFG=1                 \n"
        "M  V30 4 C 5.66506 -3.075 0.0 0 CFG=2                 \n"
        "M  V30 5 C 6.53109 -3.575 0.0 0                       \n"
        "M  V30 6 C 7.39711 -3.075 0.0 0                       \n"
        "M  V30 7 C 4.79904 -4.575 0.0 0                       \n"
        "M  V30 8 C 5.66506 -2.075 0.0 0                       \n"
        "M  V30 9 C 3.06699 -8.475 0.0 0                       \n"
        "M  V30 10 C 3.93301 -7.975 0.0 0 CFG=2                \n"
        "M  V30 11 C 4.79904 -8.475 0.0 0 CFG=2                \n"
        "M  V30 12 C 5.66506 -7.975 0.0 0                      \n"
        "M  V30 13 C 6.53109 -8.475 0.0 0                      \n"
        "M  V30 14 C 3.93301 -6.975 0.0 0                      \n"
        "M  V30 15 C 4.79904 -9.475 0.0 0                      \n"
        "M  V30 16 C 7.39711 -7.975 0.0 0                      \n"
        "M  V30 17 C 8.26314 -8.475 0.0 0                      \n"
        "M  V30 18 C 9.12916 -7.975 0.0 0                      \n"
        "M  V30 END ATOM                                       \n"
        "M  V30 BEGIN BOND                                     \n"
        "M  V30 1 1 1 2                                        \n"
        "M  V30 2 1 2 3                                        \n"
        "M  V30 3 1 3 4                                        \n"
        "M  V30 4 1 4 5                                        \n"
        "M  V30 5 1 5 6                                        \n"
        "M  V30 6 1 3 7 CFG=1                                  \n"
        "M  V30 7 1 4 8 CFG=1                                  \n"
        "M  V30 8 1 9 10                                       \n"
        "M  V30 9 1 10 11                                      \n"
        "M  V30 10 1 11 12                                     \n"
        "M  V30 11 1 12 13                                     \n"
        "M  V30 12 1 10 14 CFG=1                               \n"
        "M  V30 13 1 11 15 CFG=3                               \n"
        "M  V30 14 1 13 16                                     \n"
        "M  V30 15 1 16 17                                     \n"
        "M  V30 16 1 17 18                                     \n"
        "M  V30 END BOND                                       \n"
        "M  V30 BEGIN COLLECTION                               \n"
        "M  V30 MDLV30/STEABS ATOMS=(2 3 11)                   \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(2 4 10)                  \n"
        "M  V30 END COLLECTION                                 \n"
        "M  V30 END CTAB                                       \n"
        "M  END                                                \n";


    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C10H22.C8H18/c1-5-6-7-8-10(4)9(2)3;1-5-7(3)8(4)6-2/h9-10H,5-8H2,1-4H3;7-8H,5-6H2,1-4H3/t10-;7-,8-/m00/s1(10)3(9);1(7)3(8)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_2_different_mols_inter_enhstereo_grps)
{
    const char *molblock =
        "test_mols                                                \n"
        "  -INDIGO-01272610042D                                   \n"
        "                                                         \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000                  \n"
        "M  V30 BEGIN CTAB                                        \n"
        "M  V30 COUNTS 37 35 0 0 0                                \n"
        "M  V30 BEGIN ATOM                                        \n"
        "M  V30 1 C 5.43304 -3.47499 0.0 0 CFG=1                  \n"
        "M  V30 2 C 4.56702 -2.97499 0.0 0 CFG=1                  \n"
        "M  V30 3 C 3.70099 -3.47499 0.0 0 CFG=2                  \n"
        "M  V30 4 C 2.83496 -2.97499 0.0 0 CFG=1                  \n"
        "M  V30 5 C 1.96894 -3.47499 0.0 0 CFG=2                  \n"
        "M  V30 6 C 1.10292 -2.97499 0.0 0                        \n"
        "M  V30 7 Br 1.96894 -4.47499 0.0 0                       \n"
        "M  V30 8 Cl 2.83496 -1.97499 0.0 0                       \n"
        "M  V30 9 Cl 3.70099 -4.47499 0.0 0                       \n"
        "M  V30 10 Cl 4.56702 -1.97499 0.0 0                      \n"
        "M  V30 11 Cl 5.43304 -4.47499 0.0 0                      \n"
        "M  V30 12 C 6.29907 -2.97499 0.0 0 CFG=1                 \n"
        "M  V30 13 C 7.16509 -3.47499 0.0 0 CFG=1                 \n"
        "M  V30 14 C 8.03112 -2.975 0.0 0 CFG=2                   \n"
        "M  V30 15 C 8.89714 -3.47499 0.0 0                       \n"
        "M  V30 16 Cl 8.03112 -1.975 0.0 0                        \n"
        "M  V30 17 Cl 7.16509 -4.47499 0.0 0                      \n"
        "M  V30 18 Cl 6.29907 -1.97499 0.0 0                      \n"
        "M  V30 19 C 5.73305 -7.82502 0.0 0 CFG=2                 \n"
        "M  V30 20 C 4.86701 -7.32498 0.0 0 CFG=1                 \n"
        "M  V30 21 C 4.00104 -7.82502 0.0 0 CFG=2                 \n"
        "M  V30 22 C 3.13499 -7.32498 0.0 0 CFG=1                 \n"
        "M  V30 23 C 2.26894 -7.82502 0.0 0 CFG=2                 \n"
        "M  V30 24 C 6.59902 -7.32498 0.0 0 CFG=2                 \n"
        "M  V30 25 C 7.46507 -7.82502 0.0 0 CFG=2                 \n"
        "M  V30 26 C 8.33112 -7.32498 0.0 0 CFG=2                 \n"
        "M  V30 27 C 1.40298 -7.32498 0.0 0                       \n"
        "M  V30 28 Br 2.26894 -8.82501 0.0 0                      \n"
        "M  V30 29 Cl 3.13499 -6.32499 0.0 0                      \n"
        "M  V30 30 Cl 4.00104 -8.82501 0.0 0                      \n"
        "M  V30 31 Cl 4.86701 -6.32499 0.0 0                      \n"
        "M  V30 32 Cl 5.73305 -8.82501 0.0 0                      \n"
        "M  V30 33 C 9.19708 -7.82502 0.0 0                       \n"
        "M  V30 34 Cl 8.33112 -6.32499 0.0 0                      \n"
        "M  V30 35 Cl 7.46507 -8.82501 0.0 0                      \n"
        "M  V30 36 Cl 6.59902 -6.32499 0.0 0                      \n"
        "M  V30 37 C 9.19702 -8.82502 0.0 0                       \n"
        "M  V30 END ATOM                                          \n"
        "M  V30 BEGIN BOND                                        \n"
        "M  V30 1 1 1 2                                           \n"
        "M  V30 2 1 1 11 CFG=3                                    \n"
        "M  V30 3 1 1 12                                          \n"
        "M  V30 4 1 2 3                                           \n"
        "M  V30 5 1 2 10 CFG=1                                    \n"
        "M  V30 6 1 3 4                                           \n"
        "M  V30 7 1 3 9 CFG=1                                     \n"
        "M  V30 8 1 4 5                                           \n"
        "M  V30 9 1 4 8 CFG=1                                     \n"
        "M  V30 10 1 5 6                                          \n"
        "M  V30 11 1 5 7 CFG=1                                    \n"
        "M  V30 12 1 12 13                                        \n"
        "M  V30 13 1 12 18 CFG=3                                  \n"
        "M  V30 14 1 13 14                                        \n"
        "M  V30 15 1 13 17 CFG=1                                  \n"
        "M  V30 16 1 14 15                                        \n"
        "M  V30 17 1 14 16 CFG=1                                  \n"
        "M  V30 18 1 19 20                                        \n"
        "M  V30 19 1 20 21                                        \n"
        "M  V30 20 1 21 22                                        \n"
        "M  V30 21 1 22 23                                        \n"
        "M  V30 22 1 19 24                                        \n"
        "M  V30 23 1 24 25                                        \n"
        "M  V30 24 1 25 26                                        \n"
        "M  V30 25 1 23 27                                        \n"
        "M  V30 26 1 23 28 CFG=1                                  \n"
        "M  V30 27 1 22 29 CFG=1                                  \n"
        "M  V30 28 1 21 30 CFG=1                                  \n"
        "M  V30 29 1 20 31 CFG=1                                  \n"
        "M  V30 30 1 19 32 CFG=3                                  \n"
        "M  V30 31 1 26 33                                        \n"
        "M  V30 32 1 26 34 CFG=1                                  \n"
        "M  V30 33 1 25 35 CFG=3                                  \n"
        "M  V30 34 1 24 36 CFG=1                                  \n"
        "M  V30 35 1 33 37                                        \n"
        "M  V30 END BOND                                          \n"
        "M  V30 BEGIN COLLECTION                                  \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(2 1 19)                     \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(4 2 3 20 21)                \n"
        "M  V30 MDLV30/STEABS ATOMS=(4 4 5 22 23)                 \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(4 12 13 24 25)              \n"
        "M  V30 MDLV30/STEREL2 ATOMS=(2 14 26)                    \n"
        "M  V30 END COLLECTION                                    \n"
        "M  V30 END CTAB                                          \n"
        "M  END                                                   \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/C11H16BrCl7.C10H14BrCl7/c1-3-5(13)7(15)9(17)11(19)10(18)8(16)6(14)4(2)12;1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h4-11H,3H2,1-2H3;3-10H,1-2H3/t4-,5-,6+,7-,8-,9-,10+,11-;3-,4-,5+,6-,7-,8-,9+,10-/m00/s1(4,6)2(5)(7,9)3(8,10)(11);1(3,5)2(4)(6,8)3(7,9)(10)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_differing_AND_groups_of_same_molecule_1)
{
    const char *molblock1 =
        "test mol                                     \n"
        "  -INDIGO-02192610442D                       \n"
        "                                             \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000      \n"
        "M  V30 BEGIN CTAB                            \n"
        "M  V30 COUNTS 7 6 0 0 0                      \n"
        "M  V30 BEGIN ATOM                            \n"
        "M  V30 1 O 1.54199 -5.725 0.0 0              \n"
        "M  V30 2 C 2.40801 -5.225 0.0 0              \n"
        "M  V30 3 C 3.27404 -5.725 0.0 0 CFG=1        \n"
        "M  V30 4 C 4.14006 -5.225 0.0 0 CFG=2        \n"
        "M  V30 5 C 5.00609 -5.725 0.0 0              \n"
        "M  V30 6 Br 4.14006 -4.225 0.0 0             \n"
        "M  V30 7 Cl 3.27404 -6.725 0.0 0             \n"
        "M  V30 END ATOM                              \n"
        "M  V30 BEGIN BOND                            \n"
        "M  V30 1 1 1 2                               \n"
        "M  V30 2 1 2 3                               \n"
        "M  V30 3 1 3 4                               \n"
        "M  V30 4 1 4 5                               \n"
        "M  V30 5 1 4 6 CFG=1                         \n"
        "M  V30 6 1 3 7 CFG=1                         \n"
        "M  V30 END BOND                              \n"
        "M  V30 BEGIN COLLECTION                      \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(1 3)            \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 4)            \n"
        "M  V30 END COLLECTION                        \n"
        "M  V30 END CTAB                              \n"
        "M  END                                       \n";

    const char *molblock2 =
        "test mol                                 \n"
        "  -INDIGO-02192610462D                   \n"
        "                                         \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000  \n"
        "M  V30 BEGIN CTAB                        \n"
        "M  V30 COUNTS 7 6 0 0 0                  \n"
        "M  V30 BEGIN ATOM                        \n"
        "M  V30 1 O 1.54199 -5.725 0.0 0          \n"
        "M  V30 2 C 2.40801 -5.225 0.0 0          \n"
        "M  V30 3 C 3.27404 -5.725 0.0 0 CFG=1    \n"
        "M  V30 4 C 4.14006 -5.225 0.0 0 CFG=2    \n"
        "M  V30 5 C 5.00609 -5.725 0.0 0          \n"
        "M  V30 6 Br 4.14006 -4.225 0.0 0         \n"
        "M  V30 7 Cl 3.27404 -6.725 0.0 0         \n"
        "M  V30 END ATOM                          \n"
        "M  V30 BEGIN BOND                        \n"
        "M  V30 1 1 1 2                           \n"
        "M  V30 2 1 2 3                           \n"
        "M  V30 3 1 3 4                           \n"
        "M  V30 4 1 4 5                           \n"
        "M  V30 5 1 4 6 CFG=1                     \n"
        "M  V30 6 1 3 7 CFG=1                     \n"
        "M  V30 END BOND                          \n"
        "M  V30 BEGIN COLLECTION                  \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 3)        \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(1 4)        \n"
        "M  V30 END COLLECTION                    \n"
        "M  V30 END CTAB                          \n"
        "M  END                                   \n";

    char options[] = "-EnhancedStereochemistry";
    const char expected_inchi[] = "InChI=1B/C4H8BrClO/c1-3(5)4(6)2-7/h3-4,7H,2H2,1H3/t3-,4-/s3(3)(4)";

    inchi_Output output1;
    inchi_Output *poutput1 = &output1;
    EXPECT_EQ(MakeINCHIFromMolfileText(molblock1, options, poutput1), 0);
    EXPECT_STREQ(poutput1->szInChI, expected_inchi);

    inchi_Output output2;
    inchi_Output *poutput2 = &output2;
    EXPECT_EQ(MakeINCHIFromMolfileText(molblock2, options, poutput2), 0);
    EXPECT_STREQ(poutput2->szInChI, expected_inchi);

    poutput1->szLog = nullptr;
    poutput1->szMessage = nullptr;
    FreeINCHI(poutput1);

    poutput2->szLog = nullptr;
    poutput2->szMessage = nullptr;
    FreeINCHI(poutput2);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_differing_AND_groups_of_same_molecule_2)
{
    const char *molblock =
        "test_mol                                  \n"
        "  -INDIGO-02192611042D                    \n"
        "                                          \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000   \n"
        "M  V30 BEGIN CTAB                         \n"
        "M  V30 COUNTS 14 12 0 0 0                 \n"
        "M  V30 BEGIN ATOM                         \n"
        "M  V30 1 O 4.21699 -6.575 0.0 0           \n"
        "M  V30 2 C 5.08301 -6.075 0.0 0           \n"
        "M  V30 3 C 5.94904 -6.575 0.0 0 CFG=1     \n"
        "M  V30 4 C 6.81506 -6.075 0.0 0 CFG=2     \n"
        "M  V30 5 C 7.68109 -6.575 0.0 0           \n"
        "M  V30 6 Br 6.81506 -5.075 0.0 0          \n"
        "M  V30 7 Cl 5.94904 -7.575 0.0 0          \n"
        "M  V30 8 O 11.2179 -6.4 0.0 0             \n"
        "M  V30 9 C 12.084 -5.9 0.0 0              \n"
        "M  V30 10 C 12.95 -6.4 0.0 0 CFG=1        \n"
        "M  V30 11 C 13.816 -5.9 0.0 0 CFG=2       \n"
        "M  V30 12 C 14.6821 -6.4 0.0 0            \n"
        "M  V30 13 Br 13.816 -4.9 0.0 0            \n"
        "M  V30 14 Cl 12.95 -7.4 0.0 0             \n"
        "M  V30 END ATOM                           \n"
        "M  V30 BEGIN BOND                         \n"
        "M  V30 1 1 1 2                            \n"
        "M  V30 2 1 2 3                            \n"
        "M  V30 3 1 3 4                            \n"
        "M  V30 4 1 4 5                            \n"
        "M  V30 5 1 4 6 CFG=1                      \n"
        "M  V30 6 1 3 7 CFG=1                      \n"
        "M  V30 7 1 8 9                            \n"
        "M  V30 8 1 9 10                           \n"
        "M  V30 9 1 10 11                          \n"
        "M  V30 10 1 11 12                         \n"
        "M  V30 11 1 11 13 CFG=1                   \n"
        "M  V30 12 1 10 14 CFG=1                   \n"
        "M  V30 END BOND                           \n"
        "M  V30 BEGIN COLLECTION                   \n"
        "M  V30 MDLV30/STERAC2 ATOMS=(1 3)         \n"
        "M  V30 MDLV30/STERAC1 ATOMS=(1 4)         \n"
        "M  V30 MDLV30/STERAC3 ATOMS=(1 10)        \n"
        "M  V30 MDLV30/STERAC4 ATOMS=(1 11)        \n"
        "M  V30 END COLLECTION                     \n"
        "M  V30 END CTAB                           \n"
        "M  END                                    \n";

    char options[] = "-EnhancedStereochemistry";
    inchi_Output output;
    inchi_Output *poutput = &output;
    const char expected_inchi[] = "InChI=1B/2C4H8BrClO/c2*1-3(5)4(6)2-7/h2*3-4,7H,2H2,1H3/t2*3-,4-/s2*3(3)(4)";

    EXPECT_EQ(MakeINCHIFromMolfileText(molblock, options, poutput), 0);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;

    FreeINCHI(poutput);
}

TEST(test_enhancedStereo, test_EnhancedStereochemistry_test_file_1)
{

    const char* inchi_filename = "../../../../../INCHI-1-TEST/tests/test_unit/fixtures/enh_stereo_test_file_1.sdf";
    // const char* inchi_filename = "/workspaces/InChI/INCHI-1-TEST/tests/test_unit/fixtures/enh_stereo_test_file_1.sdf";

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
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/m1/s1",
        "InChI=1B/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3/t4-/s2",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s2",
        "InChI=1B/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3/t4-/m1/s1",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s2(4)(5)",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s3",
        "InChI=1B/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3/t4-/s3",
        "InChI=1B/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s3(4)(5)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5+,6-/m0/s1(4,5)2(6)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5+,6?/m0/s1",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5-,6+/m0/s1(4)2(5,6)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5?,6?/m0/s1",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5-,6-/m0/s1(4)2(5,6)",
        "InChI=1B/C7H17NO/c1-4-5(2)7(8)6(3)9/h5-7,9H,4,8H2,1-3H3/t5-,6-,7+/m1/s1(6)2(5,7)",
        "InChI=1B/C7H17NO/c1-4-5(2)7(8)6(3)9/h5-7,9H,4,8H2,1-3H3/t5?,6-,7?/m1/s1",
        "InChI=1B/C7H17NO/c1-4-5(2)7(8)6(3)9/h5-7,9H,4,8H2,1-3H3/t5-,6-,7-/m1/s1(6)2(5,7)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5-,6-/m0/s1(4)2(5)(6)",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/m1/s1(4)3(5)",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5?/s3",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5-,6-/m0/s1(4)3(5,6)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5-,6+/m0/s1(4)3(5,6)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5?,6?/s3",
        "InChI=1B/C7H17NO/c1-4-5(2)7(8)6(3)9/h5-7,9H,4,8H2,1-3H3/t5?,6-,7?/s3",
        "InChI=1B/C7H17NO/c1-4-5(2)7(8)6(3)9/h5-7,9H,4,8H2,1-3H3/t5-,6-,7-/m1/s1(6)3(5,7)",
        "InChI=1B/C7H17NO/c1-4-5(2)7(8)6(3)9/h5-7,9H,4,8H2,1-3H3/t5-,6-,7+/m1/s1(6)3(5,7)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5-,6-/m0/s1(4)3(5)(6)",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2/t4-,5-,6-/m0/s1(4)2(5)3(6)",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5?/m1/s1",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5?/s3",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5?/s2",
        "InChI=1B/C6H12O3/c7-4-1-2-5(8)6(9)3-4/h4-9H,1-3H2",
    };

    int nof_inchis = 33;

    EXPECT_EQ(nof_inchis, molblocks.size());
    EXPECT_EQ(nof_inchis, list_expected_inchis.size());

    char options[] = "-EnhancedStereochemistry";

    for (int i = 0; i < nof_inchis; ++i) {

        inchi_Output output;
        inchi_Output* poutput = &output;

        poutput->szLog = nullptr;
        poutput->szMessage = nullptr;
        poutput->szInChI = nullptr;

        int ret = MakeINCHIFromMolfileText(molblocks[i].c_str(), options, poutput);

        EXPECT_LT(ret, 2);

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

/* Regression: V2000 input has no V3000 collection block, so orig_inp_data->v3000
   is NULL. With -EnhancedStereochemistry the /s layer must still be emitted from
   the plain stereo type instead of dereferencing that NULL pointer. */
TEST(test_enhancedStereo, test_EnhancedStereochemistry_v2000_with_stereocentre)
{
    const char *molblock =
        "L-alanine_v2000                                                      \n"
        "  test              2D                                               \n"
        "                                                                     \n"
        "  6  5  0  0  1  0  0  0  0  0999 V2000                              \n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.8660    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "   -0.8660    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.0000   -1.0000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    1.7320    0.0000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    0.8660    1.5000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "  1  2  1  0  0  0  0                                                \n"
        "  1  3  1  0  0  0  0                                                \n"
        "  1  4  1  1  0  0  0                                                \n"
        "  2  5  1  0  0  0  0                                                \n"
        "  2  6  2  0  0  0  0                                                \n"
        "M  END                                                               \n";

    char options[] = "-EnhancedStereochemistry";
    const char expected_inchi[] = "InChI=1B/C3H7NO2/c1-2(4)3(5)6/h2H,4H2,1H3,(H,5,6)/t2-/m0/s1";

    inchi_Output output;
    inchi_Output *poutput = &output;
    poutput->szLog = nullptr;
    poutput->szMessage = nullptr;
    poutput->szInChI = nullptr;

    EXPECT_LT(MakeINCHIFromMolfileText(molblock, options, poutput), 2);
    EXPECT_STREQ(poutput->szInChI, expected_inchi);

    FreeINCHI(poutput);
}

/* Regression: V3000 atom indexes need not be 1..n. The collection atom numbers are
   remapped to the final atom order, and every atom of a collection must be remapped
   - not all but the last two - otherwise centres silently drop out of /s and /t. */
TEST(test_enhancedStereo, test_EnhancedStereochemistry_collections_independent_of_atom_numbering)
{
    const char *molblock_1_to_n =
        "enh_stereo_or_groups_atoms_1_to_n        \n"
        "  ACD/LABS08242216132D                   \n"
        "                                         \n"
        "  0  0  0  0  0  0  0  0  0  0999 V3000  \n"
        "M  V30 BEGIN CTAB                        \n"
        "M  V30 COUNTS 18 17 0 0 1                \n"
        "M  V30 BEGIN ATOM                        \n"
        "M  V30 1 C 3424.1946 -1936.7935 0 0      \n"
        "M  V30 2 C 3352.3145 -1895.2935 0 0      \n"
        "M  V30 3 C 3280.4346 -1936.7935 0 0      \n"
        "M  V30 4 C 3208.5542 -1895.2935 0 0      \n"
        "M  V30 5 C 3136.6743 -1936.7935 0 0      \n"
        "M  V30 6 C 3064.7944 -1895.2935 0 0      \n"
        "M  V30 7 Br 3136.6743 -2019.7935 0 0     \n"
        "M  V30 8 Cl 3208.5542 -1812.2935 0 0     \n"
        "M  V30 9 Cl 3280.4346 -2019.7935 0 0     \n"
        "M  V30 10 Cl 3352.3145 -1812.2935 0 0    \n"
        "M  V30 11 Cl 3424.1946 -2019.7935 0 0    \n"
        "M  V30 12 C 3496.075 -1895.2935 0 0      \n"
        "M  V30 13 C 3567.9548 -1936.7942 0 0     \n"
        "M  V30 14 C 3639.835 -1895.2944 0 0      \n"
        "M  V30 15 C 3711.7148 -1936.7942 0 0     \n"
        "M  V30 16 Cl 3639.835 -1812.2944 0 0     \n"
        "M  V30 17 Cl 3567.9548 -2019.7942 0 0    \n"
        "M  V30 18 Cl 3496.075 -1812.2937 0 0     \n"
        "M  V30 END ATOM                          \n"
        "M  V30 BEGIN BOND                        \n"
        "M  V30 1 1 1 2                           \n"
        "M  V30 2 1 1 11 CFG=3                    \n"
        "M  V30 3 1 1 12                          \n"
        "M  V30 4 1 2 3                           \n"
        "M  V30 5 1 2 10 CFG=1                    \n"
        "M  V30 6 1 3 4                           \n"
        "M  V30 7 1 3 9 CFG=1                     \n"
        "M  V30 8 1 4 5                           \n"
        "M  V30 9 1 4 8 CFG=1                     \n"
        "M  V30 10 1 5 6                          \n"
        "M  V30 11 1 5 7 CFG=1                    \n"
        "M  V30 12 1 12 13                        \n"
        "M  V30 13 1 12 18 CFG=3                  \n"
        "M  V30 14 1 13 14                        \n"
        "M  V30 15 1 13 17 CFG=1                  \n"
        "M  V30 16 1 14 15                        \n"
        "M  V30 17 1 14 16 CFG=1                  \n"
        "M  V30 END BOND                          \n"
        "M  V30 BEGIN COLLECTION                  \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(5 1 2 3 4 5)\n"
        "M  V30 MDLV30/STEREL2 ATOMS=(3 12 13 14) \n"
        "M  V30 END COLLECTION                    \n"
        "M  V30 END CTAB                          \n"
        "M  END                                   \n";

    const char *molblock_offset =
        "enh_stereo_or_groups_atoms_offset_by_100           \n"
        "  ACD/LABS08242216132D                             \n"
        "                                                   \n"
        "  0  0  0  0  0  0  0  0  0  0999 V3000            \n"
        "M  V30 BEGIN CTAB                                  \n"
        "M  V30 COUNTS 18 17 0 0 1                          \n"
        "M  V30 BEGIN ATOM                                  \n"
        "M  V30 101 C 3424.1946 -1936.7935 0 0              \n"
        "M  V30 102 C 3352.3145 -1895.2935 0 0              \n"
        "M  V30 103 C 3280.4346 -1936.7935 0 0              \n"
        "M  V30 104 C 3208.5542 -1895.2935 0 0              \n"
        "M  V30 105 C 3136.6743 -1936.7935 0 0              \n"
        "M  V30 106 C 3064.7944 -1895.2935 0 0              \n"
        "M  V30 107 Br 3136.6743 -2019.7935 0 0             \n"
        "M  V30 108 Cl 3208.5542 -1812.2935 0 0             \n"
        "M  V30 109 Cl 3280.4346 -2019.7935 0 0             \n"
        "M  V30 110 Cl 3352.3145 -1812.2935 0 0             \n"
        "M  V30 111 Cl 3424.1946 -2019.7935 0 0             \n"
        "M  V30 112 C 3496.075 -1895.2935 0 0               \n"
        "M  V30 113 C 3567.9548 -1936.7942 0 0              \n"
        "M  V30 114 C 3639.835 -1895.2944 0 0               \n"
        "M  V30 115 C 3711.7148 -1936.7942 0 0              \n"
        "M  V30 116 Cl 3639.835 -1812.2944 0 0              \n"
        "M  V30 117 Cl 3567.9548 -2019.7942 0 0             \n"
        "M  V30 118 Cl 3496.075 -1812.2937 0 0              \n"
        "M  V30 END ATOM                                    \n"
        "M  V30 BEGIN BOND                                  \n"
        "M  V30 1 1 101 102                                 \n"
        "M  V30 2 1 101 111 CFG=3                           \n"
        "M  V30 3 1 101 112                                 \n"
        "M  V30 4 1 102 103                                 \n"
        "M  V30 5 1 102 110 CFG=1                           \n"
        "M  V30 6 1 103 104                                 \n"
        "M  V30 7 1 103 109 CFG=1                           \n"
        "M  V30 8 1 104 105                                 \n"
        "M  V30 9 1 104 108 CFG=1                           \n"
        "M  V30 10 1 105 106                                \n"
        "M  V30 11 1 105 107 CFG=1                          \n"
        "M  V30 12 1 112 113                                \n"
        "M  V30 13 1 112 118 CFG=3                          \n"
        "M  V30 14 1 113 114                                \n"
        "M  V30 15 1 113 117 CFG=1                          \n"
        "M  V30 16 1 114 115                                \n"
        "M  V30 17 1 114 116 CFG=1                          \n"
        "M  V30 END BOND                                    \n"
        "M  V30 BEGIN COLLECTION                            \n"
        "M  V30 MDLV30/STEREL1 ATOMS=(5 101 102 103 104 105)\n"
        "M  V30 MDLV30/STEREL2 ATOMS=(3 112 113 114)        \n"
        "M  V30 END COLLECTION                              \n"
        "M  V30 END CTAB                                    \n"
        "M  END                                             \n";

    char options[] = "-EnhancedStereochemistry";
    const char expected_inchi[] = "InChI=1B/C10H14BrCl7/c1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h3-10H,1-2H3/t3-,4-,5+,6+,7-,8+,9+,10-/s2(3,5,7,9,10)(4,6,8)";

    const char *molblocks[2] = { molblock_1_to_n, molblock_offset };

    for (int i = 0; i < 2; ++i)
    {
        inchi_Output output;
        inchi_Output *poutput = &output;
        poutput->szLog = nullptr;
        poutput->szMessage = nullptr;
        poutput->szInChI = nullptr;

        EXPECT_LT(MakeINCHIFromMolfileText(molblocks[i], options, poutput), 2);
        EXPECT_STREQ(poutput->szInChI, expected_inchi) << "molblock " << i;

        FreeINCHI(poutput);
    }
}
