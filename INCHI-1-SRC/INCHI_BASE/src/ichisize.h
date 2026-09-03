/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef _ICHISIZE_H_
#define _ICHISIZE_H_

typedef unsigned short AT_NUMB;
typedef unsigned short AT_RANK;
#define AT_RANK_MASK   ((AT_RANK)~0)

typedef signed short NUM_H;
#define MAX_ATOMS  32766
#define NORMALLY_ALLOWED_INP_MAX_ATOMS 1024


#define CHAR_MASK  0xFF


typedef AT_RANK  *pAT_RANK;
typedef pAT_RANK *ppAT_RANK;

typedef unsigned long INCHI_MODE;

#define LEN_COORD 10
#define NUM_COORD 3
typedef char MOL_COORD[LEN_COORD*NUM_COORD + NUM_COORD - 1]; /*copied 30 bytes from MOLfile */


#endif    /* _ICHISIZE_H_ */
