/*
** $Id: server.c,v 1.49 2003/09/04 06:02:53 weiym Exp $
** 
** server.c: routines for server.
** 
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/12/20
**
** NOTE: The idea comes from sample code in APUE.
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

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/poll.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "internals.h"
#include "gal.h"
#include "ial.h"
#include "cursor.h"
#include "event.h"
#include "menu.h"

#include "ourhdr.h"
#include "sockio.h"
#include "client.h"
#include "server.h"
#include "sharedres.h"
#include "drawsemop.h"

BLOCKHEAP __mg_free_spare_rect_list;

unsigned int __mg_timer_counter = 0;

ON_NEW_DEL_CLIENT OnNewDelClient = NULL;

static struct sigaction old_alarm_handler;
static struct itimerval old_timer;

#if 1
static void itimersig_handler (int v)
{
    SHAREDRES_TIMER_COUNTER += 1;
}

static BOOL InstallTimer (void)
{
    struct itimerval timerv;
    struct sigaction siga;
    
    siga.sa_handler = itimersig_handler;
    siga.sa_flags = 0;
    
    memset (&siga.sa_mask, 0, sizeof (sigset_t));

    sigaction (SIGALRM, &siga, &old_alarm_handler);

    timerv.it_interval.tv_sec = 0;
    timerv.it_interval.tv_usec = 10000;     // 10 ms
    timerv.it_value = timerv.it_interval;

    if (setitimer (ITIMER_REAL, &timerv, &old_timer)) {
#ifdef _DEBUG
        log_sys ("setitimer call failed!\n");
#else
        return FALSE;
#endif
    }

    return TRUE;
}

static BOOL UninstallTimer (void)
{
    if (setitimer (ITIMER_REAL, &old_timer, 0) == -1) {
#ifdef _DEBUG
        log_sys ("setitimer call failed!\n");
#else
        return FALSE;
#endif
    }

    if (sigaction (SIGALRM, &old_alarm_handler, NULL) == -1) {
#ifdef _DEBUG
        log_sys ("sigaction call failed!\n");
#else
        return FALSE;
#endif
    }

    return TRUE;
}

#else
static clock_t start_tick;
static struct tms my_tms;
static struct timeval timev;
static BOOL InstallTimer (void)
{
    timev.tv_sec = 0;
    timev.tv_usec = 100000;     // 100 ms

    start_tick = times (&my_tms);
    return TRUE;
}

static BOOL UninstallTimer (void)
{
    return TRUE;
}

#endif

static BOOL QueueMessage (PMSGQUEUE msg_que, PMSG msg)
{
    if ((msg_que->writepos + 1) % msg_que->len == msg_que->readpos) {
        return FALSE;
    }

    // Write the data and advance write pointer */
    msg_que->msg [msg_que->writepos] = *msg;

    msg_que->writepos++;
    if (msg_que->writepos >= msg_que->len) msg_que->writepos = 0;

    msg_que->dwState |= QS_POSTMSG;
    return TRUE;
}

static MG_Client* CheckClientMousePos (int* x, int* y)
{
    MG_Client* mousein = NULL;
    MG_Client* client;

    if (SHAREDRES_TOPMOST_LAYER == 0)
        return NULL;

    if (mgTopmostLayer && PtInRect (SHAREDRES_CLI_SCR_RC, *x, *y)) {
        client = mgTopmostLayer->cli_head;
        while (client) {
            if (PtInRect (&client->rc, *x, *y)) {
                mousein = client;
                break;
            }

            client = client->next;
        }
    }

    if (SHAREDRES_CLI_SCR_LX > *x) *x = SHAREDRES_CLI_SCR_LX;
    if (SHAREDRES_CLI_SCR_TY > *y) *y = SHAREDRES_CLI_SCR_TY;
    if (SHAREDRES_CLI_SCR_RX < *x) *x = SHAREDRES_CLI_SCR_RX;
    if (SHAREDRES_CLI_SCR_BY < *y) *y = SHAREDRES_CLI_SCR_BY;

    return mousein;
}

static SRVEVTHOOK srv_evt_hook = NULL;

SRVEVTHOOK GUIAPI SetServerEventHook (SRVEVTHOOK SrvEvtHook)
{
    SRVEVTHOOK old_hook = srv_evt_hook;

    srv_evt_hook = SrvEvtHook;

    return old_hook;
}

