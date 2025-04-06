#include <assert.h>
#include <stdio.h>   /* for fprintf etc */
#include <stdlib.h>  /* for free etc */
#include <string.h>  /* for strlen */
#include "header.h"

/* Define this to get warning messages when optimisations can't be used. */
/* #define OPTIMISATION_WARNINGS */

/* recursive use: */

static void generate(struct generator * g, struct node * p);
static void w(struct generator * g, const char * s);

static int new_label(struct generator * g) {
    return g->next_label++;
}

/* Write routines for simple entities */

static void wi3(struct generator * g, int i) {
    if (i < 100) write_char(g, ' ');
    if (i < 10)  write_char(g, ' ');
    write_int(g, i); /* integer (width 3) */
}

/* Write routines for items from the syntax tree */

static void write_varname(struct generator * g, struct name * p) {
    int ch = "SIIrxg"[p->type];
    switch (p->type) {
        case t_external:
            write_string(g, g->options->externals_prefix); break;
        case t_string:
        case t_boolean:
        case t_integer: {
            int count = p->count;
            if (count < 0) {
                p->s[SIZE(p->s)] = 0;
                fprintf(stderr, "Reference to optimised out variable %s attempted\n",
                        p->s);
                exit(1);
            }
            if (p->type == t_boolean) {
                /* We use a single array for booleans and integers, with the
                 * integers first.
                 */
                count += g->analyser->name_count[t_integer];
            }
            write_char(g, ch);
            write_char(g, '[');
            write_int(g, count);
            write_char(g, ']');
            return;
        }
        default:
            write_char(g, ch); write_char(g, '_');
    }
    write_s(g, p->s);
}

static void write_varref(struct generator * g, struct name * p) {  /* reference to variable */
    if (p->type < t_routine) write_string(g, "z->");
    write_varname(g, p);
}

static void write_hexdigit(struct generator * g, int i) {
    str_append_ch(g->outbuf, "0123456789ABCDEF"[i & 0xF]); /* hexchar */
}

extern void write_hex4(struct generator * g, int ch) {
    int i;
    for (i = 12; i >= 0; i -= 4) write_hexdigit(g, ch >> i);
}

static void write_hex(struct generator * g, int i) {
    if (i >> 4) write_hex(g, i >> 4);
    write_hexdigit(g, i); /* hex integer */
}

/* write character literal */
static void wlitch(struct generator * g, int ch) {
    if (32 <= ch && ch < 127) {
        write_char(g, '\'');
        if (ch == '\'' || ch == '\\') {
            write_char(g, '\\');
        }
        write_char(g, ch);
        write_char(g, '\'');
    } else {
        write_string(g, "0x"); write_hex(g, ch);
    }
}

static void wlitarray(struct generator * g, symbol * p) {  /* write literal array */
    write_string(g, "{ ");
    {
        int i;
        for (i = 0; i < SIZE(p); i++) {
            wlitch(g, p[i]);
            if (i < SIZE(p) - 1) write_string(g, ", ");
        }
    }
    write_string(g, " }");
}

static void wlitref(struct generator * g, symbol * p) {  /* write ref to literal array */
    if (SIZE(p) == 0) {
        write_char(g, '0');
    } else {
        struct str * s = g->outbuf;
        g->outbuf = g->declarations;
        write_string(g, "static const symbol s_"); write_int(g, g->literalstring_count); write_string(g, "[] = ");
        wlitarray(g, p);
        write_string(g, ";\n");
        g->outbuf = s;
        write_string(g, "s_"); write_int(g, g->literalstring_count);
        g->literalstring_count++;
    }
}

static void write_margin(struct generator * g) {
    int i;
    for (i = 0; i < g->margin; i++) write_string(g, "    ");
}

extern void write_c_relop(struct generator * g, int relop) {
    switch (relop) {
        case c_eq: write_string(g, " == "); break;
        case c_ne: write_string(g, " != "); break;
        case c_gt: write_string(g, " > "); break;
        case c_ge: write_string(g, " >= "); break;
        case c_lt: write_string(g, " < "); break;
        case c_le: write_string(g, " <= "); break;
        default:
            fprintf(stderr, "Unexpected type #%d in generate_integer_test\n", relop);
            exit(1);
    }
}

void write_comment_content(struct generator * g, struct node * p) {
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
            write_string(g, " <integer expression>");
            break;
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
            write_string(g, "$(<integer expression> ");
            write_string(g, name_of_token(p->type));
            write_string(g, " <integer expression>)");
            break;
        case c_define:
            if (p->mode == m_forward) {
                write_string(g, "forwardmode ");
            } else {
                write_string(g, "backwardmode ");
            }
            /* FALLTHRU */
        default:
            write_string(g, name_of_token(p->type));
            if (p->name) {
                write_char(g, ' ');
                write_s(g, p->name->s);
            }
    }
    write_string(g, ", line ");
    write_int(g, p->line_number);
}

static void write_comment(struct generator * g, struct node * p) {
    if (!g->options->comments) return;
    write_margin(g);
    write_string(g, "/* ");
    write_comment_content(g, p);
    write_string(g, " */");
    write_newline(g);
}

/* margin + string */
static void wms(struct generator * g, const char * s) {
    write_margin(g); write_string(g, s);
}

static void write_block_start(struct generator * g) {
    w(g, "~M{~+~N");
}

static void write_block_end(struct generator * g) {
    if (g->line_labelled == g->line_count) {
        // Before C23, `;` is required between a label and the block end.
        w(g, "~M;~N");
    }
    w(g, "~-~M}~N");
}

static void w(struct generator * g, const char * s);

/* keep c */
static void wk(struct generator * g, struct node * p, int keep_limit) {
    ++g->keep_count;
    if (p->mode == m_forward) {
        write_string(g, "int c");
        write_int(g, g->keep_count);
        write_string(g, " = z->c");
        if (keep_limit) {
            write_string(g, ", mlimit");
            write_int(g, g->keep_count);
        }
        write_char(g, ';');
    } else {
        write_string(g, "int m");
        write_int(g, g->keep_count);
        write_string(g, " = z->l - z->c");
        if (keep_limit) {
            write_string(g, ", mlimit");
            write_int(g, g->keep_count);
        }
        write_string(g, "; (void)m");
        write_int(g, g->keep_count);
        write_char(g, ';');
    }
}

static void wrestore(struct generator * g, struct node * p, int keep_token) {     /* restore c */
    if (p->mode == m_forward) {
        write_string(g, "z->c = c");
    } else {
        write_string(g, "z->c = z->l - m");
    }
    write_int(g, keep_token); write_char(g, ';');
}

