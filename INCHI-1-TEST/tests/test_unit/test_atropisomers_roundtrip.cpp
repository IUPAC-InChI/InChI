#include <gtest/gtest.h>

#include <cstring>
#include <string>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

// Atropisomer axis through 0D stereo and InChI -> structure -> InChI (AT-R8).
//
//   0D input:   n1        n2      an axis is a DoubleBond-type 0D record
//                \       /        on the SINGLE bond a1-a2:
//                 a1 -- a2        neighbor = { n1, a1, a2, n2 }
//                /       \        .
//               x         y

static const char k_opts[] = "-EnhancedStereochemistry";

// 2,2'-dibromo-6,6'-dichlorobiphenyl, (M) and (P)
static const char k_biaryl_m[] =
    "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m0/s1";
static const char k_biaryl_p[] =
    "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H/t11-/m1/s1";
static const char k_biaryl_flat[] =
    "InChI=1B/C12H6Br2Cl2/c13-7-3-1-5-9(15)11(7)12-8(14)4-2-6-10(12)16/h1-6H";

/* Axis atoms of the biaryl: canonical 11 and 12, 0-based 10 and 11 */
enum { BIARYL_A1 = 10, BIARYL_A2 = 11 };

/* InChI of a structure given as atoms + 0D records */
static std::string inchi_of(inchi_Atom *atom, int num_atoms,
                            inchi_Stereo0D *stereo, int num_stereo)
{
    inchi_Input in;
    inchi_Output out;
    char opts[sizeof(k_opts)];

    memcpy(opts, k_opts, sizeof(k_opts));
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.atom = atom;
    in.num_atoms = (AT_NUM) num_atoms;
    in.stereo0D = stereo;
    in.num_stereo0D = (AT_NUM) num_stereo;
    in.szOptions = opts;

    GetINCHI(&in, &out);
    std::string s = out.szInChI ? out.szInChI : "";
    FreeINCHI(&out);

    return s;
}

/* InChI -> structure -> InChI */
static std::string round_trip(const char *inchi)
{
    inchi_InputINCHI ii;
    inchi_OutputStruct os;
    char opts[sizeof(k_opts)];

    memcpy(opts, k_opts, sizeof(k_opts));
    memset(&ii, 0, sizeof(ii));
    memset(&os, 0, sizeof(os));
    ii.szInChI = (char *) inchi;
    ii.szOptions = opts;

    GetStructFromINCHI(&ii, &os);
    std::string s = inchi_of(os.atom, os.num_atoms, os.stereo0D, os.num_stereo0D);
    FreeStructFromINCHI(&os);

    return s;
}

/* First neighbour of atom a other than b */
static int other_neighbor(const inchi_Atom *atom, int a, int b)
{
    for (int k = 0; k < atom[a].num_bonds; k++)
    {
        if (atom[a].neighbor[k] != b)
        {
            return atom[a].neighbor[k];
        }
    }

    return NO_ATOM;
}

/* Biaryl skeleton with one DoubleBond-type 0D record on the axis */
static std::string biaryl_with_0d_axis(S_CHAR parity)
{
    inchi_InputINCHI ii;
    inchi_OutputStruct os;
    inchi_Stereo0D axis;
    char opts[] = "";

    memset(&ii, 0, sizeof(ii));
    memset(&os, 0, sizeof(os));
    ii.szInChI = (char *) k_biaryl_flat;
    ii.szOptions = opts;
    GetStructFromINCHI(&ii, &os);

    memset(&axis, 0, sizeof(axis));
    axis.type = INCHI_StereoType_DoubleBond;
    axis.central_atom = NO_ATOM;
    axis.parity = parity;
    axis.neighbor[0] = (AT_NUM) other_neighbor(os.atom, BIARYL_A1, BIARYL_A2);
    axis.neighbor[1] = BIARYL_A1;
    axis.neighbor[2] = BIARYL_A2;
    axis.neighbor[3] = (AT_NUM) other_neighbor(os.atom, BIARYL_A2, BIARYL_A1);

    std::string s = inchi_of(os.atom, os.num_atoms, &axis, 1);
    FreeStructFromINCHI(&os);

    return s;
}

// A 0D axis record must define the axis; the two parities are the two
// enantiomers.
TEST(test_atropisomers_roundtrip, zero_d_axis_is_defined)
{
    std::string odd = biaryl_with_0d_axis(INCHI_PARITY_ODD);
    std::string even = biaryl_with_0d_axis(INCHI_PARITY_EVEN);

    EXPECT_TRUE(odd == k_biaryl_m || odd == k_biaryl_p) << odd;
    EXPECT_TRUE(even == k_biaryl_m || even == k_biaryl_p) << even;
    EXPECT_NE(odd, even);
}
