#include <assert.h>
#include <stdlib.h> /* for exit */
#include <string.h> /* for strlen */
#include <stdio.h> /* for fprintf etc */
#include "header.h"

/* prototypes */

static void generate(struct generator * g, struct node * p);
static void w(struct generator * g, const char * s);
static void writef(struct generator * g, const char * s, struct node * p);

static int new_label(struct generator * g) {
    return g->next_label++;
}

static struct str * vars_newname(struct generator * g) {
    struct str * output;
    g->var_number++;
    output = str_new();
    str_append_string(output, "v_");
    str_append_int(output, g->var_number);
    return output;
}

/* Write routines for items from the syntax tree */

static void write_varname(struct generator * g, struct name * p) {
    int ch = "SBIrxg"[p->type];
    if (p->type != t_external) {
        write_char(g, ch);
        write_char(g, '_');
    }
    write_s(g, p->s);
}

static void write_varref(struct generator * g, struct name * p) {
    write_varname(g, p);
}

static void write_literal_string(struct generator * g, symbol * p) {
    write_string(g, "\"");
    for (int i = 0; i < SIZE(p); i++) {
        int ch = p[i];
        if (32 <= ch && ch < 127) {
            if (ch == '\"' || ch == '\\') write_string(g, "\\");
            write_char(g, ch);
        } else {
            write_string(g, "\\u");
            write_hex4(g, ch);
        }
    }
    write_string(g, "\"");
}

static void write_margin(struct generator * g) {
    for (int i = 0; i < g->margin; i++) write_string(g, "    ");
}

static void write_comment(struct generator * g, struct node * p) {
    if (!g->options->comments) return;
    write_margin(g);
    write_string(g, "// ");
    write_comment_content(g, p);
    write_newline(g);
}

static void write_block_start(struct generator * g) {
    w(g, "~M{~+~N");
}

static void write_block_end(struct generator * g) {
    w(g, "~-~M}~N");
}

static void write_savecursor(struct generator * g, struct node * p,
                             struct str * savevar) {
    g->B[0] = str_data(savevar);
    g->S[1] = "";
    if (p->mode != m_forward) g->S[1] = "base.limit - ";
    writef(g, "~M~C /** number */ ~B0 = ~S1base.cursor;~N", p);
}

static void append_restore_string(struct node * p, struct str * out, struct str * savevar) {
    str_append_string(out, "base.cursor = ");
    if (p->mode != m_forward) str_append_string(out, "base.limit - ");
    str_append(out, savevar);
    str_append_string(out, ";");
}

static void write_restorecursor(struct generator * g, struct node * p, struct str * savevar) {
    write_margin(g);
    append_restore_string(p, g->outbuf, savevar);
    write_newline(g);
}

static void write_inc_cursor(struct generator * g, struct node * p) {
    write_margin(g);
    write_string(g, p->mode == m_forward ? "base.cursor++;" : "base.cursor--;");
    write_newline(g);
}

static void wsetlab_begin(struct generator * g, int n) {
    g->I[0] = n;
    w(g, "~Mlab~I0: {~N~+");
}

static void wsetlab_end(struct generator * g) {
    w(g, "~-~M}~N");
}

static void wgotol(struct generator * g, int n) {
    write_margin(g);
    write_string(g, "break lab");
    write_int(g, n);
    write_string(g, ";");
    write_newline(g);
}

static void write_failure(struct generator * g) {
    if (str_len(g->failure_str) != 0) {
        write_margin(g);
        write_str(g, g->failure_str);
        write_newline(g);
    }
    write_margin(g);
    switch (g->failure_label) {
        case x_return:
            write_string(g, "return false;");
            g->unreachable = true;
            break;
        default:
            write_string(g, "break lab");
            write_int(g, g->failure_label);
            write_string(g, ";");
            g->unreachable = true;
    }
    write_newline(g);
}

static void write_failure_if(struct generator * g, const char * s, struct node * p) {
    writef(g, "~Mif (", p);
    writef(g, s, p);
    writef(g, ")~N", p);
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
}

/* if at limit fail */
static void write_check_limit(struct generator * g, struct node * p) {
    if (p->mode == m_forward) {
        write_failure_if(g, "base.cursor >= base.limit", p);
    } else {
        write_failure_if(g, "base.cursor <= base.limit_backward", p);
    }
}

