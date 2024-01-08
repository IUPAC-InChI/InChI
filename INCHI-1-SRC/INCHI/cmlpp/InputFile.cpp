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

#include <cstring> // Added 04-25-2010 (to work with gcc 4.3/4.4; thanks to Peter Linstrom)

#include "pragma.hpp"
#include "InputFile.hpp"

namespace CML
{


InputFile :: InputFile (FILE *file_p)
	: if_file_p (file_p)
{}


InputFile :: ~InputFile ()
{
	if (if_file_p)
		{
			;// fclose (if_file_p); // commented out 11-17-2005 by DT */ 
		}
}

#define FIX_CML_INPUT 1   /* DT 11-17-2005 */
bool InputFile :: getLine (string *buffer_p)
{
	char temp [2048] = { 0 }; // was 1024

	const bool success = (fgets (temp, sizeof(temp), if_file_p) != 0);

#if( FIX_CML_INPUT == 1 )
    if ( success ) {
        char *p, *q;
        int len;
        // remove cr
        q = temp;
        len = strlen(q);
        while ( len && (p = (char *)memchr( q, '\r', len )) ) {
            if( p[1] == '\n' ) {
                memmove( p, p+1, len-(p-q));
                len -= (p-q)+1;
                q = p;
            } else {
                *p = '\n';
                len -= (p-q)+1;
                q = p+1;
            }
        }
        // replace tabs with spaces
        q = temp;
        len = strlen(q);
        while ( len && (p = (char *)memchr( q, '\t', len )) ) {
            *p = ' ';
            q  = p+1;
            len -= (p-q) + 1;
        }
        
    } else {
        temp[0] = '\0';
    }
    *buffer_p = temp;
#else
	*buffer_p = temp;
#endif

	return success;
}



}	// namespace CML

