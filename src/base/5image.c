/*
 * 5image.c - manipulating split images, the main data structure of minidjvu
 */

#include "../base/mdjvucfg.h"
#include <minidjvu/minidjvu.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define IMG ((Image *) image)



/* _______________________________   artifacts   ___________________________ */

/* Artifacts are pieces of information stored for every bitmap. */
/* Each artifact type is initially disabled, but may be turned on later. */

/* When adding new artifact type, see if it needs destructor other than just
 * cleaning the memory. Modify mdjvu_image_disable_artifact() as necessary.
 */

typedef enum
{
    mdjvu_artifact_prototype,
    mdjvu_artifact_substitution,
    mdjvu_artifact_not_a_letter_flag,
    mdjvu_artifact_suspiciously_big_flag,
    mdjvu_artifact_mass,
    mdjvu_artifact_dictionary_index,
    mdjvu_artifact_center,
    mdjvu_artifacts_count
} mdjvu_artifact_type_enum;

typedef struct
{
    int32 x, y;
} Point;

#define MDJVU_ARTIFACT_SIZES \
    { \
        sizeof(mdjvu_bitmap_t), \
        sizeof(mdjvu_bitmap_t), \
        sizeof(unsigned char), \
        sizeof(unsigned char), \
        sizeof(int32), \
        sizeof(int32), \
        sizeof(Point)  \
    }

const int32 artifact_sizes[] = MDJVU_ARTIFACT_SIZES;

#define MAX_ARTIFACT_SIZE 16  /* supposing that pointers can't have size > 16 */



typedef struct
{
    int32 width, height;

    /* bitmaps */
    mdjvu_bitmap_t *bitmaps;
    int32 bitmaps_count, bitmaps_allocated;

    /* blits */
    int32 *x, *y;
    mdjvu_bitmap_t *blits;
    int32 blits_count, blits_allocated;

    /* image additional info */
    mdjvu_image_t dictionary;
    int32 resolution; /* 0 - unknown */

    void *artifacts[mdjvu_artifacts_count];
} Image;



static void initialize_artifact(void **artifacts, mdjvu_bitmap_t bitmap, mdjvu_artifact_type_enum a)
{
    int32 i = mdjvu_bitmap_get_index(bitmap);
    switch(a)
    {
        case mdjvu_artifact_prototype:
            ((mdjvu_bitmap_t *) artifacts[mdjvu_artifact_prototype])[i] = NULL;
        break;
        case mdjvu_artifact_substitution:
            ((mdjvu_bitmap_t *) artifacts[mdjvu_artifact_substitution])[i] = NULL;
        break;
        case mdjvu_artifact_mass:
            ((int32 *) artifacts[mdjvu_artifact_mass])[i] =
                mdjvu_bitmap_get_mass(bitmap);
        break;
        case mdjvu_artifact_dictionary_index:
            ((int32 *) artifacts[mdjvu_artifact_dictionary_index])[i] = -1;
        break;
        case mdjvu_artifact_not_a_letter_flag:
            ((unsigned char *) artifacts[mdjvu_artifact_not_a_letter_flag])[i] = 0;
        break;
        case mdjvu_artifact_suspiciously_big_flag:
            ((unsigned char *) artifacts[mdjvu_artifact_suspiciously_big_flag])[i] = 0;
        break;
        case mdjvu_artifact_center:  /* initializing centers may be non-obvious */
        case mdjvu_artifacts_count:; /* just to complete switch */
    }
}

static void initialize_artifacts(void **artifacts, mdjvu_bitmap_t bitmap)
{
    int a;
    for (a = 0; a < mdjvu_artifacts_count; a++)
    {
        if (artifacts[a])
            initialize_artifact(artifacts, bitmap,
                                (mdjvu_artifact_type_enum) a);
    }
}

static void mdjvu_image_enable_artifact
    (mdjvu_image_t image, mdjvu_artifact_type_enum artifact_index)
{
    int32 i;
    int32 n = IMG->bitmaps_count;
    if (!IMG->artifacts[artifact_index])
    {
        IMG->artifacts[artifact_index] =
            malloc(IMG->bitmaps_allocated * artifact_sizes[artifact_index]);
    }
    for (i = 0; i < n; i++)
    {
        initialize_artifact(IMG->artifacts, IMG->bitmaps[i], artifact_index);
    }
}

