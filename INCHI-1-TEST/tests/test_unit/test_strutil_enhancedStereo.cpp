#include <gtest/gtest.h>

#include <vector>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/strutil.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
}

TEST(test_strutil_enhancedStereo, test_set_EnhancedStereo_t_m_layers_1)
{

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

    EXPECT_EQ(ret, orig_inp_data->num_inp_atoms);

    int num_at = orig_inp_data->num_inp_atoms;

    int found_num_bonds = 0;
    int found_num_isotopic = 0;

    inp_ATOM *at = CreateInpAtom(num_at);
    INChI *inchi = Alloc_INChI(at, num_at, &found_num_bonds, &found_num_isotopic, 0);

    AT_NUMB at_tmp[] = {0,1,2,3,4,5,6,7,8};
    S_CHAR parity_tmp[] = {2,1,1,2,2,1,2,2,1};
    for (int i = 0; i < 9; i++) {
        inchi->Stereo->nNumber[i] = at_tmp[i];
        inchi->Stereo->t_parity[i] = parity_tmp[i];
    }

    inchi->Stereo->nNumberOfStereoCenters = 9;
    inchi->Stereo->nCompInv2Abs = -1;
    inchi->nNumberOfAtoms = num_at;

    int nAllocMode = 0;
    int bOrigCoord = 0;

    INChI_Aux *pAux = Alloc_INChI_Aux( num_at,
                                       0, // inp_cur_data->num_isotopic,
                                       nAllocMode,
                                       bOrigCoord );


    pAux->nNumberOfAtoms = 9;

    pAux->nOrigAtNosInCanonOrd[0] = 4;
    pAux->nOrigAtNosInCanonOrd[1] = 5;
    pAux->nOrigAtNosInCanonOrd[2] = 1;
    pAux->nOrigAtNosInCanonOrd[3] = 2;
    pAux->nOrigAtNosInCanonOrd[4] = 3;
    pAux->nOrigAtNosInCanonOrd[5] = 12;
    pAux->nOrigAtNosInCanonOrd[6] = 13;
    pAux->nOrigAtNosInCanonOrd[7] = 14;

    ret = set_EnhancedStereo_t_m_layers(orig_inp_data, inchi, pAux);

    EXPECT_EQ(ret, 0);

    FreeOrigAtData(orig_inp_data);
    inchi_free(orig_inp_data);
    Free_INChI_Aux(&pAux);
    Free_INChI(&inchi);
    FreeInpAtom(&at);

    inchi_ios_free_str(&input_stream);

}

TEST(test_strutil_enhancedStereo, test_get_canonical_atom_number_1)
{

    INChI_Aux aux;
    int n_atoms = 5;
    AT_NUMB orig_atoms[] = {10, 20, 30, 40, 50};
    aux.nNumberOfAtoms = n_atoms;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    // Should return 1-based canonical atom number for each original atom number
    EXPECT_EQ(get_canonical_atom_number(&aux, 10), 1);
    EXPECT_EQ(get_canonical_atom_number(&aux, 20), 2);
    EXPECT_EQ(get_canonical_atom_number(&aux, 30), 3);
    EXPECT_EQ(get_canonical_atom_number(&aux, 40), 4);
    EXPECT_EQ(get_canonical_atom_number(&aux, 50), 5);

    // Should return -1 for atom numbers not present
    EXPECT_EQ(get_canonical_atom_number(&aux, 99), -1);
    EXPECT_EQ(get_canonical_atom_number(&aux, 0), -1);

}

/* The orig->canon reverse map replaces the linear scan in
   the enhanced-stereo hot loops. It must answer exactly like the linear scan. */
TEST(test_strutil_enhancedStereo, orig_to_canon_map_matches_linear_scan)
{
    INChI_Aux aux = {0};
    AT_NUMB orig_atoms[] = {10, 20, 30, 40, 50};
    aux.nNumberOfAtoms = 5;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    int map_size = 0;
    int *map = make_orig_to_canon_map(&aux, &map_size);
    ASSERT_NE(map, nullptr);
    EXPECT_GT(map_size, 50);

    for (int orig = 0; orig <= 60; orig++)
    {
        EXPECT_EQ(lookup_canonical_atom_number(map, map_size, &aux, orig),
                  get_canonical_atom_number(&aux, orig))
            << "orig atom " << orig;
    }

    inchi_free(map);
}

TEST(test_strutil_enhancedStereo, orig_to_canon_map_duplicates_keep_lowest_canon)
{
    INChI_Aux aux = {0};
    AT_NUMB orig_atoms[] = {7, 3, 7, 3};
    aux.nNumberOfAtoms = 4;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    int map_size = 0;
    int *map = make_orig_to_canon_map(&aux, &map_size);
    ASSERT_NE(map, nullptr);

    EXPECT_EQ(lookup_canonical_atom_number(map, map_size, &aux, 7), 1);
    EXPECT_EQ(lookup_canonical_atom_number(map, map_size, &aux, 3), 2);
    EXPECT_EQ(get_canonical_atom_number(&aux, 7), 1);
    EXPECT_EQ(get_canonical_atom_number(&aux, 3), 2);

    inchi_free(map);
}

TEST(test_strutil_enhancedStereo, orig_to_canon_map_rejects_invalid_aux)
{
    int map_size = -1;

    EXPECT_EQ(make_orig_to_canon_map(nullptr, &map_size), nullptr);
    EXPECT_EQ(map_size, 0);

    INChI_Aux aux = {0};
    aux.nNumberOfAtoms = 3;
    aux.nOrigAtNosInCanonOrd = nullptr;
    EXPECT_EQ(make_orig_to_canon_map(&aux, &map_size), nullptr);
    EXPECT_EQ(map_size, 0);

    AT_NUMB orig_atoms[] = {1};
    aux.nOrigAtNosInCanonOrd = orig_atoms;
    aux.nNumberOfAtoms = 0;
    EXPECT_EQ(make_orig_to_canon_map(&aux, &map_size), nullptr);
    EXPECT_EQ(map_size, 0);
}