static void wrestorelimit(struct generator * g, struct node * p, int keep_token) {     /* restore limit */
    if (p->mode == m_forward) {
        w(g, "z->l += mlimit");
    } else {
        w(g, "z->lb = mlimit");
    }
    write_int(g, keep_token); write_string(g, ";");
}

static void winc(struct generator * g, struct node * p) {     /* increment c */
    write_string(g, p->mode == m_forward ? "z->c++;" :
                                 "z->c--;");
}

static void wsetl(struct generator * g, int n) {
    g->margin--;
    wms(g, "lab"); write_int(g, n); write_char(g, ':'); write_newline(g);
    g->line_labelled = g->line_count;
    g->margin++;
}

static void wgotol(struct generator * g, int n) {
    wms(g, "goto lab"); write_int(g, n); write_char(g, ';'); write_newline(g);
}

static void write_failure(struct generator * g, struct node * p) {          /* fail */
    if (g->failure_keep_count != 0) {
        write_string(g, "{ ");
        if (g->failure_keep_count > 0) {
            wrestore(g, p, g->failure_keep_count);
        } else {
            wrestorelimit(g, p, -g->failure_keep_count);
        }
        write_char(g, ' ');
    }
    switch (g->failure_label) {
        case x_return:
            write_string(g, "return 0;");
            break;
        default:
            write_string(g, "goto lab");
            write_int(g, g->failure_label);
            write_char(g, ';');
            g->label_used = 1;
    }
    if (g->failure_keep_count != 0) write_string(g, " }");
}


/* if at limit fail */
static void write_check_limit(struct generator * g, struct node * p) {
    write_string(g, p->mode == m_forward ? "if (z->c >= z->l) " :
                                 "if (z->c <= z->lb) ");
    write_failure(g, p);
}

static void write_data_address(struct generator * g, struct node * p) {
    symbol * b = p->literalstring;
    if (b != NULL) {
        write_int(g, SIZE(b)); w(g, ", ");
        wlitref(g, b);
    } else {
        write_varref(g, p->name);
    }
}

/* Formatted write. */
static void writef(struct generator * g, const char * input, struct node * p) {
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
            case 'k': wk(g, p, false); continue;
            case 'K': wk(g, p, true); continue;
            case 'i': winc(g, p); continue;
            case 'l': write_check_limit(g, p); continue;
            case 'f': write_failure(g, p); continue;
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
            case 'I':
            case 'J':
            case 'c': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->I) / sizeof(g->I[0])))
                    goto invalid_escape2;
                if (ch == 'I')
                    write_int(g, g->I[j]);
                else if (ch == 'J')
                    wi3(g, g->I[j]);
                else
                    wlitch(g, g->I[j]);
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
            case 'L':
            case 'A': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->L) / sizeof(g->L[0])))
                    goto invalid_escape2;
                if (ch == 'L')
                    wlitref(g, g->L[j]);
                else
                    wlitarray(g, g->L[j]);
                continue;
            }
            case 'a': write_data_address(g, p); continue;
            case '+': g->margin++; continue;
            case '-': g->margin--; continue;
            case '$': /* insert_s, insert_v etc */
                write_char(g, p->literalstring == NULL ? 'v' : 's');
                continue;
            case 'p': write_string(g, g->options->externals_prefix); continue;
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
            write_string(g, "MAXINT"); break;
        case c_minint:
            write_string(g, "MININT"); break;
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
            w(g, "z->c"); break;
        case c_limit:
            w(g, p->mode == m_forward ? "z->l" : "z->lb"); break;
        case c_len:
            if (g->options->encoding == ENC_UTF8) {
                w(g, "len_utf8(z->p)");
                break;
            }
            /* FALLTHRU */
        case c_size:
            w(g, "SIZE(z->p)");
            break;
        case c_lenof:
            if (g->options->encoding == ENC_UTF8) {
                g->V[0] = p->name;
                w(g, "len_utf8(~V0)");
                break;
            }
            /* FALLTHRU */
        case c_sizeof:
            g->V[0] = p->name;
            w(g, "SIZE(~V0)");
            break;
    }
}

// Return 0 for always f.
// Return 1 for always t.
// Return -1 for don't know (or can raise t or f).
static int check_possible_signals(struct generator * g,
                                  struct node * p, int call_depth) {
    switch (p->type) {
        case c_fail:
        case c_false:
            /* Always gives signal f. */
            return 0;
        case c_assign:
        case c_attach:
        case c_debug:
        case c_delete:
        case c_do:
        case c_insert:
        case c_leftslice:
        case c_repeat:
        case c_rightslice:
        case c_set:
        case c_setmark:
        case c_slicefrom:
        case c_sliceto:
        case c_tolimit:
        case c_tomark:
        case c_true:
        case c_try:
        case c_unset:
        case c_mathassign:
        case c_plusassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_divideassign:
        case c_functionend:
            /* Always gives signal t. */
            return 1;
        case c_not: {
            int res = check_possible_signals(g, p->left, call_depth);
            if (res >= 0)
                res = !res;
            if (res == 0 && p->right) {
                if (p->right->type != c_functionend) {
                    fprintf(stderr, "%s:%d: warning: 'not' always signals f so following commands are unreachable\n",
                            g->analyser->tokeniser->file, p->line_number);
                }
                p->right = NULL;
            }
            return res;
        }
        case c_setlimit: {
            /* If p->left signals f, setlimit does. */
            int res = check_possible_signals(g, p->left, call_depth);
            if (res == 0) {
                return res;
            }
            /* Otherwise gives same signal as p->aux. */
            int res2 = check_possible_signals(g, p->aux, call_depth);
            if (res2 <= 0)
                return res2;
            return res;
        }
        case c_and:
        case c_bra:
            /* Gives same signal as list p->left. */
            return check_possible_signals_list(g, p->left, p->type, call_depth);
        case c_atleast:
        case c_backwards:
        case c_loop:
        case c_reverse:
        case c_test:
            /* Give same signal as p->left. */
            return check_possible_signals(g, p->left, call_depth);
        case c_call:
            if (call_depth >= 100) {
                /* Recursive functions aren't typical in snowball programs,
                 * so make the pessimistic assumption that both t and f are
                 * possible if we hit a generous limit on recursion.  It's
                 * not likely to make a difference to any real world
                 * program, but means we won't recurse until we run out of
                 * stack for pathological cases.
                 */
                return -1;
            }
            return check_possible_signals_list(g, p->name->definition, c_define,
                                               call_depth + 1);
        case c_gopast:
        case c_goto:
        case c_goto_grouping:
        case c_gopast_grouping:
        case c_goto_non:
        case c_gopast_non:
            /* FIXME: unless we can prove that c is either definitely atlimit
             * or definitely not atlimit... */
            return -1;
        case c_atlimit:
        case c_atmark:
        case c_booltest:
        case c_hop:
        case c_literalstring:
        case c_next:
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
        case c_grouping:
        case c_non:
        case c_name:
            /* FIXME: unless we can prove... */
            return -1;
        case c_substring: {
            struct among * x = p->among;
            if (x->always_matches) {
                return 1;
            }
            return -1;
        }
        case c_among: {
            struct among * x = p->among;
            int r = 1;

            if (x->substring == NULL) {
                if (x->always_matches) {
                    return 1;
                }
                r = -1;
            }

            if (x->command_count > 0) {
                int trues = (x->nocommand_count > 0);
                int falses = false;
                int i;
                for (i = 1; i <= x->command_count; i++) {
                    int res = check_possible_signals(g, x->commands[i - 1],
                                                     call_depth);
                    if (res == 0) {
                        falses = true;
                    } else if (res > 0) {
                        trues = true;
                    } else {
                        falses = trues = true;
                    }
                    if (falses && trues) break;
                }
                if (!trues) {
                    // All commands in among always fail.
                    return 0;
                }
                if (falses) {
                    // Commands in among can succeed or fail.
                    return -1;
                }
            }
            return r;
        }
        case c_or: {
            struct node * q;
            int r = 0;
            for (q = p->left; q; q = q->right) {
                // Just check this node - q->right is a separate clause of
                // the OR.
                int res = check_possible_signals(g, q, call_depth);
                if (res > 0) {
                    // If any clause of the OR always signals t, then the OR
                    // always signals t.
                    if (q->right) {
                        if (q->right->type != c_functionend) {
                            fprintf(stderr, "%s:%d: warning: command always signals t here so rest of 'or' is unreachable\n",
                                    g->analyser->tokeniser->file,
                                    q->line_number);
                        }
                        q->right = NULL;
                    }
                    return 1;
                }
                if (res < 0) {
                    r = res;
                }
            }
            return r;
        }
        default:
            return -1;
    }
}

