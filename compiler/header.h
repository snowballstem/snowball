#include <stdio.h>

#define SNOWBALL_VERSION "2.2.0"

typedef unsigned char byte;
typedef unsigned short symbol;

#define true 1
#define false 0

#define MALLOC check_malloc
#define FREE check_free

#define NEW(type, p) struct type * p = (struct type *) MALLOC(sizeof(struct type))
#define NEWVEC(type, p, n) struct type * p = (struct type *) MALLOC(sizeof(struct type) * (n))

#define SIZE(p)     ((int *)(p))[-1]
#define CAPACITY(p) ((int *)(p))[-2]

extern symbol * create_b(int n);
extern void report_b(FILE * out, const symbol * p);
extern void lose_b(symbol * p);
extern symbol * increase_capacity_b(symbol * p, int n);
extern symbol * add_to_b(symbol * p, const symbol * q, int n);
extern symbol * copy_b(const symbol * p);
extern char * b_to_sz(const symbol * p);
extern symbol * add_symbol_to_b(symbol * p, symbol ch);

// These routines are like those above but work in byte instead of symbol.

extern byte * create_s(int n);
extern void report_s(FILE * out, const byte * p);
extern void lose_s(byte * p);
extern byte * increase_capacity_s(byte * p, int n);
extern byte * copy_s(const byte * p);
extern byte * add_s_to_s(byte * p, const char * s, int n);
extern byte * add_sz_to_s(byte * p, const char * s);
extern byte * add_char_to_s(byte * p, char ch);
// "" LIT is a trick to make compilation fail if LIT is not a string literal.
#define add_literal_to_s(P, LIT) add_s_to_s(P, "" LIT, sizeof(LIT) - 1)

struct str; /* defined in space.c */

extern struct str * str_new(void);
extern void str_delete(struct str * str);
extern void str_append(struct str * str, const struct str * add);
extern void str_append_ch(struct str * str, char add);
extern void str_append_s(struct str * str, const byte * q);
extern void str_append_string(struct str * str, const char * s);
extern void str_append_int(struct str * str, int i);
extern void str_clear(struct str * str);
extern void str_assign(struct str * str, const char * s);
extern struct str * str_copy(const struct str * old);
extern byte * str_data(const struct str * str);
extern int str_len(const struct str * str);
extern int str_back(const struct str *str);
extern void str_pop(const struct str *str);
extern void output_str(FILE * outfile, struct str * str);

extern int get_utf8(const symbol * p, int * slot);
extern int put_utf8(int ch, symbol * p);

typedef enum { ENC_SINGLEBYTE, ENC_UTF8, ENC_WIDECHARS } enc;

/* stringdef name and value */
struct m_pair {

    struct m_pair * next;
    byte * name;
    symbol * value;

};

/* struct input must be a prefix of struct tokeniser. */
struct input {

    struct input * next;
    byte * p;
    int c;
    char * file;
    // -1 : Release file with: lose_s((byte *)file)
    //  0 : We don't own file.
    //  1 : Release file with: free(file)
    int file_owned;
    int line_number;

};

struct include {

    struct include * next;
    byte * s;

};

enum token_codes {
    /* The relational operator token values are chosen such that we can
     * invert the relation with a simple xor with 1.
     */
    c_gt = 0, c_le,
    c_ge, c_lt,
    c_eq, c_ne,

    /* Other token values just need to be unique. */
    c_among, c_and, c_as, c_assign, c_assignto, c_atleast,
    c_atlimit, c_atmark, c_attach, c_backwardmode, c_backwards,
    c_booleans, c_bra, c_comment1, c_comment2, c_cursor, c_debug,
    c_decimal, c_define, c_delete, c_divide, c_divideassign, c_do,
    c_dollar, c_externals, c_fail, c_false, c_for, c_get,
    c_gopast, c_goto, c_groupings, c_hex, c_hop, c_insert,
    c_integers, c_ket, c_leftslice, c_len, c_lenof, c_limit, c_loop,
    c_maxint, c_minint, c_minus, c_minusassign, c_multiply,
    c_multiplyassign, c_next, c_non, c_not, c_or, c_plus,
    c_plusassign, c_repeat, c_reverse, c_rightslice, c_routines,
    c_set, c_setlimit, c_setmark, c_size, c_sizeof, c_slicefrom,
    c_sliceto, c_stringdef, c_stringescapes, c_strings, c_substring,
    c_test, c_tolimit, c_tomark, c_true, c_try, c_unset,

    /* These token values don't directly correspond to a keyword. */
    c_name,
    c_number,
    c_literalstring,

    /* These token values are synthesised by the analyser. */
    c_mathassign,
    c_neg,
    c_call,
    c_grouping,
    c_booltest,
    c_functionend,

    NUM_TOKEN_CODES
};

