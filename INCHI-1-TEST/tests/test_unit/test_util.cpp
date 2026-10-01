#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/util.h"
}

TEST(test_util, test_get_chemical_symbols)
{

    char szElement[4];

    EXPECT_EQ(get_element_chemical_symbol(-4, szElement), -1);
    EXPECT_STREQ(szElement, "??");
    EXPECT_EQ(get_element_chemical_symbol(0, szElement), -1);
    EXPECT_STREQ(szElement, "??");
    EXPECT_EQ(get_element_chemical_symbol(1, szElement), 0);
    EXPECT_STREQ(szElement, "H");
    EXPECT_EQ(get_element_chemical_symbol(2, szElement), 0);
    EXPECT_STREQ(szElement, "He");
    EXPECT_EQ(get_element_chemical_symbol(5, szElement), 0);
    EXPECT_STREQ(szElement, "B");
    EXPECT_EQ(get_element_chemical_symbol(9, szElement), 0);
    EXPECT_STREQ(szElement, "F");
    EXPECT_EQ(get_element_chemical_symbol(45, szElement), 0);
    EXPECT_STREQ(szElement, "Rh");
    EXPECT_EQ(get_element_chemical_symbol(82, szElement), 0);
    EXPECT_STREQ(szElement, "Pb");
    EXPECT_EQ(get_element_chemical_symbol(5000, szElement), -1);
    EXPECT_STREQ(szElement, "??");
}

TEST(test_util, test_get_element_or_pseudoelement_symbol)
{

    char szElement[4];

    EXPECT_EQ(get_element_or_pseudoelement_symbol(-4, szElement), -1);
    EXPECT_STREQ(szElement, "??");
    EXPECT_EQ(get_element_or_pseudoelement_symbol(0, szElement), -1);
    EXPECT_STREQ(szElement, "??");
    EXPECT_EQ(get_element_or_pseudoelement_symbol(1, szElement), 0);
    EXPECT_STREQ(szElement, "H");
    EXPECT_EQ(get_element_or_pseudoelement_symbol(119, szElement), 0);
    EXPECT_STREQ(szElement, "Zz");
    EXPECT_EQ(get_element_or_pseudoelement_symbol(120, szElement), 0);
    EXPECT_STREQ(szElement, "Zz");
    EXPECT_EQ(get_element_or_pseudoelement_symbol(255, szElement), -1);
    EXPECT_STREQ(szElement, "??");
    EXPECT_EQ(get_element_or_pseudoelement_symbol(45, szElement), 0);
    EXPECT_STREQ(szElement, "Rh");
    EXPECT_EQ(get_element_or_pseudoelement_symbol(5000, szElement), -1);
    EXPECT_STREQ(szElement, "??");
}

TEST(test_util, test_get_periodic_table_number)
{

    char szElement[4];

    EXPECT_EQ(get_periodic_table_number(""), 255);
    EXPECT_EQ(get_periodic_table_number(" "), 255);
    EXPECT_EQ(get_periodic_table_number("??"), 255);
    EXPECT_EQ(get_periodic_table_number("S"), 16);
    EXPECT_EQ(get_periodic_table_number("Ca"), 20);
    EXPECT_EQ(get_periodic_table_number("B"), 5);
    EXPECT_EQ(get_periodic_table_number("F"), 9);
    EXPECT_EQ(get_periodic_table_number("Rh"), 45);
    EXPECT_EQ(get_periodic_table_number("Zz"), 120);
}

TEST(test_util, test_if_skip_add_H)
{

    EXPECT_EQ(if_skip_add_H(-1), 0);
    EXPECT_EQ(if_skip_add_H(5), 0);
    EXPECT_EQ(if_skip_add_H(54), 0);
    EXPECT_EQ(if_skip_add_H(25), 1);
    EXPECT_EQ(if_skip_add_H(57), 1);
    // EXPECT_EQ(if_skip_add_H(300), 0); --> SEG FAULT
}

TEST(test_util, test_get_el_valence)
{
    // nPeriodicNum, charge, val_num
    EXPECT_EQ(get_el_valence(0, -3, 0), 0);
    EXPECT_EQ(get_el_valence(12, 5, 8), 0);
    EXPECT_EQ(get_el_valence(12, 1, 5), 0);

    EXPECT_EQ(get_el_valence(6, 0, 0), 4);
    EXPECT_EQ(get_el_valence(6, 1, 0), 3);
    EXPECT_EQ(get_el_valence(6, 1, 1), 0);

    EXPECT_EQ(get_el_valence(17, 0, 0), 1);
    EXPECT_EQ(get_el_valence(17, 0, 3), 7);

    EXPECT_EQ(get_el_valence(82, 0, 1), 4);
    EXPECT_EQ(get_el_valence(82, -2, 2), 6);
}

