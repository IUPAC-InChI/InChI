#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inpdef.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ring_detection.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/strutil.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
}

TEST(test_ring_detection, find_rings__no_ring) {

    int num_at = 3;

    inp_ATOM *atoms = CreateInpAtom(num_at);

    // Atom 0 connected to 1
    atoms[0].neighbor[0] = 1;
    atoms[0].valence = 1;

    // Atom 1 connected to 0
    atoms[1].neighbor[0] = 0;
    atoms[1].neighbor[1] = 2;
    atoms[1].valence = 2;

    // Atom 1 connected to 0
    atoms[2].neighbor[0] = 1;
    atoms[2].valence = 1;

    RingSystems* rs = find_rings(atoms, num_at);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 0);

    free_ring_system(rs);

    FreeInpAtom(&atoms);
}

TEST(test_ring_detection, find_rings__single_triangle_ring) {

    int num_at = 3;

    inp_ATOM *atoms = CreateInpAtom(num_at);

    // Atom 0 connected to 1 and 2
    atoms[0].neighbor[0] = 1;
    atoms[0].neighbor[1] = 2;
    atoms[0].valence = 2;

    // Atom 1 connected to 0 and 2
    atoms[1].neighbor[0] = 0;
    atoms[1].neighbor[1] = 2;
    atoms[1].valence = 2;

    // Atom 2 connected to 0 and 1
    atoms[2].neighbor[0] = 0;
    atoms[2].neighbor[1] = 1;
    atoms[2].valence = 2;

    RingSystems* rs = find_rings(atoms, num_at);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 1);

    const Ring& ring = rs->rings[0];
    EXPECT_EQ(ring.size, 3);

    bool has0 = false, has1 = false, has2 = false;
    for (int i = 0; i < ring.size; ++i) {
        if (ring.atom_ids[i] == 0) has0 = true;
        if (ring.atom_ids[i] == 1) has1 = true;
        if (ring.atom_ids[i] == 2) has2 = true;
    }
    EXPECT_TRUE(has0 && has1 && has2);

    free_ring_system(rs);

    FreeInpAtom(&atoms);
}

TEST(test_ring_detection, find_rings__benzene_ring) {

    int num_at = 6;

    inp_ATOM *atoms = CreateInpAtom(num_at);

    // Atom 0 connected to 1 and 5
    atoms[0].neighbor[0] = 5;
    atoms[0].neighbor[1] = 1;
    atoms[0].valence = 2;

    // Atom 1 connected to 0 and 2
    atoms[1].neighbor[0] = 0;
    atoms[1].neighbor[1] = 2;
    atoms[1].valence = 2;

    // Atom 2 connected to 1 and 3
    atoms[2].neighbor[0] = 1;
    atoms[2].neighbor[1] = 3;
    atoms[2].valence = 2;

    // Atom 3 connected to 2 and 4
    atoms[3].neighbor[0] = 2;
    atoms[3].neighbor[1] = 4;
    atoms[3].valence = 2;

    // Atom 4 connected to 3 and 5
    atoms[4].neighbor[0] = 3;
    atoms[4].neighbor[1] = 5;
    atoms[4].valence = 2;

    // Atom 5 connected to 4 and 0
    atoms[5].neighbor[0] = 4;
    atoms[5].neighbor[1] = 0;
    atoms[5].valence = 2;

    RingSystems* rs = find_rings(atoms, num_at);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 1);

    const Ring& ring = rs->rings[0];
    EXPECT_EQ(ring.size, 6);

    free_ring_system(rs);

    FreeInpAtom(&atoms);
}

