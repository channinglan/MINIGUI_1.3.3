//
// $Id: libggi.h,v 1.4 2000/06/21 01:26:01 weiym Exp $
//
// libggi.h: the head file of Low Level Graphics Engine based on LibGGI
//
// Written by WEI Yongming, 2000/06/11
//

#ifndef GUI_GAL_LIBGGI_H
    #define GUI_GAL_LIBGGI_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

BOOL    InitLibGGI (struct tagGFX* gfx);
void    TermLibGGI (struct tagGFX* gfx);
	
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_GAL_LIBGGI_H */


