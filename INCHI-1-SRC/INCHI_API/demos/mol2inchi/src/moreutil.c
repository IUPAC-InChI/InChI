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

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "moreutil.h"


typedef unsigned char U_CHAR;

static int GetMaxPrintfLength( const char *lpszFormat, va_list argList );


/****************************************************************************
 Just print date & time
****************************************************************************/
void print_time( void )
{
    time_t tt;
    char st[36] = "";
    tt = time( &tt );
    strncat( st, (char *) ctime( &tt ), 32 );
    fprintf( stderr, "%-s", st );
}


/****************************************************************************
 For portability - own version of stricmp
****************************************************************************/
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
int own_memicmp( const void *p1, const void *p2, size_t length )
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
                             char *pat1,
                             char *pat2,
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
        return NULL;

    n += (int) ( p2 - p1a );

    if (n)
    {
        *copied = n;
        if (*copied > max_symbols)
            *copied = max_symbols;
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
        if ( 0 != (q=strchr( line, '\r' )) )
        {
            q[0] = '\n';
            q[1] = '\0';
        }
    }

    return p;
}


/****************************************************************************
 GrowStr - dynamically growing string buffer
****************************************************************************/


/****************************************************************************
 Make new buffer; returns 1 if OK, 0 on error
****************************************************************************/
int GrowStr_init( GrowStr *buf, int start_size, int incr_size )
{
    char *new_str = NULL;
    memset( buf, 0, sizeof( *buf ) );
    if (start_size <= 0)
    {
        start_size = GROWSTR_INITIAL_SIZE;
    }
    if (incr_size <= 0)
    {
        incr_size = GROWSTR_SIZE_INCREMENT;
    }
    new_str = (char *) calloc( start_size, sizeof( char ) );
    if (!new_str)
    {
        return 0;
    }
    buf->ps = new_str;
    buf->allocated = start_size;
    buf->incr = incr_size;

    return 1;
}


/****************************************************************************
 Deep copy; returns 1 if OK, 0 on error
****************************************************************************/
int GrowStr_copy( GrowStr *dest, GrowStr *src )
{
    int res = 1;
    if (src->used < 1)
    {
        return 1;
    }
    if (dest->allocated < src->allocated)
    {
        res = GrowStr_update( dest, src->allocated - dest->allocated );
    }
    if (!res)
    {
        return 0;
    }
    memcpy( dest->ps, src->ps, src->used * sizeof( src->ps[0] ) );

    return 1;
}


/****************************************************************************
 Reset GrowStr (zeroing memory but not freing it); return 1 if OK, 0 on error
****************************************************************************/
int GrowStr_reset( GrowStr *buf )
{
    if (buf && buf->ps)
    {
        /* memset( buf->ps, 0, buf->allocated*sizeof(buf->ps[0]) );
        for speed, changed to: */
        buf->ps[0] = '\0';
        buf->used = 0;
    }
    else
    {
        return 0;
    }

    return 1;
}


/****************************************************************************
 Close buffer
****************************************************************************/
void GrowStr_close( GrowStr *buf )
{
    if (!buf)
    {
        return;
    }
    if (buf->ps)
    {
        free( buf->ps );
    }
    memset( buf, 0, sizeof( *buf ) );
}


/****************************************************************************
 Update buffer (length);; returns 1 if OK, 0 on error
****************************************************************************/
int GrowStr_update( GrowStr *buf, int requested_addition )
{
    int size_increment, new_size;
    char *new_str;

    if (!buf)
    {
        return 0;
    }
    if (!( buf->ps ))
    {
        return 0;
    }
    if (requested_addition < 1)
    {
        return 1;
    }

    if (( buf->used + requested_addition ) < buf->allocated)
    {
        return 1;
    }
    size_increment = my_max( buf->allocated / 2, requested_addition );
    new_size = buf->allocated + size_increment;
    if (new_size > GROWSTR_MAX_SIZE)
    {
        size_increment = my_min( buf->allocated / 2, requested_addition );
    }

    new_size = buf->allocated + size_increment;
    if (new_size > GROWSTR_MAX_SIZE)
    {
        return 0;
    }

    new_str = (char *) calloc( new_size, sizeof( new_str[0] ) );
    if (!new_str)
    {
        return 0;
    }
    if (buf->used > 0)
    {
        memcpy( new_str, buf->ps, sizeof( new_str[0] )* buf->used );
    }

    free( buf->ps );
    buf->ps = new_str;
    buf->allocated = new_size;

    return 1;
}


/****************************************************************************
 (replaces GrowStr_printf while reading )
****************************************************************************/
int GrowStr_putc( GrowStr *buf, char c )
{
    int n_printed = 0, to_add = 1;
    if (!buf)
    {
        return 0;
    }
    if (!GrowStr_update( buf, to_add ))
    {
        return 0;
    }
    n_printed = sprintf( buf->ps + buf->used, "%c", c );
    if (n_printed == 1)
    {
        buf->used += n_printed;
    }

    return 1;
}


