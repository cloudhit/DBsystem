// ********************************************************
// Some helper macros for working with bitmaps
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

#endif  //_BITMAP_H