/* A NULL map (e.g. allocation failure) must still yield correct answers. */
TEST(test_strutil_enhancedStereo, lookup_canonical_atom_number_falls_back_without_map)
{
    INChI_Aux aux = {0};
    AT_NUMB orig_atoms[] = {10, 20, 30};
    aux.nNumberOfAtoms = 3;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    EXPECT_EQ(lookup_canonical_atom_number(nullptr, 0, &aux, 20), 2);
    EXPECT_EQ(lookup_canonical_atom_number(nullptr, 0, &aux, 99), -1);
    EXPECT_EQ(lookup_canonical_atom_number(nullptr, 0, nullptr, 20), -1);
}

TEST(test_strutil_enhancedStereo, orig_to_canon_map_matches_linear_scan_large)
{
    const int n_atoms = 5000;
    std::vector<AT_NUMB> orig_atoms(n_atoms);
    for (int i = 0; i < n_atoms; i++)
    {
        orig_atoms[i] = (AT_NUMB)(n_atoms - i); // reversed: worst case for the scan
    }

    INChI_Aux aux = {0};
    aux.nNumberOfAtoms = n_atoms;
    aux.nOrigAtNosInCanonOrd = orig_atoms.data();

    int map_size = 0;
    int *map = make_orig_to_canon_map(&aux, &map_size);
    ASSERT_NE(map, nullptr);

    for (int orig = 1; orig <= n_atoms; orig++)
    {
        ASSERT_EQ(lookup_canonical_atom_number(map, map_size, &aux, orig),
                  n_atoms - orig + 1)
            << "orig atom " << orig;
    }
    EXPECT_EQ(lookup_canonical_atom_number(map, map_size, &aux, n_atoms + 1), -1);

    inchi_free(map);
}

TEST(test_strutil_enhancedStereo, test_get_parity_idx_from_canonical_atom_number)
{

    // Example: canonical atom numbers for a molecule with 4 stereo centers
    AT_NUMB nNumber[] = {3, 1, 4, 2};
    int nof_atoms = 4;

    // Should return the index where the canonical atom number is found
    EXPECT_EQ(get_parity_idx_from_canonical_atom_number(3, nNumber, nof_atoms), 0);
    EXPECT_EQ(get_parity_idx_from_canonical_atom_number(1, nNumber, nof_atoms), 1);
    EXPECT_EQ(get_parity_idx_from_canonical_atom_number(4, nNumber, nof_atoms), 2);
    EXPECT_EQ(get_parity_idx_from_canonical_atom_number(2, nNumber, nof_atoms), 3);

    // Should return -1 for a canonical atom number not present
    EXPECT_EQ(get_parity_idx_from_canonical_atom_number(5, nNumber, nof_atoms), -1);
    EXPECT_EQ(get_parity_idx_from_canonical_atom_number(0, nNumber, nof_atoms), -1);
}

TEST(test_strutil_enhancedStereo, invert_parities_basic)
{
    // Setup INChI_Stereo
    INChI_Stereo stereo;
    AT_NUMB nNumber[] = {1, 2, 3};
    S_CHAR t_parity[] = {2, 1, 2}; // 2=+, 1=-
    stereo.nNumber = nNumber;
    stereo.t_parity = t_parity;
    stereo.nNumberOfStereoCenters = 3;
    stereo.nCompInv2Abs = 1;

    // Setup INChI
    INChI inchi = {0};
    inchi.Stereo = &stereo;

    // Setup INChI_Aux
    INChI_Aux aux = {0};
    AT_NUMB orig_atoms[] = {1, 2, 3};
    aux.nNumberOfAtoms = 3;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    // Setup list_atoms: one group, 3 atoms (original numbers 1,2,3)
    int group1[] = {0, 3, 1, 2, 3}; // [unused, n_atoms, orig_atom1, orig_atom2, orig_atom3]
    int* lists[1] = {group1};

    // Call invert_parities for absolute group
    int ret = invert_parities(&inchi, &aux, lists, 1, 1);

    EXPECT_EQ(ret, 0);

    EXPECT_EQ(stereo.t_parity[0], 1);
    EXPECT_EQ(stereo.t_parity[1], 2);
    EXPECT_EQ(stereo.t_parity[2], 1);

    EXPECT_EQ(stereo.nCompInv2Abs, -1);

    t_parity[0] = 2;
    t_parity[1] = 1;
    t_parity[2] = 2;

    stereo.nCompInv2Abs = 1;

    ret = invert_parities(&inchi, &aux, lists, 1, 0);

    EXPECT_EQ(ret, 0);

    EXPECT_EQ(stereo.t_parity[0], 1);
    EXPECT_EQ(stereo.t_parity[1], 2);
    EXPECT_EQ(stereo.t_parity[2], 1);

    EXPECT_EQ(stereo.nCompInv2Abs, 1);

    ret = invert_parities(&inchi, &aux, lists, 1, 0);

    EXPECT_EQ(ret, 0);

    EXPECT_EQ(stereo.t_parity[0], 1);
    EXPECT_EQ(stereo.t_parity[1], 2);
    EXPECT_EQ(stereo.t_parity[2], 1);

    EXPECT_EQ(stereo.nCompInv2Abs, 1);
}

