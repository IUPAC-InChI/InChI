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
 * http://www.opensource.org/licenses/lgpl-license.php
 */

#include <cstdlib> // Added 04-25-2010 (to work with gcc 4.3/4.4; thanks to Peter Linstrom)

#include "mode.h"

#if ( ADD_CMLPP == 1 )

#include "pragma.hpp"

#include "readcml.hpp"

#include <vector>

#include "CMLAtom.hpp"
#include "CMLAtomArray.hpp"
#include "CMLAtomArrayEditor.hpp"
#include "CMLBond.hpp"
#include "CMLBondArray.hpp"
#include "CMLBondArrayEditor.hpp"
#include "CMLDocument.hpp"
#include "CMLMolecule.hpp"
#include "CMLMoleculeEditor.hpp"
#include "hashtable.hpp"
#include "InputFile.hpp"
#include "NodeList.hpp"

#include "incomdef.h"
#include "inpdef.h"
#include "ichicomp.h"
#include "util.h"
#include "mol2atom.h"
#include "ichierr.h"

#ifdef CML_DEBUG
#include "debug.h"
#endif

#ifdef _VC6
  #include "vc_mem_leak.hpp"

  #ifdef _DEBUG
    //#define new DEBUG_NEW
    #undef THIS_FILE
    #define THIS_FILE __FILE__
  #endif  //#ifdef _DEBUG
#endif  // #ifdef _VC6

#ifdef MSC_DELAY_LOAD_CMLPPDLL

// The following SEH-handling code borrowed from:
// Module name: app.cpp
// Notices:     Written 1998 by Jeffrey Richter
// Description: Demonstration of delay-load DLL error handling.

// these 2 headers are nedded for delayed load of the cmlpp.dll
#include <windows.h>
#include <delayimp.h>

// in VC++ .NET (v.7.x) the folowing comment/options are not accepted by the linker
// Therefore options for the delayed cmlpp.dll load should be entered in the IDE:
// (1) Configuration Properties->Linker->Input->Delay Loaded DLLs=cmlpp.dll
// (2) Configuration Properties->Linker->Advanced->Delay Loaded DLLs=Support Unload
// As the result, the linker options include the following delay-load-specific items:
// /DELAYLOAD:"cmlpp.dll" /DELAY:UNLOAD DelayImp.lib
// The path/cmlpp.lib *IS* listed among other libraries to be linked

#if( _MSC_VER == 1200 )
// 1200 means MS VC++ 6.0 only
// Statically link __delayLoadHelper/__FUnloadDelayLoadedDLL
#pragma comment(lib, "Delayimp.lib")
/*==== the following three (the 3rd is commented out) do not work under VC++ 7.0 ====*/
// Tell the linker that my DLL should be delay loaded
#pragma comment(linker, "/DELAYLOAD:\"cmlpp.dll\"")
// Tell the linker that I want to be able to unload by DLL
#pragma comment(linker, "/DELAY:UNLOAD")
// Tell the linker to make the delay-load DLL unbindable
//#pragma comment(linker, "/Delay:nobind")
/*=========================================================*/
#endif

// fowrard declaration -- delayed load exception filte
LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep);
/*************************************************************
Source: 1998 by Jeffrey Richter
Description: Demonstration of delay-load DLL error handling.
*************************************************************/
LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep)
{
   LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;  // Assume we recognize this exception
   // If this is a Delay-load problem, ExceptionInformation[0] points 
   // to a DelayLoadInfo structure that has detailed error info
   PDelayLoadInfo pdli = (PDelayLoadInfo)pep->ExceptionRecord->ExceptionInformation[0];
   // Create a buffer where we construct error messages
   //char sz[500] = { 0 };
   switch (pep->ExceptionRecord->ExceptionCode) {
   case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
      //wsprintf(sz, "Dll not found: %s", pdli->szDll); // The DLL module was not found at runtime
      printf( "Dll not found: %s\n", pdli->szDll); // The DLL module was not found at runtime
      break;
   case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
      // The DLL module was found but it doesn't contain the function
      if (pdli->dlp.fImportByName) {
         //wsprintf(sz, "Function %s was not found in %s", pdli->dlp.szProcName, pdli->szDll);
         printf( "Function %s was not found in %s\n", pdli->dlp.szProcName, pdli->szDll);
      } else {
         //wsprintf(sz, "Function ordinal %d was not found in %s",pdli->dlp.dwOrdinal, pdli->szDll);
         printf( "Function ordinal %d was not found in %s",pdli->dlp.dwOrdinal, pdli->szDll);
      }
      break; 
   default:
      lDisposition = EXCEPTION_CONTINUE_SEARCH;  // We don't recognize this                                                      // exception
      break;
   }
   if (lDisposition == EXCEPTION_EXECUTE_HANDLER) {
      // We recognized this error and constructed a message, show it
      // MessageBox(NULL, sz, NULL, MB_OK|MB_TASKMODAL|MB_ICONEXCLAMATION|MB_TOPMOST );
   }
   return(lDisposition);
}
#endif

using namespace CML;

//CMLDocument s_doc;
CMLDocument *ps_doc = 0;
NodeList *s_mols_p = 0;
int s_current_mol_index = -1;
bool s_read_flag = false;
HashTable <string, AT_NUMB> **s_ids_map_pp = 0;

// local prototypes
bool GetMolecules (FILE *in_f);
int CmlAtomToInchi (const CMLAtom &cml_atom, inp_ATOM *inp_p, MOL_COORD * const coord_p, int * const err_p, char *pStrErr);
int CmlAtomStereoToInchi (const CMLAtom &cml_atom, inp_ATOM *inp_p, int * const err_p, char *pStrErr);
int CmlAtomArrayTolnchi (const CMLAtomArray &cml_atom_array, inp_ATOM *inp_atoms_p, int * const dims_p, MOL_COORD ** const coord_pp, int * const err_p, char *pStrErr);
bool CmlBondToInchi (const CMLBond &cml_bond, inp_ATOM *inp_p, int * const err_p, char *pStrErr);
bool CmlBondStereoToInchi (const CMLBond &cml_bond, inp_ATOM *inp_p, int * const err_p, char *pStrErr);
int CmlBondArrayTolnchi (const CMLBondArray &cml_bond_array, inp_ATOM *inp_atoms_p, int * const err_p, char *pStrErr);
CMLDocument *pGetCmlDoc( int n );
int CmlfileToOrigAtomCPP( FILE *inp_cmlfile, ORIG_ATOM_DATA *orig_at_data, int bMergeAllInputStructures,
                       int bGetOrigCoord, int bDoNotAddH, int inp_index, int *out_index,
                       char *pSdfLabel, char *pSdfValue, long *lSdfId, int *err, char *pStrErr );

