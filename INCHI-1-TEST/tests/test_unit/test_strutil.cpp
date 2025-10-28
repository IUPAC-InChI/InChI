#include <gtest/gtest.h>

extern "C" {
#include "../../../INCHI-1-SRC/INCHI_BASE/src/strutil.h"
}

TEST(strutil_testing, test_SetConnectedComponentNumber)
{

    int num_atoms             = 1;
    int test_component_number = 23;
    inp_ATOM *new_mol         = CreateInpAtom(num_atoms);

    // int SetConnectedComponentNumber( inp_ATOM *at, int num_at, int component_number )
    EXPECT_EQ(SetConnectedComponentNumber(new_mol, 1, test_component_number), 0);

    for (int i = 0; i < num_atoms; i++) {
        EXPECT_EQ(new_mol[i].component, test_component_number);
    }
    FreeInpAtom(&new_mol);
}

TEST(strutil_testing, test_UnMarkRingSystemsInp)
{

    int num_atoms     = 5;
    inp_ATOM *new_mol = CreateInpAtom(num_atoms);

    for (int i = 0; i < num_atoms; i++) {
        new_mol[i].nRingSystem = i + 1;
    }

    // int UnMarkRingSystemsInp( inp_ATOM *at, int num_atoms )
    EXPECT_EQ(UnMarkRingSystemsInp(new_mol, num_atoms), 0);

    for (int i = 0; i < num_atoms; i++) {
        EXPECT_EQ(new_mol[i].nRingSystem, 0);
    }
    FreeInpAtom(&new_mol);
}

TEST(strutil_testing, test_CreateAndFreeInpAtom)
{

    int num_atoms     = 10;
    inp_ATOM *new_mol = CreateInpAtom(num_atoms);

    EXPECT_NE(new_mol, nullptr);

    FreeInpAtom(&new_mol);

    EXPECT_EQ(new_mol, nullptr);

    int num_atoms1     = -1;
    inp_ATOM *new_mol1 = CreateInpAtom(num_atoms1);

    EXPECT_EQ(new_mol1, nullptr);

    FreeInpAtom(&new_mol1);

    EXPECT_EQ(new_mol1, nullptr);

    int num_atoms2     = 0;
    inp_ATOM *new_mol2 = CreateInpAtom(num_atoms2);

    EXPECT_NE(new_mol2, nullptr);

    FreeInpAtom(&new_mol2);

    EXPECT_EQ(new_mol2, nullptr);
}

// void FreeInpAtomData( INP_ATOM_DATA *inp_at_data )
TEST(strutil_testing, test_CreateAndFreeInpAtomData)
{

    int num_atoms             = 10;
    INP_ATOM_DATA inp_at_data = {0};

    EXPECT_EQ(CreateInpAtomData(&inp_at_data, num_atoms, 1), 1);
    EXPECT_NE(inp_at_data.at, nullptr);
    EXPECT_NE(inp_at_data.at_fixed_bonds, nullptr);
    EXPECT_EQ(inp_at_data.num_at, num_atoms);

    FreeInpAtomData(&inp_at_data);

    EXPECT_EQ(inp_at_data.at, nullptr);
    EXPECT_EQ(inp_at_data.at_fixed_bonds, nullptr);
    EXPECT_EQ(inp_at_data.num_at, 0);

    EXPECT_EQ(CreateInpAtomData(&inp_at_data, num_atoms, 0), 1);
    EXPECT_NE(inp_at_data.at, nullptr);
    EXPECT_EQ(inp_at_data.at_fixed_bonds, nullptr);
    EXPECT_EQ(inp_at_data.num_at, num_atoms);

    FreeInpAtomData(&inp_at_data);

    EXPECT_EQ(inp_at_data.at, nullptr);
    EXPECT_EQ(inp_at_data.at_fixed_bonds, nullptr);
    EXPECT_EQ(inp_at_data.num_at, 0);
}
