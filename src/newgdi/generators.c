/*
** $Id: generators.c,v 1.8 2003/09/04 06:02:53 weiym Exp $
**
** generators.c: general line, circle, ellipse, arc generators.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** The line clipping algorithm comes from GGI.
** Copyright (C) 1998 Alexander Larsson <alla@lysator.liu.se>
**
** The line generator comes from MicroWindows.
** Copyright (C) 1999 Greg Haerr <greg@censoft.com>
**
** All other generators come from Allegro by Shawn Hargreaves and others. 
** Thank for their great work and good license.
**
** "Allegro is a gift-software"
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
#include "fixedmath.h"

/* Line clipper */
/*
  This is a line-clipper using the algorithm by cohen-sutherland.

  It is modified to do pixel-perfect clipping. This means that it
  will generate the same endpoints that would be drawn if an ordinary
  bresenham line-drawer where used and only visible pixels drawn.

  It can be used with a bresenham-like linedrawer if it is modified to
  start with a correct error-term.
*/

#define OC_LEFT 1
#define OC_RIGHT 2
#define OC_TOP 4
#define OC_BOTTOM 8

/* Outcodes:
+-> x
|       |      | 
V  0101 | 0100 | 0110
y ---------------------
   0001 | 0000 | 0010
  ---------------------
   1001 | 1000 | 1010
        |      | 
 */
#define outcode(code, xx, yy) \
{\
  code = 0;\
 if (xx < cliprc->left)\
    code |= OC_LEFT;\
  else if (xx >= cliprc->right)\
    code |= OC_RIGHT;\
  if (yy < cliprc->top)\
    code |= OC_TOP;\
  else if (yy >= cliprc->bottom)\
    code |= OC_BOTTOM;\
}

/*
  Calculates |_ a/b _| with mathematically correct floor
  */
static int FloorDiv(int a, int b)
{
    int floor;
    if (b>0) {
        if (a>0) {
            return a /b;
        } else {
            floor = -((-a)/b);
            if ((-a)%b != 0)
                floor--;
        }
        return floor;
    } else {
        if (a>0) {
            floor = -(a/(-b));
            if (a%(-b) != 0)
                floor--;
            return floor;
        } else {
            return (-a)/(-b);
        }
    }
}
/*
  Calculates |^ a/b ^| with mathamatically correct floor
  */
static int CeilDiv(int a,int b)
{
    if (b>0)
        return FloorDiv(a-1,b)+1;
    else
        return FloorDiv(-a-1,-b)+1;
}

