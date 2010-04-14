/* $Id: ggi_input.c,v 1.4 2000/08/28 03:08:22 weiym Exp $
******************************************************************************

   SVGAlib wrapper for LibGGI - LibGGI specific input handling

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

#include <stdio.h>

#include "common.h"
#include "svgaggi_internal.h"


static inline int
send_b2k(int but, int press)
{
	switch (but) {
	case 4:
		_ggigsw_keyhandler(GSW_BUT4,
				   press ? KEY_EVENTPRESS : KEY_EVENTRELEASE);
		break;
	case 5:
		_ggigsw_keyhandler(GSW_BUT5,
				   press ? KEY_EVENTPRESS : KEY_EVENTRELEASE);
		break;
	case 6:
		_ggigsw_keyhandler(GSW_BUT6,
				   press ? KEY_EVENTPRESS : KEY_EVENTRELEASE);
		break;
	default:
		return 0;
	}
	return 1;
}

	
int
_ggigsw_getmouseevent(int wait)
{
	ggi_event ev;
	int handled = 0;
	struct timeval *tvp, tv = {0,0};
	
	if (_ggigsw_async == GSW_ASYNCMOUSE) {
		ggiFlush(_ggigsw_visual);
	}
	
	if (wait) tvp = NULL;
	else tvp = &tv;

	while (ggiEventPoll(_ggigsw_visual,
			    emPtrButton | emPtrAbsolute | emPtrRelative,
			    tvp)) {
		int queueevent = 1;
		static int buttons = 0;
		static int mouse_x = 0, mouse_y = 0, mouse_z = 0;
		int x = 0, y = 0, z = 0, rx = 0, ry = 0, rz = 0;
		ggiEventRead(_ggigsw_visual, &ev, 
			     emPtrButton | emPtrAbsolute | emPtrRelative);
		switch (ev.any.type) {
		case evPtrButtonRelease:
			switch (ev.pbutton.button) {
			case 1:
				buttons &= ~0x04;
				break;
			case 2:
				buttons &= ~0x01;
				break;
			case 3:
				buttons &= ~0x02;
				break;
			default:
				if (_ggigsw_but2key == GSW_HAVEIT) {
					if (send_b2k(ev.pbutton.button, 0)) {
						queueevent = 0;
					}
				}
			}
			break;
		case evPtrButtonPress:
			switch (ev.pbutton.button) {
			case 1:
				buttons |= 0x04;
				break;
			case 2:
				buttons |= 0x01;
				break;
			case 3:
				buttons |= 0x02;
				break;
			default:
				if (_ggigsw_but2key == GSW_HAVEIT) {
					if (send_b2k(ev.pbutton.button, 1)) {
						queueevent = 0;
					}
				}
			}
			break;
		case evPtrRelative:
			x = ev.pmove.x;
			y = ev.pmove.y;
			z = ev.pmove.wheel;
			break;
		case evPtrAbsolute:
			if (mouse_x != ev.pmove.x
			    || mouse_y != ev.pmove.y
			    || mouse_z != ev.pmove.wheel) {
				x = ev.pmove.x - mouse_x;
				y = ev.pmove.y - mouse_y;
				z = ev.pmove.wheel - mouse_z;
				mouse_x = ev.pmove.x;
				mouse_y = ev.pmove.y;
				mouse_z = ev.pmove.wheel;
			} else {
				queueevent = 0;
			}
			break;
		}
		if (queueevent) {
			_ggigsw_mousehandler(buttons,
					     x, y, z, rx, ry, rz);
			handled = 1;
		}
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		tvp = &tv;
	}

	return handled;
}

static inline int
ev2scancode(gii_key_event *kev)
{
	switch (kev->label) {
	case GIIUC_Escape:
		return SCANCODE_ESCAPE;
	case GIIUC_Return:
		return SCANCODE_ENTER;
	case GIIK_CtrlL:
		return SCANCODE_LEFTCONTROL;
	case GIIUC_BackSpace:
		return SCANCODE_BACKSPACE;
	case GIIUC_Tab:
		return SCANCODE_TAB;
	case GIIK_ShiftL:
		return SCANCODE_LEFTSHIFT;
	case GIIK_ShiftR:
		return SCANCODE_RIGHTSHIFT;
	case GIIK_PStar:
		return SCANCODE_KEYPADMULTIPLY;
	case GIIK_AltL:
		return SCANCODE_LEFTALT;
	case GIIK_CapsLock:
		return SCANCODE_CAPSLOCK;
			
	case GIIK_F1:
		return SCANCODE_F1;
	case GIIK_F2:
		return SCANCODE_F2;
	case GIIK_F3:
		return SCANCODE_F3;
	case GIIK_F4:
		return SCANCODE_F4;
	case GIIK_F5:
		return SCANCODE_F5;
	case GIIK_F6:
		return SCANCODE_F6;
	case GIIK_F7:
		return SCANCODE_F7;
	case GIIK_F8:
		return SCANCODE_F8;
	case GIIK_F9:
		return SCANCODE_F9;
	case GIIK_F10:
		return SCANCODE_F10;
	case GIIK_NumLock:
		return SCANCODE_NUMLOCK;
    case 0xe00e:
    case 0xe01b:
		return SCANCODE_PRINTSCREEN;
#if 0
    case GIIK_ScrollLock:
		return SCANCODE_SCROLLLOCK;
#endif
	case GIIK_P7:
		return SCANCODE_KEYPAD7;
	case GIIK_P8:
		return SCANCODE_KEYPAD8;
	case GIIK_P9:
		return SCANCODE_KEYPAD9;
	case GIIK_PMinus:
		return SCANCODE_KEYPADMINUS;
	case GIIK_P4:
		return SCANCODE_KEYPAD4;
	case GIIK_P5:
		return SCANCODE_KEYPAD5;
	case GIIK_P6:
		return SCANCODE_KEYPAD6;
	case GIIK_PPlus:
		return SCANCODE_KEYPADPLUS;
	case GIIK_P1:
		return SCANCODE_KEYPAD1;
	case GIIK_P2:
		return SCANCODE_KEYPAD2;
	case GIIK_P3:
		return SCANCODE_KEYPAD3;
	case GIIK_P0:
		return SCANCODE_KEYPAD0;
	case GIIK_PSeparator:
	case GIIK_PDecimal:
		return SCANCODE_KEYPADPERIOD;
			
	case GIIUC_Less:
		return SCANCODE_LESS;

	case GIIK_F11:
		return SCANCODE_F11;
	case GIIK_F12:
		return SCANCODE_F12;

	case GIIK_PEnter:
		return SCANCODE_KEYPADENTER;
	case GIIK_CtrlR:
		return SCANCODE_RIGHTCONTROL;
	case GIIK_PSlash:
		return SCANCODE_KEYPADDIVIDE;
	case GIIK_AltR:
		return SCANCODE_RIGHTALT;
	case GIIK_Break:
		return SCANCODE_BREAK;
	case GIIK_Home:
		return SCANCODE_HOME;
	case GIIK_Up:
		return SCANCODE_CURSORBLOCKUP;
	case GIIK_PageUp:
		return SCANCODE_PAGEUP;
	case GIIK_Left:
		return SCANCODE_CURSORBLOCKLEFT;
	case GIIK_Right:
		return SCANCODE_CURSORBLOCKRIGHT;
	case GIIK_End:
		return SCANCODE_END;
	case GIIK_Down:
		return SCANCODE_CURSORBLOCKDOWN;
	case GIIK_PageDown:
		return SCANCODE_PAGEDOWN;
	case GIIK_Insert:
		return SCANCODE_INSERT;
	case GIIUC_Delete:
		return SCANCODE_REMOVE;
#if 0
	case GIIK_P:
		return SCANCODE_BREAK_ALTERNATIVE;
	case GIIK_P:
		return SCANCODE_CURSORUPRIGHT;
	case GIIK_P:
		return SCANCODE_CURSORUPLEFT;
	case GIIK_P:
		return SCANCODE_CURSORUP;
	case GIIK_P:
		return SCANCODE_CURSORLEFT;
	case GIIK_P:
		return SCANCODE_CURSORRIGHT;
	case GIIK_P:
		return SCANCODE_CURSORDOWNLEFT;
	case GIIK_P:
		return SCANCODE_CURSORDOWN;
	case GIIK_P:
		return SCANCODE_CURSORDOWNRIGHT;
#endif
	case GIIUC_1:
		return SCANCODE_1;
	case GIIUC_2:
		return SCANCODE_2;
	case GIIUC_3:
		return SCANCODE_3;
	case GIIUC_4:
		return SCANCODE_4;
	case GIIUC_5:
		return SCANCODE_5;
	case GIIUC_6:
		return SCANCODE_6;
	case GIIUC_7:
		return SCANCODE_7;
	case GIIUC_8:
		return SCANCODE_8;
	case GIIUC_9:
		return SCANCODE_9;
	case GIIUC_0:
		return SCANCODE_0;
	case GIIUC_Minus:
		return SCANCODE_MINUS;
	case GIIUC_Equal:
		return SCANCODE_EQUAL;
    
	case GIIUC_q:
	case GIIUC_Q:
		return SCANCODE_Q;
	case GIIUC_w:
	case GIIUC_W:
		return SCANCODE_W;
	case GIIUC_e:
	case GIIUC_E:
		return SCANCODE_E;
	case GIIUC_r:
	case GIIUC_R:
		return SCANCODE_R;
	case GIIUC_t:
	case GIIUC_T:
		return SCANCODE_T;
	case GIIUC_y:
	case GIIUC_Y:
		return SCANCODE_Y;
	case GIIUC_u:
	case GIIUC_U:
		return SCANCODE_U;
	case GIIUC_i:
	case GIIUC_I:
		return SCANCODE_I;
	case GIIUC_o:
	case GIIUC_O:
		return SCANCODE_O;
	case GIIUC_p:
	case GIIUC_P:
		return SCANCODE_P;
	case GIIUC_BracketLeft:
		return SCANCODE_BRACKET_LEFT;
	case GIIUC_BracketRight:
		return SCANCODE_BRACKET_RIGHT;
    
	case GIIUC_a:
	case GIIUC_A:
		return SCANCODE_A;
	case GIIUC_s:
	case GIIUC_S:
		return SCANCODE_S;
	case GIIUC_d:
	case GIIUC_D:
		return SCANCODE_D;
	case GIIUC_f:
	case GIIUC_F:
		return SCANCODE_F;
	case GIIUC_g:
	case GIIUC_G:
		return SCANCODE_G;
	case GIIUC_h:
	case GIIUC_H:
		return SCANCODE_H;
	case GIIUC_j:
	case GIIUC_J:
		return SCANCODE_J;
	case GIIUC_k:
	case GIIUC_K:
		return SCANCODE_K;
	case GIIUC_l:
	case GIIUC_L:
		return SCANCODE_L;
	case GIIUC_Semicolon:
		return SCANCODE_SEMICOLON;
	case GIIUC_Apostrophe:
		return SCANCODE_APOSTROPHE;
	case GIIUC_Grave:
		return SCANCODE_GRAVE;
	case GIIUC_BackSlash:
		return SCANCODE_BACKSLASH;
    
	case GIIUC_z:
	case GIIUC_Z:
		return SCANCODE_Z;
	case GIIUC_x:
	case GIIUC_X:
		return SCANCODE_X;
	case GIIUC_c:
	case GIIUC_C:
		return SCANCODE_C;
	case GIIUC_v:
	case GIIUC_V:
		return SCANCODE_V;
	case GIIUC_b:
	case GIIUC_B:
		return SCANCODE_B;
	case GIIUC_n:
	case GIIUC_N:
		return SCANCODE_N;
	case GIIUC_m:
	case GIIUC_M:
		return SCANCODE_M;
	case GIIUC_Comma:
		return SCANCODE_COMMA;
	case GIIUC_Period:
		return SCANCODE_PERIOD;
	case GIIUC_Slash:
		return SCANCODE_SLASH;
    
	case GIIUC_Space:
		return SCANCODE_SPACE;
	}
	fprintf(stderr, "Unknown LABEL: 0x%x\n", kev->label);
	return 0;
}


int
_ggigsw_getkeyevent(int wait)
{
	ggi_event ev;
	int handled = 0;
	struct timeval *tvp, tv = {0,0};
	int scancode;

	if (_ggigsw_async == GSW_ASYNCKEY) {
		ggiFlush(_ggigsw_visual);
	}

	if (wait) tvp = NULL;
	else tvp = &tv;

	while (ggiEventPoll(_ggigsw_visual, emKeyPress | emKeyRelease, tvp)) {
		int type;

		ggiEventRead(_ggigsw_visual, &ev, emKeyPress | emKeyRelease);

		type  = ev.any.type;
		scancode = ev2scancode(&ev.key);

		if (scancode == SCANCODE_C) {
			if (type == evKeyPress)	_ggigsw_ctrlc &= ~GSW_CTRLC_C;
			else _ggigsw_ctrlc |= GSW_CTRLC_C;
		} else if (scancode == SCANCODE_CONTROL ||
			   scancode == SCANCODE_LEFTCONTROL ||
			   scancode == SCANCODE_RIGHTCONTROL) {
			if (type == evKeyPress)	{
				_ggigsw_ctrlc &= ~GSW_CTRLC_CTRL;
			} else _ggigsw_ctrlc |= GSW_CTRLC_CTRL;
		}
		
		if (_ggigsw_ctrlc == 0
		    && !(_ggigsw_keytransmask & DONT_CATCH_CTRLC)) {
			ggiPanic("CTRL-C pressed - terminating\n");
		}
		
		if (type == evKeyPress) {
			_ggigsw_keyhandler(scancode, KEY_EVENTPRESS);
		} else {
			_ggigsw_keyhandler(scancode, KEY_EVENTRELEASE);
		}
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		tvp = &tv;
		handled = 1;
	}

	return handled;
}
