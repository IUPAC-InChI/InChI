/*
 * CMLReader for InChI
 * Developed in collaboration with NIST and IUPAC
 * Version 1.0
 * Software version 1.0
 * 2004
 * Developed by Peter Murray-Rust and Simon (Billy) Tyrrell
 * Unilever Centre for Molecular Sciences Informatics
 * Department of Chemistry, University of Cambridge
 * Cambridge, CB2 1EW, UK
 *
 * CMLReader is free software 
 * you can redistribute this software and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software
 * Foundation:
 * http://www.opensource.org/licenses/lgpl-license.php
 */


#ifndef __PRAGMA_HPP__
#define __PRAGMA_HPP__

#if defined(_MSC_VER) && defined(WIN32)  // DTch 2004-06-19
// MS VC++ specific warnings sippression:
// C4786: 'identifier' : identifier was truncated to 'number' characters in the debug information
// C4251: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
// C4717: 'function' : recursive on all control paths, function will cause runtime stack overflow

// Notes:
// C4786 -- typical for STL under MS VC++
// C4251 -- I hope this is not a problem; so far everything works
// C4717 -- occurs under VC++.NET when encountering debug operator new redefinition

#pragma warning ( disable: 4786 4251 4717 4996 )
#endif

#endif
