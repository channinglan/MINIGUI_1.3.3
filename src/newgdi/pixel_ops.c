/*
** $Id: pixel_ops.c,v 1.11 2003/09/04 06:02:53 weiym Exp $
**
** pixel_ops.c: pixel, horizontal, and vertical line operations
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
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

BOOL _dc_cliphline (const RECT* cliprc, int* px, int* py, int* pw)
{
    if ( (*px >= cliprc->right) || (*py >= cliprc->bottom) || 
            (*px + *pw < cliprc->left) || (*py < cliprc->top) )
        return FALSE;

    if ( (*px >= cliprc->left) && (*py >= cliprc->top) && 
            (*px + *pw < cliprc->right) && (*py < cliprc->bottom) )
        return TRUE;
            
    if (*px < cliprc->left) {
        *pw -= cliprc->left - *px;
        *px = cliprc->left;
    }
    if (*px + *pw - 1 >= cliprc->right)    
        *pw = cliprc->right - *px;

    if (*pw <= 0)
        return FALSE;

    return TRUE;        
}

BOOL _dc_clipvline (const RECT* cliprc, int* px, int* py, int* ph)
{
    if ( (*px >= cliprc->right) || (*py >= cliprc->bottom) || 
            (*px < cliprc->left) || (*py + *ph < cliprc->top) )
        return FALSE;

    if ( (*px >= cliprc->left) && (*py >= cliprc->top) && 
            (*px < cliprc->right) && (*py + *ph < cliprc->bottom) )
        return TRUE;
            
    if (*py < cliprc->top) {
        *ph -= cliprc->top - *py;
        *py = cliprc->top;
    }
    if (*py + *ph >= cliprc->bottom)    
        *ph = cliprc->bottom - *py;

    if (*ph <= 0)
        return FALSE;

    return TRUE;        
}

void _dc_drawvline (PDC pdc, int h)
{
    while (h > 0) {
        pdc->set_pixel (pdc);
        _dc_step_y (pdc, pdc->step);
        h -= pdc->step;
    }
}

/* find the ban in which the scanline lies */
static PCLIPRECT _dc_which_region_ban (PDC pdc, int y)
{
    CLIPRGN* region = &pdc->ecrgn;
    PCLIPRECT cliprect;

    /* check with bounding rect of clipping region */
    if (y >= region->tail->rc.bottom || y < region->head->rc.top) {
        pdc->cur_ban = NULL;
        goto ret;
    }

    if (pdc->cur_ban)
        cliprect = pdc->cur_ban;
    else
        cliprect = region->head;

    /* find the ban in which this point lies */
    if (y < cliprect->rc.top) { /* backward */
        while (cliprect && y < cliprect->rc.top) {
            cliprect = cliprect->prev;
        }

        if (cliprect) {
            int top;
            top = cliprect->rc.top;
            while (cliprect && top == cliprect->rc.top) {
                cliprect = cliprect->prev;
            }

            if (cliprect)
                cliprect = cliprect->next;
            else
                cliprect = region->head;
        }
    }
    else { /* forward */
        while (cliprect && y >= cliprect->rc.bottom) {
            cliprect = cliprect->next;
        }
    }

    pdc->cur_ban = cliprect;
ret:
    return pdc->cur_ban;
}

#define INRECT(r, x, y) \
      ( ( ((r).right >  x)) && \
      ( ((r).left <= x)) && \
      ( ((r).bottom >  y)) && \
      ( ((r).top <= y)) )

void _dc_set_pixel_clip (void* context, int x, int y)
{
    PDC pdc = (PDC)context;
    PCLIPRECT cliprect;

    if (_dc_which_region_ban (pdc, y)) {
        int top;

        /* draw in this ban */
        cliprect = pdc->cur_ban;
        top = cliprect->rc.top;
        while (cliprect && cliprect->rc.top == top) {
            if (INRECT (pdc->rc_output, x, y) && INRECT (cliprect->rc, x, y)) {
                pdc->move_to (pdc, x, y);
                pdc->set_pixel (pdc);
                break;
            }

            cliprect = cliprect->next;
        }
    }
}

void _dc_set_pixel_noclip (void* context, int stepx, int stepy)
{
    PDC pdc = (PDC)context;

    if (stepx) pdc->step_x (pdc, stepx);
    if (stepy) _dc_step_y (pdc, stepy);

    pdc->set_pixel (pdc);
}

