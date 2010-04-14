/*
** $Id: fbvga16.c,v 1.12 2003/11/22 11:49:29 weiym Exp $
**
** fbvga16.c: Frame buffer VGA16 video driver for MiniGUI
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 Song Lixin and Wei Yongming
**
** 2000/11/11: Created by Song Lixin.
**
** 2000/11/15: Song Lixin: 
**      I have used packed memory mode. That means when use 4 bit color, 
**      two pixels use one byte, but now, we alien to byte. 
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
#include <string.h>

#include "common.h"

#ifdef _FBVGA16_SUPPORT

#include "native.h"
#include "fb.h"
#include "fbvga16.h"

#define SCREENBASE(psd)         ((char *)psd->addr)
#define BYTESPERLINE(psd)        (psd->linelen)

/* Values for the data rotate register to implement drawing modes. */
static unsigned char mode_table[MODE_MAX + 1] = {
    0x00, 0x18, 0x10, 0x08
};

/* precalculated mask bits*/
static unsigned char mask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static unsigned char notmask[2] = {0x0f, 0xf0};

static void _mem16_putbox (PSD psd, int x, int y, int w, int h, gal_uint8 * src, int srcwidth)
{
    gal_uint8* dst= (gal_uint8 *)psd->addr + y * psd->linelen + x;
    while (h--) {
        memcpy (dst,src,w);
        dst += psd->linelen;
        src += srcwidth;
    }
}

static void _mem16_getbox (PSD psd, int x, int y, int w, int h, gal_uint8 * dst, int dstwidth)
{
    gal_uint8* src= (gal_uint8 *)psd->addr + y * psd->linelen + x;
    while (h--) {
        memcpy (dst,src,w);
        src += psd->linelen;
        dst += dstwidth;
    }
}

static void _vga16_putbox (PSD psd, int x, int y, int w, int h, gal_uint8    *src, int srcwidth)
{
    gal_uint8* dst;
    gal_uint8* dst1;
    gal_uint8 *src1;
    int c, lastc;
    int  h1;

    set_op(0);        /* mode_table[MODE_SET]*/
    dst = (gal_uint8*)SCREENBASE(psd) + (x >> 3) + y * BYTESPERLINE(psd);
    lastc = -1;

    while (w--) {
        src1 = src;
        h1 = h;
        dst1 = dst;
        select_and_set_mask (mask[x & 7]);
        while (h1--) {
            c = *src1;
            if (c != lastc)
                set_color (lastc = c);
            RMW (dst1);
            src1 += srcwidth;
            dst1 += BYTESPERLINE(psd);
        }
        if ((++x & 7) == 0) dst++;
        src++;
    }
}

static void _vga16_putboxmask (PSD psd, int x, int y, int w, int h, gal_uint8 * src, int srcwidth, gal_pixel cxx)
{
    gal_uint8* dst;
    gal_uint8* dst1;
    gal_uint8 *src1;
    int c,w1,x1,lastc;
    lastc = -1;

    set_op(0);        /* mode_table[MODE_SET]*/
    dst = (gal_uint8*)SCREENBASE(psd) + (x >> 3) + y * BYTESPERLINE(psd);

    while (h--) {
        src1 = src;
        w1 = w;
        x1 = x;
        dst1 = dst;
        
        while (w1--) {
            c = *src1++;
            if (c != cxx) {
                if (c != lastc)
                    set_color (lastc = c);
                select_and_set_mask (mask[x1 & 7]);
                RMW (dst1);
            }
            if ((++x1 & 7) == 0) dst1++;
        }

        dst += BYTESPERLINE(psd);
        src += srcwidth;
    }
}

