#include <assert.h>
#include <stdio.h>   /* for fprintf etc */
#include <stdlib.h>  /* for free etc */
#include <string.h>  /* for strlen */
#include "header.h"

/* Generator functions common to multiple target languages. */

extern struct generator * create_generator(struct analyser * a, struct options * o) {
    NEW(generator, g);
    *g = (struct generator){0};
    g->analyser = a;
    g->options = o;
    g->failure_label = -1;
    g->varname_prefix = "v_";
    g->margin_indent = "    ";
    return g;
}

extern void close_generator(struct generator * g) {
    FREE(g);
}

extern struct str * vars_newname(struct generator * g) {
    struct str * output;
    g->var_number++;
    output = str_new();
    str_append_string(output, g->varname_prefix);
    str_append_int(output, g->var_number);
    return output;
}

extern void write_margin(struct generator * g) {
    for (int i = 0; i < g->margin; i++) write_string(g, g->margin_indent);
}

/* Language-independent write routines for simple entities */

static void write_hexdigit(struct generator * g, int i) {
    str_append_ch(g->outbuf, "0123456789ABCDEF"[i & 0xF]); /* hexchar */
}

extern void write_hex4(struct generator * g, int ch) {
    for (int i = 12; i >= 0; i -= 4) write_hexdigit(g, ch >> i);
}

extern void write_hex(struct generator * g, int i) {
    if (i >> 4) write_hex(g, i >> 4);
    write_hexdigit(g, i); /* hex integer */
}

extern void write_char(struct generator * g, int ch) {
    str_append_ch(g->outbuf, ch); /* character */
}

extern void write_newline(struct generator * g) {
    /* Avoid generating trailing whitespace. */
    while (true) {
        int ch = str_back(g->outbuf);
        if (ch != ' ' && ch != '\t') break;
        str_pop(g->outbuf);
    }
    str_append_ch(g->outbuf, '\n'); /* newline */
    g->line_count++;
}

extern void write_string(struct generator * g, const char * s) {
    str_append_string(g->outbuf, s);
}

extern void write_wchar_as_utf8(struct generator * g, symbol ch) {
    str_append_wchar_as_utf8(g->outbuf, ch);
}

extern void write_int(struct generator * g, int i) {
    str_append_int(g->outbuf, i);
}

// Write an integer, left-padded to width 3 with spaces.
extern void wi3(struct generator * g, int i) {
    if (i < 100) write_char(g, ' ');
    if (i < 10)  write_char(g, ' ');
    write_int(g, i);
}

extern void write_s(struct generator * g, const byte * s) {
    str_append_s(g->outbuf, s);
}

extern void write_str(struct generator * g, struct str * str) {
    str_append(g->outbuf, str);
}
