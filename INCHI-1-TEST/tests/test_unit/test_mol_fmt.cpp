#include <gtest/gtest.h>

extern "C" {
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mol_fmt.h"
}

TEST(mol_fmt_testing, test_MolfileStrnread)
{
    char dest[100];
    char source[] = "Hello World";
    char *first_space;
    int len = MolfileStrnread(dest, source, 100, &first_space);
    EXPECT_EQ(len, 11);
    EXPECT_STREQ(dest, "Hello World");
    EXPECT_EQ(first_space, dest + 11);
    EXPECT_EQ(first_space[0], '\0'); // null terminator

    len = MolfileStrnread(dest, source, 6, &first_space);
    EXPECT_EQ(len, 6);
    EXPECT_STREQ(dest, "Hello ");
    EXPECT_EQ(first_space, dest + 5);
    EXPECT_EQ(first_space[0], ' '); // space terminator
}

TEST(mol_fmt_testing, test_MolfileReadField)
{
    // int MolfileReadField(void *data, int field_len, int data_type,  char **line_ptr)

    // SEG FAULT?
    // EXPECT_EQ(MolfileReadField(NULL, 0, MOL_FMT_STRING_DATA, NULL), 0); // no data, no line pointer

    // MOL_FMT_STRING_DATA
    char data_str[5];
    char source_data1[] = "ABCD 1234 56789";
    char *line_ptr1     = source_data1;
    int ret_val_str     = MolfileReadField(data_str, sizeof(data_str), MOL_FMT_STRING_DATA, &line_ptr1);
    EXPECT_EQ(ret_val_str, 4);
    EXPECT_STREQ(data_str, "ABCD");
    EXPECT_EQ(line_ptr1, source_data1 + 5);

    // MOL_FMT_CHAR_INT_DATA -128 to 127
    char data_char[2];
    char source_data2[] = "13";
    char *line_ptr2     = source_data2;
    int ret_val_char    = MolfileReadField(data_char, sizeof(data_char), MOL_FMT_CHAR_INT_DATA, &line_ptr2);
    EXPECT_EQ(ret_val_char, 1);
    EXPECT_EQ(*data_char, static_cast<char>(13));
    EXPECT_EQ(line_ptr2, source_data2 + 2);

    // MOL_FMT_SHORT_INT_DATA  -32,768 to 32,767
    signed short data_s_int[10];
    char source_data3[] = "12345";
    char *line_ptr3     = source_data3;
    int ret_val_s_int   = MolfileReadField(data_s_int, sizeof(data_s_int), MOL_FMT_SHORT_INT_DATA, &line_ptr3);
    EXPECT_EQ(ret_val_s_int, 1);
    EXPECT_EQ(*data_s_int, static_cast<signed short>(12345));
    EXPECT_EQ(line_ptr3, source_data3 + 5);

    // MOL_FMT_LONG_INT_DATA -2,147,483,648 to 2,147,483,647
    long data_long[1];
    char source_data4[] = "1234567";
    char *line_ptr4     = source_data4;
    int ret_val_long    = MolfileReadField(data_long, sizeof(data_long), MOL_FMT_LONG_INT_DATA, &line_ptr4);
    EXPECT_EQ(ret_val_long, 1);
    EXPECT_EQ(*data_long, static_cast<long>(1234567));
    EXPECT_EQ(line_ptr4, source_data4 + 7);

    // MOL_FMT_DOUBLE_DATA
    double data_double[1];
    char source_data5[] = "123.4567";
    char *line_ptr5     = source_data5;
    int ret_val_double  = MolfileReadField(data_double, sizeof(data_double), MOL_FMT_DOUBLE_DATA, &line_ptr5);
    EXPECT_EQ(ret_val_double, 1);
    EXPECT_EQ(*data_double, static_cast<double>(123.4567));
    EXPECT_EQ(line_ptr5, source_data5 + 8);

    // MOL_FMT_FLOAT_DATA
    float data_float[2];
    char source_data6[] = "12233.45";
    char *line_ptr6     = source_data6;
    int ret_val_float   = MolfileReadField(data_float, sizeof(data_float), MOL_FMT_FLOAT_DATA, &line_ptr6);
    EXPECT_EQ(ret_val_float, 1);
    EXPECT_EQ(*data_float, static_cast<float>(12233.45));
    EXPECT_EQ(line_ptr6, source_data6 + 8);

    // MOL_FMT_JUMP_TO_RIGHT
    int jmp_idx = 9;
    char data_jmp[jmp_idx];
    char source_data7[] = "ABCD 1234 56789";
    char *line_ptr7     = source_data7;
    int ret_val_jmp     = MolfileReadField(data_jmp, sizeof(data_jmp), MOL_FMT_JUMP_TO_RIGHT, &line_ptr7);
    EXPECT_EQ(ret_val_jmp, jmp_idx);
    EXPECT_EQ(line_ptr7, source_data7 + jmp_idx);

    // MOL_FMT_FLOAT_DATA
    float data_float_neg[2];
    char source_data8[]   = "AVCD 23d";
    char *line_ptr8       = source_data8;
    int ret_val_float_neg = MolfileReadField(data_float_neg, sizeof(data_float_neg), MOL_FMT_FLOAT_DATA, &line_ptr8);
    EXPECT_EQ(ret_val_float_neg, -1);
    EXPECT_EQ(*data_float_neg, static_cast<float>(0));
    EXPECT_EQ(line_ptr8, source_data8 + 8);
}

