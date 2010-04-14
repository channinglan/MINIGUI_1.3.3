/*
** $Id: text.c,v 1.31 2003/09/04 03:09:52 weiym Exp $
**
** The Text Support of GDI.
**
** Copyright (C) 2000 ~ 2002 Wei Yongming.
** Copyright (C) 2003 Feynman Software.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000.4.19
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
#include "drawtext.h"
#include "cursor.h"

extern BOOL dc_GenerateECRgn (PDC pdc, BOOL fForce);

int GUIAPI GetFontHeight (HDC hdc)
{
    PDC pdc = dc_HDC2PDC(hdc);

    return pdc->pLogFont->size;
}

int GUIAPI GetMaxFontWidth (HDC hdc)
{
    PDC pdc = dc_HDC2PDC(hdc);
    DEVFONT* sbc_devfont = pdc->pLogFont->sbc_devfont;
    DEVFONT* mbc_devfont = pdc->pLogFont->mbc_devfont;
    int sbc_max_width = (*sbc_devfont->font_ops->get_max_width) (pdc->pLogFont, sbc_devfont);
    int mbc_max_width = 0;

    if (mbc_devfont)
        mbc_max_width = (*mbc_devfont->font_ops->get_max_width) (pdc->pLogFont, mbc_devfont);
    
    return (sbc_max_width > mbc_max_width) ? sbc_max_width : mbc_max_width;
}

void GUIAPI GetTextExtent (HDC hdc, const char* spText, int len, SIZE* pSize)
{
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);
    if (len < 0) len = strlen (spText);

    gdi_get_TextOut_extent (pdc, pdc->pLogFont, spText, len, pSize);
}

void GUIAPI GetTabbedTextExtent (HDC hdc, const char* spText, int len, SIZE* pSize)
{
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);
    if (len < 0) len = strlen (spText);

    gdi_get_TabbedTextOut_extent (pdc, pdc->pLogFont, pdc->tabstop, 
                spText, len, pSize, NULL);
}

void GUIAPI GetLastTextOutPos (HDC hdc, POINT* pt)
{
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);
    *pt = pdc->CurTextPos;
}

int GUIAPI TextOutLen (HDC hdc, int x, int y, const char* spText, int len)
{
    PCLIPRECT pClipRect;
    PDC pdc;
    RECT rcOutput;
    SIZE size;

    if (len == 0) return 0;
    if (len < 0) len = strlen (spText);

    pdc = dc_HDC2PDC(hdc);

    gdi_get_TextOut_extent (pdc, pdc->pLogFont, spText, len, &size);
    {
        // update text out position
        int width = size.cx;

        extent_x_SP2LP (pdc, &width);
        pdc->CurTextPos.x = x + width;
        pdc->CurTextPos.y = y;
    }

    if (dc_IsGeneralHDC(hdc)) {
        pthread_mutex_lock (&pdc->pGCRInfo->lock);
        if (!dc_GenerateECRgn (pdc, FALSE)) {
            pthread_mutex_unlock (&pdc->pGCRInfo->lock);
            return size.cx;
        }
    }

    // Transfer logical to device to screen here.
    coor_LP2SP(pdc, &x, &y);

    rcOutput.left = x;
    rcOutput.top  = y;
    rcOutput.right = x + size.cx + 1;
    rcOutput.bottom = y + size.cy + 1;
    NormalizeRect(&rcOutput);

    pthread_mutex_lock (&__mg_gdilock);
    IntersectRect (&rcOutput, &rcOutput, &pdc->ecrgn.rcBound);
    if( !dc_IsMemHDC(hdc) ) ShowCursorForGDI(FALSE, &rcOutput);

    // set graphics context.
    GAL_SetGC(pdc->gc);

    // set text out mode.
    pClipRect = pdc->ecrgn.head;
    while(pClipRect)
    {
        if (DoesIntersect (&rcOutput, &pClipRect->rc)) {
            GAL_SetClipping(pdc->gc, pClipRect->rc.left, pClipRect->rc.top,
                    pClipRect->rc.right - 1, pClipRect->rc.bottom - 1);

            gdi_strnwrite (pdc, x, y, spText, len);
        }
            
        pClipRect = pClipRect->next;
    }

    if( !dc_IsMemHDC(hdc) ) ShowCursorForGDI(TRUE, &rcOutput);
    pthread_mutex_unlock(&__mg_gdilock);
    if (dc_IsGeneralHDC(hdc)) pthread_mutex_unlock (&pdc->pGCRInfo->lock);

    return size.cx;
}

int GUIAPI TabbedTextOutLen (HDC hdc, int x, int y, const char* spText, int len) 
{
    PCLIPRECT pClipRect;
    PDC pdc;
    SIZE size;
    RECT rcOutput;

    if (len == 0) return 0;
    if (len < 0) len = strlen (spText);

    pdc = dc_HDC2PDC(hdc);

    coor_LP2SP (pdc, &pdc->CurTextPos.x, &pdc->CurTextPos.y);
    gdi_get_TabbedTextOut_extent (pdc, pdc->pLogFont, pdc->tabstop, spText, len, 
                &size, &pdc->CurTextPos);
    coor_SP2LP (pdc, &pdc->CurTextPos.x, &pdc->CurTextPos.y);

    if (dc_IsGeneralHDC(hdc)) {
        pthread_mutex_lock (&pdc->pGCRInfo->lock);
        if (!dc_GenerateECRgn (pdc, FALSE)) {
            pthread_mutex_unlock (&pdc->pGCRInfo->lock);
            return size.cx;
        }
    }

    // Transfer logical to device to screen here.
    coor_LP2SP(pdc, &x, &y);
    
    rcOutput.left = x;
    rcOutput.top  = y;
    rcOutput.right = x + size.cx + 1;
    rcOutput.bottom = y + size.cy + 1;
    NormalizeRect(&rcOutput);

    pthread_mutex_lock (&__mg_gdilock);
    IntersectRect (&rcOutput, &rcOutput, &pdc->ecrgn.rcBound);
    if( !dc_IsMemHDC(hdc) ) ShowCursorForGDI(FALSE, &rcOutput);

    // set graphics context.
    GAL_SetGC(pdc->gc);

    pClipRect = pdc->ecrgn.head;
    while(pClipRect)
    {
        if (DoesIntersect (&rcOutput, &pClipRect->rc)) {
            GAL_SetClipping(pdc->gc, pClipRect->rc.left, pClipRect->rc.top,
                    pClipRect->rc.right - 1, pClipRect->rc.bottom - 1);

            gdi_tabbedtextout (pdc, x, y, spText, len);
        }
            
        pClipRect = pClipRect->next;
    }

    if( !dc_IsMemHDC(hdc) ) ShowCursorForGDI(TRUE, &rcOutput);
    pthread_mutex_unlock(&__mg_gdilock);
    if (dc_IsGeneralHDC(hdc)) pthread_mutex_unlock (&pdc->pGCRInfo->lock);

    return size.cx;
}

char* strnchr (const char* s, size_t n, int c)
{
    size_t i;
    
    for (i=0; i<n; i++) {
        if ( *s == c)
            return (char *)s;

        s ++;
    }

    return NULL;
}

int substrlen (const char* text, int len, char delimiter, int* nr_delim)
{
    char* substr;

    *nr_delim = 0;

    if ( (substr = strnchr (text, len, delimiter)) == NULL)
        return len;

    len = substr - text;

    while (*substr == delimiter) {
        (*nr_delim) ++;
        substr ++;
    }

    return len;
}

int GUIAPI TabbedTextOutEx (HDC hdc, int x, int y, const char* spText,
		int nCount, int nTabs, int *pTabPos, int nTabOrig)
{
    PDC pdc;
    int line_len, sub_len;
    int nr_tab = 0, tab_pos, def_tab;
    int x_orig = x, max_x = x;
    int line_height;
    int nr_delim_newline, nr_delim_tab;

    if (nCount == 0) return 0;
    if (nCount < 0) nCount = strlen (spText);

    pdc = dc_HDC2PDC(hdc);

    line_height = pdc->pLogFont->size + pdc->alExtra + pdc->blExtra;
    y += pdc->alExtra;
    if (nTabs == 0 || pTabPos == NULL) {
        int ave_width = (*pdc->pLogFont->sbc_devfont->font_ops->get_ave_width)
                        (pdc->pLogFont, pdc->pLogFont->sbc_devfont);
        def_tab = ave_width * pdc->tabstop;
    }
    else
        def_tab = pTabPos [nTabs - 1];

    while (nCount) {
        line_len = substrlen (spText, nCount, '\n', &nr_delim_newline);

        nCount -= line_len + nr_delim_newline;

        nr_tab = 0;
        x = x_orig;
        tab_pos = nTabOrig;
        while (line_len) {
            int i, width;

            sub_len = substrlen (spText, line_len, '\t', &nr_delim_tab);

            width = TextOutLen (hdc, x, y, spText, sub_len);

            x += width; 
            if (x >= tab_pos) {
                while (x >= tab_pos)
                    tab_pos += (nr_tab >= nTabs) ? def_tab : pTabPos [nr_tab++];
                for (i = 0; i < nr_delim_tab - 1; i ++)
                    tab_pos += (nr_tab >= nTabs) ? def_tab : pTabPos [nr_tab++];
            }
            else {
                for (i = 0; i < nr_delim_tab; i ++)
                    tab_pos += (nr_tab >= nTabs) ? def_tab : pTabPos [nr_tab++];
            }

            x = tab_pos;

            line_len -= sub_len + nr_delim_tab;
            spText += sub_len + nr_delim_tab;
        }

        if (max_x < x) max_x = x;

        spText += nr_delim_newline;
        y += line_height * nr_delim_newline;
    }

    return max_x - x_orig;
}

static void txtDrawOneLine (PDC pdc, const char* pText, int nLen, int x, int y,
                    const RECT* prcOutput, UINT nFormat, int nTabWidth)
{
    RECT rcInter;
    PCLIPRECT pClipRect;

    pClipRect = pdc->ecrgn.head;
    while(pClipRect)
    {
        if (IntersectRect (&rcInter, prcOutput, &pClipRect->rc)) {
            GAL_SetClipping(pdc->gc, rcInter.left, rcInter.top,
                    rcInter.right - 1, rcInter.bottom - 1);

            if (nFormat & DT_EXPANDTABS) {
                const char* sub = pText;
                const char* left;
                int nSubLen = nLen;
                int nOutputLen;
                
                while ((left = strnchr (sub, nSubLen, '\t'))) {
                    
                    nOutputLen = left - sub;
                    x += gdi_strnwrite (pdc, x, y, sub, nOutputLen);
                    
                    nSubLen -= (nOutputLen + 1);
                    sub = left + 1;
                    x += nTabWidth;
                }

                if (nSubLen != 0)
                    gdi_strnwrite (pdc, x, y, sub, nSubLen);
            }
            else
                gdi_strnwrite (pdc, x, y, pText, nLen);
        }
            
        pClipRect = pClipRect->next;
    }
}

static int txtGetWidthOfNextWord (PDC pdc, const char* pText, int nCount, int* nChars)
{
    int width;
    DEVFONT* sbc_devfont = pdc->pLogFont->sbc_devfont;
    DEVFONT* mbc_devfont = pdc->pLogFont->mbc_devfont;
    WORDINFO word_info; 

    *nChars = 0;
    if (nCount == 0) return 0;

    if (mbc_devfont) {
        int mbc_pos, sub_len;

        mbc_pos = (*mbc_devfont->charset_ops->pos_first_char) (pText, nCount);
        if (mbc_pos == 0) {
            sub_len = (*mbc_devfont->charset_ops->len_first_substr) (pText, nCount);

            (*mbc_devfont->charset_ops->get_next_word) (pText, sub_len, &word_info);
            width = (*mbc_devfont->font_ops->get_str_width) 
                        (pdc->pLogFont, mbc_devfont, pText, word_info.len, pdc->cExtra);

            *nChars = word_info.len;
            return width;
        }
        else if (mbc_pos > 0)
            nCount = mbc_pos;
    }

    (*sbc_devfont->charset_ops->get_next_word) (pText, nCount, &word_info);
    width = (*sbc_devfont->font_ops->get_str_width) 
                    (pdc->pLogFont, sbc_devfont, pText, word_info.len, pdc->cExtra);
    *nChars = word_info.len;

#if 0
    fprintf (stderr, "text: %s, width: %d, word len: %d\n", pText, width, word_info.len);
#endif

    return width;
}

// This function return the normal characters' number (refrence)
// and tab's number per line (return value).
static int txtGetOneLine (PDC pdc, const char* pText, int nCount, int nTabWidth, 
        int maxwidth, UINT uFormat, int* nChar)
{
    int tabs = 0;
    int wordLen;
    int lineWidth = 0;
    int wordWidth;
    int x = 0, y = 0;

    if (uFormat & DT_SINGLELINE) {
        *nChar = nCount;
        return 0;
    }

    *nChar = 0;
    while (TRUE) {
        wordWidth = txtGetWidthOfNextWord (pdc, pText, nCount, &wordLen);

        if ((wordWidth < maxwidth)
                && ((lineWidth + wordWidth) > maxwidth) 
                && (uFormat & DT_WORDBREAK))
            break;

        pText += wordLen;
        nCount -= wordLen;
        lineWidth += wordWidth;
        *nChar += wordLen;

        if (nCount == 0)
            break;

        if (*pText == '\t') {
            tabs ++;
            if (uFormat & DT_EXPANDTABS) {
                lineWidth += nTabWidth;
                gdi_start_new_line (pdc->pLogFont);
            }
            else
                lineWidth += gdi_width_one_char 
                        (pdc->pLogFont, pdc->pLogFont->sbc_devfont, pText, 1, &x, &y);
            pText ++;
            nCount --;
        }
        else if (*pText == '\n' || *pText == '\r') {
            (*nChar) ++;
            break;
        }
        else if (*pText == ' ') {
            lineWidth += gdi_width_one_char
                        (pdc->pLogFont, pdc->pLogFont->sbc_devfont, pText, 1, &x, &y);
            (*nChar) ++;
            pText ++;
            nCount --;
        }
    }
    
    if (!(uFormat & DT_EXPANDTABS)) {
        *nChar += tabs;
        tabs = 0;
    }

    return tabs;
}

int DrawTextEx (HDC hdc, const char* pText, int nCount, 
                RECT* pRect, int indent, UINT nFormat)
{
    PDC pdc;
    int n, nLines = 0, width = 0;
    BOOL bOutput = TRUE;
    int x, y;
    RECT rcDraw, rcOutput;
    int nTabWidth, tabs; 
    SIZE size;
    int line_height;

    pdc = dc_HDC2PDC(hdc);

    if (nCount == -1)
        nCount = strlen (pText);

    line_height = pdc->pLogFont->size + pdc->alExtra + pdc->blExtra;

    if (nFormat & DT_TABSTOP)
        nTabWidth = HIWORD (nFormat) * 
                    (*pdc->pLogFont->sbc_devfont->font_ops->get_ave_width)
                    (pdc->pLogFont, pdc->pLogFont->sbc_devfont);

    else
        nTabWidth = pdc->tabstop * 
                    (*pdc->pLogFont->sbc_devfont->font_ops->get_ave_width)
                    (pdc->pLogFont, pdc->pLogFont->sbc_devfont);

    // Transfer logical to device to screen here.
    rcDraw = *pRect;
    coor_LP2SP(pdc, &rcDraw.left, &rcDraw.top);
    coor_LP2SP(pdc, &rcDraw.right, &rcDraw.bottom);
    NormalizeRect (&rcDraw);

    if (dc_IsGeneralHDC(hdc)) {
        pthread_mutex_lock (&pdc->pGCRInfo->lock);
        if (!dc_GenerateECRgn (pdc, FALSE))
            bOutput = FALSE;
    }

    pthread_mutex_lock (&__mg_gdilock);

    // set graphics context.
    GAL_SetGC (pdc->gc);

    // Draw text here.
    if (nFormat & DT_CALCRECT)
        bOutput = FALSE;

    y = rcDraw.top;
    if (nFormat & DT_SINGLELINE) {
        if (nFormat & DT_BOTTOM)
            y = rcDraw.bottom - pdc->pLogFont->size;
        else if (nFormat & DT_VCENTER)
            y = rcDraw.top + ((RECTH (rcDraw) - pdc->pLogFont->size) >> 1);
    }

    while (nCount != 0) {
        int nOutput;
        int maxwidth;

        if (nLines == 0) {
            maxwidth = rcDraw.right - rcDraw.left - indent;
            if (maxwidth <= 0) {
                // new line
                y += pdc->pLogFont->size;
                nLines ++;
                continue;
            }
        }
        else
            maxwidth = rcDraw.right - rcDraw.left;
        
        gdi_start_new_line (pdc->pLogFont);
        tabs = txtGetOneLine (pdc, pText, nCount, nTabWidth, maxwidth, nFormat, &n);

        gdi_get_TextOut_extent (pdc, pdc->pLogFont, pText, n, &size);
        width = size.cx + tabs * nTabWidth;
        n += tabs;

        if ( (pText[n-1] == '\n' || pText[n-1] == '\r') 
             && !(nFormat & DT_SINGLELINE) ) {
            int tmpx = 0, tmpy = 0;

            nOutput = n - 1;
            width -= gdi_width_one_char (pdc->pLogFont, pdc->pLogFont->sbc_devfont, 
                            pText + n - 1, 1, &tmpx, &tmpy);
        }
        else
            nOutput = n;
            
        if (nFormat & DT_RIGHT)
            x = rcDraw.right - width;
        else if (nFormat & DT_CENTER)
            x = rcDraw.left + ((RECTW (rcDraw) - width) >> 1);
        else
            x = rcDraw.left;
        x += (nLines ? 0 : indent);

        rcOutput.left   = x;
        rcOutput.top    = y;
        rcOutput.right  = rcOutput.left + width;
        rcOutput.bottom = rcOutput.top + line_height;
        NormalizeRect(&rcOutput);

        if (nFormat & DT_CALCRECT) {
            if (nLines == 0)
                *pRect = rcOutput;
            else
                GetBoundRect (pRect, pRect, &rcOutput);
        }

        // draw one line
        if (bOutput && width > 0) {
            if (!dc_IsMemHDC(hdc)) ShowCursorForGDI(FALSE, &rcOutput);

            if (nFormat & DT_NOCLIP)
                txtDrawOneLine (pdc, pText, nOutput, x, y, 
                        &rcOutput, nFormat, nTabWidth);
            else {
                RECT rcClip;
                IntersectRect (&rcClip, &rcOutput, &rcDraw);
                txtDrawOneLine (pdc, pText, nOutput, x, y, 
                        &rcClip, nFormat, nTabWidth);
            }
        
            if (!dc_IsMemHDC(hdc)) ShowCursorForGDI (TRUE, &rcOutput);
        }

        pText += n;

        // new line
        y += line_height;
        nLines ++;

        // left characters
        nCount = nCount - n;
    }

    pthread_mutex_unlock (&__mg_gdilock);
    if (dc_IsGeneralHDC(hdc)) pthread_mutex_unlock (&pdc->pGCRInfo->lock);

    if (nFormat & DT_CALCRECT) {
        coor_SP2LP (pdc, &pRect->left, &pRect->top);
        coor_SP2LP (pdc, &pRect->right, &pRect->bottom);
    }

    if (!(nFormat & DT_CALCRECT)) {
        // update text out position
        x += width;
        y -= line_height;
        coor_SP2LP (pdc, &x, &y);
        pdc->CurTextPos.x = x;
        pdc->CurTextPos.y = y;
    }

    return line_height * nLines;
}

/************************* Text parse support ********************************/
int GUIAPI GetTextMCharInfo (PLOGFONT log_font, const char* mstr, int len, 
                int* pos_chars)
{
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;
    int count = 0;
    int left_bytes = len;
    int len_cur_char;

    while (left_bytes) {
        if (pos_chars)
            pos_chars [count] = len - left_bytes;

        if (mbc_devfont) {
            len_cur_char = (*mbc_devfont->charset_ops->len_first_char) (mstr, left_bytes);
            if (len_cur_char != 0) {
                count ++;
                left_bytes -= len_cur_char;
                mstr += len_cur_char;
                continue;
            }
        }

        len_cur_char = (*sbc_devfont->charset_ops->len_first_char) (mstr, left_bytes);
        if (len_cur_char != 0) {
            count ++;
            left_bytes -= len_cur_char;
            mstr += len_cur_char;
        }
    }

    return count;
}

