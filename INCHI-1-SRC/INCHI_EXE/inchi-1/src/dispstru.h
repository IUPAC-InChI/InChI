/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
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
