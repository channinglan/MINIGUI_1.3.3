/*
** $Id: svgalib.c,v 1.19 2003/09/25 04:08:55 snig Exp $
**
** svgalib.c: Low Level Graphics Engine based on SVGALib
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Create date: 2000/06/11 by Wei Yongming
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
#include <vga.h>
#include <vgagl.h>
#include <vgamouse.h>
#include <vgakeyboard.h>
#ifdef _LIBGGI
#include <ggi/ggi.h>
#endif

#include "common.h"
#include "minigui.h"
#include "gal.h"
#include "misc.h"
#include "svgalib.h"

/**********************  Low Level Graphics Operations **********************/
// GC properties
static int bytesperpixel (GAL_GC gc)
{
    return gc.gc->bytesperpixel;
}

static int bitsperpixel (GAL_GC gc)
{
    return gc.gc->bitsperpixel;
}

static int width (GAL_GC gc)
{
    return gc.gc->width;
}

static int height (GAL_GC gc)
{
    return gc.gc->height;
}

static int colors (GAL_GC gc)
{
    return gc.gc->colors;
}

// Allocation and release of graphics context
static int allocategc (GAL_GC gc, int width, int height, int depth, 
                GAL_GC* newgc)
{
    unsigned char* vbuf;
    GraphicsContext* svga_gc;
    
    vbuf = malloc (width * height * depth);
    if (vbuf == NULL)
        return -1;

    gl_setcontextvirtual (width, height, depth,
                gc.gc->bitsperpixel, vbuf);
    svga_gc = gl_allocatecontext ();
    if (svga_gc == NULL) {
        free (vbuf);
        return -1;
    }

    gl_getcontext (svga_gc);

    newgc->gc = svga_gc;

    return 0;
}

static void freegc (GAL_GC gc)
{
    gl_freecontext (gc.gc);
    free (gc.gc);
}

static void setgc (GAL_GC gc)
{
    gl_setcontext (gc.gc);
}


// Clipping of graphics context
static void enableclipping (GAL_GC gc)
{
    gl_enableclipping ();
}

static void disableclipping (GAL_GC gc)
{
    gl_disableclipping ();
}

static int setclipping (GAL_GC gc, int x1, int y1, int x2, int y2)
{
    gl_setclippingwindow (x1, y1, x2, y2);

    return 0;
}

static int getclipping (GAL_GC gc, int* x1, int* y1, int* x2, int* y2)
{
    *x1 = gc.gc->clipx1;
    *x2 = gc.gc->clipx2;
    *y1 = gc.gc->clipy1;
    *y2 = gc.gc->clipy2;

    return 0;
}

// Background and foreground colors
static int getbgcolor (GAL_GC gc, gal_pixel* color)
{
    return 0;
}

static int getfgcolor (GAL_GC gc, gal_pixel* color)
{
    return 0;
}

/* Will be set to NULL
static int setbgcolor (GAL_GC gc, gal_pixel color)
{
}

static int setfgcolor (GAL_GC gc, gal_pixel color)
{
}
*/

// Convertion between GAL_Color and gal_pixel
static gal_pixel mapcolor (GAL_GC gc, GAL_Color *color)
{
    return (gal_pixel) gl_rgbcolor (color->r, color->g, color->b);
}

static int unmappixel (GAL_GC gc, gal_pixel pixel, GAL_Color* color)
{
    gal_uint32 v;
    int r, g, b;

    switch (gc.gc->bitsperpixel) {
    case 8:
        gl_getpalettecolor (pixel, &r, &g, &b);
        color->r = r << 2;
        color->g = g << 2;
        color->b = b << 2;
        break;

    case 24:
    case 32:
        if (gc.gc->modeflags & MODEFLAG_32BPP_SHIFT8)
            v = pixel >> 8;
        
        color->r = (gal_uint8) ((pixel >> 16) & 0xFF);
        color->g = (gal_uint8) ((pixel >> 8) & 0xFF);
        color->b = (gal_uint8) (pixel & 0xFF);
        break;

    case 15:
        color->r = (gal_uint8)((pixel & 0x7C00) >> 7) | 0x07;
        color->g = (gal_uint8)((pixel & 0x03E0) >> 2) | 0x07;
        color->b = (gal_uint8)((pixel & 0x001F) << 3) | 0x07;
        break;
        
    case 16:
        color->r = (gal_uint8)((pixel & 0xF800) >> 8) | 0x07;
        color->g = (gal_uint8)((pixel & 0x07E0) >> 3) | 0x03;
        color->b = (gal_uint8)((pixel & 0x001F) << 3) | 0x07;
        break;
        
    case 4:
        color->r = SysPixelColor [pixel].r;
        color->g = SysPixelColor [pixel].g;
        color->b = SysPixelColor [pixel].b;
        break;
    }
    
    return 0;
}

