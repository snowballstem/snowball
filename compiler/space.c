
#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* memmove */

#include "header.h"

#define HEAD 2*sizeof(int)
#define EXTENDER 40


/*  This modules provides a simple mechanism for arbitrary length writable
    strings, called 'blocks'. They are 'symbol *' items rather than 'char *'
    items however.

    The calls are:

        symbol * b = create_b(n);
            - create an empty block b with room for n symbols
        b = increase_capacity_b(b, n);
            - increase the capacity of block b by n symbols (b may change)
        b2 = copy_b(b)
            - copy block b into b2
        lose_b(b);
            - lose block b
        b = add_to_b(b, p, n);
            - add the n symbols at address p to the end of the data in b
        SIZE(b)
            - is the number of symbols in b

    For example:

        symbol * b = create_b(0);
        {   symbol i;
            for (i = 'A'; i <= 'Z'; i++) {
                add_symbol_to_b(b, i);
            }
        }

    After running the above code b contains:

        { (symbol)'A', (symbol)'B', ..., (symbol)'Z' }
*/

/*  For a block b, SIZE(b) is the number of symbols so far written into it,
    CAPACITY(b) the total number it can contain, so SIZE(b) <= CAPACITY(b).
    In fact blocks have 1 extra character over the promised capacity so
    they can be zero terminated by 'b[SIZE(b)] = 0;' without fear of
    overwriting.
*/

extern symbol * create_b(int n) {
    symbol * p = (symbol *) (HEAD + (char *) MALLOC(HEAD + (n + 1) * sizeof(symbol)));
    CAPACITY(p) = n;
    SIZE(p) = 0;
    return p;
}

extern void report_b(FILE * out, const symbol * p) {
    int i;
    for (i = 0; i < SIZE(p); i++) {
        if (p[i] > 255) {
            printf("In report_b, can't convert p[%d] to char because it's 0x%02x\n", i, (int)p[i]);
            exit(1);
        }
        putc(p[i], out);
    }
}

extern void output_str(FILE * outfile, struct str * str) {
    report_s(outfile, str_data(str));
}

extern void lose_b(symbol * p) {
    if (p == NULL) return;
    FREE((char *) p - HEAD);
}

extern symbol * increase_capacity_b(symbol * p, int n) {
    symbol * q = create_b(CAPACITY(p) + n + EXTENDER);
    memmove(q, p, CAPACITY(p) * sizeof(symbol));
    SIZE(q) = SIZE(p);
    lose_b(p); return q;
}

extern symbol * add_to_b(symbol * p, const symbol * q, int n) {
    int x = SIZE(p) + n - CAPACITY(p);
    if (x > 0) p = increase_capacity_b(p, x);
    memmove(p + SIZE(p), q, n * sizeof(symbol)); SIZE(p) += n; return p;
}

extern symbol * copy_b(const symbol * p) {
    int n = SIZE(p);
    symbol * q = create_b(n);
    add_to_b(q, p, n);
    return q;
}

int space_count = 0;

static void * xmalloc(size_t n) {
    void * result = malloc(n);
    if (result == NULL) {
        fprintf(stderr, "Failed to allocate %lu bytes\n", (unsigned long)n);
        exit(1);
    }
    return result;
}

extern void * check_malloc(size_t n) {
    space_count++;
    return xmalloc(n);
}

extern void check_free(void * p) {
    space_count--;
    free(p);
}

/* To convert a block to a zero terminated string:  */

extern char * b_to_sz(const symbol * p) {
    int n = SIZE(p);
    char * s = (char *)xmalloc(n + 1);
    {
        int i;
        for (i = 0; i < n; i++) {
            if (p[i] > 255) {
                printf("In b_to_s, can't convert p[%d] to char because it's 0x%02x\n", i, (int)p[i]);
                exit(1);
            }
            s[i] = (char)p[i];
        }
    }
    s[n] = 0;
    return s;
}

/* Add a single symbol to a block. If p = 0 the
   block is created. */

extern symbol * add_symbol_to_b(symbol * p, symbol ch) {
    int k;
    if (p == NULL) p = create_b(1);
    k = SIZE(p);
    {
        int x = k + 1 - CAPACITY(p);
        if (x > 0) p = increase_capacity_b(p, x);
    }
    p[k] = ch;
    SIZE(p)++;
    return p;
}

extern byte * create_s(int n) {
    byte * p = (byte *) (HEAD + (byte *) MALLOC(HEAD + (n + 1)));
    CAPACITY(p) = n;
    SIZE(p) = 0;
    return p;
}

extern void report_s(FILE * out, const byte * p) {
    fwrite(p, 1, SIZE(p), out);
}

extern void lose_s(byte * p) {
    if (p == NULL) return;
    FREE((byte *) p - HEAD);
}

