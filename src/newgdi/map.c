/*
** $Id: map.c,v 1.6 2003/09/04 06:02:53 weiym Exp $
**
** map.c: Mapping operations of GDI.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
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
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"

/****************************** Mapping support ******************************/
void GUIAPI GetDCLCS (HDC hdc, int which, POINT* pt)
{
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);

    if (which < NR_DC_LCS_PTS && which >= 0) {
        POINT* pts = &pdc->ViewOrig;

        *pt = pts [which];
    }
}

void GUIAPI SetDCLCS (HDC hdc, int which, const POINT* pt)
{
    PDC pdc;

    if (hdc == HDC_SCREEN)
        return;

    pdc = dc_HDC2PDC(hdc);
    if (which < NR_DC_LCS_PTS && which >= 0) {
        POINT* pts = &pdc->ViewOrig;

        pts [which] = *pt;
    }
}

