/*
** $Id: freetype.h,v 1.5 2003/04/29 07:16:34 weiym Exp $
**
** freetype.h: TrueType font support based on FreeType.
**
** Copyright (C) 2000 ~ 2002 Wei Yongmin
** Copyright (C) 2003 Feynman Software.
**
** Created by WEI Yongming, 2000/8/21
*/

#ifndef GUI_FREETYP_H
    #define GUI_FREETYP_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct tagTTFGLYPHINFO {
    TT_Face     face;

    TT_Glyph    glyph;
    TT_UShort   last_glyph_index;

    TT_CharMap  char_map;
    TT_UShort   first_char;
    TT_UShort   last_char;

    BOOL        can_kern;
    TT_Kerning  directory;

    BOOL        valid;
} TTFGLYPHINFO, *PTTFGLYPHINFO;

typedef struct tagTTFINSTANCEINFO {
    PTTFGLYPHINFO ttf_glyph_info;

    TT_Instance instance;
    int         rotation;
    TT_Matrix   matrix;

    int         max_width;
    int         ave_width;
    int         height;
    int         ascent;
    int         descent;
    unsigned short* widths;

    short       cur_glyph_code;
    TT_Outline  cur_outline;
    TT_Pos      cur_xmin, cur_ymin;
    TT_F26Dot6  cur_width, cur_height;
    TT_Pos      cur_advance;

    short       last_glyph_code;
    short       last_pen_pos;
} TTFINSTANCEINFO, *PTTFINSTANCEINFO;

#define TTF_GLYPH_INFO_P(devfont) ((TTFGLYPHINFO*)(devfont->data))
#define TTF_INST_INFO_P(devfont) ((TTFINSTANCEINFO*)(devfont->data))

extern FONTOPS freetype_font_ops;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_FREETYP_H

