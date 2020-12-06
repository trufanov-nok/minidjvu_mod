/*
 * mdjvucfg.h - file to be included first in all C sources in the library
 */

#ifdef MINIDJVU_INCLUDED_FROM_INSIDE
#error mdjvucfg.h should be included only once in every minidjvu-mod source file
#endif


/*
 * This is needed, for example, to distinguish between dllexport and dllimport.
 */
#define MINIDJVU_INCLUDED_FROM_INSIDE

/*
 * Bridge to autoconf configuration file.
 */
#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

/* IM: I'm commenting #ifdef out because it looks like we must use dgettext in
 *     the shared library. Yes, it's ugly to redefine a system macro but...
 * #ifndef _ 
 */

#ifdef HAVE_GETTEXT
    #include <libintl.h>
    #define _(msgid)	dgettext("minidjvu-mod", msgid)
#else
    #define _(msgid)	(msgid)
#endif

/* #endif */


/**
 * Global initialization of the shared library.
 * It's OK to call it more than once.
 * This function is to be called from every constructor
 * so that there's no type-correct way to use minidjvu-mod
 * without calling mdjvu_init() behind the scenes.
 *
 * This function is implemented in 0porting.c.
 */
void mdjvu_init(void);
