/*
 * 5image.h - manipulating split images, the main data structure of minidjvu
 */

typedef struct MinidjvuImage *mdjvu_image_t;

/* Create a split image with the given read-only parameters.
 * The created image will not contain any blits or bitmaps.
 * Resolution is set to default (300 dpi) and may be changed later.
 */
MDJVU_FUNCTION mdjvu_image_t mdjvu_image_create(int32 width, int32 height);

/* Returns size of an image in memory in bytes. */
MDJVU_FUNCTION int mdjvu_image_mem_size(mdjvu_image_t img);

/* Destroy a split image, freeing all its bitmaps and blits. */
MDJVU_FUNCTION void mdjvu_image_destroy(mdjvu_image_t);

/* Get the width of a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_width(mdjvu_image_t);

/* Get the height of a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_height(mdjvu_image_t);

/* Free some memory by indication that no additions to the image are planned.
 * You still may do any additions after freezing.
 * Calling freeze() after every addition is possible, but inefficient.
 */
MDJVU_FUNCTION void mdjvu_image_freeze(mdjvu_image_t);

/* Remove white margins from each bitmap and adjust blits accordingly. */
MDJVU_FUNCTION void mdjvu_image_remove_bitmap_margins(mdjvu_image_t);


/* _______________________________   blits   _______________________________ */

/* Get the number of blits in a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_blit_count(mdjvu_image_t);

MDJVU_FUNCTION int32 mdjvu_image_get_blit_x(mdjvu_image_t, int32 blit_index);
MDJVU_FUNCTION int32 mdjvu_image_get_blit_y(mdjvu_image_t, int32 blit_index);

MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_image_get_blit_bitmap(mdjvu_image_t, int32 blit_index);

MDJVU_FUNCTION void mdjvu_image_set_blit_x(mdjvu_image_t, int32 blit_index, int32 x);
MDJVU_FUNCTION void mdjvu_image_set_blit_y(mdjvu_image_t, int32 blit_index, int32 y);

MDJVU_FUNCTION void mdjvu_image_set_blit_bitmap(mdjvu_image_t, int32 blit_index, mdjvu_bitmap_t);

MDJVU_FUNCTION int32 mdjvu_image_add_blit(mdjvu_image_t, int32 x, int32 y, mdjvu_bitmap_t);

MDJVU_FUNCTION void mdjvu_image_exchange_blits
    (mdjvu_image_t, int32, int32);

/* It's a good idea to remove NULL blits (blits with NULL blit bitmap) as soon
 * as possible because such jokes are poorly understood by most algorithms.
 */
MDJVU_FUNCTION void mdjvu_image_remove_NULL_blits(mdjvu_image_t);

/* _________________________   bitmaps in an image   _______________________ */

/* Get the number of bitmaps in a split image. */
MDJVU_FUNCTION int32 mdjvu_image_get_bitmap_count(mdjvu_image_t);

/* Get a bitmap by index. (index may be from 0 to get_bitmap_count() - 1. */
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_image_get_bitmap(mdjvu_image_t, int32);

/* Test if the bitmap belongs to this image (uses constant time). */
MDJVU_FUNCTION int mdjvu_image_has_bitmap(mdjvu_image_t image, mdjvu_bitmap_t bitmap);

/* Append a bitmap. */
MDJVU_FUNCTION int32 mdjvu_image_add_bitmap(mdjvu_image_t, mdjvu_bitmap_t);

/* Exchange two bitmaps (bitmap pointers) and all additional info they have.
 * Does not change the image look; does not touch blits.
 */
MDJVU_FUNCTION void mdjvu_image_exchange_bitmaps(mdjvu_image_t, int32, int32);

/* Create a new bitmap and add it. */
MDJVU_FUNCTION mdjvu_bitmap_t
mdjvu_image_new_bitmap(mdjvu_image_t, int32 w, int32 h);

MDJVU_FUNCTION void mdjvu_image_delete_bitmap(mdjvu_image_t, mdjvu_bitmap_t);

/*
 * Returns 1 if the bitmap indices are set correctly.
 * (for using with assert())
 */
MDJVU_FUNCTION int mdjvu_image_check_indices(mdjvu_image_t);

/*
 * Don't run this on dictionaries.
 * Also, this doesn't detect usage by substitutions,
 * so be sure to run mdjvu_adjust() before.
 */
MDJVU_FUNCTION void mdjvu_image_remove_unused_bitmaps(mdjvu_image_t);

/* ______________________   additional info for images   ___________________ */

