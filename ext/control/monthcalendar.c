/*
** $Id: monthcalendar.c,v 1.26 2003/11/12 10:17:15 snig Exp $
**
** monthcalendar.c: the month calendar Control module.
**
** Copyright (C) 2001, Zhong Shuyi
** Copyright (C) 2002, 2003 Feynman Software
**
** Create date: 2001/01/02
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/* Modify records:
**
**   Who             When        Where       For What                Status
**-----------------------------------------------------------------------------
**  Wei Yongming    2001/02/07  BluePoint   Do some code clean up.  Done.
**  snig            2001/02/22  BluePoint    Add tab month             Done.
**  snig            2001/05/24  BluePoint   Fix lang style bug      Done.
**
** TODO:
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __MINIGUI_LIB__
    #include "common.h"
    #include "minigui.h"
    #include "gdi.h"
    #include "window.h"
    #include "control.h"
#else
    #include <minigui/common.h>
    #include <minigui/minigui.h>
    #include <minigui/gdi.h>
    #include <minigui/window.h>
    #include <minigui/control.h>
#endif

#ifdef _EXT_CTRL_MONTHCAL

#include "mgext.h"
#include "monthcalendar.h"

#define MINWNDRECT_W    120
#define MINWNDRECT_H    110

#define DATECAP_H(hwnd)        (GetWindowFont(hwnd)->size+8)
#define MONFIELD_W       50
#define YEARFIELD_W      50
#define WEEKFIELD_W      30
#define WEEKFIELD_H(hwnd)      (GetWindowFont(hwnd)->size+4)
#define WEEK_BORDER      5
#define WEEK_VBORDER1    2
#define WEEK_VBORDER2    3
#define TEXT_BORDER      5
#define LINE_D           2

#define ARROW_H          7
#define ARROW_W          (ARROW_H+1)/2
#define ARROWRECT_W      (ARROW_W+4)

#define HEIGHT_BORDER    6
#define MON_WIDTH        60
#define MON_HEIGHT       14
#define MON_BORDER       15
#define YEAR_WIDTH       40
#define YEAR_HEIGHT      14
//#define YEAR_HEIGHT    18
#define YEAR_BORDER      15
#define ARROW_BORDER     10

// color info
#define MCCLR_DF_TITLEBK        PIXEL_lightgray
#define MCCLR_DF_TITLETEXT        PIXEL_black
#define MCCLR_DF_ARROW            PIXEL_black
#define MCCLR_DF_ARROWHIBK        PIXEL_lightwhite
#define MCCLR_DF_DAYBK            PIXEL_lightwhite
#define MCCLR_DF_DAYHIBK        GetWindowElementColor(BKC_MENUITEM_HILITE)
#define MCCLR_DF_DAYTEXT        PIXEL_black
#define MCCLR_DF_TRAILINGTEXT        PIXEL_lightwhite
#define MCCLR_DF_DAYHITEXT        GetWindowElementColor(FGC_MENUITEM_HILITE)
#define MCCLR_DF_WEEKCAPTBK        PIXEL_darkblue
#define MCCLR_DF_WEEKCAPTTEXT        PIXEL_lightwhite

static int MonthCalendarCtrlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

BOOL RegisterMonthCalendarControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName     = "monthcalendar";
    WndClass.dwStyle        = WS_NONE;
    WndClass.dwExStyle        = WS_EX_NONE;
    WndClass.hCursor        = GetSystemCursor (IDC_ARROW);
    WndClass.iBkColor        = PIXEL_lightwhite;
    WndClass.WinProc        = MonthCalendarCtrlProc;

#if 0
    return AddNewControlClass (&WndClass) == ERR_OK;
#else
    return RegisterWindowClass (&WndClass);
#endif
}

void MonthCalendarControlCleanup (void)
{
#if 0
    // do nothing
#else
    UnregisterWindowClass (CTRL_MONTHCALENDAR);
#endif
    return;
}

// -----------------------------------------------------------------------------------
// draw 3D frame
#if 0
static void mcDraw3DMCDownFrame (HDC hdc, int x0, int y0, int x1, int y1, 
            int fillc)
{
    int left, top, right, bottom;
    left = MIN (x0, x1);
    top  = MIN (y0, y1);
    right = MAX (x0, x1);
    bottom = MAX (y0, y1);
    
        SetPenColor (hdc, PIXEL_lightwhite);
        MoveTo (hdc, left, bottom);
        LineTo (hdc, right, bottom);
        LineTo (hdc, right, top);
        SetPenColor (hdc, PIXEL_black);
        MoveTo (hdc, left, bottom);
        LineTo (hdc, left, top);
        LineTo (hdc, right+1, top);

        left++; top++; right--; bottom--;
        SetPenColor (hdc, PIXEL_lightwhite);
        MoveTo (hdc, left, bottom);
        LineTo (hdc, right, bottom);
        LineTo (hdc, right, top);
        SetPenColor (hdc, PIXEL_black);
        MoveTo (hdc, left, bottom);
        LineTo (hdc, left, top);
        LineTo (hdc, right+1, top);

    if (fillc > 0) {
        SetBrushColor (hdc, fillc);
        FillBox (hdc, left + 1, top + 1, right - left - 1 , bottom - top - 1); 
    }
}
#endif

// draw page arrow
static void mcDrawPageArrow (PMONCALDDATA mc_data, HDC hdc, RECT* prcArrow, 
                                BOOL bFaceR, BOOL bHilight)
{
    int arrow_w, arrow_h = ARROW_H;
    int line_x, line_h, line_y;
    int stepx;
    int rc_w, rc_h;
    PMCCOLORINFO pmcci;

    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    arrow_w = (arrow_h + 1)/2;
    rc_w = prcArrow->right - prcArrow->left;
    rc_h = prcArrow->bottom - prcArrow->top;
    SetPenColor (hdc, pmcci->clr_arrow);
    if (bHilight) {
        SetBrushColor (hdc, pmcci->clr_arrowHibk);
        FillBox (hdc, prcArrow->left, prcArrow->top, rc_w, rc_h);
    }
    else {
        SetBrushColor (hdc, pmcci->clr_titlebk);
        FillBox (hdc, prcArrow->left, prcArrow->top, rc_w, rc_h);
    }
    line_y = prcArrow->top + (rc_h-arrow_h)/2;
    line_h = arrow_h;
    if (bFaceR) {
        stepx = 1;
        line_x = prcArrow->left + (rc_w - arrow_w)/2;
    }
    else {
        stepx = -1;
        line_x = prcArrow->right - (rc_w - arrow_w)/2 - 1;
    }
    while (line_h > 0) {
        MoveTo (hdc, line_x, line_y);
        LineTo (hdc, line_x, line_y+line_h);
        line_x += stepx;
        line_y ++;
        line_h -= 2;
    }
}

// --------------------------------------------------------------------------
static void mcMonthCalendarCleanUp (MONCALDDATA* mc_data)
{
    free ((PMCCOLORINFO) mc_data->dwClrData);
    return;
}

// --------------------------------------------------------------------------

// find out whether a year is a leap year
static BOOL IsLeapYear (int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) return TRUE;
    else return FALSE;
}


// get month length from year and month
static int GetMonLen (int year, int month)
{
    int mon_len;

    if (month < 1 || month > 12) return -1;
    if ((month <= 7 && month % 2 == 1) || (month >= 8 && month % 2 == 0))
        mon_len = 31;
    else if (month == 2) 
        if (IsLeapYear (year)) mon_len = 29;
        else mon_len = 28;
    else
        mon_len = 30;
    return mon_len;
}

static int GetYearLen (int year)
{
    int i, year_len = 0;
    for (i = 1; i <= 12; i++) {
        year_len += GetMonLen (year, i);    
    }
    return year_len;
}

// get previous month length from year and month
static int GetPMonLen (int year, int month)
{
    if (month > 1 && month <= 12) return GetMonLen (year, month - 1);
    else if (month == 1) return GetMonLen (year - 1, 12);
    else return -1;
}


static int MyGetWeekDay (int year, int month, int day)
{
    // days from 2036/12/31
    int weekday, weekday1 = 3, daylen = 0, i;

    if (year < 2037) return -1;
    for (i = 2037; i < year; i++) {
        daylen += GetYearLen (i);
    }
    for (i = 1; i < month; i++)    {
        daylen += GetMonLen (year, i);
    }    
    daylen += day;
    weekday = (weekday1 + daylen) - (int)((weekday1 + daylen) / 7)*7;
    return weekday;    
}

// get weekday from date
static int GetWeekDay (int year, int month, int day)
{
    struct tm nowday;

    if (year < 1902) return -1;
    if (month < 1 || month > 12) return -1;
    if (day < 1 || day > 31) return -1;
    
    if (year >= 2037) return MyGetWeekDay (year, month, day);
    nowday.tm_sec = 0;
    nowday.tm_min = 0;
    nowday.tm_hour = 0;
    nowday.tm_mday = day;
    nowday.tm_mon = month-1;
    nowday.tm_year = year-1900;

    if (mktime(&nowday) == -1) {
        fprintf (stderr, "mktime error\n"); //test
        return -1;
    }
    else
        return nowday.tm_wday;    
}

// get line and weekday from date according to weekday1
static void mcGetCurDayPos (PMONCALDDATA mc_data, int day, int* pline, int* pWeekDay)
{
    *pWeekDay = (mc_data->WeekDay1 + day - 1) % 7;
    *pline = (day + 6 - *pWeekDay - 1)/7;
}

/*
// get line and weekday from date
static void mcGetDayPos (int year, int month, int day, int* pline, int* pWeekDay)
{
    *pWeekDay = GetWeekDay (year, month, day);
    *pline = (day + 6 - *pWeekDay - 1)/7;    
}
*/