static void _vga16_getbox (PSD psd, int x, int y, int w, int h, gal_uint8 *dst, int dstwidth)
{
    gal_uint8*        src;
    gal_uint8*        src1;
    gal_uint8 *dst1;
    int        plane, c, w1, x1;
    src = SCREENBASE(psd) + (x>>3) + y * BYTESPERLINE(psd);
    while ( h--) {
        dst1 = dst;
        w1 = w;
        x1 = x;
        src1 = src;

        while (w1--) {
            c = 0;
            for(plane=0; plane<4; ++plane) {
                set_read_plane(plane);
                if(GETBYTE(src1) & mask[x1&7])
                    c |= 1 << plane;
            }
            if ((++x1 & 7) == 0) src1++;
            *dst1++ = c;    
        }
        src += BYTESPERLINE(psd);
        dst += dstwidth;
    }
}

/* copy from vga memory to vga memory, psd's ignored for speed*/
static void _vga16_to_vga16_blit(PSD dstpsd, int dstx, int dsty, int w, int h, PSD srcpsd, int srcx, int srcy)
{
    gal_uint8*    dst;
    gal_uint8*    src;
    int    i, plane;
    int    x1, x2;

    set_enable_sr(0);
    dst = SCREENBASE(dstpsd) + (dstx>>3) + dsty * BYTESPERLINE(dstpsd);
    src = SCREENBASE(srcpsd) + (srcx>>3) + srcy * BYTESPERLINE(srcpsd);
    x1 = dstx>>3;
    x2 = (dstx + w - 1) >> 3;
    while(--h >= 0) {
        for(plane=0; plane<4; ++plane) {
            gal_uint8* d = dst;
            gal_uint8* s = src;

            set_read_plane(plane);
            set_write_planes(1 << plane);

            /* FIXME: only works if srcx and dstx are same modulo*/
            if(x1 == x2) {
                  select_and_set_mask((0xff >> (x1 & 7)) & (0xff << (7 - (x2 & 7))));
                PUTBYTE(d, GETBYTE(s));
            } else {
                select_and_set_mask(0xff >> (x1 & 7));
                PUTBYTE(d++, GETBYTE(s++));

                set_mask(0xff);
                  for(i=x1+1; i<x2; ++i)
                    PUTBYTE(d++, GETBYTE(s++));

                  set_mask(0xff << (7 - (x2 & 7)));
                PUTBYTE(d, GETBYTE(s));
            }
        }
        dst += BYTESPERLINE(dstpsd);
        src += BYTESPERLINE(srcpsd);
    }
    set_write_planes(0x0f);
    set_enable_sr(0x0f);
}

/* copy from linear 4 bit mem to vga memory*/
static void _mem16_to_vga16_blit(PSD dstpsd, int dstx, int dsty, int w, int h, PSD srcpsd, int srcx, int srcy)
{
    gal_uint8*    dst;
    gal_uint8 *    src;
    int    slinelen = srcpsd->linelen;
    int    c, lastc = -1;

    set_op(0);        /* mode_table[MODE_SET]*/
    dst = SCREENBASE(dstpsd) + (dstx>>3) + dsty * BYTESPERLINE(dstpsd);
    src = (gal_uint8 *)srcpsd->addr + srcx + srcy * slinelen;

    while (w--) {
        gal_uint8  *src1 = src;
        gal_uint8* dst1 = dst;
        int h1 = h;

        select_and_set_mask (mask[dstx & 7]);
        while (h1--) {
            c = *src1;
            if (c != lastc)
                set_color (lastc = c);
            RMW (dst1);
            src1 += slinelen;
            dst1 += BYTESPERLINE(dstpsd);
        }
        if ((++dstx & 7) == 0) dst++;
        src++;
    }
}

