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


#ifndef _MOL2INCHI_H_
#define _MOL2INCHI_H_

#define MOLBUFSIZE 16777216 /* 16 MB */
#define INCHIPRINTOUTSIZE 100000

#define APP_DESCRIPTION "InChI version 1, Software v. 1.05 (mol2inchi - Library call example, API v. 1.05)"

/*#define RELEASE_IS_FINAL  0 *//* 1=> pre-release version; comment out to disable */
#ifndef RELEASE_IS_FINAL
#define RELEASE_IS_FINAL  1    /* final release */
#endif


/*
#define BUILD_WITH_ENG_OPTIONS 1
*/


struct WorkerDetails
{
    int do_not_print_inchi;
    int get_inchikey;
    int output_error_inchi;
    int do_benchmark;
    int nperms;
    int verbose;
    long tmax_shuffle;
    double dtmax_shuffle;
    char options[4096];    /* common ('old') API options */
    char* out;
    char* log;
    char* ikey;
    long rstart;
    long rend;
    long nmol;
    long n_inchi_err;
} WorkerDetails;

/* Actual worker */
int work_on_molfile_text( const char* text, struct WorkerDetails* details);

/* Even more actual worker */
int run_once_on_molfile_text( const char* mftext, struct WorkerDetails* details,
                              char *ikey, char *output, char *errstr);

/* Print program usage instructions */
void print_help(void);

/* Calculate InChIKey for the given InChI string */
int CalcPrintINCHIKEY( char *INCHI, char *ik_string );

#endif /* _MOL2INCHI_H_ */