BOOL GUIAPI LineClipper (const RECT* cliprc, int *_x0, int *_y0, int *_x1, int *_y1)
{
    int first, last, code;
    int x0, y0, x1, y1;
    int x, y;
    int dx, dy;
    int xmajor;
    int slope;
    
    if (*_x0 == *_x1 && *_y0 == *_y1) { /* a pixel*/
        return PtInRect (cliprc, *_x0, *_y0);
    }
    else if (*_x0 == *_x1) { /* a vertical line */
        int sy, ey;

        if (*_y1 > *_y0) {
            sy = *_y0;
            ey = *_y1;
        }
        else {
            sy = *_y1;
            ey = *_y0;
        }

        if ( (*_x0 >= cliprc->right) || (sy >= cliprc->bottom) || 
                    (*_x0 < cliprc->left) || (ey < cliprc->top) )
            return FALSE;

        if ( (*_x0 >= cliprc->left) && (sy >= cliprc->top) && 
                    (*_x0 < cliprc->right) && (ey < cliprc->bottom) )
            return TRUE;
                    
        if (sy < cliprc->top)
            sy = cliprc->top;
        if (ey >= cliprc->bottom)
            ey = cliprc->bottom - 1;

        if (ey < sy)
            return FALSE;

        *_y0 = sy;
        *_y1 = ey;
        return TRUE;
    }
    else if (*_y0 == *_y1) { /* a horizontal line */
        int sx, ex;

        if (*_x1 > *_x0) {
            sx = *_x0;
            ex = *_x1;
        }
        else {
            sx = *_x1;
            ex = *_x0;
        }

        if ( (sx >= cliprc->right) || (*_y0 >= cliprc->bottom) || 
                    (ex < cliprc->left) || (*_y0 < cliprc->top) )
            return FALSE;

        if ( (sx >= cliprc->left) && (*_y0 >= cliprc->top) && 
                    (ex < cliprc->right) && (*_y0 < cliprc->bottom) )
            return TRUE;
                    
        if (sx < cliprc->left)
            sx = cliprc->left;
        if (ex >= cliprc->right)
            ex = cliprc->right - 1;

        if (ex < sx)
            return FALSE;

        *_x0 = sx;
        *_x1 = ex;
        return TRUE;
    }

    first = 0;
    last = 0;
    outcode (first, *_x0, *_y0);
    outcode (last, *_x1, *_y1);

    if ((first | last) == 0) {
        return TRUE; /* Trivially accepted! */
    }

    if ((first & last) != 0) {
        return FALSE; /* Trivially rejected! */
    }

    x0 = *_x0; y0 = *_y0;
    x1 = *_x1; y1 = *_y1;

    dx = x1 - x0;
    dy = y1 - y0;
  
    xmajor = (ABS (dx) > ABS (dy));
    slope = ((dx>=0) && (dy>=0)) || ((dx<0) && (dy<0));
  
    while (TRUE) {
        code = first;
        if (first == 0)
            code = last;

        if (code & OC_LEFT) {
            x = cliprc->left;
            if (xmajor) {
                y = *_y0 + FloorDiv (dy * (x - *_x0) * 2 + dx, 2 * dx);
            } else {
                if (slope) {
                    y = *_y0 + CeilDiv (dy * ((x - *_x0) * 2 - 1), 2 * dx);
                } else {
                    y = *_y0 + FloorDiv (dy * ((x - *_x0) * 2 - 1), 2 * dx);
                }
            }
        } else if (code & OC_RIGHT) {
            x = cliprc->right - 1;
            if (xmajor) {
                y = *_y0 +  FloorDiv (dy * (x - *_x0) * 2 + dx, 2 * dx);
            } else {
                if (slope) {
                    y = *_y0 + CeilDiv (dy * ((x - *_x0) * 2 + 1), 2 * dx) - 1;
                } else {
                    y = *_y0 + FloorDiv (dy * ((x - *_x0) * 2 + 1), 2 * dx) + 1;
                }
            }
        } else if (code & OC_TOP) {
            y = cliprc->top;
            if (xmajor) {
                if (slope) {
                    x = *_x0 + CeilDiv (dx * ((y - *_y0) * 2 - 1), 2 * dy);
                } else {
                    x = *_x0 + FloorDiv (dx * ((y - *_y0) * 2 - 1), 2 * dy);
                }
            } else {
                x = *_x0 +  FloorDiv (dx * (y - *_y0) * 2 + dy, 2 * dy);
            }
        } else { /* OC_BOTTOM */
            y = cliprc->bottom - 1;
            if (xmajor) {
                if (slope) {
                    x = *_x0 + CeilDiv (dx * ((y - *_y0) * 2 + 1), 2 * dy) - 1;
                } else {
                    x = *_x0 + FloorDiv (dx * ((y - *_y0) * 2 + 1), 2 * dy) + 1;
                }
            } else {
                x = *_x0 +  FloorDiv (dx * (y - *_y0) * 2 + dy, 2 * dy);
            }
        }

        if (first) {
            x0 = x;
            y0 = y;
            outcode (first, x0, y0);
        } else {
            x1 = x;
            y1 = y;
            last = code;
            outcode (last, x1, y1);
        }
    
        if ((first & last) != 0) {
            return FALSE; /* Trivially rejected! */
        }

        if ((first | last) == 0) {
            *_x0 = x0; *_y0 = y0;
            *_x1 = x1; *_y1 = y1;
            return TRUE; /* Trivially accepted! */
        }
    }
}

/* Breshenham line generator */
void GUIAPI LineGenerator (void* context, int x1, int y1, int x2, int y2, CB_LINE cb)
{
    int xdelta;     /* width of rectangle around line */
    int ydelta;     /* height of rectangle around line */
    int xinc;       /* increment for moving x coordinate */
    int yinc;       /* increment for moving y coordinate */
    int rem;        /* current remainder */
    
    cb (context, 0, 0);

    if (x1 == x2 && y1 == y2) { /* a pixel */
        return;
    }
    else if (x1 == x2) { /* a vertical line */
        int dir = (y2 > y1) ? 1 : -1;

        do {
            cb (context, 0, dir);
            y1 += dir;
        } while (y1 != y2);
        return;
    }
    else if (y1 == y2) { /* a horizontal line */
        int dir = (x2 > x1) ? 1 : -1;

        do {
            cb (context, dir, 0);
            x1 += dir;
        } while (x1 != x2);
        return;
    }

    xdelta = x2 - x1;
    ydelta = y2 - y1;
    if (xdelta < 0) xdelta = -xdelta;
    if (ydelta < 0) ydelta = -ydelta;

    xinc = (x2 > x1) ? 1 : -1;
    yinc = (y2 > y1) ? 1 : -1;

    if (xdelta >= ydelta) {
        rem = xdelta >> 1;
        while (x1 != x2) {
            x1 += xinc;
            rem += ydelta;
            if (rem >= xdelta) {
                rem -= xdelta;
                y1 += yinc;
                cb (context, xinc, yinc);
            }
            else
                cb (context, xinc, 0);
        }
    } else {
        rem = ydelta >> 1;
        while (y1 != y2) {
            y1 += yinc;
            rem += xdelta;
            if (rem >= ydelta) {
                rem -= ydelta;
                x1 += xinc;
                cb (context, xinc, yinc);
            }
            else
                cb (context, 0, yinc);
        }
    }
}

