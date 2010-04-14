/* $Id: svgaggi_internal.h,v 1.2 2000/06/21 01:26:01 weiym Exp $
******************************************************************************

   SVGAlib wrapper for LibGGI - internal definitions

   Copyright (C) 1998-1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#ifndef _SVGAGGI_INTERNAL_H
#define _SVGAGGI_INTERNAL_H

#include <ggi/ggi.h>
#ifdef HAVE_GGI_GGI_UNIX_H
#include <ggi/ggi-unix.h>
#endif

#define MOUSE_NOWRAP 0
#define MOUSE_WRAPX 1
#define MOUSE_WRAPY 2
#define MOUSE_WRAPZ 4
#define MOUSE_WRAPRX 8
#define MOUSE_WRAPRY 16
#define MOUSE_WRAPRZ 32
#define MOUSE_WRAP 63
#define MOUSE_ROT_COORDS 196
#define MOUSE_ROT_INFINITESIMAL 0 /* report changes in angle about axes */
#define MOUSE_ROT_RX_RY_RZ 64     /* report angle about axes */

/* Warning! Next two not yet supported! */
#define MOUSE_ROT_ZXZ 128     /* use x convention Euler angles */
#define MOUSE_ROT_YPR 196     /* use yaw, pitch, and roll */

#define KEY_EVENTRELEASE 0
#define KEY_EVENTPRESS 1

#define TRANSLATE_CURSORKEYS  0x01  /* Map cursor block to keypad cursor. */
#define TRANSLATE_DIAGONAL    0x02  /* Map keypad diagonal to keypad cursor. */
#define TRANSLATE_KEYPADENTER 0x04  /* Map keypad enter to main enter key. */
#define DONT_CATCH_CTRLC      0x08  /* Disable Crtl-C check. */

extern ggi_visual_t _ggigsw_visual;
extern int _ggigsw_inggi;
extern int _ggigsw_async;
extern int _ggigsw_but2key;
extern int _ggigsw_modeemu;

#define GSW_CTRLC_C	1
#define GSW_CTRLC_CTRL	2
extern int _ggigsw_ctrlc;

extern int _ggigsw_keytransmask;

/* Internal functions */
void _ggigsw_mode5flush(void);
int _ggigsw_getmouseevent(int wait);
int _ggigsw_getkeyevent(int wait);
extern void (*_ggigsw_mousehandler) (int, int, int, int, int, int, int);
extern void (*_ggigsw_keyhandler) (int, int);

#define COLORSHIFT	10

#define GSW_PAGESIZE	(1<<16)

#define GSW_WANTIT	1
#define GSW_HAVEIT	2
#define GSW_HAVEMODE5	3

#define GSW_ASYNCKEY	1
#define GSW_ASYNCMOUSE	2

#define GSW_BUT4	SCANCODE_I
#define GSW_BUT5	SCANCODE_J
#define GSW_BUT6	SCANCODE_K

/* Debugging macros */
#if 0
#define _GSWPRINT(s)		printf(s)
#define _GSWPRINT_I(s,val) 	printf(s, val)
#else
#define _GSWPRINT(s)
#define _GSWPRINT_I(s,val)
#endif

#endif /* _SVGAGGI_INTERNAL_H */
