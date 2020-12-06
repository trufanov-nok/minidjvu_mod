/*
 * 1error.h - error handling
 */

typedef const struct MinidjvuError *mdjvu_error_t;

/* TODO: distinguish non-bitonal and corrupted files */

typedef enum
{
    mdjvu_error_fopen_write,
    mdjvu_error_fopen_read,
    mdjvu_error_io,
    mdjvu_error_corrupted_pbm,
    mdjvu_error_corrupted_bmp,
    mdjvu_error_corrupted_djvu,
    mdjvu_error_corrupted_jb2,
    mdjvu_error_corrupted_tiff,
    mdjvu_error_wrong_djvu_type,
    mdjvu_error_djvu_no_Sjbz,
    mdjvu_error_recursive_prototypes,
    mdjvu_error_tiff_support_disabled,
    mdjvu_error_png_support_disabled
} MinidjvuErrorType;

MDJVU_FUNCTION const char *mdjvu_get_error_message(mdjvu_error_t);
MDJVU_FUNCTION mdjvu_error_t mdjvu_get_error(MinidjvuErrorType);
