/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <locale.h>

#include "../../../INCHI_BASE/src/mode.h"

#if( BUILD_WITH_AMI == 1 && defined( _MSC_VER ) && MSC_AMI == 1 )
#include <malloc.h>
#include <io.h>
#endif


#ifdef _WIN32
#include <crtdbg.h>
#endif
#include "../../../INCHI_BASE/src/ichitime.h"
#include "../../../INCHI_BASE/src/incomdef.h"
#include "../../../INCHI_BASE/src/ichidrp.h"
#include "../../../INCHI_BASE/src/inpdef.h"
#include "../../../INCHI_BASE/src/ichi.h"
#include "../../../INCHI_BASE/src/strutil.h"
#include "../../../INCHI_BASE/src/util.h"
#include "../../../INCHI_BASE/src/ichierr.h"
#include "../../../INCHI_BASE/src/ichimain.h"
#include "../../../INCHI_BASE/src/ichicomp.h"
#include "../../../INCHI_BASE/src/ichi_io.h"
#ifdef TARGET_EXE_STANDALONE
#include "../../../INCHI_BASE/src/inchi_api.h"
#endif

#include "../../../INCHI_BASE/src/bcf_s.h"
#include "../../../INCHI_BASE/src/permutation_util.h"

/****************************************************************************/
int main(int argc, char* argv[])
{
#ifdef GHI100_FIX
#if ((SPRINTF_FLAG != 1) && (SPRINTF_FLAG != 2))
    setlocale(LC_ALL, "en-US"); /* djb-rwth: setting all locales to "en-US" */
#endif
#endif

/*************************/
// #if ( BUILD_WITH_AMI == 1 )
/*************************/

/**** IF IN AMI MODE, main() STARTS HERE ****/
    int i, ret = 0, ami = 0;  /* djb-rwth: ignoring LLVM warning: variable used to store function return value */

    /* Check if multiple inputs expected */
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == INCHI_OPTION_PREFX)
        {
            if (!inchi_stricmp(argv[i] + 1, "AMI"))
            {
                ami = 1;
                break;
            }
        }
    }

    if (ami)
    {
        ret = ProcessMultipleInputFiles(argc, argv); /* djb-rwth: ignoring LLVM warning: variable used to store function return value */
    }
    else
    {
        ret = ProcessSingleInputFile(argc, argv); /* djb-rwth: ignoring LLVM warning: variable used to store function return value */
    }

    return 0;
}


// #endif  /* ifndef TARGET_LIB_FOR_WINCHI */