/****************************************************************************
 Print; returns 1 if OK, 0 on error
****************************************************************************/
int GrowStr_printf( GrowStr *buf, const char* lpszFormat, ... )
{
    int max_len, n_printed, to_add;
    va_list argList;
    if (!buf)
    {
        return 0;
    }
    my_va_start( argList, lpszFormat );
    max_len = GetMaxPrintfLength( lpszFormat, argList );
    va_end( argList );

    if (max_len <= 0)
    {
        return 1; /* nothing to do is not an error */
    }

    to_add = max_len + 2;
    if (!GrowStr_update( buf, to_add ))
    {
        return 0;
    }

    my_va_start( argList, lpszFormat );
    n_printed = vsprintf( buf->ps + buf->used, lpszFormat, argList );
    va_end( argList );
    if (n_printed >= 0)
    {
        buf->used += n_printed;
    }

    return 1;
}


/****************************************************************************
 Read line from file; returns 1 if OK, 0 on error, -1 on EOF
****************************************************************************/
int GrowStr_readline( GrowStr *buf, FILE *f )
{
    int c;
    while (1)
    {
        c = fgetc( f );
        if (ferror( f ))
        {
            return 0;
        }
        if (c == EOF)
        {
            return -1;
        }
        GrowStr_putc( buf, (char) c );
        if (c == '\n')
        {
            break;
        }
    }

    return 1;
}


/****************************************************************************
    Estimate printf string length.
    The code is based on Microsoft Knowledge Base article Q127038:
    "FIX: CString::Format Gives Assertion Failed, Access Violation"
    (Related to Microsoft Visual C++, 32-bit Editions, versions 2.0, 2.1)
****************************************************************************/
#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
int GetMaxPrintfLength( const char *lpszFormat, va_list argList )
{
     /*ASSERT(AfxIsValidString(lpszFormat, FALSE));*/
    const char * lpsz;
    int nMaxLen, nWidth, nPrecision, nModifier, nItemLen;

    nMaxLen = 0;
    /* make a guess at the maximum length of the resulting string */
    for (lpsz = lpszFormat; *lpsz; lpsz++)
    {
         /* handle '%' character, but watch out for '%%' */
        if (*lpsz != '%' || *( ++lpsz ) == '%')
        {
            nMaxLen += 1;
            continue;
        }

        nItemLen = 0;

        /*  handle '%' character with format */
        nWidth = 0;
        for (; *lpsz; lpsz++)
        {
             /* check for valid flags */
            if (*lpsz == '#')
            {
                nMaxLen += 2;   /* for '0x' */
            }
            else if (*lpsz == '*')
            {
                nWidth = va_arg( argList, int );
            }
            else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0'
                     || *lpsz == ' ')
            {
                ;
            }
            else /* hit non-flag character */
            {
                break;
            }
        }
        /* get width and skip it */
        if (nWidth == 0)
        {
             /* width indicated by */
            nWidth = atoi( lpsz );
            for (; *lpsz && isdigit( *lpsz ); lpsz++)
                ;
        }
        /*ASSERT(nWidth >= 0);*/
        if (nWidth < 0)
        {
            goto exit_error; /* instead of exception */
        }

        nPrecision = 0;
        if (*lpsz == '.')
        {
             /* skip past '.' separator (width.precision)*/
            lpsz++;

            /* get precision and skip it*/
            if (*lpsz == '*')
            {
                nPrecision = va_arg( argList, int );
                lpsz++;
            }
            else
            {
                nPrecision = atoi( lpsz );
                for (; *lpsz && isdigit( *lpsz ); lpsz++)
                    ;
            }
            if (nPrecision < 0)
            {
                goto exit_error; /* instead of exception */
            }
        }

        /* should be on type modifier or specifier */
        nModifier = 0;
        switch (*lpsz)
        {
        /* modifiers that affect size */
            case 'h':
                switch (lpsz[1])
                {
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'x':
                    case 'X':
                    case 'u':
                        /* short unsigned, short double, etc. -- added to the original MS example */
                        /* ignore the fact that these modifiers do affect size */
                        lpsz++;
                        break;
                    default:
                        nModifier = FORCE_ANSI;
                        lpsz++;
                        break;
                }
                break;
            case 'l':
                switch (lpsz[1])
                {
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'x':
                    case 'X':
                    case 'u':
                    case 'f': /* long float -- post ANSI C */
                        /* long unsigned, long double, etc. -- added to the original MS example */
                        /* ignore the fact that these modifiers do affect size */
                        lpsz++;
                        break;
                    default:
                        /*
                        nModifier = FORCE_UNICODE;
                        lpsz ++;
                        break;
                        */
                        goto exit_error;  /* no UNICODE, please */
                }
                break;
           /* modifiers that do not affect size */
            case 'F':
            case 'N':
            case 'L':
                lpsz++;
                break;
        }

        /* now should be on specifier */
        switch (*lpsz | nModifier)
        {
        /* single characters*/
            case 'c':
            case 'C':
                nItemLen = 2;
                va_arg( argList, int );
                break;
            case 'c' | FORCE_ANSI:
            case 'C' | FORCE_ANSI:
                nItemLen = 2;
                va_arg( argList, int );
                break;
            case 'c' | FORCE_UNICODE:
            case 'C' | FORCE_UNICODE:
                goto exit_error;  /* no UNICODE, please */
                /*
                nItemLen = 2;
                va_arg(argList, wchar_t);
                break;
                */

           /* strings*/
            case 's':
            case 'S':
                nItemLen = (int) strlen( va_arg( argList, char* ) );
                nItemLen = my_max( 1, nItemLen );
                break;
            case 's' | FORCE_ANSI:
            case 'S' | FORCE_ANSI:
                nItemLen = (int) strlen( va_arg( argList, char* ) );
                nItemLen = my_max( 1, nItemLen );
                break;

            case 's' | FORCE_UNICODE:
            case 'S' | FORCE_UNICODE:
                goto exit_error;  /* no UNICODE, please */
                /*
                nItemLen = wcslen(va_arg(argList, wchar_t*));
                nItemLen = my_max(1, nItemLen);
                break;
                */
        }

        /* adjust nItemLen for strings */
        if (nItemLen != 0)
        {
            nItemLen = my_max( nItemLen, nWidth );
            if (nPrecision != 0)
                nItemLen = my_min( nItemLen, nPrecision );
        }
        else
        {
            switch (*lpsz)
            {
            /* integers */
                case 'd':
                case 'i':
                case 'u':
                case 'x':
                case 'X':
                case 'o':
                    va_arg( argList, int );
                    nItemLen = 32;
                    nItemLen = my_max( nItemLen, nWidth + nPrecision );
                    break;

                case 'e':
                case 'f':
                case 'g':
                case 'G':
                    va_arg( argList, double );
                    nItemLen = 32;
                    nItemLen = my_max( nItemLen, nWidth + nPrecision );
                    break;

                case 'p':
                    va_arg( argList, void* );
                    nItemLen = 32;
                    nItemLen = my_max( nItemLen, nWidth + nPrecision );
                    break;

               /* no output */
                case 'n':
                    va_arg( argList, int* );
                    break;

                default:
                    /*ASSERT(FALSE);*/  /* unknown formatting option*/
                    goto exit_error; /* instead of exception */
            }
        }

        /* adjust nMaxLen for output nItemLen */
        nMaxLen += nItemLen;
    }
    return nMaxLen;

