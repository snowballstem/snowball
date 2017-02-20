#include <stdio.h>   /* for fprintf etc */
#include <stdlib.h>  /* for free etc */
#include <string.h>  /* for strcmp */
#include "header.h"

#define DEFAULT_PACKAGE "org.tartarus.snowball.ext"
#define DEFAULT_BASE_CLASS "org.tartarus.snowball.SnowballProgram"
#define DEFAULT_AMONG_CLASS "org.tartarus.snowball.Among"
#define DEFAULT_STRING_CLASS "java.lang.StringBuilder"

#define DEFAULT_CS_NAMESPACE "Snowball"
#define DEFAULT_CS_BASE_CLASS "Stemmer"
#define DEFAULT_CS_AMONG_CLASS "Among"
#define DEFAULT_CS_STRING_CLASS "StringBuilder"

static int eq(const char * s1, const char * s2) {
    return strcmp(s1, s2) == 0;
}

static void print_arglist(void) {
    fprintf(stderr, "Usage: snowball <file> [options]\n\n"
                    "options are: [-o[utput] file]\n"
                    "             [-s[yntax]]\n"
#ifndef DISABLE_JAVA
                    "             [-j[ava]]\n"
#endif
#ifndef DISABLE_CSHARP
                    "             [-cs[harp]]\n"
#endif
                    "             [-c++]\n"
#ifndef DISABLE_PYTHON
                    "             [-py[thon]]\n"
#endif
#ifndef DISABLE_JSX
                    "             [-jsx]\n"
#endif
                    "             [-w[idechars]]\n"
                    "             [-u[tf8]]\n"
                    "             [-n[ame] class name]\n"
                    "             [-ep[refix] string]\n"
                    "             [-vp[refix] string]\n"
                    "             [-i[nclude] directory]\n"
                    "             [-r[untime] path to runtime headers]\n"
#if !defined(DISABLE_JAVA) || !defined(DISABLE_CSHARP)
                    "             [-p[arentclassname] fully qualified parent class name]\n"
                    "             [-P[ackage] package name for stemmers]\n"
                    "             [-S[tringclass] StringBuffer-compatible class]\n"
                    "             [-a[mongclass] fully qualified name of the Among class]\n"
#endif
           );
    exit(1);
}

static void check_lim(int i, int argc) {
    if (i >= argc) {
        fprintf(stderr, "argument list is one short\n");
        print_arglist();
    }
}

static FILE * get_output(symbol * b) {
    char * s = b_to_s(b);
    FILE * output = fopen(s, "w");
    if (output == 0) {
        fprintf(stderr, "Can't open output %s\n", s);
        exit(1);
    }
    free(s);
    return output;
}

static void read_options(struct options * o, int argc, char * argv[]) {
    char * s;
    int i = 2;

    /* set defaults: */

    o->output_file = 0;
    o->syntax_tree = false;
    o->externals_prefix = NULL;
    o->variables_prefix = 0;
    o->runtime_path = 0;
    o->parent_class_name = DEFAULT_BASE_CLASS;
    o->string_class = DEFAULT_STRING_CLASS;
    o->among_class = DEFAULT_AMONG_CLASS;
    o->package = DEFAULT_PACKAGE;
    o->name = "";
    o->make_lang = LANG_C;
    o->widechars = false;
    o->includes = 0;
    o->includes_end = 0;
    o->utf8 = false;

    /* read options: */

    while (i < argc) {
        s = argv[i++];
        {   if (eq(s, "-o") || eq(s, "-output")) {
                check_lim(i, argc);
                o->output_file = argv[i++];
                continue;
            }
            if (eq(s, "-n") || eq(s, "-name")) {
                check_lim(i, argc);
                o->name = argv[i++];
                continue;
            }
#ifndef DISABLE_JSX
            if (eq(s, "-jsx")) {
                o->make_lang = LANG_JSX;
                o->widechars = true;
                continue;
            }
#endif
#ifndef DISABLE_JAVA
            if (eq(s, "-j") || eq(s, "-java")) {
                o->make_lang = LANG_JAVA;
                o->widechars = true;
                continue;
            }
#endif
#ifndef DISABLE_CSHARP
            if (eq(s, "-cs") || eq(s, "-csharp")) {
                o->make_lang = LANG_CSHARP;
                o->widechars = true;
                o->parent_class_name = DEFAULT_CS_BASE_CLASS;
                o->string_class = DEFAULT_CS_STRING_CLASS;
                o->among_class = DEFAULT_CS_AMONG_CLASS;
                o->package = DEFAULT_CS_NAMESPACE;
                continue;
            }
#endif
            if (eq(s, "-c++")) {
                o->make_lang = LANG_CPLUSPLUS;
                continue;
            }
#ifndef DISABLE_PYTHON
            if (eq(s, "-py") || eq(s, "-python")) {
                o->make_lang = LANG_PYTHON;
                o->widechars = true;
                continue;
            }
#endif
            if (eq(s, "-w") || eq(s, "-widechars")) {
                o->widechars = true;
                o->utf8 = false;
                continue;
            }
            if (eq(s, "-s") || eq(s, "-syntax")) {
                o->syntax_tree = true;
                continue;
            }
            if (eq(s, "-ep") || eq(s, "-eprefix")) {
                check_lim(i, argc);
                o->externals_prefix = argv[i++];
                continue;
            }
            if (eq(s, "-vp") || eq(s, "-vprefix")) {
                check_lim(i, argc);
                o->variables_prefix = argv[i++];
                continue;
            }
            if (eq(s, "-i") || eq(s, "-include")) {
                check_lim(i, argc);

                {
                    NEW(include, p);
                    symbol * b = add_s_to_b(0, argv[i++]);
                    b = add_s_to_b(b, "/");
                    p->next = 0; p->b = b;

                    if (o->includes == 0) o->includes = p; else
                                          o->includes_end->next = p;
                    o->includes_end = p;
                }
                continue;
            }
            if (eq(s, "-r") || eq(s, "-runtime")) {
                check_lim(i, argc);
                o->runtime_path = argv[i++];
                continue;
            }
            if (eq(s, "-u") || eq(s, "-utf8")) {
                o->utf8 = true;
                o->widechars = false;
                continue;
            }
#if !defined(DISABLE_JAVA) || !defined(DISABLE_CSHARP)
            if (eq(s, "-p") || eq(s, "-parentclassname")) {
                check_lim(i, argc);
                o->parent_class_name = argv[i++];
                continue;
            }
            if (eq(s, "-P") || eq(s, "-Package")) {
                check_lim(i, argc);
                o->package = argv[i++];
                continue;
            }
            if (eq(s, "-S") || eq(s, "-stringclass")) {
                check_lim(i, argc);
                o->string_class = argv[i++];
                continue;
            }
            if (eq(s, "-a") || eq(s, "-amongclass")) {
                check_lim(i, argc);
                o->among_class = argv[i++];
                continue;
            }
#endif
            fprintf(stderr, "'%s' misplaced\n", s);
            print_arglist();
        }
    }

    if (o->make_lang != LANG_C && o->make_lang != LANG_CPLUSPLUS) {
	if (o->runtime_path) {
	    fprintf(stderr, "warning: -r/-runtime only meaningful for C and C++\n");
	}
	if (o->externals_prefix) {
	    fprintf(stderr, "warning: -ep/-eprefix only meaningful for C and C++\n");
	}
    }
    if (!o->externals_prefix) o->externals_prefix = "";
}

