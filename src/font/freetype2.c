/*
** $Id: freetype2.c,v 1.7 2003/09/25 04:08:55 snig Exp $
** 
** freetype.c: TrueType font support based on FreeType 2.
**
** NOTE: this source is incompleted.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2002 Wei Yongming.
**
** Current maintainer: WEI Yongming.
**
** Create date: 2002/01/18
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
** TODO: When the freetype 2 is stabler than now...
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "misc.h"
#include "devfont.h"
#include "charset.h"
#include "fontname.h"

#if defined(_TTF_SUPPORT) && defined(_HAS_FREETYPE2)

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TRUETYPE_IDS_H
#include FT_SIZES_H

#include "freetype2.h"

/******************************* Global data ********************************/
static FT_Library ft_library;  /* The ONLY freetype 2 library engine */

/************************ Create/Destroy FreeType font ***********************/
static BOOL CreateFreeTypeFont (const char *name, FTFACEINFO* ft_face_info)
{
    int i;
    FT_Face face;

    /* Load face */
    if (FT_New_Face (ft_library, name, 0, &ft_face_info->face))
        return FALSE;

    face = ft_face_info->face;

    /* Look for a Unicode charmap: Windows flavor of Apple flavor only */
    for (i = 0; i < face->num_charmaps; i++) {
        FT_CharMap charmap = face->charmaps [i];

        charmap = face->charmaps[i];
        if (((charmap->platform_id == TT_PLATFORM_MICROSOFT) &&
                (charmap->encoding_id == ft_encoding_unicode)) ||
            ((charmap->platform_id == TT_PLATFORM_APPLE_UNICODE) &&
                (charmap->encoding_id == ft_encoding_none)))
        {
            if ((FT_Select_Charmap (face, charmap->encoding_id))) {
                fprintf (stderr, "GDI (When Create TTF Font): can not set UNICODE CharMap\n");
                return FALSE;
            }
        }
    }

    if (i == face->num_charmaps) {
        fprintf (stderr, "GDI (When Create TTF Font): No UNICODE CharMap\n");
        return FALSE;
    }
    
    return TRUE;
}

static void DestroyFreeTypeFont (FTFACEINFO* ft_face_info)
{
    if (ft_face_info->valid) {
        FT_Done_Face (ft_face_info->face);
        ft_face_info->valid = FALSE;
    }
}

/************************ Init/Term of FreeType fonts ************************/
static int nr_fonts;
static FTFACEINFO* ft_face_infos;
static DEVFONT* ft_dev_fonts;

#define SECTION_NAME    "truetypefonts"

BOOL InitFreeTypeFonts (void)
{
    int i;
    char font_name [LEN_DEVFONT_NAME + 1];

    /* Does load TrueType fonts? */
    if (GetMgEtcIntValue (SECTION_NAME, "font_number",
                &nr_fonts) < 0 )
        return FALSE;

    if ( nr_fonts < 1) return TRUE;

    /* Alloc space for devfont and ttfinfo. */
    ft_face_infos = calloc (nr_fonts, sizeof (FTFACEINFO));
    ft_dev_fonts = calloc (nr_fonts, sizeof (DEVFONT));
    if (ft_face_infos == NULL || ft_dev_fonts == NULL) {
        goto error_alloc;
    }

    for (i = 0; i < nr_fonts; i++)
        ft_face_infos [i].valid = FALSE;

    /* Init freetype library */
    if (FT_Init_FreeType (&ft_library)) {
        goto error_library;
    }

    for (i = 0; i < nr_fonts; i++) {
        char key [11];
        char charset [LEN_FONT_NAME + 1];
        char file [MAX_PATH + 1];
        CHARSETOPS* charset_ops;

        sprintf (key, "name%d", i);
        if (GetMgEtcValue (SECTION_NAME, key,
                           font_name, LEN_DEVFONT_NAME) < 0 )
            goto error_load;

        if (!fontGetCharsetFromName (font_name, charset)) {
            fprintf (stderr, "GDI: Invalid font name (charset): %s.\n",
                    font_name);
            goto error_load;
        }

        if ((charset_ops = GetCharsetOps (charset)) == NULL) {
            fprintf (stderr, "GDI: Not supported charset: %s.\n", charset);
            goto error_load;
        }

        sprintf (key, "fontfile%d", i);
        if (GetMgEtcValue (SECTION_NAME, key,
                           file, MAX_PATH) < 0)
            goto error_load;

        if (!CreateFreeTypeFont (file, ft_face_infos + i))
            goto error_load;

        strncpy (ft_dev_fonts[i].name, font_name, LEN_DEVFONT_NAME);
        ft_dev_fonts[i].name [LEN_DEVFONT_NAME] = '\0';
        ft_dev_fonts[i].font_ops = &freetype_font_ops;
        ft_dev_fonts[i].charset_ops = charset_ops;
        ft_dev_fonts[i].data = ft_face_infos + i;
#if 0
        fprintf (stderr, "GDI: TTFDevFont %i: %s.\n", i, ft_dev_fonts[i].name);
#endif

        ft_face_infos [i].valid = TRUE;
    }

    for (i = 0; i < nr_fonts; i++) {
        if (ft_dev_fonts [i].charset_ops->bytes_maxlen_char > 1) {
            AddMBDevFont (ft_dev_fonts + i);
            AddSBDevFont (ft_dev_fonts + i);
        }
        else
            AddSBDevFont (ft_dev_fonts + i);
    }

    return TRUE;

error_load:
    fprintf (stderr, "GDI: Error in loading TrueType fonts\n");
    for (i = 0; i < nr_fonts; i++)
        DestroyFreeTypeFont (ft_face_infos + i);

error_library:
    fprintf (stderr, "Could not initialise FreeType 2 library\n");

error_alloc:
    free (ft_face_infos);
    free (ft_dev_fonts);
    ft_face_infos = NULL;
    ft_dev_fonts = NULL;
    FT_Done_FreeType (ft_library);
    return FALSE;
}

