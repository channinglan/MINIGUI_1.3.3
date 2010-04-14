/*
** $Id: palette.c,v 1.6 2003/09/04 06:02:53 weiym Exp $
**
** palette.c: Palette operations of GDI.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/08/02
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
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"

BOOL GUIAPI GetPalette (HDC hdc, int start, int len, GAL_Color* cmap)
{
    PDC pdc;
    GAL_Palette* pal;

    pdc = dc_HDC2PDC (hdc);

    pal = pdc->surface->format->palette;

    if (pal && ((start + len) <= pal->ncolors)) {
        memcpy (cmap, pal->colors + start, sizeof (GAL_Color) * len);
        return TRUE;
    }
    
    return FALSE;
}

BOOL GUIAPI SetPalette (HDC hdc, int start, int len, GAL_Color* cmap)
{
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);
    return GAL_SetPalette (pdc->surface, GAL_LOGPAL, cmap, start, len);
}

BOOL GUIAPI SetColorfulPalette (HDC hdc)
{
    PDC pdc;
    GAL_Color pal[256];

    pdc = dc_HDC2PDC (hdc);

    if (pdc->surface->format->BitsPerPixel == 8) {
        GAL_DitherColors (pal, 8);
        GAL_SetColors (pdc->surface, pal, 0, 256);
    }

    return FALSE;
}