TEST(test_util, test_get_atomic_mass)
{
    EXPECT_EQ(get_atomic_mass(""), 0);
    EXPECT_EQ(get_atomic_mass(" "), 0);
    EXPECT_EQ(get_atomic_mass("                      "), 0);
    EXPECT_EQ(get_atomic_mass("34sd"), 0);

    EXPECT_EQ(get_atomic_mass("H"), 1);
    EXPECT_EQ(get_atomic_mass("He"), 4);
    EXPECT_EQ(get_atomic_mass("Li"), 7);
    EXPECT_EQ(get_atomic_mass("Be"), 9);
    EXPECT_EQ(get_atomic_mass("B"), 11);
    EXPECT_EQ(get_atomic_mass("C"), 12);
    EXPECT_EQ(get_atomic_mass("N"), 14);
    EXPECT_EQ(get_atomic_mass("O"), 16);
    EXPECT_EQ(get_atomic_mass("F"), 19);
    EXPECT_EQ(get_atomic_mass("Ne"), 20);
    EXPECT_EQ(get_atomic_mass("Rh"), 103);
    EXPECT_EQ(get_atomic_mass("Pb"), 207);
}

TEST(test_util, test_get_atomic_mass_from_elnum)
{
    EXPECT_EQ(get_atomic_mass_from_elnum(-1), 0);
    EXPECT_EQ(get_atomic_mass_from_elnum(0), 0);
    EXPECT_EQ(get_atomic_mass_from_elnum(1), 1);
    EXPECT_EQ(get_atomic_mass_from_elnum(2), 4);
    EXPECT_EQ(get_atomic_mass_from_elnum(3), 7);
    EXPECT_EQ(get_atomic_mass_from_elnum(4), 9);
    EXPECT_EQ(get_atomic_mass_from_elnum(5), 11);
    EXPECT_EQ(get_atomic_mass_from_elnum(6), 12);
    EXPECT_EQ(get_atomic_mass_from_elnum(7), 14);
    EXPECT_EQ(get_atomic_mass_from_elnum(8), 16);
    EXPECT_EQ(get_atomic_mass_from_elnum(9), 19);
    EXPECT_EQ(get_atomic_mass_from_elnum(10), 20);
    EXPECT_EQ(get_atomic_mass_from_elnum(11), 23);

    EXPECT_EQ(get_atomic_mass_from_elnum(57), 139);

    EXPECT_EQ(get_atomic_mass_from_elnum(23423423), 0);
}

TEST(test_util, test_get_el_type)
{
    EXPECT_EQ(get_el_type(-1), 0);
    EXPECT_EQ(get_el_type(0), 0);
    EXPECT_EQ(get_el_type(1), 0);
    EXPECT_EQ(get_el_type(2), 0);
    EXPECT_EQ(get_el_type(3), 1);
    EXPECT_EQ(get_el_type(4), 1);
    EXPECT_EQ(get_el_type(5), 0);
    EXPECT_EQ(get_el_type(6), 0);
    EXPECT_EQ(get_el_type(7), 0);
    EXPECT_EQ(get_el_type(8), 0);
    EXPECT_EQ(get_el_type(9), 0);
    EXPECT_EQ(get_el_type(10), 0);
    EXPECT_EQ(get_el_type(11), 1);

    EXPECT_EQ(get_el_type(25), 3);

    EXPECT_EQ(get_el_type(57), 1);

    // EXPECT_EQ(get_el_type(23423423), 0); --> seg fault
}

TEST(test_util, test_is_el_a_metal)
{
    EXPECT_EQ(is_el_a_metal(-1), false);
    EXPECT_EQ(is_el_a_metal(0), false);
    EXPECT_EQ(is_el_a_metal(1), false);
    EXPECT_EQ(is_el_a_metal(2), false);
    EXPECT_EQ(is_el_a_metal(3), true);
    EXPECT_EQ(is_el_a_metal(4), true);
    EXPECT_EQ(is_el_a_metal(5), false);
    EXPECT_EQ(is_el_a_metal(6), false);
    EXPECT_EQ(is_el_a_metal(7), false);
    EXPECT_EQ(is_el_a_metal(8), false);
    EXPECT_EQ(is_el_a_metal(9), false);
    EXPECT_EQ(is_el_a_metal(10), false);
    EXPECT_EQ(is_el_a_metal(11), true);

    EXPECT_EQ(is_el_a_metal(57), true);

    // EXPECT_EQ(is_el_a_metal(23423423), false); // --> seg fault
}

// ---------------------------

// lrtrim
TEST(test_util, test_lrtrim)
{

    char test_string1[7] = " InChI";
    int nLen1 = 0;
    EXPECT_STREQ(lrtrim(test_string1, &nLen1), "InChI");
    EXPECT_EQ(nLen1, 5);

    char test_string2[9] = " InChI  ";
    int nLen2 = 0;
    EXPECT_STREQ(lrtrim(test_string2, &nLen2), "InChI");
    EXPECT_EQ(nLen2, 5);

    char test_string3[6] = "InChI";
    int nLen3 = 0;
    EXPECT_STREQ(lrtrim(test_string3, &nLen3), "InChI");
    EXPECT_EQ(nLen3, 5);
}

// ---------------------------