/* srccopy bitblt, opcode is currently ignored */
/* copy from vga memory to memdc */
static void _vga16_to_mem16_blit(PSD dstpsd, int dstx, int dsty, int w, int h, PSD srcpsd, int srcx, int srcy)
{
    gal_uint8*        src, src1;
    gal_uint8    *dst, *dst1;
    int        plane, c, w1, x1;

    src = SCREENBASE(srcpsd) + (srcx>>3) + srcy * BYTESPERLINE(srcpsd);
    dst= (gal_uint8 *)dstpsd->addr + dsty * dstpsd->linelen + dstx;
    while (h--) {
        w1 = w;
        x1 = srcx;
        src1 = src;
        dst1 = dst;

        while (w1--) {
            c = 0;
            for(plane=0; plane<4; ++plane) {
                set_read_plane(plane);
                if(GETBYTE(src1) & mask[x1&7])
                    c |= 1 << plane;
            }
            if ((++x1 & 7) == 0) src1++;
            *dst1++ = c;    
        }
        src += BYTESPERLINE(srcpsd);
        dst += BYTESPERLINE(dstpsd);
    }
}

static int fbvga16_init (PSD psd)
{
    if (!psd->size) {
        if (psd->flags & PSF_MEMORY) { 
            psd->size = psd->yres * psd->linelen;
            return 1;
        }

        /* allow direct access to vga controller space*/
        if(ioperm(0x3c0, 0x20, 1) == -1) {
            fprintf(stderr,"GAL ENGINE: Can't set i/o permissions: %m\n");
            return 0;
        }

        /* framebuffer mmap size*/
        psd->size = 0x10000;
        /* Set up some default values for the VGA Graphics Registers. */
        set_enable_sr (0x0f);
        set_op (0);
        set_mode (0);
    }

    return 1;
}

/* Set pixel at x, y, to gal_pixel c*/
static void fbvga16_drawpixel (PSD psd, int x, int y, gal_pixel c)
{
    gal_uint8  * addr = (gal_uint8 *) psd->addr;

    if (psd->flags & PSF_MEMORY) {
        addr += (x >> 1) + y * psd->linelen;
        if(psd->gr_mode == MODE_XOR)
            *addr ^= c << ((1 - (x & 1)) << 2);
        else
            *addr = (*addr & notmask[x & 1]) | (c << ((1-(x & 1)) << 2));
        return;
    } 
    set_op(mode_table[psd->gr_mode]);
    set_color (c);
    select_and_set_mask (mask[x & 7]);
    RMW ((gal_uint8*)SCREENBASE(psd) + (x >> 3) + y * BYTESPERLINE(psd));
}

