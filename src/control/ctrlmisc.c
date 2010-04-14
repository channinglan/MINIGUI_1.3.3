/*
** $Id: ctrlmisc.c,v 1.19 2003/09/22 01:21:18 snig Exp $
**
** ctrlmisc.c: the Control Misc module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 1999/8/23
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

#include "stdio.h"

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"

#include "ctrlmisc.h"

void GUIAPI NotifyParentEx (HWND hwnd, int id, int code, DWORD add_data)
{
    NOTIFPROC notif_proc = GetNotificationCallback (hwnd);

    if (notif_proc) {
        notif_proc (hwnd, id, code, add_data);
    }
    else {
        SendNotifyMessage (GetParent (hwnd), MSG_COMMAND, 
                                 (WPARAM) MAKELONG (id, code), (LPARAM)hwnd);
    }
}

void Draw3DControlFrame (HDC hdc, int x0, int y0, int x1, int y1, 
            gal_pixel fillc, BOOL updown)
{
    int left, top, right, bottom;
    left = MIN (x0, x1);
    top  = MIN (y0, y1);
    right = MAX (x0, x1);
    bottom = MAX (y0, y1);
    
    if (fillc != 0) {
        SetBrushColor (hdc, fillc);
        FillBox (hdc, left + 1, top + 1, right - left - 1 , bottom - top - 1); 
    }

    if (updown) {
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_RIGHT_OUTER));
        MoveTo (hdc, left, bottom);
        LineTo (hdc, left, top);
        LineTo (hdc, right, top);
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT_OUTER));
        LineTo (hdc, right, bottom);
        LineTo (hdc, left, bottom);

        left++; top++; right--; bottom--;
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT_INNER));
        MoveTo (hdc, left, bottom);
        LineTo (hdc, left, top);
        LineTo (hdc, right, top);
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT_INNER));
        LineTo (hdc, right, bottom);
        LineTo (hdc, left, bottom);
    }
    else {
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT_INNER));
        MoveTo (hdc, left, bottom);
        LineTo (hdc, left, top);
        LineTo (hdc, right, top);
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_RIGHT_OUTER));
        MoveTo (hdc, left, bottom);
        LineTo (hdc, right, bottom);
        LineTo (hdc, right, top);

        left++; top++; right--; bottom--;
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT_OUTER));
        MoveTo (hdc, left, bottom);
        LineTo (hdc, left, top);
        LineTo (hdc, right, top);
        SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT_INNER));
        MoveTo (hdc, left, bottom);
        LineTo (hdc, right, bottom);
        LineTo (hdc, right, top);
    }
}

void DrawFlatControlFrameEx (HDC hdc, int x0, int y0, int x1, int y1, 
            gal_pixel fillc, int corner, int updown)
{
    int left, top, right, bottom;
    left = MIN (x0, x1);
    top  = MIN (y0, y1);
    right = MAX (x0, x1);
    bottom = MAX (y0, y1);
    
    if (fillc != 0) {
        SetBrushColor (hdc, fillc);
        FillBox (hdc, left, top, right - left + 1, bottom - top + 1); 
    }

    SetPenColor (hdc, GetWindowElementColor (WEC_FLAT_BORDER));

    if (corner < 1) {
        Rectangle (hdc, left, top, right, bottom);
        return;
    }

    if (updown == 1) {
        right --; bottom --;
    }

    MoveTo (hdc, left + corner, top);
    LineTo (hdc, right - corner, top);
    LineTo (hdc, right, top + corner);
    LineTo (hdc, right, bottom - corner);
    LineTo (hdc, right - corner, bottom);
    LineTo (hdc, left + corner, bottom);
    LineTo (hdc, left, bottom - corner);
    LineTo (hdc, left, top + corner);
    LineTo (hdc, left + corner, top);

    corner++;
    if (updown == 1) {
        MoveTo (hdc, right + 1, top + corner);
        LineTo (hdc, right + 1, bottom - corner);
        MoveTo (hdc, left + corner, bottom + 1);
        LineTo (hdc, right - corner + 1, bottom + 1);
    }
    else if (updown == 0) {
        MoveTo (hdc, left + corner, top + 1);
        LineTo (hdc, right - corner, top + 1);
        MoveTo (hdc, left + 1, top + corner);
        LineTo (hdc, left + 1, bottom - corner);
    }
}

