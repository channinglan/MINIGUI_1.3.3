/*
** $Id: vga16.c,v 1.21 2003/11/21 12:44:01 weiym Exp $
**
** vga16.c: VGA 16-color GAL engine built on SVGALib
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001, 2002 Wei Yongming.
**
** Create by Wei Yongming, 2001/09/21
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "gal.h"

#ifdef _VGA16_GAL

#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <vga.h>

#include "vga16.h"

#ifdef _LITE_VERSION
#include "ourhdr.h"
#include "client.h"
#include "sharedres.h"

static int __mg_saved_clip_minx, __mg_saved_clip_miny, __mg_saved_clip_maxx, __mg_saved_clip_maxy;

static int set_effective_clip_rect (GC_VGA16* gc)
{
    if (__mg_saved_clip_minx < SHAREDRES_CLI_SCR_LX)
        gc->clipminx = SHAREDRES_CLI_SCR_LX;
    else
        gc->clipminx = __mg_saved_clip_minx;

    if (__mg_saved_clip_miny < SHAREDRES_CLI_SCR_TY)
        gc->clipminy = SHAREDRES_CLI_SCR_TY;
    else
        gc->clipminy = __mg_saved_clip_miny;
    
    if (__mg_saved_clip_maxx > (SHAREDRES_CLI_SCR_RX + 1))
        gc->clipmaxx = SHAREDRES_CLI_SCR_RX;
    else
        gc->clipmaxx = __mg_saved_clip_maxx;

    if (__mg_saved_clip_maxy > (SHAREDRES_CLI_SCR_BY + 1))
        gc->clipmaxy = SHAREDRES_CLI_SCR_BY;
    else
        gc->clipmaxy = __mg_saved_clip_maxy;

    if (gc->clipmaxx < gc->clipminx)
        return -1;
    else if (gc->clipmaxy < gc->clipminy)
        return -1;

    gc->clipmaxx ++;
    gc->clipmaxy ++;

    return 0;
}

#define BLOCK_DRAW_SEM                                          \
    if (!mgIsServer && cur_gfx->phygc.gc_vga16 == gc.gc_vga16)  \
        lock_draw_sem ();                                       \
    if (((!mgIsServer && (SHAREDRES_TOPMOST_LAYER != __mg_layer)) || __mg_switch_away)) \
        goto leave_drawing;                                     \
    if (!mgIsServer && cur_gfx->phygc.gc_vga16 == gc.gc_vga16)  \
        if (set_effective_clip_rect (gc.gc_vga16))              \
            goto leave_drawing;

#define UNBLOCK_DRAW_SEM                                        \
    if (!mgIsServer && cur_gfx->phygc.gc_vga16 == gc.gc_vga16)  \
leave_drawing:                                                  \
        unlock_draw_sem ()

#else

#define BLOCK_DRAW_SEM
#define UNBLOCK_DRAW_SEM

#endif

/*
 * Low Level Graphics Operations
 */
static int bytesperpixel (GAL_GC gc) 
{ 
    return 1;
}

static int bitsperpixel (GAL_GC gc) 
{ 
    return 4;
}

static int width (GAL_GC gc) 
{ 
    return gc.gc_vga16->xres; 
}

static int height (GAL_GC gc) 
{ 
    return gc.gc_vga16->yres; 
}

static int colors (GAL_GC gc) 
{    
    return 16;
}

static int setclipping (GAL_GC gc, int x1, int y1, int x2, int y2)
{
    GC_VGA16* mygc;
    mygc = gc.gc_vga16;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > mygc->xres - 1) x2 = mygc->xres - 1;
    if (y2 > mygc->yres - 1) y2 = mygc->yres - 1;

    mygc->doclip = 1;
#ifdef _LITE_VERSION
    if (!mgIsServer && mygc == cur_gfx->phygc.gc_vga16) {
        __mg_saved_clip_minx = x1;
        __mg_saved_clip_miny = y1;
        __mg_saved_clip_maxx = x2;
        __mg_saved_clip_maxy = y2;
    }
    else {
#endif
        mygc->clipminx = x1;
        mygc->clipminy = y1;
        mygc->clipmaxx = x2 + 1;
        mygc->clipmaxy = y2 + 1;
#ifdef _LITE_VERSION
    }

#endif
    return 0;
}

static void enableclipping (GAL_GC gc)
{
    GC_VGA16* mygc = gc.gc_vga16;

    setclipping (gc, 0, 0, mygc->xres - 1, mygc->yres - 1);
}

static void disableclipping (GAL_GC gc)
{
    gc.gc_vga16->doclip = 0;
}

static int getclipping (GAL_GC gc, int* x1, int* y1, int* x2, int* y2)
{
    GC_VGA16* mygc;
    mygc = gc.gc_vga16;

#ifdef _LITE_VERSION
    if (!mgIsServer && mygc == cur_gfx->phygc.gc_vga16) {
        *x1 = __mg_saved_clip_minx;
        *y1 = __mg_saved_clip_miny;
        *x2 = __mg_saved_clip_maxx;
        *y2 = __mg_saved_clip_maxy;
    }
    else {
#endif
        *x1 = mygc->clipminx;
        *y1 = mygc->clipminy;
        *x2 = mygc->clipmaxx - 1;
        *y2 = mygc->clipmaxy - 1;    
#ifdef _LITE_VERSION
    }
#endif

    return 0;
}

/*
 * Allocation and release of graphics context
 */
static int allocategc (GAL_GC gc, int width, int height, int depth, GAL_GC* newgc)
{
    GC_VGA16* mygc;
    if (!(newgc->gc_vga16 = (GC_VGA16*)malloc (sizeof (GC_VGA16))))
        return -1;

    mygc = newgc->gc_vga16;
    mygc->xres = width;
    mygc->yres = height;
    mygc->pitch = width;
    mygc->gr_background = 0;
    mygc->gr_foreground = 15;

    enableclipping (*newgc);

    if (!(mygc->fb_buff = calloc (1, mygc->pitch * height))) {
        free (mygc);
        return -1;
    }

    return 0;
}

static void freegc (GAL_GC gc)
{
    GC_VGA16* mygc;

    mygc = gc.gc_vga16;
    free (mygc->fb_buff);
    free (mygc);
}

/*
 * Background and foreground colors
 */
static int getbgcolor (GAL_GC gc, gal_pixel* color)
{
    GC_VGA16* mygc = gc.gc_vga16;
    *color = mygc->gr_background;
    return 0;
}

