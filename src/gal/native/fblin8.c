/*
** $Id: fblin8.c,v 1.17 2003/11/22 11:49:29 weiym Exp $
**
** fblin8.c: 8bpp Linear Video Driver for MiniGUI
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 Song Lixin and Wei Yongming
** 
** 2000/10/20: Create by Song Lixin
**
** 2003/07/10: Cleanup by Wei Yongming.
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


#include <stdlib.h>
#include <string.h>

#include "native.h"
#include "fb.h"

#ifdef _FBLIN8_SUPPORT

static int linear8_init (PSD psd)
{
    if (!psd->size) {
        psd->size = psd->yres * psd->linelen;
        /* convert linelen from byte to pixel len for bpp 16, 24, 32*/
    }
    return 1;
}

/* Set pixel at x, y, to gal_pixel c*/
static void linear8_drawpixel (PSD psd, int x, int y, gal_pixel c)
{
    gal_uint8  * addr = (gal_uint8 *) psd->addr;

    if (psd->gr_mode == MODE_XOR)
        addr[x + y * psd->linelen] ^= (gal_uint8) c; 
    else
        addr[x + y * psd->linelen] = (gal_uint8) c;
}

/* Read pixel at x, y*/
static gal_pixel linear8_readpixel (PSD psd, int x, int y)
{
    gal_uint8    *addr = (gal_uint8 *) psd->addr;

    return (gal_pixel) addr[x + y * psd->linelen];
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void linear8_drawhline (PSD psd, int x, int y, int w, gal_pixel c)
{
    gal_uint8    *dst;

    dst = (gal_uint8 *)(psd->addr) + y * psd->linelen + x;

    if (psd->gr_mode == MODE_XOR) {
        while (w --)
            *dst++ ^= (gal_uint8)c;
    }
    else {
        while (w --)
            *dst++ = (gal_uint8)c;
    }
}

#if 0
/*do clip*/
static void linear8_puthline (PSD psd,int x, int y, int w, void *buf)
{
    gal_uint8    *mem;
    gal_uint8    *src = (gal_uint8 *) buf;
    
    if (psd->doclip) {
        if (x < psd->clipminx) {
            src += psd->clipminx - x;
            w   -= psd->clipminx - x; 
            x    = psd->clipminx;
        }
        if (x + w - 1 >= psd->clipmaxx) {
            w    = psd->clipmaxx - x ;
        }        

    } else {
        if (x < 0) {
            src -= x;
            w   += x;
            x    = 0;
        }
        if (x + w -1 >= psd->xres) {
            w    = psd->xres - x;
        }
    }

    mem = (gal_uint8 *)(psd->addr) + y * psd->linelen  + x;
    memcpy(mem,src,w);    
}

/*do clip*/
static void linear8_gethline (PSD psd, int x, int y, int w, void *buf)
{
    gal_uint8    *mem;
    gal_uint8    *dst = (gal_uint8 *) buf;

    if (x < 0) {
        dst -= x;
        w   += x;
        x    = 0;
    }
    if (x + w -1 >= psd->xres) {
        w    = psd->xres - x;
    }

    mem = (gal_uint8 *)(psd->addr) + (y * psd->linelen + x);
    memcpy(dst,mem,w);    
}
#endif

static void linear8_drawvline(PSD psd, int x, int y, int h, gal_pixel c)
{
    gal_uint8    *dst;
    int    linelen = psd->linelen;

    dst = (gal_uint8 *)(psd->addr) + y * psd->linelen + x;

    if (psd->gr_mode == MODE_XOR) {
        while (h --) {
            *dst^= (gal_uint8)c;
            dst+= linelen;
        }
    }
    else {
        while (h --) {
            *dst = (gal_uint8)c;
            dst+= linelen;
        }
    }
}

#if 0
/*do clip*/
static void linear8_putvline (PSD psd,int x, int y, int h, void *buf)
{
    gal_uint8     *mem;
    gal_uint8    *src= (gal_uint8 *) buf;

    if (psd->doclip) {
        if (y < psd->clipminy) {
            src += psd->clipminy - y;
            h   -= psd->clipminy - y; 
            y    = psd->clipminy;
        }
        if (y + h - 1 >= psd->clipmaxy) {
            h    = psd->clipmaxy - y ;
        }        

    } else {
        if (y < 0) {
            src -= y;
            h   += y;
            y    = 0;
        }
        if (y + h -1 >= psd->yres) {
            h    = psd->yres - y;
        }
    }

    mem = (gal_uint8 *)(psd->addr) + y * psd->linelen + x;

    while (h --) {
        *mem = *src++;
        mem += psd->linelen;
    }
}

/*do clip*/
static void linear8_getvline (PSD psd, int x, int y, int h, void *buf)
{
    gal_uint8    *mem;
    gal_uint8    *dst= (gal_uint8 *) buf;

    if (y < 0) {
        dst -= y;
        h   += y;
        y    = 0;
    }
    if (y + h -1 >= psd->yres) {
        h    = psd->yres - y;
    }

    mem = (gal_uint8 *)(psd->addr) + y * psd->linelen + x;

    while (h --) {
        *dst++  = *mem;
        mem+= psd->linelen;
    }
    return;
}
#endif

/* srccopy bitblt,not do clip*/
static void linear8_blit (PSD dstpsd, int dstx, int dsty, int w, int h,
    PSD srcpsd, int srcx, int srcy)
{
    gal_uint8    *dst = (gal_uint8 *) dstpsd->addr;
    gal_uint8    *src = (gal_uint8 *) srcpsd->addr;
    int    dlinelen = dstpsd->linelen;
    int    slinelen = srcpsd->linelen;

    dst += dstx + dsty * dlinelen;
    src += srcx + srcy * slinelen;

    while(--h >= 0) {
        /* a _fast_ memcpy is a _must_ in this routine*/
        memcpy(dst, src, w);
        dst += dlinelen;
        src += slinelen;
    }
}

/*do clip*/
static void linear8_putbox ( PSD psd, int x, int y, int w, int h, void *buf)
{
    gal_uint8    *src = (gal_uint8*) buf;
    int srcwidth = w;
    gal_uint8 *dst;
    int dst_pitch = psd->linelen;
    int oldx = x;

    if (psd->doclip) {    
        if (y < psd->clipminy) {
            h -= psd->clipminy - y;
            src += (psd->clipminy - y) * srcwidth;
            y = psd->clipminy;
        }
        if (x < psd->clipminx) {
            w -= psd->clipminx - x;
            src += psd->clipminx - x;
            x = psd->clipminx;
        }        
        if (y + h - 1 >= psd->clipmaxy) 
            h =  psd->clipmaxy- y;
        if (x + w - 1 >= psd->clipmaxx) 
            w =  psd->clipmaxx- x;
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
        if ( y + h  -1 >= psd->yres) 
            h = psd->yres - y ;
        if ( x + w  -1 >= psd->xres) 
            w = psd->xres - x ;
    }

    dst = (gal_uint8 *)(psd->addr) + y*dst_pitch + x;

    if (w <= 0 || h<=0) return;

    if (w == dst_pitch && psd->linelen == srcwidth && oldx == 0 && psd->clipminx == 0) {
        memcpy (dst, src, w*h);
        return;
    }

    while (h > 0) {
        memcpy(dst, src, w);
        dst += dst_pitch;
        src += srcwidth;
        h--;
    }
}
/*clip to screen*/
static void linear8_getbox ( PSD psd, int x, int y, int w, int h, void* buf )
{
    gal_uint8 *dst = (gal_uint8*) buf;
    int dst_pitch = w;
    gal_uint8 *src;
    int srcwidth = psd->linelen;
    int oldx = x;

    if ( y < 0 ) {
        h += y;
        dst -= y * dst_pitch;
        y = 0;
    }
    if ( x < 0 ) {
        w += x;
        dst -= x;
        x = 0;
    }        
    if ( y + h  -1 >= psd->yres) 
        h = psd->yres - y ;
    if ( x + w  -1 >= psd->xres) 
        w = psd->xres - x ;

    if (w <= 0 || h <= 0) return;

    src = (gal_uint8 *)(psd->addr) + y*srcwidth + x;

    if (w == dst_pitch && psd->linelen == srcwidth && oldx == 0 && psd->clipminx == 0) {
        memcpy (dst, src, w*h);
        return;
    }

    while (h-- > 0) {
        memcpy(dst, src, w);
        dst += dst_pitch;
        src += srcwidth;
    }
}

static    void linear8_putboxmask (PSD psd, int x, int y, int w, int h, void *buf, gal_pixel cxx)
{
    gal_uint8 *src= (gal_uint8*) buf;
    gal_uint8 *src1;
    gal_uint8 *dst;
    gal_uint8 *endoflinesrc;
    int i;
    int srcwidth =  w ;
    int dst_pitch = psd->linelen;

    if (psd->doclip) {    
        if (y < psd->clipminy) {
            h -= psd->clipminy - y;
            src += (psd->clipminy - y) * srcwidth;
            y = psd->clipminy;
        }
        if (x < psd->clipminx) {
            w -= psd->clipminx - x;
            src += psd->clipminx - x;
            x = psd->clipminx;
        }        
        if (y + h - 1 >= psd->clipmaxy) 
            h =  psd->clipmaxy- y;
        if (x + w - 1 >= psd->clipmaxx) 
            w =  psd->clipmaxx- x;
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
        if ( y + h  -1 >= psd->yres) 
            h = psd->yres - y ;
        if ( x + w  -1 >= psd->xres) 
            w = psd->xres - x ;
    }

    if (w <= 0 || h<=0) return;

    dst= (gal_uint8 *)(psd->addr) + y*dst_pitch + x;

    for (i = 0; i < h; i++) {

        src1 = src ; 
        endoflinesrc = src +  w;

        while (src1 < endoflinesrc - 7) {
            gal_uint32 c2 = *(gal_uint32 *) src1;
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) dst = (gal_uint8) c2;
            c2 >>= 8;
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) (dst + 1) = (gal_uint8) c2;
            c2 >>= 8;
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) (dst + 2) = (gal_uint8) c2;
            c2 >>= 8;
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) (dst + 3)= (gal_uint8) c2;

            c2 = *(gal_uint32 *) (src1+4);
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) (dst + 4) = (gal_uint8) c2;
            c2 >>= 8;
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) (dst + 5) = (gal_uint8) c2;
            c2 >>= 8;
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) (dst + 6) = (gal_uint8) c2;
            c2 >>= 8;
            if ((c2 & 0xff) != cxx)
            *(gal_uint8 *) (dst + 7)= (gal_uint8) c2;
            src1 += 8;
            dst += 8;
        }
        while (src1 < endoflinesrc) {
            gal_uint8 c = *(gal_uint8 *) src1;
            if (c != cxx)
                *(gal_uint8 *) dst = c;
            src1 ++;
            dst ++;
        }
        dst += psd->linelen  - w ;

        src += srcwidth ;
    }
}

