#include <cstdlib>
#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inpdef.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichimain.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/permutation_util.h"
}

const int MAX_MOLTEXT_SIZE = 8192;
const char *const MOLTEXT =
    "\n"
    "  InChIV10                                     \n"
    "\n"
    "  5  4  0  0  0  0  0  0  0  0  1 V2000\n"
    "    1.2124    0.0000    0.0000 O   0  0  0     0  0  0  0  0  0\n"
    "    2.4249    0.7000    0.0000 C   0  0  0     0  0  0  0  0  0\n"
    "    3.6373    0.0000    0.0000 C   0  0  0     0  0  0  0  0  0\n"
    "    2.4249    2.1000    0.0000 O   0  0  0     0  0  0  0  0  0\n"
    "    0.0000    0.7000    0.0000 Y   0  0  0     0  0  0  0  0  0\n"
    "  1  2  1  0  0  0  0\n"
    "  1  5  1  0  0  0  0\n"
    "  2  3  1  0  0  0  0\n"
    "  2  4  2  0  0  0  0\n"
    "M  END\n"
    "$$$$\n";

TEST(permutation_util_testing, test_OrigAtData_Permute)
{

    // 1. Populate atom data

    INCHI_IOSTREAM input_stream;
    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, "%s", MOLTEXT);

    ORIG_ATOM_DATA atom_data{};
    INCHI_MODE input_atom_flags = 0;
    int struct_read_error = 0;

    int num_atoms = 0;
    num_atoms = CreateOrigInpDataFromMolfile(
        &input_stream,
        &atom_data,
        0,
        0,
        0,
        0,
        0,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &input_atom_flags,
        &struct_read_error,
        nullptr,
        0);

    ASSERT_EQ(num_atoms, 5);

    // 2. Permute atom indices

    ORIG_ATOM_DATA permuted_atom_data{};

    int duplication_failed = 0;
    duplication_failed = OrigAtData_Duplicate(&permuted_atom_data, &atom_data);

    int permutation_mapping[num_atoms];
    for (int i = 0; i < num_atoms; i++)
    {
        permutation_mapping[i] = i;
    }
    shuffle((void *)permutation_mapping, num_atoms, sizeof(int));
    OrigAtData_Permute(&permuted_atom_data, &atom_data, permutation_mapping);

    // 3. Write molfile with permuted atom indices

    INCHI_IOSTREAM output_stream{}, permuted_output_stream{};
    inchi_ios_init(&output_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_init(&permuted_output_stream, INCHI_IOS_TYPE_STRING, nullptr);

    int return_code = 0;
    return_code = OrigAtData_WriteToSDfile(
        &atom_data,
        &output_stream,
        nullptr,
        nullptr,
        0,
        0,
        nullptr,
        nullptr);
    return_code = OrigAtData_WriteToSDfile(
        &permuted_atom_data,
        &permuted_output_stream,
        nullptr,
        nullptr,
        0,
        0,
        nullptr,
        nullptr);

    INCHI_IOSTREAM *output_file = &output_stream;
    INCHI_IOSTREAM *permuted_output_file = &permuted_output_stream;

    EXPECT_STREQ(MOLTEXT, output_file->s.pStr);
    EXPECT_STRNE(output_file->s.pStr, permuted_output_file->s.pStr);

    inchi_ios_free_str(&input_stream);
    inchi_ios_free_str(&output_stream);
    inchi_ios_free_str(&permuted_output_stream);

    FreeOrigAtData(&atom_data);
    FreeOrigAtData(&permuted_atom_data);
}

