/*
 * 3graymap.h - just a couple of functions, possibly will be bigger later
 */


/* There's no special graymap type in minidjvu-mod, and adding it is not planned.
 * So, graymaps are stored in three variables: unsigned char **, int32 and int32
 * (data, width and height).
 * 
 * Overall, graymap support is poor: there is only create/destroy functions.
 * Since no algorithms in minidjvu-mod use graymaps yet, there's no need to have more.
 */


/* Create a two-dimensional array of pixels with initial value 0.
 * The array is in fact one-dimensional, and you may use that.
 * But do not permute the array of row pointers.
 * The returned array must be released by mdjvu_destroy_2d_array().
 */
MDJVU_FUNCTION unsigned char **mdjvu_create_2d_array(int32 w, int32 h);

/* Destroy a two-dimensional pixel array. */
MDJVU_FUNCTION void mdjvu_destroy_2d_array(unsigned char **);
