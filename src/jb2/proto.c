/*
 * proto.c - searching for prototypes
 */

#include "../base/mdjvucfg.h"
#include <minidjvu/minidjvu.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h> // macros __BYTE_ORDER

#define THRESHOLD 21

static const int32 bigint = INT32_MAX / 100 - 1;

// Generate a lookup table for 32 bit integers
#define B2(n) n, n + 1, n + 1, n + 2
#define B4(n) B2(n), B2(n + 1), B2(n + 1), B2(n + 2)
#define B6(n) B4(n), B4(n + 1), B4(n + 1), B4(n + 2)

// Lookup table that store the sum of bits for all uchar values
const unsigned int lookuptable[256] = { B6(0), B6(1), B6(1), B6(2) };
#include <endian.h>
inline int sum8(unsigned char val) { return lookuptable[val]; }
inline int sum16(int16 val) { unsigned char* c = (unsigned char *) &val; return lookuptable[c[0]] + lookuptable[c[1]]; }
inline int sum32(int32 val) { unsigned char* c = (unsigned char *) &val; return lookuptable[c[0]] + lookuptable[c[1]] + lookuptable[c[2]] + lookuptable[c[3]]; }

#if __BYTE_ORDER == __BIG_ENDIAN
inline uint32 get_int32(uint32*  a) { return *a; }
inline uint16 get_int16(uint16*  a) { return *a; }
#elif __BYTE_ORDER == __LITTLE_ENDIAN
inline uint32 get_int32(unsigned char*  a) { return a[0] << 24 | a[1] << 16 | a[2] << 8 | a[3]; }
inline uint16 get_int16(unsigned char*  a) { return a[0] << 8  | a[1]; }
#endif

static int diff_bits_to_zero(unsigned char * a, int a_len)
{
    // non-meaning bits in last bytes must be 0s
    int res = 0;

    const int a_len_int32 = a_len / 32;
    const int a_len_int16 = a_len / 16;
    const int a_len_int8  = a_len / 8;
    const int a_len_tail8 = a_len % 8;

    const char a_tail_mask = 0xFF << (8 - a_len_tail8);


    uint32 * a_32 = (uint32 *) a;
    uint16 * a_16 = (uint16 *) a;

    for (int i = 0; i < a_len_int32; i++) {
        res += sum32(a_32[i]);
    }

    if (a_len_int16 & 1) {
        res += sum16(a_16[a_len_int16-1]);
    }

    if (a_len_int8 & 1) {
        res += sum8(a[a_len_int8-1]);
    }

    if (a_len_tail8) {
        res += sum8( a[a_len_int8] & a_tail_mask);
    }

    return res;
}


static int diff_bits_no_shift(unsigned char * a, int a_len, unsigned char * b, int b_len)
{
    // abs(a_len-b_len) must be < 32
    // non-meaning bits in last bytes must be 0s
    int res = 0;

    const int min_len = a_len < b_len ? a_len : b_len;
    const int min_len_int32 = min_len / 32;
    const int min_len_int16 = min_len / 16;
    const int min_len_int8  = min_len / 8;
    const int a_tail = (a_len - min_len_int8 * 8);
    const int b_tail = (b_len - min_len_int8 * 8);
    const int a_tail_int8 = (a_tail + 7) / 8;
    const int b_tail_int8 = (b_tail + 7) / 8;

    uint32 * a_32 = (uint32 *) a;
    uint32 * b_32 = (uint32 *) b;
    uint16 * a_16 = (uint16 *) a;
    uint16 * b_16 = (uint16 *) b;

    for (int i = 0; i < min_len_int32; i++) {
        res += sum32( a_32[i] ^ b_32[i] );
    }

    if (min_len_int16 & 1) {
        res += sum16(a_16[min_len_int16-1] ^ b_16[min_len_int16-1]);
    }

    if (min_len_int8 & 1) {
        res += sum8(a[min_len_int8-1] ^ b[min_len_int8-1]);
    }

    uint32 a_tail_buf = 0;
    uint32 b_tail_buf = 0;

    if (a_tail) {
        memcpy(&a_tail_buf, a + min_len_int8, a_tail_int8);
        a_tail_buf = get_int32((unsigned char *)&a_tail_buf) & (0xFFFFFFFF << (32 - a_tail));
    }
    if (b_tail) {
        memcpy(&b_tail_buf, b + min_len_int8, b_tail_int8);
        b_tail_buf = get_int32((unsigned char *)&b_tail_buf) & (0xFFFFFFFF << (32 - b_tail));
    }

    res += sum32(a_tail_buf ^ b_tail_buf);

    return res;
}


