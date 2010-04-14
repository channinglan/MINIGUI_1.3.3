/* 
** $Id: init.c,v 1.44 2003/11/23 05:07:36 weiym Exp $
**
** init.c: The Initialization/Termination routines
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/11/05
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
#include <unistd.h>
#include <signal.h>

#ifndef __NOLINUX__
#include <sys/termios.h>
#endif

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "ial.h"
#include "internals.h"
#include "ctrlclass.h"
#include "cursor.h"
#include "event.h"
#include "misc.h"
#include "menu.h"
#include "timer.h"
#include "accelkey.h"

/******************************* extern data *********************************/
extern void* DesktopMain (void* data);

/************************* Entry of the thread of parsor *********************/
static BOOL QueueMessage (PMSG msg)
{
    int sem_value;

    pthread_mutex_lock (&__mg_dsk_msgs.lock);

    if ((__mg_dsk_msgs.writepos + 1) % __mg_dsk_msgs.len == __mg_dsk_msgs.readpos) {
        pthread_mutex_unlock (&__mg_dsk_msgs.lock);
        return FALSE;
    }

    // Write the data and advance write pointer */
    __mg_dsk_msgs.msg [__mg_dsk_msgs.writepos] = *msg;

    __mg_dsk_msgs.writepos++;
    if (__mg_dsk_msgs.writepos >= __mg_dsk_msgs.len) __mg_dsk_msgs.writepos = 0;

    __mg_dsk_msgs.dwState |= QS_POSTMSG;

    pthread_mutex_unlock (&__mg_dsk_msgs.lock);
   
    // Signal that the msg queue contains one more element for reading
    sem_getvalue (&__mg_dsk_msgs.wait, &sem_value);
    if (sem_value == 0)
       sem_post (&__mg_dsk_msgs.wait);

    return TRUE;
}

static void ParseEvent (PLWEVENT lwe)
{
    PMOUSEEVENT me;
    PKEYEVENT ke;
    MSG Msg;
    static int mouse_x = 0, mouse_y = 0;

    ke = &(lwe->data.ke);
    me = &(lwe->data.me);
    Msg.hwnd = HWND_DESKTOP;
    Msg.wParam = 0;
    Msg.lParam = 0;

    gettimeofday (&Msg.time, NULL);
    if (lwe->type == LWETYPE_TIMEOUT) {
        Msg.message = MSG_TIMEOUT;
        Msg.wParam = (WPARAM)lwe->count;
        Msg.lParam = 0;
        QueueMessage (&Msg);
    }
    else if (lwe->type == LWETYPE_KEY) {
        Msg.wParam = ke->scancode;
        Msg.lParam = ke->status;
        if(ke->event == KE_KEYDOWN){
            Msg.message = MSG_KEYDOWN;
        }
        else if(ke->event == KE_KEYUP) {
            Msg.message = MSG_KEYUP;
        }
        QueueMessage (&Msg);
    }
    else if(lwe->type == LWETYPE_MOUSE) {
        Msg.wParam = me->status;
        Msg.lParam = MAKELONG (me->x, me->y);

        switch (me->event) {
        case ME_MOVED:
            Msg.message = MSG_MOUSEMOVE;
            break;
        case ME_LEFTDOWN:
            Msg.message = MSG_LBUTTONDOWN;
            break;
        case ME_LEFTUP:
            Msg.message = MSG_LBUTTONUP;
            break;
        case ME_LEFTDBLCLICK:
            Msg.message = MSG_LBUTTONDBLCLK;
            break;
        case ME_RIGHTDOWN:
            Msg.message = MSG_RBUTTONDOWN;
            break;
        case ME_RIGHTUP:
            Msg.message = MSG_RBUTTONUP;
            break;
        case ME_RIGHTDBLCLICK:
            Msg.message = MSG_RBUTTONDBLCLK;
            break;
        }

        if (me->event != ME_MOVED && (mouse_x != me->x || mouse_y != me->y)) {
            int old = Msg.message;

            Msg.message = MSG_MOUSEMOVE;
            QueueMessage (&Msg);
            Msg.message = old;

            mouse_x = me->x; mouse_y = me->y;
        }

        QueueMessage (&Msg);
    }
}

