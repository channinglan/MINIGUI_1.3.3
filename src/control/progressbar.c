/*
** $Id: progressbar.c,v 1.27 2003/09/04 02:40:36 weiym Exp $
**
** progressbar.c: the Progress Bar Control module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yonming.
**
** Note:
**   Originally by Zhao Jianghua. 
**
** Create date: 1999/8/29
**
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

#ifdef _CTRL_PROGRESSBAR

#include "ctrlmisc.h"
#include "progressbar.h"

#define  WIDTH_PBAR_BORDER  2 

static int ProgressBarCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam);

BOOL RegisterProgressBarControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = CTRL_PROGRESSBAR;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (0);
    WndClass.iBkColor    = GetWindowElementColor (BKC_CONTROL_DEF);
    WndClass.WinProc     = ProgressBarCtrlProc;

    return AddNewControlClass (&WndClass) == ERR_OK;
}

#if 0
void ProgressBarControlCleanup (void)
{
    // do nothing
    return;
}
#endif

static void pbarOnDraw (HWND hwnd, HDC hdc, PROGRESSDATA* pData, BOOL fVertical)
{
    RECT    rcClient;
    int     x, y, w, h;
    ldiv_t   ndiv_progress;
    unsigned int     nAllPart;
    unsigned int     nNowPart;
    int     whOne, nRem;
    int     ix, iy;
    int     i;
#ifndef _GRAY_SCREEN
    char    szText[8] = "\0";
#endif
    int     step;
    
    if (pData->nMax == pData->nMin)
        return;
    
    if ((pData->nMax - pData->nMin) > 5)
        step = 5;
    else
        step = 1;

    GetClientRect (hwnd, &rcClient);

    x = rcClient.left + WIDTH_PBAR_BORDER;
    y = rcClient.top + WIDTH_PBAR_BORDER;
    w = RECTW (rcClient) - (WIDTH_PBAR_BORDER << 1);
    h = RECTH (rcClient) - (WIDTH_PBAR_BORDER << 1);

    ndiv_progress = ldiv (pData->nMax - pData->nMin, step);
    nAllPart = ndiv_progress.quot;
    
    ndiv_progress = ldiv (pData->nPos - pData->nMin, step);
    nNowPart = ndiv_progress.quot;
    if (fVertical)
        ndiv_progress = ldiv (h, nAllPart);
    else
        ndiv_progress = ldiv (w, nAllPart);
        
    whOne = ndiv_progress.quot;
    nRem = ndiv_progress.rem;

    SetBrushColor (hdc, GetWindowElementColor (BKC_CONTROL_DEF));
    FillBox (hdc, x, y, w, h);

#ifdef _GRAY_SCREEN
    SetBrushColor (hdc, PIXEL_black);
#else
    SetBrushColor (hdc, PIXEL_blue);
#endif
 
    if(whOne >= 4) {
        if (fVertical) {
            for (i = 0, iy = y + h - 1; i < nNowPart; ++i) {
                if ((iy - whOne) < y) 
                    whOne = iy - y;

                FillBox (hdc, x + 1, iy - whOne, w - 2, whOne - 1);
                iy -= whOne;
                if(nRem > 0) {
                    iy --;
                    nRem --;
                }
            }
        }
        else {
            for (i = 0, ix = x + 1; i < nNowPart; ++i) {
                if ((ix + whOne) > (x + w)) 
                    whOne = x + w - ix;

                FillBox (hdc, ix, y + 1, whOne - 1, h - 2);
                ix += whOne;
                if(nRem > 0) {
                    ix ++;
                    nRem --;
                }
            }
        }
    }
    else {
        // no vertical support
        double d = (nNowPart*1.0)/nAllPart;
        
#ifdef _GRAY_SCREEN
        SetBrushColor (hdc, PIXEL_black);
#else
        SetBrushColor (hdc, PIXEL_blue);
#endif
        FillBox (hdc, x, y + 1, (int)(w*d), h - 2);

#ifndef _GRAY_SCREEN
        {
            SIZE text_ext;

            SetTextColor (hdc, GetWindowElementColor (FGC_CONTROL_DEF));
            SetBkColor (hdc, PIXEL_blue);
            SetBkMode (hdc, BM_TRANSPARENT);
            sprintf (szText, "%d%%", (int)(d*100));
            GetTextExtent (hdc, szText, -1, &text_ext);
            TextOut (hdc, x + ((w - text_ext.cx) >> 1), 
                      y + ((h - text_ext.cy) >> 1), szText);
        }
#endif
    }
}

static void pbarNormalizeParams (const CONTROL* pCtrl, 
                PROGRESSDATA* pData, BOOL fNotify)
{
    if (pData->nPos > pData->nMax) {
        if (fNotify)
            NotifyParent ((HWND)pCtrl, pCtrl->id, PBN_REACHMAX);
        pData->nPos = pData->nMax;
    }

    if (pData->nPos < pData->nMin) {
        if (fNotify)
            NotifyParent ((HWND)pCtrl, pCtrl->id, PBN_REACHMIN);
        pData->nPos = pData->nMin;
    }
}

static int ProgressBarCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC           hdc;
    PCONTROL      pCtrl;
    PROGRESSDATA* pData;
    
    pCtrl = Control (hwnd); 
    
    switch (message)
    {
        case MSG_CREATE:
            if (!(pData = malloc (sizeof (PROGRESSDATA)))) {
                fprintf(stderr, "Create progress bar control failure!\n");
                return -1;
            }
            
            pData->nMax     = 100;
            pData->nMin     = 0;
            pData->nPos     = 0;
            pData->nStepInc = 10;
            
            pCtrl->dwAddData2 = (DWORD)pData;
        break;
    
        case MSG_DESTROY:
            free ((void *)(pCtrl->dwAddData2));
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
            HDC mem_dc;
            
            GetClientRect (hwnd, &rcClient);

            hdc = BeginPaint (hwnd);
            mem_dc = CreateCompatibleDC (hdc);

#ifdef _FLAT_WINDOW_STYLE
            SetPenColor (mem_dc, GetWindowElementColor (BKC_CONTROL_DEF));
            Rectangle (mem_dc, rcClient.left, rcClient.top, 
                             rcClient.right - 1, rcClient.bottom - 1);

            SetPenColor (mem_dc, PIXEL_black);
            Rectangle (mem_dc, rcClient.left + 1, rcClient.top + 1, 
                             rcClient.right - 2, rcClient.bottom - 2);
#else
            Draw3DDownFrame (mem_dc, rcClient.left, rcClient.top, 
                             rcClient.right - 1, rcClient.bottom - 1, 
                             PIXEL_invalid);
#endif
            pbarOnDraw (hwnd, mem_dc, (PROGRESSDATA *)pCtrl->dwAddData2, 
                            pCtrl->dwStyle & PBS_VERTICAL);
            BitBlt (mem_dc, 0, 0, rcClient.right, rcClient.bottom, hdc, 0, 0, 0);
            DeleteCompatibleDC (mem_dc);

            EndPaint (hwnd, hdc);
            return 0;
        }

        case PBM_SETRANGE:
            pData = (PROGRESSDATA *)pCtrl->dwAddData2;
            if (wParam == lParam)
                return PB_ERR;

            pData->nMin = MIN (wParam, lParam);
            pData->nMax = MAX (wParam, lParam);
            if (pData->nPos > pData->nMax)
                pData->nPos = pData->nMax;
            if (pData->nPos < pData->nMin)
                pData->nPos = pData->nMin;

            if (pData->nStepInc > (pData->nMax - pData->nMin))
                pData->nStepInc = pData->nMax - pData->nMin;
            return PB_OKAY;
        
        case PBM_SETSTEP:
            pData = (PROGRESSDATA *)pCtrl->dwAddData2;
            if (wParam > (pData->nMax - pData->nMin))
                return PB_ERR;

            pData->nStepInc = wParam;
            return PB_OKAY;
        
        case PBM_SETPOS:
            pData = (PROGRESSDATA *)pCtrl->dwAddData2;
            
            if (pData->nPos == wParam)
                return PB_OKAY;

            pData->nPos = wParam;
            pbarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & PBS_NOTIFY);

            InvalidateRect (hwnd, NULL, FALSE);
            return PB_OKAY;
        
        case PBM_DELTAPOS:
            pData = (PROGRESSDATA *)pCtrl->dwAddData2;

            if (wParam == 0)
                return PB_OKAY;
            
            pData->nPos += wParam;
            pbarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & PBS_NOTIFY);
                
            InvalidateRect (hwnd, NULL, FALSE);
            return PB_OKAY;
        
        case PBM_STEPIT:
            pData = (PROGRESSDATA *)pCtrl->dwAddData2;
            
            if (pData->nStepInc == 0)
                return PB_OKAY;

            pData->nPos += pData->nStepInc;
            pbarNormalizeParams (pCtrl, pData, pCtrl->dwStyle & PBS_NOTIFY);

            InvalidateRect (hwnd, NULL, FALSE);
            return PB_OKAY;
            
        case MSG_FONTCHANGED:
            InvalidateRect (hwnd, NULL, FALSE);
            break;
    }
    
    return DefaultControlProc (hwnd, message, wParam, lParam);
}

#endif /* _CTRL_PROGRESSBAR */

