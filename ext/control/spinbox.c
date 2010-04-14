/*
** $Id: spinbox.c,v 1.22 2003/09/29 02:52:46 snig Exp $
**
** spinbox.c: the SpinBox control
**
** Copyright (C) 2001 ~ 2002, Zhang Yunfan, Wei Yongming
** Copyright (C) 2003 Feynman Software
** 
** Create date: 2001/3/27
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
#else
	#include <minigui/common.h>
	#include <minigui/minigui.h>
	#include <minigui/gdi.h>
	#include <minigui/window.h>
	#include <minigui/control.h>
#endif

#ifdef _EXT_CTRL_SPINBOX

#include "mgext.h"
#include "spinbox.h"

#define _WIDTH	SPINBOX_WIDTH
#define _HEIGHT	SPINBOX_HEIGHT

static BITMAP bmp_spinbox;

#define SPINBOX_BMP (&bmp_spinbox)

typedef struct SpinData {
	SPININFO spinfo;
	HWND hTarget;
	int canup, candown;
} SPINDATA;
typedef SPINDATA* PSPINDATA;

static int SpinCtrlProc (HWND hWnd, int uMsg, WPARAM wParam, LPARAM lParam);

BOOL RegisterSpinControl (void)
{
    WNDCLASS WndClass;

    if (!LoadSystemBitmap (&bmp_spinbox, SYSBMP_SPINBOX))
        return FALSE;

    WndClass.spClassName = CTRL_SPINBOX;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (IDC_ARROW);
    WndClass.iBkColor    = GetWindowElementColor (BKC_CONTROL_DEF);
    WndClass.WinProc     = SpinCtrlProc;

	return RegisterWindowClass (&WndClass);
}

void SpinControlCleanup (void)
{
	UnregisterWindowClass (CTRL_SPINBOX);
}

static void DrawSpinBox ( HDC hdc, SPININFO* spinfo , SPINDATA* psd, int flag)
{
	if (spinfo->cur > spinfo->min || (flag && psd->canup)) {
		FillBoxWithBitmapPart (hdc, 0, 0, 13, 7, 0, 0,
				SPINBOX_BMP, 0, 14);
	}
    else {
		FillBoxWithBitmapPart (hdc, 0, 0, 13, 7, 0, 0,
				SPINBOX_BMP, 0, 21);
	}

	if (spinfo->cur < spinfo->max || (flag && psd->candown)) {
		FillBoxWithBitmapPart (hdc, 0, 9, 13, 7, 0, 0,
				SPINBOX_BMP, 0, 0);
	}
    else {
		FillBoxWithBitmapPart (hdc, 0, 9, 13, 7, 0, 0,
				SPINBOX_BMP, 0, 7);
	}
}

///////////////////////////////////////////////////////////////////////
static int SpinCtrlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC         hdc;
	PSPINDATA pData = (PSPINDATA) GetWindowAdditionalData2 (hWnd);
	int bAuto = (GetWindowStyle ( hWnd ) & SPS_AUTOSCROLL );
	int id = GetDlgCtrlID (hWnd);

    switch (message)
    {   
        case MSG_CREATE:
            pData = (PSPINDATA) calloc (1, sizeof (SPINDATA));
			if (!pData) {
				return HWND_INVALID;
			}
			pData->canup = 1;
			pData->candown =  1;
			pData->hTarget = GetParent ( hWnd );
			SetWindowAdditionalData2 (hWnd, (DWORD)pData);
        break;

        case MSG_DESTROY:
			free (pData);
        break;

		case MSG_SIZECHANGING:
		{
			const RECT* rcExpect = (const RECT*)wParam;
			RECT* rcResult = (RECT*)lParam;

			rcResult->left = rcExpect->left;
			rcResult->top = rcExpect->top;
			rcResult->right = rcExpect->left + _WIDTH;
			rcResult->bottom = rcExpect->top + _HEIGHT;
		    return 0;
		}

		case MSG_SIZECHANGED:
		{
			RECT* rcWin = (RECT*)wParam;
			RECT* rcClient = (RECT*)lParam;
			*rcClient = *rcWin;
			return 1;
		}

		case SPM_SETINFO: 
		{
			PSPININFO spinfo = (PSPININFO) lParam;
			if ( !spinfo )
				return -1;

			if (!bAuto) {
				memcpy ( &(pData->spinfo), spinfo, sizeof (SPININFO) );
			} else {
				if ( (spinfo->max >= spinfo->min &&
						spinfo->max >= spinfo->cur &&
						spinfo->cur >= spinfo-> min )) {
					memcpy ( &(pData->spinfo), spinfo, sizeof (SPININFO) );
				} else {
					return -1;
				}
			}
			InvalidateRect ( hWnd, NULL, FALSE );
			return 0;
		}

		case SPM_GETINFO:
		{
			PSPININFO spinfo = (PSPININFO) lParam;
			if ( !spinfo )
				return -1;
			memcpy ( spinfo, &(pData->spinfo), sizeof (SPININFO) );
			return 0;
		}

		case SPM_SETCUR:
			if ( wParam > pData->spinfo.max || wParam < pData->spinfo.min)
				return -1;
			pData->spinfo.cur = wParam;
			InvalidateRect (hWnd, NULL, FALSE );
			return 0;

		case SPM_GETCUR:
			return pData->spinfo.cur;

		case SPM_ENABLEUP:
			pData->canup = 1;
			InvalidateRect ( hWnd, NULL, FALSE );
			return 0;

		case SPM_ENABLEDOWN:
			pData->candown = 1;
			InvalidateRect ( hWnd, NULL, FALSE );
			return 0;

		case SPM_DISABLEUP:
			pData->canup = 0;
			InvalidateRect ( hWnd, NULL, FALSE );
			return 0;

		case SPM_DISABLEDOWN:
			pData->candown = 0;
			InvalidateRect ( hWnd, NULL, FALSE );
			return 0;

		case SPM_SETTARGET:
			pData->hTarget = (HWND) lParam;
			return 0;

		case SPM_GETTARGET:
			return pData->hTarget;

		case MSG_NCPAINT:
		case MSG_ERASEBKGND:
			return 0;

        case MSG_PAINT:
        {
            hdc = BeginPaint (hWnd);
			DrawSpinBox (hdc, &pData->spinfo, pData, !bAuto );
            EndPaint (hWnd, hdc);
            return 0;
        }

        case MSG_LBUTTONDOWN:
		{
			int posy = HIWORD (lParam);
			if ( posy <= (int) ( _HEIGHT / 2 - 1) ) {
				if ( bAuto ) {
					if (pData->spinfo.cur > pData->spinfo.min ) {
						pData->spinfo.cur --;
						PostMessage ( pData->hTarget, MSG_KEYDOWN, SCANCODE_CURSORBLOCKUP, (wParam|KS_SPINPOST));
						PostMessage ( pData->hTarget, MSG_KEYUP, SCANCODE_CURSORBLOCKUP, (wParam|KS_SPINPOST));
						if ( pData->spinfo.cur <= pData->spinfo.min ) {
							NotifyParent ( hWnd, id, SPN_REACHMIN);
						}
					}
				} else {
					if (pData->canup) {
						pData->spinfo.cur --;
						PostMessage ( pData->hTarget, MSG_KEYDOWN, SCANCODE_CURSORBLOCKUP, wParam|KS_SPINPOST);
						PostMessage ( pData->hTarget, MSG_KEYUP, SCANCODE_CURSORBLOCKUP, wParam|KS_SPINPOST);
						if ( pData->spinfo.cur <= pData->spinfo.min ) {
							NotifyParent ( hWnd, id, SPN_REACHMIN);
						}
					}
				}

			} else if ( posy >= (int) ( _HEIGHT / 2 + 1) ) {
				if (bAuto) {
					if (pData->spinfo.cur < pData->spinfo.max) {
						pData->spinfo.cur ++;
						PostMessage ( pData->hTarget, MSG_KEYDOWN, SCANCODE_CURSORBLOCKDOWN, wParam|KS_SPINPOST);
						PostMessage ( pData->hTarget, MSG_KEYUP, SCANCODE_CURSORBLOCKDOWN, wParam|KS_SPINPOST);
						if ( pData->spinfo.cur >= pData->spinfo.min ) {
							NotifyParent ( hWnd, id, SPN_REACHMAX);
						}
					}
				} else {
					if (pData->candown) {
						pData->spinfo.cur ++;
						PostMessage ( pData->hTarget, MSG_KEYDOWN, SCANCODE_CURSORBLOCKDOWN, wParam|KS_SPINPOST);
						PostMessage ( pData->hTarget, MSG_KEYUP, SCANCODE_CURSORBLOCKDOWN, wParam|KS_SPINPOST);
						if ( pData->spinfo.cur >= pData->spinfo.min ) {
							NotifyParent ( hWnd, id, SPN_REACHMAX);
						}
					}
				}
			}

			InvalidateRect ( hWnd, NULL, FALSE );

            if (GetCapture () == hWnd)
                break;
            SetCapture (hWnd);
            SetAutoRepeatMessage (hWnd, message, wParam, lParam);
       	    break;
		}
			
        case MSG_LBUTTONUP:
            SetAutoRepeatMessage (hWnd, 0, 0, 0);
            ReleaseCapture ();
            break;

		default:
            break;
    }

    return DefaultControlProc (hWnd, message, wParam, lParam);
}

#endif /* _EXT_CTRL_SPINBOX */

