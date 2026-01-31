/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.07
 * April 30, 2024
 *
 * MIT License
 *
 * Copyright (c) 2024 IUPAC and InChI Trust
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*
* The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * info@inchi-trust.org
 *
*/

/* djb-rwth: implementation of missing bounds - checking functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ichirvrs.h"
#include <math.h>
#include "bcf_s.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

static int dbl2int_f(double dblinp, int fwidth, int ndecpl, char* str);
static int dbl2int_e(double dblinp, int fwidth, int ndecpl, char* str);
static int dbl2int_g(double dblinp, int fwidth, int ndecpl, char* str);

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

/* djb-rwth: main sprintf emulation for avoiding locales */
int dbl2int(char* str, int fwidth, int ndecpl, char dbl_flag, double dblinp)
{
	switch (dbl_flag)
	{
	case 'f':
	case 'F':
		return dbl2int_f(dblinp, fwidth, ndecpl, str);
		break;
	case 'e':
	case 'E':
		return dbl2int_e(dblinp, fwidth, ndecpl, str);
		break;
	case 'g':
	case 'G':
		return dbl2int_g(dblinp, fwidth, ndecpl, str);
		break;
	default:
		printf("dbl2int format flag not valid.\n");
		break;
	}

	return -1;
}

/* djb-rwth: sprintf("%X.Yf", arg) emulation */
static int dbl2int_f(double dblinp, int fwidth, int ndecpl, char* str)
{
	double dec_part, dblinpabs;
	char* intpl_str;
	const char* dblinp_sign;
	int fw_int, fw_dec, i, fw_real, ret = 0;
	long long int intpl, decpl = 0, intpl_size;

	dblinp_sign = (dblinp >= 0.0) ? "" : "-";
	dblinpabs = fabsl(dblinp);
	intpl = (long long int)trunc(dblinpabs);
	dec_part = dblinpabs - intpl;
	fw_int = (int)log10((double)intpl > 0 ? (double)intpl : 1.0) + 1 + !strcmp(dblinp_sign, "-");
	intpl_size = ((long long int)fw_int + 3) * sizeof(unsigned long long int);
	intpl_str = (char*)inchi_malloc(intpl_size);

	if (ndecpl > 0)
	{
		fw_dec = (ndecpl > 9) ? 9 : ndecpl;
	}
	else if (ndecpl == 0)
	{
		fw_dec = 0;
	}
	else
	{
		fw_dec = 6;
	}
	fw_real = fw_int + (ndecpl != 0) + fw_dec;

	if (ndecpl)
	{
		fw_int += (fwidth - fw_real > 0) ? fwidth - fw_real : 0;

		for (i = 0; i < fw_dec; i++)
		{
			dec_part *= 10;
			decpl = 10 * decpl + (long long int)dec_part;
			dec_part = dec_part - trunc(dec_part);
		}

		decpl += (long long int)round(dec_part);
		if (((int)log10((double)decpl > 0 ? (double)decpl : 1.0) + 1) > fw_dec)
		{
			intpl++;
			decpl -= (int)pow(10, fw_dec);
		}

		if (intpl_str)
		{
			sprintf(intpl_str, "%s%lld", dblinp_sign, intpl); /* djb-rwth: ignoring LLVM warning */
		}
		else
		{
			return -1;
		}
		
		ret = sprintf(str, "%*s.%0*lld", fw_int, intpl_str, fw_dec, decpl); /* djb-rwth: ignoring LLVM warning */
		inchi_free(intpl_str);
		return ret;
	}
	else
	{
		intpl = (long long int)round(dblinp);
		
		inchi_free(intpl_str);
		return sprintf(str, "%*lld", fw_real, intpl); /* djb-rwth: ignoring LLVM warning */
	}
}

