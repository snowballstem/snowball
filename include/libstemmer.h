
/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct sb_stemmer;
typedef char sb_symbol;

/* FIXME - should be able to get a version number for each stemming
 * algorithm (which will be incremented each time the output changes). */

/** Returns an array of the names of the available stemming algorithms.
 *  Note that these are the canonical names - aliases (ie, other names for
 *  the same algorithm) will not be included in the list.
 *  The list is terminated with a null pointer.
 *
 *  The list must not be modified in any way.
 */
const char ** sb_stemmer_list(void);

/** Create a new stemmer object, using the specified algorithm.
 *
 *  @return If the specified algorithm is not recognised, 0 will be 
 *  returned; otherwise a pointer to a newly created stemmer for that
 *  algorithm will be returned.
 *
 *  @note NULL will also be returned if an out of memory error occurs.
 */
struct sb_stemmer * sb_stemmer_new(const char * algorithm);

/** Delete a stemmer object.
 *
 *  This frees all resources allocated for the stemmer.  After calling
 *  this function, the supplied stemmer may no longer be used in any way.
 *
 *  It is safe to pass a null pointer to this function - this will have
 *  no effect.
 */
void                sb_stemmer_delete(struct sb_stemmer * stemmer);

/** Stem a word.
 *
 *  The return value is owned by the stemmer - it must not be freed or
 *  modified, and it will become invalid when the stemmer is called again,
 *  or if the stemmer is freed.
 *
 *  The length of the return value can be obtained using sb_stemmer_length().
 *
 *  If an out-of-memory error occurs, this will return NULL.
 */
const sb_symbol *   sb_stemmer_stem(struct sb_stemmer * stemmer,
				    const sb_symbol * word, int size);

/** Get the length of the result of the last stemmed word.
 *  This should not be called before sb_stemmer_stem() has been called.
 */
int                 sb_stemmer_length(struct sb_stemmer * stemmer);

#ifdef __cplusplus
}
#endif