MDJVU_FUNCTION int32 mdjvu_image_get_resolution(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_set_resolution(mdjvu_image_t, int32 dpi);
MDJVU_FUNCTION void mdjvu_image_set_dictionary(mdjvu_image_t, mdjvu_image_t);
MDJVU_FUNCTION mdjvu_image_t mdjvu_image_get_dictionary(mdjvu_image_t);

/* ______________________   additional info for bitmaps   __________________ */


/* Prototypes */

MDJVU_FUNCTION int mdjvu_image_has_prototypes
    (mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_enable_prototypes
    (mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_disable_prototypes
    (mdjvu_image_t);
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_image_get_prototype
    (mdjvu_image_t, mdjvu_bitmap_t);
MDJVU_FUNCTION void mdjvu_image_set_prototype
    (mdjvu_image_t, mdjvu_bitmap_t, mdjvu_bitmap_t prototype);

/* Substitutions */
MDJVU_FUNCTION int mdjvu_image_has_substitutions(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_enable_substitutions(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_disable_substitutions(mdjvu_image_t);
MDJVU_FUNCTION mdjvu_bitmap_t
    mdjvu_image_get_substitution(mdjvu_image_t, mdjvu_bitmap_t);
MDJVU_FUNCTION void
    mdjvu_image_set_substitution(mdjvu_image_t, mdjvu_bitmap_t, mdjvu_bitmap_t);

/* no-substitution flag
 * bitmaps having this flag can't have a substitution
 * and can't serve as substitution to others.
 * This is for small pieces of large picture, which are not to be displaced.
 */
MDJVU_FUNCTION int mdjvu_image_has_not_a_letter_flags(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_enable_not_a_letter_flags(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_disable_not_a_letter_flags(mdjvu_image_t);
MDJVU_FUNCTION int mdjvu_image_get_not_a_letter_flag
    (mdjvu_image_t, mdjvu_bitmap_t);
MDJVU_FUNCTION void mdjvu_image_set_not_a_letter_flag
    (mdjvu_image_t, mdjvu_bitmap_t, int new_val);

/* suspiciously big flag */
MDJVU_FUNCTION int mdjvu_image_has_suspiciously_big_flags(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_enable_suspiciously_big_flags(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_disable_suspiciously_big_flags(mdjvu_image_t);
MDJVU_FUNCTION int mdjvu_image_get_suspiciously_big_flag
    (mdjvu_image_t, mdjvu_bitmap_t);
MDJVU_FUNCTION void mdjvu_image_set_suspiciously_big_flag
    (mdjvu_image_t, mdjvu_bitmap_t, int new_val);


/* masses */

MDJVU_FUNCTION int mdjvu_image_has_masses(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_enable_masses(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_disable_masses(mdjvu_image_t);
MDJVU_FUNCTION int32 mdjvu_image_get_mass(mdjvu_image_t, mdjvu_bitmap_t);

/* centers */
/* Centers are stored in 1/MDJVU_CENTER_QUANT of pixels.
 * A center is found by pattern matcher.
 */
#define MDJVU_CENTER_QUANT 8
MDJVU_FUNCTION int mdjvu_image_has_centers(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_enable_centers(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_disable_centers(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_get_center(mdjvu_image_t, mdjvu_bitmap_t, int32 *px, int32 *py);
MDJVU_FUNCTION void mdjvu_image_set_center(mdjvu_image_t, mdjvu_bitmap_t, int32 x, int32 y);


/* dictionary index */
MDJVU_FUNCTION int mdjvu_image_has_dictionary_indices(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_enable_dictionary_indices(mdjvu_image_t);
MDJVU_FUNCTION void mdjvu_image_disable_dictionary_indices(mdjvu_image_t);
MDJVU_FUNCTION int32 mdjvu_image_get_dictionary_index(mdjvu_image_t, mdjvu_bitmap_t);
MDJVU_FUNCTION void mdjvu_image_set_dictionary_index(mdjvu_image_t image, mdjvu_bitmap_t b, int32 v);

/* sort bitmaps according to blits */
MDJVU_FUNCTION void mdjvu_image_sort_bitmaps(mdjvu_image_t img);


/* _______________________________   wrapper   _____________________________ */

/* DO NOT USE THIS YET */

#ifdef MINIDJVU_WRAPPERS

    struct MinidjvuImage
    {
         inline int32 get_resolution()
             {return mdjvu_image_get_resolution(this);}
         inline void set_resolution(int32 dpi)
             {return mdjvu_image_set_resolution(this, dpi);}
    };

#endif
