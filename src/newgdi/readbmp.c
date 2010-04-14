/*
** $Id: readbmp.c,v 1.24 2003/09/04 06:02:53 weiym Exp $
**
** Top-level bitmap file read/save function.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/08/26, derived from original bitmap.c
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

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"
#include "readbmp.h"

/* Init a bitmap as a normal bitmap  */
BOOL GUIAPI InitBitmap (HDC hdc, Uint32 w, Uint32 h, Uint32 pitch, BYTE* bits, PBITMAP bmp)
{
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);

    if (w == 0 || h == 0)
        return FALSE;

    if (bits && pitch) {
        if (pdc->surface->format->BytesPerPixel * w > pitch)
            return FALSE;
        bmp->bmBits = bits;
        bmp->bmPitch = pitch;
    }
    else if (!bits) {

        size_t size = GAL_GetBoxSize (pdc->surface, w, h, &bmp->bmPitch);
        if (!(bmp->bmBits = malloc (size)))
            return FALSE;
    }
    else /* bits is not zero, but pitch is zero */
        return FALSE;

    bmp->bmType = BMP_TYPE_NORMAL;
    bmp->bmAlpha = 0;
    bmp->bmColorKey = 0;
    bmp->bmWidth = w;
    bmp->bmHeight = h;
    bmp->bmBitsPerPixel = pdc->surface->format->BitsPerPixel;
    bmp->bmBytesPerPixel = pdc->surface->format->BytesPerPixel;
    bmp->bmAlphaPixelFormat = NULL;

    return TRUE;
}

/* Init a standard pixel format with alpha of a bitmap */
BOOL GUIAPI InitBitmapPixelFormat (HDC hdc, PBITMAP bmp)
{
    PDC pdc;
    Uint32 Rmask = 0, Gmask = 0, Bmask = 0, Amask = 0;

    pdc = dc_HDC2PDC (hdc);

    bmp->bmBitsPerPixel = pdc->surface->format->BitsPerPixel;
    bmp->bmBytesPerPixel = pdc->surface->format->BytesPerPixel;

    if (bmp->bmType & BMP_TYPE_ALPHA) {
        if (pdc->surface->format->Amask || bmp->bmBitsPerPixel <= 8) {
            bmp->bmAlphaPixelFormat = pdc->surface->format;
            return TRUE;
        }

        switch (bmp->bmBitsPerPixel) {
            case 16:
                Rmask = 0x0000F000;
                Gmask = 0x00000F00;
                Bmask = 0x000000F0;
                Amask = 0x0000000F;
                break;
            case 24:
                Rmask = 0x00FC0000;
                Gmask = 0x0003F000;
                Bmask = 0x00000FC0;
                Amask = 0x0000003F;
                break;
            case 32:
                Rmask = 0xFF000000;
                Gmask = 0x00FF0000;
                Bmask = 0x0000FF00;
                Amask = 0x000000FF;
                break;
            default:
                return FALSE;
        }
        bmp->bmAlphaPixelFormat = GAL_AllocFormat (bmp->bmBitsPerPixel,
                    Rmask, Gmask, Bmask, Amask);
        if (!bmp->bmAlphaPixelFormat)
            return FALSE;

        bmp->bmType |= BMP_TYPE_PRIV_PIXEL;
    }
    else
        bmp->bmAlphaPixelFormat = NULL;

    return TRUE;
}

/* Function: int ExpandMyBitmap (HDC hdc, PBITMAP bmp, const MYBITMAP* my_bmp, int frame);
 *      This function expand a mybitmap to compiled bitmap.
 */
