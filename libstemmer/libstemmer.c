
#include "libstemmer.h"
#include "../runtime/api.h"
#include "modules.h"

struct sb_stemmer {
    struct SN_env * (*create)(void);
    void (*close)(struct SN_env *);
    int (*stem)(struct SN_env *);

    struct SN_env * env;
};

/** Create a new stemmer object, using the specified algorithm.
 *
 *  @return If the specified algorithm is not recognised, 0 will be 
 *  returned; otherwise a pointer to a newly created stemmer for that
 *  algorithm will be returned.
 */
struct sb_stemmer * sb_stemmer_new(const char * algorithm)
{
    struct stemmer_modules * module;
    struct sb_stemmer * stemmer =
	    (struct sb_stemmer *) malloc(sizeof(struct sb_stemmer));
    if (stemmer == 0) return 0;

    for (module = modules; module->name != 0; module++) {
	if (strcmp(module->name, algorithm) == 0) break;
    }
    if (module->name == 0) return 0;
    
    stemmer->create = module->create;
    stemmer->close = module->close;
    stemmer->stem = module->stem;

    stemmer->env = stemmer->create();

    return stemmer;
}

/** Delete a stemmer object.
 *
 *  This frees all resources allocated for the stemmer.  After calling
 *  this function, the supplied stemmer may no longer be used in any way.
 *
 *  It is safe to pass a null pointer to this function - this will have
 *  no effect.
 */
void sb_stemmer_delete(struct sb_stemmer * stemmer)
{
    if (stemmer == 0) return;
    if (stemmer->close == 0) return;
    stemmer->close(stemmer->env);
    stemmer->close = 0;
    free(stemmer);
}

/** Stem a word.
 *
 *  The return value is owned by the stemmer - it must not be freed or
 *  modified, and it will become invalid when the stemmer is called again,
 *  or if the stemmer is freed.
 */
const sb_symbol *
sb_stemmer_stem(struct sb_stemmer * stemmer, const sb_symbol * word, int size)
{
    SN_set_current(stemmer->env, size, word);
    (void) stemmer->stem(stemmer->env);
    stemmer->env->p[stemmer->env->l] = 0;
    return stemmer->env->p;
}
