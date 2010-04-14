/* $Id: tga.c,v 1.5 2003/12/01 09:26:36 weiym Exp $
**
** Low-level TGA bitmap file read/save function.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Create date: 2000/08/27, derived from original bitmap.c
** 
** Current maintainer: Wei Yongming.
**
** FIXME: Some errors occurred when RLE used.
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
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      TGA reader by Tim Gunn.
 *
 *      RLE support added by Michal Mertl and Salvador Eduardo Tropea.
 *
 *      See allegro source for copyright information.
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "gdi.h"
#include "readbmp.h"

#ifdef _TGA_FILE_SUPPORT

/* rle_tga_read:
 *  Helper for reading 256 color RLE data from TGA files.
 */
static void rle_tga_read(unsigned char *b, int len, MG_RWops *f)
{
   unsigned char value;
   int count;
   int c = 0;

   do {
      count = fp_getc(f);
      if (count & 0x80) {
         count = (count & 0x7F) + 1;
         c += count;
         value = fp_getc(f);
         while (count--)
            *(b++) = value;
      }
      else {
         count++;
         c += count;
         MGUI_RWread (f, b, 1, count);
         b += count;
      }
   } while (!MGUI_RWeof (f) && c < len);
}



/* rle_tga_read32:
 *  Helper for reading 32 bit RLE data from TGA files.
 */
static void rle_tga_read32(unsigned char *b, int len, MG_RWops *f)
{
   unsigned char value[4];
   int count;
   int c = 0;

   do {
      count = fp_getc(f);
      if (count & 0x80) {
         count = (count & 0x7F) + 1;
         c += count;
         MGUI_RWread (f, value, 1, 4);
         while (count--) {
            b[3] = value[3];
            b[2] = value[2];
            b[1] = value[1];
            b[0] = value[0];
            b += 4;
         }
      }
      else {
         count++;
         c += count;
         while (count--) {
            MGUI_RWread (f, value, 1, 4);
            b[3] = value[3];
            b[2] = value[2];
            b[1] = value[1];
            b[0] = value[0];
            b += 4;
         }
      }
   } while (!MGUI_RWeof (f) && c < len);
}

/* rle_tga_read24:
 *  Helper for reading 24 bit RLE data from TGA files.
 */
static void rle_tga_read24(unsigned char *b, int len, MG_RWops *f)
{
   unsigned char value[4];
   int count;
   int c = 0;

   do {
      count = fp_getc(f);
      if (count & 0x80) {
         count = (count & 0x7F) + 1;
         c += count;
         MGUI_RWread (f, value, 1, 3);
         while (count--) {
            b[2] = value[0];
            b[1] = value[1];
            b[0] = value[2];
            b += 3;
         }
      }
      else {
         count++;
         c += count;
         while (count--) {
            MGUI_RWread (f, value, 1, 3);
            b[2] = value[0];
            b[1] = value[1];
            b[0] = value[2];
            b += 3;
         }
      }
   } while (!MGUI_RWeof (f) && c < len);
}



/* rle_tga_read16:
 *  Helper for reading 16 bit RLE data from TGA files.
 */
static void rle_tga_read16(unsigned short *b, int len, MG_RWops *f)
{
   unsigned int value;
   unsigned short color;
   int count;
   int c = 0;

   do {
      count = fp_getc(f);
      if (count & 0x80) {
         count = (count & 0x7F) + 1;
         c += count;
         value = fp_igetw(f);
         color = value;
         while (count--)
            *(b++) = color;
      }
      else {
         count++;
         c += count;
         while (count--) {
            value = fp_igetw(f);
            color = value;
            *(b++) = color;
         }
      }
   } while (!MGUI_RWeof (f) && c < len);
}



/* load_tga:
 *  Loads a 256 color or 24 bit uncompressed TGA file, returning a bitmap
 *  structure and storing the palette data in the specified palette (this
 *  should be an array of at least 256 RGB structures).
 */
