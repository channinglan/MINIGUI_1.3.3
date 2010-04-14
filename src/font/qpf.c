/*
** $Id: qpf.c,v 1.14 2003/09/25 04:08:55 snig Exp $
** 
** qpf.c: The Qt Prerendered Font operation set.
**
** Copyright (C) 2003, Feynman Software.
**
** Create date: 2003/01/28
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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "misc.h"

#ifdef _QPF_SUPPORT

#ifdef HAVE_MMAP
    #include <sys/mman.h>
#endif

#include "charset.h"
#include "devfont.h"
#include "qpf.h"
#include "fontname.h"

/********************** Load/Unload of QPF font ***********************/

typedef unsigned char uchar;

static void readNode (GLYPHTREE* tree, uchar** data)
{
    uchar rw, cl;
    int flags, n;

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    tree->min = (rw << 8) | cl;

    rw = **data; (*data)++;
    cl = **data; (*data)++;
    tree->max = (rw << 8) | cl;

    flags = **data; (*data)++;
    if (flags & 1)
        tree->less = calloc (1, sizeof (GLYPHTREE));
    else
        tree->less = NULL;

    if (flags & 2)
        tree->more = calloc (1, sizeof (GLYPHTREE));
    else
        tree->more = NULL;

    n = tree->max - tree->min + 1;
    tree->glyph = calloc (n, sizeof (GLYPH));

    if (tree->less)
        readNode (tree->less, data);
    if (tree->more)
        readNode (tree->more, data);
}

static void readMetrics (GLYPHTREE* tree, uchar** data)
{
    int i;
    int n = tree->max - tree->min + 1;

    for (i = 0; i < n; i++) {
        tree->glyph[i].metrics = (GLYPHMETRICS*) *data;

        *data += sizeof (GLYPHMETRICS);
    }

    if (tree->less)
        readMetrics (tree->less, data);
    if (tree->more)
        readMetrics (tree->more, data);
}

static void readData (GLYPHTREE* tree, uchar** data)
{
    int i;
    int n = tree->max - tree->min + 1;

    for (i = 0; i < n; i++) {
        int datasize;

        datasize = tree->glyph[i].metrics->linestep * tree->glyph[i].metrics->height;
        tree->glyph[i].data = *data; *data += datasize;
    }

    if (tree->less)
        readData (tree->less, data);
    if (tree->more)
        readData (tree->more, data);
}

static void BuildGlyphTree (GLYPHTREE* tree, uchar** data)
{
    readNode (tree, data);
    readMetrics (tree, data);
    readData (tree, data);
}

BOOL LoadQPFont (const char* file, QPFINFO* QPFInfo)
{
    int fd;
    struct stat st;
    uchar* data;

    if ((fd = open (file, O_RDONLY)) < 0) return FALSE;
    if (fstat (fd, &st)) return FALSE;
    QPFInfo->file_size = st.st_size;

#ifdef HAVE_MMAP

    QPFInfo->fm = (QPFMETRICS*) mmap( 0, st.st_size,
                    PROT_READ, MAP_PRIVATE, fd, 0 );

    if (!QPFInfo->fm || QPFInfo->fm == (QPFMETRICS *)MAP_FAILED)
        goto error;

#else

    QPFInfo->fm = calloc (1, st.st_size);
    if (QPFInfo->fm == NULL)
        goto error;

    read (fd, QPFInfo->fm, st.st_size);

#endif

    QPFInfo->tree = calloc (1, sizeof (GLYPHTREE));

    data = (uchar*)QPFInfo->fm;
    data += sizeof (QPFMETRICS);
    BuildGlyphTree (QPFInfo->tree, &data);

#if 0
    if (QPFInfo->fm->flags & FM_SMOOTH) {
        QPFInfo->max_bmp_size = QPFInfo->fm->maxwidth
                * (QPFInfo->fm->ascent + QPFInfo->fm->descent);
    }
    else {
        QPFInfo->max_bmp_size = ((QPFInfo->fm->maxwidth + 7) >> 3) 
                * (QPFInfo->fm->ascent + QPFInfo->fm->descent);
    }

    QPFInfo->std_bmp = calloc (1, QPFInfo->max_bmp_size);
#endif

    close (fd);
    return TRUE;

error:
    close (fd);
    return FALSE;
}

