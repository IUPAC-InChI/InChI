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


#ifndef IMWATCHINGYOULEAK
#define IMWATCHINGYOULEAK

#ifdef _DEBUG
	//#ifdef _CRTDBG_MAP_ALLOC
	//	#undef _CRTDBG_MAP_ALLOC
	//#endif

#include <crtdbg.h>

#define THIS_FILE __FILE__

inline void* __cdecl operator new(size_t nSize, const char * lpszFileName, int nLine)
{
	return ::operator new(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}

inline void __cdecl operator delete(void * _P, const char * lpszFileName, int nLine)
{
	::operator delete(_P, lpszFileName, nLine);
}

#define DEBUG_NEW       new(THIS_FILE, __LINE__)

#define malloc(s)       _malloc_dbg(s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
#define calloc(c, s)    _calloc_dbg(c, s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
#define realloc(p, s)   _realloc_dbg(p, s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
#define _expand(p, s)   _expand_dbg(p, s, _NORMAL_BLOCK, THIS_FILE, __LINE__)
#define free(p)         _free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)       _msize_dbg(p, _NORMAL_BLOCK)



#endif 	// #ifdef _DEBUG

#endif 	// #ifndef IMWATCHINGYOULEAK