// extract_H_atoms( char *elname, S_CHAR num_iso_H[] )
TEST(test_util, test_extract_H_atoms)
{
    S_CHAR num_iso_H[NUM_H_ISOTOPES] = {0};

    char elname[4] = "C";

    EXPECT_EQ(extract_H_atoms(elname, num_iso_H), 0);
    EXPECT_STREQ(elname, "C");

    strcpy(elname, "H");
    EXPECT_EQ(extract_H_atoms(elname, num_iso_H), 1);
    EXPECT_STREQ(elname, "");

    EXPECT_EQ(num_iso_H[0], '\0'); // H

    strcpy(elname, "Rh");
    EXPECT_EQ(extract_H_atoms(elname, num_iso_H), 0);

    strcpy(elname, "D");
    EXPECT_EQ(extract_H_atoms(elname, num_iso_H), 0);
    EXPECT_STREQ(elname, "");

    EXPECT_EQ(num_iso_H[0], '\0'); // D

    strcpy(elname, "???");
    EXPECT_EQ(extract_H_atoms(elname, num_iso_H), 0);

    strcpy(elname, "T");
    EXPECT_EQ(extract_H_atoms(elname, num_iso_H), 0);
    EXPECT_STREQ(elname, "");

    EXPECT_EQ(num_iso_H[0], '\0');  // T
    EXPECT_EQ(num_iso_H[1], '\x1'); // T

    strcpy(elname, "");
    EXPECT_EQ(extract_H_atoms(elname, num_iso_H), 0);
}

TEST(test_util, test_get_num_H)
{
    // const char *elname, int inp_num_H, S_CHAR *num_iso_H, int charge, int radical,
    // int chem_bonds_valence, int atom_input_valence, int bAliased, int bDoNotAddH, int bHasMetalNeighbor

    S_CHAR num_iso_H[NUM_H_ISOTOPES] = {0};

    EXPECT_EQ(get_num_H("C", 0, num_iso_H, 0, 0, 0, 0, 0, 0, 0), 4);
}

TEST(test_util, test_get_unusual_el_valence)
{
    EXPECT_EQ(get_unusual_el_valence(0, 0, 0, 0, 0, 0), 0);
    EXPECT_EQ(get_unusual_el_valence(6, 0, 0, 4, 0, 0), 0);
    EXPECT_EQ(get_unusual_el_valence(6, 1, 0, 3, 1, 1), 4);
    EXPECT_EQ(get_unusual_el_valence(6, -1, 1, 3, 2, 2), 5);
    EXPECT_EQ(get_unusual_el_valence(17, -1, 0, 7, 2, 2), 9);
    EXPECT_EQ(get_unusual_el_valence(82, -2, 2, 6, 3, 3), 9);
}

TEST(test_util, test_detect_unusual_el_valence)
{
    // nPeriodicNum, charge, radical, bonds_valence, num_H, num_bonds
    EXPECT_EQ(detect_unusual_el_valence(0, 0, 0, 0, 0, 0), 0);
    EXPECT_EQ(detect_unusual_el_valence(6, 0, 0, 4, 0, 0), 0);
    EXPECT_EQ(detect_unusual_el_valence(6, 1, 0, 3, 1, 1), 4);
    EXPECT_EQ(detect_unusual_el_valence(6, -1, 1, 3, 2, 2), 5);
    EXPECT_EQ(detect_unusual_el_valence(17, -1, 0, 7, 2, 2), 9);
    EXPECT_EQ(detect_unusual_el_valence(82, -2, 2, 6, 3, 3), 9);
}

TEST(test_util, test_normalize_string)
{
    char test_string1[7] = " InChI";
    EXPECT_EQ(normalize_string(test_string1), 5);
    EXPECT_STREQ(test_string1, "InChI");

    char test_string2[9] = " InChI  ";
    EXPECT_EQ(normalize_string(test_string2), 5);
    EXPECT_STREQ(test_string2, "InChI");

    char test_string3[6] = "     ";
    EXPECT_EQ(normalize_string(test_string3), 0);
    EXPECT_STREQ(test_string3, "");

    char test_string4[20] = "  sdfds   sfsfd ew ";
    EXPECT_EQ(normalize_string(test_string4), 14);
    EXPECT_STREQ(test_string4, "sdfds sfsfd ew");
}

TEST(test_util, test_read_upto_delim)
{
    // read_upto_delim(char **pstring, char *field, int maxlen, char *delims)

    char test_string1[] = "field1,field2,field3";
    char *test_string1_inp = test_string1;
    char field[20];
    int maxlen = 10;
    char delims[] = ",";

    EXPECT_EQ(read_upto_delim(&test_string1_inp, field, maxlen, delims), 6);
    EXPECT_STREQ(field, "field1");
    EXPECT_STREQ(test_string1_inp, ",field2,field3");

    char test_string2[] = "";
    char *test_string2_inp = test_string2;

    EXPECT_EQ(read_upto_delim(&test_string2_inp, field, maxlen, delims), 0);
    EXPECT_STREQ(field, "");
    EXPECT_STREQ(test_string2_inp, NULL);

    char test_string3[] = "afdfs,fdsfff323sd";
    char *test_string3_inp = test_string3;

    EXPECT_EQ(read_upto_delim(&test_string3_inp, field, maxlen, delims), 5);
    EXPECT_STREQ(field, "afdfs");
    EXPECT_STREQ(test_string3_inp, ",fdsfff323sd");

    char test_string4[] = "afdfsfdsfff323s";
    char *test_string4_inp = test_string4;

    EXPECT_EQ(read_upto_delim(&test_string4_inp, field, maxlen, delims), -1);
    // EXPECT_STREQ(field, "");
    EXPECT_STREQ(test_string4_inp, "afdfsfdsfff323s");

    char test_string5[] = "afdfsfdsfff323s";
    maxlen = 50;
    char *test_string5_inp = test_string5;

    EXPECT_EQ(read_upto_delim(&test_string5_inp, field, maxlen, delims), 15);
    EXPECT_STREQ(field, "afdfsfdsfff323s");
    EXPECT_STREQ(test_string5_inp, NULL);

    char test_string6[] = "field1,field2,field3";
    maxlen = 3;
    char *test_string6_inp = test_string6;

    EXPECT_EQ(read_upto_delim(&test_string6_inp, field, maxlen, delims), -1);
    // EXPECT_STREQ(field, "fie");
    EXPECT_STREQ(test_string6_inp, "field1,field2,field3");
}

