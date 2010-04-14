/*
** $Id: yuv.c,v 1.4 2003/09/04 06:02:53 weiym Exp $
**
** yuv.c: YUV overlay support.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/11/06
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
** TODO:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"
#include "pixel_ops.h"
#include "cursor.h"

/****************************** YUV overlay support **************************/
/* Create a YUV overlay */
GAL_Overlay* GUIAPI CreateYUVOverlay (int width, int height, Uint32 format, HDC hdc)
{
    PDC pdc = dc_HDC2PDC (hdc);

    return GAL_CreateYUVOverlay (width, height, format, pdc->surface);
}

/* Display a YUV overlay */
void GUIAPI DisplayYUVOverlay (GAL_Overlay* overlay, const RECT* dstrect)
{
    GAL_Rect dst;
    PDC pdc;
    RECT rcOutput = {0, 0, WIDTHOFPHYGC, HEIGHTOFPHYGC};

    if (!(pdc = check_ecrgn (HDC_SCREEN)))
        return;

    if (dstrect) {
        rcOutput = *dstrect;
        dst.x = dstrect->left;
        dst.y = dstrect->top;
        dst.w = RECTWP(dstrect);
        dst.h = RECTHP(dstrect);
    }
    else {
        dst.x = 0;
        dst.y = 0;
        dst.w = WIDTHOFPHYGC;
        dst.h = HEIGHTOFPHYGC;
    }

    pdc->rc_output = rcOutput;
    ENTER_DRAWING (pdc);
    GAL_DisplayYUVOverlay (overlay, &dst);
    LEAVE_DRAWING (pdc);
    UNLOCK_GCRINFO (pdc);
}

