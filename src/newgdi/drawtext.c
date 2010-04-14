/*
** $Id: drawtext.c,v 1.31 2003/11/21 13:22:24 weiym Exp $
** 
** drawtext.c: Low level text drawing.
** 
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 WEI Yongming.
**
** Current maintainer: WEI Yongming.
**
** Create date: 2000/06/15
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
#include "dc.h"
#include "pixel_ops.h"
#include "cursor.h"
#include "drawtext.h"

static BITMAP char_bmp;
static size_t char_bits_size;

BOOL InitTextBitmapBuffer (void)
{
    return TRUE;
}

void TermTextBitmapBuffer (void)
{
    free (char_bmp.bmBits);
    char_bmp.bmBits = NULL;
    char_bits_size = 0;
}

static void prepare_bitmap (PDC pdc, int w, int h)
{
    Uint32 size = GAL_GetBoxSize (pdc->surface, w, h, &char_bmp.bmPitch);

    char_bmp.bmType = BMP_TYPE_NORMAL;
    char_bmp.bmBitsPerPixel = pdc->surface->format->BitsPerPixel;
    char_bmp.bmBytesPerPixel = pdc->surface->format->BytesPerPixel;
    char_bmp.bmWidth = w;
    char_bmp.bmHeight = h;

    if (size <= char_bits_size)
        return;

    char_bits_size = ((size + 31) >> 5) << 5;
    char_bmp.bmBits = realloc (char_bmp.bmBits, char_bits_size);
}

void gdi_start_new_line (LOGFONT* log_font)
{
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;

    if (mbc_devfont) {
        if (mbc_devfont->font_ops->start_str_output)
            (*mbc_devfont->font_ops->start_str_output) (log_font, mbc_devfont);
    }
    if (sbc_devfont->font_ops->start_str_output)
            (*sbc_devfont->font_ops->start_str_output) (log_font, sbc_devfont);
}

inline static int get_light (int fg, int bg)
{
    return bg + (fg - bg) / 4;
}

inline static int get_medium (int fg, int bg)
{
    return bg + (fg - bg) * 2 / 4;
}

inline static int get_dark (int fg, int bg)
{
    return bg + (fg - bg) * 3 / 4;
}

static void expand_char_pixmap (PDC pdc, int w, int h, const BYTE* bits, 
            BYTE* expanded, int bold, int italic, int cols)
{
    GAL_Color pal [5];
    gal_pixel pixel [5];
    int i, x, y;
    int b = 0;
    BYTE* line;

    GAL_GetRGB (pdc->bkcolor, pdc->surface->format, &pal->r, &pal->g, &pal->b);
    GAL_GetRGB (pdc->textcolor, pdc->surface->format, &pal[4].r, &pal[4].g, &pal[4].b);
    pal [1].r = get_light  (pal [4].r, pal [0].r);
    pal [1].g = get_light  (pal [4].g, pal [0].g);
    pal [1].b = get_light  (pal [4].b, pal [0].b);
    pal [2].r = get_medium (pal [4].r, pal [0].r);
    pal [2].g = get_medium (pal [4].g, pal [0].g);
    pal [2].b = get_medium (pal [4].b, pal [0].b);
    pal [3].r = get_dark  (pal [4].r, pal [0].r);
    pal [3].g = get_dark  (pal [4].g, pal [0].g);
    pal [3].b = get_dark  (pal [4].b, pal [0].b);
    for (i = 0; i < 5; i++) {
        pixel [i] = GAL_MapRGB (pdc->surface->format, pal[i].r, pal[i].g, pal[i].b);
    }

    line = expanded;
    switch (GAL_BytesPerPixel (pdc->surface)) {
    case 1:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic); x++) {
                *(expanded + x) = pixel [0];
            }

            if (italic)
                expanded += (h - y) >> 1;
            for (x = 0; x < w; x++) {
                b = *(bits+x);
                if (b == 255) b = 4;
                else if (b >= 128) b = 3;
                else if (b >= 64) b = 2;
                else if (b >= 32) b = 1;
                else if (b >= 0) b = 0;
                *expanded = pixel [b];
                if (bold)
                    *(expanded + 1) = pixel [b];

                expanded++;
            }
            bits += cols;
            line += char_bmp.bmPitch;
        }
    break;

    case 2:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic) << 1; x += 2) {
                *(Uint16 *) (expanded + x) = pixel [0];
            }

            if (italic)
                expanded += ((h - y) >> 1) << 1;
            for (x = 0; x < w; x++) {
                b = *(bits+x);
                if (b == 255) b = 4;
                else if (b >= 128) b = 3;
                else if (b >= 64) b = 2;
                else if (b >= 32) b = 1;
                else if (b >= 0) b = 0;
                *(Uint16 *) expanded = pixel [b];
                if (bold)
                    *(Uint16 *)(expanded + 2) = pixel [b];

                expanded += 2;
            }
            bits += cols;
            line += char_bmp.bmPitch;
        }
    break;

    case 3:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic) * 3; x += 3) {
                *(Uint16 *) (expanded + x) = pixel [0];
                *(expanded + x + 2) = pixel [0] >> 16;
            }

            if (italic)
                expanded += 3 * ((h - y) >> 1);

            for (x = 0; x < w; x++) {
                b = *(bits+x);
                if (b == 255) b = 4;
                else if (b >= 128) b = 3;
                else if (b >= 64) b = 2;
                else if (b >= 32) b = 1;
                else if (b >= 0) b = 0;
                *(Uint16 *) expanded = pixel[b];
                *(expanded + 2) = pixel[b] >> 16;
                if (bold) {
                    *(Uint16 *)(expanded + 3) = pixel[b];
                    *(expanded + 5) = pixel[b] >> 16;
                }
                
                expanded += 3;
            }
            bits += cols;
            line += char_bmp.bmPitch;
        }
    break;

    case 4:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic) << 2; x += 4) {
                *(Uint32 *) (expanded + x)= pixel [0];
            }

            if (italic)
                expanded += ((h - y) >> 1) << 2;

            for (x = 0; x < w; x++) {
                b = *bits++;
                if (b == 255) b = 4;
                else if (b >= 128) b = 3;
                else if (b >= 64) b = 2;
                else if (b >= 32) b = 1;
                else if (b >= 0) b = 0;
                *(Uint32 *) expanded = pixel[b];
                if (bold)
                    *(Uint32 *) (expanded + 4) = pixel[b];

                expanded += 4;
            }
            line += char_bmp.bmPitch;
        }
    }
}

static void expand_char_bitmap (int w, int h, 
            const BYTE* bits, int bpp, BYTE* expanded, 
            int bg, int fg, int bold, int italic)
{
    int x, y;
    int b = 0;
    BYTE* line;

    line = expanded;
    switch (bpp) {
    case 1:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic); x++) {
                *(expanded + x) = bg;
            }

            if (italic)
                expanded += (h - y) >> 1;
            for (x = 0; x < w; x++) {
                if (x % 8 == 0)
                    b = *bits++;
                if ((b & (128 >> (x % 8)))) {
                    *expanded = fg;
                    if (bold)
                        *(expanded + 1) = fg;
                }

                expanded++;
            }
            line += char_bmp.bmPitch;
        }
    break;

    case 2:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic) << 1; x += 2) {
                *(Uint16 *) (expanded + x) = bg;
            }

            if (italic)
                expanded += ((h - y) >> 1) << 1;
            for (x = 0; x < w; x++) {
                if (x % 8 == 0)
                    b = *bits++;
                if ((b & (128 >> (x % 8)))) {
                    *(Uint16 *) expanded = fg;
                    if (bold)
                        *(Uint16 *)(expanded + 2) = fg;
                }

                expanded += 2;
            }
            line += char_bmp.bmPitch;
        }
    break;

    case 3:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic) * 3; x += 3) {
                *(Uint16 *) (expanded + x) = bg;
                *(expanded + x + 2) = bg >> 16;
            }

            if (italic)
                expanded += 3 * ((h - y) >> 1);

            for (x = 0; x < w; x++) {
                if (x % 8 == 0)
                    b = *bits++;
                if ((b & (128 >> (x % 8)))) {
                    *(Uint16 *) expanded = fg;
                    *(expanded + 2) = fg >> 16;
                    if (bold) {
                        *(Uint16 *)(expanded + 3) = fg;
                        *(expanded + 5) = fg >> 16;
                    }
                }
                
                expanded += 3;
            }
            line += char_bmp.bmPitch;
        }
    break;

    case 4:
        for (y = 0; y < h; y++) {
            expanded = line;
            for (x = 0; x < (w + bold + italic) << 2; x += 4) {
                *(Uint32 *) (expanded + x)= bg;
            }

            if (italic)
                expanded += ((h - y) >> 1) << 2;

            for (x = 0; x < w; x++) {
                if (x % 8 == 0)
                    b = *bits++;
                if ((b & (128 >> (x % 8)))) {
                    *(Uint32 *) expanded = fg;
                    if (bold)
                        *(Uint32 *) (expanded + 4) = fg;
                }

                expanded += 4;
            }
            line += char_bmp.bmPitch;
        }
    }
}

/* return width of output */
static void put_one_char (PDC pdc, LOGFONT* logfont, DEVFONT* devfont,
                int* px, int* py, int h, int ascent, const char* mchar, int len)
{
    const BYTE* bits;
    int bbox_x = *px, bbox_y = *py;
    int bbox_w, bbox_h, bold = 0, italic = 0;
    int bpp, pitch = 0;
    int old_x, old_y;
    GAL_Rect fg_rect, bg_rect;
    RECT rcOutput, rcTemp;
    
    if (devfont->font_ops->get_char_bbox) {
        (*devfont->font_ops->get_char_bbox) (logfont, devfont,
                            mchar, len, &bbox_x, &bbox_y, &bbox_w, &bbox_h);
#if 0
        printf ("bbox of %c: %d, %d, %d, %d\n", *mchar, bbox_x, bbox_y, bbox_w, bbox_h);
#endif
    }
    else {
        bbox_w = (*devfont->font_ops->get_char_width) (logfont, devfont, 
                mchar, len);
        bbox_h = h;
        bbox_y -= ascent;
    }

    if (logfont->style & FS_WEIGHT_BOLD 
        && !(devfont->style & FS_WEIGHT_BOLD)) {
        bold = 1;
    }
    if (logfont->style & FS_SLANT_ITALIC
        && !(devfont->style & FS_SLANT_ITALIC)) {
        italic = (bbox_h - 1) >> 1;
    }

    if (logfont->style & FS_WEIGHT_BOOK
            && devfont->font_ops->get_char_pixmap) {
        bits = (*devfont->font_ops->get_char_pixmap) (logfont, devfont, 
                mchar, len, &pitch);
    }
    else {
        bits = (*devfont->font_ops->get_char_bitmap) (logfont, devfont, 
                mchar, len);
    }

    if (bits == NULL)
        return;

    fg_rect.x = bbox_x; fg_rect.y = bbox_y;
    fg_rect.w = (bbox_w + bold + italic); fg_rect.h = bbox_h;

    old_x = *px; old_y = *py;
    if (devfont->font_ops->get_char_advance) {
        (*devfont->font_ops->get_char_advance) (logfont, devfont, mchar, len, px, py);
        if (pdc->bkmode != BM_TRANSPARENT && old_y == *py) {
            bg_rect.y = *py - ascent;
            bg_rect.h = h;
            if (*px > old_x) {
                bg_rect.x = old_x;
                bg_rect.w = *px - old_x;
            }
            else {
                bg_rect.x = *px;
                bg_rect.w = old_x - *px;
            }
        }
        else
            bg_rect = fg_rect;
    }
    else {
        *px += bbox_w + bold;
        bg_rect = fg_rect;
    }

    rcOutput.left = MIN (fg_rect.x, bg_rect.x);
    rcOutput.top = MIN (fg_rect.y, bg_rect.y);
    rcOutput.right = rcOutput.left + MAX (fg_rect.w, bg_rect.w);
    rcOutput.bottom = rcOutput.top + MAX (fg_rect.h, bg_rect.h);
    
    if (!IntersectRect (&rcOutput, &rcOutput, &pdc->rc_output))
        return;

    rcTemp = pdc->rc_output;
    pdc->rc_output = rcOutput;
    ENTER_DRAWING (pdc);

    pdc->step = 1;
    pdc->cur_ban = NULL;

    if (bg_rect.x != fg_rect.x || bg_rect.y != fg_rect.y
            || bg_rect.w != fg_rect.w || bg_rect.h != fg_rect.h) {
        pdc->cur_pixel = pdc->bkcolor;
        _dc_fillbox_clip (pdc, &bg_rect);
    }

    bpp = GAL_BytesPerPixel (pdc->surface);
    prepare_bitmap (pdc, (bbox_w + bold + italic), bbox_h);

    if (pdc->bkmode == BM_TRANSPARENT || italic != 0) {
        char_bmp.bmType = BMP_TYPE_COLORKEY;
        char_bmp.bmColorKey = pdc->bkcolor;
    }
    else
        char_bmp.bmType = BMP_TYPE_NORMAL;

    if (logfont->style & FS_WEIGHT_BOOK
            && devfont->font_ops->get_char_pixmap) {
        expand_char_pixmap (pdc, bbox_w, bbox_h, bits, char_bmp.bmBits,
                bold, italic, pitch);
    }
    else {
        expand_char_bitmap (bbox_w, bbox_h, bits, bpp,
                char_bmp.bmBits, pdc->bkcolor, pdc->textcolor, bold, italic);
    }

    pdc->cur_pixel = pdc->brushcolor;
    pdc->skip_pixel = pdc->bkcolor;
    _dc_fillbox_bmp_clip (pdc, &fg_rect, &char_bmp);

    LEAVE_DRAWING (pdc);
    pdc->rc_output = rcTemp;
}

