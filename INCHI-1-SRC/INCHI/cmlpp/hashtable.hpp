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


#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <cstring>  // for memset ()
#include <iostream>

#include "symbols.hpp"

/*
#ifdef _VC6
	#include "vc_mem_leak.hpp"

	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif	//#ifdef _DEBUG
#endif	// #ifdef _VC6
*/

using namespace std;

/***
	Hashtable: a data structure allowing for fast key-based lookup.
*/

namespace CML {



/*********************/
/**** DEFINITIONS ****/
/*********************/

//#define HASH_DEBUG
//#define HASH_DEBUG2

const static unsigned int HT_DEF_CAPACITY 	= 101;
const static float HT_DEF_LOAD 			= 0.7f;

CMLPP_API size_t hashCString		(void *ptr, size_t size);
CMLPP_API size_t hashCPPString	(void *ptr, size_t size);
CMLPP_API size_t hashObject			(void *ptr, size_t size);

template <class KEYTYPE, class VALUETYPE>
class CMLPP_API HashTable
{
	friend ostream &operator << (ostream &output, const HashTable <KEYTYPE, VALUETYPE> &ht_r)
		{
			output << "cap " << ht_r.ht_capacity << " size " << ht_r.ht_size << " load " << ht_r.ht_load << endl;

			for (unsigned int i = 0; i < ht_r.ht_capacity; i ++)
				{
					output << "i " << i << " key " << (* (ht_r.ht_keys_p + i)) << " value ";

					if (* (ht_r.ht_values_pp + i))
						{
							output << * (* (ht_r.ht_values_pp + i));
						}
					else
						{
							output << " NULL";
						}
					output << endl;
				}
			return output;
		}

private:
  unsigned int	ht_size;						///< The number of objects in the hashtable
  unsigned int	ht_capacity;				///< The actual space allocated for the objects
  float     		ht_load;						///< How full to let the hash table get before rehashing
  size_t     	 *ht_hashed_keys_p;   ///< The hashvalues of the keys used for the putting and getting of objects
  KEYTYPE    	 *ht_keys_p;					///< The keys used for the putting and getting of objects
  VALUETYPE 	**ht_values_pp;				///< The objects in the table
  unsigned int  ht_load_limit;			///< The capacity * load
  bool     			ht_clone_flag;			///< Do we make copies of the objects that are put or not
	size_t				(*ht_hash_fn) (void *, size_t);
  void rehash 	();

public:
	/** Create a new HashTable with initial size capacity, load factor of load, using hash_fn to hash the key entries and cloning or storing objects depeneding upon clone. */
  HashTable               					(const unsigned int capacity = HT_DEF_CAPACITY, const float load = HT_DEF_LOAD, size_t	(*hash_fn) (void *, size_t) = hashObject, const bool clone = false);
  ~HashTable              					();

	/** Puts the VALUETYPE pointed to by obj_p in to the HashTable using key_r as the key. */
  void put                					(const KEYTYPE &key_r, const VALUETYPE *obj_p);

	/** Returns a pointer to the VALUETYPE associated with key_r in the HashTable, zero if it isn't in the HashTable. */
  const VALUETYPE *getValue 				(const KEYTYPE &key_r) const;

	/** Returns a pointer to the VALUETYPE associated with key_r in the HashTable and removes it from the HashTable.
		* Returns zero if it isn't in the HashTable.
		*/
  VALUETYPE *remValue 							(const KEYTYPE &key_r);

  /** Returns true if the KEYTYPE key_r is in the HashTable, false otherwise. */
  bool contains           				(const KEYTYPE &key_r) const;

  /** Returns the capacity of the HashTable */
  unsigned int getCapacity       					() const;

  /** Returns the used size of the HashTable */
  unsigned int getSize           					() const;