/* Formatted write. */
static void writef(struct generator * g, const char * input, struct node * p) {
    (void)p;
    int i = 0;

    while (input[i]) {
        int ch = input[i++];
        if (ch != '~') {
            write_char(g, ch);
            continue;
        }
        ch = input[i++];
        switch (ch) {
            case '~': write_char(g, '~'); continue;
            case 'f': write_block_start(g);
                      write_failure(g);
                      g->unreachable = false;
                      write_block_end(g);
                      continue;
            case 'M': write_margin(g); continue;
            case 'N': write_newline(g); continue;
            case '{': write_block_start(g); continue;
            case '}': write_block_end(g); continue;
            case 'S': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->S) / sizeof(g->S[0]))) {
                    printf("Invalid escape sequence ~%c%c in writef(g, \"%s\", p)\n",
                           ch, input[i - 1], input);
                    exit(1);
                }
                write_string(g, g->S[j]);
                continue;
            }
            case 'B': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->B) / sizeof(g->B[0])))
                    goto invalid_escape2;
                write_s(g, g->B[j]);
                continue;
            }
            case 'I': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->I) / sizeof(g->I[0])))
                    goto invalid_escape2;
                write_int(g, g->I[j]);
                continue;
            }
            case 'V':
            case 'W': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->V) / sizeof(g->V[0])))
                    goto invalid_escape2;
                if (ch == 'V')
                    write_varref(g, g->V[j]);
                else
                    write_varname(g, g->V[j]);
                continue;
            }
            case 'L': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->L) / sizeof(g->L[0])))
                    goto invalid_escape2;
                write_literal_string(g, g->L[j]);
                continue;
            }
            case '+': g->margin++; continue;
            case '-': g->margin--; continue;
            case 'n': write_string(g, g->options->name); continue;
            case 'P': write_string(g, g->options->parent_class_name); continue;
            case 'C': { // Constant.
                if (g->options->js_esm) {
                    w(g, "const");
                } else {
                    w(g, "/** @const */ var");
                }
                continue;
            }
            case 'D': { // Declare variable.
                if (g->options->js_esm) {
                    w(g, "let");
                } else {
                    w(g, "var");
                }
                continue;
            }
            default:
                printf("Invalid escape sequence ~%c in writef(g, \"%s\", p)\n",
                       ch, input);
                exit(1);
            invalid_escape2:
                printf("Invalid escape sequence ~%c%c in writef(g, \"%s\", p)\n",
                       ch, input[i - 1], input);
                exit(1);
        }
    }
}

static void w(struct generator * g, const char * s) {
    writef(g, s, NULL);
}

static void generate_AE(struct generator * g, struct node * p) {
    const char * s;
    switch (p->type) {
        case c_name:
            write_varref(g, p->name); break;
        case c_number:
            write_int(g, p->number); break;
        case c_maxint:
            write_string(g, "(-1>>>1)"); break;
        case c_minint:
            write_string(g, "(~(-1>>>1))"); break;
        case c_neg:
            write_char(g, '-'); generate_AE(g, p->right); break;
        case c_multiply:
            s = " * "; goto label0;
        case c_plus:
            s = " + "; goto label0;
        case c_minus:
            s = " - ";
        label0:
            write_char(g, '('); generate_AE(g, p->left);
            write_string(g, s); generate_AE(g, p->right); write_char(g, ')'); break;
        case c_divide:
            /* Snowball specifies integer division with semantics matching C,
             * so we need to use `Math.trunc(x/y)` here.
             */
            write_string(g, "Math.trunc(");
            generate_AE(g, p->left);
            write_string(g, " / ");
            generate_AE(g, p->right);
            write_char(g, ')');
            break;
        case c_cursor:
            w(g, "base.cursor"); break;
        case c_limit:
            w(g, p->mode == m_forward ? "base.limit" : "base.limit_backward"); break;
        case c_lenof: /* Same as sizeof() for Javascript. */
        case c_sizeof:
            g->V[0] = p->name;
            w(g, "~V0.length");
            break;
        case c_len: /* Same as size() for Javascript. */
        case c_size:
            w(g, "base.current.length");
            break;
    }
}

static void generate_bra(struct generator * g, struct node * p) {
    write_comment(g, p);
    p = p->left;
    while (p) {
        generate(g, p);
        p = p->right;
    }
}

static void generate_and(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    write_comment(g, p);

    if (savevar) write_savecursor(g, p, savevar);

    p = p->left;
    while (p) {
        generate(g, p);
        if (g->unreachable) break;
        if (savevar && p->right != NULL) write_restorecursor(g, p, savevar);
        p = p->right;
    }

    if (savevar) {
        str_delete(savevar);
    }
}

static void generate_or(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    int out_lab = new_label(g);
    int end_unreachable = true;

    write_comment(g, p);
    wsetlab_begin(g, out_lab);

    if (savevar) write_savecursor(g, p, savevar);

    p = p->left;
    str_clear(g->failure_str);

    if (p == NULL) {
        /* p should never be NULL after an or: there should be at least two
         * sub nodes. */
        fprintf(stderr, "Error: \"or\" node without children nodes.");
        exit(1);
    }
    while (p->right != NULL) {
        g->failure_label = new_label(g);
        wsetlab_begin(g, g->failure_label);
        generate(g, p);
        if (!g->unreachable) {
            wgotol(g, out_lab);
            end_unreachable = false;
        }
        wsetlab_end(g);
        g->unreachable = false;
        if (savevar) write_restorecursor(g, p, savevar);
        p = p->right;
    }

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    generate(g, p);
    wsetlab_end(g);
    if (!end_unreachable) {
        g->unreachable = false;
    }

    if (savevar) {
        str_delete(savevar);
    }
}

