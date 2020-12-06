/*
 * tiff.c - just a couple of functions that are neither loading nor saving
 */

#include "../base/mdjvucfg.h"

#ifdef HAVE_LIBTIFF
    #include <tiffio.h>
    #define MDJVU_USE_TIFFIO
#endif

#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>

MDJVU_IMPLEMENT int mdjvu_have_tiff_support(void)
{
    #ifdef HAVE_LIBTIFF
        return 1;
    #else
        return 0;
    #endif
}

MDJVU_IMPLEMENT void mdjvu_disable_tiff_warnings(void)
{
    #ifdef HAVE_LIBTIFF
        TIFFSetWarningHandler(NULL);
    #endif
}
