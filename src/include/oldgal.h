/*
** $Id: oldgal.h,v 1.5 2003/08/12 07:46:18 weiym Exp $
**
** oldgal.h: the head file of old Graphics Abstract Layer
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming
**
** Create date: 2000/06/11
*/

#ifndef GUI_OLDGAL_H
    #define GUI_OLDGAL_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
#ifdef _SVGALIB
    #include <vga.h>
    #include <vgagl.h>
#endif
#ifdef _LIBGGI
    #include <ggi/ggi.h>
#endif
#ifdef _EP7211_GAL
#include "../gal/ep7211_internal.h"
#endif
#ifdef _ADS_GAL
	#include "../gal/ads_internal.h"
#endif
#ifdef _VGA16_GAL
typedef struct tagGC_VGA16* PGC_VGA16;
#endif

#ifdef _NATIVE_GAL_ENGINE
typedef struct _screendevice *PSD;
#endif

#ifdef _LITE_VERSION
#include <signal.h>
extern sigset_t __mg_blockset;
extern sigset_t __mg_savedset;

typedef struct tagGALClipInfo
{
	int	doclip;
	int	clipminx;
	int	clipminy;
	int	clipmaxx;
	int	clipmaxy;
} GALCLIPINFO;
#endif

typedef union tagGAL_GC
{
#ifdef _SVGALIB
    GraphicsContext*    gc;
#endif
#ifdef _LIBGGI
    ggi_visual_t        visual;
#endif
#ifdef _EP7211_GAL
	VIS_EP7211*         vis_ep7211;
#endif
#ifdef _ADS_GAL
	ADS*                ads;
#endif
#ifdef _NATIVE_GAL_ENGINE
    PSD                 psd;
#endif
#ifdef _VGA16_GAL
	PGC_VGA16           gc_vga16;
#endif
#ifdef _LITE_VERSION
    GALCLIPINFO*        clip_info;      // aliase to clipping info in PSD and GC_VGA16
#endif
} GAL_GC;

extern gal_pixel SysPixelIndex [];
extern const RGB SysPixelColor [];

