/*
 * jb2load.cpp - loading JB2 files by interpreting them record-by-record
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>
#include <string.h>
#include "zp.h"
#include "jb2coder.h"

// A piece of my old code... should it be eliminated?
template<class T> inline T *
append_to_list(T *&list, int32 &count, int32 &allocated)
{
    if (allocated == count)
    {
        allocated <<= 1;
        list = (T *) realloc(list, allocated * sizeof(T));
    }
    return &list[count++];
}

static mdjvu_bitmap_t decode_lib_shape/*{{{*/
    (JB2Decoder &jb2, mdjvu_image_t img, bool with_blit, mdjvu_bitmap_t proto)
{
    int32 blit = -1; // to please compilers
    int32 index = mdjvu_image_get_bitmap_count(img);

    mdjvu_bitmap_t shape = jb2.decode(img, proto);
    if (with_blit)
    {
        blit = jb2.decode_blit(img, index);
    }

    int32 x, y;
    mdjvu_bitmap_remove_margins(shape, &x, &y);

    if (with_blit)
    {
        mdjvu_image_set_blit_x(img, blit, mdjvu_image_get_blit_x(img, blit) + x);
        mdjvu_image_set_blit_y(img, blit, mdjvu_image_get_blit_y(img, blit) + y);
    }

    return shape;
}/*}}}*/

#define COMPLAIN \
{ \
    if (perr) *perr = mdjvu_get_error(mdjvu_error_corrupted_jb2); \
    return NULL; \
}
MDJVU_IMPLEMENT mdjvu_image_t mdjvu_file_load_jb2(mdjvu_file_t file, int32 length, mdjvu_error_t *perr)/*{{{*/
{
    if (perr) *perr = NULL;
    FILE *f = (FILE *) file;
    JB2Decoder jb2(f, length);
    ZPDecoder &zp = jb2.zp;

    int32 d = 0;
    int32 t = jb2.decode_record_type();

    if (t == jb2_require_dictionary_or_reset)
    {
        d = zp.decode(jb2.required_dictionary_size);
        t = jb2.decode_record_type();
    }

    if (t != jb2_start_of_image) COMPLAIN;

    int32 w = zp.decode(jb2.image_size);
    int32 h = zp.decode(jb2.image_size);
    zp.decode(jb2.eventual_image_refinement); // dropped
    jb2.symbol_column_number.set_interval(1, w);
    jb2.symbol_row_number.set_interval(1, h);

    mdjvu_image_t img = mdjvu_image_create(w, h); /* d is dropped for now - XXX*/

    int32 lib_count = 0, lib_alloc = 128;
    mdjvu_bitmap_t *library;

    library = (mdjvu_bitmap_t *) malloc(lib_alloc * sizeof(mdjvu_bitmap_t));

    while(1)
    {
        t = jb2.decode_record_type();
        switch(t)
        {
            case jb2_new_symbol_add_to_image_and_library:
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, true, NULL);
            break;
            case jb2_new_symbol_add_to_library_only:
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, false, NULL);
            break;
            case jb2_new_symbol_add_to_image_only:
            {
                jb2.decode(img);
                jb2.decode_blit(img, mdjvu_image_get_bitmap_count(img)-1);
            }
            break;
            case jb2_matched_symbol_with_refinement_add_to_image_and_library:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    COMPLAIN;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, true, library[match]);
            }
            break;
            case jb2_matched_symbol_with_refinement_add_to_library_only:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    COMPLAIN;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                *(append_to_list<mdjvu_bitmap_t>(library, lib_count, lib_alloc))
                 = decode_lib_shape(jb2, img, false, library[match]);
            }
            break;
            case jb2_matched_symbol_with_refinement_add_to_image_only:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    COMPLAIN;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                jb2.decode(img, library[match]);
                jb2.decode_blit(img, mdjvu_image_get_bitmap_count(img)-1);
            }
            break;
            case jb2_matched_symbol_copy_to_image_without_refinement:
            {
                if (!lib_count)
                {
                    mdjvu_image_destroy(img);
                    free(library);
                    COMPLAIN;
                }
                jb2.matching_symbol_index.set_interval(0, lib_count - 1);
                int32 match = zp.decode(jb2.matching_symbol_index);
                jb2.decode_blit(img, match);
            }
            break;
            case jb2_non_symbol_data:
            {
                mdjvu_bitmap_t bmp = jb2.decode(img);
                int32 x = zp.decode(jb2.symbol_column_number) - 1;
                int32 y = h - zp.decode(jb2.symbol_row_number);
                mdjvu_image_add_blit(img, x, y, bmp);
            }
            break;

            case jb2_require_dictionary_or_reset:
                jb2.reset();
            break;

            case jb2_comment:
            {
                int32 len = zp.decode(jb2.comment_length);
                while (len--) zp.decode(jb2.comment_octet);
            }
            break;

            case jb2_end_of_data:
                free(library);
                return img;
            default:
                free(library);
                mdjvu_image_destroy(img);
                COMPLAIN;
        } // switch
    } // while(1)
}/*}}}*/
