/*
 * djvuinfo.c - dealing with DjVu INFO chunk
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>

#define DEFAULT_VERSION_STAMP 24
#define DEFAULT_RESOLUTION 300 /* used only if unknown */
#define DEFAULT_GAMMA 278

MDJVU_FUNCTION void mdjvu_write_info_chunk(mdjvu_file_t f, mdjvu_image_t image)
{
    int32 w = mdjvu_image_get_width(image);
    int32 h = mdjvu_image_get_height(image);
    int32 dpi = mdjvu_image_get_resolution(image);

    if (!dpi) dpi = DEFAULT_RESOLUTION;

    mdjvu_write_big_endian_int16((uint16) w, f);
    mdjvu_write_big_endian_int16((uint16) h, f);
    mdjvu_write_little_endian_int16(DEFAULT_VERSION_STAMP, f);
    mdjvu_write_little_endian_int16((uint16) dpi, f);
    mdjvu_write_little_endian_int16(DEFAULT_GAMMA, f);
}

MDJVU_IMPLEMENT void mdjvu_read_info_chunk(mdjvu_file_t f, int32 *pw, int32 *ph, int32 *pdpi)
{
    int16 w = mdjvu_read_big_endian_int16(f);
    int16 h = mdjvu_read_big_endian_int16(f);
    int16 dpi;
    mdjvu_read_little_endian_int16(f); /* version stamp */
    dpi = mdjvu_read_little_endian_int16(f);
    mdjvu_read_little_endian_int16(f); /* gamma */

    if (pw) *pw = w;
    if (ph) *ph = h;
    if (pdpi) *pdpi = dpi;
}
