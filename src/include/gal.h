/*
** $Id: gal.h,v 1.27 2003/08/15 07:41:26 weiym Exp $
**
** gal.h: the head file of Graphics Abstract Layer
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming
**
** Create date: 2001/10/07
*/

#ifndef GUI_GAL_H
    #define GUI_GAL_H

#ifdef _LITE_VERSION

extern BOOL __mg_switch_away; // always be zero for clients.

#ifndef _STAND_ALONE
extern GHANDLE __mg_layer;

void unlock_draw_sem (void);
void lock_draw_sem (void);
#endif

#endif

#ifdef _USE_NEWGAL
    #include "newgal.h"
#else
    #include "oldgal.h"
#endif

#endif  /* GUI_GAL_H */

