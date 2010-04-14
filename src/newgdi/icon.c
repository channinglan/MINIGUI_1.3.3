/*
** $Id: icon.c,v 1.22 2003/11/23 04:09:08 weiym Exp $
**
** icon.c: Icon operations of GDI.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/06/12, derived from original gdi.c
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

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "inline.h"
#include "memops.h"
#include "dc.h"
#include "icon.h"
#include "cursor.h"
#include "readbmp.h"

#define align_32_bits(b) (((b) + 3) & -4)

extern BOOL dc_GenerateECRgn(PDC pdc, BOOL fForce);

/************************* Icon support **************************************/
// Icon creating and destroying.
HICON GUIAPI LoadIconFromFile (HDC hdc, const char* filename, int which)
{
    FILE* fp;
    WORD wTemp;
    BYTE bTemp;
    int  w, h, colornum;
    DWORD size, offset;
    DWORD imagesize, imagew, imageh;
    BYTE* image;
    HICON icon = 0;
    
    if( !(fp = fopen(filename, "rb")) ) return 0;

    fseek(fp, sizeof(WORD), SEEK_SET);

    // the cbType of struct ICONDIR.
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp != 1) goto error;

    // get ICON images count.
    wTemp = MGUI_ReadLE16FP (fp);
    if (which >= wTemp)
        which = wTemp - 1;
    if (which < 0)
        which = 0;

    // seek to the right ICONDIRENTRY if needed.
    if (which != 0)
        fseek (fp, SIZEOF_ICONDIRENTRY * which, SEEK_CUR);

    // cursor info, read the members of struct ICONDIRENTRY.
    w = fgetc (fp);       // the width of first cursor
    h = fgetc (fp);       // the height of first cursor
    if ((w%16) != 0 || (h%16) != 0) goto error;
    bTemp = fgetc (fp);   // the bColorCount
    if(bTemp != 2 && bTemp != 16) goto error;
    fseek(fp, sizeof(BYTE), SEEK_CUR); // skip the bReserved
    wTemp = MGUI_ReadLE16FP (fp);   // the wPlanes
    if(wTemp != 0) goto error;
    wTemp = MGUI_ReadLE16FP (fp);   // the wBitCount
    if(wTemp > 4) goto error;
    size = MGUI_ReadLE32FP (fp);
    offset = MGUI_ReadLE32FP (fp);

    // read the cursor image info.
    fseek(fp, offset, SEEK_SET);
    fseek(fp, sizeof(DWORD), SEEK_CUR); // skip the biSize member.
    imagew = MGUI_ReadLE32FP (fp);
    imageh = MGUI_ReadLE32FP (fp);
    // check the biPlanes member;
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp != 1) goto error;
    // check the biBitCount member;
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp > 4) goto error;
    colornum = (int)wTemp;
    fseek(fp, sizeof(DWORD), SEEK_CUR); // skip the biCompression members.

    if (colornum == 1)
        imagesize = align_32_bits(w>>3) * h;
    else
        imagesize = align_32_bits(w>>1) * h;

    imagesize += align_32_bits(w>>3) * h;
    fseek(fp, sizeof(DWORD), SEEK_CUR);

    // skip the rest members and the color table.
    fseek(fp, sizeof(DWORD)*4 + sizeof(BYTE)*(4<<colornum), SEEK_CUR);
    
    // allocate memory for image.
#ifdef HAVE_ALLOCA
    // Use alloca, the stack may be enough for the image.
    if ((image = (BYTE*) alloca (imagesize)) == NULL)
#else
    if ((image = (BYTE*) malloc (imagesize)) == NULL)
#endif
        goto error;

    // read image
    fread (image, imagesize, 1, fp);

    icon = CreateIcon (hdc, w, h, 
                    image + (imagesize - (align_32_bits(w>>3) * h)), image, colornum);

#ifndef HAVE_ALLOCA
    free (image);
#endif

error:
    fclose (fp);
    return icon;
}