/* djb-rwth: sprintf("%X.Ye", arg) emulation */
static int dbl2int_e(double dblinp, int fwidth, int ndecpl, char* str)
{
	double dec_part, dblinpabs;
	int expnr = 0, fw_int, fw_dec, fw_real, dblinp_sign, nintpl, i, j;
	long long int intpl = 0, decpl = 0;

	dblinp_sign = (dblinp >= 0.0) ? 1 : -1;
	dblinpabs = fabsl(dblinp);
	intpl = (long long int)trunc(dblinpabs);
	nintpl = (int)log10((double)intpl > 0 ? (double)intpl : 1) + 1;

	if (nintpl > 1)
	{
		for (j = 0; j < nintpl - 1; j++)
		{
			dblinp /= 10.0;
			expnr++;
		}
	}
	else
	{
		if (dblinp)
		{
			while (!trunc(dblinp))
			{
				dblinp *= 10.0;
				expnr--;
			}
		}
	}

	dblinpabs = fabsl(dblinp);
	intpl = (long long int)trunc(dblinpabs);
	dec_part = dblinpabs - intpl;
	fw_int = (dblinp_sign < 0) + 1;

	if (ndecpl > 0)
	{
		fw_dec = (ndecpl > 9) ? 9 : ndecpl;
	}
	else if (ndecpl == 0)
	{
		fw_dec = 0;
	}
	else
	{
		fw_dec = 6;
	}
	fw_real = fw_int + (ndecpl != 0) + fw_dec + 4;

	if (ndecpl)
	{
		fw_int += (fwidth - fw_real > 0) ? fwidth - fw_real : 0;

		for (i = 0; i < fw_dec; i++)
		{
			dec_part *= 10;
			decpl = 10 * decpl + (int)dec_part;
			dec_part = dec_part - trunc(dec_part);
		}

		decpl += (long long int)round(dec_part);
		if (((int)log10((double)decpl > 0 ? (double)decpl : 1.0) + 1) > fw_dec)
		{
			intpl++;
			decpl -= (int)pow(10, fw_dec);
		}

		nintpl = (int)log10((double)intpl > 0 ? (double)intpl : 1.0) + 1;
		if (nintpl > 1)
		{
			intpl /= 10;
			expnr++;
		}

		intpl *= dblinp_sign;

		return sprintf(str, "%*lld.%0*llde%+0*d", fw_int, intpl, fw_dec, decpl, 3, expnr); /* djb-rwth: ignoring LLVM warning */
	}
	else
	{
		intpl = (int)round(dblinp);

		return sprintf(str, "%*llde%+0*d", fw_real, intpl, 3, expnr); /* djb-rwth: ignoring LLVM warning */
	}
}

