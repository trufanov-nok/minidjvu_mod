/*
 * render.c - rendering a split image into a bitmap
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>
#include <string.h>

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_render(mdjvu_image_t img)
{
    int32 width  = mdjvu_image_get_width (img);
    int32 height = mdjvu_image_get_height(img);
    unsigned char **b = mdjvu_create_2d_array(width, height);
    unsigned char *row_buffer = (unsigned char *) malloc(width);
    int32 blit_count = mdjvu_image_get_blit_count(img);
    int32 i;
    mdjvu_bitmap_t result = mdjvu_bitmap_create(width, height);

    /* Fill the canvas with white */
    for (i = 0; i < height; i++)
        memset(b[i], 0, width);

    /* Render the split image blit by blit */
    for (i = 0; i < blit_count; i++)
    {
        int32 x = mdjvu_image_get_blit_x(img, i);
        int32 y = mdjvu_image_get_blit_y(img, i);
        mdjvu_bitmap_t current_bitmap = mdjvu_image_get_blit_bitmap(img, i);
        int32 w = mdjvu_bitmap_get_width(current_bitmap);
        int32 h = mdjvu_bitmap_get_height(current_bitmap);

        /* Now w and h are dimensions of the current shape,
         * and width and height are dimensions of the whole image.
         */

        int32 min_col = x >= 0 ? 0 : - x;
        int32 max_col_plus_one = x + w <= width ? w : width - x;

        int32 min_row = y >= 0 ? 0 : - y;
        int32 max_row_plus_one = y + h <= height ? h : height - y;

        int32 row;

        /* Render the current blit row by row */
        for (row = min_row; row < max_row_plus_one; row++)
        {
            int32 col;
            unsigned char *target = b[y + row] + x;
            mdjvu_bitmap_unpack_row(current_bitmap, row_buffer, row);
            for (col = min_col; col < max_col_plus_one; col++)
                target[col] |= row_buffer[col];
        }
    }

    free(row_buffer);

    /* Convert 2D array to a Bitmap and return it */
    mdjvu_bitmap_pack_all(result, b);
    mdjvu_destroy_2d_array(b);
    return result;
}