TEST(test_ring_detection, find_rings__molblock_ring_1_no_rings) {


    INCHI_IOSTREAM input_stream;

    const char *molblock =
        "enhanc_stereo1									  \n"
        "  ACD/LABS08242216132D                            \n"
        "												  \n"
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

    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    ORIG_ATOM_DATA *orig_inp_data = (ORIG_ATOM_DATA*)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));
    int bMergeAllInputStructures = 0;
    int bGetOrigCoord = 0;
    int bDoNotAddH = 0;
    int treat_polymers = 0;
    int treat_NPZz = 0;
    const char *pSdfLabel = nullptr;
    char *pSdfValue = nullptr;
    unsigned long *lSdfId = nullptr;
    long *lMolfileNumber = nullptr;
    INCHI_MODE pInpAtomFlags = 0;
    int err = 0;
    char *pStrErr = nullptr;
    int bNoWarnings = 0;

    int ret = CreateOrigInpDataFromMolfile(
        &input_stream,
        orig_inp_data,
        bMergeAllInputStructures,
        bGetOrigCoord,
        bDoNotAddH,
        treat_polymers,
        treat_NPZz,
        pSdfLabel,
        pSdfValue,
        lSdfId,
        lMolfileNumber,
        &pInpAtomFlags,
        &err,
        pStrErr,
        bNoWarnings);


    RingSystems *rs = find_rings(orig_inp_data->at, orig_inp_data->num_inp_atoms);

    // print_ring_result(rs);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 0);

    FreeOrigAtData(orig_inp_data);
    inchi_free(orig_inp_data);

    free_ring_system(rs);

    inchi_ios_free_str(&input_stream);
}

TEST(test_ring_detection, find_rings__molblock_ring_2_two_rings) {


    INCHI_IOSTREAM input_stream;

    const char *molblock =
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

    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    ORIG_ATOM_DATA *orig_inp_data = (ORIG_ATOM_DATA*)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));
    int bMergeAllInputStructures = 0;
    int bGetOrigCoord = 0;
    int bDoNotAddH = 0;
    int treat_polymers = 0;
    int treat_NPZz = 0;
    const char *pSdfLabel = nullptr;
    char *pSdfValue = nullptr;
    unsigned long *lSdfId = nullptr;
    long *lMolfileNumber = nullptr;
    INCHI_MODE pInpAtomFlags = 0;
    int err = 0;
    char *pStrErr = nullptr;
    int bNoWarnings = 0;

    int ret = CreateOrigInpDataFromMolfile(
        &input_stream,
        orig_inp_data,
        bMergeAllInputStructures,
        bGetOrigCoord,
        bDoNotAddH,
        treat_polymers,
        treat_NPZz,
        pSdfLabel,
        pSdfValue,
        lSdfId,
        lMolfileNumber,
        &pInpAtomFlags,
        &err,
        pStrErr,
        bNoWarnings);


    RingSystems *rs = find_rings(orig_inp_data->at, orig_inp_data->num_inp_atoms);

    // print_ring_result(rs);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 2);

    EXPECT_EQ(rs->rings[0].size, 6);
    EXPECT_EQ(rs->rings[1].size, 6);

    FreeOrigAtData(orig_inp_data);
    inchi_free(orig_inp_data);

    free_ring_system(rs);

    inchi_ios_free_str(&input_stream);
}

