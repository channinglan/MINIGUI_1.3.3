/*
** $Id: polygon.c,v 1.11 2003/09/04 06:02:53 weiym Exp $
**
** polygon.c: monoton vertical polygon and general polygon generators.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
**      Monotone vertical polygon generator comes from 
**      "Michael Abrash's Graphics Programming Black Book Special Edition"
**      by Michael Abrash.
**
**      General polygon generator comes from Allegro by 
**      Shawn Hargreaves and others. 
**      Thank them for their great work and good copyright statement.
**
**      "Allegro is a gift-software"
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/10/31.
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
 * Modify records:
 *
 *  Who             When        Where       For What                Status
 *-----------------------------------------------------------------------------
 *
 * TODO:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "fixedmath.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"
#include "pixel_ops.h"
#include "cursor.h"
#include "polygon.h"

/* 
 * Returns TRUE if polygon described by passed-in vertex list is 
 * monotone with respect to a vertical line, FALSE otherwise. 
 * Doesn't matter if polygon is simple (non-self-intersecting) or not.
 */

#define SIGNUM(a) ((a>0)?1:((a<0)?-1:0))

BOOL GUIAPI PolygonIsMonotoneVertical (const POINT* pts, int vertices)
{
    int i, delta_y, prev_delta_y;
    int nr_yreversals = 0;

    /* Three or fewer points can't make a non-vertical-monotone polygon */
    if (vertices < 4)
        return TRUE;

    /* Scan to the first non-horizontal edge */
    prev_delta_y = SIGNUM(pts[vertices-1].y - pts[0].y);
    i = 0;
    while ((prev_delta_y == 0) && (i < (vertices-1))) {
        prev_delta_y = SIGNUM(pts[i].y - pts[i+1].y);
        i++;
    }

    if (i == (vertices-1))
        return TRUE;  /* polygon is a flat line */

    /* 
     * Now count Y reversals. Might miss one reversal, at the last vertex, 
     * but because reversal counts must be even, being off by one 
     * isn't a problem 
     */
    do {
        if ((delta_y = SIGNUM(pts[i].y - pts[i+1].y)) != 0) {
            if (delta_y != prev_delta_y) {
                /* Switched Y direction; not vertical-monotone if
                   reversed Y direction as many as three times */
                if (++nr_yreversals > 2)
                    return FALSE;
                prev_delta_y = delta_y;
            }
        }
    } while (i++ < (vertices-1));

   return TRUE;  /* it's a vertical-monotone polygon */
}

typedef struct _hline {
    int x1, x2;
} HLINE;

/* 
 * Scan converts an edge from (x1, y1) to (x2, y2), not including 
 * the point at (x2, y2). This avoids overlapping the end of 
 * one line with the start of the next, and causes the bottom 
 * scan line of the polygon not to be drawn. If skip_first != 0, 
 * the point at (x1, y1) isn't drawn. For each scan line, 
 * the pixel closest to the scanned line without being to the 
 * left of the scanned line is chosen
 */ 

static void scan_edge (int x1, int y1, int x2, int y2, int set_x1, int skip_first, HLINE** hlines_p)
{
#if 1
    int y, delta_x, delta_y, dir;
    fixed inverse_slope;
    HLINE* hlines;

    /* Calculate x and y lengths of the line and the inverse slope */
    delta_x = x2 - x1;
    if ((delta_y = y2 - y1) <= 0)
        return;     /* guard against 0-length and horizontal edges */
    if (delta_x < 0) {
        dir = -1;
        inverse_slope = fdiv (itofix (-delta_x), itofix (delta_y));
    }
    else {
        dir = 1;
        inverse_slope = fdiv (itofix (delta_x), itofix (delta_y));
    }

    /* 
     * Store the x coordinate of the pixel closest to but not to 
     * the left of the line for each y coordinate between y1 and y2, 
     * not including y2 and also not including y1 if skip_first != 0 
     */

    hlines = *hlines_p;
    for (y = y1 + skip_first; y < y2; y++, hlines++) {
        if (set_x1)
            hlines->x1 = x1 + fceil (fmul (itofix (y-y1), inverse_slope)) * dir;
        else
            hlines->x2 = x1 + fceil (fmul (itofix (y-y1), inverse_slope)) * dir;
    }
    *hlines_p = hlines;
#else
    int y, delta_x, delta_y;
    double inverse_slope;
    HLINE* hlines;

    /* Calculate x and y lengths of the line and the inverse slope */
    delta_x = x2 - x1;
    if ((delta_y = y2 - y1) <= 0)
        return;     /* guard against 0-length and horizontal edges */
    inverse_slope = (double)delta_x / (double)delta_y;

    /* 
     * Store the x coordinate of the pixel closest to but not to 
     * the left of the line for each y coordinate between y1 and y2, 
     * not including y2 and also not including y1 if skip_first != 0 
     */

    hlines = *hlines_p;
    for (y = y1 + skip_first; y < y2; y++, hlines++) {
        if (set_x1)
            hlines->x1 = x1 + (int)ceil ((y-y1) * inverse_slope);
        else
            hlines->x2 = x1 + (int)ceil ((y-y1) * inverse_slope);
    }
    *hlines_p = hlines;
#endif
}

