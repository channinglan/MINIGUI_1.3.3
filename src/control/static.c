/*
** $Id: static.c,v 1.31 2003/09/04 02:40:36 weiym Exp $
**
** static.c: the Static Control module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maitainer: Wei Yongming.
**
** Create date: 1999/5/22
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
#include "control.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"

#ifdef _CTRL_STATIC

#include "ctrlmisc.h"
#include "static.h"

static int StaticControlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam);

BOOL RegisterStaticControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = CTRL_STATIC;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (IDC_ARROW);
    WndClass.iBkColor    = GetWindowElementColor (BKC_CONTROL_DEF);
    WndClass.WinProc     = StaticControlProc;

    return AddNewControlClass (&WndClass) == ERR_OK;
}

#if 0
void StaticControlCleanup (void)
{
    // do nothing.
    return;
}
#endif

static int StaticControlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    RECT        rcClient;
    HDC         hdc;
    const char* spCaption;
    PCONTROL    pCtrl;
    UINT        uFormat;
    DWORD       dwStyle;         
    
    pCtrl = Control (hwnd);                        
    switch (message) {
        case MSG_CREATE:
            pCtrl->dwAddData2 = pCtrl->dwAddData;
            return 0;
            
        case STM_GETIMAGE:
            return (int)(pCtrl->dwAddData2); 
        
        case STM_SETIMAGE:
        {
            int pOldValue;
            
            pOldValue  = (int)(pCtrl->dwAddData2);
            pCtrl->dwAddData2 = (DWORD)wParam;
            InvalidateRect (hwnd, NULL, (GetWindowStyle (hwnd) & SS_TYPEMASK) == SS_ICON);
            return pOldValue;
        }
           
        case MSG_GETDLGCODE:
            return DLGC_STATIC;

        case MSG_PAINT:
            hdc = BeginPaint (hwnd);
            GetClientRect (hwnd, &rcClient);
            dwStyle = GetWindowStyle (hwnd);
            switch (dwStyle & SS_TYPEMASK)
            {
                case SS_GRAYRECT:
                    SetBrushColor (hdc, PIXEL_lightgray);
                    FillBox(hdc, 0, 0, RECTW(rcClient), RECTH(rcClient)); 
                break;
                
                case SS_BLACKRECT:
                    SetBrushColor (hdc, PIXEL_black);
                    FillBox(hdc, 0, 0, RECTW(rcClient), RECTH(rcClient)); 
                break;
                
                case SS_WHITERECT:
                    SetBrushColor (hdc, PIXEL_lightwhite);
                    FillBox(hdc, 0, 0, RECTW(rcClient), RECTH(rcClient)); 
                break;
                
                case SS_BLACKFRAME:
                    SetPenColor (hdc, PIXEL_black);
                    Rectangle (hdc, 0, 0, rcClient.right - 1, rcClient.bottom - 1); 
                break;
                
                case SS_GRAYFRAME:
                    SetPenColor (hdc, PIXEL_lightgray);
                    Rectangle (hdc, 0, 0, rcClient.right - 1, rcClient.bottom - 1); 
                break;
                
                case SS_WHITEFRAME:
                    SetPenColor (hdc, PIXEL_lightwhite);
                    Rectangle (hdc, 0, 0, rcClient.right - 1, rcClient.bottom - 1); 
                break;
                
                case SS_BITMAP:
                    if (pCtrl->dwAddData2) {
                        int x = 0, y = 0, w, h;
                        PBITMAP bmp = (PBITMAP)(pCtrl->dwAddData2);

                        if (dwStyle & SS_REALSIZEIMAGE) {
                            w = bmp->bmWidth;
                            h = bmp->bmHeight;
                            if (dwStyle & SS_CENTERIMAGE) {
                                x = (rcClient.right - w) >> 1;
                                y = (rcClient.bottom - h) >> 1;
                            }
                        }
                        else {
                            x = y = 0;
                            w = RECTW (rcClient);
                            h = RECTH (rcClient);
                        }

                        FillBoxWithBitmap(hdc, x, y, w, h, bmp);
                    }
                break;
                
                case SS_ICON:
                    if (pCtrl->dwAddData2) {
                        int x = 0, y = 0, w, h;
                        HICON hIcon = (HICON)(pCtrl->dwAddData2);

                        if (dwStyle & SS_REALSIZEIMAGE) {
                            GetIconSize (hIcon, &w, &h);
                            if (dwStyle & SS_CENTERIMAGE) {
                                x = (rcClient.right - w) >> 1;
                                y = (rcClient.bottom - h) >> 1;
                            }
                        }
                        else {
                            x = y = 0;
                            w = RECTW (rcClient);
                            h = RECTH (rcClient);
                        }

                        DrawIcon (hdc, x, y, w, h, hIcon);
                    }
                break;
      
                case SS_SIMPLE:
                    SetBrushColor (hdc, GetWindowBkColor (hwnd));
                    FillBox (hdc, 0, 0, rcClient.right, rcClient.bottom);
        
                    if (dwStyle & WS_DISABLED)
                        SetTextColor (hdc, PIXEL_darkgray);
                    else
                        SetTextColor (hdc, PIXEL_black);

                    SetBkColor (hdc, GetWindowBkColor (hwnd));
                    spCaption = GetWindowCaption (hwnd);
                    if (spCaption)
                        TextOut (hdc, 0, 0, spCaption); 
                break; 

                case SS_OWNERDRAW: /* Hack for a special style */
                    SetBrushColor (hdc, GetWindowElementColor (BKC_STATIC));
                    FillBox (hdc, 0, 0, rcClient.right, rcClient.bottom);
        
                    if (dwStyle & WS_DISABLED)
                        SetTextColor (hdc, PIXEL_darkgray);
                    else
                        SetTextColor (hdc, PIXEL_black);

                    SetBkColor (hdc, GetWindowElementColor (BKC_STATIC));
                    spCaption = GetWindowCaption (hwnd);
                    if (spCaption)
                        TextOut (hdc, 0, 0, spCaption); 
                break; 

                case SS_LEFT:
                case SS_CENTER:
                case SS_RIGHT:
                case SS_LEFTNOWORDWRAP:
                    uFormat = DT_TOP;
                    if ( (dwStyle & SS_TYPEMASK) == SS_LEFT)
                        uFormat |= DT_LEFT | DT_WORDBREAK;
                    else if ( (dwStyle & SS_TYPEMASK) == SS_CENTER)
                        uFormat |= DT_CENTER | DT_WORDBREAK;
                    else if ( (dwStyle & SS_TYPEMASK) == SS_RIGHT)
                        uFormat |= DT_RIGHT | DT_WORDBREAK;
                    else if ( (dwStyle & SS_TYPEMASK) == SS_LEFTNOWORDWRAP)
                        uFormat |= DT_LEFT | DT_SINGLELINE | DT_EXPANDTABS;
                    
                    if (dwStyle & WS_DISABLED)
                        SetTextColor (hdc, PIXEL_darkgray);
                    else
                        SetTextColor (hdc, PIXEL_black);

                    SetBkColor (hdc, GetWindowBkColor (hwnd));
                    spCaption = GetWindowCaption (hwnd);
                    if (dwStyle & SS_NOPREFIX)
                        uFormat |= DT_NOPREFIX;
                        
                    if (spCaption)
                        DrawText (hdc, spCaption, -1, &rcClient, uFormat);
                break;

                case SS_GROUPBOX:
