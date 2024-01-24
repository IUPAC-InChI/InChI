/* djb-rwth: implementation of missing bounds - checking functions */  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bcf_s.h"

int max_3(int a, int b, int c)
{
    if ((a > b) && (a > c))
        return a;
    else if ((b > a) && (b > c))
        return b;
    else
        return c;
}