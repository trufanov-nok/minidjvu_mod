/*
 * 6string.c - some standard funtions for string manipulation
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

/* Under Windows (MSVC), there is usually no strcasecmp.
 * So here's the rewrite.
 */
static int my_strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;
    while(*s1)
    {
        int d;
        c1 = tolower(*s1++); c2 = tolower(*s2++);
        d = c1 - c2;
        if (d) return d;
    }
    return *s2;
}

MDJVU_IMPLEMENT int mdjvu_ends_with_ignore_case(const char *s, const char *prefix)
{
    size_t sl = strlen(s);
    size_t pl = strlen(prefix);
    if (sl < pl) return 0;
    return !my_strcasecmp(s + sl - pl, prefix);
}


