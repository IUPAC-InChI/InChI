#include <gtest/gtest.h>
#include <string>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichi_io.h"
}

// A print of INCHI_ADD_STR_LEN (32768) or more characters into an empty string
// stream must leave room for the terminating NUL written by vsprintf().

static const std::string long_text(40000, 'x');

static void expect_room_for_terminator(const INCHI_IOSTREAM &ios)
{
    EXPECT_EQ(ios.s.nUsedLength, (int)long_text.size());
    EXPECT_GT(ios.s.nAllocatedLength, ios.s.nUsedLength);
    EXPECT_EQ(ios.s.pStr, long_text);
}

TEST(test_github_298, inchi_ios_print_leaves_room_for_terminator)
{
    INCHI_IOSTREAM ios;
    inchi_ios_init(&ios, INCHI_IOS_TYPE_STRING, NULL);

    inchi_ios_print(&ios, "%s", long_text.c_str());
    expect_room_for_terminator(ios);

    inchi_ios_close(&ios);
}

TEST(test_github_298, inchi_ios_eprint_leaves_room_for_terminator)
{
    INCHI_IOSTREAM ios;
    inchi_ios_init(&ios, INCHI_IOS_TYPE_STRING, NULL);

    inchi_ios_eprint(&ios, "%s", long_text.c_str());
    expect_room_for_terminator(ios);

    inchi_ios_close(&ios);
}
