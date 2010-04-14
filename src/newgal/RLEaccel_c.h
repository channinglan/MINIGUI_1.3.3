/*
**  $Id: RLEaccel_c.h,v 1.2 2003/09/04 06:02:53 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/

/* Useful functions and variables from RLEaccel.c */

extern int GAL_RLESurface(GAL_Surface *surface);
extern int GAL_RLEBlit(GAL_Surface *src, GAL_Rect *srcrect,
                       GAL_Surface *dst, GAL_Rect *dstrect);
extern int GAL_RLEAlphaBlit(GAL_Surface *src, GAL_Rect *srcrect,
			    GAL_Surface *dst, GAL_Rect *dstrect);
extern void GAL_UnRLESurface(GAL_Surface *surface, int recode);