static void mdjvu_image_disable_artifact
    (mdjvu_image_t image, mdjvu_artifact_type_enum artifact_index)
{
    if (IMG->artifacts[artifact_index])
    {
        free(IMG->artifacts[artifact_index]);
        IMG->artifacts[artifact_index] = NULL;
    }
}


/* ______________________________   create   ________________________________ */

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_image_create(int32 width, int32 height)
{
    int i;
    Image *image = (Image *) malloc(sizeof(Image));
    
    mdjvu_init();

    image->width = width;
    image->height = height;

    image->bitmaps_allocated = 16;
    image->bitmaps = (mdjvu_bitmap_t *)
        malloc(image->bitmaps_allocated * sizeof(mdjvu_bitmap_t));
    image->bitmaps_count = 0;

    image->blits_allocated = 32;
    image->x = (int32 *) malloc(image->blits_allocated * sizeof(int32));
    image->y = (int32 *) malloc(image->blits_allocated * sizeof(int32));
    image->blits = (mdjvu_bitmap_t *)
        malloc(image->blits_allocated * sizeof(mdjvu_bitmap_t));
    image->blits_count = 0;

    image->dictionary = NULL;
    image->resolution = 0;

    for (i = 0; i < mdjvu_artifacts_count; i++)
        image->artifacts[i] = NULL;

    return (mdjvu_image_t) image;
}

/* ______________________________   size of image in memory  __________________________ */

MDJVU_IMPLEMENT int mdjvu_image_mem_size(mdjvu_image_t img)
{
    int res = sizeof(Image);
    Image *image = (Image *) img;
    res += image->blits_allocated * sizeof(int32) * 2 +
            image->blits_allocated * sizeof(mdjvu_bitmap_t);

    for (int i = 0; i < mdjvu_artifacts_count; i++)
        res += artifact_sizes[i]*image->bitmaps_allocated;

    for (int i = 0; i < image->bitmaps_count; i++)
        res += mdjvu_bitmap_mem_size(image->bitmaps[i]);

    if (image->dictionary) {
        res += mdjvu_image_mem_size(image->dictionary);
    }

    return res;
}


/* ______________________________   destroy   _______________________________ */

MDJVU_IMPLEMENT void mdjvu_image_destroy(mdjvu_image_t image)
{
    int32 i;
    int k;
    free(IMG->blits);
    free(IMG->x);
    free(IMG->y);
    for (k = 0; k < mdjvu_artifacts_count; k++)
    {
        mdjvu_image_disable_artifact(image, (mdjvu_artifact_type_enum) k);
    }
    for (i = 0; i < IMG->bitmaps_count; i++)
        mdjvu_bitmap_destroy(IMG->bitmaps[i]);
    free(IMG->bitmaps);
    free(IMG);
}

