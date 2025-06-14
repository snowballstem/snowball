#include <ctype.h>   /* for toupper etc */
#include <stdio.h>   /* for fprintf etc */
#include <stdlib.h>  /* for free etc */
#include <string.h>  /* for strcmp */
#include "header.h"

#define DEFAULT_JAVA_PACKAGE "org.tartarus.snowball.ext"
#define DEFAULT_JAVA_BASE_CLASS "org.tartarus.snowball.SnowballProgram"
#define DEFAULT_JAVA_AMONG_CLASS "org.tartarus.snowball.Among"
#define DEFAULT_JAVA_STRING_CLASS "java.lang.StringBuilder"

#define DEFAULT_GO_PACKAGE "snowball"
#define DEFAULT_GO_SNOWBALL_RUNTIME "github.com/snowballstem/snowball/go"

#define DEFAULT_ADA_PACKAGE "Snowball"
#define DEFAULT_ADA_SNOWBALL_RUNTIME "github.com/snowballstem/snowball/ada"

#define DEFAULT_CS_NAMESPACE "Snowball"
#define DEFAULT_CS_BASE_CLASS "Stemmer"
#define DEFAULT_CS_AMONG_CLASS "Among"
#define DEFAULT_CS_STRING_CLASS "StringBuilder"

#define DEFAULT_JS_BASE_CLASS "BaseStemmer"

#define DEFAULT_PYTHON_BASE_CLASS "BaseStemmer"

static int eq(const char * s1, const char * s2) {
    return strcmp(s1, s2) == 0;
}

static void print_arglist(int exit_code) {
    FILE * f = exit_code ? stderr : stdout;
    fprintf(f, "Usage: snowball SOURCE_FILE... [OPTIONS]\n\n"
               "Supported options:\n"
               "  -o, -output OUTPUT_BASE\n"
               "  -s, -syntax                      show syntax tree and stop\n"
               "  -comments                        generate comments\n"
#ifndef DISABLE_JAVA
               "  -j, -java                        generate Java\n"
#endif
#ifndef DISABLE_CSHARP
               "  -cs, -csharp                     generate C#\n"
#endif
               "  -c++                             generate C++\n"
#ifndef DISABLE_PASCAL
               "  -pascal                          generate Pascal\n"
#endif
#ifndef DISABLE_PYTHON
               "  -py, -python                     generate Python\n"
#endif
#ifndef DISABLE_JS
               "  -js                              generate Javascript\n"
#endif
#ifndef DISABLE_RUST
               "  -rust                            generate Rust\n"
#endif
#ifndef DISABLE_GO
               "  -go                              generate Go\n"
#endif
#ifndef DISABLE_ADA
               "  -ada                             generate Ada\n"
#endif
               "  -w, -widechars\n"
               "  -u, -utf8\n"
               "  -n, -name CLASS_NAME\n"
               "  -ep, -eprefix EXTERNAL_PREFIX\n"
               "  -vp, -vprefix VARIABLE_PREFIX\n"
               "  -i, -include DIRECTORY\n"
               "  -r, -runtime DIRECTORY\n"
               "  -p, -parentclassname CLASS_NAME  fully qualified parent class name\n"
#if !defined(DISABLE_JAVA) || !defined(DISABLE_CSHARP)
               "  -P, -Package PACKAGE_NAME        package name for stemmers\n"
               "  -S, -Stringclass STRING_CLASS    StringBuffer-compatible class\n"
               "  -a, -amongclass AMONG_CLASS      fully qualified name of the Among class\n"
#endif
#ifndef DISABLE_GO
               "  -gop, -gopackage PACKAGE_NAME    Go package name for stemmers\n"
               "  -gor, -goruntime PACKAGE_NAME    Go snowball runtime package\n"
#endif
               "  --help                           display this help and exit\n"
               "  --version                        output version information and exit\n"
           );
    exit(exit_code);
}

static void check_lim(int i, int argc) {
    if (i >= argc) {
        fprintf(stderr, "argument list is one short\n");
        print_arglist(1);
    }
}

static FILE * get_output(byte * s) {
    s[SIZE(s)] = 0;
    const char * filename = (const char *)s;
    FILE * output = fopen(filename, "w");
    if (output == NULL) {
        fprintf(stderr, "Can't open output %s\n", filename);
        exit(1);
    }
    return output;
}

