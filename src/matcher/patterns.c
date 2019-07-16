/*
 * patterns.c - pattern matching algorithm
 */

/* This is `patterns.c', the unit that handles pattern matching.
 * Its task is only to compare pairs of images, not to classify a set of them.
 * And this has absolutely nothing to do with choosing a cross-coding prototype.
 */

#include "../base/mdjvucfg.h"
#include <minidjvu/minidjvu.h>
#include "bitmaps.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>


#define TIMES_TO_THIN 1
#define TIMES_TO_THICKEN 1


#define SIGNATURE_SIZE 32


typedef struct
{
    double pithdiff1_threshold;
    double pithdiff2_threshold;
    double shiftdiff1_threshold;
    double shiftdiff2_threshold;
    double shiftdiff3_threshold;
    int aggression;
    int method;
    mdjvu_classify_options_t classify_options;
} Options;

/* These are hand-tweaked parameters of this classifier. */

static const double pithdiff1_veto_threshold       = 23;
static const double pithdiff2_veto_threshold       = 4;
static const double shiftdiff1_veto_threshold      = 1000;
static const double shiftdiff2_veto_threshold      = 1500;
static const double shiftdiff3_veto_threshold      = 2000;

static const double size_difference_threshold = 10;
static const double mass_difference_threshold = 15;

static const double shiftdiff1_falloff        = .9;
static const double shiftdiff2_falloff        = 1;
static const double shiftdiff3_falloff        = 1.15;

static void interpolate(Options *opt, const double *v1, const double *v2,
                        int l, int r, int x)
{
    double w1 = ((double)(r - x)) / (r - l); /* weights */
    double w2 = 1 - w1;
    opt->pithdiff1_threshold  = v1[0] * w1 + v2[0] * w2;
    opt->pithdiff2_threshold  = v1[1] * w1 + v2[1] * w2;
    opt->shiftdiff1_threshold = v1[2] * w1 + v2[2] * w2;
    opt->shiftdiff2_threshold = v1[3] * w1 + v2[3] * w2;
    opt->shiftdiff3_threshold = v1[4] * w1 + v2[4] * w2;
}


/* Sets `aggression' for pattern matching.
 * Lower values are safer, bigger values produce smaller files.
 */

MDJVU_IMPLEMENT void mdjvu_set_aggression(mdjvu_matcher_options_t opt, int level)
{
    const double set200[5] = {9, 1.2, 70, 90, 180};
    const double set120[5] = {5,  .2, 50, 70, 150};
    const double   set0[5] = {0,   0,  0,  0,   0};

    if (level < 0) level = 0;

    ((Options *) opt)->aggression = level;

    if (level > 120)
        interpolate((Options *) opt, set120, set200, 120, 200, level);
    else
        interpolate((Options *) opt, set0,   set120, 0,   120, level);
}

/* ========================================================================== */

MDJVU_IMPLEMENT mdjvu_matcher_options_t mdjvu_matcher_options_create(void)
{
    mdjvu_matcher_options_t options = (mdjvu_matcher_options_t) MALLOC1(Options);

    mdjvu_init();
    mdjvu_set_aggression(options, 100);
    ((Options *) options)->method = 0;
    ((Options *) options)->classify_options = NULL;
    return options;
}

MDJVU_IMPLEMENT void mdjvu_set_classify_options(mdjvu_matcher_options_t opt, mdjvu_classify_options_t v)
{
    Options * options = (Options *) opt;
    if (options->classify_options)
        mdjvu_classify_options_destroy(options->classify_options);
    options->classify_options = v;
}

MDJVU_IMPLEMENT mdjvu_classify_options_t mdjvu_get_classify_options(mdjvu_matcher_options_t opt)
{
    Options * options = (Options *) opt;
    return options->classify_options;
}

MDJVU_IMPLEMENT void mdjvu_use_matcher_method(mdjvu_matcher_options_t opt, int method)
{
    ((Options *) opt)->method |= method;
}

MDJVU_IMPLEMENT void mdjvu_matcher_options_destroy(mdjvu_matcher_options_t opt)
{
    Options * options = (Options *) opt;
    if (options->classify_options)
        mdjvu_classify_options_destroy(options->classify_options);
    FREE1(options);
}

/* ========================================================================== */

typedef unsigned char byte;