int GUIAPI GetTextWordInfo (PLOGFONT log_font, const char* mstr, int len, 
                int* pos_words, WORDINFO* info_words)
{
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;
    WORDINFO word_info; 
    int count = 0;
    int left_bytes = len;
    int mbc_sub_len, sbc_sub_len, word_len;

    while (left_bytes) {
        sbc_sub_len = left_bytes;

        if (mbc_devfont) {
            int mbc_pos;

            mbc_pos = (*mbc_devfont->charset_ops->pos_first_char) (mstr, left_bytes);
            if (mbc_pos == 0) {
                mbc_sub_len = (*mbc_devfont->charset_ops->len_first_substr) (mstr, left_bytes);

                while (mbc_sub_len) {
                    (*mbc_devfont->charset_ops->get_next_word) (mstr, mbc_sub_len, &word_info);

                    if (pos_words)
                        pos_words [count] = len - left_bytes;
                    if (info_words)
                        memcpy (info_words + count, &word_info, sizeof (WORDINFO));

                    count ++;
                    word_len = word_info.len + word_info.nr_delimiters;
                    mbc_sub_len -= word_len;
                    left_bytes -= word_len;
                    mstr += word_len;
                }

                continue;
            }
            else if (mbc_pos > 0)
                sbc_sub_len = mbc_pos;
        }

        while (sbc_sub_len) {
            (*sbc_devfont->charset_ops->get_next_word) (mstr, sbc_sub_len, &word_info);
            if (pos_words)
                pos_words [count] = len - left_bytes;
            if (info_words)
                memcpy (info_words + count, &word_info, sizeof (WORDINFO));

            count ++;
            word_len = word_info.len + word_info.nr_delimiters;
            sbc_sub_len -= word_len;
            left_bytes -= word_len;
            mstr += word_len;
        }
    }

    return count;
}

