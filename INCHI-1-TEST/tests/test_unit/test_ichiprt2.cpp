#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichiprt2.c"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
}

TEST(test_ichiprt2, MakeStereoString_outputs_expected_sp3_string)
{
    AT_NUMB at1[8] = {3,4,5,6,7,8,9,10};
    S_CHAR parity[8] = {1,1,2,2,1,2,2,1};

    INCHI_IOS_STRING strbuf = {0};

    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);

    int bOverflow = 0;
    int ret = MakeStereoString(at1, nullptr, parity, 0, 8, &strbuf, 0, &bOverflow);

    EXPECT_EQ(strbuf.nUsedLength, strlen(strbuf.pStr));
    EXPECT_NE(std::string(strbuf.pStr).find("3-,4-,5+,6+,7-,8+,9+,10-"), std::string::npos);
    EXPECT_EQ(std::string(strbuf.pStr), "3-,4-,5+,6+,7-,8+,9+,10-");
    EXPECT_EQ(strbuf.pStr[0], '3');
    EXPECT_EQ(bOverflow, 0);
    EXPECT_EQ(ret, 24);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeMult_mult_gt_1_appends_number_and_delim)
{
    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);

    int bOverflow = 0;
    int ret = MakeMult(5, "-", &strbuf, 0, &bOverflow);

    EXPECT_EQ(std::string(strbuf.pStr), "5-");
    EXPECT_EQ(ret, 2);
    EXPECT_EQ(bOverflow, 0);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeMult_mult_2)
{
    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);

    int bOverflow = 0;
    int ret = MakeMult(10, "+", &strbuf, 0, &bOverflow);

    EXPECT_EQ(std::string(strbuf.pStr), "10+");
    EXPECT_EQ(ret, 3);
    EXPECT_EQ(bOverflow, 0);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeMult_mult_eq_1_does_nothing)
{
    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    inchi_strbuf_printf(&strbuf, "start");

    int bOverflow = 0;
    int ret = MakeMult(1, "-", &strbuf, 0, &bOverflow);

    EXPECT_EQ(std::string(strbuf.pStr), "start");
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(bOverflow, 0);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, Eql_INChI_Stereo_sp2_equal)
{
    // Setup two identical SP2 stereo objects
    INChI_Stereo s1 = {0}, s2 = {0};
    int num_bonds = 2;
    AT_NUMB bond_atom1[2] = {1, 3};
    AT_NUMB bond_atom2[2] = {2, 4};
    S_CHAR b_parity[2] = {1, 2};

    s1.nNumberOfStereoBonds = num_bonds;
    s1.nBondAtom1 = bond_atom1;
    s1.nBondAtom2 = bond_atom2;
    s1.b_parity = b_parity;

    s2.nNumberOfStereoBonds = num_bonds;
    s2.nBondAtom1 = bond_atom1;
    s2.nBondAtom2 = bond_atom2;
    s2.b_parity = b_parity;

    // Should be equal
    EXPECT_EQ(Eql_INChI_Stereo(&s1, EQL_SP2, &s2, EQL_SP2, 0), 1);
}

TEST(test_ichiprt2, Eql_INChI_Stereo_sp2_not_equal)
{
    // Setup two different SP2 stereo objects
    INChI_Stereo s1 = {0}, s2 = {0};
    AT_NUMB bond_atom1_1[2] = {1, 3};
    AT_NUMB bond_atom2_1[2] = {2, 4};
    S_CHAR b_parity_1[2] = {1, 2};

    AT_NUMB bond_atom1_2[2] = {1, 5};
    AT_NUMB bond_atom2_2[2] = {2, 6};
    S_CHAR b_parity_2[2] = {1, 1};

    s1.nNumberOfStereoBonds = 2;
    s1.nBondAtom1 = bond_atom1_1;
    s1.nBondAtom2 = bond_atom2_1;
    s1.b_parity = b_parity_1;

    s2.nNumberOfStereoBonds = 2;
    s2.nBondAtom1 = bond_atom1_2;
    s2.nBondAtom2 = bond_atom2_2;
    s2.b_parity = b_parity_2;

    // Should not be equal
    EXPECT_EQ(Eql_INChI_Stereo(&s1, EQL_SP2, &s2, EQL_SP2, 0), 0);
}

