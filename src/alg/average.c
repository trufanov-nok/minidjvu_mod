/*
 * average.c - computing average bitmap
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_average(mdjvu_bitmap_t *bitmaps,
                                             int32 n,
                                             int32 *cx, int32 *cy)
{
    int32 i;
    int32 min_x = 0, min_y = 0, max_x_plus_1 = 0, max_y_plus_1 = 0;
    int32 *buf;
    int32 buf_w, buf_h;
    unsigned char *row;
    int32 tmp_x, tmp_y;
    int32 threshold = n / 2;
    mdjvu_bitmap_t result;

    if (n == 1)
    {
        return mdjvu_bitmap_clone(bitmaps[0]);
    }

    for (i = 0; i < n; i++)
    {
        int32 w = mdjvu_bitmap_get_width(bitmaps[i]);
        int32 h = mdjvu_bitmap_get_height(bitmaps[i]);
        int32 ncx = cx[i] / MDJVU_CENTER_QUANT;
        int32 ncy = cy[i] / MDJVU_CENTER_QUANT;
        
        assert(ncx >= 0 && ncx < w);
        assert(ncy >= 0 && ncy < h);

        if (-ncx < min_x) min_x = -ncx;
        if (-ncy < min_y) min_y = -ncy;
        if (w-ncx > max_x_plus_1) max_x_plus_1 = w-ncx;
        if (h-ncy > max_y_plus_1) max_y_plus_1 = h-ncy;
    }

    buf_w = max_x_plus_1 - min_x;
    buf_h = max_y_plus_1 - min_y;
    buf = (int32 *) calloc(buf_w * buf_h, sizeof(int32));
    row = (unsigned char *) malloc(buf_w);

    /* Now adding the bitmaps to the buffer */
    for (i = 0; i < n; i++)
    {
        int32 w = mdjvu_bitmap_get_width(bitmaps[i]);
        int32 h = mdjvu_bitmap_get_height(bitmaps[i]);
        int32 sx = min_x + cx[i] / MDJVU_CENTER_QUANT, sy = min_y + cy[i] / MDJVU_CENTER_QUANT;
        int32 x, y;

        for (y = 0; y < h; y++)
        {
            int32 *buf_row = buf + buf_w * (y - sy);
            mdjvu_bitmap_unpack_row(bitmaps[i], row, y);
            for (x = 0; x < w; x++)
            {
                if (row[x])
                    buf_row[x - sx]++;
            }
        }
    }

    result = mdjvu_bitmap_create(buf_w, buf_h);
    for (i = 0; i < buf_h; i++)
    {
        int32 j;
        for (j = 0; j < buf_w; j++)
        {
            row[j] = ( buf[i * buf_w + j] > threshold ? 1 : 0 );
        }
        mdjvu_bitmap_pack_row(result, row, i);
    }

    mdjvu_bitmap_remove_margins(result, &tmp_x, &tmp_y);

    free(row);
    free(buf);

    return result;
}
