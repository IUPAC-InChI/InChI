#include <cstdlib>
#include <cstring>
#include <string>
#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inpdef.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichimain.h"
}

// GH #75: V2000 counts fields are 3 chars wide; > 999 atoms or bonds
// needs V3000.
static const int V2000_MAX_COUNT = 999;

// Build an n-carbon chain (n atoms, n - 1 bonds); free with FreeChain.
static ORIG_ATOM_DATA MakeChain(int n)
{
    ORIG_ATOM_DATA data{};
    data.num_inp_atoms = n;
    data.at = (inp_ATOM *)calloc(n, sizeof(inp_ATOM));

    for (int i = 0; i < n; i++)
    {
        inp_ATOM *a = data.at + i;
        strcpy(a->elname, "C");
        a->el_number = 6;
        a->x = 1.5 * i;

        // Link to previous atom
        if (i > 0)
        {
            a->neighbor[a->valence] = (AT_NUMB)(i - 1);
            a->bond_type[a->valence++] = 1;
        }

        // Link to next atom
        if (i < n - 1)
        {
            a->neighbor[a->valence] = (AT_NUMB)(i + 1);
            a->bond_type[a->valence++] = 1;
        }

        a->chem_bonds_valence = a->valence;
        a->num_H = (S_CHAR)(4 - a->valence);
    }

    return data;
}

static void FreeChain(ORIG_ATOM_DATA &data)
{
    free(data.at);
}

// Write data as SDfile.
static std::string Write(ORIG_ATOM_DATA &data)
{
    INCHI_IOSTREAM out{};
    inchi_ios_init(&out, INCHI_IOS_TYPE_STRING, nullptr);
    OrigAtData_WriteToSDfile(&data, &out, nullptr, nullptr, 0, 0, nullptr, nullptr);
    std::string sdf = out.s.pStr ? out.s.pStr : "";

    inchi_ios_free_str(&out);

    return sdf;
}

// Write an n-carbon chain as SDfile.
static std::string WriteChain(int n)
{
    ORIG_ATOM_DATA data = MakeChain(n);
    std::string sdf = Write(data);
    FreeChain(data);

    return sdf;
}

// Return line idx (0-based) of text.
static std::string Line(const std::string &text, int idx)
{
    size_t pos = 0;
    for (int i = 0; i < idx; i++)
    {
        pos = text.find('\n', pos) + 1;
    }

    return text.substr(pos, text.find('\n', pos) - pos);
}

TEST(test_github_75, small_molecule_stays_v2000)
{
    std::string sdf = WriteChain(V2000_MAX_COUNT);

    EXPECT_EQ(Line(sdf, 3), "999998  0  0  0  0  0  0  0  0  1 V2000");
}

TEST(test_github_75, more_than_999_atoms_writes_v3000)
{
    std::string sdf = WriteChain(V2000_MAX_COUNT + 1);

    EXPECT_EQ(Line(sdf, 3), "  0  0  0     0  0  0  0  0  0999 V3000");
    EXPECT_EQ(Line(sdf, 4), "M  V30 BEGIN CTAB");
    EXPECT_EQ(Line(sdf, 5), "M  V30 COUNTS 1000 999 0 0 0");
    EXPECT_EQ(Line(sdf, 6), "M  V30 BEGIN ATOM");
    EXPECT_NE(sdf.find("M  V30 1000 C 1498.5000 0.0000 0.0000 0\n"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 999 1 999 1000\n"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 END CTAB\nM  END\n$$$$\n"), std::string::npos);
}

TEST(test_github_75, deuterium_is_v3000_atom_not_alias)
{
    // Terminal CH2D: V2000 would write alias "A  1000" + "CH2D"
    ORIG_ATOM_DATA data = MakeChain(V2000_MAX_COUNT + 1);
    data.at[V2000_MAX_COUNT].num_H--;
    data.at[V2000_MAX_COUNT].num_iso_H[1] = 1;
    std::string sdf = Write(data);
    FreeChain(data);

    // No V2000 property lines; D is an explicit atom bonded to C 1000
    EXPECT_EQ(sdf.find("\nA  "), std::string::npos);
    EXPECT_EQ(Line(sdf, 5), "M  V30 COUNTS 1001 1000 0 0 0");
    EXPECT_NE(sdf.find("M  V30 1001 H 1500.0000 0.0000 0.0000 0 MASS=2\n"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 1000 1 1000 1001\n"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 END CTAB\nM  END\n"), std::string::npos);

    // Round trip; terminal carbon has canonical number 1
    char options[] = "-LargeMolecules";
    inchi_Output output{};
    MakeINCHIFromMolfileText(sdf.c_str(), options, &output);
    ASSERT_NE(output.szInChI, nullptr);
    std::string inchi = output.szInChI;
    EXPECT_EQ(inchi.substr(inchi.rfind('/')), "/i1D");

    FreeINCHI(&output);
}