TEST(test_ring_detection, find_rings__molblock_ring_3_two_fused_rings) {

    INCHI_IOSTREAM input_stream;

    const char *molblock =
        "test mol fused benzene                   \n"
        "  -INDIGO-03302612192D                   \n"
        "                                         \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000  \n"
        "M  V30 BEGIN CTAB                        \n"
        "M  V30 COUNTS 10 11 0 0 0                \n"
        "M  V30 BEGIN ATOM                        \n"
        "M  V30 1 C 4.83485 -5.40007 0.0 0        \n"
        "M  V30 2 C 6.56515 -5.39959 0.0 0        \n"
        "M  V30 3 C 5.70164 -4.89997 0.0 0        \n"
        "M  V30 4 C 6.56515 -6.40053 0.0 0        \n"
        "M  V30 5 C 4.83485 -6.40502 0.0 0        \n"
        "M  V30 6 C 5.70382 -6.90003 0.0 0        \n"
        "M  V30 7 C 7.42965 -4.90121 0.0 0        \n"
        "M  V30 8 C 8.29683 -5.40155 0.0 0        \n"
        "M  V30 9 C 7.4358 -6.90287 0.0 0         \n"
        "M  V30 10 C 8.29903 -6.39746 0.0 0       \n"
        "M  V30 END ATOM                          \n"
        "M  V30 BEGIN BOND                        \n"
        "M  V30 1 2 3 1                           \n"
        "M  V30 2 1 1 5                           \n"
        "M  V30 3 2 5 6                           \n"
        "M  V30 4 1 6 4                           \n"
        "M  V30 5 2 4 2                           \n"
        "M  V30 6 1 2 3                           \n"
        "M  V30 7 1 4 9                           \n"
        "M  V30 8 2 9 10                          \n"
        "M  V30 9 1 10 8                          \n"
        "M  V30 10 2 8 7                          \n"
        "M  V30 11 1 7 2                          \n"
        "M  V30 END BOND                          \n"
        "M  V30 END CTAB                          \n"
        "M  END                                   \n";


    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    ORIG_ATOM_DATA *orig_inp_data = (ORIG_ATOM_DATA*)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));
    int bMergeAllInputStructures = 0;
    int bGetOrigCoord = 0;
    int bDoNotAddH = 0;
    int treat_polymers = 0;
    int treat_NPZz = 0;
    const char *pSdfLabel = nullptr;
    char *pSdfValue = nullptr;
    unsigned long *lSdfId = nullptr;
    long *lMolfileNumber = nullptr;
    INCHI_MODE pInpAtomFlags = 0;
    int err = 0;
    char *pStrErr = nullptr;
    int bNoWarnings = 0;

    int ret = CreateOrigInpDataFromMolfile(
        &input_stream,
        orig_inp_data,
        bMergeAllInputStructures,
        bGetOrigCoord,
        bDoNotAddH,
        treat_polymers,
        treat_NPZz,
        pSdfLabel,
        pSdfValue,
        lSdfId,
        lMolfileNumber,
        &pInpAtomFlags,
        &err,
        pStrErr,
        bNoWarnings);


    RingSystems *rs = find_rings(orig_inp_data->at, orig_inp_data->num_inp_atoms);

    // print_ring_result(rs);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 3);

    EXPECT_EQ(rs->rings[0].size, 6);
    EXPECT_EQ(rs->rings[1].size, 10);
    EXPECT_EQ(rs->rings[2].size, 6);

    FreeOrigAtData(orig_inp_data);
    inchi_free(orig_inp_data);

    free_ring_system(rs);

    inchi_ios_free_str(&input_stream);
}