class CMLDocumentPtr {
// provides object destruction upon (abnormal) exit from the program
// the dumbest "smart pointer" I could invent
public:
    CMLDocument *pDoc;
    // default constructor
    CMLDocumentPtr(){ pDoc = 0; }
    // default destructor
    ~CMLDocumentPtr(){ if ( pDoc ) { delete pDoc; pDoc = 0; }}
};


/*
CMLDocument *pGetCmlDoc( int n )
{
    static CMLDocument s_doc;
    return &s_doc;
}
*/
/**
**    This function should be called only once and
**    parses the entire contents of in_f into a
**  CMLDocument which is stored in s_doc and
**  stores the pointers to all of the molecules
**  within that document in s_mols_p.
**
**  @param in_f The input file
**  @returns true if molecules were found, false if not
*/
bool GetMolecules (FILE *in_f)
{

  InputFile input (in_f);
  //ps_doc is &s_doc: delayed instantiation of the CML Document;
  ps_doc = pGetCmlDoc( 0 );
  ps_doc->readXML (input);

    s_read_flag = true;

    Element *root_p = ps_doc->getRootElementPtr ();

/*
    cerr << "ROOT ELEMENT IS " << root_p << endl;

    vector <Node *> doc_children = s_doc.getChildVector ();

    for (int i = 0; i < doc_children.size (); ++ i)
        {
            cerr << "DOC " << i << " : " << doc_children [i] << endl;
        }
*/

    if (root_p)
        {
            NodeList *nodes_p  = root_p -> getElementsByTagName (C_MOLECULE);

            if (nodes_p)
                {
                    vector <Node *> v;
                    unsigned int num_nodes = nodes_p -> getLength ();
                    unsigned int i = 0;

                    for ( ; i < num_nodes; ++ i)
                        {
                            CMLMolecule *mol_p = dynamic_cast <CMLMolecule *> (nodes_p -> item (i));

                            if (mol_p)
                                {
                                    CMLMoleculeEditor *mol_ed_p = CMLMoleculeEditor :: getEditor (mol_p);

                                    CMLAtomArray *aa_p = mol_ed_p -> getAtomArray ();

                                    if (aa_p)
                                        {
                                            CMLAtomArrayEditor *aa_ed_p = CMLAtomArrayEditor :: getEditor (aa_p);

                                            if (aa_ed_p -> getNumberOfAtoms () > 0)
                                                {
                                                    v.push_back (mol_p);
                                                }
                                            delete aa_ed_p; // DTch -- memory leak ???
                                        }
                                        delete mol_ed_p; //DTch -- memory leak ???
                                }
                        }


                    if ((num_nodes = v.size()) > 0)
                        {
                            s_mols_p = new NodeList (v);
                        }

                    delete nodes_p;
                }

            if ((!s_mols_p) && (root_p -> getNodeName () == C_MOLECULE))
                {
                    s_mols_p = new NodeList (root_p, 1);
                }
        }

    return (s_mols_p != 0);
}

////////////////////////////////////////////////////////////////////////////////////
// Added to use MS Win SEH __try { } __except
int CmlfileToOrigAtom( FILE *inp_cmlfile, ORIG_ATOM_DATA *orig_at_data, int bMergeAllInputStructures,
                       int bGetOrigCoord, int bDoNotAddH, int inp_index, int *out_index,
                       char *pSdfLabel, char *pSdfValue, long *lSdfId, int *err, char *pStrErr )
{
#ifdef MSC_DELAY_LOAD_CMLPPDLL
#if _MSC_VER > 1200
    if (!s_read_flag)
        {
            //UINT uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX); -- does not work
            __try {
                if (FAILED(__HrLoadAllImportsForDll("cmlpp.dll")))
                {
                    *err = -1;
                    printf ( "failed on load \"cmlpp.dll\", exiting\n" );
                *err = -1;
                return 0;
                }
            }
            __except ( DelayLoadDllExceptionFilter(GetExceptionInformation()) )
            {
                *err = -1;
                return 0;
            }
        }
#else
    if (!s_read_flag)
        {
            // MS VC++ 6.0 crt does not have __HrLoadAllImportsForDll() so call a helper function
            // to test whether the dll can be loaded. This is an incomplete check: missing
            // functions will not be detected.
            __try {
                if (FAILED(TestLoadDll(0)))
                {
                    *err = -1;
                    printf ( "failed on load \"cmlpp.dll\", exiting\n" );
                *err = -1;
                return 0;
                }
            }
            __except ( DelayLoadDllExceptionFilter(GetExceptionInformation()) )
            {
                *err = -1;
                return 0;
            }
        }
#endif
#endif

    int ret = CmlfileToOrigAtomCPP( inp_cmlfile, orig_at_data, bMergeAllInputStructures,
                       bGetOrigCoord, bDoNotAddH, inp_index, out_index,
                       pSdfLabel, pSdfValue, lSdfId, err, pStrErr );
    *out_index = s_current_mol_index;
    return ret;
}


