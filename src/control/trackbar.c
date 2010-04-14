/*
** $Id: trackbar.c,v 1.28 2003/09/04 02:40:36 weiym Exp $
**
** trackbar.c: the TrackBar Control module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** NOTE: Originally by Zheng Yiran
**
** Create date: 2000/12/02
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
#include "control.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"

#ifdef _CTRL_TRACKBAR

#include "ctrlmisc.h"
#include "trackbar.h"

/* use these definition when drawing trackbar */
#define WIDTH_VERT_BLANK        10
#define WIDTH_HORZ_BLANK        15
#define GAP_SLIDER              5
#define WIDTH_MID_BLANK         6   /* please keep it even for good appearance */
#define LEN_TICK                4
#define GAP_TIP_SLIDER          12
#define GAP_TICK_SLIDER         6
#define TB_BORDER               2

#define MAX_WIDTH_SLIDER        24
#define MAX_HEIGHT_SLIDER       12

static int TrackBarCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam);

BOOL RegisterTrackBarControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = "trackbar";
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (0);
    WndClass.iBkColor    = GetWindowElementColor (BKC_CONTROL_DEF);
    WndClass.WinProc     = TrackBarCtrlProc;

    return AddNewControlClass (&WndClass) == ERR_OK;
}

#if 0
void TrackBarControlCleanup (void)
{
    return;
}
#endif