static void ClearGlyphTree (GLYPHTREE* tree)
{
    if (tree->less) {
        ClearGlyphTree (tree->less);
    }
    if (tree->more) {
        ClearGlyphTree (tree->more);
    }

    free (tree->glyph);
    free (tree->less);
    free (tree->more);
}

void UnloadQPFont (QPFINFO* QPFInfo)
{
    if (QPFInfo->file_size == 0)
        return;

    ClearGlyphTree (QPFInfo->tree);
    free (QPFInfo->tree);
#if 0
    free (QPFInfo->std_bmp);
#endif

#ifdef HAVE_MMAP
    munmap (QPFInfo->fm, QPFInfo->file_size);
#else
    free (QPFInfo->fm);
#endif
}

/********************** Init/Term of QPF font ***********************/
static int nr_fonts;
static QPFINFO* qpf_infos;
static DEVFONT* qpf_dev_fonts;

#define SECTION_NAME "qpf"

BOOL InitQPFonts (void)
{
    int i;
    char font_name [LEN_UNIDEVFONT_NAME + 1];

    if (GetMgEtcIntValue (SECTION_NAME, "font_number", 
                           &nr_fonts) < 0 )
        return FALSE;
    if ( nr_fonts < 1) return TRUE;

    qpf_infos = calloc (nr_fonts, sizeof (QPFINFO));
    qpf_dev_fonts = calloc (nr_fonts, sizeof (DEVFONT));

    if (qpf_infos == NULL || qpf_dev_fonts == NULL) {
        free (qpf_infos);
        free (qpf_dev_fonts);
        return FALSE;
    }

    for (i = 0; i < nr_fonts; i++) {
        char key [11];
        char charset [LEN_FONT_NAME + 1];
        char file [MAX_PATH + 1];
        CHARSETOPS* charset_ops;

        sprintf (key, "name%d", i);
        if (GetMgEtcValue (SECTION_NAME, key, 
                           font_name, LEN_UNIDEVFONT_NAME) < 0 )
            goto error_load;

        if (!fontGetCharsetFromName (font_name, charset)) {
            fprintf (stderr, "GDI: Invalid font name (charset): %s.\n", 
                    font_name);
            goto error_load;
        }

        if ((charset_ops 
               = GetCharsetOps (charset)) == NULL) {
            fprintf (stderr, "GDI: Not supported charset: %s.\n", charset);
            goto error_load;
        }

        if ((qpf_infos [i].height = fontGetHeightFromName (font_name)) == -1) {
            fprintf (stderr, "GDI: Invalid font name (height): %s.\n",
                    font_name);
            goto error_load;
        }
        
        if ((qpf_infos [i].width = fontGetWidthFromName (font_name)) == -1) {
            fprintf (stderr, "GDI: Invalid font name (width): %s.\n",
                    font_name);
            goto error_load;
        }
        
        sprintf (key, "fontfile%d", i);
        if (GetMgEtcValue (SECTION_NAME, key, 
                           file, MAX_PATH) < 0)
            goto error_load;

        if (!LoadQPFont (file, qpf_infos + i))
            goto error_load;

        strncpy (qpf_dev_fonts[i].name, font_name, LEN_UNIDEVFONT_NAME);
        qpf_dev_fonts[i].name [LEN_UNIDEVFONT_NAME] = '\0';
        qpf_dev_fonts[i].font_ops = &qpf_font_ops;
        qpf_dev_fonts[i].charset_ops = charset_ops;
        qpf_dev_fonts[i].data = qpf_infos + i;

    }

    for (i = 0; i < nr_fonts; i++) {
        int nr_charsets;
        char charsets [LEN_UNIDEVFONT_NAME + 1];

        if (qpf_dev_fonts [i].charset_ops->bytes_maxlen_char > 1)
            AddMBDevFont (qpf_dev_fonts + i);
        else
            AddSBDevFont (qpf_dev_fonts + i);

        fontGetCharsetPartFromName (qpf_dev_fonts[i].name, charsets);
        if ((nr_charsets = charsetGetCharsetsNumber (charsets)) > 1) {

            int j;

            for (j = 1; j < nr_charsets; j++) {
                char charset [LEN_FONT_NAME + 1];
                CHARSETOPS* charset_ops;
                DEVFONT* new_devfont;

                charsetGetSpecificCharset (charsets, j, charset);
                if ((charset_ops = GetCharsetOps (charset)) == NULL)
                    continue;

                new_devfont = calloc (1, sizeof (DEVFONT));
                memcpy (new_devfont, qpf_dev_fonts + i, sizeof (DEVFONT));
                new_devfont->charset_ops = charset_ops;
                if (new_devfont->charset_ops->bytes_maxlen_char > 1)
                    AddMBDevFont (new_devfont);
                else
                    AddSBDevFont (new_devfont);
            }
        }
    }

    return TRUE;

error_load:
    fprintf (stderr, "GDI: Error in loading QPF fonts!\n");
    for (i = 0; i < nr_fonts; i++)
        UnloadQPFont (qpf_infos + i);
    
    free (qpf_infos);
    free (qpf_dev_fonts);
    qpf_infos = NULL;
    qpf_dev_fonts = NULL;
    return FALSE;
}

