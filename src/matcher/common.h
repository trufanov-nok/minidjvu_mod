/*
 * common.h - some common macros
 */


#ifndef MDJVU_COMMON_H
#define MDJVU_COMMON_H


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif


#ifdef __cplusplus
#   define FUNCTIONS_BEGIN extern "C" {
#   define FUNCTIONS_END }
#else
#   define FUNCTIONS_BEGIN
#   define FUNCTIONS_END
#endif



#include <stdlib.h>

#ifdef __cplusplus
/* Stuff for not using malloc in C++
 * (made by Leon Bottou; has no use in minidjvu-mod,
 * but left here for potential DjVuLibre compatibility)
 */
# define MALLOC1(Type)    new Type
# define FREE1(p)         delete (p)
# define MALLOC(Type,n)   new Type[n]
# define FREE(p)          delete [] (p)
# define REALLOC          oops! I hope we have no REALLOC in minidjvu-mod...
#else
# define MALLOC1(TYPE)           ( (TYPE *) malloc(sizeof(TYPE)) )
# define MALLOC(TYPE, N)         ( (TYPE *) malloc((N) * sizeof(TYPE)) )
# define REALLOC(TYPE, PTR, N)   ( (TYPE *) realloc(PTR, (N) * sizeof(TYPE)) )
# define FREE1(PTR)              free(PTR)
# define FREE(PTR)               free(PTR)
#endif

/* Yeah, I know this is ugly and there are C++ templates for that.
 * But this is for C code.
 */
#define LIST_APPEND(TYPE, LIST, COUNT, ALLOCATED) \
{                                                 \
    if ((COUNT) == (ALLOCATED))                   \
    {                                             \
        (ALLOCATED) <<= 1;  /* double the list */ \
        (LIST) = REALLOC(TYPE, LIST, ALLOCATED);  \
    }                                             \
    return &(LIST)[(COUNT)++];                    \
}


#endif