static int setbgcolor (GAL_GC gc, gal_pixel color)
{
    GC_VGA16* mygc = gc.gc_vga16;
    mygc->gr_background = color;    
    return 0;
}


static int getfgcolor (GAL_GC gc, gal_pixel* color)
{
    GC_VGA16* mygc = gc.gc_vga16;
    *color = mygc->gr_foreground;    
    return 0;
}

static int setfgcolor (GAL_GC gc, gal_pixel color)
{
    GC_VGA16* mygc = gc.gc_vga16;
    mygc->gr_foreground = color;    

    if (mygc->fb_buff == 0) {
        vga_setcolor (color & 0x0F);
    }

    return 0;
}

/*
 * Convertion between GAL_Color and gal_pixel
 * borrowed  from gl lib.
 */
static gal_pixel mapcolor (GAL_GC gc, GAL_Color *color)
{
    unsigned int v;

    /* Now this is real fun. Map to standard EGA palette. */
    v = 0;
    if (color->b >= 64)
        v += 1;
    if (color->g >= 64)
        v += 2;
    if (color->r >= 64)
        v += 4;
    if (color->b >= 192 || color->g >= 192 || color->r >= 192)
        v += 8;
    return v;
}

static int unmappixel (GAL_GC gc, gal_pixel pixel, GAL_Color* color)
{
    color->r = SysPixelColor [pixel].r;
    color->g = SysPixelColor [pixel].g;
    color->b = SysPixelColor [pixel].b;
    return 0;
}

/*
 * Palette operations
 */
static int getpalette (GAL_GC gc, int s, int len, GAL_Color* cmap)
{
    int i;
    int r, g, b;

    if (gc.gc_vga16 != cur_gfx->phygc.gc_vga16)
        return 0;
    
    for (i = 0; i < len; i++) {
        vga_getpalette (i + s, &r, &g, &b);
        cmap [i].r = r << 2;
        cmap [i].g = g << 2;
        cmap [i].b = b << 2;
    }
    
    return 0;
}

static int setpalette (GAL_GC gc, int s, int len, GAL_Color* cmap)
{
    int i;

    if (gc.gc_vga16 != cur_gfx->phygc.gc_vga16)
        return 0;
    
    for (i = s; i < s + len; i++) {
        vga_setpalette (i, cmap->r >> 2, cmap->g >> 2, cmap->b >> 2);
        cmap ++;
    }

    return 0;
}

/*
 * Specical functions work for <=8 bit color mode.
 */

static int setcolorfulpalette (GAL_GC gc)
{
    int i;

    if (gc.gc_vga16 != cur_gfx->phygc.gc_vga16)
        return 0;

    for (i = 0; i < 16; i++) {
        vga_setpalette (i, SysPixelColor[i].r >> 2, 
                        SysPixelColor[i].g >> 2, 
                        SysPixelColor[i].b >> 2);
    }

    return 0;
}

/*
 * Box operations
 */
static size_t boxsize (GAL_GC gc, int w, int h)
{
    if ((w <= 0) || (h <= 0)) return -1;

    return w * h;
}

#define CLIP_INVISIBLE  0
#define CLIP_VISIBLE    1
#define CLIP_PARTIAL    2

static int vga16_clippoint (PGC_VGA16 gc, int x ,int y)
{
    if (gc->doclip) {
        if ((x >= gc->clipminx) && (x < gc->clipmaxx) &&
            (y >= gc->clipminy) && (y < gc->clipmaxy)) 
            return CLIP_VISIBLE;
    } else {
        if ((x >= 0) && (x < gc->xres) && (y >= 0) && (y < gc->yres)) 
            return CLIP_VISIBLE;
    }

    return CLIP_INVISIBLE;
}


static int vga16_cliphline (PGC_VGA16 gc,int * px,int * py, int * pw)
{
    if (gc->doclip) {
        if ( (*px >= gc->clipmaxx) || (*py >= gc->clipmaxy) || 
             (*px + *pw - 1 < gc->clipminx) || (*py < gc->clipminy) )
                return CLIP_INVISIBLE;

        if ( (*px >= gc->clipminx) && (*py >= gc->clipminy) && 
             (*px + *pw -1 < gc->clipmaxx) && (*py < gc->clipmaxy) )
                return CLIP_VISIBLE;
            
        if (*px < gc->clipminx) {
            *pw -= gc->clipminx - *px;
            *px = gc->clipminx;
        }
        if (*px + *pw - 1 >= gc->clipmaxx)    
            *pw = gc->clipmaxx - *px;
    } else {
        if ( (*px >= gc->xres) || (*py >= gc->yres) || (*px + *pw - 1 < 0) || (*py < 0) )
                return CLIP_INVISIBLE;

        if ( (*px >= 0) && (*py >= 0) && (*px + *pw -1 < gc->xres) && (*py < gc->yres) )
                return CLIP_VISIBLE;
            
        if (*px < 0) {
            *pw += *px;    
            *px = 0;
        }
        if (*px + *pw - 1 >= gc->xres)    
            *pw = gc->xres - *px;
    }

    if (*pw <= 0)
        return CLIP_INVISIBLE;

    return CLIP_PARTIAL;        
}

static int vga16_clipvline(PGC_VGA16 gc,int * px,int * py, int *ph)
{
    if (gc->doclip) {
        if ( (*px >= gc->clipmaxx) || (*py >= gc->clipmaxy) || 
             (*px < gc->clipminx) || (*py + *ph - 1 < gc->clipminy) )
                return CLIP_INVISIBLE;

        if ( (*px >= gc->clipminx) && (*py >= gc->clipminy) && 
             (*px < gc->clipmaxx) && (*py + *ph - 1 < gc->clipmaxy) )
                return CLIP_VISIBLE;
            
        if (*py < gc->clipminy) {
            *ph -= gc->clipminy - *py;
            *py = gc->clipminy;
        }
        if (*py + *ph - 1 >= gc->clipmaxy)    
            *ph = gc->clipmaxy - *py;
    } else {
        if ( (*py >= gc->yres) || (*px >= gc->xres) || (*py + *ph - 1 < 0) || (*px < 0) )
                return CLIP_INVISIBLE;

        if ( (*py >= 0) && (*px >= 0) && (*py + *ph -1 < gc->yres) && (*px < gc->xres) )
                return CLIP_VISIBLE;
            
        if (*py < 0) {
            *ph += *py;    
            *py = 0;
        }
        if (*py + *ph - 1 >= gc->yres)    
            *ph = gc->yres - *py;
    }

    if (*ph <= 0)
        return CLIP_INVISIBLE;

    return CLIP_PARTIAL;        
}

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
 if (xx < tempclipminx)\
    code |= OC_LEFT;\
  else if (xx > tempclipmaxx)\
    code |= OC_RIGHT;\
  if (yy < tempclipminy)\
    code |= OC_TOP;\
  else if (yy>tempclipmaxy)\
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

