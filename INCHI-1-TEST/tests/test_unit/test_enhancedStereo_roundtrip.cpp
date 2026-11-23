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
