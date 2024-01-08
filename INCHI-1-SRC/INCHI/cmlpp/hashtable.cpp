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
#include <string>

#include "hashtable.hpp"

using namespace std;

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
//
//  hashObject
//
//  This is a simple hashing function which should
//  hopefully be ok. It goes through the object a byte
//  at a time.
//
size_t hashObject (void *ptr, size_t size)
{
  size_t res = 0;
	const char *c_ptr = (reinterpret_cast <const char *> (ptr));

  while (size --)
    {
      res = (res << 1) ^ *c_ptr ++;
    }

  return res;
}

//
//  Hash C String
//
//  This is a simple hashing function which should
//  hopefully be ok. It goes through the object a byte
//  at a time.
//
size_t hashCString (void *ptr, size_t size)
{
	const char *c_ptr = (reinterpret_cast <const char *> (ptr));
  size_t res = 0;


  while (*c_ptr)
    {
      res = (res << 1) ^ *c_ptr ++;
    }

  return res;
}


//
//  Hash CPP String
//
//  This is a simple hashing function which should
//  hopefully be ok. It goes through the object a byte
//  at a time.
//
size_t hashCPPString (void *ptr, size_t size)
{
	const char *c_ptr = (reinterpret_cast <string *> (ptr)) -> c_str ();
	size_t res = 0;

  while (*c_ptr)
    {
      res = (res << 1) ^ *c_ptr ++;
    }

  return res;
}

}/*; extraneous semicolon commented out by DCh 1-18-2005 */	// namespace CML
