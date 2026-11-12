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

TEST(test_ichiread_mobile_h_multiplier, rejects_multiplier_past_last_component)
{
    /* One component in the formula, but "/h2*" claims two */
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C2H4O2/c1-2(3)4/h2*1H3,(H,3,4)"), inchi_Ret_ERROR);
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C2H4O2/c1-2(3)4/h9*1H3,(H,3,4)"), inchi_Ret_ERROR);
}

TEST(test_ichiread_mobile_h_multiplier, accepts_multiplier_matching_components)
{
    EXPECT_EQ(inchi_to_inchi("InChI=1S/C2H4O2/c1-2(3)4/h1H3,(H,3,4)"), inchi_Ret_OKAY);
    EXPECT_EQ(inchi_to_inchi("InChI=1S/2C2H4O2/c2*1-2(3)4/h2*1H3,(H,3,4)"), inchi_Ret_OKAY);
}