void _dc_set_pixel_pair_clip (void* context, int x1, int x2, int y)
{
    PDC pdc = (PDC)context;
    PCLIPRECT cliprect;
    BOOL first_drawn = FALSE;

    if (x1 > x2) {
        int tmp = x2;
        x2 = x1;
        x1 = tmp;
    }

    if (_dc_which_region_ban (pdc, y)) {
        int top;

        /* draw in this ban */
        cliprect = pdc->cur_ban;
        top = cliprect->rc.top;
        while (cliprect && cliprect->rc.top == top) {
            if (INRECT (pdc->rc_output, x1, y) && INRECT (cliprect->rc, x1, y)) {
                pdc->move_to (pdc, x1, y);
                pdc->set_pixel (pdc);
                first_drawn = TRUE;
                break;
            }

            cliprect = cliprect->next;
        }

        if (x1 != x2) {
            if (!first_drawn) cliprect = pdc->cur_ban;
            while (cliprect && cliprect->rc.top == top) {
                if (INRECT (pdc->rc_output, x2, y) && INRECT (cliprect->rc, x2, y)) {
                    pdc->move_to (pdc, x2, y);
                    pdc->set_pixel (pdc);
                    break;
                }

                cliprect = cliprect->next;
            }
        }
    }
}

void _dc_draw_hline_clip (void* context, int x1, int x2, int y)
{
    PDC pdc = (PDC)context;
    PCLIPRECT cliprect;
    int top, w;

    if (x1 == x2) {
        if (!(x1 % pdc->step))
            _dc_set_pixel_clip (pdc, x1, y);
        return;
    }

    if (x1 > x2) {
        int tmp = x2;
        x2 = x1;
        x1 = tmp;
    }

    if (_dc_which_region_ban (pdc, y)) {
        /* draw this horizontal line in this ban */
        cliprect = pdc->cur_ban;
        top = cliprect->rc.top;
        w = x2 - x1 + 1;
        while (cliprect && cliprect->rc.top == top) {
            RECT eff_rc;
            int _x = x1, _y = y, _w = w;

            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)
                    && _dc_cliphline (&eff_rc, &_x, &_y, &_w)) {
                /* hack for dot line */
                _x += (_x % pdc->step);
                _w -= (_x % pdc->step) * 2;

                if (_w > 0) {
                    pdc->move_to (pdc, _x, _y);
                    pdc->draw_hline (pdc, _w);
                }
            }

            cliprect = cliprect->next;
        }
    }
}

void _dc_draw_vline_clip (void* context, int y1, int y2, int x)
{
    PDC pdc = (PDC)context;
    PCLIPRECT cliprect;
    int top, h;

    if (y1 == y2) {
        if (!(y1 % pdc->step))
            _dc_set_pixel_clip (pdc, x, y1);
        return;
    }

    if (y1 > y2) {
        int tmp = y2;
        y2 = y1;
        y1 = tmp;
    }

    /* check with bounding rect of clipping region */
    if (y1 >= pdc->ecrgn.tail->rc.bottom || y2 < pdc->ecrgn.head->rc.top) {
        return;
    }

    cliprect = pdc->ecrgn.head;
    do {
        /* find the next ban intersects with this vertical line */
        while (cliprect && y1 >= cliprect->rc.bottom) {
            cliprect = cliprect->next;
        }

        if (!cliprect) return;

        /* find the clipping rect intersects with this vertical line in this ban */
        top = cliprect->rc.top;
        while (cliprect && cliprect->rc.top == top) {
            int _y, _h;
            RECT eff_rc;
            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)
                    && (x >= eff_rc.left && x < eff_rc.right)) {

                if (y1 < eff_rc.top)
                    y1 = eff_rc.top;

                if (y2 >= eff_rc.bottom)
                    h = eff_rc.bottom - y1;
                else
                    h = y2 - y1;

                /* hack for dot line */
                _y = y1 + (y1 % pdc->step);
                _h = h - (y1 % pdc->step) * 2;

                if (_h > 0) {
                    pdc->move_to (pdc, x, _y);
                    _dc_drawvline (pdc, _h);
                }
                
                if (h > 0)
                    y1 += h;
            }

            cliprect = cliprect->next;
        }

    } while (cliprect && y2 > y1);
}

