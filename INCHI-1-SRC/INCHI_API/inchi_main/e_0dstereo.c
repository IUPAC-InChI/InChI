/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.03
 * May 9, 2010
 *
 * Originally developed at NIST
 * Modifications and additions by IUPAC and the InChI Trust
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC);
 * you can redistribute this software and/or modify it under the terms of 
 * the GNU Lesser General Public License as published by the Free Software 
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-2.1.php
 */


/* this file is used only in case of #define CREATE_0D_PARITIES       */
/* hardcoded bFixSp3Bug = 1 fixes sp3 bugs in original InChI v. 1.00  */
/* Phosphine and Arsine sp3 stereo are not supported in this mode     */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>


#include "e_mode.h"

#include "e_ctl_data.h"
#include "inchi_api.h"


#include "e_inchi_atom.h"
#include "e_ichisize.h"
#include "e_comdef.h"
#include "e_ichicomp.h"
#include "e_util.h"
#include "e_0dstereo.h"



#define ALWAYS_SET_STEREO_PARITY   0

#define SB_PARITY_FLAG  0x38 /* disconnected structure has undef. parity */
#define SB_PARITY_SHFT  3
#define SB_PARITY_MASK  0x07
#define SB_PARITY_1(X) (X & SB_PARITY_MASK)  /* refers to connected structure */
#define SB_PARITY_2(X) (((X) >> SB_PARITY_SHFT) & SB_PARITY_MASK) /* refers to connected structure */

#define inchi_NUMH(AT,CUR_AT)  (AT[CUR_AT].num_iso_H[0]+AT[CUR_AT].num_iso_H[1]+AT[CUR_AT].num_iso_H[2]+AT[CUR_AT].num_iso_H[3])
#define inchi_NUMH2(AT,CUR_AT) ((AT[CUR_AT].num_iso_H[0]>0?AT[CUR_AT].num_iso_H[0]:0) +AT[CUR_AT].num_iso_H[1]+AT[CUR_AT].num_iso_H[2]+AT[CUR_AT].num_iso_H[3])
#define IS_METAL(a) (a == AtType_Metal || a == AtType_Sn4 || a == AtType_Sn3 || a == AtType_Sn2)

typedef struct tagStereo0D {
    inchi_Stereo0D *stereo0D;         /* array of num_stereo0D 0D stereo elements or NULL */
    int            num_stereo0D;      /* number of 0D stereo elements */
    int            max_num_Stereo0D;  /* allocated length of stereo0D */
    int            delta_num_stereo0D;/* allocation increments */ 
    S_CHAR        *cAtType;
} Stereo0D;

typedef enum tagAtType {
    /* possible stereocenter */
    AtType_C4    = 1, /* >C< */
    AtType_Si4   = 2, /* >Si< */
    AtType_Ge4   = 3, /* >Ge< */
    AtType_Sn4   = 4, /* >Sn< */
    AtType_B4m   = 5, /* B(-) */
    AtType_S4    = 6, /* =S< */
    AtType_S6    = 7, /* >S<, 2 double bonds */
    AtType_S3p   = 8, /* -S(+)< */
    AtType_S5p   = 9, /* >S(+)<, 1 double bond */
    AtType_Se4   =10, /* =Se< */
    AtType_Se6   =11, /* >Se<, 2 double bonds */
    AtType_Se3p  =12, /* -Se(+)< */
    AtType_Se5p  =13, /* >Se(+)<, 1 double bond */
    AtType_N5    =14, /* >N<, 1 double bond */
    AtType_N4p   =15, /* >N(+)< */
    AtType_N3r   =16, /* -N<|  (N in a 3-member ring) */
    AtType_P4p   =17, /* >N(+)< */
    AtType_P5    =18, /* >P<, 1 double bond */
    AtType_As4p  =19, /* >As(+)< */
    AtType_As5   =20, /* >As<, 1 double bond */
    /* possible stereobond */
    AtType_C3    =21, /* =C<, =C/ */
    AtType_Si3   =22,
    AtType_Ge3   =23,
    AtType_Sn3   =24,
    AtType_N3p   =25, /* =N(+)< */
    AtType_N3    =26, /* =N/ */
    /* middle allene/cumulene */
    AtType_C2    =27, /* =C= */
    AtType_Si2   =28, /* =Si= */
    AtType_Ge2   =29, /* =Ge= */
    AtType_Sn2   =30, /* =Ge= */
    /* may become stereobond after charge or radical shift */
    AtType_Nns   =31,
    /* metal */
    AtType_Metal =32,
    /* terminal H */
    AtType_TermH =33,
    AtType_TermD =34,
    AtType_TermT =35
} AT_TYPE;

typedef enum tagElType {
    ElType_C  = 1,
    ElType_Si = 2,
    ElType_Ge = 3,
    ElType_Sn = 4,
    ElType_B  = 5,
    ElType_S  = 6,
    ElType_Se = 7,
    ElType_N  = 8,
    ElType_P  = 9,
    ElType_As =10,
    ElType_H  =11,
    ElType_D  =12,
    ElType_T  =13

} EL_TYPE;

inchi_Stereo0D *e_GetNewStereo( Stereo0D *pStereo );


static int ee_extract_ChargeRadical( char *elname, int *pnRadical, int *pnCharge );
static int ee_extract_H_atoms( char *elname, S_CHAR num_iso_H[] );
static int e_GetElType( inchi_Atom *at, int cur_atom );
static int e_bCanInpAtomBeAStereoCenter( int cur_at, S_CHAR *cAtType );
static int e_nNumNonMetalNeigh( inchi_Atom *atom, int cur_at, Stereo0D *pStereo, int *i_ord_LastMetal );

void e_inchi_swap ( char *a, char *b, size_t width );
int e_insertions_sort( void *base, size_t num, size_t width, int ( *compare )(const void *e1, const void *e2 ) );
int e_bCanAtomHaveAStereoBond( inchi_Atom *at, int cur_at, S_CHAR *cAtType );
int e_bCanAtomBeMiddleAllene( int cur_at, S_CHAR *cAtType );
int e_bCanAtomBeTerminalAllene( int cur_at, S_CHAR *cAtType );

/**********************************************************************************/
#define AMBIGUOUS_STEREO           1
#define AMBIGUOUS_STEREO_ERROR     32


#define AB_MAX_WELL_DEFINED_PARITY inchi_max(INCHI_PARITY_ODD, INCHI_PARITY_EVEN) /* 1, 2 => well defined parities, uncluding 'unknown' */
#define AB_MIN_WELL_DEFINED_PARITY inchi_min(INCHI_PARITY_ODD, INCHI_PARITY_EVEN) /* min(INCHI_PARITY_ODD, INCHI_PARITY_EVEN) */

#define AMBIGUOUS_STEREO           1

#define MIN_DOT_PROD 50          /* min value of at->stereo_bond_z_prod[i] to define parity */

#define ATOM_PARITY_WELL_DEF(X)     (AB_MIN_WELL_DEFINED_PARITY <= (X) && (X) <= AB_MAX_WELL_DEFINED_PARITY)
/**********************************************************************************/

#define CT_ERR_FIRST         (-30000)
#define CT_OUT_OF_RAM        (CT_ERR_FIRST- 2)  /*(-30002) */
#define CT_ISO_H_ERR         (CT_ERR_FIRST- 9)  /*(-30009) */
#define CT_CALC_STEREO_ERR   (CT_ERR_FIRST-15)  /*(-30015) */
#define CT_UNKNOWN_ERR       (CT_ERR_FIRST-18)  /*(-30018) */

#define CT_ERR_MIN CT_UNKNOWN_ERR
#define CT_ERR_MAX CT_ERR_FIRST

#define RETURNED_ERROR(nVal) (CT_ERR_MIN<=(nVal) && (nVal)<=CT_ERR_MAX)
/**********************************************************************************/
#define MAX_CUMULENE_LEN   2     /* max number of bonds in a cumulene chain - 1 */
/**********************************************************************************/
int ee_extract_ChargeRadical( char *elname, int *pnRadical, int *pnCharge )
{
    char *q, *r, *p;
    int  nCharge=0, nRad = 0, charge_len = 0, k, nVal, nSign, nLastSign=1, len;

    p = elname;

    /*  extract radicals & charges */
    while ( q = strpbrk( p, "+-^" ) ) {
        switch ( *q ) {
        case '+':
        case '-':
            for ( k = 0, nVal=0; (nSign = ('+' == q[k])) || (nSign = -('-' == q[k])); k++ ) {
                nVal += (nLastSign = nSign);
                charge_len ++;
            }
            if ( nSign = (int)strtol( q+k, &r, 10 ) ) { /*  fixed 12-5-2001 */
                nVal += nLastSign * (nSign-1);
            }
            charge_len = r - q;
            nCharge += nVal;
            break;
        /* case '.': */ /*  singlet '.' may be confused with '.' in formulas like CaO.H2O */
        case '^':
            nRad = 1; /* doublet here is 1. See below */
            charge_len = 1;
            for ( k = 1; q[0] == q[k]; k++ ) {
                nRad ++;
                charge_len ++;
            }
            break;
        }
        memmove( q, q+charge_len, strlen(q+charge_len)+1 );
    }
    len = strlen(p);
    /*  radical */
    if ( (q = strrchr( p, ':' )) && !q[1]) {
        nRad = RADICAL_SINGLET;
        q[0] = '\0';
        len --;
    } else {
        while( (q = strrchr( p, '.' )) && !q[1] ) {
            nRad ++;
            q[0] = '\0';
            len --;
        }

        nRad = nRad == 1? RADICAL_DOUBLET :
               nRad == 2? RADICAL_TRIPLET : 0;
    }
    *pnRadical = nRad;
    *pnCharge  = nCharge;
    return ( nRad || nCharge );

}
/***********************************************************************/
int ee_extract_H_atoms( char *elname, S_CHAR num_iso_H[] )
{
    int i, len, c, k, val, bExtracted = 0;
    char *q;
    i = 0;
    len = (int)strlen(elname);
    c =  UCINT elname[0];
    while ( i < len ) {
        switch ( c ) {
        case 'H':
            k = 0;
            break;
        case 'D':
            k = 2;
            break;
        case 'T':
            k = 3;
            break;
        default:
            k = -1;
            break;
        }
        q = elname+i+1; /*  pointer to the next to elname[i] character */
        c = UCINT q[0];
        if ( k >= 0 && !islower( c ) ) {
            /*  found a hydrogen */
            bExtracted ++;
            if ( isdigit( c ) ) {
                val = (int)strtol( q, &q, 10 );
                /*  q = pointer to the next to number of hydrogen atom(s) character */
            } else {
                val = 1;
            }
            num_iso_H[k] += val;
            /*  remove the hydrogen atom from the string */
            len -= (q-elname)-i;
            memmove( elname+i, q, len + 1 );
            /*  c =  UCINT elname[i]; */
        } else {
            i ++;
        }
        c =  UCINT elname[i]; /*  moved here 11-04-2002 */
    }
    return bExtracted;
}
/************************************************/
#define MAX_BOND_TYPE 4
int e_GetElType( inchi_Atom *at, int cur_atom )
{
    char szEl[ATOM_EL_LEN];
    int  nRadical, nCharge, bChargeOrRad, bH, bAddH = 0, nElType=0, bond_valence, valence, num_H=0;
    S_CHAR num_iso_H[NUM_H_ISOTOPES+1];
    S_CHAR num_bonds[MAX_BOND_TYPE];
    int  i;
    int  nRadicalValence = 0;

    if ( sizeof(at->num_iso_H)    != sizeof(num_iso_H) ||
         sizeof(at->num_iso_H[0]) != sizeof(num_iso_H[0]) ) {
        /* program error */
        return -1;
    }
    strcpy( szEl, at[cur_atom].elname);
    memset( num_iso_H, 0, sizeof(num_iso_H) );
    bChargeOrRad  = ee_extract_ChargeRadical( szEl, &nRadical, &nCharge );
    bH            = ee_extract_H_atoms( szEl, num_iso_H );
    if ( !bChargeOrRad ) {
        nRadical = at[cur_atom].radical;
        nCharge  = at[cur_atom].charge;
    }
    if ( !bH ) {
        memcpy( num_iso_H, at[cur_atom].num_iso_H, sizeof(num_iso_H) );
        if ( bAddH = (num_iso_H[0] < 0 ) ) {
            num_iso_H[0] = 0;
        }
    }
    num_H = num_iso_H[0]+num_iso_H[1]+num_iso_H[2]+num_iso_H[3];

    if (nRadical==INCHI_RADICAL_DOUBLET) {
        nRadicalValence = 1;
    } else
    if ( nRadical==INCHI_RADICAL_TRIPLET || nRadical==INCHI_RADICAL_SINGLET ){
        nRadicalValence = 2;
    }
    
    /* element type */
    if ( !strcmp( szEl, "C" ) ) {
        nElType = ElType_C;
    } else
    if ( !strcmp( szEl, "Si" ) ) {
        nElType = ElType_Si;
    } else
    if ( !strcmp( szEl, "Ge" ) ) {
        nElType = ElType_Ge;
    } else
    if ( !strcmp( szEl, "Sn" ) ) {
        nElType = ElType_Sn;
    } else
    if ( !strcmp( szEl, "B" ) ) {
        nElType = ElType_B;
    } else
    if ( !strcmp( szEl, "S" ) ) {
        nElType = ElType_S;
    } else
    if ( !strcmp( szEl, "Se" ) ) {
        nElType = ElType_Se;
    } else
    if ( !strcmp( szEl, "N" ) ) {
        nElType = ElType_N;
    } else
    if ( !strcmp( szEl, "P" ) ) {
        nElType = ElType_P;
    } else
    if ( !strcmp( szEl, "As" ) ) {
        nElType = ElType_As;
    } else
    if ( !szEl[0] ) {
        if ( 1 == num_H && (num_iso_H[0] == 1 || num_iso_H[1] == 1) ) {
            nElType = ElType_H;
        } else
        if ( 1 == num_H && num_iso_H[2] == 1 ) {
            nElType = ElType_D;
        } else
        if ( 1 == num_H && num_iso_H[3] == 1 ) {
            nElType = ElType_T;
        } else {
            return -1;
        }
    } else {
        if ( e_is_element_a_metal( szEl ) ) {
            return AtType_Metal;
        }
        return -1; /* no stereo */
    }

    /* atom type */
    memset( num_bonds, 0, sizeof(num_bonds) );
    bond_valence = 0;
    valence      = at[cur_atom].num_bonds;
    for ( i = 0; i < valence; i ++ ) {
        if ( 0 < at[cur_atom].bond_type[i] && at[cur_atom].bond_type[i] <= MAX_BOND_TYPE ) {
            num_bonds[at[cur_atom].bond_type[i]-1] ++;
        }
    }
    bond_valence = num_bonds[0] + 2*num_bonds[1] + 3*num_bonds[2];
    if ( num_bonds[3] ) {
        if ( num_bonds[3] == 2 ) {
            bond_valence += 3; /* -C= */
        } else
        if ( num_bonds[3] == 3 ) {
            bond_valence += 4; /* >C= */
        } else {
            return -1;
        }
    }
    
    switch( nElType ) {
    
    case ElType_C:
    case ElType_Si:
    case ElType_Ge:
    case ElType_Sn:

        if ( bAddH && bond_valence + num_H + (abs(nCharge)==1) + nRadicalValence == 4 ) {
            bAddH = 0; /* no H will be added */
        }
    
        if ( bond_valence == valence && (valence==4 || valence == 3 && (bAddH || num_H==1)) && !nRadical ) {
            switch( nElType ) {
            case ElType_C:
                return AtType_C4;
            case ElType_Si:
                return AtType_Si4;
            case ElType_Ge:
                return AtType_Ge4;
            case ElType_Sn:
                return AtType_Sn4;
            }
        } else
        if ( bond_valence == valence && (valence==3 || valence == 2 && (bAddH || num_H==1)) && nRadical == INCHI_RADICAL_DOUBLET ) {
            switch( nElType ) {
            case ElType_C:
                return AtType_C3;
            case ElType_Si:
                return -1;
            case ElType_Ge:
                return -1;
            case ElType_Sn:
                return AtType_Metal;
            }
        } else
        if ( bond_valence == 4 && valence == 2 && !num_H  && !nRadical ) {
            /* two double bonds or single & triple */
            switch( nElType ) {
            case ElType_C:
                return AtType_C2;
            case ElType_Si:
                return AtType_Si2;
            case ElType_Ge:
                return AtType_Ge2;
            case ElType_Sn:
                return AtType_Sn2;
            }
        } else
        if ( bond_valence == 3 && valence == 2 && !num_H && nRadical == INCHI_RADICAL_DOUBLET ) {
            /* two double bonds or single & triple */
            switch( nElType ) {
            case ElType_C:
                return AtType_C2;
            case ElType_Si:
                return -1;
            case ElType_Ge:
                return -1;
            case ElType_Sn:
                return AtType_Metal;
            }
        } else
        if ( bond_valence > valence && (valence==3 || valence == 2 && (bAddH || num_H==1)) && !nRadical ) {
            /* "bond_valence > valence" instead of "bond_valence == valence+1" to accommodate
             *  erroneouse acceptance by 1.12Beta of stereo bond case when C has valence > 5 */
            switch( nElType ) {
            case ElType_C:
                return AtType_C3;
            case ElType_Si:
                return AtType_Si3;
            case ElType_Ge:
                return AtType_Ge3;
            case ElType_Sn:
                return AtType_Sn3;
            }
        } else
        if ( bond_valence == valence+1 && bond_valence > 3 ) {
            /* trying to accommodate hypervalence in coord compounds before disconnection */
            switch( nElType ) {
            case ElType_C:
                return AtType_C3;
            case ElType_Si:
                return -1;
            case ElType_Ge:
                return -1;
            case ElType_Sn:
                return AtType_Metal;
            }
        } else
        if ( bond_valence == valence && bond_valence >= 2 && abs(nCharge) == 1 && 3 == (valence + (bAddH || num_H==1)) ) {
            /* trying to accommodate C(-)-N(+) bond that may become double after ion pair removal. added 2004-01-31 */
            switch( nElType ) {
            case ElType_C:
                return AtType_C3;
            case ElType_Si:
                return AtType_Si3;
            case ElType_Ge:
                return -1;
            case ElType_Sn:
                return AtType_Metal;
            }
        }
        return (nElType == ElType_Sn)? AtType_Metal : -1;

    case ElType_B:
        if ( bond_valence == valence && (valence==4 || valence==3 && (bAddH || num_H==1)) && nCharge == -1 && !nRadical ) {
            return AtType_B4m;
        }
        return -1;

    case ElType_S:
    case ElType_Se:
        if ( (valence == 3 && bond_valence == 3) && (nCharge == 1 || nRadical == INCHI_RADICAL_DOUBLET) ) {
            switch( nElType ) {
            case ElType_S:
                return AtType_S3p;
            case ElType_Se:
                return AtType_Se3p;
            }
        } else
        if ( (valence == 3 && bond_valence == 4) && nCharge == 0 && !nRadical ) {
            switch( nElType ) {
            case ElType_S:
                return AtType_S4;
            case ElType_Se:
                return AtType_Se4;
            }
        } else
        if ( (valence == 4 && bond_valence == 5) && (nCharge == 1 || nRadical == INCHI_RADICAL_DOUBLET) ) {
            switch( nElType ) {
            case ElType_S:
                return AtType_S5p;
            case ElType_Se:
                return AtType_Se5p;
            }
        } else
        if ( (valence == 4 && bond_valence == 6) && nCharge == 0 && !nRadical ) {
            switch( nElType ) {
            case ElType_S:
                return AtType_S6;
            case ElType_Se:
                return AtType_S6;
            }
        } else {
            return -1;
        }
    case ElType_N:
    case ElType_P:
    case ElType_As:
        if ( bAddH && bond_valence + num_H - (abs(nCharge)==1? nCharge:0) + nRadicalValence == 3 ) {
            bAddH = 0; /* no H will be added */
        } else
        if ( bAddH && bond_valence + num_H == 5 ) {
            bAddH = 0; /* no H will be added */
        }
        if ( bond_valence == valence && (valence==4 || valence==3 && (bAddH || num_H==1)) && nCharge == 1 && !nRadical ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_N4p;
            case ElType_P:
                return AtType_P4p;
            case ElType_As:
                return AtType_As4p;
            }
        } else
        if ( bond_valence == valence+1 && (valence==4 /*|| valence == 3*/) && nCharge == 0 ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_N5;
            case ElType_P:
                return AtType_P5;
            case ElType_As:
                return AtType_As5;
            }
        } else
        if ( bond_valence == valence && valence==3 && nCharge == 0 && !nRadical ) {
            switch( nElType ) {
            case ElType_N:
            { 
                AT_NUM neigh1, neigh2;
                int    j1, j2, bIn3MembRing = 0;
                for ( j1 = 0; j1 < valence && !bIn3MembRing; j1 ++ ) {
                    neigh1 = at[cur_atom].neighbor[j1];
                    for ( j2 = j1+1; j2 < valence && !bIn3MembRing; j2 ++ ) {
                        neigh2 = at[cur_atom].neighbor[j2];
                        if ( e_is_in_the_slist( at[neigh1].neighbor, neigh2, at[neigh1].num_bonds ) ) {
                            bIn3MembRing ++;
                        }
                    }
                }
                return bIn3MembRing? AtType_N3r : AtType_Nns /* -1*/;
            }
            case ElType_P:
            case ElType_As:
                return -1;
            }
        } else
        if ( bond_valence == valence+1 && valence==2 && nCharge == 0 && !nRadical ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_N3;
            case ElType_P:
            case ElType_As:
                return -1;
            }
        } else
        if ( bond_valence == valence+1 && valence==3 && nCharge == 0 && !nRadical ) {
            /* reproduce 1.12Beta bug: =N< is accepted in stereogenic bonds */
            switch( nElType ) {
            case ElType_N:
                return AtType_N3;
            case ElType_P:
            case ElType_As:
                return -1;
            }
        } else
        if ( bond_valence == valence && valence==2 && nCharge == 0 && nRadical == INCHI_RADICAL_DOUBLET ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_N3;
            case ElType_P:
            case ElType_As:
                return -1;
            }
        } else
        if ( bond_valence == valence+1 && (valence==3 || valence == 2 && (bAddH || num_H==1)) && nCharge == 1 ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_N3p;
            case ElType_P:
            case ElType_As:
                return -1;
            }
        } else
        if ( bond_valence == valence && (valence==3 || valence == 2 && (bAddH || num_H==1)) && nCharge == 1 && nRadical == INCHI_RADICAL_DOUBLET ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_N3p;
            case ElType_P:
            case ElType_As:
                return -1;
            }
        } else
        if ( bond_valence == valence && valence == 2 && (bAddH || num_H==1) && nCharge == 0 ||
             bond_valence == valence && valence == 2 && nCharge == -1 ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_Nns;
            case ElType_P:
            case ElType_As:
                return -1;
            }
        } else
        if ( (bond_valence == valence+2 || bond_valence + nRadicalValence == valence+2) && valence==3 ) {
            switch( nElType ) {
            case ElType_N:
                return AtType_Nns; /* -N<< may be in a stereogenic bond in case of double bond metal disconnection */
            case ElType_P:
            case ElType_As:
                return -1;
            }
        }
        return -1;

        case ElType_H:
            if ( valence == 1 )
                return AtType_TermH;
            break;
        case ElType_D:
            if ( valence == 1 )
                return AtType_TermD;
            break;
        case ElType_T:
            if ( valence == 1 )
                return AtType_TermT;
            break;

    }

    return -1;
}