typedef struct ComparableImageData
{
    byte **pixels; /* 0 - purely white, 255 - purely black (inverse to PGM!) */
    byte **pith2_inner;
    byte **pith2_outer;
    int32 width, height, mass;
    int32 mass_center_x, mass_center_y;
    byte signature[SIGNATURE_SIZE];  /* for shiftdiff 1 and 3 tests */
    byte signature2[SIGNATURE_SIZE]; /* for shiftdiff 2 test */
} Image;



/* Each image pair undergoes simple tests (dimensions and mass)
 * and at most five more advanced tests.
 * Each test may end up with three outcomes: veto (-1), doubt (0) and match(1).
 * Images are equivalent if and only if
 *     there was no `veto'
 *     and there was at least one `match'.
 */


/* We check whether images' dimensions are different
 *     no more than by size_difference_threshold percent.
 * Return value is usual: veto (-1) or doubt (0).
 * Mass checking was introduced by Leon Bottou.
 */

static int simple_tests(Image *i1, Image *i2)
{
    int32 w1 = i1->width, h1 = i1->height, m1 = i1->mass;
    int32 w2 = i2->width, h2 = i2->height, m2 = i2->mass;

    if (100.* w1 > (100.+ size_difference_threshold) * w2) return -1;
    if (100.* w2 > (100.+ size_difference_threshold) * w1) return -1;
    if (100.* h1 > (100.+ size_difference_threshold) * h2) return -1;
    if (100.* h2 > (100.+ size_difference_threshold) * h1) return -1;
    if (100.* m1 > (100.+ mass_difference_threshold) * m2) return -1;
    if (100.* m2 > (100.+ mass_difference_threshold) * m1) return -1;

    return 0;
}


#define USE_PITHDIFF 1
#define USE_SHIFTDIFF_1 1
#define USE_SHIFTDIFF_2 1
#define USE_SHIFTDIFF_3 1


/* Computing distance by comparing pixels {{{ */

/* This function compares two images pixel by pixel.
 * The exact way to compare pixels is defined by two functions,
 *     compare_row and compare_with_white.
 * Both functions take pointers to byte rows and their length.
 *
 * Now images are aligned by mass centers.
 * Code needs some clarification, yes...
 */
static int32 distance_by_pixeldiff_functions_by_shift(Image *i1, Image *i2,
    int32 (*compare_row)(byte *, byte *, int32),
    int32 (*compare_1_with_white)(byte *, int32),
    int32 (*compare_2_with_white)(byte *, int32),
    int32 ceiling,
    int32 shift_x, int32 shift_y) /* of i1's coordinate system with respect to i2 */
{
    int32 w1 = i1->width, w2 = i2->width, h1 = i1->height, h2 = i2->height;
    int32 min_y = shift_y < 0 ? shift_y : 0;
    int32 right1 = shift_x + w1;
    int32 max_y_plus_1 = h2 > shift_y + h1 ? h2 : shift_y + h1;
    int32 i;
    int32 min_overlap_x = shift_x > 0 ? shift_x : 0;
    int32 max_overlap_x_plus_1 = w2 < right1 ? w2 : right1;
    int32 min_overlap_x_for_i1 = min_overlap_x - shift_x;
    int32 max_overlap_x_plus_1_for_i1 = max_overlap_x_plus_1 - shift_x;
    int32 overlap_length = max_overlap_x_plus_1 - min_overlap_x;
    int32 score = 0;

    if (overlap_length <= 0) return INT32_MAX;

    for (i = min_y; i < max_y_plus_1; i++)
    {
        int32 y1 = i - shift_y;

        /* calculate difference in the i-th line */

        if (i < 0 || i >= h2)
        {
            /* calculate difference of i1 with white */
            score += compare_1_with_white(i1->pixels[y1], w1);
        }
        else if (i < shift_y || i >= shift_y + h1)
        {
            /* calculate difference of i2 with white */
            score += compare_2_with_white(i2->pixels[i], w2);
        }
        else
        {
            /* calculate difference in a line where the bitmaps overlap */
            score += compare_row(i1->pixels[y1] + min_overlap_x_for_i1,
                                 i2->pixels[i] + min_overlap_x,
                                 overlap_length);


            /* calculate penalty for the left margin */
            if (min_overlap_x > 0)
                score += compare_2_with_white(i2->pixels[i], min_overlap_x);
            else
                score += compare_1_with_white(i1->pixels[y1], min_overlap_x_for_i1);

            /* calculate penalty for the right margin */
            if (max_overlap_x_plus_1 < w2)
            {
                score += compare_2_with_white(
                    i2->pixels[i] + max_overlap_x_plus_1,
                    w2 - max_overlap_x_plus_1);
            }
            else
            {
                score += compare_1_with_white(
                     i1->pixels[y1] + max_overlap_x_plus_1_for_i1,
                     w1 - max_overlap_x_plus_1_for_i1);

            }
        }

        if (score >= ceiling) return INT32_MAX;
    }
    return score;
}

