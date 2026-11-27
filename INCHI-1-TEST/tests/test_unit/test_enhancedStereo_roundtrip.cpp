#include <gtest/gtest.h>

#include <cstring>
#include <string>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

// Enhanced stereo (/s groups) through InChI -> InChI and
// InChI -> structure -> InChI.

static const char k_opts[] = "-EnhancedStereochemistry";

/* InChI -> InChI (GetINCHIfromINCHI, same as -InChI2InChI) */
static std::string inchi2inchi(const char *inchi)
{
    inchi_InputINCHI in;
    inchi_Output out;
    char opts[sizeof(k_opts)];

    memcpy(opts, k_opts, sizeof(k_opts));
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.szInChI = (char *) inchi;
    in.szOptions = opts;

    GetINCHIfromINCHI(&in, &out);
    std::string s = out.szInChI ? out.szInChI : "";
    FreeINCHI(&out);

    return s;
}

// butane-2,3-diol, one OR group over both centres
TEST(test_enhancedStereo_roundtrip, inchi2inchi_single_digit)
{
    const char inchi[] = "InChI=1B/C4H10O2/c1-3(5)4(2)6/h3-6H,1-2H3/t3-,4-/s2";

    EXPECT_EQ(inchi2inchi(inchi), inchi);
}

// Grouped, multi-component and placeholder forms of /s and /m (spec 1, §6)
class Inchi2InchiTest : public ::testing::TestWithParam<const char *>
{
};