static int cs_clipline (PGC_VGA16 gc,int *_x0, int *_y0, int *_x1, int *_y1,
               int *clip_first, int *clip_last)
{
    int first,last, code;
    int x0,y0,x1,y1;
    int x,y;
    int dx,dy;
    int xmajor;
    int slope;
    
    int tempclipminx,tempclipminy,tempclipmaxx,tempclipmaxy;
    if (gc->doclip) {
        tempclipminx = gc->clipminx;
        tempclipminy = gc->clipminy;
        tempclipmaxx = gc->clipmaxx;
        tempclipmaxy = gc->clipmaxy;
    } else {
        tempclipminx = 0;
        tempclipminy = 0;
        tempclipmaxx = gc->xres - 1;
        tempclipmaxy = gc->yres - 1;
    }    

    first = 0;
    last = 0;
    outcode(first,*_x0,*_y0);
    outcode(last,*_x1,*_y1);

    if ((first | last) == 0) {
        return CLIP_VISIBLE; /* Trivially accepted! */
    }
    if ((first & last) != 0) {
        return CLIP_INVISIBLE; /* Trivially rejected! */
    }

    x0=*_x0; y0=*_y0;
    x1=*_x1; y1=*_y1;

    dx = x1 - x0;
    dy = y1 - y0;
  
    xmajor = (abs(dx) > abs(dy));
    slope = ((dx>=0) && (dy>=0)) || ((dx<0) && (dy<0));
  
    for (;;) {
        code = first;
        if (first==0)
            code = last;

        if (code&OC_LEFT) {
            x = tempclipminx;
            if (xmajor) {
                y = *_y0 +  FloorDiv(dy*(x - *_x0)*2 + dx,
                              2*dx);
            } else {
                if (slope) {
                    y = *_y0 + CeilDiv(dy*((x - *_x0)*2
                                   - 1), 2*dx);
                } else {
                    y = *_y0 + FloorDiv(dy*((x - *_x0)*2
                                - 1), 2*dx);
                }
            }
        } else if (code&OC_RIGHT) {
            x = tempclipmaxx;
            if (xmajor) {
                y = *_y0 +  FloorDiv(dy*(x - *_x0)*2 + dx, 2*dx);
            } else {
                if (slope) {
                    y = *_y0 + CeilDiv(dy*((x - *_x0)*2
                                   + 1), 2*dx)-1;
                } else {
                    y = *_y0 + FloorDiv(dy*((x - *_x0)*2
                                + 1), 2*dx)+1;
                }
            }
        } else if (code&OC_TOP) {
            y = tempclipminy;
            if (xmajor) {
                if (slope) {
                    x = *_x0 + CeilDiv(dx*((y - *_y0)*2
                                   - 1), 2*dy);
                } else {
                    x = *_x0 + FloorDiv(dx*((y - *_y0)*2
                                - 1), 2*dy);
                }
            } else {
                x = *_x0 +  FloorDiv( dx*(y - *_y0)*2 + dy,
                              2*dy);
            }
        } else { /* OC_BOTTOM */
            y = tempclipmaxy;
            if (xmajor) {
                if (slope) {
                    x = *_x0 + CeilDiv(dx*((y - *_y0)*2
                                   + 1), 2*dy)-1;
                } else {
                    x = *_x0 + FloorDiv(dx*((y - *_y0)*2
                                + 1), 2*dy)+1;
                }
            } else {
                x = *_x0 +  FloorDiv(dx*(y - *_y0)*2 + dy,
                             2*dy);
            }
        }

        if (first!=0) {
            x0 = x;
            y0 = y;
            outcode(first,x0,y0);
            *clip_first = 1;
        } else {
            x1 = x;
            y1 = y;
            last = code;
            outcode(last,x1,y1);
            *clip_last = 1;
        }
    
        if ((first & last) != 0) {
            return CLIP_INVISIBLE; /* Trivially rejected! */
        }
        if ((first | last) == 0) {
            *_x0=x0; *_y0=y0;
            *_x1=x1; *_y1=y1;
            return CLIP_PARTIAL; /* Trivially accepted! */
        }
    }
}

static int vga16_clipline (PGC_VGA16 gc,int * px1,int * py1, int * px2,int *py2)
{
    int w,h;
    int r1;
    int clip_first;
    int clip_last;

    if(*px1 == *px2) {
        if (*py1 == *py2)
            return vga16_clippoint (gc,*px1,*px2);
        if (*py1 > *py2) {
            h = *py1 - *py2 + 1;
            r1 = vga16_clipvline (gc, px1, py2, &h);
            if (r1 == CLIP_PARTIAL)    
                *py1 = *py2 + h - 1;    
            return r1;
        } else {
            h = *py2 - *py1 + 1;
            r1 = vga16_clipvline (gc, px1, py1, &h);
            if (r1 == CLIP_PARTIAL)    
                *py2 = *py1 + h - 1;    
            return r1;
        }    
    }
    
    if(*py1 == *py2) {
        if (*px1 > *px2) {
            w = *px1 - *px2 + 1;
            r1 = vga16_clipvline (gc, px2, py1, &w);
            if (r1 == CLIP_PARTIAL)    
                *px1 = *px2 + w - 1;    
            return r1;
        } else {
            w = *px2 - *px1 + 1;
            r1 = vga16_clipvline (gc, px2, py2, &w);
            if (r1 == CLIP_PARTIAL)    
                *px2 = *px1 + w - 1;    
            return r1;
        }    
    }
    
    return cs_clipline(gc,px1,py1,px2,py2,&clip_first,&clip_last);    
}

