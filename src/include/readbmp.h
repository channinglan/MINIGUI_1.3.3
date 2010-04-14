/*
** $Id: readbmp.h,v 1.9 2003/08/12 07:46:18 weiym Exp $
**
** readbmp.h: Low Level bitmap file read/save routines.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Create date: 2001/xx/xx
*/

#ifndef GUI_GDI_READBMP_H
    #define GUI_GDI_READBMP_H

#include "endianrw.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int load_bmp (MG_RWops* fp, MYBITMAP *bmp, RGB *pal);
int save_bmp (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
BOOL check_bmp (MG_RWops* fp);

#ifdef _LBM_FILE_SUPPORT
int load_lbm (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
int save_lbm (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
BOOL check_lbm (MG_RWops* fp);
#endif

#ifdef _PCX_FILE_SUPPORT
int load_pcx (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
int save_pcx (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
BOOL check_pcx (MG_RWops* fp);
#endif

#ifdef _TGA_FILE_SUPPORT
int load_tga (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
int save_tga (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
BOOL check_tga (MG_RWops* fp);
#endif

#ifdef _GIF_FILE_SUPPORT
int load_gif (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
int save_gif (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
BOOL check_gif (MG_RWops* fp);
#endif

#ifdef _JPG_FILE_SUPPORT
int load_jpg (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
int save_jpg (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
BOOL check_jpg (MG_RWops* fp);
#endif

#ifdef _PNG_FILE_SUPPORT
int load_png (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
int save_png (MG_RWops* fp, MYBITMAP* bmp, RGB* pal);
BOOL check_png (MG_RWops* fp);
#endif

int bmpComputePitch (int bpp, Uint32 width, Uint32 *pitch, BOOL does_round);

#define fp_getc(fp)     MGUI_RWgetc(fp)
#define fp_igetw(fp)    MGUI_ReadLE16(fp)
#define fp_igetl(fp)    MGUI_ReadLE32(fp)
#define fp_mgetw(fp)    MGUI_ReadBE16(fp)
#define fp_mgetl(fp)    MGUI_ReadBE32(fp)

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_GDI_READBMP_H