int GUIAPI ExpandMyBitmap (HDC hdc, PBITMAP bmp, const MYBITMAP* my_bmp, const RGB* pal, int frame)
{
    PDC pdc;
    Uint8* bits;

    pdc = dc_HDC2PDC (hdc);

    bmp->bmBitsPerPixel = pdc->surface->format->BitsPerPixel;
    bmp->bmBytesPerPixel = pdc->surface->format->BytesPerPixel;
    bmp->bmAlphaPixelFormat = NULL;

    if (!(bmp->bmBits = malloc (GAL_GetBoxSize (pdc->surface, 
            my_bmp->w, my_bmp->h, &bmp->bmPitch)))) {
        return ERR_BMP_MEM;
    }

    bmp->bmWidth = my_bmp->w;
    bmp->bmHeight = my_bmp->h;

    bmp->bmType = BMP_TYPE_NORMAL;
    if (my_bmp->flags & MYBMP_TRANSPARENT) {
        bmp->bmType |= BMP_TYPE_COLORKEY;
        if (pal && my_bmp->depth <= 8)
            bmp->bmColorKey = GAL_MapRGB (pdc->surface->format, 
                pal [my_bmp->transparent].r,
                pal [my_bmp->transparent].g,
                pal [my_bmp->transparent].b);
        else {
            Uint8 r, g, b;
#if 1
            r = GetRValue (my_bmp->transparent);
            g = GetGValue (my_bmp->transparent);
            b = GetBValue (my_bmp->transparent);
#else
            Uint8* src = (Uint8*) (&my_bmp->transparent);
            if ((my_bmp->flags & MYBMP_TYPE_MASK) == MYBMP_TYPE_BGR) {
                b = *src++; g = *src++; r = *src++;
            }
            else {
                r = *src++; g = *src++; b = *src++;
            }
#endif
            bmp->bmColorKey = GAL_MapRGB (pdc->surface->format, r, g, b);
        }
    }
    else
        bmp->bmColorKey = 0;

    if (my_bmp->flags & MYBMP_ALPHACHANNEL) {
        bmp->bmType |= BMP_TYPE_ALPHACHANNEL;
        bmp->bmAlpha = my_bmp->alpha;
    }
    else
        bmp->bmAlpha = 0;

    if (my_bmp->flags & MYBMP_ALPHA && my_bmp->depth == 32) {
        Uint32 Rmask = 0, Gmask = 0, Bmask = 0, Amask = 0;
        switch (bmp->bmBitsPerPixel) {
                case 16:
                    Rmask = 0x0000F000;
                    Gmask = 0x00000F00;
                    Bmask = 0x000000F0;
                    Amask = 0x0000000F;
                    break;
                case 24:
                    Rmask = 0x00FC0000;
                    Gmask = 0x0003F000;
                    Bmask = 0x00000FC0;
                    Amask = 0x0000003F;
                    break;
                case 32:
                    Rmask = 0xFF000000;
                    Gmask = 0x00FF0000;
                    Bmask = 0x0000FF00;
                    Amask = 0x000000FF;
                    break;
        }

        if (bmp->bmBitsPerPixel > 8) {

            bmp->bmAlphaPixelFormat 
                        = GAL_AllocFormat (my_bmp->depth, Rmask, Gmask, Bmask, Amask);
            if (!bmp->bmAlphaPixelFormat)
                return ERR_BMP_MEM;

            bmp->bmType |= BMP_TYPE_ALPHA;
            bmp->bmType |= BMP_TYPE_PRIV_PIXEL;
        }
        else { /* for bpp <= 8, just strip alpha */
            bmp->bmAlphaPixelFormat = pdc->surface->format;
        }
    }
    else
        bmp->bmAlphaPixelFormat = NULL;

    if (frame <= 0 || frame >= my_bmp->frames)
        bits = my_bmp->bits;
    else
        bits = my_bmp->bits + frame * my_bmp->size;

    switch (my_bmp->depth) {
    case 1:
        ExpandMonoBitmap (hdc, bmp->bmBits, bmp->bmPitch, bits, my_bmp->pitch, 
                        my_bmp->w, my_bmp->h, my_bmp->flags, 
                        GAL_MapRGB (pdc->surface->format, pal[0].r, pal[0].g, pal[0].b),
                        GAL_MapRGB (pdc->surface->format, pal[1].r, pal[1].g, pal[1].b));
        break;
    case 4:
        Expand16CBitmap (hdc, bmp->bmBits, bmp->bmPitch, bits, my_bmp->pitch, 
                        my_bmp->w, my_bmp->h, my_bmp->flags, pal);
        break;
    case 8:
        Expand256CBitmap (hdc, bmp->bmBits, bmp->bmPitch, bits, my_bmp->pitch, 
                        my_bmp->w, my_bmp->h, my_bmp->flags, pal);
        break;
    case 24:
        CompileRGBABitmap (hdc, bmp->bmBits, bmp->bmPitch, bits, my_bmp->pitch, 
                        my_bmp->w, my_bmp->h, my_bmp->flags & ~MYBMP_RGBSIZE_4, 
                        bmp->bmAlphaPixelFormat);
        break;
    case 32:
        CompileRGBABitmap (hdc, bmp->bmBits, bmp->bmPitch, bits, my_bmp->pitch, 
                        my_bmp->w, my_bmp->h, my_bmp->flags | MYBMP_RGBSIZE_4, 
                        bmp->bmAlphaPixelFormat);
        break;
    default:
        return ERR_BMP_NOT_SUPPORTED;
    }

    return ERR_BMP_OK;
}