// get rects of arrows
static void mcGetArrowRects (RECT* prcMonth, RECT* prcYear, 
                                RECT* prcAML, RECT* prcAMR, 
                                RECT* prcAYL, RECT* prcAYR)
{
    prcAML->right = prcMonth->left - 1;
    prcAML->left = prcAML->right - ARROWRECT_W;
    prcAML->top = prcMonth->top;
    prcAML->bottom = prcMonth->bottom;
    
    prcAMR->left = prcMonth->right + 2;    
    prcAMR->right = prcAMR->left + ARROWRECT_W;
    prcAMR->top = prcMonth->top;
    prcAMR->bottom = prcMonth->bottom;

    prcAYL->right = prcYear->left - 2;
    prcAYL->left = prcAYL->right - ARROWRECT_W;
    prcAYL->top = prcYear->top;
    prcAYL->bottom = prcYear->bottom;
    
    prcAYR->left = prcYear->right + 2;    
    prcAYR->right = prcAYR->left + ARROWRECT_W;
    prcAYR->top = prcYear->top;
    prcAYR->bottom = prcYear->bottom;
}

// get rect of month day area
static void mcGetMDayRect (HWND hwnd, RECT* prcClient, RECT* prcMDay)
{
    prcMDay->left = prcClient->left + WEEK_BORDER;
    prcMDay->right = prcClient->right - WEEK_BORDER;
    prcMDay->top = prcClient->top + 2 + DATECAP_H(hwnd) + 2*WEEK_VBORDER2 + WEEKFIELD_H(hwnd);
    prcMDay->bottom = prcClient->bottom - WEEK_VBORDER2;
}

// get rects of ...
static void mcGetRects (HWND hWnd,  RECT* prcClient,
                                    RECT* prcMonth,
                                    RECT* prcYear,
                                    RECT* prcMDay)
{
    if (prcClient) {
        GetClientRect (hWnd, prcClient);
            prcClient->right --;
            prcClient->bottom --;
    }    
    
    if (prcMonth) 
        SetRect (prcMonth, prcClient->left + MON_BORDER,
                      //prcClient->top + 4,
                      prcClient->top + 6,
                      prcClient->left + MON_BORDER + MON_WIDTH,
                      prcClient->top + 2 + DATECAP_H(hWnd) - 4);

    if (prcYear)
        SetRect (prcYear, prcClient->right - YEAR_BORDER - YEAR_WIDTH,
                      //prcClient->top + 4,
                      prcClient->top + 6,
                      prcClient->right - YEAR_BORDER,
                      prcClient->top + 2 + DATECAP_H(hWnd) - 4);

    if (prcMDay) 
        mcGetMDayRect (hWnd, prcClient, prcMDay);
}