// Return 0 for always f.
// Return 1 for always t.
// Return -1 for don't know (or can raise t or f).
int check_possible_signals_list(struct generator * g, struct node * p,
                                int type, int call_depth) {
    int r = 1;
    while (p) {
        int res = check_possible_signals(g, p, call_depth);
        if (res == 0) {
            // If any command always signals f, then the list always signals f.
            if (p->right) {
                if (p->right->type != c_functionend) {
                    fprintf(stderr, "%s:%d: warning: command always signals f here so rest of %s is unreachable\n",
                            g->analyser->tokeniser->file, p->line_number,
                            (type == c_and ? "'and'" : "command list"));
                }
                p->right = NULL;
            }
            return res;
        }
        if (res < 0) r = res;
        p = p->right;
    }
    return r;
}

/* K_needed() tests to see if we really need to keep c. Not true when the
   command does not touch the cursor. This and repeat_score() could be
   elaborated almost indefinitely.
*/

static int K_needed_(struct node * p, int call_depth) {
    while (p) {
        switch (p->type) {
            case c_atlimit:
            case c_do:
            case c_dollar:
            case c_leftslice:
            case c_rightslice:
            case c_mathassign:
            case c_plusassign:
            case c_minusassign:
            case c_multiplyassign:
            case c_divideassign:
            case c_eq:
            case c_ne:
            case c_gt:
            case c_ge:
            case c_lt:
            case c_le:
            case c_sliceto:
            case c_booltest:
            case c_set:
            case c_unset:
            case c_true:
            case c_false:
            case c_debug:
            case c_functionend:
            case c_not:
                break;

            case c_call:
                /* Recursive functions aren't typical in snowball programs, so
                 * make the pessimistic assumption that keep is needed if we
                 * hit a generous limit on recursion.  It's not likely to make
                 * a difference to any real world program, but means we won't
                 * recurse until we run out of stack for pathological cases.
                 */
                if (call_depth >= 100) return true;
                if (K_needed_(p->name->definition, call_depth + 1))
                    return true;
                break;

            case c_bra:
                if (K_needed_(p->left, call_depth)) return true;
                break;

            default: return true;
        }
        p = p->right;
    }
    return false;
}

extern int K_needed(struct generator * g, struct node * p) {
    (void)g;
    return K_needed_(p, 0);
}

static int repeat_score(struct generator * g, struct node * p, int call_depth) {
    int score = 0;
    while (p) {
        switch (p->type) {
            case c_dollar:
            case c_leftslice:
            case c_rightslice:
            case c_mathassign:
            case c_plusassign:
            case c_minusassign:
            case c_multiplyassign:
            case c_divideassign:
            case c_eq:
            case c_ne:
            case c_gt:
            case c_ge:
            case c_lt:
            case c_le:
            case c_sliceto:   /* case c_not: must not be included here! */
            case c_debug:
            case c_functionend:
                break;

            case c_call:
                /* Recursive functions aren't typical in snowball programs, so
                 * make the pessimistic assumption that repeat requires cursor
                 * reinstatement if we hit a generous limit on recursion.  It's
                 * not likely to make a difference to any real world program,
                 * but means we won't recurse until we run out of stack for
                 * pathological cases.
                 */
                if (call_depth >= 100) {
                    return 2;
                }
                score += repeat_score(g, p->name->definition, call_depth + 1);
                if (score >= 2)
                    return score;
                break;

            case c_bra:
                score += repeat_score(g, p->left, call_depth);
                if (score >= 2)
                    return score;
                break;

            case c_name:
            case c_literalstring:
            case c_next:
            case c_grouping:
            case c_non:
            case c_goto_grouping:
            case c_gopast_grouping:
            case c_goto_non:
            case c_gopast_non:
            case c_hop:
                if (++score >= 2)
                    return score;
                break;

            default:
                return 2;
        }
        p = p->right;
    }
    return score;
}

/* tests if an expression requires cursor reinstatement in a repeat */

extern int repeat_restore(struct generator * g, struct node * p) {
    return repeat_score(g, p, 0) >= 2;
}

static void generate_bra(struct generator * g, struct node * p) {
    p = p->left;
    while (p) {
        generate(g, p);
        p = p->right;
    }
}