static void draw_text_lines (PDC pdc, PLOGFONT logfont, int x1, int y1, int x2, int y2)
{
    int h = logfont->size;
    RECT eff_rc, rcOutput, rcTemp;
    CLIPRECT* cliprect;

    if (x1 == x2 && y1 == y2)
        return;

    SetRect (&rcOutput, x1, y1, x2, y2);
    NormalizeRect (&rcOutput);
    rcOutput.left -= 1;
    rcOutput.top -= h;
    rcOutput.right += 1;
    rcOutput.bottom += 1;

    rcTemp = pdc->rc_output;
    pdc->rc_output = rcOutput;

    ENTER_DRAWING (pdc);

    pdc->cur_pixel = pdc->textcolor;
    if (logfont->style & FS_UNDERLINE_LINE) {
        cliprect = pdc->ecrgn.head;
        while (cliprect) {
            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)
                        && LineClipper (&eff_rc, &x1, &y1, &x2, &y2)) {
                pdc->move_to (pdc, x1, y1);
                LineGenerator (pdc, x1, y1, x2, y2, _dc_set_pixel_noclip);
            }
            cliprect = cliprect->next;
       }
    } 

    if (logfont->style & FS_STRUCKOUT_LINE) {
        y1 -= h >> 1; y2 -= h >> 1;

        cliprect = pdc->ecrgn.head;
        while (cliprect) {
            if (IntersectRect (&eff_rc, &pdc->rc_output, &cliprect->rc)
                        && LineClipper (&eff_rc, &x1, &y1, &x2, &y2)) {
                pdc->move_to (pdc, x1, y1);
                LineGenerator (pdc, x1, y1, x2, y2, _dc_set_pixel_noclip);
            }
            cliprect = cliprect->next;
        }
    }

    LEAVE_DRAWING (pdc);
    pdc->rc_output = rcTemp;
}

