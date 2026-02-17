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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <locale.h>

#include "mode.h"

#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
#include <malloc.h>
#include <io.h>
#endif


#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "ichimain.h"
#include "inchi_api.h"
#include "bcf_s.h"
#include "ichi_io.h"
#include "inpdef.h"
#include "permutation_util.h"


int rrand(int m)
{
    return
        (int)((double)m * (rand() / (RAND_MAX + 1.0)));
}

void shuffle(void* obj, size_t nmemb, size_t size)
{
    void* temp = inchi_malloc(size);
    size_t n = nmemb;
    while (n > 1)
    {
        size_t k = rrand((int)n--);
        if (temp) /* djb-rwth: fixing a NULL pointer dereference */
        {
            memcpy(temp, BYTE(obj) + n * size, size);
            memcpy(BYTE(obj) + n * size, BYTE(obj) + k * size, size);
            memcpy(BYTE(obj) + k * size, temp, size);
        }
    }
#ifdef _WIN32
    _free_dbg(temp, _NORMAL_BLOCK); /* djb-rwth: _free_dbg for _malloc_dbg must be used if Windows SDK is used */
#else
    free(temp); /* djb-rwth: otherwise just free */
#endif
}


/* Use after OrigAtData_Duplicate (permuted <-- saved) */
void OrigAtData_Permute(ORIG_ATOM_DATA* permuted, ORIG_ATOM_DATA* saved, int* numbers)
{
    int i, j, k;
    int nat = saved->num_inp_atoms;
    size_t atsize = sizeof(saved->at[0]);
    for (i = 0; i < nat; i++)
    {
        j = numbers[i];
        memcpy(permuted->at + j, saved->at + i, atsize);
        for (k = 0; k < permuted->at[j].valence; k++)
        {
            permuted->at[j].neighbor[k] = numbers[permuted->at[j].neighbor[k]];
        }
        permuted->at[j].orig_at_number = 1 + numbers[permuted->at[j].orig_at_number - 1];
    }
    if (saved->polymer && permuted->polymer)
    {
        if (saved->polymer->pzz)
        {
            for (k = 0; k < saved->polymer->n_pzz; k++)
            {
                permuted->polymer->pzz[k] = numbers[permuted->polymer->pzz[k]];
            }
        }
        if (saved->polymer->units)
        {
            for (k = 0; k < saved->polymer->n; k++)
            {
                permuted->polymer->units[k]->cap1 = 1 + numbers[permuted->polymer->units[k]->cap1 - 1];
                permuted->polymer->units[k]->cap1 = 1 + numbers[permuted->polymer->units[k]->end_atom1 - 1];
                permuted->polymer->units[k]->cap1 = 1 + numbers[permuted->polymer->units[k]->cap2 - 1];
                permuted->polymer->units[k]->cap1 = 1 + numbers[permuted->polymer->units[k]->end_atom2 - 1];
                if (permuted->polymer->units[k]->alist)
                {
                    for (j = 0; j < permuted->polymer->units[k]->na; j++)
                    {
                        permuted->polymer->units[k]->alist[j] = 1 + numbers[permuted->polymer->units[k]->alist[j] - 1];
                    }
                    for (j = 0; j < permuted->polymer->units[k]->nb; j++)
                    {
                        permuted->polymer->units[k]->blist[2 * j] = 1 + numbers[permuted->polymer->units[k]->blist[2 * j] - 1];
                        permuted->polymer->units[k]->blist[2 * j + 1] = 1 + numbers[permuted->polymer->units[k]->blist[2 * j + 1] - 1];
                    }
                }
            }
        }
    }
    if (saved->v3000 && permuted->v3000)
    {
        if (saved->v3000->atom_index_orig && permuted->v3000->atom_index_orig)
        {
            for (k = 0; k < nat; k++)
            {
                permuted->v3000->atom_index_orig[k] = numbers[permuted->v3000->atom_index_orig[k]];
            }
        }
        if (saved->v3000->atom_index_fin && permuted->v3000->atom_index_fin)
        {
            for (k = 0; k < nat; k++)
            {
                permuted->v3000->atom_index_fin[k] = numbers[permuted->v3000->atom_index_fin[k]];
            }
        }
        if (saved->v3000->n_haptic_bonds && saved->v3000->lists_haptic_bonds && permuted->v3000->n_haptic_bonds && permuted->v3000->lists_haptic_bonds)
        {
            for (j = 0; j < saved->v3000->n_haptic_bonds; j++)
            {
                permuted->v3000->lists_haptic_bonds[j][1] = numbers[permuted->v3000->lists_haptic_bonds[j][1]];
                for (k = 3; k < saved->v3000->lists_haptic_bonds[j][2]; k++)
                {
                    permuted->v3000->lists_haptic_bonds[j][k] = numbers[permuted->v3000->lists_haptic_bonds[j][k]];
                }
            }
        }
        if (saved->v3000->n_steabs && saved->v3000->lists_steabs && permuted->v3000->n_steabs && permuted->v3000->lists_steabs)
        {
            for (j = 0; j < saved->v3000->n_steabs; j++)
            {
                for (k = 2; k < saved->v3000->lists_steabs[j][1] + 2; k++)
                {
                    permuted->v3000->lists_steabs[j][k] = numbers[permuted->v3000->lists_steabs[j][k]];
                }
            }
        }
        if (saved->v3000->n_sterel && saved->v3000->lists_sterel && permuted->v3000->n_sterel && permuted->v3000->lists_sterel)
        {
            for (j = 0; j < saved->v3000->n_sterel; j++)
            {
                for (k = 2; k < saved->v3000->lists_sterel[j][1] + 2; k++)
                {
                    permuted->v3000->lists_sterel[j][k] = numbers[permuted->v3000->lists_sterel[j][k]];
                }
            }
        }
        if (saved->v3000->n_sterac && saved->v3000->lists_sterac && permuted->v3000->n_sterac && permuted->v3000->lists_sterac)
        {
            for (j = 0; j < saved->v3000->n_sterac; j++)
            {
                for (k = 2; k < saved->v3000->lists_sterac[j][1] + 2; k++)
                {
                    permuted->v3000->lists_sterac[j][k] = numbers[permuted->v3000->lists_sterac[j][k]];
                }
            }
        }
    }

    return;
}