void TermQPFonts (void)
{
    int i;

    for (i = 0; i < nr_fonts; i++)
        UnloadQPFont (qpf_infos + i);
    
    free (qpf_infos);
    free (qpf_dev_fonts);

    qpf_infos = NULL;
    qpf_dev_fonts = NULL;
}

/*************** QPF font operations *********************************/
static GLYPH* get_glyph (GLYPHTREE* tree, unsigned int ch)
{
    if (ch < tree->min) {

        if (!tree->less)
            return NULL;

        return get_glyph (tree->less, ch);
    } else if ( ch > tree->max ) {

        if (!tree->more) {
            return NULL;
        }
        return get_glyph (tree->more, ch);
    }

    return &tree->glyph [ch - tree->min];
}

static int get_char_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    unsigned int uc16;
    GLYPH* glyph;

    uc16 = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
    glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, uc16);

    if (glyph == NULL) {
        glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, '?');
    }

    return glyph->metrics->width;
}

static int get_str_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mstr, int len, int cExtra)
{
    unsigned short uc16;
    GLYPH* glyph;
    int x = 0;

    while (len > 0) {
        int char_len = (*devfont->charset_ops->len_first_char) (mstr, len);
        uc16 = (*devfont->charset_ops->conv_to_uc16) (mstr, char_len);
        len -= char_len;
        mstr += char_len;

        glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, uc16);
        if (glyph == NULL) {
            glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, '?');
        }

        x += glyph->metrics->advance;
        x += cExtra;
    }

    return x;
}

static int get_ave_width (LOGFONT* logfont, DEVFONT* devfont)
{
    return QPFONT_INFO_P (devfont)->width;
}

static int get_max_width (LOGFONT* logfont, DEVFONT* devfont)
{
    return QPFONT_INFO_P (devfont)->fm->maxwidth;
}

static int get_font_height (LOGFONT* logfont, DEVFONT* devfont)
{
    return QPFONT_INFO_P (devfont)->fm->ascent + QPFONT_INFO_P (devfont)->fm->descent;
}

static int get_font_size (LOGFONT* logfont, DEVFONT* devfont, int expect)
{
    return get_font_height (logfont, devfont);
}

static int get_font_ascent (LOGFONT* logfont, DEVFONT* devfont)
{
    return QPFONT_INFO_P (devfont)->fm->ascent;
}

static int get_font_descent (LOGFONT* logfont, DEVFONT* devfont)
{
    return QPFONT_INFO_P (devfont)->fm->descent;
}