static BOOL should_gain_focus (HWND hwnd)
{
    if (hwnd) {
        if (hwnd == __mg_ime_wnd)
            return FALSE;
        if (GetWindowStyle (hwnd) & WS_DISABLED)
            return FALSE;
        if (GetWindowExStyle (hwnd) & WS_EX_TOOLWINDOW)
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

static void ParseEvent (PMSGQUEUE msg_que, int event)
{
    LWEVENT lwe;
    PMOUSEEVENT me;
    PKEYEVENT ke;
    MSG Msg;

    ke = &(lwe.data.ke);
    me = &(lwe.data.me);
    me->x = 0; me->y = 0;
    Msg.hwnd = HWND_DESKTOP;
    Msg.wParam = 0;
    Msg.lParam = 0;

    lwe.status = 0L;

    if (!GetLWEvent (event, &lwe))
        return;

    Msg.time = __mg_timer_counter;
    if (lwe.type == LWETYPE_TIMEOUT) {
        Msg.message = MSG_TIMEOUT;
        Msg.wParam = (WPARAM)lwe.count;
        Msg.lParam = 0;

        Send2Client (&Msg, CLIENT_ACTIVE);
        QueueMessage (msg_que, &Msg);
    }
    else if (lwe.type == LWETYPE_KEY) {
        Msg.wParam = ke->scancode;
        Msg.lParam = ke->status;
        if(ke->event == KE_KEYDOWN){
            Msg.message = MSG_KEYDOWN;
        }
        else if(ke->event == KE_KEYUP) {
            Msg.message = MSG_KEYUP;
        }

        if (!(srv_evt_hook && srv_evt_hook (&Msg))) {
            if (__mg_active_mainwnd || __mg_ime_wnd || __mg_ptmi ||
                (GetShiftKeyStatus () & KS_ALT))
                QueueMessage (msg_que, &Msg);
            else {
                Send2Client (&Msg, CLIENT_ACTIVE);
            }
        }
    }
    else if (lwe.type == LWETYPE_MOUSE) {
        int cli_x = me->x, cli_y = me->y;
        static MG_Client* down_client = NULL;
        MG_Client* target_client;
        MG_Client* cur_client = CheckClientMousePos (&cli_x, &cli_y);
        HWND under_pointer = (HWND)GetMainWindowPtrUnderPoint (me->x, me->y);

        Msg.wParam = me->status;
        switch (me->event) {
        case ME_MOVED:
            Msg.message = MSG_MOUSEMOVE;
            if (cur_client == NULL)
                SetCursor (GetSystemCursor (IDC_ARROW));
            break;
        case ME_LEFTDOWN:
            Msg.message = MSG_LBUTTONDOWN;
            if (should_gain_focus (under_pointer) || cur_client)
                set_active_client (cur_client);
            down_client = cur_client;
            break;
        case ME_LEFTUP:
            Msg.message = MSG_LBUTTONUP;
            break;
        case ME_LEFTDBLCLICK:
            Msg.message = MSG_LBUTTONDBLCLK;
            break;
        case ME_RIGHTDOWN:
            Msg.message = MSG_RBUTTONDOWN;
            if (should_gain_focus (under_pointer) || cur_client)
                set_active_client (cur_client);
            down_client = cur_client;
            break;
        case ME_RIGHTUP:
            Msg.message = MSG_RBUTTONUP;
            break;
        case ME_RIGHTDBLCLICK:
            Msg.message = MSG_RBUTTONDBLCLK;
            break;
        }

        if (down_client && (down_client->fd != -1))
            target_client = down_client;
        else
            target_client = cur_client;

        if (me->event == ME_LEFTUP || me->event == ME_RIGHTUP)
            down_client = NULL;

        Msg.lParam = MAKELONG (me->x, me->y);
        if (!(srv_evt_hook && srv_evt_hook (&Msg))) {
            QueueMessage (msg_que, &Msg);
            if (!__mg_capture_wnd && (SHAREDRES_TOPMOST_LAYER != 0) && target_client) {
                Msg.lParam = MAKELONG (cli_x, cli_y);
                Send2Client (&Msg, target_client - mgClients);
            }
        }
    }
}

static int listenfd;
static int maxi;
BOOL GUIAPI ServerStartup (void)
{
#ifdef _DEBUG
    log_open ("mginit", LOG_PID, LOG_USER);
#endif

    InitFreeClipRectList (&__mg_free_spare_rect_list, SIZE_SPARERECTHEAP);

    /* obtain fd to listen for client requests on */
    if ( (listenfd = serv_listen (CS_PATH)) < 0)
        return FALSE;

    FD_ZERO (&mg_rfdset);
    FD_SET (listenfd, &mg_rfdset);
    mg_maxfd = listenfd;
    maxi = -1;

    InstallTimer ();

    return TRUE;
}

void ServerCleanup (void)
{
    UninstallTimer ();

    unlink (CS_PATH);

    DestroyFreeClipRectList (&__mg_free_spare_rect_list);
}

void remove_client (int cli, int clifd)
{
#ifdef _DEBUG
    log_msg ("client closed: uid %d, fd %d", mgClients [cli].uid, clifd);
#endif
    client_del (cli);    /* client has closed conn */
    FD_CLR (clifd, &mg_rfdset);
    close (clifd);
}

BOOL IdleHandler4Server (PMSGQUEUE msg_queue)
{
    int    i, n, clifd, nread;
    pid_t  pid;
    uid_t  uid;
    struct timeval sel_timeout = {0, 0};
    fd_set rset, wset, eset;
    fd_set* wsetptr = NULL;
    fd_set* esetptr = NULL;

    if (__mg_timer_counter != SHAREDRES_TIMER_COUNTER) {
        __mg_timer_counter = SHAREDRES_TIMER_COUNTER;
        __mg_dsk_msgs.dwState |= QS_DESKTIMER;
    }

    rset = mg_rfdset;        /* rset gets modified each time around */
    if (mg_wfdset) {
        wset = *mg_wfdset;
        wsetptr = &wset;
    }
    if (mg_efdset) {
        eset = *mg_efdset;
        esetptr = &eset;
    }

#ifdef _CURSOR_SUPPORT
    /* if the cursor has been hide by GDI function of clients
     * this call will show the cursor
     */
    ReShowCursor ();
#endif

    if ( (n = IAL_WaitEvent (IAL_MOUSEEVENT | IAL_KEYEVENT, 
                mg_maxfd, &rset, wsetptr, esetptr, msg_queue?NULL:(&sel_timeout))) < 0) {

        /* It is time to check event again. */
        if (errno == EINTR) {
            if (msg_queue)
                ParseEvent (msg_queue, 0);
            return FALSE;
        }
#ifdef _DEBUG
        log_msg ("select error on server");
#endif
    }
    else if (msg_queue == NULL)
        return (n > 0);

    /* handle intput event (mouse/touch-screen or keyboard) */
    if (n & IAL_MOUSEEVENT) ParseEvent (msg_queue, IAL_MOUSEEVENT);
    if (n & IAL_KEYEVENT) ParseEvent (msg_queue, IAL_KEYEVENT);
    if (n == 0) ParseEvent (msg_queue, 0);

    if (FD_ISSET (listenfd, &rset)) {
        /* accept new client request */
        if ( (clifd = serv_accept (listenfd, &pid, &uid)) < 0) {
#ifdef _DEBUG
            log_msg ("serv_accept error: %d", clifd);
#endif
            return TRUE;
        }

        if ((i = client_add (clifd, pid, uid)) == -1) {
            /* can not accept this client */
            close (clifd);
            return TRUE;
        }
        if (OnNewDelClient) OnNewDelClient (LCO_NEW_CLIENT, i);

        FD_SET (clifd, &mg_rfdset);
        if (clifd > mg_maxfd)
            mg_maxfd = clifd;  /* max fd for select() */
        if (i > maxi)
            maxi = i;       /* max index in client[] array */
#ifdef _DEBUG
        log_msg ("new connection: uid %d, fd %d", uid, clifd);
#endif
        return TRUE;
    }

    for (i = 0; i <= maxi; i++) {    /* go through client[] array */
        if ( (clifd = mgClients [i].fd) < 0)
            continue;
        if (FD_ISSET (clifd, &rset)) {
            int req_id;

            /* read request id from client */
            if ( (nread = sock_read (clifd, &req_id, sizeof (int))) == SOCKERR_IO) {
#ifdef _DEBUG
                log_msg ("server: read error on fd %d", clifd);
#endif
                if (OnNewDelClient) OnNewDelClient (LCO_DEL_CLIENT, i);
                remove_client (i, clifd);
            }
            else if (nread == SOCKERR_CLOSED) {
                if (OnNewDelClient) OnNewDelClient (LCO_DEL_CLIENT, i);
                remove_client (i, clifd);
            } else            /* process client's rquest */
                handle_request (clifd, req_id, i);
        }
    }

    /* go through registered listen fds */
    for (i = 0; i < MAX_NR_LISTEN_FD; i++) {
        MSG Msg;

        Msg.message = MSG_FDEVENT;

        if (mg_listen_fds [i].fd) {
            fd_set* temp = NULL;
            int type = mg_listen_fds [i].type;

            switch (type) {
            case POLLIN:
                temp = &rset;
                break;
            case POLLOUT:
                temp = wsetptr;
                break;
            case POLLERR:
                temp = esetptr;
                break;
            }

            if (temp && FD_ISSET (mg_listen_fds [i].fd, temp)) {
                Msg.hwnd = (HWND)mg_listen_fds [i].hwnd;
                Msg.wParam = MAKELONG (mg_listen_fds [i].fd, type);
                Msg.lParam = (LPARAM)mg_listen_fds [i].context;
                QueueMessage (msg_queue, &Msg);
            }
        }
    }

    return TRUE;
}