/**********************************************************************************/
void e_inchi_swap ( char *a, char *b, size_t width )
{
    char tmp;
    if ( a != b )
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}
/**********************************************************************************/
/*  Sort by insertions */
int e_insertions_sort( void *base, size_t num, size_t width, int ( *compare )(const void *e1, const void *e2 ) )
{
  char *i, *j, *pk;
  int  num_trans = 0;
  size_t k;
  for( k=1, pk = (char*)base; k < num; k++, pk += width ) {
     for( i = pk, j = pk + width; j > (char*)base && (*compare)(i,j) > 0; j=i, i -= width ) {
        e_inchi_swap( i, j, width );
        num_trans ++;
     }
  }
  return num_trans;
}


#define    ZTYPE_DOWN     (-1)  /*  should be equal to -ZTYPE_UP */
#define    ZTYPE_NONE     0
#define    ZTYPE_UP       1     /*  should be equal to -ZTYPE_DOWN */
#define    ZTYPE_3D       3
#define    ZTYPE_EITHER   9999

/*  criteria for ill-defined */
#define MIN_ANGLE             0.10   /*  5.73 degrees */
#define MIN_SINE              0.03   /*  min edge/plane angle in case the tetrahedra has significantly different edge length */
#define MIN_ANGLE_DBOND       0.087156 /* 5 degrees = max angle considered as too small for unambiguous double bond stereo */
#define MIN_SINE_OUTSIDE      0.06   /*  min edge/plane angle to determine whether the central atom is outside of the tetrahedra */
#define MIN_SINE_SQUARE       0.125  /*  min edge/plane angle in case the tetrahedra is somewhat close to a parallelogram */
#define MIN_SINE_EDGE         0.167  /*  min sine/(min.edge) ratio to avoid undefined in case of long edges */
#define MIN_LEN_STRAIGHT      1.900  /*  min length of two normalized to 1 bonds in a straight line */
#define MAX_SINE              0.70710678118654752440084436210485 /*  1/sqrt(2)=sin(pi/4) */
#define MIN_BOND_LEN          0.000001
#define ZERO_LENGTH           MIN_BOND_LEN
#define ZERO_FLOAT            1.0e-12
#define BOND_PARITY_UNDEFINED 64
#if( STEREO_CENTER_BONDS_NORM == 1 )
#define MPY_SINE              1.00  /*  was 3.0 */
#define MAX_EDGE_RATIO        2.50   /*  max max/min edge ratio for a tetrahedra close to a parallelogram  */
#else
#define MPY_SINE              3.00
#define MAX_EDGE_RATIO        6.00   /*  max max/min edge ratio for a tetrahedra close to a parallelogram  */
#endif
/*  local prototypes */
static double e_get_z_coord( inchi_Atom* at, int cur_atom, int neigh_no,  int *nType,int bPointedEdgeStereo );
static double e_len3( const double c[] );
static double e_len2( const double c[] );
static double* e_diff3( const double a[], const double b[], double result[] );
static double* e_add3( const double a[], const double b[], double result[] );
static double* e_mult3( const double a[], double b, double result[] );
static double* e_copy3( const double a[], double result[] );
static double* e_change_sign3( const double a[], double result[] );
static double e_dot_prod3( const double a[], const double b[] );
static int e_dot_prodchar3( const S_CHAR a[], const S_CHAR b[] );
static double* e_cross_prod3( const double a[], const double b[], double result[] );
static double e_triple_prod( double a[], double b[], double c[], double *sine_value );
static double e_triple_prod_and_min_abs_sine(double at_coord[][3], double *min_sine);
static int are_3_vect_in_one_plane( double at_coord[][3], double min_sine);
static int e_triple_prod_char( inchi_Atom *at, int at_1, int i_next_at_1, S_CHAR *z_dir1,
                                    int at_2, int i_next_at_2, S_CHAR *z_dir2 );

static int e_CompDble( const void *a1, const void *a2 );
static int e_Get2DTetrahedralAmbiguity( double at_coord[][3], int bAddExplicitNeighbor );
static double e_triple_prod_and_min_abs_sine2(double at_coord[][3], double central_at_coord[], int bAddedExplicitNeighbor, double *min_sine, int *bAmbiguous);
static int e_are_4at_in_one_plane( double at_coord[][3], double min_sine);
static int e_half_stereo_bond_parity( inchi_Atom *at, int cur_at, S_CHAR *z_dir, int *bOnlyNonMetal, int bPointedEdgeStereo, Stereo0D *pStereo );
static int e_set_stereo_bonds_parity( Stereo0D *pStereo, inchi_Atom *at, int at_1, int bPointedEdgeStereo );
static int e_set_stereo_atom_parity( Stereo0D *pStereo, inchi_Atom *at, int cur_at, int bPointedEdgeStereo );
static int e_FixSb0DParities( inchi_Atom *at, Stereo0D *pStereo, int chain_length, AT_NUM at_middle,
                     int at_1, int i_next_at_1, S_CHAR z_dir1[], S_CHAR z_dir1NM[], int bOnlyNM1, int bAnomaly1NM, int parity1, int parity1NM,
                     int at_2, int i_next_at_2, S_CHAR z_dir2[], S_CHAR z_dir2NM[], int bOnlyNM2, int bAnomaly2NM, int parity2, int parity2NM );


/******************************************************************/


static double         *pDoubleForSort;