TEST(permutation_util_testing, test_OrigAtData_Permute_atom_lists)
{
    // GitHub issue #333: one-based and exceptional (0) atom indices
    // in polymer units and V3000 lists must be permuted correctly.
    // Positive and -1 cap/end-atom indices are covered by
    // test_OrigAtData_Permute_polymer_caps_and_end_atoms.
    const char *const moltext =
        "\n"
        "  test\n"
        "\n"
        "  0  0  0     0  0            999 V3000\n"
        "M  V30 BEGIN CTAB\n"
        "M  V30 COUNTS 2 1 1 0 0\n"
        "M  V30 BEGIN ATOM\n"
        "M  V30 1 Fe 0 0 0 0\n"
        "M  V30 2 C 0 0 0 0\n"
        "M  V30 END ATOM\n"
        "M  V30 BEGIN BOND\n"
        "M  V30 1 9 1 2 ENDPTS=(1 2) ATTACH=ALL\n"
        "M  V30 END BOND\n"
        "M  V30 BEGIN SGROUP\n"
        "M  V30 1 SRU 1 ATOMS=(2 1 2)\n"
        "M  V30 END SGROUP\n"
        "M  V30 BEGIN COLLECTION\n"
        "M  V30 MDLV30/STEABS ATOMS=(1 1)\n"
        "M  V30 MDLV30/STEREL1 ATOMS=(1 2)\n"
        "M  V30 MDLV30/STERAC1 ATOMS=(1 2)\n"
        "M  V30 END COLLECTION\n"
        "M  V30 END CTAB\n"
        "M  END\n";

    INCHI_IOSTREAM input_stream;
    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, "%s", moltext);

    ORIG_ATOM_DATA atom_data{};
    INCHI_MODE input_atom_flags = 0;
    int struct_read_error = 0;
    ASSERT_EQ(CreateOrigInpDataFromMolfile(
                  &input_stream,
                  &atom_data,
                  0, 0, 0, POLYMERS_MODERN, 0,
                  nullptr, nullptr, nullptr, nullptr,
                  &input_atom_flags,
                  &struct_read_error,
                  nullptr, 0),
              2);
    ASSERT_EQ(struct_read_error, 0);

    // Preconditions: the lists contain one-based and exceptional indices.
    const OAD_PolymerUnit *unit = atom_data.polymer->units[0];
    ASSERT_EQ(unit->cap1, 0);
    ASSERT_EQ(unit->cap2, 0);
    ASSERT_EQ(unit->end_atom1, 0);
    ASSERT_EQ(unit->end_atom2, 0);
    const OAD_V3000 *v3000 = atom_data.v3000;
    ASSERT_EQ(v3000->atom_index_orig[0], 1);
    ASSERT_EQ(v3000->atom_index_orig[1], 0); // GitHub issue #330
    ASSERT_EQ(v3000->atom_index_fin[0], 1);
    ASSERT_EQ(v3000->atom_index_fin[1], 0); // GitHub issue #330
    ASSERT_EQ(v3000->n_haptic_bonds, 1);
    ASSERT_EQ(v3000->lists_haptic_bonds[0][1], 1); // metal atom
    ASSERT_EQ(v3000->lists_haptic_bonds[0][2], 1); // number of endpoints
    ASSERT_EQ(v3000->lists_haptic_bonds[0][3], 2); // endpoint
    ASSERT_EQ(v3000->n_steabs, 1);
    ASSERT_EQ(v3000->lists_steabs[0][2], 1);
    ASSERT_EQ(v3000->n_sterel, 1);
    ASSERT_EQ(v3000->lists_sterel[0][2], 2);
    ASSERT_EQ(v3000->n_sterac, 1);
    ASSERT_EQ(v3000->lists_sterac[0][2], 2);

    ORIG_ATOM_DATA permuted_atom_data{};
    ASSERT_EQ(OrigAtData_Duplicate(&permuted_atom_data, &atom_data), 0);

    // Swap the two atoms: one-based atom 1 <-> 2.
    int permutation_mapping[] = {1, 0};
    OrigAtData_Permute(&permuted_atom_data, &atom_data, permutation_mapping);

    const OAD_PolymerUnit *permuted_unit = permuted_atom_data.polymer->units[0];
    EXPECT_EQ(permuted_unit->cap1, 0);
    EXPECT_EQ(permuted_unit->cap2, 0);
    EXPECT_EQ(permuted_unit->end_atom1, 0);
    EXPECT_EQ(permuted_unit->end_atom2, 0);

    const OAD_V3000 *permuted_v3000 = permuted_atom_data.v3000;
    EXPECT_EQ(permuted_v3000->atom_index_orig[0], 2);
    EXPECT_EQ(permuted_v3000->atom_index_orig[1], 0);
    EXPECT_EQ(permuted_v3000->atom_index_fin[0], 2);
    EXPECT_EQ(permuted_v3000->atom_index_fin[1], 0);
    EXPECT_EQ(permuted_v3000->lists_haptic_bonds[0][1], 2);
    EXPECT_EQ(permuted_v3000->lists_haptic_bonds[0][3], 1);
    EXPECT_EQ(permuted_v3000->lists_steabs[0][2], 2);
    EXPECT_EQ(permuted_v3000->lists_sterel[0][2], 1);
    EXPECT_EQ(permuted_v3000->lists_sterac[0][2], 1);

    inchi_ios_free_str(&input_stream);
    FreeOrigAtData(&atom_data);
    FreeOrigAtData(&permuted_atom_data);
}

