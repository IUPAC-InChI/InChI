#include <gtest/gtest.h>

#include <cstring>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

// GH #324: a tetrahedral 0D descriptor with central_atom = NO_ATOM made
// Extract0DParities read at[-1]. It must be rejected like any other
// malformed descriptor ("Wrong 0D stereo descriptor(s)").
TEST(test_github_324, tetrahedral_without_central_atom_is_rejected)
{
    // C-H; atom 0 is the would-be centre
    inchi_Atom atom[2];
    memset(atom, 0, sizeof(atom));
    strcpy(atom[0].elname, "C");
    strcpy(atom[1].elname, "H");
    atom[0].neighbor[0] = 1;
    atom[0].bond_type[0] = INCHI_BOND_TYPE_SINGLE;
    atom[0].num_bonds = 1;

    inchi_Stereo0D stereo;
    memset(&stereo, 0, sizeof(stereo));
    stereo.neighbor[0] = 0;
    stereo.neighbor[1] = NO_ATOM;
    stereo.neighbor[2] = NO_ATOM;
    stereo.neighbor[3] = NO_ATOM;
    stereo.central_atom = NO_ATOM;
    stereo.type = INCHI_StereoType_Tetrahedral;
    stereo.parity = INCHI_PARITY_ODD;

    char options[] = "";
    inchi_Input in;
    memset(&in, 0, sizeof(in));
    in.atom = atom;
    in.num_atoms = 2;
    in.stereo0D = &stereo;
    in.num_stereo0D = 1;
    in.szOptions = options;

    inchi_Output out;
    memset(&out, 0, sizeof(out));
    GetINCHI(&in, &out);

    ASSERT_NE(out.szMessage, nullptr);
    EXPECT_NE(strstr(out.szMessage, "Wrong 0D stereo descriptor"), nullptr) << out.szMessage;

    FreeINCHI(&out);
}