TEST(test_ring_detection, find_rings__molblock_ring_4_three_fused_rings) {

    INCHI_IOSTREAM input_stream;

    const char *molblock =
        "test mol                                 \n"
        "  -INDIGO-03302612232D                   \n"
        "                                         \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000  \n"
        "M  V30 BEGIN CTAB                        \n"
        "M  V30 COUNTS 13 15 0 0 0                \n"
        "M  V30 BEGIN ATOM                        \n"
        "M  V30 1 C 4.83485 -5.40007 0.0 0        \n"
        "M  V30 2 C 6.56515 -5.39959 0.0 0        \n"
        "M  V30 3 C 5.70164 -4.89997 0.0 0        \n"
        "M  V30 4 C 6.56515 -6.40053 0.0 0        \n"
        "M  V30 5 C 4.83485 -6.40502 0.0 0        \n"
        "M  V30 6 C 5.70382 -6.90003 0.0 0        \n"
        "M  V30 7 C 7.42965 -4.90121 0.0 0        \n"
        "M  V30 8 C 8.29683 -5.40155 0.0 0        \n"
        "M  V30 9 C 7.4358 -6.90287 0.0 0         \n"
        "M  V30 10 C 8.29903 -6.39746 0.0 0       \n"
        "M  V30 11 C 9.82314 -5.90132 0.0 0       \n"
        "M  V30 12 C 9.246 -5.10027 0.0 0         \n"
        "M  V30 13 C 9.23791 -6.703 0.0 0         \n"
        "M  V30 END ATOM                          \n"
        "M  V30 BEGIN BOND                        \n"
        "M  V30 1 2 3 1                           \n"
        "M  V30 2 1 1 5                           \n"
        "M  V30 3 2 5 6                           \n"
        "M  V30 4 1 6 4                           \n"
        "M  V30 5 2 4 2                           \n"
        "M  V30 6 1 2 3                           \n"
        "M  V30 7 1 4 9                           \n"
        "M  V30 8 2 9 10                          \n"
        "M  V30 9 1 10 8                          \n"
        "M  V30 10 2 8 7                          \n"
        "M  V30 11 1 7 2                          \n"
        "M  V30 12 1 10 13                        \n"
        "M  V30 13 1 13 11                        \n"
        "M  V30 14 2 11 12                        \n"
        "M  V30 15 1 12 8                         \n"
        "M  V30 END BOND                          \n"
        "M  V30 END CTAB                          \n"
        "M  END                                   \n";

    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    ORIG_ATOM_DATA *orig_inp_data = (ORIG_ATOM_DATA*)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));
    int bMergeAllInputStructures = 0;
    int bGetOrigCoord = 0;
    int bDoNotAddH = 0;
    int treat_polymers = 0;
    int treat_NPZz = 0;
    const char *pSdfLabel = nullptr;
    char *pSdfValue = nullptr;
    unsigned long *lSdfId = nullptr;
    long *lMolfileNumber = nullptr;
    INCHI_MODE pInpAtomFlags = 0;
    int err = 0;
    char *pStrErr = nullptr;
    int bNoWarnings = 0;

    int ret = CreateOrigInpDataFromMolfile(
        &input_stream,
        orig_inp_data,
        bMergeAllInputStructures,
        bGetOrigCoord,
        bDoNotAddH,
        treat_polymers,
        treat_NPZz,
        pSdfLabel,
        pSdfValue,
        lSdfId,
        lMolfileNumber,
        &pInpAtomFlags,
        &err,
        pStrErr,
        bNoWarnings);


    RingSystems *rs = find_rings(orig_inp_data->at, orig_inp_data->num_inp_atoms);

    // print_ring_result(rs);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 6);

    EXPECT_EQ(rs->rings[0].size, 6);
    EXPECT_EQ(rs->rings[1].size, 10);
    EXPECT_EQ(rs->rings[2].size, 13);
    EXPECT_EQ(rs->rings[3].size, 6);
    EXPECT_EQ(rs->rings[4].size, 9);
    EXPECT_EQ(rs->rings[5].size, 5);

    FreeOrigAtData(orig_inp_data);
    inchi_free(orig_inp_data);

    free_ring_system(rs);

    inchi_ios_free_str(&input_stream);
}

TEST(test_ring_detection, find_rings__molblock_ring_4_two_connected_rings) {

    INCHI_IOSTREAM input_stream;

    const char *molblock =
        "2 connected rings                                                         \n"
        "  Ketcher  3302612292D 1   1.00000     0.00000     0                      \n"
        "                                                                          \n"
        " 11 12  0  0  0  0  0  0  0  0999 V2000                                   \n"
        "    7.7000   -4.5965    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    9.2000   -5.4625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    8.7000   -4.5965    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    8.7000   -6.3285    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    7.2000   -5.4625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    7.7000   -6.3285    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   10.7000   -4.5965    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    9.7000   -4.5965    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   11.2000   -5.4625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "    9.7000   -6.3285    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "   10.7000   -6.3285    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0     \n"
        "  3  1  2  0     0  0                                                     \n"
        "  1  5  1  0     0  0                                                     \n"
        "  5  6  2  0     0  0                                                     \n"
        "  6  4  1  0     0  0                                                     \n"
        "  4  2  2  0     0  0                                                     \n"
        "  2  3  1  0     0  0                                                     \n"
        "  8  2  2  0     0  0                                                     \n"
        "  2 10  1  0     0  0                                                     \n"
        " 10 11  2  0     0  0                                                     \n"
        " 11  9  1  0     0  0                                                     \n"
        "  9  7  2  0     0  0                                                     \n"
        "  7  8  1  0     0  0                                                     \n"
        "M  END                                                                    \n";

    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    ORIG_ATOM_DATA *orig_inp_data = (ORIG_ATOM_DATA*)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));
    int bMergeAllInputStructures = 0;
    int bGetOrigCoord = 0;
    int bDoNotAddH = 0;
    int treat_polymers = 0;
    int treat_NPZz = 0;
    const char *pSdfLabel = nullptr;
    char *pSdfValue = nullptr;
    unsigned long *lSdfId = nullptr;
    long *lMolfileNumber = nullptr;
    INCHI_MODE pInpAtomFlags = 0;
    int err = 0;
    char *pStrErr = nullptr;
    int bNoWarnings = 0;

    int ret = CreateOrigInpDataFromMolfile(
        &input_stream,
        orig_inp_data,
        bMergeAllInputStructures,
        bGetOrigCoord,
        bDoNotAddH,
        treat_polymers,
        treat_NPZz,
        pSdfLabel,
        pSdfValue,
        lSdfId,
        lMolfileNumber,
        &pInpAtomFlags,
        &err,
        pStrErr,
        bNoWarnings);


    RingSystems *rs = find_rings(orig_inp_data->at, orig_inp_data->num_inp_atoms);

    // print_ring_result(rs);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 2);

    EXPECT_EQ(rs->rings[0].size, 6);
    EXPECT_EQ(rs->rings[1].size, 6);

    FreeOrigAtData(orig_inp_data);
    inchi_free(orig_inp_data);

    free_ring_system(rs);

    inchi_ios_free_str(&input_stream);
}