/* djb-rwth: sprintf("%X.Yg", arg) emulation */
static int dbl2int_g(double dblinp, int fwidth, int ndecpl, char* str)
{
	double dec_part, dblinpabs, dblinpc = dblinp;
	char* intpl_str;
	const char* dblinp_signf;
	int expnr = 0, fw_int, fw_dec, fw_real, dblinp_signe, nintpl, i, j, ret = 0;
	long long int intpl = 0, decpl = 0, intpl_size;

	dblinpabs = fabsl(dblinpc);
	intpl = (long long int)trunc(dblinpabs);
	nintpl = (int)log10((double)intpl > 0 ? (double)intpl : 1) + 1;

	if (nintpl > 1)
	{
		for (j = 0; j < nintpl - 1; j++)
		{
			dblinpc /= 10.0;
			expnr++;
		}
	}
	else
	{
		if (dblinp)
		{
			while (!trunc(dblinpc))
			{
				dblinpc *= 10.0;
				expnr--;
			}
		}
	}

	if (ndecpl > 0)
	{
		fw_dec = (ndecpl > 9) ? 9 : ndecpl;
	}
	else if (ndecpl == 0)
	{
		fw_dec = 1;
	}
	else
	{
		fw_dec = 6;
	}

	if (!dblinpc)
	{
		ret = sprintf(str, "%lld", intpl); /* djb-rwth: ignoring LLVM warning */
		return ret;
	}

	if ((fw_dec > expnr) && (expnr >= -4))
	{
		dblinp_signf = (dblinp >= 0.0) ? "" : "-";
		dblinpabs = fabsl(dblinp);
		intpl = (long long int)trunc(dblinpabs);
		dec_part = dblinpabs - intpl;
		fw_int = (int)log10((double)intpl > 0 ? (double)intpl : 1.0) + 1 + !strcmp(dblinp_signf, "-");
		intpl_size = ((long long int)fw_int + 3) * sizeof(unsigned long long int);
		intpl_str = (char*)inchi_malloc(intpl_size);

		fw_real = fw_int + (ndecpl != 0) + fw_dec;

		if (fw_dec)
		{
			fw_int += (fwidth - fw_real > 0) ? fwidth - fw_real : 0;

			for (i = 0; i < fw_dec; i++)
			{
				dec_part *= 10;
				decpl = 10 * decpl + (long long int)dec_part;
				dec_part -= trunc(dec_part);
			}

			decpl += (long long int)round(dec_part);

			if (((int)log10((double)decpl > 0 ? (double)decpl : 1.0) + 1) > fw_dec)
			{
				intpl++;
				decpl -= (int)pow(10, fw_dec);
			}

			if (decpl)
			{
				while (!(decpl % 10))
				{
					decpl /= 10;
					fw_dec--;
				}
			}

			if (intpl_str)
			{
				sprintf(intpl_str, "%s%lld", intpl ? dblinp_signf : "", intpl); /* djb-rwth: ignoring LLVM warning */
			}
			else
			{
				return -1;
			}

			if (decpl)
			{
				ret = sprintf(str, "%*s.%0*lld", fw_int, intpl_str, fw_dec, decpl); /* djb-rwth: ignoring LLVM warning */
				inchi_free(intpl_str);
				return ret;
			}
			else
			{
				ret = sprintf(str,  "%*s", fw_int, intpl_str); /* djb-rwth: ignoring LLVM warning */
				inchi_free(intpl_str);
				return ret;
			}
		}
		else
		{	/* djb-rwth: addressing coverity ID #499558 -- currently leaving this as it is still a part of GHI #100 */
			intpl = (long long int)round(dblinp);
			ret = sprintf(str, "%*lld", fw_real, intpl); /* djb-rwth: ignoring LLVM warning */
			return ret;
		}
	}
	else
	{
		dblinp_signe = (dblinpc >= 0.0) ? 1 : -1;
		dblinpabs = fabsl(dblinpc);
		intpl = (long long int)trunc(dblinpabs);
		dec_part = dblinpabs - intpl;
		fw_int = (dblinp_signe < 0) + 1;

		fw_real = fw_int + (ndecpl != 0) + fw_dec + 4;

		if (ndecpl)
		{
			fw_int += (fwidth - fw_real > 0) ? fwidth - fw_real : 0;

			for (i = 0; i < fw_dec; i++)
			{
				dec_part *= 10;
				decpl = 10 * decpl + (int)dec_part;
				dec_part = dec_part - trunc(dec_part);
			}

			decpl += (long long int)round(dec_part);
			if (((int)log10((double)decpl > 0 ? (double)decpl : 1.0) + 1) > fw_dec)
			{
				intpl++;
				decpl -= (int)pow(10, fw_dec);
			}

			nintpl = (int)log10((double)intpl > 0 ? (double)intpl : 1.0) + 1;
			if (nintpl > 1)
			{
				intpl /= 10;
				expnr++;
			}

			intpl *= dblinp_signe;
			ret = sprintf(str, "%*lld.%0*llde%+0*d", fw_int, intpl, fw_dec, decpl, 3, expnr); /* djb-rwth: ignoring LLVM warning */
			return ret;
		}
		else
		{
			intpl = (int)round(dblinpc);
			ret = sprintf(str, "%*llde%+0*d", fw_real, intpl, 3, expnr); /* djb-rwth: ignoring LLVM warning */
			return ret;
		}
	}
}