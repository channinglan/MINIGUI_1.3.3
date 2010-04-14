/*
**  $Id: yuvfuncs.h,v 1.2 2003/09/04 06:02:53 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/

/* This is the definition of the YUV video surface function structure */

#include "newgal.h"
#include "sysvideo.h"

#ifndef _THIS
#define _THIS	GAL_VideoDevice *_this
#endif
struct private_yuvhwfuncs {
	int (*Lock)(_THIS, GAL_Overlay *overlay);
	void (*Unlock)(_THIS, GAL_Overlay *overlay);
	int (*Display)(_THIS, GAL_Overlay *overlay, GAL_Rect *dstrect);
	void (*FreeHW)(_THIS, GAL_Overlay *overlay);
};
