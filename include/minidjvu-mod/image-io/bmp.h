/*
 * bmp.h - saving bitmaps in Windows BMP format
 */

/*
 * 1 - success, 0 - failure
 */
MDJVU_FUNCTION int mdjvu_save_bmp(mdjvu_bitmap_t, const char *path, int32 resolution, mdjvu_error_t *);
MDJVU_FUNCTION int mdjvu_file_save_bmp(mdjvu_bitmap_t, mdjvu_file_t, int32 resolution, mdjvu_error_t *);

/*
 * These functions return NULL if failed
 */
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_load_bmp(const char *path, mdjvu_error_t *);
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_file_load_bmp(mdjvu_file_t, mdjvu_error_t *);