static int32 distance_by_pixeldiff_functions(Image *i1, Image *i2,
    int32 (*compare_row)(byte *, byte *, int32),
    int32 (*compare_1_with_white)(byte *, int32),
    int32 (*compare_2_with_white)(byte *, int32),
    int32 ceiling)
{
    byte **p1, **p2;
    int32 w1, w2, h1, h2;
    int32 shift_x, shift_y; /* of i1's coordinate system with respect to i2 */
    /*int32 s = 0, i, i_start, i_cap;
    int32 right_margin_start, right_margin_width;*/

    /* make i1 to be narrower than i2 */
    if (i1->width > i2->width)
    {
        Image *img = i1;
        i1 = i2;
        i2 = img;
    }

    w1 = i1->width; h1 = i1->height; p1 = i1->pixels;
    w2 = i2->width; h2 = i2->height; p2 = i2->pixels;

    /* (shift_x, shift_y) */
    /*     is what should be added to i1's coordinates to get i2's coordinates. */
    shift_x = (w2 - w2/2) - (w1 - w1/2); /* center favors right */
    shift_y = h2/2 - h1/2;               /* center favors top */

    shift_x = i2->mass_center_x - i1->mass_center_x;
    if (shift_x < 0)
        shift_x = (shift_x - MDJVU_CENTER_QUANT / 2) / MDJVU_CENTER_QUANT;
    else
        shift_x = (shift_x + MDJVU_CENTER_QUANT / 2) / MDJVU_CENTER_QUANT;

    shift_y = i2->mass_center_y - i1->mass_center_y;
    if (shift_y < 0)
        shift_y = (shift_y - MDJVU_CENTER_QUANT / 2) / MDJVU_CENTER_QUANT;
    else
        shift_y = (shift_y + MDJVU_CENTER_QUANT / 2) / MDJVU_CENTER_QUANT;

    return distance_by_pixeldiff_functions_by_shift(
        i1, i2, compare_row, compare_1_with_white, compare_2_with_white,
        ceiling, shift_x, shift_y);
}

/* Computing distance by comparing pixels }}} */
/* inscribed framework penalty counting {{{ */

/* (Look at `frames.c' to see what it's all about) */

#if USE_PITHDIFF

/* If the framework of one letter is inscribed into another and vice versa,
 *     then those letters are probably equivalent.
 * That's the idea...
 * Counting penalty points here for any pixel
 *     that's framework in one image and white in the other.
 */

static int32 pithdiff_compare_row(byte *row1, byte *row2, int32 n)
{
    int32 i, s = 0;
    for (i = 0; i < n; i++) {
        if (row1[i] == 0xFF || row2[i] == 0xFF)
            s += fabs(row1[i] - row2[i]);
    }
    return s;
}

static int32 pithdiff_compare_with_white(byte *row, int32 n)
{
    int32 i, s = 0;
    for (i = 0; i < n; i++) if (row[i] == 255) s += 255;
    return s;
}

static int32 pithdiff_distance(Image *i1, Image *i2, int32 ceiling)
{
    return distance_by_pixeldiff_functions(i1, i2,
            &pithdiff_compare_row,
            &pithdiff_compare_with_white,
            &pithdiff_compare_with_white,
            ceiling);
}

static int pithdiff_equivalence(Image *i1, Image *i2, double threshold, int32 dpi)
{
    int32 perimeter = i1->width + i1->height + i2->width + i2->height;
    int32 ceiling = (int32) (pithdiff1_veto_threshold * dpi * perimeter / 100);
    int32 d = pithdiff_distance(i1, i2, ceiling);
    if (d == INT32_MAX) return -1;
    if (d < threshold * dpi * perimeter / 100) return 1;
    return 0;
}

