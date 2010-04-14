/* $Id: attr.c,v 1.5 2003/09/04 06:02:53 weiym Exp $
**
** Drawing attributes of GDI
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/06/12, derived from original gdi.c
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

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "gal.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"

/******************* General drawing attributes *******************************/
Uint32 GUIAPI GetDCAttr (HDC hdc, int attr)
{
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);

#if 1
    if (attr < NR_DC_ATTRS && attr >= 0) {
        Uint32* attrs = (Uint32*) (&pdc->bkcolor);

        return attrs [attr];
    }
#else
    switch (attr) {
    case DC_ATTR_BK_COLOR:
        return (Uint32) pdc->bkcolor;
    case DC_ATTR_BK_MODE:
        return (Uint32) pdc->bkmode;
    case DC_ATTR_TEXT_COLOR:
        return (Uint32) pdc->textcolor;
    case DC_ATTR_TAB_STOP:
        return (Uint32) pdc->tabstop;
    case DC_ATTR_PEN_COLOR:
        return (Uint32) pdc->pencolor;
    case DC_ATTR_BRUSH_COLOR:
        return (Uint32) pdc->brushcolor;
    case DC_ATTR_PEN_TYPE:
        return (Uint32) pdc->pentype;
    case DC_ATTR_BRUSH_TYPE:
        return (Uint32) pdc->brushtype;
    case DC_ATTR_CHAR_EXTRA:
        return (Uint32) pdc->cExtra;
    case DC_ATTR_ALINE_EXTRA:
        return (Uint32) pdc->alExtra;
    case DC_ATTR_BLINE_EXTRA:
        return (Uint32) pdc->blExtra;
    case DC_ATTR_MAP_MODE:
        return (Uint32) pdc->mapmode;
    }
#endif

    return 0;
}

Uint32 GUIAPI SetDCAttr (HDC hdc, int attr, Uint32 value)
{
    Uint32 old_value;
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);

#if 1
    if (attr < NR_DC_ATTRS && attr >= 0) {
        Uint32* attrs = (Uint32*) (&pdc->bkcolor);

        old_value = attrs [attr];
        attrs [attr] = value;
        return old_value;
    }
#else
    switch (attr) {
    case DC_ATTR_BK_COLOR:
        old_value = (Uint32) pdc->bkcolor;
        pdc->bkcolor = (gal_pixel)value;
        return old_value;
    case DC_ATTR_BK_MODE:
        old_value = (Uint32) pdc->bkmode;
        pdc->bkmode = (int)value;
        return old_value;
    case DC_ATTR_TEXT_COLOR:
        old_value = (Uint32) pdc->textcolor;
        pdc->textcolor = (gal_pixel)value;
        return old_value;
    case DC_ATTR_TAB_STOP:
        old_value = (Uint32) pdc->tabstop;
        pdc->tabstop = (int)value;
        return old_value;
    case DC_ATTR_PEN_COLOR:
        old_value = (Uint32) pdc->pencolor;
        pdc->pencolor = (gal_pixel)value;
        return old_value;
    case DC_ATTR_BRUSH_COLOR:
        old_value = (Uint32) pdc->brushcolor;
        pdc->brushcolor = (gal_pixel)value;
        return old_value;
    case DC_ATTR_PEN_TYPE:
        old_value = (Uint32) pdc->pentype;
        pdc->pentype = (int)value;
        return old_value;
    case DC_ATTR_BRUSH_TYPE:
        old_value = (Uint32) pdc->brushtype;
        pdc->brushtype = (int)value;
        return old_value;
    case DC_ATTR_CHAR_EXTRA:
        old_value = (Uint32) pdc->cExtra;
        pdc->cExtra = (int)value;
        return old_value;
    case DC_ATTR_ALINE_EXTRA:
        old_value = (Uint32) pdc->alExtra;
        pdc->alExtra = (int)value;
        return old_value;
    case DC_ATTR_BLINE_EXTRA:
        old_value = (Uint32) pdc->blExtra;
        pdc->blExtra = (int)value;
        return old_value;
    case DC_ATTR_MAP_MODE:
        old_value = (Uint32) pdc->mapmode;
        pdc->mapmode = (int)value;
        return old_value;
    }
#endif

    return 0;
}


