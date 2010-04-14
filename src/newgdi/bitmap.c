/* 
** $Id: bitmap.c,v 1.41 2003/09/17 03:36:26 weiym Exp $
**
** Bitmap operations of GDI.
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

/****************************** Bitmap Support *******************************/
void _dc_fillbox_clip (PDC pdc, const GAL_Rect* rect)
{
    PCLIPRECT cliprect;
    RECT eff_rc;

    cliprect = pdc->ecrgn.head;
    if (pdc->rop == ROP_SET) {
        while (cliprect) {
            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)) {
                SET_GAL_CLIPRECT (pdc, eff_rc);
                GAL_FillRect (pdc->surface, rect, pdc->cur_pixel);
            }
            cliprect = cliprect->next;
        }
    }
    else {
        pdc->step = 1;
        while (cliprect) {
            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)) {
                int _w = RECTW(eff_rc), _h = RECTH(eff_rc);
                pdc->move_to (pdc, eff_rc.left, eff_rc.top);
                while (_h--) {
                    pdc->draw_hline (pdc, _w);
                    pdc->cur_dst += pdc->surface->pitch;
                }
            }
            cliprect = cliprect->next;
        }
    }

}

void _dc_fillbox_bmp_clip (PDC pdc, const GAL_Rect* rect, BITMAP* bmp)
{
    PCLIPRECT cliprect;
    RECT eff_rc;

    cliprect = pdc->ecrgn.head;
    if (pdc->rop == ROP_SET) {
        while (cliprect) {
            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)) {
                SET_GAL_CLIPRECT (pdc, eff_rc);
                GAL_PutBox (pdc->surface, rect, bmp);
            }

            cliprect = cliprect->next;
        }
    }
    else {
        BYTE* row;
        int _w, _h;

        pdc->step = 1;
        while (cliprect) {
            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)) {

                pdc->move_to (pdc, eff_rc.left, eff_rc.top);
                row = bmp->bmBits + bmp->bmPitch * (eff_rc.top - rect->y)
                        + bmp->bmBytesPerPixel * (eff_rc.left - rect->x);

                _h = RECTH(eff_rc); _w = RECTW(eff_rc);
                while (_h--) {
                    pdc->put_hline (pdc, row, _w);
                    row += bmp->bmPitch;
                    _dc_step_y (pdc, 1);
                }
            }

            cliprect = cliprect->next;
        }
    }
}

void GUIAPI FillBox (HDC hdc, int x, int y, int w, int h)
{
    PDC pdc;
    GAL_Rect rect;

    if (w == 0 || h == 0)
        return;

    if (!(pdc = check_ecrgn (hdc)))
        return;

    if (w < 0) w = RECTW (pdc->DevRC);
    if (h < 0) h = RECTH (pdc->DevRC);

    /* Transfer logical to device to screen here. */
    w += x; h += y;
    coor_LP2SP (pdc, &x, &y);
    coor_LP2SP (pdc, &w, &h);
    SetRect (&pdc->rc_output, x, y, w, h);
    NormalizeRect (&pdc->rc_output);
    w = RECTW (pdc->rc_output); h = RECTH (pdc->rc_output);
    rect.x = x; rect.y = y; rect.w = w; rect.h = h;

    pdc->cur_pixel = pdc->brushcolor;
    pdc->cur_ban = NULL;
    pdc->step = 1;

    ENTER_DRAWING (pdc);

    _dc_fillbox_clip (pdc, &rect);
    
    LEAVE_DRAWING (pdc);
    UNLOCK_GCRINFO (pdc);
}

