#include "bitmaps.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <endian.h>


unsigned char **allocate_bitmap(int w, int h)
{
    unsigned char *data, **result;
    int i;

    assert(w > 0 && h > 0);

    data = MALLOC(unsigned char, w * h);
    if (!data) return NULL;

    result = MALLOC(unsigned char *, h);
    if (!result)
    {
        FREE(data);
        return NULL;
    }

    for (i = 0; i < h; i++)
    {
        result[i] = data + w * i;
    }

    return result;
}


void free_bitmap(unsigned char **p)
{
    assert(p);
    FREE(p[0]);
    FREE(p);
}


unsigned char **allocate_bitmap_with_margins(int w, int h)
{
    unsigned char **result = allocate_bitmap(w + 2, h + 2);
    int y;

    if (!result)
        return NULL;

    /* set the `result' array so that it points to buffer rows (plus margin) */
    for (y = 0; y < h + 2; y++)
    {
        result[y]++;
    }
    result++; /* now the first byte in the buffer is result[-1][-1] */
    return result;
}


/* Allocate a w * h bitmap with margins of 1 pixels at each side.
 * Copy `pixels' there and clear the margins.
 */
unsigned char **provide_margins(unsigned char **pixels,
                                int w, int h,
                                int make_it_0_or_1)
{
    unsigned char **result = allocate_bitmap_with_margins(w, h);
    int y;

    if (!result)
        return NULL;

    /* clear the top and the bottom row */
    memset(result[-1] - 1, 0, w + 2);
    memset(result[h]  - 1, 0, w + 2);

    for (y = 0; y < h; y++)
    {
        unsigned char *src_row = pixels[y];
        unsigned char *dst_row = result[y];

        /* clear left and right margin */
        dst_row[-1] = 0;
        dst_row[w]  = 0;

        if (!make_it_0_or_1)
            memcpy(dst_row, src_row, w);
        else
        {
            int x;
            for (x = 0; x < w; x++)
                dst_row[x] = (src_row[x] ? 1 : 0);
        }
    }

    return result;
}


/* Simply undo the work of allocate_bitmap_with_margin(). */
void free_bitmap_with_margins(unsigned char **pixels)
{
    assert(pixels);
    FREE(&pixels[-1][-1]); /* because we have both left and top margins of 1 pixel */
    FREE(pixels - 1);
}


void assign_bitmap(unsigned char **dst, unsigned char **src, int w, int h)
{
    int i;
    for (i = 0; i < h; i++)
        memcpy(dst[i], src[i], w);
}

void assign_unpacked_bitmap(unsigned char **dst, unsigned char **src, int w, int h)
{
    const unsigned int row_size = (w + 7) >> 3;
    int i;
    for (i = 0; i < h; i++)
        memcpy(dst[i], src[i], row_size);
}


#if __BYTE_ORDER == __BIG_ENDIAN
inline size_t swap_t(size_t* val, int size) {
    if (size >= sizeof (size_t))
        return *val;

    size_t res = 0;
    memcpy(&res, val, size);
    return res;
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
inline size_t swap_t(size_t* val, unsigned int size) {
    size_t res = 0;
    unsigned char * a = (unsigned char *) &res;
    memcpy(&res, val, size);
    unsigned char t;
    for (unsigned int i = 0; i < (sizeof (size_t)/2); i++) {
        t = a[i];
        a[i] = a[sizeof (size_t) - i - 1];
        a[sizeof (size_t) - i - 1] = t;
    }
    return res;
}
#endif

void assign_unpacked_bitmap_with_shift(unsigned char **dst, unsigned char **src, int w, int h, int N)
{
    const size_t int_len_in_bits = sizeof (size_t)*8;
    assert(N < 8);

    const size_t true_len = (w + (int_len_in_bits -1) ) / int_len_in_bits;
    const size_t true_tail_len = (w % int_len_in_bits) ? ((w % int_len_in_bits) + 7) >> 3 : sizeof (size_t);

    w += N;
    const size_t len = (w + (int_len_in_bits -1) ) / int_len_in_bits;
    const size_t tail_len = (w % int_len_in_bits) ? ((w % int_len_in_bits) + 7) >> 3 : sizeof (size_t);

    for (int y = 0; y < h; y++) {
        size_t *d_p = (size_t *) dst[y+N];
        size_t *s_p = (size_t *) src[y];

        size_t buf = 0;

        for (unsigned int i = 0; i < len; i++) {
            size_t cur = 0;
            if (i < true_len) {
               cur = swap_t(s_p++, i==true_len-1?true_tail_len:sizeof (size_t));
            }

            size_t val = buf | (cur >> N);
            buf = cur << (int_len_in_bits - N);

            val = swap_t(&val, /*i==len-1?tail_len:*/sizeof (size_t));
            memcpy(d_p++, &val, (i==len-1)?tail_len:sizeof (size_t));
        }
    }
}

unsigned char **copy_bitmap(unsigned char **src, int w, int h)
{
    unsigned char **result = allocate_bitmap(w, h);
    assign_bitmap(result, src, w, h);
    return result;
}


void strip_endpoints_4(unsigned char **result, unsigned char **pixels, int w, int h)
{
    int x, y;

    assert(result != pixels);

    for (y = 0; y < h; y++) for (x = 0; x < w; x++) if (pixels[y][x])
    {
        int degree = pixels[y - 1][x] + pixels[y + 1][x]
                   + pixels[y][x - 1] + pixels[y][x + 1];

        if(degree != 1)
            result[y][x] = 1;
    }
}

void strip_endpoints_8(unsigned char **result, unsigned char **pixels, int w, int h)
{
    int x, y;

    assert(result != pixels);

    for (y = 0; y < h; y++) for (x = 0; x < w; x++) if (pixels[y][x])
    {
        int degree = pixels[y - 1][x - 1] + pixels[y - 1][x] + pixels[y - 1][x + 1]
                   + pixels[y    ][x - 1] +                    pixels[y    ][x + 1]
                   + pixels[y + 1][x - 1] + pixels[y + 1][x] + pixels[y + 1][x + 1];

        if(degree != 1)
            result[y][x] = 1;
    }
}


void print_bitmap(unsigned char **pixels, int w, int h)
{
    int x, y;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            putchar(pixels[y][x] ? '@' : '.');
        }
        putchar('\n');
    }
}


