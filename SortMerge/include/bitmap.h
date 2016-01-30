// ********************************************************
// Some helper macros for working with bitmaps
// $Id: bitmap.h,v 1.1 1997/01/02 12:46:36 flisakow Exp $
// ********************************************************

#ifndef  _BITMAP_H
#define  _BITMAP_H

#include <limits.h>

#define    BIT(n)      (1 << (n))
#define  NOBIT(n)    (~(1 << (n)))

    // Assumes prototypes of (char *bm, int n)
#define  set_bit(bm,n)    (bm)[(n) / CHAR_BIT] |=   BIT((n) % CHAR_BIT)
#define  clr_bit(bm,n)    (bm)[(n) / CHAR_BIT] &= NOBIT((n) % CHAR_BIT)
#define  is_set(bm,n)     (bm)[(n) / CHAR_BIT] &    BIT((n) % CHAR_BIT)
#define  is_clr(bm,n)   (((bm)[(n) / CHAR_BIT] &    BIT((n) % CHAR_BIT)) == 0)

#endif