TEST(test_ichiprt2, Eql_INChI_Stereo_sp2_exists)
{
    // s1 has stereo, s2 is nullptr, eql2 is EQL_EXISTS
    INChI_Stereo s1 = {0};
    AT_NUMB bond_atom1[1] = {1};
    AT_NUMB bond_atom2[1] = {2};
    S_CHAR b_parity[1] = {1};

    s1.nNumberOfStereoBonds = 1;
    s1.nBondAtom1 = bond_atom1;
    s1.nBondAtom2 = bond_atom2;
    s1.b_parity = b_parity;

    EXPECT_EQ(Eql_INChI_Stereo(&s1, EQL_SP2, nullptr, EQL_EXISTS, 0), 1);
}

TEST(test_ichiprt2, Eql_INChI_Stereo_sp3_equal)
{
    // Setup two identical SP3 stereo objects
    INChI_Stereo s1 = {0}, s2 = {0};
    int num_centers = 2;
    AT_NUMB nNumber1[2] = {5, 6};
    S_CHAR t_parity1[2] = {1, 2};

    s1.nNumberOfStereoCenters = num_centers;
    s1.nNumber = nNumber1;
    s1.t_parity = t_parity1;

    s2.nNumberOfStereoCenters = num_centers;
    s2.nNumber = nNumber1;
    s2.t_parity = t_parity1;

    // Should be equal
    EXPECT_EQ(Eql_INChI_Stereo(&s1, EQL_SP3, &s2, EQL_SP3, 0), 1);
}

TEST(test_ichiprt2, Eql_INChI_Stereo_sp3_not_equal)
{
    // Setup two different SP3 stereo objects
    INChI_Stereo s1 = {0}, s2 = {0};
    int num_centers = 2;
    AT_NUMB nNumber1[2] = {5, 6};
    S_CHAR t_parity1[2] = {1, 2};

    AT_NUMB nNumber2[2] = {7, 8};
    S_CHAR t_parity2[2] = {2, 1};

    s1.nNumberOfStereoCenters = num_centers;
    s1.nNumber = nNumber1;
    s1.t_parity = t_parity1;

    s2.nNumberOfStereoCenters = num_centers;
    s2.nNumber = nNumber2;
    s2.t_parity = t_parity2;

    // Should not be equal
    EXPECT_EQ(Eql_INChI_Stereo(&s1, EQL_SP3, &s2, EQL_SP3, 0), 0);
}

TEST(test_ichiprt2, test_compare_ints_basic)
{
    int a = 5, b = 10, c = 5;

    // a < b
    EXPECT_LT(compare_ints(&a, &b), 0);

    // b > a
    EXPECT_GT(compare_ints(&b, &a), 0);

    // a == c
    EXPECT_EQ(compare_ints(&a, &c), 0);
}