TEST(test_util, test_is_matching_any_delim)
{
    char delims1[] = "abc";

    char delims2[] = "";

    // is_matching_any_delim(char c, char *delims)
    EXPECT_TRUE(is_matching_any_delim('a', delims1));
    EXPECT_TRUE(is_matching_any_delim('b', delims1));
    EXPECT_TRUE(is_matching_any_delim('c', delims1));
    EXPECT_FALSE(is_matching_any_delim('d', delims1));
    EXPECT_FALSE(is_matching_any_delim(' ', delims1));

    EXPECT_FALSE(is_matching_any_delim(' ', delims2));
    EXPECT_FALSE(is_matching_any_delim('a', delims2));
}

TEST(test_util, test_dotify_non_printable_chars)
{
    // dotify_non_printable_chars(char *line)
    char test_string1[] = "Hello\x01World";
    EXPECT_EQ(dotify_non_printable_chars(test_string1), 1);
    EXPECT_STREQ(test_string1, "Hello.World");

    char test_string2[] = "\x02Test";
    EXPECT_EQ(dotify_non_printable_chars(test_string2), 1);
    EXPECT_STREQ(test_string2, ".Test");

    char test_string3[] = "NoChange";
    EXPECT_EQ(dotify_non_printable_chars(test_string3), 0);
    EXPECT_STREQ(test_string3, "NoChange");
}

TEST(test_util, test_remove_trailing_spaces)
{
    // remove_trailing_spaces(char *p)
    char test_string1[] = "Hello World   ";
    remove_trailing_spaces(test_string1);
    EXPECT_STREQ(test_string1, "Hello World");

    char test_string3[] = "NoTrailingSpaces";
    remove_trailing_spaces(test_string3);
    EXPECT_STREQ(test_string3, "NoTrailingSpaces");
}

TEST(test_util, test_remove_one_lf)
{
    // remove_one_lf(char *p)
    char test_string1[] = "Hello World\n";
    remove_one_lf(test_string1);
    EXPECT_STREQ(test_string1, "Hello World");

    char test_string2[] = "Hello World\r\n";
    remove_one_lf(test_string2);
    EXPECT_STREQ(test_string2, "Hello World");

    char test_string3[] = "NoLineFeed";
    remove_one_lf(test_string3);
    EXPECT_STREQ(test_string3, "NoLineFeed");

    char test_string4[] = "Hello World\r\n\n";
    remove_one_lf(test_string4);
    EXPECT_STREQ(test_string4, "Hello World\r\n");

    char test_string5[] = "";
    remove_one_lf(test_string5);
    EXPECT_STREQ(test_string5, "");

    char test_string6[] = "\r\n";
    remove_one_lf(test_string6);
    EXPECT_STREQ(test_string6, "");
}

TEST(test_util, test_mystrncpy)
{
    // mystrncpy(char *target, const char *source, unsigned maxlen)
    char target[20];
    const char *source = "Hello World";

    EXPECT_EQ(mystrncpy(target, source, 5), 1);
    EXPECT_STREQ(target, "Hell");

    EXPECT_EQ(mystrncpy(target, source, 20), 1);
    EXPECT_STREQ(target, "Hello World");

    EXPECT_EQ(mystrncpy(target, NULL, 10), 0);
    EXPECT_STREQ(target, "Hello World"); // should not change

    EXPECT_EQ(mystrncpy(NULL, source, 10), 0); // target is NULL
}

TEST(test_util, test_inchi_memicmp)
{

    // inchi_memicmp(const void *p1, const void *p2, size_t length)

    const char *str1 = "Hello";
    const char *str2 = "hello";
    EXPECT_EQ(inchi_memicmp(str1, str2, 5), 0);
    EXPECT_EQ(inchi_memicmp(str1, str2, 4), 0);

    const char *str7 = "a";
    const char *str8 = "s";

    EXPECT_TRUE(inchi_memicmp(str7, str8, 1) < 0);

    const char *str3 = "Hello";
    const char *str4 = "hellosdfds";

    EXPECT_TRUE(inchi_memicmp(str3, str4, 9) < 0);

    const char *str5 = "131234";
    const char *str6 = "hellosdfds";

    EXPECT_TRUE(inchi_memicmp(str5, str6, 20) < 0);

    const char *str9 = "XYZdqwdsa";
    const char *str10 = "qwd";

    EXPECT_TRUE(inchi_memicmp(str9, str10, 20) > 0);
}