/* Read pixel at x, y*/
static gal_pixel fbvga16_readpixel (PSD psd, int x, int y)
{
    gal_uint8  * addr = (gal_uint8 *) psd->addr;
    gal_uint8*        src;
    int        plane;
    gal_pixel c = 0;

    if (psd->flags & PSF_MEMORY) 
        return (addr[(x>>1) + y * psd->linelen] >> ((1-(x&1))<<2) ) & 0x0f;

    src = SCREENBASE(psd) + (x>>3) + y * BYTESPERLINE(psd);
    for(plane = 0; plane < 4; ++plane) {
        set_read_plane(plane);
        if(GETBYTE(src) & mask[x&7])
            c |= 1 << plane;
    }
    return c;
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void fbvga16_drawhline (PSD psd, int x, int y, int w, gal_pixel c)
{
    gal_uint8    *addr= (gal_uint8 *)psd->addr;
    gal_uint8* dst, last;
    int cc;

    if (psd->flags & PSF_MEMORY) {
        addr += (x>>1) + y * psd->linelen;
        cc = c | (c << 4);
        if(psd->gr_mode == MODE_XOR) {
            if (x & 1) {
                *addr = (*addr & notmask[1]) | (c ^ (*addr & notmask[0]));
                addr++;
                w --;
            }
            while (w > 1) {
                *addr++ ^= cc ;
                w -= 2;
            }
            if (w == 1) 
                *addr = (*addr & notmask[0]) | ((c << 4)^(*addr & notmask[1]));
        } else {
            if (x & 1) {
                *addr = (*addr & notmask[1]) | c;
                addr++;
                w --;
            }
            while (w > 1) {
                *addr++ = cc ;
                w -= 2;
            }
            if (w == 1) 
                *addr = (*addr & notmask[0]) | (c << 4);
        }
        return;
    }


    set_color (c);
    set_op(mode_table[psd->gr_mode]);
    /*
    * The following fast drawhline code is buggy for XOR drawing,
    * for some reason.  So, we use the equivalent slower drawpixel
    * method when not drawing MODE_SET.
    */
    if(psd->gr_mode == MODE_SET) {
        dst = SCREENBASE(psd) + (x>>3) + y * BYTESPERLINE(psd);
        if ((x>>3) == ( (x + w - 1)>>3) ) {
            select_and_set_mask ((0xff >> (x & 7)) & (0xff << (7 - ((x + w - 1) & 7))));
            RMW (dst);
        } else {
            select_and_set_mask (0xff >> (x & 7));
            RMW (dst++);

            set_mask (0xff);
            last = SCREENBASE(psd)  + ( (x + w - 1) >>3) + y * BYTESPERLINE(psd);
            while (dst < last)
                PUTBYTE(dst++, 1);

            set_mask (0xff << (7 - ((x + w - 1) & 7)));
            RMW (dst);
        }
    } else {
        /* slower method, draw pixel by pixel*/
        select_mask ();
        while(w--) {
            set_mask (mask[x&7]);
            RMW ((gal_uint8*)SCREENBASE(psd)  + (x++>>3) + y * BYTESPERLINE(psd));
        }
    }
}

static void fbvga16_drawvline(PSD psd, int x, int y, int h, gal_pixel c)
{
    gal_uint8* addr= (gal_uint8 *)psd->addr;
    gal_uint8* dst;
    int linelen = psd->linelen;

    if (psd->flags & PSF_MEMORY) {
        addr += (x>>1) + y * linelen;
        if(psd->gr_mode == MODE_XOR)
            while(h--) {
                *addr ^= (*addr & notmask[x&1]) | (c << ((1-(x&1))<<2) ^ (*addr&notmask[1 - (x&1)]));
                addr += linelen;
            }
        else
            while(h--) {
                *addr = (*addr & notmask[x&1]) | (c << ((1-(x&1))<<2));
                addr += linelen;
            }
        return;
    }

    set_op(mode_table[psd->gr_mode]);
    set_color (c);
    select_and_set_mask (mask[x&7]);
    dst = SCREENBASE(psd)  + (x>>3) + y * BYTESPERLINE(psd);

    while (h--) {
        RMW (dst);
        dst += BYTESPERLINE(psd);
    }
}

/*clip to screen*/
static void fbvga16_getbox ( PSD psd, int x, int y, int w, int h, void* buf )
{
    gal_uint8 *dst = (gal_uint8*) buf;
    int dstwidth = w;

    if ( y < 0 ) {
        h += y;
        dst += -y * dstwidth;
        y = 0;
    }
    if ( x < 0 ) {
        w += x;
        dst += -x;
        x = 0;
    }        
    if ( y + h  - 1 >= psd->yres) 
        h = psd->yres - y ;
    if ( x + w  - 1 >= psd->xres) 
        w = psd->xres - x ;

    if (psd->flags & PSF_MEMORY) {
        gal_uint8    *src= (gal_uint8 *)psd->addr + y * psd->linelen + x;
        while ( h--) {
            memcpy(dst,src,w);
            src += psd->linelen;
            dst += dstwidth;
        }
    }
    else
            _vga16_getbox (psd, x, y, w, h, dst, dstwidth);
}

/*do clip*/
static void fbvga16_putbox ( PSD psd, int x, int y, int w, int h, void *buf)
{
    gal_uint8    *src = (gal_uint8*) buf;
    int srcwidth = w;

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
            src += -y * srcwidth;
            y = 0;
        }
        if ( x < 0 ) {
            w += x;
            src += -x;
            x = 0;
        }        
        if ( y + h  -1 >= psd->yres) 
            h = psd->yres - y ;
        if ( x + w  -1 >= psd->xres) 
            w = psd->xres - x ;
    }

    if (psd->flags & PSF_MEMORY) {
        gal_uint8    *dst= (gal_uint8 *)psd->addr + y * psd->linelen + x;
        while ( h--) {
            memcpy(dst,src,w);
            dst += psd->linelen;
            src += srcwidth;
        }
    }
    else
        _vga16_putbox (psd, x, y, w, h, src, srcwidth);
}