static size_t char_bitmap_size (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    unsigned int uc16;
    GLYPH* glyph;

    uc16 = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
    glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, uc16);

    if (glyph == NULL) {
        glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, '?');
    }

    return ((glyph->metrics->width + 7 ) >> 3) * glyph->metrics->height;
}

static size_t max_bitmap_size (LOGFONT* logfont, DEVFONT* devfont)
{
    int max_width = QPFONT_INFO_P (devfont)->fm->maxwidth;
    int height = QPFONT_INFO_P (devfont)->fm->ascent
                    + QPFONT_INFO_P (devfont)->fm->descent;

    return ((max_width + 7) >> 3) * height;
}

static const void* get_char_bitmap (LOGFONT* logfont, DEVFONT* devfont,
            const unsigned char* mchar, int len)
{
    unsigned int uc16;
    GLYPH* glyph;

    uc16 = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
    glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, uc16);

    if (glyph == NULL) {
        glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, '?');
    }

    if (QPFONT_INFO_P (devfont)->fm->flags & FM_SMOOTH) {
        return NULL;
    }

    return glyph->data;

#if 0
    int i;
    int std_pitch, height;
    uchar *src, *dst;
    std_pitch = ((glyph->metrics->width + 7) >> 3);
    if (std_pitch == glyph->metrics->linestep)
        return glyph->data;

    height = glyph->metrics->height;

    if ((std_pitch * height) > QPFONT_INFO_P (devfont)->max_bmp_size) {
        return NULL;
    }

    src = glyph->data;
    dst = QPFONT_INFO_P (devfont)->std_bmp;

    for (i = 0; i < height; i++) {
        memcpy (dst, src, std_pitch);
        src += glyph->metrics->linestep;
        dst += std_pitch;
    }

    return QPFONT_INFO_P (devfont)->std_bmp;
#endif
}

static const void* get_char_pixmap (LOGFONT* logfont, DEVFONT* devfont,
            const unsigned char* mchar, int len, int* pitch)
{
    unsigned int uc16;
    GLYPH* glyph;

    uc16 = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
    glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, uc16);

    if (glyph == NULL) {
        glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, '?');
    }

    if (!(QPFONT_INFO_P (devfont)->fm->flags & FM_SMOOTH)) {
        return NULL;
    }

    *pitch = glyph->metrics->linestep;
    return glyph->data;
}

static int get_char_bbox (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len, 
                int* px, int* py, int* pwidth, int* pheight)
{
    unsigned int uc16;
    GLYPH* glyph;

    uc16 = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
    glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, uc16);

    if (glyph == NULL) {
        glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, '?');
    }

    if (pwidth) *pwidth = glyph->metrics->width;
    if (pheight) *pheight = glyph->metrics->height;

    if (px) *px += glyph->metrics->bearingx;
    if (py) *py -= glyph->metrics->bearingy;
    return glyph->metrics->width;
}

static void get_char_advance (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len, int* px, int* py)
{
    unsigned int uc16;
    GLYPH* glyph;

    uc16 = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
    glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, uc16);

    if (glyph == NULL) {
        glyph = get_glyph (QPFONT_INFO_P (devfont)->tree, '?');
    }

    *px += glyph->metrics->advance;
}

static DEVFONT* new_instance (LOGFONT* logfont, DEVFONT* devfont, BOOL need_sbc_font)
{
    if (QPFONT_INFO_P (devfont)->fm->flags & FM_SMOOTH) {
        logfont->style |= FS_WEIGHT_BOOK;
    }

    return devfont;
}

/**************************** Global data ************************************/
FONTOPS qpf_font_ops = {
    get_char_width,
    get_str_width,
    get_ave_width,
    get_max_width,
    get_font_height,
    get_font_size,
    get_font_ascent,
    get_font_descent,

    char_bitmap_size,
    max_bitmap_size,
    get_char_bitmap,
    get_char_pixmap,
    NULL,
    get_char_bbox,
    get_char_advance,
    new_instance,
    NULL
};

#endif  /* _QPF_SUPPORT */