/* ______________________________   get/set   ______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_get_width(mdjvu_image_t image)
    { return IMG->width; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_height(mdjvu_image_t image)
    { return IMG->height; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_bitmap_count(mdjvu_image_t image)
    { return IMG->bitmaps_count; }

MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_count(mdjvu_image_t image)
    { return IMG->blits_count; }

MDJVU_IMPLEMENT void mdjvu_image_set_dictionary(mdjvu_image_t image, mdjvu_image_t dict)
{
    IMG->dictionary = dict;
}

MDJVU_IMPLEMENT mdjvu_image_t mdjvu_image_get_dictionary(mdjvu_image_t image)
{
    return IMG->dictionary;
}


/* ______________________________   bitmaps   ______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_add_bitmap(mdjvu_image_t image, mdjvu_bitmap_t bmp)
{
    if (IMG->bitmaps_count == IMG->bitmaps_allocated)
    {
        int i;
        IMG->bitmaps_allocated <<= 1;
        IMG->bitmaps = (mdjvu_bitmap_t *) realloc(IMG->bitmaps,
                            IMG->bitmaps_allocated * sizeof(mdjvu_bitmap_t));
        for (i = 0; i < mdjvu_artifacts_count; i++)
        {
            if (IMG->artifacts[i])
            {
                IMG->artifacts[i] = realloc(IMG->artifacts[i],
                            IMG->bitmaps_allocated * artifact_sizes[i]);
            }
        }
    }
    IMG->bitmaps[IMG->bitmaps_count] = bmp;
    mdjvu_bitmap_set_index(bmp, IMG->bitmaps_count);
    initialize_artifacts(IMG->artifacts, bmp);
    return IMG->bitmaps_count++;
}

MDJVU_IMPLEMENT int mdjvu_image_has_bitmap(mdjvu_image_t image, mdjvu_bitmap_t bitmap)
{
    int32 i = mdjvu_bitmap_get_index(bitmap);
    if (i >= IMG->bitmaps_count) return 0;
    return bitmap == IMG->bitmaps[i];
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_get_bitmap(mdjvu_image_t image, int32 i)
{
    return IMG->bitmaps[i];
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_new_bitmap(mdjvu_image_t image, int32 w, int32 h)
{
    mdjvu_bitmap_t bmp = mdjvu_bitmap_create(w, h);
    mdjvu_image_add_bitmap(image, bmp);
    return bmp;
}

MDJVU_IMPLEMENT void mdjvu_image_exchange_bitmaps
    (mdjvu_image_t image, int32 i1, int32 i2)
{
    int a;
    mdjvu_bitmap_t b1 = IMG->bitmaps[i1], b2 = IMG->bitmaps[i2];
    if (i1 == i2) return;

    IMG->bitmaps[i1] = b2;
    IMG->bitmaps[i2] = b1;
    mdjvu_bitmap_set_index(b1, i2);
    mdjvu_bitmap_set_index(b2, i1);

    for (a = 0; a < mdjvu_artifacts_count; a++)
    {
        if (IMG->artifacts[a])
        {
            char buf[MAX_ARTIFACT_SIZE];
            memcpy(buf,
                   ((char *) IMG->artifacts[a]) + i1 * artifact_sizes[a],
                   artifact_sizes[a]);
            memcpy(((char *) IMG->artifacts[a]) + i1 * artifact_sizes[a],
                   ((char *) IMG->artifacts[a]) + i2 * artifact_sizes[a],
                   artifact_sizes[a]);
            memcpy(((char *) IMG->artifacts[a]) + i2 * artifact_sizes[a],
                   buf,
                   artifact_sizes[a]);
        }
    }
}

MDJVU_IMPLEMENT int mdjvu_image_check_indices(mdjvu_image_t image)
{
    int32 n = IMG->bitmaps_count;
    int32 i;
    for (i = 0; i < n; i++)
    {
        if (mdjvu_bitmap_get_index(IMG->bitmaps[i]) != i)
            return 0;
    }
    return 1;
}

MDJVU_IMPLEMENT void mdjvu_image_remove_unused_bitmaps(mdjvu_image_t image)
{
    int32 b = IMG->blits_count;
    int32 n = IMG->bitmaps_count;
    int32 i, filled;
    int32 *use_count = (int32 *) calloc(n, sizeof(int32));
    mdjvu_bitmap_t *new_bitmaps;
    int32 new_bitmaps_count, a;
    void *new_artifacts[mdjvu_artifacts_count];

    for (i = 0; i < b; i++)
    {
        mdjvu_bitmap_t bmp = IMG->blits[i];
        if (mdjvu_image_has_bitmap(image, bmp))
            use_count[mdjvu_bitmap_get_index(bmp)]++;
    }

    new_bitmaps_count = 0;
    for (i = 0; i < n; i++)
    {
        if (use_count[i])
            new_bitmaps_count++;
    }

    /* create new bitmap and artifact placeholders */
    new_bitmaps = (mdjvu_bitmap_t *)
        malloc(new_bitmaps_count * sizeof(mdjvu_bitmap_t));
    for (a = 0; a < mdjvu_artifacts_count; a++)
    {
        if (IMG->artifacts[a])
            new_artifacts[a] = malloc(new_bitmaps_count * artifact_sizes[a]);
        else
            new_artifacts[a] = NULL;
    }

    filled = 0;
    for (i = 0; i < n; i++)
    {
        if (use_count[i])
        {
            mdjvu_bitmap_set_index(IMG->bitmaps[i], filled);
            new_bitmaps[filled] = IMG->bitmaps[i];
            for (a = 0; a < mdjvu_artifacts_count; a++)
            {
                if (IMG->artifacts[a])
                {
                    memcpy((char *) new_artifacts[a] + filled * artifact_sizes[a],
                           (char *) IMG->artifacts[a] + i * artifact_sizes[a],
                           artifact_sizes[a]);
                }
            }
            filled++;
        }
        else
            mdjvu_bitmap_destroy(IMG->bitmaps[i]);
    }

    free(use_count);
    free(IMG->bitmaps);
    IMG->bitmaps = new_bitmaps;
    IMG->bitmaps_count = IMG->bitmaps_allocated = new_bitmaps_count;

    for (a = 0; a < mdjvu_artifacts_count; a++)
    {
        if (IMG->artifacts[a])
        {
            free(IMG->artifacts[a]);
            IMG->artifacts[a] = new_artifacts[a];
            /* another place to delete artifacts that need deleting... */
        }
    }
}


