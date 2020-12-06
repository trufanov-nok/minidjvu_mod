/*
 * 2io.c - a stdio wrapper
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdio.h>

MDJVU_IMPLEMENT mdjvu_file_t mdjvu_fopen(const char *path, const char *mode)
    { return (mdjvu_file_t) fopen(path, mode); }

MDJVU_IMPLEMENT void mdjvu_fclose(mdjvu_file_t f)
    { fclose((FILE *) f); }

MDJVU_IMPLEMENT int32 mdjvu_fread(void *p, int32 size, int32 n, mdjvu_file_t f)
    { return (int32) fread(p, size, n, (FILE *) f); }

MDJVU_IMPLEMENT int32
    mdjvu_fwrite(const void *p, int32 size, int32 n, mdjvu_file_t f)
    { return (int32) fwrite(p, size, n, (FILE *) f); }

MDJVU_IMPLEMENT void mdjvu_write_big_endian_int32(int32 i, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    putc(i >> 24, f);
    putc(i >> 16, f);
    putc(i >> 8, f);
    putc(i, f);
}

MDJVU_IMPLEMENT void mdjvu_write_little_endian_int32(int32 i, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    putc(i, f);
    putc(i >> 8, f);
    putc(i >> 16, f);
    putc(i >> 24, f);
}

MDJVU_IMPLEMENT void mdjvu_write_big_endian_int16(int16 i, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    putc(i >> 8, f);
    putc(i, f);
}

MDJVU_IMPLEMENT void mdjvu_write_little_endian_int16(int16 i, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    putc(i, f);
    putc(i >> 8, f);
}

MDJVU_IMPLEMENT int32 mdjvu_read_big_endian_int32(mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    int32 r = getc(f) << 24;
    r |= getc(f) << 16;
    r |= getc(f) << 8;
    r |= getc(f);
    return r;
}

MDJVU_IMPLEMENT int16 mdjvu_read_big_endian_int16(mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    int16 r = getc(f) << 8;
    r |= getc(f);
    return r;
}

MDJVU_IMPLEMENT int32 mdjvu_read_little_endian_int32(mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    int32 r = getc(f);
    r |= getc(f) << 8;
    r |= getc(f) << 16;
    r |= getc(f) << 24;
    return r;
}

MDJVU_IMPLEMENT int16 mdjvu_read_little_endian_int16(mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    int32 r = getc(f);
    r |= getc(f) << 8;
    return r;
}
