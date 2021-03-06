/*
** $Id: pixel.c,v 1.18 2003/09/04 06:02:53 weiym Exp $
**
** pixel.c: drawing of pixel.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/10/18, derived from original draw.c
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
#include "pixel_ops.h"
#include "cursor.h"

/*********************** Generl drawing support ******************************/
void _set_pixel_helper (PDC pdc, int x, int y)
{
    SetRect (&pdc->rc_output, x - 1, y - 1, x + 1, y + 1);

    ENTER_DRAWING (pdc);

    if (PtInRect (&pdc->rc_output, x, y) && PtInRegion (&pdc->ecrgn, x, y)) {
        pdc->move_to (pdc, x, y);
        pdc->set_pixel (pdc);
    }

    LEAVE_DRAWING (pdc);
}

static void _my_set_pixel (PDC pdc, int x, int y, gal_pixel pixel)
{
    if (dc_IsGeneralDC (pdc)) {
        LOCK (&pdc->pGCRInfo->lock);
        if (!dc_GenerateECRgn (pdc, FALSE)) {
            UNLOCK (&pdc->pGCRInfo->lock);
            return;
        }
    }

#ifdef _LITE_VERSION
    if (CHECK_DRAWING (pdc)) return;
#endif

    coor_LP2SP (pdc, &x, &y);
    pdc->cur_pixel = pixel;
    _set_pixel_helper (pdc, x, y);
    UNLOCK_GCRINFO (pdc);
}

void GUIAPI SetPixel (HDC hdc, int x, int y, gal_pixel pixel)
{
    _my_set_pixel (dc_HDC2PDC (hdc), x, y, pixel);
}

gal_pixel GUIAPI SetPixelRGB (HDC hdc, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = GAL_MapRGB (pdc->surface->format, r, g, b);
    _my_set_pixel (pdc, x, y, pixel);

    return pixel;
}

gal_pixel GUIAPI SetPixelRGBA (HDC hdc, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = GAL_MapRGBA (pdc->surface->format, r, g, b, a);
    _my_set_pixel (pdc, x, y, pixel);

    return pixel;
}

/* FIXME: I do not know why this can not work in MiniGUI-Lite. :( */
#ifndef _LITE_VERSION 

gal_pixel _dc_get_pixel_cursor (PDC pdc, int x, int y)
{
    gal_pixel pixel = 0;
    Uint8* dst = NULL;

    if (x >= pdc->DevRC.left && y >= pdc->DevRC.top
                && x < pdc->DevRC.right && y < pdc->DevRC.bottom) {
#ifdef _CURSOR_SUPPORT
        if (!dc_IsMemDC (pdc)) {
            dst = GetPixelUnderCursor (x, y, &pixel);
        }

        if (dst == NULL) {
#endif
            dst = _dc_get_dst (pdc, x, y);
            pixel = _mem_get_pixel (dst, pdc->surface->format->BytesPerPixel);
#ifdef _CURSOR_SUPPORT
        }
#endif
    }

    return pixel;
}

#else

gal_pixel _dc_get_pixel_cursor (PDC pdc, int x, int y)
{
    gal_pixel pixel = 0;
    Uint8* dst = NULL;

    if (x >= pdc->DevRC.left && y >= pdc->DevRC.top
                && x < pdc->DevRC.right && y < pdc->DevRC.bottom) {
        SetRect (&pdc->rc_output, x, y, x+1, y+1);

        ENTER_DRAWING_NOCHECK (pdc);
        dst = _dc_get_dst (pdc, x, y);
        pixel = _mem_get_pixel (dst, pdc->surface->format->BytesPerPixel);
        LEAVE_DRAWING_NOCHECK (pdc);
    }

    return pixel;
}

#endif

static gal_pixel get_pixel (PDC pdc, int x, int y)
{
    gal_pixel pixel;

    if (dc_IsGeneralDC (pdc)) {
        LOCK (&pdc->pGCRInfo->lock);
        dc_GenerateECRgn (pdc, FALSE);
    }

    coor_LP2SP (pdc, &x, &y);
    pixel = _dc_get_pixel_cursor (pdc, x, y);

    UNLOCK_GCRINFO (pdc);

    return pixel;
}

gal_pixel GUIAPI GetPixel (HDC hdc, int x, int y)
{
    return get_pixel (dc_HDC2PDC (hdc), x, y);
}

gal_pixel GUIAPI GetPixelRGB (HDC hdc, int x, int y, Uint8* r, Uint8* g, Uint8* b)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = get_pixel (pdc, x, y);
    GAL_GetRGB (pixel, pdc->surface->format, r, g, b);

    return pixel;
}

gal_pixel GUIAPI GetPixelRGBA (HDC hdc, int x, int y, Uint8* r, Uint8* g, Uint8* b, Uint8* a)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = get_pixel (pdc, x, y);
    GAL_GetRGBA (pixel, pdc->surface->format, r, g, b, a);

    return pixel;
}

gal_pixel GUIAPI RGB2Pixel (HDC hdc, Uint8 r, Uint8 g, Uint8 b)
{
    return GAL_MapRGB (dc_HDC2PDC (hdc)->surface->format, r, g, b);
}

gal_pixel GUIAPI RGBA2Pixel (HDC hdc, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    return GAL_MapRGBA (dc_HDC2PDC (hdc)->surface->format, r, g, b, a);
}

void GUIAPI Pixel2RGB (HDC hdc, gal_pixel pixel, Uint8 *r, Uint8 *g, Uint8 *b)
{
    return GAL_GetRGB (pixel, dc_HDC2PDC (hdc)->surface->format, r, g, b);
}

void GUIAPI Pixel2RGBA (HDC hdc, gal_pixel pixel, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
    return GAL_GetRGBA (pixel, dc_HDC2PDC (hdc)->surface->format, r, g, b, a);
}

