/*
 * jb2.h - functions to load from JB2 raw streams (that's part of DjVu format)
 */


/*
 * These functions return NULL if failed to read JB2.
 * Loading jb2 by path is not supported.
 */
MDJVU_FUNCTION mdjvu_image_t mdjvu_file_load_jb2(mdjvu_file_t, int32 length, mdjvu_error_t *);

/*
 * 1 - success, 0 - error
 * Cannot save images that use shared dictionary.
 * With erosion turned on, this function changes the image.
 */
MDJVU_FUNCTION int mdjvu_save_jb2(mdjvu_image_t, const char *path, mdjvu_error_t *, int erosion);
MDJVU_FUNCTION int mdjvu_file_save_jb2(mdjvu_image_t, mdjvu_file_t, mdjvu_error_t *, int erosion);

MDJVU_FUNCTION int mdjvu_save_jb2_dictionary(mdjvu_image_t, const char *path, mdjvu_error_t *, int erosion);
MDJVU_FUNCTION int mdjvu_file_save_jb2_dictionary(mdjvu_image_t, mdjvu_file_t, mdjvu_error_t *, int erosion);


/*
 * This is called automatically by xxx_save_jb2() functions.
 * This function finds "cross-coding prototypes" (see DjVu spec).
 * It's VERY SLOW.
 */

MDJVU_FUNCTION void mdjvu_find_prototypes(mdjvu_image_t);

/*
 * This is the multipage version. Does not search prototypes in the dictionary.
 * Is not invoked by xxx_save_jb2().
 */
MDJVU_FUNCTION void mdjvu_multipage_find_prototypes
    (mdjvu_image_t dict, int32 npages, mdjvu_image_t *pages,
     void (*report)(void *param, int page), void *param);