TEST(mol_fmt_testing, test_MolfileExtractStrucNum)
{
    // long MolfileExtractStrucNum(MOL_FMT_HEADER_BLOCK *pHdr);

    MOL_FMT_HEADER_BLOCK hdr;
    memset(&hdr, 0, sizeof(hdr));

    /* valid header: "Structure #42" and line2 starts with INCHI_NAME and contains "SDfile Output" */
    strncpy(hdr.molname, "Structure #42", sizeof(hdr.molname) - 1);
    snprintf(hdr.line2, sizeof(hdr.line2), "%s SDfile Output", INCHI_NAME);

    long num = MolfileExtractStrucNum(&hdr);
    EXPECT_EQ(num, 42);

    //--------------------------
    memset(&hdr, 0, sizeof(hdr));

    /* correct "Structure #5" but line2 does not start with INCHI_NAME -> should return 0 */
    strncpy(hdr.molname, "Structure #5", sizeof(hdr.molname) - 1);
    strncpy(hdr.line2, "SomeOtherProducer SDfile Output", sizeof(hdr.line2) - 1);

    num = MolfileExtractStrucNum(&hdr);
    EXPECT_EQ(num, 0);

    //--------------------------
    memset(&hdr, 0, sizeof(hdr));

    /* non-numeric structure number should return 0 */
    strncpy(hdr.molname, "Structure #XX", sizeof(hdr.molname) - 1);
    snprintf(hdr.line2, sizeof(hdr.line2), "%s SDfile Output", INCHI_NAME);

    num = MolfileExtractStrucNum(&hdr);
    EXPECT_EQ(num, 0);

    //--------------------------
    memset(&hdr, 0, sizeof(hdr));

    /* non-numeric structure number should return 0 */
    strncpy(hdr.molname, "Structure #", sizeof(hdr.molname) - 1);
    snprintf(hdr.line2, sizeof(hdr.line2), "%s SDfile Output", INCHI_NAME);

    num = MolfileExtractStrucNum(&hdr);
    EXPECT_EQ(num, 0);

    //--------------------------
    memset(&hdr, 0, sizeof(hdr));

    /* non-numeric structure number should return 0 */
    strncpy(hdr.molname, "Strructure #12", sizeof(hdr.molname) - 1);
    snprintf(hdr.line2, sizeof(hdr.line2), "%s SDfile Output", INCHI_NAME);

    num = MolfileExtractStrucNum(&hdr);
    EXPECT_EQ(num, 0);
}