extern struct timeval __mg_event_timeout;

static void* EventLoop (void* data)
{
    LWEVENT lwe;
    int event;

    lwe.data.me.x = 0; lwe.data.me.y = 0;

    sem_post ((sem_t*)data);

    while (TRUE) {
        event = IAL_WaitEvent (IAL_MOUSEEVENT | IAL_KEYEVENT, 
                        NULL, NULL, NULL, &__mg_event_timeout);
        if (event < 0)
            continue;

        lwe.status = 0L;
        if (event & IAL_MOUSEEVENT && GetLWEvent (IAL_MOUSEEVENT, &lwe))
            ParseEvent (&lwe);

        lwe.status = 0L;
        if (event & IAL_KEYEVENT && GetLWEvent (IAL_KEYEVENT, &lwe))
            ParseEvent (&lwe);

        if (event == 0 && GetLWEvent (0, &lwe))
            ParseEvent (&lwe);
    }

    return NULL;
}

/************************* Entry of the thread of timer **********************/
static void* TimerEntry (void* data)
{
    if (!InitTimer ()) {
        fprintf (stderr, "TIMER: Init Timer failure!\n");
        exit (1);
        return NULL;
    }

    sem_post ((sem_t*)data);

    pthread_join (__mg_desktop, NULL);

    return NULL;
}

/************************** System Initialization ****************************/
static BOOL SystemThreads(void)
{
    sem_t wait;

    if (!InitDesktop ()) {
        fprintf (stderr, "DESKTOP: Init Desktop error!\n");
        return FALSE;
    }
   
    if (!InitFreeQMSGList ()) {
        fprintf (stderr, "DESKTOP: Init free QMSG list error!\n");
        return FALSE;
    }

    if (!InitMsgQueue(&__mg_dsk_msgs, 0)) {
        DestroyFreeQMSGList ();
        fprintf (stderr, "DESKTOP: Init MSG queue error!\n");
        return FALSE;
    }

    sem_init (&wait, 0, 0);

    // this is the thread for desktop window.
    // this thread should have a normal priority same as
    // other main window threads. 
    // if there is no main window can receive the low level events,
    // this desktop window is the only one can receive the events.
    // to close a MiniGUI application, we should close the desktop 
    // window.
    pthread_create(&__mg_desktop, NULL, DesktopMain, &wait);
    sem_wait (&wait);

    // this is the thread of timer.
    // when this thread start, it init timer data and install
    // a signal handler of SIGPROF, and call setitimer system call
    // to install an interval timer.
    // after initialization, this thread wait desktop to terminate,
    // and then remove the interval timer and free data structs.
    // the signal handler will alert desktop a MSG_TIMER message.
    pthread_create(&__mg_timer, NULL, TimerEntry, &wait);
    sem_wait (&wait);
    
    // this thread collect low level event from SVGALib,
    // if there is no event, this thread will suspend to wait a event.
    // the events maybe mouse event, keyboard event, or timeout event.
    //
    // this thread also parse low level event and translate it to message,
    // then post the message to the approriate message queue.
    // this thread should also have a higher priority.
    // this thread also translate SVGALib events to MiniGUI message, 
    // for example, it translate mouse button event to mouse button
    // down and mouse button up, as well as mouse double click event.
    // this thread works as a mouse and keyboard driver.
    pthread_create(&__mg_parsor, NULL, EventLoop, &wait);
    sem_wait (&wait);

    sem_destroy (&wait);

    return TRUE;
}

BOOL GUIAPI ReinitDesktopEx (BOOL init_sys_text)
{
    return SendMessage (HWND_DESKTOP, MSG_REINITSESSION, init_sys_text, 0) == 0;
}

#ifndef __NOLINUX__
static struct termios savedtermio;

void* GUIAPI GetOriginalTermIO (void)
{
    return &savedtermio;
}
#endif

static void segvsig_handler (int v)
{
    TerminateLWEvent ();
    TerminateGAL ();

    if (v == SIGSEGV)
        kill (getpid(), SIGABRT); /* cause core dump */
    else
        _exit (v);
}

