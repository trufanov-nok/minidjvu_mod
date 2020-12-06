/*
 * compress.h - using all the stuff to compress
 */

typedef struct MinidjvuCompressionOptions *mdjvu_compression_options_t;

/*
 * By default, options correspond to `minidjvu-mod' run with no options.
 * That is, lossless encoding and not verbose.
 */
MDJVU_FUNCTION mdjvu_compression_options_t mdjvu_compression_options_create(void);
MDJVU_FUNCTION void mdjvu_compression_options_destroy(mdjvu_compression_options_t);

/*
 * This function gives sets matcher options.
 * To disable the matcher, pass NULL here (it's the default).
 * By calling this, you give the matcher options into ownership.
 * That is, DON'T destroy matcher options afterwards.
 */
MDJVU_FUNCTION void mdjvu_set_matcher_options(mdjvu_compression_options_t, mdjvu_matcher_options_t);
MDJVU_FUNCTION void mdjvu_set_clean(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_verbose(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_no_prototypes(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_averaging(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_report(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_report_start_page(mdjvu_compression_options_t, int);
MDJVU_FUNCTION void mdjvu_set_report_total_pages(mdjvu_compression_options_t, int);

MDJVU_FUNCTION void mdjvu_compress_image(mdjvu_image_t, mdjvu_compression_options_t);
MDJVU_FUNCTION mdjvu_image_t mdjvu_compress_multipage(int n, mdjvu_image_t *pages, mdjvu_compression_options_t);
