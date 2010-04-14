/*
    MiniGUI - A compact GUI support system for real-time embedded Linux.
    Copyright (C) 2001 Wei Yongming

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Wei Yongming
    ymwei@minigui.org
*/

#ifndef _GAL_qvfb_h
#define _GAL_qvfb_h

#include <sys/types.h>

#include "sysvideo.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	GAL_VideoDevice *this

#define QT_VFB_MOUSE_PIPE	"/tmp/.qtvfb_mouse-%d"
#define QT_VFB_KEYBOARD_PIPE	"/tmp/.qtvfb_keyboard-%d"

struct QVFbHeader
{
    int width;
    int height;
    int depth;
    int linestep;
    int dataoffset;
    RECT update;
    BYTE dirty;
    int  numcols;
    unsigned int clut[256];
};

struct QVFbKeyData
{
    unsigned int unicode;
    unsigned int modifiers;
    BOOL press;
    BOOL repeat;
};

/* Private display data */
struct GAL_PrivateVideoData {
    unsigned char* shmrgn;
    struct QVFbHeader* hdr;
};

#endif /* _GAL_qvfb_h */

