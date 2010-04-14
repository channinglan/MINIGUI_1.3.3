/*
** $Id: freetype2.h,v 1.4 2003/04/29 07:16:34 weiym Exp $
**
** freetype.h: TrueType font support based on FreeType.
**
** Copyright (C) 2000 ~ 2002 Wei Yongming
** Copyright (C) 2003 Feynman Software.
**
** Created by WEI Yongming, 2000/8/21
*/

#ifndef GUI_FREETYP2_H
    #define GUI_FREETYP2_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct tagFTFACEINFO {
    FT_Face     face;
    BOOL        valid;
} FTFACEINFO, *PFTFACEINFO;

typedef struct tagFTINSTANCEINFO {
    PFTFACEINFO ft_face_info;

    FT_Size     size;
    int         rotation;
    FT_Matrix   matrix;
    FT_Vector   pos;

    int         max_width;
    int         ave_width;
    int         height;
    int         ascent;
    int         descent;

    FT_Glyph    glyph;
    FT_Bool     use_kerning;
    FT_UInt     cur_index;
    FT_UInt     prev_index;
} FTINSTANCEINFO, *PFTINSTANCEINFO;

#define FT_FACE_INFO_P(devfont) ((FTFACEINFO*)(devfont->data))
#define FT_INST_INFO_P(devfont) ((FTINSTANCEINFO*)(devfont->data))

extern FONTOPS freetype_font_ops;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_FREETYP2_H

