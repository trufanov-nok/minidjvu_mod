/*
 * pbm.c - loading and saving in PBM ("portable bitmap") format
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdio.h>

static void skip_to_the_end_of_line(FILE *file)
{
    while (1)
    {
        switch(fgetc(file))
        {
            case -1: case '\r': case '\n':
                return;
        }
    }
}

static void mdjvu_skip_pbm_whitespace_and_comments(mdjvu_file_t f)
{
    FILE *file = (FILE *) f;
    int c = fgetc(file);
    while(1)
    {
        switch(c)
        {
            case ' ': case '\t': case '\r': case '\n':
                c = fgetc(file);
            break;
            case '#':
                skip_to_the_end_of_line(file);
                c = fgetc(file);
            break;
            default:
                ungetc(c,file);
                return;
        }
    }
}

MDJVU_IMPLEMENT int mdjvu_save_pbm(mdjvu_bitmap_t b, const char *path, mdjvu_error_t *perr)
{
    FILE *file = fopen(path, "wb");
    int result;
    if (perr) *perr = NULL;
    if (!file)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_write);
        return 0;
    }
    result = mdjvu_file_save_pbm(b, (mdjvu_file_t) file, perr);
    fclose(file);
    return result;
}

MDJVU_IMPLEMENT int mdjvu_file_save_pbm(mdjvu_bitmap_t b, mdjvu_file_t f, mdjvu_error_t *perr)
{
    FILE *file = (FILE *) f;
    int32 bytes_per_row = mdjvu_bitmap_get_packed_row_size(b);
    int32 width = mdjvu_bitmap_get_width(b);
    int32 height = mdjvu_bitmap_get_height(b);
    int32 i;

    if (perr) *perr = NULL;

    fprintf(file, "P4\n"MDJVU_INT32_FORMAT" "MDJVU_INT32_FORMAT"\n",
            width, height);

    for (i = 0; i < height; i++)
    {
        unsigned char *row = mdjvu_bitmap_access_packed_row(b, i);
        if (fwrite(row, bytes_per_row, 1, file) != 1)
        {
            if (perr) *perr = mdjvu_get_error(mdjvu_error_io);
            return 0;
        }
    }
    return 1;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_load_pbm(const char *path, mdjvu_error_t *perr)
{
    FILE *file = fopen(path, "rb");
    mdjvu_bitmap_t result;
    if (perr) *perr = NULL;
    if (!file)
    {
        if(perr) *perr = mdjvu_get_error(mdjvu_error_fopen_read);
        return NULL;
    }
    result = mdjvu_file_load_pbm((mdjvu_file_t) file, perr);
    fclose(file);
    return result;
}

#define COMPLAIN \
{ \
    if (perr) *perr = mdjvu_get_error(mdjvu_error_corrupted_pbm); \
    return NULL; \
}
MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_file_load_pbm(mdjvu_file_t f, mdjvu_error_t *perr)
{
    FILE *file = (FILE *) f;
    int32 width, height, bytes_per_row, i;
    mdjvu_bitmap_t result;
    if (perr) *perr = NULL;
    if (fgetc(file) != 'P') COMPLAIN;
    if (fgetc(file) != '4') COMPLAIN;
    mdjvu_skip_pbm_whitespace_and_comments((mdjvu_file_t) file);
    if (fscanf(file,
        MDJVU_INT32_FORMAT" "MDJVU_INT32_FORMAT, &width, &height) != 2)
    {
        COMPLAIN;
    }

    /* a fancy way to write if ( || || || ) - maybe, abandon this switch? */
    switch(fgetc(file))
    {
        case ' ': case '\t': case '\r': case '\n':
            break;
        default:
            COMPLAIN;
    }

    result = mdjvu_bitmap_create(width, height);
    bytes_per_row = mdjvu_bitmap_get_packed_row_size(result);
    for (i = 0; i < height; i++)
    {
        unsigned char *current_row = mdjvu_bitmap_access_packed_row(result, i);
        if (fread(current_row, bytes_per_row, 1, file) != 1)
        {
            mdjvu_bitmap_destroy(result);
            COMPLAIN;
        }
    }
    return result;
}