TEST(test_util, test_inchi_stricmp)
{

    // inchi_stricmp(const char *s1, const char *s2)
    EXPECT_EQ(inchi_stricmp("Hello", "hello"), 0);
    EXPECT_TRUE(inchi_stricmp("Hello", "Hell") > 0);
    EXPECT_TRUE(inchi_stricmp("Hello", "HelloWorld") < 0);
    EXPECT_TRUE(inchi_stricmp("Hello", "Hello") == 0);
    EXPECT_TRUE(inchi_stricmp("Hello", "HELLO") == 0);
    EXPECT_TRUE(inchi_stricmp("", "") == 0);
    EXPECT_TRUE(inchi_stricmp("123", "12") > 0);
    EXPECT_TRUE(inchi_stricmp("123", "12455454") < 0);
}

TEST(test_util, test_inchi_strnset)
{

    // char *inchi__strnset(char *s, int val, size_t length)
    char test_string1[] = "Hello";
    EXPECT_STREQ(inchi__strnset(test_string1, 'X', 3), "XXXlo");

    char test_string2[] = "World";
    EXPECT_STREQ(inchi__strnset(test_string2, 'Y', 5), "YYYYY");

    char test_string3[] = "NoChange";
    EXPECT_STREQ(inchi__strnset(test_string3, 'Z', 0), "NoChange");
}

TEST(test_util, test_inchi_strdup)
{

    // char *inchi__strdup(const char *string)
    char *dup1 = inchi__strdup("Hello");
    EXPECT_STREQ(dup1, "Hello");
    free(dup1);

    char *dup2 = inchi__strdup("");
    EXPECT_STREQ(dup2, "");
    free(dup2);
}

TEST(test_util, test_inchi_strtol)
{

    // long inchi_strtol(const char *str, const char **p, int base)
    // base is for numerical type: 10 -> decimal, 16 -> hexadecimal, 27 -> compressed inchi, 0 -> auto detect
    long result1 = inchi_strtol("12345", NULL, 0);
    EXPECT_EQ(result1, 12345);

    // TODO: what is a compressed inchi? -> base 27
    long result2 = inchi_strtol("12345", NULL, 27);
    EXPECT_EQ(result2, 0);
}

