/*
** $Id: arc.c,v 1.17 2003/09/04 06:02:53 weiym Exp $
**
** arc.c: drawing and filling arc, circle, and ellipse.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/10/12, derived from original draw.c
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

#define _USE_GENERATOR  1

/************************ Arc and Circle *******************************/

void GUIAPI Arc (HDC hdc, int sx, int sy, int r, fixed ang1, fixed ang2)
{
    PDC pdc;

    if (!(pdc = check_ecrgn (hdc)))
        return;

    coor_LP2SP (pdc, &sx, &sy);
    pdc->cur_pixel = pdc->pencolor;
    pdc->cur_ban = NULL;

    if (r < 1) {
        _set_pixel_helper (pdc, sx, sy);
        goto ret;
    }

    pdc->rc_output.left = sx - r;
    pdc->rc_output.top  = sy - r;
    pdc->rc_output.right = sx + r + 1;
    pdc->rc_output.bottom = sy + r + 1;

    ENTER_DRAWING (pdc);

    ArcGenerator (pdc, sx, sy, r, ang1, ang2, _dc_set_pixel_clip);

    LEAVE_DRAWING (pdc);

ret:
    UNLOCK_GCRINFO (pdc);
}

void GUIAPI Ellipse (HDC hdc, int sx, int sy, int rx, int ry)
{
    PDC pdc;

    if (!(pdc = check_ecrgn (hdc)))
        return;

    coor_LP2SP (pdc, &sx, &sy);

    pdc->cur_pixel = pdc->pencolor;
    pdc->cur_ban = NULL;

    if (rx < 1 || ry < 1) {
        _set_pixel_helper (pdc, sx, sy);
        goto ret;
    }

    pdc->rc_output.left = sx - rx;
    pdc->rc_output.top  = sy - ry;
    pdc->rc_output.right = sx + rx + 1;
    pdc->rc_output.bottom = sy + ry + 1;

    ENTER_DRAWING (pdc);

    EllipseGenerator (pdc, sx, sy, rx, ry, _dc_set_pixel_pair_clip);

    LEAVE_DRAWING (pdc);

ret:
    UNLOCK_GCRINFO (pdc);
}

void GUIAPI FillEllipse (HDC hdc, int sx, int sy, int rx, int ry)
{
    PDC pdc;

    if (!(pdc = check_ecrgn (hdc)))
        return;

    coor_LP2SP (pdc, &sx, &sy);
    pdc->cur_pixel = pdc->brushcolor;
    pdc->cur_ban = NULL;

    if (rx < 1 || ry < 1) {
        _set_pixel_helper (pdc, sx, sy);
        goto ret;
    }

    pdc->rc_output.left = sx - rx;
    pdc->rc_output.top  = sy - ry;
    pdc->rc_output.right = sx + rx + 1;
    pdc->rc_output.bottom = sy + ry + 1;

    ENTER_DRAWING (pdc);

    EllipseGenerator (pdc, sx, sy, rx, ry, _dc_draw_hline_clip);

    LEAVE_DRAWING (pdc);

ret:
    UNLOCK_GCRINFO (pdc);
}

void GUIAPI Circle (HDC hdc, int sx, int sy, int r)
{
    PDC pdc;

    if (!(pdc = check_ecrgn (hdc)))
        return;

    coor_LP2SP (pdc, &sx, &sy);

    pdc->cur_pixel = pdc->pencolor;
    pdc->cur_ban = NULL;

    if (r < 1) {
        _set_pixel_helper (pdc, sx, sy);
        goto ret;
    }

    pdc->rc_output.left = sx - r;
    pdc->rc_output.top  = sy - r;
    pdc->rc_output.right = sx + r + 1;
    pdc->rc_output.bottom = sy + r + 1;

    ENTER_DRAWING (pdc);

    CircleGenerator (pdc, sx, sy, r, _dc_set_pixel_pair_clip);

    LEAVE_DRAWING (pdc);

ret:
    UNLOCK_GCRINFO (pdc);
}

void GUIAPI FillCircle (HDC hdc, int sx, int sy, int r)
{
    PDC pdc;

    if (!(pdc = check_ecrgn (hdc)))
        return;

    coor_LP2SP (pdc, &sx, &sy);

    pdc->cur_pixel = pdc->brushcolor;
    pdc->cur_ban = NULL;

    if (r < 1) {
        _set_pixel_helper (pdc, sx, sy);
        goto ret;
    }

    pdc->rc_output.left = sx - r;
    pdc->rc_output.top  = sy - r;
    pdc->rc_output.right = sx + r + 1;
    pdc->rc_output.bottom = sy + r + 1;

    ENTER_DRAWING (pdc);

    CircleGenerator (pdc, sx, sy, r, _dc_draw_hline_clip);

    LEAVE_DRAWING (pdc);

ret:
    UNLOCK_GCRINFO (pdc);
}

