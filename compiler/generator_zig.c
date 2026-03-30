#include <assert.h>
#include <stdlib.h> /* for exit */
#include <string.h> /* for strlen */
#include <stdio.h> /* for fprintf etc */
#include <ctype.h> /* for toupper */
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
    write_char(g, "SbirrG"[p->type]);
    write_char(g, '_');
    write_s(g, p->s);
}

static void write_varref(struct generator * g, struct name * p) {
    if (p->local_to == NULL) {
        write_string(g, "context");
        write_char(g, '.');
    }
    write_varname(g, p);
}

static void write_stringref(struct generator * g, struct name * p) {
    assert(p->type == t_string);
    write_varref(g, p);
    write_string(g, ".slice()");
}

static void write_literal_string(struct generator * g, symbol * p) {
    int i = 0;
    write_char(g, '"');
    while (i < SIZE(p)) {
        int ch;
        i += get_utf8(p + i, &ch);
        if (32 <= ch && ch < 0x590 && ch != 127) {
            if (ch == '"' || ch == '\\') write_char(g, '\\');
            write_wchar_as_utf8(g, ch);
        } else {
            /* Zig uses \u{XXXX} for unicode escapes (variable width) */
            char buf[16];
            snprintf(buf, sizeof(buf), "\\u{%X}", ch);
            write_string(g, buf);
        }
    }
    write_char(g, '"');
}

static void write_margin(struct generator * g) {
    for (int i = 0; i < g->margin; i++) write_string(g, "    ");
}

static void write_comment(struct generator * g, struct node * p) {
    if (!g->options->comments) return;
    write_margin(g);
    write_string(g, "// ");
    write_comment_content(g, p, NULL);
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
    writef(g, "~Mconst ~B0 = ~S1env.cursor;~N", p);
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
    write_string(g, p->mode == m_forward ? "env.nextChar();" : "env.prevChar();");
    write_newline(g);
}

static void wsetlab_begin(struct generator * g, int n) {
    g->I[0] = n;
    w(g, "~Mlab~I0: while (true) {~N~+");
}

static void wsetlab_end(struct generator * g, int n) {
    if (!g->unreachable) {
        g->I[0] = n;
        w(g, "~Mbreak :lab~I0;~N");
    }
    w(g, "~-~M}~N");
}

static void wgotol(struct generator * g, int n) {
    g->I[0] = n;
    w(g, "~Mbreak :lab~I0;~N");
    g->unreachable = true;
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
            w(g, "~Mbreak :lab");
            write_int(g, g->failure_label);
            write_string(g, ";");
            write_newline(g);
            g->unreachable = true;
    }
}

static void write_failure_if(struct generator * g, const char * s, struct node * p) {
    writef(g, "~Mif (", p);
    writef(g, s, p);
    writef(g, ") ", p);
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
            case 'E': {
                write_s(g, p->name->s);
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
            case 'n': write_s(g, g->options->name); continue;
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
            write_string(g, "snowball.MaxInt"); break;
        case c_minint:
            write_string(g, "snowball.MinInt"); break;
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
            write_string(g, "@divTrunc("); generate_AE(g, p->left);
            write_string(g, ", "); generate_AE(g, p->right); write_char(g, ')'); break;
        case c_cursor:
            w(g, "@as(i32, @intCast(env.cursor))"); break;
        case c_limit:
            w(g, p->mode == m_forward ? "@as(i32, @intCast(env.limit))" : "@as(i32, @intCast(env.limit_backward))"); break;
        case c_lenof:
            write_string(g, "snowball.runeCountInString(");
            write_stringref(g, p->name);
            write_char(g, ')');
            break;
        case c_sizeof:
            write_string(g, "@as(i32, @intCast(");
            write_stringref(g, p->name);
            write_string(g, ".len))");
            break;
        case c_len:
            w(g, "snowball.runeCountInString(env.getCurrent())");
            break;
        case c_size:
            w(g, "@as(i32, @intCast(env.getCurrent().len))");
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
        wsetlab_end(g, label);
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
    wsetlab_end(g, label);

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
        writef(g, "~M_ = ~W(env, @ptrCast(context));~N", p->left);
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
    g->S[0] = p->mode == m_forward ? "" : "B";
    g->S[1] = complement ? "In" : "Out";
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    write_failure_if(g, "!env.go~S1Grouping~S0(&~W, ~I0, ~I1)", p);
    if (!is_goto) {
        w(g, p->mode == m_forward ? "~Menv.nextChar();~N" : "~Menv.prevChar();~N");
    }
}

static void generate_GO(struct generator * g, struct node * p, int style) {
    write_comment(g, p);

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    int end_unreachable = false;

    int golab = new_label(g);
    g->I[0] = golab;
    w(g, "~Mgolab~I0: while (true) {~N~+");

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
        end_unreachable = true;
    } else {
        if (style == 1) write_restorecursor(g, p, savevar);
        g->I[0] = golab;
        w(g, "~Mbreak :golab~I0;~N");
        g->unreachable = true;
    }
    wsetlab_end(g, g->failure_label);
    g->unreachable = false;
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
    w(g, "~M{~N~+~Mvar ~B0: i32 = ");
    generate_AE(g, p->AE);
    w(g, ";~N");
    w(g, "~Mwhile (~B0 > 0) {~N~+");
    w(g, "~M~B0 -= 1;~N");

    generate(g, p->left);

    w(g, "~-~M}~N");
    w(g, "~-~M}~N");
    str_delete(loopvar);
    g->unreachable = false;
}