/* _______________________________   blits   _______________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_add_blit(mdjvu_image_t image,
                                          int32 x, int32 y,
                                          mdjvu_bitmap_t bitmap)
{
    if (IMG->blits_count == IMG->blits_allocated)
    {
        IMG->blits_allocated <<= 1;
        IMG->x = (int32 *) realloc(IMG->x,
                                IMG->blits_allocated * sizeof(int32));
        IMG->y = (int32 *) realloc(IMG->y,
                                IMG->blits_allocated * sizeof(int32));
        IMG->blits = (mdjvu_bitmap_t *) realloc(IMG->blits,
                                IMG->blits_allocated * sizeof(mdjvu_bitmap_t));
    }
    IMG->x[IMG->blits_count] = x;
    IMG->y[IMG->blits_count] = y;
    IMG->blits[IMG->blits_count] = bitmap;
    return IMG->blits_count++;
}

MDJVU_IMPLEMENT void mdjvu_image_remove_NULL_blits(mdjvu_image_t image)
{
    int32 *new_x = (int32 *) malloc(IMG->blits_count * sizeof(int32));
    int32 *new_y = (int32 *) malloc(IMG->blits_count * sizeof(int32));
    mdjvu_bitmap_t *new_blits = (mdjvu_bitmap_t *)
        malloc(IMG->blits_count * sizeof(mdjvu_bitmap_t));
    int32 filled = 0, i;

    for (i = 0; i < IMG->blits_count; i++)
    {
        if (IMG->blits[i])
        {
            new_x[filled] = IMG->x[i];
            new_y[filled] = IMG->y[i];
            new_blits[filled] = IMG->blits[i];
            filled++;
        }
    }

    free(IMG->x);
    free(IMG->y);
    free(IMG->blits);
    IMG->x = new_x;
    IMG->y = new_y;
    IMG->blits = new_blits;
    IMG->blits_allocated = IMG->blits_count;
    IMG->blits_count = filled;
}


MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_x(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->x[i];
}

MDJVU_IMPLEMENT int32 mdjvu_image_get_blit_y(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->y[i];
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_x(mdjvu_image_t image, int32 i, int32 x)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->x[i] = x;
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_y(mdjvu_image_t image, int32 i, int32 y)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->y[i] = y;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t
    mdjvu_image_get_blit_bitmap(mdjvu_image_t image, int32 i)
{
    assert(i >= 0 && i < IMG->blits_count);
    return IMG->blits[i];
}

MDJVU_IMPLEMENT void mdjvu_image_set_blit_bitmap(mdjvu_image_t image, int32 i, mdjvu_bitmap_t b)
{
    assert(i >= 0 && i < IMG->blits_count);
    IMG->blits[i] = b;
}


MDJVU_IMPLEMENT void mdjvu_image_exchange_blits(mdjvu_image_t image, int32 i, int32 j)
{
    int32 t; mdjvu_bitmap_t b;
    if (i == j) return;
    t = IMG->x[i];     IMG->x[i]     = IMG->x[j];     IMG->x[j] = t;
    t = IMG->y[i];     IMG->y[i]     = IMG->y[j];     IMG->y[j] = t;
    b = IMG->blits[i]; IMG->blits[i] = IMG->blits[j]; IMG->blits[j] = b;
}


/* _____________________________   image info   ____________________________ */

MDJVU_IMPLEMENT int32 mdjvu_image_get_resolution(mdjvu_image_t image)
{
    return IMG->resolution;
}

MDJVU_IMPLEMENT void mdjvu_image_set_resolution(mdjvu_image_t image, int32 dpi)
{
    IMG->resolution = dpi;
}

/* _____________________________   artifacts   _____________________________ */


MDJVU_IMPLEMENT void mdjvu_image_enable_prototypes(mdjvu_image_t image)
    { mdjvu_image_enable_artifact(image, mdjvu_artifact_prototype); }

MDJVU_IMPLEMENT void mdjvu_image_enable_substitutions(mdjvu_image_t image)
    { mdjvu_image_enable_artifact(image, mdjvu_artifact_substitution); }

MDJVU_IMPLEMENT void mdjvu_image_enable_masses(mdjvu_image_t image)
    { mdjvu_image_enable_artifact(image, mdjvu_artifact_mass); }

