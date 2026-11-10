#include <gtest/gtest.h>

#include <string>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

/*
 * Stereo perception and the options that control it, using 2-butanol with a
 * single stereocenter (atom 2).
 *
 * With SNon, stereo perception is skipped entirely: the output must be the
 * standard InChI without its stereo layers, also when undefined stereo would
 * otherwise be reported (SUU).
 */

namespace
{

/* 2-butanol, 2D, with a wedge from the stereocenter to O */
const char *BUTANOL_WEDGE =
    "\n"
    "  test\n"
    "\n"
    "  5  4  0  0  0  0  0  0  0  0999 V2000\n"
    "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.8660    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.7321    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    2.5981    0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.8660    1.5000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  1  0\n"
    "  2  3  1  0\n"
    "  3  4  1  0\n"
    "  2  5  1  1\n"
    "M  END\n";

/* 2-butanol without coordinates (all atoms at the origin) and no stereo
   bonds: the stereocenter has neither geometry nor 0D parity */
const char *BUTANOL_NO_COORDINATES =
    "\n"
    "  test\n"
    "\n"
    "  5  4  0  0  0  0  0  0  0  0999 V2000\n"
    "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000    0.0000    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  1  0\n"
    "  2  3  1  0\n"
    "  3  4  1  0\n"
    "  2  5  1  0\n"
    "M  END\n";

std::string GetInChI(const char *molfile, const char *options)
{
    std::string opts(options);
    inchi_Output output = {};

    MakeINCHIFromMolfileText(molfile, &opts[0], &output);
    std::string inchi = output.szInChI ? output.szInChI : "";
    FreeINCHI(&output);

    return inchi;
}

}  // namespace

TEST(test_stereoPerception, defined_stereocenter)
{
    EXPECT_EQ(GetInChI(BUTANOL_WEDGE, ""),
              "InChI=1S/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3/t4-/m1/s1");
}

TEST(test_stereoPerception, snon_drops_only_the_stereo_layers)
{
    EXPECT_EQ(GetInChI(BUTANOL_WEDGE, "-SNon"),
              "InChI=1S/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3");
}

TEST(test_stereoPerception, stereocenter_without_coordinates_is_undefined)
{
    /* undefined stereo is omitted from standard InChI ... */
    EXPECT_EQ(GetInChI(BUTANOL_NO_COORDINATES, ""),
              "InChI=1S/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3");
    /* ... and reported as '?' when requested */
    EXPECT_EQ(GetInChI(BUTANOL_NO_COORDINATES, "-SUU"),
              "InChI=1N/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3/t4?");
}

TEST(test_stereoPerception, snon_overrides_suu)
{
    EXPECT_EQ(GetInChI(BUTANOL_NO_COORDINATES, "-SNon -SUU"),
              "InChI=1N/C4H10O/c1-3-4(2)5/h4-5H,3H2,1-2H3");
}
