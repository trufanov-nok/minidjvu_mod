/*
 * 2io.h - a stdio wrapper
 */


/* Under Windows there is no libc. So it may well occur that minidjvu-mod uses one
 * stdio implementation, and an application linked against minidjvu-mod uses another.
 * In that case we can't exchange (FILE *) pointers reliably. So here is a simple
 * stdio wrapper to allow applications invoke our stdio.
 * 
 * Also added some integer read/write functions.
 */


/* Structure MinidjvuFile is never defined.
 * Inside the minidjvu-mod library, mdjvu_file_t is FILE *.
 */
typedef struct MinidjvuFile *mdjvu_file_t;

/* These functions just call stdio. So there's no documentation for them. */

MDJVU_FUNCTION mdjvu_file_t mdjvu_fopen(const char *path, const char *mode);
MDJVU_FUNCTION void mdjvu_fclose(mdjvu_file_t);

MDJVU_FUNCTION int32 mdjvu_fread
    (void *, int32 size, int32 n, mdjvu_file_t);
MDJVU_FUNCTION int32 mdjvu_fwrite
    (const void *, int32 size, int32 n, mdjvu_file_t);

MDJVU_FUNCTION void mdjvu_write_big_endian_int32(int32, mdjvu_file_t);
MDJVU_FUNCTION void mdjvu_write_little_endian_int32(int32, mdjvu_file_t);
MDJVU_FUNCTION int32 mdjvu_read_big_endian_int32(mdjvu_file_t);
MDJVU_FUNCTION int32 mdjvu_read_little_endian_int32(mdjvu_file_t);
MDJVU_FUNCTION void mdjvu_write_big_endian_int16(int16, mdjvu_file_t);
MDJVU_FUNCTION void mdjvu_write_little_endian_int16(int16, mdjvu_file_t);
MDJVU_FUNCTION int16 mdjvu_read_big_endian_int16(mdjvu_file_t);
MDJVU_FUNCTION int16 mdjvu_read_little_endian_int16(mdjvu_file_t);