int load_tga (MG_RWops* f, MYBITMAP* bmp, RGB *pal)
{
   unsigned char image_id[256], image_palette[256][3], rgb[4];
   unsigned char id_length, palette_type, image_type, palette_entry_size;
   unsigned char bpp, descriptor_bits;
   short unsigned int first_color, palette_colors;
   short unsigned int left, top, image_width, image_height;
   unsigned int i, x, y;
   unsigned short *s;
   int compressed;
   int pitch, bytes_per_pixel;
   unsigned int size;
   unsigned char* bits;

   id_length = fp_getc(f);
   palette_type = fp_getc(f);
   image_type = fp_getc(f);
   first_color = fp_igetw(f);
   palette_colors  = fp_igetw(f);
   palette_entry_size = fp_getc(f);
   left = fp_igetw(f);
   top = fp_igetw(f);
   image_width = fp_igetw(f);
   image_height = fp_igetw(f);
   bpp = fp_getc(f);
   descriptor_bits = fp_getc(f);

   MGUI_RWread (f, image_id, 1, id_length);
   image_id [id_length - 1] = '\0';
#if 0
    fprintf (stderr, "TGA image id: %s, bpp: %d, colors: %d\n", image_id, bpp, palette_colors);
#endif

   MGUI_RWread (f, image_palette, 1, palette_colors*3);

   /* Image type:
    *    0 = no image data
    *    1 = uncompressed color mapped
    *    2 = uncompressed true color
    *    3 = grayscale
    *    9 = RLE color mapped
    *   10 = RLE true color
    *   11 = RLE grayscale
    */
   compressed = (image_type & 8);
   image_type &= 7;

   if ((image_type < 1) || (image_type > 3)) {
      return ERR_BMP_NOT_SUPPORTED;
   }

   switch (image_type) {

      case 1:
         /* paletted image */
         if ((palette_type != 1) || (bpp != 8)) {
            return ERR_BMP_NOT_SUPPORTED;
         }

         for(i=0; i<palette_colors; i++) {
             pal[i].r = image_palette[i][2];
             pal[i].g = image_palette[i][1];
             pal[i].b = image_palette[i][0];
         }

         break;

      case 2:
         /* truecolor image */
         if ((palette_type == 0) 
                    && ((bpp == 15) || (bpp == 16)
                    || (bpp == 24) || (bpp == 32))) {
         }
         else {
            return ERR_BMP_NOT_SUPPORTED;
         }
         break;

      case 3:
         /* grayscale image */
         if ((palette_type != 0) || (bpp != 8)) {
            return ERR_BMP_NOT_SUPPORTED;
         }

         for (i=0; i<256; i++) {
             pal[i].r = i;
             pal[i].g = i;
             pal[i].b = i;
         }

         break;

      default:
         return ERR_BMP_NOT_SUPPORTED;
   }

    bytes_per_pixel = bmpComputePitch (bpp, image_width, &pitch, TRUE);
    size = pitch * image_height;
    if( !(bits = malloc (size)) ) {
        return ERR_BMP_MEM;
    }

    bmp->flags = MYBMP_TYPE_RGB;
    if (bpp == 24)
        bmp->flags |= MYBMP_RGBSIZE_3;
    else if (bpp == 32)
        bmp->flags |= MYBMP_RGBSIZE_4;

    if (descriptor_bits & 0x20)
        bmp->flags |= MYBMP_FLOW_DOWN;
    else
        bmp->flags |= MYBMP_FLOW_UP;

    bmp->depth = bpp;
    bmp->w     = image_width;
    bmp->h     = image_height;
    bmp->pitch = (compressed) ? (image_width * bytes_per_pixel): pitch;
    bmp->frames = 1;
    bmp->size  = size;
    bmp->bits  = bits;

   for (y=image_height; y; y--) {
      switch (image_type) {

         case 1:
         case 3:
            if (compressed) {
               rle_tga_read(bits, image_width * image_height, f);
               return ERR_BMP_OK;
            }
            else
               MGUI_RWread (f, bits, 1, image_width);
            break;

         case 2:
            if (bpp == 32) {
               if (compressed) {
                  rle_tga_read32(bits, image_width * image_height, f);
                  return ERR_BMP_OK;
               }
               else {
                  for (x=0; x<image_width; x++) {
                     MGUI_RWread (f, rgb, 1, 4);
                     bits[x*4+3] = rgb[3];
                     bits[x*4+2] = rgb[2];
                     bits[x*4+1] = rgb[1];
                     bits[x*4+0] = rgb[0];
                  }
               }
            }
            else if (bpp == 24) {
               if (compressed) {
                  rle_tga_read24(bits, image_width * image_height, f);
                  return ERR_BMP_OK;
               }
               else {
                  for (x=0; x<image_width; x++) {
                     MGUI_RWread (f, rgb, 1, 3);
                     bits[x*3+2] = rgb[2];
                     bits[x*3+1] = rgb[1];
                     bits[x*3+0] = rgb[0];
                  }
               }
            }
            else {
               if (compressed) {
                  rle_tga_read16((unsigned short *)bits, image_width * image_height, f);
                  return ERR_BMP_OK;
               }
               else {
                  s = (unsigned short *)bits;
                  for (x=0; x<image_width; x++) {
                     s [x] = fp_igetw(f);
                  }
               }
            }
            break;
      }
      bits += pitch;
   }

   return ERR_BMP_OK;
}


