/* This is a simple program which uses libstemmer to provide a command
 * line interface for stemming using any of the algorithms provided.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strlen, memcmp */

#include "libstemmer.h"

#define EMOJI_FACE_THROWING_A_KISS "\xf0\x9f\x98\x98"
#define U_40079 "\xf1\x80\x81\xb9"
static const struct testcase {
    const char * language;
    const char * charenc;
    const char * input;
    const char * expect;
} testcases[] = {
    { "en", 0,
      "a" EMOJI_FACE_THROWING_A_KISS "ing",
      "a" EMOJI_FACE_THROWING_A_KISS "e" },
    { "en", 0, U_40079 "wing", 0 },
    { 0, 0, 0, 0 }
};

int
main(int argc, char * argv[])
{
    const struct testcase * p;
    (void)argc;
    (void)argv;
    for (p = testcases; p->language; ++p) {
	const char * language = p->language;
	const char * charenc = p->charenc;
	const char * input = p->input;
	const char * expect = p->expect;
	struct sb_stemmer * stemmer = sb_stemmer_new(language, charenc);
	const sb_symbol * stemmed;
	int len;

	if (expect == NULL) expect = input;
	if (stemmer == 0) {
	    if (charenc == NULL) {
		fprintf(stderr, "language `%s' not available for stemming\n", language);
		exit(1);
	    } else {
		fprintf(stderr, "language `%s' not available for stemming in encoding `%s'\n", language, charenc);
		exit(1);
	    }
	}
	stemmed = sb_stemmer_stem(stemmer, (const unsigned char*)input, strlen(input));
	if (stemmed == NULL) {
	    fprintf(stderr, "Out of memory");
	    exit(1);
	}

	len = sb_stemmer_length(stemmer);
	if (len != (int)strlen(expect) || memcmp(stemmed, expect, len) != 0) {
	    fprintf(stderr, "%s stemmer output for %s was %.*s not %s\n",
			    language, input, len, stemmed, expect);
	    exit(1);
	}
	sb_stemmer_delete(stemmer);
    }

    return 0;
}
