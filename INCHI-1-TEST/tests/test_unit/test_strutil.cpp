#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/strutil.h"
}

TEST(test_strutil, test_ExtractConnectedComponent)
{

    int num_atoms = 3;
    int test_component_number = 23;
    inp_ATOM *new_mol = CreateInpAtom(num_atoms);
    inp_ATOM *cmp_mol = CreateInpAtom(num_atoms);

    EXPECT_EQ(ExtractConnectedComponent(nullptr, 0, test_component_number, nullptr), 0);

    for (int i = 0; i < num_atoms; i++)
    {
        new_mol[i].component = test_component_number;
    }

    EXPECT_EQ(ExtractConnectedComponent(new_mol, num_atoms, test_component_number, cmp_mol), num_atoms);

    FreeInpAtom(&new_mol);
    FreeInpAtom(&cmp_mol);
}

TEST(test_strutil, test_SetConnectedComponentNumber)
{

    int num_atoms = 1;
    int test_component_number = 23;
    inp_ATOM *new_mol = CreateInpAtom(num_atoms);

    EXPECT_EQ(SetConnectedComponentNumber(new_mol, num_atoms, test_component_number), 0);

    for (int i = 0; i < num_atoms; i++)
    {
        EXPECT_EQ(new_mol[i].component, test_component_number);
    }
    FreeInpAtom(&new_mol);
}

/* (NaumanUllahKhan :: @nnuk)
 * Test to validate the disconnection of bond(s)
 * between a Metal and a Non-Metal atom.
 */
TEST(strutil_testing, test1_MolecularInorganicsPreprocessing)
{
    INPUT_PARMS ip;
    memset(&ip, 0, sizeof(ip));
    ip.bMolecularInorganics = true;
    ip.bMolecularInorganicsReconnectedInChI = false;

    ORIG_ATOM_DATA orig_at_data;
    memset(&orig_at_data, 0, sizeof(orig_at_data));

    inp_ATOM atoms[3];
    AT_NUMB nOldCompNumber[3] = {0};

    /* Initialize all atoms */
    for (int i = 0; i < 3; i++)
    {
        memset(&atoms[i], 0, sizeof(inp_ATOM));
        atoms[i].num_iso_H[0] = 0;
        atoms[i].num_iso_H[1] = 0;
        atoms[i].num_iso_H[2] = 0;
        atoms[i].num_H = 0;
        atoms[i].charge = 0;
        atoms[i].chem_bonds_valence = 0;
        atoms[i].valence = 0;
        atoms[i].el_number = 0;
        atoms[i].orig_at_number = i + 1;
    }

    /* Central Metal atom (Iron) */
    atoms[0].el_number = 26;
    atoms[0].valence = 2;
    atoms[0].chem_bonds_valence = 2;
    strcpy(atoms[0].elname, "Fe");
    atoms[0].neighbor[0] = 1;
    atoms[0].neighbor[1] = 2;
    atoms[0].bond_type[0] = 1;
    atoms[0].bond_type[1] = 1;

    /* Non-Metal atom (Fluorine) */
    atoms[1].el_number = 9;
    atoms[1].valence = 1;
    atoms[1].chem_bonds_valence = 1;
    strcpy(atoms[1].elname, "F");
    atoms[1].neighbor[0] = 0;
    atoms[1].bond_type[0] = 1;

    /* Non-Metal atom (Fluorine) */
    atoms[2].el_number = 9;
    atoms[2].valence = 1;
    atoms[2].chem_bonds_valence = 1;
    strcpy(atoms[2].elname, "F");
    atoms[2].neighbor[0] = 0;
    atoms[2].bond_type[0] = 1;

    orig_at_data.at = atoms;
    orig_at_data.num_inp_atoms = 3;
    orig_at_data.nOldCompNumber = nOldCompNumber;

    int result = MolecularInorganicsPreprocessing(&orig_at_data, &ip);

    /* Should disconnect both F bonds to Fe */
    EXPECT_EQ(result, 2);
    EXPECT_FALSE(ip.bMolecularInorganicsReconnectedInChI);

    /* Charges: Fe -> +2, both F -> -1 */
    EXPECT_EQ(atoms[0].charge, 2);
    EXPECT_EQ(atoms[1].charge, -1);
    EXPECT_EQ(atoms[2].charge, -1);
}

