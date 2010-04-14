/*
** $Id: edit.c,v 1.55 2003/11/23 04:09:08 weiym Exp $
**
** edit.c: the Single Line Edit Control module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
** 
** Current maintainer: Wei Yongming.
**
** Note:
**  Although there was a version by Zhao Jianghua, this version of
**  EDIT control is written by Wei Yongming from scratch.
**
** Create date: 1999/8/26
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
**    * Selection.
**    * Undo.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "control.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"

#ifdef _CTRL_SLEDIT

#include "ctrlmisc.h"
#include "edit.h"

#define DX_CHARS(Pos)  (sled->dx_chars[sled->Pos])
#define POS_CHARS(Pos)  (sled->pos_chars[sled->Pos])

static int SLEditCtrlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

BOOL RegisterSLEditControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = CTRL_SLEDIT;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (IDC_IBEAM);
    WndClass.iBkColor    = PIXEL_lightwhite;
    WndClass.WinProc     = SLEditCtrlProc;

    return AddNewControlClass (&WndClass) == ERR_OK;
}

#if 0
void SLEditControlCleanup (void)
{
    // do nothing
    return;
}
#endif

static int edtGetOutWidth (HWND hWnd, PSLEDITDATA sled)
{
    RECT rc;

    GetClientRect(hWnd, &rc);

    return RECTW(rc) - sled->leftMargin - sled->rightMargin;
}

static void edtGetLineInfo (HWND hWnd, PSLEDITDATA sled)
{
    HDC hdc;

    hdc = GetClientDC (hWnd);

    if (GetWindowStyle(hWnd) & ES_PASSWORD) {

        sled->nr_chars = GetTextMCharInfo (GetWindowFont (hWnd), 
                        sled->buffer, sled->dataEnd, sled->pos_chars);

        if (sled->nr_chars) {
#ifdef HAVE_ALLOCA
            unsigned char* temp = alloca (sled->nr_chars);
#else
            unsigned char* temp = FixStrAlloc (sled->nr_chars);
#endif
            memset (temp, '*', sled->nr_chars);
            GetTextExtentPoint (hdc, temp, sled->nr_chars, 0, NULL, NULL, 
                            sled->dx_chars, &(sled->extent));
#ifndef HAVE_ALLOCA
            FreeFixStr (temp);
#endif
        }
    }
    else {
        GetTextExtentPoint (hdc, sled->buffer, sled->dataEnd, 0, 
                        &(sled->nr_chars), 
                        sled->pos_chars, sled->dx_chars, &(sled->extent));
    }

    sled->pos_chars[sled->nr_chars] = sled->dataEnd;
    sled->dx_chars[sled->nr_chars] = sled->extent.cx;

    ReleaseDC (hdc);
}

static int edtGetPosByX (PSLEDITDATA sled, int x)
{
    int i = 0;

    if (x < 0) return 0;
    
    for (i = 0; i < sled->nr_chars; i++) {
        if (sled->dx_chars[i] >= x)
            return i;
    }
    
    return i;
}

static int edtGetStartDispPosAtEnd (const HWND hWnd, PSLEDITDATA sled)
{
    int out_width = edtGetOutWidth (hWnd, sled);

    if (sled->extent.cx <= out_width)
        return 0;

    return edtGetPosByX (sled, sled->extent.cx - out_width);
}

static void edtOnEraseBackground (HWND hWnd, DWORD dwStyle, HDC hdc, const RECT* pClipRect)
{
    RECT rcTemp;
    BOOL fGetDC = FALSE;
    PSLEDITDATA sled = (PSLEDITDATA) GetWindowAdditionalData2(hWnd);

    if (hdc == 0) {
        hdc = GetClientDC (hWnd);
        fGetDC = TRUE;
    }

    GetClientRect (hWnd, &rcTemp);

    if (pClipRect)
        ClipRectIntersect (hdc, pClipRect);

    if (dwStyle & WS_DISABLED) {
        SetBrushColor (hdc, PIXEL_lightgray);
    }
    else {
        SetBrushColor (hdc, PIXEL_lightwhite);
    }

    FillBox (hdc, rcTemp.left, rcTemp.top, 
                 RECTW (rcTemp), RECTH (rcTemp));

    if (dwStyle & ES_BASELINE) {
        SetPenColor (hdc, PIXEL_black);
        DrawHDotLine (hdc, 
                    sled->leftMargin, 
                    sled->topMargin + GetWindowFont (hWnd)->size + 1,
                    RECTW (rcTemp) - sled->leftMargin - sled->rightMargin);
    }

    if (fGetDC)
        ReleaseDC (hdc);
}

static int SLEditCtrlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{   
    PCONTROL    pCtrl;
    DWORD       dwStyle;
    HDC         hdc;
    PSLEDITDATA sled;

    pCtrl       = Control(hWnd);
    dwStyle     = GetWindowStyle(hWnd);

    switch (message)
    {
        case MSG_CREATE:
            if (!(sled = calloc (1, sizeof (SLEDITDATA)))) {
                return -1;
            }

            sled->status         = 0;
            sled->editPos        = 0;
            sled->startPos       = 0;
            
            sled->nr_chars      = 0;

            sled->bufferLen      = LEN_SLEDIT_BUFFER - 1;
            sled->buffer         = calloc (LEN_SLEDIT_BUFFER, sizeof(char));
            sled->pos_chars      = calloc (LEN_SLEDIT_BUFFER, sizeof(int));
            sled->dx_chars       = calloc (LEN_SLEDIT_BUFFER, sizeof(int));

            if (!sled->buffer || !sled->pos_chars || !sled->dx_chars) {
                free (sled->buffer);
                free (sled->pos_chars);
                free (sled->dx_chars);
                free (sled);
                return -1;
            }

            sled->passwdChar     = '*';
            sled->leftMargin     = MARGIN_EDIT_LEFT;
            sled->topMargin      = MARGIN_EDIT_TOP;
            sled->rightMargin    = MARGIN_EDIT_RIGHT;
            sled->bottomMargin   = MARGIN_EDIT_BOTTOM;

            sled->hardLimit      = -1;
            
            sled->dataEnd        = strlen (GetWindowCaption (hWnd));
            sled->dataEnd        = MIN (sled->bufferLen, sled->dataEnd);
            memcpy (sled->buffer, GetWindowCaption (hWnd), sled->dataEnd);

            if (!CreateCaret (hWnd, NULL, 1, GetWindowFont (hWnd)->size)) {
                free (sled->buffer);
                free (sled->pos_chars);
                free (sled->dx_chars);
                free (sled);
                return -1;
            }

            edtGetLineInfo (hWnd, sled);
           
            SetWindowAdditionalData2 (hWnd,(DWORD)sled);
            break;

        case MSG_DESTROY:
            sled = (PSLEDITDATA) GetWindowAdditionalData2(hWnd);
            DestroyCaret (hWnd);
            free (sled->buffer);
            free (sled->pos_chars);
            free (sled->dx_chars);
            free (sled);
            break;

        case MSG_ERASEBKGND:
        {
            BOOL hidden = HideCaret (hWnd);
            edtOnEraseBackground (hWnd, dwStyle, (HDC)wParam, (const RECT*)lParam);
            if (hidden)
                ShowCaret (hWnd);
            return 0;
        }

        case MSG_FONTCHANGING:
            return 0;

        case MSG_FONTCHANGED:
        {
            sled =(PSLEDITDATA) GetWindowAdditionalData2 (hWnd);

            sled->startPos = 0;
            sled->editPos = 0;
            edtGetLineInfo (hWnd, sled);

            DestroyCaret (hWnd);
            CreateCaret (hWnd, NULL, 1, GetWindowFont (hWnd)->size);
            SetCaretPos (hWnd, sled->leftMargin, sled->topMargin);
            InvalidateRect (hWnd, NULL, TRUE);
            return 0;
        }
        
        case MSG_SETCURSOR:
            if (dwStyle & WS_DISABLED) {
                SetCursor (GetSystemCursor (IDC_ARROW));
                return 0;
            }
            break;

        case MSG_KILLFOCUS:
            sled = (PSLEDITDATA) GetWindowAdditionalData2(hWnd);
            if (sled->status & EST_FOCUSED) {
                sled->status &= ~EST_FOCUSED;
                HideCaret (hWnd);
                NotifyParent (hWnd, GetDlgCtrlID(hWnd), EN_KILLFOCUS);
            }
            break;
        
        case MSG_SETFOCUS:
            sled = (PSLEDITDATA) GetWindowAdditionalData2(hWnd);
            if (sled->status & EST_FOCUSED)
                break;
            
            sled->status |= EST_FOCUSED;

            SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                         sled->topMargin);
            ShowCaret (hWnd);
            ActiveCaret (hWnd);

            NotifyParent (hWnd, GetDlgCtrlID(hWnd), EN_SETFOCUS);
            break;
        
        case MSG_ENABLE:
            if ( (!(dwStyle & WS_DISABLED) && !wParam)
                    || ((dwStyle & WS_DISABLED) && wParam) ) {
                if (wParam)
                    ExcludeWindowStyle(hWnd,WS_DISABLED);
                else
                    IncludeWindowStyle(hWnd,WS_DISABLED);

                InvalidateRect (hWnd, NULL, TRUE);
            }
            return 0;

        case MSG_NCPAINT:
            if (wParam)
                hdc = wParam;
            else
                hdc = GetDC (hWnd);

            if (lParam)
                ClipRectIntersect (hdc, (RECT*)lParam);

            if (dwStyle & WS_BORDER)
#ifdef _FLAT_WINDOW_STYLE
                DrawFlatControlFrameEx (hdc, 0, 0,
                                      pCtrl->right - pCtrl->left - 1,
                                      pCtrl->bottom - pCtrl->top - 1,
                                      PIXEL_invalid, 1, -1);
#else
                Draw3DDownFrame (hdc, 0, 0, 
                                      pCtrl->right - pCtrl->left - 1, 
                                      pCtrl->bottom - pCtrl->top - 1,
                                      PIXEL_invalid);
#endif
            if (!wParam)
                ReleaseDC (hdc);
            return 0;

        case MSG_PAINT:
        {
            char*   dispBuffer;
            int     len;
            RECT    rect;
            
            sled = (PSLEDITDATA) (pCtrl->dwAddData2);

            if (dwStyle & ES_PASSWORD) {
                len = sled->nr_chars - sled->startPos;
                //dispBuffer = alloca (len);
                dispBuffer = FixStrAlloc (len);
                memset (dispBuffer, '*', len);
            }
            else {
                dispBuffer = sled->buffer + POS_CHARS(startPos);
                len = sled->dataEnd - POS_CHARS(startPos);
            }

            GetClientRect (hWnd, &rect);
            rect.left += sled->leftMargin;
            rect.top += sled->topMargin;
            rect.right -= sled->rightMargin;
            rect.bottom -= sled->bottomMargin;
            
            hdc = BeginPaint (hWnd);

            if (dwStyle & WS_DISABLED) {
                SetBrushColor (hdc, PIXEL_lightgray);
                SetBkColor (hdc, PIXEL_lightgray);
            }
            else {
                SetBrushColor (hdc, PIXEL_lightwhite);
                SetBkColor (hdc, PIXEL_lightwhite);
            }

            SetTextColor (hdc, PIXEL_black);
            
            ClipRectIntersect (hdc, &rect);
            TextOutLen (hdc, sled->leftMargin, sled->topMargin, 
                     dispBuffer, len);
            
            if (dwStyle & ES_PASSWORD) {
                FreeFixStr (dispBuffer);
            }

            EndPaint (hWnd, hdc);
            return 0;
        }

        case MSG_KEYDOWN:
        {
            BOOL    bChange = FALSE;
            RECT    InvRect;
        
            sled = (PSLEDITDATA) (pCtrl->dwAddData2);

            InvRect.left = sled->leftMargin - 1;
            InvRect.top = sled->topMargin;
            InvRect.right = pCtrl->cr - pCtrl->cl;
            InvRect.bottom = pCtrl->cb - pCtrl->ct;
        
            switch (LOWORD (wParam)) {
                case SCANCODE_ENTER:
                    NotifyParent (hWnd, pCtrl->id, EN_ENTER);
                    return 0;

                case SCANCODE_HOME:
                    if (sled->editPos == 0)
                        return 0;

                    sled->editPos  = 0;

                    if (sled->startPos != 0) {
                        sled->startPos = 0;
                        InvalidateRect (hWnd, NULL, TRUE);
                    }
                    SetCaretPos (hWnd, sled->leftMargin, sled->topMargin);
                return 0;
           
                case SCANCODE_END:
                {
                    int newStartPos;
                   
                    if (sled->editPos == sled->nr_chars)
                        return 0;

                    sled->editPos = sled->nr_chars;

                    newStartPos = edtGetStartDispPosAtEnd (hWnd, sled);
                    if (sled->startPos != newStartPos) {
                        sled->startPos = newStartPos;
                        InvalidateRect (hWnd, NULL, TRUE);
                    }

                    SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                                    sled->topMargin);
                }
                return 0;

                case SCANCODE_CURSORBLOCKLEFT:
                {
                    BOOL bScroll = FALSE;
                    
                    if (sled->editPos == 0)
                        return 0;

                    if (sled->editPos == sled->startPos) {
                        sled->editPos --;
                        sled->startPos = edtGetPosByX (sled, 
                                        DX_CHARS(editPos) - edtGetOutWidth (hWnd, sled)/4);
                        bScroll = TRUE;
                    }
                    else
                        sled->editPos --;

                    SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                                 sled->topMargin);

                    if (bScroll)
                        InvalidateRect (hWnd, NULL, TRUE);
                }
                return 0;
                
                case SCANCODE_CURSORBLOCKRIGHT:
                {
                    BOOL bScroll = FALSE;
                    int  out_width = edtGetOutWidth (hWnd, sled);

                    if (sled->editPos == sled->nr_chars)
                        return 0;

                    if ((DX_CHARS(editPos) - DX_CHARS(startPos) <= out_width) 
                                    && (sled->dx_chars[sled->editPos + 1] - DX_CHARS(startPos) > out_width)) {
                            bScroll = TRUE;
                            sled->editPos ++;
        
                        sled->startPos = edtGetPosByX (sled, (DX_CHARS(editPos) - out_width /4));

                        if ((DX_CHARS(startPos) + out_width) > sled->extent.cx) 
                            sled->startPos = edtGetStartDispPosAtEnd (hWnd, sled);
                    }
                    else 
                        sled->editPos ++;

                    SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                                 sled->topMargin);

                    if (bScroll)
                        InvalidateRect (hWnd, NULL, TRUE);
                }
                return 0;
                
                case SCANCODE_INSERT:
                    sled = (PSLEDITDATA) (pCtrl->dwAddData2);
                    sled->status ^= EST_REPLACE;
                break;

                case SCANCODE_REMOVE:
                {
                    int deleted, i;
                
                    sled = (PSLEDITDATA) (pCtrl->dwAddData2);

                    if ((dwStyle & ES_READONLY) || (sled->editPos == sled->nr_chars))
                        return 0;
                    
                    deleted = sled->pos_chars[sled->editPos + 1] - POS_CHARS(editPos);
                    for (i = POS_CHARS(editPos); i < sled->dataEnd - deleted; i++)
                        sled->buffer [i] = sled->buffer [i + deleted];

                    sled->dataEnd -= deleted;

                    edtGetLineInfo (hWnd, sled);
                    bChange = TRUE;

                    SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                                    sled->topMargin);

                    InvRect.left = (DX_CHARS(editPos) - DX_CHARS(startPos)) + sled->leftMargin - 1;
                    InvalidateRect (hWnd, &InvRect, TRUE);
                }
                break;

                case SCANCODE_BACKSPACE:
                {
                    int deleted, i;

                    sled = (PSLEDITDATA) (pCtrl->dwAddData2);
            
                    if ((dwStyle & ES_READONLY)
                            || (sled->editPos == 0)) {
                        return 0;
                    }

                    deleted = POS_CHARS(editPos) - sled->pos_chars[sled->editPos - 1];
                    for (i = POS_CHARS(editPos); i < sled->dataEnd; i++)
                        sled->buffer [i - deleted] = sled->buffer [i];

                    sled->dataEnd -= deleted;

                    if (sled->startPos == sled->editPos) {
                        sled->editPos --;
                        sled->startPos = edtGetPosByX (sled, 
                                        DX_CHARS(editPos) - edtGetOutWidth (hWnd, sled)/4);
                    }
                    else {
                        sled->editPos --;
                        InvRect.left = (DX_CHARS(editPos) - DX_CHARS(startPos))
                                       + sled->leftMargin - 1;
                    }
                    bChange = TRUE;
                    
                    edtGetLineInfo (hWnd, sled);
                    
                    SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                                    sled->topMargin);

                    InvalidateRect (hWnd, &InvRect, TRUE);
                }
                break;

                default:
                break;
            }
       
            if (bChange)
                NotifyParent (hWnd, pCtrl->id, EN_CHANGE);
            return 0;
        }

        case MSG_CHAR:
        {
            unsigned char charBuffer [2];
            int  i, chars, inserting, olddispLen, out_width;
            RECT InvRect;
            BOOL bScroll = FALSE;

            sled = (PSLEDITDATA) (pCtrl->dwAddData2);

            InvRect.left = sled->leftMargin - 1;
            InvRect.top = sled->topMargin;
            InvRect.right = pCtrl->cr - pCtrl->cl;
            InvRect.bottom = pCtrl->cb - pCtrl->ct;

            if (dwStyle & ES_READONLY) {
                return 0;
            }
            
            if (HIBYTE (wParam)) {
                charBuffer [0] = LOBYTE (wParam);
                charBuffer [1] = HIBYTE (wParam);
                chars = 2;
            }
            else {
                chars = 1;

                if (dwStyle & ES_UPPERCASE) {
                    charBuffer [0] = toupper (LOBYTE (wParam));
                }
                else if (dwStyle & ES_LOWERCASE) {
                    charBuffer [0] = tolower (LOBYTE (wParam));
                }
                else
                    charBuffer [0] = LOBYTE (wParam);
            }
            
            if (chars == 1) {
                if (charBuffer [0] < 0x20 || charBuffer [0] == 0x7F)
                    return 0;
            }

            if (sled->status & EST_REPLACE) {
                if (sled->nr_chars == sled->editPos)
                    inserting = chars;
                else 
                    inserting = chars - ( sled->pos_chars[sled->editPos + 1] - POS_CHARS(editPos) );
            }
            else
                inserting = chars;

            // check space
            if (sled->dataEnd + inserting > sled->bufferLen) {
                Ping ();
                NotifyParent (hWnd, pCtrl->id, EN_MAXTEXT);
                return 0;
            }
            else if ((sled->hardLimit >= 0) && ((sled->dataEnd + inserting) > sled->hardLimit)) {
                Ping ();
                NotifyParent (hWnd, pCtrl->id, EN_MAXTEXT);
                return 0;
            }

            olddispLen = DX_CHARS(editPos) - DX_CHARS(startPos);

            if (inserting == -1) {
                for (i = POS_CHARS(editPos); i < sled->dataEnd - 1; i++)
                    sled->buffer [i] = sled->buffer [i + 1];
            }
            else if (inserting > 0) {
                for (i = sled->dataEnd + inserting - 1; i > POS_CHARS(editPos) + inserting - 1; i--)
                    sled->buffer [i] = sled->buffer [i - inserting];
            }

            for (i = 0; i < chars; i++)
                sled->buffer [POS_CHARS(editPos) + i] = charBuffer [i];
            
            sled->editPos += 1;
            sled->dataEnd += inserting;

            edtGetLineInfo (hWnd, sled);

            out_width = edtGetOutWidth (hWnd, sled);
            if (((DX_CHARS(editPos) - DX_CHARS(startPos)) >= out_width)
                    && ((sled->dx_chars[sled->editPos - 1] - DX_CHARS(startPos)) < out_width)) {
                bScroll = TRUE;
                sled->startPos = edtGetPosByX (sled, DX_CHARS(editPos) - out_width * 3 / 4);
            }
            
            SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                            sled->topMargin);

            if (!bScroll)
                InvRect.left = sled->leftMargin + olddispLen - 1;

            InvalidateRect (hWnd, &InvRect, TRUE);

            NotifyParent (hWnd, pCtrl->id, EN_CHANGE);
            return 0;
        }

        case MSG_GETTEXTLENGTH:
            sled = (PSLEDITDATA) (pCtrl->dwAddData2);
            return sled->dataEnd;
        
        case MSG_GETTEXT:
        {
            char*   buffer = (char*)lParam;
            int     len;

            sled = (PSLEDITDATA) (pCtrl->dwAddData2);

            len = MIN ((int)wParam, sled->dataEnd);

            memcpy (buffer, sled->buffer, len);
            buffer [len] = '\0';

            return len;
        }

        case MSG_SETTEXT:
        {
            int len;
            sled = (PSLEDITDATA) (pCtrl->dwAddData2);
            
            if (lParam == 0)
                return -1;

            len = strlen ((char*)lParam);
            len = MIN (len, sled->bufferLen);
            
            if (sled->hardLimit >= 0)
                len = MIN (len, sled->hardLimit);
           
            sled->dataEnd  = len;
            memcpy (sled->buffer, (char*)lParam, len);

            edtGetLineInfo (hWnd, sled);

            sled->editPos  = 0;
            sled->startPos = 0;
            if (sled->status & EST_FOCUSED)
                SetCaretPos (hWnd, sled->leftMargin, sled->topMargin);

            InvalidateRect (hWnd, NULL, TRUE);
            return 0;
        }

        case MSG_LBUTTONDBLCLK:
            NotifyParent (hWnd, pCtrl->id, EN_DBLCLK);
            break;
        
        case MSG_LBUTTONDOWN:
        {
            sled = (PSLEDITDATA) (pCtrl->dwAddData2);
            
            sled->editPos = edtGetPosByX (sled, LOSWORD (lParam) + DX_CHARS(startPos));

            SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                            sled->topMargin);

            NotifyParent (hWnd, pCtrl->id, EN_CLICKED);
            break;
        }

#if 0
        case MSG_LBUTTONUP:
        case MSG_MOUSEMOVE:
            break;
#endif
        
        case MSG_GETDLGCODE:
            return DLGC_WANTCHARS | DLGC_HASSETSEL | DLGC_WANTARROWS | DLGC_WANTENTER;

        case MSG_DOESNEEDIME:
            if (dwStyle & ES_READONLY)
                return FALSE;
            return TRUE;
        
        case EM_GETCARETPOS:
        {
            int* line_pos = (int *)wParam;
            int* char_pos = (int *)lParam;

            sled = (PSLEDITDATA) (pCtrl->dwAddData2);

            if (line_pos) *line_pos = 0;
            if (char_pos) *char_pos = sled->editPos;

            return sled->editPos;
        }

        case EM_SETCARETPOS:
        {
            int char_pos = (int)lParam;

            sled = (PSLEDITDATA) (pCtrl->dwAddData2);

            if (char_pos >= 0 && char_pos <= sled->nr_chars) {
                sled->editPos = char_pos;
                if (sled->status & EST_FOCUSED)
                    SetCaretPos (hWnd, DX_CHARS(editPos) - DX_CHARS(startPos) + sled->leftMargin, 
                                    sled->topMargin);
                return 0;
            }

            return -1;
        }

        case EM_SETREADONLY:
            if (wParam)
                pCtrl->dwStyle |= ES_READONLY;
            else
                pCtrl->dwStyle &= ~ES_READONLY;
            return 0;
        
        case EM_SETPASSWORDCHAR:
            sled = (PSLEDITDATA) (pCtrl->dwAddData2);

            if (sled->passwdChar != (int)wParam) {
                if (dwStyle & ES_PASSWORD) {
                    sled->passwdChar = (int)wParam;
                    InvalidateRect (hWnd, NULL, TRUE);
                }
            }
            return 0;
    
        case EM_GETPASSWORDCHAR:
        {
            int* passwdchar;
            
            sled = (PSLEDITDATA) (pCtrl->dwAddData2);
            passwdchar = (int*) lParam;

            *passwdchar = sled->passwdChar;
            return 0;
        }
    
        case EM_LIMITTEXT:
        {
            int newLimit = (int)wParam;
            
            if (newLimit >= 0) {
                sled = (PSLEDITDATA) (pCtrl->dwAddData2);
                if (sled->bufferLen < newLimit)
                    sled->hardLimit = -1;
                else
                    sled->hardLimit = newLimit;
            }
            return 0;
        }
    
        default:
            break;
    } 

    return DefaultControlProc (hWnd, message, wParam, lParam);
}

#endif /* _CTRL_SLEDIT */