EXPIMP_TEMPLATE INCHI_API int INCHI_DECL PermuteMolfileText(
    const char *moltext,
    char *permuted_moltext,
    size_t permuted_moltext_len)
{
    int status = -1;
    if (!moltext || !permuted_moltext || permuted_moltext_len == 0) return status;

    int *permutation_mapping = NULL;
    ORIG_ATOM_DATA atom_data;
    ORIG_ATOM_DATA permuted_atom_data;
    memset(&atom_data, 0, sizeof(atom_data));
    memset(&permuted_atom_data, 0, sizeof(permuted_atom_data));

    INCHI_IOSTREAM input_stream;
    inchi_ios_init(&input_stream, INCHI_IOS_TYPE_STRING, NULL);

    INCHI_IOSTREAM output_stream;
    inchi_ios_init(&output_stream, INCHI_IOS_TYPE_STRING, NULL);

    if (inchi_ios_print_nodisplay(&input_stream, "%s", moltext) <= 0)
        goto exit;

    INCHI_MODE input_atom_flags = 0;
    int struct_read_error = 0;
    int num_atoms = CreateOrigInpDataFromMolfile(
        &input_stream,
        &atom_data,
        0, 1, 1, 0, 0, NULL, NULL, NULL, NULL,
        &input_atom_flags,
        &struct_read_error,
        NULL, 0
    );

    if (num_atoms <= 0)
        goto exit;

    if (struct_read_error != 0)
        goto exit;

    if (OrigAtData_Duplicate(&permuted_atom_data, &atom_data) != 0)
        goto exit;

    permutation_mapping = malloc(num_atoms * sizeof(int));
    if (!permutation_mapping)
        goto exit;

    /* Enforce different permutation for molecules with more than one atom. */
    int is_identity;
    do {
        for (int i = 0; i < num_atoms; ++i)
            permutation_mapping[i] = i;
        shuffle(permutation_mapping, num_atoms, sizeof(int));
        is_identity = 1;
        for (int i = 0; i < num_atoms; ++i) {
            if (permutation_mapping[i] != i) {
                is_identity = 0;
                break;
            }
        }
    } while (num_atoms > 1 && is_identity);

    OrigAtData_Permute(&permuted_atom_data, &atom_data, permutation_mapping);

    if (OrigAtData_WriteToSDfile(
            &permuted_atom_data,
            &output_stream,
            NULL, NULL, 0, 0, NULL, NULL) != 0)
        goto exit;

    size_t out_len = output_stream.s.nUsedLength;
    if (out_len + 1 > permuted_moltext_len)
        goto exit;

    memcpy(permuted_moltext, output_stream.s.pStr, out_len);
    permuted_moltext[out_len] = '\0';
    status = 0;

exit:
    inchi_ios_close(&input_stream);
    inchi_ios_close(&output_stream);
    free(permutation_mapping);
    FreeOrigAtData(&permuted_atom_data);
    FreeOrigAtData(&atom_data);

    return status;
}
