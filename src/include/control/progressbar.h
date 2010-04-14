/*
** $Id: progressbar.h,v 1.6 2003/09/04 03:40:35 weiym Exp $
**
** prograssbar.h: the head file of PrograssBar control.
**
** Copyright (c) 2003 Feynman Software.
** Copyright (c) 1999 ~ 2002 Wei Yongming.
**
** Create date: 1999/8/29
*/

#ifndef __PROGRESSBAR_H
#define __PROGRESSBAR_H

#ifdef  __cplusplus
extern  "C" {
#endif

typedef  struct tagPROGRESSDATA
{
    unsigned int nMin;
    unsigned int nMax;
    unsigned int nPos;
    unsigned int nStepInc;
}PROGRESSDATA;
typedef PROGRESSDATA* PPROGRESSDATA;

BOOL RegisterProgressBarControl (void);
void ProgressBarControlCleanup (void);

#ifdef  __cplusplus
}
#endif

#endif // __PROGRESSBAR_H