/**********************************************************************************/
double e_get_z_coord( inchi_Atom* at, int cur_atom, int neigh_no,  int *nType, int bPointedEdgeStereo )
{
    int stereo_value = at[cur_atom].bond_stereo[neigh_no];
    int stereo_type  = abs( stereo_value );
    int neigh        = (int)at[cur_atom].neighbor[neigh_no];
    double z         = at[neigh].z - at[cur_atom].z;
    int    bFlat;

    if ( bFlat = (fabs(z) < ZERO_LENGTH) ) {
        int i;
        for ( i = 0; i < at[cur_atom].num_bonds; i ++ ) {
            if ( fabs(at[cur_atom].z - at[(int)at[cur_atom].neighbor[i]].z) > ZERO_LENGTH ) {
                bFlat = 0;
                break;
            }
        }
    }

    if ( bFlat ) {
        if ( !bPointedEdgeStereo || bPointedEdgeStereo * stereo_value >= 0 ) {
            /* bPointedEdgeStereo > 0: define stereo from pointed end of the stereo bond only */
            /* bPointedEdgeStereo < 0: define stereo from wide end of the stereo bond only (case of removed H) */
            switch( stereo_type ) {
                /*  1=Up (solid triangle), 6=Down (Dashed triangle), 4=Either (zigzag triangle) */
            case 0: /*  No stereo */
                *nType = ZTYPE_NONE;
                break;
            case INCHI_BOND_STEREO_SINGLE_1UP: /*  1= Up */
                *nType = ZTYPE_UP;
                break;
            case INCHI_BOND_STEREO_SINGLE_1EITHER: /*  4 = Either */
                *nType = ZTYPE_EITHER;
                break;
            case INCHI_BOND_STEREO_SINGLE_1DOWN: /*  6 = Down */
                *nType = ZTYPE_DOWN;
                break;
            default:
                *nType = ZTYPE_NONE; /*  ignore unexpected values */
            }
            if ( stereo_value < 0 && (*nType == ZTYPE_DOWN || *nType == ZTYPE_UP) )
                *nType = -*nType;
        } else {
            *nType = ZTYPE_NONE; /* no stereo */
        }
    } else
    if ( stereo_type == INCHI_BOND_STEREO_SINGLE_1EITHER &&
         ( !bPointedEdgeStereo || bPointedEdgeStereo * stereo_value >= 0 ) ) {
        *nType = ZTYPE_EITHER; 
    } else {
        *nType = ZTYPE_3D;
    }
    return z;
}
/******************************************************************/
double e_len3( const double c[] )
{
    return sqrt( c[0]*c[0]   + c[1]*c[1]   + c[2]*c[2] );
}
/******************************************************************/
double e_len2( const double c[] )
{
    return sqrt( c[0]*c[0]   + c[1]*c[1] );
}
/******************************************************************/
double* e_diff3( const double a[], const double b[], double result[] )
{

    result[0] =  a[0] - b[0];
    result[1] =  a[1] - b[1];
    result[2] =  a[2] - b[2];

    return result;
}
/******************************************************************/
double* e_add3( const double a[], const double b[], double result[] )
{
    result[0] =  a[0] + b[0];
    result[1] =  a[1] + b[1];
    result[2] =  a[2] + b[2];

    return result;
}
/******************************************************************/
double* e_mult3( const double a[], double b, double result[] )
{
    result[0] = a[0] * b;
    result[1] = a[1] * b;
    result[2] = a[2] * b;

    return result;
}
/*************************************************************/
double* e_copy3( const double a[], double result[] )
{
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];

    return result;
}
/*************************************************************/
double* e_change_sign3( const double a[], double result[] )
{
    result[0] = -a[0];
    result[1] = -a[1];
    result[2] = -a[2];

    return result;
}
/*************************************************************/
double e_dot_prod3( const double a[], const double b[] )
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
/*************************************************************/
int e_dot_prodchar3( const S_CHAR a[], const S_CHAR b[] )
{
    int prod = ((int)a[0]*(int)b[0] + (int)a[1]*(int)b[1] + (int)a[2]*(int)b[2])/100;
    if ( prod > 100 )
        prod = 100;
    else
    if ( prod < -100 )
        prod = -100;
    return prod;
}
/*************************************************************/
double* e_cross_prod3( const double a[], const double b[], double result[] )
{
    double tmp[3];
    
    tmp[0] =  (a[1]*b[2]-a[2]*b[1]);
    tmp[1] = -(a[0]*b[2]-a[2]*b[0]);
    tmp[2] =  (a[0]*b[1]-a[1]*b[0]);

    result[0] = tmp[0];
    result[1] = tmp[1];
    result[2] = tmp[2];

    return result;
}
/*************************************************************/
double e_triple_prod( double a[], double b[], double c[], double *sine_value )
{
    double ab[3], dot_prod_ab_c, abs_c, abs_ab;
    e_cross_prod3( a, b, ab );
    /* ab[0] =  (a[1]*b[2]-a[2]*b[1]); */
    /* ab[1] = -(a[0]*b[2]-a[2]*b[0]); */
    /* ab[2] =  (a[0]*b[1]-a[1]*b[0]); */
    dot_prod_ab_c   =  e_dot_prod3( ab, c );
    /* dot_prod_ab_c   =  ab[0]*c[0] + ab[1]*c[1] + ab[2]*c[2]; */
    if ( sine_value ) {
        abs_c  = e_len3( c );
        /* abs_c  = sqrt( c[0]*c[0]   + c[1]*c[1]   + c[2]*c[2] ); */
        abs_ab = e_len3( ab );
        /* abs_ab = sqrt( ab[0]*ab[0] + ab[1]*ab[1] + ab[2]*ab[2] ); */

        if ( abs_c > 1.e-7 /* otherwise c has zero length */ && abs_ab > 1.e-7 /* otherwise a is parallel to b*/ ) {
            *sine_value = MPY_SINE * dot_prod_ab_c / ( abs_c * abs_ab);
            /*  *sine_value = dot_prod_ab_c / ( abs_c * abs_ab); */
        } else {
            *sine_value = 0.0;
        }
    }
    return dot_prod_ab_c;
}
/*************************************************************/
int e_CompDble( const void *a1, const void *a2 )
{
    double diff = pDoubleForSort[*(const int*)a1] - pDoubleForSort[*(const int*)a2];
    if ( diff > 0.0 )
        return 1;
    if ( diff < 0.0 )
        return -1;
    return 0;
}
/*************************************************************/
#define T2D_OKAY  1
#define T2D_WARN  2
#define T2D_UNDF  4
int e_Get2DTetrahedralAmbiguity( double at_coord[][3], int bAddExplicitNeighbor )
{
#define ZERO_ANGLE  0.000001
    const double one_pi = 2.0*atan2(1.0 /* y */, 0.0 /* x */);
    const double two_pi = 2.0*one_pi;
    const double dAngleAndPiMaxDiff = 2.0*atan2(1.0, sqrt(7.0)); /*  min sine between 2 InPlane bonds */
    int    nBondType[MAX_NUM_STEREO_ATOM_NEIGH], nBondOrder[MAX_NUM_STEREO_ATOM_NEIGH];
    double dBondDirection[MAX_NUM_STEREO_ATOM_NEIGH], dAngle, dAlpha, dLimit, dBisector;
    int  nNumNeigh = MAX_NUM_STEREO_ATOM_NEIGH - (bAddExplicitNeighbor != 0);
    int  i, num_Up, num_Dn, bPrev_Up, cur_len_Up, cur_first_Up, len_Up, first_Up;
    int  ret;
    int  bFixSp3Bug = 1; /* 1=> FixSp3Bug option for overlapping 2D stereo bonds */

    ret = 0;
    for ( i = 0, num_Up = num_Dn = 0; i < nNumNeigh; i ++ ) {
        dAngle = atan2( at_coord[i][1], at_coord[i][0] ); /*  range from -pi to +pi */
        if ( dAngle < 0.0 ) {
            dAngle += two_pi;
        }
        dBondDirection[i] = dAngle;
        nBondType[i] = (at_coord[i][2] > 0.0)? 1 : (at_coord[i][2] < 0.0)? -1 : 0; /* z-coord sign */
        if ( nBondType[i] > 0 ) {
            num_Up ++;
        } else
        if ( nBondType[i] < 0 ) {
            num_Dn ++;
        }
        nBondOrder[i] = i;
    }
    if ( num_Up < num_Dn ) {
        for ( i = 0; i < nNumNeigh; i ++ ) {
            nBondType[i] = -nBondType[i];
        }
        e_inchi_swap( (char*)&num_Dn, (char*)&num_Up, sizeof(num_Dn) );
    }
    if ( !num_Up ) {
        return T2D_UNDF;
    }
    /*  sort according to the bond orientations */
    pDoubleForSort = dBondDirection;
    e_insertions_sort( nBondOrder, nNumNeigh, sizeof(nBondOrder[0]), e_CompDble );
    
    /*  find the longest contiguous sequence of Up bonds */
    if ( num_Up == nNumNeigh ) {
        /*  all bonds are Up */
        len_Up   = cur_len_Up = nNumNeigh; /* added cur_len_Up initialization 1/8/2002 */
        first_Up = 0;
    } else {
        /*  at least one bond is not Up */
        cur_len_Up = len_Up = bPrev_Up = 0;
        /* prev. cycle header version ---
        for ( i = 0; 1; i ++ ) {
            if ( i >= nNumNeigh && !bPrev_Up ) {
                break;
            }
        ----------} */
        /* look at all bonds and continue (circle therough the beginning) as long as the current bond is Up */
        for ( i = 0; i < nNumNeigh || bPrev_Up; i ++ ) {
            if ( nBondType[nBondOrder[i % nNumNeigh]] > 0 ) {
                if ( bPrev_Up ) {
                    cur_len_Up ++; /* uncrement number of Up bonds in current contiguous sequence of them */
                } else {
                    bPrev_Up     = 1; /* start new contiguous sequence of Up bonds */
                    cur_len_Up   = 1;
                    cur_first_Up = i % nNumNeigh;
                }
            } else
            if ( bPrev_Up ) { /* end of contiguous sequence of Up bonds */
                if ( cur_len_Up > len_Up ) {
                    first_Up = cur_first_Up; /* store the sequence because it is longer than the ptrvious one */
                    len_Up   = cur_len_Up;
                }
                bPrev_Up = 0;
            }
        }
    }
    if ( bFixSp3Bug ) {
        /* check if the bonds with ordering numbers first_Up+len_Up and first_Up+len_Up+1 */
        /* have identical angles. In this case switch their order to enlarge the Up sequence */
        if ( nNumNeigh - len_Up >= 2 ) {
            int next1, next2;
            for ( i = 1; i < nNumNeigh - len_Up; i ++ ) {
                next2 = (first_Up+len_Up + i) % nNumNeigh; /* the 2nd after Up sequence */
                if ( nBondType[nBondOrder[next2]] > 0 ) {
                    next1 = (first_Up+len_Up) % nNumNeigh; /* the 1st after Up sequence */
                    dAngle = dBondDirection[nBondOrder[next1]] - dBondDirection[nBondOrder[next2]];
                    if ( fabs(dAngle) < ZERO_ANGLE ) {
                        e_inchi_swap( (char*)&nBondOrder[next1], (char*)&nBondOrder[next2], sizeof(nBondOrder[0]) );
                        len_Up ++;
                        break;
                    }
                }
            }
        }
        /* check whether the not-Up bond (located before the found first-Up) has */
        /* same angle as the Up bond that precedes this not-Up bond */
        if ( nNumNeigh - len_Up >= 2 ) {
            int next1, next2;
            for ( i = 1; i < nNumNeigh - len_Up; i ++ ) {
                next2 = (first_Up+nNumNeigh - i - 1 ) % nNumNeigh; /* the 2nd before Up sequence */
                if ( nBondType[nBondOrder[next2]] > 0 ) {
                    next1 = (first_Up+nNumNeigh-1) % nNumNeigh; /* the 1st before Up sequence */
                    dAngle = dBondDirection[nBondOrder[next1]] - dBondDirection[nBondOrder[next2]];
                    if ( fabs(dAngle) < ZERO_ANGLE ) {
                        e_inchi_swap( (char*)&nBondOrder[next1], (char*)&nBondOrder[next2], sizeof(nBondOrder[0]) );
                        first_Up = next1; 
                        len_Up ++;
                        break;
                    }
                }
            }
        }
    }
    /*  Turn all the bonds around the center so that */
    /*  the 1st Up bond has zero radian direction */
    dAlpha = dBondDirection[nBondOrder[first_Up]];
    for ( i = 0; i < nNumNeigh; i ++ ) {
        if ( i == nBondOrder[first_Up] ) {
            dBondDirection[i] = 0.0;
        } else {
            dAngle = dBondDirection[i] - dAlpha;
            if ( dAngle < 0.0 ) {
                dAngle += two_pi;
            }
            dBondDirection[i] = dAngle;
        }
    }

    /********************************************************
     * Process particular cases
     ********************************************************/

    switch( nNumNeigh ) {

    /************************ 3 bonds ***********************
     */
    case 3:
        switch( num_Up ) {
        /* -------------------------- 0 Up ------------ */
        case 0:
            return T2D_UNDF;
        /* -------------------------- 1 Up ------------ */
        case 1:
            if ( num_Dn ) {
#ifdef _DEBUG
                if ( num_Dn != 1 )  /*  debug only */
                    return -1;
#endif
                ret = (T2D_UNDF | T2D_WARN);
            } else {
                dAngle = dBondDirection[nBondOrder[(first_Up + 2) % nNumNeigh]] -
                         dBondDirection[nBondOrder[(first_Up + 1) % nNumNeigh]];
                if ( dAngle < 0.0 ) {
                    dAngle += two_pi;
                }
                if ( dAngle - one_pi < -MIN_ANGLE || dAngle - one_pi > MIN_ANGLE  ) {
                    ret = T2D_OKAY;
                } else {
                    ret = (T2D_UNDF | T2D_WARN);
                }
            }
            break;
        /* -------------------------- 2 Up ------------ */
        case 2:
            if ( num_Dn ) {
                dAlpha = dBondDirection[nBondOrder[(first_Up + 1) % nNumNeigh]] -
                         dBondDirection[nBondOrder[(first_Up    ) % nNumNeigh]];
                if ( dAlpha < 0.0 ) {
                    dAlpha += two_pi;
                }
                if ( dAlpha > one_pi - MIN_ANGLE ) {
                    ret = T2D_OKAY;
                } else
                if ( dAlpha < two_pi / 3.0 - MIN_ANGLE ) {
                    ret = (T2D_UNDF | T2D_WARN);
                } else {
                    /*  angle between 2 Up bonds is between 120 and 180 degrees */
                    /*  direction of the (Alpha angle bisector) + 180 degrees  */
                    dBisector = (dBondDirection[nBondOrder[(first_Up     ) % nNumNeigh]] +
                                dBondDirection[nBondOrder[(first_Up + 1 ) % nNumNeigh]] ) / 2.0 - one_pi;
                    if ( dBisector < 0.0 ) {
                        dBisector += two_pi;
                    }
                    if ( dAlpha < two_pi / 3.0 + MIN_ANGLE ) {
                        /*  dAlpha is inside ( 2pi/3 - eps, 2pi/3 + eps ) interval */
                        dLimit = MIN_ANGLE * 3.0 / 2.0;
                    } else {
                        dLimit = dAlpha * 3.0 / 2.0 - one_pi;
                    }
                    dAngle = dBondDirection[nBondOrder[(first_Up + 2 ) % nNumNeigh]];
                    if (  dBisector - dAngle < -dLimit ||
                          dBisector - dAngle >  dLimit  ) {
                        ret = (T2D_UNDF | T2D_WARN);
                    } else {
                        ret = T2D_OKAY;
                    }
                }
            } else {
                ret = T2D_OKAY;
            }

            break;
        /* -------------------------- 3 Up ------------ */
        case 3:
            ret = T2D_OKAY;
            break;
        /* -------------------------- other Up -------- */
        default:
            return -1;

        }

        break;

    /************************************** 4 bonds **************************
     */
    case 4:
        switch( num_Up ) {
        /* -------------------------- 0 Up ------------ */
        case 0:
            return T2D_UNDF;
        /* -------------------------- 1 Up ------------ */
        case 1:
            if ( num_Dn ) {
                if ( nBondType[nBondOrder[(first_Up + 2) % nNumNeigh]] < 0 ) {
                    /*
                     * Up, In Plane, Dn, In Plane. Undefined if angle between
                     * two In Plane bonds is wuthin pi +/- 2*arcsine(1/sqrt(8)) interval
                     * That is, 138.5 to 221.4 degrees; for certainty the interval is
                     * increased by 5.7 degrees at each end to
                     * 134.8 to 227.1 degrees
                     */
                    dAngle = dBondDirection[nBondOrder[(first_Up + 3) % nNumNeigh]] -
                             dBondDirection[nBondOrder[(first_Up + 1) % nNumNeigh]];
                    if ( dAngle < 0.0 ) {
                        dAngle += two_pi;
                    }
                    if ( fabs( dAngle - one_pi ) < dAngleAndPiMaxDiff + MIN_ANGLE ) {
                        ret = (T2D_UNDF | T2D_WARN); 
                    } else {
                        ret = T2D_OKAY;
                    }
                } else {
                    ret = T2D_OKAY;
                }
#ifdef _DEBUG
                if ( num_Dn != 1 )  /*  debug only */
                    return -1;
#endif
            } else {
                ret    = T2D_OKAY;
                dAngle = dBondDirection[nBondOrder[(first_Up + 3) % nNumNeigh]] -
                         dBondDirection[nBondOrder[(first_Up + 1) % nNumNeigh]];
                if ( dAngle < 0.0 ) {
                    dAngle += two_pi;
                }
                if ( dAngle < one_pi - MIN_ANGLE ) {
                    ret |= T2D_WARN;
                }
            }
            break;
        /* -------------------------- 2 Up ------------ */
        case 2:
            if ( bFixSp3Bug ) {
                /* bug fix */
                if ( len_Up == 1 ) {
                    ret = T2D_OKAY;
                } else {
                    dAngle = dBondDirection[nBondOrder[(first_Up + 3) % nNumNeigh]] -
                             dBondDirection[nBondOrder[(first_Up + 0) % nNumNeigh]];
                    dAngle = fabs(two_pi - dAngle);
                    dAlpha = dBondDirection[nBondOrder[(first_Up + 2) % nNumNeigh]] -
                             dBondDirection[nBondOrder[(first_Up + 1) % nNumNeigh]];
                    dAlpha = fabs(dAlpha);
                    if ( dAngle < 2.0 * ZERO_ANGLE && dAlpha > MIN_ANGLE ||
                         dAlpha < 2.0 * ZERO_ANGLE && dAngle > MIN_ANGLE  ) {
                        ret = (T2D_OKAY | T2D_WARN);
                    } else {
                        ret = (T2D_UNDF | T2D_WARN);
                    }
                }
            } else {
                /* original InChI v. 1 bug */
                if ( cur_len_Up == 1 ) {
                    ret = T2D_OKAY;
                } else {
                    ret = (T2D_UNDF | T2D_WARN);
                }
            }
            break;
            
        /* -------------------------- 3 Up ------------ */
        case 3:
            ret    = T2D_OKAY;
            dAngle = dBondDirection[nBondOrder[(first_Up + 2) % nNumNeigh]] -
                     dBondDirection[nBondOrder[(first_Up + 0) % nNumNeigh]];
            if ( dAngle < 0.0 ) {
                dAngle += two_pi;
            }
            if ( dAngle < one_pi - MIN_ANGLE ) {
                ret |= T2D_WARN;
            }
            break;
        /* -------------------------- 4 Up ------------ */
        case 4:
            ret = (T2D_UNDF | T2D_WARN);
            break;
        /* -------------------------- other Up -------- */
        default:
            return -1; /*  program error */
        }

        if ( ret == T2D_OKAY ) {
            /*  check whether all bonds are inside a less than 180 degrees sector */
            for ( i = 0; i < nNumNeigh; i ++ ) {
                dAngle = dBondDirection[nBondOrder[(i + nNumNeigh - 1) % nNumNeigh]] -
                         dBondDirection[nBondOrder[ i % nNumNeigh]];
                if ( dAngle < 0.0 ) {
                    dAngle += two_pi;
                }
                if ( dAngle < one_pi - MIN_ANGLE ) {
                    ret |= T2D_WARN;
                    break;
                }
            }
        }

        break;
    /*************************** other nuber of bonds ******************/
    default:
        return -1; /*  error */
    }

    return ret;

#undef ZERO_ANGLE
}
/*************************************************************/
double e_triple_prod_and_min_abs_sine2(double at_coord[][3], double central_at_coord[], int bAddedExplicitNeighbor, double *min_sine, int *bAmbiguous)
{
    double min_sine_value=9999.0, sine_value, min_edge_len, max_edge_len, min_edge_len_NoExplNeigh, max_edge_len_NoExplNeigh;
    double s0, s1, s2, s3, e01, e02, e03, e12, e13, e23, tmp[3], e[3][3];
    double prod, ret, central_prod[4];
    int    bLongEdges;

    if ( !min_sine ) {
        return e_triple_prod( at_coord[0], at_coord[1], at_coord[2], NULL );
    }
    
    ret = e_triple_prod( at_coord[0], at_coord[1], at_coord[2], &sine_value );
    sine_value = MPY_SINE * fabs( sine_value );
    
    e_diff3( at_coord[1], at_coord[0], e[2] );
    e_diff3( at_coord[0], at_coord[2], e[1] );
    e_diff3( at_coord[2], at_coord[1], e[0] );
    
    /*  lengths of the 6 edges of the tetrahedra */
    e03 = e_len3( at_coord[0] ); /* 1 */
    e13 = e_len3( at_coord[1] );
    e23 = e_len3( at_coord[2] ); /* includes added neighbor if bAddedExplicitNeighbor*/
    e02 = e_len3( e[1] );        /* includes added neighbor if bAddedExplicitNeighbor*/
    e12 = e_len3( e[0] );        /* includes added neighbor if bAddedExplicitNeighbor*/
    e01 = e_len3( e[2] );        
    
    /*  min & max edge length */
    max_edge_len =
    min_edge_len = e03;

    if ( min_edge_len > e13 )
        min_edge_len = e13;
    if ( min_edge_len > e01 )
        min_edge_len = e01;
    min_edge_len_NoExplNeigh = min_edge_len;

    if ( min_edge_len > e23 )
        min_edge_len = e23;
    if ( min_edge_len > e02 )
        min_edge_len = e02;
    if ( min_edge_len > e12 )
        min_edge_len = e12;

    if ( max_edge_len < e13 )
        max_edge_len = e13;
    if ( max_edge_len < e01 )
        max_edge_len = e01;
    max_edge_len_NoExplNeigh = max_edge_len;

    if ( max_edge_len < e23 )
        max_edge_len = e23;
    if ( max_edge_len < e02 )
        max_edge_len = e02;
    if ( max_edge_len < e12 )
        max_edge_len = e12;
    
    if ( !bAddedExplicitNeighbor ) {
        min_edge_len_NoExplNeigh = min_edge_len;
        max_edge_len_NoExplNeigh = max_edge_len;
    }

    bLongEdges = bAddedExplicitNeighbor? 
                  ( max_edge_len_NoExplNeigh < MAX_EDGE_RATIO * min_edge_len_NoExplNeigh ) :
                  ( max_edge_len             < MAX_EDGE_RATIO * min_edge_len );

    if ( sine_value > MIN_SINE && ( min_sine || bAmbiguous ) ) {
        if ( min_sine ) {
            prod = fabs( ret );
            /*  tetrahedra height = volume(prod) / area of a plane(cross_prod) */
            /*  (instead of a tetrahedra calculate parallelogram/parallelepiped area/volume) */

            /*  4 heights from each of the 4 vertices to the opposite plane */
            s0  = prod / e_len3( e_cross_prod3( at_coord[1], at_coord[2], tmp ) );
            s1  = prod / e_len3( e_cross_prod3( at_coord[0], at_coord[2], tmp ) );
            s2  = prod / e_len3( e_cross_prod3( at_coord[0], at_coord[1], tmp ) );
            s3  = prod / e_len3( e_cross_prod3( e[0], e[1], tmp ) );
            /*  abs. value of a sine of an angle between each tetrahedra edge and plane */
            /*  sine = height / edge length */
            if ( (sine_value = s0/e01) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s0/e02) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s0/e03) < min_sine_value )
                min_sine_value = sine_value;

            if ( (sine_value = s1/e01) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s1/e12) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s1/e13) < min_sine_value )
                min_sine_value = sine_value;

            if ( (sine_value = s2/e02) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s2/e12) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s2/e23) < min_sine_value )
                min_sine_value = sine_value;

            if ( (sine_value = s3/e03) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s3/e13) < min_sine_value )
                min_sine_value = sine_value;
            if ( (sine_value = s3/e23) < min_sine_value )
                min_sine_value = sine_value;
            /*  actually use triple sine */
            *min_sine = sine_value = MPY_SINE * min_sine_value;
        }

        if ( bAmbiguous && sine_value >= MIN_SINE ) {
            /*  check whether the central atom is outside the tetrahedra (0,0,0), at_coord[0,1,2] */
            /*  compare the tetrahedra volume and the volume of a tetrahedra having central_at_coord[] vertex */
            int i;
            e_diff3( central_at_coord, at_coord[0], tmp );
            central_prod[0] = e_triple_prod( at_coord[0], at_coord[1], central_at_coord, NULL );
            central_prod[1] = e_triple_prod( at_coord[1], at_coord[2], central_at_coord, NULL );
            central_prod[2] = e_triple_prod( at_coord[2], at_coord[0], central_at_coord, NULL );
            central_prod[3] = e_triple_prod( e[2], e[1], tmp, NULL );
            for ( i = 0; i <= 3; i ++ ) {
                if ( central_prod[i] / ret < -MIN_SINE_OUTSIDE ) {
                    *bAmbiguous |= AMBIGUOUS_STEREO;
                    break;
                }
            }
        }
