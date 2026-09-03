/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef __INCHIERR_H__
#define __INCHIERR_H__

#define _IS_OKAY    0
#define _IS_WARNING 1
#define _IS_ERROR   2    /* Microsoft defined its own IS_ERROR() macro */
#define _IS_FATAL   3
#define _IS_UNKNOWN 4    /* unknown error: used in INChI DLL only */
#define _IS_EOF    -1    /* end of file */
#define _IS_SKIP   -2



#define INCHI_INP_ERROR_ERR   40
#define INCHI_INP_ERROR_RET  (-1)

#define INCHI_INP_FATAL_ERR    1
#define INCHI_INP_FATAL_RET    0

#define INCHI_INP_EOF_ERR     11
#define INCHI_INP_EOF_RET      0

#define LOG_MASK_WARN  1
#define LOG_MASK_ERR   2
#define LOG_MASK_FATAL 4

#define LOG_MASK_ALL     (LOG_MASK_WARN | LOG_MASK_ERR | LOG_MASK_FATAL)
#define LOG_MASK_NO_WARN (LOG_MASK_ERR | LOG_MASK_FATAL)

#endif /* __INCHIERR_H__ */