static void generate_and(struct generator * g, struct node * p) {
    int keep_c = 0;
    write_comment(g, p);
    if (K_needed(g, p->left)) {
        writef(g, "~{~M~k~N", p);
        keep_c = g->keep_count;
    }
    p = p->left;
    while (p) {
        generate(g, p);
        if (keep_c && p->right != NULL) {
            w(g, "~M"); wrestore(g, p, keep_c); w(g, "~N");
        }
        p = p->right;
    }
    if (keep_c) w(g, "~}");
}

static void generate_or(struct generator * g, struct node * p) {
    int keep_c = 0;

    int used = g->label_used;
    int a0 = g->failure_label;
    int a1 = g->failure_keep_count;

    int out_lab = new_label(g);
    write_comment(g, p);

    if (K_needed(g, p->left)) {
        writef(g, "~{~M~k~N", p);
        keep_c = g->keep_count;
    }
    p = p->left;
    g->failure_keep_count = 0;
    while (p->right) {
        g->failure_label = new_label(g);
        g->label_used = 0;
        generate(g, p);
        wgotol(g, out_lab);
        if (g->label_used)
            wsetl(g, g->failure_label);
        if (keep_c) {
            w(g, "~M"); wrestore(g, p, keep_c); w(g, "~N");
        }
        p = p->right;
    }
    g->label_used = used;
    g->failure_label = a0;
    g->failure_keep_count = a1;

    generate(g, p);
    if (keep_c) w(g, "~}");
    wsetl(g, out_lab);
}

static void generate_backwards(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~Mz->lb = z->c; z->c = z->l;~N", p);
    generate(g, p->left);
    w(g, "~Mz->c = z->lb;~N");
}


static void generate_not(struct generator * g, struct node * p) {
    int keep_c = 0;

    int used = g->label_used;
    int a0 = g->failure_label;
    int a1 = g->failure_keep_count;

    write_comment(g, p);
    if (K_needed(g, p->left)) {
        writef(g, "~{~M~k~N", p);
        keep_c = g->keep_count;
    }

    g->failure_label = new_label(g);
    g->label_used = 0;
    g->failure_keep_count = 0;
    generate(g, p->left);

    {
        int l = g->failure_label;
        int u = g->label_used;

        g->label_used = used;
        g->failure_label = a0;
        g->failure_keep_count = a1;

        writef(g, "~M~f~N", p);
        if (u)
            wsetl(g, l);
    }
    if (keep_c) {
        w(g, "~M"); wrestore(g, p, keep_c); w(g, "~N~}");
    }
}


static void generate_try(struct generator * g, struct node * p) {
    write_comment(g, p);
    int keep_c = 0;
    if (K_needed(g, p->left)) {
        writef(g, "~{~M~k~N", p);
        keep_c = g->keep_count;
    }
    g->failure_keep_count = keep_c;

    g->failure_label = new_label(g);
    g->label_used = 0;
    generate(g, p->left);

    if (g->label_used)
        wsetl(g, g->failure_label);

    if (keep_c) w(g, "~}");
}

static void generate_set(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = 1;~N", p);
}

static void generate_unset(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = 0;~N", p);
}

static void generate_fail(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate(g, p->left);
    writef(g, "~M~f~N", p);
}

/* generate_test() also implements 'reverse' */

static void generate_test(struct generator * g, struct node * p) {
    write_comment(g, p);

    int keep_c = 0;
    if (K_needed(g, p->left)) {
        keep_c = ++g->keep_count;
        w(g, p->mode == m_forward ? "~{~Mint c_test" :
                                    "~{~Mint m_test");
        write_int(g, keep_c);
        w(g, p->mode == m_forward ? " = z->c;~N" :
                                    " = z->l - z->c;~N");
    }

    generate(g, p->left);

    if (keep_c) {
        w(g, p->mode == m_forward ? "~Mz->c = c_test" :
                                    "~Mz->c = z->l - m_test");
        write_int(g, keep_c);
        writef(g, ";~N~}", p);
    }
}

static void generate_do(struct generator * g, struct node * p) {
    write_comment(g, p);
    int keep_c = 0;
    if (K_needed(g, p->left)) {
        writef(g, "~{~M~k~N", p);
        keep_c = g->keep_count;
    }

    if (p->left->type == c_call) {
        /* Optimise do <call> */
        write_comment(g, p->left);
        g->V[0] = p->left->name;
        writef(g, "~{~Mint ret = ~V0(z);~N", p->left);
        w(g, "~Mif (ret < 0) return ret;~N~}");
    } else {
        g->failure_label = new_label(g);
        g->label_used = 0;
        g->failure_keep_count = 0;
        generate(g, p->left);

        if (g->label_used)
            wsetl(g, g->failure_label);
    }
    if (keep_c) {
        w(g, "~M"); wrestore(g, p, keep_c);
        w(g, "~N~}");
    }
}

static void generate_next(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (g->options->encoding == ENC_UTF8) {
        if (p->mode == m_forward)
            w(g, "~{~Mint ret = skip_utf8(z->p, z->c, z->l, 1");
        else
            w(g, "~{~Mint ret = skip_b_utf8(z->p, z->c, z->lb, 1");
        writef(g, ");~N"
              "~Mif (ret < 0) ~f~N"
              "~Mz->c = ret;~N"
              "~}", p);
    } else
        writef(g, "~M~l~N"
              "~M~i~N", p);
}

static void generate_GO_grouping(struct generator * g, struct node * p, int is_goto, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->S[1] = complement ? "in" : "out";
    g->S[2] = g->options->encoding == ENC_UTF8 ? "_U" : "";
    g->V[0] = p->name;
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    if (is_goto) {
        writef(g, "~Mif (~S1_grouping~S0~S2(z, ~V0, ~I0, ~I1, 1) < 0) ~f~N", p);
    } else {
        writef(g, "~{"
              "~Mint ret = ~S1_grouping~S0~S2(z, ~V0, ~I0, ~I1, 1);~N"
              "~Mif (ret < 0) ~f~N", p);
        if (p->mode == m_forward)
            w(g, "~Mz->c += ret;~N");
        else
            w(g, "~Mz->c -= ret;~N");
        w(g, "~}");
    }
}

static void generate_GO(struct generator * g, struct node * p, int style) {
    write_comment(g, p);
    int keep_c = 0;

    int used = g->label_used;
    int a0 = g->failure_label;
    int a1 = g->failure_keep_count;

    w(g, "~Mwhile(1) ~{");

    if (style == 1 || repeat_restore(g, p->left)) {
        writef(g, "~M~k~N", p);
        keep_c = g->keep_count;
    }

    g->failure_label = new_label(g);
    g->label_used = 0;
    g->failure_keep_count = 0;
    generate(g, p->left);

    if (style == 1) {
        /* include for goto; omit for gopast */
        w(g, "~M"); wrestore(g, p, keep_c); w(g, "~N");
    }
    w(g, "~Mbreak;~N");
    if (g->label_used)
        wsetl(g, g->failure_label);
    if (keep_c) {
        w(g, "~M"); wrestore(g, p, keep_c); w(g, "~N");
    }

    g->label_used = used;
    g->failure_label = a0;
    g->failure_keep_count = a1;

    generate_next(g, p);
    w(g, "~}");
}