static void fbvga16_putboxmask (PSD psd, int x, int y, int w, int h, void *buf, gal_pixel cxx)
{
    gal_uint8 *src= (gal_uint8*) buf;
    int srcwidth =  w ;

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
            src += -y * srcwidth;
            y = 0;
        }
        if ( x < 0 ) {
            w += x;
            src += -x;
            x = 0;
        }        
        if ( y + h  -1 >= psd->yres) 
            h = psd->yres - y ;
        if ( x + w  -1 >= psd->xres) 
            w = psd->xres - x ;
    }

    if (psd->flags & PSF_MEMORY) {
        gal_uint8    *dst= (gal_uint8 *)psd->addr + y * psd->linelen + x;
        gal_uint8 c;

        while ( h--) {
            int w1 = w;
            gal_uint8 *src1 = src;
            gal_uint8 *dst1 = dst;
            while(w1--) {
                c = *src1;
                if (c != cxx) 
                    *dst1 = c;
                    src1++;
                    dst1++;
            }
            dst += psd->linelen;
            src += srcwidth;
        }
    }
    else
        _vga16_putboxmask (psd, x, y, w, h, src, srcwidth, cxx);

}

static void fbvga16_copybox
(PSD psd,int x1, int y1, int w, int h, int x2, int y2)
{
    gal_uint8 *buf;

    buf = (gal_uint8 *) malloc (w * h);
    if (psd->flags & PSF_MEMORY) {
        _mem16_getbox (psd, x1, y1, w, h, buf, w);
        _mem16_putbox (psd, x1, y1, w, h, buf, w);
    } else {
        _vga16_getbox (psd, x2, y2, w, h, buf, w);
        _vga16_putbox (psd, x2, y2, w, h, buf, w);
    }
    free (buf);
}

/* 
 *  Bitblt,not do clip
 *  opcode is currently ignored
 *  WARNING: src & dst can not be same psd!
 */
static void fbvga16_blit (PSD dstpsd, int dstx, int dsty, int w, int h,
    PSD srcpsd, int srcx, int srcy)
{
    int srcvga, dstvga;

    srcvga = srcpsd->flags & PSF_SCREEN;
    dstvga = dstpsd->flags & PSF_SCREEN;

    if(srcvga) {
        if(dstvga)
            _vga16_to_vga16_blit(dstpsd, dstx, dsty, w, h,
                srcpsd, srcx, srcy);
        else
            _vga16_to_mem16_blit(dstpsd, dstx, dsty, w, h,
                srcpsd, srcx, srcy);
    } else {
        if(dstvga)
            _mem16_to_vga16_blit(dstpsd, dstx, dsty, w, h,
                srcpsd, srcx, srcy);
        else {
            gal_uint8 *    dst;
            gal_uint8 *    src;
            dst = (char *)dstpsd->addr + dstx + dsty * dstpsd->linelen;
            src = (char *)srcpsd->addr + srcx + srcy * srcpsd->linelen;
            memcpy(dst, src, w*h);
        }
    }
}

SUBDRIVER fbvga16 = {
    fbvga16_init,
    fbvga16_drawpixel,
    fbvga16_readpixel,
    fbvga16_drawhline,
    fbvga16_drawvline,
    fbvga16_blit,
    fbvga16_putbox,
    fbvga16_getbox,
    fbvga16_putboxmask,
    fbvga16_copybox
};

#endif /* _FBVGA16_SUPPORT */