// get item rect from the x-y mouse position
static void mcGetItemRectFromPos (PMONCALDDATA mc_data,    RECT* prcMDay, 
                            RECT* prcItem, int* pline, int* pweekday, int x, int y)
{
    *pline = (y - prcMDay->top) / mc_data->item_h;
    *pweekday = (x - prcMDay->left) / mc_data->item_w;
    prcItem->left = prcMDay->left + *pweekday*mc_data->item_w;
    prcItem->right = prcItem->left + mc_data->item_w;
    prcItem->top = prcMDay->top + *pline*mc_data->item_h;
    prcItem->bottom = prcItem->top + mc_data->item_h;
}

// get current item rect from mc_data
static void mcGetCurRect (RECT* prcMDay, RECT* prcItem, PMONCALDDATA mc_data)
{
    prcItem->left = prcMDay->left + mc_data->cur_WeekDay*mc_data->item_w;    
    prcItem->top = prcMDay->top + mc_data->cur_line*mc_data->item_h;
    prcItem->right = prcItem->left + mc_data->item_w;
    prcItem->bottom = prcItem->top + mc_data->item_h;
}

// textout in a rect center style
static void mcTextOutCenter (HDC hdc, RECT* prcText, const char* pchText)
{

#if 0
    int ch_w, ch_h, x, y;
    int bkMode;
    
    ch_w = GetSysCharWidth ();
    ch_h = GetSysCharHeight ();
    x = (prcText->right + prcText->left)/2 - strlen(pchText)*ch_w/2;
    y = (prcText->bottom + prcText->top)/2 - ch_h/2+1;
    if (pchText) {
        bkMode = SetBkMode (hdc, BM_TRANSPARENT);
        TextOut (hdc, x, y, pchText, -1);
        SetBkMode (hdc, bkMode);
    }
#else

    if (pchText) {
        int bkMode;

        bkMode = SetBkMode (hdc, BM_TRANSPARENT);
        DrawText (hdc, pchText, -1, prcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        SetBkMode (hdc, bkMode);
    }
#endif
}

static void mcUnHilightRect (PMONCALDDATA mc_data, HDC hdc, RECT* prcItem, int day)
{
    int item_w, item_h;
    char daytext[3];
    PMCCOLORINFO pmcci;
    
    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    item_w = prcItem->right - prcItem->left;
    item_h = prcItem->bottom - prcItem->top;
    SetBrushColor (hdc, pmcci->clr_daybk);
    FillBox (hdc, prcItem->left, prcItem->top, item_w, item_h);
    if (day < 10)
        sprintf (daytext, " %d", day);
    else
        sprintf (daytext, "%d", day);
    SetBkColor(hdc, pmcci->clr_dayHibk);
    SetTextColor (hdc, pmcci->clr_daytext);
    mcTextOutCenter (hdc, prcItem, daytext);
}

static void mcHilightRect (PMONCALDDATA mc_data, HDC hdc, RECT* prcItem, int day)
{
    int item_w, item_h;
    char daytext[3];
    PMCCOLORINFO pmcci;
    
    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    SetBrushColor (hdc, pmcci->clr_dayHibk);
    item_w = prcItem->right - prcItem->left;
    item_h = prcItem->bottom - prcItem->top;
    FillBox (hdc, prcItem->left, prcItem->top, item_w, item_h);
    SetPenColor(hdc, GetWindowElementColor(FGC_MENUITEM_FRAME));
    Rectangle (hdc, prcItem->left, prcItem->top, prcItem->left+item_w-1, prcItem->top+item_h-1);
    if (day < 10)
        sprintf (daytext, " %d", day);
    else
        sprintf (daytext, "%d", day);
    SetTextColor (hdc, pmcci->clr_dayHitext);
    mcTextOutCenter (hdc, prcItem, daytext);
}

// draw change day
static void mcDrawDay (HDC hdc, PMONCALDDATA mc_data, RECT* prcMDay, 
                                int newday)
{
    RECT rcPItemDay, rcItemDay;
    PMCCOLORINFO pmcci;
    
    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    mcGetCurRect (prcMDay, &rcPItemDay, mc_data);
    mcUnHilightRect (mc_data, hdc, &rcPItemDay, mc_data->cur_day);    
    mc_data->cur_day = newday;
    mcGetCurDayPos (mc_data, mc_data->cur_day, &mc_data->cur_line, 
                        &mc_data->cur_WeekDay);    
    mcGetCurRect (prcMDay, &rcItemDay, mc_data);
    mcHilightRect (mc_data, hdc, &rcItemDay, mc_data->cur_day);         
}


// draw month day area
static void mcDrawMonthDay (HDC hdc, RECT* prcMDay, PMONCALDDATA mc_data)
{
    int i, WeekDayPM = 0, LineIndex = 0;
    int mdaypm = 0, MonLenPM, mdaynm;
    int iWeekDay = 0;
    char chMonthDay[3];
    RECT rcMonthDay;
    PMCCOLORINFO pmcci;
    
    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    SetBkColor (hdc, pmcci->clr_daybk);    
    SetTextColor (hdc, pmcci->clr_daytext);
    for (i = 1; i <= mc_data->monlen; i++) {
        if (i < 10) 
            sprintf (chMonthDay, " %d", i);
        else
            sprintf (chMonthDay, "%d", i);
        iWeekDay = (mc_data->WeekDay1 + i - 1)%7;
        rcMonthDay.left = prcMDay->left + iWeekDay*mc_data->item_w;
        rcMonthDay.right = rcMonthDay.left + mc_data->item_w;
        LineIndex = (mc_data->WeekDay1 + i - 1)/7;
        rcMonthDay.top = prcMDay->top + mc_data->item_h*LineIndex;
        rcMonthDay.bottom = rcMonthDay.top + mc_data->item_h;
        if (i == mc_data->cur_day) {
            SetBkColor (hdc, pmcci->clr_dayHibk);
            SetTextColor (hdc, pmcci->clr_dayHitext);
            SetBrushColor (hdc, pmcci->clr_dayHibk);
            FillBox (hdc, rcMonthDay.left, rcMonthDay.top, 
                    mc_data->item_w, mc_data->item_h);
            SetPenColor(hdc, GetWindowElementColor(FGC_MENUITEM_FRAME));
            Rectangle (hdc, rcMonthDay.left, rcMonthDay.top, 
                    rcMonthDay.left+mc_data->item_w-1 , 
                    rcMonthDay.top+mc_data->item_h-1);
        }
        mcTextOutCenter (hdc, &rcMonthDay, chMonthDay);
        if (i == mc_data->cur_day) {
            SetBkColor (hdc, pmcci->clr_daybk);
            SetTextColor (hdc, pmcci->clr_daytext);
        }
    }
    
    SetTextColor (hdc, pmcci->clr_trailingtext);
    
    LineIndex += (iWeekDay+1)/7;
    iWeekDay = (iWeekDay + 1)%7;
    mdaynm = 1;
    while (LineIndex <= 5) {
        if (mdaynm < 10) 
            sprintf (chMonthDay, " %d", mdaynm);
        else
            sprintf (chMonthDay, "%d", mdaynm);
        rcMonthDay.left = prcMDay->left + iWeekDay*mc_data->item_w;
        rcMonthDay.right = rcMonthDay.left + mc_data->item_w;
        rcMonthDay.top = prcMDay->top + mc_data->item_h*LineIndex;
        rcMonthDay.bottom = rcMonthDay.top + mc_data->item_h;
        mcTextOutCenter (hdc, &rcMonthDay, chMonthDay);
        mdaynm++;
        iWeekDay++;
        if (iWeekDay == 7) {
            iWeekDay = 0;
            LineIndex++;
        }
    }
    
    WeekDayPM = mc_data->WeekDay1 - 1;
    if (WeekDayPM >= 0) {
        rcMonthDay.top = prcMDay->top;
        rcMonthDay.bottom = rcMonthDay.top + mc_data->item_h;
        rcMonthDay.left = prcMDay->left + WeekDayPM*mc_data->item_w;
        rcMonthDay.right = rcMonthDay.left + mc_data->item_w;
        MonLenPM = GetPMonLen (mc_data->cur_year, mc_data->cur_month);
        mdaypm = MonLenPM;
    }
    while (WeekDayPM >= 0) {
        sprintf (chMonthDay, "%d", mdaypm);
        mcTextOutCenter (hdc, &rcMonthDay, chMonthDay);
        OffsetRect(&rcMonthDay, -mc_data->item_w, 0);
        mdaypm--;
        WeekDayPM--;
    }
}

// draw change month
static void mcDrawMonth (HWND hWnd, HDC hdc, PMONCALDDATA mc_data, RECT* prcMDay, 
                            RECT* prcMonth, int newmonth)
{
    if (newmonth > 12) 
        mc_data->cur_month = 12;
    else if (newmonth < 1) {
        mc_data->cur_month = 1;
    } else 
        mc_data->cur_month = newmonth; 
    mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);
    while (mc_data->cur_day > mc_data->monlen) {
        mc_data->cur_day--;
    }
    mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
    mcGetCurDayPos (mc_data, mc_data->cur_day, &mc_data->cur_line, 
                        &mc_data->cur_WeekDay);    
    InvalidateRect (hWnd, prcMonth, FALSE);
    InvalidateRect (hWnd, prcMDay, FALSE);
}