static void generate_repeat_or_atleast(struct generator * g, struct node * p, struct str * loopvar) {
    int replab = new_label(g);
    g->I[0] = replab;
    writef(g, "~Mreplab~I0: while (true) {~N~+", p);

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
            w(g, "~M~B0 -= 1;~N");
        }

        g->I[0] = replab;
        w(g, "~Mcontinue :replab~I0;~N");
        g->unreachable = true;
    }
    wsetlab_end(g, g->failure_label);
    g->unreachable = false;

    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }

    g->I[0] = replab;
    w(g, "~Mbreak :replab~I0;~N~-~M}~N");
}

static void generate_repeat(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate_repeat_or_atleast(g, p, NULL);
}

static void generate_atleast(struct generator * g, struct node * p) {
    struct str * loopvar = vars_newname(g);

    write_comment(g, p);
    g->B[0] = str_data(loopvar);
    w(g, "~Mvar ~B0: i32 = ");
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
    writef(g, "~M~V = @as(i32, @intCast(env.cursor));~N", p);
}

static void generate_tomark(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? ">" : "<";

    w(g, "~Mif (@as(i32, @intCast(env.cursor)) ~S0 "); generate_AE(g, p->AE); writef(g, ") ", p);
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
    w(g, "~Menv.cursor = @intCast(@as(u32, @intCast("); generate_AE(g, p->AE); writef(g, ")));~N", p);
}

static void generate_atmark(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~Mif (@as(i32, @intCast(env.cursor)) != "); generate_AE(g, p->AE); writef(g, ") ", p);
    write_block_start(g);
    write_failure(g);
    write_block_end(g);
    g->unreachable = false;
}

static void generate_hop(struct generator * g, struct node * p) {
    write_comment(g, p);
    struct str * ae = str_new();
    struct str * s = g->outbuf;
    g->outbuf = ae;
    generate_AE(g, p->AE);
    g->outbuf = s;
    g->B[0] = str_data(ae);
    g->S[0] = p->mode == m_forward ? "" : "Back";
    g->S[1] = p->AE->type == c_number ? "" : "Checked";
    write_failure_if(g, "!env.hop~S0~S1(~B0)", p);
    str_delete(ae);
}

static void generate_delete(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~Menv.sliceDel() catch return false;~N", p);
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
    write_margin(g);
    if (p->name->local_to == NULL) {
        write_string(g, "context.");
    }
    write_varname(g, p->name);
    write_string(g, ".assign(env.allocator, env.assignTo()) catch return false;");
    write_newline(g);
}

static void generate_sliceto(struct generator * g, struct node * p) {
    write_comment(g, p);
    write_margin(g);
    if (p->name->local_to == NULL) {
        write_string(g, "context.");
    }
    write_varname(g, p->name);
    write_string(g, ".assign(env.allocator, env.sliceTo()) catch return false;");
    write_newline(g);
}

