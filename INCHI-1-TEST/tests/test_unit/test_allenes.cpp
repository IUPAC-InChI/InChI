#include <gtest/gtest.h>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

/* Allene gates of Spec 2 (InChI-Specifications/allenes): classic axis,
   rotamer/numbering invariance, single stereobond, enhanced OR/AND. */

namespace
{

enum Cfg { FLAT = 0, UP = 1, DOWN = 3 };

struct Atom { const char *el; double x, y; };
struct Bond { int a, b, order; Cfg cfg; };

/* 1-bromo-1-fluoro-3-chlorobuta-1,2-diene drawn as on DECK slide 18:

        F7              Cl5
          \            /
           C2 == C1 == C3
          /            \
        Br6             C4

   Atom 1 is the central (axial) atom. */
const std::vector<Atom> kAllene = {
    { "C", 5.7812, -4.0938 }, { "C", 4.6005, -4.0938 }, { "C", 6.9621, -4.0938 },
    { "C", 7.5524, -5.1163 }, { "Cl", 7.5524, -3.0712 }, { "Br", 4.0101, -5.1163 },
    { "F", 4.0101, -3.0712 } };

std::vector<Bond> allene_bonds( Cfg c3_cl, Cfg c2_br, Cfg c2_f = FLAT )
{
    return { { 2, 1, 2, FLAT }, { 1, 3, 2, FLAT }, { 3, 4, 1, FLAT },
             { 3, 5, 1, c3_cl }, { 2, 6, 1, c2_br }, { 2, 7, 1, c2_f } };
}

/* V3000 molblock; 'perm[i]' is the output number of input atom i+1 and
   'angle' rotates the drawing, both leaving the molecule unchanged */
std::string molblock( const std::vector<Atom> &atoms, const std::vector<Bond> &bonds,
                      const std::string &collection, const std::vector<int> &perm = {},
                      double angle = 0.0 )
{
    std::vector<int> order( atoms.size() );
    for (size_t i = 0; i < atoms.size(); i++) {
        order[perm.empty() ? i : perm[i] - 1] = (int)i;
    }

    std::ostringstream s;
    s << "\n  test\n\n  0  0  0     0  0            999 V3000\n"
      << "M  V30 BEGIN CTAB\n"
      << "M  V30 COUNTS " << atoms.size() << " " << bonds.size() << " 0 0 0\n"
      << "M  V30 BEGIN ATOM\n";
    for (size_t k = 0; k < order.size(); k++) {
        const Atom &a = atoms[order[k]];
        double x = a.x * std::cos( angle ) - a.y * std::sin( angle );
        double y = a.x * std::sin( angle ) + a.y * std::cos( angle );
        s << "M  V30 " << k + 1 << " " << a.el << " " << x << " " << y << " 0 0\n";
    }
    s << "M  V30 END ATOM\nM  V30 BEGIN BOND\n";
    for (size_t k = 0; k < bonds.size(); k++) {
        const Bond &b = bonds[k];
        int a1 = perm.empty() ? b.a : perm[b.a - 1];
        int a2 = perm.empty() ? b.b : perm[b.b - 1];
        s << "M  V30 " << k + 1 << " " << b.order << " " << a1 << " " << a2;
        if (b.cfg != FLAT) {
            s << " CFG=" << b.cfg;
        }
        s << "\n";
    }
    s << "M  V30 END BOND\n";
    if (!collection.empty()) {
        s << "M  V30 BEGIN COLLECTION\n" << collection << "M  V30 END COLLECTION\n";
    }
    s << "M  V30 END CTAB\nM  END\n";
    return s.str();
}

std::string inchi( const std::string &mol, const char *opts )
{
    inchi_Output out = {};
    std::vector<char> o( opts, opts + strlen( opts ) + 1 );
    MakeINCHIFromMolfileText( mol.c_str(), o.data(), &out );
    std::string r = out.szInChI ? out.szInChI : "";
    FreeINCHI( &out );
    return r;
}

const char kStd[] = "";
const char kEnh[] = "-EnhancedStereochemistry";
const std::string kAbs = "M  V30 MDLV30/STEABS ATOMS=(1 1)\n";
const std::string kRel = "M  V30 MDLV30/STEREL1 ATOMS=(1 1)\n";
const std::string kRac = "M  V30 MDLV30/STERAC1 ATOMS=(1 1)\n";
const std::string kNoStereo = "InChI=1S/C4H3BrClF/c1-3(6)2-4(5)7/h1H3";
const std::string kAxis = "/c1-3(6)2-4(5)7/h1H3/t2-";

} // namespace

/* A1: Ra/Sa pair; absolute output identical to standard InChI modulo prefix */
TEST( test_allenes, A1_classic_enantiomers )
{
    std::string up = molblock( kAllene, allene_bonds( UP, UP ), kAbs );
    std::string down = molblock( kAllene, allene_bonds( DOWN, DOWN ), kAbs );

    EXPECT_EQ( inchi( up, kStd ), "InChI=1S/C4H3BrClF" + kAxis + "/m1/s1" );
    EXPECT_EQ( inchi( down, kStd ), "InChI=1S/C4H3BrClF" + kAxis + "/m0/s1" );
    EXPECT_EQ( inchi( up, kEnh ), "InChI=1B/C4H3BrClF" + kAxis + "/m1/s1" );
    EXPECT_EQ( inchi( down, kEnh ), "InChI=1B/C4H3BrClF" + kAxis + "/m0/s1" );
}

