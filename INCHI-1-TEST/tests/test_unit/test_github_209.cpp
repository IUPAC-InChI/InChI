#include <gtest/gtest.h>
#include <cstring>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichicant.h"
#include "../../../INCHI-1-SRC/INCHI_BASE/src/ichicomn.h"
}

/* Leaks are only visible to LeakSanitizer (Debug test builds use ASAN).
   Without it these tests just exercise the code path. */
#if defined(__SANITIZE_ADDRESS__)
#define GH209_LSAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define GH209_LSAN 1
#endif
#endif

#ifdef GH209_LSAN
#include <sanitizer/lsan_interface.h>
#endif

static void ExpectNoLeaks()
{
#ifdef GH209_LSAN
    EXPECT_EQ(__lsan_do_recoverable_leak_check(), 0) << "see LeakSanitizer report above";
#endif
}

static void GetStruct(const char *inchi)
{
    char buf[256];
    inchi_InputINCHI input = {};
    inchi_OutputStruct output = {};

    strncpy(buf, inchi, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    input.szInChI = buf;

    GetStructFromINCHI(&input, &output);
    FreeStructFromINCHI(&output);
}

/* oss-fuzz #489561592: polymer unit bond list leaked in SetInChIExtInputByExtOrigAtData */
TEST(test_github_209, polymer_bond_list_not_leaked)
{
    GetStruct("InChI=1B/C3H6OZz2/c1-3(2-5)4-6/h3H,2H2,1H3/z101-1-4(6-4,5-2)");

    ExpectNoLeaks();
}

/* oss-fuzz #490920729: empty neighbor list leaked by CreateNeighList */
TEST(test_github_209, empty_neigh_list_not_leaked)
{
    NEIGH_LIST *nl = CreateNeighList(0, 0, NULL, 0, NULL);

    ASSERT_NE(nl, nullptr);
    FreeNeighList(nl);
    ExpectNoLeaks();
}