TEST(test_ring_detection, are_atoms_in_same_small_ring__one_ring) {

    int num_at = 7;

    inp_ATOM *atoms = CreateInpAtom(num_at);

    // Atom 0 connected to 1 and 5
    atoms[0].neighbor[0] = 5;
    atoms[0].neighbor[1] = 1;
    atoms[0].valence = 2;

    // Atom 1 connected to 0 and 2
    atoms[1].neighbor[0] = 0;
    atoms[1].neighbor[1] = 2;
    atoms[1].valence = 2;

    // Atom 2 connected to 1 and 3
    atoms[2].neighbor[0] = 1;
    atoms[2].neighbor[1] = 3;
    atoms[2].valence = 2;

    // Atom 3 connected to 2 and 4
    atoms[3].neighbor[0] = 2;
    atoms[3].neighbor[1] = 4;
    atoms[3].valence = 2;

    // Atom 4 connected to 3 and 5
    atoms[4].neighbor[0] = 3;
    atoms[4].neighbor[1] = 5;
    atoms[4].valence = 2;

    // Atom 5 connected to 4 and 0
    atoms[5].neighbor[0] = 4;
    atoms[5].neighbor[1] = 0;
    atoms[5].neighbor[2] = 6;
    atoms[5].valence = 3;

    // Atom 6 connected to 5
    atoms[6].neighbor[0] = 5;
    atoms[6].valence = 1;

    RingSystems* rs = find_rings(atoms, num_at);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 1);

    const Ring& ring = rs->rings[0];
    EXPECT_EQ(ring.size, 6);

    // print_ring_result(rs);

    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 0, 1,10), 1);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 0, 1, 2), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 6, 1, 10), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 6, 1, 10), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 6, 1, 10), 0);

    EXPECT_EQ(are_atoms_in_same_small_ring(nullptr, num_at, rs, 6, 1, 10), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, nullptr, 6, 1, 10), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, -15, rs, 6, 1, 10), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, 100, rs, 6, 1, 10), 0);

    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, -2, 1, 10), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 6, 12312, 10), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 6, 1, -1), 0);
    EXPECT_EQ(are_atoms_in_same_small_ring(atoms, num_at, rs, 6, 1, 100), 0);

    free_ring_system(rs);

    FreeInpAtom(&atoms);
}