/* 
 * "Monoton vertical" means "monotone with respect to a vertical line"; 
 * that is, every horizontal line drawn through the polygon at any point 
 * would cross exactly two active edges (neither horizontal lines 
 * nor zero-length edges count as active edges; both are acceptable 
 * anywhere in the polygon). Right & left edges may cross (polygons may be nonsimple). 
 * Polygons that are not convex according to this definition won't be drawn properly. 
 */

/* Advances the index by one vertex forward through the vertex list,
wrapping at the end of the list */
#define INDEX_FORWARD(Index) \
    Index = (Index + 1) % vertices;

/* Advances the index by one vertex backward through the vertex list,
wrapping at the start of the list */
#define INDEX_BACKWARD(Index) \
    Index = (Index - 1 + vertices) % vertices;

/* Advances the index by one vertex either forward or backward through
the vertex list, wrapping at either end of the list */
#define INDEX_MOVE(Index,Direction)                 \
    if (Direction > 0)                              \
        Index = (Index + 1) % vertices;             \
    else                                            \
        Index = (Index - 1 + vertices) % vertices;

/* Monotone vertical polygon generator */
BOOL GUIAPI MonotoneVerticalPolygonGenerator (void* context, const POINT* pts, int vertices, CB_POLYGON cb)
{
    int i, min_index, max_index, min_point_y, max_point_y;
    int cur_index, prev_index;
    HLINE *hlines, *working_hlines;
    int nr_hlines;

    if (vertices < 3)
        return TRUE;  /* reject null polygons */

    /* Scan the list to find the top and bottom of the polygon */
    max_point_y = min_point_y = pts[min_index = max_index = 0].y;
    for (i = 1; i < vertices; i++) {
        if (pts[i].y < min_point_y)
            min_point_y = pts[min_index = i].y; /* new top */
        else if (pts[i].y > max_point_y)
            max_point_y = pts[max_index = i].y; /* new bottom */
    }

    /* Set the # of scan lines in the polygon, skipping the bottom edge */
    if ((nr_hlines = max_point_y - min_point_y) <= 0)
        return TRUE;  /* there's nothing to draw, so we're done */

    /* Get memory in which to store the line list we generate */
    if ((hlines = (HLINE*) (malloc (sizeof (HLINE) * nr_hlines))) == NULL)
        return FALSE;  /* couldn't get memory for the line list */

    /* Scan the first edge and store the boundary points in the list */
    /* Initial pointer for storing scan converted first-edge coords */
    working_hlines = hlines;
    prev_index = cur_index = min_index;
    /* Start from the top of the first edge */
    /* Scan convert each line in the first edge from top to bottom */
    do {
        INDEX_BACKWARD(cur_index);
        scan_edge (pts[prev_index].x, pts[prev_index].y,
                   pts[cur_index].x, pts[cur_index].y, 1, 0, &working_hlines);
        prev_index = cur_index;
    } while (cur_index != max_index);

    /* Scan the second edge and store the boundary points in the list */
    working_hlines = hlines;
    prev_index = cur_index = min_index;
    /* Scan convert the second edge, top to bottom */
    do {
        INDEX_FORWARD(cur_index);
        scan_edge (pts[prev_index].x, pts[prev_index].y,
                   pts[cur_index].x, pts[cur_index].y, 0, 0, &working_hlines);
        prev_index = cur_index;
    } while (cur_index != max_index);

    for (i = 0; i < nr_hlines; i++) {
        cb (context, hlines [i].x1, hlines [i].x2, i + min_point_y);
    }

    /* Release the line list's memory and we're successfully done */
    free (hlines);

    return TRUE;
}