static void generate_address(struct generator * g, struct node * p) {
    symbol * b = p->literalstring;
    if (b != NULL) {
        write_literal_string(g, b);
    } else {
        write_stringref(g, p->name);
    }
}

static void generate_insert(struct generator * g, struct node * p, int style) {
    int keep_c = style == c_attach;

    write_margin(g);
    write_block_start(g);
    write_comment(g, p);
    if (p->mode == m_backward) keep_c = !keep_c;
    if (keep_c) w(g, "~Mconst c = env.cursor;~N");
    w(g, "~Mconst bra = env.cursor;~N");
    w(g, "~Mconst ket = env.cursor;~N");
    writef(g, "~Menv.insert(bra, ket, ", p);
    generate_address(g, p);
    writef(g, ") catch return false;~N", p);
    if (keep_c) w(g, "~Menv.cursor = c;~N");
    write_block_end(g);
}

static void generate_assignfrom(struct generator * g, struct node * p) {
    int keep_c = p->mode == m_forward;

    write_margin(g);
    write_block_start(g);
    write_comment(g, p);
    if (keep_c) writef(g, "~Mconst c = env.cursor;~N", p);
    if (p->mode == m_forward) {
        writef(g, "~Menv.insert(env.cursor, env.limit, ", p);
    } else {
        writef(g, "~Menv.insert(env.limit_backward, env.cursor, ", p);
    }
    generate_address(g, p);
    writef(g, ") catch return false;~N", p);
    if (keep_c) w(g, "~Menv.cursor = c;~N");
    write_block_end(g);
}

