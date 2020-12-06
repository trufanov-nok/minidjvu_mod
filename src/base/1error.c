/*
 * 1error.c - error handling
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>

MDJVU_IMPLEMENT const char *mdjvu_get_error_message(mdjvu_error_t error)
{
    return (const char *) error;
}

MDJVU_IMPLEMENT mdjvu_error_t mdjvu_get_error(MinidjvuErrorType e)
{
    switch(e)
    {
        case mdjvu_error_fopen_write:
            return (mdjvu_error_t) _("unable to write to file");
        case mdjvu_error_fopen_read:
            return (mdjvu_error_t) _("unable to read from file");
        case mdjvu_error_io:
            return (mdjvu_error_t) _("I/O error");
        case mdjvu_error_corrupted_pbm:
            return (mdjvu_error_t) _("bad PBM file");
        case mdjvu_error_corrupted_bmp:
            return (mdjvu_error_t) _("bad Windows BMP file (perhaps it has non-bitonal data)");
        case mdjvu_error_corrupted_djvu:
            return (mdjvu_error_t) _("bad DjVu file");
        case mdjvu_error_corrupted_jb2:
            return (mdjvu_error_t) _("bad bilevel data in DjVu file");
        case mdjvu_error_corrupted_tiff:
            return (mdjvu_error_t) _("bad TIFF file (perhaps it has non-bitonal data)");
        case mdjvu_error_wrong_djvu_type:
            return (mdjvu_error_t) _("unsupported type of DjVu file");
        case mdjvu_error_djvu_no_Sjbz:
            return (mdjvu_error_t) _("bilevel data not found in DjVu file");
        case mdjvu_error_recursive_prototypes:
            return (mdjvu_error_t) _("somehow prototype references recursed");
        case mdjvu_error_tiff_support_disabled:
            return (mdjvu_error_t) _("minidjvu-mod was compiled without TIFF support");
        case mdjvu_error_png_support_disabled:
            return (mdjvu_error_t) _("minidjvu-mod was compiled without PNG support");
    }
    return (mdjvu_error_t)
        _("some weird error happened, probably caused by a bug in minidjvu-mod");
}
