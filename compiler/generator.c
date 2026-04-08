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

static void write_comment_literalstring(struct generator * g, const symbol *s,
                                        const char * end) {
    if (end) {
        // Check if the literal string contains the target language end comment
        // string.  Don't try to be clever here as real-world literal strings
        // are unlikely to contain even partial matches.
        int end_len = strlen(end);
        if (end_len <= SIZE(s)) {
            for (int i = 0; i <= SIZE(s) - end_len; ++i) {
                for (int j = 0; j < end_len; ++j) {
                    if (s[i + j] != end[j]) goto next_outer;
                }
                write_string(g, "<literal string>");
                return;
next_outer: ;
            }
        }
    }
    write_char(g, '\'');
    for (int i = 0; i < SIZE(s); ++i) {
        symbol c = s[i];
        if (c == '\'' || c == '{') {
            write_char(g, '{');
            write_char(g, c);
            write_char(g, '}');
        } else if (c < 32 || c == 127) {
            write_string(g, "{U+");
            write_hex(g, c);
            write_char(g, '}');
        } else {
            if (g->options->encoding == ENC_WIDECHARS) {
                write_wchar_as_utf8(g, s[i]);
            } else {
                write_char(g, s[i]);
            }
        }
    }
    write_char(g, '\'');
}

static void write_comment_AE(struct generator * g, struct node * p) {
    switch (p->type) {
        case c_name:
            write_s(g, p->name->s);
            break;
        case c_number:
            write_int(g, p->number);
            break;
        case c_cursor:
        case c_len:
        case c_lenof:
        case c_limit:
        case c_maxint:
        case c_minint:
        case c_size:
        case c_sizeof:
            write_string(g, name_of_token(p->type));
            if (p->name) {
                write_char(g, ' ');
                write_s(g, p->name->s);
            }
            break;
        case c_neg:
            write_char(g, '-');
            write_comment_AE(g, p->right);
            break;
        case c_multiply:
        case c_plus:
        case c_minus:
        case c_divide:
            write_char(g, '(');
            write_comment_AE(g, p->left);
            write_char(g, ' ');
            write_string(g, name_of_token(p->type));
            write_char(g, ' ');
            write_comment_AE(g, p->right);
            write_char(g, ')');
            break;
        default:
            fprintf(stderr, "Unexpected type #%d in write_comment_AE\n", p->type);
            exit(1);
    }
}

void write_comment_content(struct generator * g, struct node * p,
                           const char * end) {
    switch (p->type) {
        case c_mathassign:
        case c_plusassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_divideassign:
            if (p->name) {
                write_char(g, '$');
                write_s(g, p->name->s);
                write_char(g, ' ');
            }
            write_string(g, name_of_token(p->type));
            write_char(g, ' ');
            write_comment_AE(g, p->AE);
            break;
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
            write_string(g, "$(");
            write_comment_AE(g, p->left);
            write_char(g, ' ');
            write_string(g, name_of_token(p->type));
            write_char(g, ' ');
            write_comment_AE(g, p->AE);
            write_char(g, ')');
            break;
        case c_define:
            if (p->mode == m_forward) {
                write_string(g, "forwardmode define ");
            } else {
                write_string(g, "backwardmode define ");
            }
            write_s(g, p->name->s);
            break;
        case c_literalstring:
            write_comment_literalstring(g, p->literalstring, end);
            break;
        case c_call:
        case c_grouping:
        case c_name:
            write_s(g, p->name->s);
            break;
        default:
            write_string(g, name_of_token(p->type));
            if (p->name) {
                write_char(g, ' ');
                write_s(g, p->name->s);
            } else if (p->literalstring) {
                write_char(g, ' ');
                write_comment_literalstring(g, p->literalstring, end);
            }
    }
    write_string(g, ", line ");
    write_int(g, p->line_number);
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