#endif /* if USE_PITHDIFF */

/* inscribed framework penalty counting }}} */

/* shift signature comparison {{{ */

/* Just finding the square of a normal Euclidean distance between vectors
 * (but with falloff)
 */

#if USE_SHIFTDIFF_1 || USE_SHIFTDIFF_2 || USE_SHIFTDIFF_3
static int shiftdiff_equivalence(byte *s1, byte *s2, double falloff, double veto, double threshold)
{
    int i, delay_before_falloff = 1, delay_counter = 1;
    double penalty = 0;
    double weight = 1;

    for (i = 1; i < SIGNATURE_SIZE; i++) /* kluge: ignores the first byte */
    {
        int difference = s1[i] - s2[i];
        penalty += difference * difference * weight;
        if (!--delay_counter)
        {
            weight *= falloff;
            delay_counter = delay_before_falloff <<= 1;
        }
    }

    if (penalty >= veto * SIGNATURE_SIZE) return -1;
    if (penalty <= threshold * SIGNATURE_SIZE) return 1;
    return 0;
}
#endif
/* shift signature comparison }}} */

#ifndef NO_MINIDJVU
mdjvu_pattern_t mdjvu_pattern_create(mdjvu_matcher_options_t opt, mdjvu_bitmap_t bitmap)
{
    /* not calling mdjvu_init() since we already have a bitmap */
    int32 w = mdjvu_bitmap_get_width(bitmap);
    int32 h = mdjvu_bitmap_get_height(bitmap);
    mdjvu_pattern_t pattern;
    byte **pixels = mdjvu_create_2d_array(w, h);
    mdjvu_bitmap_unpack_all(bitmap, pixels);
    pattern = mdjvu_pattern_create_from_array(opt, pixels, w, h);
    mdjvu_destroy_2d_array(pixels);
    return pattern;
}
#endif

/* Finding mass center {{{ */

static void get_mass_center(unsigned char **pixels, int32 w, int32 h,
                     int32 *pmass_center_x, int32 *pmass_center_y)
{
    double x_sum = 0, y_sum = 0, mass = 0;
    int32 i, j;

    for (i = 0; i < h; i++)
    {
        unsigned char *row = pixels[i];
        for (j = 0; j < w; j++)
        {
            unsigned char pixel = row[j];
            x_sum += pixel * j;
            y_sum += pixel * i;
            mass  += pixel;
        }
    }

    *pmass_center_x = (int32) (x_sum * MDJVU_CENTER_QUANT / mass);
    *pmass_center_y = (int32) (y_sum * MDJVU_CENTER_QUANT / mass);
}

/* Finding mass center }}} */


/* get a center (in 1/MDJVU_CENTER_QUANT pixels; defined in the header for image) */
MDJVU_IMPLEMENT void mdjvu_pattern_get_center(mdjvu_pattern_t p, int32 *cx, int32 *cy)
{
    *cx = ((Image *) p)->mass_center_x;
    *cy = ((Image *) p)->mass_center_y;
}

static void sweep(unsigned char **pixels, unsigned char **source, int w, int h)
{
    const int int_len_in_bytes = sizeof (size_t);
    const int len = w / int_len_in_bytes;
    const int tail_len = w % int_len_in_bytes;

    for (int y = 0; y < h; y++) {
        size_t *row    = (size_t*) pixels[y];
        size_t *srow   = (size_t*) source[y];
        size_t *srow_l = (size_t*) (source[y] - 1);
        size_t *srow_r = (size_t*) (source[y] + 1);
        size_t *supper = (size_t*) source[y-1];
        size_t *slower = (size_t*) source[y+1];

        for (int i = 0; i < len; i++) {
            *row++ = *supper++ | *srow_l++ | *srow++ | *srow_r++ | *slower++;
         }

        if (tail_len) {
            size_t r = 0;
            memcpy(&r, supper, tail_len);
            size_t val = r;
            memcpy(&r, srow_l, tail_len);
            val |= r;
            memcpy(&r, srow, tail_len);
            val |= r;
            memcpy(&r, srow_r, tail_len);
            val |= r;
            memcpy(&r, slower, tail_len);
            val |= r;
            memcpy(row, &val, tail_len);
        }
    }

}