void make_bitmap_0_or_1(unsigned char **pixels, int w, int h)
{
    int x, y;

    for (y = 0; y < h; y++)
    {
        unsigned char *row = pixels[y];

        for (x = 0; x < w; x++)
            row[x] = ( row[x] ? 1 : 0 );
    }
}

void invert_bitmap_0_or_1(unsigned char **pixels, int w, int h)
{
    const int int_len_in_bytes = sizeof (size_t);
    const int len = w / int_len_in_bytes;
    const int tail_len = w % int_len_in_bytes;

    size_t mask = 0x01010101;
    for (int i = 1; i < int_len_in_bytes/4; i++) {
       mask |= (mask << 32);
    }

    for (int j = 0; j < h; j++) {
        size_t * row_i = (size_t *) pixels[j];

        for (int i = 0; i < len; i++) {
            *row_i++ ^= mask;
        }

        if (tail_len) {
            size_t val = 0;
            memcpy(&val, row_i, tail_len);
            val ^= mask;
            memcpy(row_i, &val, tail_len);
        }

    }


}

void invert_bitmap(unsigned char **pixels, int w, int h)
{
    const int int_len_in_bytes = sizeof (size_t);
    const size_t tail_mask = (~(size_t)0) << (sizeof (size_t)*8 - (w % (sizeof (size_t)*8)));

//    w = (w + 7) >> 3;
    const int len = w / (int_len_in_bytes*8);
    const int tail_len = ((w % (int_len_in_bytes*8)) + 7) >> 3;

    for (int j = 0; j < h; j++) {
        size_t * row_i = (size_t *) pixels[j];

        for (int i = 0; i < len; i++) {
            *row_i = ~*row_i;
            row_i++;
        }

        if (tail_len) {
            size_t val = swap_t(row_i, tail_len);
            val = ~val & tail_mask;
            val = swap_t(&val, sizeof (size_t));
            memcpy(row_i, &val, tail_len);
        }
    }
}

void invert_bitmap_old(unsigned char **pixels, int w, int h, int first_make_it_0_or_1)
{
    int x, y;

       for (y = 0; y < h; y++)
       {
           unsigned char *row = pixels[y];

           if (first_make_it_0_or_1)
           {
               for (x = 0; x < w; x++)
                   row[x] = ( row[x] ? 0 : 1 );
           }
           else
           {
               for (x = 0; x < w; x++)
                   row[x] = 1 - row[x];
           }
       }
}


unsigned char **allocate_bitmap_with_white_margins(int w, int h)
{
    unsigned char **result = allocate_bitmap_with_margins(w, h);
    int y;

    memset(result[-1] - 1, 0, w + 2);
    memset(result[ h] - 1, 0, w + 2);
    for (y = 0; y < h; y++)
    {
        result[y][-1] = 0;
        result[y][w] = 0;
    }

    return result;
}


void clear_bitmap(unsigned char **pixels, int w, int h)
{
    int y;

    for (y = 0; y < h; y++)
        memset(pixels[y], 0, w);
}