#if 0
static int packcolors (GAL_GC gc, void* buf, GAL_Color* colors, int len)
{
    int ww = len;
    gal_pixel p;

    gal_uint8  *buf1 = (gal_uint8  *) buf;
    gal_uint16 *buf2 = (gal_uint16 *) buf;
    gal_uint32 *buf4 = (gal_uint32 *) buf;

    switch ( bytesperpixel (gc)) {

    case 1:
        for (; ww > 0; ww--) {
            p = gl_rgbcolor (colors->r, 
                    colors->g, colors->b);
            colors ++;
            *buf1++ = (gal_uint8)p;
        }
        break;

    case 2:
        for (; ww > 0; ww--) {
            p = gl_rgbcolor (colors->r, 
                    colors->g, colors->b);
            colors ++;
            *buf2++ = (gal_uint16)p;
        }
        break;

    case 3:
        for (; ww > 0; ww--) {
            p = gl_rgbcolor (colors->r, 
                    colors->g, colors->b);
            colors ++;

            *buf1++ = (gal_uint8)p; p >>= 8;
            *buf1++ = (gal_uint8)p; p >>= 8;
            *buf1++ = (gal_uint8)p;
        }
        break;

    case 4:
        for (; ww > 0; ww--) {
            p = gl_rgbcolor (colors->r, 
                    colors->g, colors->b);
            colors ++;

            *buf4++ = p;
        }
        break;
    }

    return 0;
}

static int unpackpixels (GAL_GC gc, void* buf, GAL_Color* colors, int len)
{
    int ww = len;
    gal_pixel p = 0;

    gal_uint8  *buf1 = (gal_uint8  *) buf;
    gal_uint16 *buf2 = (gal_uint16 *) buf;
    gal_uint32 *buf4 = (gal_uint32 *) buf;

    for (; ww > 0; ww--) {
        switch ( bytesperpixel (gc)) {
        case 1:
            p = *buf1; buf1 ++;
            break;

        case 2:
            p = *buf2; buf2 ++;
            break;

        case 3:
            p  = *buf1; p <<= 8; buf1 ++;
            p |= *buf1; p <<= 8; buf1 ++;
            p |= *buf1; buf1 ++;
            break;

        case 4:
            p  = *buf4;
            break;
        }

        unmappixel (gc, p, colors);
        colors ++;
    }
    
    return 0;
}
#endif

// Palette operations
static int getpalette (GAL_GC gc, int s, int len, GAL_Color* cmap)
{
    int i;
    int r, g, b;
    
    for (i = 0; i < len; i++) {
        gl_getpalettecolor (i + s, &r, &g, &b);
        cmap [i].r = r << 2;
        cmap [i].g = g << 2;
        cmap [i].b = b << 2;
    }
    
    return 0;
}

static int setpalette (GAL_GC gc, int s, int len, GAL_Color* cmap)
{
    int i;
    
    for (i = s; i < s + len; i++) {
        gl_setpalettecolor (i, cmap->r >> 2, cmap->g >> 2, cmap->b >> 2);
    }
    
    return 0;
}

static int setcolorfulpalette (GAL_GC gc)
{
    gl_setrgbpalette ();
    
    return 0;
}


// Box operations
static size_t boxsize (GAL_GC gc, int w, int h)
{
    return w * h * gc.gc->bytesperpixel;
}

