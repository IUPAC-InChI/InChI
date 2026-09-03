/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef _DISP_STRU_H_
#define _DISP_STRU_H_



#include <windows.h>
#include "../../../INCHI_BASE/src/inpdef.h" /* djb-rwth: necessary header file */



/* Local types */

/****************************************************************************/

typedef struct tagInternalDrawParms
{
    double xmin, xmax, ymin, ymax;
    int    max_label_width_char;
    int    max_left_label_width_pix;
    int    max_right_label_width_pix;
    int bInit;
} INT_DRAW_PARMS; /* internal: saved for redisplaying one structure */


/****************************************************************************/
/*      Window data                                                            */
/****************************************************************************/

typedef struct tagWindowData
{

    inp_ATOM  *at0;    /* [MAX_ATOMS]; */
    inp_ATOM  *at1;    /* [MAX_ATOMS]; */
    INF_ATOM_DATA inf_at_data;
    /*inf_ATOM  *inf_at;*/ /* [MAX_ATOMS]; */
    int       num_at;
    int       bOrigAtom;
    int       bHighlight;
    int       bEsc;
    int       bUserIntervened;
    UINT      nTimerId;

    unsigned long  ulDisplTime;
    int            nFontSize;
    RECT           rc;        /* window rectangle size for saving */
    INT_DRAW_PARMS idp;       /* structure geom. parameters for redrawing */
    TBL_DRAW_PARMS tdp;       /* table data for displaying */
    char      *szTitle;       /* for TARGET_LIB_FOR_WINCHI printing */

    /* component equivalence info */
    AT_NUMB *nEquLabels;    /* num_at elements or NULL */
    AT_NUMB  nNumEquSets;   /* number of equivalent sets or 0 */
    AT_NUMB  nCurEquLabel;  /* in range 0..nNumEquSets; 0=>do not display equivalent components */

    AT_NUMB  nNewEquLabel;  /* non-zero only if DISPLAY_EQU_COMPONENTS==1 */
} MY_WINDOW_DATA;



#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif



    void FreeWinData( MY_WINDOW_DATA* pWinData );

    int CreateInputStructPicture( HDC hDC,
                                  MY_WINDOW_DATA *pWinData,
                                  RECT *rc,
                                  int bPrint,
                                  AT_NUMB nNewEquLabel );



#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif    /* _DISP_STRU_H_ */
