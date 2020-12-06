/*
 * iff.h - read/write IFF files (DjVu files are IFF)
 */


/* IFF is a very simple format that allows some parts of a file to be decorated
 * as "IFF chunks". An IFF chunk looks like this:
 *  __________________________________________     _
 * |                     |    |        |       ...  |
 * | padding (if needed) | ID | length | data  ...  |
 * |_____________________|____|________|______ ... _|
 *
 * IFF chunks may be nested.
 *
 * See DjVu 2 Spec., page 5, "Structure of DjVu files" for deeper description.
 */


/* IFF chunk identifiers are 32-bit integers.
 * The macro is here because MDJVU_IFF_ID("DJVU") looks better than 0x444A5655.
 */
#define MDJVU_IFF_ID(S) \
    ((S)[0] << 24) | \
    ((S)[1] << 16) | \
    ((S)[2] << 8)  | \
     (S)[3]

/* mdjvu_iff_t represents one IFF chunk. */
typedef struct MdjvuNonexistentIffStruct *mdjvu_iff_t;

/* Get the ID of the chunk. */
MDJVU_FUNCTION int32 mdjvu_iff_get_id(mdjvu_iff_t);

/* Get the length of the chunk's data.
 * Useful only when reading; returns 0 if we're writing to the chunk instead.
 */
MDJVU_FUNCTION int32 mdjvu_iff_get_length(mdjvu_iff_t);

/* Opens a chunk for reading. The file must be seekable. */
MDJVU_FUNCTION mdjvu_iff_t mdjvu_iff_read_chunk(mdjvu_file_t);

/* Opens a chunk for writing. The file must be seekable. */
MDJVU_FUNCTION mdjvu_iff_t mdjvu_iff_write_chunk(int32 id, mdjvu_file_t);

/* Closes a chunk.
 * If we're reading, the file cursor is set to the end of chunk.
 * If we're writing, the file cursor must be at the end of chunk before calling.
 *
 * The mdjvu_iff_t object is destroyed.
 *
 * No checks are made to ensure that chunks are closed in the proper order
 * (that is, first opened - last closed). But please do it properly.
 */
MDJVU_FUNCTION void mdjvu_iff_close_chunk(mdjvu_iff_t, mdjvu_file_t);