/* return width of output */
int gdi_strnwrite (PDC pdc, int x, int y, const char* text, int len)
{
    DEVFONT* sbc_devfont = pdc->pLogFont->sbc_devfont;
    DEVFONT* mbc_devfont = pdc->pLogFont->mbc_devfont;
    int len_cur_char;
    int left_bytes = len;
    int origx;
    int origy;
    int sbc_height = (*sbc_devfont->font_ops->get_font_height) (pdc->pLogFont, sbc_devfont);
    int sbc_ascent = (*sbc_devfont->font_ops->get_font_ascent) (pdc->pLogFont, sbc_devfont);
    int mbc_height = 0;
    int mbc_ascent = 0;

    if (mbc_devfont) {
        mbc_height = (*mbc_devfont->font_ops->get_font_height) (pdc->pLogFont, mbc_devfont);
        mbc_ascent = (*mbc_devfont->font_ops->get_font_ascent) (pdc->pLogFont, mbc_devfont);
    }

    y += MAX (sbc_ascent, mbc_ascent); /* convert y-coordinate to baseline */
    y += pdc->alExtra;

    origx = x;
    origy = y;

    gdi_start_new_line (pdc->pLogFont);
    while (left_bytes) {
        if (mbc_devfont != NULL) {
            len_cur_char = (*mbc_devfont->charset_ops->len_first_char) (text, left_bytes);
            if (len_cur_char != 0) {
                put_one_char (pdc, pdc->pLogFont, mbc_devfont, &x, &y, 
                                    mbc_height, mbc_ascent, text, len_cur_char);
                
                left_bytes -= len_cur_char;
                text += len_cur_char;
                x += pdc->cExtra;
                continue;
            }
        }

        len_cur_char = (*sbc_devfont->charset_ops->len_first_char) (text, left_bytes);
        if (len_cur_char != 0)
            put_one_char (pdc, pdc->pLogFont, sbc_devfont, &x, &y, 
                                    sbc_height, sbc_ascent, text, len_cur_char);
        else
            break;

        left_bytes -= len_cur_char;
        text += len_cur_char;
        x += pdc->cExtra;
    }

    draw_text_lines (pdc, pdc->pLogFont, origx, origy, x, y);

    return x - origx;
}

