/*
** $Id: main.c,v 1.25 2003/11/22 14:02:48 weiym Exp $
**
** main.c: The main function wrapper.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
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

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "gal.h"
#include "ial.h"

BOOL GUIAPI InitGUI (void /*char* spAppName*/ );
void GUIAPI TerminateGUI (int rcByGUI);

// This is the main function.
// This function will call the MiniGUIMain, which works as
// the wrapper to the main function, and is the only entry of
// any MiniGUI application.


#ifdef __ECOS
int my_ecos_main (int args, const char* agr[])
#else
int main (int args, const char* agr[])
#endif

{
    int iRet = 0;

    if (!InitGUI ()) {
#ifdef _INCORE_RES
        fprintf (stderr, "InitGUI failure when using incore resource.\n");
#else
        fprintf (stderr, "InitGUI failure when using %s as cfg file.\n",
                                ETCFILEPATH);
#endif /* _INCORE_RES */
        return 1;
    }

    iRet = MiniGUIMain (args, agr);

    TerminateGUI (iRet);

    return iRet;
}

void GUIAPI MiniGUIPanic (int exitcode)
{
    TerminateIAL ();
    TerminateGAL ();
    _exit (exitcode);
}