BOOL GUIAPI GetBitmapFromDC (HDC hdc, int x, int y, int w, int h, BITMAP* bmp)
{
    PDC pdc;
    GAL_Rect rect;
    int ret;

    pdc = dc_HDC2PDC (hdc);
    if (dc_IsGeneralDC (pdc)) {
        LOCK (&pdc->pGCRInfo->lock);
        dc_GenerateECRgn (pdc, FALSE);
    }

    w += x; h += y;
    coor_LP2SP (pdc, &x, &y);
    coor_LP2SP (pdc, &w, &h);
    SetRect (&pdc->rc_output, x, y, w, h);
    NormalizeRect (&pdc->rc_output);
    w = RECTW (pdc->rc_output); h = RECTH (pdc->rc_output);
    rect.x = x; rect.y = y; rect.w = w; rect.h = h;

    ENTER_DRAWING_NOCHECK (pdc);

    ret = GAL_GetBox (pdc->surface, &rect, bmp);

    LEAVE_DRAWING_NOCHECK (pdc);
    UNLOCK_GCRINFO (pdc);

    if (ret) return FALSE;
    return TRUE;
}

BOOL GUIAPI FillBoxWithBitmap (HDC hdc, int x, int y, int w, int h, const BITMAP* bmp)
{
    PDC pdc;
    BITMAP scaled;
    int sw = bmp->bmWidth, sh = bmp->bmHeight;
    GAL_Rect rect;

    if (bmp->bmWidth <= 0 || bmp->bmHeight <= 0 || bmp->bmBits == NULL)
        return FALSE;

    if (!(pdc = check_ecrgn (hdc)))
        return TRUE;

    if (w <= 0) w = sw;
    if (h <= 0) h = sh;

    // Transfer logical to device to screen here.
    w += x; h += y;
    coor_LP2SP (pdc, &x, &y);
    coor_LP2SP (pdc, &w, &h);
    SetRect (&pdc->rc_output, x, y, w, h);
    NormalizeRect (&pdc->rc_output);
    w = RECTW (pdc->rc_output); h = RECTH (pdc->rc_output);
    rect.x = x; rect.y = y; rect.w = w; rect.h = h;

    if (w == sw && h == sh)
        scaled = *bmp;
    else {
        scaled = *bmp;
        scaled.bmWidth = w;
        scaled.bmHeight = h;
        if ((scaled.bmBits = malloc (GAL_GetBoxSize (pdc->surface, w, h, &scaled.bmPitch))) == NULL)
            goto error_ret;

        ScaleBitmap (&scaled, bmp);
    }

    pdc->step = 1;
    pdc->cur_ban = NULL;
    pdc->cur_pixel = pdc->brushcolor;
    pdc->skip_pixel = scaled.bmColorKey;

    ENTER_DRAWING (pdc);

    _dc_fillbox_bmp_clip (pdc, &rect, &scaled);

    LEAVE_DRAWING (pdc);

    if (w != sw || h != sh)
        free (scaled.bmBits);

error_ret:
    UNLOCK_GCRINFO (pdc);

    if (!scaled.bmBits)
        return FALSE;

    return TRUE;
}

BOOL GUIAPI FillBoxWithBitmapPart (HDC hdc, int x, int y, int w, int h,
                int bw, int bh, const BITMAP* bmp, int xo, int yo)
{
    PDC pdc;
    BYTE* my_bits;
    BITMAP scaled;
    GAL_Rect rect;

    if (bmp->bmWidth <= 0 || bmp->bmHeight <= 0 || bmp->bmBits == NULL)
        return FALSE;

    if (!(pdc = check_ecrgn (hdc)))
        return TRUE;

    // Transfer logical to device to screen here.
    w += x; h += y;
    coor_LP2SP(pdc, &x, &y);
    coor_LP2SP(pdc, &w, &h);
    SetRect (&pdc->rc_output, x, y, w, h);
    NormalizeRect (&pdc->rc_output);
    w = RECTW (pdc->rc_output); h = RECTH (pdc->rc_output);
    rect.x = x; rect.y = y; rect.w = w; rect.h = h;

    if (bw <= 0) bw = bmp->bmWidth;
    if (bh <= 0) bh = bmp->bmHeight;

    if (bw == bmp->bmWidth && bh == bmp->bmHeight) {
        scaled = *bmp;
        my_bits = NULL;
    }
    else {
        scaled = *bmp;
        scaled.bmWidth = bw;
        scaled.bmHeight = bh;
        if ((my_bits = malloc (GAL_GetBoxSize (pdc->surface, bw, bh, &scaled.bmPitch))) == NULL)
            goto error_ret;

        scaled.bmBits = my_bits;
        ScaleBitmap (&scaled, bmp);
    }

    if (xo != 0 || yo != 0) {
        scaled.bmBits += scaled.bmPitch * yo + xo * GAL_BytesPerPixel (pdc->surface);
    }

    pdc->step = 1;
    pdc->cur_ban = NULL;
    pdc->cur_pixel = pdc->brushcolor;
    pdc->skip_pixel = scaled.bmColorKey;

    ENTER_DRAWING (pdc);

    _dc_fillbox_bmp_clip (pdc, &rect, &scaled);

    LEAVE_DRAWING (pdc);

    if (bw != bmp->bmWidth || bh != bmp->bmWidth)
        free (my_bits);

error_ret:
    UNLOCK_GCRINFO (pdc);

    if (!my_bits)
        return FALSE;

    return TRUE;
}

