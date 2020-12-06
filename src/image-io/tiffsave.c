/*
 * tiffsave.c - saving TIFF bitmaps
 */

#include "../base/mdjvucfg.h"

#ifdef HAVE_LIBTIFF
    #include <tiffio.h>
    #define MDJVU_USE_TIFFIO
#endif

#include <minidjvu-mod/minidjvu-mod.h>

#ifdef HAVE_LIBTIFF

#ifndef COMPRESSION_PACKBITS
    #define COMPRESSION_PACKBITS 32771
#endif

static int save_tiff(mdjvu_bitmap_t bitmap, const char *path, mdjvu_error_t *perr)
{
    int32 w = mdjvu_bitmap_get_width(bitmap);
    int32 h = mdjvu_bitmap_get_height(bitmap);
    int32 compression = COMPRESSION_NONE;
    int32 i;
    TIFF * tiff;

    *perr = NULL;

    if (TIFFFindCODEC(COMPRESSION_PACKBITS))
        compression = COMPRESSION_PACKBITS;

    tiff = TIFFOpen(path, "w");

    if (!tiff)
    {
        *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }

    /* FIXME: save resolution */
    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, (uint32) w);
    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, (uint32) h);
    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, (uint16) 1);
    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, (uint16) 1);
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);
    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);

    if (mdjvu_bitmap_get_packed_row_size(bitmap) != TIFFScanlineSize(tiff))
    {
        /* FIXME: not very accurate error reporting */
        *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }

    for (i = 0; i < h; i++)
        TIFFWriteScanline(tiff,
                          mdjvu_bitmap_access_packed_row(bitmap, i), i,
                          0);

    TIFFClose(tiff);

    return 1;
}

#endif /* HAVE_LIBTIFF */

MDJVU_IMPLEMENT int mdjvu_save_tiff(mdjvu_bitmap_t bitmap, const char *path, mdjvu_error_t *perr)
{
    #ifdef HAVE_LIBTIFF
        return save_tiff(bitmap, path, perr);
    #else
        *perr = mdjvu_get_error(mdjvu_error_tiff_support_disabled);
        return 0;
    #endif
}
