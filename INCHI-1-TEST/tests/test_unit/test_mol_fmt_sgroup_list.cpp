#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/mode.h"
}

/*
 * An Sgroup "M  SAL" / "M  SBL" line carries a count that was copied straight
 * into the fixed sg_atoms[15] / sg_bonds[15] stack arrays in
 * MolfileReadSgroupOfPolymer() without an upper bound, unlike the sibling
 * STY/SST/SLB/SCN handlers. A line that declares more than 15 entries writes
 * past the array. Reject it (inchi_Ret_ERROR) instead.
 *
 * Under an AddressSanitizer build these two cases abort on the unpatched tree;
 * with the fix they return cleanly. The third case checks that a valid 15-atom
 * list is still accepted.
 */
static int RunMolfile( const char* molblock )
{
    char options[] = "-Polymers";
    inchi_Output output;
    memset( &output, 0, sizeof( output ) );
    int ret = MakeINCHIFromMolfileText( molblock, options, &output );
    FreeINCHI( &output );
    return ret;
}

/* 16-atom chain, SAL declares 16 atoms (one past sg_atoms[15]) */
static const char* molblock_sal16 = R"(
  repro

 16 15  0  0  0  0  0  0  0  0999 V2000
    1.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  1  0  0  0  0
  7  8  1  0  0  0  0
  8  9  1  0  0  0  0
  9 10  1  0  0  0  0
 10 11  1  0  0  0  0
 11 12  1  0  0  0  0
 12 13  1  0  0  0  0
 13 14  1  0  0  0  0
 14 15  1  0  0  0  0
 15 16  1  0  0  0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SAL   1 16   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
M  END
)";

/* 17-atom chain, SBL declares 16 bonds (one past sg_bonds[15]) */
static const char* molblock_sbl16 = R"(
  repro

 17 16  0  0  0  0  0  0  0  0999 V2000
    1.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   16.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   17.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  1  0  0  0  0
  7  8  1  0  0  0  0
  8  9  1  0  0  0  0
  9 10  1  0  0  0  0
 10 11  1  0  0  0  0
 11 12  1  0  0  0  0
 12 13  1  0  0  0  0
 13 14  1  0  0  0  0
 14 15  1  0  0  0  0
 15 16  1  0  0  0  0
 16 17  1  0  0  0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SBL   1 16   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
M  END
)";

/* 15-atom chain, SAL declares all 15 atoms (the per-line maximum) */
static const char* molblock_sal15 = R"(
  repro

 15 14  0  0  0  0  0  0  0  0999 V2000
    1.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   12.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   13.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   15.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  1  0  0  0  0
  7  8  1  0  0  0  0
  8  9  1  0  0  0  0
  9 10  1  0  0  0  0
 10 11  1  0  0  0  0
 11 12  1  0  0  0  0
 12 13  1  0  0  0  0
 13 14  1  0  0  0  0
 14 15  1  0  0  0  0
M  STY  1   1 SRU
M  SLB  1   1   1
M  SAL   1 15   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
M  END
)";

TEST( test_mol_fmt_sgroup_list, sal_over_15_is_rejected )
{
    EXPECT_EQ( RunMolfile( molblock_sal16 ), inchi_Ret_ERROR );
}

TEST( test_mol_fmt_sgroup_list, sbl_over_15_is_rejected )
{
    EXPECT_EQ( RunMolfile( molblock_sbl16 ), inchi_Ret_ERROR );
}

TEST( test_mol_fmt_sgroup_list, sal_at_15_is_accepted )
{
    int ret = RunMolfile( molblock_sal15 );
    EXPECT_TRUE( ret == inchi_Ret_OKAY || ret == inchi_Ret_WARNING );
}
