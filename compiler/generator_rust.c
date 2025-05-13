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
    switch (p->type) {
        case t_external:
            break;
        default: {
            int ch = "SbirxG"[p->type];
            write_char(g, ch);
            write_char(g, '_');
            break;
        }
    }
    write_s(g, p->s);
}

static void write_varref(struct generator * g, struct name * p) {
    write_string(g, "context.");
    write_varname(g, p);
}

static void write_literal_string(struct generator * g, symbol * p) {
    int i = 0;
    write_string(g, "\"");
    while (i < SIZE(p)) {
        int ch;
        i += get_utf8(p + i, &ch);
        if (32 <= ch && ch < 0x590 && ch != 127) {
            if (ch == '"' || ch == '\\') write_char(g, '\\');
            write_wchar_as_utf8(g, ch);
        } else {
            // Use escapes for anything over 0x590 as a crude way to avoid
            // LTR characters affecting the rendering of source character
            // order in confusing ways.
            write_string(g, "\\u{");
            write_hex4(g, ch);
            write_string(g, "}");
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
    w(g, "~+{~N");
}

static void write_block_end(struct generator * g) {
    w(g, "~-~M}~N");
}

static void write_savecursor(struct generator * g, struct node * p,
                             struct str * savevar) {
    g->B[0] = str_data(savevar);
    g->S[1] = "";
    if (p->mode != m_forward) g->S[1] = "env.limit - ";
    writef(g, "~Mlet ~B0 = ~S1env.cursor;~N", p);
}

static void append_restore_string(struct node * p, struct str * out, struct str * savevar) {
    str_append_string(out, "env.cursor = ");
    if (p->mode != m_forward) str_append_string(out, "env.limit - ");
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
    write_string(g, p->mode == m_forward ? "env.next_char();" : "env.previous_char();");
    write_newline(g);
}

static void wsetlab_begin(struct generator * g, int n) {
    g->I[0] = n;
    w(g, "~M'lab~I0: loop {~N~+");
}

static void wsetlab_end(struct generator * g, int n) {
    if (!g->unreachable) {
        g->I[0] = n;
        w(g, "~Mbreak 'lab~I0;~N");
    }
    w(g, "~-~M}~N");
}

static void wgotol(struct generator * g, int n) {
    g->I[0] = n;
    w(g, "~Mbreak 'lab~I0;~N");
}

static void write_failure(struct generator * g) {
    if (str_len(g->failure_str) != 0) {
        write_margin(g);
        write_str(g, g->failure_str);
        write_newline(g);
    }
    switch (g->failure_label) {
        case x_return:
            w(g, "~Mreturn false;~N");
            g->unreachable = true;
            break;
        default:
            w(g, "~Mbreak 'lab");
            write_int(g, g->failure_label);
            w(g, ";~N");
            g->unreachable = true;
    }
}

static void write_failure_if(struct generator * g, const char * s, struct node * p) {
    writef(g, "~Mif ", p);
    writef(g, s, p);
    writef(g, " ", p);
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
}

/* if at limit fail */
static void write_check_limit(struct generator * g, struct node * p) {
    if (p->mode == m_forward) {
        write_failure_if(g, "env.cursor >= env.limit", p);
    } else {
        write_failure_if(g, "env.cursor <= env.limit_backward", p);
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
                write_varref(g, p->name);
                continue;
            case 'W':
                write_varname(g, p->name);
                continue;
            case 'L':
                write_literal_string(g, p->literalstring);
                continue;
            case '+': g->margin++; continue;
            case '-': g->margin--; continue;
            case 'n': write_string(g, g->options->name); continue;
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
            write_string(g, "i32::MAX"); break;
        case c_minint:
            write_string(g, "i32::MIN"); break;
        case c_neg:
            write_char(g, '-'); generate_AE(g, p->right); break;
        case c_multiply:
            s = " * "; goto label0;
        case c_plus:
            s = " + "; goto label0;
        case c_minus:
            s = " - "; goto label0;
        case c_divide:
            s = " / ";
        label0:
            write_char(g, '('); generate_AE(g, p->left);
            write_string(g, s); generate_AE(g, p->right); write_char(g, ')'); break;
        case c_cursor:
            w(g, "env.cursor"); break;
        case c_limit:
            w(g, p->mode == m_forward ? "env.limit" : "env.limit_backward"); break;
        case c_lenof:
            writef(g, "(~V.chars().count() as i32)", p);
            break;
        case c_sizeof:
            writef(g, "(~V.len() as i32)", p);
            break;
        case c_len:
            w(g, "(env.current.chars().count() as i32)");
            break;
        case c_size:
            w(g, "(env.current.len() as i32)");
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
        int label = new_label(g);
        g->failure_label = label;
        wsetlab_begin(g, label);
        generate(g, p);
        if (!g->unreachable) {
            wgotol(g, out_lab);
            end_unreachable = false;
        }
        w(g, "~-~M}~N");
        g->unreachable = false;
        if (savevar) write_restorecursor(g, p, savevar);
        p = p->right;
    }

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    generate(g, p);
    wsetlab_end(g, out_lab);
    if (!end_unreachable) {
        g->unreachable = false;
    }

    if (savevar) {
        str_delete(savevar);
    }
}

static void generate_backwards(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g,"~Menv.limit_backward = env.cursor;~N"
             "~Menv.cursor = env.limit;~N", p);
    generate(g, p->left);
    w(g, "~Menv.cursor = env.limit_backward;~N");
}


static void generate_not(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);
    int label = new_label(g);
    g->failure_label = label;

    write_comment(g, p);
    if (savevar) {
        write_savecursor(g, p, savevar);
    }

    str_clear(g->failure_str);

    wsetlab_begin(g, label);

    generate(g, p->left);

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    if (!g->unreachable) write_failure(g);
    w(g, "~-~M}~N");

    g->unreachable = false;

    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }
}


