
#include "libstemmer.h"

/* test code */
void error(const char * err) {
    printf("%s\n", err);
    exit(1);
}

int main () {
    const char * stemmed;
    const char * unstemmed;
    struct sb_stemmer * s;
    
    s = sb_stemmer_create("e");
    if (s != 0) error("TEST FAIL: non zero return for unrecognised language");
    s = sb_stemmer_create("english");
    if (s == 0) error("TEST FAIL: zero return for recognised language");
    unstemmed = "recognised";
    stemmed = sb_stemmer_stem(s, unstemmed, 10);
    printf("%s -> %s\n", unstemmed, stemmed);
    unstemmed = "recognized";
    printf("%s -> %s\n", unstemmed, stemmed);
    sb_stemmer_close(s);
    printf("Success\n");
    return 0;
}