/* Circle generator */
void GUIAPI CircleGenerator (void* context, int x, int y, int r, CB_CIRCLE cb)
{
   int cx = 0;
   int cy = r;
   int df = 1 - r; 
   int d_e = 3;
   int d_se = -2 * r + 5;

   do {
      cb (context, x-cx, x+cx, y+cy); 
      if (cy) 
         cb (context, x-cx, x+cx, y-cy); 

      if (cx != cy) {
         cb (context, x-cy, x+cy, y+cx);
         if (cx)
            cb (context, x-cy, x+cy, y-cx); 
      }

      if (df < 0)  {
         df += d_e;
         d_e += 2;
         d_se += 2;
      }
      else { 
         df += d_se;
         d_e += 2;
         d_se += 4;
         cy--;
      } 

      cx++; 

   } while (cx <= cy);
}

/* Ellipse generator */
void GUIAPI EllipseGenerator (void* context, int x, int y, int rx, int ry, CB_ELLIPSE cb)
{
   int ix, iy;
   int h, i, j, k;
   int oh, oi, oj, ok;

   if (rx < 1) 
      rx = 1; 

   if (ry < 1) 
      ry = 1;

   h = i = j = k = 0xFFFF;

   if (rx > ry) {
      ix = 0; 
      iy = rx * 64;

      do {
         oh = h;
         oi = i;
         oj = j;
         ok = k;

         h = (ix + 32) >> 6; 
         i = (iy + 32) >> 6;
         j = (h * ry) / rx; 
         k = (i * ry) / rx;

         if (((h != oh) || (k != ok)) && (h < oi)) {
            cb (context, x-h, x+h, y+k);
            if (k) {
               cb (context, x-h, x+h, y-k); 
            }
         }

         if (((i != oi) || (j != oj)) && (h < i)) {
            cb (context, x-i, x+i, y+j); 
            if (j) {
               cb (context, x-i, x+i, y-j); 
            }
         }

         ix = ix + iy / rx; 
         iy = iy - ix / rx;

      } while (i > h);
   } 
   else {
      ix = 0; 
      iy = ry * 64;

      do {
         oh = h;
         oi = i;
         oj = j;
         ok = k;

         h = (ix + 32) >> 6; 
         i = (iy + 32) >> 6;
         j = (h * rx) / ry; 
         k = (i * rx) / ry;

         if (((j != oj) || (i != oi)) && (h < i)) {
            cb (context, x-j, x+j, y+i);
            if (i) {
               cb (context, x-j, x+j, y-i); 
            }
         }

         if (((k != ok) || (h != oh)) && (h < oi)) {
            cb (context, x-k, x+k, y+h);
            if (h) {
               cb (context, x-k, x+k, y-h);
            }
         }

         ix = ix + iy / ry; 
         iy = iy - ix / ry;

      } while(i > h);
   }
}