int gdi_tabbedtextout (PDC pdc, int x, int y, const char* text, int len)
{
    DEVFONT* sbc_devfont = pdc->pLogFont->sbc_devfont;
    DEVFONT* mbc_devfont = pdc->pLogFont->mbc_devfont;
    int len_cur_char;
    int left_bytes = len;
    int origx, origy;
    int x_start = x, max_x = x;
    int tab_width, line_height;

    int sbc_height = (*sbc_devfont->font_ops->get_font_height) (pdc->pLogFont, sbc_devfont);
    int sbc_ascent = (*sbc_devfont->font_ops->get_font_ascent) (pdc->pLogFont, sbc_devfont);
    int mbc_height = 0;
    int mbc_ascent = 0;

    if (mbc_devfont) {
        mbc_height = (*mbc_devfont->font_ops->get_font_height) (pdc->pLogFont, mbc_devfont);
        mbc_ascent = (*mbc_devfont->font_ops->get_font_ascent) (pdc->pLogFont, mbc_devfont);
    }

    y += MAX (sbc_ascent, mbc_ascent);
    y += pdc->alExtra;

    origx = x; origy = y;

    tab_width = (*sbc_devfont->font_ops->get_ave_width) (pdc->pLogFont, 
                    sbc_devfont) * pdc->tabstop;
    line_height = pdc->pLogFont->size + pdc->alExtra + pdc->blExtra;

    gdi_start_new_line (pdc->pLogFont);
    while (left_bytes) {
        if (mbc_devfont != NULL) {
            len_cur_char = (*mbc_devfont->charset_ops->len_first_char) (text, left_bytes);
            if (len_cur_char != 0) {
                put_one_char (pdc, pdc->pLogFont, mbc_devfont, &x, &y, 
                                        mbc_height, mbc_ascent, text, len_cur_char);
                x += pdc->cExtra;
                left_bytes -= len_cur_char;
                text += len_cur_char;
                continue;
            }
        }

        len_cur_char = (*sbc_devfont->charset_ops->len_first_char) (text, left_bytes);
        if (len_cur_char != 0)
            switch (*text) {
            case '\n':
                y += line_height;
            case '\r':
                if (max_x < x) max_x = x;
                x = x_start;

                draw_text_lines (pdc, pdc->pLogFont, origx, origy, x, y);
                origx = x; origy = y;

                gdi_start_new_line (pdc->pLogFont);
                break;

            case '\t':
                x = x_start + ((x - x_start) / tab_width + 1) * tab_width;
                gdi_start_new_line (pdc->pLogFont);
                break;

            default:
                put_one_char (pdc, pdc->pLogFont, sbc_devfont, &x, &y, 
                                        sbc_height, sbc_ascent, text, len_cur_char);
                x += pdc->cExtra;
                break;
            }
        else
            break;

       left_bytes -= len_cur_char;
       text += len_cur_char;
    }

    draw_text_lines (pdc, pdc->pLogFont, origx, origy, x, y);

    if (max_x < x) max_x = x;
    return max_x - x_start;
}

