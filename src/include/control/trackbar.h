/*
** $Id: trackbar.h,v 1.7 2003/09/04 03:40:35 weiym Exp $
**
** trackbar.h: the head file of TrackBar(Slider) control.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming and others.
**
** Note:
**   Originally by Zheng Yiran.
**
** Create date: 2000/12/02
*/

#ifndef __TRACKBAR_H
#define __TRACKBAR_H

#ifdef  __cplusplus
extern  "C" {
#endif

typedef  struct tagTRACKBARDATA
{
    int nMin;
    int nMax;
    int nPos;
    int nLineSize;
    int nPageSize;
    char sStartTip [TBLEN_TIP + 1];
    char sEndTip [TBLEN_TIP + 1];
    int nTickFreq;
	int mousepos;
}TRACKBARDATA;
typedef TRACKBARDATA* PTRACKBARDATA;

BOOL RegisterTrackBarControl (void);
void TrackBarControlCleanup (void);

#ifdef  __cplusplus
}
#endif

#endif // __TRACKBAR_H

