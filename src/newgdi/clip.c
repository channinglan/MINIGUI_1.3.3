/*
** $Id: clip.c,v 1.13 2003/09/04 06:02:53 weiym Exp $
**
** clip.c: Clipping operations of GDI.
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
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"

/************************* Clipping support **********************************/
#define RESET_LCRGN(pdc)\
    if (pdc->lcrgn.head == NULL) {\
        RECT my_rc = {0, 0, 65535, 65535};\
        SetClipRgn (&pdc->lcrgn, &my_rc);\
    }

void GUIAPI ExcludeClipRect (HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc;

    pdc = dc_HDC2PDC (hdc);

    rc = *prc;
    NormalizeRect (&rc);
    if (IsRectEmpty (&rc))
        return;

    if (dc_IsGeneralDC (pdc)) {
        RESET_LCRGN(pdc);
        SubtractClipRect (&pdc->lcrgn, &rc);

        // Transfer logical to device to screen here.
        coor_DP2SP (pdc, &rc.left, &rc.top);
        coor_DP2SP (pdc, &rc.right, &rc.bottom);
    }

    SubtractClipRect (&pdc->ecrgn, &rc);
}

void GUIAPI IncludeClipRect (HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc;

    pdc = dc_HDC2PDC(hdc);

    rc = *prc;
    NormalizeRect(&rc);
    if (IsRectEmpty (&rc))
        return;

    if (dc_IsGeneralDC (pdc)) {
        if (pdc->lcrgn.head)
            AddClipRect (&pdc->lcrgn, &rc);
        else
            SetClipRgn (&pdc->lcrgn, &rc);
    
        // Transfer logical to device to screen here.
        coor_DP2SP (pdc, &rc.left, &rc.top);
        coor_DP2SP (pdc, &rc.right, &rc.bottom);

        LOCK (&pdc->pGCRInfo->lock);
        dc_GenerateECRgn (pdc, TRUE);
        UNLOCK (&pdc->pGCRInfo->lock);
    }
    else {
        AddClipRect (&pdc->ecrgn, &rc);
    }
}

void GUIAPI ClipRectIntersect (HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc;

    pdc = dc_HDC2PDC(hdc);

    rc = *prc;
    NormalizeRect(&rc);
    if (IsRectEmpty (&rc))
        return;

    if (dc_IsGeneralDC (pdc)) {
        RESET_LCRGN(pdc);
        IntersectClipRect (&pdc->lcrgn, &rc);

        // Transfer logical to device to screen here.
        coor_DP2SP (pdc, &rc.left, &rc.top);
        coor_DP2SP (pdc, &rc.right, &rc.bottom);
    }

    IntersectClipRect (&pdc->ecrgn, &rc);
}

void GUIAPI SelectClipRect (HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc;

    pdc = dc_HDC2PDC(hdc);

    if (prc) {
        rc = *prc;
        NormalizeRect (&rc);
        if (IsRectEmpty (&rc))
            return;
    }
    else 
        rc = pdc->DevRC;

    if (dc_IsGeneralDC (pdc)) {
        if (prc)
            SetClipRgn (&pdc->lcrgn, &rc);
        else
            EmptyClipRgn (&pdc->lcrgn);

#ifdef _REGION_DEBUG
        fprintf (stderr, "\n----------------------------\n");
        dumpRegion (&pdc->ecrgn);
#endif

        /* for general DC, regenerate effective region. */
        LOCK (&pdc->pGCRInfo->lock);
        dc_GenerateECRgn (pdc, TRUE);
        UNLOCK (&pdc->pGCRInfo->lock);

#ifdef _REGION_DEBUG
        dumpRegion (&pdc->ecrgn);
        fprintf (stderr, "----------------------------\n");
#endif
    }
    else {
        if (IntersectRect (&rc, &rc, &pdc->DevRC))
            SetClipRgn (&pdc->ecrgn, &rc);
        else
            EmptyClipRgn (&pdc->ecrgn);
    }
}

void GUIAPI SelectClipRegion (HDC hdc, const CLIPRGN* pRgn)
{
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);
    if (dc_IsGeneralDC (pdc)) {
        ClipRgnCopy (&pdc->lcrgn, pRgn);

        /* for general DC, regenerate effective region. */
        LOCK (&pdc->pGCRInfo->lock);
        dc_GenerateECRgn (pdc, TRUE);
        UNLOCK (&pdc->pGCRInfo->lock);
    }
    else {
        ClipRgnCopy (&pdc->ecrgn, pRgn);
        IntersectClipRect (&pdc->ecrgn, &pdc->DevRC);
    }
}

void GUIAPI GetBoundsRect (HDC hdc, RECT* pRect)
{
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);

    if (dc_IsGeneralDC (pdc))
        *pRect = pdc->lcrgn.rcBound;
    else
        *pRect = pdc->ecrgn.rcBound;
}

BOOL GUIAPI PtVisible (HDC hdc, int x, int y)
{
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);

    if (dc_IsGeneralDC (pdc)) {
        if (pdc->lcrgn.head == NULL)
            return TRUE;
        return PtInRegion (&pdc->lcrgn, x, y);
    }

    return PtInRegion (&pdc->ecrgn, x, y);
}

BOOL GUIAPI RectVisible(HDC hdc, const RECT* pRect)
{
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);

    if (dc_IsGeneralDC (pdc)) {
        if (pdc->lcrgn.head == NULL)
            return TRUE;
        return RectInRegion (&pdc->lcrgn, pRect);
    }

    return RectInRegion (&pdc->ecrgn, pRect);
}

