
#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* memmove */

#include "header.h"

#define HEAD 2*sizeof(int)
#define EXTENDER 40
#define DEBUG 0


/*  This modules provides a simple mechanism for arbitrary length writable
    strings, called 'blocks'. They are 'symbol *' items rather than 'char *'
    items however.

    The calls are:

        symbol * b = create_b(n);
            - create an empty block b with room for n symbols
        b = increase_capacity(b, n);
            - increase the capacity of block b by n symbols (b may change)
        b2 = copy_b(b)
            - copy block b into b2
        lose_b(b);
            - lose block b
        b = move_to_b(b, n, p);
            - set the data in b to be the n symbols at address p
        b = addto_to_b(b, n, p);
            - add the n symbols at address p to the end of the data in b
        SIZE(b)
            - is the number of symbols in b
        For example:

        symbol * b = create_b(0);
        {   int i;
            char p[10];
            for (i = 0; i < 100; i++) {
                sprintf(p, " %d", i);
                add_s_to_b(b, p);
            }
        }

    and b contains " 0 1 2 ... 99" spaced out as symbols.
*/

/*  For a block b, SIZE(b) is the number of symbols so far written into it,
    CAPACITY(b) the total number it can contain, so SIZE(b) <= CAPACITY(b).
    In fact blocks have 1 extra character over the promised capacity so
    they can be zero terminated by 'b[SIZE(b)] = 0;' without fear of
    overwriting.
*/

extern symbol * create_b(int n)
{   symbol * p = (symbol *) (HEAD + (char *) MALLOC(HEAD + (n + 1) * sizeof(symbol)));
    CAPACITY(p) = n;
    SIZE(p) = 0;
    if (DEBUG) printf("<-- %d\n", p);
    return p;
}

extern void report_b(FILE * out, symbol * p)
{   int i;
    for (i = 0; i < SIZE(p); i++) fprintf(out, "%c", p[i]);
}

extern void lose_b(symbol * p)
{   if (p == 0) return;
    if (DEBUG) printf("--> %d\n", p);
    FREE((char *) p - HEAD);
}

extern symbol * increase_capacity(symbol * p, int n)
{   symbol * q = create_b(CAPACITY(p) + n + EXTENDER);
    if (DEBUG) printf("%d --> %d\n", p, q);
    memmove(q, p, CAPACITY(p) * sizeof(symbol));
    SIZE(q) = SIZE(p);
    lose_b(p); return q;
}

extern symbol * move_to_b(symbol * p, int n, symbol * q)
{   int x = n - CAPACITY(p);
    if (x > 0) p = increase_capacity(p, x);
    memmove(p, q, n * sizeof(symbol)); SIZE(p) = n; return p;
}

extern symbol * add_to_b(symbol * p, int n, symbol * q)
{   int x = SIZE(p) + n - CAPACITY(p);
    if (x > 0) p = increase_capacity(p, x);
    memmove(p + SIZE(p), q, n * sizeof(symbol)); SIZE(p) += n; return p;
}

extern symbol * copy_b(symbol * p)
{   int n = SIZE(p);
    symbol * q = create_b(n);
    move_to_b(q, n, p);
    return q;
}

int space_count = 0;

extern void * check_malloc(int n)
{   space_count++;
    return malloc(n);
}

extern void check_free(void * p)
{   space_count--;
    free(p);
}

/* To convert a block to a zero terminated string:  */

extern char * b_to_s(symbol * p)
{   int n = SIZE(p);
    char * s = malloc(n + 1);
    {   int i;
        for (i = 0; i < n; i++) s[i] = p[i];
    }
    s[n] = 0;
    return s;
}

/* To add a zero terminated string to a block. If p = 0 the
   block is created. */

extern symbol * add_s_to_b(symbol * p, char * s)
{   int n = strlen(s);
    int k;
    if (p == 0) p = create_b(n);
    k = SIZE(p);
    {   int x = k + n - CAPACITY(p);
        if (x > 0) p = increase_capacity(p, x);
    }
    {   int i;
        for (i = 0; i < n; i++) p[i + k] = s[i];
    }
    SIZE(p) += n;
    return p;
}

/* The next section defines string handling capabilities in terms
   of the lower level block handling capabilities of space.c */
/* -------------------------------------------------------------*/

struct str {
    symbol * data;
};

/* Create a new string. */
extern struct str * str_new()
{
    struct str * output = (struct str *) malloc(sizeof(struct str));
    output->data = create_b(0);
    return output;
}

/* Delete a string. */
extern void str_delete(struct str * str)
{
    lose_b(str->data);
    free(str);
}

/* Append a str to this str. */
extern void str_append(struct str * str, struct str * add)
{
    symbol * q = add->data;
    str->data = add_to_b(str->data, SIZE(q), q);
}

/* Append a character to this str. */
extern void str_append_ch(struct str * str, char add)
{
    symbol q[1];
    q[0] = add;
    str->data = add_to_b(str->data, 1, q);
}

/* Append a low level block to a str. */
extern void str_append_b(struct str * str, symbol * q)
{
    str->data = add_to_b(str->data, SIZE(q), q);
}

/* Append a (char *, null teminated) string to a str. */
extern void str_append_string(struct str * str, char * s)
{
    str->data = add_s_to_b(str->data, s);
}

/* Append an integer to a str. */
extern void str_append_int(struct str * str, int i)
{
    char s[30];
    sprintf(s, "%d", i);
    str_append_string(str, s);
}

/* Clear a string */
extern void str_clear(struct str * str)
{
    SIZE(str->data) = 0;
}

/* Set a string */
extern void str_assign(struct str * str, char * s)
{
    str_clear(str);
    str_append_string(str, s);
}

/* Copy a string. */
extern struct str * str_copy(struct str * old)
{
    struct str * new = str_new();
    str_append(new, old);
    return new;
}

/* Get the data stored in this str. */
extern symbol * str_data(struct str * str)
{
    return str->data;
}

/* Get the length of the str. */
extern int str_len(struct str * str)
{
    return SIZE(str->data);
}

