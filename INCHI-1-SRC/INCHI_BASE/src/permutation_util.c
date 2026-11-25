/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


/*
    Map a one-based atom index through the zero-based permutation `numbers`.
    Indices outside [1, nat] (e.g., 0 or -1 marking missing or star atoms)
    are exceptional and returned unchanged.
*/
static int get_permuted_index(const int* numbers, int nat, int index)
{
    if (index < 1 || index > nat)
    {
        return index;
    }
    return 1 + numbers[index - 1];
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
        if (saved->polymer->pzz && permuted->polymer->pzz)
        {
            for (k = 0; k < saved->polymer->n_pzz; k++)
            {
                permuted->polymer->pzz[k] = get_permuted_index(numbers, nat, permuted->polymer->pzz[k]);
            }
        }
        if (saved->polymer->units && permuted->polymer->units)
        {
            for (k = 0; k < saved->polymer->n; k++)
            {
                OAD_PolymerUnit* unit = permuted->polymer->units[k];
                if (!unit)
                {
                    continue;
                }
                unit->cap1 = get_permuted_index(numbers, nat, unit->cap1);
                unit->end_atom1 = get_permuted_index(numbers, nat, unit->end_atom1);
                unit->cap2 = get_permuted_index(numbers, nat, unit->cap2);
                unit->end_atom2 = get_permuted_index(numbers, nat, unit->end_atom2);
                if (unit->alist)
                {
                    for (j = 0; j < unit->na; j++)
                    {
                        unit->alist[j] = get_permuted_index(numbers, nat, unit->alist[j]);
                    }
                }
                if (unit->blist)
                {
                    for (j = 0; j < 2 * unit->nb; j++)
                    {
                        unit->blist[j] = get_permuted_index(numbers, nat, unit->blist[j]);
                    }
                }
            }
        }
    }
    if (saved->v3000 && permuted->v3000)
    {
        OAD_V3000* v3000 = permuted->v3000;
        if (saved->v3000->atom_index_orig && v3000->atom_index_orig)
        {
            for (k = 0; k < nat; k++)
            {
                v3000->atom_index_orig[k] = get_permuted_index(numbers, nat, v3000->atom_index_orig[k]);
            }
        }
        if (saved->v3000->atom_index_fin && v3000->atom_index_fin)
        {
            for (k = 0; k < nat; k++)
            {
                v3000->atom_index_fin[k] = get_permuted_index(numbers, nat, v3000->atom_index_fin[k]);
            }
        }
        if (saved->v3000->n_haptic_bonds && saved->v3000->lists_haptic_bonds && v3000->n_haptic_bonds && v3000->lists_haptic_bonds)
        {
            /* Layout: [0] bond type, [1] metal atom, [2] number of endpoints, [3..] endpoints */
            for (j = 0; j < saved->v3000->n_haptic_bonds; j++)
            {
                int* lst = v3000->lists_haptic_bonds[j];
                lst[1] = get_permuted_index(numbers, nat, lst[1]);
                for (k = 3; k < lst[2] + 3; k++)
                {
                    lst[k] = get_permuted_index(numbers, nat, lst[k]);
                }
            }
        }
        if (saved->v3000->n_steabs && saved->v3000->lists_steabs && v3000->n_steabs && v3000->lists_steabs)
        {
            for (j = 0; j < saved->v3000->n_steabs; j++)
            {
                int* lst = v3000->lists_steabs[j];
                for (k = 2; k < lst[1] + 2; k++)
                {
                    lst[k] = get_permuted_index(numbers, nat, lst[k]);
                }
            }
        }
        if (saved->v3000->n_sterel && saved->v3000->lists_sterel && v3000->n_sterel && v3000->lists_sterel)
        {
            for (j = 0; j < saved->v3000->n_sterel; j++)
            {
                int* lst = v3000->lists_sterel[j];
                for (k = 2; k < lst[1] + 2; k++)
                {
                    lst[k] = get_permuted_index(numbers, nat, lst[k]);
                }
            }
        }
        if (saved->v3000->n_sterac && saved->v3000->lists_sterac && v3000->n_sterac && v3000->lists_sterac)
        {
            for (j = 0; j < saved->v3000->n_sterac; j++)
            {
                int* lst = v3000->lists_sterac[j];
                for (k = 2; k < lst[1] + 2; k++)
                {
                    lst[k] = get_permuted_index(numbers, nat, lst[k]);
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
    if (CreateOrigInpDataFromMolfile(
        &input_stream,
        &atom_data,
        0, 1, 0, 0, 0, NULL, NULL, NULL, NULL, /* bDoNotAddH = 0: keep implicit H (#311) */
        &input_atom_flags,
        &struct_read_error,
        NULL, 0) <= 0)
        goto exit;

    if (struct_read_error != 0)
        goto exit;

    if (OrigAtData_Duplicate(&permuted_atom_data, &atom_data) != 0)
        goto exit;

    const int num_atoms = atom_data.num_inp_atoms;
    permutation_mapping = inchi_malloc(num_atoms * sizeof(int));
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
    inchi_free(permutation_mapping);
    FreeOrigAtData(&permuted_atom_data);
    FreeOrigAtData(&atom_data);

    return status;
}
