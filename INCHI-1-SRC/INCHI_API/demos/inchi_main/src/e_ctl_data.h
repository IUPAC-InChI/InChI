/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.07
 * 20/11/2023
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


#ifndef __CTL_DATA_H__
#define __CTL_DATA_H__
#include "e_ichisize.h"
/***********************************************/
#define STR_ERR_LEN 256

typedef struct tagStructData {
    unsigned long ulStructTime;
    int           nErrorCode;
    int           nErrorType;
    int           nStructReadError;
    int           bChiralFlag;
    char          pStrErrStruct[STR_ERR_LEN];
    long          fPtrStart;
    long          fPtrEnd;
    /* debugging info */
#if( bRELEASE_VERSION == 0 )
    int           bExtract;
#endif
} STRUCT_DATA;
/***********************************************/

#define MAX_NUM_PATHS 4


/* SDF treatment */
#define MAX_SDF_VALUE        255 /* max lenght of the SDFile data value */
#define MAX_SDF_HEADER        64 /* max length of the SDFile data header */

/***********************************************/

/* bCalcInChIHash values */
typedef enum tagInChIHashCalc
{
    INCHIHASH_NONE = 0,
    INCHIHASH_KEY = 1,
    INCHIHASH_KEY_XTRA1 = 2,
    INCHIHASH_KEY_XTRA2 = 3,
    INCHIHASH_KEY_XTRA1_XTRA2 = 4
}
INCHI_HASH_CALC;


typedef struct tagInputParms {
    char            szSdfDataHeader[MAX_SDF_HEADER + 1];
    char           *pSdfLabel;
    char           *pSdfValue;
    long            lSdfId;
    long            lMolfileNumber;


#if ( defined( TARGET_LIB_FOR_WINCHI ) || defined(TARGET_EXE_STANDALONE) )
#ifndef COMPILE_ANSI_ONLY
    DRAW_PARMS      dp;
    PER_DRAW_PARMS  pdp;
    TBL_DRAW_PARMS  tdp;
#endif
#endif


    /*
    -- Files --
    ip->path[0] => Input
    ip->path[1] => Output (INChI)
    ip->path[2] => Log
    ip->path[3] => Problem structures
    ip->path[4] => Errors file (ACD)

    */
    const char     *path[MAX_NUM_PATHS];
    int             num_paths;
    long            first_struct_number;
    long            last_struct_number;
    INPUT_TYPE      nInputType;
    INCHI_MODE      nMode;
    int             bAbcNumbers;
    int             bINChIOutputOptions;    /* !(ip->bINChIOutputOptions & INCHI_OUT_PLAIN_TEXT) */
    int             bINChIOutputOptions2;   /* v. 1.05 */
    int             bCtPredecessors;
    int             bDisplayEachComponentINChI;
    long            msec_MaxTime;           /* was ulMaxTime; max time to run ProsessOneStructure */
    long            msec_LeftTime;
    long            ulDisplTime;            /* not used: max structure or question display time */
    int             bDisplay;
    int             bDisplayIfRestoreWarnings; /* InChI->Struct debug */
    int             bMergeAllInputStructures;
    int             bSaveWarningStructsAsProblem;
    int             bSaveAllGoodStructsAsProblem;
    int             bGetSdfileId;
    int             bGetMolfileNumber;      /* read molfile number from the name line like "Structure #22"          */
    int             bCompareComponents;     /* see flags CMP_COMPONENTS, etc.                                       */
    int             bDisplayCompositeResults;
    int             bDoNotAddH;
    int             bNoStructLabels;
    int             bChiralFlag;            /* used only with "SUCF" switch                                         */
    int             bAllowEmptyStructure;   /* Issue warning on empty structure                                     */
    int             bLargeMolecules;        /* v. 1.05 */
    int             bLooseTSACheck;         /* v. 1.06 Relax criteria of ambiguous drawing for in-ring stereocenter */
    int             bPolymers;              /* v. 1.05  allow treatment of polymers                                 */
    int             bFoldPolymerSRU;        /* v. 1.06 consider possible CRU folding in case of inner repeat(s)     */
    int             bFrameShiftScheme;      /* v. 1.06 polymer CRU frame shift scheme to check for                  */
    int             bUseZz;                 /* v. 1.06 use Zz ('star') atoms in polymers or other context           */
    int             bStereoAtZz;            /* v. 1.06+ enable stereo centers having Zz neighbors (v. 1.06:always 0)*/

    int             bNoWarnings;            /* v. 1.06+ suppress warning messages                                   */

    int             bCalcInChIHash;
    int             bFixNonUniformDraw;     /* correct non-uniformly drawn oxoanions and amidinium cations.         */
                                            /* */
    INCHI_MODE      bTautFlags;
    INCHI_MODE      bTautFlagsDone;

#if ( READ_INCHI_STRING == 1 )
    int             bReadInChIOptions;
#endif

    /* post v.1 features */
#if ( UNDERIVATIZE == 1 )
    int             bUnderivatize;
#endif
#if ( RING2CHAIN == 1 )
    int             bRing2Chain;
#endif
#if ( RING2CHAIN == 1 || UNDERIVATIZE == 1 )
    int             bIgnoreUnchanged;
#endif
} INPUT_PARMS;