static void generate_slicefrom(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~Menv.sliceFrom(");
    generate_address(g, p);
    writef(g, ") catch return false;~N", p);
}

static void generate_setlimit(struct generator * g, struct node * p) {
    struct str * varname = vars_newname(g);
    write_comment(g, p);
    if (p->left && p->left->type == c_tomark) {
        struct node * q = p->left;
        write_comment(g, q);
        g->S[0] = q->mode == m_forward ? ">" : "<";
        w(g, "~Mif (@as(i32, @intCast(env.cursor)) ~S0 "); generate_AE(g, q->AE); w(g, ") ");
        write_block_start(g);
        write_failure(g);
        write_block_end(g);
        g->unreachable = false;

        g->B[0] = str_data(varname);
        if (p->mode == m_forward) {
            w(g, "~Mconst ~B0 = env.limit - env.cursor;~N");
            w(g, "~Menv.limit = @intCast(@as(u32, @intCast(");
        } else {
            w(g, "~Mconst ~B0 = env.limit_backward;~N");
            w(g, "~Menv.limit_backward = @intCast(@as(u32, @intCast(");
        }
        generate_AE(g, q->AE); writef(g, ")));~N", q);

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
                w(g, "~Mconst ~B0 = env.limit - env.cursor;~N");
                w(g, "~Menv.limit = env.cursor;~N");
            } else {
                w(g, "~Mconst ~B0 = env.limit_backward;~N");
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

static void generate_dollar(struct generator * g, struct node * p) {
    write_comment(g, p);

    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);
    g->failure_label = new_label(g);
    str_clear(g->failure_str);

    struct str * savevar = vars_newname(g);
    g->B[0] = str_data(savevar);
    writef(g, "~Mvar ~B0 = env.clone() catch return false;~N", p);
    write_margin(g);
    write_string(g, "env.setCurrent(");
    write_stringref(g, p->name);
    write_string(g, ") catch return false;");
    write_newline(g);
    if (p->left->possible_signals == -1) {
        w(g, "~Mvar ~B0_f = true;~N");
    }

    wsetlab_begin(g, g->failure_label);

    generate(g, p->left);

    if (!g->unreachable && p->left->possible_signals == -1) {
        g->B[0] = str_data(savevar);
        w(g, "~M~B0_f = false;~N");
    }

    wsetlab_end(g, g->failure_label);

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    g->B[0] = str_data(savevar);
    write_margin(g);
    if (p->name->local_to == NULL) {
        write_string(g, "context.");
    }
    write_varname(g, p->name);
    write_string(g, ".assign(env.allocator, env.getCurrent()) catch return false;");
    write_newline(g);
    writef(g, "~Menv.copyFrom(&~B0) catch return false;~N", p);
    if (p->left->possible_signals == 0) {
        write_failure(g);
    } else if (p->left->possible_signals == -1) {
        write_failure_if(g, "~B0_f", p);
    }

    str_delete(savevar);
}

static void generate_integer_assign(struct generator * g, struct node * p, const char * s) {
    write_comment(g, p);
    g->S[0] = s;
    writef(g, "~M~V ~S0 ", p);
    generate_AE(g, p->AE);
    w(g, ";~N");
}

static void generate_integer_test(struct generator * g, struct node * p) {
    write_comment(g, p);
    int relop = p->type;
    int optimise_to_return = tailcallable(g, p);
    if (optimise_to_return) {
        w(g, "~Mreturn ");
        p->right = NULL;
    } else {
        w(g, "~Mif (");
        relop ^= 1;
    }
    generate_AE(g, p->left);
    write_c_relop(g, relop);
    generate_AE(g, p->AE);
    if (optimise_to_return) {
        w(g, ";~N");
        g->unreachable = true;
    } else {
        write_string(g, ") ");
        write_block_start(g);
        write_failure(g);
        write_block_end(g);
        g->unreachable = false;
    }
}

static void generate_call(struct generator * g, struct node * p) {
    int signals = p->name->definition->possible_signals;
    write_comment(g, p);
    if (tailcallable(g, p)) {
        writef(g, "~Mreturn ~W(env, @ptrCast(context));~N", p);
        p->right = NULL;
        g->unreachable = true;
        return;
    }
    if (just_return_on_fail(g) && signals == 0) {
        writef(g, "~Mreturn ~W(env, @ptrCast(context));~N", p);
        g->unreachable = true;
        return;
    }
    if (signals == 1) {
        writef(g, "~M_ = ~W(env, @ptrCast(context));~N", p);
    } else if (signals == 0) {
        writef(g, "~M_ = ~W(env, @ptrCast(context));~N", p);
        write_failure(g);
    } else {
        write_failure_if(g, "!~W(env, @ptrCast(context))", p);
    }
}

static void generate_grouping(struct generator * g, struct node * p, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "B";
    g->S[1] = complement ? "out" : "in";
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    if (tailcallable(g, p)) {
        writef(g, "~Mreturn env.~S1Grouping~S0(&~W, ~I0, ~I1);~N", p);
        p->right = NULL;
        g->unreachable = true;
    } else {
        write_failure_if(g, "!env.~S1Grouping~S0(&~W, ~I0, ~I1)", p);
    }
}

static void generate_namedstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "B";
    if (tailcallable(g, p)) {
        write_margin(g);
        write_string(g, "return env.eqS");
        write_string(g, g->S[0]);
        write_char(g, '(');
        write_stringref(g, p->name);
        write_string(g, ");");
        write_newline(g);
        p->right = NULL;
        g->unreachable = true;
    } else {
        write_margin(g);
        write_string(g, "if (!env.eqS");
        write_string(g, g->S[0]);
        write_char(g, '(');
        write_stringref(g, p->name);
        write_string(g, ")) ");
        write_block_start(g);
        write_failure(g);
        write_block_end(g);
        g->unreachable = false;
    }
}

static void generate_literalstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "B";
    if (tailcallable(g, p)) {
        writef(g, "~Mreturn env.eqS~S0(~L);~N", p);
        p->right = NULL;
        g->unreachable = true;
    } else {
        write_failure_if(g, "!env.eqS~S0(~L)", p);
    }
}

static void generate_cast_context(struct generator * g) {
    w(g, "~Mconst context: *Context = @ptrCast(@alignCast(ctx));~N");
    w(g, "~Msuppress_any_unused_warning(@as(*anyopaque, @ptrCast(context)));~N");
}

static void generate_setup_context(struct generator * g) {
    w(g, "~Mvar context_val = Context{};~N");
    w(g, "~Mconst context = &context_val;~N");
    w(g, "~Msuppress_any_unused_warning(@as(*anyopaque, @ptrCast(context)));~N");
    for (struct name * q = g->analyser->names; q; q = q->next) {
        if (q->local_to != NULL || q->type != t_string) continue;
        w(g, "~Mdefer context.");
        write_varname(g, q);
        w(g, ".deinit(env.allocator);~N");
    }
}

