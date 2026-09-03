/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


/* inchi_dll_main.c : Defines the entry point for the DLL application. */

#include "../../../INCHI_BASE/src/mode.h"
#include "../../../INCHI_BASE/src/bcf_s.h"

#if defined(_WIN32) && defined(_USRDLL) && defined(_DEBUG) && !(defined(__STDC__) && __STDC__ == 1)
#include "inchi_dll_main.h"


/****************************************************************************/
int INCHI_DLLMAIN_TYPE DllMain( HANDLE hModule,
                                DWORD  ul_reason_for_call,
                                LPVOID lpReserved )
{
    return TRUE;
}
#else
int dummy_inchi_dll_main = 0;  /* avoid empty module to keep C compiler happy */
#endif