static int diff_bits_shifted(unsigned char * a, int a_len, unsigned char * b, int b_len, int shift)
{
    // abs(a_len-b_len) must be < 24
    // shift must be < 8
    // shifted array must be "a", only one array is shifted
    // non-meaning bits in last bytes must be 0s

    int res = 0;

    const int a_len_shifted = a_len + shift;
    const int min_len = a_len_shifted < b_len ? a_len_shifted : b_len;
    const int min_len_int32 = min_len / 32;
    const int min_len_int16 = min_len / 16;
    const int min_len_int8  = min_len / 8;
    const int a_tail = (a_len_shifted - min_len_int8*8);
    const int b_tail = (b_len - min_len_int8*8);
    const int a_tail_int8 = (a_tail + 7) / 8;
    const int b_tail_int8 = (b_tail + 7) / 8;

    uint32 * a_32 = (uint32 *) a;
    uint32 * b_32 = (uint32 *) b;
    uint16 * a_16 = (uint16 *) a;
    uint16 * b_16 = (uint16 *) b;

    const uint16 shift_mask32 = 0xFFFFFFFF >> (32 - shift);
    const uint16 shift_mask16 = 0xFFFF >> (16 - shift);

    uint32 a_bits_buf32 = 0;
    uint32 val32;

    for (int i = 0; i < min_len_int32; i++) {
        val32 = get_int32((unsigned char*)(a_32+i));
        res += sum32( (a_bits_buf32 | (val32 >> shift)) ^ get_int32((unsigned char*)(b_32+i)) );
        a_bits_buf32 = val32 << (32 - shift);
    }

    a_bits_buf32 = a_bits_buf32 >> 16;

    if (min_len_int16 & 1) {
        uint16 val16 = get_int16((unsigned char*)(a_16+min_len_int16-1));
        res += sum16( ((uint16)a_bits_buf32 | (val16 >> shift)) ^ get_int16((unsigned char*)(b_16 + min_len_int16-1)) );
        a_bits_buf32 = val16 << (16 - shift);
    }

    a_bits_buf32 = a_bits_buf32 >> 8;

    if (min_len_int8 & 1) {
        res += sum8( ((unsigned char)a_bits_buf32 | (a[min_len_int8-1] >> shift)) ^ b[min_len_int8-1] );
        a_bits_buf32 = a[min_len_int8-1] << (8 - shift);
    }

    a_bits_buf32 = a_bits_buf32 << 24;

    uint32 a_tail_buf = 0;
    uint32 b_tail_buf = 0;

    if (a_tail) {
        if (a_tail > shift) {
            memcpy(&a_tail_buf, a + min_len_int8, a_tail_int8);
            a_tail_buf = a_bits_buf32 | (get_int32((unsigned char*)&a_tail_buf) >> shift);
        } else {
            a_tail_buf = a_bits_buf32;
        }
        a_tail_buf &= (0xFFFFFFFF << (32 - a_tail));
    }

    if (b_tail) {
        memcpy(&b_tail_buf, b + min_len_int8, b_tail_int8);
        b_tail_buf = get_int32((unsigned char*)&b_tail_buf) & (0xFFFFFFFF << (32 - b_tail));
    }

    res += sum32(a_tail_buf ^ b_tail_buf);

    return res;
}


static int diff(mdjvu_bitmap_t image,
                mdjvu_bitmap_t prototype,
                int32 ceiling)
{
    int32 pw = mdjvu_bitmap_get_width (prototype);
    int32 ph = mdjvu_bitmap_get_height(prototype);
    int32 iw = mdjvu_bitmap_get_width (image);
    int32 ih = mdjvu_bitmap_get_height(image);
    int32 shift_x, shift_y;
    int32 s = 0, i, n = pw + 2;
    unsigned char *ir;
    unsigned char *pr;

    if (abs(iw - pw) > 2) return INT32_MAX;
    if (abs(ih - ph) > 2) return INT32_MAX;

    shift_x = (pw - pw/2) - (iw - iw/2); /* center favors right, always >= 0 */
    shift_y = ph/2 - ih/2;

    int shift_pr = 0;
    if (shift_x < 0) {
        shift_pr = 1;
        shift_x *= -1; // in fact only shift_x == -1 is expected
    }

    for (i = -1; i <= ph; i++)
    {
        int32 y = i - shift_y;

        ir = (y >= 0 && y < ih) ? mdjvu_bitmap_access_packed_row(image, y) : NULL;
        pr = (i >= 0 && i < ph) ? mdjvu_bitmap_access_packed_row(prototype, i) : NULL;

        if (pr == ir)  {
            continue;
        } else if (!pr) {
            s += diff_bits_to_zero(ir, iw);
        } else if (!ir) {
            s += diff_bits_to_zero(pr, pw);
        } else {
            if (!shift_x) {
                s += diff_bits_no_shift(ir, iw, pr, pw);
            } else {
                if (shift_pr) {
                    s += diff_bits_shifted(pr, pw, ir, iw, shift_x);
                } else {
                    s += diff_bits_shifted(ir, iw, pr, pw, shift_x);
                }
            }
        }

        if (s > ceiling) {
            return s;
        }

    }

    return s;
}

