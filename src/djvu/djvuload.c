/*
 * djvuload.c - functions to load from single-page DjVuBitonal files
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>
#include <stdio.h>

static uint32 read_uint32_most_significant_byte_first(FILE *f)
{
    uint32 r = fgetc(f) << 24;
    r |= fgetc(f) << 16;
    r |= fgetc(f) << 8;
    r |= fgetc(f);
    return r;
}

typedef struct IFFChunk
{
    uint32 id;
    uint32 length;
    uint32 skipped;
    struct IFFChunk *parent;
} IFFChunk;

static void skip_in_chunk(IFFChunk *chunk, unsigned len)
{
    while (chunk)
    {
        chunk->skipped += len;
        chunk = chunk->parent;
    }
}

static void skip_to_next_sibling_chunk(FILE *file, IFFChunk *chunk)
{
    skip_in_chunk(chunk->parent, chunk->length + 8);
    fseek(file, (chunk->length + 1) & ~1, SEEK_CUR);
    chunk->id = read_uint32_most_significant_byte_first(file);
    chunk->length = read_uint32_most_significant_byte_first(file);
    chunk->skipped = 0;
}

static void get_child_chunk(FILE *file, IFFChunk *chunk, IFFChunk *parent)
{
    chunk->id = read_uint32_most_significant_byte_first(file);
    chunk->length = read_uint32_most_significant_byte_first(file);
    chunk->skipped = 0;
    chunk->parent = parent;
    skip_in_chunk(parent, 8);
}

static int find_sibling_chunk(FILE *file, IFFChunk *chunk, uint32 id)
{
    while (chunk->id != id)
    {
        if (chunk->parent && chunk->parent->skipped >= chunk->parent->length)
            return 0;
        skip_to_next_sibling_chunk(file, chunk);
    }
    return 1;
}

#define CHUNK_ID_AT_AND_T 0x41542654
#define CHUNK_ID_FORM     0x464F524D
#define ID_DJVU           0x444A5655
#define CHUNK_ID_Sjbz     0x536A627A

MDJVU_IMPLEMENT int mdjvu_locate_jb2_chunk(mdjvu_file_t file, int32 *plength, mdjvu_error_t *perr)
{
    IFFChunk FORM, Sjbz;
    FILE *f = (FILE *) file;
    uint32 i = read_uint32_most_significant_byte_first(f);
    if (perr) *perr = NULL;
    if (i != CHUNK_ID_AT_AND_T)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_corrupted_djvu);
        return 0;
    }

    get_child_chunk(f, &FORM, NULL);
    if (!find_sibling_chunk(f, &FORM, CHUNK_ID_FORM))
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_corrupted_djvu);
        return 0;
    }

    if (read_uint32_most_significant_byte_first(f) != ID_DJVU)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_wrong_djvu_type);
        return 0;
    }

    skip_in_chunk(&FORM, 4);

    get_child_chunk(f, &Sjbz, &FORM);
    if (!find_sibling_chunk(f, &Sjbz, CHUNK_ID_Sjbz))
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_djvu_no_Sjbz);
        return 0;
    }

    *plength = (int32) Sjbz.length;

    return 1;
}

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_file_load_djvu_page(mdjvu_file_t file, mdjvu_error_t *perr)
{
    int32 length;
    if (!mdjvu_locate_jb2_chunk(file, &length, perr))
        return NULL;
    return mdjvu_file_load_jb2(file, length, perr);
}

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_load_djvu_page(const char *path, mdjvu_error_t *perr)
{
    mdjvu_image_t result;
    FILE *f = fopen(path, "rb");
    if (perr) *perr = NULL;
    if (!f)
    {
        if (perr) *perr = mdjvu_get_error(mdjvu_error_fopen_read);
        return NULL;
    }
    result = mdjvu_file_load_djvu_page((mdjvu_file_t) f, perr);
    fclose(f);
    return result;
}

