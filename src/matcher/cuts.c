/*
 * cuts.c - finding "cuts signature" consisting of consecutive cut positions
 */


/* We cut an image horizontally in such a way
 *     that below and above the cut the blackness is roughly the same.
 * Than cutting each of the two pieces vertically in the same fashion.
 * Then horizontally, and so on until SIGNATURE_SIZE - 1 cuts.
 * The position of each cut is normalized into 0..255 and put into signature.
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <assert.h>


/* Stuff for not using malloc in C++
 * (made by Leon Bottou; has no use in minidjvu-mod,
 * but left here for potential DjVuLibre compatibility)
 */
#ifdef __cplusplus
# define MALLOC(Type)    new Type
# define FREE(p)         delete p
# define MALLOCV(Type,n) new Type[n]
# define FREEV(p)        delete [] p
#else
# define MALLOC(Type)    ((Type*)malloc(sizeof(Type)))
# define FREE(p)         do{if(p)free(p);}while(0)
# define MALLOCV(Type,n) ((Type*)malloc(sizeof(Type)*(n)))
# define FREEV(p)        do{if(p)free(p);}while(0)
#endif


typedef unsigned char byte;

static int32 sum_column_gray(byte **pixels, int32 x, int32 y1, int32 y2)
{
    int sum = 0, y;
    for (y = y1; y <= y2; y++) sum += pixels[y][x];
    return sum;
}

static int32 sum_row_gray(byte *row, int32 x1, int32 x2)
{
    int sum = 0, x, n = x2 - x1;
    byte *p = row + x1;
    for (x = 0; x <= n; x++) sum += p[x];
    return sum;
}

static int32 sum_column_black_and_white(byte **pixels, int32 x, int32 y1, int32 y2)
{
    int sum = 0, y;
    for (y = y1; y <= y2; y++) if (pixels[y][x]) sum++;
    return sum;
}

static int32 sum_row_black_and_white(byte *row, int32 x1, int32 x2)
{
    int sum = 0, x, n = x2 - x1;
    byte *p = row + x1;
    for (x = 0; x <= n; x++) if (p[x]) sum++;
    return sum;
}

static void make_vcut(int32 a, int32 l, int32 w, int32 h, byte **pixels,
                      byte *sig, int32 k,
                      int32 s_row(byte *, int32, int32),
                      int32 s_col(byte **, int32, int32, int32),
                      int32 size);

static void make_hcut(int32 a, int32 l, int32 w, int32 h,
                      byte **pixels, byte *sig, int32 k,
                      int32 s_row(byte *, int32, int32),
                      int32 s_col(byte **, int32, int32, int32),
                      int32 size)
{
    int32 cut = 0; /* how many rows are in the top part */
    int32 up_weight = 0;

    if (k >= size) return;

    if (a)
    {
        int32 last_row_weight = 0;

        assert(w && h);

        while ((up_weight << 1) < a)
        {
            last_row_weight = s_row(pixels[cut], l, l + w - 1);
            up_weight += last_row_weight;
            cut++;
        }
        cut--;
        up_weight -= last_row_weight;
        sig[k] = (byte) ((256 *
                    (cut * w + w * ((a >> 1) - up_weight) / last_row_weight))
                 / (w * h));
        if (a - (up_weight << 1) > last_row_weight)
        {
            cut++;
            up_weight += last_row_weight;
        }
    }
    else
    {
        cut = h / 2;
        sig[k] = 128;
    }

    make_vcut(up_weight, l, w, cut, pixels, sig, k << 1, s_row, s_col, size);
    make_vcut(a - up_weight, l, w, h - cut, pixels + cut, sig, (k << 1) | 1, s_row, s_col, size);
}

static void make_vcut(int32 a, int32 l, int32 w, int32 h,
                      byte **pixels, byte *sig, int32 k,
                      int32 s_row(byte *, int32, int32),
                      int32 s_col(byte **, int32, int32, int32),
                      int32 size)
{
    int32 cut = 0;          /* how many columns are in the left part */
    int32 left_weight = 0;

    if (k >= size) return;

    if (a)
    {
        int32 last_col_weight = 0;

        assert(w && h);

        while ((left_weight << 1) < a)
        {
            last_col_weight = s_col(pixels, l + cut, 0, h-1);
            left_weight += last_col_weight;
            cut++;
        }
        cut--;
        left_weight -= last_col_weight;
        sig[k] = (byte) ((256 *
                    (cut * h + h * ((a >> 1) - left_weight) / last_col_weight))
                 / (w * h));
        if (a - (left_weight << 1) > last_col_weight)
        {
            cut++; left_weight += last_col_weight;
        }
    }
    else
    {
        cut = w / 2;
        sig[k] = 128;
    }

    make_hcut(left_weight, l, cut, h, pixels, sig, k << 1, s_row, s_col, size);
    make_hcut(a - left_weight, l + cut, w - cut, h, pixels, sig, (k << 1) | 1, s_row, s_col, size);
}

static void get_signature(int32 width, int32 height, byte **pixels, byte *sig,
            int32 s_row(byte *, int32, int32),
            int32 s_col(byte **, int32, int32, int32),
            int32 size)
{
    int32 area = 0, i;
    for (i = 0; i < height; i++)
    {
        area += s_row(pixels[i], 0, width - 1);
    }
    /* FIXME: sig[0] is wasted */
    make_hcut(area, 0, width, height, pixels, sig, 1, s_row, s_col, size);
}

MDJVU_IMPLEMENT void mdjvu_get_gray_signature(byte **data, int32 w, int32 h,
                                              byte *result, int32 size)
{
    get_signature(w, h, data, result, sum_row_gray, sum_column_gray, size);
}

MDJVU_IMPLEMENT void mdjvu_get_black_and_white_signature
                                        (byte **data, int32 w, int32 h,
                                              byte *result, int32 size)
{
    get_signature(w, h, data, result, sum_row_black_and_white, sum_column_black_and_white, size);
}