void TermFreeTypeFonts (void)
{
    if (ft_face_infos)
        FT_Done_FreeType (ft_library);
}

/*************** TrueType on FreeType font operations ************************/
static int get_char_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    return ft_inst_info->ave_width;
}

static int get_max_width (LOGFONT* logfont, DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    return ft_inst_info->max_width;
}

static int get_str_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mstr, int n, int cExtra)
{
    return 0;
}

static int get_ave_width (LOGFONT* logfont, DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    return ft_inst_info->ave_width;
}

static int get_font_height (LOGFONT* logfont, DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    return ft_inst_info->height;
}

static int get_font_size (LOGFONT* logfont, DEVFONT* devfont, int expect)
{
    return expect;
}

static int get_font_ascent (LOGFONT* logfont, DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    return ft_inst_info->ascent;
}

static int get_font_descent (LOGFONT* logfont, DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    return -ft_inst_info->descent;
}

static size_t max_bitmap_size (LOGFONT* logfont, DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    return ((ft_inst_info->max_width + 7) >> 3) * ft_inst_info->height;
}

static size_t char_bitmap_size (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    return max_bitmap_size (logfont, devfont);
}

/* call this function before output a string */
static void start_str_output (LOGFONT* logfont, DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    FTFACEINFO* ft_face_info = ft_inst_info->ft_face_info;

    FT_Activate_Size (ft_inst_info->size);
    FT_Set_Transform (ft_face_info->face, 
                    &ft_inst_info->matrix, &ft_inst_info->pos);

    ft_inst_info->prev_index = 0;
    ft_inst_info->pos.x = 0;
    ft_inst_info->pos.y = (-ft_inst_info->descent) << 6;
}

/* call this function before getting the bitmap/pixmap of the char
 * to get the bbox of the char */
static int get_char_bbox (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len, 
                int* px, int* py, int* pwidth, int* pheight)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    FTFACEINFO* ft_face_info = ft_inst_info->ft_face_info;
    FT_BBox bbox;

    FT_UInt uni_char = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
    ft_inst_info->cur_index = FT_Get_Char_Index (ft_face_info->face, uni_char);

    if (ft_inst_info->use_kerning 
                    && ft_inst_info->prev_index && ft_inst_info->cur_index) {
        FT_Vector  delta;
        FT_Get_Kerning (ft_face_info->face, ft_inst_info->prev_index, ft_inst_info->cur_index,
            ft_kerning_default, &delta);

        ft_inst_info->pos.x += delta.x;
    }

    if (FT_Load_Glyph (ft_face_info->face, ft_inst_info->cur_index, FT_LOAD_DEFAULT))
        return 0;

    if (FT_Get_Glyph (ft_face_info->face->glyph, &ft_inst_info->glyph))
        return 0;

    FT_Glyph_Get_CBox (ft_inst_info->glyph, ft_glyph_bbox_pixels, &bbox);

    if (pwidth) *pwidth = (bbox.xMax - bbox.xMin) >> 6;
    if (pheight) *pheight = (bbox.yMax - bbox.yMin) >> 6;

    if (px) *px = (ft_inst_info->pos.x + bbox.xMin) >> 6;
    if (py) *py = (ft_inst_info->pos.y - bbox.yMax) >> 6;

    ft_inst_info->prev_index = ft_inst_info->cur_index;

    FT_Done_Glyph (ft_inst_info->glyph);

    return (int)(bbox.xMax - bbox.xMin) >> 6;
}

/* call this function to get the bitmap/pixmap of the char */ 
static const void* 
char_bitmap_pixmap (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len, int* pitch) 
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);
    FTFACEINFO* ft_face_info = ft_inst_info->ft_face_info;
    FT_BitmapGlyph glyph_bitmap;
    FT_Bitmap* source;

    if (FT_Get_Glyph (ft_face_info->face->glyph, &ft_inst_info->glyph))
        return 0;

    // convert to a bitmap (default render mode + destroy old)
    if (ft_inst_info->glyph->format != ft_glyph_format_bitmap) {
        if (FT_Glyph_To_Bitmap (&ft_inst_info->glyph, 
                                pitch ? ft_render_mode_normal : ft_render_mode_normal,
                                0, 1))
            return NULL;
    }
    
    // access bitmap content by typecasting
    glyph_bitmap = (FT_BitmapGlyph) ft_inst_info->glyph;
    source = &glyph_bitmap->bitmap;