void GUIAPI BitBlt (HDC hsdc, int sx, int sy, int sw, int sh,
                   HDC hddc, int dx, int dy, DWORD dwRop)
{
    PCLIPRECT cliprect;
    PDC psdc, pddc;
    RECT srcOutput, dstOutput;
    GAL_Rect dst, src;
    RECT eff_rc;

    psdc = dc_HDC2PDC (hsdc);
    if (!(pddc = check_ecrgn (hddc)))
        return;

    if (sw <= 0 || sh <= 0) {
        sw = RECTW (psdc->DevRC);
        sh = RECTH (psdc->DevRC);
    }

    // Transfer logical to device to screen here.
    sw += sx; sh += sy;
    coor_LP2SP (psdc, &sx, &sy);
    coor_LP2SP (psdc, &sw, &sh);
    SetRect (&srcOutput, sx, sy, sw, sh);
    NormalizeRect (&srcOutput);
    (sw > sx) ? (sw -= sx) : (sw = sx - sw);
    (sh > sy) ? (sh -= sy) : (sh = sy - sh);
    coor_LP2SP (pddc, &dx, &dy);
    SetRect (&dstOutput, dx, dy, dx + sw, dy + sh);
    NormalizeRect (&dstOutput);

    if (!dc_IsMemDC (psdc) && !dc_IsMemDC (pddc))
        GetBoundRect (&pddc->rc_output, &srcOutput, &dstOutput);
    else
        pddc->rc_output = dstOutput;

    ENTER_DRAWING (pddc);
    if (dc_IsMemDC (pddc) && !dc_IsMemDC (psdc)) ShowCursorForGDI (FALSE, &srcOutput);

    if (pddc->surface == psdc->surface && dy > sy)
        cliprect = pddc->ecrgn.tail;
    else
        cliprect = pddc->ecrgn.head;
    while (cliprect) {
        if (IntersectRect (&eff_rc, &pddc->rc_output, &cliprect->rc)) {
            SET_GAL_CLIPRECT (pddc, eff_rc);

            src.x = sx; src.y = sy; src.w = sw; src.h = sh;
            dst.x = dx; dst.y = dy; dst.w = sw; dst.h = sh;
            GAL_BlitSurface (psdc->surface, &src, pddc->surface, &dst);
        }

        if (pddc->surface == psdc->surface && dy > sy)
            cliprect = cliprect->prev;
        else
            cliprect = cliprect->next;
    }

    if (dc_IsMemDC (pddc) && !dc_IsMemDC (psdc)) ShowCursorForGDI (TRUE, &srcOutput);
    LEAVE_DRAWING (pddc);

    UNLOCK_GCRINFO (pddc);
}