static void generate_try(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(g, p->left)) {
        savevar = vars_newname(g);
    }

    int label = new_label(g);
    g->failure_label = label;
    str_clear(g->failure_str);

    write_comment(g, p);
    if (savevar) {
        write_savecursor(g, p, savevar);
        append_restore_string(p, g->failure_str, savevar);
    }
    wsetlab_begin(g, label);
    generate(g, p->left);
    wsetlab_end(g, label);
    g->unreachable = false;

    if (savevar) {
        str_delete(savevar);
    }
}

static void generate_set(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~M~V = true;~N", p);
}

static void generate_unset(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~M~V = false;~N", p);
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
        writef(g, "~M~W(env, context);~N", p->left);
    } else {
        int label = new_label(g);
        g->failure_label = label;
        str_clear(g->failure_str);

        wsetlab_begin(g, label);
        generate(g, p->left);
        wsetlab_end(g, label);
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
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    write_failure_if(g, "!env.go_~S1_grouping~S0(~W, ~I0, ~I1)", p);
    if (!is_goto) {
        write_string(g, p->mode == m_forward ? "env.next_char();" : "env.previous_char();");
    }
}

static void generate_GO(struct generator * g, struct node * p, int style) {
    write_comment(g, p);

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    int end_unreachable = false;

    int golab = new_label(g);
    g->I[0] = golab;
    w(g, "~M'golab~I0: loop {~N~+");

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
        w(g, "~Mbreak 'golab~I0;~N");
    }
    g->unreachable = false;
    w(g, "~-~M}~N");
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
    w(g, "~Mfor _ in 0..");
    generate_AE(g, p->AE);
    writef(g, " {~+~N", p);

    generate(g, p->left);

    w(g, "~-~M}~N");
    str_delete(loopvar);
    g->unreachable = false;
}

static void generate_repeat_or_atleast(struct generator * g, struct node * p, struct str * loopvar) {
    int replab = new_label(g);
    g->I[0] = replab;
    writef(g, "~M'replab~I0: loop{~N~+", p);

    struct str * savevar = NULL;
    if (repeat_restore(g, p->left)) {
        savevar = vars_newname(g);
        write_savecursor(g, p, savevar);
    }

    g->failure_label = new_label(g);
    str_clear(g->failure_str);
    g->I[0] = g->failure_label;
    w(g, "~M'lab~I0: for _ in 0..1 {~N~+");
    generate(g, p->left);

    if (!g->unreachable) {
        if (loopvar != NULL) {
            g->B[0] = str_data(loopvar);
            w(g, "~M~B0 -= 1;~N");
        }

        g->I[0] = replab;
        w(g, "~Mcontinue 'replab~I0;~N");
    }
    w(g, "~-~M}~N");
    g->unreachable = false;

    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }

    g->I[0] = replab;
    w(g, "~Mbreak 'replab~I0;~N~-~M}~N");
}