static void generate_loop(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~{~Mint i; for (i = ");
    generate_AE(g, p->AE);
    writef(g, "; i > 0; i--) ~{", p);

    generate(g, p->left);

    w(g,    "~}"
         "~}");
}

static void generate_repeat_or_atleast(struct generator * g, struct node * p, int atleast_case) {
    int keep_c = 0;
    writef(g, "~Mwhile(1) {~+~N", p);

    if (repeat_restore(g, p->left)) {
        writef(g, "~M~k~N", p);
        keep_c = g->keep_count;
    }

    g->failure_label = new_label(g);
    g->label_used = 0;
    g->failure_keep_count = 0;

    int possible_signals = check_possible_signals_list(g, p->left, p->type, 0);
    if (possible_signals != -1) {
        fprintf(stderr, "%s:%d: warning: body of '%s' always signals '%c'\n",
                g->analyser->tokeniser->file, p->line_number,
                atleast_case ? "atleast" : "repeat", possible_signals ? 't' : 'f');
    }
    generate(g, p->left);

    if (atleast_case) w(g, "~Mi--;~N");

    w(g, "~Mcontinue;~N");
    if (g->label_used)
        wsetl(g, g->failure_label);

    if (keep_c) {
        w(g, "~M"); wrestore(g, p, keep_c); w(g, "~N");
    }

    w(g, "~Mbreak;~N"
      "~}");
}

static void generate_repeat(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate_repeat_or_atleast(g, p, false);
}

static void generate_atleast(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~{~Mint i = "); generate_AE(g, p->AE); writef(g, ";~N", p);
    {
        int used = g->label_used;
        int a0 = g->failure_label;
        int a1 = g->failure_keep_count;

        generate_repeat_or_atleast(g, p, true);

        g->label_used = used;
        g->failure_label = a0;
        g->failure_keep_count = a1;
    }
    writef(g, "~Mif (i > 0) ~f~N"
       "~}", p);
}

static void generate_setmark(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = z->c;~N", p);
}

static void generate_tomark(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? ">" : "<";

    w(g, "~Mif (z->c ~S0 "); generate_AE(g, p->AE); writef(g, ") ~f~N", p);
    w(g, "~Mz->c = "); generate_AE(g, p->AE); writef(g, ";~N", p);
}

static void generate_atmark(struct generator * g, struct node * p) {
    write_comment(g, p);
    w(g, "~Mif (z->c != "); generate_AE(g, p->AE); writef(g, ") ~f~N", p);
}

static void generate_hop(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (g->options->encoding == ENC_UTF8) {
        g->S[0] = p->mode == m_forward ? "" : "_b";
        g->S[1] = p->mode == m_forward ? "z->l" : "z->lb";
        w(g, "~{~Mint ret = skip~S0_utf8(z->p, z->c, ~S1, ");
        generate_AE(g, p->AE);
        writef(g, ");~N", p);
        writef(g, "~Mif (ret < 0) ~f~N", p);
        writef(g, "~Mz->c = ret;~N"
               "~}", p);
    } else {
        // Fixed-width characters.
        g->S[0] = p->mode == m_forward ? "+" : "-";
        if (p->AE->type == c_number) {
            // Constant distance hop.
            //
            // No need to check for negative hop as that's converted to false by
            // the analyser.
            //
            // Note that if we signal f then z->c will be reset when this is
            // handled - we rely on this here and unconditionally update z->c.
            w(g, "z->c = z->c ~S0 ");
            generate_AE(g, p->AE);
            writef(g, ";~N", p);
            if (p->mode == m_forward) {
                writef(g, "~Mif (z->c > z->l) ~f~N", p);
            } else {
                writef(g, "~Mif (z->c < z->lb) ~f~N", p);
            }
        } else {
            w(g, "~{~Mint ret = z->c ~S0 ");
            generate_AE(g, p->AE);
            writef(g, ";~N", p);
            if (p->mode == m_forward) {
                writef(g, "~Mif (ret > z->l || ret < z->c) ~f~N", p);
            } else {
                writef(g, "~Mif (ret < z->lb || ret > z->c) ~f~N", p);
            }
            writef(g, "~Mz->c = ret;~N"
                      "~}", p);
        }
    }
}

static void generate_delete(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~{~Mint ret = slice_del(z);~N", p);
    writef(g, "~Mif (ret < 0) return ret;~N"
          "~}", p);
}

static void generate_tolimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "b";
    writef(g, "~Mz->c = z->l~S0;~N", p);
}

static void generate_atlimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "b";
    g->S[1] = p->mode == m_forward ? "<" : ">";
    writef(g, "~Mif (z->c ~S1 z->l~S0) ~f~N", p);
}

static void generate_leftslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "bra" : "ket";
    writef(g, "~Mz->~S0 = z->c;~N", p);
}

static void generate_rightslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "ket" : "bra";
    writef(g, "~Mz->~S0 = z->c;~N", p);
}

static void generate_assignto(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = assign_to(z, ~V0);~N"
          "~Mif (~V0 == 0) return -1;~N", p);
}

static void generate_sliceto(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~M~V0 = slice_to(z, ~V0);~N"
          "~Mif (~V0 == 0) return -1;~N", p);
}

static void generate_insert(struct generator * g, struct node * p, int style) {
    int keep_c = style == c_attach;
    write_comment(g, p);
    if (p->mode == m_backward) keep_c = !keep_c;
    writef(g, "~{", p);
    if (keep_c) w(g, "~Mint saved_c = z->c;~N");
    writef(g, "~Mint ret = insert_~$(z, z->c, z->c, ~a);~N", p);
    if (keep_c) w(g, "~Mz->c = saved_c;~N");
    writef(g, "~Mif (ret < 0) return ret;~N"
          "~}", p);
}