static int vga16_clipbox (PGC_VGA16 gc,int * px,int * py, int * pw,int *ph)
{
    if (*pw == 1) 
        return vga16_clipvline(gc, px, py, ph);    
    
    if (*ph == 1)
        return vga16_cliphline(gc, px, py, pw);    

    if (gc->doclip) {
        if ( (*px >= gc->clipmaxx) || (*py >= gc->clipmaxy) || 
             (*px + *pw - 1 < gc->clipminx) || (*py + *ph - 1 < gc->clipminy) )
                return CLIP_INVISIBLE;

        if ( (*px >= gc->clipminx) && (*py >= gc->clipminy) && 
             (*px + *pw -1 < gc->clipmaxx) && (*py + *ph - 1 < gc->clipmaxy) )
                return CLIP_VISIBLE;
            
        if (*px < gc->clipminx) {
            *pw -= gc->clipminx - *px;
            *px = gc->clipminx;
        }
        if (*py < gc->clipminy) {
            *ph -= gc->clipminy - *py;
            *py = gc->clipminy;
        }
        if (*px + *pw - 1 >= gc->clipmaxx)    
            *pw = gc->clipmaxx - *px;
        if (*py + *ph - 1 >= gc->clipmaxy)    
            *ph = gc->clipmaxy - *py;
    } else {
        if ( (*px >= gc->xres) || (*py >= gc->yres) || 
                 (*px + *pw - 1 < 0) || (*py +*ph - 1 < 0) )
                return CLIP_INVISIBLE;

        if ( (*px >= 0) && (*py >= 0) && (*px + *pw -1 < gc->xres) 
            && (*py + *ph - 1 < gc->yres) )
                return CLIP_VISIBLE;
            
        if (*px < 0) {
            *pw += *px;    
            *px = 0;
        }
        if (*px + *pw - 1 >= gc->xres)    
            *pw = gc->xres - *px;

        if (*py < 0) {
            *ph += *py;    
            *py = 0;
        }
        if (*py + *ph - 1 >= gc->yres)    
            *ph = gc->yres - *py;
        
    }

    if (*pw <= 0 || *ph <= 0)
        return CLIP_INVISIBLE;

    return CLIP_PARTIAL;        
}

static int fillbox (GAL_GC gc, int x, int y, int w, int h, gal_pixel pixel)
{
    GC_VGA16* mygc = gc.gc_vga16;

    if ((w <= 0) || (h <= 0)) return -1;

    BLOCK_DRAW_SEM;

    if (vga16_clipbox (mygc, &x, &y, &w, &h)) {
        memset (mygc->scanline, mygc->gr_foreground, w);
        if (mygc->fb_buff) {
            unsigned char* dst = mygc->fb_buff + y * mygc->pitch + x;
            while (h--) {
                memcpy (dst, mygc->scanline, w);
                dst += mygc->pitch;
            }
        }
        else {
            while (h--) {
                vga_drawscansegment (mygc->scanline, x, y++, w);
            }
        }
    }

    UNBLOCK_DRAW_SEM;
    return 0;
}

static gal_uint8* putbox_helper (GC_VGA16* gc, int* px, int* py, int* pw, int* ph, void* buf)
{
    int x = *px, y = *py, w = *pw, h = *ph;
    gal_uint8 *src = (gal_uint8*) buf;
    int srcwidth = *pw;

    if (gc->doclip) {
        if (y < gc->clipminy) {
            h -= gc->clipminy - y;
            src += (gc->clipminy - y) * srcwidth;
            y = gc->clipminy;
        }
        if (x < gc->clipminx) {
            w -= gc->clipminx - x;
            src += gc->clipminx - x;
            x = gc->clipminx;
        }        
        if (y + h - 1 >= gc->clipmaxy) 
            h =  gc->clipmaxy- y;
        if (x + w - 1 >= gc->clipmaxx) 
            w =  gc->clipmaxx- x;
    }
    else {
        if ( y < 0 ) {
            h += y;
            src -= y * srcwidth;
            y = 0;
        }
        if ( x < 0 ) {
            w += x;
            src -= x;
            x = 0;
        }        
        if ( y + h  -1 >= gc->yres) 
            h = gc->yres - y ;
        if ( x + w  -1 >= gc->xres) 
            w = gc->xres - x ;
    }

    *px = x; *py = y;
    *pw = w; *ph = h;

    return src;
}

static int putbox (GAL_GC gc, int x, int y, int w, int h, void* buf)
{
    GC_VGA16* mygc = gc.gc_vga16;
    gal_uint8* src;
    int srcw = w;

    if ((w <= 0) || (h <= 0)) return -1;
    
    BLOCK_DRAW_SEM;

    if (mygc->doclip) {
        if ((x + w - 1 < mygc->clipminx) || (x >= mygc->clipmaxx))
            goto inv_args;
        if ((y + h - 1 < mygc->clipminy) || (y >= mygc->clipmaxy))
            goto inv_args;
    } else {
        if ((x + w - 1 < 0) || (x >= mygc->xres))
            goto inv_args;
        if ((y + h - 1 < 0) || (y >= mygc->yres))
            goto inv_args;
    }
    
    src = putbox_helper (mygc, &x, &y, &w, &h, buf);

    if (mygc->fb_buff) {
        gal_uint8* dst;

        dst = mygc->fb_buff + y * mygc->pitch + x;
        while (h--) {
            memcpy (dst, src, w);
            dst += mygc->pitch;
            src += srcw;
        }
    }
    else {
        while (h--) {
            vga_drawscansegment (src, x, y++, w);
            src += srcw;
        }
    }

inv_args:
    UNBLOCK_DRAW_SEM;
    return 0;
}

static int putboxmask (GAL_GC gc, int x, int y, int w, int h, void* buf, gal_pixel cxx)
{
    GC_VGA16* mygc = gc.gc_vga16;
    gal_uint8* src;
    int srcw = w;

    if ((w <= 0) || (h <= 0)) return -1;

    BLOCK_DRAW_SEM;

    if (mygc->doclip) {
        if ((x + w - 1 < mygc->clipminx) || (x >= mygc->clipmaxx))
            goto inv_args;
        if ((y + h - 1 < mygc->clipminy) || (y >= mygc->clipmaxy))
            goto inv_args;
    } else {
        if ((x + w - 1 < 0) || (x >= mygc->xres))
            goto inv_args;
        if ((y + h - 1 < 0) || (y >= mygc->yres))
            goto inv_args;
    }
    
    src = putbox_helper (mygc, &x, &y, &w, &h, buf);

    if (mygc->fb_buff) {
        gal_uint8* dst;

        dst = mygc->fb_buff + y * mygc->pitch + x;
        while (h--) {
            int i;
            for (i = 0; i < w; i++) {
                if (src [i] != cxx)
                    dst [i] = src [i];
            }
            dst += mygc->pitch;
            src += srcw;
        }
    }
    else {
        // FIXME:
        while (h--) {
            vga_drawscansegment (src, x, y++, w);
            src += srcw;
        }
    }

inv_args:
    UNBLOCK_DRAW_SEM;
    return 0;
}


