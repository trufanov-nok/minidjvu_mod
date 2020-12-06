/*
 * nosubst.c - guessing what chunks are not letters and should not be changed
 */

/*
 * This implementation is very dumb and sub-optimal,
 * but since it's not a bottleneck, I'm not going to optimize it.
 */


#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <assert.h>


/*
 * returns true if segments [a1, a1 + l1 - 1] and [a2, a2 + l2 - 1] intersect.
 */
static int segments_intersect_or_touch(int32 a1, int32 l1, int32 a2, int32 l2)
{
    return a1 <= a2 + l2 && a2 <= a1 + l1;
}

static int blits_intersect_or_touch(mdjvu_image_t image, int32 b1, int32 b2)
{
    int32 x1 = mdjvu_image_get_blit_x(image, b1);
    int32 x2 = mdjvu_image_get_blit_x(image, b2);
    int32 y1 = mdjvu_image_get_blit_y(image, b1);
    int32 y2 = mdjvu_image_get_blit_y(image, b2);
    mdjvu_bitmap_t bitmap1 = mdjvu_image_get_blit_bitmap(image, b1);
    mdjvu_bitmap_t bitmap2 = mdjvu_image_get_blit_bitmap(image, b2);
    int32 w1 = mdjvu_bitmap_get_width(bitmap1);
    int32 w2 = mdjvu_bitmap_get_width(bitmap2);
    int32 h1 = mdjvu_bitmap_get_height(bitmap1);
    int32 h2 = mdjvu_bitmap_get_height(bitmap2);
    return segments_intersect_or_touch(x1, w1, x2, w2)
        && segments_intersect_or_touch(y1, h1, y2, h2);
}

static void make_no_subst(mdjvu_image_t image, int32 blit)
{
    int32 i, b;
    mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(image, blit);
    if (mdjvu_image_get_not_a_letter_flag(image, bitmap)) return;
    mdjvu_image_set_not_a_letter_flag(image, bitmap, 1);

    /* infect all blits that intersect with this */
    b = mdjvu_image_get_blit_count(image);
    for (i = 0; i < b; i++)
    {
        if (blits_intersect_or_touch(image, blit, i))
            make_no_subst(image, i);
    }
}

MDJVU_IMPLEMENT void mdjvu_calculate_not_a_letter_flags(mdjvu_image_t image)
{
    int32 i, b;
    assert(mdjvu_image_has_suspiciously_big_flags(image));
    mdjvu_image_enable_not_a_letter_flags(image);
    b = mdjvu_image_get_blit_count(image);
    for (i = 0; i < b; i++)
    {
        mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(image, i);
        if (mdjvu_image_get_suspiciously_big_flag(image, bitmap))
            make_no_subst(image, i);
    }
}
