/* Plasma OCR - an OCR engine
 *
 * bitmaps.h - some routines that work with bitmaps
 *
 * Copyright (C) 2006  Ilya Mezhirov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* Since Plasma is going into touch with many other projects,
 * I've decided not to write nor use any class for bitmaps.
 * It's always just (unsigned char **pixels, int w, int h).
 */

#ifndef PLASMA_OCR_BITMAP_H
#define PLASMA_OCR_BITMAP_H


#include "common.h"

FUNCTIONS_BEGIN

/* Just allocate a w * h array. */
unsigned char **allocate_bitmap(int w, int h);
void free_bitmap(unsigned char **);
void assign_bitmap(unsigned char **dst, unsigned char **src, int w, int h);
unsigned char **copy_bitmap(unsigned char **, int w, int h);


/* Allocate a w * h bitmap with margins of 1 pixels at each side.
 * Indices into such a bitmap range from -1 to w or h.
 * Free the allocated bitmap with free_bitmap_with_margins().
 */
unsigned char **allocate_bitmap_with_margins(int w, int h);
void free_bitmap_with_margins(unsigned char **);

/* Allocate bitmap with margins and clear those margins. */
unsigned char **allocate_bitmap_with_white_margins(int w, int h);


void print_bitmap(unsigned char **, int w, int h);


/* Allocate a w * h bitmap with margins of 1 pixels at each side.
 * Copy `pixels' there and clear the margins.
 */
unsigned char **provide_margins(unsigned char **, int w, int h, int make_it_0_or_1);


void make_bitmap_0_or_1(unsigned char **, int w, int h);


void invert_bitmap_0_or_1(unsigned char **, int w, int h);

void clear_bitmap(unsigned char **pixels, int w, int h);


/* Clear pixels that have exactly 1 of 4 (or 8) neighbors.
 * (Well, I have in mind some usage for this one).
 *
 * `pixels' must have margins.
 *
 * `result' cannot coincide with `pixels'.
 */
void strip_endpoints_4(unsigned char **result, unsigned char **pixels, int w, int h);
void strip_endpoints_8(unsigned char **result, unsigned char **pixels, int w, int h);

FUNCTIONS_END

#endif
