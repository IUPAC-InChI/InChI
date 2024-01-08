/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.05
 * January 27, 2017
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
 * Copyright (C) IUPAC and InChI Trust Limited
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
 * Licence No. 1.0 with this library; if not, please write to:
 *
 * The InChI Trust
 * 8 Cavendish Avenue
 * Cambridge CB1 7US
 * UK
 *
 * or e-mail to alan@inchi-trust.org
 *
 */


#ifndef __ICHI_PARMS_H__
#define __ICHI_PARMS_H__

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

int e_ReadCommandLineParms( int argc,
                            const char *argv[],
                            INPUT_PARMS *ip,
                            char *szSdfDataValue,
                            unsigned long *ulDisplTime,
                            int bReleaseVersion,
                            INCHI_IOSTREAM *log_file );
int e_PrintInputParms( INCHI_IOSTREAM *log_file,
                       INPUT_PARMS *ip );
int e_OpenFiles( FILE **inp_file,
                 FILE **output_file,
                 FILE **log_file,
                 FILE **prb_file,
                 INPUT_PARMS *ip );
void e_HelpCommandLineParms(INCHI_IOSTREAM *f );
#ifdef CREATE_INCHI_STEP_BY_STEP
void e_HelpCommandLineParmsReduced(INCHI_IOSTREAM *f );
#endif


#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* __ICHI_PARMS_H__ */