int GUIAPI GetFirstMCharLen (PLOGFONT log_font, const char* mstr, int len)
{
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;
    int len_cur_char = 0;

    if (mbc_devfont) {
        len_cur_char = (*mbc_devfont->charset_ops->len_first_char) (mstr, len);
        if (len_cur_char != 0)
            return len_cur_char;
    }

    len_cur_char = (*sbc_devfont->charset_ops->len_first_char) (mstr, len);
    if (len_cur_char != 0)
        return len_cur_char;

    return len_cur_char;
}

int GUIAPI GetFirstWord (PLOGFONT log_font, const char* mstr, int len,
                    WORDINFO* word_info)
{
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;

    if (mbc_devfont) {
        int mbc_pos;

        mbc_pos = (*mbc_devfont->charset_ops->pos_first_char) (mstr, len);
        if (mbc_pos == 0) {
            len = (*mbc_devfont->charset_ops->len_first_substr) (mstr, len);

            (*mbc_devfont->charset_ops->get_next_word) (mstr, len, word_info);
            return word_info->len + word_info->nr_delimiters;
        }
        else if (mbc_pos > 0)
            len = mbc_pos;
    }

    (*sbc_devfont->charset_ops->get_next_word) (mstr, len, word_info);
    return word_info->len + word_info->nr_delimiters;
}

