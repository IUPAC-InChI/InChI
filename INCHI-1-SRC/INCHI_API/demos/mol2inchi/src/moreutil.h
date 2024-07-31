/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.07
 * April 30, 2024
 *
 * MIT License
 *
 * Copyright (c) 2024 IUPAC and InChI Trust
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * info@inchi-trust.org
 *
 */

#ifndef _MOREUTIL_H_
#define _MOREUTIL_H_


#define MOLMAXLINELEN 1024

#define my_va_start va_start
#define my_max(a,b)  (((a)>(b))?(a):(b))
#define my_min(a,b)  (((a)<(b))?(a):(b))



/* Just print date & time */
void print_time( void );
/* For portability - use own version of stricmp and memicmp */
int own_stricmp( const char *s1, const char *s2 );
int  own_memicmp( const void*, const void*, size_t );
/* Yet anothe helper */
char* get_substr_in_between( char *s, char *pat1, char *pat2, char *buf, size_t max_symbols, size_t *copied );



/************************************************/
/* GrowStr - growing (dynamic) string buffer    */
/************************************************/
#define GROWSTR_INITIAL_SIZE           128
/*1024*/
#define GROWSTR_MAX_SIZE               INT_MAX/2
#define GROWSTR_SIZE_INCREMENT         1024
/*
#define GROWSTR_OUT_OF_RAM             -1
#define GROWSTR_OK                     0
#define GROWSTR_ERROR                  1
#define GROWSTR_EOFRECORD              2
#define GROWSTR_READ_EOF               10
#define GROWSTR_READ_ERROR             11
*/

typedef struct GrowStr
{
    char *ps;
    int  allocated;
    int  used;
    int  incr;
} GrowStr;
int GrowStr_init( GrowStr *buf, int start_size, int incr_size );
int GrowStr_update( GrowStr *buf, int requested_addition );
int GrowStr_copy( GrowStr *dest, GrowStr *src );
int GrowStr_reset( GrowStr *buf );
void GrowStr_close( GrowStr *buf );
int GrowStr_printf( GrowStr *buf, const char* lpszFormat, ... );
int GrowStr_putc( GrowStr *buf, char c );
int GrowStr_readline( GrowStr *buf, FILE *f );


int get_next_molfile_as_growing_str_buffer( FILE *f, GrowStr *buf, int *at_eof );

unsigned int get_msec_timer( void );

#endif /* _MOREUTIL_H_ */
