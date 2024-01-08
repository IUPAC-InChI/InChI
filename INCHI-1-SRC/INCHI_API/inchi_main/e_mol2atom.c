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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "e_mode.h"

#include "inchi_api.h"

#include "e_ctl_data.h"
#include "e_comdef.h"
#include "e_util.h"
#include "e_ichicomp.h"

#include "e_readmol.h"
#include "e_inpdef.h"

#if( ADD_CMLPP == 1 )
#include "e_readcml.hpp"
#endif

#include "e_inchi_atom.h"


#define MIN_STDATA_X_COORD           0.0
#define MAX_STDATA_X_COORD         256.0
#define MIN_STDATA_Y_COORD           0.0
#define MAX_STDATA_Y_COORD         256.0
#define MIN_STDATA_Z_COORD           0.0
#define MAX_STDATA_Z_COORD         256.0
#define MAX_STDATA_AVE_BOND_LENGTH  20.0
#define MIN_STDATA_AVE_BOND_LENGTH  10.0


/* local prototypes */
inchi_Atom* mol_to_inchi_Atom( MOL_DATA* mol_data, int *num_atoms, int *num_bonds, inchi_Atom* at_inp,
                               int bDoNotAddH, int *err, char *pStrErr );
int mol_to_inchi_Atom_xyz( MOL_DATA* mol_data, int num_atoms, inchi_Atom* at, int *err, char *pStrErr );

int MolfileToInchi_Atom( FILE *inp_molfile, int bDoNotAddH, inchi_Atom **at, int max_num_at,
                      int *num_dimensions, int *num_bonds, const char *pSdfLabel, char *pSdfValue,
                      long *Id, long *lMolfileNumber, INCHI_MODE *pInpAtomFlags, int *err, char *pStrErr );
long GetMolfileNumber( MOL_HEADER_BLOCK *pHdr );


 /* too long ave. bond length prevents structure from displaying  */
 /* According to Steve, a standard bond length is 10. 9-24-97 DCh */
 /* Ave. bond length in MainLib is 20. Also fixed Average         */
 /* bond length calculation by introducing num_avg_bonds          */
 /* in mol_to_stdata(). 12-9-99 DCh.                              */

