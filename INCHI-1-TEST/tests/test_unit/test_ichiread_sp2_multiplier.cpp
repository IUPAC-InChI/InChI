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

TEST(test_ichiread_sp2_multiplier, rejects_zero_multiplier_past_last_component)
{
    /* One component in the formula; "0*4-3+" is a multiplier of 0 and
       would address a second, nonexistent component. */
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C4H8/c1-3-4-2/h3-4H,1-2H3/b4-3+;0*4-3+"), inchi_Ret_ERROR);
}

TEST(test_ichiread_sp2_multiplier, rejects_omitted_multiplier_past_last_component)
{
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C4H8/c1-3-4-2/h3-4H,1-2H3/b4-3+;*4-3+"), inchi_Ret_ERROR);
}

TEST(test_ichiread_sp2_multiplier, rejects_multiplier_past_last_component)
{
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C4H8/c1-3-4-2/h3-4H,1-2H3/b4-3+;2*4-3+"), inchi_Ret_ERROR);
}

TEST(test_ichiread_sp2_multiplier, accepts_multiplier_matching_components)
{
    EXPECT_EQ(inchi_to_inchi("InChI=1S/2C4H8/c2*1-3-4-2/h2*3-4H,1-2H3/b2*4-3+"), inchi_Ret_OKAY);
}
