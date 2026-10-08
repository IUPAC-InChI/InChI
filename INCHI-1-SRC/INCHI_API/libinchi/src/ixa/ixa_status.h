/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __IXA_STATUS_H__
#define __IXA_STATUS_H__

#include "../../../../INCHI_BASE/src/ixa.h"

void STATUS_PushMessage( IXA_STATUS_HANDLE hStatus,
                        IXA_STATUS        vSeverity,
                        char*             pFormat,
                        ... );

#endif
