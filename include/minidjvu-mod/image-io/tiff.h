/*
 * tiff.h - TIFF support
 */


/* TODO: save resolution */

MDJVU_FUNCTION int mdjvu_save_tiff(mdjvu_bitmap_t, const char *path, mdjvu_error_t *);


/* If the TIFF file has no resolution information,
 * then `resolution' will be unchanged.
 * `resolution' also may be NULL.
 */
MDJVU_FUNCTION mdjvu_bitmap_t mdjvu_load_tiff(const char *path, int32 *resolution, mdjvu_error_t *, uint32 idx);

MDJVU_FUNCTION int mdjvu_have_tiff_support(void);

MDJVU_FUNCTION void mdjvu_disable_tiff_warnings(void);

MDJVU_FUNCTION uint32 mdjvu_get_tiff_page_count(const char *path);