MDJVU_IMPLEMENT void mdjvu_image_enable_dictionary_indices(mdjvu_image_t image)
    { mdjvu_image_enable_artifact(image, mdjvu_artifact_dictionary_index); }

MDJVU_IMPLEMENT void mdjvu_image_enable_suspiciously_big_flags(mdjvu_image_t image)
    { mdjvu_image_enable_artifact(image, mdjvu_artifact_suspiciously_big_flag); }

MDJVU_IMPLEMENT void mdjvu_image_enable_not_a_letter_flags(mdjvu_image_t image)
    { mdjvu_image_enable_artifact(image, mdjvu_artifact_not_a_letter_flag); }

MDJVU_IMPLEMENT void mdjvu_image_enable_centers(mdjvu_image_t image)
    { mdjvu_image_enable_artifact(image, mdjvu_artifact_center); }


MDJVU_IMPLEMENT void mdjvu_image_disable_prototypes(mdjvu_image_t image)
    { mdjvu_image_disable_artifact(image, mdjvu_artifact_prototype); }

MDJVU_IMPLEMENT void mdjvu_image_disable_substitutions(mdjvu_image_t image)
    { mdjvu_image_disable_artifact(image, mdjvu_artifact_substitution); }

MDJVU_IMPLEMENT void mdjvu_image_disable_masses(mdjvu_image_t image)
    { mdjvu_image_disable_artifact(image, mdjvu_artifact_mass); }

MDJVU_IMPLEMENT void mdjvu_image_disable_dictionary_indices(mdjvu_image_t image)
    { mdjvu_image_disable_artifact(image, mdjvu_artifact_dictionary_index); }

MDJVU_IMPLEMENT void mdjvu_image_disable_suspiciously_big_flags(mdjvu_image_t image)
    { mdjvu_image_disable_artifact(image, mdjvu_artifact_suspiciously_big_flag); }

MDJVU_IMPLEMENT void mdjvu_image_disable_not_a_letter_flags(mdjvu_image_t image)
    { mdjvu_image_disable_artifact(image, mdjvu_artifact_not_a_letter_flag); }

MDJVU_IMPLEMENT void mdjvu_image_disable_centers(mdjvu_image_t image)
    { mdjvu_image_disable_artifact(image, mdjvu_artifact_center); }