static    void linear8_copybox(PSD psd,int x1, int y1, int w, int h, int x2, int y2)
{
    gal_uint8 *svp, *dvp;

    if (y1 >= y2) {
        if (y1 == y2 && x2 >= x1) {    /* tricky */
            int i;
            if (x1 == x2)
                return;
            /* use a temporary buffer to store a line */
            /* using reversed movs would be much faster */
            svp = (gal_uint8 *) psd->addr + y1 * psd->linelen + x1;
            dvp = (gal_uint8 *) psd->addr + y2 * psd->linelen + x2;
            for (i = 0; i < h; i++) {
                gal_uint8 linebuf[2048];
                memcpy (linebuf, svp, w);
                memcpy (dvp, linebuf, w);
                svp += psd->linelen;
                dvp += psd->linelen;
            }
        } else {        /* copy from top to bottom */
            int i;
            svp = (gal_uint8 *) psd->addr + y1 * psd->linelen + x1;
            dvp = (gal_uint8 *) psd->addr + y2 * psd->linelen + x2;
            for (i = 0; i < h; i++) {
                memcpy(dvp, svp, w);
                svp += psd->linelen;
                dvp += psd->linelen;
            }
        }
    } else {            /* copy from bottom to top */
        int i;

        svp = (gal_uint8 *) psd->addr + (y1 + h) * psd->linelen + x1;
        dvp = (gal_uint8 *) psd->addr + (y2 + h) * psd->linelen + x2;
        for (i = 0; i < h; i++) {
            svp -= psd->linelen;
            dvp -= psd->linelen;
            memcpy (dvp, svp, w);
        }
    }
}

SUBDRIVER fblinear8 = {
    linear8_init,
    linear8_drawpixel,
    linear8_readpixel,
    linear8_drawhline,
    linear8_drawvline,
    linear8_blit,
    linear8_putbox,
    linear8_getbox,
    linear8_putboxmask,
    linear8_copybox
};

#endif /* _FBLIN8_SUPPORT */

