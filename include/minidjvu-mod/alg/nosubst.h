/*
 * nosubst.h - guessing what chunks are not letters and should not be changed
 */

/*
 * This is the algorithm that mark bitmaps that cannot have a substitution.
 * 
 * First, it looks what blits are suspiciously big. They will be no-subst.
 * A blit with a bounding box that intersects a no-subst box will also be no-subst.
 * And so no-subst infection is spread until no more new no-substs are found.
 * 
 * You may ask, why this \expandafter when the splitter could simply mark results
 * of splitting suspiciously big bitmaps as no-subst?
 * Answer: this algorithm works even when we've read a DjVu page and didn't
 * actually render it. Well, it works at least with cjb2-encoded files.
 */

MDJVU_FUNCTION void mdjvu_calculate_not_a_letter_flags(mdjvu_image_t);