/******************************************************************************************************/
inchi_Atom* mol_to_inchi_Atom( MOL_DATA* mol_data, int *num_atoms, int *num_bonds, inchi_Atom* at_inp,
                       int bDoNotAddH, int *err, char *pStrErr )
{
    inchi_Atom *at = NULL;
    /* char      *bond_stereo = NULL; */
    AT_NUM   *p1, *p2;
    int       i, a1, a2, n1, n2, bonds;
    S_CHAR    cBondStereo, cBondType;
    S_CHAR    cStereo1, cStereo2;

    *err = 0;
    *num_atoms = *num_bonds = 0;
    /* check if MOLfile contains atoms */
    if ( !mol_data || !mol_data->ctab.MolAtom ||
         0 < mol_data->ctab.nNumberOfBonds && !mol_data->ctab.MolBond ||
         0 >= (*num_atoms = mol_data->ctab.nNumberOfAtoms) ) {
        /* MOLFILE_ERR_SET (*err, 0, "Empty structure"); */
        goto exit_function; /*  no structure */
    }
    /* allocate memory if necessary */
    if ( at_inp ) {
        at = at_inp;
    } else
    if ( !(at = e_CreateInchi_Atom( *num_atoms ) ) ) {
        *err = -1;
        MOLFILE_ERR_FIN (*err, -1, exit_function, "Out of RAM");
    }

    /* copy atom info */
    for ( i = 0; i < *num_atoms; i ++ ) {
        e_mystrncpy( at[i].elname, mol_data->ctab.MolAtom[i].szAtomSymbol, sizeof(at->elname) );
        /* at[i].chem_bonds_valence = mol_data->ctab.MolAtom[i].cValence; */ /*  MOLfile valence; will change */
        at[i].isotopic_mass      = mol_data->ctab.MolAtom[i].cMassDifference;
        at[i].charge             = mol_data->ctab.MolAtom[i].cCharge;
        at[i].radical            = mol_data->ctab.MolAtom[i].cRadical;
        /* coordinates are copied in mol_to_inchi_Atom_xyz() */
#if( SINGLET_IS_TRIPLET == 1 )
        if ( at[i].radical == RADICAL_SINGLET ) {
            at[i].radical = RADICAL_TRIPLET;
        }
#endif
        /* removed parsing at[i].elname to extract H, charge, radical from the
           Molfile alias record now this is done in the INChI dll */
    }

    /* copy bond info */
    for ( i = 0, bonds = 0; i < mol_data->ctab.nNumberOfBonds; i ++ ) {
        cBondStereo = mol_data->ctab.MolBond[i].cBondStereo;
        cBondType   = mol_data->ctab.MolBond[i].cBondType;
        a1 = mol_data->ctab.MolBond[i].nAtomNo1-1;
        a2 = mol_data->ctab.MolBond[i].nAtomNo2-1;

        if ( a1 < 0 || a1 >= *num_atoms ||
             a2 < 0 || a2 >= *num_atoms ||
             a1 == a2 ) {
            *err |= 1; /*  bond for impossible atom number(s); ignored */
            MOLFILE_ERR_SET (*err, 0, "Bond to nonexistent atom");
            continue;
        }
        /*  check for multiple bonds between same atoms */
        p1 = e_is_in_the_slist( at[a1].neighbor, (AT_NUM)a2, at[a1].num_bonds );
        p2 = e_is_in_the_slist( at[a2].neighbor, (AT_NUM)a1, at[a2].num_bonds );
        if ( (p1 || p2) && (p1 || at[a1].num_bonds < MAXVAL) && (p2 || at[a2].num_bonds < MAXVAL) ) {
            n1 = p1? (p1 - at[a1].neighbor) : at[a1].num_bonds ++;
            n2 = p2? (p2 - at[a2].neighbor) : at[a2].num_bonds ++;
            MOLFILE_ERR_SET (*err, 0, "Multiple bonds between two atoms");
            *err |= 2; /*  multiple bonds between atoms */
        } else
        if ( !p1 && !p2 && at[a1].num_bonds < MAXVAL && at[a2].num_bonds < MAXVAL ) {
            n1 = at[a1].num_bonds ++;
            n2 = at[a2].num_bonds ++;
            bonds ++;
        } else {
            char szMsg[64];
            *err |= 4; /*  too large number of bonds. Some bonds ignored. */
            sprintf( szMsg, "Atom '%s' has more than %d bonds",
                            at[a1].num_bonds>= MAXVAL? at[a1].elname:at[a2].elname, MAXVAL );
            MOLFILE_ERR_SET (*err, 0, szMsg);
            continue;
        }
        if ( cBondType < MIN_INPUT_BOND_TYPE || cBondType > MAX_INPUT_BOND_TYPE ) {
            char szBondType[16];
            sprintf( szBondType, "%d", cBondType );
            cBondType = 1;
            MOLFILE_ERR_SET (*err, 0, "Unrecognized bond type:");
            MOLFILE_ERR_SET (*err, 0, szBondType);
            *err |= 8; /*  Unrecognized Bond type replaced with single bond */
        }
        /* bond type */
        at[a1].bond_type[n1] =
        at[a2].bond_type[n2] = cBondType;
        /* connection */
        at[a1].neighbor[n1] = (AT_NUM)a2;
        at[a2].neighbor[n2] = (AT_NUM)a1;
        /* stereo */
        switch ( cBondStereo ) {
        case INPUT_STEREO_DBLE_EITHER:  /* 3 */
            cStereo1 = INCHI_BOND_STEREO_DOUBLE_EITHER;
            cStereo2 = INCHI_BOND_STEREO_DOUBLE_EITHER;
            break;
        case INPUT_STEREO_SNGL_UP:      /* 1 */
            cStereo1 = INCHI_BOND_STEREO_SINGLE_1UP;
            cStereo2 = INCHI_BOND_STEREO_SINGLE_2UP;
            break;
        case INPUT_STEREO_SNGL_EITHER:  /* 4 */
            cStereo1 = INCHI_BOND_STEREO_SINGLE_1EITHER;
            cStereo2 = INCHI_BOND_STEREO_SINGLE_2EITHER;
            break;
        case INPUT_STEREO_SNGL_DOWN:    /* 6 */ 
            cStereo1 = INCHI_BOND_STEREO_SINGLE_1DOWN;
            cStereo2 = INCHI_BOND_STEREO_SINGLE_2DOWN;
            break;
        case 0:
            cStereo1 = INCHI_BOND_STEREO_NONE;
            cStereo2 = INCHI_BOND_STEREO_NONE;
            break;
        default:
            *err |= 16; /*  Ignored unrecognized Bond stereo */
            MOLFILE_ERR_SET (*err, 0, "Unrecognized bond stereo");
            continue;
        }
        at[a1].bond_stereo[n1] = cStereo1; /*  >0: the wedge (pointed) end is at this atom */
        at[a2].bond_stereo[n2] = cStereo2; /*  <0: the wedge (pointed) end is at the opposite atom */
    }
    *num_bonds = bonds;

    /* special Molfile valences */
    for ( a1 = 0; a1 < *num_atoms; a1 ++ ) {
        int num_bond_type[MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE + 1], bond_type;
        int chem_bonds_valence, valence;
        if (  mol_data->ctab.MolAtom[a1].cValence &&
             (mol_data->ctab.MolAtom[a1].cValence != 15 || at[a1].num_bonds) ) {
            /* Molfile contains special valence => calculate number of H */
            memset( num_bond_type, 0, sizeof(num_bond_type) );
            valence = mol_data->ctab.MolAtom[a1].cValence; /*  save atom valence if available */
            for ( n1 = 0; n1 < at[a1].num_bonds; n1 ++ ) {
                bond_type = at[a1].bond_type[n1] - MIN_INPUT_BOND_TYPE;
                if (  bond_type < 0 || bond_type > MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE ) {
                    bond_type = 0;
                    MOLFILE_ERR_SET (*err, 0, "Unknown bond type in MOLfile assigned as a single bond");
                }
                num_bond_type[ bond_type ] ++;
            }
            chem_bonds_valence = 0;
            for ( n1 = 0; MIN_INPUT_BOND_TYPE + n1 <= 3 && MIN_INPUT_BOND_TYPE + n1 <= MAX_INPUT_BOND_TYPE; n1 ++ ) {
                chem_bonds_valence += (MIN_INPUT_BOND_TYPE + n1) * num_bond_type[n1];
            }
            if ( MIN_INPUT_BOND_TYPE <= INCHI_BOND_TYPE_ALTERN && INCHI_BOND_TYPE_ALTERN <= MAX_INPUT_BOND_TYPE &&
                 ( n2 = num_bond_type[INCHI_BOND_TYPE_ALTERN-MIN_INPUT_BOND_TYPE] ) ) {
                /* accept input aromatic bonds for now */
                switch ( n2 ) {
                case 2:
                    chem_bonds_valence += 3;  /* =A- */
                    break;
                case 3:
                    chem_bonds_valence += 4;  /* =A< */
                    break;
                default:
                    /*  if 1 or >= 4 aromatic bonds then replace such bonds with single bonds */
                    for ( n1 = 0; n1 < at[a1].num_bonds; n1 ++ ) {
                        if ( at[a1].bond_type[n1] == INCHI_BOND_TYPE_ALTERN ) {
                            a2 = at[a1].neighbor[n1];
                            p1 = e_is_in_the_slist( at[a2].neighbor, (AT_NUM)a1, at[a2].num_bonds );
                            if ( p1 ) {
                                at[a1].bond_type[n1] = 
                                at[a2].bond_type[p1-at[a2].neighbor] = INCHI_BOND_TYPE_SINGLE;
                            } else {
                                *err = -2;  /*  Program error */
                                MOLFILE_ERR_SET (*err, 0, "Program error interpreting MOLfile");
                                goto exit_function; /*  no structure */
                            }
                        }
                    }
                    chem_bonds_valence += n2;
                    *err |= 32;
                    MOLFILE_ERR_SET (*err, 0, "Atom has more than 3 aromatic bonds");
                    break;
                }
            }
            /*************************************************************************************
             *
             *  Set number of hydrogen atoms
             */
            if ( valence >= chem_bonds_valence ) {
                at[a1].num_iso_H[0] = valence - chem_bonds_valence;
            }
        } else
        if ( mol_data->ctab.MolAtom[a1].cAtomAliasedFlag ) {
            at[a1].num_iso_H[0] = 0;
        } else
        if ( mol_data->ctab.MolAtom[a1].cValence == 15 && !at[a1].num_bonds ) {
            at[a1].num_iso_H[0] = 0;
        } else
        if ( !bDoNotAddH ) {
            at[a1].num_iso_H[0] = -1;
        }
    }

exit_function:;
    return at;
}
/******************************************************************************************************/
int mol_to_inchi_Atom_xyz( MOL_DATA* mol_data, int num_atoms, inchi_Atom* at, int *err, char *pStrErr )
{
    int      i, num_dimensions=0;
    int      num_bonds;
    double max_x=-1.0e32, max_y=-1.0e32, max_z=-1.0e32;
    double min_x= 1.0e32, min_y= 1.0e32, min_z= 1.0e32;
    double macheps = 1.0e-10, small_coeff = 0.00001;
    double x_coeff, y_coeff, z_coeff, coeff, average_bond_length;

    /*  *err = 0; */
    /* check if MOLfile contains atoms */
    if ( !mol_data || !mol_data->ctab.MolAtom ||
         0 < mol_data->ctab.nNumberOfBonds && !mol_data->ctab.MolBond ||
         0 >= (num_atoms = mol_data->ctab.nNumberOfAtoms) ) {
        goto exit_function; /*  no structure */
    }
    /* copy atom info */
    for ( i = 0; i < num_atoms; i ++ ) {
        max_x = inchi_max(mol_data->ctab.MolAtom[i].fX, max_x);
        min_x = inchi_min(mol_data->ctab.MolAtom[i].fX, min_x);
        max_y = inchi_max(mol_data->ctab.MolAtom[i].fY, max_y);
        min_y = inchi_min(mol_data->ctab.MolAtom[i].fY, min_y);
        max_z = inchi_max(mol_data->ctab.MolAtom[i].fZ, max_z);
        min_z = inchi_min(mol_data->ctab.MolAtom[i].fZ, min_z);
    }

    /* copy bond info */
    num_bonds = 0;
    average_bond_length = 0.0;
    for ( i = 0; i < mol_data->ctab.nNumberOfBonds; i ++ ) {
        int  a1 = mol_data->ctab.MolBond[i].nAtomNo1-1;
        int  a2 = mol_data->ctab.MolBond[i].nAtomNo2-1;
        double dx = mol_data->ctab.MolAtom[a1].fX-mol_data->ctab.MolAtom[a2].fX;
        double dy = mol_data->ctab.MolAtom[a1].fY-mol_data->ctab.MolAtom[a2].fY;
        double dz = mol_data->ctab.MolAtom[a1].fZ-mol_data->ctab.MolAtom[a2].fZ;

        if ( a1 < 0 || a1 >= num_atoms ||
             a2 < 0 || a2 >= num_atoms ||
             a1 == a2 ) {
            *err |= 1; /*  bond for impossible atom number(s); ignored */
            MOLFILE_ERR_SET (*err, 0, "Bond to nonexistent atom");
            continue;
        }
        average_bond_length += sqrt( dx*dx + dy*dy + dz*dz );
        num_bonds ++;
    }

    /* convert to integral coordinates */

    if ( max_x - min_x <= small_coeff*(fabs(max_x) + fabs(min_x)) )
        x_coeff = 0.0;
    else
        x_coeff = (MAX_STDATA_X_COORD - MIN_STDATA_X_COORD)/(max_x - min_x);

    if ( max_y - min_y <= small_coeff*(fabs(max_y) + fabs(min_y)) )
        y_coeff = 0.0;
    else
        y_coeff = (MAX_STDATA_Y_COORD - MIN_STDATA_Y_COORD)/(max_y - min_y);
    if ( max_z - min_z <= small_coeff*(fabs(max_z) + fabs(min_z)) )
        z_coeff = 0.0;
    else
        z_coeff = (MAX_STDATA_Z_COORD - MIN_STDATA_Z_COORD)/(max_z - min_z);

    num_dimensions = ((x_coeff > macheps || y_coeff >macheps ) && fabs(z_coeff) < macheps)? 2:
                     (fabs(z_coeff) > macheps)? 3: 0;

    switch ( num_dimensions ) {
    case 0:
        coeff = 0.0;
        break;
    case 2:
        /* choose the smallest stretching coefficient */
        if ( x_coeff > macheps && y_coeff > macheps ) {
            coeff = inchi_min( x_coeff, y_coeff );
        }else
        if ( x_coeff > macheps ){
            coeff = x_coeff;
        }else
        if ( y_coeff > macheps ){
            coeff = y_coeff;
        }else{
            coeff = 1.0;
        }
        break;
    case 3:
        /* choose the smallest stretching coefficient */
        if ( x_coeff > macheps && y_coeff > macheps ) {
            coeff = inchi_min( x_coeff, y_coeff );
            coeff = inchi_min( coeff, z_coeff );
        }else
        if ( x_coeff > macheps ){
            coeff = inchi_min( x_coeff, z_coeff );
        }else
        if ( y_coeff > macheps ){
            coeff = inchi_min( y_coeff, z_coeff );
        }else{
            coeff = z_coeff;
        }
        break;
    default:
        coeff = 0.0;
    }

    if ( num_bonds > 0 ) {
        average_bond_length /= (double)num_bonds;
        if ( average_bond_length * coeff > MAX_STDATA_AVE_BOND_LENGTH ) {
            coeff = MAX_STDATA_AVE_BOND_LENGTH / average_bond_length; /* avoid too long bonds */
        } else
        if ( average_bond_length * coeff < macheps ) {
            coeff = 1.0; /* all lengths are of zero length */
        } else
        if ( average_bond_length * coeff < MIN_STDATA_AVE_BOND_LENGTH ) {
            coeff = MIN_STDATA_AVE_BOND_LENGTH / average_bond_length; /* avoid too short bonds */
        }
    }
#if( NORMALIZE_INP_COORD == 1 )
    /* set integral coordinates */
    for ( i = 0; i < num_atoms; i ++ ) {
        double x = mol_data->ctab.MolAtom[i].fX;
        double y = mol_data->ctab.MolAtom[i].fY;
        double z = mol_data->ctab.MolAtom[i].fZ;
        x = (x - min_x)*coeff + MIN_STDATA_X_COORD;
        y = (y - min_y)*coeff + MIN_STDATA_Y_COORD;
        z = (z - min_z)*coeff + MIN_STDATA_Z_COORD;
        /* floor() behavior is not well defined for negative arguments.
         * Use positive arguments only to get nearest integer.
         */
        at[i].x = ( x >= 0.0 )? (int)floor( x + 0.5 ) : -(int)floor( -x + 0.5 );
        at[i].y = ( y >= 0.0 )? (int)floor( y + 0.5 ) : -(int)floor( -y + 0.5 );
        at[i].z = ( z >= 0.0 )? (int)floor( z + 0.5 ) : -(int)floor( -z + 0.5 );
    }
#else
    /* set input coordinates */
    for ( i = 0; i < num_atoms; i ++ ) {
        double x = mol_data->ctab.MolAtom[i].fX;
        double y = mol_data->ctab.MolAtom[i].fY;
        double z = mol_data->ctab.MolAtom[i].fZ;
        at[i].x = x;
        at[i].y = y;
        at[i].z = z;
    }
#endif

exit_function:;
    return num_dimensions;
}
/****************************************************************************/
long GetMolfileNumber( MOL_HEADER_BLOCK *pHdr )
{
    static char sStruct[] = "Structure #";
    static char sINCHI[]  = INCHI_NAME;
    long   lMolfileNumber = 0;
    char   *p, *q = NULL;
    if ( pHdr ) {
        if ( !memicmp( pHdr->szMoleculeName, sStruct, sizeof(sStruct)-1 ) ) {
            p = pHdr->szMoleculeName + sizeof(sStruct)-1;
            lMolfileNumber = strtol( p, &q, 10 );
            p = pHdr->szMoleculeLine2;
            if ( !q || *q ||
                 memicmp( p, sINCHI, sizeof(sINCHI)-1) ||
                 !strstr( p+sizeof(sINCHI)-1, "SDfile Output" ) ) {
                lMolfileNumber = 0;
            }
        }
    }
    return lMolfileNumber;
}