void GUIAPI StretchBlt (HDC hsdc, int sx, int sy, int sw, int sh,
                       HDC hddc, int dx, int dy, int dw, int dh, DWORD dwRop)
{
    PDC psdc, pddc;
    PCLIPRECT cliprect;
    RECT srcOutput, dstOutput;
    GAL_Rect src, dst;
    RECT eff_rc;

    psdc = dc_HDC2PDC (hsdc);
    if (!(pddc = check_ecrgn (hddc)))
        return;

    if (sw <= 0 || sh <= 0) {
        sw = RECTW (psdc->DevRC);
        sh = RECTH (psdc->DevRC);
    }
    if (dw <= 0 || dh <= 0) {
        dw = RECTW (pddc->DevRC);
        dh = RECTH (pddc->DevRC);
    }
    // Transfer logical to device to screen here.
    sw += sx; sh += sy;
    coor_LP2SP(psdc, &sx, &sy);
    coor_LP2SP(psdc, &sw, &sh);
    SetRect (&srcOutput, sx, sy, sw, sh);
    NormalizeRect (&srcOutput);
    sw = RECTW (srcOutput); sh = RECTH (srcOutput);

    dw += dx; dh += dy;
    coor_LP2SP (pddc, &dx, &dy);
    coor_LP2SP (pddc, &dw, &dh);
    SetRect (&dstOutput, dx, dy, dw, dh);
    NormalizeRect (&dstOutput);
    dw = RECTW (dstOutput); dh = RECTH (dstOutput);

    if (!dc_IsMemDC (psdc) && !dc_IsMemDC (pddc))
        GetBoundRect (&pddc->rc_output, &srcOutput, &dstOutput);
    else
        pddc->rc_output = dstOutput;

    ENTER_DRAWING (pddc);
    if (!dc_IsMemDC (psdc) && dc_IsMemDC (pddc)) ShowCursorForGDI (FALSE, &srcOutput);

    if (pddc->surface == psdc->surface && dy > sy)
        cliprect = pddc->ecrgn.tail;
    else
        cliprect = pddc->ecrgn.head;
    while(cliprect) {
        if (IntersectRect (&eff_rc, &pddc->rc_output, &cliprect->rc)) {
            SET_GAL_CLIPRECT (pddc, eff_rc);

            src.x = sx; src.y = sy; src.w = sw; src.h = sh;
            dst.x = dx; dst.y = dy; dst.w = dw; dst.h = dh;
            GAL_SoftStretch (psdc->surface, &src, pddc->surface, &dst);
        }

        if (pddc->surface == psdc->surface && dy > sy)
            cliprect = cliprect->prev;
        else
            cliprect = cliprect->next;
    }
    if (!dc_IsMemDC (psdc) && dc_IsMemDC (pddc)) ShowCursorForGDI (TRUE, &srcOutput);
    LEAVE_DRAWING (pddc);

    UNLOCK_GCRINFO (pddc);
}

/* 
 * This function performs a fast box scaling.
 * This is a DDA-based algorithm; Iteration over target bitmap.
 *
 * This function comes from SVGALib, Copyright 1993 Harm Hanemaayer
 */

/* We use the 32-bit to 64-bit multiply and 64-bit to 32-bit divide of the
 * 386 (which gcc doesn't know well enough) to efficiently perform integer
 * scaling without having to worry about overflows.
 */

#if defined(__GNUC__) && defined(i386)

static inline int muldiv64(int m1, int m2, int d)
{
    /* int32 * int32 -> int64 / int32 -> int32 */
    int result;
    int dummy;
    __asm__ __volatile__ (
               "imull %%edx\n\t"
               "idivl %4\n\t"
  :               "=a"(result), "=d"(dummy)        /* out */
  :               "0"(m1), "1"(m2), "g"(d)                /* in */
               /***rjr***:               "ax", "dx"***/        /* mod */
        );
    return result;
}

#else

static inline int muldiv64 (int m1, int m2, int d)
{
    long long int mul = (long long int) m1 * m2;

    return (int) (mul / d);
}

#endif