/* _fill_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 2d
 *  rasteriser.
 */
void _fill_edge_structure (POLYGON_EDGE *edge, const int *i1, const int *i2)
{
    if (i2[1] < i1[1]) {
        const int *it;

        it = i1;
        i1 = i2;
        i2 = it;
    }

    edge->top = i1[1];
    edge->bottom = i2[1] - 1;
    edge->dx = ((i2[0] - i1[0]) << POLYGON_FIX_SHIFT) / (i2[1] - i1[1]);
    edge->x = (i1[0] << POLYGON_FIX_SHIFT) + (1<<(POLYGON_FIX_SHIFT-1)) - 1;
    edge->prev = NULL;
    edge->next = NULL;

    if (edge->dx < 0)
        edge->x += MIN(edge->dx+(1<<POLYGON_FIX_SHIFT), 0);

    edge->w = MAX(ABS(edge->dx)-(1<<POLYGON_FIX_SHIFT), 0);
}

/* _add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
POLYGON_EDGE *_add_edge (POLYGON_EDGE *list, POLYGON_EDGE *edge, int sort_by_x)
{
    POLYGON_EDGE *pos = list;
    POLYGON_EDGE *prev = NULL;

    if (sort_by_x) {
        while ((pos) && ((pos->x + (pos->w + pos->dx) / 2) < 
                       (edge->x + (edge->w + edge->dx) / 2))) {
            prev = pos;
            pos = pos->next;
        }
    }
    else {
        while ((pos) && (pos->top < edge->top)) {
            prev = pos;
            pos = pos->next;
        }
    }

    edge->next = pos;
    edge->prev = prev;

    if (pos)
        pos->prev = edge;

    if (prev) {
        prev->next = edge;
        return list;
    }
    else
        return edge;
}

/* _remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
POLYGON_EDGE *_remove_edge(POLYGON_EDGE *list, POLYGON_EDGE *edge)
{
    if (edge->next) 
        edge->next->prev = edge->prev;

    if (edge->prev) {
        edge->prev->next = edge->next;
        return list;
    }
    else
        return edge->next;
}

/* General polygon generator */
BOOL GUIAPI PolygonGenerator (void* context, const POINT* pts, int vertices, CB_POLYGON cb)
{
    int c;
    int top = INT_MAX;
    int bottom = INT_MIN;
    const int *i1, *i2;
    void* _scratch_mem;
    const int* points = (int*) pts;
    POLYGON_EDGE *edge, *next_edge;
    POLYGON_EDGE *active_edges = NULL;
    POLYGON_EDGE *inactive_edges = NULL;

    /* allocate some space and fill the edge table */
    _scratch_mem = malloc (sizeof(POLYGON_EDGE) * vertices);
    if (!(edge = (POLYGON_EDGE *)_scratch_mem))
        return FALSE;

    i1 = points;
    i2 = points + (vertices-1) * 2;

    for (c=0; c<vertices; c++) {
        if (i1[1] != i2[1]) {
            _fill_edge_structure (edge, i1, i2);

            if (edge->bottom >= edge->top) {

                if (edge->top < top)
                    top = edge->top;

                if (edge->bottom > bottom)
                    bottom = edge->bottom;

                inactive_edges = _add_edge(inactive_edges, edge, FALSE);
                edge++;
            }
        }
        i2 = i1;
        i1 += 2;
    }

    /* for each scanline in the polygon... */
    for (c=top; c<=bottom; c++) {

        /* check for newly active edges */
        edge = inactive_edges;
        while ((edge) && (edge->top == c)) {
            next_edge = edge->next;
            inactive_edges = _remove_edge(inactive_edges, edge);
            active_edges = _add_edge(active_edges, edge, TRUE);
            edge = next_edge;
        }

        /* draw horizontal line segments */
        edge = active_edges;
        while ((edge) && (edge->next)) {
            int x1 = edge->x>>POLYGON_FIX_SHIFT;
            int x2 = ((edge->next->x+edge->next->w)>>POLYGON_FIX_SHIFT);
            cb (context, x1, x2, c); 
            edge = edge->next->next;
        }

        /* update edges, sorting and removing dead ones */
        edge = active_edges;
        while (edge) {
            next_edge = edge->next;
            if (c >= edge->bottom) {
                active_edges = _remove_edge(active_edges, edge);
            }
            else {
                edge->x += edge->dx;
                while ((edge->prev) && 
                        (edge->x+edge->w/2 < edge->prev->x+edge->prev->w/2)) {
                    if (edge->next)
                        edge->next->prev = edge->prev;
                    edge->prev->next = edge->next;
                    edge->next = edge->prev;
                    edge->prev = edge->prev->prev;
                    edge->next->prev = edge;
                    if (edge->prev)
                        edge->prev->next = edge;
                    else
                        active_edges = edge;
                }
            }
            edge = next_edge;
        }
    }

    free (_scratch_mem);
    return TRUE;
}

