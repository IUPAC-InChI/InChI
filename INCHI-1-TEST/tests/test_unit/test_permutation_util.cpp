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

TEST(permutation_util_testing, test_PermuteMolfileText_happy_path)
{
    char permuted_moltext[MAX_MOLTEXT_SIZE] = {0};

    srand(42);
    ASSERT_EQ(PermuteMolfileText(MOLTEXT, permuted_moltext, MAX_MOLTEXT_SIZE), 0);

    EXPECT_STRNE(permuted_moltext, MOLTEXT);

    char options[] = "-DoNotAddH";
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