static unsigned char **quick_thin(unsigned char **pixels, int w, int h, int N)
{
    unsigned char **aux = provide_margins(pixels, w, h, 1);
    unsigned char **buf = allocate_bitmap_with_white_margins(w, h);

    clear_bitmap(buf, w, h);
    invert_bitmap_0_or_1(aux, w, h);

    while (N--)
    {
        sweep(buf, aux, w, h);
        if (N) {
            assign_bitmap(aux, buf, w, h);
        }
    }

    invert_bitmap_0_or_1(buf, w, h);

    free_bitmap_with_margins(aux);
    return buf;
}

static unsigned char **quick_thicken(unsigned char **pixels, int w, int h, int N)
{
    int r_w = w + N * 2;
    int r_h = h + N * 2;
    int y, x;
    unsigned char **aux = allocate_bitmap_with_white_margins(r_w, r_h);
    unsigned char **buf = allocate_bitmap_with_white_margins(r_w, r_h);

    clear_bitmap(buf, r_w, r_h);
    clear_bitmap(aux, r_w, r_h);

    for (y = 0; y < h; y++) {
        memcpy(aux[y + N] + N, pixels[y], w);
    }

    while (N--)
    {
        sweep(buf, aux, r_w, r_h);
        if (N) {
            assign_bitmap(aux, buf, r_w, r_h);
        }
    }

    free_bitmap_with_margins(aux);
    return buf;
}

MDJVU_IMPLEMENT mdjvu_pattern_t mdjvu_pattern_create_from_array(mdjvu_matcher_options_t m_opt, byte **pixels, int32 w, int32 h)/*{{{*/
{
    Options *opt = (Options *) m_opt;
    int32 i, mass;
    Image *img = MALLOC1(Image);

    mdjvu_init();

    img->width = w;
    img->height = h;

    img->pixels = allocate_bitmap(w, h);
    memset(img->pixels[0], 0, w * h);

    mass = 0;
    for (i = 0; i < h; i++)
    {
        int32 j;
        for (j = 0; j < w; j++)
            if (pixels[i][j])
            {
                img->pixels[i][j] = 255; /* i don't remember what for */
                mass += 1;
            }
    }
    img->mass = mass;

    mdjvu_soften_pattern(img->pixels, img->pixels, w, h);

    get_mass_center(img->pixels, w, h,
                    &img->mass_center_x, &img->mass_center_y);
    mdjvu_get_gray_signature(img->pixels, w, h,
                             img->signature, SIGNATURE_SIZE);

    mdjvu_get_black_and_white_signature(img->pixels, w, h,
                                        img->signature2, SIGNATURE_SIZE);

    if (!opt->aggression)
    {
        free_bitmap(img->pixels);
        img->pixels = NULL;
    }

    if (opt->method & MDJVU_MATCHER_PITH_2)
    {
        img->pith2_inner = quick_thin(pixels, w, h, TIMES_TO_THIN);
        img->pith2_outer = quick_thicken(pixels, w, h, TIMES_TO_THICKEN);
        assert(img->pith2_inner);
        assert(img->pith2_outer);
    }
    else
    {
        img->pith2_inner = NULL;
        img->pith2_outer = NULL;
    }

    return (mdjvu_pattern_t) img;
}/*}}}*/



static int32 pith2_row_subset(byte *A, byte *B, int32 length)
{
    int32 i, s = 0;
    for (i = 0; i < length; i++)
    {
        if (A[i] & !B[i])
            s += 255;
    }
    return s;
}

static int32 pith2_row_has_black(byte *row, int32 length)
{
    int32 i, s = 0;
    for (i = 0; i < length; i++)
    {
        if (row[i])
            s += 255;
    }
    return s;
}

static int32 pith2_return_0(byte *A, int32 length)
{
    return 0;
}