int gdi_width_one_char (LOGFONT* logfont, DEVFONT* devfont, const char* mchar, int len, int* x, int* y)
{
    int w;
    
    if (devfont->font_ops->get_char_bbox) {
        int oldx = *x;
        (*devfont->font_ops->get_char_bbox) (logfont, devfont,
                            mchar, len, NULL, NULL, NULL, NULL);
        (*devfont->font_ops->get_char_advance) (logfont, devfont, mchar, len, x, y);
        w = *x - oldx;
    }
    else
        w = (*devfont->font_ops->get_char_width) (logfont, devfont, mchar, len);
    
    if (logfont->style & FS_WEIGHT_BOLD 
        && !(devfont->style & FS_WEIGHT_BOLD)) {
        w ++;
    }

    return w;
}

void gdi_get_TextOut_extent (PDC pdc, LOGFONT* log_font, const char* text, int len, SIZE* size)
{
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;
    int len_cur_char;
    int left_bytes = len;
    int x = 0, y = 0;

    gdi_start_new_line (log_font);

    /* FIXME: cy is not the height when rotate font */
    size->cy = log_font->size + pdc->alExtra + pdc->blExtra;
    size->cx = 0;
    while (left_bytes) {
        if (mbc_devfont != NULL) {
            len_cur_char = (*mbc_devfont->charset_ops->len_first_char) (text, left_bytes);
            if (len_cur_char != 0) {
                size->cx += gdi_width_one_char (log_font, mbc_devfont, 
                    text, len_cur_char, &x, &y);
                size->cx += pdc->cExtra;
                left_bytes -= len_cur_char;
                text += len_cur_char;
                continue;
            }
        }

        len_cur_char = (*sbc_devfont->charset_ops->len_first_char) (text, left_bytes);
        if (len_cur_char != 0) {
            size->cx += gdi_width_one_char (log_font, sbc_devfont, 
                    text, len_cur_char, &x, &y);
            size->cx += pdc->cExtra;
        }
        else
            break;

        left_bytes -= len_cur_char;
        text += len_cur_char;
    }
}

