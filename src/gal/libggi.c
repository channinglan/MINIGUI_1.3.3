/*
** $Id: libggi.c,v 1.19 2003/09/04 02:47:51 weiym Exp $
**
** libggi.c: Low Level Graphics Engine based on LibGGI
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Create date: 2000/06/11 by Wei Yongming.
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
#include <ggi/ggi.h>

#include "common.h"
#include "gal.h"
#include "libggi.h"

// GC properties
static int bytes_per_pixel (GAL_GC gc)
{
    const ggi_pixelformat* pf;

    pf = ggiGetPixelFormat (gc.visual);

    return (pf->size + 7) >> 3;
}

static int bits_per_pixel (GAL_GC gc)
{
    const ggi_pixelformat* pf;

    pf = ggiGetPixelFormat (gc.visual);

    return pf->depth;
}

static int width (GAL_GC gc)
{
    ggi_mode tm;
    ggiGetMode(gc.visual, &tm);
    return tm.visible.x;
}

static int height (GAL_GC gc)
{
    ggi_mode tm;
    ggiGetMode(gc.visual, &tm);
    return tm.visible.y;
}

static int colors (GAL_GC gc)
{
    int depth = bits_per_pixel (gc);

    if (depth == 32)
        return 1 << 24;

    return 1 << depth;
}

// Allocation and release of graphics context
static int allocategc (GAL_GC gc, int width, int height, int depth, 
                GAL_GC* newgc)
{
    ggi_mode tm;
    newgc->visual = ggiOpen ("display-memory", NULL);
    if(newgc->visual == NULL) {
        return -1;
    }

    ggiGetMode (gc.visual, &tm);
    if (ggiSetSimpleMode (newgc->visual, width, height, 1, tm.graphtype)) {
        ggiClose (newgc->visual);
        return -1;
    }

    return 0;
}

// without any code
static void freegc (GAL_GC gc)
{
    ggiClose (gc.visual);
}

// Clipping of graphics context
// without any code
static void enableclipping (GAL_GC gc)
{
    ggi_mode tm;
    ggiGetMode (gc.visual, &tm);
    ggiSetGCClipping (gc.visual, 0, 0, tm.virt.x, tm.virt.y);
}

static void disableclipping (GAL_GC gc)
{
    ggi_mode tm;
    ggiGetMode(gc.visual, &tm);
    ggiSetGCClipping(gc.visual, 0, 0, tm.virt.x, tm.virt.y);
}

static int setclipping (GAL_GC gc, int x1, int y1, int x2, int y2)
{
    ggiSetGCClipping (gc.visual, x1, y1, x2 + 1, y2 + 1);
    return 0;
}

static int getclipping (GAL_GC gc, int* x1, int* y1, int* x2, int* y2)
{
    ggiGetGCClipping (gc.visual, x1, y1, x2, y2);
    *x2 -= 1;
    *y2 -= 1;
    return 0;
}


// Background and foreground colors
static int getbgcolor (GAL_GC gc, gal_pixel* color)
{
    ggiGetGCBackground(gc.visual, (ggi_pixel*)color);
    return 0;
}

static int setbgcolor (GAL_GC gc, gal_pixel color)
{
    ggiSetGCBackground(gc.visual, (ggi_pixel)color);
    return 0;
}

static int getfgcolor (GAL_GC gc, gal_pixel* color)
{
    ggiGetGCForeground(gc.visual, (ggi_pixel*)color);
    return 0;
}

static int setfgcolor (GAL_GC gc, gal_pixel color)
{
    ggiSetGCForeground(gc.visual, (ggi_pixel)color);
    return 0;
}

// Convertion between GAL_Color and gal_pixel
inline static gal_uint16 rgb8to16 (gal_uint8 rgb)
{
    return (gal_uint16)((gal_uint32) rgb * 0xFFFF / 0xFF);
}

inline static gal_uint8 rgb16to8 (gal_uint16 rgb)
{
    return (gal_uint8)((gal_uint32) rgb >> 8);
}

static gal_pixel mapcolor(GAL_GC gc, GAL_Color *color)
{
    ggi_color my_color;

    my_color.r = rgb8to16 (color->r);
    my_color.g = rgb8to16 (color->g);
    my_color.b = rgb8to16 (color->b);
    return (gal_pixel) ggiMapColor(gc.visual, &my_color);
}

static int unmappixel (GAL_GC gc, gal_pixel pixel, GAL_Color* color)
{
    ggi_color my_color;

    ggiUnmapPixel(gc.visual, (ggi_pixel)pixel, &my_color);

    color->r = rgb16to8 (my_color.r);
    color->g = rgb16to8 (my_color.g);
    color->b = rgb16to8 (my_color.b);
    return 0;
}

#if 0
static int packcolors (GAL_GC gc, void* buf, GAL_Color* colors, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        colors [i].r = rgb8to16 (colors [i].r);
        colors [i].g = rgb8to16 (colors [i].g);
        colors [i].b = rgb8to16 (colors [i].b);
    }
    ggiPackColors (gc.visual, buf, (ggi_color*)colors, len);
    return 0;
}

static int unpackpixels (GAL_GC gc, void* buf, GAL_Color* colors, int len)
{
    int i;

    ggiUnpackPixels (gc.visual, buf, (ggi_color*)colors, len);
    for (i = 0; i < len; i++) {
        colors [i].r = rgb16to8 (colors [i].r);
        colors [i].g = rgb16to8 (colors [i].g);
        colors [i].b = rgb16to8 (colors [i].b);
    }
    return 0;
}
#endif

// Palette operations
static int getpalette (GAL_GC gc, int s, int len, GAL_Color* cmap)
{
    int i;

    ggiGetPalette(gc.visual, s, len, (ggi_color*)cmap);
    for (i = 0; i < len; i++) {
        cmap [i].r = rgb16to8 (cmap [i].r);
        cmap [i].g = rgb16to8 (cmap [i].g);
        cmap [i].b = rgb16to8 (cmap [i].b);
    }
    return 0;
}

static int setpalette (GAL_GC gc, int s, int len, GAL_Color* cmap)
{
    int i;
    for (i = 0; i < len; i++) {
        cmap [i].r = rgb8to16 (cmap [i].r);
        cmap [i].g = rgb8to16 (cmap [i].g);
        cmap [i].b = rgb8to16 (cmap [i].b);
    }
    ggiSetPalette(gc.visual, s, len, (ggi_color *)cmap);
    return 0;
}

static int setcolorfulpalette (GAL_GC gc)
{
    ggiSetColorfulPalette (gc.visual);
    return 0;
}


// Box operations
static size_t boxsize (GAL_GC gc, int w, int h)
{
    return w * h * bytes_per_pixel (gc);
}

static int fillbox (GAL_GC gc, int x, int y, int w, int h,
                gal_pixel pixel)
{
    ggiDrawBox(gc.visual, x, y, w, h);
    return 0;
}

static int putbox (GAL_GC gc, int x, int y, int w, int h, void* buf)
{
    ggiPutBox(gc.visual, x, y, w, h, buf);
    return 0;
}

static int getbox (GAL_GC gc, int x, int y, int w, int h, void* buf)
{
    ggiGetBox(gc.visual, x, y, w, h, buf);
    return 0;
}

static int putboxmask (GAL_GC gc, int x, int y, int w, int h, void* buf, gal_pixel transparent)
{
    return 0;
}

/****************************************************************************/
/*      scalebox comes from SVGALib, Copyright 1993 Harm Hanemaayer         */
typedef unsigned char uchar;