static void find_prototypes
    (mdjvu_image_t dict, unsigned char ***uncompressed_dict, mdjvu_image_t img)
{
    int32 d = dict ? mdjvu_image_get_bitmap_count(dict) : 0;
    int32 i, n = mdjvu_image_get_bitmap_count(img);

    if (!mdjvu_image_has_prototypes(img))
        mdjvu_image_enable_prototypes(img);
    if (!mdjvu_image_has_substitutions(img))
        mdjvu_image_enable_substitutions(img);
    if (!mdjvu_image_has_masses(img))
        mdjvu_image_enable_masses(img); /* calculates them, not just enables */
    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t current = mdjvu_image_get_bitmap(img, i);
        int32 mass = mdjvu_image_get_mass(img, current);
        int32 w = mdjvu_bitmap_get_width(current);
        int32 h = mdjvu_bitmap_get_height(current);
        int32 max_score = w * h * THRESHOLD / 100;
        int32 j;
        mdjvu_bitmap_t best_match = NULL;
        int32 best_score = max_score;

        for (j = 0; j < d; j++)
        {
            int32 score;
            mdjvu_bitmap_t candidate = mdjvu_image_get_bitmap(dict, j);
            int32 c_mass = mdjvu_image_get_mass(dict, candidate);
            if (abs(mass - c_mass) > best_score) continue;
            score = diff(current,
                         candidate,
                         best_score);
            if (score < best_score)
            {
                best_score = score;
                best_match = candidate;
                if (!score)
                    break; /* a perfect match is found */
            }
        }

        if (best_score) for (j = 0; j < i; j++)
        {
            int32 score;
            mdjvu_bitmap_t candidate = mdjvu_image_get_bitmap(img, j);
            int32 c_mass = mdjvu_image_get_mass(img, candidate);
            if (abs(mass - c_mass) > best_score) continue;
            score = diff(current,
                         candidate,
                         best_score);
            if (score < best_score)
            {
                best_score = score;
                best_match = candidate;
                if (!score)
                    break; /* a perfect match is found */
            }
        }

        if (best_score)
            mdjvu_image_set_prototype(img, current, best_match);
        else
            mdjvu_image_set_substitution(img, current, best_match);
    }
}

MDJVU_IMPLEMENT void mdjvu_find_prototypes(mdjvu_image_t img)
{
    find_prototypes(NULL, NULL, img);
}

MDJVU_IMPLEMENT void mdjvu_multipage_find_prototypes(mdjvu_image_t dict,
                                                     int32 npages,
                                                     mdjvu_image_t *pages,
                                                     void (*report)(void *, int ),
                                                     void *param)
{
    int i;
    int32 n = mdjvu_image_get_bitmap_count(dict);
    unsigned char ***uncompressed_dict_bitmaps = (unsigned char ***)
        malloc(n * sizeof(unsigned char **));

    for (i = 0; i < n; i++)
    {
        mdjvu_bitmap_t current = mdjvu_image_get_bitmap(dict, i);
        int32 w = mdjvu_bitmap_get_width(current);
        int32 h = mdjvu_bitmap_get_height(current);
        uncompressed_dict_bitmaps[i] = mdjvu_create_2d_array(w, h);
        mdjvu_bitmap_unpack_all_0_or_1(current, uncompressed_dict_bitmaps[i]);
    }

    if (!mdjvu_image_has_masses(dict))
        mdjvu_image_enable_masses(dict); /* calculates them, not just enables */

    for (i = 0; i < npages; i++)
    {
        find_prototypes(dict, uncompressed_dict_bitmaps, pages[i]);
        report(param, i);
    }

    /* destroy uncompressed bitmaps */
    for (i = 0; i < n; i++)
    {
        mdjvu_destroy_2d_array(uncompressed_dict_bitmaps[i]);
    }
    free(uncompressed_dict_bitmaps);
}
