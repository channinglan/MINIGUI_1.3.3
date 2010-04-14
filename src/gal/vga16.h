//
// $Id: vga16.h,v 1.2 2001/09/24 02:44:43 ymwei Exp $
//
// vga16.h: the head file of VGA 16-color GAL engine.
//
// Created by WEI Yongming, 2001/09/21
//

#ifndef GUI_GAL_VGA16_H
    #define GUI_GAL_VGA16_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct tagGC_VGA16
{
    int doclip;
	int clipminx, clipminy, clipmaxx, clipmaxy;

    int xres, yres;
    int pitch;

    gal_pixel gr_background;
    gal_pixel gr_foreground;

    unsigned char scanline[640];

	unsigned char* fb_buff;

} GC_VGA16;

BOOL InitVGA16 (GFX* gfx);
void TermVGA16 (GFX* gfx);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_GAL_VGA16_H */