static void generate_repeat(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate_repeat_or_atleast(g, p, NULL);
}

static void generate_atleast(struct generator * g, struct node * p) {
    struct str * loopvar = vars_newname(g);

    write_comment(g, p);
    g->B[0] = str_data(loopvar);
    w(g, "~Mlet mut ~B0 = ");
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
    str_delete(loopvar);
}

static void generate_setmark(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~M~V = env.cursor;~N", p);
}

static void generate_tomark(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? ">" : "<";

    w(g, "~Mif env.cursor ~S0 "); generate_AE(g, p->AE);
    writef(g, " ", p);
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
    w(g, "~Menv.cursor = "); generate_AE(g, p->AE); writef(g, ";~N", p);
}

static void generate_atmark(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~Mif env.cursor != "); generate_AE(g, p->AE); writef(g, " ", p);
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
}

static void generate_hop(struct generator * g, struct node * p) {
    write_comment(g, p);
    // Generate the AE to a temporary block so we can substitute it in
    // write_failure_if().
    struct str * ae = str_new();
    struct str * s = g->outbuf;
    g->outbuf = ae;
    generate_AE(g, p->AE);
    g->outbuf = s;
    g->B[0] = str_data(ae);
    g->S[0] = p->mode == m_forward ? "" : "_back";
    g->S[1] = p->AE->type == c_number ? "" : "_checked";
    write_failure_if(g, "!env.hop~S0~S1(~B0)", p);
    str_delete(ae);
}

static void generate_delete(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~Mif !env.slice_del() {~N"
              "~+~Mreturn false;~N~-"
              "~M}~N", p);
}

static void generate_tolimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "env.limit" : "env.limit_backward";
    writef(g, "~Menv.cursor = ~S0;~N", p);
}

static void generate_atlimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "env.limit" : "env.limit_backward";
    g->S[1] = p->mode == m_forward ? "<" : ">";
    write_failure_if(g, "env.cursor ~S1 ~S0", p);
}

static void generate_leftslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "env.bra" : "env.ket";
    writef(g, "~M~S0 = env.cursor;~N", p);
}

static void generate_rightslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "env.ket" : "env.bra";
    writef(g, "~M~S0 = env.cursor;~N", p);
}

static void generate_assignto(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~M~V = env.assign_to();~N", p);
}

static void generate_sliceto(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~M~V = env.slice_to();~N"
              "~Mif ~V.is_empty() {~N"
              "~+~Mreturn false;~N~-~M}~N", p);
}

static void generate_address(struct generator * g, struct node * p) {
    /* If we deal with a string variable which is of type String we need to
     * pass it by reference not by value.  Literalstrings on the other hand are
     * of type &'static str so we can pass them by value.
     */
    symbol * b = p->literalstring;
    if (b != NULL) {
        write_literal_string(g, b);
    } else {
        write_char(g, '&');
        write_varref(g, p->name);
    }
}

static void generate_insert(struct generator * g, struct node * p, int style) {
    int keep_c = style == c_attach;
    write_comment(g, p);
    if (p->mode == m_backward) keep_c = !keep_c;
    if (keep_c) w(g, "~Mlet c = env.cursor;~N");
    w(g, "~Mlet (bra, ket) = (env.cursor, env.cursor);~N");
    writef(g, "~Menv.insert(bra, ket, ", p);
    generate_address(g, p);
    writef(g, ");~N", p);
    if (keep_c) w(g, "~Menv.cursor = c;~N");
}