/* Function: int LoadBitmapEx (HDC hdc, PBITMAP bmp, MG_RWops* area, const char* ext)
 *      This function loads a bitmap from an image source and fills
 *      the specified BITMAP struct.
 */
int GUIAPI LoadBitmapEx (HDC hdc, PBITMAP bmp, MG_RWops* area, const char* ext)
{
    MYBITMAP myBitmap;
    RGB pal [256];
    int ret;

    if ((ret = LoadMyBitmapEx (&myBitmap, pal, area, ext)) < 0)
        return ret;

    ret = ExpandMyBitmap (hdc, bmp, &myBitmap, pal, 0);
    free (myBitmap.bits);

    return ret;
}

int GUIAPI LoadBitmapFromFile (HDC hdc, PBITMAP bmp, const char* file_name)
{
    MYBITMAP myBitmap;
    RGB pal [256];
    int ret;

    if ((ret = LoadMyBitmapFromFile (&myBitmap, pal, file_name)) < 0)
        return ret;

    ret = ExpandMyBitmap (hdc, bmp, &myBitmap, pal, 0);
    free (myBitmap.bits);

    return ret;
}

int GUIAPI LoadBitmapFromMem (HDC hdc, PBITMAP bmp, const void* mem, int size, const char* ext)
{
    MYBITMAP myBitmap;
    RGB pal [256];
    int ret;

    if ((ret = LoadMyBitmapFromMem (&myBitmap, pal, mem, size, ext)) < 0)
        return ret;

    ret = ExpandMyBitmap (hdc, bmp, &myBitmap, pal, 0);
    free (myBitmap.bits);

    return ret;
}

/* this function delete the pixel format of a bitmap */
void GUIAPI DeleteBitmapAlphaPixel (PBITMAP bmp)
{
    if (bmp->bmType & BMP_TYPE_PRIV_PIXEL) {
        GAL_FreeFormat (bmp->bmAlphaPixelFormat);
        bmp->bmType &= ~BMP_TYPE_PRIV_PIXEL;
        bmp->bmAlphaPixelFormat = NULL;
    }
}

/* this function unloads bitmap */
void GUIAPI UnloadBitmap (PBITMAP bmp)
{
    DeleteBitmapAlphaPixel (bmp);

    free (bmp->bmBits);
    bmp->bmBits = NULL;
}

#ifdef _SAVE_BITMAP
int GUIAPI SaveBitmapToFile (HDC hdc, PBITMAP bmp, const char* file_name)
{
    PDC pdc;
    MYBITMAP myBitmap;
    int i;
    RGB pal [256];
    GAL_Palette* palette;

    pdc = dc_HDC2PDC (hdc);
    palette = pdc->surface->format->palette;
    switch (GAL_BitsPerPixel (pdc->surface)) {
    case 4:
        for (i = 0; i < 16; i++) {
            pal [i].r = palette->colors [i].r;
            pal [i].g = palette->colors [i].g;
            pal [i].b = palette->colors [i].b;
        }
        break;
    case 8:
        for (i = 0; i < 256; i++) {
            pal [i].r = palette->colors [i].r;
            pal [i].g = palette->colors [i].g;
            pal [i].b = palette->colors [i].b;
        }
        break;
    }

    myBitmap.flags = MYBMP_TYPE_NORMAL;
    myBitmap.frames = 1;
    myBitmap.depth = GAL_BitsPerPixel (pdc->surface);
    myBitmap.w = bmp->bmWidth;
    myBitmap.h = bmp->bmHeight;
    myBitmap.pitch = bmp->bmPitch;
    myBitmap.size = myBitmap.h * bmp->bmPitch;
    myBitmap.bits = bmp->bmBits;

    return SaveMyBitmapToFile (&myBitmap, pal, file_name);
}

#endif

