/*
** $Id: ads.c,v 1.15 2003/11/21 12:15:37 weiym Exp $
**
** ads.c: Low Level Input Engine for ADS Graphics Client.
** 
** Copyright (C) 2000, Wei Yongming 
** Copyright (C) 2000, 2001, BluePoint Software.
**
** Created by Wei Yongming, 2000/08/24
**
** After some hard work, this engine now runs well.
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
#include <fcntl.h>
#include "common.h"

#ifdef _ADS_IAL

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "misc.h"
#include "ads_internal.h"
#include "ial.h"
#include "ads.h"

static int ts;
static int mousex = 0;
static int mousey = 0;
static POS pos;

/************************  Low Level Input Operations **********************/
/*
 * Mouse operations -- Event
 */
static int mouse_update(void)
{
	return 1;
}

static void mouse_getxy(int* x, int* y)
{
	*x = mousex;
	*y = mousey;
}

static int mouse_getbutton(void)
{
	return pos.b;
}

static int keyboard_update(void)
{
	return 0;
}

static const char * keyboard_getstate(void)
{
	return NULL;
}

#ifdef _LITE_VERSION 
static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
{
	fd_set rfds;
	int	e;

    if (!in) {
        in = &rfds;
        FD_ZERO (in);
    }

	if (which & IAL_MOUSEEVENT) {
		FD_SET (ts, in);
        if (ts > maxfd) maxfd = ts;
	}

	e = select (maxfd + 1, in, out, except, timeout) ;

	if (e > 0) { 
		if (ts >= 0 && FD_ISSET (ts, in))
		{
            FD_CLR (ts, in);
			read (ts, &pos, sizeof (POS));	
			if ( pos.x !=-1 && pos.y !=-1) {
				mousex = pos.x;
				mousey = pos.y;
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
	struct pollfd ufd;
	if ( (which & IAL_MOUSEEVENT) == IAL_MOUSEEVENT)
	{
		ufd.fd     = ts;
		ufd.events = POLLIN; 
		if ( poll (&ufd, 1, timeout) > 0)
		{
			read (ts, &pos, sizeof(POS));	
			return IAL_MOUSEEVENT;
		}
	}
	return 0;
}
#endif

BOOL InitADSInput (INPUT* input, const char* mdev, const char* mtype)
{
	int i;
	
	ts = open ("/dev/ts", O_RDONLY);
    if ( ts < 0 ) {
        fprintf (stderr, "IAL: Can not open touch screen!\n");
        return FALSE;
    }

    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;

    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_getstate;
    input->set_leds = NULL;

    input->wait_event = wait_event;
	mousex = 0;
	mousey = 0;
    return TRUE;
}

void TermADSInput (void)
{
	if ( ts >= 0 )
		close(ts);
}

#endif /* _ADS_IAL */