TEST(test_ichiprt2, MakeNumber_EnhStereo_decimal)
{
    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);

    int bOverflow = 0;
    int nCtMode = 0; // decimal mode

    int n = MakeNumber_EnhStereo(42, ",", &strbuf, nCtMode, &bOverflow);

    EXPECT_EQ(bOverflow, 0);
    EXPECT_EQ(std::string(strbuf.pStr), "42,");
    EXPECT_EQ(n, 3);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeNumber_EnhStereo_abc)
{
    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    int bOverflow = 0;
    int nCtMode = CT_MODE_ABC_NUMBERS; // alphabetic mode

    int n = MakeNumber_EnhStereo(28, ";", &strbuf, nCtMode, &bOverflow);

    EXPECT_EQ(bOverflow, 0);
    // 28 in base-27 is "aa", so expect "Aa;"
    EXPECT_EQ(std::string(strbuf.pStr), "Aa;");
    EXPECT_EQ(n, 3);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeEnhStereoString_basic)
{
    // Setup INChI_Aux with 3 atoms, canonical numbers 1, 2, 3
    INChI_Aux aux = {0};
    AT_NUMB orig_atoms[] = {1, 2, 3};
    aux.nNumberOfAtoms = 3;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    // Enhanced stereo group: one group, 3 atoms (original numbers 1,2,3)
    int group1[] = {0, 3, 1, 2, 3}; // [unused, n_atoms, orig_atom1, orig_atom2, orig_atom3]
    int* enh_stereo[1] = {group1};

    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    int bOverflow = 0;
    int nCtMode = 0;

    int len = MakeEnhStereoString(&aux, &strbuf, "1", enh_stereo, 1, nCtMode, &bOverflow);

    EXPECT_EQ(bOverflow, 0);
    EXPECT_EQ(std::string(strbuf.pStr), "1(1,2,3)");
    EXPECT_EQ(len, 8);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeEnhStereoString_multiple_groups)
{
    INChI_Aux aux = {0};
    AT_NUMB orig_atoms[] = {1, 2, 3, 4};
    aux.nNumberOfAtoms = 4;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    int group1[] = {0, 2, 1, 2}; // [unused, n_atoms, orig_atom1, orig_atom2]
    int group2[] = {0, 2, 3, 4}; // [unused, n_atoms, orig_atom3, orig_atom4]
    int* enh_stereo[2] = {group1, group2};

    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    int bOverflow = 0;
    int nCtMode = 0;

    int len = MakeEnhStereoString(&aux, &strbuf, "2", enh_stereo, 2, nCtMode, &bOverflow);

    EXPECT_EQ(bOverflow, 0);
    EXPECT_EQ(std::string(strbuf.pStr), "2(1,2)(3,4)");
    EXPECT_EQ(len, 11);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeEnhStereoString_empty_group)
{
    INChI_Aux aux = {0};
    AT_NUMB orig_atoms[] = {1, 2, 3};
    aux.nNumberOfAtoms = 3;
    aux.nOrigAtNosInCanonOrd = orig_atoms;

    // Group with no valid atoms
    int group1[] = {0, 2, 99, 100}; // [unused, n_atoms, orig_atom99, orig_atom100]
    int* enh_stereo[1] = {group1};

    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    int bOverflow = 0;
    int nCtMode = 0;

    int len = MakeEnhStereoString(&aux, &strbuf, "3", enh_stereo, 1, nCtMode, &bOverflow);

    EXPECT_EQ(bOverflow, 0);
    EXPECT_EQ(std::string(strbuf.pStr), "");
    EXPECT_EQ(len, 0);

    inchi_strbuf_close(&strbuf);
}

TEST(test_ichiprt2, MakeSlayerString_basic)
{

    ORIG_ATOM_DATA *oad = (ORIG_ATOM_DATA *)inchi_calloc(1, sizeof(ORIG_ATOM_DATA));

    OAD_V3000 *v3000 = (OAD_V3000 *)inchi_calloc(1, sizeof(OAD_V3000));

    // One absolute group: 2 atoms (original numbers 1,2)
    int group_abs[] = {0, 2, 1, 2};
    v3000->n_steabs = 1;
    v3000->lists_steabs = (int**)inchi_calloc(1, sizeof(int*));
    v3000->lists_steabs[0] = group_abs;

    // One relative group: 1 atom (original number 3)
    int group_rel[] = {0, 1, 3};
    v3000->n_sterel = 1;
    v3000->lists_sterel = (int**)inchi_calloc(1, sizeof(int*));
    v3000->lists_sterel[0] = group_rel;

    // No racemic groups
    v3000->n_sterac = 0;

    oad->v3000 = v3000;

    // Setup INCHI_SORT and INChI_Aux
    INCHI_SORT *inchi_sort = (INCHI_SORT*)inchi_calloc(1, sizeof(INCHI_SORT));

    int num_at = 8;
    int num_iso_at = 1;
    int alloc_mode = 0;
    int bOrigatomflag = 0;
    int found_num_bonds = 0;
    int found_num_isotopic = 0;

    inp_ATOM *atoms = CreateInpAtom(num_at);
    INChI *inchi = Alloc_INChI(atoms, num_at, &found_num_bonds, &found_num_isotopic, 0);
    inchi->nNumberOfAtoms = num_at;

    INChI_Aux *pAux = Alloc_INChI_Aux(num_at, num_iso_at, alloc_mode, bOrigatomflag);
    pAux->nNumberOfAtoms = 3;
    pAux->nOrigAtNosInCanonOrd[0] = 1;
    pAux->nOrigAtNosInCanonOrd[1] = 2;
    pAux->nOrigAtNosInCanonOrd[2] = 3;

    inchi_sort->pINChI_Aux[0] = pAux;
    inchi_sort->pINChI[0] = inchi;

    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    int bOverflow = 0;
    int nCtMode = 0;

    int len = MakeSlayerString(oad, inchi_sort, &strbuf, OUT_TN, 1, nCtMode, &bOverflow);

    EXPECT_EQ(bOverflow, 0);
    EXPECT_EQ(std::string(strbuf.pStr), "1(1,2)2(3)");
    EXPECT_EQ(len, 10);

    inchi_free(oad->v3000->lists_steabs);
    inchi_free(oad->v3000->lists_sterel);
    inchi_free(oad->v3000);
    inchi_free(oad);
    inchi_free(inchi_sort);

    inchi_strbuf_close(&strbuf);
    FreeInpAtom(&atoms);
    Free_INChI_Aux(&pAux);
    Free_INChI(&inchi);
}