TEST_P(Inchi2InchiTest, reprints_unchanged)
{
    EXPECT_EQ(inchi2inchi(GetParam()), GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    test_enhancedStereo_roundtrip, Inchi2InchiTest,
    ::testing::Values(
        /* one component: ABS + AND, two OR groups, OR next to an ungrouped wedge */
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/m1/s1(4)3(5)",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s2(4)(5)",
        "InChI=1B/C6H13ClO/c1-5(7)3-4-6(2)8/h5-6,8H,3-4H2,1-2H3/t5-,6-/m0/s2(6)",
        /* several components: bare digits per slot, '.' in /m, n* repetition */
        "InChI=1B/2CBrClFI/c2*2-1(3,4)5/t2*1-/s2;3",
        "InChI=1B/C4H8Br2.C4H8Cl2/c2*1-3(5)4(2)6/h2*3-4H,1-2H3/t2*3-,4-/m.1/s2;1",
        "InChI=1B/2C4H8BrClO/c2*1-3(5)4(6)2-7/h2*3-4,7H,2H2,1H3/t2*3-,4-/s2*3(3)(4)",
        "InChI=1B/2C10H14BrCl7/c2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h2*3-10H,1-2H3/t2*3-,4-,5+,6-,7-,8-,9+,10-/m00/s2*1(3,5)2(4)(6,8)3(7,9)(10)",
        "InChI=1B/C12H26.2C10H14BrCl7.C9H20/c1-8(2)11(7)12(9(3)4)10(5)6;2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12;1-6-8(4)9(5)7(2)3/h8-12H,1-7H3;2*3-10H,1-2H3;7-9H,6H2,1-5H3/t11-;2*3-,4-,5+,6-,7-,8-,9+,10-;8-,9+/m100./s1;2*1(3,5)2(4)(6,8)3(7,9)(10);3",
        /* isotopic layer */
        "InChI=1B/C2H6O/c1-2-3/h3H,2H2,1H3/i2D/t2-/s3"));

// Malformed /s is a syntax error, not silently reshaped
class Inchi2InchiRejectTest : public ::testing::TestWithParam<const char *>
{
};

TEST_P(Inchi2InchiRejectTest, is_rejected)
{
    EXPECT_EQ(inchi2inchi(GetParam()), "");
}

INSTANTIATE_TEST_SUITE_P(
    test_enhancedStereo_roundtrip, Inchi2InchiRejectTest,
    ::testing::Values(
        /* atoms not ascending */
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s2(5,4)",
        /* atom beyond the component */
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s2(4)(9)",
        /* class digit out of range, group without class */
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s4(4)",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s(4)(5)",
        /* more slots than components */
        "InChI=1B/2CBrClFI/c2*2-1(3,4)5/t2*1-/s2;3;1"));

/* InChI -> structure (Ex, with V3000 stereo collections) -> InChI */
static std::string round_trip(const char *inchi)
{
    inchi_InputINCHI ii;
    inchi_OutputStructEx os;
    inchi_InputEx in;
    inchi_Output out;
    char opts[sizeof(k_opts)];
    char opts2[sizeof(k_opts)];

    memcpy(opts, k_opts, sizeof(k_opts));
    memcpy(opts2, k_opts, sizeof(k_opts));
    memset(&ii, 0, sizeof(ii));
    memset(&os, 0, sizeof(os));
    ii.szInChI = (char *) inchi;
    ii.szOptions = opts;
    GetStructFromINCHIEx(&ii, &os);

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.atom = os.atom;
    in.num_atoms = os.num_atoms;
    in.stereo0D = os.stereo0D;
    in.num_stereo0D = os.num_stereo0D;
    in.v3000 = os.v3000;
    in.szOptions = opts2;
    GetINCHIEx(&in, &out);

    std::string s = out.szInChI ? out.szInChI : "";
    FreeINCHI(&out);
    FreeStructFromINCHIEx(&os);

    return s;
}

class RoundTripTest : public ::testing::TestWithParam<const char *>
{
};

TEST_P(RoundTripTest, structure_keeps_groups)
{
    EXPECT_EQ(round_trip(GetParam()), GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    test_enhancedStereo_roundtrip, RoundTripTest,
    ::testing::Values(
        /* bare digits: one OR / one AND group over all centres */
        "InChI=1B/C4H10O2/c1-3(5)4(2)6/h3-6H,1-2H3/t3-,4-/s2",
        "InChI=1B/C10H15NO/c1-8(11-2)10(12)9-6-4-3-5-7-9/h3-8,10-12H,1-2H3/t8-,10-/s2",
        /* groups inside one component */
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/m1/s1(4)3(5)",
        "InChI=1B/C5H13NO/c1-3-5(6)4(2)7/h4-5,7H,3,6H2,1-2H3/t4-,5-/s2(4)(5)",
        "InChI=1B/C6H13ClO/c1-5(7)3-4-6(2)8/h5-6,8H,3-4H2,1-2H3/t5-,6-/m0/s2(6)",
        /* several components */
        "InChI=1B/2CBrClFI/c2*2-1(3,4)5/t2*1-/s2;3",
        "InChI=1B/C4H8Br2.C4H8Cl2/c2*1-3(5)4(2)6/h2*3-4H,1-2H3/t2*3-,4-/m.1/s2;1",
        "InChI=1B/2C10H14BrCl7/c2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12/h2*3-10H,1-2H3/t2*3-,4-,5+,6-,7-,8-,9+,10-/m00/s2*1(3,5)2(4)(6,8)3(7,9)(10)",
        "InChI=1B/C12H26.2C10H14BrCl7.C9H20/c1-8(2)11(7)12(9(3)4)10(5)6;2*1-3(11)5(13)7(15)9(17)10(18)8(16)6(14)4(2)12;1-6-8(4)9(5)7(2)3/h8-12H,1-7H3;2*3-10H,1-2H3;7-9H,6H2,1-5H3/t11-;2*3-,4-,5+,6-,7-,8-,9+,10-;8-,9+/m100./s1;2*1(3,5)2(4)(6,8)3(7,9)(10);3",
        /* centre stereogenic only through an isotope */
        "InChI=1B/C2H6O/c1-2-3/h3H,2H2,1H3/i2D/t2-/s3"));

// Grouped /b (spec 3, §6): InChI -> InChI reprints it; the structure keeps
// each bond's E/Z but not its group (no bond groups in the public API)
class Inchi2InchiBTest : public ::testing::TestWithParam<const char *>
{
};

TEST_P(Inchi2InchiBTest, reprints_unchanged)
{
    EXPECT_EQ(inchi2inchi(GetParam()), GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    test_enhancedStereo_roundtrip, Inchi2InchiBTest,
    ::testing::Values(
        /* one OR / one AND block */
        "InChI=1B/C2BrCl2F/c3-1(4)2(5)6/b2(2-1-)",
        "InChI=1B/C10H16O/c1-9(2)5-4-6-10(3)7-8-11/h5,7-8H,4,6H2,1-3H3/b3(10-7-)",
        /* flat entries before the classes, OR class before AND class */
        "InChI=1B/C20H28O/c1-16(8-6-9-17(2)13-15-21)11-12-19-18(3)10-7-14-20(19,4)5/h6,8-9,11-13,15H,7,10,14H2,1-5H3/b9-6+,12-11+,2(17-13-),3(16-8-)",
        "InChI=1B/C5H2BrCl2F2I/c6-4(9)2(7)1-3(8)5(10)11/h1H2/b4-2+,2(5-3-)",
        /* two blocks of one class, two bonds in one block (relative +) */
        "InChI=1B/C5H2BrCl2F2I/c6-4(9)2(7)1-3(8)5(10)11/h1H2/b3(4-2-)(5-3-)",
        "InChI=1B/C5H2BrCl2F2I/c6-4(9)2(7)1-3(8)5(10)11/h1H2/b3(4-2-,5-3+)",
        /* next to /t, several components */
        "InChI=1B/C4H5BrClFO/c1-2(8)3(5)4(6)7/h2,8H,1H3/b2(4-3-)/t2-/m1/s1",
        "InChI=1B/2C2BrCl2F/c2*3-1(4)2(5)6/b2(2-1-);3(2-1-)"));

// The structure keeps the E/Z of a grouped bond: the group's relative
// geometry, read as absolute (lowest bond '-')
TEST(test_enhancedStereo_roundtrip, grouped_b_structure_keeps_geometry)
{
    EXPECT_EQ(round_trip("InChI=1B/C5H2BrCl2F2I/c6-4(9)2(7)1-3(8)5(10)11/h1H2/b3(4-2-,5-3+)"),
              "InChI=1B/C5H2BrCl2F2I/c6-4(9)2(7)1-3(8)5(10)11/h1H2/b4-2-,5-3+");
}

INSTANTIATE_TEST_SUITE_P(
    test_enhancedStereo_roundtrip_b, Inchi2InchiRejectTest,
    ::testing::Values(
        /* first bond of a block not '-' */
        "InChI=1B/C2BrCl2F/c3-1(4)2(5)6/b2(2-1+)",
        /* class digit not 2/3 */
        "InChI=1B/C2BrCl2F/c3-1(4)2(5)6/b4(2-1-)",
        /* unclosed block, bonds not ascending in a block */
        "InChI=1B/C2BrCl2F/c3-1(4)2(5)6/b2(2-1-",
        "InChI=1B/C5H2BrCl2F2I/c6-4(9)2(7)1-3(8)5(10)11/h1H2/b3(5-3-,4-2+)"));