typedef enum tagFrameShifScheme
{
    FSS_STARS_CYCLED,       /* 0 bis-starred CRU - cycle, capped-ends - no frame shift (v.1.05) */
    FSS_NONE,               /* 1 no frame shift at all                                          */
    FSS_STARS_CYCLED_SORTED,/* 2 bis-starred CRU - cycle w sorted links , capped-ends - nothing */
    FSS_STARS_OPENED,       /* 2 bis-starred CRU - opened cycle, capped-ends - no frame shift   */
    FSS_STARS_ENDS_OPENED   /* 3 bis-starred CRU - opened cycle, capped-ends - no frame shift   */
} FRAME_SHIFT_SCHEME;

typedef enum tagExpoZZScheme
{
    EXPO_ZZ_NONE = 0,
    EXPO_ZZ_MINOR = 1,
    EXPO_ZZ_TOTAL = 2
}
EXPOZZ_SCHEME;


/*************************** INChI mode *******************************/
/* ip->nMode */
#define REQ_MODE_BASIC              0x000001    /* B    */
#define REQ_MODE_TAUT               0x000002    /* T    */
#define REQ_MODE_ISO                0x000004    /* I    */
#define REQ_MODE_NON_ISO            0x000008    /* NI   */
#define REQ_MODE_STEREO             0x000010    /* S    */
#define REQ_MODE_ISO_STEREO         0x000020    /* IS   */
#define REQ_MODE_NOEQ_STEREO        0x000040    /* SS   */
#define REQ_MODE_REDNDNT_STEREO     0x000080    /* RS   */
#define REQ_MODE_NO_ALT_SBONDS      0x000100    /* NASB */
/* new 10-10-2003 */
#define REQ_MODE_RELATIVE_STEREO    0x000200    /* REL All Relative Stereo */
#define REQ_MODE_RACEMIC_STEREO     0x000400    /* RAC All Racemic Stereo */
#define REQ_MODE_SC_IGN_ALL_UU      0x000800    /* IAUSC Ignore stereocenters if All Undef/Unknown */
#define REQ_MODE_SB_IGN_ALL_UU      0x001000    /* IAUSC Ignore stereobonds if All Undef/Unknown */
#define REQ_MODE_CHIR_FLG_STEREO    0x002000    /* SUCF  If Chiral flag then Abs otherwise Rel stereo */
/* end of 10-10-2003 */
/*^^^ 2009-12-05 */
#define REQ_MODE_DIFF_UU_STEREO     0x004000    /* SLUUD Make labels for unknown and undefined stereo different */
/*^^^ 2009-12-05 */

#define REQ_MODE_MIN_SB_RING_MASK   0x0F0000    /* RSB  */
#define REQ_MODE_MIN_SB_RING_SHFT      16

#define REQ_MODE_DEFAULT  (REQ_MODE_BASIC | REQ_MODE_TAUT | REQ_MODE_ISO | REQ_MODE_NON_ISO | REQ_MODE_STEREO)

/*********** compare components flags **********************************/
/* ip->bCompareComponents */
#define CMP_COMPONENTS              0x0001     /* perform compare components */
#define CMP_COMPONENTS_NONISO       0x0002     /* ignore isotopic */
#define CMP_COMPONENTS_NONTAUT      0x0004     /* compare non-tautomeric */

/****************** chemical identifier member definitions *************/
/* ip->bINChIOutputOptions */
#define INCHI_OUT_NO_AUX_INFO           0x0001   /* do not output Aux Info */
#define INCHI_OUT_SHORT_AUX_INFO        0x0002   /* output short version of Aux Info */
#define INCHI_OUT_ONLY_AUX_INFO         0x0004   /* output only Aux Info */
#define INCHI_OUT_EMBED_REC             0x0008   /* embed reconnected INChI into disconnected INChI */
#define INCHI_OUT_SDFILE_ONLY           0x0010   /* save input data in a Molfile instead of creating INChI */
#define INCHI_OUT_XML                   0x0020   /* output xml INChI */
#define INCHI_OUT_PLAIN_TEXT            0x0040   /* output plain text INChI */
#define INCHI_OUT_PLAIN_TEXT_COMMENTS   0x0080   /* output plain text annotation */
#define INCHI_OUT_XML_TEXT_COMMENTS     0x0100   /* output xml text annotation */
#define INCHI_OUT_WINCHI_WINDOW         0x0200   /* output into wINChI text window */
#define INCHI_OUT_TABBED_OUTPUT         0x0400   /* tab-delimited (only for plain text) */
#define INCHI_OUT_SDFILE_ATOMS_DT       0x0800   /* SDfile output H isotopes as D and T */
#define INCHI_OUT_SDFILE_SPLIT          0x1000   /* Split SDfile into components */
/*^^^ */
#define INCHI_OUT_FIX_TRANSPOSITION_CHARGE_BUG 0x2000
                                    /* used to accomodate FIX_TRANSPOSITION_CHARGE_BUG */
/*^^^ */
#define INCHI_OUT_STDINCHI 0x4000
#define INCHI_OUT_SAVEOPT  0x8000

#define INCHI_OUT_INCHI_GEN_ERROR        0x0001 /* v. 1.05 */
#define INCHI_OUT_MISMATCH_AS_ERROR      0x0002 /* v. 1.05 */

#define FLAG_INP_AT_CHIRAL         1
#define FLAG_INP_AT_NONCHIRAL      2
#define FLAG_SET_INP_AT_CHIRAL     4
#define FLAG_SET_INP_AT_NONCHIRAL  8

/* unknown/undefined stereo - constants  */
#define AB_PARITY_UNKN   3  /* 3 => user marked as unknown parity */
#define AB_PARITY_UNDF   4  /* 4 => parity cannot be defined because of symmetry or not well defined geometry */

#endif /* __CTL_DATA_H__ */