static BOOL InstallSEGVHandler (void)
{
    struct sigaction siga;
    
    siga.sa_handler = segvsig_handler;
    siga.sa_flags = 0;
    
    memset (&siga.sa_mask, 0, sizeof (sigset_t));
    sigaction (SIGSEGV, &siga, NULL);
    sigaction (SIGTERM, &siga, NULL);
    sigaction (SIGINT, &siga, NULL);

    return TRUE;
}

BOOL GUIAPI InitGUI (void)
{
#ifndef __NOLINUX__
    // Save original termio
    tcgetattr (0, &savedtermio);
#endif

    // Init miscelleous
    if (!InitMisc ()) {
        fprintf (stderr, "DESKTOP: Initialization of misc things failure!\n");
        return FALSE;
    }

    switch (InitGAL ()) {
    case ERR_CONFIG_FILE:
        fprintf (stderr, 
            "GDI: Reading configuration failure!\n");
        return FALSE;

    case ERR_NO_ENGINE:
        fprintf (stderr, 
            "GDI: No graphics engine defined!\n");
        return FALSE;

    case ERR_NO_MATCH:
        fprintf (stderr, 
            "GDI: Can not get graphics engine information!\n");
        return FALSE;

    case ERR_GFX_ENGINE:
        fprintf (stderr, 
            "GDI: Can not initialize graphics engine!\n");
        return FALSE;
    }

    InstallSEGVHandler ();

    /* Init GDI. */
    if(!InitGDI()) {
        fprintf (stderr, "DESKTOP: Initialization of GDI failure!\n");
        goto failure1;
    }

#ifdef _USE_NEWGAL
    /* Init Screen DC here */
    if (!InitScreenDC ()) {
        fprintf (stderr, "Can not initialize screen DC!\n");
        goto failure1;
    }
#endif

    if (!InitWindowElementColors ()) {
        fprintf (stderr, "DESKTOP: Can not initialize colors of window element!\n");
        goto failure1;
    }

    // Init low level event
    if(!InitLWEvent()) {
        fprintf(stderr, "DESKTOP: Low level event initialization failure!\n");
        goto failure1;
    }

    if (!InitFixStr ()) {
        fprintf (stderr, "DESKTOP: Init Fixed String module failure!\n");
        goto failure;
    }
    
#ifdef _CURSOR_SUPPORT
    // Init mouse cursor.
    if( !InitCursor() ) {
        fprintf (stderr, "DESKTOP: Count not initialize mouse cursor support!\n");
        goto failure;
    }
#endif

    // Init menu
    if (!InitMenu ()) {
        fprintf (stderr, "DESKTOP: Init Menu module failure!\n");
        goto failure;
    }

    // Init control class
    if(!InitControlClass()) {
        fprintf(stderr, "DESKTOP: Init Control Class failure!\n");
        goto failure;
    }

    // Init accelerator
    if(!InitAccel()) {
        fprintf(stderr, "DESKTOP: Init Accelerator failure!\n");
        goto failure;
    }

    if (!SystemThreads())
        goto failure;

    SetKeyboardLayout ("default");

    SetCursor (GetSystemCursor (IDC_ARROW));
    SetCursorPos (g_rcScr.right >> 1, g_rcScr.bottom >> 1);

    TerminateMgEtc ();
    return TRUE;

failure:
    TerminateLWEvent ();
failure1:
    TerminateGAL ();
    return FALSE;
}

void GUIAPI TerminateGUI (int rcByGUI)
{
    if (rcByGUI >= 0) {
        pthread_join (__mg_timer, NULL);
    }
    else {
#ifndef __ECOS            
	// cyli fix
//        pthread_kill_other_threads_np ();
#endif
    }
   
    DestroyMsgQueue (&__mg_dsk_msgs);

    DestroyFreeQMSGList ();

    TerminateTimer ();
    TerminateDesktop ();
    TerminateAccel ();
    TerminateControlClass ();
    TerminateMenu ();
#ifdef _CURSOR_SUPPORT
    TerminateCursor ();
#endif
    TerminateFixStr ();
    TerminateLWEvent ();

#ifdef _USE_NEWGAL
    TerminateScreenDC ();
#endif

    TerminateGDI ();
    TerminateGAL ();
    TerminateMisc ();

    // Restore original termio
    // tcsetattr (0, TCSAFLUSH, &savedtermio);
}