static POINT* convert_vertices (PDC pdc, const POINT* pts, int vertices, RECT* rcOutput)
{
    int i;
    POINT* points;

    if (vertices < 3) return NULL;

    if (!(points = malloc (sizeof (POINT) * vertices))) {
        return NULL;
    }

    /* Transfer logical to device to screen and find rcOutput here. */
    points [0] = pts [0];
    coor_LP2SP (pdc, &points [0].x, &points [0].y);
    rcOutput->left = rcOutput->right = points [0].x;
    rcOutput->top = rcOutput->bottom = points [0].y;

    for (i = 1; i < vertices; i++) {
        points [i] = pts [i];
        coor_LP2SP (pdc, &points [i].x, &points [i].y);
        if (points [i].x < rcOutput->left) {
            rcOutput->left = points [i].x;
        }
        else if (points [i].x > rcOutput->right) {
            rcOutput->right = points [i].x;
        }
        if (points [i].y < rcOutput->top) {
            rcOutput->top = points [i].y;
        }
        else if (points [i].y > rcOutput->bottom) {
            rcOutput->bottom = points [i].y;
        }
    }

    return points;
}

BOOL GUIAPI FillPolygon (HDC hdc, const POINT* pts, int vertices)
{
    PDC pdc;
    RECT rc_tmp;
    POINT* points;
    BOOL is_mv;

    if (vertices < 3) return FALSE;

    if (!(pdc = check_ecrgn (hdc)))
        return TRUE;

    is_mv = PolygonIsMonotoneVertical (pts, vertices);

    if (!(points = convert_vertices (pdc, pts, vertices, &pdc->rc_output)))
        return FALSE;

    pdc->cur_pixel = pdc->brushcolor;
    pdc->cur_ban = NULL;

    rc_tmp = pdc->rc_output;
    pdc->rc_output.right ++;
    pdc->rc_output.bottom ++;
    ENTER_DRAWING (pdc);

    if (rc_tmp.left == rc_tmp.right) {
        _dc_draw_vline_clip (pdc, rc_tmp.top, rc_tmp.bottom, rc_tmp.left);
    }
    else if (rc_tmp.top == rc_tmp.bottom) {
        _dc_draw_hline_clip (pdc, rc_tmp.left, rc_tmp.right, rc_tmp.top);
    }
    else {
        if (is_mv)
            MonotoneVerticalPolygonGenerator (pdc, points, vertices, _dc_draw_hline_clip);
        else
            PolygonGenerator (pdc, points, vertices, _dc_draw_hline_clip);
    }

    LEAVE_DRAWING (pdc);

    UNLOCK_GCRINFO (pdc);

    free (points);
    return TRUE;
}