TEST(test_util, test_inchi_strtod)
{

    // double inchi_strtod(const char *str, const char **p)
    const char *endptr;
    double result = inchi_strtod("123.45", &endptr);
    EXPECT_EQ(result, 123.45);
    EXPECT_EQ(*endptr, '\0');

    result = inchi_strtod("abc", &endptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(*endptr, 'a');
}

TEST(test_util, test_is_in_the_list)
{

    // AT_NUMB *is_in_the_list(AT_NUMB *pathAtom, AT_NUMB nNextAtom, int nPathLen)
    AT_NUMB pathAtom1[] = {1, 2, 3, 4, 5};
    AT_NUMB nNextAtom1 = 3;
    int nPathLen1 = 5;
    EXPECT_TRUE(is_in_the_list(pathAtom1, nNextAtom1, nPathLen1));

    AT_NUMB pathAtom2[] = {1, 2, 3, 4, 5};
    AT_NUMB nNextAtom2 = 6;
    int nPathLen2 = 5;
    EXPECT_FALSE(is_in_the_list(pathAtom2, nNextAtom2, nPathLen2));

    AT_NUMB nNextAtom3 = 6;
    int nPathLen3 = 0;
    EXPECT_FALSE(is_in_the_list(nullptr, nNextAtom3, nPathLen3));
}

TEST(test_util, test_is_in_the_ilist)
{

    // int *is_in_the_ilist(int *pathAtom, int nNextAtom, int nPathLen)

    int pathAtom1[] = {1, 2, 3, 4, 5};
    int nNextAtom1 = 3;
    int nPathLen1 = 5;
    EXPECT_TRUE(is_in_the_ilist(pathAtom1, nNextAtom1, nPathLen1));

    int pathAtom2[] = {1, 2, 3, 4, 5};
    int nNextAtom2 = 6;
    int nPathLen2 = 5;
    EXPECT_FALSE(is_in_the_ilist(pathAtom2, nNextAtom2, nPathLen2));

    int nNextAtom3 = -6;
    int nPathLen3 = 0;
    EXPECT_FALSE(is_in_the_ilist(nullptr, nNextAtom3, nPathLen3));
}

TEST(test_util, test_is_ilist_inside)
{

    // int is_ilist_inside(int *ilist, int nlist, int *ilist2, int nlist2)

    int pathAtom1[] = {1, 2, 3, 4, 5};
    int pathAtom2[] = {2, 3};
    EXPECT_TRUE(is_ilist_inside(pathAtom2, 2, pathAtom1, 5));

    int pathAtom3[] = {6, 7};
    EXPECT_FALSE(is_ilist_inside(pathAtom3, 2, pathAtom1, 5));

    EXPECT_FALSE(is_ilist_inside(pathAtom2, 2, pathAtom3, 2));

    EXPECT_TRUE(is_ilist_inside(nullptr, 0, nullptr, 0));
}

TEST(test_util, test_nBondsValToMetal)
{

    // int nBondsValToMetal(inp_ATOM *at, int iat)

    // EXPECT_EQ(nBondsValToMetal(NULL, 0), 0);  SEG FAULT

    inp_ATOM *atoms1 = CreateInpAtom(2);
    atoms1[0].el_number = get_periodic_table_number("C");  // Carbon 6 valence 4
    atoms1[1].el_number = get_periodic_table_number("Fe"); // Iron 26 valence 8

    atoms1[0].valence = static_cast<S_CHAR>(get_el_valence(atoms1[0].el_number, atoms1[0].charge, 0));
    atoms1[1].valence = static_cast<S_CHAR>(get_el_valence(atoms1[1].el_number, atoms1[1].charge, 0));

    EXPECT_EQ(atoms1[0].valence, 4);
    EXPECT_EQ(atoms1[1].valence, 2);

    atoms1[0].neighbor[0] = 1;
    atoms1[1].neighbor[0] = 0;

    atoms1[0].bond_type[0] = 1;
    atoms1[1].bond_type[0] = 1;

    EXPECT_EQ(nBondsValToMetal(atoms1, 0), 1);
    EXPECT_EQ(nBondsValToMetal(atoms1, 1), 0);

    inp_ATOM *atoms2 = CreateInpAtom(3);
    atoms2[0].el_number = get_periodic_table_number("C");  // Carbon 6 valence 4
    atoms2[1].el_number = get_periodic_table_number("Fe"); // Iron 26 valence 8
    atoms2[2].el_number = get_periodic_table_number("Fe"); // Iron 26 valence 8

    atoms2[0].valence = static_cast<S_CHAR>(get_el_valence(atoms2[0].el_number, atoms2[0].charge, 0));
    atoms2[1].valence = static_cast<S_CHAR>(get_el_valence(atoms2[1].el_number, atoms2[1].charge, 0));
    atoms2[2].valence = static_cast<S_CHAR>(get_el_valence(atoms2[2].el_number, atoms2[2].charge, 0));

    EXPECT_EQ(atoms2[0].valence, 4);
    EXPECT_EQ(atoms2[1].valence, 2);
    EXPECT_EQ(atoms2[2].valence, 2);

    atoms2[0].neighbor[0] = 1;
    atoms2[0].neighbor[1] = 2;
    atoms2[1].neighbor[0] = 0;
    atoms2[2].neighbor[0] = 0;

    atoms2[0].bond_type[0] = 1;
    atoms2[0].bond_type[1] = 1; // dont forget to set bond types for each atom
    atoms2[1].bond_type[0] = 1;
    atoms2[2].bond_type[0] = 1;

    EXPECT_EQ(nBondsValToMetal(atoms2, 0), 2);
    EXPECT_EQ(nBondsValToMetal(atoms2, 1), 0);
    EXPECT_EQ(nBondsValToMetal(atoms2, 2), 0);

    FreeInpAtom(&atoms1);
    FreeInpAtom(&atoms2);
}

TEST(test_util, test_num_of_H)
{

    // int num_of_H(inp_ATOM *at, int iat)

    inp_ATOM *atoms1 = CreateInpAtom(5);
    atoms1[0].el_number = get_periodic_table_number("C");
    atoms1[1].el_number = get_periodic_table_number("H");
    atoms1[2].el_number = get_periodic_table_number("H");
    atoms1[3].el_number = get_periodic_table_number("H");
    atoms1[4].el_number = get_periodic_table_number("H");

    atoms1[0].valence = static_cast<S_CHAR>(get_el_valence(atoms1[0].el_number, atoms1[0].charge, 0));
    atoms1[1].valence = static_cast<S_CHAR>(get_el_valence(atoms1[1].el_number, atoms1[1].charge, 0));
    atoms1[2].valence = static_cast<S_CHAR>(get_el_valence(atoms1[2].el_number, atoms1[2].charge, 0));
    atoms1[3].valence = static_cast<S_CHAR>(get_el_valence(atoms1[3].el_number, atoms1[3].charge, 0));
    atoms1[4].valence = static_cast<S_CHAR>(get_el_valence(atoms1[4].el_number, atoms1[4].charge, 0));

    atoms1[0].neighbor[0] = 1;
    atoms1[0].neighbor[1] = 2;
    atoms1[0].neighbor[2] = 3;
    atoms1[0].neighbor[3] = 4;

    atoms1[1].neighbor[0] = 0;
    atoms1[2].neighbor[0] = 0;
    atoms1[3].neighbor[0] = 0;
    atoms1[4].neighbor[0] = 0;

    EXPECT_EQ(num_of_H(atoms1, 0), 4);

    FreeInpAtom(&atoms1);

    inp_ATOM *atoms2 = CreateInpAtom(8);

    atoms2[0].el_number = get_periodic_table_number("C");
    atoms2[1].el_number = get_periodic_table_number("H");
    atoms2[2].el_number = get_periodic_table_number("H");
    atoms2[3].el_number = get_periodic_table_number("H");

    atoms2[4].el_number = get_periodic_table_number("C");
    atoms2[5].el_number = get_periodic_table_number("H");
    atoms2[6].el_number = get_periodic_table_number("H");
    atoms2[7].el_number = get_periodic_table_number("H");

    atoms2[0].valence = static_cast<S_CHAR>(get_el_valence(atoms2[0].el_number, atoms2[0].charge, 0));
    atoms2[1].valence = static_cast<S_CHAR>(get_el_valence(atoms2[1].el_number, atoms2[1].charge, 0));
    atoms2[2].valence = static_cast<S_CHAR>(get_el_valence(atoms2[2].el_number, atoms2[2].charge, 0));
    atoms2[3].valence = static_cast<S_CHAR>(get_el_valence(atoms2[3].el_number, atoms2[3].charge, 0));
    atoms2[4].valence = static_cast<S_CHAR>(get_el_valence(atoms2[4].el_number, atoms2[4].charge, 0));
    atoms2[5].valence = static_cast<S_CHAR>(get_el_valence(atoms2[5].el_number, atoms2[5].charge, 0));
    atoms2[6].valence = static_cast<S_CHAR>(get_el_valence(atoms2[6].el_number, atoms2[6].charge, 0));
    atoms2[7].valence = static_cast<S_CHAR>(get_el_valence(atoms2[7].el_number, atoms2[7].charge, 0));

    atoms2[0].neighbor[0] = 1;
    atoms2[0].neighbor[1] = 2;
    atoms2[0].neighbor[2] = 3;
    atoms2[0].neighbor[3] = 4;

    atoms2[1].neighbor[0] = 0;
    atoms2[2].neighbor[0] = 0;
    atoms2[3].neighbor[0] = 0;

    atoms2[4].neighbor[0] = 0;
    atoms2[4].neighbor[1] = 5;
    atoms2[4].neighbor[2] = 6;
    atoms2[4].neighbor[3] = 7;

    atoms2[5].neighbor[0] = 4;
    atoms2[6].neighbor[0] = 4;
    atoms2[7].neighbor[0] = 4;

    EXPECT_EQ(num_of_H(atoms2, 0), 3);

    EXPECT_EQ(num_of_H(atoms2, 4), 3);

    FreeInpAtom(&atoms2);
}

TEST(test_util, test_ion_el_group)
{

    // U_CHAR ion_el_group(int el)
    EXPECT_EQ(ion_el_group(get_periodic_table_number("C")), EL_NUMBER_C);
    EXPECT_EQ(ion_el_group(get_periodic_table_number("O")), EL_NUMBER_O);

    EXPECT_EQ(ion_el_group(get_periodic_table_number("Mn")), 0);
}

TEST(test_util, test_has_other_ion_neigh)
{

    // int has_other_ion_neigh(inp_ATOM *at, int iat, int iat_ion_neigh)
    inp_ATOM *atoms1 = CreateInpAtom(2);

    atoms1[0].el_number = get_periodic_table_number("C");
    atoms1[1].el_number = get_periodic_table_number("O");

    atoms1[0].valence = static_cast<S_CHAR>(get_el_valence(atoms1[0].el_number, atoms1[0].charge, 0));
    atoms1[1].valence = static_cast<S_CHAR>(get_el_valence(atoms1[1].el_number, atoms1[1].charge, 0));

    atoms1[0].neighbor[0] = 1;
    atoms1[1].neighbor[0] = 0;

    EXPECT_EQ(has_other_ion_neigh(atoms1, 0, 1), 1);
    EXPECT_EQ(has_other_ion_neigh(atoms1, 1, 0), 0); // ??

    FreeInpAtom(&atoms1);
}

TEST(test_util, test_extract_charges_and_radicals)
{

    // int extract_charges_and_radicals(char *elname, int *pnRadical, int *pnCharge)

    int nRadical = 0;
    int nCharge = 0;

    char el_name[10] = "";
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 0);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, "C");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 0);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, ":CH2");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 0);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, "Fe+2");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 2);

    strcpy(el_name, "Cl+2");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 2);

    strcpy(el_name, "Cl2+");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 1);

    strcpy(el_name, "Cl++");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 2);

    strcpy(el_name, "Cl+");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 0);
    EXPECT_EQ(nCharge, 1);

    strcpy(el_name, "Cl+.");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 2);
    EXPECT_EQ(nCharge, 1);

    strcpy(el_name, "Cl:");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 1);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, "Cl::");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 1);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, "Cl^");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 2);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, "Cl.");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 2);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, "Cl^^");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 3);
    EXPECT_EQ(nCharge, 0);

    strcpy(el_name, "C^^");
    EXPECT_EQ(extract_charges_and_radicals(el_name, &nRadical, &nCharge), 1);
    EXPECT_EQ(nRadical, 3);
    EXPECT_EQ(nCharge, 0);
}

