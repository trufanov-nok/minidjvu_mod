/*
 * average.h - computing average bitmap
 */


/* Compute an "average" bitmap and return it.
 * The result is to be destroyed with mdjvu_bitmap_destroy().
 * 
 * Additionally, centers must be given.
 */
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_average(mdjvu_bitmap_t *bitmaps,
                                            int32 n,
                                            int32 *centers_x,
                                            int32 *centers_y);
