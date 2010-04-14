/*
** $Id: monthcalendar.h,v 1.4 2003/09/04 06:12:04 weiym Exp $
**
** monthcalendar.h: the header for MonthCalendar control.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2000 ~ 2002 Zhong Shuyi
**
** Create date: 2001/01/03
*/

#ifndef __MONTHCALENDAR_H_
#define __MONTHCALENDAR_H_

#ifdef  __cplusplus
extern  "C" {
#endif

#define MCS_LANG 0x0003L

// structs 
typedef struct tagMonthCalendarData
{
	int		sys_month;
	int		sys_year;
	int		sys_day;
	int 	sys_WeekDay;

	int 	cur_month;
	int 	cur_year;
	int 	cur_day;

	int 	cur_line;
	int 	cur_WeekDay;
	int 	item_w;
	int 	item_h;
	int 	WeekDay1;
	int 	monlen;

	DWORD 	dwClrData;
} MONCALDDATA;
typedef MONCALDDATA* PMONCALDDATA;

BOOL RegisterMonthCalendarControl (void);
void MonthCalendarControlCleanup (void);

#ifdef  __cplusplus
}
#endif

#endif	// __MONTHCALENDAR_H__