static int fillbox (GAL_GC gc, int x, int y, int w, int h,
        gal_pixel pixel)
{
    gl_fillbox (x, y, w, h, (int)pixel);
    
    return 0;
}

static int putbox (GAL_GC gc, int x, int y, int w, int h, void* buf)
{
    gl_putbox (x, y, w, h, buf);
    
    return 0;
}

static int getbox (GAL_GC gc, int x, int y, int w, int h, void* buf)
{
    gl_getbox (x, y, w, h, buf);
    
    return 0;
}

static int putboxmask (GAL_GC gc, int x, int y, int w, int h, void* buf, gal_pixel transparent)
{
    gl_putboxmask (x, y, w, h, buf);
    
    return 0;
}

static int scalebox (GAL_GC gc, int sw, int sh, void* srcbuf,
        int dw, int dh, void* dstbuf)
{
    gl_scalebox (sw, sh, srcbuf, dw, dh, dstbuf);

    return 0;
}


static int copybox (GAL_GC gc, int x, int y, int w, int h, int nx, int ny)
{
    gl_copybox (x, y, w, h, nx, ny);

    return 0;
}

// Must set destination graphics context
static int crossblit (GAL_GC src, int sx, int sy, int sw, int sh,
        GAL_GC dst, int dx, int dy)
{
    gl_copyboxfromcontext (src.gc, sx, sy, sw, sh, dx, dy);

    return 0;
}


// Horizontal line operaions
static int drawhline (GAL_GC gc, int x, int y, int w, gal_pixel pixel)
{
    gl_hline (x, y, x + w, (int)pixel);

    return 0;
}


// Vertical line operations
static int drawvline (GAL_GC gc, int x, int y, int h, gal_pixel pixel)
{
    gl_line (x, y, x, y + h, (int)pixel);

    return 0;
}

// Pixel operations
static int drawpixel (GAL_GC gc, int x, int y, gal_pixel pixel)
{
    gl_setpixel (x, y, (int)pixel);

    return 0;
}

static int getpixel (GAL_GC gc, int x, int y, gal_pixel* pixel)
{
    *pixel = (gal_pixel) gl_getpixel (x, y);

    return 0;
}


// Other drawing
static int circle (GAL_GC gc, int x, int y, int r, gal_pixel pixel)
{
    gl_circle (x, y, r, (int)pixel);

    return 0;
}

static int line (GAL_GC gc, int x1, int y1, int x2, int y2, 
        gal_pixel pixel)
{
    gl_line (x1, y1, x2, y2, (int)pixel);
    
    return 0;
}

// NOTE: must be normalized rect.
static int rectangle (GAL_GC gc, int l, int t, int r, int b, 
        gal_pixel pixel)
{
    gl_hline (l, t, r, (int)pixel);
    gl_line (r, t, r, b, (int)pixel);
    gl_hline (l, b, r, (int)pixel);
    gl_line (l, b, l, t, (int)pixel);
    
    return 0;
}

#if 0
// Simple Character output
static int myputc (GAL_GC gc, int x, int y, char c)
{
    gl_writen (x, y, 1, &c);
    
    return 0;
}

static int putstr (GAL_GC gc, int x, int y, const char* str)
{
    gl_write (x, y, (char*)str);
    
    return 0;
}

static int getcharsize (GAL_GC gc, int* width, int* height)
{
    *width = 8;
    *height = 8;
    
    return 0;
}

static int setputcharmode (GAL_GC gc, int bkmode)
{
    gl_setwritemode (FONT_COMPRESSED + bkmode); 
    
    return 0;
}

static int setfontcolors (GAL_GC gc, gal_pixel fg, gal_pixel bg)
{
    gl_setfontcolors ((int)bg, (int)fg);
    
    return 0;
}
#endif

static void panic (int exitcode)
{
    fprintf (stderr, "MiniGUI Panic. Exit Code: %d.\n", exitcode);

    mouse_close ();
    keyboard_close ();
    vga_unlockvc ();
    vga_setmode (TEXT);

    exit (exitcode);
}