#if 0
    font_bmp.rows   = source->rows;
    font_bmp.width  = source->width;
    font_bmp.pitch  = source->pitch;
    font_bmp.buffer = source->buffer;
    switch (source->pixel_mode) {
        case ft_pixel_mode_mono:
            font_bmp.bpp = 1;
            break;
        case ft_pixel_mode_grays:
            font_bmp.bpp = 8;
            break;
        default:
            return NULL;
    }
#endif
    switch (source->pixel_mode) {
        case ft_pixel_mode_mono:
            break;
        case ft_pixel_mode_grays:
            break;
        default:
            return NULL;
    }

    *pitch = source->pitch;
    return source->buffer;
}

static const void* get_char_bitmap (LOGFONT* logfont, DEVFONT* devfont,
            const unsigned char* mchar, int len)
{
    return char_bitmap_pixmap (logfont, devfont, mchar, len, NULL);
}

static const void* get_char_pixmap (LOGFONT* logfont, DEVFONT* devfont,
            const unsigned char* mchar, int len, int* pitch)
{
    return char_bitmap_pixmap (logfont, devfont, mchar, len, pitch);
}

/* call this function after getting the bitmap/pixmap of the char
 * to get the advance of the char */
static void get_char_advance (LOGFONT* logfont, DEVFONT* devfont, 
                int* px, int* py)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);

    *px = ft_inst_info->pos.x >> 6;
    *py = ft_inst_info->pos.y >> 6;

    FT_Done_Glyph (ft_inst_info->glyph);
}

static DEVFONT* new_instance (LOGFONT* logfont, DEVFONT* devfont, BOOL need_sbc_font)
{
    FTFACEINFO* ft_face_info = FT_FACE_INFO_P (devfont);
    FTINSTANCEINFO* ft_inst_info = NULL;
    DEVFONT* new_devfont = NULL;
    float angle;

    if ((new_devfont = calloc (1, sizeof (DEVFONT))) == NULL)
        goto out;

    if ((ft_inst_info = calloc (1, sizeof (FTINSTANCEINFO))) == NULL)
        goto out;

    memcpy (new_devfont, devfont, sizeof (DEVFONT));

    if (need_sbc_font && devfont->charset_ops->bytes_maxlen_char > 1) {
        char charset [LEN_FONT_NAME + 1];

        fontGetCompatibleCharsetFromName (devfont->name, charset);
        new_devfont->charset_ops = GetCharsetOps (charset);
    }

    new_devfont->data = ft_inst_info;
    ft_inst_info->ft_face_info = ft_face_info;

    /* Create instance */
    if (FT_New_Size (ft_face_info->face, &ft_inst_info->size))
        goto out_size;
    if (FT_Activate_Size (ft_inst_info->size))
        goto out_size;
    if (FT_Set_Pixel_Sizes (ft_face_info->face, logfont->size, logfont->size))
        goto out_size;
        
    ft_inst_info->use_kerning = FT_HAS_KERNING(ft_face_info->face) 
            && logfont->style & FS_SETWIDTH_CONDENSED;

    /* Font rotation */
    ft_inst_info->rotation = logfont->rotation; /* in tenthdegrees */

    angle = ft_inst_info->rotation * M_PI / 1800;
    ft_inst_info->matrix.yy = cos (angle) * (1 << 16);
    ft_inst_info->matrix.yx = sin (angle) * (1 << 16);
    ft_inst_info->matrix.xx = ft_inst_info->matrix.yy;
    ft_inst_info->matrix.xy = -ft_inst_info->matrix.yx;

    /* Fill up the info fields */
    ft_inst_info->max_width = (ft_inst_info->size->metrics.max_advance / 0x10000) >> 6;
    ft_inst_info->ave_width = ft_inst_info->max_width;
    ft_inst_info->height = (ft_inst_info->size->metrics.height / 0x10000) >> 6;
    ft_inst_info->ascent = (ft_inst_info->size->metrics.ascender / 0x10000) >> 6;
    ft_inst_info->descent = (ft_inst_info->size->metrics.descender / 0x10000) >> 6;

    return new_devfont;

out_size:
    FT_Done_Size (ft_inst_info->size);
out:
    free (ft_inst_info);
    free (new_devfont);
    return NULL;
}

static void delete_instance (DEVFONT* devfont)
{
    FTINSTANCEINFO* ft_inst_info = FT_INST_INFO_P (devfont);

    FT_Done_Size (ft_inst_info->size);
    free (ft_inst_info);
    free (devfont);
}

/**************************** Global data ************************************/
static FONTOPS freetype_font_ops = {
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
    start_str_output,
    get_char_bbox,
    get_char_advance,
    new_instance,
    delete_instance
};

#endif /* _TTF_SUPPORT && FREE_TYPE2 */

