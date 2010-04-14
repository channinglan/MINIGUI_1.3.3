//
// $Id: svgalib.h,v 1.2 2000/06/19 03:33:29 weiym Exp $
//
// svgalib.h: the head file of Low Level Graphics Engine based on SVGALib
//
// Written by WEI Yongming, 2000/06/11
//

#ifndef GUI_GAL_SVGALIB_H
    #define GUI_GAL_SVGALIB_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

BOOL    InitSVGALib (GFX* gfx);
void    TermSVGALib (GFX* gfx);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_GAL_SVGALIB_H */


