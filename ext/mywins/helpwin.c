/*
** $Id: helpwin.c,v 1.7 2003/09/04 06:12:04 weiym Exp $
**
** helpwin.c: a useful help window.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/12/21
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
#include "mywindows.h"
#include "mgext.h"

#define IDC_SPIN    100

static CTRLDATA _help_win_ctrls [] =
{ 
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
        0, 0, 0, 0, IDOK, "OK"
    },
    {
        CTRL_SPINBOX,
        WS_CHILD | SPS_AUTOSCROLL | WS_VISIBLE,
        0, 0, 0, 0, IDC_SPIN, "",
    }
};

static DLGTEMPLATE _help_win =
{
    WS_BORDER | WS_CAPTION | WS_VISIBLE,
    WS_EX_NONE,
    0, 0, 0, 0,
    NULL,
    0, 0, 2
};

typedef struct tagHELPMSGINFO {
    const char* msg;
    int nr_lines;

    int vis_lines;
    int start_line;

    RECT rc;
} HELPMSGINFO;

static int _help_win_proc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    HELPMSGINFO* info;
    SPININFO spinfo;

    info = (HELPMSGINFO*) GetWindowAdditionalData (hDlg);

    switch (message) {
    case MSG_INITDIALOG:
        info = (HELPMSGINFO*) lParam;
        spinfo.min = 0;
        spinfo.max = info->nr_lines - info->vis_lines;
        if (spinfo.max < 0) spinfo.max = 0;
        spinfo.cur = 0;

        SendMessage ( GetDlgItem ( hDlg, IDC_SPIN), SPM_SETTARGET, 0, (LPARAM) hDlg);
        SendMessage ( GetDlgItem ( hDlg, IDC_SPIN), SPM_SETINFO, 0, (LPARAM) &spinfo);
        SetWindowAdditionalData (hDlg, (DWORD) lParam);
        return 1;
        
    case MSG_COMMAND:
        if (wParam == IDOK)
            EndDialog (hDlg, IDOK);
        break;

    case MSG_PAINT:
    {
        HDC hdc = BeginPaint (hDlg);
        RECT rc = info->rc;

        ClipRectIntersect (hdc, &info->rc);

        rc.top -= info->start_line * GetSysCharHeight ();
        SetBkMode (hdc, BM_TRANSPARENT);
        DrawText (hdc, info->msg, -1, &rc,
            DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EXPANDTABS);

        EndPaint (hDlg, hdc);
        return 0;
    }

    case MSG_KEYDOWN:
        if (wParam == SCANCODE_CURSORBLOCKUP) {
            if (info->start_line > 0) {
                info->start_line--;
                if (info->start_line == 0 && !(lParam & KS_SPINPOST))
                    SendDlgItemMessage ( hDlg, IDC_SPIN, SPM_SETCUR, info->start_line, 0);
                InvalidateRect (hDlg, &info->rc, TRUE);
            }
            return 0;
        } else if ( wParam == SCANCODE_CURSORBLOCKDOWN ) {
            if (info->start_line + info->vis_lines < info->nr_lines) {
                info->start_line++;
                if (info->start_line + info->vis_lines == info->nr_lines
                                && !(lParam & KS_SPINPOST))
                    SendDlgItemMessage ( hDlg, IDC_SPIN, SPM_SETCUR, info->start_line, 0);
                InvalidateRect (hDlg, &info->rc, TRUE);
            }
            return 0;
        } 
        break;

    case MSG_CLOSE:
        EndDialog (hDlg, IDOK);
        return 0;        
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

#define LEFT_MARGIN     12
#define BOTTOM_MARGIN   6

int myWinHelpMessage (HWND hwnd, int width, int height, 
                const char* help_title, const char* help_msg)
{
    HELPMSGINFO info;
    RECT rc = {0, 0, width - (LEFT_MARGIN + GetMainWinMetrics(MWM_BORDER)) * 2, height};

    _help_win_ctrls[0].x = LEFT_MARGIN;
    _help_win_ctrls[0].y = height - GetMainWinMetrics(MWM_BORDER) * 2 
            - GetMainWinMetrics(MWM_CAPTIONY) - SPINBOX_HEIGHT - BOTTOM_MARGIN;
    _help_win_ctrls[0].w = SPINBOX_WIDTH * 4;
    _help_win_ctrls[0].h = SPINBOX_HEIGHT + 2;

    _help_win_ctrls[0].caption = GetSysText ("OK");

    _help_win_ctrls[1].x = width - GetMainWinMetrics(MWM_BORDER) * 2
            - SPINBOX_WIDTH - LEFT_MARGIN;
    _help_win_ctrls[1].y = _help_win_ctrls[0].y;
    _help_win_ctrls[1].w = SPINBOX_WIDTH;
    _help_win_ctrls[1].h = SPINBOX_HEIGHT;

    _help_win.w = width;
    _help_win.h = height;
    _help_win.caption = help_title;
    _help_win.controls = _help_win_ctrls;

    info.msg = help_msg;

    DrawText (HDC_SCREEN, info.msg, -1, &rc,
        DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);

    info.nr_lines = RECTH (rc) / GetSysCharHeight (); 

    if (info.nr_lines <= 0) return -1;

    info.rc.top = BOTTOM_MARGIN;
    info.rc.left = LEFT_MARGIN;
    info.rc.right = width - GetMainWinMetrics(MWM_BORDER) * 2 - LEFT_MARGIN;
    info.rc.bottom = height - GetMainWinMetrics(MWM_BORDER) * 2
                        - GetMainWinMetrics(MWM_CAPTIONY) - BOTTOM_MARGIN
                        - SPINBOX_HEIGHT - BOTTOM_MARGIN;

    info.vis_lines = RECTH (info.rc) / GetSysCharHeight ();

    info.rc.bottom = info.rc.top + info.vis_lines * GetSysCharHeight ();

    info.start_line = 0;

    if (!InitMiniGUIExt ())
        return -1;

    DialogBoxIndirectParam (&_help_win, hwnd, _help_win_proc, (LPARAM)&info);

    MiniGUIExtCleanUp ();

    return 0;
}

