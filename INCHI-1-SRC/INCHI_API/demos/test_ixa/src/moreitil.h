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


#ifndef _MOREUTIL_H_
#define _MOREUTIL_H_


#define MOLMAXLINELEN 1024




/* Just print date & time */
void print_time( void );
/* For portability - use own version of stricmp and memicmp */
int own_stricmp( const char *s1, const char *s2 );
int  own_memicmp( const void*, const void*, size_t );
/* Yet anothe helper */
char* get_substr_in_between( char *s,
                             char *pat1,
                             char *pat2,
                             char *buf,
                             size_t max_symbols,
                             size_t *copied );


/* (SD)File i/o related */

/* Read Molfile (SDFile segment) to text buffer */
/* Return 1 if got something otherwise 0        */
int get_next_molfile_as_text( FILE *f, char *buf, size_t buflen );
/* Fgets which ensures single linefeed at the end */
char* fgets_lf( char* line, int line_len, FILE *f );
int is_empty_text( char *buf );

#endif /* _MOREUTIL_H_ */
