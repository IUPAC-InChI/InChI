#include <cstdio>
#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

static int inchi_to_inchi(const char *inchi)
{
    char input[256];
    inchi_InputINCHI request = {};
    inchi_Output response = {};
    int ret;

    snprintf(input, sizeof(input), "%s", inchi);
    request.szInChI = input;

    ret = GetINCHIfromINCHI(&request, &response);
    FreeINCHI(&response);

    return ret;
}

TEST(test_ichiread_sp3_multiplier, rejects_zero_multiplier_past_last_component)
{
    /* One component in the formula; "*1-" reads as a multiplier of 0 and
       would address a second, nonexistent component. */
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3/t2-;*1-/m0/s1"), inchi_Ret_ERROR);
}

TEST(test_ichiread_sp3_multiplier, rejects_multiplier_past_last_component)
{
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3/t2-;2*1-/m0/s1"), inchi_Ret_ERROR);
}

TEST(test_ichiread_sp3_multiplier, accepts_multiplier_matching_components)
{
    EXPECT_EQ(inchi_to_inchi("InChI=1S/2C4H9Br/c2*1-3-4(2)5/h2*4H,3H2,1-2H3/t2*4-/m00/s1"), inchi_Ret_OKAY);
}