static void generate_assignfrom(struct generator * g, struct node * p) {
    int keep_c = p->mode == m_forward; /* like 'attach' */
    write_comment(g, p);
    writef(g, "~{", p);
    if (keep_c) w(g, "~Mint saved_c = z->c;~N");
    w(g, "~Mint ret = ");
    writef(g, keep_c ? "insert_~$(z, z->c, z->l, ~a);~N" : "insert_~$(z, z->lb, z->c, ~a);~N", p);
    if (keep_c) w(g, "~Mz->c = saved_c;~N");
    writef(g, "~Mif (ret < 0) return ret;~N"
          "~}", p);
}

static void generate_slicefrom(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~{~Mint ret = slice_from_~$(z, ~a);~N", p);
    writef(g, "~Mif (ret < 0) return ret;~N"
          "~}", p);
}

static void generate_setlimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    int keep_c;
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
        assert(q->right == NULL);

        ++g->keep_count;
        writef(g, "~N~{~Mint mlimit", p);
        write_int(g, g->keep_count);
        writef(g, ";~N", p);
        keep_c = g->keep_count;

        g->S[0] = q->mode == m_forward ? ">" : "<";

        w(g, "~Mif (z->c ~S0 "); generate_AE(g, q->AE); writef(g, ") ~f~N", q);
        w(g, "~Mmlimit");
        write_int(g, keep_c);
        if (p->mode == m_forward) {
            w(g, " = z->l - z->c; z->l = ");
        } else {
            w(g, " = z->lb; z->lb = ");
        }
        generate_AE(g, q->AE);
        w(g, ";~N");
    } else {
        writef(g, "~{~M~K~N", p);
        keep_c = g->keep_count;
        generate(g, p->left);

        w(g, "~Mmlimit");
        write_int(g, keep_c);
        if (p->mode == m_forward)
            w(g, " = z->l - z->c; z->l = z->c;~N");
        else
            w(g, " = z->lb; z->lb = z->c;~N");
        w(g, "~M"); wrestore(g, p, keep_c); w(g, "~N");
    }

    g->failure_keep_count = -keep_c;
    generate(g, p->aux);
    w(g, "~M");
    wrestorelimit(g, p, -g->failure_keep_count);
    w(g, "~N"
      "~}");
}

/* dollar sets snowball up to operate on a string variable as if it were the
 * current string */
static void generate_dollar(struct generator * g, struct node * p) {
    write_comment(g, p);

    int used = g->label_used;
    int a0 = g->failure_label;
    int a1 = g->failure_keep_count;
    int keep_token;
    g->failure_label = new_label(g);
    g->label_used = 0;
    g->failure_keep_count = 0;

    keep_token = ++g->keep_count;
    g->I[0] = keep_token;
    writef(g, "~{~Mstruct SN_env env~I0 = * z;~N", p);
    g->V[0] = p->name;
    /* Assume failure. */
    writef(g, "~Mint failure = 1;~N"
          "~Mz->p = ~V0;~N"
          "~Mz->lb = z->c = 0;~N"
          "~Mz->l = SIZE(z->p);~N", p);
    generate(g, p->left);
    /* Mark success. */
    w(g, "~Mfailure = 0;~N");
    if (g->label_used)
        wsetl(g, g->failure_label);
    g->V[0] = p->name; /* necessary */

    g->label_used = used;
    g->failure_label = a0;
    g->failure_keep_count = a1;

    g->I[0] = keep_token;
    writef(g, "~M~V0 = z->p;~N"
          "~M* z = env~I0;~N"
          "~Mif (failure) ~f~N~}", p);
}

static void generate_integer_assign(struct generator * g, struct node * p, const char * s) {
    write_comment(g, p);
    g->V[0] = p->name;
    g->S[0] = s;
    w(g, "~M~V0 ~S0 "); generate_AE(g, p->AE); writef(g, ";~N", p);
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
    write_c_relop(g, relop);
    generate_AE(g, p->AE);
    if (optimise_to_return) {
        writef(g, ";~N", p);
    } else {
        writef(g, ") ~f~N", p);
    }
}

static void generate_call(struct generator * g, struct node * p) {
    int signals = check_possible_signals_list(g, p->name->definition, c_define, 0);
    write_comment(g, p);
    g->V[0] = p->name;
    if (g->failure_keep_count == 0 && g->failure_label == x_return &&
        (signals == 0 || (p->right && p->right->type == c_functionend))) {
        /* Always fails or tail call. */
        writef(g, "~Mreturn ~V0(z);~N", p);
        return;
    }
    writef(g, "~{~Mint ret = ~V0(z);~N", p);
    if (g->failure_keep_count == 0 && g->failure_label == x_return) {
        /* Combine the two tests in this special case for better optimisation
         * and clearer generated code. */
        writef(g, "~Mif (ret <= 0) return ret;~N", p);
    } else {
        if (signals == 1) {
            /* Always succeeds - just need to handle runtime errors. */
            writef(g, "~Mif (ret < 0) return ret;~N", p);
        } else if (signals == 0) {
            /* Always fails. */
            writef(g, "~Mif (ret < 0) return ret;~N", p);
            writef(g, "~M~f~N", p);
        } else {
            writef(g, "~Mif (ret == 0) ~f~N", p);
            writef(g, "~Mif (ret < 0) return ret;~N", p);
        }
    }
    writef(g, "~}", p);
}

static void generate_grouping(struct generator * g, struct node * p, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->S[1] = complement ? "out" : "in";
    g->S[2] = g->options->encoding == ENC_UTF8 ? "_U" : "";
    g->V[0] = p->name;
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    writef(g, "~Mif (~S1_grouping~S0~S2(z, ~V0, ~I0, ~I1, 0)) ~f~N", p);
}

static void generate_namedstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->V[0] = p->name;
    writef(g, "~Mif (!(eq_v~S0(z, ~V0))) ~f~N", p);
}

static void generate_literalstring(struct generator * g, struct node * p) {
    symbol * b = p->literalstring;
    write_comment(g, p);
    if (SIZE(b) == 1) {
        /* It's quite common to compare with a single character literal string,
         * so just inline the simpler code for this case rather than making a
         * function call.  In UTF-8 mode, only do this for the ASCII subset,
         * since multi-byte characters are more complex to test against.
         */
        if (g->options->encoding == ENC_UTF8 && *b >= 128) {
            printf("single byte %d\n", *b);
            exit(1);
        }
        g->I[0] = *b;
        if (p->mode == m_forward) {
            writef(g, "~Mif (z->c == z->l || z->p[z->c] != ~c0) ~f~N"
                  "~Mz->c++;~N", p);
        } else {
            writef(g, "~Mif (z->c <= z->lb || z->p[z->c - 1] != ~c0) ~f~N"
                  "~Mz->c--;~N", p);
        }
    } else {
        g->S[0] = p->mode == m_forward ? "" : "_b";
        g->I[0] = SIZE(b);
        g->L[0] = b;

        writef(g, "~Mif (!(eq_s~S0(z, ~I0, ~L0))) ~f~N", p);
    }
}