enum uplus_modes {
    UPLUS_NONE,
    UPLUS_DEFINED,
    UPLUS_UNICODE
};

/* struct input must be a prefix of struct tokeniser. */
struct tokeniser {

    struct input * next;
    byte * p;
    int c;
    char * file;
    // -1 : Release file with: lose_s((byte *)file)
    //  0 : We don't own file.
    //  1 : Release file with: free(file)
    int file_owned;
    int line_number;

    // Used for c_literalstring values.
    symbol * b;
    // Used for c_name names.
    byte * s;
    int number;
    int m_start;
    int m_end;
    struct m_pair * m_pairs;
    int get_depth;
    int error_count;
    int token;
    int previous_token;
    byte token_held;
    enc encoding;

    int omission;
    struct include * includes;

    /* Mode in which U+ has been used:
     * UPLUS_NONE - not used yet
     * UPLUS_DEFINED - stringdef U+xxxx ....
     * UPLUS_UNICODE - {U+xxxx} used with implicit meaning
     */
    int uplusmode;

    char token_disabled[NUM_TOKEN_CODES];
};

extern byte * get_input(const char * filename);
extern struct tokeniser * create_tokeniser(byte * b, char * file);
extern int read_token(struct tokeniser * t);
extern const char * name_of_token(int code);
extern void disable_token(struct tokeniser * t, int code);
extern void close_tokeniser(struct tokeniser * t);

extern int space_count;
extern void * check_malloc(size_t n);
extern void check_free(void * p);

struct node;

struct name {

    struct name * next;
    byte * s;
    int type;                   /* t_string etc */
    int mode;                   /*    )_  for routines, externals */
    struct node * definition;   /*    )                           */
    int count;                  /* 0, 1, 2 for each type */
    struct grouping * grouping; /* for grouping names */
    byte referenced;
    byte used_in_among;         /* Function used in among? */
    byte value_used;            /* (For variables) is its value ever used? */
    byte initialised;           /* (For variables) is it ever initialised? */
    byte used_in_definition;    /* (grouping) used in grouping definition? */
    struct node * used;         /* First use, or NULL if not used */
    struct name * local_to;     /* Local to one routine/external */
    int declaration_line_number;/* Line number of declaration */

};

struct literalstring {

    struct literalstring * next;
    symbol * b;

};

struct amongvec {

    symbol * b;      /* the string giving the case */
    int size;        /* - and its size */
    struct node * action; /* the corresponding action */
    int i;           /* the amongvec index of the longest substring of b */
    int result;      /* the numeric result for the case */
    int line_number; /* for diagnostics and stable sorting */
    struct name * function;

};

struct among {

    struct among * next;
    struct amongvec * b;      /* pointer to the amongvec */
    int number;               /* amongs are numbered 0, 1, 2 ... */
    int literalstring_count;  /* in this among */
    int command_count;        /* in this among (includes "no command" entries) */
    int nocommand_count;      /* number of "no command" entries in this among */
    int function_count;       /* in this among */
    int amongvar_needed;      /* do we need to set among_var? */
    int always_matches;       /* will this among always match? */
    int shortest_size;        /* smallest non-zero string length in this among */
    struct node * substring;  /* i.e. substring ... among ( ... ) */
    struct node ** commands;  /* array with command_count entries */
};

struct grouping {

    struct grouping * next;
    symbol * b;               /* the characters of this group */
    int largest_ch;           /* character with max code */
    int smallest_ch;          /* character with min code */
    struct name * name;       /* so g->name->grouping == g */
    int line_number;
};

struct node {

    struct node * next;
    struct node * left;
    struct node * aux;     /* used in setlimit */
    struct among * among;  /* used in among */
    struct node * right;
    int type;
    int mode;
    struct node * AE;
    struct name * name;
    symbol * literalstring;
    int number;
    int line_number;
    int amongvar_needed;   /* used in routine definitions */
};

enum name_types {

    t_size = 6,

    t_string = 0, t_boolean = 1, t_integer = 2, t_routine = 3, t_external = 4,
    t_grouping = 5

/*  If this list is extended, adjust write_varname in generator.c  */
};

/*  In name_count[i] below, remember that
    type   is
    ----+----
      0 |  string
      1 |  boolean
      2 |  integer
      3 |  routine
      4 |  external
      5 |  grouping
*/

struct analyser {

    struct tokeniser * tokeniser;
    struct node * nodes;
    struct name * names;
    struct literalstring * literalstrings;
    int mode;
    byte modifyable;          /* false inside reverse(...) */
    struct node * program;
    struct node * program_end;
    int name_count[t_size];   /* name_count[i] counts the number of names of type i */
    struct among * amongs;
    struct among * amongs_end;
    int among_count;
    int amongvar_needed;      /* used in reading routine definitions */
    int among_with_function_count; /* number of amongs with functions */
    struct grouping * groupings;
    struct grouping * groupings_end;
    struct node * substring;  /* pending 'substring' in current routine definition */
    enc encoding;
    byte int_limits_used;     /* are maxint or minint used? */
};

