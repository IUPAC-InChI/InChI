/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef _IKEY_BASE26_H_
#define _IKEY_BASE26_H_

/*
    Base-26 encoding procedures.

    'Base26' characters here are considered to be uppercase English
    letters 'A..Z'
*/


/* Uncomment the next line to fix base-26 encoding bug */
/*#define FIX_BASE26_ENC_BUG 1*/

typedef unsigned int UINT32;
typedef unsigned short int UINT16;

#ifdef __cplusplus
extern "C" {
#endif

/*    Get a character representing 1st 14-bit triplet
    (bits 0..13 of contiguous array of octets)        */
    const char* base26_triplet_1( const unsigned char *a );
    /*    Get a character representing 2nd 14-bit triplet (bits 14..27)    */
    const char* base26_triplet_2( const unsigned char *a );
    /*    Get a character representing 3rd 14-bit triplet (bits 28..41)    */
    const char* base26_triplet_3( const unsigned char *a );
    /*    Get a character representing 4th 14-bit triplet (bits 42..55)    */
    const char* base26_triplet_4( const unsigned char *a );

    /*
        Tail dublets
    */

    /*    Get dublet (bits 28..36)    */
    const char* base26_dublet_for_bits_28_to_36( unsigned char *a );
    /*    Get dublet (bits 56..64)    */
    const char* base26_dublet_for_bits_56_to_64( unsigned char *a );
    /*    Get hash extension in hexadecimal representation for the major block.
        Len(extension) = 256 - 65 = 191 bit.                                */
    void get_xtra_hash_major_hex( const unsigned char *a, char* szXtra );
    /*    Get hash extension in hexadecimal representation for the minor block.
        Len(extension) = 256 - 37 = 219 bit.                                */
    void get_xtra_hash_minor_hex( const unsigned char *a, char* szXtra );

    /*    Used instead of isupper() to avoid locale interference.    */
#define isbase26(_c)    ( ((unsigned)(_c) >= 'A') && ((unsigned)(_c) <= 'Z') )

#ifdef __cplusplus
}
#endif


#endif    /* _IKEY_BASE26_H_ */
