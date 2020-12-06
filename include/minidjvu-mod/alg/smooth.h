/*
 * smooth.h - pre-filtering bitmap before splitting
 */


/*
 * `smooth' is applied to a bitmap even before it is split.
 * 
 * Right now, the algorithm flips pixels which are surrounded
 * by at least 3 of 4 neighboring pixels of another color.
 */

MDJVU_FUNCTION void mdjvu_smooth(mdjvu_bitmap_t b);