static int read_options(struct options * o, int argc, char * argv[]) {
    int i = 1;
    int new_argc = 1;
    /* Note down the last option used to specify an explicit encoding so
     * we can warn we ignored it for languages with a fixed encoding.
     */
    const char * encoding_opt = NULL;

    /* set defaults: */

    o->output_file = NULL;
    o->syntax_tree = false;
    o->comments = false;
    o->externals_prefix = NULL;
    o->variables_prefix = NULL;
    o->runtime_path = NULL;
    o->parent_class_name = NULL;
    o->string_class = NULL;
    o->among_class = NULL;
    o->package = NULL;
    o->go_snowball_runtime = DEFAULT_GO_SNOWBALL_RUNTIME;
    o->name = NULL;
    o->make_lang = LANG_C;
    o->includes = NULL;
    o->includes_end = NULL;
    o->encoding = ENC_SINGLEBYTE;

    /* read options: */

    while (i < argc) {
        char * s = argv[i++];
        if (s[0] != '-' || s[1] == '\0') {
            /* Non-option argument - shuffle down. */
            argv[new_argc++] = s;
            continue;
        }

        {
            if (eq(s, "-o") || eq(s, "-output")) {
                check_lim(i, argc);
                o->output_file = argv[i++];
                continue;
            }
            if (eq(s, "-n") || eq(s, "-name")) {
                char * new_name;
                size_t len;

                check_lim(i, argc);
                /* Take a copy of the argument here, because
                 * later we will free o->name */
                len = strlen(argv[i]);
                new_name = MALLOC(len + 1);
                memcpy(new_name, argv[i++], len);
                new_name[len] = '\0';
                o->name = new_name;
                continue;
            }
#ifndef DISABLE_JS
            if (eq(s, "-js")) {
                o->make_lang = LANG_JAVASCRIPT;
                continue;
            }
#endif
#ifndef DISABLE_RUST
            if (eq(s, "-rust")) {
                o->make_lang = LANG_RUST;
                continue;
            }
#endif
#ifndef DISABLE_GO
            if (eq(s, "-go")) {
                o->make_lang = LANG_GO;
                continue;
            }
#endif
#ifndef DISABLE_JAVA
            if (eq(s, "-j") || eq(s, "-java")) {
                o->make_lang = LANG_JAVA;
                continue;
            }
#endif
#ifndef DISABLE_CSHARP
            if (eq(s, "-cs") || eq(s, "-csharp")) {
                o->make_lang = LANG_CSHARP;
                continue;
            }
#endif
            if (eq(s, "-c++")) {
                o->make_lang = LANG_CPLUSPLUS;
                continue;
            }
#ifndef DISABLE_PASCAL
            if (eq(s, "-pascal")) {
                o->make_lang = LANG_PASCAL;
                continue;
            }
#endif
#ifndef DISABLE_PYTHON
            if (eq(s, "-py") || eq(s, "-python")) {
                o->make_lang = LANG_PYTHON;
                continue;
            }
#endif
#ifndef DISABLE_ADA
            if (eq(s, "-ada")) {
                o->make_lang = LANG_ADA;
                continue;
            }
#endif
            if (eq(s, "-w") || eq(s, "-widechars")) {
                encoding_opt = s;
                o->encoding = ENC_WIDECHARS;
                continue;
            }
            if (eq(s, "-s") || eq(s, "-syntax")) {
                o->syntax_tree = true;
                continue;
            }
            if (eq(s, "-comments")) {
                o->comments = true;
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
                    byte * include_dir = add_sz_to_s(NULL, argv[i++]);
                    include_dir = add_char_to_s(include_dir, '/');
                    p->next = NULL; p->s = include_dir;

                    if (o->includes == NULL) {
                        o->includes = p;
                    } else {
                        o->includes_end->next = p;
                    }
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
                encoding_opt = s;
                o->encoding = ENC_UTF8;
                continue;
            }
            if (eq(s, "-p") || eq(s, "-parentclassname")) {
                check_lim(i, argc);
                o->parent_class_name = argv[i++];
                continue;
            }
#if !defined(DISABLE_JAVA) || !defined(DISABLE_CSHARP)
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
#ifndef DISABLE_GO
            if (eq(s, "-gop") || eq(s, "-gopackage")) {
                check_lim(i, argc);
                o->package = argv[i++];
                continue;
            }
            if (eq(s, "-gor") || eq(s, "-goruntime")) {
                check_lim(i, argc);
                o->go_snowball_runtime = argv[i++];
                continue;
            }
#endif
            if (eq(s, "--help")) {
                print_arglist(0);
            }

            if (eq(s, "--version")) {
                printf("Snowball compiler version " SNOWBALL_VERSION "\n");
                exit(0);
            }

            fprintf(stderr, "'%s' misplaced\n", s);
            print_arglist(1);
        }
    }
    if (new_argc == 1) {
        fprintf(stderr, "no source files specified\n");
        print_arglist(1);
    }
    argv[new_argc] = NULL;

    /* Set language-dependent defaults. */
    switch (o->make_lang) {
        case LANG_C:
        case LANG_CPLUSPLUS:
            encoding_opt = NULL;
            break;
        case LANG_CSHARP:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_CS_BASE_CLASS;
            if (!o->string_class)
                o->string_class = DEFAULT_CS_STRING_CLASS;
            if (!o->among_class)
                o->among_class = DEFAULT_CS_AMONG_CLASS;
            if (!o->package)
                o->package = DEFAULT_CS_NAMESPACE;
            break;
        case LANG_GO:
            o->encoding = ENC_UTF8;
            if (!o->package)
                o->package = DEFAULT_GO_PACKAGE;
            break;
        case LANG_ADA:
            o->encoding = ENC_UTF8;
            if (!o->package)
                o->package = DEFAULT_ADA_PACKAGE;
            break;
        case LANG_JAVA:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_JAVA_BASE_CLASS;
            if (!o->string_class)
                o->string_class = DEFAULT_JAVA_STRING_CLASS;
            if (!o->among_class)
                o->among_class = DEFAULT_JAVA_AMONG_CLASS;
            if (!o->package)
                o->package = DEFAULT_JAVA_PACKAGE;
            break;
        case LANG_JAVASCRIPT:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_JS_BASE_CLASS;
            break;
        case LANG_PYTHON:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_PYTHON_BASE_CLASS;
            break;
        case LANG_RUST:
            o->encoding = ENC_UTF8;
            break;
        default:
            break;
    }

    if (encoding_opt) {
        fprintf(stderr, "warning: %s only meaningful for C and C++\n",
                encoding_opt);
    }

    if (o->make_lang != LANG_C && o->make_lang != LANG_CPLUSPLUS) {
        if (o->runtime_path) {
            fprintf(stderr, "warning: -r/-runtime only meaningful for C and C++\n");
        }
        if (o->externals_prefix) {
            fprintf(stderr, "warning: -ep/-eprefix only meaningful for C and C++\n");
        }
        if (o->variables_prefix) {
            fprintf(stderr, "warning: -vp/-vprefix only meaningful for C and C++\n");
        }
    }
    if (!o->externals_prefix) o->externals_prefix = "";

    if (!o->name && o->output_file) {
        /* Default class name to basename of output_file - this is the standard
         * convention for at least Java and C#.
         */
        const char * slash = strrchr(o->output_file, '/');
        size_t len;
        const char * leaf = (slash == NULL) ? o->output_file : slash + 1;

        slash = strrchr(leaf, '\\');
        if (slash != NULL) leaf = slash + 1;

        {
            const char * dot = strchr(leaf, '.');
            len = (dot == NULL) ? strlen(leaf) : (size_t)(dot - leaf);
        }

        {
            char * new_name = MALLOC(len + 1);
            switch (o->make_lang) {
                case LANG_CSHARP:
                case LANG_PASCAL:
                    /* Upper case initial letter. */
                    memcpy(new_name, leaf, len);
                    new_name[0] = toupper(new_name[0]);
                    break;
                case LANG_JAVASCRIPT:
                case LANG_PYTHON: {
                    /* Upper case initial letter and change each
                     * underscore+letter or hyphen+letter to an upper case
                     * letter.
                     */
                    size_t new_len = 0;
                    int uc_next = true;
                    for (size_t j = 0; j != len; ++j) {
                        unsigned char ch = leaf[j];
                        if (ch == '_' || ch == '-') {
                            uc_next = true;
                        } else {
                            if (uc_next) {
                                new_name[new_len] = toupper(ch);
                                uc_next = false;
                            } else {
                                new_name[new_len] = ch;
                            }
                            ++new_len;
                        }
                    }
                    len = new_len;
                    break;
                }
                default:
                    /* Just copy. */
                    memcpy(new_name, leaf, len);
                    break;
            }
            new_name[len] = '\0';
            o->name = new_name;
        }
    }

    return new_argc;
}

