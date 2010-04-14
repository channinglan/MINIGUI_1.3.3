/*
** $Id: client.c,v 1.32 2003/09/04 02:40:35 weiym Exp $
** 
** client.c: routines for client.
** 
** Copyright (C) 2000 ~ 2002 Wei Yongming.
** Copyright (C) 2003 Feynman Software.
**
** Current maintainer: Wei Yongming.
**
** NOTE: The idea comes from sample code in APUE.
** Thank Mr. Richard Stevens for his perfect work.
**
** Create date: 2000/12/20
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

#include <sys/types.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "gal.h"
#include "cliprect.h"
#include "internals.h"
#include "timer.h"
#include "ourhdr.h"
#include "client.h"
#include "server.h"
#include "sockio.h"
#include "sharedres.h"
#include "drawsemop.h"

static int conn_fd = -1;         /* connected socket */

static struct timeval my_timeout;
void set_select_timeout (unsigned int usec)
{
    if (usec > 0 && usec < USEC_1S) {
        my_timeout.tv_sec = 0;
        my_timeout.tv_usec = MAX (usec/10, 1000);
    }
    else {
        my_timeout.tv_sec = 1;
        my_timeout.tv_usec = 0;
    }
}

BOOL ClientStartup (void)
{
    struct sigaction siga;

    /* connect to server */
    if ( (conn_fd = cli_conn (CS_PATH, 'a')) < 0)
        return FALSE;

    FD_ZERO (&mg_rfdset);
    FD_SET (conn_fd, &mg_rfdset);
    mg_maxfd = conn_fd;

    set_select_timeout (0);

    /* ignore the SIGPIPE signal */
    siga.sa_handler = SIG_IGN;
    siga.sa_flags = 0;
    memset (&siga.sa_mask, 0, sizeof (sigset_t));
    sigaction (SIGPIPE, &siga, NULL);

    return TRUE;
}

void ClientCleanup (void)
{
    close (conn_fd);
    conn_fd = -1;
}

static BOOL QueueMessage (PMSGQUEUE msg_que, PMSG msg)
{
    if ((msg_que->writepos + 1) % msg_que->len == msg_que->readpos) {
        return FALSE;
    }

    /* Write the data and advance write pointer */
    msg_que->msg [msg_que->writepos] = *msg;

    msg_que->writepos++;
    if (msg_que->writepos >= msg_que->len) msg_que->writepos = 0;

    msg_que->dwState |= QS_POSTMSG;
    return TRUE;
}

int get_sock_fd2srv (void)
{
    return conn_fd;
}

int cli_request (PREQUEST request, void* result, int len_rslt)
{
    int n;
    MSG msg;

    if ((n = sock_write (conn_fd, &request->id, sizeof (int))) == SOCKERR_IO)
        return -1;
    else if (n == SOCKERR_CLOSED)
        exit (-1);

    if ((n = sock_write (conn_fd, &request->len_data, sizeof (size_t))) == SOCKERR_IO)
        return -1;
    else if (n == SOCKERR_CLOSED)
        exit (-1);

    if ((n = sock_write (conn_fd, request->data, request->len_data)) == SOCKERR_IO)
        return -1;
    else if (n == SOCKERR_CLOSED)
        exit (-1);

    if (result == NULL || len_rslt == 0)
        return 0;

    do {
        if ((n = sock_read (conn_fd, &msg, sizeof (MSG))) == SOCKERR_IO)
            return -1;
        else if (n == SOCKERR_CLOSED)
            exit (-1);

        if (msg.hwnd == HWND_INVALID)
            break;
        else
            QueueMessage (&__mg_dsk_msgs, &msg);
    } while (TRUE);

    if ((n = sock_read (conn_fd, result, len_rslt)) == SOCKERR_IO)
        return -1;
    else if (n == SOCKERR_CLOSED)
        exit (-1);

    return 0;
}

static void check_live (void)
{
    REQUEST req;

    if (__mg_timer_counter != SHAREDRES_TIMER_COUNTER) {
        __mg_dsk_msgs.dwState |= QS_DESKTIMER;
        __mg_timer_counter = SHAREDRES_TIMER_COUNTER;
    }

    /* Tell server that I am live */
    req.id = REQID_IAMLIVE;
    req.data = &__mg_timer_counter;
    req.len_data = sizeof (unsigned int);
    cli_request (&req, NULL, 0);
}

BOOL IdleHandler4Client (PMSGQUEUE msg_que)
{
    fd_set rset, wset, eset;
    fd_set* wsetptr = NULL;
    fd_set* esetptr = NULL;
    int i, n, nread;
    struct timeval sel_timeout;
    MSG Msg;

    check_live ();

    rset = mg_rfdset;        /* rset gets modified each time around */
    if (mg_wfdset) {
        wset = *mg_wfdset;
        wsetptr = &wset;
    }
    if (mg_efdset) {
        eset = *mg_efdset;
        esetptr = &eset;
    }

    if (msg_que)
        sel_timeout = my_timeout;
    else {  /* check fd only: for HavePendingMessage function */
        sel_timeout.tv_sec = 0;
        sel_timeout.tv_usec = 0;
    }

    if ( (n = select (mg_maxfd + 1, &rset, wsetptr, esetptr, &sel_timeout)) < 0) {
        if (errno == EINTR) {
            /* it is time to check message again. */
            return FALSE;
        }
        err_sys ("select error on client");
    }
    else if (n == 0) {
        check_live ();
        return FALSE;
    }
    else if (msg_que == NULL) /* check fd only: for HavePendingMessage function. */
        return TRUE;

    if (FD_ISSET (conn_fd, &rset)) {
        if ( (nread = sock_read (conn_fd, &Msg, sizeof (MSG))) < 0)
            err_sys ("client: read error on fd %d", conn_fd);
        else if (nread == 0) {
            err_msg ("client: server closed");
            close (conn_fd);
        }
        else {           /* process event from server */
            Msg.hwnd = HWND_DESKTOP;
            QueueMessage (msg_que, &Msg);
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
                QueueMessage (msg_que, &Msg);
            }
        }
    }

    check_live ();
    return TRUE;
}