typedef struct tagGFX
{
    char*   id;

    // Initialization and termination
    BOOL    (*initgfx) (struct tagGFX* gfx);
    void    (*termgfx) (struct tagGFX* gfx);

    // Phisical graphics context
    GAL_GC  phygc;
    int     bytes_per_phypixel;
    int     bits_per_phypixel;
    int     width_phygc;
    int     height_phygc;
    int     colors_phygc;
    BOOL    grayscale_screen;

    // GC properties
    int     (*bytesperpixel) (GAL_GC gc);
    int     (*bitsperpixel) (GAL_GC gc);
    int     (*width) (GAL_GC gc);
    int     (*height) (GAL_GC gc);
    int     (*colors) (GAL_GC gc);

    // Allocation and release of graphics context
    int     (*allocategc) (GAL_GC gc, int width, int height, int depth, 
                GAL_GC* newgc);
    void    (*freegc) (GAL_GC gc);
    void    (*setgc) (GAL_GC gc);

    // Clipping of graphics context
    void    (*enableclipping) (GAL_GC gc);
    void    (*disableclipping) (GAL_GC gc);
    int     (*setclipping) (GAL_GC gc, int x1, int y1, int x2, int y2);
    int     (*getclipping) (GAL_GC gc, int* x1, int* y1, int* x2, int* y2);

    // Background and foreground colors
    int     (*getbgcolor) (GAL_GC gc, gal_pixel* color);
    int     (*setbgcolor) (GAL_GC gc, gal_pixel color);
    int     (*getfgcolor) (GAL_GC gc, gal_pixel* color);
    int     (*setfgcolor) (GAL_GC gc, gal_pixel color);

    // Convertion between GAL_Color and gal_pixel
    gal_pixel (*mapcolor) (GAL_GC gc, GAL_Color *color);
    int     (*unmappixel) (GAL_GC gc, gal_pixel pixel, GAL_Color* color);

    // Palette operations
    int     (*getpalette) (GAL_GC gc, int s, int len, GAL_Color* cmap);
    int     (*setpalette) (GAL_GC gc, int s, int len, GAL_Color* cmap);
    int     (*setcolorfulpalette) (GAL_GC gc);

    // Box operations
    size_t  (*boxsize) (GAL_GC gc, int w, int h);
    int     (*fillbox) (GAL_GC gc, int x, int y, int w, int h, 
                gal_pixel pixel);
    int     (*putbox) (GAL_GC gc, int x, int y, int w, int h, void* buf);
    int     (*getbox) (GAL_GC gc, int x, int y, int w, int h, void* buf);
    int     (*putboxmask) (GAL_GC gc, int x, int y, int w, int h, void* buf, gal_pixel cxx);
    int     (*scalebox) (GAL_GC gc, int sw, int sh, void* srcbuf,
                int dw, int dh, void* dstbuf);
    
    int     (*copybox) (GAL_GC gc, int x, int y, int w, int h, int nx, int ny);
    int     (*crossblit) (GAL_GC src, int sx, int sy, int sw, int sh,
                GAL_GC dst, int dx, int dy); 

    // Horizontal line operaions
    int     (*drawhline) (GAL_GC gc, int x, int y, int w, gal_pixel pixel);

    // Vertical line operations
    int     (*drawvline) (GAL_GC gc, int x, int y, int h, gal_pixel pixel);

    // Pixel operations
    int     (*drawpixel) (GAL_GC gc, int x, int y, gal_pixel pixel);
    int     (*getpixel) (GAL_GC gc, int x, int y, gal_pixel* color);

    // Other drawing
    int     (*circle) (GAL_GC gc, int x, int y, int r, gal_pixel pixel);
    int     (*line) (GAL_GC gc, int x1, int y1, int x2, int y2, 
                gal_pixel pixel);
    int     (*rectangle) (GAL_GC gc, int l, int t, int r, int b, 
                gal_pixel pixel);

    // Panic
    void (*panic) (int exitcode);

    // unused operations
#if 0
    int     (*packcolors) (GAL_GC gc, void* buf, GAL_Color* colors, int len);
    int     (*unpackpixels) (GAL_GC gc, void* buf, GAL_Color* colors, int len);

    int     (*putboxwithop) (GAL_GC gc, int x, int y, int w, int h, 
                void* buf, int raster_op);
    int     (*putboxpart) (GAL_GC gc, int x, int y, int w, int h, int bw,
                int bh, void* buf, int xo, int yo);

    int     (*puthline)  (GAL_GC gc, int x, int y, int w, void* buf);
    int     (*gethline)  (GAL_GC gc, int x, int y, int w, void* buf);

    int     (*putvline)  (GAL_GC gc, int x, int y, int h, void* buf);
    int     (*getvline)  (GAL_GC gc, int x, int y, int h, void* buf);

    int     (*putpixel) (GAL_GC gc, int x, int y, gal_pixel color);

    // Simple Character output
    int     (*putchar) (GAL_GC gc, int x, int y, char c);
    int     (*putstr) (GAL_GC gc, int x, int y, const char* str);
    int     (*getcharsize) (GAL_GC gc, int* width, int* height);
    int     (*setputcharmode) (GAL_GC gc, int bkmode);
    int     (*setfontcolors) (GAL_GC gc, 
                gal_pixel fg, gal_pixel bg);

    // Asynchronous mode support
    void (*flush) (GAL_GC gc);
    void (*flushregion) (GAL_GC gc, int x, int y, int w, int h);
#endif

} GFX;

extern GFX* cur_gfx;

#define PHYSICALGC              (cur_gfx->phygc)
#define BYTESPERPHYPIXEL        (cur_gfx->bytes_per_phypixel)
#define BITSPERPHYPIXEL         (cur_gfx->bits_per_phypixel)
#define WIDTHOFPHYGC            (cur_gfx->width_phygc)
#define HEIGHTOFPHYGC           (cur_gfx->height_phygc)
#define COLORSOFPHYGC           (cur_gfx->colors_phygc)
#define GRAYSCALESCREEN         (cur_gfx->grayscale_screen)

