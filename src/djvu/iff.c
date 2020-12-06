/*
 * iff.c - read/write IFF files (DjVu files are IFF)
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>
#include <stdio.h>


static void read_align(FILE *f)
{
    long pos = ftell(f);
    if (pos & 1) fgetc(f);
}

static void write_align(FILE *f)
{
    long pos = ftell(f);
    if (pos & 1) fputc(0, f);
}


typedef struct
{
    int32 id;
    uint32 start;
    uint32 length;
    int write;
} Chunk;

MDJVU_IMPLEMENT int32 mdjvu_iff_get_id(mdjvu_iff_t iff)
{
    return ((Chunk *) iff) -> id;
}

MDJVU_IMPLEMENT int32 mdjvu_iff_get_length(mdjvu_iff_t iff)
{
    return ((Chunk *) iff) -> length;
}

MDJVU_IMPLEMENT mdjvu_iff_t mdjvu_iff_read_chunk(mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    Chunk *chunk = (Chunk *) malloc(sizeof(Chunk));
    read_align(f);
    chunk->id = mdjvu_read_big_endian_int32(file);
    chunk->length = mdjvu_read_big_endian_int32(file);
    chunk->start = ftell(f);
    chunk->write = 0;

    return (mdjvu_iff_t) chunk;
}

MDJVU_IMPLEMENT mdjvu_iff_t mdjvu_iff_write_chunk(int32 id, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    Chunk *chunk = (Chunk *) malloc(sizeof(Chunk));
    write_align(f);
    mdjvu_write_big_endian_int32(id, file);
    mdjvu_write_big_endian_int32(0, file); /* will be length */

    chunk->id = id;
    chunk->start = ftell(f);
    chunk->length = 0;
    chunk->write = 1;

    return (mdjvu_iff_t) chunk;
}

MDJVU_IMPLEMENT void mdjvu_iff_close_chunk(mdjvu_iff_t iff, mdjvu_file_t file)
{
    FILE *f = (FILE *) file;
    Chunk *chunk = (Chunk *) iff;

    if (chunk->write)
    {
        long pos = ftell(f);
        uint32 length = pos - chunk->start;
        fseek(f, chunk->start - 4, SEEK_SET);
        mdjvu_write_big_endian_int32(length, file);
        fseek(f, pos, SEEK_SET);
    }
    else
    {
        fseek(f, chunk->start + chunk->length, SEEK_SET);
    }

    free(chunk);
}