extern int main(int argc, char * argv[]) {
    int i;
    NEW(options, o);
    argc = read_options(o, argc, argv);
    {
        char * file = argv[1];
        byte * u = get_input(file);
        if (u == NULL) {
            fprintf(stderr, "Can't open input %s\n", file);
            exit(1);
        }
        {
            struct tokeniser * t = create_tokeniser(u, file);
            struct analyser * a = create_analyser(t);
            struct input ** next_input_ptr = &(t->next);
            a->encoding = t->encoding = o->encoding;
            t->includes = o->includes;
            /* If multiple source files are specified, set up the others to be
             * read after the first in order, using the same mechanism as
             * 'get' uses. */
            for (i = 2; i != argc; ++i) {
                NEW(input, q);
                file = argv[i];
                u = get_input(file);
                if (u == NULL) {
                    fprintf(stderr, "Can't open input %s\n", file);
                    exit(1);
                }
                q->p = u;
                q->c = 0;
                q->file = file;
                q->file_owned = 0;
                q->line_number = 1;
                *next_input_ptr = q;
                next_input_ptr = &(q->next);
            }
            *next_input_ptr = NULL;
            read_program(a);
            if (t->error_count > 0) exit(1);
            if (o->syntax_tree) print_program(a);
            if (!o->syntax_tree) {
                struct generator * g;

                const char * output_base = o->output_file;
                if (!output_base) {
                    fprintf(stderr, "Please include the -o option\n");
                    print_arglist(1);
                }
                g = create_generator(a, o);
                if (o->make_lang == LANG_C || o->make_lang == LANG_CPLUSPLUS) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".h");
                    o->output_h = get_output(s);
                    s[SIZE(s) - 1] = 'c';
                    if (o->make_lang == LANG_CPLUSPLUS) {
                        s = add_char_to_s(s, 'c');
                    }
                    o->output_src = get_output(s);
                    lose_s(s);

                    generate_program_c(g);
                    fclose(o->output_src);
                    fclose(o->output_h);
                }
#ifndef DISABLE_JAVA
                if (o->make_lang == LANG_JAVA) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".java");
                    o->output_src = get_output(s);
                    lose_s(s);
                    generate_program_java(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_PASCAL
                if (o->make_lang == LANG_PASCAL) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".pas");
                    o->output_src = get_output(s);
                    lose_s(s);
                    generate_program_pascal(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_PYTHON
                if (o->make_lang == LANG_PYTHON) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".py");
                    o->output_src = get_output(s);
                    lose_s(s);
                    generate_program_python(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_JS
                if (o->make_lang == LANG_JAVASCRIPT) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".js");
                    o->output_src = get_output(s);
                    lose_s(s);
                    generate_program_js(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_CSHARP
                if (o->make_lang == LANG_CSHARP) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".cs");
                    o->output_src = get_output(s);
                    lose_s(s);
                    generate_program_csharp(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_RUST
                if (o->make_lang == LANG_RUST) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".rs");
                    o->output_src = get_output(s);
                    lose_s(s);
                    generate_program_rust(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_GO
                if (o->make_lang == LANG_GO) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".go");
                    o->output_src = get_output(s);
                    lose_s(s);
                    generate_program_go(g);
                    fclose(o->output_src);
                }
#endif
#ifndef DISABLE_ADA
                if (o->make_lang == LANG_ADA) {
                    byte * s = add_sz_to_s(NULL, output_base);
                    s = add_literal_to_s(s, ".ads");
                    o->output_h = get_output(s);
                    s[SIZE(s) - 1] = 'b';
                    o->output_src = get_output(s);
                    lose_s(s);

                    generate_program_ada(g);
                    fclose(o->output_src);
                    fclose(o->output_h);
                }
#endif
                close_generator(g);
            }
            close_tokeniser(t);
            close_analyser(a);
        }
        lose_s(u);
    }
    {   struct include * p = o->includes;
        while (p) {
            struct include * q = p->next;
            lose_s(p->s);
            FREE(p);
            p = q;
        }
    }
    FREE(o->name);
    FREE(o);
    if (space_count) fprintf(stderr, "%d blocks unfreed\n", space_count);
    return 0;
}