BOOL ScaleBitmap (BITMAP *dst, const BITMAP *src)
{
    BYTE *dp1 = src->bmBits;
    BYTE *dp2 = dst->bmBits;
    int xfactor;
    int yfactor;

    if (dst->bmWidth == 0 || dst->bmHeight == 0)
        return TRUE;

    if (dst->bmBytesPerPixel != src->bmBytesPerPixel)
        return FALSE;

    xfactor = muldiv64 (src->bmWidth, 65536, dst->bmWidth);         /* scaled by 65536 */
    yfactor = muldiv64 (src->bmHeight, 65536, dst->bmHeight);       /* scaled by 65536 */

    switch (dst->bmBytesPerPixel) {
    case 1:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
#if 0 // defined(__GNUC__) && defined(i386)
                while (x < dst->bmWidth - 8) {
                    /* This saves just a couple of cycles per */
                    /* pixel on a 486, but I couldn't resist. */
                    __asm__ __volatile__("movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 1 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 2 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 3 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 4 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 5 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 6 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 7 (%1, %2)\n\t"
                                         :        /* output */
                                         :        /* input */
                                         "ax"(0), "r"(dp2), "r"(x), "r"(dp1),
                                         "r"(sx), "r"(xfactor)
                                         :"ax", "4"
                    );
                    *(dp2 + x) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 1) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 2) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 3) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 4) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 5) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 6) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 7) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    x += 8;
                }
#endif
                while (x < dst->bmWidth) {
                    *(dp2 + x) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    case 2:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < dst->bmWidth) {
                    *(unsigned short *) (dp2 + x * 2) =
                        *(unsigned short *) (dp1 + (sx >> 16) * 2);
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    case 3:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < dst->bmWidth) {
                    *(unsigned short *) (dp2 + x * 3) =
                        *(unsigned short *) (dp1 + (sx >> 16) * 3);
                    *(unsigned char *) (dp2 + x * 3 + 2) =
                        *(unsigned char *) (dp1 + (sx >> 16) * 3 + 2);
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    case 4:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < dst->bmWidth) {
                    *(unsigned *) (dp2 + x * 4) =
                        *(unsigned *) (dp1 + (sx >> 16) * 4);
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    }

    return TRUE;
}

gal_pixel GUIAPI GetPixelInBitmap (const BITMAP* bmp, int x, int y)
{
    BYTE* dst;

    if (x < 0 || y < 0 || x >= bmp->bmWidth || y >= bmp->bmHeight)
        return 0;

    dst = bmp->bmBits + y * bmp->bmPitch + x * bmp->bmBytesPerPixel;
    return _mem_get_pixel (dst, bmp->bmBytesPerPixel);
}

BOOL GUIAPI SetPixelInBitmap (const BITMAP* bmp, int x, int y, gal_pixel pixel)
{
    BYTE* dst;

    if (x < 0 || y < 0 || x >= bmp->bmWidth || y >= bmp->bmHeight)
        return FALSE;

    dst = bmp->bmBits + y * bmp->bmPitch + x * bmp->bmBytesPerPixel;
    _mem_set_pixel (dst, bmp->bmBytesPerPixel, pixel);
    return TRUE;
}

// This function expand monochorate bitmap.
void GUIAPI ExpandMonoBitmap (HDC hdc, BYTE* bits, Uint32 pitch, const BYTE* my_bits, Uint32 my_pitch,
                Uint32 w, Uint32 h, DWORD flags, Uint32 bg, Uint32 fg)
{
    Uint32 x, y;
    BYTE *dst, *dst_line;
    const BYTE *src, *src_line;
    Uint32 pixel;
    int bpp = GAL_BytesPerPixel (dc_HDC2PDC (hdc)->surface);
    BYTE byte = 0;

    dst_line = bits;
    if (flags & MYBMP_FLOW_UP)
        src_line = my_bits + my_pitch * (h - 1);
    else
        src_line = my_bits;

    // expand bits here.
    for (y = 0; y < h; y++) {
        src = src_line;
        dst = dst_line;

        for (x = 0; x < w; x++) {
            if (x % 8 == 0)
                byte = *src++;

            if ((byte & (128 >> (x % 8))))   /* pixel */
                pixel = fg;
            else
                pixel = bg;

            dst = _mem_set_pixel (dst, bpp, pixel);
        }
        
        if (flags & MYBMP_FLOW_UP)
            src_line -= my_pitch;
        else
            src_line += my_pitch;

        dst_line += pitch;
    }
}