static int pith2_is_subset(mdjvu_pattern_t ptr1, mdjvu_pattern_t ptr2, double threshold, int32 dpi)
{
    Image *i1 = (Image *) ptr1;
    Image *i2 = (Image *) ptr2;
    Image ptr1_inner;
    Image ptr2_outer;
    int32 perimeter = i1->width + i1->height + i2->width + i2->height;
    int32 ceiling = (int32) (pithdiff2_veto_threshold * dpi * perimeter / 100);
    int32 d = 0;

    ptr1_inner.pixels = i1->pith2_inner;
    assert(i1->pith2_inner);
    ptr1_inner.width  = i1->width;
    ptr1_inner.height = i1->height;
    ptr1_inner.mass_center_x = i1->mass_center_x;
    ptr1_inner.mass_center_y = i1->mass_center_y;

    ptr2_outer.pixels = i2->pith2_outer;
    assert(i2->pith2_outer);
    ptr2_outer.width  = i2->width  + TIMES_TO_THICKEN*2;
    ptr2_outer.height = i2->height + TIMES_TO_THICKEN*2;
    ptr2_outer.mass_center_x = i2->mass_center_x + MDJVU_CENTER_QUANT;
    ptr2_outer.mass_center_y = i2->mass_center_y + MDJVU_CENTER_QUANT;

    d = distance_by_pixeldiff_functions(&ptr1_inner, &ptr2_outer,
        &pith2_row_subset,
        &pith2_row_has_black,
        &pith2_return_0,
        ceiling);

    if (d == INT32_MAX) return -1;
    else if (d < threshold * dpi * perimeter / 100) return 1;
    return 0;
}

/* Requires `opt' to be non-NULL */
static int compare_patterns(mdjvu_pattern_t ptr1, mdjvu_pattern_t ptr2,/*{{{*/
                            int32 dpi, Options *opt)

{
    Image *i1 = (Image *) ptr1, *i2 = (Image *) ptr2;
    int i, state = 0; /* 0 - unsure, 1 - equal unless veto */

    if (simple_tests(i1, i2)) return -1;

    #if USE_SHIFTDIFF_1
        i = shiftdiff_equivalence(i1->signature, i2->signature,
            shiftdiff1_falloff, shiftdiff1_veto_threshold, opt->shiftdiff1_threshold);
        if (i == -1) return -1;
        state |= i;
    #endif

    #if USE_SHIFTDIFF_2
        i = shiftdiff_equivalence(i1->signature2, i2->signature2,
            shiftdiff2_falloff, shiftdiff2_veto_threshold, opt->shiftdiff2_threshold);
        if (i == -1) return -1;
        state |= i;
    #endif

    #if USE_SHIFTDIFF_3
        i = shiftdiff_equivalence(i1->signature, i2->signature,
            shiftdiff3_falloff, shiftdiff3_veto_threshold, opt->shiftdiff3_threshold);
        if (i == -1) return -1;
        state |= i;
    #endif

    i = pith2_is_subset(ptr1, ptr2, opt->pithdiff2_threshold, dpi);
    if (i < 1) return i;
    i = pith2_is_subset(ptr2, ptr1, opt->pithdiff2_threshold, dpi);
    if (i < 1) return i;

    if (opt->method & MDJVU_MATCHER_RAMPAGE)
        return 1;

    #if USE_PITHDIFF
        if (opt->aggression > 0)
        {
            i = pithdiff_equivalence(i1, i2, opt->pithdiff1_threshold, dpi);
            if (i == -1) return 0; /* pithdiff has no right to veto at upper level */
            state |= i;
        }
    #endif

    #if 0
        if (opt->aggression > 0)
        {
            i = softdiff_equivalence(i1, i2, opt->softdiff_threshold, dpi);
            if (i == -1) return 0;  /* softdiff has no right to veto at upper level */
            state |= i;
        }
    #endif

    return state;
}/*}}}*/

MDJVU_IMPLEMENT int mdjvu_match_patterns(mdjvu_pattern_t ptr1, mdjvu_pattern_t ptr2,
                    int32 dpi, mdjvu_matcher_options_t options)
{
    Options *opt;
    int result;
    if (options)
        opt = (Options *) options;
    else
        opt = (Options *) mdjvu_matcher_options_create();

    result = compare_patterns(ptr1, ptr2, dpi, opt);

    if (!options)
        mdjvu_matcher_options_destroy((mdjvu_matcher_options_t) opt);

    return result;
}


MDJVU_IMPLEMENT void mdjvu_pattern_destroy(mdjvu_pattern_t p)/*{{{*/
{
    Image *img = (Image *) p;
    if (img->pixels)
        free_bitmap(img->pixels);

    if (img->pith2_inner)
        free_bitmap_with_margins(img->pith2_inner);

    if (img->pith2_outer)
        free_bitmap_with_margins(img->pith2_outer);

    FREE1(img);
}/*}}}*/