  //friend ostream &operator << (ostream &output, const HashTable <KEYTYPE, VALUETYPE> &ht_r);

};

/*********************/
/*** DECLARARTIONS ***/
/*********************/


//
//  Constructor
//
//  create a hashtable of capacity <capacity> and load <load>
//
template <class KEYTYPE, class VALUETYPE>
HashTable <KEYTYPE, VALUETYPE> :: HashTable (const unsigned int capacity, const float load, size_t	(*hash_fn) (void *, size_t), const bool clone)
  : ht_size (0), ht_capacity (capacity), ht_load (load), ht_clone_flag (clone), ht_hash_fn (hash_fn)
{
  ht_hashed_keys_p = new size_t [capacity];
  //^^^
  //memset (ht_hashed_keys_p, 0, capacity * sizeof (unsigned int));  // bug: in general, sizeof (unsigned int) != sizeof(size_t)
  memset (ht_hashed_keys_p, 0, capacity * sizeof (size_t)); // bug fixed 2008-11-15 DT
  //^^^

  ht_keys_p = new KEYTYPE [capacity];
  if ( hashObject == ht_hash_fn )  // added to avoid applying memset to an array of STL strings or other non-primitive objects - 2008-11-16 DT
	memset (ht_keys_p, 0, capacity * sizeof (KEYTYPE)); // uncommented to avoid compare to uninitialized in HashTable::getValue(...) -  2008-11-14 DT


  ht_values_pp = new VALUETYPE * [capacity];
  memset (ht_values_pp, 0, capacity * sizeof (VALUETYPE *));

  ht_load_limit = static_cast <unsigned int> (ht_capacity * ht_load);
}


//
//  Destructor
//
template <class KEYTYPE, class VALUETYPE>
HashTable <KEYTYPE, VALUETYPE> :: ~HashTable ()
{
  if (ht_clone_flag)
    {
      //VALUETYPE *ptr = *ht_values_pp;

      for (unsigned int i = 0; i < ht_capacity; i ++)
        {
          delete (* (ht_values_pp + i));
        }
    }

  delete [] ht_values_pp;
  ht_values_pp = 0;

  delete [] ht_keys_p;
  ht_keys_p = 0;

  delete [] ht_hashed_keys_p;
  ht_hashed_keys_p = 0;
}


//
//  put
//
//  put an object into the table with
//  the specified key
//
template <class KEYTYPE, class VALUETYPE>
void HashTable <KEYTYPE, VALUETYPE> :: put (const KEYTYPE &key_r, const VALUETYPE *obj_p)
{
  if (ht_size == ht_load_limit)
    {
      rehash ();
    }

  const unsigned int hashed_key = ht_hash_fn ((void *) (&key_r), sizeof (key_r));
  unsigned int i = hashed_key % ht_capacity;

	#ifdef HASH_DEBUG
		cerr << "i " << i << endl;
	#endif

  while (((* (ht_values_pp + i)) != 0))
    {
      if (++ i == ht_capacity)
        {
          i = 0;
        }
    }

	#ifdef HASH_DEBUG
		VALUETYPE *ptr = * (ht_values_pp + i);
		if (ptr)
			cerr << "value " << *ptr << endl;
		else
			cerr << "null ptr" << endl;
	#endif


      #ifdef HASH_DEBUG
        cerr << "putting " << *obj_p << " at " << i << endl;
      #endif

      * (ht_hashed_keys_p + i) = hashed_key;

      if (ht_clone_flag)
        {
          VALUETYPE *ptr = new VALUETYPE (*obj_p);
          (* (ht_values_pp + i)) = ptr;
        }
      else
        {
          (* (ht_values_pp + i)) = const_cast <VALUETYPE *> (obj_p);
        }

			 * (ht_keys_p + i) = key_r;

      ++ ht_size;



	#ifdef HASH_DEBUG
		ptr = * (ht_values_pp + i);
		if (ptr)
			cerr << "value " << *ptr << endl;
		else
			cerr << "null ptr" << endl;

	#endif

}


//
//  get value
//
//  gets an object with the specified key
//  or 0 if it isn't in the table
//
template <class KEYTYPE, class VALUETYPE>
const VALUETYPE *HashTable <KEYTYPE, VALUETYPE> :: getValue (const KEYTYPE &key_r) const
{
  const unsigned int hashed_key = ht_hash_fn ((void *) (&key_r), sizeof (key_r));

  unsigned int i = hashed_key % ht_capacity;

	#ifdef HASH_DEBUG
	cerr << "getting value for " << key_r << endl;
	#endif

 unsigned int count =0;

  while (((* (ht_keys_p + i )) != key_r) && ((* (ht_values_pp + i)) != 0) )
    {
			#ifdef HASH_DEBUG
			cerr << "i " << i << endl;
			#endif

		if (++ i == ht_capacity)
        {
          i = 0;
        }
		count ++;
		if (count == ht_capacity)
			return 0;

    }

  return ((* (ht_values_pp + i)) != 0 ? const_cast <const VALUETYPE *> (* (ht_values_pp + i)) : 0);
}


//
//  rem value
//
//  gets an object with the specified key
//  or 0 if it isn't in the table
//
template <class KEYTYPE, class VALUETYPE>
VALUETYPE *HashTable <KEYTYPE, VALUETYPE> :: remValue (const KEYTYPE &key_r)
{
  const unsigned int hashed_key = ht_hash_fn ((void *) (&key_r), sizeof (key_r));
	VALUETYPE *ptr = 0;
  unsigned int i = hashed_key % ht_capacity;

	#ifdef HASH_DEBUG
	cerr << "removing value for " << key_r << endl;
	#endif

  while (((* (ht_values_pp + i)) != 0) && ((* (ht_hashed_keys_p + i)) != hashed_key) && ((* (ht_keys_p + i )) != key_r))
    {
			#ifdef HASH_DEBUG
			cerr << "i " << i << endl;
			#endif

			if (++ i == ht_capacity)
				{
					i = 0;
				}
    }

	#ifdef HASH_DEBUG
		ptr = * (ht_values_pp + i);
		if (ptr)
			cerr << "value " << *ptr << endl;
		else
			cerr << "null ptr" << endl;
	#endif

  if ((* (ht_values_pp + i)) != 0)
    {
      #ifdef HASH_DEBUG
        cerr << "removing " << *obj_p << " at " << i << endl;
      #endif

      * (ht_hashed_keys_p + i) = 0;
      * (ht_keys_p + i) = 0;
			ptr = * (ht_values_pp + i);
			* (ht_values_pp + i) = 0;

      -- ht_size;

      return ptr;
    }
  else
  	{
  		return 0;
		}
}


//
//  contains
//
//  returns whether the object is in the table or not
//
template <class KEYTYPE, class VALUETYPE>
bool HashTable <KEYTYPE, VALUETYPE> :: contains (const KEYTYPE &key_r) const
{
  return (getValue (key_r) != 0);
}


//
//  get capacity
//
template <class KEYTYPE, class VALUETYPE>
unsigned int HashTable <KEYTYPE, VALUETYPE> :: getCapacity () const
{
  return ht_capacity;
}


//
//  get size
//
template <class KEYTYPE, class VALUETYPE>
unsigned int HashTable <KEYTYPE, VALUETYPE> :: getSize () const
{
  return ht_size;
}


//
//  rehash
//
//  recreate the table with a larger size
//
template <class KEYTYPE, class VALUETYPE>
void HashTable <KEYTYPE, VALUETYPE> :: rehash ()
{
  unsigned int new_capacity = (ht_capacity << 1) + 1;
  size_t     *new_hashed_keys_p = new size_t [new_capacity];
  KEYTYPE    *new_keys_p        = new KEYTYPE [new_capacity];
  VALUETYPE **new_values_pp     = new VALUETYPE * [new_capacity];

  memset( new_hashed_keys_p, 0, sizeof(size_t)  * new_capacity ); // added DT 2008-11-15
  if ( hashObject == ht_hash_fn )  // added to avoid applying memset to an array of STL strings or other non-primitive objects - 2008-11-16 DT
      memset( new_keys_p,        0, sizeof(KEYTYPE) * new_capacity ); // added DT 2008-11-15
  memset( new_values_pp, 0, sizeof(VALUETYPE *) * new_capacity ); // added DT 2006-08-11
  
  // resinsert the keys and values
  size_t     *old_hash_p    = ht_hashed_keys_p;
  KEYTYPE    *old_keys_p    = ht_keys_p;
  VALUETYPE **old_values_pp = ht_values_pp;

  for (unsigned int i = ht_capacity; i > 0; i --)
    {
      if (old_values_pp != 0)
        {
          unsigned int new_hash = *old_hash_p % new_capacity;

          while (* (new_values_pp + new_hash) != 0)
            {
              if (++ new_hash == new_capacity)
                new_hash = 0;
            }

          * (new_keys_p + new_hash)         = *old_keys_p;
          * (new_hashed_keys_p + new_hash)  = *old_hash_p;
          * (new_values_pp + new_hash)      = *old_values_pp;
        }

      ++ old_hash_p;
      ++ old_keys_p;
      ++ old_values_pp;
    }

  delete [] ht_keys_p;
  ht_keys_p = new_keys_p;

  delete [] ht_hashed_keys_p;
  ht_hashed_keys_p = new_hashed_keys_p;

  delete [] ht_values_pp;
  ht_values_pp = new_values_pp;

  ht_capacity = new_capacity;

  ht_load_limit = static_cast <unsigned int> (ht_capacity * ht_load);
}

/*
template <class KEYTYPE, class VALUETYPE>
friend ostream &operator << (ostream &output, const HashTable <KEYTYPE, VALUETYPE> &ht_r)
	{
		output << "cap " << ht_r.ht_capacity << " size " << ht_r.ht_size << " load " << ht_r.ht_load << endl;

		for (unsigned int i = 0; i < ht_r.ht_capacity; i ++)
			{
				output << "i " << i << " key " << (* (ht_r.ht_keys_p + i)) << " value ";

				if (* (ht_r.ht_values_pp + i))
					{
						output << * (* (ht_r.ht_values_pp + i));
					}
				else
					{
						output << " NULL";
					}
				output << endl;
			}
		return output;
	}
*/


}/*; extraneous semicolon commented out by DCh 1-18-2005 */ // namespace CML

#endif  /* HASH_TABLE_HPP */