exit_error:

    return -1; /* wrong format */
}


/****************************************************************************
 Read Molfile (SDFile segment) to dynamic string buffer
 Return 1 if OK, 0 on error; fills in *eof
****************************************************************************/
int get_next_molfile_as_growing_str_buffer( FILE *f, GrowStr *buf, int *at_eof )
{
    int res = 1, got_record_end = 0;
    int n_prev = buf->used;
    *at_eof = 0;

    /* possible return cases:
        BAD, got error              ==> return 0
        OK and got $$$$             ==> return 1 while keep *at_eof = 0
        OK and got EOF at the end   ==> return 1 and set    *at_eof = 1
    */

    while (( res = GrowStr_readline( buf, f ) ) == 1)    /* 1 if OK, 0 on error, -1 on EOF */
    {
        /* check for eorecord */
        if ( buf->used >= n_prev + 3     &&
             buf->ps[n_prev + 0] == '$'  && buf->ps[n_prev + 1] == '$'  &&
             buf->ps[n_prev + 2] == '$'  && buf->ps[n_prev + 3] == '$'   )
        {
            got_record_end = 1;
            break;
        }
        else
        {
            /* continue reading */
            n_prev = buf->used;
        }
    }

    if (res == 0)
    {
        return 0;
    }
    else if (res == -1)
    {
        *at_eof = 1;
    }
    else
    {
        *at_eof = 0;
    }

    return 1;
}

/****************************************************************************
 Get timer value since some fixed start in milliseconds;
 note that only difference of two values makes a sense.
****************************************************************************/
unsigned int get_msec_timer( void )
{
    unsigned t = 0;

#if ( defined(_WIN32) && defined(_MSC_VER) )
    t = GetTickCount( );
#endif
#ifndef _WIN32 
    /* NB: not reboot/date-change safe */
    struct timeval tv;
    gettimeofday( &tv, NULL );
    t = (unsigned) tv.tv_sec * 1000 + (unsigned) ( (double) tv.tv_usec / 1000.0 );
#endif

    return t;
}