static const RGB WindowsStdColor [] = {
    {0x00, 0x00, 0x00},     // black         --0
    {0x80, 0x00, 0x00},     // dark red      --1
    {0x00, 0x80, 0x00},     // dark green    --2
    {0x80, 0x80, 0x00},     // dark yellow   --3
    {0x00, 0x00, 0x80},     // dark blue     --4
    {0x80, 0x00, 0x80},     // dark magenta  --5
    {0x00, 0x80, 0x80},     // dark cyan     --6
    {0xC0, 0xC0, 0xC0},     // light gray    --7
    {0x80, 0x80, 0x80},     // dark gray     --8
    {0xFF, 0x00, 0x00},     // red           --9
    {0x00, 0xFF, 0x00},     // green         --10
    {0xFF, 0xFF, 0x00},     // yellow        --11
    {0x00, 0x00, 0xFF},     // blue          --12
    {0xFF, 0x00, 0xFF},     // magenta       --13
    {0x00, 0xFF, 0xFF},     // cyan          --14
    {0xFF, 0xFF, 0xFF},     // light white   --15
};

// This function expand 16-color bitmap.
void GUIAPI Expand16CBitmap (HDC hdc, BYTE* bits, Uint32 pitch, const BYTE* my_bits, Uint32 my_pitch,
                Uint32 w, Uint32 h, DWORD flags, const RGB* pal)
{
    PDC pdc;
    Uint32 x, y;
    BYTE *dst, *dst_line;
    const BYTE *src, *src_line;
    int index, bpp;
    Uint32 pixel;
    BYTE byte = 0;

    pdc = dc_HDC2PDC(hdc);
    bpp = GAL_BytesPerPixel (pdc->surface);

    dst_line = bits;
    if (flags & MYBMP_FLOW_UP)
        src_line = my_bits + my_pitch * (h - 1);
    else
        src_line = my_bits;

    for (y = 0; y < h; y++) {
        src = src_line;
        dst = dst_line;
        for (x = 0; x < w; x++) {
            if (x % 2 == 0) {
                byte = *src++;
                index = (byte >> 4) & 0x0f;
            }
            else
                index = byte & 0x0f;

            if (pal)
                pixel = GAL_MapRGB (pdc->surface->format, pal[index].r, pal[index].g, pal[index].b);
            else
                pixel = GAL_MapRGB (pdc->surface->format, 
                                WindowsStdColor[index].r, WindowsStdColor[index].g, WindowsStdColor[index].b);

            dst = _mem_set_pixel (dst, bpp, pixel);
        }

        if (flags & MYBMP_FLOW_UP)
            src_line -= my_pitch;
        else
            src_line += my_pitch;

        dst_line += pitch;
    }
}

// This function expands 256-color bitmap.
void GUIAPI Expand256CBitmap (HDC hdc, BYTE* bits, Uint32 pitch, const BYTE* my_bits, Uint32 my_pitch,
                Uint32 w, Uint32 h, DWORD flags, const RGB* pal)
{
    PDC pdc;
    Uint32 x, y;
    int bpp;
    BYTE *dst, *dst_line;
    const BYTE *src, *src_line;
    Uint32 pixel;
    BYTE byte;

    pdc = dc_HDC2PDC(hdc);
    bpp = GAL_BytesPerPixel (pdc->surface);

    dst_line = bits;
    if (flags & MYBMP_FLOW_UP)
        src_line = my_bits + my_pitch * (h - 1);
    else
        src_line = my_bits;

    for (y = 0; y < h; y++) {
        src = src_line;
        dst = dst_line;
        for (x = 0; x < w; x++) {
            byte = *src++;

            if (pal)
                pixel = GAL_MapRGB (pdc->surface->format, pal[byte].r, pal[byte].g, pal[byte].b);
            else
                pixel = GAL_MapRGB (pdc->surface->format, byte, byte, byte);

            dst = _mem_set_pixel (dst, bpp, pixel);
        }

        if (flags & MYBMP_FLOW_UP)
            src_line -= my_pitch;
        else
            src_line += my_pitch;

        dst_line += pitch;
    }
}