/* We use the 32-bit to 64-bit multiply and 64-bit to 32-bit divide of the */
/* 386 (which gcc doesn't know well enough) to efficiently perform integer */
/* scaling without having to worry about overflows. */
#ifdef USE_ASM
static inline int muldiv64(int m1, int m2, int d)
{
/* int32 * int32 -> int64 / int32 -> int32 */
    int result;
    __asm__(
           "imull %%edx\n\t"
           "idivl %3\n\t"
  :           "=a"(result)    /* out */
  :           "a"(m1), "d"(m2), "g"(d)        /* in */
  :           "ax", "dx"    /* mod */
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

/* This is a DDA-based algorithm. */
/* Iteration over target bitmap. */
int scalebox (GAL_GC gc, int w1, int h1, void *_dp1, int w2, int h2, void *_dp2)
{
    uchar *dp1 = _dp1;
    uchar *dp2 = _dp2;
    int xfactor;
    int yfactor;

    if (w2 == 0 || h2 == 0)
        return 0;

    xfactor = muldiv64(w1, 65536, w2);        /* scaled by 65536 */
    yfactor = muldiv64(h1, 65536, h2);        /* scaled by 65536 */

    switch (bytes_per_pixel (gc)) {
    case 1:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < h2;) {
                int sx = 0;
                uchar *dp2old = dp2;
                int x;
                x = 0;
                while (x < w2 - 8) {
#ifdef USE_ASM

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
#else
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
#endif
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
        }
        break;
    case 2:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < h2;) {
                int sx = 0;
                uchar *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < w2) {
                    *(unsigned short *) (dp2 + x * 2) =
                        *(unsigned short *) (dp1 + (sx >> 16) * 2);
                    sx += xfactor;
                    x++;
                }
                dp2 += w2 * 2;
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
                dp1 = _dp1 + (sy >> 16) * w1 * 2;
            }
        }
        break;
    case 3:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < h2;) {
                int sx = 0;
                uchar *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < w2) {
                    *(unsigned short *) (dp2 + x * 3) =
                        *(unsigned short *) (dp1 + (sx >> 16) * 3);
                    *(unsigned char *) (dp2 + x * 3 + 2) =
                        *(unsigned char *) (dp1 + (sx >> 16) * 3 + 2);
                    sx += xfactor;
                    x++;
                }
                dp2 += w2 * 3;
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
                dp1 = _dp1 + (sy >> 16) * w1 * 3;
            }
        }
        break;
    case 4:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < h2;) {
                int sx = 0;
                uchar *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < w2) {
                    *(unsigned *) (dp2 + x * 4) =
                        *(unsigned *) (dp1 + (sx >> 16) * 4);
                    sx += xfactor;
                    x++;
                }
                dp2 += w2 * 4;
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
                dp1 = _dp1 + (sy >> 16) * w1 * 4;
            }
        }
        break;
    }

    return 0;
}
/****************************************************************************/