static void mcDrawYear (HWND hWnd, HDC hdc, PMONCALDDATA mc_data, RECT* prcMDay, 
                            RECT* prcYear, int newyear)
{
    if (newyear < 1902)    mc_data->cur_year = 1902;
    else    mc_data->cur_year = newyear;
    mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);
    while (mc_data->cur_day > mc_data->monlen) {
        mc_data->cur_day--;
    }
    mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
    mcGetCurDayPos (mc_data, mc_data->cur_day, &mc_data->cur_line, 
                        &mc_data->cur_WeekDay);    
    InvalidateRect (hWnd, prcYear, FALSE);
    InvalidateRect (hWnd, prcMDay, FALSE);
}


static const char *chMon_EL[] = {"January", "Febuary", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"};
static const char *chMon_ES[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug",
                            "Sep", "Oct", "Nov", "Dec"};
static const char *chMon_C[] = {"一月", "二月", "三月", "四月", "五月", "六月", "七月", "八月", 
                            "九月", "十月", "十一月", "十二月"};
static const char *chWeek_C[] = {"日", "一", "二", "三", "四", "五", "六"}; 
static const char *chWeek_E[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *chWeek_E_S[] = {"S", "1", "2", "3", "4", "5", "6"};

// draw the whole calendar
static void mcDrawCalendar (HWND hWnd, HDC hdc, PMONCALDDATA mc_data)
{
    RECT rcClient, rcMonth, rcYear, rcMDay;
    RECT rcAML, rcAMR, rcAYL, rcAYR;
    DWORD dwStyle;
    PMCCOLORINFO pmcci;
    //int bk_color = GetWindowBkColor (hWnd);
    UINT uFormat;
    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    dwStyle = GetWindowStyle (hWnd);
    mcGetRects (hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay);
    
#ifdef  _FLAT_WINDOW_STYLE
    DrawFlatControlFrameEx (hdc, rcClient.left, rcClient.top, rcClient.right,
            rcClient.bottom, pmcci->clr_daybk, 1, -1);
    DrawFlatControlFrameEx (hdc, rcClient.left+2, rcClient.top+2,
                            rcClient.right-2, rcClient.top+2+DATECAP_H(hWnd),
                            pmcci->clr_titlebk, 1, -1);
#else
/*
    mcDraw3DMCDownFrame (hdc, rcClient.left, rcClient.top, rcClient.right,
            rcClient.bottom, pmcci->clr_daybk);
*/
    Draw3DDownFrame (hdc, rcClient.left, rcClient.top, rcClient.right,
            rcClient.bottom, pmcci->clr_daybk);
    Draw3DUpFrame (hdc, rcClient.left+2, rcClient.top+2,
                        rcClient.right-2, rcClient.top+2+DATECAP_H(hWnd), 
                        pmcci->clr_titlebk);
#endif
    mcGetArrowRects (&rcMonth, &rcYear, &rcAML, &rcAMR, &rcAYL, &rcAYR);
    mcDrawPageArrow (mc_data, hdc, &rcAML, FALSE, FALSE);
    mcDrawPageArrow (mc_data, hdc, &rcAMR, TRUE, FALSE);
    mcDrawPageArrow (mc_data, hdc, &rcAYL, FALSE, FALSE);
    mcDrawPageArrow (mc_data, hdc, &rcAYR, TRUE, FALSE);
    {
        char chYear[5];
            const char* pchMon;
        
        uFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
        if ((dwStyle & MCS_LANG) == MCS_CHN)
            pchMon = chMon_C[mc_data->cur_month-1];
        else if ((dwStyle & MCS_LANG) == MCS_ENG_L)
            pchMon = chMon_EL[mc_data->cur_month-1];
        else if ((dwStyle & MCS_LANG) == MCS_ENG_S)
            pchMon = chMon_ES[mc_data->cur_month-1];
        else
            pchMon = chMon_C[mc_data->cur_month-1];

        mcTextOutCenter (hdc, &rcMonth, pchMon);

        sprintf(chYear, "%d", mc_data->cur_year);
        mcTextOutCenter (hdc, &rcYear, chYear);
    }
    // draw weekday caption
    {
        int i, weekitem_w;
        const char *pchDay;
        RECT rcWeek, rcWeekItem;
        
        rcWeek.left = rcClient.left + WEEK_BORDER;
        rcWeek.right = rcClient.right-WEEK_BORDER;
        rcWeek.top = rcClient.top + 2 + WEEK_VBORDER1 + DATECAP_H(hWnd);
        rcWeek.bottom = rcWeek.top + WEEKFIELD_H(hWnd);
        weekitem_w = (rcWeek.right - rcWeek.left)/7;
        rcWeekItem.left = rcWeek.left;
        rcWeekItem.right = rcWeekItem.left + weekitem_w;
        rcWeekItem.top = rcWeek.top;
        rcWeekItem.bottom = rcWeek.bottom;

        SetBrushColor (hdc, pmcci->clr_weekcaptbk);
        FillBox (hdc, rcWeek.left, rcWeek.top, rcWeek.right-rcWeek.left,
                    rcWeek.bottom-rcWeek.top);

        SetBkColor (hdc, pmcci->clr_weekcaptbk);
        SetTextColor (hdc, pmcci->clr_weekcapttext);
        for (i = 0; i <= 6; i++) {
            //RECT tmp;
            if ((dwStyle & MCS_LANG) == MCS_CHN) {
                pchDay = chWeek_C[i];
            }
            else if ((dwStyle & MCS_LANG) == MCS_ENG_L)
                pchDay = chWeek_E[i];
            else if ((dwStyle & MCS_LANG) == MCS_ENG_S)
                pchDay = chWeek_E_S[i];
            else {
                pchDay = chWeek_C[i];
            }

            //tmp = rcWeekItem;
            TextOut (hdc, rcWeekItem.left+11, rcWeekItem.top+1, pchDay);
            //mcTextOutCenter (hdc, &rcWeekItem, pchDay);
            //mcTextOutCenter (hdc, &tmp, pchDay);
            OffsetRect (&rcWeekItem, weekitem_w, 0);
        }
    }
    // draw month day text
    {
        mcDrawMonthDay (hdc, &rcMDay, mc_data);
    }
}

// initialize mc_data  
static BOOL mcInitMonthCalendarData (HWND hWnd, MONCALDDATA* mc_data)
{
    time_t nowtime;
    struct tm *pnt;
    RECT rcClient, rcMDay;
    PMCCOLORINFO pmcci;
    
    time(&nowtime);
    pnt = localtime(&nowtime);
    mc_data->sys_year = mc_data->cur_year = pnt->tm_year + 1900;
    mc_data->sys_month = mc_data->cur_month = pnt->tm_mon + 1;
    mc_data->sys_day = mc_data->cur_day = pnt->tm_mday;
    
    mcGetRects (hWnd, &rcClient, NULL, NULL, &rcMDay);
    mc_data->item_w = (rcMDay.right - rcMDay.left) / 7;
    mc_data->item_h = (rcMDay.bottom - rcMDay.top) / 6;
    mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
    mcGetCurDayPos (mc_data, mc_data->cur_day,
                    &mc_data->cur_line, &mc_data->cur_WeekDay);    
    mc_data->sys_WeekDay = mc_data->cur_WeekDay;    
    mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);

    // color info
    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    pmcci->clr_titlebk         = MCCLR_DF_TITLEBK;
    pmcci->clr_titletext     = MCCLR_DF_TITLETEXT;
    pmcci->clr_arrow         = MCCLR_DF_ARROW;
    pmcci->clr_arrowHibk     = MCCLR_DF_ARROWHIBK;
    pmcci->clr_daybk        = MCCLR_DF_DAYBK;
    pmcci->clr_dayHibk        = MCCLR_DF_DAYHIBK;
    pmcci->clr_daytext        = MCCLR_DF_DAYTEXT;
    pmcci->clr_trailingtext    = MCCLR_DF_TRAILINGTEXT;
    pmcci->clr_dayHitext    = MCCLR_DF_DAYHITEXT;
    pmcci->clr_weekcaptbk    = MCCLR_DF_WEEKCAPTBK;
    pmcci->clr_weekcapttext = MCCLR_DF_WEEKCAPTTEXT;
    return TRUE;
}

/********************************************************************************/

static int MonthCalendarCtrlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    //PCONTROL pCtrl;
    PMONCALDDATA    mc_data;
    DWORD dwStyle = GetWindowStyle (hWnd);
    
    //pCtrl    = Control (hWnd);

    switch (message) {
        case MSG_CREATE:
        {
            RECT rcWnd;
            int rc_w, rc_h;
            PMCCOLORINFO pmcci;
            
            GetWindowRect (hWnd, &rcWnd);
            rc_w = rcWnd.right - rcWnd.left;
            rc_h = rcWnd.bottom - rcWnd.top;
            if (rc_w < MINWNDRECT_W) {
                rc_w = MINWNDRECT_W;
                MoveWindow (hWnd, rcWnd.left, rcWnd.top, rc_w, 
                                rc_h, FALSE);
            }
            if (rc_h < MINWNDRECT_H) {
                rc_h = MINWNDRECT_H;
                MoveWindow (hWnd, rcWnd.left, rcWnd.top, rc_w, 
                                rc_h, FALSE);
            }
            
            if ((mc_data = (MONCALDDATA *) malloc (sizeof(MONCALDDATA))) == NULL)
                return -1;
            SetWindowAdditionalData2 (hWnd, (DWORD) mc_data);
            if ((pmcci = (MCCOLORINFO*) malloc (sizeof(MCCOLORINFO))) == NULL) {
                free (mc_data);
                return -1;
            }
            mc_data->dwClrData = (DWORD) pmcci;
            if (!mcInitMonthCalendarData (hWnd, mc_data)) {
                free (pmcci);
                free (mc_data);
                return -1;
            }
        }
            break;
            
        case MCM_GETCURDAY:
        {
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            return mc_data->cur_day;
        }
            break;
            
        case MCM_GETCURMONTH:
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            return mc_data->cur_month;
            break;
            
        case MCM_GETCURYEAR:
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            return mc_data->cur_year;
            break;
            
        case MCM_GETCURMONLEN:
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            return mc_data->monlen;
            break;
        
        case MCM_SETCURDAY:
        {
            int newday = (int) wParam;
            RECT rcClient, rcMDay;
            
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            if (newday < 1) 
                newday = 1;
            else if (newday > mc_data->monlen) 
                newday = mc_data->monlen;
            mcGetRects (hWnd, &rcClient, NULL, NULL, &rcMDay);
            hdc = GetClientDC (hWnd);
            mcDrawDay (hdc, mc_data, &rcMDay, newday);
            ReleaseDC (hdc);
            // draw day
        }
            break;
            
        case MCM_SETCURMONTH:
        {
            int newmonth = (int) wParam;
            RECT rcClient, rcMonth, rcMDay;
            
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            if (newmonth > 12)
                newmonth = 12;
            else if (newmonth < 1) 
                newmonth = 1;
            mcGetRects (hWnd, &rcClient, &rcMonth, NULL, &rcMDay);
            hdc = GetClientDC (hWnd);
            mcDrawMonth (hWnd, hdc, mc_data, &rcMDay, &rcMonth, newmonth);
            ReleaseDC (hdc);
        }
            break;
            
        case MCM_SETCURYEAR:
        {
            int newyear = (int) wParam;
            RECT rcClient, rcYear, rcMDay;

            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            if (newyear < 1902)    newyear = 1902;
            mcGetRects (hWnd, &rcClient, NULL, &rcYear, &rcMDay);
            hdc = GetClientDC (hWnd);
            mcDrawYear (hWnd, hdc, mc_data, &rcMDay, &rcYear, newyear);
            ReleaseDC (hdc);
        }
            break;
        
        case MCM_SETTODAY:
        {
            RECT rcClient, rcYear, rcMonth, rcMDay;
            
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            mcGetRects (hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay);
            mc_data->cur_day = mc_data->sys_day;
            mc_data->cur_month = mc_data->sys_month;
            mc_data->cur_year = mc_data->sys_year;
            mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
            mcGetCurDayPos (mc_data, mc_data->cur_day,
                        &mc_data->cur_line, &mc_data->cur_WeekDay);    
            mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);

            InvalidateRect (hWnd, &rcYear, FALSE);
            InvalidateRect (hWnd, &rcMonth, FALSE);
            InvalidateRect (hWnd, &rcMDay, FALSE);
        }
            break;

        case MCM_SETCURDATE:
        {
            RECT rcClient, rcYear, rcMonth, rcMDay;
            PSYSTEMTIME pcurdate = NULL;

            mcGetRects (hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay);
            pcurdate = (PSYSTEMTIME) lParam;
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            mc_data->cur_year = pcurdate->year;
            mc_data->cur_month = pcurdate->month;
            mc_data->cur_day = pcurdate->day;
            mc_data->WeekDay1 = GetWeekDay (mc_data->cur_year, mc_data->cur_month, 1);
            mcGetCurDayPos (mc_data, mc_data->cur_day, 
                    &mc_data->cur_line, &mc_data->cur_WeekDay);
            mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);
            
            InvalidateRect (hWnd, &rcYear, FALSE);
            InvalidateRect (hWnd, &rcMonth, FALSE);
            InvalidateRect (hWnd, &rcMDay, FALSE);
        }
            break;
            

        case MCM_GETFIRSTWEEKDAY:
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            return mc_data->WeekDay1;
            break;
            
        case MCM_GETCURDATE:
        {
            PSYSTEMTIME pcurdate = NULL;

            pcurdate = (PSYSTEMTIME) lParam;
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            pcurdate->year = mc_data->cur_year;
            pcurdate->month = mc_data->cur_month;
            pcurdate->day = mc_data->cur_day;
            pcurdate->weekday = mc_data->cur_WeekDay;
        }
            break;
            
        case MCM_GETTODAY:
        {
            PSYSTEMTIME ptoday = NULL;

            ptoday = (PSYSTEMTIME) lParam;
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            ptoday->year = mc_data->sys_year;
            ptoday->month = mc_data->sys_month;
            ptoday->day = mc_data->sys_day;
            ptoday->weekday = mc_data->sys_WeekDay;
        }
            break;
            
        case MCM_GETMINREQRECTW:
            return MINWNDRECT_W;    
            break;
            
        case MCM_GETMINREQRECTH:
            return MINWNDRECT_H;
            break;
            
        case MCM_GETCOLOR:
        {
            PMCCOLORINFO pmcci, pmccinfo;

            pmccinfo = (PMCCOLORINFO) lParam;
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            pmcci = (PMCCOLORINFO) mc_data->dwClrData;
            
            pmccinfo->clr_titlebk         = pmcci->clr_titlebk;
            pmccinfo->clr_titletext     = pmcci->clr_titletext;
            pmccinfo->clr_arrow         = pmcci->clr_arrow;
            pmccinfo->clr_arrowHibk     = pmcci->clr_arrowHibk;
            pmccinfo->clr_daybk            = pmcci->clr_daybk;
            pmccinfo->clr_dayHibk        = pmcci->clr_dayHibk;
            pmccinfo->clr_daytext        = pmcci->clr_daytext;
            pmccinfo->clr_trailingtext    = pmcci->clr_trailingtext;
            pmccinfo->clr_dayHitext        = pmcci->clr_dayHitext;
            pmccinfo->clr_weekcaptbk    = pmcci->clr_weekcaptbk;
            pmccinfo->clr_weekcapttext     = pmcci->clr_weekcapttext;
        }
            break;
            
        case MCM_SETCOLOR:
        {
            PMCCOLORINFO pmcci, pmcci_new;
            
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            pmcci = (PMCCOLORINFO) mc_data->dwClrData;
            pmcci_new = (PMCCOLORINFO) lParam;
            
            if (pmcci_new->clr_titlebk >= 0) 
                pmcci->clr_titlebk         = pmcci_new->clr_titlebk;
            if (pmcci_new->clr_titletext >= 0)
                pmcci->clr_titletext     = pmcci_new->clr_titletext;
            if (pmcci->clr_arrow >= 0)
                pmcci->clr_arrow         = pmcci_new->clr_arrow;
            if (pmcci->clr_arrowHibk >= 0)
                pmcci->clr_arrowHibk     = pmcci_new->clr_arrowHibk;
            if (pmcci->clr_daybk >= 0)
                pmcci->clr_daybk        = pmcci_new->clr_daybk;
            if (pmcci->clr_dayHibk >= 0)
                pmcci->clr_dayHibk        = pmcci_new->clr_dayHibk;
            if (pmcci->clr_daytext >= 0)
                pmcci->clr_daytext        = pmcci_new->clr_daytext;
            if (pmcci->clr_trailingtext >= 0)
                pmcci->clr_trailingtext    = pmcci_new->clr_trailingtext;
            if (pmcci->clr_dayHitext >= 0)
                pmcci->clr_dayHitext    = pmcci_new->clr_dayHitext;
            if (pmcci->clr_weekcaptbk >= 0)
                pmcci->clr_weekcaptbk    = pmcci_new->clr_weekcaptbk;
            if (pmcci->clr_weekcapttext >= 0) 
                pmcci->clr_weekcapttext = pmcci_new->clr_weekcapttext;
            
            InvalidateRect (hWnd, NULL, FALSE);    
        }
            break;
            
        case MSG_DESTROY:
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            mcMonthCalendarCleanUp (mc_data);
            free (mc_data);    
            break;
            
            /*
        case MSG_MOUSEMOVE:
        {
            int x, y;
            RECT rcClient, rcMonth, rcYear, rcMDay;
            RECT rcAML, rcAMR, rcAYL, rcAYR;
            
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);

            x = LOSWORD (lParam);
            y = HISWORD (lParam);
            mcGetRects (hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay);
            mcGetArrowRects (&rcMonth, &rcYear, &rcAML, &rcAMR, &rcAYL, &rcAYR);
                
            if (PtInRect (&rcAMR, x, y)) {
                hdc = GetClientDC (hWnd);
                mcDrawPageArrow (mc_data, hdc, &rcAMR, TRUE, TRUE);
                ReleaseDC (hdc);
            } 
            else if (PtInRect (&rcAML, x, y)) {
                hdc = GetClientDC (hWnd);
                mcDrawPageArrow (mc_data, hdc, &rcAML, FALSE, TRUE);
                ReleaseDC (hdc);
            }
            else if (PtInRect (&rcAYR, x, y)) {
                hdc = GetClientDC (hWnd);
                mcDrawPageArrow (mc_data, hdc, &rcAYR, TRUE, TRUE);
                ReleaseDC (hdc);
            }
            else if (PtInRect (&rcAYL, x, y)) {
                hdc = GetClientDC (hWnd);
                mcDrawPageArrow (mc_data, hdc, &rcAYL, FALSE, TRUE);
                ReleaseDC (hdc);
            }
            else {
                hdc = GetClientDC (hWnd);
                mcDrawPageArrow (mc_data, hdc, &rcAMR, TRUE, FALSE);
                mcDrawPageArrow (mc_data, hdc, &rcAML, FALSE, FALSE);
                mcDrawPageArrow (mc_data, hdc, &rcAYR, TRUE, FALSE);
                mcDrawPageArrow (mc_data, hdc, &rcAYL, FALSE, FALSE);
                ReleaseDC (hdc);
            }
        }
            break;
            */
            
        case MSG_LBUTTONDOWN:
        {
            int x, y;
            RECT rcClient, rcMonth, rcYear, rcMDay;
            RECT rcAML, rcAMR, rcAYL, rcAYR;
            
            x = LOSWORD (lParam);
            y = HISWORD (lParam);
            mcGetRects (hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay);
            mcGetArrowRects (&rcMonth, &rcYear, &rcAML, &rcAMR, &rcAYL, &rcAYR);

            InflateRect (&rcAML, 6, 2);
            InflateRect (&rcAMR, 6, 2);
            InflateRect (&rcAYR, 6, 2);
            InflateRect (&rcAYL, 6, 2);

            if (PtInRect (&rcAMR, x, y)) {
                if (dwStyle & MCS_NOTIFY)
                    NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DATECHANGE);        
                mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
                if (mc_data->cur_month != 12) 
                    mc_data->cur_month++;
                else {
                    mc_data->cur_month = 1; 
                    mc_data->cur_year++;
                    InvalidateRect (hWnd, &rcYear, FALSE);
                }
                mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);
                while (mc_data->cur_day > mc_data->monlen) {
                    mc_data->cur_day--;
                }
                mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
                mcGetCurDayPos (mc_data, mc_data->cur_day,
                    &mc_data->cur_line, &mc_data->cur_WeekDay);    
                InvalidateRect (hWnd, &rcMonth, FALSE);
                InvalidateRect (hWnd, &rcMDay, FALSE);
            }
            else if (PtInRect (&rcAML, x, y)) {
                if (dwStyle & MCS_NOTIFY)
                    NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DATECHANGE);        
                mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
                if (mc_data->cur_month != 1) 
                    mc_data->cur_month--;
                else {
                    if (mc_data->cur_year <= 1902)
                        break;
                    mc_data->cur_month = 12;
                    mc_data->cur_year--;
                    InvalidateRect (hWnd, &rcYear, FALSE);
                }
                mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);
                while (mc_data->cur_day > mc_data->monlen) {
                    mc_data->cur_day--;
                }
                mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
                mcGetCurDayPos (mc_data, mc_data->cur_day,
                    &mc_data->cur_line, &mc_data->cur_WeekDay);    
                InvalidateRect (hWnd, &rcMonth, FALSE);
                InvalidateRect (hWnd, &rcMDay, FALSE);
            }
            else if (PtInRect (&rcAYR, x, y)) {
                if (dwStyle & MCS_NOTIFY)
                    NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DATECHANGE);        
                mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
                mc_data->cur_year++;
                mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);
                while (mc_data->cur_day > mc_data->monlen) {
                    mc_data->cur_day--;
                }
                mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
                mcGetCurDayPos (mc_data, mc_data->cur_day,
                    &mc_data->cur_line, &mc_data->cur_WeekDay);    
                InvalidateRect (hWnd, &rcYear, FALSE);
                InvalidateRect (hWnd, &rcMDay, FALSE);
                //InvalidateRect (hWnd, NULL, FALSE);
            }
            else if (PtInRect (&rcAYL, x, y)) {
                if (dwStyle & MCS_NOTIFY)
                    NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DATECHANGE);        
                mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
                if (mc_data->cur_year <= 1902)
                    break;
                mc_data->cur_year--;
                mc_data->monlen = GetMonLen (mc_data->cur_year, mc_data->cur_month);
                while (mc_data->cur_day > mc_data->monlen) {
                    mc_data->cur_day--;
                }
                mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, mc_data->cur_month, 1);
                mcGetCurDayPos (mc_data, mc_data->cur_day,
                    &mc_data->cur_line, &mc_data->cur_WeekDay);    
                InvalidateRect (hWnd, &rcYear, FALSE);
                InvalidateRect (hWnd, &rcMDay, FALSE);
                //InvalidateRect (hWnd, NULL, FALSE);
            }
                
            else if (PtInRect (&rcMDay, x, y)) {
                int line, weekday, pline, pweekday, posindex;
                RECT rcItemDay; //rcPItemDay;
                
                mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
                
                pline = mc_data->cur_line;
                pweekday = mc_data->cur_WeekDay;
                mcGetItemRectFromPos (mc_data, &rcMDay, 
                                        &rcItemDay, &line, &weekday, x, y);
                posindex = line * 7 + weekday + 1;
                if (posindex < mc_data->WeekDay1+1) {
                    if (dwStyle & MCS_NOTIFY)
                        NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DATECHANGE);    
                    if (mc_data->cur_month != 1) 
                        mc_data->cur_month--;
                    else {
                        if (mc_data->cur_year <= 1902)
                            break;
                        mc_data->cur_month = 12;
                        mc_data->cur_year--;
                        InvalidateRect (hWnd, &rcYear, FALSE);
                    }
                    mc_data->monlen = GetMonLen (mc_data->cur_year, 
                                                 mc_data->cur_month);
                    while (mc_data->cur_day > mc_data->monlen) {
                        mc_data->cur_day--;
                    }
                    mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, 
                                                   mc_data->cur_month, 1);
                    mcGetCurDayPos (mc_data, mc_data->cur_day,
                        &mc_data->cur_line, &mc_data->cur_WeekDay);    
                    InvalidateRect (hWnd, &rcMonth, FALSE);
                    InvalidateRect (hWnd, &rcMDay, FALSE);
                }
                else if (posindex >mc_data->monlen + mc_data->WeekDay1) {
                    if (dwStyle & MCS_NOTIFY)
                        NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DATECHANGE);    
                    if (mc_data->cur_month != 12) 
                        mc_data->cur_month++;
                    else {
                        mc_data->cur_month = 1; 
                        mc_data->cur_year++;
                        InvalidateRect (hWnd, &rcYear, FALSE);
                    }
                    mc_data->monlen = GetMonLen (mc_data->cur_year, 
                                                 mc_data->cur_month);
                    while (mc_data->cur_day > mc_data->monlen) {
                        mc_data->cur_day--;
                    }
                    mc_data->WeekDay1 = GetWeekDay(mc_data->cur_year, 
                                                   mc_data->cur_month, 1);
                    mcGetCurDayPos (mc_data, mc_data->cur_day,
                            &mc_data->cur_line, &mc_data->cur_WeekDay);    
                    InvalidateRect (hWnd, &rcMonth, FALSE);
                    InvalidateRect (hWnd, &rcMDay, FALSE);
                }
                else if (line != pline || weekday != pweekday) {
                    int newday;
                    
                    if (dwStyle & MCS_NOTIFY)
                        NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DATECHANGE);        
                    if (dwStyle & MCS_NOTIFY)
                        NotifyParent (hWnd, GetDlgCtrlID (hWnd), MCN_DAYCHANGE);
                    hdc = GetClientDC (hWnd);
                    newday = (line*7+weekday+1) - (mc_data->WeekDay1+1) + 1;
                    mcDrawDay (hdc, mc_data, &rcMDay, newday);
                    ReleaseDC (hdc);
                }
            }
        }    
            break;
            
        case MSG_LBUTTONUP:
            break;

        case MSG_PAINT:
            hdc = BeginPaint (hWnd);
            mc_data = (PMONCALDDATA) GetWindowAdditionalData2 (hWnd);
            mcDrawCalendar (hWnd, hdc, mc_data);
            EndPaint (hWnd, hdc);
            return 0;
            
        default:
            break;
    }

    return DefaultControlProc (hWnd, message, wParam, lParam);
}

#endif /* _EXT_CTRL_MONTHCAL */

