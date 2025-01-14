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


#pragma warning( disable : 4996 )

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>


#include "moreitil.h"

typedef unsigned char U_CHAR;

int replace_num_in_at_line( char *str, char *line, long num );
int replace_nums_in_bo_line( char *str, char *line, int shuffled_numbers[] );


/****************************************************************************/
/* Just print date & time */
void print_time( void )
{
    time_t tt;
    char st[36] = "";
    tt = time( &tt );
    strncat( st, (char *) ctime( &tt ), 32 );
    fprintf( stderr, "%-s", st );
}


/****************************************************************************/
/* For portability - own version of stricmp */
#define __MYTOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') : (c) )
int own_stricmp( const char *s1, const char *s2 )
{
    while (*s1)
    {
        if (*s1 == *s2 ||
              __MYTOLOWER( (int) *s1 ) == __MYTOLOWER( (int) *s2 ))
        {
            s1++;
            s2++;
        }
        else
        {
            return
                __MYTOLOWER( (int) *s1 ) - __MYTOLOWER( (int) *s2 );
        }
    }

    if (*s2)
    {
        return -1;
    }

    return 0;
}


/****************************************************************************/
int own_memicmp( const void * p1, const void * p2, size_t length )
{
    const U_CHAR *s1 = (const U_CHAR*) p1;
    const U_CHAR *s2 = (const U_CHAR*) p2;
    while (length--)
    {
        if (*s1 == *s2 ||
              __MYTOLOWER( (int) *s1 ) == __MYTOLOWER( (int) *s2 ))
        {
            s1++;
            s2++;
        }
        else
        {
            return
                __MYTOLOWER( (int) *s1 ) - __MYTOLOWER( (int) *s2 );
        }
    }

    return 0;
}


/****************************************************************************
   Extracts a substring sitting in 's' in
    between patterns 'pat1' and 'pat2'.
    Returns pointer to the end of pat2 plus 1.
    Fills in copied, the actual n bytes found.
    Null-terminated 'buf' contains not more
    than 'max_symbols' leading characters
    from found substring.
****************************************************************************/
char* get_substr_in_between( char *s,
                            char *pat1, char *pat2,
                            char *buf,
                            size_t max_symbols,
                            size_t *copied )
{
    size_t n = 0;
    char *p1 = NULL, *p1a = NULL, *p2 = NULL;

    *copied = 0;

    p1 = strstr( s, pat1 );
    if (!p1)
    {
        return NULL;
    }

    p1a = p1 + strlen( pat1 );

    if (!strcmp( pat2, "" ))
    {
        p2 = s + strlen( s ) - 1;
        n = 1;
    }
    else
    {
        p2 = strstr( p1a, pat2 );
    }
    if (!p2)
    {
        return NULL;
    }

    n += (int) ( p2 - p1a );

    if (n)
    {
        *copied = n;
        if (*copied > max_symbols)
        {
            *copied = max_symbols;
        }
        memcpy( buf, p1a, *copied );
        buf[( *copied )++] = '\0';
    }

    return p2 + 1;
}


/****************************************************************************
 Fgets which ensures single linefeed at the end
****************************************************************************/
char* fgets_lf( char* line, int line_len, FILE *f )
{
    char *p = NULL, *q;

        /* Read from file */
    memset( line, 0, line_len );
    if (NULL != ( p = fgets( line, line_len, f ) ) &&
         NULL == strchr( p, '\n' ))
    {
        char temp[64];
        /* bypass up to '\n' or up to end of file whichever comes first*/
        while (NULL != fgets( temp, sizeof( temp ), f ) &&
                NULL == strchr( temp, '\n' ))
            ;
    }

    /*  fix CR CR LF line terminator. */
    if (p)
    {
        if (q = strchr( line, '\r' ))
        {
            q[0] = '\n';
            q[1] = '\0';
        }
    }

    return p;
}


/****************************************************************************
 Read Molfile (SDFile segment) to text buffer
 Return 1 if got something, -1 if no room, otherwise 0
****************************************************************************/
int get_next_molfile_as_text( FILE *f, char *buf, size_t buflen )
{
    char *p = NULL, line[MOLMAXLINELEN];
    size_t pos = 0l, bufsize = 0;
    int empty = 1;

    while (p = fgets_lf( line, MOLMAXLINELEN, f ))
    {
        size_t n = strlen( line );
        if (n)
        {
            if (empty)
            {
                empty = 0;    /* Got something non-empty, do not mind the content */
            }

            if (buflen < pos + n - 1)
            {
                return -1; /* buffer buf is too small */
            }
            memcpy( buf + pos, line, n );
            pos += n;
            /* Check if we at the end of Molfile segment */
            if (line[0] == '$' && n > 3 &&
                 line[1] == '$' && line[2] == '$' && line[3] == '$')
            {
                break;
            }
        }
    }


    if (empty)
    {
        return 0;
    }
    else
    {
        buf[pos] = '\0';
        return 1;
    }
}


/****************************************************************************/
int is_empty_text( char *buf )
{
    while (*buf)
    {
        if (!isspace( *buf++ ))
        {
            return 0;
        }
    }

    return 1;
}