TEST(test_github_75, v3000_writes_radical_and_wedge)
{
    // Atom 1: CH2 radical; bond 1-2 wedge up from atom 1
    ORIG_ATOM_DATA data = MakeChain(V2000_MAX_COUNT + 1);
    data.at[0].num_H = 2;
    data.at[0].radical = RADICAL_DOUBLET;
    data.at[0].bond_stereo[0] = STEREO_SNGL_UP;
    data.at[1].bond_stereo[0] = -STEREO_SNGL_UP;
    std::string sdf = Write(data);
    FreeChain(data);

    EXPECT_NE(sdf.find("M  V30 1 C 0.0000 0.0000 0.0000 0 RAD=2\n"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 1 1 1 2 CFG=1\n"), std::string::npos);
}

TEST(test_github_75, v3000_writes_polymer_sgroup)
{
    // SRU on atoms 2-3, crossing bonds 1-2 and 3-4, head-to-tail
    int alist[] = {2, 3};
    int blist[] = {1, 2, 3, 4};
    OAD_PolymerUnit unit{};
    unit.id = 1;
    unit.label = 1;
    unit.type = MOL_FMT_M_STY_SRU;
    unit.conn = MOL_FMT_M_CONN_HT;
    unit.na = 2;
    unit.nb = 2;
    unit.alist = alist;
    unit.blist = blist;
    strcpy(unit.smt, "n");
    OAD_PolymerUnit *units[] = {&unit};
    OAD_Polymer polymer{};
    polymer.units = units;
    polymer.n = 1;

    ORIG_ATOM_DATA data = MakeChain(V2000_MAX_COUNT + 1);
    data.polymer = &polymer;
    data.valid_polymer = 1;
    std::string sdf = Write(data);
    FreeChain(data);

    // No V2000 M  STY/SAL/SBL/SDI lines; one Sgroup inside the CTAB
    EXPECT_EQ(sdf.find("\nM  S"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 END BOND\n"
                       "M  V30 BEGIN SGROUP\n"
                       "M  V30 1 SRU 1 ATOMS=(2 2 3) XBONDS=(2 1 3) -\n"
                       "M  V30 BRKXYZ=(9 -1.0000 -1.0000 0 -1.0000 1.0000 0 0 0 0) -\n"
                       "M  V30 BRKXYZ=(9 1.0000 1.0000 0 1.0000 -1.0000 0 0 0 0) CONNECT=HT LABEL=n\n"
                       "M  V30 END SGROUP\n"
                       "M  V30 END CTAB\n"),
              std::string::npos);
}

TEST(test_github_75, v3000_mass_is_absolute)
{
    // 13C: iso_atw_diff 1 above the rounded mass 12 is stored as 2
    ORIG_ATOM_DATA data = MakeChain(V2000_MAX_COUNT + 1);
    data.at[4].iso_atw_diff = 2;
    std::string sdf = Write(data);
    FreeChain(data);

    EXPECT_NE(sdf.find("M  V30 5 C 6.0000 0.0000 0.0000 0 MASS=13\n"), std::string::npos);
}

// Return InChI of SDfile text, "" on failure.
static std::string InChIOf(const std::string &sdf)
{
    char options[] = "-LargeMolecules";
    inchi_Output output{};
    MakeINCHIFromMolfileText(sdf.c_str(), options, &output);
    std::string inchi = output.szInChI ? output.szInChI : "";
    FreeINCHI(&output);

    return inchi;
}

TEST(test_github_75, implicit_d_keeps_2d_stereo)
{
    // Zigzag chain; atom 501 is CHD(R)(R') with a wedge to atom 502
    const int c = 500;
    const int n = V2000_MAX_COUNT + 1;
    ORIG_ATOM_DATA data = MakeChain(n);
    for (int i = 0; i < n; i++)
    {
        data.at[i].y = (i % 2) * 0.75;
    }
    data.at[c].num_H = 1;
    data.at[c].bond_stereo[1] = STEREO_SNGL_UP;
    data.at[c + 1].bond_stereo[0] = -STEREO_SNGL_UP;

    // Reference: same structure, D as explicit atom 1001 below atom 501
    ORIG_ATOM_DATA ref = data;
    ref.num_inp_atoms = n + 1;
    ref.at = (inp_ATOM *)calloc(n + 1, sizeof(inp_ATOM));
    memcpy(ref.at, data.at, n * sizeof(inp_ATOM));
    inp_ATOM *d = ref.at + n;
    strcpy(d->elname, "H");
    d->el_number = 1;
    d->iso_atw_diff = 2;
    d->x = ref.at[c].x;
    d->y = -1.0;
    d->neighbor[0] = (AT_NUMB)c;
    d->bond_type[0] = 1;
    d->valence = d->chem_bonds_valence = 1;
    ref.at[c].neighbor[2] = (AT_NUMB)n;
    ref.at[c].bond_type[2] = 1;
    ref.at[c].valence = ref.at[c].chem_bonds_valence = 3;
    std::string expected = InChIOf(Write(ref));
    FreeChain(ref);

    // Implicit D written as explicit atom
    data.at[c].num_iso_H[1] = 1;
    std::string sdf = Write(data);
    FreeChain(data);

    ASSERT_NE(expected.find("/t"), std::string::npos);
    ASSERT_NE(expected.find("/i"), std::string::npos);
    EXPECT_EQ(InChIOf(sdf), expected);
    EXPECT_EQ(sdf.find("M  V30 1001 H 750.0000 0.0000 0.0000"), std::string::npos);
}

TEST(test_github_75, implicit_d_stays_at_origin_in_0d)
{
    ORIG_ATOM_DATA data = MakeChain(V2000_MAX_COUNT + 1);
    for (int i = 0; i < data.num_inp_atoms; i++)
    {
        data.at[i].x = 0;
    }
    data.at[0].num_H--;
    data.at[0].num_iso_H[1] = 1;
    std::string sdf = Write(data);
    FreeChain(data);

    EXPECT_NE(sdf.find("M  V30 1001 H 0.0000 0.0000 0.0000 0 MASS=2\n"), std::string::npos);
}
