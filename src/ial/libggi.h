/*
** $Id: libggi.h,v 1.4 2003/09/04 03:38:25 weiym Exp $
**
** libggi.h: the head file of Low Level Graphics Engine based on LibGGI
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Written by WEI Yongming, 2000/06/11
*/

#ifndef GUI_GAL_LIBGGI_H
    #define GUI_GAL_LIBGGI_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

BOOL    InitLibGGIInput (INPUT* input, const char* mdev, const char* mtype);
void    TermLibGGIInput (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_GAL_LIBGGI_H */