extern byte * increase_capacity_s(byte * p, int n) {
    byte * q = create_s(CAPACITY(p) + n + EXTENDER);
    memmove(q, p, CAPACITY(p));
    SIZE(q) = SIZE(p);
    lose_s(p);
    return q;
}

extern byte * copy_s(const byte * p) {
    return add_s_to_s(NULL, (const char*)p, SIZE(p));
}

/* Add a string with given length to a byte block. If p = 0 the
   block is created. */

extern byte * add_s_to_s(byte * p, const char * s, int n) {
    int k;
    if (p == NULL) p = create_s(n);
    k = SIZE(p);
    {
        int x = k + n - CAPACITY(p);
        if (x > 0) p = increase_capacity_s(p, x);
    }
    memcpy(p + k, s, n);
    SIZE(p) += n;
    return p;
}

/* Add a zero terminated string to a byte block. If p = 0 the
   block is created. */

extern byte * add_sz_to_s(byte * p, const char * s) {
    return add_s_to_s(p, s, strlen(s));
}

/* Add a single character to a byte block. If p = 0 the
   block is created. */

extern byte * add_char_to_s(byte * p, char ch) {
    int k;
    if (p == NULL) p = create_s(1);
    k = SIZE(p);
    {
        int x = k + 1 - CAPACITY(p);
        if (x > 0) p = increase_capacity_s(p, x);
    }
    p[k] = ch;
    SIZE(p)++;
    return p;
}

/* The next section defines string handling capabilities in terms
   of the lower level byte block handling capabilities of space.c */
/* -------------------------------------------------------------*/

struct str {
    byte * data;
};

/* Create a new string. */
extern struct str * str_new(void) {
    struct str * output = (struct str *) xmalloc(sizeof(struct str));
    output->data = create_s(0);
    return output;
}

/* Delete a string. */
extern void str_delete(struct str * str) {
    lose_s(str->data);
    free(str);
}

/* Append a str to this str. */
extern void str_append(struct str * str, const struct str * add) {
    byte * q = add->data;
    str->data = add_s_to_s(str->data, (char *)q, SIZE(q));
}

/* Append a character to this str. */
extern void str_append_ch(struct str * str, char add) {
    str->data = add_char_to_s(str->data, add);
}

/* Append a low level byte block to a str. */
extern void str_append_s(struct str * str, const byte * q) {
    str->data = add_s_to_s(str->data, (const char *)q, SIZE(q));
}

/* Append a (char *, null terminated) string to a str. */
extern void str_append_string(struct str * str, const char * s) {
    str->data = add_sz_to_s(str->data, s);
}

/* Append an integer to a str. */
extern void str_append_int(struct str * str, int i) {
    char s[30];
    sprintf(s, "%d", i);
    str_append_string(str, s);
}

/* Clear a string */
extern void str_clear(struct str * str) {
    SIZE(str->data) = 0;
}

/* Set a string */
extern void str_assign(struct str * str, const char * s) {
    str_clear(str);
    str_append_string(str, s);
}

/* Copy a string. */
extern struct str * str_copy(const struct str * old) {
    struct str * newstr = str_new();
    str_append(newstr, old);
    return newstr;
}

/* Get the data stored in this str. */
extern byte * str_data(const struct str * str) {
    return str->data;
}

/* Get the length of the str. */
extern int str_len(const struct str * str) {
    return SIZE(str->data);
}

/* Get the last character of the str.
 *
 * Or -1 if the string is empty.
 */
extern int str_back(const struct str *str) {
    return SIZE(str->data) ? str->data[SIZE(str->data) - 1] : -1;
}

/* Remove the last character of the str.
 *
 * Or do nothing if the string is empty.
 */
extern void str_pop(const struct str *str) {
    if (SIZE(str->data)) --SIZE(str->data);
}

extern int get_utf8(const symbol * p, int * slot) {
    int b0, b1;
    b0 = *p++;
    if (b0 < 0xC0) {   /* 1100 0000 */
        * slot = b0; return 1;
    }
    b1 = *p++;
    if (b0 < 0xE0) {   /* 1110 0000 */
        * slot = (b0 & 0x1F) << 6 | (b1 & 0x3F); return 2;
    }
    * slot = (b0 & 0xF) << 12 | (b1 & 0x3F) << 6 | (*p & 0x3F); return 3;
}

extern int put_utf8(int ch, symbol * p) {
    if (ch < 0x80) {
        p[0] = ch; return 1;
    }
    if (ch < 0x800) {
        p[0] = (ch >> 6) | 0xC0;
        p[1] = (ch & 0x3F) | 0x80; return 2;
    }
    p[0] = (ch >> 12) | 0xE0;
    p[1] = ((ch >> 6) & 0x3F) | 0x80;
    p[2] = (ch & 0x3F) | 0x80; return 3;
}