static void generate_define(struct generator * g, struct node * p) {
    struct name * q = p->name;

    write_newline(g);
    write_comment(g, p);

    /* Generate the body into a buffer. */
    struct str * saved_output = g->outbuf;
    g->outbuf = str_new();

    g->next_label = 0;
    g->var_number = 0;

    str_clear(g->failure_str);
    g->failure_label = x_return;
    g->unreachable = false;
    int signals = p->left->possible_signals;

    int save_margin = g->margin;
    g->margin = 1;
    if (q->amongvar_needed) w(g, "~Mvar among_var: i32 = 0;~N");

    /* Declare local variables. */
    for (struct name * name = g->analyser->names; name; name = name->next) {
        if (name->local_to == q) {
            switch (name->type) {
                case t_string:
                    w(g, "~Mvar ");
                    write_varname(g, name);
                    w(g, ": snowball.String = .{};~N");
                    w(g, "~Mdefer ");
                    write_varname(g, name);
                    w(g, ".deinit(env.allocator);~N");
                    break;
                case t_integer:
                    w(g, "~Mvar ");
                    write_varname(g, name);
                    w(g, ": i32 = 0;~N");
                    break;
                case t_boolean:
                    w(g, "~Mvar ");
                    write_varname(g, name);
                    w(g, ": bool = false;~N");
                    break;
            }
        }
    }

    generate(g, p->left);
    if (p->left->right) {
        assert(p->left->right->type == c_functionend);
        if (signals) {
            generate(g, p->left->right);
        }
    }
    w(g, "~-~M}~N");

    struct str * body = g->outbuf;
    g->outbuf = saved_output;
    g->margin = save_margin;

    /* Emit the function header. */
    if (q->type == t_routine) {
        writef(g, "~Mfn ~W(env: *snowball.Env, ctx: *anyopaque) bool {~+~N", p);
        generate_cast_context(g);
    } else {
        writef(g, "~Mpub fn ~E(env: *snowball.Env) bool {~+~N", p);
        generate_setup_context(g);
        if (q->used != q->definition) {
            writef(g, "~Mreturn ~W(env, @as(*anyopaque, @ptrCast(context)));~N", p);
            w(g, "~-~M}~N~N");
            writef(g, "~Mfn ~W(env: *snowball.Env, ctx: *anyopaque) bool {~+~N", p);
            generate_cast_context(g);
        }
    }

    /* Append the pre-generated body. */
    str_append(g->outbuf, body);
    str_delete(body);
    g->margin = save_margin;
}

static void generate_functionend(struct generator * g, struct node * p) {
    (void)p;
    w(g, "~Mreturn true;~N");
}

static void generate_substring(struct generator * g, struct node * p) {
    write_comment(g, p);

    struct among * x = p->among;

    g->S[0] = p->mode == m_forward ? "" : "B";
    g->I[0] = x->number;

    if (x->amongvar_needed) {
        writef(g, "~Mamong_var = env.findAmong~S0(&a_~I0, @as(*anyopaque, @ptrCast(context)));~N", p);
        if (!x->always_matches) {
            write_failure_if(g, "among_var == 0", p);
        }
    } else if (x->always_matches) {
        writef(g, "~M_ = env.findAmong~S0(&a_~I0, @as(*anyopaque, @ptrCast(context)));~N", p);
    } else if (x->command_count == 0 && tailcallable(g, p)) {
        writef(g, "~Mreturn env.findAmong~S0(&a_~I0, @as(*anyopaque, @ptrCast(context))) != 0;~N", p);
        x->node->right = NULL;
        g->unreachable = true;
    } else {
        write_failure_if(g, "env.findAmong~S0(&a_~I0, @as(*anyopaque, @ptrCast(context))) == 0", p);
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
        generate(g, x->commands[0]);
    } else if (x->command_count > 0) {
        w(g, "~Mswitch (among_var) {~N~+");
        for (int i = 1; i <= x->command_count; i++) {
            g->I[0] = i;
            w(g, "~M~I0 => {~N~+");
            generate(g, x->commands[i - 1]);
            w(g, "~-~M},~N");
            g->unreachable = false;
        }
        w(g, "~Melse => {},~N");
        w(g, "~-~M}~N");
    }
}