static void generate_assignfrom(struct generator * g, struct node * p) {
    int keep_c = p->mode == m_forward; /* like 'attach' */

    write_comment(g, p);
    if (keep_c) writef(g, "~Mlet c = env.cursor;~N", p);
    /* Copying limits and cursors is necessary here because the rust
     * borrowchecker does not like taking something from someone you are about
     * to mutate... */
    if (p->mode == m_forward) {
        writef(g, "~Mlet (bra, ket) = (env.cursor, env.limit);~N", p);
    } else {
        writef(g, "~Mlet (bra, ket) = (env.limit_backward, env.cursor);~N", p);
    }
    writef(g, "~Menv.insert(bra, ket, ", p);
    generate_address(g, p);
    writef(g, ");~N", p);
    if (keep_c) w(g, "~Menv.cursor = c;~N");
}

static void generate_slicefrom(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~Mif !env.slice_from(");
    generate_address(g, p);
    writef(g, ") {~N"
              "~+~Mreturn false;~N~-~M}~N", p);
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
        w(g, "~Mif env.cursor ~S0 "); generate_AE(g, q->AE); w(g, " ");
        write_block_start(g);
        write_failure(g);
        write_block_end(g);
        g->unreachable = false;

        g->B[0] = str_data(varname);
        if (p->mode == m_forward) {
            w(g, "~Mlet ~B0 = env.limit - env.cursor;~N");
            w(g, "~Menv.limit = ");
        } else {
            w(g, "~Mlet ~B0 = env.limit_backward;~N");
            w(g, "~Menv.limit_backward = ");
        }
        generate_AE(g, q->AE);
        writef(g, ";~N", q);

        if (p->mode == m_forward) {
            str_assign(g->failure_str, "env.limit += ");
            str_append(g->failure_str, varname);
            str_append_string(g->failure_str, ";");
        } else {
            str_assign(g->failure_str, "env.limit_backward = ");
            str_append(g->failure_str, varname);
            str_append_string(g->failure_str, ";");
        }
    } else {
        struct str * savevar = vars_newname(g);
        write_savecursor(g, p, savevar);

        generate(g, p->left);

        if (!g->unreachable) {
            g->B[0] = str_data(varname);
            if (p->mode == m_forward) {
                w(g, "~Mlet ~B0 = env.limit - env.cursor;~N");
                w(g, "~Menv.limit = env.cursor;~N");
            } else {
                w(g, "~Mlet ~B0 = env.limit_backward;~N");
                w(g, "~Menv.limit_backward = env.cursor;~N");
            }
            write_restorecursor(g, p, savevar);

            if (p->mode == m_forward) {
                str_assign(g->failure_str, "env.limit += ");
                str_append(g->failure_str, varname);
                str_append_string(g->failure_str, ";");
            } else {
                str_assign(g->failure_str, "env.limit_backward = ");
                str_append(g->failure_str, varname);
                str_append_string(g->failure_str, ";");
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
    writef(g, "~Mlet ~B0 = env.clone();~N"
              "~Menv.set_current_s(~V.clone());~N"
              "~Menv.cursor = 0;~N"
              "~Menv.limit = env.current.len() as i32;~N", p);
    generate(g, p->left);
    if (!g->unreachable) {
        g->B[0] = str_data(savevar);
        /* Update string variable. */
        writef(g, "~M~V = env.current.clone().into_owned();~N", p);
        /* Reset env */
        w(g, "~M*env = ~B0;~N");
    }
    str_delete(savevar);
}

static void generate_integer_assign(struct generator * g, struct node * p, const char * s) {
    write_comment(g, p);
    g->S[0] = s;
    writef(g, "~M~V ~S0 ", p); generate_AE(g, p->AE); w(g, ";~N");
}

static void generate_integer_test(struct generator * g, struct node * p) {
    write_comment(g, p);
    int relop = p->type;
    int optimise_to_return = (g->failure_label == x_return && p->right && p->right->type == c_functionend);
    if (optimise_to_return) {
        w(g, "~Mreturn ");
        p->right = NULL;
    } else {
        w(g, "~Mif ");
        // We want the inverse of the snowball test here.
        relop ^= 1;
    }
    generate_AE(g, p->left);
    // Relational operators are the same as C.
    write_c_relop(g, relop);
    generate_AE(g, p->AE);
    if (optimise_to_return) {
        w(g, "~N");
    } else {
        write_block_start(g);
        write_failure(g);
        write_block_end(g);
        g->unreachable = false;
    }
}

static void generate_call(struct generator * g, struct node * p) {
    int signals = check_possible_signals_list(g, p->name->definition, c_define, 0);
    write_comment(g, p);
    if (g->failure_label == x_return &&
        (signals == 0 || (p->right && p->right->type == c_functionend))) {
        /* Always fails or tail call. */
        writef(g, "~Mreturn ~W(env, context);~N", p);
        if (p->right && p->right->type == c_functionend) {
            p->right = NULL;
        }
        g->unreachable = true;
        return;
    }
    if (signals == 1) {
        /* Always succeeds. */
        writef(g, "~M~W(env, context);~N", p);
    } else if (signals == 0) {
        /* Always fails. */
        writef(g, "~M~W(env, context);~N", p);
        write_failure(g);
    } else {
        write_failure_if(g, "!~W(env, context)", p);
    }
}

static void generate_grouping(struct generator * g, struct node * p, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->S[1] = complement ? "out" : "in";
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    write_failure_if(g, "!env.~S1_grouping~S0(~W, ~I0, ~I1)", p);
}

static void generate_namedstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "_b";
    write_failure_if(g, "!env.eq_s~S0(&~V)", p);
}

static void generate_literalstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "_b";
    write_failure_if(g, "!env.eq_s~S0(&~L)", p);
}

static void generate_setup_context(struct generator * g) {
    w(g, "~Mlet mut context = &mut Context {~+~N");
    for (struct name * q = g->analyser->names; q; q = q->next) {
        switch (q->type) {
            case t_string:
                write_margin(g);
                write_varname(g, q);
                w(g, ": String::new(),~N");
                break;
            case t_integer:
                write_margin(g);
                write_varname(g, q);
                w(g, ": 0,~N");
                break;
            case t_boolean:
                write_margin(g);
                write_varname(g, q);
                w(g, ": false,~N");
                break;
        }
    }
    w(g, "~-~M};~N");
}

static void generate_define(struct generator * g, struct node * p) {
    struct name * q = p->name;
    if (q->type == t_routine && !q->used) return;

    write_newline(g);
    write_comment(g, p);

    if (q->type == t_routine) {
        writef(g, "~Mfn ~W(env: &mut SnowballEnv, context: &mut Context) -> bool {~+~N", p);
    } else {
        writef(g, "~Mpub fn ~W(env: &mut SnowballEnv) -> bool {~+~N", p);
        generate_setup_context(g);
    }
    if (p->amongvar_needed) w(g, "~Mlet mut among_var;~N");

    /* Save output. */
    struct str * saved_output = g->outbuf;
    g->outbuf = str_new();

    g->next_label = 0;
    g->var_number = 0;

    str_clear(g->failure_str);
    g->failure_label = x_return;
    g->unreachable = false;
    int signals = check_possible_signals_list(g, p->left, c_define, 0);

    /* Generate function body. */
    generate(g, p->left);
    if (p->left->right) {
        assert(p->left->right->type == c_functionend);
        if (signals) {
            generate(g, p->left->right);
        }
    }
    w(g, "~-~M}~N");

    str_append(saved_output, g->outbuf);
    str_delete(g->outbuf);
    g->outbuf = saved_output;
}

static void generate_functionend(struct generator * g, struct node * p) {
    (void)p;
    w(g, "~Mreturn true~N");
}

static void generate_substring(struct generator * g, struct node * p) {
    write_comment(g, p);

    struct among * x = p->among;
    int block = -1;
    unsigned int bitmap = 0;
    struct amongvec * among_cases = x->b;
    int empty_case = -1;
    int n_cases = 0;
    symbol cases[2];
    int shortest_size = x->shortest_size;
    int block_opened = 0;

    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->I[0] = x->number;
    g->I[1] = x->literalstring_count;

    /* In forward mode with non-ASCII UTF-8 characters, the first byte
     * of the string will often be the same, so instead look at the last
     * common byte position.
     *
     * In backward mode, we can't match if there are fewer characters before
     * the current position than the minimum length.
     */
    for (int c = 0; c < x->literalstring_count; ++c) {
        symbol ch;
        if (among_cases[c].size == 0) {
            empty_case = c;
            continue;
        }
        if (p->mode == m_forward) {
            ch = among_cases[c].b[shortest_size - 1];
        } else {
            ch = among_cases[c].b[among_cases[c].size - 1];
        }
        if (n_cases == 0) {
            block = ch >> 5;
        } else if (ch >> 5 != block) {
            block = -1;
            if (n_cases > 2) break;
        }
        if (block == -1) {
            if (n_cases > 0 && ch == cases[0]) continue;
            if (n_cases < 2) {
                cases[n_cases++] = ch;
            } else if (ch != cases[1]) {
                ++n_cases;
                break;
            }
        } else {
            if ((bitmap & (1u << (ch & 0x1f))) == 0) {
                bitmap |= 1u << (ch & 0x1f);
                if (n_cases < 2)
                    cases[n_cases] = ch;
                ++n_cases;
            }
        }
    }

    if (block != -1 || n_cases <= 2) {
        char buf[64];
        g->I[2] = block;
        g->I[3] = bitmap;
        g->I[4] = shortest_size - 1;
        if (p->mode == m_forward) {
            sprintf(buf, "env.current.as_bytes()[(env.cursor + %d) as usize]", shortest_size - 1);
            g->S[1] = buf;
            if (shortest_size == 1) {
                writef(g, "~Mif (env.cursor >= env.limit", p);
            } else {
                writef(g, "~Mif (env.cursor + ~I4 >= env.limit", p);
            }
        } else {
            g->S[1] = "env.current.as_bytes()[(env.cursor - 1) as usize]";
            if (shortest_size == 1) {
                writef(g, "~Mif (env.cursor <= env.limit_backward", p);
            } else {
                writef(g, "~Mif (env.cursor - ~I4 <= env.limit_backward", p);
            }
        }
        if (n_cases == 0) {
            /* We get this for the degenerate case: among ( '' )
             * This doesn't seem to be a useful construct, but it is
             * syntactically valid.
             */
        } else if (n_cases == 1) {
            g->I[4] = cases[0];
            writef(g, " || ~S1 as u8 != ~I4 as u8", p);
        } else if (n_cases == 2) {
            g->I[4] = cases[0];
            g->I[5] = cases[1];
            writef(g, " || (~S1 as u8 != ~I4 as u8 && ~S1 as u8 != ~I5 as u8)", p);
        } else {
            writef(g, " || ~S1 as u8 >> 5 != ~I2 as u8 || ((~I3 as i32 >> (~S1 as u8 & 0x1f)) & 1) == 0", p);
        }
        write_string(g, ") ");
        if (empty_case != -1) {
            /* If the among includes the empty string, it can never fail
             * so not matching the bitmap means we match the empty string.
             */
            g->I[4] = among_cases[empty_case].result;
            writef(g, "{among_var = ~I4;}~N~Melse ", p);
            write_block_start(g);
            block_opened = 1;
        } else {
            writef(g, "~f~N", p);
        }
    } else {
#ifdef OPTIMISATION_WARNINGS
        printf("Couldn't shortcut among %d\n", x->number);
#endif
    }

    if (x->amongvar_needed) {
        writef(g, "~Mamong_var = env.find_among~S0(A_~I0, context);~N", p);
        if (!x->always_matches) {
            write_failure_if(g, "among_var == 0", p);
        }
    } else if (x->always_matches) {
        writef(g, "~Menv.find_among~S0(A_~I0, context);~N", p);
    } else if (x->command_count == 0 &&
               x->node->right && x->node->right->type == c_functionend) {
        writef(g, "~Mreturn env.find_among~S0(A_~I0, context) != 0;~N", p);
        x->node->right = NULL;
        g->unreachable = true;
    } else {
        write_failure_if(g, "env.find_among~S0(A_~I0, context) == 0", p);
    }
    if (block_opened) write_block_end(g);
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
        w(g, "~Mmatch among_var {~N~+");
        for (int i = 1; i <= x->command_count; i++) {
            g->I[0] = i;
            w(g, "~M~I0 => {~N~+");
            generate(g, x->commands[i - 1]);
            w(g, "~-~M}~N");
            g->unreachable = false;
        }
        w(g, "~M_ => ()~N");
        w(g, "~-~M}~N");
    }
}

