/*
** $Id: sockio.c,v 1.8 2003/09/04 03:46:47 weiym Exp $
**
** sockio.c: routines for socket i/o.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2002 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/12/26
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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "minigui.h"
#include "ourhdr.h"
#include "sharedres.h"
#include "sockio.h"

int sock_write_t (int fd, const void* buff, int count, unsigned int timeout)
{
    const void* pts = buff;
    int status = 0, n;
    unsigned int start_tick = SHAREDRES_TIMER_COUNTER;

    if (count < 0) return SOCKERR_OK;

    while (status != count) {
        n = write (fd, pts + status, count - status);
        if (n < 0) {
            if (errno == EPIPE)
                return SOCKERR_CLOSED;
            else if (errno == EINTR) {
                if (timeout && (SHAREDRES_TIMER_COUNTER > start_tick + timeout))
                    return SOCKERR_TIMEOUT;
                continue;
            }
            else
                return SOCKERR_IO;
        }
        status += n;
    }

    return status;
}

int sock_read_t (int fd, void* buff, int count, unsigned int timeout)
{
    void* pts = buff;
    int status = 0, n;
    unsigned int start_tick = SHAREDRES_TIMER_COUNTER;

    if (count <= 0) return SOCKERR_OK;

    while (status != count) {
        n = read (fd, pts + status, count - status);

        if (n < 0) {
            if (errno == EINTR) {
                if (timeout && (SHAREDRES_TIMER_COUNTER > start_tick + timeout))
                    return SOCKERR_TIMEOUT;
                continue;
            }
            else
                return SOCKERR_IO;
        }

        if (n == 0)
            return SOCKERR_CLOSED;

        status += n;
    }

    return status;
}