#ifdef _FLAT_WINDOW_STYLE
                    DrawFlatControlFrameEx (hdc,  rcClient.left, 
                                    rcClient.top + (pCtrl->pLogFont->size >> 1),
                                    rcClient.right - 1,
                                    rcClient.bottom - 1, PIXEL_invalid, 2, PIXEL_invalid);
#else
                    Draw3DBorder (hdc,  rcClient.left, 
                                    rcClient.top + (pCtrl->pLogFont->size >> 1),
                                    rcClient.right,
                                    rcClient.bottom);
#endif
                    
                    if (dwStyle & WS_DISABLED)
                        SetTextColor (hdc, PIXEL_darkgray);
                    else
                        SetTextColor (hdc, PIXEL_black);

                    SetBkColor(hdc, GetWindowBkColor (hwnd));
                    spCaption = GetWindowCaption (hwnd);
                    if (spCaption)
                        TextOut (hdc, pCtrl->pLogFont->size, 2, spCaption);
                break;
            }
            EndPaint (hwnd, hdc);
            return 0;

        case MSG_LBUTTONDBLCLK:
            if (GetWindowStyle (hwnd) & SS_NOTIFY)
                NotifyParent (hwnd, pCtrl->id, STN_DBLCLK);
            break;

        case MSG_LBUTTONDOWN:
            if (GetWindowStyle (hwnd) & SS_NOTIFY)
                NotifyParent (hwnd, pCtrl->id, STN_CLICKED);
            break;

        case MSG_NCLBUTTONDBLCLK:
            break;

        case MSG_NCLBUTTONDOWN:
            break;

        case MSG_HITTEST:
            dwStyle = GetWindowStyle (hwnd);
            if ((dwStyle & SS_TYPEMASK) == SS_GROUPBOX)
                return HT_TRANSPARENT;

            if (GetWindowStyle (hwnd) & SS_NOTIFY)
                return HT_CLIENT;
            else
                return HT_OUT;
        break;

        case MSG_FONTCHANGED:
            InvalidateRect (hwnd, NULL, TRUE);
            return 0;
            
        case MSG_SETTEXT:
            SetWindowCaption (hwnd, (char*)lParam);
            InvalidateRect (hwnd, NULL, TRUE);
            break;
            
        default:
            break;
    }

    return DefaultControlProc (hwnd, message, wParam, lParam);
}

#endif /* _CTRL_STATIC */