#if( STEREO_CENTER_BONDS_NORM == 1 )        
        
        if ( bLongEdges && !bAddedExplicitNeighbor && max_edge_len >= MIN_LEN_STRAIGHT ) {
            /*  possible planar tetragon */
            if ( sine_value < MIN_SINE_SQUARE ) {
                *min_sine = MIN_SINE / 2.0; /*  force parity to be undefined */
                if ( bAmbiguous && !*bAmbiguous ) {
                    *bAmbiguous |= AMBIGUOUS_STEREO;
                }
            }
        }
        
        if ( bLongEdges && sine_value < MIN_SINE_SQUARE && sine_value < MIN_SINE_EDGE * min_edge_len_NoExplNeigh ) {
            *min_sine = MIN_SINE / 2.0; /*  force parity to be undefined */
            if ( bAmbiguous && !*bAmbiguous ) {
                *bAmbiguous |= AMBIGUOUS_STEREO;
            }
        }
#endif

    } else
    if ( min_sine ) {
        *min_sine = sine_value;
    }
    
    return ret;
}
/*************************************************************/
double e_triple_prod_and_min_abs_sine(double at_coord[][3], double *min_sine)
{
    double min_sine_value=9999.0, sine_value;
    double prod=0.0;

    if ( !min_sine ) {
        return e_triple_prod( at_coord[0], at_coord[1], at_coord[2], NULL );
    }
    
    prod = e_triple_prod( at_coord[0], at_coord[1], at_coord[2], &sine_value );
    sine_value = fabs( sine_value );
    min_sine_value = inchi_min( min_sine_value, sine_value );
    
    prod = e_triple_prod( at_coord[1], at_coord[2], at_coord[0], &sine_value );
    sine_value = fabs( sine_value );
    min_sine_value = inchi_min( min_sine_value, sine_value );
    
    prod = e_triple_prod( at_coord[2], at_coord[0], at_coord[1], &sine_value );
    sine_value = fabs( sine_value );
    min_sine_value = inchi_min( min_sine_value, sine_value );

    *min_sine = min_sine_value;
    
    return prod;
}
/*************************************************************/
/*  Find if point (0,0,0)a and 3 atoms are in one plane */
int are_3_vect_in_one_plane( double at_coord[][3], double min_sine)
{
    double actual_min_sine;
    double prod;
    prod = e_triple_prod_and_min_abs_sine( at_coord, &actual_min_sine);
    return actual_min_sine <= min_sine;
}
/*************************************************************/
/*  Find if 4 atoms are in one plane */
int e_are_4at_in_one_plane( double at_coord[][3], double min_sine)
{
    double actual_min_sine, min_actual_min_sine;
    double coord[3][3], prod;
    int i, k, j;
    for ( k = 0; k < 4; k ++ ) { /* cycle added 4004-08-15 */
        for ( i = j = 0; i < 4; i ++ ) {
            if ( i != k ) {
                e_diff3( at_coord[i], at_coord[k], coord[j] );
                j ++;
            }
        }
        prod = e_triple_prod_and_min_abs_sine( coord, &actual_min_sine);
        if ( !k || actual_min_sine < min_actual_min_sine ) {
            min_actual_min_sine = actual_min_sine;
        }
    }
    return min_actual_min_sine <= min_sine;
}
/*************************************************************/
int e_triple_prod_char( inchi_Atom *at, int at_1, int i_next_at_1, S_CHAR *z_dir1,
                                    int at_2, int i_next_at_2, S_CHAR *z_dir2 )
{
    inchi_Atom *at1, *at2;
    double    pnt[3][3], len;
    int       i;
    int       ret = 0;

    at1 = at + at_1;
    at2 = at + at[at_1].neighbor[i_next_at_1];

    pnt[0][0] = at2->x - at1->x;
    pnt[0][1] = at2->y - at1->y;
    pnt[0][2] = at2->z - at1->z;

    at2 = at + at_2;
    at1 = at + at[at_2].neighbor[i_next_at_2];

    pnt[1][0] = at2->x - at1->x; 
    pnt[1][1] = at2->y - at1->y; 
    pnt[1][2] = at2->z - at1->z;
/*
 *  resultant pnt vector directions:
 *
 *         pnt[0]              pnt[1]
 *
 *   [at_1]---->[...]    [...]---->[at_2]
 *
 *  
 *  e_add3 below: (pnt[0] + pnt[1]) -> pnt[1]
 */
    e_add3( pnt[0], pnt[1], pnt[1] );



    for ( i = 0; i < 3; i ++ ) {
        pnt[0][i] = (double)z_dir1[i];
        pnt[2][i] = (double)z_dir2[i];
    }
    for ( i = 0; i < 3; i ++ ) {
        len = e_len3( pnt[i] );
        if ( len < MIN_BOND_LEN ) {
            goto exit_function; /*  too short bond */
        }
        e_mult3( pnt[i], 1.0/len, pnt[i] );
    }
    len = 100.0*e_triple_prod(pnt[0], pnt[1], pnt[2], NULL );
/*
 *   ^ pnt[0]
 *   |                         The orientation on this diagram
 *   |                         produces len = -100
 *  [at_1]------>[at_2]
 *        pnt[1]    /
 *                 /
 *                / pnt[2]  (up from the plane)
 *               v
 *
 * Note: len is invariant upon at_1 <--> at_2 transposition because
 *       triple product changes sign upon pnt[0]<-->pnt[2] transposition and
 *       triple product changes sign upon pnt[1]--> -pnt[1] change of direction:
 *
 * e_triple_prod(pnt[0],  pnt[1], pnt[2], NULL ) =
 * e_triple_prod(pnt[2], -pnt[1], pnt[0], NULL )
 *
 */
    
    ret = len >= 0.0? (int)floor(len+0.5) : -(int)floor(0.5-len);

exit_function:

    return ret;
}

#ifdef NEVER
/********************************************************************************************/
int bCanInpAtomBeAStereoCenter( inchi_Atom *at, int cur_at )
{
    
/*************************************************************************************
 * current version
 *************************************************************************************
 *       Use #define to split the stereocenter description table into parts
 *       to make it easier to read
 *
 *                      --------- 4 single bonds stereocenters -------
 *                                                                    
 *                       |       |       |       |      |       |     
 *                      -C-     -Si-    -Ge-    -Sn-   >As[+]  >B[-]  
 *                       |       |       |       |      |       |     
 */
#define SZELEM1         "C\000","Si",   "Ge",   "Sn",   "As", "B\000",
#define CCHARGE1         0,      0,      0,      0,      1,   -1,    
#define CNUMBONDSANDH1   4,      4,      4,      4,      4,    4,    
#define CCHEMVALENCEH1   4,      4,      4,      4,      4,    4,    
#define CHAS3MEMBRING1   0,      0,      0,      0,      0,    0,    
#define CREQUIRDNEIGH1   0,      0,      0,      0,      3,    0,    
/*
 *                      --------------- S, Se stereocenters ----------
 *                                                                                
 *                               |       |      ||             |     |     ||     
 *                      -S=     =S=     -S[+]   >S[+]   -Se=  =Se=  -Se[+] >Se[+] 
 *                       |       |       |       |       |     |     |      |     
 */
#define SZELEM2         "S\000","S\000","S\000","S\000","Se", "Se", "Se",  "Se",  
#define CCHARGE2         0,      0,      1,      1,      0,    0,    1,     1,    
#define CNUMBONDSANDH2   3,      4,      3,      4,      3,    4,    3,     4,    
#define CCHEMVALENCEH2   4,      6,      3,      5,      4,    6,    3,     5,    
#define CHAS3MEMBRING2   0,      0,      0,      0,      0,    0,    0,     0,    
#define CREQUIRDNEIGH2   3,      3,      3,      3,      3,    3,    3,     3,
/*
 *                      ------------------ N, P stereocenters --------
 *
 *                                      X---Y                  
 *                        |      |       \ /     |       |     
 *                       =N-    >N[+]     N     >P[+]   =P-    
 *                        |      |        |      |       |     
 */
#define SZELEM3         "N\000","N\000","N\000","P\000","P\000",
#define CCHARGE3         0,      1,      0,      1,      0,     
#define CNUMBONDSANDH3   4,      4,      3,      4,      4,     
#define CCHEMVALENCEH3   5,      4,      3,      4,      5,     
#define CHAS3MEMBRING3   0,      0,      1,      0,      0,     
#define CREQUIRDNEIGH3   3,      3,      1,      3,      3,     



    static char        szElem[][3]={ SZELEM1         SZELEM2         SZELEM3        };
    static S_CHAR        cCharge[]={ CCHARGE1        CCHARGE2        CCHARGE3       };
    static S_CHAR  cNumBondsAndH[]={ CNUMBONDSANDH1  CNUMBONDSANDH2  CNUMBONDSANDH3 };
    static S_CHAR  cChemValenceH[]={ CCHEMVALENCEH1  CCHEMVALENCEH2  CCHEMVALENCEH3 };
    static S_CHAR  cHas3MembRing[]={ CHAS3MEMBRING1  CHAS3MEMBRING2  CHAS3MEMBRING3 };
    static S_CHAR  cRequirdNeigh[]={ CREQUIRDNEIGH1  CREQUIRDNEIGH2  CREQUIRDNEIGH3 };

    static int n = sizeof(szElem)/sizeof(szElem[0]);
    /* reqired neighbor types (bitmap):
       0 => check bonds only
       1 => no terminal hydrogen atom neighbors
       2 => no terminal -X and -XH together (don't care the bond type, charge, radical)
            (X = tautomeric endpoint atom)
       Note: whenever cChemValenceH[] > cNumBondsAndH[]
             the tautomeric and/or alternating bonds
             are permitted

    */
    int i, ret = 0;
    for ( i = 0; i < n; i++ ) {
        if ( !strcmp( at[cur_at].elname, szElem[i]) &&
             at[cur_at].charge == cCharge[i] &&
             (!at[cur_at].radical || at[cur_at].radical == 1) &&
             at[cur_at].num_bonds +  inchi_NUMH(at,cur_at) == cNumBondsAndH[i] &&
             at[cur_at].chem_bonds_valence+inchi_NUMH(at,cur_at) == cChemValenceH[i] &&
             (cHas3MembRing[i]? is_atom_in_3memb_ring( at, cur_at ) : 1) &&
             e_bInpAtomHasRequirdNeigh ( at, cur_at, cRequirdNeigh[i], cChemValenceH[i]-cNumBondsAndH[i]) ) {
            ret = cNumBondsAndH[i];
            break;
        }
    }
    return ret;
}

