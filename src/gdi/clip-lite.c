/*
** $Id: clip-lite.c,v 1.6 2003/09/04 03:09:52 weiym Exp $
**
** clip-lite.c: Clipping operations of GDI for MiniGUI-Lite.
**
** Copyright (C) 2003 Feynman Software.
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
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"

extern BOOL dc_GenerateECRgn (PDC pdc, BOOL fForce);

/************************* Clipping support **********************************/
void GUIAPI ExcludeClipRect(HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc = *prc;

    pdc = dc_HDC2PDC(hdc);

    NormalizeRect(&rc);
    if (IsRectEmpty (&rc)) return;

    SubtractClipRect (&pdc->lcrgn, &rc);

    // Transfer logical to device to screen here.
    coor_LP2SP(pdc, &rc.left, &rc.top);
    coor_LP2SP(pdc, &rc.right, &rc.bottom);
    NormalizeRect(&rc);

    SubtractClipRect (&pdc->ecrgn, &rc);
}

void GUIAPI IncludeClipRect(HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc = *prc;

    pdc = dc_HDC2PDC(hdc);

    NormalizeRect(&rc);
    if (IsRectEmpty (&rc)) return;

    SubtractClipRect (&pdc->lcrgn, &rc);
    AddClipRect (&pdc->lcrgn, &rc);
    
    // Transfer logical to device to screen here.
    coor_LP2SP(pdc, &rc.left, &rc.top);
    coor_LP2SP(pdc, &rc.right, &rc.bottom);
    NormalizeRect(&rc);

    if (dc_IsGeneralHDC(hdc)) {
        dc_GenerateECRgn (pdc, TRUE);
    }
    else {
        SubtractClipRect (&pdc->ecrgn, &rc);
        IntersectClipRect (&pdc->ecrgn, &rc);
    }
}

void GUIAPI ClipRectIntersect(HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc;

    pdc = dc_HDC2PDC(hdc);

    rc = *prc;
    NormalizeRect(&rc);
    if (IsRectEmpty (&rc)) return;

    IntersectClipRect (&pdc->lcrgn, &rc);

    // Transfer logical to device to screen here.
    coor_LP2SP(pdc, &rc.left, &rc.top);
    coor_LP2SP(pdc, &rc.right, &rc.bottom);
    NormalizeRect(&rc);

    IntersectClipRect (&pdc->ecrgn, &rc);
}

void GUIAPI SelectClipRect(HDC hdc, const RECT* prc)
{
    PDC pdc;
    RECT rc;

    pdc = dc_HDC2PDC(hdc);

    if (prc) {
        rc = *prc;
        NormalizeRect(&rc);
        if (IsRectEmpty (&rc)) return;

        SetClipRgn (&pdc->lcrgn, &rc);
        coor_LP2SP(pdc, &rc.left, &rc.top);
        coor_LP2SP(pdc, &rc.right, &rc.bottom);
        NormalizeRect(&rc);
        
        if (!IntersectRect (&rc, &rc, &pdc->DevRC))
            return;
    }
    else {
        EmptyClipRgn (&pdc->lcrgn);
        rc = pdc->DevRC;
    }
    
    if (dc_IsGeneralHDC(hdc)) {
        dc_GenerateECRgn (pdc, TRUE);
    }
    else
        SetClipRgn (&pdc->ecrgn, &rc);
}

void GUIAPI SelectClipRegion (HDC hdc, const CLIPRGN* pRgn)
{
    PDC pdc;
    PCLIPRECT pCRect;
    PCLIPRGN pLcrgn;

    pdc = dc_HDC2PDC(hdc);
    pLcrgn = &pdc->lcrgn;

    pCRect = pRgn->head;
    if (pCRect) {
        SetClipRgn (pLcrgn, &pCRect->rc);

        pCRect = pCRect->next;
        while (pCRect) {
            SubtractClipRect (pLcrgn, &pCRect->rc);
            AddClipRect (pLcrgn, &pCRect->rc);
            pCRect = pCRect->next;
        }
    }
    else
        return;

    if (dc_IsGeneralHDC(hdc)) {
        dc_GenerateECRgn (pdc, TRUE);
    }
    else {
        // not implemented.
    }
}

void GUIAPI GetBoundsRect(HDC hdc, RECT* pRect)
{
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);

    *pRect = pdc->lcrgn.rcBound;
}

BOOL GUIAPI PtVisible (HDC hdc, int x, int y)
{
    PCLIPRECT pClipRect;
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);

    pClipRect = pdc->lcrgn.head;
    if (pClipRect == NULL)
        return PtInRect (&pdc->DevRC, x, y);
        
    while(pClipRect)
    {
        if(PtInRect(&(pClipRect->rc), x, y)) return TRUE;

        pClipRect = pClipRect->next;
    }

    return FALSE;
}

BOOL GUIAPI RectVisible(HDC hdc, const RECT* pRect)
{
    PCLIPRECT pClipRect;
    PDC pdc;
    RECT rc;

    rc = *pRect; 

    pdc = dc_HDC2PDC(hdc);

    pClipRect = pdc->lcrgn.head;
    if (pClipRect == NULL)
        return DoesIntersect (&pdc->DevRC, &rc);

    while(pClipRect)
    {
        if (DoesIntersect (&rc, &(pClipRect->rc)))
            return TRUE;

        pClipRect = pClipRect->next;
    }

    return FALSE;
}

