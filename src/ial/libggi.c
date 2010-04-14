/*
** $Id: libggi.c,v 1.13 2003/09/04 03:38:25 weiym Exp $
**
** libggi.c: Low Level Graphics Engine based on LibGGI
**
** Written by Wei Yongming, 2000/06/11
**
** Create date: 2000/06/11
**
** 2000.6.16 gengyue. 
**    To complete  the code. use some pieces code from /degas/lib/svgalib
**    /vga/ggi_input.c. it must add this file to project. some problems will occur 
**    when compiles it, go back to locate the bugs.
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ggi/ggi.h>
#ifdef HAVE_LINUX_KEYBOARD_H
  #include <linux/keyboard.h>
#endif

#include "common.h"
#include "misc.h"
#include "ial.h"
#include "gal.h"
#include "libggi.h"

#include "svgaggi_internal.h"

#define VIS _ggigsw_visual

int _ggigsw_async = 0;
int _ggigsw_but2key = 0;
ggi_visual_t _ggigsw_visual;

/************************  Low Level Input Operations **********************/

/****************************************************************************
   SVGAlib wrapper for LibGGI - mouse handling

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

static int dowrap = 0;
static int scalefactor = 1;

static int buttons = 0;
static int cur_x = 0;
static int cur_y = 0;
static int cur_z = 0;
static int cur_rx = 0;
static int cur_ry = 0;
static int cur_rz = 0;

static int xmin = 0;
static int xmax = 32767;
static int ymin = 0;
static int ymax = 32767;
static int zmin = 0;
static int zmax = 32767;
static int rxmin = 0;
static int rxmax = 32767;
static int rymin = 0;
static int rymax = 32767;
static int rzmin = 0;
static int rzmax = 32767;

static void
default_mousehandler(int button, int dx, int dy, int dz, int drx, int dry, int drz)
{
	cur_x += dx;
	cur_y += dy;
	cur_z += dz;
	buttons = button;
	if (cur_x / scalefactor > xmax) {
		if (dowrap & MOUSE_WRAPX) cur_x -= (xmax - xmin) * scalefactor;
		else cur_x = xmax * scalefactor;
	}
	if (cur_x / scalefactor < xmin) {
		if (dowrap & MOUSE_WRAPY) cur_x += (xmax - xmin) * scalefactor;
		else cur_x = xmin * scalefactor;
	}
	if (cur_y / scalefactor > ymax) {
		if (dowrap & MOUSE_WRAPX) cur_y -= (ymax - ymin) * scalefactor;
		else cur_y = ymax * scalefactor;
	}
	if (cur_y / scalefactor < ymin) {
		if (dowrap & MOUSE_WRAPY) cur_y += (ymax - ymin) * scalefactor;
		else cur_y = ymin * scalefactor;
	}
	if (cur_z / scalefactor > zmax) {
		if (dowrap & MOUSE_WRAPZ) cur_z -= (zmax - zmin) * scalefactor;
		else cur_z = zmax * scalefactor;
	}
	if (cur_z / scalefactor < zmin) {
		if (dowrap & MOUSE_WRAPZ) cur_z += (zmax - zmin) * scalefactor;
		else cur_z = zmin * scalefactor;
	}
	switch (dowrap & MOUSE_ROT_COORDS) {
	case MOUSE_ROT_INFINITESIMAL:
		cur_rx = drx;
		cur_ry = dry;
		cur_rz = drz;
		break;
	case MOUSE_ROT_RX_RY_RZ:
		cur_rx += drx;
		cur_ry += dry;
		cur_rz += drz;
		break;
	}
	if (cur_rx / scalefactor > rxmax) {
		if (dowrap & MOUSE_WRAPRX) {
			cur_rx -= (rxmax - rxmin) * scalefactor;
		} else {
			cur_rx = rxmax * scalefactor;
		}
	}
	if (cur_rx / scalefactor < rxmin) {
		if (dowrap & MOUSE_WRAPRX) {
			cur_rx += (rxmax - rxmin) * scalefactor;
		} else {
			cur_rx = rxmin * scalefactor;
		}
	}
	if (cur_ry / scalefactor > rymax) {
		if (dowrap & MOUSE_WRAPRY) {
			cur_ry -= (rymax - rymin) * scalefactor;
		} else {
			cur_ry = rymax * scalefactor;
		}
	}
	if (cur_ry / scalefactor < rymin) {
		if (dowrap & MOUSE_WRAPRY) {
			cur_ry += (rymax - rymin) * scalefactor;
		} else {
			cur_ry = rymin * scalefactor;
		}
	}
	if (cur_rz / scalefactor > rzmax) {
		if (dowrap & MOUSE_WRAPRZ) {
			cur_rz -= (rzmax - rzmin) * scalefactor;
		} else {
			cur_rz = rzmax * scalefactor;
		}
	}
	if (cur_rz / scalefactor < rzmin) {
		if (dowrap & MOUSE_WRAPRZ) {
			cur_rz += (rzmax - rzmin) * scalefactor;
		} else {
			cur_rz = rzmin * scalefactor;
		}
	}
}

void (*_ggigsw_mousehandler) (int, int, int, int, int, int, int)
	= default_mousehandler;

static int mouse_update(void)
{
	return _ggigsw_getmouseevent(0);
}

static void mouse_setposition(int x, int y)
{
	cur_x = x;
	cur_y = y;
}

static void mouse_setrange(int minx, int miny,int maxx,int maxy)
{
	xmin = minx;
	xmax = maxx;
	ymin = miny;
	ymax = maxy;
}

static void mouse_getxy (int* x, int* y)
{
    *x = cur_x / scalefactor;
	*y = cur_y / scalefactor;
}

static int mouse_getbutton(void)
{
	return buttons;
}

/*
********************** Not currently used mouse functions *******************

static void mouse_setposition_6d(int x, int y, int z,
		     int rx, int ry, int rz, int dim_mask)
{
	if (dim_mask & MOUSE_XDIM) cur_x = x * scalefactor;
	if (dim_mask & MOUSE_YDIM) cur_y = y * scalefactor;
	if (dim_mask & MOUSE_ZDIM) cur_z = z * scalefactor;
	if (dim_mask & MOUSE_RXDIM) cur_rx = rx * scalefactor;
	if (dim_mask & MOUSE_RYDIM) cur_ry = ry * scalefactor;
	if (dim_mask & MOUSE_RZDIM) cur_rz = rz * scalefactor;
}

static void mouse_setrange_6d(int x1, int x2, int y1, int y2, int z1, int z2,
		  int rx1, int rx2, int ry1, int ry2, int rz1, int rz2,
		  int dim_mask)
{
	if (dim_mask & MOUSE_XDIM) {
		xmin = x1;
		xmax = x2;
	}
	if (dim_mask & MOUSE_YDIM) {
		ymin = y1;
		ymax = y2;
	}
	if (dim_mask & MOUSE_ZDIM) {
		zmin = z1;
		zmax = z2;
	}
	if (dim_mask & MOUSE_RXDIM) {
		rxmin = rx1;
		rxmax = rx2;
	}
	if (dim_mask & MOUSE_RYDIM) {
		rymin = ry1;
		rymax = ry2;
	}
	if (dim_mask & MOUSE_RZDIM) {
		rzmin = rz1;
		rzmax = rz2;
	}
}

static void mouse_setscale(int s)
{
	if (scalefactor > 0) {
		cur_x = (cur_x*s)/scalefactor;
		cur_y = (cur_y*s)/scalefactor;
		cur_z = (cur_z*s)/scalefactor;
		cur_rx = (cur_rx*s)/scalefactor;
		cur_ry = (cur_ry*s)/scalefactor;
		cur_rz = (cur_rz*s)/scalefactor;
		scalefactor = s;
	}
}

static void mouse_setwrap(int state)
{
	dowrap = state;
}

static void mouse_getposition_6d(int *x, int *y, int *z,
		     int *rx, int *ry, int *rz)
{
	if (x) *x = cur_x / scalefactor;
	if (y) *y = cur_y / scalefactor;
	if (z) *z = cur_z / scalefactor;
	if (rx) *rx = cur_rx / scalefactor;
	if (ry) *ry = cur_ry / scalefactor;
	if (rz) *rz = cur_rz / scalefactor;
}

*********************** Not currently used mouse functions ******************
*/