#endif  /* NEVER */
/********************************************************************************************/
int e_bCanInpAtomBeAStereoCenter( int cur_at, S_CHAR *cAtType )
{
    switch ( cAtType[cur_at] ) {
    case AtType_C4  :
    case AtType_Si4 :
    case AtType_Ge4 :
    case AtType_Sn4 :
    case AtType_B4m :
    case AtType_S6  :
    case AtType_S5p :
    case AtType_Se6 :
    case AtType_Se5p:
    case AtType_N5  :
    case AtType_N4p :
    case AtType_P4p :
    case AtType_P5  :
    case AtType_As4p:
    case AtType_As5 :
        return 4;

    case AtType_S3p :
    case AtType_Se3p:
    case AtType_S4  :
    case AtType_Se4 :
    case AtType_N3r :
        return 3;
    }
    return 0;
}
/****************************************************************/
/*  used for atoms adjacent to stereogenic bonds only */
int e_bCanAtomHaveAStereoBond( inchi_Atom *at, int cur_at, S_CHAR *cAtType )
{
    int i, neigh, nNumFound;
    char *p;
    static char sNeigh[] = "O;S;Se;Te;";
    switch ( cAtType[cur_at] ) {
    case AtType_C3 :
    case AtType_Si3:
    case AtType_Ge3:
    case AtType_Sn3:
    case AtType_N3p:
    case AtType_N3 :
        return 3;
    case AtType_Nns:
        /*
                  |
           found =N=

           if it has one neighbor =O or =S or =Se or =Te then return 3, otherwise return 1

         */
        nNumFound = 0;
        for ( i = 0; i < at[cur_at].num_bonds; i ++ ) {
            if ( at[cur_at].bond_type[i] == INCHI_BOND_TYPE_DOUBLE ) {
                neigh = (int)at[cur_at].neighbor[i];
                if ( (p = strstr(sNeigh, at[neigh].elname)) &&
                     ';' == p[strlen(at[neigh].elname)] ) {
                     nNumFound ++;
                }
            }
        }
        return nNumFound==1? 3 : 1;
    }
    return 0;
}
/****************************************************************/
int e_bCanAtomBeMiddleAllene( int cur_at, S_CHAR *cAtType )
{
    switch ( cAtType[cur_at] ) {
    case AtType_C2 :
    case AtType_Si2:
    case AtType_Ge2:
    case AtType_Sn2:
        return 2;
    }
    return 0;
}
/****************************************************************/
int e_bCanAtomBeTerminalAllene( int cur_at, S_CHAR *cAtType )
{
    switch ( cAtType[cur_at] ) {
    case AtType_C3 :
    case AtType_Si3:
    case AtType_Ge3:
    case AtType_Sn3:
        return 3;
    }
    return 0;
}
/*******************************************************************************************/
int e_FixSb0DParities( inchi_Atom *at, Stereo0D *pStereo, int chain_length, AT_NUM at_middle,
                     int at_1, int i_next_at_1, S_CHAR z_dir1[], S_CHAR z_dir1NM[], int bOnlyNM1, int bAnomaly1NM, int parity1, int parity1NM,
                     int at_2, int i_next_at_2, S_CHAR z_dir2[], S_CHAR z_dir2NM[], int bOnlyNM2, int bAnomaly2NM, int parity2, int parity2NM )
{
    int             i_neigh_at_1, i_neigh_at_2, i, j1, j2;
    int             j_neigh_at_1, j_neigh_at_2, j_next_at_1, j_next_at_2; /* positions after metal removal */
    inchi_Stereo0D *stereo0D = NULL;
    int             dot_prod_z = 0, dot_prod_zNM = 0, parity, parityNM;

    if ( (!parity1 || !parity2) && (!parity1NM || !parity2NM) )
        return 0;

    if ( ATOM_PARITY_WELL_DEF( parity1 ) && ATOM_PARITY_WELL_DEF( parity2 ) ) {
        /*  find how the whole bond parity depend on geometry */
        /*  if dot_prod_z < 0 then bond_parity := 3-bond_parity */
        /*  can be done only for a well-defined geometry */
        /********************************************************************
         *  Case of bOnlyNonMetal: stereobond end atoms have valence > 3
         *  therefore they will not be recognized until metals have
         *  been disconnected. Until then the assigned parities will
         *  not change: ReconcileCmlIncidentBondParities() in the dll
         *  ignores hypervanlence atoms marked as belonging to a stereobond.
         *  Explicit H are removed only after metal diconnection.
         *
         *  However, there is a possibility that the normalization
         *  removes an explicit H thus reducing the valence to acceptable
         *  value for the bond to be treated as stereogenic.
         *  In this case a wrong INChI stereolayer for the reconnected
         *  structure will be produced.
         *
         *  After the metal disconnection ReconcileAllCmlBondParities()
         *  will be called to reconcile newly appearing stereobonds
         ********************************************************************/

        dot_prod_z  = (chain_length%2)? 
                       e_triple_prod_char( at, at_1, i_next_at_1, z_dir1, at_2, i_next_at_2, z_dir2 ) :
                       e_dot_prodchar3(z_dir1, z_dir2);

        if ( abs(dot_prod_z) < MIN_DOT_PROD ) {
            /*  The geometry is not well-defined */
            parity1 = parity2 = INCHI_PARITY_UNDEFINED;
            dot_prod_z = 0;
        }
    
    }
    if ( ATOM_PARITY_WELL_DEF( parity1NM ) && ATOM_PARITY_WELL_DEF( parity2NM ) ) {
        /*  find how the whole bond parity depend on geometry */
        /*  if dot_prod_z < 0 then bond_parity := 3-bond_parity */
        /*  can be done only for a well-defined geometry */
        /********************************************************************
         *  Case of bOnlyNonMetal: stereobond end atoms have valence > 3
         *  therefore they will not be recognized until metals have
         *  been disconnected. Until then the assigned parities will
         *  not change: ReconcileCmlIncidentBondParities() in the dll
         *  ignores hypervanlence atoms marked as belonging to a stereobond.
         *  Explicit H are removed only after metal diconnection.
         *
         *  However, there is a possibility that the normalization
         *  removes an explicit H thus reducing the valence to acceptable
         *  value for the bond to be treated as stereogenic.
         *  In this case a wrong INChI stereolayer for the reconnected
         *  structure will be produced.
         *
         *  After the metal disconnection ReconcileAllCmlBondParities()
         *  will be called to reconcile newly appearing stereobonds
         ********************************************************************/

        dot_prod_zNM  = (chain_length%2)? 
                       e_triple_prod_char( at, at_1, i_next_at_1, z_dir1NM, at_2, i_next_at_2, z_dir2NM ) :
                       e_dot_prodchar3(z_dir1NM, z_dir2NM);

        if ( abs(dot_prod_zNM) < MIN_DOT_PROD ) {
            /*  The geometry is not well-defined */
            parity1NM = parity2NM = INCHI_PARITY_UNDEFINED;
            dot_prod_zNM = 0;
        }
    
    }

    i_neigh_at_1 = i_neigh_at_2 = -1;
    j_neigh_at_1 = j_neigh_at_2 = j_next_at_1 = j_next_at_2 = -1;

    /************************************************************************
     *
     * The possibly stereogenic bond is defined by atoms at_1, at_2 and
     * positions in their adjacency lists i_next_at_1, i_next_at_2.
     *
     * We need one more neighbor of at_1 and at_2 to establist cis or trans.
     * The current API allows only same neighbors for both connected and
     * metal-disconnected (or non-metal, NM) structures. Therefore if an
     * atom has both metal and non-metal neighbors, select the non-metal one.
     *
     * In case of bOnlyNM we have to use ordering numbers of the bonds
     * counted as if all metals have been disconnected.
     *
     * In all other cases we use real ordering numbers of the bonds
     * to calculate the parities.
     *
     * The input parities meaning:
     * ===========================
     * EVEN=2: the atoms in the adjacency list are arranged clockwise as
     *         seen from the arrowhead of the z_dir vector
     *
     * The output parities meaning:
     * ============================
     * EVEN=2: trans geometry. For more details see the API description.
     *
     **************************************************************************/

    if ( bOnlyNM1 || ATOM_PARITY_WELL_DEF( parity1NM ) ) {
        /* disconnected metal case */
        for ( i = j1 = 0; i < at[at_1].num_bonds; i ++ ) {
            if ( IS_METAL(pStereo->cAtType[at[at_1].neighbor[i]]) ) {
                continue;
            }
            if ( i == i_next_at_1 ) {
                j_next_at_1 = j1; /* position of the stereo bond in adjacency list
                                     when metals are disconnected */
            } else
            if ( i_neigh_at_1 < 0 ) {
                i_neigh_at_1 = i;  /* position of the first non-metal neighbor (metal is connected) */
                j_neigh_at_1 = j1; /* position of the first non-metal neighbor (metal is disconnected) */
            }
            j1 ++;
        }
    } else {
        /* no metal case */
        j_neigh_at_1 = i_neigh_at_1 = (i_next_at_1==0);
        j_next_at_1  = i_next_at_1;
        j1           = at[at_1].num_bonds;
    }

    if ( bOnlyNM2 || ATOM_PARITY_WELL_DEF( parity2NM ) ) {
        for ( i = j2 = 0; i < at[at_2].num_bonds; i ++ ) {
            if ( IS_METAL(pStereo->cAtType[at[at_2].neighbor[i]]) ) {
                continue;
            }
            if ( i == i_next_at_2 ) {
                j_next_at_2 = j2;
            } else
            if ( i_neigh_at_2 < 0 ) {
                i_neigh_at_2 = i;
                j_neigh_at_2 = j2;
            }
            j2 ++;
        }
    } else {
        j_neigh_at_2 = i_neigh_at_2 = (i_next_at_2==0);
        j_next_at_2  = i_next_at_2;
        j2           = at[at_2].num_bonds;
    }

    if ( i_neigh_at_1 < 0 || i_neigh_at_2 < 0 ||
         j_neigh_at_1 < 0 || j_neigh_at_2 < 0 ||
         j_next_at_1  < 0 || j_next_at_2  < 0 ) {
        return 0; /* debugging */
    }
    if ( parity1 || parity1NM ) {
        /* create a new stereo descriptor */
        stereo0D = e_GetNewStereo( pStereo );
        if ( !stereo0D ) {
            return -1; /* error */
        }
        /* choose the smallest (earliest in the adjacency list) neighbor of at_1 */
        stereo0D->neighbor[0] = at[at_1].neighbor[i_neigh_at_1];
        /* stereibond atoms */
        stereo0D->neighbor[1] = at_1;
        stereo0D->neighbor[2] = at_2;
        /* choose the smallest neighbor of at_2 */
        stereo0D->neighbor[3] = at[at_2].neighbor[i_neigh_at_2];
        stereo0D->central_atom = at_middle;
        stereo0D->type = (chain_length % 2)? INCHI_StereoType_Allene : INCHI_StereoType_DoubleBond;

        if ( dot_prod_z || dot_prod_zNM ) {

            int i_parity1 = i_neigh_at_1 + i_next_at_1 + (i_neigh_at_1 > i_next_at_1);
            int j_parity1 = j_neigh_at_1 + j_next_at_1 + (j_neigh_at_1 > j_next_at_1);
            int i_parity2 = i_neigh_at_2 + i_next_at_2 + (i_neigh_at_2 > i_next_at_2);
            int j_parity2 = j_neigh_at_2 + j_next_at_2 + (j_neigh_at_2 > j_next_at_2);
            
            /* regular (connected) parity; parity1 and parity2 are well-defined only together */
            if ( ATOM_PARITY_WELL_DEF( parity1 ) ) {
                parity = 2 - ( parity1 + parity2 + (dot_prod_z < 0) +
                               ((bOnlyNM1)? j_parity1 : i_parity1) +
                               ((bOnlyNM2)? j_parity2 : i_parity2) ) % 2;
            } else {
                parity = parity1;
            }


            /* disconnected parity; parity1NM and parity2NM are well-defined only together */
            if ( ATOM_PARITY_WELL_DEF( parity1NM ) ) {
                parityNM = 2 - ( parity1NM + parity2NM + (dot_prod_zNM < 0) +
                                 ((bOnlyNM1||bAnomaly1NM)? j_parity1 : i_parity1) +
                                 ((bOnlyNM2||bAnomaly2NM)? j_parity2 : i_parity2) ) % 2;
            } else {
                parityNM = parity1NM;
            }
        } else {
            parity   = parity1;
            parityNM = parity1NM;
        }
        stereo0D->parity = parity;
        if ( parityNM != parity || bAnomaly1NM || bAnomaly2NM ) {
            stereo0D->parity |= (parityNM << SB_PARITY_SHFT);
        }

        return stereo0D->parity;
    }
    return 0;
}
/**************************************************************************/
int e_nNumNonMetalNeigh( inchi_Atom *atom, int cur_at, Stereo0D *pStereo, int *i_ord_LastMetal )
{
    int          j, nNumNonMetal;
    inchi_Atom *at  = atom + cur_at;
    int         val = at->num_bonds;
    int         atype;

    *i_ord_LastMetal = -1;
    nNumNonMetal     = 0;

    atype = pStereo->cAtType[cur_at];
    
    if ( IS_METAL(atype) )
        return 0;
    for ( j = 0; j < val; j ++ ) {
        atype = pStereo->cAtType[(int)at->neighbor[j]];
        if ( IS_METAL(atype) )
            *i_ord_LastMetal = j;
        else
            nNumNonMetal ++;
    }
    return nNumNonMetal;
}
/*======================================================================================================
 
e_half_stereo_bond_parity() General Description:

    A) find projections of 3 bonds on a reasonable plane defined
       by a vector z_dir perpendicular to the plane
    B) calculate parity (parity=2[EVEN]: neighbors are clockwise as seen from the arrowhead of z_dir[] vector)

e_half_stereo_bond_parity() Detailed Description:

    1) Find at_coord[] = vectors from the central atoms to its neighbors
    2) If only 2 neighbors are present, then create a reasonable 3rd neighbor
       (an implicit H or a fictitious atom in case of =NX) coordinates
    3) Normalize at_coord[] to unit length
    4) Find unit vector pnt[2] perpendicular to the plane containing
       at_coord[] arrow ends.
       Even though it is not necessary, make z-coordinate of pnt[2] positive.
       ** pnt[2] has the new z-axis direction **
    5) Let pnt[0] = perpendicular to pnt[2] component of at_coord[0];
       Normalize pnt[0] to unit length.
       ** pnt[0] has the new x-axis direction **
    6) Let pnt[1] = pnt[2] x pnt[0] (cross-product);
       ** pnt[1] has the new y-axis direction **
    7) Find at_coord[] in the new xyz-basis and normalize their xy-projections
       to a unit length
    8) In the new xy-plane find (counterclockwise) angles:
       tmp1 = (from at_coord[0] to at_coord[1])
       tmp2 = (from at_coord[0] to at_coord[2])
    9) Calculate the parity: if tmp1 < tmp2 then 1 (odd) else 2 (even)
       (even: looking from the arrow end of the new z-axis, 0, 1, and 2 neighbors
        are in clockwise order)
   10) Calculate z_dir = 100*pnt[2].
   
   Note1. If z_dir vectors of atoms located at the opposite ends of a double bond have approximately
          opposite directions (that is, their dot-product is negative) then the parity of the
          stereogenic bond calculated from half-bond-parities should be inverted

   Note2. In case of a tetrahedral cumulene a triple product (z_dir1, (1->2), z_dir2) is used instead
          of the dot-product. (1->2) is a vector from the atom#1 to the atom #2. This triple product
          is invariant with respect to the atom numbering because it does not change upon (1,2)
          permutation.
  
  Stereo ambiguity in case of 2 neighbors:
  ----------------------------------------
  Undefined: single-double bond angle > pi - arcsin(0.03) = 178.28164199834454285275613218975 degrees
  Ambiguous: single-double bond angle > 175 degrees = pi - 0.087156 Rad
 
   Return values 
   (cases: I=only in case of isotopic H atoms the neighbors are different,
           N=in case of non-isotopic H atoms the neighbors are different)

  -4 = INCHI_PARITY_UNDEFINED => atom is adjacent to a stereogenic bond, but the geometry is undefined, I
  -3 = INCHI_PARITY_UNKNOWN => atom is adjacent to a stereogenic bond, but the geometry is not known to the iuser, I
  -2 =-INCHI_PARITY_EVEN => parity of an atom adjacent to a stereogenic bond, I
  -1 =-INCHI_PARITY_ODD  => parity of an atom adjacent to a stereogenic bond, I
   0 = INCHI_PARITY_NONE => the atom is not adjacent to a stereogenic bond
   1 = INCHI_PARITY_ODD  => parity of an atom adjacent to a stereogenic bond, N&I
   2 = INCHI_PARITY_EVEN => parity of an atom adjacent to a stereogenic bond, N&I
   3 = INCHI_PARITY_UNKNOWN => atom is adjacent to a stereogenic bond, but the geometry is not known to the iuser, N&I
   4 = INCHI_PARITY_UNDEFINED => atom is adjacent to a stereogenic bond, but the geometry is undefined, N&I


=====================================================================================================*/

