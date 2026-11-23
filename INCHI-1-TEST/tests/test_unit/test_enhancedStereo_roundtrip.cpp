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
