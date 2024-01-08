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


#ifndef SYMBOLS_H
#define SYMBOLS_H

#ifdef LATE_BINDING
	#ifdef WIN32
		#ifdef CMLPP_DLL_EXPORTS
			#define CMLPP_API __declspec(dllexport)
            #define EXPIMP_TEMPLATE
		#else
			#define CMLPP_API __declspec(dllimport)
            #define EXPIMP_TEMPLATE extern
		#endif
	#else
		#define CMLPP_API
        #define EXPIMP_TEMPLATE
	#endif
#else
	#define CMLPP_API
    #define EXPIMP_TEMPLATE
#endif

#endif 	// SYMBOLS_H