static void generate_backwards(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~Mbase.limit_backward = base.cursor; base.cursor = base.limit;~N", p);
    generate(g, p->left);
    w(g, "~Mbase.cursor = base.limit_backward;~N");
}


static void generate_not(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    write_comment(g, p);
    if (savevar) {
        write_block_start(g);
        write_savecursor(g, p, savevar);
    }

    g->failure_label = new_label(g);
    str_clear(g->failure_str);

    wsetlab_begin(g, g->failure_label);

    generate(g, p->left);

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    if (!g->unreachable) write_failure(g);

    wsetlab_end(g);
    g->unreachable = false;

    if (savevar) {
        write_restorecursor(g, p, savevar);
        write_block_end(g);
        str_delete(savevar);
    }
}


static void generate_try(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    g->failure_label = new_label(g);
    str_clear(g->failure_str);

    write_comment(g, p);
    if (savevar) {
        write_savecursor(g, p, savevar);
        append_restore_string(p, g->failure_str, savevar);
    }

    wsetlab_begin(g, g->failure_label);
    generate(g, p->left);
    wsetlab_end(g);
    g->unreachable = false;

    if (savevar) {
        str_delete(savevar);
    }
}

static void generate_set(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = true;~N", p);
}

static void generate_unset(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = false;~N", p);
}

static void generate_fail(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate(g, p->left);
    if (!g->unreachable) write_failure(g);
}

/* generate_test() also implements 'reverse' */

static void generate_test(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    write_comment(g, p);

    if (savevar) {
        write_savecursor(g, p, savevar);
    }

    generate(g, p->left);

    if (savevar) {
        if (!g->unreachable) {
            write_restorecursor(g, p, savevar);
        }
        str_delete(savevar);
    }
}

static void generate_do(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    write_comment(g, p);
    if (savevar) write_savecursor(g, p, savevar);

    if (p->left->type == c_call) {
        /* Optimise do <call> */
        write_comment(g, p->left);
        g->V[0] = p->left->name;
        w(g, "~M~V0();~N");
    } else {
        g->failure_label = new_label(g);
        str_clear(g->failure_str);

        wsetlab_begin(g, g->failure_label);
        generate(g, p->left);
        wsetlab_end(g);
        g->unreachable = false;
    }

    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }
}

static void generate_next(struct generator * g, struct node * p) {
    write_comment(g, p);
    write_check_limit(g, p);
    write_inc_cursor(g, p);
}

static void generate_GO_grouping(struct generator * g, struct node * p, int is_goto, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->S[1] = complement ? "in" : "out";
    g->V[0] = p->name;
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    write_failure_if(g, "!base.go_~S1_grouping~S0(~V0, ~I0, ~I1)", p);
    if (!is_goto) {
        if (p->mode == m_forward)
            w(g, "~Mbase.cursor++;~N");
        else
            w(g, "~Mbase.cursor--;~N");
    }
}

static void generate_GO(struct generator * g, struct node * p, int style) {
    write_comment(g, p);

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    int end_unreachable = false;
    int golab = new_label(g);
    g->I[0] = golab;
    w(g, "~Mgolab~I0: while(true)~N");
    w(g, "~{");

    struct str * savevar = NULL;
    if (style == 1 || repeat_restore(g, p->left)) {
        savevar = vars_newname(g);
        write_savecursor(g, p, savevar);
    }

    g->failure_label = new_label(g);
    str_clear(g->failure_str);
    wsetlab_begin(g, g->failure_label);
    generate(g, p->left);

    if (g->unreachable) {
        /* Cannot break out of this loop: therefore the code after the
         * end of the loop is unreachable.*/
        end_unreachable = true;
    } else {
        /* include for goto; omit for gopast */
        if (style == 1) write_restorecursor(g, p, savevar);
        g->I[0] = golab;
        w(g, "~Mbreak golab~I0;~N");
    }
    g->unreachable = false;
    wsetlab_end(g);
    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    write_check_limit(g, p);
    write_inc_cursor(g, p);
    write_block_end(g);
    g->unreachable = end_unreachable;
}

static void generate_loop(struct generator * g, struct node * p) {
    struct str * loopvar = vars_newname(g);
    write_comment(g, p);
    g->B[0] = str_data(loopvar);
    w(g, "~Mfor (~D /** number */ ~B0 = ");
    generate_AE(g, p->AE);
    g->B[0] = str_data(loopvar);
    writef(g, "; ~B0 > 0; ~B0--)~N", p);
    writef(g, "~{", p);

    generate(g, p->left);

    w(g, "~}");
    str_delete(loopvar);
    g->unreachable = false;
}

