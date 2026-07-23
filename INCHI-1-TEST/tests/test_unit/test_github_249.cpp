#include <gtest/gtest.h>

extern "C"
{
#include "../../../INCHI-1-SRC/INCHI_BASE/src/inchi_api.h"
}

TEST(test_github_249, rejects_out_of_range_mobile_h_atom_number)
{
    char input[] = "InChI=1/H/h4294967295H";
    inchi_InputINCHI request = {};
    inchi_Output response = {};

    request.szInChI = input;

    EXPECT_EQ(GetINCHIfromINCHI(&request, &response), inchi_Ret_ERROR);

    FreeINCHI(&response);
}
