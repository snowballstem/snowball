
/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct sb_stemmer;
typedef char sb_symbol;

/* FIXME - should be able to get a version number for each stemming
 * algorithm (which will be incremented each time the output changes). */
/* FIXME - should be able to get a list of available stemmers. */
struct sb_stemmer * sb_stemmer_new(const char * algorithm);
void                sb_stemmer_delete(struct sb_stemmer * stemmer);

/* FIXME - this should return the length of the stemmed word. */
const sb_symbol *   sb_stemmer_stem(struct sb_stemmer * stemmer,
				    const sb_symbol * word, int size);

#ifdef __cplusplus
}
#endif