static gal_uint8* getbox_helper (GC_VGA16* gc, int* px, int* py, int* pw, int* ph, void* buf)
{
    int x = *px, y = *py, w = *pw, h = *ph;
    gal_uint8 *dst = (gal_uint8*)buf;
    int dstwidth = *pw;

    if ( y < 0 ) {
        h += y;
        dst -= y * dstwidth;
        y = 0;
    }
    if ( x < 0 ) {
        w += x;
        dst -= x;
        x = 0;
    }        
    if ( y + h  -1 >= gc->yres) 
        h = gc->yres - y ;
    if ( x + w  -1 >= gc->xres) 
        w = gc->xres - x ;

    *px = x; *py = y;
    *pw = w; *ph = h;

    return dst;
}

static int getbox (GAL_GC gc, int x, int y, int w, int h, void* buf)
{
    GC_VGA16* mygc = gc.gc_vga16;
    gal_uint8* dst;
    int dstw = w;

    if ((w <= 0) || (h <= 0)) return -1;

    if ((x + w - 1 < 0) || (x >= mygc->xres))
        return -1;
    if ((y + h - 1 < 0) || (y >= mygc->yres))
        return -1;

    dst = getbox_helper (mygc, &x, &y, &w, &h, buf);

    if (mygc->fb_buff) {
        gal_uint8* src;

        src = mygc->fb_buff + y * mygc->pitch + x;
        while (h--) {
            memcpy (dst, src, w);
            dst += dstw;
            src += mygc->pitch;
        }
    }
    else {
        while (h--) {
            vga_getscansegment (dst, x, y++, w);
            dst += dstw;
        }
    }
    return 0;
}

static inline int muldiv64 (int m1, int m2, int d)
{
    long long int mul = (long long int) m1 * m2;

    return (int) (mul / d);
}