/*
******************************************************************************

   SVGAlib wrapper for LibGGI - keyboard handling

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

static unsigned char keystate[NR_KEYS];
int _ggigsw_ctrlc = GSW_CTRLC_CTRL | GSW_CTRLC_C;
int _ggigsw_keytransmask = DONT_CATCH_CTRLC;

/*
***************************************************************************
 Internal routines
***************************************************************************
*/

static int chkscancode(int scancode)
{
	if (scancode < 0 || scancode >= NR_KEYS) {
		return 1;
	}
	return 0;
}

static void default_keyhandler(int scancode, int newstate)
{
	if (chkscancode(scancode)) return;
	if (_ggigsw_keytransmask & TRANSLATE_CURSORKEYS)
		switch (scancode) {
		case SCANCODE_CURSORBLOCKLEFT:
			scancode = SCANCODE_CURSORLEFT;
			break;
		case SCANCODE_CURSORBLOCKRIGHT:
			scancode = SCANCODE_CURSORRIGHT;
			break;
		case SCANCODE_CURSORBLOCKUP:
			scancode = SCANCODE_CURSORUP;
			break;
		case SCANCODE_CURSORBLOCKDOWN:
			scancode = SCANCODE_CURSORDOWN;
			break;
		}
	if (_ggigsw_keytransmask & TRANSLATE_DIAGONAL) {
		switch (scancode) {
		case SCANCODE_CURSORDOWNLEFT:
			keystate[SCANCODE_CURSORDOWN] = newstate;
			keystate[SCANCODE_CURSORLEFT] = newstate;
			return;
		case SCANCODE_CURSORDOWNRIGHT:
			keystate[SCANCODE_CURSORDOWN] = newstate;
			keystate[SCANCODE_CURSORRIGHT] = newstate;
			return;
		case SCANCODE_CURSORUPLEFT:
			keystate[SCANCODE_CURSORUP] = newstate;
			keystate[SCANCODE_CURSORLEFT] = newstate;
			return;
		case SCANCODE_CURSORUPRIGHT:
			keystate[SCANCODE_CURSORUP] = newstate;
			keystate[SCANCODE_CURSORRIGHT] = newstate;
			return;
		}
	}
	if ((_ggigsw_keytransmask & TRANSLATE_KEYPADENTER)
	    && scancode == SCANCODE_KEYPADENTER) {
		scancode = SCANCODE_ENTER;
	}

	keystate[scancode] = newstate;
}

