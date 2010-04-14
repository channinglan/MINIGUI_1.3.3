/*
 * \file dti.c
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 1998/12/xx
 *
 * This file defines the desktop interface funtions.  You should always 
 * include the file in your projects for MiniGUI-Threads.
 *
 \verbatim
    Copyright (C) 1999-2002 Wei Yongming.
    Copyright (C) 2002-2003 Feynman Software.
  
    This file is part of MiniGUI, a lightweight Graphics User Interface 
    support library for real-time embedded Linux.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 \endverbatim
 */

/*
 * $Id: dti.c,v 1.14 2003/11/22 14:02:47 weiym Exp $
 *
 *             MiniGUI for Linux Version 1.3.x
 *             Copyright (C) 1998-2002 Wei Yongming.
 *             Copyright (C) 2002-2003 Feynman Software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#ifndef _LITE_VERSION

#ifdef __ECOS
int my_ecos_main (int argc, const char* argv[]);
int main (int argc, const char* argv[])
{
    return my_ecos_main (argc, argv);
}
#endif

/*
 * MiniGUI will call PreInitGUI on startup time. 
 * You can do something before GUI by implementing this function.
 * Please return TRUE to continue initialize MiniGUI.
 */
#ifndef DONT_USE_SYS_PREINITGUI
BOOL PreInitGUI (int args, const char* arg [], int* retp)
{
    return TRUE;
}
#endif

/*
 * MiniGUI will call PostTerminateGUI after switch text mode.
 * You can do something after GUI by implementing this function.
 * rcByGUI will be the return code of this program.
 */
#ifndef DONT_USE_SYS_POSTTERMINATEGUI
int PostTerminateGUI (int args, const char* arg [], int rcByGUI)
{
    return rcByGUI;
}
#endif

/*
 * When the user clicks right mouse button on desktop, 
 * MiniGUI will display a menu for user. You can use this 
 * function to customize the desktop menu. e.g. add a new 
 * menu item.
 * Please use an integer larger than IDM_DTI_FIRST as the 
 * command ID.
 */
#ifndef DONT_USE_SYS_CUSTOMIZEDESKTOPMENU
#define IDC_DTI_ABOUT   (IDM_DTI_FIRST)
void CustomizeDesktopMenu (HMENU hmnu, int iPos)
{
#ifdef _MISC_ABOUTDLG
    MENUITEMINFO mii;

    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = IDC_DTI_ABOUT;
    mii.typedata    = (DWORD)GetSysText(SysText [19]);
    mii.hsubmenu    = 0;
    InsertMenuItem (hmnu, iPos, TRUE, &mii);
#endif
}

/*
 * When user choose a custom menu item on desktop menu,
 * MiniGUI will call this function, and pass the command ID
 * of selected menu item.
 */
int CustomDesktopCommand (int id)
{
#ifdef _MISC_ABOUTDLG
#ifndef _LITE_VERSION
    if (id == IDC_DTI_ABOUT)
        OpenAboutDialog ();
#endif
#endif

    return 0;
}
#endif

#endif /* _LITE_VERSION */

