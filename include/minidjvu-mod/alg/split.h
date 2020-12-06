/*
 * split.h - splitting bitmaps to letters
 */

typedef struct MinidjvuSplitOptions *mdjvu_split_options_t;


MDJVU_FUNCTION mdjvu_split_options_t mdjvu_split_options_create(void);
/*
 * This is only a recomendation, nothing is guaranteed.
 */
MDJVU_FUNCTION void mdjvu_split_options_set_maximum_shape_size(mdjvu_split_options_t, int32 s);
MDJVU_FUNCTION void mdjvu_split_options_destroy(mdjvu_split_options_t);


MDJVU_FUNCTION mdjvu_image_t
    mdjvu_split(mdjvu_bitmap_t, int32 dpi, mdjvu_split_options_t);
