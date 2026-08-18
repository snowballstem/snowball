
#include <stdlib.h> /* for malloc, free */
#include <string.h> /* for memset */
#include "snowball_runtime.h"

extern struct SN_env * SN_new_env(int alloc_size)
{
    struct SN_env * z = (struct SN_env *) malloc(alloc_size);
    if (z == NULL) return NULL;
    /* This memset() will initialise z->p to all-bits-zero which (at least
     * theoretically) may not be the NULL pointer representation, but that's OK
     * as immediately afterwards we assign to z->p.
     */
    memset(z, 0, alloc_size);
    z->p = create_s();
    if (z->p == NULL) {
        SN_delete_env(z);
        return NULL;
    }
    return z;
}

extern void SN_delete_env(struct SN_env * z)
{
    if (z == NULL) return;
    if (z->p) lose_s(z->p);
    free(z);
}

extern int SN_set_current(struct SN_env * z, int size, const symbol * s)
{
    int err = replace_s(z, 0, z->l, size, s);
    z->c = 0;
    return err;
}
