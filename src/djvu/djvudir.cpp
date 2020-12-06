/*
 * djvusave.c - saving DjVuBitonal pages
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include "bs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

MDJVU_IMPLEMENT void mdjvu_write_dirm_bundled(char **elements, int * sizes, 
    int n, mdjvu_file_t f, mdjvu_error_t *perr)
{
    int i, flag, offpos, end, off, *offsets;
    
    // version number and the DJVU bundled flag
    fputc(1 | ((1)<<7), (FILE *) f);
    // Number of files
    mdjvu_write_big_endian_int16((uint16) n, f);
    
    offpos = ftell((FILE *) f);
    offsets = (int *) calloc(n, sizeof(int));
    // Dummy offsets (will rewrite them later)
    for (i=0; i<n; i++)
    {
        mdjvu_write_big_endian_int32((uint32) 0, f);
    }
    
    {
        BSEncoder bse((FILE *) f);
        // Encode file sizes and calculate offsets
        bse.write24(sizes[0]);
        for (i=1; i<n; i++) 
        {
            offsets[i] = offsets[i-1] + sizes[i-1];
            if (sizes[i-1] & 1) offsets[i]++;
            bse.write24(sizes[i]);
        }
        
        // Encode DJVU flags (the only bit meaningful in our context indicates
        // if this is a DJVU page (1) or a shared file (0))
        for (i=0; i<n; i++)
        {
            flag = mdjvu_ends_with_ignore_case(elements[i],".djvu") ? 1 : 0;
            bse.write8(flag);
        }
    
        // Encode file IDs (no names, no titles)
        for (i=0; i<n; i++)
        {
            bse.write(elements[i],strlen(elements[i]));
            bse.write8(0);
        }

        // Close BZZ encoder
    }
    end = off = ftell((FILE *) f);
    if (end & 1) off++;
        
    // Rewind to the offset array
    fseek((FILE *) f, offpos, SEEK_SET);
    for (i=0; i<n; i++)
        mdjvu_write_big_endian_int32((uint32) (offsets[i] + off), f);
    free(offsets);

    fseek((FILE *) f, end, SEEK_SET);
}

MDJVU_IMPLEMENT void mdjvu_write_dirm_indirect(char **elements, int * sizes, 
    int n, mdjvu_file_t f, mdjvu_error_t *perr)
{
    int i, flag;
    BSEncoder bse((FILE *) f);
    
    // version number
    fputc(1, (FILE *) f);
    // Number of files
    mdjvu_write_big_endian_int16((uint16) n, f);
    
    // Encode file sizes
    for (i=0; i<n; i++) 
        bse.write24(sizes[i]);
        
    // Encode DJVU flags (the only bit meaningful in our context indicates
    // if this is a DJVU page (1) or a shared file (0))
    for (i=0; i<n; i++)
    {
        flag = mdjvu_ends_with_ignore_case(elements[i],".djvu") ? 1 : 0;
        bse.write8(flag);
    }

    // Encode file IDs (no names, no titles)
    for (i=0; i<n; i++)
    {
        bse.write(elements[i],strlen(elements[i]));
        bse.write8(0);
    }
}


