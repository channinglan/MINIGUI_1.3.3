/*
** $Id: newgal.c,v 1.21 2003/09/25 04:08:55 snig Exp $
** 
** The New Graphics Abstract Layer of MiniGUI.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/10/07
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
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "newgal.h"
#include "misc.h"


GAL_Surface* __gal_screen;

#define LEN_ENGINE_NAME 8
#define LEN_MODE        20

int InitGAL (void)
{
    int i;
    int w, h, depth;
    char engine [LEN_ENGINE_NAME + 1];
    char mode [LEN_MODE + 1];

    if (GetMgEtcValue ("system", "gal_engine", engine, LEN_ENGINE_NAME) < 0 )
        return ERR_CONFIG_FILE;
    
    if (GAL_VideoInit (engine, 0)) {
        GAL_VideoQuit ();
        fprintf (stderr, "NEWGAL: Does not find matched engine: %s.\n", engine);
        return ERR_NO_MATCH;
    }

    if (GetMgEtcValue (engine, "defaultmode", mode, LEN_MODE) < 0)
        return ERR_CONFIG_FILE;

    w = atoi (mode);
    h = atoi (strchr (mode, 'x') + 1);
    depth = atoi (strrchr (mode, '-') + 1);

    if (!(__gal_screen = GAL_SetVideoMode (w, h, depth, GAL_HWPALETTE))) {
        GAL_VideoQuit ();
        fprintf (stderr, "NEWGAL: Set video mode failure.\n");
        return ERR_GFX_ENGINE;
    }

#ifdef _LITE_VERSION
    if (w != __gal_screen->w || h != __gal_screen->h) {
        fprintf (stderr, "The resolution specified in MiniGUI.cfg is not the same as "
                         "the actual resolution: %dx%d.\n" 
                         "This may confuse the clients. Please change it.\n", 
                         __gal_screen->w, __gal_screen->h);
        GAL_VideoQuit ();
        return ERR_GFX_ENGINE;
    }
#endif

    for (i = 0; i < 17; i++) {
        SysPixelIndex [i] = GAL_MapRGB (__gal_screen->format, 
                        SysPixelColor [i].r, SysPixelColor [i].g, SysPixelColor [i].b);
    }

    return 0;
}

