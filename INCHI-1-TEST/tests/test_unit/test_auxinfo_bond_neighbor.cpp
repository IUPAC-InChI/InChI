#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

/*
 * InchiToInchiAtom() reads the /rB: bond list of an AuxInfo string and only
 * checked the neighbor number against the upper bound. A neighbor of 0 becomes
 * index -1 and the bond is written into atom[-1], before the start of the
 * atom array. Reject it as "Bond to nonexistent atom" instead, as
 * InchiToInpAtom() in readinch.c already does.
 *
 * Under an AddressSanitizer build the first case aborts on the unpatched
 * tree; with the fix it returns inchi_Ret_ERROR. The second case checks that
 * a valid AuxInfo is still accepted.
 */
static int RunAuxInfo( const char* auxinfo, int* num_atoms )
{
    char buf[256];
    inchi_Input inp;
    InchiInpData data;

    snprintf( buf, sizeof( buf ), "%s", auxinfo );
    memset( &inp, 0, sizeof( inp ) );
    memset( &data, 0, sizeof( data ) );
    data.pInp = &inp;

    int ret = Get_inchi_Input_FromAuxInfo( buf, 0, 0, &data );
    *num_atoms = inp.num_atoms;
    Free_inchi_Input( &inp );
    return ret;
}

TEST( test_auxinfo_bond_neighbor, test_zero_neighbor_rejected )
{
    int num_atoms = -1;
    EXPECT_EQ( RunAuxInfo( "AuxInfo=1/0/N:1,2/rA:2CC/rB:s0;/rC:0,0,0;1,0,0;", &num_atoms ),
               inchi_Ret_ERROR );
    EXPECT_EQ( num_atoms, 0 );
}

TEST( test_auxinfo_bond_neighbor, test_valid_bond_accepted )
{
    int num_atoms = -1;
    EXPECT_EQ( RunAuxInfo( "AuxInfo=1/0/N:1,2/rA:2CC/rB:s1;/rC:0,0,0;1,0,0;", &num_atoms ),
               inchi_Ret_OKAY );
    EXPECT_EQ( num_atoms, 2 );
}
