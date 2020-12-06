/*
 * adjust_y.h - adjust y coordinates of blits so the text won't look bumpy
 */

/*
 * For every blit B pointing to a bitmap with a substitution S,
 * set B pointing to S instead and adjust its X and Y accordingly.
 * X is matched by bounding box centers, but Y is not that simple...
 */
MDJVU_FUNCTION void mdjvu_adjust(mdjvu_image_t image);


/*
 * The same for multipage case.
 * Before the call, pages *may not* contain blits from dictionary bitmaps.
 */
MDJVU_FUNCTION void mdjvu_multipage_adjust(mdjvu_image_t dict,
                                           int32 npages,
                                           mdjvu_image_t *);
