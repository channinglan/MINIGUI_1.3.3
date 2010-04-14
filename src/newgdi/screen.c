/*
** $Id: screen.c,v 1.5 2003/09/04 06:02:53 weiym Exp $
**
** screen.c: Screen operations of GDI
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/06/12, derived from original gdi.c
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

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"
#include "cursor.h"

#ifdef _MISC_SAVESCREEN
BOOL GUIAPI SaveMainWindowContent (HWND hWnd, const char* filename)
{
    RECT rcScreen;
    RECT rcWin;
    BITMAP bitmap;
    int save_ret;

    SetRect (&rcScreen, 0, 0, WIDTHOFPHYGC, HEIGHTOFPHYGC);
    if (hWnd) {
        GetWindowRect (hWnd, &rcWin);
        if (!IntersectRect (&rcWin, &rcWin, &rcScreen))
            return FALSE;
    }
    else
        rcWin = rcScreen;

    bitmap.bmWidth = RECTW (rcWin);
    bitmap.bmHeight = RECTH (rcWin);

    if (bitmap.bmWidth == 0 || bitmap.bmHeight == 0) {
#ifdef _DEBUG
        fprintf (stderr, "SaveMainWindowContent: Empty Rect.\n");
#endif
        return FALSE;
    }
    
    bitmap.bmBits = NULL;
    GetBitmapFromDC (HDC_SCREEN, rcWin.left, rcWin.top,
                    RECTW (rcWin), RECTH (rcWin), &bitmap);

    if (!bitmap.bmBits) {
#ifdef _DEBUG
        fprintf (stderr, "SaveMainWindowContent: SaveBox error.\n");
#endif
        return FALSE;
    }
    
    save_ret = SaveBitmap (HDC_SCREEN, &bitmap, filename);
    free (bitmap.bmBits);
    return (save_ret == 0);
}
#endif /* _MISC_SAVESCREEN */