static void generate_repeat_or_atleast(struct generator * g, struct node * p, struct str * loopvar) {
    writef(g, "~Mwhile(true)~N~{", p);

    struct str * savevar = NULL;
    if (repeat_restore(g, p->left)) {
        savevar = vars_newname(g);
        write_savecursor(g, p, savevar);
    }

    g->failure_label = new_label(g);
    str_clear(g->failure_str);
    wsetlab_begin(g, g->failure_label);
    generate(g, p->left);

    if (!g->unreachable) {
        if (loopvar != NULL) {
            g->B[0] = str_data(loopvar);
            w(g, "~M~B0--;~N");
        }

        w(g, "~Mcontinue;~N");
    }

    wsetlab_end(g);
    g->unreachable = false;

    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }

    w(g, "~Mbreak;~N~}");
}

static void generate_repeat(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate_repeat_or_atleast(g, p, NULL);
}

static void generate_atleast(struct generator * g, struct node * p) {
    struct str * loopvar = vars_newname(g);

    write_comment(g, p);
    w(g, "~{");
    g->B[0] = str_data(loopvar);
    w(g, "~M~D ~B0 = ");
    generate_AE(g, p->AE);
    w(g, ";~N");
    {
        int a0 = g->failure_label;
        struct str * a1 = str_copy(g->failure_str);

        generate_repeat_or_atleast(g, p, loopvar);

        g->failure_label = a0;
        str_delete(g->failure_str);
        g->failure_str = a1;
    }
    g->B[0] = str_data(loopvar);
    write_failure_if(g, "~B0 > 0", p);
    w(g, "~}");
    str_delete(loopvar);
}

static void generate_setmark(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = base.cursor;~N", p);
}

static void generate_tomark(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? ">" : "<";

    w(g, "~Mif (base.cursor ~S0 "); generate_AE(g, p->AE); w(g, ")~N");
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
    w(g, "~Mbase.cursor = "); generate_AE(g, p->AE); writef(g, ";~N", p);
}

static void generate_atmark(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~Mif (base.cursor != "); generate_AE(g, p->AE); writef(g, ")~N", p);
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
}

static void generate_hop(struct generator * g, struct node * p) {
    int c_count = ++g->keep_count;
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "+" : "-";

    g->I[0] = c_count;
    w(g, "~{~M~C /** number */ c~I0 = base.cursor ~S0 ");
    generate_AE(g, p->AE);
    w(g, ";~N");

    g->I[0] = c_count;
    g->S[1] = p->mode == m_forward ? "> base.limit" : "< base.limit_backward";
    g->S[2] = p->mode == m_forward ? "<" : ">";
    if (p->AE->type == c_number) {
        // Constant distance hop.
        //
        // No need to check for negative hop as that's converted to false by
        // the analyser.
        write_failure_if(g, "c~I0 ~S1", p);
    } else {
        write_failure_if(g, "c~I0 ~S1 || c~I0 ~S2 base.cursor", p);
    }
    writef(g, "~Mbase.cursor = c~I0;~N", p);
    writef(g, "~}", p);
}

static void generate_delete(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~Mif (!base.slice_del())~N"
              "~M{~N"
              "~+~Mreturn false;~N~-"
              "~M}~N", p);
}

static void generate_tolimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (p->mode == m_forward) {
        writef(g, "~Mbase.cursor = base.limit;~N", p);
    } else {
        writef(g, "~Mbase.cursor = base.limit_backward;~N", p);
    }
}

static void generate_atlimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (p->mode == m_forward) {
        write_failure_if(g, "base.cursor < base.limit", p);
    } else {
        write_failure_if(g, "base.cursor > base.limit_backward", p);
    }
}

static void generate_leftslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (p->mode == m_forward) {
        writef(g, "~Mbase.bra = base.cursor;~N", p);
    } else {
        writef(g, "~Mbase.ket = base.cursor;~N", p);
    }
}

static void generate_rightslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (p->mode == m_forward) {
        writef(g, "~Mbase.ket = base.cursor;~N", p);
    } else {
        writef(g, "~Mbase.bra = base.cursor;~N", p);
    }
}

static void generate_assignto(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = base.assign_to();~N", p);
}

static void generate_sliceto(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = base.slice_to();~N"
              "~Mif (~V0 == '')~N"
              "~M{~N"
              "~+~Mreturn false;~N~-"
              "~M}~N", p);
}

static void generate_address(struct generator * g, struct node * p) {
    symbol * b = p->literalstring;
    if (b != NULL) {
        write_literal_string(g, b);
    } else {
        write_varref(g, p->name);
    }
}

static void generate_insert(struct generator * g, struct node * p, int style) {
    int c_count;
    int keep_c = style == c_attach;
    write_comment(g, p);
    if (p->mode == m_backward) keep_c = !keep_c;
    if (keep_c) {
        c_count = ++g->keep_count;
        g->I[0] = c_count;
        w(g, "~{~M~C /** number */ c~I0 = base.cursor;~N");
    }
    writef(g, "~Mbase.insert(base.cursor, base.cursor, ", p);
    generate_address(g, p);
    writef(g, ");~N", p);
    if (keep_c) {
        g->I[0] = c_count;
        w(g, "~Mbase.cursor = c~I0;~N~}");
    }
}

