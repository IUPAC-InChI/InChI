/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
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