static void generate_define(struct generator * g, struct node * p) {
    struct name * q = p->name;
    if (q->type == t_routine && !q->used) return;

    write_newline(g);
    write_comment(g, p);

    g->next_label = 0;

    g->S[0] = q->type == t_routine ? "static" : "extern";
    g->V[0] = q;

    w(g, "~S0 int ~V0(struct SN_env * z) {");
    w(g, "~N~+");
    if (p->amongvar_needed) w(g, "~Mint among_var;~N");
    g->failure_keep_count = 0;
    g->failure_label = x_return;
    g->label_used = 0;
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
    w(g, "~}");
}

static void generate_functionend(struct generator * g, struct node * p) {
    (void)p;
    w(g, "~Mreturn 1;~N");
}

static void generate_substring(struct generator * g, struct node * p) {
    write_comment(g, p);

    struct among * x = p->among;
    int block = -1;
    unsigned int bitmap = 0;
    struct amongvec * among_cases = x->b;
    int c;
    int empty_case = -1;
    int n_cases = 0;
    symbol cases[2];
    int shortest_size = x->shortest_size;

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
    for (c = 0; c < x->literalstring_count; ++c) {
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
            sprintf(buf, "z->p[z->c + %d]", shortest_size - 1);
            g->S[1] = buf;
            if (shortest_size == 1) {
                writef(g, "~Mif (z->c >= z->l", p);
            } else {
                writef(g, "~Mif (z->c + ~I4 >= z->l", p);
            }
        } else {
            g->S[1] = "z->p[z->c - 1]";
            if (shortest_size == 1) {
                writef(g, "~Mif (z->c <= z->lb", p);
            } else {
                writef(g, "~Mif (z->c - ~I4 <= z->lb", p);
            }
        }
        if (n_cases == 0) {
            /* We get this for the degenerate case: among ( '' )
             * This doesn't seem to be a useful construct, but it is
             * syntactically valid.
             */
        } else if (n_cases == 1) {
            g->I[4] = cases[0];
            writef(g, " || ~S1 != ~I4", p);
        } else if (n_cases == 2) {
            g->I[4] = cases[0];
            g->I[5] = cases[1];
            writef(g, " || (~S1 != ~I4 && ~S1 != ~I5)", p);
        } else {
            writef(g, " || ~S1 >> 5 != ~I2 || !((~I3 >> (~S1 & 0x1f)) & 1)", p);
        }
        write_string(g, ") ");
        if (empty_case != -1) {
            /* If the among includes the empty string, it can never fail
             * so not matching the bitmap means we match the empty string.
             */
            g->I[4] = among_cases[empty_case].result;
            writef(g, "among_var = ~I4; else~N", p);
        } else {
            writef(g, "~f~N", p);
        }
    } else {
#ifdef OPTIMISATION_WARNINGS
        printf("Couldn't shortcut among %d\n", x->number);
#endif
    }

    if (x->amongvar_needed) {
        writef(g, "~Mamong_var = find_among~S0(z, a_~I0, ~I1);~N", p);
        if (!x->always_matches) {
            writef(g, "~Mif (!among_var) ~f~N", p);
        }
    } else if (x->always_matches) {
        writef(g, "~Mfind_among~S0(z, a_~I0, ~I1);~N", p);
    } else {
        writef(g, "~Mif (!find_among~S0(z, a_~I0, ~I1)) ~f~N", p);
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
        int i;
        writef(g, "~Mswitch (among_var) {~N~+", p);
        for (i = 1; i <= x->command_count; i++) {
            g->I[0] = i;
            w(g, "~Mcase ~I0:~N~+");
            generate(g, x->commands[i - 1]);
            w(g, "~Mbreak;~N~-");
        }
        w(g, "~}");
    }
}

static void generate_booltest(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->V[0] = p->name;
    writef(g, "~Mif (!(~V0)) ~f~N", p);
}

static void generate_false(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~M~f~N", p);
}

static void generate_debug(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->I[0] = g->debug_count++;
    g->I[1] = p->line_number;
    writef(g, "~Mdebug(z, ~I0, ~I1);~N", p);
}

static void generate(struct generator * g, struct node * p) {
    int used = g->label_used;
    int a0 = g->failure_label;
    int a1 = g->failure_keep_count;

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
        case c_booltest:      generate_booltest(g, p); break;
        case c_false:         generate_false(g, p); break;
        case c_true:          break;
        case c_debug:         generate_debug(g, p); break;
        case c_functionend:   generate_functionend(g, p); break;
        default: fprintf(stderr, "%d encountered\n", p->type);
                 exit(1);
    }

    if (g->failure_label != a0)
        g->label_used = used;
    g->failure_label = a0;
    g->failure_keep_count = a1;
}

void write_generated_comment_content(struct generator * g) {
    w(g, "Generated by Snowball " SNOWBALL_VERSION
         " - https://snowballstem.org/");
}

void write_start_comment(struct generator * g,
                         const char * comment_start,
                         const char * comment_end) {
    write_margin(g);
    w(g, comment_start);
    write_generated_comment_content(g);
    if (comment_end) {
        w(g, comment_end);
    }
    w(g, "~N~N");
}

static void generate_head(struct generator * g) {
    w(g, "#include \"");
    if (g->options->runtime_path) {
        write_string(g, g->options->runtime_path);
        if (g->options->runtime_path[strlen(g->options->runtime_path) - 1] != '/')
            write_char(g, '/');
    }
    w(g, "header.h\"~N~N");
}

static void generate_routine_headers(struct generator * g) {
    struct name * q;
    for (q = g->analyser->names; q; q = q->next) {
        g->V[0] = q;
        switch (q->type) {
            case t_routine:
                w(g, "static int ~W0(struct SN_env * z);~N");
                break;
            case t_external:
                w(g,
                  "#ifdef __cplusplus~N"
                  "extern \"C\" {~N"
                  "#endif~N"
                  "extern int ~W0(struct SN_env * z);~N"
                  "#ifdef __cplusplus~N"
                  "}~N"
                  "#endif~N"
                  );
                break;
        }
    }
}