static void TrackBarOnDraw (HWND hwnd, HDC hdc, TRACKBARDATA* pData, DWORD dwStyle)
{
    RECT    rcClient;
    int     x, y, w, h, len, mousepos;
    int     pos, max, min, TickFreq, EndTipLen;
    int     sliderx, slidery, sliderw, sliderh;
    int     TickStart, TickEnd;
    float   TickGap, Tick;
    char    sPos[10];

    GetClientRect (hwnd, &rcClient);

    /* get data of trackbar. */
    x = rcClient.left;
    y = rcClient.top;
    w = RECTW (rcClient);
    h = RECTH (rcClient);
    mousepos = pData->mousepos;
    pos = pData->nPos;
    max = pData->nMax;
    min = pData->nMin;
    TickFreq = pData->nTickFreq;

    /* erase background first */
    SetBrushColor (hdc, GetWindowBkColor (hwnd));
    FillBox (hdc, x, y, w, h);

    /* draw the border according to trackbar style. */
#ifdef _FLAT_WINDOW_STYLE
    if (dwStyle & TBS_BORDER) {
        DrawFlatControlFrameEx (hdc, x, y, x + w, y + h, PIXEL_invalid, 0, FALSE);
        x += TB_BORDER;
        y += TB_BORDER;
        w -= TB_BORDER << 1;
        h -= TB_BORDER << 1;
    }
#else
    if (dwStyle & TBS_BORDER) {
        Draw3DControlFrame (hdc, x, y, x + w, y + h, PIXEL_invalid, FALSE);
        x += TB_BORDER;
        y += TB_BORDER;
        w -= TB_BORDER << 1;
        h -= TB_BORDER << 1;
    }
#endif

    /* draw the blank in middle of trackbar. */
#ifdef _FLAT_WINDOW_STYLE
    if (dwStyle & TBS_VERTICAL) {
        DrawFlatControlFrameEx (hdc, x + (w>>1) - (WIDTH_MID_BLANK>>1), y + WIDTH_HORZ_BLANK,
                x + (w>>1) + (WIDTH_MID_BLANK>>1), y + h - WIDTH_HORZ_BLANK, 
                PIXEL_invalid, 0, FALSE);    
    } else {
        DrawFlatControlFrameEx (hdc, x + WIDTH_HORZ_BLANK, y + (h>>1) - (WIDTH_MID_BLANK>>1), 
                x + w - WIDTH_HORZ_BLANK, 
                y + (h>>1) + (WIDTH_MID_BLANK>>1), 
                PIXEL_invalid, 0, FALSE);    
    }
#else
    if (dwStyle & TBS_VERTICAL) {
        Draw3DControlFrame (hdc, x + (w>>1) - (WIDTH_MID_BLANK>>1), y + WIDTH_HORZ_BLANK,
                x + (w>>1) + (WIDTH_MID_BLANK>>1), y + h - WIDTH_HORZ_BLANK, 
                PIXEL_invalid, FALSE); 
    } else {
        Draw3DControlFrame (hdc, x + WIDTH_HORZ_BLANK, y + (h>>1) - (WIDTH_MID_BLANK>>1), 
                x + w - WIDTH_HORZ_BLANK, 
                y + (h>>1) + (WIDTH_MID_BLANK>>1), 
                PIXEL_invalid, FALSE);    
    }
#endif
    
    if (dwStyle & TBS_VERTICAL) {
        sliderw = MAX_HEIGHT_SLIDER;
        sliderh = MAX_WIDTH_SLIDER;
    }
    else {
        sliderh = MAX_HEIGHT_SLIDER;
        sliderw = MAX_WIDTH_SLIDER;
    }

    /* draw the tick of trackbar. */
    if (!(dwStyle & TBS_NOTICK) && TickFreq < (max - min)) {
        SetPenColor (hdc, PIXEL_black);
        if (dwStyle & TBS_VERTICAL) {
            TickStart = y + WIDTH_HORZ_BLANK; 
            TickGap = (h - (WIDTH_HORZ_BLANK<<1)) / (float)(max - min) * TickFreq;
            TickEnd = y + h - WIDTH_HORZ_BLANK;
            for (Tick = TickStart; Tick < TickEnd; Tick += TickGap) { 
                MoveTo (hdc, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER, (int) Tick);
                LineTo (hdc, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER + LEN_TICK, (int) Tick);
            }
            MoveTo (hdc, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER, TickEnd);
            LineTo (hdc, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER + LEN_TICK, TickEnd);
        } else {
            TickStart = x + WIDTH_HORZ_BLANK; 
            TickGap = (w - (WIDTH_HORZ_BLANK<<1)) / (float)(max - min) * TickFreq;
            TickEnd = x + w - WIDTH_HORZ_BLANK;
            for (Tick = TickStart; Tick < TickEnd; Tick += TickGap) { 
                MoveTo (hdc, (int)Tick, y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER);
                LineTo (hdc, (int)Tick, y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER + LEN_TICK);
            }
            MoveTo (hdc, TickEnd, y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER);
            LineTo (hdc, TickEnd, y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER + LEN_TICK);
        }
    }

    /* draw the slider of trackbar according to pos. */
    if (dwStyle & TBS_VERTICAL) {

        sliderx = x + (w>>1) - (sliderw>>1); 
        if (dwStyle & TBS_DRAGGED) {                        /* user is dragging slider */
            len = h - (WIDTH_HORZ_BLANK<<1);
            if ( mousepos > y + h - WIDTH_HORZ_BLANK ) {
                pos = 0 ;
                mousepos = y + h - WIDTH_HORZ_BLANK;
            } else if (mousepos < y + WIDTH_HORZ_BLANK ) {
                pos = len;
                mousepos = y + WIDTH_HORZ_BLANK;
            } else
                pos = y + h - mousepos - WIDTH_HORZ_BLANK ;
            slidery = mousepos - (sliderh>>1);
            pData->nPos = (int)((max - min) * pos / (float)len + 0.5) + min;
        } else 
            slidery = y + WIDTH_HORZ_BLANK + (int)((max - pos) * 
                    (h - (WIDTH_HORZ_BLANK<<1)) / (float)(max - min)) - (sliderh>>1);
    } else {

        slidery = y + (h>>1) - (sliderh>>1); 
        if (dwStyle & TBS_DRAGGED) {
            len = w - (WIDTH_HORZ_BLANK<<1);
            if ( mousepos > x + w - WIDTH_HORZ_BLANK ) {
                pos = len ;
                mousepos = x + w - WIDTH_HORZ_BLANK;
            } else if (mousepos < x + WIDTH_HORZ_BLANK ) {
                    pos = 0;
                mousepos = x + WIDTH_HORZ_BLANK;
            } else
                pos = mousepos - x - WIDTH_HORZ_BLANK ;
            sliderx = mousepos - (sliderw>>1);
            pData->nPos = (int)((max - min) * pos / (float)len + 0.5) + min;
        } else 
            sliderx = x + WIDTH_HORZ_BLANK + (int)((pos - min) * 
                    (w - (WIDTH_HORZ_BLANK<<1)) / (float)(max - min)) - (sliderw>>1);
    }
#ifdef _FLAT_WINDOW_STYLE
    DrawFlatControlFrameEx (hdc, sliderx, slidery, sliderx + sliderw, slidery + sliderh, 
                   GetWindowBkColor (hwnd), 0, TRUE);
#else
    Draw3DControlFrame (hdc, sliderx, slidery, sliderx + sliderw, slidery + sliderh, 
                   GetWindowBkColor (hwnd), TRUE);
#endif
    SetPenColor (hdc, PIXEL_black);
    if (dwStyle & TBS_VERTICAL) {
        MoveTo (hdc, sliderx + (sliderw >> 1) - 3, slidery + (sliderh >> 1));
        LineTo (hdc, sliderx + (sliderw >> 1) + 3, slidery + (sliderh >> 1));
        MoveTo (hdc, sliderx + (sliderw >> 1) - 2, slidery + (sliderh >> 1) - 2);
        LineTo (hdc, sliderx + (sliderw >> 1) + 2, slidery + (sliderh >> 1) - 2);
        MoveTo (hdc, sliderx + (sliderw >> 1) - 2, slidery + (sliderh >> 1) + 2);
        LineTo (hdc, sliderx + (sliderw >> 1) + 2, slidery + (sliderh >> 1) + 2);
    }
    else {
        MoveTo (hdc, sliderx + (sliderw >> 1), slidery + (sliderh >> 1) - 3);
        LineTo (hdc, sliderx + (sliderw >> 1), slidery + (sliderh >> 1) + 3);
        MoveTo (hdc, sliderx + (sliderw >> 1) - 2, slidery + (sliderh >> 1) - 2);
        LineTo (hdc, sliderx + (sliderw >> 1) - 2, slidery + (sliderh >> 1) + 2);
        MoveTo (hdc, sliderx + (sliderw >> 1) + 2, slidery + (sliderh >> 1) - 2);
        LineTo (hdc, sliderx + (sliderw >> 1) + 2, slidery + (sliderh >> 1) + 2);
    }

    /* draw the tip of trackbar. */
    if ((dwStyle & TBS_TIP) && !(dwStyle & TBS_VERTICAL)) {
        SIZE text_ext;

        SetTextColor (hdc, PIXEL_black);
        SetBkMode (hdc, BM_TRANSPARENT);
        TextOut (hdc, x + 1, y + (h>>1) - (sliderh>>1) - GAP_TIP_SLIDER, 
                            pData->sStartTip);

        GetTextExtent (hdc, pData->sEndTip, -1, &text_ext);
        EndTipLen = text_ext.cx + 4;
        TextOut (hdc, (EndTipLen > (w>>1) - 20 ? x + (w>>1) + 20 : x + w -EndTipLen), 
                        y + (h>>1) - (sliderh>>1) - GAP_TIP_SLIDER, pData->sEndTip); 
        sprintf (sPos, "%d", pData->nPos);
        GetTextExtent (hdc, sPos, -1, &text_ext);
        TextOut (hdc, x + ((w - text_ext.cx) >> 1), 
                        y + (h>>1) -(sliderh>>1) - GAP_TIP_SLIDER, sPos);
    }
    
    /* draw the focus frame. */
#ifndef _FLAT_WINDOW_STYLE
    if (dwStyle & TBS_FOCUS) {
        SetPenColor (hdc, PIXEL_lightwhite);
        FocusRect (hdc, x, y, x + w - 1, y + h - 1);
    }
#endif
}