/* (NaumanUllahKhan :: @nnuk)
 * Test to check if the functionality fails correctly for Fe-Cl bond.
 */
TEST(strutil_testing, test2_MolecularInorganicsPreprocessing)
{
    INPUT_PARMS ip;
    memset(&ip, 0, sizeof(ip));
    ip.bMolecularInorganics = false; /* KEY: Functionality disabled */

    ORIG_ATOM_DATA orig_at_data;
    memset(&orig_at_data, 0, sizeof(orig_at_data));

    /* Create valid structure that WOULD disconnect if enabled */
    inp_ATOM atoms[2];
    AT_NUMB nOldCompNumber[2] = {0};

    for (int i = 0; i < 2; i++)
    {
        memset(&atoms[i], 0, sizeof(inp_ATOM));
        atoms[i].orig_at_number = i + 1;
    }

    /* Iron-Chlorine bond that SHOULD connect... but won't because function is disabled */
    atoms[0].el_number = 26;
    atoms[0].valence = 1;
    atoms[0].chem_bonds_valence = 1;
    strcpy(atoms[0].elname, "Fe");
    atoms[0].neighbor[0] = 1;
    atoms[0].bond_type[0] = 1;

    atoms[1].el_number = 17;
    atoms[1].valence = 1;
    atoms[1].chem_bonds_valence = 1;
    strcpy(atoms[1].elname, "Cl");
    atoms[1].neighbor[0] = 0;
    atoms[1].bond_type[0] = 1;

    orig_at_data.at = atoms;
    orig_at_data.num_inp_atoms = 2;
    orig_at_data.nOldCompNumber = nOldCompNumber;

    int result = MolecularInorganicsPreprocessing(&orig_at_data, &ip);

    /* Should return -1 that failure detection works */
    EXPECT_EQ(result, -1);

    EXPECT_EQ(atoms[0].charge, 0);
    EXPECT_EQ(atoms[1].charge, 0);
    EXPECT_FALSE(ip.bMolecularInorganicsReconnectedInChI);
}

/* (NaumanUllahKhan :: @nnuk)
 * Guard checks and basic functional validation for Molecular Inorganics valence lookup.
 */

TEST(strutil_testing, test_getElValenceforMolecularInorganics)
{
    EXPECT_EQ(getElValenceforMolecularInorganics(26, MIN_ATOM_CHARGE - 1, 0), 0);
    EXPECT_EQ(getElValenceforMolecularInorganics(26, MAX_ATOM_CHARGE + 1, 0), 0);
    EXPECT_EQ(getElValenceforMolecularInorganics(26, 0, MAX_NUM_VALENCES), 0);

    const int nPeriodicNum = 26; /* Fe */
    const int charge = 0;

    int non_zero = 0;
    int v;

    for (v = 0; v < MAX_NUM_VALENCES; v++)
    {
        if (getElValenceforMolecularInorganics(nPeriodicNum, charge, v) != 0)
        {
            non_zero = 1;
            break;
        }
    }
    EXPECT_EQ(non_zero, 1);
}

/* (NaumanUllahKhan :: @nnuk)
 * metal returns non-zero, non-metal returns 0 (based on the "type" value).
 */
TEST(strutil_testing, test_getElTypeforMolecularInorganics)
{
    EXPECT_NE(getElTypeforMolecularInorganics(26), 0);

    EXPECT_EQ(getElTypeforMolecularInorganics(9), 0);
}

/* (NaumanUllahKhan :: @nnuk)
 * Tests early-return cases and one valid disconnection case.
 */
