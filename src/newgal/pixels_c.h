/*
**  $Id: pixels_c.h,v 1.3 2003/09/04 06:02:53 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/

/* Useful functions and variables from pixel.c */

#include "blit.h"

/* Pixel format functions */
extern GAL_PixelFormat *GAL_AllocFormat(int bpp,
        Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
extern GAL_PixelFormat *GAL_ReallocFormat(GAL_Surface *surface, int bpp,
        Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
extern void GAL_FormatChanged(GAL_Surface *surface);
extern void GAL_FreeFormat(GAL_PixelFormat *format);

/* Blit mapping functions */
extern GAL_BlitMap *GAL_AllocBlitMap(void);
extern void GAL_InvalidateMap(GAL_BlitMap *map);
extern int GAL_MapSurface (GAL_Surface *src, GAL_Surface *dst);
extern void GAL_FreeBlitMap(GAL_BlitMap *map);

/* Miscellaneous functions */
extern Uint16 GAL_CalculatePitch(GAL_Surface *surface);
extern void GAL_DitherColors(GAL_Color *colors, int bpp);
extern Uint8 GAL_FindColor(GAL_Palette *pal, Uint8 r, Uint8 g, Uint8 b);
extern void GAL_ApplyGamma(Uint16 *gamma, GAL_Color *colors, GAL_Color *output, int ncolors);