MDJVU_IMPLEMENT int mdjvu_image_has_prototypes(mdjvu_image_t image)
    { return IMG->artifacts[mdjvu_artifact_prototype] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_substitutions(mdjvu_image_t image)
    { return IMG->artifacts[mdjvu_artifact_substitution] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_masses(mdjvu_image_t image)
    { return IMG->artifacts[mdjvu_artifact_mass] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_dictionary_indices(mdjvu_image_t image)
    { return IMG->artifacts[mdjvu_artifact_dictionary_index] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_suspiciously_big_flags(mdjvu_image_t image)
    { return IMG->artifacts[mdjvu_artifact_suspiciously_big_flag] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_not_a_letter_flags(mdjvu_image_t image)
    { return IMG->artifacts[mdjvu_artifact_not_a_letter_flag] != NULL; }

MDJVU_IMPLEMENT int mdjvu_image_has_centers(mdjvu_image_t image)
    { return IMG->artifacts[mdjvu_artifact_center] != NULL; }


MDJVU_IMPLEMENT int mdjvu_image_get_not_a_letter_flag(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((unsigned char *) IMG->artifacts[mdjvu_artifact_not_a_letter_flag])
        [mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT void mdjvu_image_set_not_a_letter_flag(mdjvu_image_t image, mdjvu_bitmap_t b, int v)
{
    ((unsigned char *) IMG->artifacts[mdjvu_artifact_not_a_letter_flag])
        [mdjvu_bitmap_get_index(b)] = v ? 1 : 0; /* not to fail on 256... */
}

MDJVU_IMPLEMENT int mdjvu_image_get_suspiciously_big_flag(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((unsigned char *) IMG->artifacts[mdjvu_artifact_suspiciously_big_flag])
        [mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT void mdjvu_image_set_suspiciously_big_flag(mdjvu_image_t image, mdjvu_bitmap_t b, int v)
{
    ((unsigned char *) IMG->artifacts[mdjvu_artifact_suspiciously_big_flag])
        [mdjvu_bitmap_get_index(b)] = v ? 1 : 0; /* not to fail on 256... */
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_get_prototype(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((mdjvu_bitmap_t *) IMG->artifacts[mdjvu_artifact_prototype])
            [mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT void mdjvu_image_set_prototype(mdjvu_image_t image, mdjvu_bitmap_t b, mdjvu_bitmap_t prototype)
{
    ((mdjvu_bitmap_t *) IMG->artifacts[mdjvu_artifact_prototype])
        [mdjvu_bitmap_get_index(b)] = prototype;
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_image_get_substitution(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    mdjvu_bitmap_t subst;
    if (!IMG->artifacts[mdjvu_artifact_substitution]) return b;
    if (!b) return b; /* special handy case */
    subst =
        ((mdjvu_bitmap_t *) IMG->artifacts[mdjvu_artifact_substitution])
            [mdjvu_bitmap_get_index(b)];
    return subst ? subst : b;
}

MDJVU_IMPLEMENT int32 mdjvu_image_get_mass(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((int32 *) IMG->artifacts[mdjvu_artifact_mass])[mdjvu_bitmap_get_index(b)];
}

MDJVU_IMPLEMENT int32 mdjvu_image_get_dictionary_index(mdjvu_image_t image, mdjvu_bitmap_t b)
{
    return ((int32 *) IMG->artifacts[mdjvu_artifact_dictionary_index])[mdjvu_bitmap_get_index(b)];
}
MDJVU_IMPLEMENT void mdjvu_image_set_dictionary_index(mdjvu_image_t image, mdjvu_bitmap_t b, int32 v)
{
    ((int32 *) IMG->artifacts[mdjvu_artifact_dictionary_index])[mdjvu_bitmap_get_index(b)] = v;
}

MDJVU_IMPLEMENT void mdjvu_image_set_substitution(mdjvu_image_t image, mdjvu_bitmap_t b, mdjvu_bitmap_t s)
{
    if (s && b != s && mdjvu_image_has_bitmap(image, s))
        s = mdjvu_image_get_substitution(image, s);

    ((mdjvu_bitmap_t *) IMG->artifacts[mdjvu_artifact_substitution])
        [mdjvu_bitmap_get_index(b)] = s;
}

MDJVU_IMPLEMENT void mdjvu_image_get_center(mdjvu_image_t image, mdjvu_bitmap_t b, int32 *px, int32 *py)
{
    Point *p = (Point *) IMG->artifacts[mdjvu_artifact_center];
    int32 i = mdjvu_bitmap_get_index(b);
    *px = p[i].x;
    *py = p[i].y;
}

MDJVU_IMPLEMENT void mdjvu_image_set_center(mdjvu_image_t image, mdjvu_bitmap_t b, int32 x, int32 y)
{
    Point *p = (Point *) IMG->artifacts[mdjvu_artifact_center];
    int32 i = mdjvu_bitmap_get_index(b);
    p[i].x = x;
    p[i].y = y;
}


/* This function sorts bitmaps approximately according to blits.
 * Algorithm is simple: which shape is used earlier goes first.
 */
MDJVU_IMPLEMENT void mdjvu_image_sort_bitmaps(mdjvu_image_t img)
{
    int32 blit_count  = mdjvu_image_get_blit_count (img);
    int32 blits_passed, shapes_passed = 0;
    for (blits_passed = 0; blits_passed < blit_count; blits_passed++)
    {
        mdjvu_bitmap_t bitmap = mdjvu_image_get_blit_bitmap(img, blits_passed);
        int32 i = mdjvu_bitmap_get_index(bitmap);
        if (i < shapes_passed) continue;
        mdjvu_image_exchange_bitmaps(img, shapes_passed, i);
        shapes_passed++;
    }
}


/* __________________________   removing margins   _________________________ */

MDJVU_IMPLEMENT void mdjvu_image_remove_bitmap_margins(mdjvu_image_t image)
{
    int32 *delta_x, *delta_y;
    int32 n = IMG->bitmaps_count;
    int32 b = IMG->blits_count;
    int32 i;

    delta_x = (int32 *) malloc(n * sizeof(int32));
    delta_y = (int32 *) malloc(n * sizeof(int32));

    for (i = 0; i < n; i++)
        mdjvu_bitmap_remove_margins(IMG->bitmaps[i], &delta_x[i], &delta_y[i]);

    for (i = 0; i < b; i++)
    {
        int32 blit_index = mdjvu_bitmap_get_index(IMG->blits[i]);
        IMG->x[i] += delta_x[blit_index];
        IMG->y[i] += delta_y[blit_index];
    }

    free(delta_x);
    free(delta_y);
}
