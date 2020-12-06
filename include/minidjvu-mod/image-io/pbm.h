/*
 * pbm.h - loading and saving in PBM ("portable bitmap") format
 */

/*
 * 1 - success, 0 - failure
 */
MDJVU_FUNCTION int mdjvu_save_pbm(mdjvu_bitmap_t, const char *path, mdjvu_error_t *);
MDJVU_FUNCTION int mdjvu_file_save_pbm(mdjvu_bitmap_t, mdjvu_file_t, mdjvu_error_t *);

/*
 * These functions return NULL if failed
 */
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_load_pbm(const char *path, mdjvu_error_t *);
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_file_load_pbm(mdjvu_file_t, mdjvu_error_t *);
