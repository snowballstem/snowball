
#include <stdio.h>
#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* memmove */

#include "header.h"

static void merge_part(char * p1, char * p1_end,  /* source 1 */
                       char * p2, char * p2_end,  /* source 2 */
                       char * s,                  /* destination */
                       int unit,                  /* size of each item */
                       int (*f)())                /* comparison routine */
{
    repeat
    {   if (p1 >= p1_end) { memmove(s, p2, p2_end - p2); return; }
        if (p2 >= p2_end) { memmove(s, p1, p1_end - p1); return; }
        if (f(p1, p2) <= 0)
        {   memmove(s, p1, unit);
            p1 += unit;
        } else
        {   memmove(s, p2, unit);
            p2 += unit;
        }
        s += unit;
    }
}

static void merge_all(char * p, char * p_end,  /* source */
                      char * s,                /* destination */
                      int width,               /* size of blocks to merge */
                      int unit,                /* size of each item */
                      int (*f)())              /* comparison routine */
{
    repeat
    {   char * p2 = p + width;
        char * p3 = p2 + width;
        if (p3 > p_end)
        {   p3 = p_end;
            if (p2 > p_end) p2 = p_end;
        }
        merge_part(p, p2, p2, p3, s, unit, f);
        if (p3 == p_end) return;
        p = p3; s += width + width;
    }
}

extern void sort(void * p, void * p_end, int unit, int (*f)()) {
    int size = (char *) p_end - (char *) p;
    char * s = (char *) MALLOC(size);
    char * s_end = s + size;
    int width = unit;
    repeat
    {   merge_all((char *) p, (char *) p_end, s, width, unit, f); width += width;
        if (width >= size) { memmove(p, s, size); break; }
        merge_all(s, s_end, (char *) p, width, unit, f); width += width;
        if (width >= size) break;
    }
    FREE(s);
}

