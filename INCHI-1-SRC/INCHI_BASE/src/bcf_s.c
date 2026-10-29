/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

/* djb-rwth: implementation of missing bounds - checking functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ichirvrs.h"
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

int memcpy_custom(char** dst, char* src, unsigned long long len)
{
    char* dst_local = (char*)calloc(len, sizeof(char));
    char* src_local = src;
    int k;

    if (dst_local)
    {
        *dst = dst_local;
        for (k = 0; k < len; k++)
        {
            dst_local[k] = src_local[k];
        }
    }
    else
    {
        free(dst_local);
        free(src_local);
        return RI_ERR_ALLOC;
    }
    return 0;
}
