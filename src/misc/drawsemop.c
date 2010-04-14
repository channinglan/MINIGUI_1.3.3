/*
** $Id: drawsemop.c,v 1.14 2003/09/04 03:46:47 weiym Exp $
**
** drawsemop.c: operations for drawing semaphore.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/12/31
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
#include "drawsemop.h"

#define SEM_TIMEOUT     500

void unlock_draw_sem (void)
{
    struct sembuf sb;

again:
    sb.sem_num = 0;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;

    if (semop (SHAREDRES_SEMID, &sb, 1) == -1) {
        if (errno == EINTR) {
            goto again;
        }
    }
}

void lock_draw_sem (void)
{
    struct sembuf sb;

again:
    sb.sem_num = 0;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;

    if (semop (SHAREDRES_SEMID, &sb, 1) == -1) {
        if (errno == EINTR) {
            goto again;
        }
    }
}

#ifdef _CURSOR_SUPPORT
void inc_hidecursor_sem (void)
{
    struct sembuf sb;

again:
    sb.sem_num = 2;
    sb.sem_op = 1;
    sb.sem_flg = 0;

    if (semop (SHAREDRES_SEMID, &sb, 1) == -1) {
        if (errno == EINTR) {
            goto again;
        }
    }
}

#endif /* _CURSOR_SUPPORT */