static void TrackBarNormalizeParams (const CONTROL* pCtrl, 
                TRACKBARDATA* pData, BOOL fNotify)
{
    if (pData->nPos >= pData->nMax) {
        if (fNotify) {
            NotifyParent ((HWND)pCtrl, pCtrl->id, TBN_CHANGE);
            NotifyParent ((HWND)pCtrl, pCtrl->id, TBN_REACHMAX);
        }
        pData->nPos = pData->nMax;
        return;
    }
    if (pData->nPos <= pData->nMin) {
        if (fNotify) {
            NotifyParent ((HWND)pCtrl, pCtrl->id, TBN_CHANGE);
            NotifyParent ((HWND)pCtrl, pCtrl->id, TBN_REACHMIN);
        }
            pData->nPos = pData->nMin;
        return;
    }
    if (pData->nPos < pData->nMax && pData->nPos > pData->nMin) {
        if (fNotify)
            NotifyParent ((HWND)pCtrl, pCtrl->id, TBN_CHANGE);
        return;
    }
}

static int TrackBarCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    PCONTROL      pCtrl;
    TRACKBARDATA* pData;
    pCtrl = Control (hwnd);
    
    switch (message)
    {
        case MSG_CREATE:
            if (!(pData = malloc (sizeof (TRACKBARDATA)))) {
                fprintf(stderr, "Create trackbar control failure!\n");
                return -1;
            }
            pData->nMax = 10;
            pData->nMin = 0;
            pData->nPos = 0;
            pData->nLineSize = 1;
            pData->nPageSize = 5;
            strcpy (pData->sStartTip, "Start");
            strcpy (pData->sEndTip, "End");
            pData->nTickFreq = 1;
            pCtrl->dwAddData2 = (DWORD)pData;
        break;
    
        case MSG_DESTROY:
            free((void *)(pCtrl->dwAddData2));
        break;

        case MSG_NCPAINT:
        return 0;
       
        case MSG_GETDLGCODE:
            return DLGC_STATIC;

        case MSG_GETTEXTLENGTH:
        case MSG_GETTEXT:
        case MSG_SETTEXT:
            return -1;

        case MSG_PAINT:
        {
            RECT rcClient;
            HDC hdc, mem_dc;
            GetClientRect (hwnd, &rcClient);

            hdc = BeginPaint (hwnd);
            mem_dc = CreateCompatibleDC (hdc);
            TrackBarOnDraw (hwnd, mem_dc, (TRACKBARDATA *)pCtrl->dwAddData2, pCtrl->dwStyle);
            BitBlt (mem_dc, 0, 0, 0, 0, hdc, 0, 0, 0);
            DeleteCompatibleDC (mem_dc);
            EndPaint (hwnd, hdc);
            return 0;
        }

        case TBM_SETRANGE:
        {
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;

            if (wParam == lParam)
                return -1;

            pData->nMin = MIN (wParam, lParam);
            pData->nMax = MAX (wParam, lParam);

            if (pData->nPos > pData->nMax)
                pData->nPos = pData->nMax;
            if (pData->nPos < pData->nMin)
                pData->nPos = pData->nMin;

            SendMessage (hwnd, TBM_SETPOS, pData->nPos, 0);
            return 0;
        }
        
        case TBM_GETMIN:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            return pData->nMin;
     
        case TBM_GETMAX:    
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            return pData->nMax;
    
        case TBM_SETMIN:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;

            if (wParam == pData->nMin || wParam >= pData->nMax)
                return -1;

            pData->nMin = wParam;
            if (pData->nPos < pData->nMin)
                pData->nPos = pData->nMin;
            SendMessage (hwnd, TBM_SETPOS, pData->nPos, 0);
            return 0;
    
        case TBM_SETMAX:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;

            if (wParam == pData->nMax || wParam <= pData->nMin)
                return -1;

            pData->nMax = wParam;
            if (pData->nPos > pData->nMax)
                pData->nPos = pData->nMax;
            SendMessage (hwnd, TBM_SETPOS, pData->nPos, 0);
            return 0;
        
        case TBM_SETLINESIZE:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            if (wParam > (pData->nMax - pData->nMin))
                return -1;
            pData->nLineSize = wParam;
            return 0;

        case TBM_GETLINESIZE:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            return pData->nLineSize;
        
        case TBM_SETPAGESIZE:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            if (wParam > (pData->nMax - pData->nMin))
                return -1;
            pData->nPageSize = wParam;
            return 0;
        
        case TBM_GETPAGESIZE:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            return pData->nPageSize;
    
        case TBM_SETPOS:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            pData->nPos = wParam;
            TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
            InvalidateRect (hwnd, NULL, FALSE);
            return 0;
        
        case TBM_GETPOS:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            return pData->nPos;
        
        case TBM_SETTICKFREQ:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            if (wParam > (pData->nMax - pData->nMin))
                return -1;
            pData->nTickFreq = wParam;
            InvalidateRect (hwnd, NULL, FALSE);
            return 0;

        case TBM_GETTICKFREQ:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            return pData->nTickFreq;
    
        case TBM_SETTIP:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            if (wParam) 
                strncpy(pData->sStartTip, (char *) wParam, TBLEN_TIP);
            if (lParam)
                strncpy (pData->sEndTip, (char *) lParam, TBLEN_TIP);
            InvalidateRect (hwnd, NULL, FALSE);
            return 0;

        case TBM_GETTIP:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            if (wParam)
                strcpy ((char *) wParam, pData->sStartTip);
            if (lParam)
                strcpy ((char *) lParam, pData->sEndTip);
            return 0;
        
        case MSG_SETFOCUS:
            if (pCtrl->dwStyle & TBS_FOCUS)
                break;
            pCtrl->dwStyle |= TBS_FOCUS;
            InvalidateRect (hwnd, NULL, FALSE);
            break;
    
        case MSG_KILLFOCUS:
            pCtrl->dwStyle &= ~TBS_FOCUS;
            InvalidateRect (hwnd, NULL, FALSE);
            break;
    
        case MSG_KEYDOWN:
            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            switch (LOWORD (wParam)) {
                case SCANCODE_CURSORBLOCKUP:
                case SCANCODE_CURSORBLOCKRIGHT:
                    pData->nPos += pData->nLineSize;
                    TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
                    InvalidateRect (hwnd, NULL, FALSE);
                break;

                case SCANCODE_CURSORBLOCKDOWN:
                case SCANCODE_CURSORBLOCKLEFT:
                    pData->nPos -= pData->nLineSize;
                    TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
                    InvalidateRect (hwnd, NULL, FALSE);
                break;
            
                case SCANCODE_PAGEDOWN:
                    pData->nPos -= pData->nPageSize;
                    TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
                    InvalidateRect (hwnd, NULL, FALSE);
                break;
            
                case SCANCODE_PAGEUP:
                    pData->nPos += pData->nPageSize;
                    TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
                    InvalidateRect (hwnd, NULL, FALSE);
                break;
            
                case SCANCODE_HOME:
                    pData->nPos = pData->nMin;
                    TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
                    InvalidateRect (hwnd, NULL, FALSE);
                break;
            
                case SCANCODE_END:
                    pData->nPos = pData->nMax;
                    TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
                    InvalidateRect (hwnd, NULL, FALSE);
                break;
            }
        break;

        case MSG_LBUTTONDOWN:
        {
            RECT    rcClient;
            int     x, y, w, h;
            int     len, pos, max, min;    
            int     mouseX, mouseY;    

            GetClientRect (hwnd, &rcClient);
            x = rcClient.left;
            y = rcClient.top;
            w = RECTW (rcClient);
            h = RECTH (rcClient);
            mouseX = LOSWORD(lParam);
            mouseY = HISWORD(lParam);
        
            if (GetCapture() != hwnd) {
                SetCapture (hwnd);
                pCtrl->dwStyle |= TBS_DRAGGED;
            }
            else
                break;
        
            if (pCtrl->dwStyle & TBS_VERTICAL) {
                len = RECTH (rcClient) - (WIDTH_HORZ_BLANK<<1);
                if (mouseY > rcClient.bottom - WIDTH_HORZ_BLANK)
                    pos = 0;
                else if (mouseX < rcClient.top + WIDTH_HORZ_BLANK)
                    pos = len;
                else
                    pos = y + h - WIDTH_HORZ_BLANK - mouseY;
            } else {
                len = RECTW (rcClient) - (WIDTH_HORZ_BLANK<<1);
                if (mouseX > rcClient.right - WIDTH_HORZ_BLANK)
                    pos = len ;
                else if (mouseX < rcClient.left + WIDTH_HORZ_BLANK)
                    pos = 0;
                else
                    pos = mouseX - x - WIDTH_HORZ_BLANK;
            }

            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            max = pData->nMax;
            min = pData->nMin;
            pData->nPos = (int)((max - min) * pos / (float)len + 0.5) + min;

            TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
            if (pCtrl->dwStyle & TBS_VERTICAL) 
                pData->mousepos = mouseY;
            else
                pData->mousepos = mouseX;
            InvalidateRect (hwnd, NULL, FALSE);
        }
        break;
                
        case MSG_MOUSEMOVE:
        {
            int mouseX = LOSWORD(lParam);
            int mouseY = HISWORD(lParam);

            if (wParam & KS_CAPTURED)
                ScreenToClient (hwnd, &mouseX, &mouseY);
            else
                break;

            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            if (pCtrl->dwStyle & TBS_VERTICAL) 
                pData->mousepos = mouseY;
            else
                pData->mousepos = mouseX;

            InvalidateRect (hwnd, NULL, FALSE);
        }
        break;

        case MSG_LBUTTONUP:
            if (GetCapture() == hwnd) {
                ReleaseCapture ();
                pCtrl->dwStyle &= ~TBS_DRAGGED;
            }
            else
                break;

            pData = (TRACKBARDATA *)pCtrl->dwAddData2;
            TrackBarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & TBS_NOTIFY);
            InvalidateRect (hwnd, NULL, FALSE);
        break;
    
        case MSG_FONTCHANGED:
            InvalidateRect (hwnd, NULL, FALSE);
            return 0;

        default:
            break;    
    }
    
    return DefaultControlProc (hwnd, message, wParam, lParam);
}

#endif /* _CTRL_TRACKBAR */