static void generate_booltest(struct generator * g, struct node * p, int inverted) {
    write_comment(g, p);
    if (tailcallable(g, p)) {
        if (inverted) {
            writef(g, "~Mreturn !~V;~N", p);
        } else {
            writef(g, "~Mreturn ~V;~N", p);
        }
        p->right = NULL;
        g->unreachable = true;
        return;
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
        case c_divideassign:
            write_comment(g, p);
            writef(g, "~M~V = @divTrunc(~V, ", p);
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
    w(g, "const snowball = @import(\"env.zig\");~N~N"
         "fn suppress_any_unused_warning(ctx: *anyopaque) void {~N~+"
         "~M_ = ctx;~N"
         "~-~M}~N~N");
}

static void generate_among_table(struct generator * g, struct among * x) {
    write_comment(g, x->node);

    struct amongvec * v = x->b;

    g->I[0] = x->number;
    w(g, "~Mconst a_~I0 = [_]snowball.Among{~N~+");

    for (int i = 0; i < x->literalstring_count; i++) {
        g->I[0] = v[i].i;
        g->I[1] = v[i].result;

        w(g, "~Msnowball.Among{ .s = ");
        write_literal_string(g, v[i].b);
        w(g, ", .substring_i = ~I0, .result = ~I1, .method = ");

        if (v[i].function != NULL) {
            write_varname(g, v[i].function);
        } else {
            w(g, "null");
        }
        w(g, " },~N");
    }
    w(g, "~-~M};~N~N");
}

static void generate_amongs(struct generator * g) {
    for (struct among * x = g->analyser->amongs; x; x = x->next) {
        generate_among_table(g, x);
    }
}

static void set_bit(symbol * b, int i) { b[i/8] |= 1 << i%8; }

static void generate_grouping_table(struct generator * g, struct grouping * q) {
    int range = q->largest_ch - q->smallest_ch + 1;
    int size = (range + 7)/ 8;
    symbol * b = q->b;
    symbol * map = create_b(size);

    for (int i = 0; i < size; i++) map[i] = 0;

    for (int i = 0; i < SIZE(b); i++) set_bit(map, b[i] - q->smallest_ch);

    w(g, "~Mconst ");
    write_varname(g, q->name);
    w(g, " = [_]u8{");
    for (int i = 0; i < size; i++) {
        if (i) w(g, ", ");
        write_int(g, map[i]);
    }
    w(g, "};~N~N");

    lose_b(map);
}

static void generate_groupings(struct generator * g) {
    for (struct grouping * q = g->analyser->groupings; q; q = q->next) {
        generate_grouping_table(g, q);
    }
}

static void generate_members(struct generator * g) {
    w(g, "const Context = struct {~+~N");
    for (struct name * q = g->analyser->names; q; q = q->next) {
        if (q->local_to) continue;
        switch (q->type) {
            case t_string:
                write_margin(g);
                write_varname(g, q);
                w(g, ": snowball.String = .{},~N");
                break;
            case t_integer:
                write_margin(g);
                write_varname(g, q);
                w(g, ": i32 = 0,~N");
                break;
            case t_boolean:
                write_margin(g);
                write_varname(g, q);
                w(g, ": bool = false,~N");
                break;
        }
    }
    w(g, "~-};~N");
}

static void generate_methods(struct generator * g) {
    for (struct node * p = g->analyser->program; p; p = p->right) {
        generate(g, p);
        g->unreachable = false;
    }
}

extern void generate_program_zig(struct generator * g) {
    g->outbuf = str_new();
    g->failure_str = str_new();

    write_start_comment(g, "//! ", NULL);
    generate_class_begin(g);

    generate_amongs(g);
    generate_groupings(g);

    generate_members(g);
    generate_methods(g);

    output_str(g->options->output_src, g->outbuf);
    str_delete(g->failure_str);
    str_delete(g->outbuf);
}
