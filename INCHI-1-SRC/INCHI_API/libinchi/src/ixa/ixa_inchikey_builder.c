/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
 */


#include "../../../../INCHI_BASE/src/mode.h"
#include "../../../../INCHI_BASE/src/inchi_api.h"
#include "ixa_status.h"
#include <stdlib.h>
#include <string.h>


typedef struct
{
    char* inchi;
    char  inchi_key[256];
    int   dirty;
} INCHIKEYBUILDER;


/****************************************************************************/
static INCHIKEYBUILDER* KEYBUILDER_Unpack( IXA_STATUS_HANDLE hStatus,
                                           IXA_INCHIKEYBUILDER_HANDLE hKeyBuilder )
{
    if (!hKeyBuilder)
    {
        STATUS_PushMessage( hStatus, IXA_STATUS_ERROR, "Illegal keybuilder handle detected" );
        return NULL;
    }

    return (INCHIKEYBUILDER*) hKeyBuilder;
}


/****************************************************************************/
static IXA_INCHIKEYBUILDER_HANDLE KEYBUILDER_Pack( INCHIKEYBUILDER *pKeyBuilder )
{
    return (IXA_INCHIKEYBUILDER_HANDLE) pKeyBuilder;
}


/****************************************************************************/
IXA_INCHIKEYBUILDER_HANDLE INCHI_DECL IXA_INCHIKEYBUILDER_Create( IXA_STATUS_HANDLE hStatus )
{
    INCHIKEYBUILDER* key_builder = (INCHIKEYBUILDER*) inchi_malloc( sizeof( INCHIKEYBUILDER ) );
    if (!key_builder)
    {
        STATUS_PushMessage( hStatus, IXA_STATUS_ERROR, "Out of memory" );
        return NULL;
    }

    memset( key_builder, 0, sizeof( INCHIKEYBUILDER ) );
    return KEYBUILDER_Pack( key_builder );
}


/****************************************************************************/
void INCHI_DECL IXA_INCHIKEYBUILDER_Destroy( IXA_STATUS_HANDLE hStatus,
                                             IXA_INCHIKEYBUILDER_HANDLE hKeyBuilder )
{
    INCHIKEYBUILDER* key_builder = KEYBUILDER_Unpack( hStatus, hKeyBuilder );
    if (!key_builder) return;

    inchi_free( key_builder->inchi );
    inchi_free( key_builder );
}


/****************************************************************************/
void INCHI_DECL IXA_INCHIKEYBUILDER_SetInChI( IXA_STATUS_HANDLE hStatus,
                                              IXA_INCHIKEYBUILDER_HANDLE hKeyBuilder,
                                              const char *pInChI )
{
    INCHIKEYBUILDER* key_builder = KEYBUILDER_Unpack( hStatus, hKeyBuilder );
    if (!key_builder)
    {
        return;
    }

    inchi_free( key_builder->inchi );
    key_builder->inchi = (char *) inchi_malloc( strlen( pInChI ) + 1 );
    if (!key_builder->inchi)
    {
        STATUS_PushMessage( hStatus, IXA_STATUS_ERROR, "Out of memory" );
        return;
    }

    strcpy( key_builder->inchi, pInChI );
    key_builder->dirty = 1;
}


/****************************************************************************/
const char* INCHI_DECL IXA_INCHIKEYBUILDER_GetInChIKey( IXA_STATUS_HANDLE hStatus,
                                                        IXA_INCHIKEYBUILDER_HANDLE hKeyBuilder )
{
    INCHIKEYBUILDER* key_builder = KEYBUILDER_Unpack( hStatus, hKeyBuilder );
    if (!key_builder) return NULL;

    if (key_builder->dirty)
    {
        char extra1[256];
        char extra2[256];
        if (GetINCHIKeyFromINCHI( key_builder->inchi, 0, 0, key_builder->inchi_key, extra1, extra2 ) != INCHIKEY_OK)
        {
            printf( "Failed to create InChIKey\n" );
            return NULL;
        }
        key_builder->dirty = 0;
    }

    return key_builder->inchi_key;
}
