/*
** $Id: dummy.c,v 1.7 2003/11/21 12:15:37 weiym Exp $
**
** dummy.c: The dummy IAL engine.
** 
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Created by Wei Yongming, 2001/09/13
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#ifdef _DUMMY_IAL

#include <sys/select.h>

#include "ial.h"
#include "dummy.h"

static int mouse_fd = -1;
static int mouse_x, mouse_y, mouse_button;

typedef struct tagPOS
{
	short x;
	short y;
	short b;
} POS;

/************************  Low Level Input Operations **********************/
/*
 * Mouse operations -- Event
 */
static int mouse_update(void)
{
	return 1;
}

static void mouse_getxy (int* x, int* y)
{
	*x = mouse_x;
    *y = mouse_y;
}

static int mouse_getbutton(void)
{
	return mouse_button;
}

static int keyboard_update(void)
{
	return 0;
}

static const char * keyboard_get_state (void)
{
	return NULL;
}

#ifdef _LITE_VERSION 
static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
{
    POS pos;
	fd_set rfds;
	int	e;

    if (!in) {
        in = &rfds;
        FD_ZERO (in);
    }

	if (which & IAL_MOUSEEVENT && mouse_fd >= 0) {
		FD_SET (mouse_fd, in);
        if (mouse_fd > maxfd) maxfd = mouse_fd;
	}

	e = select (maxfd + 1, in, out, except, timeout) ;

	if (e > 0) { 
		if (mouse_fd >= 0 && FD_ISSET (mouse_fd, in)) {
            FD_CLR (mouse_fd, in);
			read (mouse_fd, &pos, sizeof (POS));
			if (pos.x != -1 && pos.y != -1) {
				mouse_x = pos.x;
				mouse_y = pos.y;
			}
			pos.b = ( pos.b > 0 ? 4:0);
			return IAL_MOUSEEVENT;
		}

	} else if (e < 0) {
		return -1;
	}
	return 0;
}
#else
static int wait_event (int which, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
{
	return 0;
}
#endif

BOOL InitDummyInput (INPUT* input, const char* mdev, const char* mtype)
{
#if 0
    /* open input device(s) here */
	mouse_fd = open ("/dev/mouse", O_RDONLY);
    if (mouse_fd < 0) {
        fprintf (stderr, "IAL: Can not open mouse-like device!\n");
        return FALSE;
    }
#endif

    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;

    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_get_state;
    input->set_leds = NULL;

    input->wait_event = wait_event;
	mouse_x = 0;
	mouse_y = 0;
	mouse_button= 0;
    return TRUE;
}

void TermDummyInput (void)
{
	if (mouse_fd >= 0)
		close (mouse_fd);
}

#endif /* _DUMMY_IAL */