/* Arc generator */
void GUIAPI ArcGenerator (void* context, int x, int y, int r, fixed ang1, fixed ang2, CB_ARC cb)
{
   unsigned long rr = r*r;
   unsigned long rr1, rr2, rr3;
   int px, py;
   int ex, ey;
   int px1, px2, px3;
   int py1, py2, py3;
   int d1, d2, d3;
   int ax, ay;
   int q, qe;
   long tg_cur, tg_end;
   int done = FALSE;

   rr1 = r;
   rr2 = itofix(x);
   rr3 = itofix(y);

   /* evaluate the start point and the end point */
   px = fixtoi(rr2 + rr1 * fcos(ang1));
   py = fixtoi(rr3 - rr1 * fsin(ang1));
   ex = fixtoi(rr2 + rr1 * fcos(ang2));
   ey = fixtoi(rr3 - rr1 * fsin(ang2));

   /* start quadrant */
   if (px >= x) {
      if (py <= y)
         q = 1;                           /* quadrant 1 */
      else
         q = 4;                           /* quadrant 4 */
   }
   else {
      if (py < y)
         q = 2;                           /* quadrant 2 */
      else
         q = 3;                           /* quadrant 3 */
   }

   /* end quadrant */
   if (ex >= x) {
      if (ey <= y)
         qe = 1;                          /* quadrant 1 */
      else
         qe = 4;                          /* quadrant 4 */
   }
   else {
      if (ey < y)
         qe = 2;                          /* quadrant 2 */
      else
         qe = 3;                          /* quadrant 3 */
   }

   #define loc_tg(_y, _x)  (_x-x) ? itofix(_y-y)/(_x-x) : itofix(_y-y)

   tg_end = loc_tg(ey, ex);

   while (!done) {
      cb (context, px, py);

      /* from here, we have only 3 possible direction of movement, eg.
       * for the first quadrant:
       *
       *    OOOOOOOOO
       *    OOOOOOOOO
       *    OOOOOO21O
       *    OOOOOO3*O
       */

      /* evaluate the 3 possible points */
      switch (q) {

         case 1:
            px1 = px;
            py1 = py-1;
            px2 = px-1;
            py2 = py-1;
            px3 = px-1;
            py3 = py;

            /* 2nd quadrant check */
            if (px != x) {
               break;
            }
            else {
               /* we were in the end quadrant, changing is illegal. Exit. */
               if (qe == q)
                  done = TRUE;
               q++;
            }
            /* fall through */

         case 2:
            px1 = px-1;
            py1 = py;
            px2 = px-1;
            py2 = py+1;
            px3 = px;
            py3 = py+1;

            /* 3rd quadrant check */
            if (py != y) {
               break;
            }
            else {
               /* we were in the end quadrant, changing is illegal. Exit. */
               if (qe == q)
                  done = TRUE;
               q++;
            }
            /* fall through */

         case 3:
            px1 = px;
            py1 = py+1;
            px2 = px+1;
            py2 = py+1;
            px3 = px+1;
            py3 = py;

            /* 4th quadrant check */
            if (px != x) {
               break;
            }
            else {
               /* we were in the end quadrant, changing is illegal. Exit. */
               if (qe == q)
                  done = TRUE;
               q++;
            }
            /* fall through */

         case 4:
            px1 = px+1;
            py1 = py;
            px2 = px+1;
            py2 = py-1;
            px3 = px;
            py3 = py-1;

            /* 1st quadrant check */
            if (py == y) {
               /* we were in the end quadrant, changing is illegal. Exit. */
               if (qe == q)
                  done = TRUE;

               q = 1;
               px1 = px;
               py1 = py-1;
               px2 = px-1;
               py2 = py-1;
               px3 = px-1;
               py3 = py;
            }
            break;

         default:
            return;
      }

      /* now, we must decide which of the 3 points is the right point.
       * We evaluate the distance from center and, then, choose the
       * nearest point.
       */
      ax = x-px1;
      ay = y-py1;
      rr1 = ax*ax + ay*ay;

      ax = x-px2;
      ay = y-py2;
      rr2 = ax*ax + ay*ay;

      ax = x-px3;
      ay = y-py3;
      rr3 = ax*ax + ay*ay;

      /* difference from the main radius */
      if (rr1 > rr)
         d1 = rr1-rr;
      else
         d1 = rr-rr1;
      if (rr2 > rr)
         d2 = rr2-rr;
      else
         d2 = rr-rr2;
      if (rr3 > rr)
         d3 = rr3-rr;
      else
         d3 = rr-rr3;

      /* what is the minimum? */
      if (d1 <= d2) {
         px = px1;
         py = py1;
      }
      else if (d2 <= d3) {
         px = px2;
         py = py2;
      }
      else {
         px = px3;
         py = py3;
      }

      /* are we in the final quadrant? */
      if (qe == q) {
         tg_cur = loc_tg(py, px);

         /* is the arc finished? */
         switch (q) {

            case 1:
               /* end quadrant = 1? */
               if (tg_cur <= tg_end)
                  done = TRUE;
               break;

            case 2:
               /* end quadrant = 2? */
               if (tg_cur <= tg_end)
                  done = TRUE;
               break;

            case 3:
               /* end quadrant = 3? */
               if (tg_cur <= tg_end)
                  done = TRUE;
               break;

            case 4:
               /* end quadrant = 4? */
               if (tg_cur <= tg_end)
                  done = TRUE;
               break;
         }
      }
   }

   /* draw the last evaluated point */
   cb (context, px, py);
}