int e_half_stereo_bond_parity( inchi_Atom *at, int cur_at, S_CHAR *z_dir, int *bOnlyNonMetal,
                                int bPointedEdgeStereo, Stereo0D *pStereo )
{
    double at_coord[MAX_NUM_STEREO_BOND_NEIGH][3], c, s, tmp[3], tmp1, tmp2, min_tmp, max_tmp, z;
    double temp[3], pnt[3][3];
    int j, k, p0, p1, p2, next, num_z, nType, num_either_single, num_either_double;
    int nNumExplictAttachments;
    int bond_parity  =  INCHI_PARITY_UNDEFINED;
    int    num_H=0, num_eH=0, num_nH=0 /* = num_iso_H[0] */;
    const double one_pi = 2.0*atan2(1.0 /* y */, 0.0 /* x */);
    const double two_pi = 2.0*one_pi;
    AT_NUM       nSbNeighOrigAtNumb[MAX_NUM_STEREO_BOND_NEIGH];
    int          bAmbiguousStereo = 0;
    S_CHAR       num_iso_eH[NUM_H_ISOTOPES+1]; /* count explicit H */

    if ( z_dir && !z_dir[0] && !z_dir[1] && !z_dir[2] ) {
        z_dir[2]=100;
    }
    /* *bOnlyNonMetal = 0; */
    if ( at[cur_at].num_bonds > MAX_NUM_STEREO_BOND_NEIGH || *bOnlyNonMetal ) {
        int i_ord_LastMetal;
        int nNumNonMetal = e_nNumNonMetalNeigh( at, cur_at, pStereo, &i_ord_LastMetal );
        if ( nNumNonMetal <= MAX_NUM_STEREO_BOND_NEIGH ) {
            *bOnlyNonMetal = 1;
            if ( 1 == nNumNonMetal ) {
                return bond_parity; /* INCHI_PARITY_UNDEFINED */
            }
            if ( 0 == nNumNonMetal ) {
                return INCHI_PARITY_NONE;
            }
        } else {
            return INCHI_PARITY_NONE;
        }
    }
    memset(num_iso_eH, 0, sizeof(num_iso_eH));
    for ( j = 0; j < at[cur_at].num_bonds; j ++ ) {
        next = at[cur_at].neighbor[j];
        switch( pStereo->cAtType[next] ) {
        case AtType_TermH:
            if ( 0 <= at[next].isotopic_mass && at[next].isotopic_mass <= NUM_H_ISOTOPES ) {
                num_iso_eH[at[next].isotopic_mass] ++;
            } else {
                num_iso_eH[0] ++;
            }
            num_eH ++;
            break;
        case AtType_TermD:
            num_iso_eH[2] ++;
            num_eH ++;
            break;
        case AtType_TermT:
            num_iso_eH[3] ++;
            num_eH ++;
            break;
        }
    }


    num_H  = inchi_NUMH2(at,cur_at) + num_eH;
    if ( num_H > NUM_H_ISOTOPES )
        return 0; /*  at least 2 H atoms are isotopically identical */

    for ( j = 0, num_nH = num_H; j < NUM_H_ISOTOPES; j ++ ) {
        if ( (k = (int)at[cur_at].num_iso_H[j+1]+(int)num_iso_eH[j+1]) > 1 ) {
            return INCHI_PARITY_NONE;  /*  two or more identical isotopic H atoms */
        }
        num_nH -= k;
    }
    /*  at this point num_nH = number of non-isotopic H atoms */
    if ( num_nH > 1 )
        return INCHI_PARITY_NONE; /*  two or more identical non-isotopic H atoms */
    if ( num_nH < 0 )
        return CT_ISO_H_ERR;  /*  program error */ /*   <BRKPT> */

    /********************************************************************
     * Note. At this point all (implicit and explicit) isotopic
     * terminal H neighbors are either different or not present.
     ********************************************************************/

    /*  store neighbors coordinates */
    num_z = num_either_single = num_either_double = 0;
    nNumExplictAttachments = 0;
    for ( j = 0; j < at[cur_at].num_bonds; j ++ ) {
        next = at[cur_at].neighbor[j];
        if ( *bOnlyNonMetal && IS_METAL(pStereo->cAtType[next]) )
            continue;
        at_coord[nNumExplictAttachments][0] = at[next].x - at[cur_at].x;
        at_coord[nNumExplictAttachments][1] = at[next].y - at[cur_at].y;
        nSbNeighOrigAtNumb[nNumExplictAttachments] = next;

        z = e_get_z_coord( at, cur_at, j /*neighbor #*/, &nType, bPointedEdgeStereo );
        switch ( nType ) {
        case ZTYPE_EITHER:
            num_either_single ++; /*  bond in "Either" direction. */
            break;
        case ZTYPE_UP:
        case ZTYPE_DOWN:
            z = e_len2( at_coord[nNumExplictAttachments] );
            /*
            z = sqrt( at_coord[nNumExplictAttachments][0]*at_coord[nNumExplictAttachments][0]
                    + at_coord[nNumExplictAttachments][1]*at_coord[nNumExplictAttachments][1] );
            */
            if ( nType == ZTYPE_DOWN )
                z = -z;
            /*  no break; here */
        case ZTYPE_3D:
            num_z ++;
        }
        at_coord[nNumExplictAttachments][2] = z;
        nNumExplictAttachments ++;
    }

    if ( num_either_single ) {
        bond_parity =  INCHI_PARITY_UNKNOWN;  /*  single bond is 'unknown' */
        goto exit_function;
    }

    /* nNumExplictAttachments is a total number of attachments */
    if ( nNumExplictAttachments == 2 ) {
        /*  create coordinates of the implicit hydrogen (or a fictitious atom in case of ==N-X ), */
        /*  coord[2][], attached to the cur_at. */
        for ( j = 0; j < 3; j ++ ) {
            at_coord[2][j] = - ( at_coord[0][j] + at_coord[1][j] );
        }
        nSbNeighOrigAtNumb[nNumExplictAttachments] = -1; /* implicit H or lone pair */
    }
    for ( j = 0; j < 3; j ++ ) {
        tmp[j] = e_len3( at_coord[j] );
    }
    min_tmp = inchi_min( tmp[0], inchi_min(tmp[1], tmp[2]) );
    max_tmp = inchi_max( tmp[0], inchi_max(tmp[1], tmp[2]) );
    if ( min_tmp < MIN_BOND_LEN || min_tmp < MIN_SINE*max_tmp ) {
        /*  all bonds or some of bonds are too short */
        goto exit_function; 
    }
    /*  normalize lengths to 1 */
    for ( j = 0; j < 3; j ++ ) {
        e_mult3( at_coord[j], 1.0/tmp[j], at_coord[j] );
    }

    /*  find projections of at_coord vector differences on the plane containing their arrowhead ends */
    for ( j = 0; j < 3; j ++ ) {
        /*  pnt[0..2] = {0-1, 1-2, 2-0} */
        tmp[j] = e_len3(e_diff3( at_coord[j], at_coord[(j+1)%3], pnt[j] ));
        if ( tmp[j] < MIN_SINE ) {
            goto exit_function; /*  angle #i-cur_at-#j is too small */
        }
        e_mult3( pnt[j], 1.0/tmp[j], pnt[j] ); /* 2003-10-06 */
    }
    /*  find pnt[p2], a vector perpendicular to the plane, and its length tmp[p2] */
    /*  replace previous pnt[p2], tmp[p2] with new values; the old values do not have any additional */
    /*  information because pnt[p0]+pnt[p1]+pnt[p2]=0 */
    /*  10-6-2003: a cross-product of one pair pnt[j], pnt[(j+1)%3] can be very small. Find the larges one */
    tmp1 = e_len3( e_cross_prod3( pnt[0], pnt[1], temp ) );
    for (j = 1, k = 0; j < 3; j ++ ) {
        tmp2 = e_len3( e_cross_prod3( pnt[j], pnt[(j+1)%3], temp ) );
        if ( tmp2 > tmp1 ) {
            tmp1 = tmp2;
            k     = j;
        }
    }
    /* previously p0=0, p1=1, p2=2 */
    p0 = k;
    p1 = (k+1)%3;
    p2 = (k+2)%3;
    tmp[p2] = e_len3( e_cross_prod3( pnt[p0], pnt[p1], pnt[p2] ) );
    if ( tmp[p2] < MIN_SINE*tmp[p0]*tmp[p1]  ) {
        goto exit_function; /*  pnt[p0] is almost colinear to pnt[p1] */
    }
    /*  new basis: pnt[p0], pnt[p1], pnt[p2]; set z-coord sign and make abs(pnt[p2]) = 1 */
    e_mult3( pnt[p2], (pnt[p2][2]>0.0? 1.0:-1.0)/tmp[p2], pnt[p2] ); /*  unit vector in the new z-axis direction */

    min_tmp = e_dot_prod3( at_coord[0], pnt[p2] ); /*  non-planarity measure (sine): hight of at_coord[] pyramid */
    e_mult3( pnt[p2], min_tmp, pnt[p0] ); /*  vector height of the pyramid, ideally 0 */
    /*  find new pnt[p0] = projection of at_coord[p0] on plane orthogonal to pnt[p2] */
    tmp[p0] = e_len3(e_diff3( at_coord[0], pnt[p0], pnt[p0] ));
    e_mult3( pnt[p0], 1.0/tmp[p0], pnt[p0] );  /*  new x axis basis vector */
    e_cross_prod3( pnt[p2], pnt[p0], pnt[p1] ); /*  new y axis basis vector */
    /*  find at_coord in the new basis of {pnt[p0], pnt[p1], pnt[p2]} */
    for ( j = 0; j < 3; j ++ ) {
        e_copy3( at_coord[j], temp );
        for ( k = 0; k < 3; k ++ ) {
            at_coord[j][k] = e_dot_prod3( temp, pnt[(k+p0)%3] );
        }
        /*  new xy plane projection length */
        tmp[j] = sqrt(at_coord[j][0]*at_coord[j][0] + at_coord[j][1]*at_coord[j][1]);
        /*  make new xy plane projection length = 1 */
        e_mult3( at_coord[j], 1.0/tmp[j], at_coord[j] );
    }
   
    s = fabs( at_coord[1][0]*at_coord[2][1] - at_coord[1][1]*at_coord[2][0] ); /*  1-2 sine */
    c =       at_coord[1][0]*at_coord[2][0] + at_coord[1][1]*at_coord[2][1];   /*  1-2 cosine */
    if ( s < MIN_SINE && c > 0.5 ) {
        goto exit_function; /*  bonds to neigh. 1 and 2 have almost same direction; relative angles are undefined */
    }
    c = at_coord[0][0]; /*  cosine of the angle between new Ox axis and a bond to the neighbor 0. Should be 1 */
    s = at_coord[0][1]; /*  sine. Should be 0 */
    /*  turn vectors so that vector #1 (at_coord[0]) becomes {1, 0} */
    for ( j = 0; j < MAX_NUM_STEREO_BOND_NEIGH; j ++ ) {
        tmp1 =  c*at_coord[j][0] + s*at_coord[j][1];
        tmp2 = -s*at_coord[j][0] + c*at_coord[j][1];
        at_coord[j][0] = tmp1;
        at_coord[j][1] = tmp2;
    }
    /*  counterclockwise angles from the direction to neigh 0 to to directions to neighbors 1 and 2: */
    tmp1 = atan2( at_coord[1][1], at_coord[1][0] ); /*  range -pi and +pi */
    tmp2 = atan2( at_coord[2][1], at_coord[2][0] );
    if ( tmp1 < 0.0 )
        tmp1 += two_pi; /*  range 0 to 2*pi */
    if ( tmp2 < 0.0 )
        tmp2 += two_pi;
    /*-----------------------------------
                        Example
      1 \               case tmp1 < tmp2
         \              parity is odd
          \             (counterclockwise)
           A------- 0
          /
         /
      2 /

    ------------------------------------*/
    bond_parity = 2 - ( tmp1 < tmp2 );
    for ( j = 0; j < 3; j ++ ) {
        z_dir[j] = (S_CHAR)(pnt[p2][j]>= 0.0?  floor(0.5 + 100.0 * pnt[p2][j]) :
                                              -floor(0.5 - 100.0 * pnt[p2][j])); /*  abs(z_dir) = 100 */
    }
    /*  check for ambiguity */
    if ( nNumExplictAttachments > 2 ) {
        min_tmp = inchi_min( tmp1, tmp2 );
        max_tmp = inchi_max( tmp1, tmp2 );
        if ( min_tmp > one_pi-MIN_SINE || max_tmp < one_pi+MIN_SINE || max_tmp-min_tmp > one_pi - MIN_SINE ) {
            bAmbiguousStereo |= AMBIGUOUS_STEREO;
        } else /* 3D ambiguity 8-28-2002 */
        if ( fabs(at_coord[0][2]) > MAX_SINE ) { /*  all fabs(at_coord[j][2] (j=0..2) must be equal */
            bAmbiguousStereo |= AMBIGUOUS_STEREO;
        }
    } else
    if ( nNumExplictAttachments == 2 ) {  /* 10-6-2003: added */
        min_tmp = fabs(tmp1 - one_pi);
        if ( min_tmp < MIN_SINE ) {
            bond_parity = INCHI_PARITY_UNDEFINED; /* consider as undefined 10-6-2003 */
        } else
        if ( min_tmp < MIN_ANGLE_DBOND ) {
            bAmbiguousStereo |= AMBIGUOUS_STEREO;
        }
    }

    /*  for 3 neighbors moving implicit H to the index=0 from index=2 position */
    /*  can be done in 2 transpositions and does not change atom's parity */

exit_function:
    return bond_parity;
}