static void generate_among_table(struct generator * g, struct among * x) {
    write_comment(g, x->node);

    struct amongvec * v = x->b;

    g->I[0] = x->number;
    for (int i = 0; i < x->literalstring_count; i++) {
        g->I[1] = i;
        g->I[2] = v[i].size;
        g->L[0] = v[i].b;
        if (v[i].size)
            w(g, "static const symbol s_~I0_~I1[~I2] = ~A0;~N");
    }

    g->I[1] = x->literalstring_count;
    w(g, "~Mstatic const struct among a_~I0[~I1] = {~N");

    for (int i = 0; i < x->literalstring_count; i++) {
        g->I[1] = i;
        g->I[2] = v[i].size;
        g->I[3] = (v[i].i >= 0 ? v[i].i - i : 0);
        g->I[4] = v[i].result;
        g->S[0] = i < x->literalstring_count - 1 ? "," : "";

        if (g->options->comments) {
            w(g, "/*~J1 */ ");
        }
        w(g, "{ ~I2, ");
        if (v[i].size == 0) {
            w(g, "0,");
        } else {
            w(g, "s_~I0_~I1,");
        }
        w(g, " ~I3, ~I4, ");
        if (v[i].function == NULL) {
            write_char(g, '0');
        } else {
            write_varname(g, v[i].function);
        }
        w(g, "}~S0~N");
    }
    w(g, "};~N~N");
}

static void generate_amongs(struct generator * g) {
    struct among * x;
    for (x = g->analyser->amongs; x; x = x->next) {
        generate_among_table(g, x);
    }
}

static void set_bit(symbol * b, int i) { b[i/8] |= 1 << i%8; }

static void generate_grouping_table(struct generator * g, struct grouping * q) {
    int range = q->largest_ch - q->smallest_ch + 1;
    int size = (range + 7)/ 8;  /* assume 8 bits per symbol */
    symbol * b = q->b;
    symbol * map = create_b(size);
    int i;
    for (i = 0; i < size; i++) map[i] = 0;

    for (i = 0; i < SIZE(b); i++) set_bit(map, b[i] - q->smallest_ch);

    g->V[0] = q->name;

    w(g, "static const unsigned char ~V0[] = { ");
    for (i = 0; i < size; i++) {
        write_int(g, map[i]);
        if (i < size - 1) w(g, ", ");
    }
    w(g, " };~N~N");
    lose_b(map);
}

static void generate_groupings(struct generator * g) {
    struct grouping * q;
    for (q = g->analyser->groupings; q; q = q->next) {
        if (q->name->used)
            generate_grouping_table(g, q);
    }
}

static void generate_create(struct generator * g) {
    int * p = g->analyser->name_count;
    g->I[0] = p[t_string];
    g->I[1] = p[t_integer] + p[t_boolean];
    w(g, "~N"
         "extern struct SN_env * ~pcreate_env(void) { return SN_create_env(~I0, ~I1); }"
         "~N");
}

static void generate_close(struct generator * g) {
    int * p = g->analyser->name_count;
    g->I[0] = p[t_string];
    w(g, "~Nextern void ~pclose_env(struct SN_env * z) { SN_close_env(z, ~I0); }~N~N");
}

static void generate_create_and_close_templates(struct generator * g) {
    w(g, "~N"
         "extern struct SN_env * ~pcreate_env(void);~N"
         "extern void ~pclose_env(struct SN_env * z);~N"
         "~N");
}

static void generate_header_file(struct generator * g) {
    struct name * q;
    const char * vp = g->options->variables_prefix;
    g->S[0] = vp;

    w(g, "#ifdef __cplusplus~N"
         "extern \"C\" {~N"
         "#endif~N");            /* for C++ */

    generate_create_and_close_templates(g);
    for (q = g->analyser->names; q; q = q->next) {
        g->V[0] = q;
        switch (q->type) {
            case t_external:
                w(g, "extern int ~W0(struct SN_env * z);~N");
                break;
            case t_string:
            case t_integer:
            case t_boolean:
                if (vp) {
                    int count = q->count;
                    if (count < 0) {
                        /* Unused variables should get removed from `names`. */
                        q->s[SIZE(q->s)] = 0;
                        fprintf(stderr, "Optimised out variable %s still in names list\n",
                                q->s);
                        exit(1);
                    }
                    if (q->type == t_boolean) {
                        /* We use a single array for booleans and integers,
                         * with the integers first.
                         */
                        count += g->analyser->name_count[t_integer];
                    }
                    g->I[0] = count;
                    g->I[1] = "SIIrxg"[q->type];
                    w(g, "#define ~S0");
                    write_s(g, q->s);
                    w(g, " (~c1[~I0])~N");
                }
                break;
        }
    }

    w(g, "~N"
         "#ifdef __cplusplus~N"
         "}~N"
         "#endif~N");            /* for C++ */

    w(g, "~N");
}

extern void generate_program_c(struct generator * g) {
    g->outbuf = str_new();
    write_start_comment(g, "/* ", " */");
    generate_head(g);
    generate_routine_headers(g);
    w(g, "#ifdef __cplusplus~N"
         "extern \"C\" {~N"
         "#endif~N"
         "~N");
    generate_create_and_close_templates(g);
    w(g, "~N"
         "#ifdef __cplusplus~N"
         "}~N"
         "#endif~N");
    generate_amongs(g);
    generate_groupings(g);
    g->declarations = g->outbuf;
    g->outbuf = str_new();
    g->literalstring_count = 0;
    {
        struct node * p = g->analyser->program;
        while (p) { generate(g, p); p = p->right; }
    }
    generate_create(g);
    generate_close(g);
    output_str(g->options->output_src, g->declarations);
    str_delete(g->declarations);
    output_str(g->options->output_src, g->outbuf);
    str_clear(g->outbuf);

    write_start_comment(g, "/* ", " */");
    generate_header_file(g);
    output_str(g->options->output_h, g->outbuf);
    str_delete(g->outbuf);
}

/* Generator functions common to multiple languages. */

extern struct generator * create_generator(struct analyser * a, struct options * o) {
    NEW(generator, g);
    g->analyser = a;
    g->options = o;
    g->margin = 0;
    g->debug_count = 0;
    g->copy_from_count = 0;
    g->line_count = 0;
    g->line_labelled = 0;
    g->failure_label = -1;
    g->unreachable = false;
#ifndef DISABLE_PYTHON
    g->max_label = 0;
#endif
    return g;
}

extern void close_generator(struct generator * g) {
    FREE(g);
}

/* Write routines for simple entities */

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

extern void write_int(struct generator * g, int i) {
    str_append_int(g->outbuf, i);
}

extern void write_s(struct generator * g, const byte * s) {
    str_append_s(g->outbuf, s);
}

extern void write_str(struct generator * g, struct str * str) {
    str_append(g->outbuf, str);
}
