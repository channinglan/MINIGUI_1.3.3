/*
** $Id: initmgext.c,v 1.16 2003/12/02 02:18:21 tangxf Exp $
** 
** initmgext.c: Initialization and cleanup of mgext library.
** 
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Create date: 2001/01/17
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

#ifdef __MINIGUI_LIB__
    #include "common.h"
    #include "minigui.h"
    #include "gdi.h"
    #include "window.h"
    #include "control.h"
#else
    #include <minigui/common.h>
    #include <minigui/minigui.h>
    #include <minigui/gdi.h>
    #include <minigui/window.h>
    #include <minigui/control.h>
#endif

#include "mgext.h"
#ifdef _EXT_CTRL_MONTHCAL
  #include "monthcalendar.h"
#endif
#ifdef _EXT_CTRL_TREEVIEW
  #include "treeview.h"
#endif
#ifdef _EXT_CTRL_SPINBOX
  #include "spinbox.h"
#endif
#ifdef _EXT_CTRL_COOLBAR
  #include "coolbar.h"
#endif
#ifdef _EXT_CTRL_LISTVIEW
  #include "listview.h"
#endif
#ifdef _EXT_CTRL_GRID
  #include "grid.h"
#endif

#ifdef _EXT_SKIN
/* defined in the skin module */
BOOL RegisterSkinControl (void);
void SkinControlCleanup (void);
#endif

static int init_count = 0;

BOOL InitMiniGUIExt ( void )
{
    if (init_count > 0)
        goto success;

#ifdef _EXT_CTRL_TREEVIEW
    if (!RegisterTreeViewControl ()) {
        return FALSE;
    }
#endif

#ifdef _EXT_CTRL_MONTHCAL
    if (!RegisterMonthCalendarControl ()) {
        return FALSE;
    }
#endif

#ifdef _EXT_CTRL_SPINBOX
    if (!RegisterSpinControl() ) {
        return FALSE;
    }
#endif

#ifdef _EXT_CTRL_COOLBAR
    if (!RegisterCoolBarControl() ) {
        return FALSE;
    }
#endif

#ifdef _EXT_CTRL_LISTVIEW
    if (!RegisterListViewControl() ) {
        return FALSE;
    }
#endif

#ifdef _EXT_CTRL_GRID
    if (!RegisterGridControl() ) {
	return FALSE;
    }
#endif

#ifdef _EXT_SKIN
    if (!RegisterSkinControl() ) {
        return FALSE;
    }
#endif

success:
    init_count ++;
    return TRUE;
}

void MiniGUIExtCleanUp (void)
{
    init_count --;
    if (init_count != 0)
        return;

#ifdef _EXT_CTRL_TREEVIEW
    TreeViewControlCleanup ();
#endif
#ifdef _EXT_CTRL_MONTHCAL
    MonthCalendarControlCleanup ();
#endif
#ifdef _EXT_CTRL_SPINBOX
    SpinControlCleanup ();
#endif
#ifdef _EXT_CTRL_COOLBAR
    CoolBarControlCleanup ();
#endif
#ifdef _EXT_CTRL_LISTVIEW
    ListViewControlCleanup ();
#endif
#ifdef _EXT_CTRL_GRID
    GridControlCleanup ();
#endif
#ifdef _EXT_SKIN
    SkinControlCleanup ();
#endif

}