TEST(test_util, test_extract_inchi_substring)
{

    // void extract_inchi_substring(char **buf, const char *str, size_t slen)

    //        For example:
    //        "InChI=1/Ar%"
    //        "InChI=1/Ar\n"
    //        "InChI=1/Ar\r\t"
    //    all will be trimmed to
    //        "InChI=1/Ar"

    char *buf1 = NULL;
    const char *str_inchi1 = "InChI=1/Ar%";
    size_t slen1 = strlen(str_inchi1);

    extract_inchi_substring(&buf1, str_inchi1, slen1);

    EXPECT_NE(buf1, nullptr);
    EXPECT_STREQ(buf1, "InChI=1/Ar");
    free(buf1);

    char *buf2 = NULL;
    const char *str_inchi2 = "InChI=1/Ar\r\t";
    size_t slen2 = strlen(str_inchi2);

    extract_inchi_substring(&buf2, str_inchi2, slen2);

    EXPECT_NE(buf2, nullptr);
    EXPECT_STREQ(buf2, "InChI=1/Ar");
    free(buf2);
}

TEST(test_util, test_extract_auxinfo_substring)
{
    // void extract_auxinfo_substring(char **buf, const char *str, size_t slen)

    char *buf1 = NULL;
    const char *str_auxinfo1 = "AuxInfo=1/0/N:1/rA:1nC/rB:/rC:3.025,-3.725,0;       ";
    size_t slen1 = strlen(str_auxinfo1);

    extract_auxinfo_substring(&buf1, str_auxinfo1, slen1);

    EXPECT_NE(buf1, nullptr);
    EXPECT_STREQ(buf1, "AuxInfo=1/0/N:1/rA:1nC/rB:/rC:3.025,-3.725,0;");
    free(buf1);

    char *buf2 = NULL;
    const char *str_auxinfo2 = "AuxInfo=1/0/N:1/rA:1nC/rB:/rC:3.025,-3.725,0;\r\t";
    size_t slen2 = strlen(str_auxinfo2);

    extract_auxinfo_substring(&buf2, str_auxinfo2, slen2);

    EXPECT_NE(buf2, nullptr);
    EXPECT_STREQ(buf2, "AuxInfo=1/0/N:1/rA:1nC/rB:/rC:3.025,-3.725,0;");
    free(buf2);

    char *buf3 = NULL;
    const char *str_auxinfo3 =
        "AuxInfo=1/0/N:1,3,5,2,6,4/E:(1,2,3,4,5,6)/rA:6nCCCCCC/rB:;d1s2;d2;s1;s4d5;/rC:3.2848,-6.7251,0;5.0152,-6.7246,0;4.1516,-6.225,0;5.0152,-7.7255,0;3.2848,-7.73,0;4.1538,-8.225,0;\r\t";
    size_t slen3 = strlen(str_auxinfo3);

    extract_auxinfo_substring(&buf3, str_auxinfo3, slen3);

    EXPECT_NE(buf3, nullptr);
    EXPECT_STREQ(buf3,
                 "AuxInfo=1/0/N:1,3,5,2,6,4/E:(1,2,3,4,5,6)/rA:6nCCCCCC/rB:;d1s2;d2;s1;s4d5;/rC:3.2848,-6.7251,0;5.0152,-6.7246,0;4.1516,-6.225,0;5.0152,-7.7255,0;3.2848,-7.73,0;4.1538,-8.225,0;");
    free(buf3);

    char *buf4 = NULL;
    const char *str_auxinfo4 = "Aunfo=1/0/N:1/rA:1nC/rB:/rC:3.025,-3.725,0;";
    size_t slen4 = strlen(str_auxinfo4);

    extract_auxinfo_substring(&buf4, str_auxinfo4, slen4);

    EXPECT_EQ(buf4, nullptr);
    free(buf4);
}

