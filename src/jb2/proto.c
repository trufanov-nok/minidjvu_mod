/*
 * proto.c - searching for prototypes
 */

#include "../base/mdjvucfg.h"
#include <minidjvu/minidjvu.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <endian.h> // macros __BYTE_ORDER
#endif

#define THRESHOLD 21

// Generate a lookup table for 32 bit integers
#define B2(n) n, n + 1, n + 1, n + 2
#define B4(n) B2(n), B2(n + 1), B2(n + 1), B2(n + 2)
#define B6(n) B4(n), B4(n + 1), B4(n + 1), B4(n + 2)

// Lookup table that store the sum of bits for all uchar values

const unsigned char lookuptable[256] = { B6(0), B6(1), B6(1), B6(2) };

__inline unsigned char sum_s(size_t val) {
	unsigned char* c = (unsigned char *) &val;
	if ( sizeof (size_t) == 8 ) {
		return lookuptable[c[0]] + lookuptable[c[1]] + lookuptable[c[2]] + lookuptable[c[3]] +
				lookuptable[c[4]] + lookuptable[c[5]] + lookuptable[c[6]] + lookuptable[c[7]];
	} else {
		return lookuptable[c[0]] + lookuptable[c[1]] + lookuptable[c[2]] + lookuptable[c[3]];
	}
}

#if __BYTE_ORDER == __BIG_ENDIAN
__inline size_t swap_t(size_t* val) {
	return *val;
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
__inline size_t swap_t(size_t val) {
	unsigned char * a = (unsigned char *) &val;
	unsigned char t;
	for (unsigned int i = 0; i < (sizeof (size_t)/2); i++) {
		t = a[i];
		a[i] = a[sizeof (size_t) - i - 1];
		a[sizeof (size_t) - i - 1] = t;
	}
	return val;
}
#endif

static unsigned int diff_bits_to_zero(unsigned char * a, unsigned int a_len)
{
    // non-meaning bits in last bytes must be 0s
	unsigned int res = 0;

	const unsigned int size_of_t_len = sizeof(size_t)*8;
	const unsigned int a_len_size_t = a_len / size_of_t_len;
	const unsigned int a_len_tail = ((a_len % size_of_t_len) + 7) >> 3;

	size_t * a_s = (size_t *) a;

	for (unsigned int i = 0; i < a_len_size_t; i++) {
		res += sum_s(a_s[i]);
    }

	size_t val = 0;

	memcpy(&val, a_s + a_len_size_t, a_len_tail);
	res += sum_s(val);
    return res;
}


static unsigned int diff_bits_no_shift(unsigned char * a, unsigned int a_len, unsigned char * b, unsigned int b_len)
{
	// abs(a_len-b_len) must be < sizeof(size_t)
    // non-meaning bits in last bytes must be 0s
	unsigned int res = 0;

	const unsigned int min_len = a_len < b_len ? a_len : b_len;
	const unsigned int size_of_t_len = sizeof(size_t)*8;
	const unsigned int min_len_size_t = min_len / size_of_t_len;
	const unsigned int a_left = a_len - min_len_size_t*size_of_t_len;
	const unsigned int b_left = b_len - min_len_size_t*size_of_t_len;
	const unsigned int a_tail = (a_left + 7) >> 3; // may be equal to sizeof(size_t) for length 64 compared to 63 for ex.
	const unsigned int b_tail = (b_left + 7) >> 3;

	size_t * a_s = (size_t *) a;
	size_t * b_s = (size_t *) b;

	for (unsigned int i = 0; i < min_len_size_t; i++) {
		res += sum_s( a_s[i] ^ b_s[i] );
    }

	size_t val1 = 0;
	size_t val2 = 0;

	memcpy(&val1, a_s + min_len_size_t, a_tail);
	memcpy(&val2, b_s + min_len_size_t, b_tail);
	res += sum_s(val1 ^ val2);

    return res;
}


static unsigned int diff_bits_shifted(unsigned char * a, unsigned int a_len, unsigned char * b, unsigned int b_len, unsigned int shift)
{
	// abs(a_len-b_len) must be < sizeof(size_t)-shift
    // shifted array must be "a", only one array is shifted
    // non-meaning bits in last bytes must be 0s

	unsigned int res = 0;

	const unsigned int a_len_shifted = a_len + shift;
	const unsigned int min_len = a_len_shifted < b_len ? a_len_shifted : b_len;
	const unsigned int size_of_t_len = sizeof(size_t)*8;
	const unsigned int min_len_size_t = min_len / size_of_t_len;
	const unsigned int a_left = a_len - min_len_size_t*size_of_t_len;
	const unsigned int b_left = b_len - min_len_size_t*size_of_t_len;
	const unsigned int a_tail = (a_left + 7) >> 3;
	const unsigned int b_tail = (b_left + 7) >> 3;


	const unsigned int shift_right = size_of_t_len - shift;

	size_t * a_s = (size_t *) a;
	size_t * b_s = (size_t *) b;

	size_t buf = 0;
	size_t val;

	for (unsigned int i = 0; i < min_len_size_t; i++) {
		val = swap_t(a_s[i]);
		res += sum_s( (buf | (val >> shift)) ^ swap_t(b_s[i]) );
		buf = val << shift_right;
    }

	val = 0;
	size_t val2 = 0;

	memcpy(&val, a_s+min_len_size_t, a_tail);
	memcpy(&val2, b_s+min_len_size_t, b_tail);
	val = swap_t(val) >> shift;
	val2 = swap_t(val2);
	res += sum_s( (buf | val) ^ val2);

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
	int32 s = 0, i;
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
	(mdjvu_image_t dict, mdjvu_image_t img)
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
	find_prototypes(NULL, img);
}

MDJVU_IMPLEMENT void mdjvu_multipage_find_prototypes(mdjvu_image_t dict,
                                                     int32 npages,
                                                     mdjvu_image_t *pages,
                                                     void (*report)(void *, int ),
                                                     void *param)
{
    int i;

    if (!mdjvu_image_has_masses(dict))
        mdjvu_image_enable_masses(dict); /* calculates them, not just enables */

    for (i = 0; i < npages; i++)
    {
		find_prototypes(dict, pages[i]);
        report(param, i);
    }
}