// This function compile a RGBA bitmap
void GUIAPI CompileRGBABitmap (HDC hdc, BYTE* bits, Uint32 pitch, const BYTE* my_bits, Uint32 my_pitch,
                Uint32 w, Uint32 h, DWORD flags, void* pixel_format)
{
    PDC pdc;
    Uint32 x, y;
    int bpp;
    BYTE *dst, *dst_line;
    const BYTE *src, *src_line;
    Uint32 pixel;
    GAL_Color rgb;

    pdc = dc_HDC2PDC(hdc);
    bpp = GAL_BytesPerPixel (pdc->surface);

    dst_line = bits;
    if (flags & MYBMP_FLOW_UP)
        src_line = my_bits + my_pitch * (h - 1);
    else
        src_line = my_bits;

    // expand bits here.
    for (y = 0; y < h; y++) {
        src = src_line;
        dst = dst_line;
        for (x = 0; x < w; x++) {
            if ((flags & MYBMP_TYPE_MASK) == MYBMP_TYPE_BGR) {
                rgb.b = *src++;
                rgb.g = *src++;
                rgb.r = *src++;
            }
            else {
                rgb.r = *src++;
                rgb.g = *src++;
                rgb.b = *src++;
            }

            if (flags & MYBMP_RGBSIZE_4) {
                if (flags & MYBMP_ALPHA) {
                    rgb.a = *src;
                    pixel = GAL_MapRGBA ((GAL_PixelFormat*) pixel_format, rgb.r, rgb.g, rgb.b, rgb.a);
                }
                else {
                    pixel = GAL_MapRGB (pdc->surface->format, rgb.r, rgb.g, rgb.b);
                }
                src++;
            }
            else {
                pixel = GAL_MapRGB (pdc->surface->format, rgb.r, rgb.g, rgb.b);
            }
            
            dst = _mem_set_pixel (dst, bpp, pixel);
        }

        if (flags & MYBMP_FLOW_UP)
            src_line -= my_pitch;
        else
            src_line += my_pitch;

        dst_line += pitch;
    }
}

// This function replaces one color with specified color.
void GUIAPI ReplaceBitmapColor (HDC hdc, BITMAP* bmp, gal_pixel iOColor, gal_pixel iNColor)
{
    PDC pdc;
    int w, h, i;
    BYTE* line, *bits;
    int bpp;

    pdc = dc_HDC2PDC (hdc);
    bpp = GAL_BytesPerPixel (pdc->surface);

    h = bmp->bmHeight;
    w = bmp->bmWidth;
    line = bmp->bmBits;
    switch (bpp) {
        case 1:
            while (h--) {
                bits = line;
                for (i = 0; i < w; i++) {
                    if (*bits == iOColor)
                        *bits = iNColor;
                    bits++;
                }
                line += bmp->bmPitch;
            }
            break;
        case 2:
            while (h--) {
                bits = line;
                for (i = 0; i < w; i++) {
                    if( *(Uint16 *) bits == iOColor)
                        *(Uint16 *) bits = iNColor;
                    bits += 2;
                }
                line += bmp->bmPitch;
            }
            break;
        case 3: 
            while (h--) {
                bits = line;
                for (i = 0; i < w; i++) {
                    if ((*(Uint16 *) bits == iOColor) 
                           && (*(bits + 2) == (iOColor >> 16)) )
                    {
                        *(Uint16 *) bits = iNColor;
                        *(bits + 2) = iNColor >> 16;
                    }
                    bits += 3;
                }
                line += bmp->bmPitch;
            }
            break;
        case 4:    
            while (h--) {
                bits = line;
                for (i = 0; i < w; i++) {
                    if( *(Uint32 *) bits == iOColor )
                        *(Uint32 *) bits = iNColor;
                    bits += 4;
                }
                line += bmp->bmPitch;
            }
            break;
    }
}