int GUIAPI GetTextExtentPoint (HDC hdc, const char* text, int len, int max_extent, 
                int* fit_chars, int* pos_chars, int* dx_chars, SIZE* size)
{
    PDC pdc = dc_HDC2PDC (hdc);
    LOGFONT* log_font = pdc->pLogFont;
    DEVFONT* sbc_devfont = log_font->sbc_devfont;
    DEVFONT* mbc_devfont = log_font->mbc_devfont;
    int len_cur_char, width_cur_char;
    int left_bytes = len;
    int char_count = 0;
    int x = 0, y = 0;

    gdi_start_new_line (log_font);

    size->cy = log_font->size + pdc->alExtra + pdc->blExtra;
    size->cx = 0;
    while (left_bytes) {
        if (pos_chars)
            pos_chars [char_count] = len - left_bytes;
        if (dx_chars)
            dx_chars [char_count] = size->cx;

        if (mbc_devfont && 
                (len_cur_char = (*mbc_devfont->charset_ops->len_first_char) 
                    (text, left_bytes))) {
            width_cur_char = gdi_width_one_char (log_font, mbc_devfont,
                    text, len_cur_char, &x, &y);
        }
        else {
            if ((len_cur_char = (*sbc_devfont->charset_ops->len_first_char)
                    (text, left_bytes)))
                width_cur_char = gdi_width_one_char (log_font, sbc_devfont,
                    text, len_cur_char, &x, &y);
            else
                break;
        }

        width_cur_char += pdc->cExtra;

        if (max_extent > 0 && (size->cx + width_cur_char) > max_extent) {
            goto ret;
        }

        char_count ++;
        size->cx += width_cur_char;
        left_bytes -= len_cur_char;
        text += len_cur_char;
    }

ret:
    if (fit_chars) *fit_chars = char_count;
    return len - left_bytes;
}