TEST(test_ring_detection, is_fused_ring_pivot__1_two_fused_rings) {

    INCHI_IOSTREAM input_stream;

    const char *molblock =
        "test mol fused benzene                   \n"
        "  -INDIGO-03302612192D                   \n"
        "                                         \n"
        "  0  0  0  0  0  0  0  0  0  0  0 V3000  \n"
        "M  V30 BEGIN CTAB                        \n"
        "M  V30 COUNTS 10 11 0 0 0                \n"
        "M  V30 BEGIN ATOM                        \n"
        "M  V30 1 C 4.83485 -5.40007 0.0 0        \n"
        "M  V30 2 C 6.56515 -5.39959 0.0 0        \n"
        "M  V30 3 C 5.70164 -4.89997 0.0 0        \n"
        "M  V30 4 C 6.56515 -6.40053 0.0 0        \n"
        "M  V30 5 C 4.83485 -6.40502 0.0 0        \n"
        "M  V30 6 C 5.70382 -6.90003 0.0 0        \n"
        "M  V30 7 C 7.42965 -4.90121 0.0 0        \n"
        "M  V30 8 C 8.29683 -5.40155 0.0 0        \n"
        "M  V30 9 C 7.4358 -6.90287 0.0 0         \n"
        "M  V30 10 C 8.29903 -6.39746 0.0 0       \n"
        "M  V30 END ATOM                          \n"
        "M  V30 BEGIN BOND                        \n"
        "M  V30 1 2 3 1                           \n"
        "M  V30 2 1 1 5                           \n"
        "M  V30 3 2 5 6                           \n"
        "M  V30 4 1 6 4                           \n"
        "M  V30 5 2 4 2                           \n"
        "M  V30 6 1 2 3                           \n"
        "M  V30 7 1 4 9                           \n"
        "M  V30 8 2 9 10                          \n"
        "M  V30 9 1 10 8                          \n"
        "M  V30 10 2 8 7                          \n"
        "M  V30 11 1 7 2                          \n"
        "M  V30 END BOND                          \n"
        "M  V30 END CTAB                          \n"
        "M  END                                   \n";


    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

    ORIG_ATOM_DATA *orig_inp_data = (ORIG_ATOM_DATA*)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));
    int bMergeAllInputStructures = 0;
    int bGetOrigCoord = 0;
    int bDoNotAddH = 0;
    int treat_polymers = 0;
    int treat_NPZz = 0;
    const char *pSdfLabel = nullptr;
    char *pSdfValue = nullptr;
    unsigned long *lSdfId = nullptr;
    long *lMolfileNumber = nullptr;
    INCHI_MODE pInpAtomFlags = 0;
    int err = 0;
    char *pStrErr = nullptr;
    int bNoWarnings = 0;

    int ret = CreateOrigInpDataFromMolfile(
        &input_stream,
        orig_inp_data,
        bMergeAllInputStructures,
        bGetOrigCoord,
        bDoNotAddH,
        treat_polymers,
        treat_NPZz,
        pSdfLabel,
        pSdfValue,
        lSdfId,
        lMolfileNumber,
        &pInpAtomFlags,
        &err,
        pStrErr,
        bNoWarnings);


    RingSystems *rs = find_rings(orig_inp_data->at, orig_inp_data->num_inp_atoms);

    print_ring_result(rs);

    EXPECT_NE(rs, nullptr);
    EXPECT_EQ(rs->count, 3);

    EXPECT_EQ(rs->rings[0].size, 6);
    EXPECT_EQ(rs->rings[1].size, 10);
    EXPECT_EQ(rs->rings[2].size, 6);

    EXPECT_EQ(is_fused_ring_pivot(nullptr, orig_inp_data->at, 0, 1), 0);

    EXPECT_EQ(is_fused_ring_pivot(rs, nullptr, 0, 1), 0);

    EXPECT_EQ(is_fused_ring_pivot(rs, orig_inp_data->at, -1, 1), 0);

    EXPECT_EQ(is_fused_ring_pivot(rs, orig_inp_data->at, 0, 123423), 0);

    EXPECT_EQ(is_fused_ring_pivot(rs, orig_inp_data->at, 0, 1), 0);

    EXPECT_EQ(is_fused_ring_pivot(rs, orig_inp_data->at, 5, 6), 0);

    EXPECT_EQ(is_fused_ring_pivot(rs, orig_inp_data->at, 1, 3), 1);

    FreeOrigAtData(orig_inp_data);
    inchi_free(orig_inp_data);

    free_ring_system(rs);

    inchi_ios_free_str(&input_stream);
}