extern int main(int argc, char * argv[]) {

    NEW(options, o);
    if (argc == 1) print_arglist();
    read_options(o, argc, argv);
    {
        symbol * filename = add_s_to_b(0, argv[1]);
        char * file;
        symbol * u = get_input(filename, &file);
        if (u == 0) {
            fprintf(stderr, "Can't open input %s\n", argv[1]);
            exit(1);
        }
        {
            struct tokeniser * t = create_tokeniser(u, file);
            struct analyser * a = create_analyser(t);
            t->widechars = o->widechars;
            t->includes = o->includes;
            a->utf8 = t->utf8 = o->utf8;
            read_program(a);
            if (t->error_count > 0) exit(1);
            if (o->syntax_tree) print_program(a);
            close_tokeniser(t);
            if (!o->syntax_tree) {
                struct generator * g;

                const char * s = o->output_file;
                if (!s) {
                    fprintf(stderr, "Please include the -o option\n");
                    print_arglist();
                    exit(1);
                }
                g = create_generator(a, o);
                if (o->make_lang == LANG_C || o->make_lang == LANG_CPLUSPLUS) {
                    symbol * b = add_s_to_b(0, s);
                    b = add_s_to_b(b, ".h");
                    o->output_h = get_output(b);
                    b[SIZE(b) - 1] = 'c';
                    if (o->make_lang == LANG_CPLUSPLUS) {
                        b = add_s_to_b(b, "c");
                    }
                    o->output_src = get_output(b);
                    lose_b(b);

                    generate_program_c(g);
                    fclose(o->output_src);
                    fclose(o->output_h);
                }
#ifndef DISABLE_JAVA
                if (o->make_lang == LANG_JAVA) {
                    symbol * b = add_s_to_b(0, s);
                    b = add_s_to_b(b, ".java");
                    o->output_src = get_output(b);
                    lose_b(b);
                    generate_program_java(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_PYTHON
                if (o->make_lang == LANG_PYTHON) {
                    symbol * b = add_s_to_b(0, s);
                    b = add_s_to_b(b, ".py");
                    o->output_src = get_output(b);
                    lose_b(b);
                    generate_program_python(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_JSX
                if (o->make_lang == LANG_JSX) {
                    symbol * b = add_s_to_b(0, s);
                    b = add_s_to_b(b, ".jsx");
                    o->output_src = get_output(b);
                    lose_b(b);
                    generate_program_jsx(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_CSHARP
                if (o->make_lang == LANG_CSHARP) {
                    symbol * b = add_s_to_b(0, s);
                    b = add_s_to_b(b, ".cs");
                    o->output_src = get_output(b);
                    lose_b(b);
                    generate_program_csharp(g);
                    fclose(o->output_src);
                }
#endif
                close_generator(g);
            }
            close_analyser(a);
        }
        lose_b(u);
        lose_b(filename);
    }
    {   struct include * p = o->includes;
        while (p) {
            struct include * q = p->next;
            lose_b(p->b); FREE(p); p = q;
        }
    }
    FREE(o);
    if (space_count) fprintf(stderr, "%d blocks unfreed\n", space_count);
    return 0;
}

