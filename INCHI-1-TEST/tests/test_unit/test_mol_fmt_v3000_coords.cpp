#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}

/*
 * A V3000 atom line carries free-format x/y/z coordinates that are read as
 * doubles and then rendered into the fixed 32-byte MOL_COORD slot
 * ctab->coords[i] via sprintf(szcoords, "%10g%10g%10g", ...) followed by an
 * unbounded strcpy. A finite but large-magnitude coordinate such as
 * -1.23456e+308 renders as "-1.23456e+308" (13 chars), so three of them
 * produce a 39-char string that overflows the 32-byte slot. The sibling
 * V2000 reader bounds the same copy with mystrncpy(..., 31).
 *
 * Under an AddressSanitizer build this aborts with a heap-buffer-overflow on
 * the unpatched tree; with the bounded copy it returns cleanly.
 */
static int RunMolfile( const char* molblock )
{
    char options[] = "";
    inchi_Output output;
    memset( &output, 0, sizeof( output ) );
    int ret = MakeINCHIFromMolfileText( molblock, options, &output );
    FreeINCHI( &output );
    return ret;
}

static const char* molblock_v3000_bigcoords = R"(
  repro

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C -1.23456e+308 -1.23456e+308 -1.23456e+308 0
M  V30 END ATOM
M  V30 END CTAB
M  END
)";

TEST( test_mol_fmt_v3000_coords, big_coords_do_not_overflow )
{
    /* Must not corrupt memory; any return code is acceptable. */
    (void) RunMolfile( molblock_v3000_bigcoords );
    SUCCEED();
}