static int copybox (GAL_GC gc, int x, int y, int w, int h, int nx, int ny)
{
    ggiCopyBox(gc.visual, x, y, w, h, nx, ny);
    return 0;
}

static int crossblit (GAL_GC src, int sx, int sy, int sw, int wh,
                GAL_GC dst, int dx, int dy)
{
    ggiCrossBlit(src.visual, sx, sy, sw, wh, dst.visual, dx, dy);
    return 0;
}


// Horizontal line operaions
static int drawhline (GAL_GC gc, int x, int y, int w, gal_pixel pixel)
{
    ggiDrawHLine(gc.visual, x, y, w);
    return 0;
}

// Vertical line operations
static int drawvline (GAL_GC gc, int x, int y, int h, gal_pixel pixel)
{
    ggiDrawVLine(gc.visual, x, y, h);
    return 0;
}

// Pixel operations
static int drawpixel (GAL_GC gc, int x, int y, gal_pixel pixel)
{
    ggiDrawPixel(gc.visual, x, y);
    return 0;
}

static int getpixel (GAL_GC gc, int x, int y, gal_pixel* color)
{
    ggiGetPixel(gc.visual, x, y, (ggi_pixel *)color);
    return 0;
}


// Other drawing
// without any code
static int circle (GAL_GC gc, int x, int y, int r, gal_pixel pixel)
{
    return 0;
}

static int line (GAL_GC gc, int x1, int y1, int x2, int y2, 
                gal_pixel pixel)
{
    ggiDrawLine(gc.visual, x1, y1, x2, y2);
    return 0;
}

static int rectangle (GAL_GC gc, int l, int t, int r, int b, 
                gal_pixel pixel)
{
    ggiDrawHLine(gc.visual, l, t, (r - l) + 1);
    ggiDrawVLine(gc.visual, r, t, (b - t) + 1);
    ggiDrawHLine(gc.visual, l, b, (r - l) + 1);
    ggiDrawVLine(gc.visual, l, t, (b - t) + 1);
    return 0;
}

