#include <gtest/gtest.h>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

/* Double-bond E/Z gates of Spec 3 (InChI-Specifications/double-bond-ez):
   classic /b and enhanced OR/AND bond groups (STEBABS/STEBREL/STEBRAC). */

namespace
{

enum Cfg { FLAT = 0, UP = 1, EITHER = 2, DOWN = 3 };

struct Atom { const char *el; double x, y; };
struct Bond { int a, b, order; Cfg cfg; };

/* 'reversed' numbers the atoms backwards; bond order, and so the
   collections' bond indices, stay */
std::string molblock( const std::vector<Atom> &atoms, const std::vector<Bond> &bonds,
                      const std::string &collection, bool reversed = false )
{
    const int n = (int)atoms.size();
    auto num = [&]( int a ) { return reversed ? n + 1 - a : a; };
    std::ostringstream s;
    s << "\n  test\n\n  0  0  0     0  0            999 V3000\n"
      << "M  V30 BEGIN CTAB\n"
      << "M  V30 COUNTS " << atoms.size() << " " << bonds.size() << " 0 0 0\n"
      << "M  V30 BEGIN ATOM\n";
    for (int k = 1; k <= n; k++) {
        const Atom &a = atoms[num( k ) - 1];
        s << "M  V30 " << k << " " << a.el << " " << a.x << " " << a.y << " 0 0\n";
    }
    s << "M  V30 END ATOM\nM  V30 BEGIN BOND\n";
    for (size_t k = 0; k < bonds.size(); k++) {
        const Bond &b = bonds[k];
        s << "M  V30 " << k + 1 << " " << b.order << " " << num( b.a ) << " " << num( b.b );
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

/* BrClC=CClF (deck slides 25-28). Swapping Br/Cl on C1 swaps E/Z.

     Br3          Cl5
        \\       /
         C1 == C2          bond 1 is the double bond
        /       \\
     Cl4          F6                                            */
std::string bromo( const char *el3, const char *el4, Cfg dbl, Cfg c1_br,
                   const std::string &collection )
{
    return molblock( { { "C", 0, 0 }, { "C", 1.3, 0 }, { el3, -0.65, 1.1 },
                       { el4, -0.65, -1.1 }, { "Cl", 1.95, 1.1 }, { "F", 1.95, -1.1 } },
                     { { 1, 2, 2, dbl }, { 1, 3, 1, c1_br }, { 1, 4, 1, FLAT },
                       { 2, 5, 1, FLAT }, { 2, 6, 1, FLAT } },
                     collection );
}

const std::string kBromo = "InChI=1B/C2BrCl2F/c3-1(4)2(5)6";

/* F(X)C=C(Cl)-CH2-C(Cl)=C(Y)F: two independent double bonds, molfile
   bonds 3 and 8 (canonical 4-2 and 5-3). Swapping the halogen pair at
   one end swaps that bond's E/Z without moving any coordinate. */
std::string diene( bool flip_left, bool flip_right, const std::string &collection,
                   bool reversed = false )
{
    return molblock( { { "C", 0, 0 }, { "C", -1.2, 0.7 }, { "Cl", -1.2, 2.1 },
                       { "C", -2.4, 0 }, { flip_left ? "F" : "Br", -3.6, 0.7 },
                       { flip_left ? "Br" : "F", -2.4, -1.4 }, { "C", 1.2, 0.7 },
                       { "Cl", 1.2, 2.1 }, { "C", 2.4, 0 },
                       { flip_right ? "F" : "I", 3.6, 0.7 },
                       { flip_right ? "I" : "F", 2.4, -1.4 } },
                     { { 1, 2, 1, FLAT }, { 2, 3, 1, FLAT }, { 2, 4, 2, FLAT },
                       { 4, 5, 1, FLAT }, { 4, 6, 1, FLAT }, { 1, 7, 1, FLAT },
                       { 7, 8, 1, FLAT }, { 7, 9, 2, FLAT }, { 9, 10, 1, FLAT },
                       { 9, 11, 1, FLAT } },
                     collection, reversed );
}

const std::string kDiene = "InChI=1B/C5H2BrCl2F2I/c6-4(9)2(7)1-3(8)5(10)11/h1H2";
const std::string kRac38 = "M  V30 MDLV30/STEBRAC1 BONDS=(2 3 8)\n";

} // namespace

/* Z1: absolute bond (STEBABS or no collection) keeps the flat /b */
TEST( test_doubleBondEZ, Z1_absolute )
{
    const std::string abs = "M  V30 MDLV30/STEBABS BONDS=(1 1)\n";

    EXPECT_EQ( inchi( bromo( "Br", "Cl", FLAT, FLAT, "" ), kStd ),
               "InChI=1S/C2BrCl2F/c3-1(4)2(5)6/b2-1-" );
    EXPECT_EQ( inchi( bromo( "Br", "Cl", FLAT, FLAT, abs ), kStd ),
               "InChI=1S/C2BrCl2F/c3-1(4)2(5)6/b2-1-" );
    EXPECT_EQ( inchi( bromo( "Br", "Cl", FLAT, FLAT, abs ), kEnh ), kBromo + "/b2-1-" );
    EXPECT_EQ( inchi( bromo( "Cl", "Br", FLAT, FLAT, abs ), kEnh ), kBromo + "/b2-1+" );
}

/* Z2: crossed double bond or wavy single bond: unknown, no /b, never a group */
TEST( test_doubleBondEZ, Z2_either_is_not_or )
{
    const std::string rel = "M  V30 MDLV30/STEBREL1 BONDS=(1 1)\n";

    EXPECT_EQ( inchi( bromo( "Br", "Cl", EITHER, FLAT, "" ), kEnh ), kBromo );
    EXPECT_EQ( inchi( bromo( "Br", "Cl", FLAT, EITHER, "" ), kEnh ), kBromo );
    EXPECT_EQ( inchi( bromo( "Br", "Cl", EITHER, FLAT, rel ), kEnh ), kBromo );
}

/* Z3/Z4: one OR/AND bond; the drawn geometry is normalised away */
TEST( test_doubleBondEZ, Z3_Z4_single_bond_or_and )
{
    const std::string rel = "M  V30 MDLV30/STEBREL1 BONDS=(1 1)\n";
    const std::string rac = "M  V30 MDLV30/STEBRAC1 BONDS=(1 1)\n";

    for (const char *el : { "Br", "Cl" }) {
        const char *other = strcmp( el, "Br" ) ? "Br" : "Cl";
        EXPECT_EQ( inchi( bromo( el, other, FLAT, FLAT, rel ), kEnh ), kBromo + "/b2(2-1-)" );
        EXPECT_EQ( inchi( bromo( el, other, FLAT, FLAT, rac ), kEnh ), kBromo + "/b3(2-1-)" );
    }

    /* Standard InChI ignores the collections */
    EXPECT_EQ( inchi( bromo( "Cl", "Br", FLAT, FLAT, rel ), kStd ),
               "InChI=1S/C2BrCl2F/c3-1(4)2(5)6/b2-1+" );
}

/* Two bonds in one AND group keep their relative geometry: (E,Z) and
   (Z,E) are one racemate, (E,E) and (Z,Z) another */
TEST( test_doubleBondEZ, group_keeps_relative_geometry )
{
    EXPECT_EQ( inchi( diene( false, false, "" ), kEnh ), kDiene + "/b4-2+,5-3-" );

    EXPECT_EQ( inchi( diene( false, false, kRac38 ), kEnh ), kDiene + "/b3(4-2-,5-3+)" );
    EXPECT_EQ( inchi( diene( true, true, kRac38 ), kEnh ), kDiene + "/b3(4-2-,5-3+)" );
    EXPECT_EQ( inchi( diene( true, false, kRac38 ), kEnh ), kDiene + "/b3(4-2-,5-3-)" );
    EXPECT_EQ( inchi( diene( false, true, kRac38 ), kEnh ), kDiene + "/b3(4-2-,5-3-)" );
}

/* Groups ordered by their lowest bond; classes absolute, then 2, then 3,
   comma-joined */
TEST( test_doubleBondEZ, Z5_multiple_groups )
{
    EXPECT_EQ( inchi( diene( false, false, "M  V30 MDLV30/STEBRAC2 BONDS=(1 3)\n"
                                           "M  V30 MDLV30/STEBRAC1 BONDS=(1 8)\n" ), kEnh ),
               kDiene + "/b3(4-2-)(5-3-)" );
    EXPECT_EQ( inchi( diene( false, false, "M  V30 MDLV30/STEBREL1 BONDS=(1 8)\n" ), kEnh ),
               kDiene + "/b4-2+,2(5-3-)" );
    EXPECT_EQ( inchi( diene( false, false, "M  V30 MDLV30/STEBABS BONDS=(1 3)\n"
                                           "M  V30 MDLV30/STEBRAC1 BONDS=(1 8)\n" ), kEnh ),
               kDiene + "/b4-2+,3(5-3-)" );
    EXPECT_EQ( inchi( diene( false, false, "M  V30 MDLV30/STEBRAC1 BONDS=(1 3)\n"
                                           "M  V30 MDLV30/STEBREL1 BONDS=(1 8)\n" ), kEnh ),
               kDiene + "/b2(5-3-),3(4-2-)" );
}

/* Z6: double bond and tetrahedral centre group independently (deck slide 30) */
TEST( test_doubleBondEZ, Z6_bond_plus_tetrahedral )
{
    /* CH3-CH(OH)-C(Br)=C(Cl)F; bond 1 is C=C, atom 4 the stereocentre */
    auto mol = []( const std::string &collection ) {
        return molblock( { { "C", 0, 0 }, { "C", 1.3, 0 }, { "Br", -0.65, -1.1 },
                           { "C", -0.65, 1.1 }, { "Cl", 1.95, 1.1 }, { "F", 1.95, -1.1 },
                           { "O", 0, 2.2 }, { "C", -1.95, 1.1 } },
                         { { 1, 2, 2, FLAT }, { 1, 3, 1, FLAT }, { 1, 4, 1, FLAT },
                           { 2, 5, 1, FLAT }, { 2, 6, 1, FLAT }, { 4, 7, 1, UP },
                           { 4, 8, 1, FLAT } },
                         collection );
    };
    const std::string base = "InChI=1B/C4H5BrClFO/c1-2(8)3(5)4(6)7/h2,8H,1H3";

    EXPECT_EQ( inchi( mol( "M  V30 MDLV30/STEBABS BONDS=(1 1)\n"
                           "M  V30 MDLV30/STEABS ATOMS=(1 4)\n" ), kEnh ),
               base + "/b4-3-/t2-/m1/s1" );
    EXPECT_EQ( inchi( mol( "M  V30 MDLV30/STEBREL2 BONDS=(1 1)\n"
                           "M  V30 MDLV30/STEABS ATOMS=(1 4)\n" ), kEnh ),
               base + "/b2(4-3-)/t2-/m1/s1" );
    EXPECT_EQ( inchi( mol( "M  V30 MDLV30/STEBABS BONDS=(1 1)\n"
                           "M  V30 MDLV30/STERAC1 ATOMS=(1 4)\n" ), kEnh ),
               base + "/b4-3-/t2-/s3" );
}

/* A bond listed in a collection that is not a stereo bond is ignored */
TEST( test_doubleBondEZ, non_stereo_bond_ignored )
{
    EXPECT_EQ( inchi( bromo( "Br", "Cl", FLAT, FLAT, "M  V30 MDLV30/STEBREL1 BONDS=(1 2)\n" ),
                      kEnh ),
               kBromo + "/b2-1-" );
}

/* Renumbering the atoms keeps the grouped /b */
TEST( test_doubleBondEZ, numbering_invariance )
{
    for (bool l : { false, true }) {
        for (bool r : { false, true }) {
            EXPECT_EQ( inchi( diene( l, r, kRac38, true ), kEnh ),
                       inchi( diene( l, r, kRac38 ), kEnh ) );
        }
    }
}

/* Malformed bond collections are dropped: standard flat /b */
TEST( test_doubleBondEZ, malformed_collections_dropped )
{
    const std::string flat = kDiene + "/b4-2+,5-3-";

    EXPECT_EQ( inchi( diene( false, false, "M  V30 MDLV30/STEBREL1 BONDS=(1 3)\n"
                                           "M  V30 MDLV30/STEBRAC1 BONDS=(2 3 8)\n" ), kEnh ),
               flat );
    EXPECT_EQ( inchi( diene( false, false, "M  V30 MDLV30/STEBRAC1 BONDS=(2 3 99)\n" ), kEnh ),
               flat );
    EXPECT_EQ( inchi( diene( false, false, "M  V30 MDLV30/STEBRAC1 BONDS=(1 3)\n"
                                           "M  V30 MDLV30/STEBRAC1 BONDS=(1 8)\n" ), kEnh ),
               flat );
}

/* Two BrClC=CClF components (bonds 1 and 6); 'z_first'/'z_second' swap
   Br/Cl on that copy, i.e. draw it Z instead of E */
static std::string two_bromo( bool z_first, bool z_second, const std::string &collection )
{
    return molblock( { { "C", 0, 0 }, { "C", 1.3, 0 }, { z_first ? "Cl" : "Br", -0.65, 1.1 },
                       { z_first ? "Br" : "Cl", -0.65, -1.1 }, { "Cl", 1.95, 1.1 },
                       { "F", 1.95, -1.1 }, { "C", 5, 0 }, { "C", 6.3, 0 },
                       { z_second ? "Cl" : "Br", 4.35, 1.1 }, { z_second ? "Br" : "Cl", 4.35, -1.1 },
                       { "Cl", 6.95, 1.1 }, { "F", 6.95, -1.1 } },
                     { { 1, 2, 2, FLAT }, { 1, 3, 1, FLAT }, { 1, 4, 1, FLAT },
                       { 2, 5, 1, FLAT }, { 2, 6, 1, FLAT }, { 7, 8, 2, FLAT },
                       { 7, 9, 1, FLAT }, { 7, 10, 1, FLAT }, { 8, 11, 1, FLAT },
                       { 8, 12, 1, FLAT } },
                     collection );
}

/* Components equal in all standard layers are ordered by their enhanced
   classes, not by input order or by the meaningless drawn geometry of an
   OR/AND bond */
TEST( test_doubleBondEZ, component_order_invariance )
{
    const std::string rel1 = "M  V30 MDLV30/STEBREL1 BONDS=(1 1)\n";
    const std::string rel6 = "M  V30 MDLV30/STEBREL1 BONDS=(1 6)\n";
    const std::string rac1 = "M  V30 MDLV30/STEBRAC1 BONDS=(1 1)\n";
    const std::string rac6 = "M  V30 MDLV30/STEBRAC1 BONDS=(1 6)\n";
    const std::string two = "InChI=1B/2C2BrCl2F/c2*3-1(4)2(5)6";

    EXPECT_EQ( inchi( two_bromo( false, false, rel1 + rac6 ), kEnh ), two + "/b2(2-1-);3(2-1-)" );
    EXPECT_EQ( inchi( two_bromo( false, false, rel6 + rac1 ), kEnh ), two + "/b2(2-1-);3(2-1-)" );
    EXPECT_EQ( inchi( two_bromo( false, false, rac6 ), kEnh ), two + "/b2-1-;3(2-1-)" );
    EXPECT_EQ( inchi( two_bromo( false, false, rac1 ), kEnh ), two + "/b2-1-;3(2-1-)" );

    /* OR copy drawn E, AND copy drawn Z, and the other way round */
    EXPECT_EQ( inchi( two_bromo( false, true, rel1 + rac6 ), kEnh ),
               inchi( two_bromo( true, false, rel1 + rac6 ), kEnh ) );
}

/* Collections name bonds by their V3000 index, not their line position:
   reordered and gapped bond indices group the same bonds */
TEST( test_doubleBondEZ, bond_index_not_line_order )
{
    const std::string ref = inchi( diene( false, false, kRac38 ), kEnh );
    std::string mol = diene( false, false, "M  V30 MDLV30/STEBRAC1 BONDS=(2 3 20)\n" );

    /* Bond 8 renamed 20, and its line moved to the front of the bond block */
    const std::string line8 = "M  V30 8 2 7 9\n";
    mol.erase( mol.find( line8 ), line8.size() );
    mol.insert( mol.find( "M  V30 1 1 1 2\n" ), "M  V30 20 2 7 9\n" );

    EXPECT_EQ( ref, kDiene + "/b3(4-2-,5-3+)" );
    EXPECT_EQ( inchi( mol, kEnh ), ref );
}