#define MAX_ALLENE_LEN 20
/*************************************************************/
int e_set_stereo_bonds_parity( Stereo0D *pStereo, inchi_Atom *at, int at_1, int bPointedEdgeStereo )
{
    int j, k, next_at_1, i_next_at_1, i_next_at_2, at_2, next_at_2, num_stereo_bonds, bFound, bAllene;
    int bond_type, num_2s_1, num_alt_1, bNxtStereobond, bCurStereobond, bNxtCumulene;
    int num_stored_stereo_bonds, num_stored_isotopic_stereo_bonds;
    int chain_length, num_chains, cur_chain_length;
    int all_at_2[MAX_NUM_STEREO_BONDS];
    int all_pos_1[MAX_NUM_STEREO_BONDS], all_pos_2[MAX_NUM_STEREO_BONDS];
    S_CHAR all_unkn[MAX_NUM_STEREO_BONDS], all_chain_len[MAX_NUM_STEREO_BONDS];
    int    all_middle_at[MAX_NUM_STEREO_BONDS];
    AT_NUM chain_atoms[MAX_ALLENE_LEN], at_middle;
    int    nUnknown;
    int    bOnlyNM1, bOnlyNM2;
    
    bCurStereobond = e_bCanAtomHaveAStereoBond( at, at_1, pStereo->cAtType );
    if ( !bCurStereobond )
        return 0;

    /*  count bonds and find the second atom on the stereo bond */
    num_2s_1 = num_alt_1 = 0;
    chain_length      = 0;
    num_chains        = 0;
    for ( i_next_at_1 = 0, num_stereo_bonds = 0; i_next_at_1 < at[at_1].num_bonds; i_next_at_1 ++ ) {
        
        at_2 = next_at_1 = at[at_1].neighbor[i_next_at_1];
        bond_type        = at[at_1].bond_type[i_next_at_1];
        nUnknown = (at[at_1].bond_stereo[i_next_at_1] == INCHI_PARITY_UNKNOWN);
        next_at_2 = at_1;

        bNxtStereobond = e_bCanAtomHaveAStereoBond( at, at_2, pStereo->cAtType );
        bNxtCumulene = 0;
        cur_chain_length = 0;
        
        if ( bNxtStereobond + bCurStereobond > 3 ) {
            /* this includes single (ring) bonds in -NH-CH=N(+)< that may be deprotonated to -N=C-N< */
            if ( bond_type == INCHI_BOND_TYPE_TRIPLE ) {
                continue;
            }
            /* possibly stereogenic bond */
        } else {
            bNxtCumulene =
                e_bCanAtomBeMiddleAllene(at_2, pStereo->cAtType ) &&
                e_bCanAtomBeTerminalAllene( at_1, pStereo->cAtType );
            if ( !bNxtCumulene ) {
                continue;
            }
            if ( bond_type != INCHI_BOND_TYPE_DOUBLE ) {
                continue;
            }
            /* possibly allene or cumulenr */
            chain_atoms[cur_chain_length]   = at_1;
            chain_atoms[cur_chain_length+1] = at_2;
            
            /*
             * Example of cumulene
             * chain length = 2:     >X=C=C=Y<
             *                        | | | |
             *  1st cumulene atom= at_1 | | at_2 =last cumlene chain atom
             *  next to at_1=   next_at_1 next_at_2  =previous to at_2
             *
             *  chain length odd:  stereocenter on the middle atom ( 1=> allene )
             *  chain length even: "long stereogenic bond"
             */
            while ( cur_chain_length < MAX_ALLENE_LEN-3 &&
                    (bAllene = 
                    e_bCanAtomBeMiddleAllene( at_2, pStereo->cAtType ))) {
                k = ((int)at[at_2].neighbor[0]==next_at_2); /*  opposite neighbor position */
                next_at_2 = at_2;
                nUnknown += (at[at_2].bond_stereo[k] == INCHI_BOND_STEREO_DOUBLE_EITHER);
                at_2 = (int)at[at_2].neighbor[k];
                cur_chain_length ++;  /*  count =C= atoms */
                chain_atoms[cur_chain_length+1] = at_2;
            }
            if ( cur_chain_length ) {
                if ( bAllene /* at the end of the chain atom Y is =Y=, not =Y< or =Y- */ ||
                     !e_bCanAtomBeTerminalAllene( at_2, pStereo->cAtType)) {
                    cur_chain_length = 0;
                    continue; /*  ignore: does not fit cumulene description; go to check next at_1 neighbor */
                }
            }
            if ( !cur_chain_length &&
                 !e_bCanAtomHaveAStereoBond( at, at_2, pStereo->cAtType ) ) {
                    continue; /*  reject non-stereogenic bond to neighbor #i_next_at_1 */
            }
        } 

        /*  check atom at the opposite end of possibly stereogenic bond */

        bFound   = ( at_1 > at_2 ); /*  i_next_at_1 = at_1 stereogenic bond neighbor attachment number */

        if ( bFound ) {
            i_next_at_2 = -1;  /*  unassigned mark */
            for ( j = 0; j < at[at_2].num_bonds; j ++ ) {
                if ( (int)at[at_2].neighbor[j] == next_at_2 ) {
                    i_next_at_2 = j; /*  assigned */
                    break;
                }
            }
            if ( i_next_at_2 < 0 ) {
                continue;
            }
            /* store the results */
            all_pos_1[num_stereo_bonds]     = i_next_at_1; /* neighbor to at_1 position */
            all_pos_2[num_stereo_bonds]     = i_next_at_2; /* neighbor to at_2 position */
            all_at_2[num_stereo_bonds]      = at_2;        /* at_2 */
            all_unkn[num_stereo_bonds]      = nUnknown;    /* stereogenic bond has Unknown configuration */
            /* allene/cumulene stuff */
            all_chain_len[num_stereo_bonds] = cur_chain_length;
            all_middle_at[num_stereo_bonds] = (cur_chain_length%2)? chain_atoms[(cur_chain_length+1)/2] : -1; 
            num_stereo_bonds ++;
        }
    }
    if ( !num_stereo_bonds || num_stereo_bonds > 3 /*|| num_chains > 1*/ ) {
        return 0; /*  At the end, we cannot be more than 1 cumulene chain. */
    }

    /* ================== calculate parities ====================== */
    /*  find possibly stereo bonds and save them */
    num_stored_isotopic_stereo_bonds = 0;
    num_stored_stereo_bonds          = 0;
    for ( k = 0; k < num_stereo_bonds; k ++ ) {
        /* NM stands for non-metal */
        int parity_at_1, parity_at_2, parity_at_1NM=0, parity_at_2NM=0, bOnlyNM;
        S_CHAR z_dir1[3], z_dir2[3], z_dir1NM[3], z_dir2NM[3]; /*  3D vectors for half stereo bond parity direction */
        int  i_ord_LastMetal1, i_ord_LastMetal2, bAnomaly1NM, bAnomaly2NM;
        
        at_2             = all_at_2[k];
        i_next_at_1      = all_pos_1[k];
        i_next_at_2      = all_pos_2[k];
        nUnknown         = all_unkn[k];
        at_middle        = all_middle_at[k];
        cur_chain_length = all_chain_len[k];
        memset(z_dir1, 0, sizeof(z_dir1));
        memset(z_dir2, 0, sizeof(z_dir2));
        memset(z_dir1NM, 0, sizeof(z_dir1NM));
        memset(z_dir2NM, 0, sizeof(z_dir2NM));
        bOnlyNM1 = bOnlyNM2 = 0;
        bAnomaly1NM = bAnomaly2NM = 0;
        i_ord_LastMetal1 = i_ord_LastMetal2 = -1;
        
        if ( nUnknown ) {
            parity_at_1 = parity_at_2 = parity_at_1NM = parity_at_2NM = INCHI_PARITY_UNKNOWN;
        } else {
            /***********************************************************************
             *
             * Obtain each atom parity from the geometry
             *
             * Warning: case when dot-product
             *          (z_dir1,z_dir1NM) < 0 or (z_dir2,z_dir2NM) < 0
             *          is not treated proprly.
             *          This case may happen especially when z_dir?[3] << 100
             *
             ***********************************************************************/

            parity_at_1 =  e_half_stereo_bond_parity( at, at_1, z_dir1, &bOnlyNM1, bPointedEdgeStereo, pStereo );
            parity_at_2 =  e_half_stereo_bond_parity( at, at_2, z_dir2, &bOnlyNM2, bPointedEdgeStereo, pStereo );

            if ( RETURNED_ERROR(parity_at_1) || RETURNED_ERROR(parity_at_2) ) {
                return CT_CALC_STEREO_ERR;
            }
            parity_at_1   = abs(parity_at_1);
            parity_at_2   = abs(parity_at_2);
            /* bond parities for the disconnected structure */
            if ( bOnlyNM1 ) {
                parity_at_1NM = parity_at_1;
                memcpy( z_dir1NM, z_dir1, sizeof(z_dir1NM) );
                parity_at_1    = INCHI_PARITY_NONE;
            } else
            if ( at[at_1].num_bonds == e_nNumNonMetalNeigh( at, at_1, pStereo, &i_ord_LastMetal1 ) ) {
                parity_at_1NM = parity_at_1; /* no metal bond present */
                memcpy( z_dir1NM, z_dir1, sizeof(z_dir1NM) );
            } else {
                bOnlyNM = 1; /* at_1 has a metal neighbor; in adjacency list
                                it is located in (zero-based) position i_ord_LastMetal1 */
                parity_at_1NM = e_half_stereo_bond_parity( at, at_1, z_dir1NM, &bOnlyNM,
                                                           bPointedEdgeStereo, pStereo );
                if ( RETURNED_ERROR(parity_at_1NM) ) {
                    return CT_CALC_STEREO_ERR;
                }
                parity_at_1NM = abs(parity_at_1NM);
                /*
                if ( ATOM_PARITY_WELL_DEF(parity_at_1NM) && !ATOM_PARITY_WELL_DEF(parity_at_1) ) {
                    memcpy( z_dir1, z_dir1NM, sizeof(z_dir1) );
                }
                */
                if ( ATOM_PARITY_WELL_DEF(parity_at_1NM) && ATOM_PARITY_WELL_DEF(parity_at_1) &&
                     0 <= i_ord_LastMetal1 ) {
                    if ( 1 == i_ord_LastMetal1 % 2 && parity_at_1NM == parity_at_1 ||
                         0 == i_ord_LastMetal1 % 2 && parity_at_1NM != parity_at_1 ) {
                        /* abnormal geometry: all bonds in a sector < 180 degrees */
                        /*parity_at_1NM = 3 - parity_at_1;*/
                        bAnomaly1NM = 1;
                    } else {
                        parity_at_1 = parity_at_1NM;
                        bOnlyNM1 = 1;
                    }
                } else
                if ( ATOM_PARITY_WELL_DEF(parity_at_1NM) && !ATOM_PARITY_WELL_DEF(parity_at_1) &&
                     0 <= i_ord_LastMetal1 ) {
                    bAnomaly1NM = 1; /* 2005-02-01 force atom parity from non-metal neighbors */
                }

            }
        
            if ( bOnlyNM2 ) {
                parity_at_2NM = parity_at_2;
                memcpy( z_dir2NM, z_dir2, sizeof(z_dir2NM) );
                parity_at_2    = INCHI_PARITY_NONE;
            } else
            if ( at[at_2].num_bonds == e_nNumNonMetalNeigh( at, at_2, pStereo, &i_ord_LastMetal2 ) ) {
                memcpy( z_dir2NM, z_dir2, sizeof(z_dir2NM) );
                parity_at_2NM = parity_at_2; /* no metal bond present */
            } else {
                bOnlyNM = 1;
                parity_at_2NM = e_half_stereo_bond_parity( at, at_2, z_dir2NM, &bOnlyNM,
                                                           bPointedEdgeStereo, pStereo );
                if ( RETURNED_ERROR(parity_at_2NM) ) {
                    return CT_CALC_STEREO_ERR;
                }
                parity_at_2NM = abs(parity_at_2NM);
                if ( ATOM_PARITY_WELL_DEF(parity_at_2NM) && ATOM_PARITY_WELL_DEF(parity_at_2) &&
                     0 <= i_ord_LastMetal2 ) {
                    if ( 1 == i_ord_LastMetal2 % 2 && parity_at_2NM == parity_at_2 ||
                         0 == i_ord_LastMetal2 % 2 && parity_at_2NM != parity_at_2 ) {
                        /* abnormal geometry: all bonds in a sector < 180 degrees */
                        /*parity_at_2NM = 3 - parity_at_2;*/
                        bAnomaly2NM = 1;
                    } else {
                        parity_at_2 = parity_at_2NM;
                        bOnlyNM2 = 1;
                    }
                } else
                if ( ATOM_PARITY_WELL_DEF(parity_at_2NM) && !ATOM_PARITY_WELL_DEF(parity_at_2) &&
                     0 <= i_ord_LastMetal2 ) {
                    bAnomaly2NM = 1; /* 2005-02-01 force atom parity from non-metal neighbors */
                }
            }
            /* make both atoms have non-metal neighbors only */
            if ( (bOnlyNM1 || bAnomaly1NM) && ATOM_PARITY_WELL_DEF(parity_at_1NM) &&
                !(bOnlyNM2 || bAnomaly2NM) && ATOM_PARITY_WELL_DEF(parity_at_2NM) &&
                0 <= i_ord_LastMetal2 ) {
                
                bAnomaly2NM = 1;
                parity_at_2NM = 2 - (parity_at_2NM + i_ord_LastMetal2) % 2;
            } else
            if ( !(bOnlyNM1 || bAnomaly1NM) && ATOM_PARITY_WELL_DEF(parity_at_1NM) &&
                  (bOnlyNM2 || bAnomaly2NM) && ATOM_PARITY_WELL_DEF(parity_at_2NM) &&
                0 <= i_ord_LastMetal1 ) {
                
                bAnomaly1NM = 1;
                parity_at_1NM = 2 - (parity_at_1NM + i_ord_LastMetal1) % 2;
            }

            /* consistency */
            if ( parity_at_1 == INCHI_PARITY_NONE || parity_at_2 == INCHI_PARITY_NONE ) {
                parity_at_1 = parity_at_2 = INCHI_PARITY_NONE; /* =zero */
            } else {
                switch( !ATOM_PARITY_WELL_DEF( parity_at_1 ) + 2*!ATOM_PARITY_WELL_DEF( parity_at_2 ) ) {
                case 1:
                    parity_at_2 = parity_at_1; /* set both to not-well-def */
                    break;
                case 2:
                    parity_at_1 = parity_at_2; /* set both to not-well-def */
                    break;
                case 3:
                    parity_at_1 = parity_at_2 = inchi_min(parity_at_1, parity_at_2);
                    break;
                }
            }

            if ( parity_at_1NM == INCHI_PARITY_NONE || parity_at_2NM == INCHI_PARITY_NONE ) {
                parity_at_1NM = parity_at_2NM = INCHI_PARITY_NONE; /* =zero */
            } else {
                switch( !ATOM_PARITY_WELL_DEF( parity_at_1NM ) + 2*!ATOM_PARITY_WELL_DEF( parity_at_2NM ) ) {
                case 1:
                    parity_at_2NM = parity_at_1NM; /* set both to not-well-def */
                    break;
                case 2:
                    parity_at_1NM = parity_at_2NM; /* set both to not-well-def */
                    break;
                case 3:
                    parity_at_1NM = parity_at_2NM = inchi_min(parity_at_1NM, parity_at_2NM);
                    break;
                }
            }
            if ( parity_at_1 != INCHI_PARITY_NONE && parity_at_1NM == INCHI_PARITY_NONE ) {
                parity_at_1NM = parity_at_2NM = INCHI_PARITY_UNDEFINED;
            } else
            if ( parity_at_1 == INCHI_PARITY_NONE && parity_at_1NM != INCHI_PARITY_NONE ) {
                parity_at_1 = parity_at_2 = INCHI_PARITY_UNDEFINED;
            }


        
            if ( (parity_at_1   == INCHI_PARITY_UNDEFINED || parity_at_1   == INCHI_PARITY_NONE) &&
                 (parity_at_1NM == INCHI_PARITY_UNDEFINED || parity_at_1NM == INCHI_PARITY_NONE) ) {
                continue;
            }
        }
        parity_at_1 = e_FixSb0DParities( at, pStereo, cur_chain_length, at_middle,
                                      at_1, i_next_at_1, z_dir1, z_dir1NM, bOnlyNM1, bAnomaly1NM, parity_at_1, parity_at_1NM,
                                      at_2, i_next_at_2, z_dir2, z_dir2NM, bOnlyNM2, bAnomaly2NM, parity_at_2, parity_at_2NM);
        num_stored_stereo_bonds += (parity_at_1 != 0);

    }
    return num_stored_stereo_bonds;
}


/***************************************************************
 * Get stereo atom parity for the current order of attachments
 * The result in at[cur_at].parity is valid for previously removed
 * explicit hydrogen atoms, including isotopic ones, that are located in at_removed_H[]
 * The return value is a calculated parity.
 */
