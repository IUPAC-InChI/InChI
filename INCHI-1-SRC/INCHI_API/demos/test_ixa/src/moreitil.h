/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
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
