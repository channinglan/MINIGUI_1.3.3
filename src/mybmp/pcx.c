/*
** $Id: pcx.c,v 1.6 2003/12/01 09:26:36 weiym Exp $
** 
** pcx.c: Low-level PCX bitmap file read/save routines.
** 
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Some code comes from pcx.c of Allegro by Shawn Hargreaves.
** 
** Current maintainer:  Wei Yongming.
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

/*         ______   ___    ___
**        /\  _  \ /\_ \  /\_ \
**        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
**         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
**          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
**           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
**            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
**                                           /\____/
**                                           \_/__/
*/

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "gdi.h"
#include "readbmp.h"

#ifdef _PCX_FILE_SUPPORT

/* load_pcx:
 *  Loads a 256 color PCX file, returning in a bitmap structure and storing
 *  the palette data in the specified palette (this should be an array of
 *  at least 256 RGB structures).
 */
int load_pcx (MG_RWops* f, MYBITMAP* bmp, RGB *pal)
{
    int c;
    int width, height;
    int bpp, bytes_per_line;
    unsigned long size;
    unsigned char* bits;
    int xx, po;
    int x, y;
    int pitch;
    char ch;

    fp_getc(f);                    /* skip manufacturer ID */
    fp_getc(f);                    /* skip version flag */
    fp_getc(f);                    /* skip encoding flag */

    if (fp_getc(f) != 8) {         /* we like 8 bit color planes */
        return ERR_BMP_NOT_SUPPORTED;
    }

    width = -(fp_igetw(f));        /* xmin */
    height = -(fp_igetw(f));       /* ymin */
    width += fp_igetw(f) + 1;      /* xmax */
    height += fp_igetw(f) + 1;     /* ymax */

    fp_igetl(f);                   /* skip DPI values */

    for (c=0; c<16; c++) {           /* read the 16 color palette */
        pal[c].r = fp_getc(f);
        pal[c].g = fp_getc(f);
        pal[c].b = fp_getc(f);
    }

    fp_getc(f);

    bpp = fp_getc(f) * 8;          /* how many color planes? */
    if ((bpp != 8) && (bpp != 24)) {
        return ERR_BMP_NOT_SUPPORTED;
    }

    bytes_per_line = fp_igetw(f);

    for (c=0; c<60; c++)             /* skip some more junk */
        fp_getc(f);

    pitch = bytes_per_line * bpp / 8;
    size = pitch * height;
    if( !(bits = malloc (size)) ) {
        return ERR_BMP_MEM;
    } 
    bmp->bits = bits;

    for (y=0; y<height; y++) {       /* read RLE encoded PCX data */
        x = xx = 0;
        po = 0;

        while (x < pitch) {
            ch = fp_getc(f);
            if ((ch & 0xC0) == 0xC0) {
                c = (ch & 0x3F);
                ch = fp_getc(f);
            }
            else
                c = 1;

            if (bpp == 8) {
                while (c--) {
                    if (x < width)
                        bits [x] = ch;
                    x++;
                }
            }
            else {
                while (c--) {
                    if (xx < width)
                        bits [xx*3+po] = ch;
                    x++;
                    if (x == bytes_per_line) {
                        xx = 0;
                        po = 1;
                    }
                    else if (x == bytes_per_line*2) {
                        xx = 0;
                        po = 2;
                    }
                    else
                        xx++;
                }
            }
        }
        bits += pitch;
    }

    if (bpp == 8) {                  /* look for a 256 color palette */
        while (!MGUI_RWeof(f)) { 
            if (fp_getc(f)==12) {
                for (c=0; c<256; c++) {
                    pal[c].r = fp_getc(f);
                    pal[c].g = fp_getc(f);
                    pal[c].b = fp_getc(f);
                }
                break;
            }
        }
    }

    bmp->depth = bpp;
    bmp->w     = width;
    bmp->h     = height;
    bmp->flags = MYBMP_FLOW_DOWN;
    bmp->pitch = pitch;
    bmp->frames = 1;
    bmp->size  = size;

    return ERR_BMP_OK;
}

#if 0
/* save_pcx:
 *  Writes a bitmap into a PCX file, using the specified palette (this
 *  should be an array of at least 256 RGB structures).
 */
int save_pcx (FILE* f, MYBITMAP *bmp, RGB *pal)
{
   FILE *f;
   PALETTE tmppal;
   int c;
   int x, y;
   int runcount;
   int depth, planes;
   char runchar;
   char ch;

   if (!pal) {
      get_palette(tmppal);
      pal = tmppal;
   }

   f = fp_fopen(filename, F_WRITE);
   if (!f)
      return *allegro_errno;

   depth = bitmap_color_depth(bmp);
   if (depth == 8)
      planes = 1;
   else
      planes = 3;

   fp_putc(10, f);                      /* manufacturer */
   fp_putc(5, f);                       /* version */
   fp_putc(1, f);                       /* run length encoding  */
   fp_putc(8, f);                       /* 8 bits per pixel */
   fp_iputw(0, f);                      /* xmin */
   fp_iputw(0, f);                      /* ymin */
   fp_iputw(bmp->w-1, f);               /* xmax */
   fp_iputw(bmp->h-1, f);               /* ymax */
   fp_iputw(320, f);                    /* HDpi */
   fp_iputw(200, f);                    /* VDpi */

   for (c=0; c<16; c++) {
      fp_putc(_rgb_scale_6[pal[c].r], f);
      fp_putc(_rgb_scale_6[pal[c].g], f);
      fp_putc(_rgb_scale_6[pal[c].b], f);
   }

   fp_putc(0, f);                       /* reserved */
   fp_putc(planes, f);                  /* one or three color planes */
   fp_iputw(bmp->w, f);                 /* number of bytes per scanline */
   fp_iputw(1, f);                      /* color palette */
   fp_iputw(bmp->w, f);                 /* hscreen size */
   fp_iputw(bmp->h, f);                 /* vscreen size */
   for (c=0; c<54; c++)                   /* filler */
      fp_putc(0, f);

   for (y=0; y<bmp->h; y++) {             /* for each scanline... */
      runcount = 0;
      runchar = 0;
      for (x=0; x<bmp->w*planes; x++) {   /* for each pixel... */
     if (depth == 8) {
        ch = getpixel(bmp, x, y);
     }
     else {
        if (x<bmp->w) {
           c = getpixel(bmp, x, y);
           ch = getr_depth(depth, c);
        }
        else if (x<bmp->w*2) {
           c = getpixel(bmp, x-bmp->w, y);
           ch = getg_depth(depth, c);
        }
        else {
           c = getpixel(bmp, x-bmp->w*2, y);
           ch = getb_depth(depth, c);
        }
     }
     if (runcount==0) {
        runcount = 1;
        runchar = ch;
     }
     else {
        if ((ch != runchar) || (runcount >= 0x3f)) {
           if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
          fp_putc(0xC0 | runcount, f);
           fp_putc(runchar,f);
           runcount = 1;
           runchar = ch;
        }
        else
           runcount++;
     }
      }
      if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
     fp_putc(0xC0 | runcount, f);
      fp_putc(runchar,f);
   }

   if (depth == 8) {                      /* 256 color palette */
      fp_putc(12, f); 

      for (c=0; c<256; c++) {
     fp_putc(_rgb_scale_6[pal[c].r], f);
     fp_putc(_rgb_scale_6[pal[c].g], f);
     fp_putc(_rgb_scale_6[pal[c].b], f);
      }
   }

   fp_fclose(f);
   return *allegro_errno;
}

#endif

#endif /* _PCX_FILE_SUPPORT */

