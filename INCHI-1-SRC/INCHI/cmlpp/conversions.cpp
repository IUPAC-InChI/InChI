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


#include "pragma.hpp"
#include "conversions.hpp"

#include <cstdio>
#include <cstring>
#include <vector>

#ifdef USE_MPATROL
#include "mpatrol.hpp"
#endif

#ifdef _VC6
  #include "vc_mem_leak.hpp"

  #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    #define THIS_FILE __FILE__
  #endif  //#ifdef _DEBUG
#endif  // #ifdef _VC6

namespace CML
{

char *fromDouble (const double &d)
{
	char buffer [32];
	/*sprintf (buffer, "%lf", d); -- replaced %lf with %f */
    sprintf (buffer, "%f", d);

	const size_t length = strlen (buffer);
	char *ptr = new char [length + 1];

	strcpy (ptr, buffer);
	return ptr;
}


char *fromInt (const int &i)
{
	char buffer [16];
	sprintf (buffer, "%d", i);

	const size_t length = strlen (buffer);
	char *ptr = new char [length + 1];

	strcpy (ptr, buffer);
	return ptr;
}


int tokenizeString (const string &str, const string &delim, char *** const tokens_ppp)
{
	size_t len = str.length ();

	if (len > 0)
		{
			char *s_p = new char [len + 1];

			strcpy (s_p, str.c_str ());
			* (s_p + len) = '\0';
			const char * const delim_p = delim.c_str ();
			char *ptr = strtok (s_p, delim_p);

			vector <char *> vec;

			while (ptr)
				{
					len = strlen (ptr);
					char *c_p = new char [len + 1];
					strcpy (c_p, ptr);
					* (c_p + len) = '\0';

					vec.push_back (c_p);
					ptr = strtok (0, delim_p);
				}

			const int num_strings = vec.size ();

			if (num_strings > 0)
				{
					*tokens_ppp = new char * [num_strings];

					char **tokens_pp = *tokens_ppp;

					for (int i = 0; i < num_strings; ++ i, ++ tokens_pp)
						{
							*tokens_pp = vec [i];
						}
				}

			delete [] s_p;

			return num_strings;
		}

	return 0;
}



}
