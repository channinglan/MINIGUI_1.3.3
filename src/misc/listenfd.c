/*
** $Id: listenfd.c,v 1.6 2003/09/04 03:46:47 weiym Exp $
**
** listen.c: routines for listen fd.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/03/28
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
#include <string.h>
#include <sys/poll.h>

#include "common.h"
#include "minigui.h"
#include "ourhdr.h"

LISTEN_FD mg_listen_fds [MAX_NR_LISTEN_FD];
static fd_set _wfdset, _efdset;
fd_set mg_rfdset;
fd_set* mg_wfdset = NULL;
fd_set* mg_efdset = NULL;
int mg_maxfd;

/* Register/Unregister a listen fd to MiniGUI.
 * When there is a read event on this fd, MiniGUI
 * will post a MSG_FDEVENT message with wParam being equal to
 * MAKELONG (fd, type) to target window.
 */

/* Return TRUE if all OK, and FALSE on error. */
BOOL GUIAPI RegisterListenFD (int fd, int type, HWND hwnd, void* context)
{
    int i = 0;
    for (i = 0; i < MAX_NR_LISTEN_FD; i++) {
        if (mg_listen_fds [i].fd == 0) {
            mg_listen_fds [i].fd = fd;
            mg_listen_fds [i].hwnd = hwnd;
            mg_listen_fds [i].type = type;
            mg_listen_fds [i].context = context;
            switch (type) {
            case POLLIN:
                FD_SET (fd, &mg_rfdset);
                break;

            case POLLOUT:
                if (mg_wfdset == NULL) {
                    mg_wfdset = &_wfdset;
                    FD_ZERO (mg_wfdset);
                }
                FD_SET (fd, mg_wfdset);
                break;

            case POLLERR:
                if (mg_efdset == NULL) {
                    mg_efdset = &_efdset;
                    FD_ZERO (mg_efdset);
                }
                FD_SET (fd, mg_efdset);
                break;
            }

            if (mg_maxfd < fd)
                mg_maxfd = fd;
            return TRUE;
        }
    }

    return FALSE;
}

/* Return TRUE if all OK, and FALSE on error. */
BOOL GUIAPI UnregisterListenFD (int fd)
{
    int i = 0;
    for (i = 0; i < MAX_NR_LISTEN_FD; i++) {
        if (mg_listen_fds [i].fd == fd) {
            mg_listen_fds [i].fd = 0;
            switch (mg_listen_fds [i].type) {
            case POLLIN:
                FD_CLR (fd, &mg_rfdset);
                break;

            case POLLOUT:
                if (mg_wfdset == NULL)
                    return FALSE;
                FD_CLR (fd, mg_wfdset);
                break;

            case POLLERR:
                if (mg_efdset == NULL)
                    return FALSE;
                FD_CLR (fd, mg_efdset);
                break;
            }

            return TRUE;
        }
    }

    return FALSE;
}

