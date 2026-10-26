#include <cstdlib>
#include <cstring>
#include <string>
#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inpdef.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichimain.h"
}

// GH #75: V2000 counts fields are 3 chars wide; > 999 atoms or bonds
// needs V3000.
static const int V2000_MAX_COUNT = 999;

// Write an n-carbon chain (n atoms, n - 1 bonds) as SDfile.
static std::string WriteChain(int n)
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

    INCHI_IOSTREAM out{};
    inchi_ios_init(&out, INCHI_IOS_TYPE_STRING, nullptr);
    OrigAtData_WriteToSDfile(&data, &out, nullptr, nullptr, 0, 0, nullptr, nullptr);
    std::string sdf = out.s.pStr ? out.s.pStr : "";

    inchi_ios_free_str(&out);
    free(data.at);

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
    EXPECT_NE(sdf.find("M  V30 1000 C 1498.5 0 0 0\n"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 999 1 999 1000\n"), std::string::npos);
    EXPECT_NE(sdf.find("M  V30 END CTAB\nM  END\n$$$$\n"), std::string::npos);
}