TEST(test_util, test_extract_stereo_info_from_inchi_string)
{
    // int extract_stereo_info_from_inchi_string(char *sinchi, int nat, int *orig, int *at_stereo_mark);

    char inchi1[] = "InChI=1S/C2H5BrClN/c1-2(3,4)5/h5H2,1H3/t2-/m1/s1";

    int num_atoms1 = 4;
    int orig1[4] = {0, 1, 2, 3};
    int at_stereo_mark_orig1[MAX_ATOMS]; // = {0, 0, 0, 0}

    int ret1 = extract_stereo_info_from_inchi_string(inchi1, num_atoms1, orig1, at_stereo_mark_orig1);
    EXPECT_EQ(ret1, 0);
    int found_stereo1 = 0;
    for (int i = 1; i <= num_atoms1; i++)
    {
        if (at_stereo_mark_orig1[i] != 0)
        {
            found_stereo1 = 1;
            break;
        }
    }
    EXPECT_EQ(at_stereo_mark_orig1[0], 0);
    EXPECT_EQ(at_stereo_mark_orig1[1], 0);
    EXPECT_EQ(at_stereo_mark_orig1[2], 1);
    EXPECT_EQ(at_stereo_mark_orig1[3], 0);

    EXPECT_EQ(found_stereo1, 1);

    char inchi2[] = "InChI=1S/C2H6/c1-2/h1-2H3";

    int num_atoms2 = 2;
    int orig2[2] = {0, 1};
    int at_stereo_mark_orig2[MAX_ATOMS]; //[2] = {0, 0};

    int ret2 = extract_stereo_info_from_inchi_string(inchi2, num_atoms2, orig2, at_stereo_mark_orig2);
    EXPECT_EQ(ret2, 0);
    int found_stereo2 = 0;
    for (int i = 1; i <= num_atoms2; i++)
    {
        if (at_stereo_mark_orig2[i] != 0)
        {
            found_stereo2 = 1;
            break;
        }
    }
    EXPECT_EQ(at_stereo_mark_orig2[0], 0);
    EXPECT_EQ(at_stereo_mark_orig2[1], 0);

    EXPECT_EQ(found_stereo2, 0);
}