/* A2: rotating the drawing or renumbering the atoms keeps the InChI */
TEST( test_allenes, A2_rotamer_and_numbering_invariance )
{
    std::vector<Bond> b = allene_bonds( UP, UP );
    std::string ref = inchi( molblock( kAllene, b, "" ), kStd );
    std::vector<int> reversed = { 7, 6, 5, 4, 3, 2, 1 };

    EXPECT_EQ( ref, "InChI=1S/C4H3BrClF" + kAxis + "/m1/s1" );
    EXPECT_EQ( inchi( molblock( kAllene, b, "", {}, 1.0 ), kStd ), ref );
    EXPECT_EQ( inchi( molblock( kAllene, b, "", {}, 2.5 ), kStd ), ref );
    EXPECT_EQ( inchi( molblock( kAllene, b, "", reversed ), kStd ), ref );
    EXPECT_EQ( inchi( molblock( kAllene, b, "", reversed, 4.0 ), kStd ), ref );
}

/* Both ends flat, or two wedges contradicting each other: no axis */
TEST( test_allenes, undefined_axis_has_no_t )
{
    std::string flat = molblock( kAllene, allene_bonds( FLAT, FLAT ), "" );
    std::string contra = molblock( kAllene, allene_bonds( UP, FLAT, UP ), "" );

    EXPECT_EQ( inchi( flat, kStd ), kNoStereo );
    EXPECT_EQ( inchi( flat, kEnh ), "InChI=1B/C4H3BrClF/c1-3(6)2-4(5)7/h1H3" );
    EXPECT_EQ( inchi( contra, kStd ), kNoStereo );
    EXPECT_EQ( inchi( contra, kEnh ), "InChI=1B/C4H3BrClF/c1-3(6)2-4(5)7/h1H3" );
}

/* A3: one wedge at either terminal defines the axis (enhanced only);
   standard InChI stays as before */
TEST( test_allenes, A3_single_stereobond )
{
    std::string cl_up = molblock( kAllene, allene_bonds( UP, FLAT ), "" );
    std::string br_up = molblock( kAllene, allene_bonds( FLAT, UP ), "" );
    std::string cl_down = molblock( kAllene, allene_bonds( DOWN, FLAT ), "" );
    std::string f_up = molblock( kAllene, allene_bonds( FLAT, FLAT, UP ), "" );

    EXPECT_EQ( inchi( cl_up, kStd ), kNoStereo );
    EXPECT_EQ( inchi( cl_up, kEnh ), "InChI=1B/C4H3BrClF" + kAxis + "/m1/s1" );
    EXPECT_EQ( inchi( br_up, kEnh ), "InChI=1B/C4H3BrClF" + kAxis + "/m1/s1" );
    EXPECT_EQ( inchi( cl_down, kEnh ), "InChI=1B/C4H3BrClF" + kAxis + "/m0/s1" );
    EXPECT_EQ( inchi( f_up, kEnh ), "InChI=1B/C4H3BrClF" + kAxis + "/m0/s1" );
    EXPECT_EQ( inchi( molblock( kAllene, allene_bonds( UP, FLAT ), "", {}, 2.0 ), kEnh ),
               inchi( cl_up, kEnh ) );
}

/* A4/A5: OR and AND on the axis; either enantiomer draw gives one InChI */
TEST( test_allenes, A4_A5_enhanced_or_and )
{
    for (Cfg c : { UP, DOWN }) {
        EXPECT_EQ( inchi( molblock( kAllene, allene_bonds( c, c ), kRel ), kEnh ),
                   "InChI=1B/C4H3BrClF" + kAxis + "/s2" );
        EXPECT_EQ( inchi( molblock( kAllene, allene_bonds( c, c ), kRac ), kEnh ),
                   "InChI=1B/C4H3BrClF" + kAxis + "/s3" );
    }
    EXPECT_EQ( inchi( molblock( kAllene, allene_bonds( UP, FLAT ), kRel ), kEnh ),
               "InChI=1B/C4H3BrClF" + kAxis + "/s2" );
}

/* A6: allene (central atom 1) + tetrahedral centre 6, DECK slide 22 */
TEST( test_allenes, A6_allene_plus_tetrahedral )
{
    const std::vector<Atom> atoms = {
        { "C", 0, 0 }, { "C", -1.2, 0 }, { "C", 1.2, 0 }, { "Cl", -1.8, 1.0 },
        { "Br", 1.8, 1.0 }, { "C", -1.8, -1.0 }, { "F", 1.8, -1.0 }, { "O", -1.2, -2.0 },
        { "C", -3.0, -1.0 } };
    const std::vector<Bond> bonds = {
        { 1, 2, 2, FLAT }, { 1, 3, 2, FLAT }, { 2, 4, 1, UP }, { 3, 5, 1, FLAT },
        { 2, 6, 1, FLAT }, { 3, 7, 1, UP }, { 6, 8, 1, DOWN }, { 6, 9, 1, FLAT } };
    const std::string base = "InChI=1B/C5H5BrClFO/c1-3(9)4(7)2-5(6)8/h3,9H,1H3/t2-,3-";

    EXPECT_EQ( inchi( molblock( atoms, bonds, "M  V30 MDLV30/STEABS ATOMS=(2 1 6)\n" ), kEnh ),
               base + "/m1/s1" );
    EXPECT_EQ( inchi( molblock( atoms, bonds, "M  V30 MDLV30/STEABS ATOMS=(1 6)\n" + kRel ), kEnh ),
               base + "/m1/s1(3)2(2)" );
    EXPECT_EQ( inchi( molblock( atoms, bonds, kAbs + "M  V30 MDLV30/STERAC1 ATOMS=(1 6)\n" ), kEnh ),
               base + "/m1/s1(2)3(3)" );
}