/******************  Initialization and termination of SVGALib **************/

// This function get the default vga mode.
static int GetDefaultMode ( void )
{
#ifdef _PREDEFMODE
    return _PREDEFMODE;
#else
    char szBuff [21];
    if (GetMgEtcValue ("SVGALib", "defaultmode", 
                           szBuff, 20) < 0 )
        return G320x200x256;

    if (vga_getmodenumber (szBuff) > 0)
        return vga_getmodenumber (szBuff);

    return G320x200x256;
#endif
}

BOOL InitSVGALib (GFX* gfx)
{
    int i;
    int vga_mode;
    GraphicsContext *physicalscreen;

    // Init vgalib.
    vga_init();

    // Test and set vga mode.
    if (!vga_hasmode (GetDefaultMode ()))
    {
        fprintf (stderr, 
            "GAL: the default mode is not supported by your vga card!\n");
        return FALSE;
    }

    vga_mode = GetDefaultMode(); 

    if (vga_getmodeinfo (vga_mode)->colors == 16)
        return FALSE;

    // Set vga mode information.
    vga_setmode (GetDefaultMode ());
    vga_lockvc();

    gl_setcontextvga (vga_mode);  /* Physical screen context. */

    physicalscreen = gl_allocatecontext ();
    gl_getcontext (physicalscreen);

    gl_enableclipping ();

    // Set palette. This only done when color number is 256
    if (vga_getcolors () == 256)
        gl_setrgbpalette ();

    if (vga_getcolors () != 16)
        for (i=0; i<17; i++)
            SysPixelIndex [i] = gl_rgbcolor (SysPixelColor[i].r,
                                             SysPixelColor[i].g, 
                                             SysPixelColor[i].b);

    gl_setfont (8, 8, gl_font8x8);
    gl_setwritemode (FONT_COMPRESSED + WRITEMODE_OVERWRITE);
    gl_setfontcolors (0, vga_white());

    gfx->phygc.gc           = physicalscreen;
    gfx->bytes_per_phypixel = physicalscreen->bytesperpixel;
    gfx->bits_per_phypixel  = physicalscreen->bitsperpixel;
    gfx->width_phygc        = physicalscreen->width;
    gfx->height_phygc       = physicalscreen->height;
    gfx->colors_phygc       = physicalscreen->colors;
    gfx->grayscale_screen   = FALSE;

    gfx->bytesperpixel      = bytesperpixel;
    gfx->bitsperpixel       = bitsperpixel;
    gfx->width              = width;
    gfx->height             = height;
    gfx->colors             = colors;
    gfx->allocategc         = allocategc;
    gfx->freegc             = freegc;
    gfx->setgc              = setgc;
    gfx->enableclipping     = enableclipping;
    gfx->disableclipping    = disableclipping;
    gfx->setclipping        = setclipping;
    gfx->getclipping        = getclipping;
    gfx->getbgcolor         = getbgcolor;
    gfx->setbgcolor         = NULL;
    gfx->getfgcolor         = getfgcolor;
    gfx->setfgcolor         = NULL;
    gfx->mapcolor           = mapcolor;
    gfx->unmappixel         = unmappixel;
    gfx->getpalette         = getpalette;
    gfx->setpalette         = setpalette;
    gfx->setcolorfulpalette = setcolorfulpalette;
    gfx->boxsize            = boxsize;
    gfx->fillbox            = fillbox;
    gfx->putbox             = putbox;
    gfx->getbox             = getbox;
    gfx->putboxmask         = putboxmask;
    gfx->scalebox           = scalebox;
    gfx->copybox            = copybox;
    gfx->crossblit          = crossblit;
    gfx->drawhline          = drawhline;
    gfx->drawvline          = drawvline;
    gfx->drawpixel          = drawpixel;
    gfx->getpixel           = getpixel;
    gfx->circle             = circle;
    gfx->line               = line;
    gfx->rectangle          = rectangle;
    gfx->panic              = panic;

    return TRUE;
}

void TermSVGALib (GFX* gfx)
{
    vga_unlockvc ();
    vga_setmode (TEXT);
}