static void generate_assignfrom(struct generator * g, struct node * p) {
    int c_count;
    int keep_c = p->mode == m_forward; /* like 'attach' */

    write_comment(g, p);
    if (keep_c) {
        c_count = ++g->keep_count;
        g->I[0] = c_count;
        w(g, "~{~M~C /** number */ c~I0 = base.cursor;~N");
    }
    if (p->mode == m_forward) {
        writef(g, "~Mbase.insert(base.cursor, base.limit, ", p);
    } else {
        writef(g, "~Mbase.insert(base.limit_backward, base.cursor, ", p);
    }
    generate_address(g, p);
    writef(g, ");~N", p);
    if (keep_c) {
        g->I[0] = c_count;
        w(g, "~Mbase.cursor = c~I0;~N~}");
    }
}

static void generate_slicefrom(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~Mif (!base.slice_from(");
    generate_address(g, p);
    writef(g, "))~N"
              "~M{~N"
              "~+~Mreturn false;~N~-"
              "~M}~N", p);
}

static void generate_setlimit(struct generator * g, struct node * p) {
    struct str * varname = vars_newname(g);
    write_comment(g, p);
    if (p->left && p->left->type == c_tomark) {
        /* Special case for:
         *
         *   setlimit tomark AE for C
         *
         * All uses of setlimit in the current stemmers we ship follow this
         * pattern, and by special-casing we can avoid having to save and
         * restore c.
         */
        struct node * q = p->left;
        write_comment(g, q);
        g->S[0] = q->mode == m_forward ? ">" : "<";
        w(g, "~Mif (base.cursor ~S0 "); generate_AE(g, q->AE); w(g, ")~N");
        write_block_start(g);
        write_failure(g);
        write_block_end(g);
        g->unreachable = false;

        g->B[0] = str_data(varname);
        w(g, "~M~C /** number */ ~B0 = ");
        if (p->mode == m_forward) {
            w(g, "base.limit - base.cursor;~N");
            w(g, "~Mbase.limit = ");
        } else {
            w(g, "base.limit_backward;~N");
            w(g, "~Mbase.limit_backward = ");
        }
        generate_AE(g, q->AE); writef(g, ";~N", q);

        if (p->mode == m_forward) {
            str_assign(g->failure_str, "base.limit += ");
            str_append(g->failure_str, varname);
            str_append_ch(g->failure_str, ';');
        } else {
            str_assign(g->failure_str, "base.limit_backward = ");
            str_append(g->failure_str, varname);
            str_append_ch(g->failure_str, ';');
        }
    } else {
        struct str * savevar = vars_newname(g);
        write_savecursor(g, p, savevar);

        generate(g, p->left);

        if (!g->unreachable) {
            g->B[0] = str_data(varname);
            w(g, "~M~C /** number */ ~B0 = ");
            if (p->mode == m_forward) {
                w(g, "base.limit - base.cursor;~N");
                w(g, "~Mbase.limit = base.cursor;~N");
            } else {
                w(g, "base.limit_backward;~N");
                w(g, "~Mbase.limit_backward = base.cursor;~N");
            }
            write_restorecursor(g, p, savevar);

            if (p->mode == m_forward) {
                str_assign(g->failure_str, "base.limit += ");
                str_append(g->failure_str, varname);
                str_append_ch(g->failure_str, ';');
            } else {
                str_assign(g->failure_str, "base.limit_backward = ");
                str_append(g->failure_str, varname);
                str_append_ch(g->failure_str, ';');
            }
        }
        str_delete(savevar);
    }

    if (!g->unreachable) {
        generate(g, p->aux);

        if (!g->unreachable) {
            write_margin(g);
            write_str(g, g->failure_str);
            write_newline(g);
        }
    }
    str_delete(varname);
}

/* dollar sets snowball up to operate on a string variable as if it were the
 * current string */
static void generate_dollar(struct generator * g, struct node * p) {
    write_comment(g, p);

    struct str * savevar = vars_newname(g);
    g->B[0] = str_data(savevar);
    writef(g, "~{~N"
              "~M~D /** !Object */ ~B0 = new ~P();~N", p);
    writef(g, "~M~B0.copy_from(base);~N", p);

    ++g->copy_from_count;
    str_assign(g->failure_str, "base.copy_from(");
    str_append(g->failure_str, savevar);
    str_append_string(g->failure_str, ");");
    g->V[0] = p->name;
    writef(g, "~Mbase.current = ~V0;~N"
              "~Mbase.cursor = 0;~N"
              "~Mbase.limit_backward = 0;~N"
              "~Mbase.limit = base.current.length;~N", p);
    generate(g, p->left);
    if (!g->unreachable) {
        g->V[0] = p->name;
        writef(g, "~M~V0 = base.current;~N", p);
        write_margin(g);
        write_str(g, g->failure_str);
        write_newline(g);
    }
    w(g, "~}");
    str_delete(savevar);
}

static void generate_integer_assign(struct generator * g, struct node * p, const char * s) {
    write_comment(g, p);
    g->V[0] = p->name;
    g->S[0] = s;
    w(g, "~M~V0 ~S0 ");
    generate_AE(g, p->AE);
    w(g, ";~N");
}

static void generate_integer_test(struct generator * g, struct node * p) {
    write_comment(g, p);
    int relop = p->type;
    int optimise_to_return = (g->failure_label == x_return && p->right && p->right->type == c_functionend);
    if (optimise_to_return) {
        w(g, "~Mreturn ");
        p->right = NULL;
    } else {
        w(g, "~Mif (");
        // We want the inverse of the snowball test here.
        relop ^= 1;
    }
    generate_AE(g, p->left);
    // Relational operators are the same as C.
    write_c_relop(g, relop);
    generate_AE(g, p->AE);
    if (optimise_to_return) {
        w(g, ";~N");
    } else {
        w(g, ")~N");
        write_block_start(g);
        write_failure(g);
        write_block_end(g);
        g->unreachable = false;
    }
}

static void generate_call(struct generator * g, struct node * p) {
    int signals = check_possible_signals_list(g, p->name->definition, c_define, 0);
    write_comment(g, p);
    g->V[0] = p->name;
    if (g->failure_label == x_return &&
        (signals == 0 || (p->right && p->right->type == c_functionend))) {
        /* Always fails or tail call. */
        writef(g, "~Mreturn ~V0();~N", p);
        if (p->right && p->right->type == c_functionend) {
            p->right = NULL;
        }
        g->unreachable = true;
        return;
    }
    if (signals == 1) {
        /* Always succeeds. */
        writef(g, "~M~V0();~N", p);
    } else if (signals == 0) {
        /* Always fails. */
        writef(g, "~M~V0();~N", p);
        write_failure(g);
    } else {
        write_failure_if(g, "!~V0()", p);
    }
}

static void generate_grouping(struct generator * g, struct node * p, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->S[1] = complement ? "out" : "in";
    g->V[0] = p->name;
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    write_failure_if(g, "!(base.~S1_grouping~S0(~V0, ~I0, ~I1))", p);
}

static void generate_namedstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->V[0] = p->name;
    write_failure_if(g, "!(base.eq_s~S0(~V0))", p);
}

static void generate_literalstring(struct generator * g, struct node * p) {
    symbol * b = p->literalstring;
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->L[0] = b;
    write_failure_if(g, "!(base.eq_s~S0(~L0))", p);
}

static void generate_define(struct generator * g, struct node * p) {
    struct name * q = p->name;
    if (q->type == t_routine && !q->used) return;

    write_newline(g);
    write_comment(g, p);

    g->V[0] = q;
    if (q->type == t_routine) {
        w(g, "~M/** @return {boolean} */~N"
             "~Mfunction ~W0() {~+~N");
    } else {
        w(g, "~Mthis.~W0 = /** @return {boolean} */ function() {~+~N");
    }

    /* Save output. */
    struct str * saved_output = g->outbuf;
    struct str * saved_declarations = g->declarations;
    g->outbuf = str_new();
    g->declarations = str_new();

    g->next_label = 0;
    g->var_number = 0;

    if (p->amongvar_needed) {
        w(g, "~M~D /** number */ among_var;~N");
    }
    str_clear(g->failure_str);
    g->failure_label = x_return;
    g->unreachable = false;
    g->keep_count = 0;
    int signals = check_possible_signals_list(g, p->left, c_define, 0);

    /* Generate function body. */
    generate(g, p->left);
    if (p->left->right) {
        assert(p->left->right->type == c_functionend);
        if (signals) {
            generate(g, p->left->right);
        }
    }
    w(g, "~-~M};~N");

    str_append(saved_output, g->declarations);
    str_append(saved_output, g->outbuf);
    str_delete(g->declarations);
    str_delete(g->outbuf);
    g->declarations = saved_declarations;
    g->outbuf = saved_output;
}

static void generate_functionend(struct generator * g, struct node * p) {
    (void)p;
    w(g, "~Mreturn true;~N");
}

static void generate_substring(struct generator * g, struct node * p) {
    write_comment(g, p);

    struct among * x = p->among;

    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->I[0] = x->number;

    if (x->amongvar_needed) {
        writef(g, "~Mamong_var = base.find_among~S0(a_~I0);~N", p);
        if (!x->always_matches) {
            write_failure_if(g, "among_var == 0", p);
        }
    } else if (x->always_matches) {
        writef(g, "~Mbase.find_among~S0(a_~I0);~N", p);
    } else if (x->command_count == 0 &&
               x->node->right && x->node->right->type == c_functionend) {
        writef(g, "~Mreturn base.find_among~S0(a_~I0) != 0;~N", p);
        x->node->right = NULL;
        g->unreachable = true;
    } else {
        write_failure_if(g, "base.find_among~S0(a_~I0) == 0", p);
    }
}

static void generate_among(struct generator * g, struct node * p) {
    struct among * x = p->among;

    if (x->substring == NULL) {
        generate_substring(g, p);
    } else {
        write_comment(g, p);
    }

    if (x->command_count == 1 && x->nocommand_count == 0) {
        /* Only one outcome ("no match" already handled). */
        generate(g, x->commands[0]);
    } else if (x->command_count > 0) {
        w(g, "~Mswitch (among_var) {~N~+");
        for (int i = 1; i <= x->command_count; i++) {
            g->I[0] = i;
            w(g, "~Mcase ~I0:~N~+");
            generate(g, x->commands[i - 1]);
            if (!g->unreachable) w(g, "~Mbreak;~N");
            w(g, "~-");
            g->unreachable = false;
        }
        write_block_end(g);
    }
}

static void generate_booltest(struct generator * g, struct node * p, int inverted) {
    write_comment(g, p);
    g->V[0] = p->name;
    if (g->failure_label == x_return) {
        if (p->right && p->right->type == c_functionend) {
            // Optimise at end of function.
            if (inverted) {
                writef(g, "~Mreturn !~V0;~N", p);
            } else {
                writef(g, "~Mreturn ~V0;~N", p);
            }
            p->right = NULL;
            g->unreachable = true;
            return;
        }
    }
    if (inverted) {
        write_failure_if(g, "~V0", p);
    } else {
        write_failure_if(g, "!~V0", p);
    }
}

static void generate_false(struct generator * g, struct node * p) {
    write_comment(g, p);
    write_failure(g);
}

static void generate_debug(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->I[0] = g->debug_count++;
    g->I[1] = p->line_number;
    writef(g, "~Mbase.debug(~I0, ~I1);~N", p);
}

static void generate(struct generator * g, struct node * p) {
    if (g->unreachable) return;

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    switch (p->type) {
        case c_define:        generate_define(g, p); break;
        case c_bra:           generate_bra(g, p); break;
        case c_and:           generate_and(g, p); break;
        case c_or:            generate_or(g, p); break;
        case c_backwards:     generate_backwards(g, p); break;
        case c_not:           generate_not(g, p); break;
        case c_set:           generate_set(g, p); break;
        case c_unset:         generate_unset(g, p); break;
        case c_try:           generate_try(g, p); break;
        case c_fail:          generate_fail(g, p); break;
        case c_reverse:
        case c_test:          generate_test(g, p); break;
        case c_do:            generate_do(g, p); break;
        case c_goto:          generate_GO(g, p, 1); break;
        case c_gopast:        generate_GO(g, p, 0); break;
        case c_goto_grouping: generate_GO_grouping(g, p, 1, 0); break;
        case c_gopast_grouping:
                              generate_GO_grouping(g, p, 0, 0); break;
        case c_goto_non:      generate_GO_grouping(g, p, 1, 1); break;
        case c_gopast_non:    generate_GO_grouping(g, p, 0, 1); break;
        case c_repeat:        generate_repeat(g, p); break;
        case c_loop:          generate_loop(g, p); break;
        case c_atleast:       generate_atleast(g, p); break;
        case c_setmark:       generate_setmark(g, p); break;
        case c_tomark:        generate_tomark(g, p); break;
        case c_atmark:        generate_atmark(g, p); break;
        case c_hop:           generate_hop(g, p); break;
        case c_delete:        generate_delete(g, p); break;
        case c_next:          generate_next(g, p); break;
        case c_tolimit:       generate_tolimit(g, p); break;
        case c_atlimit:       generate_atlimit(g, p); break;
        case c_leftslice:     generate_leftslice(g, p); break;
        case c_rightslice:    generate_rightslice(g, p); break;
        case c_assignto:      generate_assignto(g, p); break;
        case c_sliceto:       generate_sliceto(g, p); break;
        case c_assign:        generate_assignfrom(g, p); break;
        case c_insert:
        case c_attach:        generate_insert(g, p, p->type); break;
        case c_slicefrom:     generate_slicefrom(g, p); break;
        case c_setlimit:      generate_setlimit(g, p); break;
        case c_dollar:        generate_dollar(g, p); break;
        case c_mathassign:    generate_integer_assign(g, p, "="); break;
        case c_plusassign:    generate_integer_assign(g, p, "+="); break;
        case c_minusassign:   generate_integer_assign(g, p, "-="); break;
        case c_multiplyassign:generate_integer_assign(g, p, "*="); break;
        case c_divideassign:
            /* Snowball specifies integer division with semantics matching C,
             * so we need to use `Math.trunc(x/y)` here.
             */
            g->V[0] = p->name;
            w(g, "~M~V0 = Math.trunc(~V0 / ");
            generate_AE(g, p->AE);
            w(g, ");~N");
            break;
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
            generate_integer_test(g, p);
            break;
        case c_call:          generate_call(g, p); break;
        case c_grouping:      generate_grouping(g, p, false); break;
        case c_non:           generate_grouping(g, p, true); break;
        case c_name:          generate_namedstring(g, p); break;
        case c_literalstring: generate_literalstring(g, p); break;
        case c_among:         generate_among(g, p); break;
        case c_substring:     generate_substring(g, p); break;
        case c_booltest:      generate_booltest(g, p, false); break;
        case c_not_booltest:  generate_booltest(g, p, true); break;
        case c_false:         generate_false(g, p); break;
        case c_true:          break;
        case c_debug:         generate_debug(g, p); break;
        case c_functionend:   generate_functionend(g, p); break;
        default: fprintf(stderr, "%d encountered\n", p->type);
                 exit(1);
    }

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;
}

static void generate_class_begin(struct generator * g) {
    if (g->options->js_esm) {
        w(g, "// deno-lint-ignore-file~N"
             "import ~P from './base-stemmer.mjs'~N"
             "~N"
             "/** @typedef {{ stemWord(word: string): string }} Stemmer */~N"
             "~N"
             "/** @type {{ new(): Stemmer }} */~N"
             "~C ~n = function() {~+~N"
             "~M~D base = new ~P();~N");
    } else {
        w(g, "/**@constructor*/~N"
             "~D ~n = function() {~+~N"
             "~M~C ~P = require('./base-stemmer.js');~N"
             "~M~D base = new ~P();~N");
    }
    write_newline(g);
}

static void generate_class_end(struct generator * g) {
    w(g, "~N");
    w(g, "~M/**@return{string}*/~N");
    w(g, "~Mthis['stemWord'] = function(/**string*/word) {~+~N");
    w(g, "~Mbase.setCurrent(word);~N");
    w(g, "~Mthis.stem();~N");
    w(g, "~Mreturn base.getCurrent();~N");
    w(g, "~-~M};~N");
    w(g, "~-};~N");
    if (g->options->js_esm) {
        w(g, "~N"
             "export default ~n~N");
    } else {
        w(g, "~N"
             "if (typeof module === 'object' && module.exports) module.exports = ~n;~N");
    }
}

static void generate_among_table(struct generator * g, struct among * x) {
    write_comment(g, x->node);

    struct amongvec * v = x->b;

    g->I[0] = x->number;
    w(g, "~M~C a_~I0 = [~N~+");

    for (int i = 0; i < x->literalstring_count; i++) {
        g->I[0] = v[i].i;
        g->I[1] = v[i].result;
        g->L[0] = v[i].b;
        g->S[0] = i < x->literalstring_count - 1 ? "," : "";

        w(g, "~M[~L0, ~I0, ~I1");
        if (v[i].function != NULL) {
            w(g, ", ");
            write_varname(g, v[i].function);
        }
        w(g, "]~S0~N");
    }
    w(g, "~-~M];~N~N");
}

static void generate_amongs(struct generator * g) {
    for (struct among * x = g->analyser->amongs; x; x = x->next) {
        generate_among_table(g, x);
    }
}

static void set_bit(symbol * b, int i) { b[i/8] |= 1 << i%8; }

static void generate_grouping_table(struct generator * g, struct grouping * q) {
    int range = q->largest_ch - q->smallest_ch + 1;
    int size = (range + 7)/ 8;  /* assume 8 bits per symbol */
    symbol * b = q->b;
    symbol * map = create_b(size);

    for (int i = 0; i < size; i++) map[i] = 0;

    for (int i = 0; i < SIZE(b); i++) set_bit(map, b[i] - q->smallest_ch);

    g->V[0] = q->name;
    w(g, "~M~C /** Array<int> */ ~W0 = [");
    for (int i = 0; i < size; i++) {
        write_int(g, map[i]);
        if (i < size - 1) w(g, ", ");
    }
    w(g, "];~N~N");

    lose_b(map);
}

static void generate_groupings(struct generator * g) {
    for (struct grouping * q = g->analyser->groupings; q; q = q->next) {
        if (q->name->used)
            generate_grouping_table(g, q);
    }
}

static void generate_members(struct generator * g) {
    int wrote_members = false;

    for (struct name * q = g->analyser->names; q; q = q->next) {
        g->V[0] = q;
        switch (q->type) {
            case t_string:
                w(g, "~M~D /** string */ ~W0 = '';~N");
                wrote_members = true;
                break;
            case t_integer:
                w(g, "~M~D /** number */ ~W0 = 0;~N");
                wrote_members = true;
                break;
            case t_boolean:
                w(g, "~M~D /** boolean */ ~W0 = false;~N");
                wrote_members = true;
                break;
        }
    }
    if (wrote_members) w(g, "~N");
}

static void generate_methods(struct generator * g) {
    for (struct node * p = g->analyser->program; p; p = p->right) {
        generate(g, p);
        g->unreachable = false;
    }
}

extern void generate_program_js(struct generator * g) {
    g->outbuf = str_new();
    g->failure_str = str_new();

    write_start_comment(g, "// ", NULL);

    generate_class_begin(g);

    generate_amongs(g);
    generate_groupings(g);

    generate_members(g);
    generate_methods(g);

    generate_class_end(g);

    output_str(g->options->output_src, g->outbuf);
    str_delete(g->failure_str);
    str_delete(g->outbuf);
}