HICON GUIAPI LoadIconFromMem (HDC hdc, const void* area, int which)
{
    const Uint8* p = (Uint8*)area;
    WORD wTemp;
    BYTE bTemp;

    int  w, h, colornum;
    DWORD size, offset;
    DWORD imagesize, imagew, imageh;
    
    p += sizeof (WORD);

    // the cbType of struct ICONDIR.
    wTemp = MGUI_ReadLE16Mem (&p);
    if (wTemp != 1) goto error;

    // get ICON images count.
    wTemp = MGUI_ReadLE16Mem (&p);
    if (which >= wTemp)
        which = wTemp - 1;
    if (which < 0)
        which = 0;

    // seek to the right ICONDIRENTRY if needed.
    if (which != 0)
        p += SIZEOF_ICONDIRENTRY * which;

    // cursor info, read the members of struct ICONDIRENTRY.
    w = *p++;       // the width of first cursor
    h = *p++;       // the height of first cursor
    if ((w%16) != 0 || (h%16) != 0)
        goto error;
    bTemp = *p++;   // the bColorCount
    if(bTemp != 2 && bTemp != 16) goto error;

    // skip the bReserved
    p ++;
    wTemp = MGUI_ReadLE16Mem (&p);   // the wPlanes
    if (wTemp != 0) goto error;
    wTemp = MGUI_ReadLE16Mem (&p);   // the wBitCount
    if (wTemp > 4) goto error;
    size = MGUI_ReadLE32Mem (&p);
    offset = MGUI_ReadLE32Mem (&p);

    // read the cursor image info.
    p = (Uint8*)area + offset;
    // skip the biSize member.
    p += sizeof(DWORD); 
    imagew = MGUI_ReadLE32Mem (&p);
    imageh = MGUI_ReadLE32Mem (&p);
    // check the biPlanes member;
    wTemp = MGUI_ReadLE16Mem (&p);
    if (wTemp != 1) goto error;
    // check the biBitCount member;
    wTemp = MGUI_ReadLE16Mem (&p);
    if (wTemp > 4) goto error;
    colornum = (int)wTemp;
    // skip the biCompression members.
    p += sizeof (DWORD);

    if (colornum == 1)
        imagesize = align_32_bits(w>>3) * h;
    else
        imagesize = align_32_bits(w>>1) * h;

    imagesize += align_32_bits(w>>3) * h;
    p += sizeof (DWORD);

    // skip the rest members and the color table.
    p += sizeof(DWORD)*4 + sizeof(BYTE)*(4<<colornum);
    
    return CreateIcon (hdc, w, h, 
                    p + (imagesize - (align_32_bits(w>>3) * h)), p, colornum);

error:
    return 0;
}

HICON GUIAPI CreateIcon (HDC hdc, int w, int h, const BYTE* pAndBits, 
                        const BYTE* pXorBits, int colornum)
{
    PDC pdc;
    PICON picon;
    int bpp;
    Uint32 image_size;

    pdc = dc_HDC2PDC (hdc);
    bpp = GAL_BytesPerPixel (pdc->surface);
    
    if( (w%16) != 0 || (h%16) != 0 ) return 0;

    // allocate memory.
    if (!(picon = (PICON)malloc (sizeof(ICON))))
        return 0;

    image_size = GAL_GetBoxSize (pdc->surface, w, h, &picon->pitch);

    if (!(picon->AndBits = malloc (image_size)))
        goto error1;
    if (!(picon->XorBits = malloc (image_size)))
        goto error2;

    picon->width = w;
    picon->height = h;

    if (colornum == 1) {
        ExpandMonoBitmap (HDC_SCREEN, picon->AndBits, picon->pitch, pAndBits, align_32_bits (w >> 3),
                        w, h, MYBMP_FLOW_UP, 0, 0xFFFFFFFF);
        ExpandMonoBitmap (HDC_SCREEN, picon->XorBits, picon->pitch, pXorBits, align_32_bits (w >> 3),
                        w, h, MYBMP_FLOW_UP, 0, 0xFFFFFFFF);
    }
    else if (colornum == 4) {
        ExpandMonoBitmap (HDC_SCREEN, picon->AndBits, picon->pitch, pAndBits, align_32_bits (w >> 3),
                        w, h, MYBMP_FLOW_UP, 0, 0xFFFFFFFF);
        Expand16CBitmap (HDC_SCREEN, picon->XorBits, picon->pitch, pXorBits, align_32_bits (w >> 1),
                        w, h, MYBMP_FLOW_UP, NULL);
    }

    return (HICON)picon;

error2:
    free(picon->AndBits);
error1:
    free (picon);

    return 0;
}

