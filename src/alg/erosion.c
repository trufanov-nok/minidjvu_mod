/*
 * erosion.c - determine candidates for flipping by image encoder
 */


#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>


/* ERROR ALERT:
 * We should never declare border pixels
 * as erosion candidates or otherwise bounding boxes may change
 * with unpredictable consequences.
 * (this implementation is OK)
 */

/*
 * it all resembles `smooth' algorithm very much.
 */

/*
 * This implementation permits flipping of a pixel
 * when it has exactly 2 of 4 edge neighbors black
 *         and exactly 2 of 4 vertex (diagonal) neighbors black.
 */


/* all input rows must be 0-or-1 unpacked */
static void get_erosion_candidates_in_a_row(
                       unsigned char *r, /* result    */
                       unsigned char *u, /* upper row */
                       unsigned char *t, /* this row */
                       unsigned char *l, /* lower row */
                       int32 n)
{
    int32 i;
    n--;
    r[0] = 0;
    r[n] = 0;
    for (i = 1; i < n; i++)
    {
        int score_4 = u[i] + l[i] + t[i-1] + t[i+1] - 2;
        int score_d = u[i-1] + l[i-1] + u[i+1] + l[i+1] - 2;

        r[i] = !score_4 && !score_d ? 1 : 0;
    }
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_get_erosion_mask(mdjvu_bitmap_t bmp)
{
    int32 w = mdjvu_bitmap_get_width(bmp);
    int32 h = mdjvu_bitmap_get_height(bmp);
    mdjvu_bitmap_t result = mdjvu_bitmap_create(w, h);
    int32 i;
    unsigned char *u, *t, *l, *r;

    if (h < 3) return result;

    u = (unsigned char *) malloc(w); /* upper row */
    t = (unsigned char *) malloc(w); /* this row */
    l = (unsigned char *) malloc(w); /* lower row */
    r = (unsigned char *) malloc(w); /* result */

    mdjvu_bitmap_unpack_row_0_or_1(bmp, t, 0);
    mdjvu_bitmap_unpack_row_0_or_1(bmp, l, 1);
    for (i = 1; i < h - 1; i++)
    {
        unsigned char *tmp = u;
        u = t;
        t = l;
        l = tmp;

        mdjvu_bitmap_unpack_row_0_or_1(bmp, l, i + 1);

        get_erosion_candidates_in_a_row(r, u, t, l, w);
        mdjvu_bitmap_pack_row(result, r, i);
    }

    free(u);
    free(t);
    free(l);
    free(r);

    return result;
}