#if 0
/* save_tga:
 *  Writes a bitmap into a TGA file, using the specified palette (this
 *  should be an array of at least 256 RGB structures).
 */
int save_tga(char *filename, BITMAP *bmp, RGB *pal)
{
   unsigned char image_palette[256][3];
   int x, y, c, r, g, b;
   int depth;
   FILE *f;
   PALETTE tmppal;

   if (!pal) {
      get_palette(tmppal);
      pal = tmppal;
   }

   depth = bitmap_color_depth(bmp);

   if (depth == 15)
      depth = 16;

   f = fp_fopen(filename, F_WRITE);
   if (!f)
      return *allegro_errno;

   fp_putc(0, f);                          /* id length (no id saved) */
   fp_putc((depth == 8) ? 1 : 0, f);       /* palette type */
   fp_putc((depth == 8) ? 1 : 2, f);       /* image type */
   fp_iputw(0, f);                         /* first colour */
   fp_iputw((depth == 8) ? 256 : 0, f);    /* number of colours */
   fp_putc((depth == 8) ? 24 : 0, f);      /* palette entry size */
   fp_iputw(0, f);                         /* left */
   fp_iputw(0, f);                         /* top */
   fp_iputw(bmp->w, f);                    /* width */
   fp_iputw(bmp->h, f);                    /* height */
   fp_putc(depth, f);                      /* bits per pixel */
   fp_putc(0, f);                          /* descriptor (bottom to top) */

   if (depth == 8) {
      for (y=0; y<256; y++) {
         image_palette[y][2] = _rgb_scale_6[pal[y].r];
         image_palette[y][1] = _rgb_scale_6[pal[y].g];
         image_palette[y][0] = _rgb_scale_6[pal[y].b];
      }

      fp_fwrite(image_palette, 768, f);
   }

   switch (bitmap_color_depth(bmp)) {

      #ifdef ALLEGRO_COLOR8

         case 8:
            for (y=bmp->h; y; y--)
               for (x=0; x<bmp->w; x++)
                  fp_putc(getpixel(bmp, x, y-1), f);
            break;

      #endif

      #ifdef ALLEGRO_COLOR16

         case 15:
            for (y=bmp->h; y; y--) {
               for (x=0; x<bmp->w; x++) {
                  c = getpixel(bmp, x, y-1);
                  r = getr15(c);
                  g = getg15(c);
                  b = getb15(c);
                  c = ((r<<7)&0x7C00) | ((g<<2)&0x3E0) | ((b>>3)&0x1F);
                  fp_iputw(c, f);
               }
            }
            break;

         case 16:
            for (y=bmp->h; y; y--) {
               for (x=0; x<bmp->w; x++) {
                  c = getpixel(bmp, x, y-1);
                  r = getr16(c);
                  g = getg16(c);
                  b = getb16(c);
                  c = ((r<<7)&0x7C00) | ((g<<2)&0x3E0) | ((b>>3)&0x1F);
                  fp_iputw(c, f);
               }
            }
            break;

      #endif

      #ifdef ALLEGRO_COLOR24

         case 24:
            for (y=bmp->h; y; y--) {
               for (x=0; x<bmp->w; x++) {
                  c = getpixel(bmp, x, y-1);
                  fp_putc(getb24(c), f);
                  fp_putc(getg24(c), f);
                  fp_putc(getr24(c), f);
               }
            }
            break;

      #endif

      #ifdef ALLEGRO_COLOR32

         case 32:
            for (y=bmp->h; y; y--) {
               for (x=0; x<bmp->w; x++) {
                  c = getpixel(bmp, x, y-1);
                  fp_putc(getb32(c), f);
                  fp_putc(getg32(c), f);
                  fp_putc(getr32(c), f);
                  fp_putc(geta32(c), f);
               }
            }
            break;

      #endif
   }

   fp_fclose(f);
   return *allegro_errno;
}
#endif

#endif /* _TGA_FILE_SUPPORT */

