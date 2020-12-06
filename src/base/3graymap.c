/*
 * 3graymap.c - very simple 2d array handling
 */

#include "mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>

MDJVU_IMPLEMENT unsigned char **mdjvu_create_2d_array(int32 w, int32 h)
{
    int32 i;
    unsigned char *data, **result;
    result = (unsigned char **) calloc(h, sizeof(unsigned char *) + w);
    data = (unsigned char *) (result + h);

    for (i = 0; i < h; i++)
    {
        result[i] = data + w * i;
    }

    return result;
}

MDJVU_IMPLEMENT void mdjvu_destroy_2d_array(unsigned char **p)
{
    free(p);
}
