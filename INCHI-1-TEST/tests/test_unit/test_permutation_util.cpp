#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inpdef.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichimain.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/permutation_util.h"
}

TEST(permutation_util_testing, test_OrigAtData_Permute)
{

    // 1. Populate atom data

    INCHI_IOSTREAM input_stream;
    const char *molblock =
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
    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, nullptr);
    inchi_ios_print_nodisplay(&input_stream, molblock);

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

    printf("Initial molblock:\n%s\n", molblock);
    printf("Initial molfile:\n%s\n", output_file->s.pStr);
    printf("Permuted molfile:\n%s\n", permuted_output_file->s.pStr);

    EXPECT_STREQ(molblock, output_file->s.pStr);
    EXPECT_STRNE(output_file->s.pStr, permuted_output_file->s.pStr);
}
