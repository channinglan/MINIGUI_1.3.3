/*
** $Id: coolbar.c,v 1.28 2003/09/04 06:12:03 weiym Exp $
**
** coolbar.c: the Coolbar Control module.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming and others.
** 
** Original author: Wang Jian.
**
** Create date: 2001-08-20
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

#ifdef __MINIGUI_LIB__
    #include "common.h"
    #include "minigui.h"
    #include "gdi.h"
    #include "window.h"
    #include "control.h"
    #include "mywindows.h"
#else
    #include <minigui/common.h>
    #include <minigui/minigui.h>
    #include <minigui/gdi.h>
    #include <minigui/window.h>
    #include <minigui/control.h>
    #include <minigui/mywindows.h>
#endif

#ifdef _EXT_CTRL_COOLBAR

#include "mgext.h"
#include "coolbar.h"

#define ITEMBARWIDTH    8

static int CoolBarCtrlProc (HWND hWnd, int uMsg, WPARAM wParam, LPARAM lParam);

BOOL RegisterCoolBarControl (void)
{
    WNDCLASS WndClass;
        
    WndClass.spClassName = CTRL_COOLBAR;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (IDC_ARROW);
    WndClass.iBkColor    = GetWindowElementColor (BKC_CONTROL_DEF);
    WndClass.WinProc     = CoolBarCtrlProc;

    return RegisterWindowClass (&WndClass);
}

void CoolBarControlCleanup (void)
{
    UnregisterWindowClass (CTRL_COOLBAR);
}

static COOLBARITEMDATA* GetCurTag (int posx, int posy, PCOOLBARCTRL pdata)
{
    COOLBARITEMDATA*  tmpdata;

    tmpdata = pdata->head;
    while (tmpdata) { 
        if (PtInRect (&tmpdata->RcTitle, posx, posy)
                && tmpdata->ItemType != TYPE_BARITEM) {
            return tmpdata;    
        }
        tmpdata = tmpdata->next;
    }
    return NULL;         
}

static COOLBARITEMDATA* GetCurSel (PCOOLBARCTRL pdata)
{
    COOLBARITEMDATA*  tmpdata;

    tmpdata = pdata->head;
    while (tmpdata) { 
        if (pdata->iSel == tmpdata->insPos
                || pdata->iMvOver == tmpdata->insPos)
            return tmpdata;    

        tmpdata = tmpdata->next;
    }

    return NULL;         
}

static int enable_item (PCOOLBARCTRL TbarData, int id, BOOL flag)
{
    COOLBARITEMDATA*  tmpdata;
   
    tmpdata = TbarData->head;
    while (tmpdata) { 
        if (tmpdata->id == id) {
            tmpdata->Disable = !flag;    
            return 0;
        }
        tmpdata = tmpdata->next;
    }

    return -1;
}

static void ShowCurItemHint (HWND hWnd, COOLBARITEMDATA* tmpdata)
{
    int x = tmpdata->hintx, y = tmpdata->hinty;
    PCOOLBARCTRL TbarData = (PCOOLBARCTRL) GetWindowAdditionalData2(hWnd);

    if (tmpdata->Hint [0] == '\0')
        return;

    ClientToScreen (hWnd, &x, &y);
    if (g_rcScr.bottom - y < y - g_rcScr.top) {
        y -= 40;
    }

    if (TbarData->hToolTip == HWND_DESKTOP) {
        TbarData->hToolTip = createToolTipWin (hWnd, x, y, 1000, tmpdata->Hint);
    }
    else {
        resetToolTipWin (TbarData->hToolTip, x, y, tmpdata->Hint);
    }
}

static void unhilight (HWND hwnd)
{
    COOLBARITEMDATA* pItemdata;
    PCOOLBARCTRL TbarData;
    
    TbarData = (PCOOLBARCTRL) GetWindowAdditionalData2(hwnd);

    if ((pItemdata = GetCurSel(TbarData))) {
        TbarData->iSel = -1;
        TbarData->iMvOver = -1;
        InvalidateRect (hwnd, &pItemdata->RcTitle, TRUE);
    }
}

static void draw_hilight_box (HDC hdc, COOLBARITEMDATA* item)
{
    int  l,r,t,b; 

    l = item->RcTitle.left;
    t = item->RcTitle.top;
    r = item->RcTitle.right - 1;
    b = item->RcTitle.bottom - 1;

#ifdef  _FLAT_WINDOW_STYLE
    SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_RIGHT));
    MoveTo (hdc, l, t);
    LineTo (hdc, l, b);
    MoveTo (hdc, r, t);
    LineTo (hdc, r, b);
#else
    SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_RIGHT));
    MoveTo (hdc, l, t);
    LineTo (hdc, l, b);
    MoveTo (hdc, r - 1, t);
    LineTo (hdc, r - 1, b);

    SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT));
    MoveTo (hdc, l + 1, t);
    LineTo (hdc, l + 1, b);
    MoveTo (hdc, r, t);
    LineTo (hdc, r, b);
#endif
}

static void hilight (HWND hWnd, COOLBARITEMDATA* item)
{
    HDC hdc;

    hdc = GetClientDC (hWnd);
    draw_hilight_box (hdc, item);
    ReleaseDC (hdc);

    ShowCurItemHint (hWnd, item);
}

static void DrawCoolBox (HWND hWnd, HDC hdc, PCOOLBARCTRL pdata)
{
    COOLBARITEMDATA* tmpdata;
    RECT rc;
    int l,t;
  
    GetClientRect (hWnd, &rc);

    if (pdata->BackBmp) {
        FillBoxWithBitmap (hdc, 0, 0, rc.right, rc.bottom, pdata->BackBmp);
    }

#ifdef  _FLAT_WINDOW_STYLE
    SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_RIGHT));
    MoveTo (hdc, 0, 1);
    LineTo (hdc, rc.right, 1);
    MoveTo (hdc, 0, rc.bottom - 2);
    LineTo (hdc, rc.right, rc.bottom - 2);
#else
    SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_RIGHT));
    MoveTo (hdc, 0, 0);
    LineTo (hdc, rc.right, 0);
    MoveTo (hdc, 0, rc.bottom - 2);
    LineTo (hdc, rc.right, rc.bottom - 2);
    SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_LEFT));
    MoveTo (hdc, 0, 1);
    LineTo (hdc, rc.right, 1);
    MoveTo (hdc, 0, rc.bottom - 1);
    LineTo (hdc, rc.right, rc.bottom - 1);
#endif

    tmpdata = pdata->head;
    while (tmpdata) {
        l = tmpdata->RcTitle.left;
        t = tmpdata->RcTitle.top;
      
        switch (tmpdata->ItemType) {
        case TYPE_BARITEM:
#ifdef  _FLAT_WINDOW_STYLE
            SetPenColor (hdc, GetWindowElementColor (WEC_3DFRAME_RIGHT));
            MoveTo (hdc, l + 3, 4);
            LineTo (hdc, l + 3, rc.bottom - 4);
#else
            Draw3DDownThinFrame (hdc, l + 2, 4, l + 4, rc.bottom - 4, PIXEL_invalid);
#endif
            break;

        case TYPE_BMPITEM:
            FillBoxWithBitmap (hdc, l + 2, t + 2, 
                            pdata->ItemWidth, pdata->ItemHeight, tmpdata->Bmp);
            break;

        case TYPE_TEXTITEM:
        {
            SIZE size;
            int h;

            if (tmpdata->Caption == NULL || tmpdata->Caption [0] == '\0')
                break;

            GetTextExtent (hdc, tmpdata->Caption, -1, &size);
            h = (pdata->ItemHeight - size.cy) / 2;

            SetBkMode (hdc, BM_TRANSPARENT);
            if (tmpdata->Disable)
                DisabledTextOut (hdc, l+2, t + h + 2, tmpdata->Caption);
            else
               TextOut (hdc, l+2, t + h + 2, tmpdata->Caption);

            break;
        }

        default:
            break;
        }

        tmpdata = tmpdata->next;
    }

    if ((tmpdata = GetCurSel (pdata)) == NULL)
        return;

    draw_hilight_box (hdc, tmpdata);
}

static void set_item_rect (HWND hwnd, PCOOLBARCTRL TbarData, COOLBARITEMDATA* ptemp)
{
    SIZE size;
    int w;

    if (TbarData->tail == NULL) 
        ptemp->RcTitle.left = 2;
    else
        ptemp->RcTitle.left = TbarData->tail->RcTitle.right;

    switch (ptemp->ItemType) {
    case TYPE_BARITEM:
        w = ITEMBARWIDTH;
        break;

    case TYPE_BMPITEM:
        w = TbarData->ItemWidth + 4;
        break;

    case TYPE_TEXTITEM:
        if (strlen (ptemp->Caption)) {
            HDC hdc;
            hdc = GetClientDC (hwnd);
            GetTextExtent (hdc, ptemp->Caption, -1, &size);
            ReleaseDC (hdc);
        }
        w = size.cx + 4;
        break;

    default:
        w = 0;
        break;
    }

    ptemp->RcTitle.right = ptemp->RcTitle.left + w;
    ptemp->RcTitle.top = 2;
    ptemp->RcTitle.bottom = ptemp->RcTitle.top + TbarData->ItemHeight + 4;
  
    ptemp->hintx = ptemp->RcTitle.left;
    ptemp->hinty = ptemp->RcTitle.bottom;
}

static int CoolBarCtrlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC              hdc;
    PCOOLBARCTRL     TbarData;
    PCOOLBARITEMDATA pTbid;
        
    switch (message) {
        case MSG_CREATE:
        {
            DWORD data; 
            DWORD dwStyle;
            const char* caption;

            if ((TbarData = (COOLBARCTRL*) calloc (1, sizeof (COOLBARCTRL))) == NULL)
                return 1;

            TbarData->nCount = 0;
            TbarData->head = TbarData->tail = NULL;
            TbarData->BackBmp = NULL;
            TbarData->iSel = -1;
            TbarData->iMvOver = -1;
            TbarData->ShowHint = TRUE;
            TbarData->hToolTip = HWND_DESKTOP;

            ExcludeWindowStyle (hWnd, WS_BORDER);

            dwStyle = GetWindowStyle (hWnd);
            if (dwStyle & CBS_BMP_32X32) {
                TbarData->ItemWidth = 32;
                TbarData->ItemHeight = 32;
            }
            else if (dwStyle & CBS_BMP_CUSTOM) {
                data = GetWindowAdditionalData (hWnd);
                TbarData->ItemWidth = LOWORD (data);
                TbarData->ItemHeight = HIWORD (data);
            }
            else {
                TbarData->ItemWidth = 16;
                TbarData->ItemHeight = 16;
            }

            caption = GetWindowCaption (hWnd);
            if ((dwStyle & CBS_USEBKBMP) && caption [0]) {
                TbarData->BackBmp = (BITMAP*)calloc (1, sizeof (BITMAP));
                if (LoadBitmap (HDC_SCREEN, TbarData->BackBmp, caption) < 0) {
                    free (TbarData->BackBmp);
                    TbarData->BackBmp = NULL;
                    break;
                }
            }
            SetWindowAdditionalData2 (hWnd, (DWORD)TbarData);
        }
        break;

        case MSG_DESTROY:
        { 
            COOLBARITEMDATA* unloaddata, *tmp;
            TbarData = (PCOOLBARCTRL) GetWindowAdditionalData2(hWnd);

            if (TbarData->hToolTip != HWND_DESKTOP) {
                destroyToolTipWin (TbarData->hToolTip);
            }

            if (TbarData->BackBmp) {
                UnloadBitmap (TbarData->BackBmp);
                free (TbarData->BackBmp);
            }

            unloaddata = TbarData->head;
            while (unloaddata) {
                tmp = unloaddata->next;
                free (unloaddata);
                unloaddata = tmp;
            }

            free (TbarData);
        }
        break;

        case MSG_SIZECHANGING:
        {
            const RECT* rcExpect = (const RECT*)wParam;
            RECT* rcResult = (RECT*)lParam;

            TbarData = (PCOOLBARCTRL) GetWindowAdditionalData2(hWnd);

            rcResult->left = rcExpect->left;
            rcResult->top = rcExpect->top;
            rcResult->right = rcExpect->right;
            rcResult->bottom = rcExpect->top + TbarData->ItemHeight + 8;
            return 0;
        }

		case MSG_SIZECHANGED:
		{
			RECT* rcWin = (RECT*)wParam;
			RECT* rcClient = (RECT*)lParam;
			*rcClient = *rcWin;
			return 1;
		}

        case MSG_NCPAINT:
            return 0;

        case MSG_PAINT:
        {
            TbarData = (PCOOLBARCTRL) GetWindowAdditionalData2(hWnd);
            hdc = BeginPaint (hWnd);
            DrawCoolBox (hWnd, hdc, TbarData);
            EndPaint (hWnd, hdc);
            return 0;
        }

        case CBM_ADDITEM:
        {
            COOLBARITEMINFO* TbarInfo = NULL;
            COOLBARITEMDATA* ptemp;
            RECT rc;

            TbarData = (PCOOLBARCTRL) GetWindowAdditionalData2 (hWnd);
            TbarInfo = (COOLBARITEMINFO*) lParam;

            if (!(ptemp = (COOLBARITEMDATA*)calloc (1, sizeof (COOLBARITEMDATA)))) {
                return -1;
            }

            GetClientRect (hWnd, &rc);

            ptemp->id = TbarInfo->id;
            ptemp->Disable = 0;
            ptemp->ItemType = TbarInfo->ItemType;

            if (TbarInfo->ItemHint) {
                strncpy (ptemp->Hint, TbarInfo->ItemHint, LEN_HINT);
                ptemp->Hint [LEN_HINT] = '\0';
            }
            else
                ptemp->Hint [0] = '\0';

            if (TbarInfo->Caption) {
                strncpy (ptemp->Caption, TbarInfo->Caption, LEN_TITLE);
                ptemp->Caption [LEN_TITLE] = '\0';
            }
            else
                ptemp->Caption [0] = '\0';
             
            ptemp->Bmp = TbarInfo->Bmp;

            set_item_rect (hWnd, TbarData, ptemp); 

            ptemp->next = NULL;
            if (TbarData->nCount == 0) {
                TbarData->head = TbarData->tail = ptemp;
            }
            else if (TbarData->nCount > 0) { 
                TbarData->tail->next = ptemp;
                TbarData->tail = ptemp;
            }
            ptemp->insPos = TbarData->nCount;
            TbarData->nCount++;

            InvalidateRect (hWnd, NULL, TRUE);
            return 0;
        }

        case CBM_ENABLE:
            TbarData = (PCOOLBARCTRL)GetWindowAdditionalData2 (hWnd);
            if (enable_item (TbarData, wParam, lParam))
                return -1;

            InvalidateRect (hWnd, NULL, TRUE);
            return 0;

        case MSG_LBUTTONDOWN:
        {         
             int posx, posy;
             TbarData=(PCOOLBARCTRL) GetWindowAdditionalData2(hWnd);

             posx = LOSWORD (lParam);
             posy = HISWORD (lParam);

             if (GetCapture () == hWnd)
                break;
             
             SetCapture (hWnd);
                         
             if ((pTbid = GetCurTag (posx, posy, TbarData)) == NULL)
                break; 
             
             TbarData->iSel = pTbid->insPos;
             break;
        }

        case MSG_LBUTTONUP:
        { 
            int x, y;
            TbarData = (PCOOLBARCTRL)GetWindowAdditionalData2(hWnd);
            x = LOSWORD(lParam);
            y = HISWORD(lParam);
                      
            if (GetCapture() != hWnd)
               break;
            ReleaseCapture ();

            ScreenToClient (hWnd, &x, &y);

            if ((pTbid = GetCurTag(x, y, TbarData)) == NULL) {
                break;
            }
          
            if (TbarData->iSel == pTbid->insPos)
                NotifyParent (hWnd, GetDlgCtrlID(hWnd), pTbid->id);

            TbarData->iSel = -1;
            break;
        }

        case MSG_MOUSEMOVE:
        {  
            int x, y;
            TbarData = (PCOOLBARCTRL) GetWindowAdditionalData2(hWnd);
            x = LOSWORD(lParam);
            y = HISWORD(lParam);

            if (GetCapture() == hWnd)
                ScreenToClient (hWnd, &x, &y);
                
            if ((pTbid = GetCurTag (x, y, TbarData)) == NULL) {
                unhilight (hWnd);
                break;
            }

            if (TbarData->iMvOver == pTbid->insPos)
               break;

            unhilight (hWnd);
            TbarData->iMvOver = pTbid->insPos;
            hilight (hWnd, pTbid);
            break;
        }
        
        case MSG_MOUSEMOVEIN:
            if (!wParam)
                unhilight (hWnd);   
            break;

        case MSG_NCLBUTTONDOWN:
        case MSG_KILLFOCUS:
            unhilight (hWnd);   
            break;
    }

    return DefaultControlProc (hWnd, message, wParam, lParam);
}

#endif /* _EXT_CTRL_COOLBAR */