static void generate_booltest(struct generator * g, struct node * p, int inverted) {
    write_comment(g, p);
    if (g->failure_label == x_return) {
        if (p->right && p->right->type == c_functionend) {
            // Optimise at end of function.
            if (inverted) {
                writef(g, "~Mreturn !~V;~N", p);
            } else {
                writef(g, "~Mreturn ~V;~N", p);
            }
            p->right = NULL;
            g->unreachable = true;
            return;
        }
    }
    if (inverted) {
        write_failure_if(g, "~V", p);
    } else {
        write_failure_if(g, "!~V", p);
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
    writef(g, "~Menv.debug(~I0, ~I1);~N", p);
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
        case c_divideassign:  generate_integer_assign(g, p, "/="); break;
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

/* rustc emits warnings if variables don't match the style guide */
/* (i.e. upper-case for globals, snake case for fields etc.) */
/* To allow warning free compilation of generated code and */
/* consistency with snowball variable namings we allow some kind of warnings here */
static void generate_allow_warnings(struct generator * g) {
    w(g, "#![allow(non_snake_case)]~N");
    w(g, "#![allow(non_upper_case_globals)]~N");
    w(g, "#![allow(unused_mut)]~N");
    w(g, "#![allow(unused_parens)]~N");
    w(g, "#![allow(unused_variables)]~N");
}

static void generate_class_begin(struct generator * g) {
    w(g, "use snowball::SnowballEnv;~N");
    if (g->analyser->among_count > 0) {
        w(g, "use snowball::Among;~N~N");
    }
}

static void generate_among_table(struct generator * g, struct among * x) {
    write_comment(g, x->node);

    struct amongvec * v = x->b;

    g->I[0] = x->number;
    g->I[1] = x->literalstring_count;

    w(g, "~Mstatic A_~I0: &'static [Among<Context>; ~I1] = &[~N~+");
    for (int i = 0; i < x->literalstring_count; i++) {
        g->I[0] = v[i].i;
        g->I[1] = v[i].result;
        g->S[0] = ",";

        w(g, "~MAmong(");
        write_literal_string(g, v[i].b);
        w(g, ", ~I0, ~I1, ");
        if (v[i].function != NULL) {
            w(g, "Some(&");
            write_varname(g, v[i].function);
            w(g, ")");
        } else {
            w(g, "None");
        }
        w(g, ")~S0~N");
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

    g->I[0] = size;
    w(g, "~Mstatic ");
    write_varname(g, q->name);
    w(g, ": &'static [u8; ~I0] = &[");
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
    w(g, "#[derive(Clone)]~N");
    w(g, "struct Context {~+~N");
    for (struct name * q = g->analyser->names; q; q = q->next) {
        switch (q->type) {
            case t_string:
                write_margin(g);
                write_varname(g, q);
                w(g, ": String,~N");
                break;
            case t_integer:
                write_margin(g);
                write_varname(g, q);
                w(g, ": i32,~N");
                break;
            case t_boolean:
                write_margin(g);
                write_varname(g, q);
                w(g, ": bool,~N");
                break;
        }
    }
    w(g, "~-}~N");
}

static void generate_methods(struct generator * g) {
    for (struct node * p = g->analyser->program; p; p = p->right) {
        generate(g, p);
        g->unreachable = false;
    }
}

extern void generate_program_rust(struct generator * g) {
    g->outbuf = str_new();
    g->failure_str = str_new();

    write_start_comment(g, "//! ", NULL);
    generate_allow_warnings(g);
    if (g->analyser->int_limits_used) {
        /* std::i32 is used in the code generated for i32::MAX and i32::MIN */
        w(g, "use std::i32;~N~N");
    }
    generate_class_begin(g);

    generate_amongs(g);
    generate_groupings(g);

    generate_members(g);
    generate_methods(g);

    output_str(g->options->output_src, g->outbuf);
    str_delete(g->failure_str);
    str_delete(g->outbuf);
}
