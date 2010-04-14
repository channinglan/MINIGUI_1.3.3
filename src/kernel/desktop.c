/* 
** $Id: desktop.c,v 1.72 2003/11/21 12:44:01 weiym Exp $
**
** desktop.c: The Desktop module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 1999/04/19
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
#include "menu.h"
#include "timer.h"
#include "misc.h"

/******************************* global data *********************************/
RECT g_rcScr;

pthread_t __mg_desktop, __mg_parsor, __mg_timer;
HWND __mg_capture_wnd;
HWND __mg_ime_wnd;
MSGQUEUE __mg_dsk_msgs;
PMAINWIN __mg_active_mainwnd;
PTRACKMENUINFO __mg_ptmi;


/********************* Window management support *****************************/
static BLOCKHEAP sg_FreeInvRectList;
static BLOCKHEAP sg_FreeClipRectList;
static ZORDERINFO sg_MainWinZOrder;
static ZORDERINFO sg_TopMostWinZOrder;

static HWND sg_hCaretWnd;
static UINT sg_uCaretBTime;

static GCRINFO sg_ScrGCRInfo;

static BOOL InitWndManagementInfo (void)
{
#ifndef _INCORE_RES
    if (!InitMainWinMetrics())
        return FALSE;
#endif

    __mg_capture_wnd = HWND_DESKTOP;
    __mg_active_mainwnd = NULL;

    __mg_ptmi = NULL;

    __mg_ime_wnd = HWND_DESKTOP;
    sg_hCaretWnd = HWND_DESKTOP;

    g_rcScr.left = g_rcScr.top = 0;
    g_rcScr.right = GetGDCapability (HDC_SCREEN, GDCAP_MAXX) + 1;
    g_rcScr.bottom = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) + 1;

    InitClipRgn (&sg_ScrGCRInfo.crgn, &sg_FreeClipRectList);
    SetClipRgn (&sg_ScrGCRInfo.crgn, &g_rcScr);
    pthread_mutex_init (&sg_ScrGCRInfo.lock, NULL);
    sg_ScrGCRInfo.age = 0;

    return TRUE;
}

//#include "sysres.c"

static void InitZOrderInfo (PZORDERINFO pZOrderInfo, HWND hHost);

BOOL InitDesktop (void)
{
    /*
     * Init ZOrderInfo here.
     */
    InitZOrderInfo (&sg_MainWinZOrder, HWND_DESKTOP);
    InitZOrderInfo (&sg_TopMostWinZOrder, HWND_DESKTOP);
    
    /*
     * Init heap of clipping rects.
     */
    InitFreeClipRectList (&sg_FreeClipRectList, SIZE_CLIPRECTHEAP);

    /*
     * Init heap of invalid rects.
     */
    InitFreeClipRectList (&sg_FreeInvRectList, SIZE_INVRECTHEAP);

    /*
     * Load system resource here.
     */
    if (!InitSystemRes ()) {
        fprintf (stderr, "DESKTOP: Can not initialize system resource!\n");
        return FALSE;
    }

    // Init Window Management information.
    if (!InitWndManagementInfo()) {
        fprintf (stderr, "DESKTOP: Can not initialize window management information!\n");
        return FALSE;
    }

    return TRUE;
}

#include "desktop-comm.c"

#ifdef _TRACE_MSG

void* DesktopMain (void* data)
{
    MSG Msg;
    PSYNCMSG pSyncMsg;
    int iRet = 0;

    DesktopWinProc (HWND_DESKTOP, MSG_STARTSESSION, 0, 0);
    PostMessage (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);

    // sem_post ((sem_t*)data);

    while (GetMessage(&Msg, HWND_DESKTOP)) {
        fprintf (stderr, "Message, %s: hWnd: %#x, wP: %#x, lP: %#lx. %s\n",
            Message2Str (Msg.message),
            Msg.hwnd,
            Msg.wParam,
            Msg.lParam,
            Msg.pAdd?"Sync":"Normal");

        if ( Msg.pAdd ) // this is a sync message.
        {
            pSyncMsg = (PSYNCMSG)(Msg.pAdd);
            pSyncMsg->retval = DesktopWinProc(HWND_DESKTOP, 
                   Msg.message, Msg.wParam, Msg.lParam);
            sem_post(&pSyncMsg->sem_handle);
            iRet = pSyncMsg->retval;
        }
        else
            iRet = DesktopWinProc(HWND_DESKTOP, 
                    Msg.message, Msg.wParam, Msg.lParam);

        fprintf (stderr, "Message, %s done, return value: %#x\n",
            Message2Str (Msg.message), iRet);
    }

    return NULL;
}

#else

void* DesktopMain(void* data)
{
    MSG Msg;
    PSYNCMSG pSyncMsg;

    DesktopWinProc (HWND_DESKTOP, MSG_STARTSESSION, 0, 0);
    PostMessage (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);

    sem_post ((sem_t*)data);

    while (GetMessage(&Msg, HWND_DESKTOP)) {

        if ( Msg.pAdd ) // this is a sync message.
        {
            pSyncMsg = (PSYNCMSG)(Msg.pAdd);
            pSyncMsg->retval = DesktopWinProc(HWND_DESKTOP, 
                   Msg.message, Msg.wParam, Msg.lParam);
            sem_post(&pSyncMsg->sem_handle);
        }
        else
            DesktopWinProc(HWND_DESKTOP, Msg.message, Msg.wParam, Msg.lParam);
    }

    return NULL;
}

#endif

pthread_t GUIAPI GetMainWinThread(HWND hMainWnd)
{
    if(hMainWnd == HWND_DESKTOP) return __mg_desktop;

    return ((PMAINWIN)hMainWnd)->th;
}

