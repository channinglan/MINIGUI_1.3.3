/*
** $Id: listview.h,v 1.28 2003/09/04 06:12:04 weiym Exp $ 
**
** listview.h: header file of ListView control.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2002 Amokaqi chenjm kevin.
** Copyright (C) 2001, 2002 Shu Ming.
**
*/

#ifndef _LIST_VIEW_H
#define _LIST_VIEW_H

#ifdef __cplusplus
extern "C"
{
#endif

#define LV_ROW_MAX      	100000      // maximum item number

#define LV_COLW_DEF	        62  // default column width

//default header height
#define LV_HDRH_DEF(hwnd)       (GetWindowFont(hwnd)->size + 6)
//default item height
#define LV_ITEMH_DEF(hwnd)      (GetWindowFont(hwnd)->size + 6)

#define LV_HDR_TOP		0   // top of the header

#define COLWIDTHMIN             10  // minimum column width
#define HSCROLL                 5   // h scroll value
#define VSCROLL                 15  // v scroll value

#define LIGHTBLUE	        ( RGB2Pixel(HDC_SCREEN, 0, 0, 180) )
#define GRAYBLUE	        ( RGB2Pixel(HDC_SCREEN, 156, 166, 189) )

typedef enum sorttype
{
     NOTSORTED = 0, 
     HISORTED, 
     LOSORTED
} SORTTYPE;

#define LVIF_NORMAL	0x0000L
#define LVIF_BITMAP  	0x0001L
#define LVIF_ICON  	0x0002L

typedef struct _subitemdata
{
    struct _subitemdata *pNext;  // points to the next subitem
    DWORD  dwFlags;              // subitem flags
    char   *pszInfo;             // text of the subitem
    int    nTextColor;           // text color of the subitem
    DWORD  Image;                // image of the subitem
} SUBITEMDATA;
typedef SUBITEMDATA *PSUBITEMDATA;

typedef struct _itemdata
{
    struct _itemdata *pNext;      // points to the next item
    BOOL         bSelected;       // item is selected
    PSUBITEMDATA pSubItemHead;    // points to the subitem list
    DWORD addData;
} ITEMDATA;
typedef ITEMDATA *PITEMDATA;

/* column header struct */
typedef struct _lsthdr
{
    struct _lsthdr *pNext;        // points to the next header
    int x;                        // x position of the header
    int width;                    // width of the header/column/subitem
    SORTTYPE sort;                // sort status
    char *pTitle;                 // title text of the column header
    PFNLVCOMPARE pfnCmp;          // pointer to the application-defined or default
                                  // comparision function
    DWORD  Image;                 // image of the header
    DWORD flags;                  // header and column flags
} LSTHDR;
typedef LSTHDR *PLSTHDR;

#define LVST_NORMAL	0x0000    //normal status
#define LVST_BDDRAG	0x0001    //the border is being dragged
#define LVST_HEADCLICK	0x0002    //the header is being clicked
#define LVST_INHEAD	0x0004    //mouse move in header

/* this macro doesn't check the pointer, so, be careful */
#define LVSTATUS(hwnd)  ( ((PLVDATA)GetWindowAdditionalData2(hwnd))->status )

typedef struct _lstvwdata
{
    int nItemHeight;             // item height
    int nHeadHeight;             // header height
    int bkc_selected;            // background color of the selected item

    int nCols;                   // current column number
    int nRows;                   // current item number
    int nOriginalX;              // scroll x pos
    int nOriginalY;              // scroll y pos

    int nItemSelected;           // index of the selected item
    int nColCurSel;	         // current column selected.

    int nItemDraged;             // the header beging dragged
    DWORD status;                // list view status: dragged, clicked
    PLSTHDR pHdrClicked;         // the header being clicked

    PLSTHDR pLstHead;            // points to the header list
    PITEMDATA pItemHead;         // points to the item list
    HWND hWnd;                   // the control handle

    STRCMP str_cmp;              // default strcmp function
} LVDATA;
typedef LVDATA *PLVDATA;

BOOL RegisterListViewControl (void);
void ListViewControlCleanup (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _LIST_VIEW_H */