BOOL GUIAPI DestroyIcon (HICON hicon)
{
    PICON picon = (PICON)hicon;

    if (!picon)
        return FALSE;

    free(picon->AndBits);
    free(picon->XorBits);
    free(picon);
    return TRUE;
}

BOOL GUIAPI GetIconSize (HICON hicon, int* w, int* h)
{
    PICON picon = (PICON)hicon;

    if (!picon)
        return FALSE;

    if (w) *w = picon->width;
    if (h) *h = picon->height;
    return TRUE;
}

void GUIAPI DrawIcon (HDC hdc, int x, int y, int w, int h, HICON hicon)
{
    PDC pdc;
    PCLIPRECT cliprect;
    PICON picon = (PICON)hicon;
    Uint32 imagesize, pitch;
    BYTE* iconimage;
    BITMAP bitmap;
    BYTE* andbits = NULL;
    BYTE* xorbits = NULL;
    RECT eff_rc;
    GAL_Rect rect;

    if (!(pdc = check_ecrgn (hdc)))
        return;

    if (w <= 0) w = picon->width;
    if (h <= 0) h = picon->height;

    // Transfer logical to device to screen here.
    w += x; h += y;
    coor_LP2SP (pdc, &x, &y);
    coor_LP2SP (pdc, &w, &h);
    SetRect (&pdc->rc_output, x, y, w, h);
    NormalizeRect (&pdc->rc_output);
    w = RECTW (pdc->rc_output); h = RECTH (pdc->rc_output);

    imagesize = GAL_GetBoxSize (pdc->surface, w, h, &pitch);

    if ((iconimage = malloc (imagesize)) == NULL)
        goto free_ret;

    if (w != picon->width || h != picon->height) {
        BITMAP unscaled, scaled;

        andbits = malloc (imagesize);
        xorbits = malloc (imagesize);
        if (andbits == NULL || xorbits == NULL)
            goto free_ret;

        unscaled.bmType = BMP_TYPE_NORMAL;
        unscaled.bmBytesPerPixel = GAL_BytesPerPixel (pdc->surface);
        unscaled.bmWidth = picon->width;
        unscaled.bmHeight = picon->height;
        unscaled.bmPitch = picon->pitch;
        unscaled.bmBits = picon->AndBits;

        unscaled.bmType = BMP_TYPE_NORMAL;
        scaled.bmBytesPerPixel = unscaled.bmBytesPerPixel;
        scaled.bmWidth = w;
        scaled.bmHeight = h;
        scaled.bmPitch = pitch;
        scaled.bmBits = andbits;

        ScaleBitmap (&scaled, &unscaled);

        unscaled.bmBits = picon->XorBits;
        scaled.bmBits = xorbits;
        ScaleBitmap (&scaled, &unscaled);
    }
    else {
        andbits = picon->AndBits;
        xorbits = picon->XorBits;
        pitch = picon->pitch;
    }

    ENTER_DRAWING (pdc);

    rect.x = x; rect.y = y; rect.w = w; rect.h = h;
    bitmap.bmPitch = pitch;
    bitmap.bmBits = iconimage;
    GAL_GetBox (pdc->surface, &rect, &bitmap);

#ifdef ASM_memandcpy4
    ASM_memandcpy4 (iconimage, andbits, imagesize >> 2);
    ASM_memxorcpy4 (iconimage, xorbits, imagesize >> 2);
#else
    {
        int i;
        Uint32* dst = (Uint32*)iconimage;
        Uint32* src1 = (Uint32*)andbits;
        Uint32* src2 = (Uint32*)xorbits;
        for (i = 0; i < imagesize>>2; i++) {
            dst [i] &= src1 [i];
            dst [i] ^= src2 [i];
        }
    }
#endif

    cliprect = pdc->ecrgn.head;
    while (cliprect) {
        if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)) {
            SET_GAL_CLIPRECT(pdc, eff_rc);

            GAL_PutBox (pdc->surface, &rect, &bitmap);
        }

        cliprect = cliprect->next;
    }

    LEAVE_DRAWING (pdc);

free_ret:
    UNLOCK_GCRINFO (pdc);
    free (iconimage);
    if (w != picon->width || h != picon->height) {
        free (andbits);
        free (xorbits);
    }
}