#if 0
// Simple Character output

static int myputchar (GAL_GC gc, int x, int y, char c)
{
    ggiPutc (gc.visual, x, y, c);
    return 0;
}

static int putstr (GAL_GC gc, int x, int y, const char* str)
{
    ggiPuts (gc.visual, x, y, str);
    return 0;
}

static int getcharsize (GAL_GC gc, int* width, int* height)
{
    *width = 8;
    *height = 8;
    return 0;
}

// without any code
static int setputcharmode (GAL_GC gc, int mode)
{
    return 0;
}

static int setfontcolors(GAL_GC gc, gal_pixel fg, gal_pixel bg)
{
    ggiSetGCForeground (gc.visual, (ggi_pixel)fg);
    ggiSetGCBackground (gc.visual, (ggi_pixel)bg);

    return 0;
}

static void flush (GAL_GC gc)
{
    ggiFlush (gc.visual);
}

static void flushregion (GAL_GC gc, int x, int y, int w, int h)
{
    ggiFlushRegion (gc.visual, x, y, w, h);
}
#endif

static void panic (int exitcode)
{
    ggiPanic ("MiniGUI Panic. Exit Code: %d\n", exitcode);
}

// Initialization and termination of  LibGGI
BOOL InitLibGGI (struct tagGFX* gfx)
{
    int i;
    ggi_visual_t visual;
    const ggi_pixelformat* pf;
    ggi_mode tm = {
                1,
                {GGI_AUTO, GGI_AUTO},
                {GGI_AUTO, GGI_AUTO},
                {0, 0},
                GT_AUTO,
                {GGI_AUTO, GGI_AUTO}
    };

    if (ggiInit() != 0) {
        fprintf(stderr, "unable to initialize LibGGI, exiting.\n");
        return FALSE;
    }

    visual = ggiOpen (NULL);

    if (visual == NULL) {
        fprintf(stderr,
                "unable to open default visual, exiting.\n");
        ggiExit();
        return FALSE;
    }

//    ggiSetFlags (visual, GGIFLAG_ASYNC);
    ggiCheckMode (visual, &tm);
    if (ggiSetMode (visual, &tm))    { 
        fprintf (stderr,"Can't set mode\n");
        ggiClose (visual);
        ggiExit ();
        return FALSE;
    }

    if (GT_SCHEME (tm.graphtype) == GT_PALETTE) {
        ggiSetColorfulPalette (visual);
    }

    for (i = 0; i < 17; i++) {
        ggi_color color;

        color.r = rgb8to16 (SysPixelColor[i].r);
        color.g = rgb8to16 (SysPixelColor[i].g);
        color.b = rgb8to16 (SysPixelColor[i].b);
        SysPixelIndex [i] = ggiMapColor (visual, &color);
    }
    
    pf = ggiGetPixelFormat (visual);

    gfx->phygc.visual       = visual;
    gfx->bytes_per_phypixel = (pf->size + 7) >> 3;
    gfx->bits_per_phypixel  = pf->depth;
    gfx->width_phygc        = tm.visible.x;
    gfx->height_phygc       = tm.visible.y;
    if (pf->depth == 32)
        gfx->colors_phygc   = 1 << 24;
    else
        gfx->colors_phygc   = 1 << pf->depth;
    gfx->grayscale_screen   = FALSE;

#ifdef _DEBUG
    fprintf (stderr, "GGI Mode: bpp %d, depth %d, width %d, height %d.\n",
            gfx->bytes_per_phypixel,
            gfx->bits_per_phypixel,
            gfx->width_phygc,
            gfx->height_phygc);
#endif

    gfx->bytesperpixel      = bytes_per_pixel;
    gfx->bitsperpixel       = bits_per_pixel;
    gfx->width              = width;
    gfx->height             = height;
    gfx->colors             = colors;
    gfx->allocategc         = allocategc;
    gfx->freegc             = freegc;
    gfx->setgc              = NULL;
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

void TermLibGGI (struct tagGFX* gfx)
{
    ggiClose (gfx->phygc.visual);
    ggiExit ();
}

