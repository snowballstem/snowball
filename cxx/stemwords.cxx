/* This is a simple program which uses the generated C++ stemmers to provide a
 * command line interface for stemming using any of the algorithms provided.
 */

#include <exception>
#include <fstream>
#include <iostream>

#include <ctype.h>  /* for tolower */
#include <string.h>  /* for strcmp */

#include "stemmer.h"

const char * progname;
static int pretty = 1;

static void
stem_file(Snowball::Stemmer& stemmer, std::istream& f_in, std::ostream& f_out)
{
    std::string word;

    while (true) {
        int ch = f_in.get();
        if (ch < 0) {
            return;
        }
        {
            word.clear();
            int inlen = 0;
            while (ch != '\n' && ch >= 0) {
                /* Update count of utf-8 characters. */
                if (ch < 0x80 || ch > 0xBF) inlen += 1;
                /* force lower case: */
                if (ch < 128) ch = tolower(ch);

                word += ch;
                ch = f_in.get();
            }
            if (pretty) {
                f_out << word;
            }

            {
                word = stemmer(word);
                if (pretty == 1) {
                    f_out << " -> ";
                } else if (pretty == 2) {
                    if (word.size() > 0) {
                        int j;
                        if (inlen < 30) {
                            for (j = 30 - inlen; j > 0; j--)
                                f_out << ' ';
                        } else {
                            f_out << '\n';
                            for (j = 30; j > 0; j--)
                                f_out << ' ';
                        }
                    }
                }

                f_out << word << '\n';
            }
        }
    }
    return;
}

/** Display the command line syntax, and then exit.
 *  @param n The value to exit with.
 */
static void
usage(int n)
{
    printf("usage: %s [-l <language>] [-i <input file>] [-o <output file>] [-c <character encoding>] [-p[2]] [-h]\n"
          "\n"
          "The input file consists of a list of words to be stemmed, one per\n"
          "line. Words should be in lower case, but (for English) A-Z letters\n"
          "are mapped to their a-z equivalents anyway. If omitted, stdin is\n"
          "used.\n"
          "\n",
          progname);
    printf(
          "If -p is given the output file consists of each word of the input\n"
          "file followed by \"->\" followed by its stemmed equivalent.\n"
          "If -p2 is given the output file is a two column layout containing\n"
          "the input words in the first column and the stemmed equivalents in\n"
          "the second column.\n"
          "Otherwise, the output file consists of the stemmed words, one per\n"
          "line.\n"
          "\n"
          "-h displays this help\n");
    exit(n);
}

int
main(int argc, char * argv[])
try {
    const char * in = NULL;
    const char * out = NULL;
    const char * language = "english";

    int i = 1;
    pretty = 0;

    progname = argv[0];

    while (i < argc) {
        const char * s = argv[i++];
        if (s[0] == '-') {
            if (strcmp(s, "-o") == 0) {
                if (i >= argc) {
                    fprintf(stderr, "%s requires an argument\n", s);
                    exit(1);
                }
                out = argv[i++];
            } else if (strcmp(s, "-i") == 0) {
                if (i >= argc) {
                    fprintf(stderr, "%s requires an argument\n", s);
                    exit(1);
                }
                in = argv[i++];
            } else if (strcmp(s, "-l") == 0) {
                if (i >= argc) {
                    fprintf(stderr, "%s requires an argument\n", s);
                    exit(1);
                }
                language = argv[i++];
            } else if (strcmp(s, "-p2") == 0) {
                pretty = 2;
            } else if (strcmp(s, "-p") == 0) {
                pretty = 1;
            } else if (strcmp(s, "-h") == 0) {
                usage(0);
            } else {
                fprintf(stderr, "option %s unknown\n", s);
                usage(1);
            }
        } else {
            fprintf(stderr, "unexpected parameter %s\n", s);
            usage(1);
        }
    }

    Snowball::Stemmer* stemmer = Snowball::make_stemmer(language);
    if (stemmer == NULL) {
        fprintf(stderr, "language `%s' not available for stemming\n", language);
        exit(1);
    }

    /* prepare the files */
    std::ifstream f_in;
    if (in) {
        f_in.open(in);
        if (!f_in.is_open()) {
            fprintf(stderr, "file %s not found\n", in);
            exit(1);
        }
    }
    std::ofstream f_out;
    if (out) {
        f_out.open(out);
        if (!f_out.is_open()) {
            fprintf(stderr, "file %s cannot be opened\n", out);
            exit(1);
        }
    }

    /* do the stemming process: */
    stem_file(*stemmer, in ? f_in : std::cin, out ? f_out : std::cout);

    return 0;
}
catch (const std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what());
    exit(1);
}