void (*_ggigsw_keyhandler) (int, int) = default_keyhandler;

static int keyboard_update(void)
{
	if (_ggigsw_getkeyevent(0))
        return NR_KEYS;
       
    return 0;
}

static const char* keyboard_getstate(void)
{
	return keystate;
}

/*
*********************** Not currently used keyboard functions ******************
static void keyboard_waitforupdate(void)
{
	_ggigsw_getkeyevent(1);
}

static void keyboard_clearstate(void)
{
	memset(keystate, 0, NR_KEYS);
	_ggigsw_ctrlc = GSW_CTRLC_CTRL | GSW_CTRLC_C;
}

static void keyboard_translatekeys(int mask)
{
	_ggigsw_keytransmask = mask;
}

static int keyboard_keypressed(int scancode)
{
	if (chkscancode(scancode)) return 0;

	return keystate[scancode];
}

*********************** Not currently used keyboard functions ******************
*/

#ifdef _LITE_VERSION
static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except, 
                struct timeval *timeout)
#else
static int wait_event (int which, fd_set *in, fd_set *out, fd_set *except, 
                struct timeval *timeout)
#endif
{
    ggi_event_mask mask = 0;
    int retval;

    if (which & IAL_MOUSEEVENT) {
        mask |= emKeyPress | emKeyRelease;
    }
    if (which & IAL_KEYEVENT) {
        mask |= emPointer;
    }

#ifdef _LITE_VERSION
    retval = ggiEventSelect(PHYSICALGC.visual, &mask, maxfd + 1, in, out, except,
                timeout);
#else
    /* GRR, this braindead vga_waitevent() doesn't take the n argument of
       select() */
    retval = ggiEventSelect(PHYSICALGC.visual, &mask, FD_SETSIZE, in, out, except,
                timeout);
#endif
    if (retval < 0) {
        return -1;
    }

    retval = 0;
    if (mask & emPointer) {
        retval |= IAL_MOUSEEVENT;

        mouse_update();
    }
    if (mask & (emKeyPress | emKeyRelease)) {
        retval |= IAL_KEYEVENT;
        keyboard_update();
    }

    return retval;
}

BOOL InitLibGGIInput (INPUT* input, const char* mdev, const char* mtype)
{
    char* str;

	_ggigsw_visual = PHYSICALGC.visual;

    ggiSetEventMask (VIS, emKeyPress | emKeyRelease | emPointer);

    if ((str = getenv("GSW_ASYNC"))) {
        /* We call ggiFlush() in mouse_update() or keyboard_update() */
        ggiSetFlags(VIS, GGIFLAG_ASYNC);
        if (strcmp(str, "mouse") == 0) {
            _ggigsw_async = GSW_ASYNCMOUSE;
            _GSWPRINT("Using async-mouse mode\n");
        } else {
            _ggigsw_async = GSW_ASYNCKEY;
            _GSWPRINT("Using async-key mode\n");
        }
    }
    if (getenv("GSW_BUT2KEY")) {
        _ggigsw_but2key = GSW_HAVEIT;
    }

    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = mouse_setposition;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = mouse_setrange;
    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_getstate;
    input->set_leds = NULL;
    input->wait_event = wait_event;

	return TRUE;
}

void TermLibGGIInput (void) { }