/* This is a DDA-based algorithm. */
/* Iteration over target bitmap. */
static int vga16_scalebox (int w1, int h1, void *_dp1, int w2, int h2, void *_dp2)
{
    gal_uint8 *dp1 = _dp1;
    gal_uint8 *dp2 = _dp2;
    int xfactor;
    int yfactor;
    int y, sy;

    if (w2 == 0 || h2 == 0)
        return 1;

    xfactor = muldiv64 (w1, 65536, w2);        /* scaled by 65536 */
    yfactor = muldiv64 (h1, 65536, h2);        /* scaled by 65536 */

    sy = 0;
    for (y = 0; y < h2;) {
        int sx = 0;
        gal_uint8 *dp2old = dp2;
        int x;
        x = 0;
        while (x < w2 - 8) {
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
        while (x < w2) {
            *(dp2 + x) = *(dp1 + (sx >> 16));
            sx += xfactor;
            x++;
        }
        dp2 += w2;
        y++;
        while (y < h2) {
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
        dp1 = _dp1 + (sy >> 16) * w1;
    }

    return 0;
}

static int scalebox (GAL_GC gc, int sw, int sh, void* srcbuf,
        int dw, int dh, void* dstbuf)
{
    return vga16_scalebox (sw, sh, srcbuf, dw, dh, dstbuf);
}

static void copybox_helper1 (GC_VGA16* gc, int x1, int y1, int w, int h, int x2, int y2)
{
    gal_uint8 *svp, *dvp;

    if (y1 >= y2) {
        if (y1 == y2 && x2 >= x1) {    /* tricky */
            int i;
            if (x1 == x2)
                return;
            /* use a temporary buffer to store a line */
            /* using reversed movs would be much faster */
            svp = (gal_uint8 *) gc->fb_buff + y1 * gc->pitch + x1;
            dvp = (gal_uint8 *) gc->fb_buff + y2 * gc->pitch + x2;
            for (i = 0; i < h; i++) {
                memcpy (gc->scanline, svp, w);
                memcpy (dvp, gc->scanline, w);
                svp += gc->pitch;
                dvp += gc->pitch;
            }
        } else {        /* copy from top to bottom */
            int i;
            svp = (gal_uint8 *) gc->fb_buff + y1 * gc->pitch + x1;
            dvp = (gal_uint8 *) gc->fb_buff + y2 * gc->pitch + x2;
            for (i = 0; i < h; i++) {
                memcpy(dvp, svp, w);
                svp += gc->pitch;
                dvp += gc->pitch;
            }
        }
    } else {            /* copy from bottom to top */
        int i;

        svp = (gal_uint8 *) gc->fb_buff + (y1 + h) * gc->pitch + x1;
        dvp = (gal_uint8 *) gc->fb_buff + (y2 + h) * gc->pitch + x2;
        for (i = 0; i < h; i++) {
            svp -= gc->pitch;
            dvp -= gc->pitch;
            memcpy (dvp, svp, w);
        }
    }
}

static void copybox_helper2 (GC_VGA16* gc, int x1, int y1, int w, int h, int x2, int y2)
{
    int i;

    if (y1 == y2 && x2 == x1)
        return;

    if (y1 >= y2) {     /* copy from top to bottom */
        for (i = 0; i < h; i++) {
            vga_getscansegment (gc->scanline, x1, y1++, w);
            vga_drawscansegment (gc->scanline, x2, y2++, w);
        }
    } else {            /* copy from bottom to top */
        y1 += h;
        y2 += h;
        for (i = 0; i < h; i++) {
            vga_getscansegment (gc->scanline, x1, y1--, w);
            vga_drawscansegment (gc->scanline, x2, y2--, w);
        }
    }
}

static int copybox (GAL_GC gc, int x, int y, int w, int h, int nx, int ny)
{
    GC_VGA16* mygc = gc.gc_vga16;

    if ((w <= 0) || (h <= 0)) return -1;

    // src clip to screen
    if ((x >= mygc->xres) || (x + w - 1 < 0)) return -1;
    if ((y >= mygc->yres) || (y + h - 1 < 0)) return -1;
    if (x < 0) { nx -= x; w += x; x = 0; }
    if (y < 0) { ny -= y; h += y; y = 0; }
    if (x + w - 1 >= mygc->xres) w = mygc->xres - x;
    if (y + h - 1 >= mygc->yres) h = mygc->yres - y;        

    BLOCK_DRAW_SEM;

    // dst do clip
    if (mygc->doclip) {
        if ((nx + w - 1 < mygc->clipminx) || (nx >= mygc->clipmaxx))
            goto inv_args;
        if ((ny + h - 1 < mygc->clipminy) || (ny >= mygc->clipmaxy))
            goto inv_args;
        if (nx < mygc->clipminx) {
            x += mygc->clipminx - nx;
            w -= mygc->clipminx - nx;
            nx = mygc->clipminx;
        } 
        if (nx + w - 1 >= mygc->clipmaxx) {
            w = mygc->clipmaxx - nx;
        }
        if (ny < mygc->clipminy) {
            y += mygc->clipminy - ny;
            h -= mygc->clipminy - ny;
            ny = mygc->clipminy;
        }
        if (ny + h - 1 >= mygc->clipmaxy) {
            h = mygc->clipmaxy - ny;
        }    
    } else {
        if ((nx + w - 1 < 0) || (nx >= mygc->xres))
            goto inv_args;
        if ((ny + h - 1 < 0) || (ny >= mygc->yres))
            goto inv_args;
        if (nx < 0) {
            x -= nx;
            w += nx;
            nx = 0;
        } 
        if (nx + w - 1 >= mygc->xres) {
            w = mygc->xres- nx;
        }
        if (ny < 0) {
            y -= ny;
            h += ny;
            ny = 0;
        }
        if (ny + h - 1 >= mygc->yres) {
            h = mygc->yres- ny;
        }    
    }
    
    if ((w <= 0) || (h <= 0))
        goto inv_args;

    if ((x < 0) || (x + w - 1 >= mygc->xres))
        goto inv_args;
    if ((y < 0) || (y + h - 1 >= mygc->yres))
        goto inv_args;

    if (mygc->doclip) {
        if ((nx < mygc->clipminx) || (nx + w - 1 >= mygc->clipmaxx)) 
            goto inv_args;
        if ((ny < mygc->clipminy) || (ny + h - 1 >= mygc->clipmaxy)) 
            goto inv_args;
    } else {
        if ((nx < 0) || (nx + w - 1 >= mygc->xres)) 
            goto inv_args;
        if ((ny < 0) || (ny + h - 1 >= mygc->yres)) 
            goto inv_args;
    }

    if (mygc->fb_buff)
        copybox_helper1 (mygc, x, y, w, h, nx, ny);
    else
        copybox_helper2 (mygc, x, y, w, h, nx, ny);

inv_args:
    UNBLOCK_DRAW_SEM;
    return 0;
}

/* 
 * Must set destination graphics context
 */
static int crossblit (GAL_GC src, int sx, int sy, int w, int h,
        GAL_GC dst, int dx, int dy)
{
#ifdef _LITE_VERSION
    GAL_GC gc = dst;
#endif

    GC_VGA16* dstgc = dst.gc_vga16;
    GC_VGA16* srcgc = src.gc_vga16;

    if ((w <= 0) || (h <= 0)) return -1;

    if (src.gc_vga16 == dst.gc_vga16) {
        return copybox (src, sx, sy, w, h, dx, dy);
    }
    
    //src clip to screen
    if ((sx >= src.gc_vga16->xres) || (sx + w - 1 < 0)) return -1;
    if ((sy >= src.gc_vga16->yres) || (sy + h - 1 < 0)) return -1;
    if (sx < 0) { dx -= sx; w += sx; sx = 0; }
    if (sy < 0) { dy -= sy; h += sy; sy = 0; }
    
    if (sx + w - 1 >= src.gc_vga16->xres) w = src.gc_vga16->xres - sx;
    if (sy + h - 1 >= src.gc_vga16->yres) h = src.gc_vga16->yres - sy;        

    BLOCK_DRAW_SEM;

    //dst do clip
    if (dst.gc_vga16->doclip) {
        if ((dx + w - 1 < dst.gc_vga16->clipminx) || (dx >= dst.gc_vga16->clipmaxx))
            goto inv_args;
        if ((dy + h - 1 < dst.gc_vga16->clipminy) || (dy >= dst.gc_vga16->clipmaxy))
            goto inv_args;
        if (dx < dst.gc_vga16->clipminx) {
            sx += dst.gc_vga16->clipminx - dx;
            w -= dst.gc_vga16->clipminx - dx;
            dx = dst.gc_vga16->clipminx;
        } 
        if (dx + w - 1 >= dst.gc_vga16->clipmaxx) {
            w = dst.gc_vga16->clipmaxx - dx;
        }
        if (dy < dst.gc_vga16->clipminy) {
            sy += dst.gc_vga16->clipminy - dy;
            h -= dst.gc_vga16->clipminy - dy;
            dy = dst.gc_vga16->clipminy;
        }
        if (dy + h - 1 >= dst.gc_vga16->clipmaxy) {
            h = dst.gc_vga16->clipmaxy - dy;
        }    
    } else {
        if ((dx + w - 1 < 0) || (dx >= dst.gc_vga16->xres))
            goto inv_args;
        if ((dy + h - 1 < 0) || (dy >= dst.gc_vga16->yres))
            goto inv_args;
        if (dx < 0) {
            sx -= dx;
            w += dx;
            dx = 0;
        } 
        if (dx + w - 1 >= dst.gc_vga16->xres) {
            w = dst.gc_vga16->xres- dx;
        }
        if (dy < 0) {
            sy -= dy;
            h += dy;
            dy = 0;
        }
        if (dy + h - 1 >= dst.gc_vga16->yres) {
            h = dst.gc_vga16->yres- dy;
        }    
    }
    
    if ((w <= 0) || (h <= 0))
        goto inv_args;

    if ((sx < 0) || (sx + w - 1 >= src.gc_vga16->xres))
        goto inv_args;
    if ((sy < 0) || (sy + h - 1 >= src.gc_vga16->yres))
        goto inv_args;

    if (dst.gc_vga16->doclip) {
        if ((dx < dst.gc_vga16->clipminx) || (dx + w - 1 >= dst.gc_vga16->clipmaxx)) 
            goto inv_args;
        if ((dy < dst.gc_vga16->clipminy) || (dy + h - 1 >= dst.gc_vga16->clipmaxy))
            goto inv_args;
    } else {
        if ((dx < 0) || (dx + w - 1 >= dst.gc_vga16->xres)) 
            goto inv_args;
        if ((dy < 0) || (dy + h - 1 >= dst.gc_vga16->yres)) 
            goto inv_args;
    }

    if (dstgc->fb_buff) {
        gal_uint8 *dst = dstgc->fb_buff + dy * dstgc->pitch + dx;

        if (srcgc->fb_buff) {
            gal_uint8 *src = srcgc->fb_buff + sy * srcgc->pitch + sx;
            while (h--) {
                memcpy (dst, src, w);
                dst += dstgc->pitch;
                src += srcgc->pitch;
            }
        }
        else {
            while (h--) {
                vga_getscansegment (dst, sx, sy++, w);
                dst += dstgc->pitch;
            }
        }
    }
    else {
        gal_uint8 *src = srcgc->fb_buff + sy * srcgc->pitch + sx;

        if (srcgc->fb_buff) {
            while (h--) {
                vga_drawscansegment (src, dx, dy++, w);
                src += srcgc->pitch;
            }
        }
        else {
            while (h--) {
                vga_getscansegment (srcgc->scanline, sx, sy++, w);
                vga_drawscansegment (srcgc->scanline, dx, dy++, w);
            }
        }
    }

inv_args:
    UNBLOCK_DRAW_SEM;
    return 0;
}

static int drawhline (GAL_GC gc, int x, int y, int w, gal_pixel pixel)
{
    GC_VGA16* mygc = gc.gc_vga16;

    BLOCK_DRAW_SEM;

    if (vga16_cliphline (mygc, &x, &y, &w)) {
        if (mygc->fb_buff) {
            gal_uint8 *dst = mygc->fb_buff + y * mygc->pitch + x;
            memset (dst, mygc->gr_foreground, w);
        }
        else {
            memset (mygc->scanline, mygc->gr_foreground, w);
            vga_drawscansegment (mygc->scanline, x, y, w);
        }
    }

    UNBLOCK_DRAW_SEM;
    return 0;
}

static int drawvline (GAL_GC gc, int x, int y, int h, gal_pixel pixel)
{
    GC_VGA16* mygc = gc.gc_vga16;

    if (h <= 0 ) return -1;
    
    BLOCK_DRAW_SEM;

    if (vga16_clipvline (mygc, &x, &y, &h)) {
        if (mygc->fb_buff) {
            gal_uint8 *dst = mygc->fb_buff + y * mygc->pitch + x;
            while (h--) {
                *dst = mygc->gr_foreground;
                dst += mygc->pitch;
            }
        }
        else {
            while (h--) {
                vga_drawpixel (x, y++);
            }
        }
    }

    UNBLOCK_DRAW_SEM;
    return 0;
}

/*
 *  Pixel operations
 */
static int drawpixel (GAL_GC gc, int x, int y, gal_pixel pixel)
{
    GC_VGA16* mygc = gc.gc_vga16;

    BLOCK_DRAW_SEM;

    if (vga16_clippoint (mygc, x, y)) {
        if (mygc->fb_buff) {
            gal_uint8 *dst = mygc->fb_buff + y * mygc->pitch + x;
            *dst = mygc->gr_foreground;
        }
        else
            vga_drawpixel (x, y);
    }

    UNBLOCK_DRAW_SEM;
    return 0;
}

static int getpixel (GAL_GC gc, int x, int y, gal_pixel* pixel)
{
    GC_VGA16* mygc = gc.gc_vga16;

    if ((x >= 0) && (x < mygc->xres) && (y >= 0) && (y < mygc->yres)) {
        if (mygc->fb_buff) {
            gal_uint8 *src = mygc->fb_buff + y * mygc->pitch + x;
            *pixel = *src;
        }
        else
            *pixel = vga_getpixel (x, y);
    }

    return 0;
}

static void drawline_helper (GC_VGA16* gc, int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int ax = ABS(dx) << 1;
    int ay = ABS(dy) << 1;
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;

    int x = x1;
    int y = y1;
    gal_uint8* buf = gc->fb_buff + gc->pitch * y + x;

    if (ax > ay) {
        int d = ay - (ax >> 1);
        while (x != x2) {
            *buf = gc->gr_foreground;

            if (d > 0 || (d == 0 && sx == 1)) {
                y += sy;
                buf += gc->pitch * sy;
                d -= ax;
            }
            x += sx;
            buf += sx;
            d += ay;
        }
    } else {
        int d = ax - (ay >> 1);
        while (y != y2) {
            *buf = gc->gr_foreground;

            if (d > 0 || (d == 0 && sy == 1)) {
                x += sx;
                buf += sx;
                d -= ay;
            }
            y += sy;
            buf += gc->pitch * sy;
            d += ax;
        }
    }

    *buf = gc->gr_foreground;
}

static int line (GAL_GC gc, int x1, int y1, int x2, int y2, gal_pixel pixel)
{
    GC_VGA16* mygc = gc.gc_vga16;

    BLOCK_DRAW_SEM;

    if (vga16_clipline (mygc, &x1, &y1, &x2, &y2)) {
        if (mygc->fb_buff)
            drawline_helper (mygc, x1, y1, x2, y2);
        else
            vga_drawline (x1, y1, x2, y2);
    }

    UNBLOCK_DRAW_SEM;
    return 0;
}

/* 
 * NOTE: must be normalized rect.
 */
static int rectangle (GAL_GC gc, int l, int t, int r, int b, gal_pixel pixel)
{
    int w = r - l;
    int h = b - t;

    drawhline (gc, l, t, w, pixel);
    drawvline (gc, l, t, h, pixel);
    drawhline (gc, l, b, w, pixel);
    drawvline (gc, r, t, h, pixel);
    
    return 0;
}

static int circle (GAL_GC gc, int x, int y, int r, gal_pixel pixel)
{
    return 0;
}

static void panic (int exitcode)
{
    fprintf (stderr, "MiniGUI Panic. Exit Code: %d.\n", exitcode);

    vga_setmode (TEXT);

    _exit (exitcode);
}

#ifdef _LITE_VERSION
/* the following varibles defined in svgalib.
 * For the clients, we should not call vga_init and vga_setmode functions,
 * so we set these varibles by ourselves.
 */
struct info {
    int xdim;
    int ydim;
    int colors;
    int xbytes;
    int bytesperpixel;
};

extern unsigned long int __svgalib_banked_mem_base, __svgalib_banked_mem_size;
extern unsigned long int __svgalib_mmio_base, __svgalib_mmio_size;
extern unsigned long int __svgalib_linear_mem_base, __svgalib_linear_mem_size;
extern unsigned long int __svgalib_mmio_base, __svgalib_mmio_size;

extern unsigned char * BANKED_MEM_POINTER, * LINEAR_MEM_POINTER, *MMIO_POINTER;
extern unsigned char * B8000_MEM_POINTER;

extern struct info __svgalib_cur_info;      /* current video parameters */
extern int __svgalib_cur_mode;              /* current video mode       */
extern unsigned char *__svgalib_graph_mem;  /* graphics memory frame */
extern int __svgalib_modeX;
extern int __svgalib_screenon;
extern int __svgalib_chipset;

extern int __svgalib_mem_fd;

#define CI      __svgalib_cur_info
#define GM      __svgalib_graph_mem
#define CM      __svgalib_cur_mode
#define CHIPSET __svgalib_chipset
#define SCREENON __svgalib_screenon
#define MODEX   __svgalib_modeX

static BOOL init_vga16_client (void)
{
#if 1
    if (ioperm (0x3b4, 0x3df - 0x3b4 + 1, 1) < 0) {
#else
    if (iopl(3) < 0) {
#endif
        printf ("VGA16 GAL engine for clients: Can not get I/O permissions.\n");
        return FALSE;
    }

    if ((__svgalib_mem_fd = open ("/dev/mem", O_RDWR)) < 0) {
        printf ("VGA16 GAL engine for clients: Can not open /dev/mem.\n");
        return FALSE;
    }

    __svgalib_banked_mem_base = 0xa0000;
    __svgalib_banked_mem_size = 0x10000;
    BANKED_MEM_POINTER = mmap((caddr_t) 0, __svgalib_banked_mem_size,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            __svgalib_mem_fd, __svgalib_banked_mem_base);

    __svgalib_linear_mem_size = 0;
    __svgalib_mmio_size = 0;
    
    B8000_MEM_POINTER = mmap((caddr_t) 0, 32768,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            __svgalib_mem_fd, (off_t) 0xb8000);

    close (__svgalib_mem_fd);

    GM = (unsigned char *) BANKED_MEM_POINTER;

    if ((long) GM < 0) {
        printf ("VGA16 GAL engine for clients: mmap error.\n");
        return FALSE;
    }

    graph_mem = GM;

    CI.xdim = 640;
    CI.ydim = 480;
    CI.colors = 16;
    CI.xbytes = 80;
    CI.bytesperpixel = 0;

    CM = G640x480x16;
    MODEX = FALSE;
    SCREENON = TRUE;
    CHIPSET = VGA;

    return TRUE;
}

static void term_vga16_client (void)
{
    munmap (BANKED_MEM_POINTER, __svgalib_banked_mem_size);
    munmap (B8000_MEM_POINTER, 32768);
}

#endif

/************ Initialization and termination of VGA16 GAL engine **************/
BOOL InitVGA16 (GFX* gfx)
{
    GC_VGA16* gc;
    int i;
    
    if ((gc = malloc (sizeof (GC_VGA16))) == NULL) {
        fprintf (stderr, "VGA16 GAL: allocating physical GC error!\n");
        return FALSE;
    }

    gc->xres = 640;
    gc->yres = 480;
    gc->pitch = 0;
    gc->gr_background = 0;
    gc->gr_foreground = 15;
    gc->fb_buff = NULL;

    vga_setmousesupport (0);
#ifdef _LITE_VERSION
    if (!mgIsServer) {
        if (!init_vga16_client ())
            return FALSE;
    }
    else {
#endif
        vga_init ();
        if (vga_setmode (G640x480x16)) {
            fprintf (stderr, "VGA16 GAL: can not set correct mode.\n");
            return FALSE;
        }
#ifdef _LITE_VERSION
    }
#endif

    gfx->phygc.gc_vga16 = gc; 
    gfx->bytes_per_phypixel = 1;
    gfx->bits_per_phypixel  = 4;
    gfx->width_phygc        = 640;
    gfx->height_phygc       = 480;
    gfx->colors_phygc       = 16;
    gfx->grayscale_screen   = FALSE;
    
    gfx->bytesperpixel      = bytesperpixel;
    gfx->bitsperpixel       = bitsperpixel;
    gfx->width              = width;
    gfx->height             = height;
    gfx->colors             = colors;

    //now functions
    gfx->allocategc         = allocategc;
    gfx->freegc             = freegc;
    gfx->enableclipping     = enableclipping;
    gfx->disableclipping    = disableclipping;
    gfx->setclipping        = setclipping;
    gfx->getclipping        = getclipping;
    gfx->getbgcolor         = getbgcolor;
    gfx->setbgcolor         = setbgcolor;
    gfx->getfgcolor         = getfgcolor;
    gfx->setfgcolor         = setfgcolor;
    gfx->mapcolor           = mapcolor;
    gfx->unmappixel         = unmappixel;
    gfx->getpalette         = getpalette;
    gfx->setpalette         = setpalette;
    gfx->setcolorfulpalette = setcolorfulpalette;

    gfx->boxsize            = boxsize;
    gfx->fillbox            = fillbox;
    gfx->putbox             = putbox;
    gfx->putboxmask         = putboxmask;
    gfx->getbox             = getbox;
    gfx->scalebox           = scalebox;
    gfx->copybox            = copybox;
    gfx->crossblit          = crossblit;
    gfx->drawhline          = drawhline;
    gfx->drawvline          = drawvline;
    gfx->drawpixel          = drawpixel;
    gfx->getpixel           = getpixel;

    gfx->line               = line;
    gfx->rectangle          = rectangle;

    gfx->panic              = panic;
    gfx->circle             = circle;

    for (i = 0; i < 17; i++)
        SysPixelIndex [i] = mapcolor (gfx->phygc, (GAL_Color*)(SysPixelColor + i));

#ifdef _LITE_VERSION
    if (mgIsServer)
#endif
        setcolorfulpalette (gfx->phygc);
    
    setclipping (gfx->phygc, 0, 0, gc->xres - 1, gc->yres - 1);

    return TRUE;
}

void TermVGA16 (GFX* gfx)
{
#ifdef _LITE_VERSION
    if (mgIsServer)
        vga_setmode (TEXT);
    else
        term_vga16_client ();
#else
    vga_setmode (TEXT);
#endif
}

#endif /* _VGA16_GAL */