#define ADD_EXPLICIT_HYDROGEN_NEIGH  1
#define ADD_EXPLICIT_LONE_PAIR_NEIGH 2
int e_set_stereo_atom_parity( Stereo0D *pStereo, inchi_Atom *at, int cur_at, int bPointedEdgeStereo )
{
    int    j, k, next_at, num_z, j1, nType, num_eH, num_iH, tot_num_iso_H, nMustHaveNumNeigh, bAmbiguousStereo;
    double z, sum_xyz[3], min_sine, triple_product;
    double at_coord[MAX_NUM_STEREO_ATOM_NEIGH][3];
    double bond_len_xy[4], rmax, rmin;
    double at_coord_center[3];
    int    parity, out_parity, out_stereo_atom_parity, bAmbiguous = 0, bAddExplicitNeighbor = 0;
    int    b2D = 0, n2DTetrahedralAmbiguity = 0;
    AT_NUM nSbNeighOrigAtNumb[MAX_NUM_STEREO_ATOM_NEIGH];
    S_CHAR num_iso_eH[NUM_H_ISOTOPES+1]; /* count explicit H */
    int    bFixSp3Bug = 1; /* 1=> FixSp3Bug option for stereo bonds longer than 20 */
    
    out_parity  = out_stereo_atom_parity = INCHI_PARITY_NONE;
    parity                               = INCHI_PARITY_NONE;
    bAmbiguousStereo                     = 0;
    memset(num_iso_eH, 0, sizeof(num_iso_eH));
    num_eH = 0; /* number of explicit H -- will be found later */
    num_iH = inchi_NUMH2(at,cur_at); /* implicit H */

    if ( !(nMustHaveNumNeigh = e_bCanInpAtomBeAStereoCenter( cur_at, pStereo->cAtType ) ) ||
         num_iH > NUM_H_ISOTOPES
       ) {
        goto exit_function;
    }
    /* find explicit terminal H */
    memset(num_iso_eH, 0, sizeof(num_iso_eH));
    for ( j = 0; j < at[cur_at].num_bonds; j ++ ) {
        next_at = at[cur_at].neighbor[j];
        switch(pStereo->cAtType[next_at] ) {
        case AtType_TermH:
            if ( 0 <= at[next_at].isotopic_mass && at[next_at].isotopic_mass <= NUM_H_ISOTOPES ) {
                num_iso_eH[at[next_at].isotopic_mass] ++;
            } else {
                num_iso_eH[0] ++;
            }
            num_eH ++;
            break;
        case AtType_TermD:
            num_iso_eH[2] ++;
            num_eH ++;
            break;
        case AtType_TermT:
            num_iso_eH[3] ++;
            num_eH ++;
            break;
        }
    }

    /*  numbers of isotopic H atoms */
    for ( j = 0, tot_num_iso_H = 0; j < NUM_H_ISOTOPES; j ++ ) {
        if ( at[cur_at].num_iso_H[j+1] + num_iso_eH[j+1] > 1 ) {
            goto exit_function; /*  two or more identical hydrogen isotopic neighbors */
        }
        tot_num_iso_H += at[cur_at].num_iso_H[j+1] + num_iso_eH[j+1];
    }
   
    /*  number of non-isotopic H atoms */
    if ( inchi_NUMH2(at,cur_at) + num_eH - tot_num_iso_H > 1 ) {
        goto exit_function; /*  two or more identical hydrogen non-isotopic neighbors */
    }
    
    /*  coordinates initialization */
    num_z = 0;
    sum_xyz[0] = sum_xyz[1] = sum_xyz[2] = 0.0;

    at_coord_center[0] =
    at_coord_center[1] =
    at_coord_center[2] = 0.0;

    /*  fill out stereo center neighbors coordinates */
    /*  and obtain the parity from the geometry */

    /*  add all coordinates of other neighboring atoms */
    for ( j = j1 = 0; j < at[cur_at].num_bonds; j ++, j1 ++ ) {
        next_at = at[cur_at].neighbor[j];
        z = e_get_z_coord( at, cur_at, j, &nType, bPointedEdgeStereo );
        switch ( nType ) {
        case ZTYPE_EITHER:
            parity = INCHI_PARITY_UNKNOWN; /*  unknown parity: bond in "Either" direction. */
            goto exit_function;
        case ZTYPE_UP:
        case ZTYPE_DOWN:
            b2D ++;
        case ZTYPE_3D:
            num_z ++;
        }

        nSbNeighOrigAtNumb[j1] = next_at+1;
        at_coord[j1][0] = at[next_at].x-at[cur_at].x;
        at_coord[j1][1] = at[next_at].y-at[cur_at].y;
        bond_len_xy[j1] = e_len2(at_coord[j1]);
        at_coord[j1][2] = (nType==ZTYPE_3D?    z :
                           nType==ZTYPE_UP?    bond_len_xy[j1] :
                           nType==ZTYPE_DOWN? -bond_len_xy[j1] : 0.0 );
    }
    /* j1 is the number of explicit neighbors (that is, all neighbors except implicit H) */

    b2D = (b2D == num_z && num_z);  /*  1 => two-dimensional */

    if ( MAX_NUM_STEREO_ATOM_NEIGH   != at[cur_at].num_bonds+num_iH &&
         MAX_NUM_STEREO_ATOM_NEIGH-1 != at[cur_at].num_bonds+num_iH ) {
        /*  not enough geometry data to find the central atom parity */
        goto exit_function;
    }
    /*  make all vector lengths equal to 1; exit if too short. 9-10-2002 */
    for ( j = 0; j < j1; j ++ ) {
        z = e_len3( at_coord[j] );
        if ( z < MIN_BOND_LEN ) {
            parity = INCHI_PARITY_UNDEFINED;
            goto exit_function;
        }
#if( STEREO_CENTER_BONDS_NORM == 1 )
        else {
            e_mult3( at_coord[j], 1.0/z, at_coord[j] );
        }
#endif
        rmax = j? inchi_max( rmax, z) : z;
        rmin = j? inchi_min( rmin, z) : z;
    }
    if ( rmin / rmax < MIN_SINE ) {
        /* bond ratio is too small */
        parity = INCHI_PARITY_UNDEFINED;
        goto exit_function;
    }
    for ( j = 0; j < j1; j ++ ) {
        e_add3( sum_xyz, at_coord[j], sum_xyz );
    }



    /*  here j1 is a number of neighbors including explicit terminal isotopic H */
    /*  num_iso_eH[0] = number of explicit non-isotopic hydrogen atom neighbors */
    j = j1;
    /*  Add Explicit Neighbor */
    if ( j1 == MAX_NUM_STEREO_ATOM_NEIGH-1 ) {
        /*  add an explicit neighbor if possible */
        if ( nMustHaveNumNeigh == MAX_NUM_STEREO_ATOM_NEIGH-1 ) {
            bAddExplicitNeighbor = ADD_EXPLICIT_LONE_PAIR_NEIGH;
        } else
        if ( nMustHaveNumNeigh == MAX_NUM_STEREO_ATOM_NEIGH ) {
            /*  check whether an explicit non-isotopic hydrogen can be added */
            /*  to an atom that is a stereogenic atom */
            bAddExplicitNeighbor = ADD_EXPLICIT_HYDROGEN_NEIGH;
        }
    }

    if ( bAddExplicitNeighbor ) {
        /***********************************************************
         * May happen only if (j1 == MAX_NUM_STEREO_ATOM_NEIGH-1)
         * 3 neighbors only, no H-neighbors. Create and add coordinates of an implicit H
         * or a fake 4th neighbor, that is, a lone pair 
         */
        if ( parity == INCHI_PARITY_UNKNOWN ) {
            goto exit_function;  /*  the user insists the parity is unknown and the isotopic */
                                 /*  composition of the neighbors does not contradict */
        } else
        if ( num_z == 0 || are_3_vect_in_one_plane(at_coord, MIN_SINE) ) {
            /*  "hydrogen down" rule is needed to resolve an ambiguity */
            if ( num_z > 0 ) {
                bAmbiguous |= AMBIGUOUS_STEREO;
            }
#if( APPLY_IMPLICIT_H_DOWN_RULE == 1 )  /* { */
            /*  Although H should be at the top of the list, add it to the bottom. */
            /*  This will be taken care of later by inverting parity 1<->2 */
            at_coord[j][0] = 0.0;
            at_coord[j][1] = 0.0;
#if( STEREO_CENTER_BONDS_NORM == 1 )
            at_coord[j][2] = -1.0;
#else
            at_coord[j][2] = -(bond_len_xy[0]+bond_len_xy[1]+bond_len_xy[2])/3.0;
#endif
#else /* } APPLY_IMPLICIT_H_DOWN_RULE { */
#if (ALWAYS_SET_STEREO_PARITY == 1)
            parity =  INCHI_PARITY_EVEN; /*  suppose atoms are pre-sorted (testing) */
#else
                parity = INCHI_PARITY_UNDEFINED;
#endif
            goto exit_function;
#endif /* } APPLY_IMPLICIT_H_DOWN_RULE */
        } else {
            /*  we have enough information to find implicit hydrogen coordinates */
            e_copy3( sum_xyz, at_coord[j] );
            e_change_sign3( at_coord[j], at_coord[j] );
            z = e_len3( at_coord[j] );
            /* Comparing the original bond lengths to lenghts derived from normalized to 1 */
            /* This bug leads to pronouncing legitimate stereogenic atoms */
            /* connected by 3 bonds "undefined" if in a nicely drawn 2D structure */
            /* bond lengths are about 20 or greater. Reported by Reinhard Dunkel 2005-08-05 */
            if ( bFixSp3Bug ) {
                /* coordinate scaling bug fixed here */
                if ( z > 1.0 ) {
                    rmax *= z;
                } else {
                    rmin *= z;
                }
            } else {
                /* original InChI v.1 bug */
                rmax = inchi_max( rmax, z );
                rmin = inchi_min( rmin, z );
            }
            if ( z < MIN_BOND_LEN || rmin/rmax < MIN_SINE ) {
                /* the new 4th bond is too short */
                parity = INCHI_PARITY_UNDEFINED;
                goto exit_function;
            }
#if( STEREO_CENTER_BOND4_NORM == 1 )
            else {
                e_mult3( at_coord[j], 1.0/z, at_coord[j] );
            }
#endif
        }
    } else
    if ( j1 != MAX_NUM_STEREO_ATOM_NEIGH ) {
        if ( parity == INCHI_PARITY_UNKNOWN ) {
            parity = -INCHI_PARITY_UNDEFINED; /*  isotopic composition of H-neighbors contradicts 'unknown' */
        }
        goto exit_function;
    } else /*  j1 == MAX_NUM_STEREO_ATOM_NEIGH */
    if ( num_z == 0 || e_are_4at_in_one_plane(at_coord, MIN_SINE) ) {
        /*  all four neighours in xy plane: undefined geometry. */
        if ( num_z > 0 ) {
            bAmbiguous |= AMBIGUOUS_STEREO;
        }
        if ( parity != INCHI_PARITY_UNKNOWN ) {
#if (ALWAYS_SET_STEREO_PARITY == 1)
            parity =  INCHI_PARITY_EVEN; /*  suppose atoms are pre-sorted (testing) */
#else
            /* all 4 bonds are in one plain */
            parity = INCHI_PARITY_UNDEFINED;
#endif
        }
        goto exit_function;
    }
    /***********************************************************
     * At this point we have 4 neighboring atoms.
     * check for tetrahedral ambiguity in 2D case
     */
    if ( b2D ) {
        if ( 0 < (n2DTetrahedralAmbiguity = e_Get2DTetrahedralAmbiguity( at_coord, bAddExplicitNeighbor )) ) {
            if ( T2D_WARN & n2DTetrahedralAmbiguity ) {
                bAmbiguous |= AMBIGUOUS_STEREO;
            }
            if ( T2D_UNDF & n2DTetrahedralAmbiguity ) {
                if ( parity != INCHI_PARITY_UNKNOWN ) {
#if (ALWAYS_SET_STEREO_PARITY == 1)
                    parity =  INCHI_PARITY_EVEN; /*  suppose atoms are pre-sorted (testing) */
#else
                    parity =  INCHI_PARITY_UNDEFINED; /*  no parity */
#endif
                }
                goto exit_function;
            }
        } else
        if ( n2DTetrahedralAmbiguity < 0 ) {
            bAmbiguous |= AMBIGUOUS_STEREO_ERROR; /*  error */
            parity = INCHI_PARITY_UNDEFINED;
            goto exit_function;
        }
    }

    /************************************************************/
    /*  Move coordinates origin to the neighbor #0 */
    for ( j = 1; j < MAX_NUM_STEREO_ATOM_NEIGH; j ++ ) {
        e_diff3(at_coord[j], at_coord[0], at_coord[j]);
    }
    e_diff3(at_coord_center, at_coord[0], at_coord_center);

    /********************************************************
     * find the central (cur_at) atom's parity
     * (orientation of atoms #1-3 when looking from #0)
     ********************************************************/
    triple_product = e_triple_prod_and_min_abs_sine2(&at_coord[1], at_coord_center, bAddExplicitNeighbor, &min_sine, &bAmbiguous);
    if ( fabs(triple_product) > ZERO_FLOAT && (min_sine > MIN_SINE || fabs(min_sine) > ZERO_FLOAT && (n2DTetrahedralAmbiguity & T2D_OKAY ) ) ) {
         /* Even => sorted in correct order, Odd=>transposed */
        parity = triple_product > 0.0? INCHI_PARITY_EVEN : INCHI_PARITY_ODD;

        /*  for 4 attached atoms, moving the implicit H from index=3 to index=0 */
        /*  can be done in odd number (3) transpositions: (23)(12)(01), which inverts the parity */
        if ( j1 == MAX_NUM_STEREO_ATOM_NEIGH-1 ) {
            parity = 3 - parity;
        }
    } else {
#if (ALWAYS_SET_STEREO_PARITY == 1)
        parity =  AT_PARITY_EVEN; /*  suppose atoms are pre-sorted (testing) */
#else
        if ( num_z > 0 ) {
            bAmbiguous |= AMBIGUOUS_STEREO;
        }
        parity =  INCHI_PARITY_UNDEFINED; /*  no parity: 4 bonds are in one plane. */
#endif
    }
exit_function:

    if ( parity ) {
        bAmbiguousStereo |= bAmbiguous;
    }
    /*  isotopic parity => parity */
    if ( parity < 0 )
        parity = -parity;

    if ( 0 < parity && parity < INCHI_PARITY_UNDEFINED ) {
        inchi_Stereo0D *stereo0D;
        if ( stereo0D = e_GetNewStereo( pStereo ) ) {
            stereo0D->central_atom = cur_at;
            stereo0D->parity       = parity;
            stereo0D->type         = INCHI_StereoType_Tetrahedral;
            k = 0;
            if ( at[cur_at].num_bonds == 3 ) {
                stereo0D->neighbor[k++] = cur_at;
            }
            for ( j = 0; j < at[cur_at].num_bonds; j ++ ) {
                stereo0D->neighbor[k++] = at[cur_at].neighbor[j];
            }
        }
    }
    return parity;

}
#undef ADD_EXPLICIT_HYDROGEN_NEIGH
#undef ADD_EXPLICIT_LONE_PAIR_NEIGH

/*************************************************************/
inchi_Stereo0D *e_GetNewStereo( Stereo0D *pStereo )
{
#define DEFAULT_0D_STEREO_DELTA 64
    int delta = pStereo->delta_num_stereo0D > 0? pStereo->delta_num_stereo0D : DEFAULT_0D_STEREO_DELTA;
    if ( pStereo->num_stereo0D >= pStereo->max_num_Stereo0D ) {
        /*inchi_Stereo0D *pNew = (inchi_Stereo0D *)e_inchi_calloc( pStereo->max_num_Stereo0D + delta, sizeof(pNew[0]) );*/
        inchi_Stereo0D *pNew = e_CreateInchi_Stereo0D( pStereo->max_num_Stereo0D + delta );
        if ( pNew ) {
            if ( pStereo->num_stereo0D > 0 ) {
                memcpy( pNew, pStereo->stereo0D, pStereo->num_stereo0D * sizeof(pNew[0]) );
            }
            /*e_inchi_free(pStereo->stereo0D);*/
            e_FreeInchi_Stereo0D( &pStereo->stereo0D );
            pStereo->stereo0D = pNew;
            pStereo->max_num_Stereo0D += delta;
        }
    }
    return pStereo->stereo0D + pStereo->num_stereo0D ++;
#undef DEFAULT_0D_STEREO_DELTA
}

/*************************************************************/
int set_0D_stereo_parities( inchi_Input *pInp, int bPointedEdgeStereo )
{
    int num_3D_stereo_atoms=0, num_stereo_bonds=0;

    int i, is_stereo, num_stereo;
    inchi_Atom* at      = pInp->atom;
    int         num_at  = pInp->num_atoms;
    Stereo0D              stereo;
    Stereo0D             *pStereo = &stereo;

    /**********************************************************
     *
     * Note: this parity reflects only relative positions of
     *       the atoms-neighbors and their ordering in the
     *       lists of neighbors.
     *
     *       To obtain the actual parity, the parity of a number 
     *       of neighbors transpositions (to obtain a sorted
     *       list of numbers assigned to the atoms) should be
     *       added.
     *
     **********************************************************/

    /*********************************************************************************

     An example of parity=1 for stereogenic center, tetrahedral asymmetric atom
                   
                       
                    
                  (1)
                   |
                   |
               [C] |
                   |
         (2)------(0)
                  /
                /
              /
            /
         (3)
         

     Notation: (n) is a tetrahedral atom neighbor; n is an index of a neighbor in
     the central_at->neighbor[] array : neighbor atom number is central_at->neighbor[n].

     (0)-(1), (0)-(2), (0)-(3) are lines connecting atom [C] neighbors to neighbor (0)
     (0), (1) and (2) are in the plane
     (0)-(3) is directed from the plain to the viewer
     [C] is somewhere between (0), (1), (2), (3)
     Since (1)-(2)-(3)  are in a clockwise order when looking from (0), parity is 2, or even;
     otherwise parity would be 1, or odd.

    **********************************************************************************
    
      Examples of a stereogenic bond.
    
      Notation:   [atom number], (index of a neighbor):
                  [1] and [2] are atoms connected by the stereogenic bond
                  numbers in () are indexes of neighbors of [1] or [2].
                  (12 x 16)z = z-component of [1]-[2] and [1]-[6] cross-product

                                     atom [1]                     atom [2]                   
     [8]                    [4]      prod01 = (12 x 16)z < 0      prod01 = (21 x 24)z < 0    
       \                    /        prod02 = (12 x 18)z > 0      prod02 = (21 x 25)z > 0    
        (2)               (1)        0 transpositions because     0 transpositions because   
          \              /           double bond is in 0 posit.   double bond is in 0 position
          [1]==(0)(0)==[2]           0 = (prod01 > prod02)        0 = (prod01 > prod02)
          /              \                                                                   
        (1)               (2)        result: parity = 2, even     result: parity=2, even                                
       /                    \                                                                
     [6]                    [5]                                                              
                                                                                             
                                                                                             
                                                                                             
                                     atom [1]                     atom [2]                   
     [8]                    [5]      prod01 = (12 x 18)z > 0      prod01 = (21 x 24)z > 0    
       \                    /        prod02 = (12 x 16)z < 0      prod02 = (21 x 25)z < 0    
        (0)               (2)        2 transpositions to move     1 transposition to move    
          \              /           at [2] from 2 to 0 pos.      at [1] from 1 to 0 position
          [1]==(2)(1)==[2]           1 = (prod01 > prod02)        1 = (prod01 > prod02)      
          /              \                                                                   
        (1)               (0)        result: parity = (1+2)       result: parity=(1+1)           
       /                    \        2-(1+2)%2 = 1, odd           2-(1+1)%2 = 2, even
     [6]                    [4]           
                                          

    ***********************************************************************************
    Note: atoms' numbers [1], [2], [4],... are not used to calculate parity at this
          point. They will be used for each numbering in the canonicalization.
    Note: parity=3 for a stereo atom means entered undefined bond direction
          parity=4 for an atom means parity cannot be determined from the given geometry
    ***********************************************************************************/
    
    if ( !at ) {
        return -1;
    }

    /*  clear stereo descriptors */
    memset( pStereo, 0, sizeof(*pStereo) );
    pStereo->delta_num_stereo0D = num_at;
    pStereo->cAtType = (S_CHAR *)e_inchi_calloc( num_at, sizeof(pStereo->cAtType[0]) );
    if (!pStereo->cAtType ) {
        return -1;
    }
    /* atom stereo types */
    for ( i = 0; i < num_at; i ++ ) {
        pStereo->cAtType[i] = e_GetElType( at, i );
    }

        

    /*  calculate stereo descriptors */
    /* main cycle: set stereo parities */
    for( i = 0, num_stereo = 0; i < num_at; i ++ ) {
    
        if ( is_stereo = e_set_stereo_atom_parity( pStereo, at, i, bPointedEdgeStereo ) ) {
            num_3D_stereo_atoms += ATOM_PARITY_WELL_DEF( is_stereo );
            num_stereo          += ATOM_PARITY_WELL_DEF( is_stereo );
        } else {
            is_stereo = e_set_stereo_bonds_parity( pStereo, at, i, bPointedEdgeStereo );
            if ( RETURNED_ERROR( is_stereo ) ) {
                num_3D_stereo_atoms = is_stereo;
                is_stereo = 0;
                break;
            } else {
                num_stereo_bonds += is_stereo;
                num_stereo += is_stereo;
            }
        }
    }
    /*
    if ( (nMode & REQ_MODE_SC_IGN_ALL_UU ) 
    REQ_MODE_SC_IGN_ALL_UU
    REQ_MODE_SB_IGN_ALL_UU
    */
    if ( pStereo->cAtType ) {
        e_inchi_free( pStereo->cAtType );
    }
    if ( pInp->stereo0D ) {
    }

    pInp->stereo0D     = pStereo->stereo0D;
    pInp->num_stereo0D = pStereo->num_stereo0D;
    
    return RETURNED_ERROR( num_3D_stereo_atoms )? num_3D_stereo_atoms : num_stereo;
}
/*****************************************************************/
int Clear3D2Dstereo(inchi_Input *pInp)
{
    int i;
    if ( !pInp->atom || !pInp->num_atoms )
        return 0;
    for ( i = 0; i < pInp->num_atoms; i ++ ) {
        pInp->atom[i].x =
        pInp->atom[i].y =
        pInp->atom[i].z = 0.0;
        memset( pInp->atom[i].bond_stereo, 0, sizeof(pInp->atom[i].bond_stereo) );
    }
    return 1;
}