#define GAL_BytesPerPixel       (*cur_gfx->bytesperpixel)
#define GAL_BitsPerPixel        (*cur_gfx->bitsperpixel)
#define GAL_Width               (*cur_gfx->width)
#define GAL_Height              (*cur_gfx->height)
#define GAL_Colors              (*cur_gfx->colors)

#define GAL_InitGfx             (*cur_gfx->initgfx)
#define GAL_TermGfx             (*cur_gfx->termgfx)

#define GAL_AllocateGC          (*cur_gfx->allocategc)
#define GAL_FreeGC              (*cur_gfx->freegc)

#define GAL_SetGC(gc)           if (cur_gfx->setgc) (*cur_gfx->setgc) (gc)

#define GAL_EnableClipping      (*cur_gfx->enableclipping)
#define GAL_DisableClipping     (*cur_gfx->disableclipping)
#define GAL_SetClipping         (*cur_gfx->setclipping)
#define GAL_GetClipping         (*cur_gfx->getclipping)

#define GAL_GetBgColor          (*cur_gfx->getbgcolor)
#define GAL_GetFgColor          (*cur_gfx->getfgcolor)
#define GAL_SetBgColor(gc, pixel)   if (cur_gfx->setbgcolor) (*cur_gfx->setbgcolor) (gc, pixel)
#define GAL_SetFgColor(gc, pixel)   if (cur_gfx->setfgcolor) (*cur_gfx->setfgcolor) (gc, pixel)

#define GAL_MapColor            (*cur_gfx->mapcolor)
#define GAL_UnmapPixel          (*cur_gfx->unmappixel)

#define GAL_GetPalette          (*cur_gfx->getpalette)
#define GAL_SetPalette          (*cur_gfx->setpalette)
#define GAL_SetColorfulePalette (*cur_gfx->setcolorfulpalette)

#define GAL_BoxSize             (*cur_gfx->boxsize)
#define GAL_FillBox             (*cur_gfx->fillbox)
#define GAL_PutBox              (*cur_gfx->putbox)
#define GAL_GetBox              (*cur_gfx->getbox)
#define GAL_PutBoxMask          (*cur_gfx->putboxmask)
#define GAL_ScaleBox            (*cur_gfx->scalebox)

#define GAL_CopyBox             (*cur_gfx->copybox)
#define GAL_CrossBlit           (*cur_gfx->crossblit)

#define GAL_DrawHLine           (*cur_gfx->drawhline)
#define GAL_DrawVLine           (*cur_gfx->drawvline)

#define GAL_DrawPixel           (*cur_gfx->drawpixel)
#define GAL_GetPixel            (*cur_gfx->getpixel)

#define GAL_Circle              (*cur_gfx->circle)
#define GAL_Line                (*cur_gfx->line)
#define GAL_Rectangle           (*cur_gfx->rectangle)

#if 0
#define GAL_PutBoxPart          (*cur_gfx->putboxpart)
#define GAL_PutBoxWithOp        (*cur_gfx->putboxwithop)
#define GAL_PackColors          (*cur_gfx->packcolors)
#define GAL_UnpackPixels        (*cur_gfx->unpackpixels)
#define GAL_PutVLine            (*cur_gfx->putvline)
#define GAL_GetVLine            (*cur_gfx->getvline)
#define GAL_PutHLine            (*cur_gfx->puthline)
#define GAL_GetHLine            (*cur_gfx->gethline)
#define GAL_PutPixel            (*cur_gfx->putpixel)
#define GAL_PutChar             (*cur_gfx->putchar)
#define GAL_PutStr              (*cur_gfx->putstr)
#define GAL_GetCharSize         (*cur_gfx->getcharsize)
#define GAL_SetPutCharMode      (*cur_gfx->setputcharmode)
#define GAL_SetFontColors       (*cur_gfx->setfontcolors)

#define GAL_Flush(gc)           if (cur_gfx->flush) (*cur_gfx->flush) (gc)
#define GAL_FlushRegion(gc, x, y, w, h)  if (cur_gfx->flushregion) (*cur_gfx->flush) (gc, x, y, w, h)
#endif

#define GAL_Panic               (*cur_gfx->panic)

int InitGAL (void);
void TerminateGAL (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_OLDGAL_H */