/****************************************************************************/
int MolfileToInchi_Atom( FILE *inp_molfile, int bDoNotAddH, inchi_Atom **at, int max_num_at,
                      int *num_dimensions, int *num_bonds, const char *pSdfLabel, char *pSdfValue,
                      long *Id, long *lMolfileNumber, INCHI_MODE *pInpAtomFlags, int *err, char *pStrErr )
{
    int      num_atoms = 0;
    MOL_DATA *mol_data = NULL;
    MOL_HEADER_BLOCK OnlyHeaderBlock, *pOnlyHeaderBlock = NULL, *pHdr;
    MOL_CTAB         OnlyCtab,        *pOnlyCtab = NULL;
    char             cSdfValueFirstChar = '\0';
#ifdef CML_DEBUG
        FILE *f_p;
#endif
    if ( at ) {
        pOnlyHeaderBlock = NULL;
        if ( *at && max_num_at ) {
            memset( *at, 0, max_num_at * sizeof(**at) );
        }
    } else {
        pOnlyHeaderBlock = &OnlyHeaderBlock;
        pOnlyCtab        = &OnlyCtab;
    }
    if ( pSdfValue ) {
        cSdfValueFirstChar = pSdfValue[0];
        pSdfValue[0] = '\0';
    }

    mol_data = e_read_sdfile_segment(inp_molfile, pOnlyHeaderBlock, pOnlyCtab, 0, NULL, 0, Id, pSdfLabel, pSdfValue, err, pStrErr );

    pHdr = (  mol_data && !pOnlyHeaderBlock )? &mol_data->hdr :
           (  !mol_data && pOnlyHeaderBlock )? pOnlyHeaderBlock : NULL;
    if ( lMolfileNumber && pHdr ) {
         *lMolfileNumber = GetMolfileNumber( pHdr );
    }
    if ( pSdfValue &&
         pSdfLabel && pSdfLabel[0]  && pHdr ) {
         if ( !stricmp(pSdfLabel, "MOLFILENAME") ) {
             e_mystrncpy( pSdfValue, pHdr->szMoleculeName, MAX_SDF_VALUE+1 );
             e_LtrimRtrim( pSdfValue, NULL );
         } else
         if ( !stricmp(pSdfLabel, "MOLFILELINE2") ) {
             e_mystrncpy( pSdfValue, pHdr->szMoleculeLine2, MAX_SDF_VALUE+1 );
             e_LtrimRtrim( pSdfValue, NULL );
         } else
         if ( !stricmp(pSdfLabel, "MOLFILECOMMENT") ) {
             e_mystrncpy( pSdfValue, pHdr->szComment, MAX_SDF_VALUE+1 );
             e_LtrimRtrim( pSdfValue, NULL );
         }
         if ( !pSdfValue[0] ) {
             pSdfValue[0] = cSdfValueFirstChar;
         }
    }
    
    if ( mol_data && at && !*err ) {
        /* *at points to an allocated memory */
        if ( *at && mol_data->ctab.nNumberOfAtoms <= max_num_at ) {
            *at = mol_to_inchi_Atom( mol_data, &num_atoms, num_bonds, *at, bDoNotAddH, err, pStrErr );
            if ( *err >= 0 ) {
                *num_dimensions = mol_to_inchi_Atom_xyz( mol_data, num_atoms, *at, err, pStrErr );
            }
        } else
        /* *at points to NULL */
        if ( !*at && mol_data->ctab.nNumberOfAtoms <= max_num_at ) {
            *at = mol_to_inchi_Atom( mol_data, &num_atoms, num_bonds, *at, bDoNotAddH, err, pStrErr );
            if ( *err >= 0 ) {
                *num_dimensions = mol_to_inchi_Atom_xyz( mol_data, num_atoms, *at, err, pStrErr );
            }
        } else {
            MOLFILE_ERR_SET (*err, 0, "Too many atoms");
            *err = 70;
            num_atoms = -1;
        }
        if ( *err > 0 ) {
            *err += 100;
        }
        /* 11-16-2004: use Chiral flag */
        if ( num_atoms > 0 && at && *at && mol_data && pInpAtomFlags ) {
            if ( mol_data->ctab.cChiralFlag ) {
                *pInpAtomFlags |= FLAG_INP_AT_CHIRAL;
            } else {
                *pInpAtomFlags |= FLAG_INP_AT_NONCHIRAL;
            }
        }
    } else
    if ( !at ) {
        num_atoms = pOnlyCtab->nNumberOfAtoms;
    }

    if ( !pOnlyHeaderBlock ) {
        e_delete_mol_data( mol_data );
    }
#ifdef CML_DEBUG
        puts ("MOL");
        f_p = fopen ("mol.dbg", "a");
        if (f_p)
            {
                PrintInpAtom (f_p, *at, num_atoms);
                fclose (f_p);
            }
        else
            {
                puts ("Couldn't open file");
            }
#endif

    return num_atoms;
}
/**********************************************************************************/
int e_MolfileToInchi_Input( FILE *inp_molfile, inchi_Input *orig_at_data, int bMergeAllInputStructures,
                       int bDoNotAddH, int bAllowEmptyStructure,
                       const char *pSdfLabel, char *pSdfValue, long *lSdfId, long *lMolfileNumber,
                       INCHI_MODE *pInpAtomFlags, int *err, char *pStrErr )
{
    /* inp_ATOM       *at = NULL; */
    int            num_dimensions_new;
    int            num_inp_bonds_new;
    int            num_inp_atoms_new;
    inchi_Atom    *at_new     = NULL;
    inchi_Atom    *at_old     = NULL;
    int            nNumAtoms  = 0;
    int            i, j;

    if ( pStrErr ) {
        pStrErr[0] = '\0';
    }

    /*FreeOrigAtData( orig_at_data );*/

    do {

        at_old     = orig_at_data? orig_at_data->atom      : NULL; /*  save pointer to the previous allocation */
        num_inp_atoms_new =
            MolfileToInchi_Atom( inp_molfile, bDoNotAddH, orig_at_data? &at_new:NULL, MAX_ATOMS,
                          &num_dimensions_new, &num_inp_bonds_new,
                          pSdfLabel, pSdfValue, lSdfId, lMolfileNumber, pInpAtomFlags, err, pStrErr );


        if ( num_inp_atoms_new <= 0 && !*err ) {
            if ( !bAllowEmptyStructure ) {
                MOLFILE_ERR_SET (*err, 0, "Empty structure");  /* the message will be issued by the InChI library */
            }
            *err = 98;
        } else
        if ( orig_at_data && !num_inp_atoms_new && 10 < *err && *err < 20 && orig_at_data->num_atoms > 0 && bMergeAllInputStructures ) {
            *err = 0; /* end of file */
            break;
        } else
        if ( num_inp_atoms_new > 0 && orig_at_data ) {
            /*  merge pOrigDataTmp + orig_at_data => pOrigDataTmp; */
            nNumAtoms = num_inp_atoms_new + orig_at_data->num_atoms;
            if ( nNumAtoms >= MAX_ATOMS ) {
                MOLFILE_ERR_SET (*err, 0, "Too many atoms");
                *err = 70;
                orig_at_data->num_atoms = -1;
            } else
            if ( !at_old ) {
                /* the first structure */
                orig_at_data->atom    = at_new;
                at_new = NULL;
                orig_at_data->num_atoms  = num_inp_atoms_new;
            } else
            if ( orig_at_data->atom = e_CreateInchi_Atom( nNumAtoms ) ) {
                /*  switch at_new <--> orig_at_data->at; */
                if ( orig_at_data->num_atoms ) {
                    memcpy( orig_at_data->atom, at_old, orig_at_data->num_atoms * sizeof(orig_at_data->atom[0]) );
                    /*  adjust numbering in the newly read structure */
                    for ( i = 0; i < num_inp_atoms_new; i ++ ) {
                        for ( j = 0; j < at_new[i].num_bonds; j ++ ) {
                            at_new[i].neighbor[j] += orig_at_data->num_atoms;
                        }
                    }
                }
                e_FreeInchi_Atom( &at_old );
                /*  copy newly read structure */
                memcpy( orig_at_data->atom + orig_at_data->num_atoms,
                        at_new,
                        num_inp_atoms_new * sizeof(orig_at_data->atom[0]) );
                /*  add other things */
                orig_at_data->num_atoms += num_inp_atoms_new;
            } else {
                MOLFILE_ERR_SET (*err, 0, "Out of RAM");
                *err = -1;
            }
        } else
        if ( num_inp_atoms_new > 0 ) {
            nNumAtoms += num_inp_atoms_new;
        }
        e_FreeInchi_Atom( &at_new );

    } while ( !*err && bMergeAllInputStructures );
    /*
    if ( !*err ) {
        orig_at_data->num_components =
            MarkDisconnectedComponents( orig_at_data );
        if ( orig_at_data->num_components == 0 ) {
            MOLFILE_ERR_SET (*err, 0, "No components found");
            *err = 99;
        }
        if ( orig_at_data->num_components < 0 ) {
            MOLFILE_ERR_SET (*err, 0, "Too many components");
            *err = 99;
        }
    }
    */
    e_FreeInchi_Atom( &at_new );
    if ( *err ) {
        e_FreeInchi_Input( orig_at_data );
    }
    if ( *err && !(10 < *err && *err < 20) && pStrErr && !pStrErr[0] ) {
        MOLFILE_ERR_SET (*err, 0, "Unknown error");  /*   <BRKPT> */
    }
    return orig_at_data? orig_at_data->num_atoms : nNumAtoms;
}