void gdi_get_TabbedTextOut_extent (PDC pdc, LOGFONT* log_font, int tab_stops,
                const char* text, int len, SIZE* size, POINT* last_pos)
{
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;
    int len_cur_char;
    int left_bytes = len;
    int tab_width, line_height;
    int last_line_width = 0;
    int x = 0, y = 0;

    gdi_start_new_line (log_font);

    size->cx = 0;
    size->cy = 0;
    tab_width = (*sbc_devfont->font_ops->get_ave_width) (log_font, sbc_devfont)
                    * tab_stops;
    line_height = log_font->size + pdc->alExtra + pdc->blExtra;

    while (left_bytes) {
        if (mbc_devfont != NULL) {
            len_cur_char = (*mbc_devfont->charset_ops->len_first_char) (text, left_bytes);
            if (len_cur_char != 0) {
                last_line_width += gdi_width_one_char (log_font, mbc_devfont, 
                    text, len_cur_char, &x, &y);
                last_line_width += pdc->cExtra;
                left_bytes -= len_cur_char;
                text += len_cur_char;
                continue;
            }
        }

        len_cur_char = (*sbc_devfont->charset_ops->len_first_char) (text, left_bytes);
        if (len_cur_char != 0)
            switch (*text) {
            case '\n':
                size->cy += line_height;
            case '\r':
                if (last_line_width > size->cx)
                    size->cx = last_line_width;
                last_line_width = 0;
                gdi_start_new_line (log_font);
            break;

            case '\t':
                last_line_width = (last_line_width / tab_width + 1) * tab_width;
                gdi_start_new_line (log_font);
            break;

            default:
                last_line_width += gdi_width_one_char (log_font, sbc_devfont, 
                            text, len_cur_char, &x, &y);
                last_line_width += pdc->cExtra;
            }
        else
            break;

        left_bytes -= len_cur_char;
        text += len_cur_char;
    }

    if (last_line_width > size->cx)
        size->cx = last_line_width;

    if (last_pos) {
        last_pos->x += last_line_width;
        last_pos->y += size->cy - line_height;
    }
}