TEST(permutation_util_testing, test_OrigAtData_Permute_polymer_caps_and_end_atoms)
{
    // GitHub issue #333: each cap/end-atom field must be permuted into
    // itself, and -1 sentinels (unset, see runichi3.c) must stay unchanged.
    const char *const moltext =
        "\n"
        "  test\n"
        "\n"
        "  0  0  0     0  0            999 V3000\n"
        "M  V30 BEGIN CTAB\n"
        "M  V30 COUNTS 4 3 2 0 0\n"
        "M  V30 BEGIN ATOM\n"
        "M  V30 1 C 0 0 0 0\n"
        "M  V30 2 C 0 0 0 0\n"
        "M  V30 3 C 0 0 0 0\n"
        "M  V30 4 C 0 0 0 0\n"
        "M  V30 END ATOM\n"
        "M  V30 BEGIN BOND\n"
        "M  V30 1 1 1 2\n"
        "M  V30 2 1 2 3\n"
        "M  V30 3 1 3 4\n"
        "M  V30 END BOND\n"
        "M  V30 BEGIN SGROUP\n"
        "M  V30 1 SRU 1 ATOMS=(2 1 2)\n"
        "M  V30 2 SRU 2 ATOMS=(2 3 4)\n"
        "M  V30 END SGROUP\n"
        "M  V30 END CTAB\n"
        "M  END\n";

    INCHI_IOSTREAM input_stream;
    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, "%s", moltext);

    ORIG_ATOM_DATA atom_data{};
    INCHI_MODE input_atom_flags = 0;
    int struct_read_error = 0;
    ASSERT_EQ(CreateOrigInpDataFromMolfile(
                  &input_stream,
                  &atom_data,
                  0, 0, 0, POLYMERS_MODERN, 0,
                  nullptr, nullptr, nullptr, nullptr,
                  &input_atom_flags,
                  &struct_read_error,
                  nullptr, 0),
              4);
    ASSERT_EQ(struct_read_error, 0);
    ASSERT_NE(atom_data.polymer, nullptr);
    ASSERT_EQ(atom_data.polymer->n, 2);

    ORIG_ATOM_DATA permuted_atom_data{};
    ASSERT_EQ(OrigAtData_Duplicate(&permuted_atom_data, &atom_data), 0);

    // Caps and end atoms are assigned during polymer analysis, not parsing,
    // so seed them in the copy that OrigAtData_Permute updates in place.
    OAD_PolymerUnit *unit = permuted_atom_data.polymer->units[0];
    unit->cap1 = 1;
    unit->end_atom1 = 2;
    unit->end_atom2 = 3;
    unit->cap2 = 4;
    OAD_PolymerUnit *unset_unit = permuted_atom_data.polymer->units[1];
    unset_unit->cap1 = -1;
    unset_unit->end_atom1 = -1;
    unset_unit->end_atom2 = -1;
    unset_unit->cap2 = -1;

    // One-based mapping: 1 -> 3, 2 -> 1, 3 -> 4, 4 -> 2.
    int permutation_mapping[] = {2, 0, 3, 1};
    OrigAtData_Permute(&permuted_atom_data, &atom_data, permutation_mapping);

    EXPECT_EQ(unit->cap1, 3);
    EXPECT_EQ(unit->end_atom1, 1);
    EXPECT_EQ(unit->end_atom2, 4);
    EXPECT_EQ(unit->cap2, 2);

    EXPECT_EQ(unset_unit->cap1, -1);
    EXPECT_EQ(unset_unit->end_atom1, -1);
    EXPECT_EQ(unset_unit->end_atom2, -1);
    EXPECT_EQ(unset_unit->cap2, -1);

    inchi_ios_free_str(&input_stream);
    FreeOrigAtData(&atom_data);
    FreeOrigAtData(&permuted_atom_data);
}