TEST(mol_fmt_testing, test_MolfileHasNoChemStruc)
{
    MOL_FMT_DATA mfdata;
    MOL_FMT_CTAB ctab;
    MOL_FMT_BOND bonds;
    MOL_FMT_ATOM *atoms;

    // Test NULL mfdata
    EXPECT_EQ(MolfileHasNoChemStruc(NULL), 1);

    // Initialize mfdata with NULL atoms
    memset(&mfdata, 0, sizeof(MOL_FMT_DATA));
    memset(&ctab, 0, sizeof(MOL_FMT_CTAB));
    mfdata.ctab = ctab;
    EXPECT_EQ(MolfileHasNoChemStruc(&mfdata), 1);

    // Test with n_atoms <= 0
    atoms = (MOL_FMT_ATOM *)calloc(1, sizeof(MOL_FMT_ATOM));
    ASSERT_NE(atoms, nullptr);
    mfdata.ctab.atoms   = atoms;
    mfdata.ctab.n_atoms = 0;
    EXPECT_EQ(MolfileHasNoChemStruc(&mfdata), 1);
    mfdata.ctab.n_atoms = -1;
    EXPECT_EQ(MolfileHasNoChemStruc(&mfdata), 1);

    // Test with bonds declared but no bonds array
    mfdata.ctab.n_atoms = 1;
    mfdata.ctab.n_bonds = 1;
    mfdata.ctab.bonds   = NULL;
    EXPECT_EQ(MolfileHasNoChemStruc(&mfdata), 1);

    // Test valid structure (has atoms and either no bonds or valid bonds array)
    mfdata.ctab.n_bonds = 0;
    EXPECT_EQ(MolfileHasNoChemStruc(&mfdata), 0);

    memset(&bonds, 0, sizeof(MOL_FMT_BOND));
    mfdata.ctab.n_bonds = 1;
    mfdata.ctab.bonds   = &bonds;
    EXPECT_EQ(MolfileHasNoChemStruc(&mfdata), 0);

    // Clean up
    free(atoms);
}

TEST(mol_fmt_testing, test_FreeMolfileData)
{

    MOL_FMT_DATA *mfdata;

    MOL_FMT_HEADER_BLOCK *hdr;

    MOL_FMT_CTAB *ctab;
    MOL_FMT_BOND *bonds;
    MOL_FMT_ATOM *atoms;

    MOL_FMT_SGROUPS *sgroups;
    MOL_COORD *coords;
    MOL_FMT_v3000 *v3000;

    mfdata               = (MOL_FMT_DATA *)calloc(1, sizeof(MOL_FMT_DATA));

    hdr                  = (MOL_FMT_HEADER_BLOCK *)calloc(1, sizeof(MOL_FMT_HEADER_BLOCK));

    ctab                 = (MOL_FMT_CTAB *)calloc(1, sizeof(MOL_FMT_CTAB));
    atoms                = (MOL_FMT_ATOM *)calloc(1, sizeof(MOL_FMT_ATOM));
    bonds                = (MOL_FMT_BOND *)calloc(1, sizeof(MOL_FMT_BOND));

    sgroups              = (MOL_FMT_SGROUPS *)calloc(1, sizeof(MOL_FMT_SGROUPS));

    coords               = (MOL_COORD *)calloc(1, sizeof(MOL_COORD));
    v3000                = (MOL_FMT_v3000 *)calloc(1, sizeof(MOL_FMT_v3000));

    mfdata->hdr          = *hdr;

    mfdata->ctab         = *ctab;
    mfdata->ctab.atoms   = atoms;
    mfdata->ctab.bonds   = bonds;

    mfdata->ctab.sgroups = *sgroups;
    mfdata->ctab.coords  = coords;
    mfdata->ctab.v3000   = v3000;

    EXPECT_EQ(FreeMolfileData(NULL), nullptr);

    EXPECT_EQ(FreeMolfileData(mfdata), nullptr);
}