TEST(strutil_testing, test_MolecularInorganicsIsMetalToDisconnect)
{
    inp_ATOM at[1];
    memset(&at[0], 0, sizeof(inp_ATOM));

    at[0].el_number = 26; /* Fe */
    strcpy(at[0].elname, "Fe");

    /* Case 1: at_valence == 0, returns 0 */
    at[0].valence = 0;
    at[0].chem_bonds_valence = 0;
    at[0].charge = 0;
    EXPECT_EQ(MolecularInorganicsIsMetalToDisconnect(at, 0), 0);

    /* Case 2: abs(charge) > 1, returns 0 */
    at[0].chem_bonds_valence = 1;
    at[0].charge = 2;
    EXPECT_EQ(MolecularInorganicsIsMetalToDisconnect(at, 0), 0);

    /* Case 3: normal valence path, should return 1 */
    at[0].charge = 0;
    at[0].chem_bonds_valence = 0;

    const int type = getElTypeforMolecularInorganics(at[0].el_number);

    int found = 0;
    int j;

    for (j = 0; j < 4 && !found; j++)
    {
        const int bitmask = (1 << j);
        const bool allowed = (bitmask & type) != 0;

        if (allowed)
        {
            const int normal_val = getElValenceforMolecularInorganics(at[0].el_number, at[0].charge, j);

            if (normal_val > 0)
            {
                at[0].chem_bonds_valence = normal_val;
                EXPECT_EQ(MolecularInorganicsIsMetalToDisconnect(at, 0), 1);
                found = 1;
            }
        }
    }

    /* Case 4: returns 0 when valence matches none of allowed ones */
    int normal_val_used = 0;

    for (j = 0; j < 4; j++)
    {
        const int bitmask = (1 << j);
        const bool allowed = (bitmask & type) != 0;
        if (allowed)
        {
            const int v = getElValenceforMolecularInorganics(at[0].el_number, at[0].charge, j);
            if (v > 0)
            {
                normal_val_used = v;
                break;
            }
        }
    }

    EXPECT_GT(normal_val_used, 0);

    at[0].chem_bonds_valence = 1;
    EXPECT_EQ(MolecularInorganicsIsMetalToDisconnect(at, 0), 0);

    EXPECT_TRUE(found);
}

/* (NaumanUllahKhan :: @nnuk)
 * verifies direct lookup mapping to the binary matrix.
 */
TEST(strutil_testing, test_shouldBondBeCut)
{
    const int atom1 = 26; /*Fe*/
    const int atom2 = 9;  /*F*/

     EXPECT_EQ(shouldBondBeCut(atom1, atom2), 1);
}

TEST(strutil_testing, test_UnMarkRingSystemsInp)
{

    int num_atoms = 5;
    inp_ATOM *new_mol = CreateInpAtom(num_atoms);

    for (int i = 0; i < num_atoms; i++)
    {
        new_mol[i].nRingSystem = i + 1;
    }

    EXPECT_EQ(UnMarkRingSystemsInp(new_mol, num_atoms), 0);

    for (int i = 0; i < num_atoms; i++)
    {
        EXPECT_EQ(new_mol[i].nRingSystem, 0);
    }
    FreeInpAtom(&new_mol);
}

TEST(test_strutil, test_CreateAndFreeInpAtom)
{

    int num_atoms = 10;
    inp_ATOM *new_mol = CreateInpAtom(num_atoms);

    EXPECT_NE(new_mol, nullptr);

    FreeInpAtom(&new_mol);

    EXPECT_EQ(new_mol, nullptr);

    int num_atoms1 = -1;
    inp_ATOM *new_mol1 = CreateInpAtom(num_atoms1);

    EXPECT_EQ(new_mol1, nullptr);

    FreeInpAtom(&new_mol1);

    EXPECT_EQ(new_mol1, nullptr);

    int num_atoms2 = 0;
    inp_ATOM *new_mol2 = CreateInpAtom(num_atoms2);

    EXPECT_NE(new_mol2, nullptr);

    FreeInpAtom(&new_mol2);

    EXPECT_EQ(new_mol2, nullptr);
}

TEST(test_strutil, test_CreateAndFreeInpAtomData)
{

    int num_atoms = 10;
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