TEST(permutation_util_testing, test_PermuteMolfileText_happy_path)
{
    char permuted_moltext[MAX_MOLTEXT_SIZE] = {0};

    srand(42);
    ASSERT_EQ(PermuteMolfileText(MOLTEXT, permuted_moltext, MAX_MOLTEXT_SIZE), 0);

    EXPECT_STRNE(permuted_moltext, MOLTEXT);

    char options[] = "";
    inchi_Output original_output;
    inchi_Output *original_poutput = &original_output;
    inchi_Output permuted_output;
    inchi_Output *permuted_poutput = &permuted_output;
    memset(original_poutput, 0, sizeof(*original_poutput));
    memset(permuted_poutput, 0, sizeof(*permuted_poutput));

    ASSERT_EQ(MakeINCHIFromMolfileText(MOLTEXT, options, original_poutput), 1);
    ASSERT_EQ(MakeINCHIFromMolfileText(permuted_moltext, options, permuted_poutput), 1);
    ASSERT_STREQ(original_poutput->szInChI, permuted_poutput->szInChI);

    FreeINCHI(original_poutput);
    FreeINCHI(permuted_poutput);
}

TEST(permutation_util_testing, test_PermuteMolfileText_keeps_implicit_hydrogens)
{
    // Methanol without explicit H (GitHub issue #311).
    const char *const methanol =
        "\n"
        "  test\n"
        "\n"
        "  2  1  0  0  0  0  0  0  0  0999 V2000\n"
        "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "    1.0000    0.0000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
        "  1  2  1  0  0  0  0  0  0  0  0  0\n"
        "M  END\n";
    char permuted_moltext[MAX_MOLTEXT_SIZE] = {0};

    srand(42);
    ASSERT_EQ(PermuteMolfileText(methanol, permuted_moltext, MAX_MOLTEXT_SIZE), 0);

    char options[] = "";
    inchi_Output permuted_output{};
    ASSERT_EQ(MakeINCHIFromMolfileText(permuted_moltext, options, &permuted_output), 0);
    EXPECT_STREQ(permuted_output.szInChI, "InChI=1S/CH4O/c1-2/h2H,1H3");

    FreeINCHI(&permuted_output);
}

TEST(permutation_util_testing, test_PermuteMolfileText_empty_moltext)
{
    const char *moltext = "";
    char permuted_moltext[MAX_MOLTEXT_SIZE] = {0};

    ASSERT_EQ(PermuteMolfileText(moltext, permuted_moltext, MAX_MOLTEXT_SIZE), -1);
}

TEST(permutation_util_testing, test_PermuteMolfileText_missing_moltext)
{
    const char *moltext = nullptr;
    char permuted_moltext[MAX_MOLTEXT_SIZE] = {0};

    ASSERT_EQ(PermuteMolfileText(moltext, permuted_moltext, MAX_MOLTEXT_SIZE), -1);
}

TEST(permutation_util_testing, test_PermuteMolfileText_missing_output_buffer)
{
    ASSERT_EQ(PermuteMolfileText(MOLTEXT, nullptr, MAX_MOLTEXT_SIZE), -1);
}

TEST(permutation_util_testing, test_PermuteMolfileText_different_seed_different_permutation)
{
    char permuted_moltext_a[MAX_MOLTEXT_SIZE] = {0};
    char permuted_moltext_b[MAX_MOLTEXT_SIZE] = {0};

    srand(42);
    ASSERT_EQ(PermuteMolfileText(MOLTEXT, permuted_moltext_a, MAX_MOLTEXT_SIZE), 0);
    srand(43);
    ASSERT_EQ(PermuteMolfileText(MOLTEXT, permuted_moltext_b, MAX_MOLTEXT_SIZE), 0);

    EXPECT_STRNE(permuted_moltext_a, permuted_moltext_b);
}