////////////////////////////////////////////////////////////////////////////////////
// Originally was CmlfileToOrigAtom()
int CmlfileToOrigAtomCPP( FILE *inp_cmlfile, ORIG_ATOM_DATA *orig_at_data, int bMergeAllInputStructures,
                       int bGetOrigCoord, int bDoNotAddH, int inp_index, int *out_index,
                       char *pSdfLabel, char *pSdfValue, long *lSdfId, int *err, char *pStrErr )
{
    // clear the error string
    //char *qqq = new char [100];

    if (pStrErr)
        {
            *pStrErr = '\0';
        }

    if (!s_read_flag)
        {
            GetMolecules (inp_cmlfile);

            if (s_mols_p)
                {
                    s_current_mol_index = 0;

                    const unsigned int num_mols = s_mols_p -> getLength ();
                    #ifdef CML_DEBUG
                    printf ("found %d mols in the cml file\n", num_mols);
                    #endif
                    unsigned int i = num_mols;

                    s_current_mol_index = 0;

                    HashTable <string, AT_NUMB> **ht_pp = s_ids_map_pp = new HashTable <string, AT_NUMB> * [num_mols];
                    for (i = num_mols; i > 0; -- i, ++ ht_pp)
                        {
                            (*ht_pp) = new HashTable <string, AT_NUMB> (HT_DEF_CAPACITY, HT_DEF_LOAD, hashCPPString, true);
                        }
                }
        }

    if (!s_mols_p)
        {
            MOLFILE_ERR_SET (*err, 0, "Empty Structure");
            *err = 98;

            return 0;
        }

    int            num_new_dimensions;
    int            num_new_bonds;
    int            num_new_atoms;
    inp_ATOM      *new_atoms_p     = 0;
    inp_ATOM      *old_atoms_p        = 0;
    int            num_atoms  = 0;
    MOL_COORD     *new_coords_p = 0;
    MOL_COORD     *old_coords_p = 0;
    int i, j;

    // loop through while we have no error
    // and are merging
    
    if ( inp_index >= 0 ) {
        /* INCHI_LIB specific: request for a paricular structure number */
        s_current_mol_index = inp_index;
    }

    if (s_current_mol_index >= int(s_mols_p -> getLength ()))
        {
        *err = INCHI_INP_EOF_ERR;
        return INCHI_INP_EOF_RET; /* end of file -- 2004-06-11 DTch */
        }

    if ( pSdfLabel ) pSdfLabel[0] = '\0';
    if ( pSdfValue ) pSdfValue[0] = '\0';
    if ( lSdfId )    *lSdfId      = 0;

    do {
        // save previous allocations
        old_atoms_p = orig_at_data ? orig_at_data -> at : 0;
        old_coords_p = orig_at_data ? orig_at_data -> szCoord : 0;

        num_new_atoms = CmlfileToInpAtom (inp_cmlfile, bDoNotAddH, orig_at_data ? &new_atoms_p : 0,
                            (bGetOrigCoord && orig_at_data)? &new_coords_p : 0, MAX_ATOMS,
                            &num_new_dimensions, &num_new_bonds, inp_index, out_index,
                            pSdfLabel, pSdfValue, lSdfId, err, pStrErr );

        #ifdef CML_DEBUG
        printf ("num new atoms %d\n", num_new_atoms);
        #endif

        if ((num_new_atoms <= 0) && (!*err)) {
            MOLFILE_ERR_SET (*err, 0, "Empty Structure");
            *err = 98;
        } else
        if ( orig_at_data && !num_new_atoms && 10 < *err && *err < 20 &&
             orig_at_data->num_inp_atoms > 0 && bMergeAllInputStructures ) {
            /* *err = 0;*/ /* end of file */
            *err = INCHI_INP_EOF_ERR; /* end of file -- 2004-06-11 DTch */
            break;
        } else
        if ( num_new_atoms > 0 && orig_at_data ) {
            //  merge pOrigDataTmp + orig_at_data => pOrigDataTmp;
            num_atoms = num_new_atoms + orig_at_data->num_inp_atoms;
            if ( num_atoms >= MAX_ATOMS ) {
                MOLFILE_ERR_SET (*err, 0, "Too many atoms");
                *err = 70;
                orig_at_data->num_inp_atoms = -1;
            } else
            if ( !old_atoms_p ) {
                /* the first structure */
                orig_at_data->at      = new_atoms_p;
                orig_at_data->szCoord = new_coords_p;
                new_atoms_p = NULL;
                new_coords_p = NULL;
                orig_at_data->num_inp_atoms  = num_new_atoms;
                orig_at_data->num_inp_bonds  = num_new_bonds;
                orig_at_data->num_dimensions = num_new_dimensions;
            } else
            if ( (orig_at_data->at = ( inp_ATOM* ) inchi_calloc( num_atoms, sizeof(inp_ATOM) )) &&
                 (!new_coords_p || (orig_at_data->szCoord = (MOL_COORD *) inchi_calloc( num_atoms, sizeof(MOL_COORD) ))) ) {
                /* add newly read structure to the previously accumulated structure (merge, append) */
                /*  switch at_new <--> orig_at_data->at; */
                if ( orig_at_data->num_inp_atoms ) {
                    memcpy( orig_at_data->at, old_atoms_p, orig_at_data->num_inp_atoms * sizeof(orig_at_data->at[0]) );
                    /*  adjust numbering in the newly read structure */
                    for ( i = 0; i < num_new_atoms; i ++ ) {
                        for ( j = 0; j < (new_atoms_p + i) -> valence; j ++ ) {
                            (new_atoms_p + i) -> neighbor[j] += orig_at_data->num_inp_atoms;
                        }
                        (new_atoms_p + i) -> orig_at_number += orig_at_data->num_inp_atoms; /* 12-19-2003 */
                    }
                    if ( orig_at_data->szCoord && old_coords_p ) {
                        memcpy( orig_at_data->szCoord, old_coords_p, orig_at_data->num_inp_atoms * sizeof(MOL_COORD) );
                    }
                }
                if ( old_atoms_p ) {
                    inchi_free( old_atoms_p );
                    old_atoms_p = NULL;
                }
                if ( old_coords_p ) {
                    inchi_free( old_coords_p );
                    old_coords_p = NULL;
                }
                /*  copy newly read structure */
                memcpy( orig_at_data->at + orig_at_data->num_inp_atoms,
                        new_atoms_p,
                        num_new_atoms * sizeof(orig_at_data->at[0]) );
                if ( orig_at_data->szCoord && new_coords_p ) {
                    memcpy( orig_at_data->szCoord + orig_at_data->num_inp_atoms,
                            new_coords_p,
                            num_new_atoms * sizeof(MOL_COORD) );
                }
                /*  add other things */
                orig_at_data->num_inp_atoms += num_new_atoms;
                orig_at_data->num_inp_bonds += num_new_bonds;
                orig_at_data->num_dimensions = inchi_max(num_new_dimensions, orig_at_data->num_dimensions);
            } else {
                MOLFILE_ERR_SET (*err, 0, "Out of RAM");
                *err = -1;
            }
        } else
        if (num_new_atoms > 0 )  {
            /* orig_at_data not requested */
            num_atoms += num_new_atoms;
        }

        if (new_atoms_p) {
            inchi_free (new_atoms_p);
            new_atoms_p = 0;
        }
        ++ s_current_mol_index;
    } while (!*err && bMergeAllInputStructures);

    /* prevent memory leaks in case of unexpected failures */
    if (new_atoms_p) {
        inchi_free (new_atoms_p);
        new_atoms_p = 0;
    }
    if ( new_coords_p ) {
        inchi_free( new_coords_p );
    }

    if ( !*err && orig_at_data ) {
        if ( ReconcileAllCmlBondParities( orig_at_data->at, orig_at_data->num_inp_atoms, 0 ) ) {
            MOLFILE_ERR_SET (*err, 0, "Cannot reconcile stereobond parities");  /*   <BRKPT> */
            if (!orig_at_data->num_dimensions) {
                *err = 1;
            }
        }
    }


    if (*err) {
        FreeOrigAtData( orig_at_data );

        if ( !(10 < *err && *err < 20) && pStrErr && !(*pStrErr) ) {
            MOLFILE_ERR_SET (*err, 0, "Unknown error");  /*   <BRKPT> */
        }
    }
    return 0; /* returned value should be 0 unless error -- DTCh 2004-06-10 */
}
//////////////////////////////////////////////////////////////////////
// n=0 => create or return a pointer to previously created CMLDocument
// n=1 => destroy
CMLDocument *pGetCmlDoc( int n )
{
    static CMLDocumentPtr s_doc_ptr;
    if ( n == 0 ) {
        if ( !s_doc_ptr.pDoc )
            s_doc_ptr.pDoc = new CMLDocument;
    } else {
        if ( s_doc_ptr.pDoc ) {
            delete s_doc_ptr.pDoc;
            s_doc_ptr.pDoc = 0;
        }
    }
    return s_doc_ptr.pDoc;
}
//////////////////////////////////////////////////////////
int FreeCmlDoc( int n )
{
    if ( n ) {
        CMLDocument *pDoc = pGetCmlDoc( n );
        ps_doc = 0;
        if ( pDoc )
            return 1; // cannot happen
        else
            return 0;
    }
    return 0;
}
//////////////////////////////////////////////////////////
/*
int GetCmlStructIndex( void )
{
    return s_current_mol_index;
}
*/
//////////////////////////////////////////////////////////
int SetCmlStructIndex( int index )
{
    int prev_index = s_current_mol_index;
    s_current_mol_index = index;
    return prev_index;
}
/////////////////////////////////////////////////////////
void FreeCml ()
{
    HashTable <string, AT_NUMB> **ht_pp = s_ids_map_pp;
    if (s_mols_p)
        {
            const unsigned int num_mols = s_mols_p -> getLength ();
            unsigned int i = num_mols;

            for ( ; i > 0; -- i, ++ ht_pp)
                {
                    delete (*ht_pp);
                }

            delete s_mols_p;
            // initialize
            s_mols_p = 0;
            s_current_mol_index = -1;
        }

    if (s_ids_map_pp)
        {
            delete [] s_ids_map_pp;
            // initialize
            s_ids_map_pp = 0;
        }
    s_read_flag = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int CmlfileToInpAtom (FILE *inp_cmlfile, int bDoNotAddH, inp_ATOM **inp_atoms_pp, MOL_COORD **szCoord,
                        int max_num_at, int *num_dimensions_p, int *num_bonds_p,
                        int inp_index, int *out_index, char *pSdfLabel,
                        char *pSdfValue, long *Id, int *err, char *pStrErr)
{
    CMLMolecule *cml_mol_p = dynamic_cast <CMLMolecule *> (s_mols_p -> item (s_current_mol_index));
    int num_new_atoms = 0;

    if (cml_mol_p)
        {
            CMLMoleculeEditor *editor_p = CMLMoleculeEditor :: getEditor (const_cast <CMLMolecule * const> (cml_mol_p));

            const CMLAtomArray *cml_atom_array_p = editor_p -> getAtomArray ();
            const CMLBondArray *cml_bond_array_p = editor_p -> getBondArray ();

            inp_ATOM *new_inp_atoms_p = 0;

            const string mol_id = cml_mol_p -> getId ();
            
            if ( pSdfValue && !pSdfValue[0] && pSdfLabel ) {
                // save strucure id
                const char *pValue = mol_id.c_str ();
                if ( pValue && pValue[0] ) {
                    strcpy( pSdfLabel, "id" );
                    strcpy( pSdfValue, pValue );
                }
            }
            
            if (cml_atom_array_p)
                {
                    CMLAtomArrayEditor *aa_editor_p = CMLAtomArrayEditor :: getEditor (const_cast <CMLAtomArray * const> (cml_atom_array_p));
                    num_new_atoms = aa_editor_p -> getNumberOfAtoms ();

                    #ifdef CML_DEBUG
                    printf ("num cml atoms %d\n", num_new_atoms);
                    #endif

                    delete aa_editor_p;

                    new_inp_atoms_p = CreateInpAtom (num_new_atoms);
                    if ( !new_inp_atoms_p ) {
                        MOLFILE_ERR_SET (*err, 0, "Out of RAM");
                        delete editor_p;
                        return 0;
                    }

                    // I can't see the code that guarantees that the if clause
                    // ensures that new_inp_atoms_p points to enough memory
                    // -- See it right above this

                    // allocate memory for the coords
                    // *szCoord = (MOL_COORD *) inchi_calloc (inchi_max (num_new_atoms, 1), sizeof (MOL_COORD));
                    if ( szCoord ) {
                        if ( *szCoord )
                            inchi_free( *szCoord);
                        *szCoord = (MOL_COORD *) inchi_calloc (inchi_max (num_new_atoms, 1), sizeof (MOL_COORD));
                    }

                    if (!CmlAtomArrayTolnchi (*cml_atom_array_p, new_inp_atoms_p, num_dimensions_p, szCoord, err, pStrErr))
                        {
                            MOLFILE_ERR_SET (*err, 0, "Couldn't convert AtomArray");
                            delete editor_p;
                            FreeInpAtom( &new_inp_atoms_p );
                            return 0;
                        }
                }

            *num_bonds_p = 0;
            if (cml_bond_array_p)
                {
                    if (! (*num_bonds_p = CmlBondArrayTolnchi (*cml_bond_array_p, new_inp_atoms_p, err, pStrErr)))
                        {
                            MOLFILE_ERR_SET (*err, 0, "Couldn't convert BondArray");
                            delete editor_p;
                            FreeInpAtom( &new_inp_atoms_p );
                            return 0;
                        }
                }

            delete editor_p;

            calculate_valences (0, new_inp_atoms_p, &num_new_atoms, bDoNotAddH, err, pStrErr);

            #ifdef CML_DEBUG
            puts ("CML");
            FILE *f_p = fopen ("cml.dbg", "w");
            if (f_p)
                {
                    PrintInpAtom (f_p, new_inp_atoms_p, num_new_atoms);
                    fclose (f_p);
                }
            else
                {
                    puts ("Couldn't open file");
                }
            #endif

            if ( inp_atoms_pp ) {
                *inp_atoms_pp = new_inp_atoms_p;
            } else {
                FreeInpAtom( &new_inp_atoms_p );
            }

        }
    else
        {
            char str [32];
            sprintf (str, "Couldn't get Molecule %d", s_current_mol_index);

            MOLFILE_ERR_SET (*err, 0, str);
            return 0;
        }

    return num_new_atoms;
}



/////
// ATOM ARRAY
/////

int CmlAtomArrayTolnchi (const CMLAtomArray &cml_atom_array, inp_ATOM *inp_atoms_p, int * const dims_p, MOL_COORD **szCoord, int * const err_p, char *pStrErr)
{
    CMLAtomArrayEditor *editor_p = CMLAtomArrayEditor :: getEditor (const_cast <CMLAtomArray * const> (&cml_atom_array));

    const unsigned int num_atoms = editor_p -> getNumberOfAtoms ();
    const CMLAtom *cml_atom_p;

    inp_ATOM *inp_p = inp_atoms_p;
    unsigned int i = 0;
    int min_dims = 1024;
    int dims;

    // OB counts atom from 1 not 0
    for ( ; i < num_atoms; ++ i, ++ inp_p /*, ++ coord_p*/)
        {
            // clear the input atom's values
            memset (inp_p, 0, sizeof (*inp_p));

            cml_atom_p = editor_p -> getAtom (i);
            dims = CmlAtomToInchi (*cml_atom_p, inp_p, szCoord? *szCoord+i : NULL, err_p, pStrErr); /* replaced 0 with szCoord -- DTch 2004-06-10 */
            if (dims > 0)
                {
                    if (dims < min_dims)
                        {
                            min_dims = dims;
                        }
                }
            /*  -- allow 0D structures -- DTch 2004-06-16
            else
                {
                    char str [32];
                    sprintf (str, "Couldn't convert Atom %d", i);

                    MOLFILE_ERR_SET (*err_p, -1, str);

                    delete editor_p;
                    return 0;
                }
            */
        }
    /* get atom tetrahedral stereo parities */
    for ( i=0; i < num_atoms; ++ i )
        {
            // get tetrahedral parities
            cml_atom_p = editor_p -> getAtom (i);

            if ( !CmlAtomStereoToInchi ( *cml_atom_p, inp_atoms_p, err_p, pStrErr) ) {
                XMLUtils::cmlError("p_parity->atomRefs4 No atoms given");
                return false; /* failed */
            }
        }


    delete editor_p;

    *dims_p = min_dims;

    return (static_cast <int> (num_atoms));
}


/////
// ATOM
/////


int CmlAtomToInchi (const CMLAtom &cml_atom, inp_ATOM *inp_p, MOL_COORD * const coord_p, int * const err_p, char *pStrErr)
{
    int dims = 0;

    // coordinates
    if (cml_atom.x3IsSet () && cml_atom.y3IsSet () && cml_atom.z3IsSet ())
        {
            inp_p -> x =  cml_atom.getX3 ();
            inp_p -> y =  cml_atom.getY3 ();
            inp_p -> z =  cml_atom.getZ3 ();

            dims = 3;
        }
    else if (cml_atom.x2IsSet () && cml_atom.y2IsSet ())
        {
            inp_p -> x =  cml_atom.getX2 ();
            inp_p -> y =  cml_atom.getY2 ();
            inp_p -> z =  0.0;

            dims = 2;
        }
    else
        {
            /* no coordinates present */
            inp_p -> x =
            inp_p -> y =
            inp_p -> z =  0.0;

        }
    if ( coord_p ) {
        /* store coordinates -- DTch 2004-06-10 */
        char str[32];
        WriteCoord( str, inp_p -> x );
        memcpy( *coord_p, str, 10 );
        WriteCoord( str, inp_p -> y );
        memcpy( *coord_p+10, str, 10 );
        WriteCoord( str, inp_p -> z );
        memcpy( *coord_p+20, str, 10 );
    }

    // formal charge
    if (cml_atom.formalChargeIsSet ())
        {
            inp_p -> charge =  cml_atom.getFormalCharge ();
        }

    // isotope
    /* moved below because (inp_p -> iso_atw_diff) is signed char and cannot hold atw > 127
     * We need (inp_p -> el_number) to find isotopic shift from isotopic mass
     */

    // radical
    if (cml_atom.spinMultiplicityIsSet ())
        {
            const int spin = cml_atom.getSpinMultiplicity ();
            switch (spin)
                {
                    case 1:
                        inp_p -> radical = RADICAL_SINGLET;
                        break;

                    case 2:
                        inp_p -> radical = RADICAL_DOUBLET;
                        break;

                    case 3:
                        inp_p -> radical = RADICAL_TRIPLET;
                        break;

                    default:
                        inp_p -> radical = 0;
                        break;
                }
        }
    else
        {
            inp_p -> radical = 0;
        }


    // element type
    if (cml_atom.elementTypeIsSet ())
        {
            const char * const elem_name_p = cml_atom.getElementType ().c_str ();

            size_t elem_length = strlen (elem_name_p);

            elem_length = (elem_length < ATOM_EL_LEN) ? elem_length : ATOM_EL_LEN - 1;

            strncpy (inp_p -> elname, elem_name_p, elem_length);
            * ((inp_p -> elname) + (elem_length + 1)) = '\0';

            inp_p -> el_number = get_periodic_table_number (elem_name_p);
            if (inp_p -> el_number == ERR_ELEM)
                {
                    inp_p -> el_number = 0;

                    *err_p |= 64;    // unknown element name
                    #ifdef CML_DEBUG
                    printf ("bad elem name %s\n", elem_name_p);
                    #endif
                    MOLFILE_ERR_SET (*err_p, 0, "Unknown element(s):");
                    MOLFILE_ERR_SET (*err_p, 0, elem_name_p);
                }
        }
    else
        {
                // 11-17-2005 DT
                *err_p |= 64;    // missing element name
                #ifdef CML_DEBUG
                printf ("bad elem name %s\n", elem_name_p);
                #endif
                MOLFILE_ERR_SET (*err_p, 0, "Missing element symbol");
        }
    // isotope
    /* moved here because (inp_p -> iso_atw_diff) is signed char and cannot hold atw > 127 */
    if (cml_atom.isotopeNumberIsSet ())
        {
            int atw_iso = cml_atom.getIsotopeNumber ();
            if ( atw_iso > 0 && inp_p -> el_number != ERR_ELEM ) {
                atw_iso -= get_atw_from_elnum((int)inp_p->el_number);
                inp_p->iso_atw_diff = atw_iso < 0? atw_iso : atw_iso+1;
            } else {
                inp_p -> iso_atw_diff = 0;
            }
        }
    else
        {
            inp_p -> iso_atw_diff = 0;
        }


    // hydrogen count
    if (cml_atom.hydrogenCountIsSet ())
        {
            inp_p -> num_H = cml_atom.getHydrogenCount ();
        }


    // atom id
    /* the following code have been replaced to accommodate for multiple access to same structure */
    /* HashTable doesn't feel good when asked to map one and same string on many different numbers */
    /*------------------------------------
    HashTable <string, AT_NUMB> * const ht_p = * (s_ids_map_pp + s_current_mol_index);
    AT_NUMB orig_at_number = ht_p -> getSize ();

    string s = cml_atom.getId ();
    ht_p -> put (s, &orig_at_number);
    inp_p -> orig_at_number = orig_at_number+1;
    --------------------------------------*/
    HashTable <string, AT_NUMB> * const ht_p = * (s_ids_map_pp + s_current_mol_index);

    string s = cml_atom.getId ();
    const AT_NUMB *p_at_no = ht_p -> getValue( s );
    if ( p_at_no ) {
        // mapping has already occurred
        inp_p -> orig_at_number = *p_at_no + 1;
    } else {
        // establish a new mapping
        AT_NUMB orig_at_number = ht_p -> getSize ();
        ht_p -> put (s, &orig_at_number);
        inp_p -> orig_at_number = orig_at_number+1;
    }
    return dims;
}


int CmlAtomStereoToInchi (const CMLAtom &cml_atom, inp_ATOM *inp_p, int * const err_p, char *pStrErr)
{

    HashTable <string, AT_NUMB> * const ht_p = * (s_ids_map_pp + s_current_mol_index);

    string s      = cml_atom.getId ();
    const AT_NUMB *p_at_no = ht_p -> getValue( s );
    //inp_atoms_p[at_no]
    CMLAtomParity *p_parity = cml_atom.getAtomParity();
    if ( p_at_no && p_parity && (p_parity->atomRefs4ValueIsSet()) ) {
        AT_NUMB at_no = *p_at_no;
        string p = p_parity->getPCDATA();
        double dp = strtod( p.c_str(), NULL );
        /* positive => clockwise when looking from the 1st atom => INChI parity = 2 */
        signed char   atom_parity = dp > 0.0? 2 : dp < 0.0? 1 : 0;
        if ( atom_parity ) {
            string s2 = p_parity->getAtomRefs4();  /* ... should be implemented later */
            AT_NUMB na;
            StringVector sv;
            XMLUtils::tokenize(sv, s2, CH_SPACE);
            if (sv.size() != MAX_NUM_STEREO_ATOM_NEIGH) {
                    XMLUtils::cmlError("p_parity->atomRefs4 No atoms given");
                    return false; /* failed */
            }
            inp_p[at_no].p_parity = atom_parity;
            for ( int j = 0; j < MAX_NUM_STEREO_ATOM_NEIGH; j ++ ) {
                na = *(ht_p -> getValue( sv[j] ));
                /*-- at this moment no neighbor is known yet therefore the check is meaningless --
                if ( !is_in_the_list( inp_p[at_no].neighbor, na, inp_p[at_no].valence ) ) {
                    return false;
                }
                */
                inp_p[at_no].p_orig_at_num[j] = inp_p[na].orig_at_number;
            }
        }
    }
    return true;
}


// BOND ARRAY

int CmlBondArrayTolnchi (const CMLBondArray &cml_bond_array, inp_ATOM * const inp_atoms_p, int * const err_p, char *pStrErr)
{
    CMLBondArrayEditor *editor_p = CMLBondArrayEditor :: getEditor (const_cast <CMLBondArray * const> (&cml_bond_array));

    const unsigned int num_bonds = editor_p -> getNumberOfBonds ();

    unsigned int i;


    for ( i = 0; i < num_bonds; ++ i)
        {
            const CMLBond *cml_bond_p = editor_p -> getBond (i);
            if (!CmlBondToInchi (*cml_bond_p, inp_atoms_p, err_p, pStrErr))
                {
                    char str [32];
                    sprintf (str, "Couldn't convert Bond %d", i);

                    MOLFILE_ERR_SET (*err_p, -1, str);

                    delete editor_p;
                    return 0;
                }
        }

    for ( i = 0; i < num_bonds; ++ i)
        {
            const CMLBond *cml_bond_p = editor_p -> getBond (i);
            if (!CmlBondStereoToInchi (*cml_bond_p, inp_atoms_p, err_p, pStrErr))
                {
                    char str [32];
                    sprintf (str, "Couldn't convert stereo Bond %d", i);

                    MOLFILE_ERR_SET (*err_p, -1, str);

                    delete editor_p;
                    return 0;
                }
        }

    delete editor_p;

    return (static_cast <int> (num_bonds));
}


/////
// BOND
/////

/*
** This routinr ABSOLUTELY depends upon the AtomArray/Atom
** routines having been run first as they fill in the lookup
** table that this function uses.
*/
bool CmlBondToInchi (const CMLBond &cml_bond, inp_ATOM *inp_p, int * const err_p, char *pStrErr)
{
    const pair <string, string> cml_ids = cml_bond.getAtomRefs2 ();

    HashTable <string, AT_NUMB> * const ht_p = * (s_ids_map_pp + s_current_mol_index);
    const AT_NUMB *first_atom_index_p = ht_p -> getValue (cml_ids.first);

    if (first_atom_index_p)
        {
            const AT_NUMB *second_atom_index_p = ht_p -> getValue (cml_ids.second);

            if (second_atom_index_p)
                {
                    inp_ATOM *first_atom_p = inp_p + (*first_atom_index_p);
                    inp_ATOM *second_atom_p = inp_p + (*second_atom_index_p);

                    if (first_atom_p -> valence < MAXVAL)
                        {
                            if (second_atom_p -> valence < MAXVAL)
                                {
                                    const int n1 = ((first_atom_p -> valence) ++);
                                    const int n2 = ((second_atom_p -> valence) ++);

                                    // connection
                                    (first_atom_p -> neighbor) [(int)n1]     = *second_atom_index_p;
                                    (second_atom_p -> neighbor) [(int)n2]     = *first_atom_index_p;

                                    // bond order
                                    if (cml_bond.orderIsSet ())
                                        {
                                            const string order = cml_bond.getOrder ();

                                            if (order == "1" || order == "S" || order == "s")
                                                {
                                                    (first_atom_p -> bond_type) [(int)n1] =
                                                    (second_atom_p -> bond_type) [(int)n2] = BOND_TYPE_SINGLE;
                                                }
                                            else if (order == "2" || order == "D" || order == "d")
                                                {
                                                    (first_atom_p -> bond_type) [(int)n1] =
                                                    (second_atom_p -> bond_type) [(int)n2] = BOND_TYPE_DOUBLE;
                                                }
                                            else if (order == "3" || order == "T" || order == "t")
                                                {
                                                    (first_atom_p -> bond_type) [(int)n1] =
                                                    (second_atom_p -> bond_type) [(int)n2] = BOND_TYPE_TRIPLE;
                                                }
                                            else if (order == "A" || order == "a")
                                                {
                                                    (first_atom_p -> bond_type) [(int)n1] =
                                                    (second_atom_p -> bond_type) [(int)n2] = BOND_TYPE_ALTERN;
                                                }
                                            else
                                                {
#ifndef INCHI_LIB
                                                    cerr << "unknown bond order" << endl;
#endif
                                                }
                                        }
                                     /*--
                                    // stereo --- wrong way of reading the string ---
                                    // wedge up, hatch down
                                    if (cml_bond.stereoIsSet ())
                                        {
                                            const string stereo = cml_bond.getStereo ();

                                            if (stereo.compare ("W") || stereo.compare ("w"))
                                                {
                                                    (first_atom_p -> bond_stereo) [n1] = STEREO_SNGL_UP;
                                                    (second_atom_p -> bond_stereo) [n2] = -STEREO_SNGL_UP;
                                                }
                                            else if (stereo.compare ("H") || stereo.compare ("h"))
                                                {
                                                    (first_atom_p -> bond_stereo) [n1] = STEREO_SNGL_DOWN;
                                                    (second_atom_p -> bond_stereo) [n2] = -STEREO_SNGL_DOWN;
                                                }
                                            else
                                                {
                                                    (first_atom_p -> bond_stereo) [n1] =
                                                    (second_atom_p -> bond_stereo) [n2] = 0;
                                                }
                                        }

                                    else
                                        {
                                            (first_atom_p -> bond_stereo) [n1] =
                                            (second_atom_p -> bond_stereo) [n2] = 0;
                                        }
                                    --*/
                                    // stereo --- right way of reading the string !!!! ---
                                    CMLBondStereo *bs = cml_bond.getBondStereo();
                                    if ( bs) {
                                        string stereo = bs->getPCDATA(); // W, H, T, C
                                        if (!stereo.compare ("W") )
                                            {
                                                (first_atom_p -> bond_stereo) [n1] = STEREO_SNGL_UP;
                                                (second_atom_p -> bond_stereo) [n2] = -STEREO_SNGL_UP;
                                            }
                                        else if (!stereo.compare ("H") )
                                            {
                                                (first_atom_p -> bond_stereo) [n1] = STEREO_SNGL_DOWN;
                                                (second_atom_p -> bond_stereo) [n2] = -STEREO_SNGL_DOWN;
                                            }
                                        else
                                            {
                                                (first_atom_p -> bond_stereo) [n1] =
                                                (second_atom_p -> bond_stereo) [n2] = 0;
                                            }
                                    }

                                }
                            else
                                {
                                    char str [64];

                                    sprintf (str, "Atom %s has more than %d bonds", cml_ids.second.c_str (), MAXVAL);
                                    MOLFILE_ERR_SET (*err_p, -1, str);

                                    return false;
                                }
                        }
                    else
                        {
                            char str [64];

                            sprintf (str, "Atom %s has more than %d bonds", cml_ids.first.c_str (), MAXVAL);
                            MOLFILE_ERR_SET (*err_p, -1, str);

                            return false;
                        }
                }
            else
                {
                    char str [48];

                    sprintf (str, "Couldn't get CMLAtom with id %s", cml_ids.second.c_str ());
                    MOLFILE_ERR_SET (*err_p, -1, str);

                    return false;
                }
        }
    else
        {
            char str [48];

            sprintf (str, "Couldn't get CMLAtom with id %s", cml_ids.first.c_str ());
            MOLFILE_ERR_SET (*err_p, -1, str);

            return false;
        }


    return true;
}

/*
** This routinr ABSOLUTELY depends upon the AtomArray/Atom
** routines having been run first as they fill in the lookup
** table that this function uses.
*/
bool CmlBondStereoToInchi (const CMLBond &cml_bond, inp_ATOM *inp_p, int * const err_p, char *pStrErr)
{

    CMLBondStereo *bs = cml_bond.getBondStereo();

    if ( bs ) {
        string stereo = bs->getPCDATA(); // W, H, T, C
        if ( !stereo.compare("T") || !stereo.compare("C") ) {
            const pair <string, string> cml_ids = cml_bond.getAtomRefs2 ();
            HashTable <string, AT_NUMB> * const ht_p = * (s_ids_map_pp + s_current_mol_index);
            const AT_NUMB *first_atom_index_p  = ht_p -> getValue (cml_ids.first);
            const AT_NUMB *second_atom_index_p = ht_p -> getValue (cml_ids.second);
            if ( first_atom_index_p && second_atom_index_p ) {
                inp_ATOM *first_atom_p = inp_p + (*first_atom_index_p);
                inp_ATOM *second_atom_p = inp_p + (*second_atom_index_p);
                const AT_NUMB *p_n1 = is_in_the_list( first_atom_p->neighbor, *second_atom_index_p, first_atom_p->valence );
                const AT_NUMB *p_n2 = is_in_the_list( second_atom_p->neighbor, *first_atom_index_p, second_atom_p->valence );
                if ( p_n1 && p_n2 ) {
                    //const char n1 = p_n1 - first_atom_p->neighbor;
                    //const char n2 = p_n2 - second_atom_p->neighbor;
                    string atomRefs4 = bs->getAtomRefs4();
                    AT_NUMB na[CML_NUM_AT_IN_ATREF4], tmp, *p_na[CML_NUM_AT_IN_ATREF4];
                    int     j1, j2;
                    {
                        int j;
                        StringVector sv;
                        XMLUtils::tokenize(sv, atomRefs4, CH_SPACE);

                        if (sv.size() != CML_NUM_AT_IN_ATREF4) {
                                XMLUtils::cmlError("bond parity: not 4 atoms");
                                return false; /* failed */
                        }
                        //inp_atoms_p[at_no].parity = atom_parity;
                        for ( j = 0; j < CML_NUM_AT_IN_ATREF4; j ++ ) {
                            na[j] = *(ht_p -> getValue( sv[j] ));
                        }
                    }
                    if ( na[2] == *first_atom_index_p && na[1] == *second_atom_index_p ) {
                        /* reverse 0-1-2-3 to 3-2-1-0 */
                        tmp = na[2]; na[2] = na[1]; na[1] = tmp;
                        tmp = na[3]; na[3] = na[0]; na[0] = tmp;
                    } else
                    if ( na[1] != *first_atom_index_p || na[2] != *second_atom_index_p ) {
                        XMLUtils::cmlError("bond parity: wrong atom sequence");
                        return false; /* failed */
                    }
                    /* ord. no. of the first atom neighbor adjacent to the stereo bond */
                    p_na[0] = is_in_the_list( first_atom_p->neighbor, na[0], first_atom_p->valence );
                    /* ord. no. of the stereo bond WRT first atom */
                    p_na[2] = is_in_the_list( first_atom_p->neighbor, na[2], first_atom_p->valence );
                    /* ord. no. of the second atom neighbor adjacent to the stereo bond */
                    p_na[3] = is_in_the_list( second_atom_p->neighbor, na[3], second_atom_p->valence );
                    /* ord. no. of the stereo bond WRT second atom */
                    p_na[1] = is_in_the_list( second_atom_p->neighbor, na[1], second_atom_p->valence );
                    if ( !p_na[0] || !p_na[1] || !p_na[2] || !p_na[3] ) {
                        XMLUtils::cmlError("bond parity: wrong neighbor");
                        return false; /* failed */
                    }
                    for ( j1 = 0; j1 < MAX_NUM_STEREO_BONDS && first_atom_p->sb_parity[j1]; j1 ++ )
                        ;
                    for ( j2 = 0; j2 < MAX_NUM_STEREO_BONDS && second_atom_p->sb_parity[j2]; j2 ++ )
                        ;
                    if ( j1 >= MAX_NUM_STEREO_BONDS || j2 >= MAX_NUM_STEREO_BONDS ) {
                        XMLUtils::cmlError("too many stereo bonds to an atom");
                        return false; /* failed */
                    }
                    
                    first_atom_p->sn_orig_at_num[j1]  = inp_p[na[0]].orig_at_number;
                    first_atom_p->sn_ord[j1]          = p_na[0] - first_atom_p->neighbor;
                    first_atom_p->sb_ord[j1]          = p_na[2] - first_atom_p->neighbor;
                    first_atom_p->sb_parity[j1]       = 1;
                    
                    second_atom_p->sn_orig_at_num[j2] = inp_p[na[3]].orig_at_number;
                    second_atom_p->sn_ord[j2]         = p_na[3] - second_atom_p->neighbor;
                    second_atom_p->sb_ord[j2]         = p_na[1] - second_atom_p->neighbor;
                    second_atom_p->sb_parity[j2]      = 1;
                    /* same parities => trans, different parities => cis */
                    if ( !stereo.compare("C") ) {
                        second_atom_p->sb_parity[j2] = 2;
                    }
                    return true;
                }
            }
        } else {
            return true; // not S or T, ignored
        }
    } else {
        return true;  // no stereo bond info, ignored
    }
    return false; // otherwise error
}
#endif
