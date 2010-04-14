/*
**  $Id: yuv_sw_c.h,v 1.2 2003/09/04 06:02:53 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/

#include "newgal.h"
#include "sysvideo.h"

/* This is the software implementation of the YUV video overlay support */

extern GAL_Overlay *GAL_CreateYUV_SW(_THIS, int width, int height, Uint32 format, GAL_Surface *display);

extern int GAL_LockYUV_SW(_THIS, GAL_Overlay *overlay);

extern void GAL_UnlockYUV_SW(_THIS, GAL_Overlay *overlay);

extern int GAL_DisplayYUV_SW(_THIS, GAL_Overlay *overlay, GAL_Rect *dstrect);

extern void GAL_FreeYUV_SW(_THIS, GAL_Overlay *overlay);