enum analyser_modes {

    m_forward = 0, m_backward /*, m_integer */

};

extern void print_program(struct analyser * a);
extern struct analyser * create_analyser(struct tokeniser * t);
extern void close_analyser(struct analyser * a);

extern void read_program(struct analyser * a);

struct generator {

    struct analyser * analyser;
    struct options * options;
    int unreachable;           /* 0 if code can be reached, 1 if current code
                                * is unreachable. */
    int var_number;            /* Number of next variable to use. */
    struct str * outbuf;       /* temporary str to store output */
    struct str * declarations; /* str storing variable declarations */
    int next_label;
#ifndef DISABLE_PYTHON
    int max_label;
#endif
    int margin;

    /* if > 0, keep_count to restore in case of a failure;
     * if < 0, the negated keep_count for the limit to restore in case of
     * failure. */
    int failure_keep_count;
#if !defined(DISABLE_JAVA) && !defined(DISABLE_JS) && !defined(DISABLE_PYTHON) && !defined(DISABLE_CSHARP)
    struct str * failure_str;  /* This is used by some generators instead of failure_keep_count */
#endif

    int label_used;     /* Keep track of whether the failure label is used. */
    int failure_label;
    int debug_count;
    int copy_from_count; /* count of calls to copy_from() */

    const char * S[10];  /* strings */
    byte * B[10];        /* byte blocks */
    int I[10];           /* integers */
    struct name * V[5];  /* variables */
    symbol * L[5];       /* literals, used in formatted write */

    int line_count;      /* counts number of lines output */
    int line_labelled;   /* in ISO C, will need extra ';' if it is a block end */
    int literalstring_count;
    int keep_count;      /* used to number keep/restore pairs to avoid compiler warnings
                            about shadowed variables */
    int temporary_used;  /* track if temporary variable used (for Pascal) */
};

/* Special values for failure_label in struct generator. */
enum special_labels {
    x_return = -1
};

struct options {

    /* for the command line: */

    const char * output_file;
    char * name;
    FILE * output_src;
    FILE * output_h;
    byte syntax_tree;
    byte comments;
    byte js_esm;
    enc encoding;
    enum { LANG_JAVA, LANG_C, LANG_CPLUSPLUS, LANG_CSHARP, LANG_PASCAL, LANG_PYTHON, LANG_JAVASCRIPT, LANG_RUST, LANG_GO, LANG_ADA } make_lang;
    const char * externals_prefix;
    const char * variables_prefix;
    const char * runtime_path;
    const char * parent_class_name;
    const char * package;
    const char * go_snowball_runtime;
    const char * string_class;
    const char * among_class;
    struct include * includes;
    struct include * includes_end;
};

/* Generator functions common to several backends. */

extern struct generator * create_generator(struct analyser * a, struct options * o);
extern void close_generator(struct generator * g);

extern void write_char(struct generator * g, int ch);
extern void write_newline(struct generator * g);
extern void write_string(struct generator * g, const char * s);
extern void write_int(struct generator * g, int i);
extern void write_hex4(struct generator * g, int ch);
extern void write_symbol(struct generator * g, symbol s);
extern void write_s(struct generator * g, const byte * b);
extern void write_str(struct generator * g, struct str * str);
extern void write_c_relop(struct generator * g, int relop);

extern void write_comment_content(struct generator * g, struct node * p);
extern void write_generated_comment_content(struct generator * g);
extern void write_start_comment(struct generator * g,
                                const char * comment_start,
                                const char * comment_end);

extern int K_needed(struct generator * g, struct node * p);
extern int repeat_restore(struct generator * g, struct node * p);
extern int check_possible_signals_list(struct generator * g,
                                       struct node * p,
                                       int type,
                                       int call_depth);

/* Generator for C code. */
extern void generate_program_c(struct generator * g);

#ifndef DISABLE_JAVA
/* Generator for Java code. */
extern void generate_program_java(struct generator * g);
#endif

#ifndef DISABLE_CSHARP
/* Generator for C# code. */
extern void generate_program_csharp(struct generator * g);
#endif

#ifndef DISABLE_PASCAL
extern void generate_program_pascal(struct generator * g);
#endif

#ifndef DISABLE_PYTHON
/* Generator for Python code. */
extern void generate_program_python(struct generator * g);
#endif

#ifndef DISABLE_JS
extern void generate_program_js(struct generator * g);
#endif

#ifndef DISABLE_RUST
extern void generate_program_rust(struct generator * g);
#endif

#ifndef DISABLE_GO
extern void generate_program_go(struct generator * g);
#endif

#ifndef DISABLE_ADA
extern void generate_program_ada(struct generator * g);
#endif
