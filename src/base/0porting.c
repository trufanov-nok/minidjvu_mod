/*
 * 0porting.c - checking sanity of typedefs
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_GETTEXT
#include <locale.h>
#endif

MDJVU_IMPLEMENT const char *mdjvu_check_sanity(void)
{
    if (sizeof(int32) != 4)
        return "mdjvu_check_sanity(): sizeof(int32) != 4";

    if (sizeof(uint32) != 4)
        return "mdjvu_check_sanity(): sizeof(uint32) != 4";

    if (sizeof(int16) != 2)
        return "mdjvu_check_sanity(): sizeof(int16) != 2";

    if (sizeof(uint16) != 2)
        return "mdjvu_check_sanity(): sizeof(uint16) != 2";

    return NULL;
}


static int initialized = 0;

void mdjvu_init(void)
{
    const char *sanity_error_message;
    
    if (initialized)
        return;

    #ifdef HAVE_GETTEXT
        bindtextdomain("minidjvu-mod", LOCALEDIR);
    #endif

    /* check sizeof(int32) == 4 and such gibberish */
    sanity_error_message = mdjvu_check_sanity();
    if (sanity_error_message)
    {
        fprintf(stderr, "%s\n", sanity_error_message);
        exit(1);
    }

    initialized = 1;
}