/*
 * Runs MakeSlayerString over `n_components` components whose /s substrings are
 * all distinct: component i maps original atom 2 -> canonical 1 and original
 * atom 1 -> canonical (i+2), so its substring is "1(i+2)2(1)". Both an ABS and
 * a REL group are present so the ABS-only reduction (ES-R11) does not collapse
 * them to one shared substring.
 */
static void RunSlayerDistinctComponents(int n_components,
                                        int *bOverflow,
                                        std::string *out)
{
    ORIG_ATOM_DATA oad = {0};
    OAD_V3000 v3000 = {0};

    int group_abs[] = {0, 1, 1}; // [unused, n_atoms, orig atom 1]
    int group_rel[] = {0, 1, 2}; // [unused, n_atoms, orig atom 2]
    int *lists_abs[1] = {group_abs};
    int *lists_rel[1] = {group_rel};

    v3000.n_steabs = 1;
    v3000.lists_steabs = lists_abs;
    v3000.n_sterel = 1;
    v3000.lists_sterel = lists_rel;
    v3000.n_sterac = 0;
    oad.v3000 = &v3000;

    // Only used by GET_II() to pick the TAUT_NON slot; never dereferenced further.
    INChI dummy_inchi = {0};
    dummy_inchi.nNumberOfAtoms = 1;

    INCHI_SORT *sorts = (INCHI_SORT *)inchi_calloc(n_components, sizeof(INCHI_SORT));
    std::vector<INChI_Aux *> auxes(n_components, nullptr);

    for (int i = 0; i < n_components; i++)
    {
        int num_at = i + 2;
        INChI_Aux *pAux = Alloc_INChI_Aux(num_at, 0, 0, 0);
        pAux->nNumberOfAtoms = num_at;
        for (int j = 0; j < num_at; j++)
        {
            pAux->nOrigAtNosInCanonOrd[j] = 0;
        }
        pAux->nOrigAtNosInCanonOrd[0] = 2;           // canonical 1      -> orig 2
        pAux->nOrigAtNosInCanonOrd[num_at - 1] = 1;  // canonical num_at -> orig 1

        auxes[i] = pAux;
        sorts[i].pINChI[0] = &dummy_inchi;
        sorts[i].pINChI_Aux[0] = pAux;
    }

    INCHI_IOS_STRING strbuf = {0};
    inchi_strbuf_init(&strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT);
    *bOverflow = 0;

    MakeSlayerString(&oad, sorts, &strbuf, OUT_TN, n_components, 0, bOverflow);

    *out = strbuf.pStr ? strbuf.pStr : "";

    inchi_strbuf_close(&strbuf);
    for (int i = 0; i < n_components; i++)
    {
        Free_INChI_Aux(&auxes[i]);
    }
    inchi_free(sorts);
}

TEST(test_ichiprt2, MakeSlayerString_dictionary_exactly_full_no_overflow)
{
    int bOverflow = 0;
    std::string out;

    RunSlayerDistinctComponents(ENH_STEREO_DICT_SIZE, &bOverflow, &out);

    EXPECT_EQ(bOverflow, 0);
    // All ENH_STEREO_DICT_SIZE distinct substrings emitted, separated by ';'
    EXPECT_EQ(std::count(out.begin(), out.end(), ';'), ENH_STEREO_DICT_SIZE - 1);
    EXPECT_NE(out.find("1(2)2(1)"), std::string::npos);
    EXPECT_NE(out.find("1(" + std::to_string(ENH_STEREO_DICT_SIZE + 1) + ")2(1)"),
              std::string::npos);
}

/* ES-R14 / SPEC 1 §5.A: more distinct /s substrings than the fixed dictionary
   holds must raise the overflow flag, not silently drop a component. */
TEST(test_ichiprt2, MakeSlayerString_dictionary_overflow_sets_flag)
{
    int bOverflow = 0;
    std::string out;

    RunSlayerDistinctComponents(ENH_STEREO_DICT_SIZE + 1, &bOverflow, &out);

    EXPECT_NE(bOverflow, 0);
}
