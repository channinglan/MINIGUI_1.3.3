/*
 * $Id: paint.h,v 1.2 2001/11/05 02:17:24 ymwei Exp $
 *
 * paint.h: window painting
 * 
 * VCOnGUI - Virtual Console On MiniGUi -
 * Copyright (C) 1999-2001 Wei Yongming (ymwei@minigui.org)
 *
 */

/*
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Library General Public
**  License as published by the Free Software Foundation; either
**  version 2 of the License, or (at your option) any later version.
**
**  This software is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Library General Public License for more details.
**
**  You should have received a copy of the GNU Library General Public
**  License along with this library; if not, write to the Free
**  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
**  MA 02111-1307, USA
*/

#ifndef PAINT_VCONGUI_H
#define PAINT_VCONGUI_H

void WindowScrollUp (PCONINFO con, int scroll, int color);
void WindowScrollDown (PCONINFO con, int scroll, int color);
void WindowClearAll (PCONINFO con, int color);
void WindowStringPut (PCONINFO con, const char* text, 
                            int fc, int bc, int x, int y);
void WindowWput (PCONINFO con, WORD word, int fc, int bc, int pos);
void WindowSput (PCONINFO con, u_char ch, int fc, int bc, int pos);
void WindowSetCursor (PCONINFO con, int x, int y, bool IsKanji);

#define WindowHideCaret HideCaret
#define WindowShowCaret ShowCaret

#endif /* PAINT_VCONGUI_H */

