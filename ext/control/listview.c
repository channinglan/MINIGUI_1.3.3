/*
** $Id: listview.c,v 1.76.8.1 2004/04/30 01:21:48 snig Exp $
**
** listview.c: the ListView control
**
** Copyright (C) 2003 Feynman Software.
** 
** 2003/05/17: Rewritten by Zhong Shuyi.
**
** Create date: 2001/12/03
**
** Original authors: Shu Ming, Amokaqi, chenjm, kevin.
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

/*
** Modify records:
**
**  Who             When        Where       For What                Status
**-----------------------------------------------------------------------------
**  amokaqi  2002/9/26        add CURSORBLOCKDOWN and UP            finish
**  amokaqi  2002/10/8        add KILLFOCUS and SETFOCUS            finish
**  amokaqi  2002/10/8        add GETDLGCODE for dialog             finish
**  chenjm   2002/10/24       add LVM_SELECTITEM and LVM_SHOWITEM   finish
**  kevin    2003/01/03       modify KILLFOCUS and SETFOCUS         finish
**  snig     2003/05/06       add SB_PAGELEFT, SB_PAGERIGHT         finish
**                                SB_PAGEUP and SB_PAGEDOWN;
**  snig     2003/05/13       code cleanup, rewrite ...             finish
**
**  snig     2003/05/15       entirely new listview interfaces      done
**-----------------------------------------------------------------------------
**
** TODO:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#ifdef _EXT_CTRL_LISTVIEW

#include "mgext.h"
#include "listview.h"

/********************** internals functions declaration **********************/

static int itemDelete (PITEMDATA pHead);

#define sGetItemFromList(nSeq, plvdata)  lvGetItemByRow(plvdata, nSeq)

inline static int sGetFrontSubItemsWidth (int end, PLVDATA plvdata);
static int sGetSubItemWidth (int nCols, PLVDATA plvdata);
static int sAddOffsetToTailSubItem (int nCols, int offset,
                                    PLVDATA plvdata);

static int sGetItemWidth (PLVDATA plvdata);
static PITEMDATA lvGetItemByRow (PLVDATA plvdata, int nRows);
static int lstSetVScrollInfo (PLVDATA plvdata);
static int lstSetHScrollInfo (PLVDATA plvdata);
static PSUBITEMDATA lvGetSubItemByCol (PITEMDATA pItem, int nCols);
static PLSTHDR lvGetHdrByCol (PLVDATA plvdata, int nCols);

/* ========================================================================= 
 * size and position operation section of the listview control.
   ========================================================================= */

#define LV_HDR_HEIGHT        (plvdata->nHeadHeight)

#define LV_ITEM_Y(nRows)     ((nRows-1) * plvdata->nItemHeight)
#define LV_ITEM_YY(nRows)    ( LV_ITEM_Y(nRows) - plvdata->nOriginalY )
#define LV_ITEM_YYH(nRows)    ( LV_ITEM_YY(nRows) + plvdata->nHeadHeight)

#define sGetSubItemX(nCols)   ((lvGetHdrByCol(plvdata, nCols))->x)

/* Gets the rect of an item */
#define LV_GET_ITEM_RECT(nRow, rect)        \
	    GetClientRect(plvdata->hWnd, &rect);     \
            rect.top = LV_ITEM_YYH(nRow);    \
            rect.bottom = rect.top + plvdata->nItemHeight;

/* Gets the rect of a subitem */
#define LV_GET_SUBITEM_RECT(nRows, nCols, rect)  \
		LV_GET_ITEM_RECT(nRows, rect);                   \
		rect.left = sGetSubItemX(nCols) - plvdata->nOriginalX;  \
	        rect.right = rect.left + sGetSubItemWidth(nCols, plvdata); 

/* Gets the text rect of a subitem */
#define LV_GET_SUBITEM_TEXTRECT(rect, textrect)  \
                  textrect.left = rect.left + 2;     \
		  textrect.top = rect.top + 2;       \
		  textrect.right = rect.right - 2;   \
		  textrect.bottom = rect.bottom - 2;

#define LV_GET_HDR_RECT(p1, rect)      \
                  rect.left = p1->x - plvdata->nOriginalX + 1;   \
		  rect.right = p1->x + p1->width - plvdata->nOriginalX - 1;  \
		  rect.top = LV_HDR_TOP;     \
		  rect.bottom = plvdata->nHeadHeight;
#if 0
#define LV_GET_BD_RECT(p1, rect)      \
                  rect.left = p1->x + p1->width - plvdata->nOriginalX - 1;   \
		  rect.right = p1->x + p1->width - plvdata->nOriginalX + 1;  \
		  rect.top = 0;     \
		  rect.bottom = plvdata->nHeadHeight;

/* Gets the rect of a column */
#define LV_GET_COL_RECT(nCols, rect)  \
	    GetClientRect(plvdata->hWnd, &rect);  \
	    rect.left = sGetSubItemX (nCols) - plvdata->nOriginalX;    \
	    rect.right = rect.left + sGetSubItemWidth (nCols, plvdata);
#endif


#define LV_BE_VALID_COL(nPosition)    (nPosition <= plvdata->nCols && nPosition >= 1)
#define LV_BE_VALID_ROW(nPosition)    (nPosition <= plvdata->nRows && nPosition >= 1)

#define LV_H_OUTWND(plvdata, rcClient)    \
               ( sGetItemWidth (plvdata) - 2 > rcClient.right - rcClient.left)

inline static BOOL 
lvBeInHeadBorder (int mouseX, int mouseY, PLSTHDR p1, PLVDATA plvdata)
{
	/*
	RECT rect;
	LV_GET_BD_RECT(p1, rect);
	if (PtInRect (&rect, mouseX, mouseY))
	*/
        if ( (mouseX >= (p1->x + p1->width - plvdata->nOriginalX - 1))
             && (mouseX <= (p1->x + p1->width - plvdata->nOriginalX))
             && (mouseY >= 0) && (mouseY <= plvdata->nHeadHeight) )
	    return TRUE;
	return FALSE;
}

static int 
lvInWhichHeadBorder (int mouseX, int mouseY, PLSTHDR * pRet, PLVDATA plvdata)
{
    int nPosition = 0;
    PLSTHDR p1 = plvdata->pLstHead;

    while (p1 != NULL)
    {
        nPosition++;

	if (lvBeInHeadBorder(mouseX, mouseY, p1, plvdata))
            break;
    
        p1 = p1->pNext;
    }
    
    if (!p1) {
        if (pRet)
            *pRet = NULL;
        return -1;
    }

    if (pRet)
        *pRet = p1;
        
    return nPosition;
}

static int 
isInListViewHead (int mouseX, int mouseY, PLSTHDR * pRet, PLVDATA plvdata)
{
    int nPosition = 0;
    PLSTHDR p1 = plvdata->pLstHead;
    RECT rect;

    while (p1)
    {
        nPosition++;

	LV_GET_HDR_RECT(p1, rect);
	if (PtInRect (&rect, mouseX, mouseY))
            break;
        
        p1 = p1->pNext;
    }

    //not in head
    if (!p1 || (nPosition > plvdata->nCols) || (nPosition == 0)) {
        if (pRet)
	    *pRet = NULL;
        return -1;
    }

    //in head
    if (pRet)
        *pRet = p1;
    return nPosition;
}

static int isInLVItem (int mouseX, int mouseY, PITEMDATA * pRet, PLVDATA plvdata)
{
    int ret, j;
    PITEMDATA p1;

    if ((mouseY < plvdata->nHeadHeight))
        return -1;

    ret = (mouseY + plvdata->nOriginalY - plvdata->nHeadHeight) / plvdata->nItemHeight;
    ret++;
    *pRet = NULL;

    p1 = plvdata->pItemHead;
    j = 0;

    while ((p1 != NULL) && (j < ret))
    {
        *pRet = p1;
        p1 = p1->pNext;
        j++;
    }
    
    if (ret > j)
        return -1;
    
    return ret;
}

/* ========================================================================= 
 * Drawing section of the listview control.
   ========================================================================= */

static void sDrawText (HDC hdc, int x, int y, int width, int height, 
		const char *pszText, UINT format)
{
    RECT rect;
    //SIZE size;

    if (pszText != NULL)
    {
	SetRect (&rect, x+2, y+2, x+width, y+height);
        DrawText (hdc, pszText, -1, &rect, format);
    }
}

static void lvDrawSubItem (HDC hdc, int nRows, int nCols, PLVDATA plvdata)
{
    RECT rect, rect_text;
    PITEMDATA pItem;
    PSUBITEMDATA psub;
    PLSTHDR ph;
    UINT text_format;

    if ( !(pItem = lvGetItemByRow(plvdata, nRows)) )
	return;
    if ( !(psub = lvGetSubItemByCol(pItem, nCols)) )
	return;

    ph = lvGetHdrByCol (plvdata, nCols);
    LV_GET_SUBITEM_RECT(nRows, nCols, rect);

    if (!pItem->bSelected) {
	SetBrushColor (hdc, GetWindowBkColor(plvdata->hWnd));
	SetBkColor (hdc, GetWindowBkColor(plvdata->hWnd));
        SetTextColor (hdc, psub->nTextColor);
    }
    else {
        SetBkColor (hdc, plvdata->bkc_selected);
        SetBrushColor (hdc, plvdata->bkc_selected);
        SetTextColor (hdc, PIXEL_lightwhite);
    }

    FillBox (hdc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top);

    LV_GET_SUBITEM_TEXTRECT(rect, rect_text);

    if (psub->Image) {
       int width, height;
       if (psub->dwFlags & LVIF_BITMAP) {
	   PBITMAP bmp = (PBITMAP)psub->Image;
	   height = (bmp->bmHeight < plvdata->nItemHeight)? bmp->bmHeight : plvdata->nItemHeight;
	   rect.top += (plvdata->nItemHeight - height) / 2;
	   rect.left += 2;
	   FillBoxWithBitmap (hdc, rect.left, rect.top, 0, height, bmp);
           rect_text.left = rect.left + bmp->bmWidth + 2;
       }
       else if (psub->dwFlags & LVIF_ICON) {
	   GetIconSize ((HICON)psub->Image, &width, &height);
	   height = (height < plvdata->nItemHeight) ? height : plvdata->nItemHeight;
	   rect.top += (plvdata->nItemHeight - height) / 2;
	   rect.left += 2;
	   DrawIcon (hdc, rect.left, rect.top, 0, height,
			   (HICON)psub->Image);
           rect_text.left = rect.left + width + 2;
       }
    }

    if (!psub->pszInfo)
	return;

    if (ph->flags & LVCF_RIGHTALIGN)
	text_format = DT_SINGLELINE | DT_RIGHT | DT_VCENTER;
    else if (ph->flags & LVCF_CENTERALIGN)
	text_format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
    else
	text_format = DT_SINGLELINE | DT_LEFT | DT_VCENTER;

    DrawText (hdc, psub->pszInfo, -1, &rect_text, text_format);
}

//Draws listview header
static void lvDrawHeader (HWND hwnd, HDC hdc)
{
    PLSTHDR p1 = NULL;
    PLVDATA plvdata;
    RECT rcClient;
    BOOL up = TRUE;
    UINT format;

    GetClientRect (hwnd, &rcClient);
    plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);
    p1 = plvdata->pLstHead;

    if (LVSTATUS(hwnd) & LVST_HEADCLICK && LVSTATUS(hwnd) & LVST_INHEAD)
	up = FALSE;

    SetBkColor (hdc, PIXEL_lightgray);
    SetBrushColor (hdc, PIXEL_lightgray);
    FillBox (hdc, rcClient.left, rcClient.top, rcClient.right - rcClient.left,
             plvdata->nHeadHeight);

    SetTextColor (hdc, PIXEL_black);

    while (p1)
    {
#ifdef _FLAT_WINDOW_STYLE
        DrawFlatControlFrameEx (hdc, p1->x - plvdata->nOriginalX-1,
                          LV_HDR_TOP - plvdata->nOriginalY-1,
                          p1->x - plvdata->nOriginalX + p1->width,
                          LV_HDR_TOP + LV_HDR_HEIGHT, PIXEL_lightgray, 0, up);
#else
        Draw3DControlFrame (hdc, p1->x - plvdata->nOriginalX + 1,
		          LV_HDR_TOP,
                          p1->x - plvdata->nOriginalX + p1->width - 1,
                          LV_HDR_TOP + LV_HDR_HEIGHT, PIXEL_lightgray, up);
#endif
	if (p1->flags & LVHF_CENTERALIGN)
	    format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
	else if (p1->flags & LVHF_RIGHTALIGN)
	    format = DT_SINGLELINE | DT_RIGHT | DT_VCENTER;
	else
	    format = DT_SINGLELINE | DT_LEFT | DT_VCENTER;

        sDrawText (hdc, p1->x - plvdata->nOriginalX + 2, LV_HDR_TOP,
                 p1->width - 4, LV_HDR_HEIGHT, p1->pTitle, format);
        p1 = p1->pNext;
    }
    //draws the right most unused header
    if ( !LV_H_OUTWND(plvdata, rcClient) ) {
#ifdef _FLAT_WINDOW_STYLE
      DrawFlatControlFrameEx (hdc, sGetItemWidth (plvdata)-2,
		          LV_HDR_TOP-1,
                          rcClient.right+2,
                          LV_HDR_TOP + LV_HDR_HEIGHT, PIXEL_lightgray, 0, up);
#else
      Draw3DControlFrame (hdc, sGetItemWidth (plvdata),
		          LV_HDR_TOP,
                          rcClient.right+2,
                          LV_HDR_TOP + LV_HDR_HEIGHT, PIXEL_lightgray, up);
#endif
    }
}

static void lvDrawItem (HWND hwnd, HDC hdc, int nRows)
{
    int i;
    RECT rcClient;
    PLVDATA plvdata;
    PITEMDATA p3 = NULL;

    plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);
    p3 = lvGetItemByRow(plvdata, nRows);

    GetClientRect (hwnd, &rcClient);

    /*
    if (p3->bSelected) {
        SetBrushColor (hdc,plvdata->bkc_selected);
    }
    else
        SetBrushColor (hdc, PIXEL_lightwhite);

    FillBox (hdc, rcClient.left,
             (nRows - 1) * plvdata->nItemHeight +
             plvdata->nHeadHeight,
             sGetItemWidth(plvdata)-1,
             plvdata->nItemHeight);
    */

    for (i = 1; i <= plvdata->nCols; i++)
    {
        lvDrawSubItem (hdc, nRows, i, plvdata);
    }
}

static void lvOnDraw (HWND hwnd, HDC hdc)
{
    int j;
    RECT rcClient;
    PLVDATA plvdata;

    plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);
    GetClientRect (hwnd, &rcClient);

    SetBkColor (hdc, PIXEL_lightwhite);
    SetBrushColor (hdc, PIXEL_lightwhite);
    FillBox (hdc, rcClient.left, rcClient.top, rcClient.right - rcClient.left,
             rcClient.bottom - rcClient.top);

    //draws item area
    for (j = 1; j <= plvdata->nRows; j++)
    {
	lvDrawItem (hwnd, hdc, j);
    }

    lvDrawHeader (hwnd, hdc);
}

/*************************************  Listview Move/Scroll Action ***********************/


/* Makes an item to be visible */
static void
lvMakeItemVisible (HWND hwnd, PLVDATA plvdata, int nRows)
{
    int scrollHeight = 0;
    RECT rect;
    int area_height;

    if (!LV_BE_VALID_ROW(nRows))
	nRows = plvdata->nItemSelected;
    if (!LV_BE_VALID_ROW(nRows))
	return;

    GetClientRect (hwnd, &rect);
    area_height = rect.bottom - rect.top - plvdata->nHeadHeight;

    if ( LV_ITEM_Y(nRows) < plvdata->nOriginalY) {
        scrollHeight = plvdata->nOriginalY - LV_ITEM_Y(nRows);
    }
    else if ( LV_ITEM_Y(nRows+1) - plvdata->nOriginalY > area_height ) {
        scrollHeight = plvdata->nOriginalY + area_height - LV_ITEM_Y(nRows+1);
    }

    if (scrollHeight != 0) {
        plvdata->nOriginalY -= scrollHeight;
        lstSetVScrollInfo (plvdata);
	InvalidateRect(hwnd, NULL, FALSE);
    }
}


static void lvVScroll (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    int scrollHeight = 0;
    RECT rect;
    int scrollBoundMax;
    int scrollBoundMin;
    int vscroll = 0;
    PLVDATA plvdata;

    plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);

    GetClientRect (hwnd, &rect);
    scrollBoundMax = plvdata->nRows * plvdata->nItemHeight -
		     (rect.bottom - rect.top - plvdata->nHeadHeight);
    scrollBoundMin = 0;
    
    //decides the desired value to scroll
    if (wParam == SB_LINEUP || wParam == SB_LINEDOWN)
	vscroll = VSCROLL;
    else if (wParam == SB_PAGEUP || wParam == SB_PAGEDOWN)
	vscroll = rect.bottom - rect.top - plvdata->nHeadHeight -
		  plvdata->nItemHeight;

    //scroll down
    if ( (wParam == SB_LINEDOWN || wParam == SB_PAGEDOWN) &&
                    plvdata->nOriginalY < scrollBoundMax )
    {
	if ((plvdata->nOriginalY + vscroll) > scrollBoundMax)
	{
	    scrollHeight = plvdata->nOriginalY - scrollBoundMax;
	    plvdata->nOriginalY = scrollBoundMax;
	}
	else
	{
	    plvdata->nOriginalY += vscroll;
	    scrollHeight = -vscroll;
	}
    }
    //scroll up
    else if ( (wParam == SB_LINEUP || wParam == SB_PAGEUP) &&
	            plvdata->nOriginalY > scrollBoundMin )
    {
	if ((plvdata->nOriginalY - vscroll) > scrollBoundMin)
	{
	    plvdata->nOriginalY -= vscroll;
	    scrollHeight = vscroll;
	}
	else
	{
	    scrollHeight = plvdata->nOriginalY - scrollBoundMin;
	    plvdata->nOriginalY = scrollBoundMin;
	}
    }
    //dragging
    else if ( wParam == SB_THUMBTRACK )
    {
	    int scrollNewPos = (int) lParam;

	    if (((scrollNewPos - plvdata->nOriginalY) < 5) &&
		  ((scrollNewPos - plvdata->nOriginalY) > -5) &&
		  (scrollNewPos > 5) && ((scrollBoundMax - scrollNewPos) > 5))
		return;

	    if ((scrollNewPos < plvdata->nOriginalY) && (scrollNewPos <= VSCROLL))
	    {
		scrollHeight = plvdata->nOriginalY - 0;
		plvdata->nOriginalY = 0;
	    }
	    else
	    {
		if ((scrollNewPos > plvdata->nOriginalY) && ((scrollBoundMax - scrollNewPos) < VSCROLL))
		{
		    scrollHeight = plvdata->nOriginalY - scrollBoundMax;
		    plvdata->nOriginalY = scrollBoundMax;
		}
		else
		{
		    scrollHeight = plvdata->nOriginalY - scrollNewPos;
		    plvdata->nOriginalY = scrollNewPos;
		}
	    }
    }

    if (scrollHeight != 0) {
	InvalidateRect (hwnd, NULL, FALSE);
        lstSetVScrollInfo (plvdata);
    }
}


static void lvHScroll (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    int scrollWidth = 0;
    int scrollBoundMax;
    int scrollBoundMin;
    RECT rect;
    int hscroll = 0;
    PLVDATA plvdata;

    plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);

    GetClientRect (hwnd, &rect);
    scrollBoundMax = sGetItemWidth (plvdata) - (rect.right - rect.left);
    scrollBoundMin = 0;

    //decides the desired value to scroll
    if (wParam == SB_LINERIGHT || wParam == SB_LINELEFT)
	hscroll = HSCROLL;
    else if (wParam == SB_PAGERIGHT || wParam == SB_PAGELEFT)
	hscroll = rect.right - rect.left;

    //scroll right 
    if ( (wParam == SB_LINERIGHT || wParam == SB_PAGERIGHT) && 
		    plvdata->nOriginalX < scrollBoundMax )
    {
	if ((plvdata->nOriginalX + hscroll) > scrollBoundMax)
	{
	    scrollWidth = plvdata->nOriginalX - scrollBoundMax;
	    plvdata->nOriginalX = scrollBoundMax;
	}
	else
	{
	    plvdata->nOriginalX += hscroll;
	    scrollWidth = -hscroll;
	}
    }
    //scroll left 
    else if ( (wParam == SB_LINELEFT || wParam == SB_PAGELEFT) && 
		    plvdata->nOriginalX > scrollBoundMin)
    {
	if ((plvdata->nOriginalX - hscroll) > scrollBoundMin)
	{
	    plvdata->nOriginalX -= hscroll;
	    scrollWidth = hscroll;
	}
	else
	{
	    scrollWidth = plvdata->nOriginalX - scrollBoundMin;
	    plvdata->nOriginalX = scrollBoundMin;
	}
    }
    //draging
    else if (wParam == SB_THUMBTRACK)
    {
	int scrollNewPos = (int) lParam;

	if (((scrollNewPos - plvdata->nOriginalX) < HSCROLL) &&
		    ((scrollNewPos - plvdata->nOriginalX) > -HSCROLL) && (scrollNewPos > HSCROLL)
		    && ((scrollBoundMax - scrollNewPos) > HSCROLL))
		return;

	if ((scrollNewPos < plvdata->nOriginalX) && (scrollNewPos <= HSCROLL))
	{
		scrollWidth = plvdata->nOriginalX - 0;
		plvdata->nOriginalX = 0;
	}
	else
	{
		if ((scrollNewPos > plvdata->nOriginalX) && ((scrollBoundMax - scrollNewPos) < HSCROLL))
		{
		    scrollWidth = plvdata->nOriginalX - scrollBoundMax;
		    plvdata->nOriginalX = scrollBoundMax;
		}
		else
		{
		    scrollWidth = plvdata->nOriginalX - scrollNewPos;
		    plvdata->nOriginalX = scrollNewPos;
		}
	}
    }

    if (scrollWidth != 0) {
	InvalidateRect (hwnd, NULL, FALSE);
        lstSetHScrollInfo (plvdata);
    }
}

/*************************  header operations ********************************/
static PLSTHDR lvGetHdrByCol (PLVDATA plvdata, int nCols)
{
    int i;
    PLSTHDR p1 = plvdata->pLstHead;

    if ((nCols >  plvdata->nCols) || (nCols < 1 ))
        return NULL;

    for (i = 1; i < nCols; i++) {
	p1 = p1->pNext;
    }

    return p1;
}

/* creates a new header */
static PLSTHDR 
lvHdrNew (PLVCOLUMN pcol, PLVDATA plvdata, int *col)
{
    PLSTHDR p1;
    int nCols = pcol->nCols;

    if (!LV_BE_VALID_COL(nCols)) {
       nCols = plvdata->nCols + 1;
       *col = nCols;
    }

    p1 = (PLSTHDR) malloc (sizeof (LSTHDR));
    p1->sort = NOTSORTED;
    p1->pfnCmp = pcol->pfnCompare;
    p1->Image = pcol->image;
    p1->flags = pcol->colFlags;

    if (pcol->pszHeadText != NULL)
    {
        p1->pTitle = (char *) malloc (strlen (pcol->pszHeadText) + 1);
        strcpy (p1->pTitle, pcol->pszHeadText);
    }
    else
        p1->pTitle = NULL;
    
    p1->x = sGetFrontSubItemsWidth (nCols - 1, plvdata);
    if (pcol->width <= 0)
	p1->width = LV_COLW_DEF;
    else if (pcol->width < COLWIDTHMIN)
	p1->width = COLWIDTHMIN;
    else
        p1->width = pcol->width;

    if (nCols == 1)
    {
        p1->pNext = plvdata->pLstHead;
        plvdata->pLstHead = p1;
    }
    else
    {
	PLSTHDR p2 = lvGetHdrByCol(plvdata, nCols-1);
        p1->pNext = p2->pNext;
        p2->pNext = p1;
    }

    return p1;
}

//free a header
static void lvHdrFree (PLSTHDR pLstHdr)
{
    if (pLstHdr != NULL)
    {
        if (pLstHdr->pTitle != NULL)
            free (pLstHdr->pTitle);
        free (pLstHdr);
    }
}

//deletes a header
static void lvHdrDel (int nCols, PLVDATA plvdata)
{
    PLSTHDR p1 = plvdata->pLstHead;
    PLSTHDR pdel;

    if (nCols == 1)
    {
	pdel = p1;
        plvdata->pLstHead = p1->pNext;
    }
    else
    {
	p1 = lvGetHdrByCol(plvdata, nCols-1);
	if (!p1)
	    return;
	pdel = p1->pNext;
	if (pdel) {
            p1->pNext = pdel->pNext;
	    lvHdrFree(pdel);
	}
    }
}

/* ========================================================================= 
 * Model & Data section of the listview control.
   ========================================================================= */

static PITEMDATA lvGetItemByRow (PLVDATA plvdata, int nRows)
{
    int i;
    PITEMDATA pItem = plvdata->pItemHead;

    if ((nRows >  plvdata->nRows) || (nRows < 1 ))
        return NULL;

    for (i = 1; i < nRows; i++) {
	pItem = pItem->pNext;
    }

    return pItem;
}

static PSUBITEMDATA lvGetSubItemByCol (PITEMDATA pItem, int nCols)
{
    int i;
    PSUBITEMDATA pSubItem;

    if (!pItem)
	return NULL;
    pSubItem = pItem->pSubItemHead;

    for (i = 1; i < nCols; i++) {
	pSubItem = pSubItem->pNext;
    }

    return pSubItem;
}

#define lstSelectItem(hwnd, nRows, plvdata)    \
             lvSetItemSelect (hwnd, nRows, plvdata, TRUE)
#define lstUnSelectItem(hwnd, nRows, plvdata)    \
             lvSetItemSelect (hwnd, nRows, plvdata, FALSE)

/* select and make the original one unselected */
#define lvSelectItem(hwnd, nRows, plvdata)     \
	     lstUnSelectItem(hwnd, plvdata->nItemSelected, plvdata);   \
	     lstSelectItem(hwnd, nRows, plvdata);

/* Sets the item to be selcted or unselected status */
static int
lvSetItemSelect (HWND hwnd, int nRows, PLVDATA plvdata, BOOL bSel)
{
    PITEMDATA pSel;
    RECT rect;

    if ( !(pSel = lvGetItemByRow (plvdata, nRows)) )
	return LV_ERR;

    if (bSel == pSel->bSelected)
	return LV_ERR;
	
    if (bSel) {
        plvdata->nItemSelected = nRows;
    }
    else {
        plvdata->nItemSelected = 0;
    }
    pSel->bSelected = bSel;

    LV_GET_ITEM_RECT(nRows, rect);
    InvalidateRect (hwnd, &rect, FALSE);

    return LV_OKAY;
}

#if 0

#define lvSelectNextItem(hwnd, plvdata)    \
              lvStepItem(hwnd, plvdata, 1)
#define lvSelectPrevItem(hwnd, plvdata)    \
              lvStepItem(hwnd, plvdata, -1)
static void
lvStepItem(HWND hwnd, PLVDATA plvdata, int step)
{
    int prev;
    prev = plvdata->nItemSelected;
    if (prev+step <= plvdata->nRows && prev+step > 0) {
        lstUnSelectItem (hwnd, prev, plvdata);
        lstSelectItem (hwnd, prev+step, plvdata);
    }
}

#endif

/****************************  data init and destroy  ************************/
static void InitListViewData (HWND hwnd)
{
    PLVDATA plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);

    plvdata->nCols = 0;
    plvdata->nRows = 0;
    plvdata->hWnd = hwnd;
    plvdata->pLstHead = NULL;
    plvdata->pItemHead = NULL;
    plvdata->nOriginalX = 0;
    plvdata->nOriginalY = 0;
    LVSTATUS(hwnd) = LVST_NORMAL;

    plvdata->pHdrClicked = NULL;
    plvdata->nItemDraged = 0;
    plvdata->nItemSelected = 0;

    plvdata->bkc_selected = LIGHTBLUE;

    plvdata->nItemHeight = LV_ITEMH_DEF(hwnd);
    plvdata->nHeadHeight = LV_HDRH_DEF(hwnd);

    plvdata->str_cmp = strncmp;
}

//Destroies all the internal datas
static void lvDataDestory (PLVDATA plvdata)
{
    PITEMDATA p1, p2;
    PLSTHDR ph, ph2;

    p1 = plvdata->pItemHead;

    while (p1 != NULL)
    {
        p2 = p1;
        p1 = p1->pNext;
        itemDelete (p2);
    }

    ph = plvdata->pLstHead;
    while (ph) {
        ph2 = ph;
        ph = ph->pNext;
        lvHdrFree (ph2);
    }

    free (plvdata);
}

/******************** subitem operations ****************************/

static PSUBITEMDATA subitemNew (const char *pszInfoText)
{
    PSUBITEMDATA p1;
    
    p1 = (PSUBITEMDATA) malloc (sizeof (SUBITEMDATA));
    p1->pNext = NULL;
    
    if (pszInfoText != NULL)
    {
        p1->pszInfo = (char *) malloc (strlen (pszInfoText) + 1);
        strcpy (p1->pszInfo, pszInfoText);
    }
    else
        p1->pszInfo = NULL;
    p1->nTextColor = PIXEL_black;
    p1->Image = 0;
    p1->dwFlags = 0;

    return p1;
}

static void subitemFree (PSUBITEMDATA pSubItemData)
{
    if (pSubItemData != NULL)
    {
        if (pSubItemData->pszInfo != NULL)
            free (pSubItemData->pszInfo);
        free (pSubItemData);
    }
}

static void
lvAddSubitem (PLVDATA plvdata, int nCols)
{
    int i;
    PSUBITEMDATA p2, p3;
    PITEMDATA p1 = plvdata->pItemHead;

    while (p1)
    {
            p2 = subitemNew (NULL);
            if (nCols == 1)
            {
                p2->pNext = p1->pSubItemHead;
                p1->pSubItemHead = p2;
            }
            else
            {
                p3 = p1->pSubItemHead;
		for (i = 1; i < nCols-1; i++) {
                    p3 = p3->pNext;
		}
		p2->pNext = p3->pNext;
		p3->pNext = p2;
            }
            p1 = p1->pNext;
    }
}

static void
lvDelSubitem (PLVDATA plvdata, int nCols)
{
    int i;
    PSUBITEMDATA p2, pdel;
    PITEMDATA p1 = plvdata->pItemHead;

    while (p1)
    {
            if (nCols == 1)
            {
                p1->pSubItemHead = p1->pSubItemHead->pNext;
            }
            else
            {
                p2 = p1->pSubItemHead;
		for (i = 1; i < nCols-1; i++) {
                    p2 = p2->pNext;
		}
		pdel = p2->pNext;
		p2->pNext = pdel->pNext;
		subitemFree(pdel);
            }
            p1 = p1->pNext;
    }
}

static BOOL 
lvAddColumnToList (PLVCOLUMN pcol, PLVDATA plvdata)
{
    PLSTHDR p1 = NULL;
    int nCols = pcol->nCols;

    p1 = lvHdrNew (pcol, plvdata, &nCols);
    if (!p1)
	return FALSE;

    lvAddSubitem (plvdata, nCols);
    sAddOffsetToTailSubItem (nCols, p1->width, plvdata);
    plvdata->nCols ++;

    return TRUE;
}

static int sGetItemSeq (PLVDATA plvdata)
{
    PITEMDATA p1;
    int i = 0;

    p1 = plvdata->pItemHead;
    while (p1)
    {
        i++;

        if (p1->bSelected)
            return i;

        p1 = p1->pNext;
    }

    return -1;
}

//FIXME
static int sModifyHeadText (int nCols, const char *pszHeadText, PLVDATA plvdata)
{
    PLSTHDR p1 = NULL;

    p1 = lvGetHdrByCol(plvdata, nCols);
    if (!p1 || !pszHeadText)
        return -1;
  
    if (p1->pTitle != NULL)
        free (p1->pTitle);
  
    p1->pTitle = (char *) malloc (sizeof (pszHeadText) + 1);
    strcpy (p1->pTitle, pszHeadText);
  
    return 0;
}

inline static PSUBITEMDATA
lvGetSubItem (int nItem, int nSubItem, PLVDATA plvdata)
{
    PITEMDATA pItem = lvGetItemByRow(plvdata, nItem);
    return lvGetSubItemByCol(pItem, nSubItem);
}

/* Sets the properties of the subitem */

/* Fills the content of a subitem */
static int 
lvFillSubItem (int nItem, int subItem, const char *pszText, PLVDATA plvdata)
{
    PSUBITEMDATA p1;

    if ( !(p1 = lvGetSubItem (nItem, subItem, plvdata)) )
        return -1;

    if (pszText == NULL)
        return -1;

    if (p1->pszInfo != NULL)
        free (p1->pszInfo);
    p1->pszInfo = (char *) malloc (strlen (pszText) + 1);
    strcpy (p1->pszInfo, pszText);

    return 0;
}

/* Sets the text color of the subitem */
static int 
lvSetSubItemTextColor (int nItem, int subItem, int color, PLVDATA plvdata)
{
    PSUBITEMDATA p1;

    if ( !(p1 = lvGetSubItem (nItem, subItem, plvdata)) )
        return -1;

    p1->nTextColor = color;

    return 0;
}

static int 
lvSetSubItem (PLVSUBITEM pinfo, PLVDATA plvdata)
{
    PSUBITEMDATA p1;

    if ( !(p1 = lvGetSubItem (pinfo->nItem, pinfo->subItem, plvdata)) )
        return -1;

    if (p1->pszInfo != NULL)
        free (p1->pszInfo);
    p1->pszInfo = (char *) malloc (strlen (pinfo->pszText) + 1);
    strcpy (p1->pszInfo, pinfo->pszText);

    p1->nTextColor = pinfo->nTextColor;
    if (pinfo->flags & LVFLAG_BITMAP)
        p1->dwFlags |= LVIF_BITMAP;
    if (pinfo->flags & LVFLAG_ICON)
        p1->dwFlags |= LVIF_ICON;
    p1->Image = pinfo->image;

    return 0;
}

static int 
lvRemoveColumn (int nCols, PLVDATA plvdata)
{
    int offset;

    if (!LV_BE_VALID_COL(nCols))
	return -1;

    offset = -(sGetSubItemWidth (nCols, plvdata));
    sAddOffsetToTailSubItem (nCols + 1, offset, plvdata);

    lvHdrDel (nCols, plvdata);
    lvDelSubitem (plvdata, nCols);

    plvdata->nCols --;

    lstSetHScrollInfo (plvdata);
    InvalidateRect (plvdata->hWnd, NULL, FALSE);

    return 0;
}

static PITEMDATA itemNew (int nCols)
{
    PSUBITEMDATA pHead = NULL;
    PSUBITEMDATA p1 = NULL;
    PSUBITEMDATA p2 = NULL;
    PITEMDATA p3 = NULL;
    int i;
    int j;

    j = nCols;

    if (j >= 1)
    {
        pHead = subitemNew (NULL);
        p1 = pHead;
    }
    else
        return NULL;

    for (i = 1; i <= j - 1; i++)
    {
        p2 = subitemNew (NULL);
        p1->pNext = p2;
        p1 = p2;
    }
    p3 = (PITEMDATA) malloc (sizeof (ITEMDATA));
    p3->pNext = NULL;
    p3->bSelected = FALSE;

    p3->pSubItemHead = pHead;

    return p3;
}

static int itemDelete (PITEMDATA pItem)
{
    PSUBITEMDATA p1 = NULL;
    PSUBITEMDATA p2 = NULL;

    p1 = pItem->pSubItemHead;

    while (p1 != NULL)
    {
        p2 = p1;
        p1 = p1->pNext;
        subitemFree (p2);
    }
    pItem->pSubItemHead = NULL;
    free (pItem);

    return 0;
}

static int sAddItemToList (int nItem, PITEMDATA pnew, PLVDATA plvdata)
{
    int i;
    PITEMDATA p1 = pnew;
    PITEMDATA p2 = NULL;
    PITEMDATA p3 = NULL;

    if (plvdata->nRows > LV_ROW_MAX)
        return -1;

    if ((nItem < 1) || (nItem > plvdata->nRows))
        nItem = plvdata->nRows + 1;

    if (p1 == NULL)
        return -1;

    if (nItem == 1)
    {
        p2 = plvdata->pItemHead;
        plvdata->pItemHead = p1;
        p1->pNext = p2;
    }
    else
    {
        i = nItem;
        p2 = plvdata->pItemHead;
        while (i != 2)
        {
            i = i - 1;
            p2 = p2->pNext;
        }
        p3 = p2->pNext;
        p2->pNext = p1;
        p1->pNext = p3;
    }

    plvdata->nRows ++;

    if (nItem <= plvdata->nItemSelected)
        plvdata->nItemSelected++;

    return nItem;
}

static int sRemoveItemFromList (int nItem, PLVDATA plvdata)
{
    PITEMDATA p1 = NULL;
    PITEMDATA pp1 = NULL;

    if ((nItem < 1) || (nItem > plvdata->nRows) || (plvdata->nRows < 1))
        return -1;

    if (nItem == 1)
    {
        p1 = plvdata->pItemHead;
        plvdata->pItemHead = plvdata->pItemHead->pNext;
    }
    else
    {
	pp1 = lvGetItemByRow (plvdata, nItem-1);
        p1 = pp1->pNext;
        pp1->pNext = p1->pNext;
    }

    if (p1->bSelected)
	plvdata->nItemSelected = 0;

    itemDelete (p1);

    plvdata->nRows --;

    /*
    if () {
        plvdata->nOriginalY -= plvdata->nItemHeight;
    }
    */

    lstSetVScrollInfo (plvdata);
    InvalidateRect (plvdata->hWnd, NULL, FALSE);

    return 0;
}

static int sRemoveAllItem (PLVDATA plvdata)
{
    PITEMDATA p1 = NULL;
    PITEMDATA p2 = NULL;

    p1 = plvdata->pItemHead;

    while (p1 != NULL)
    {
        p2 = p1;
        p1 = p1->pNext;
        itemDelete (p2);
    }

    plvdata->nRows = 0;
    plvdata->pItemHead = NULL;
    //plvdata->nOriginalX = 0;

    lstSetVScrollInfo (plvdata);
    //lstSetHScrollInfo (plvdata);
    InvalidateRect (plvdata->hWnd, NULL, FALSE);

    return 0;
}

static int lvFindItem (PLVFINDINFO pFindInfo, PLVDATA plvdata)
{
    PITEMDATA p1;
    PSUBITEMDATA p2;
    int i = 0, j = 0;

    if (pFindInfo == NULL)
        return -1;

    if ( !(p1 = lvGetItemByRow (plvdata, pFindInfo->iStart)) )
        p1 = plvdata->pItemHead;

    while (p1 != NULL)
    { 
        p2 = p1->pSubItemHead;
        i = pFindInfo->nCols;

	if (pFindInfo->flags & LVFF_ADDDATA) {
	    if (pFindInfo->addData != p1->addData)
		continue;
	}

	if (pFindInfo->flags & LVFF_TEXT) {
            while ((p2 != NULL) && (i > 0))
            {
                if (plvdata->str_cmp (p2->pszInfo, pFindInfo->pszInfo[i - 1], (size_t)-1) != 0)
                    break;

                i--;
                p2 = p2->pNext;
            }
	}
        
        j++;
        p1 = p1->pNext;

        if (i == 0)
            return j;
    }

    return -1;
}

static int sGetSubItemWidth (int nCols, PLVDATA plvdata)
{
    PLSTHDR p;
    int nPosition;

    nPosition = nCols;

    if ((nCols < 1) || (nCols > plvdata->nCols))
    {
        return -1;
    }

    p = plvdata->pLstHead;

    while (nPosition != 1)
    {
        nPosition--;
        p = p->pNext;
    }
    
    return p->width;
}

static int sGetItemWidth (PLVDATA plvdata)
{
    PLSTHDR p;
    int width;

    p = plvdata->pLstHead;
    width = 0;

    while (p != NULL)
    {
        width += p->width;
        p = p->pNext;
    }
    
    return width;
}

/*
 * gets the previous nClos items width
 */
inline static int 
sGetFrontSubItemsWidth (int nCols, PLVDATA plvdata)
{
    PLSTHDR p1 = lvGetHdrByCol(plvdata, nCols);
    if (p1)
        return p1->x + p1->width;
    return -1;
}

/* be care, doesn't check p and offset */
inline static int 
sAddOffsetToSubItem (PLSTHDR p, int offset)
{
    p->width += offset;
    return 0;
}

/* offset the tail subitems from nCols */
static int sAddOffsetToTailSubItem (int nCols, int offset, PLVDATA plvdata)
{
    PLSTHDR p;

    p = lvGetHdrByCol (plvdata, nCols);
    if (!p) return -1;

    while (p)
    {
        p->x += offset;
        p = p->pNext;
    }

    return 0;
}

static void lvSetColumnWidth (int nCols, int width, PLVDATA plvdata)
{
    int offset;
    PLSTHDR ph = lvGetHdrByCol (plvdata, nCols);

    if (!ph) return;

    if (width < COLWIDTHMIN)
	width = COLWIDTHMIN;

    offset = width - ph->width;
    sAddOffsetToSubItem(ph, offset);
    sAddOffsetToTailSubItem (nCols+1, offset, plvdata);
}

/* The default comparision function for compare two items */
static int 
lvDefCompare (int row1, int row2, int ncol, PLVDATA plvdata)
{
    PITEMDATA p1, p2;
    PSUBITEMDATA psub1, psub2;

    p1 = lvGetItemByRow (plvdata, row1);
    p2 = lvGetItemByRow (plvdata, row2);
    psub1 = lvGetSubItemByCol (p1, ncol);
    psub2 = lvGetSubItemByCol (p2, ncol);

    if (plvdata->str_cmp)
	return plvdata->str_cmp (psub1->pszInfo, psub2->pszInfo, (size_t)-1);
    else
        return strcasecmp (psub1->pszInfo, psub2->pszInfo);
}

static void lvMoveItem (int old_row, int new_row, PLVDATA plvdata)
{
    PITEMDATA p1, p2, pp1, pp2;

    if (old_row == new_row)
	return;

    p1 = lvGetItemByRow (plvdata, old_row);
    p2 = lvGetItemByRow (plvdata, new_row);

    if (!p1 || !p2)
	return;

    pp1 = lvGetItemByRow (plvdata, old_row-1);
    pp2 = lvGetItemByRow (plvdata, new_row-1);

    if (pp1)
	pp1->pNext = p1->pNext;
    else
	plvdata->pItemHead = p1->pNext;
	
    if (pp2)
        pp2->pNext = p1;
    else
	plvdata->pItemHead = p1;
    p1->pNext = p2;
}

/* sorting items using a comparision function */
static int 
lvSortItem (PFNLVCOMPARE pfn_user, int nCols, SORTTYPE sort, PLVDATA plvdata)
{
    int i, j;
    PLSTHDR ph;
    PFNLVCOMPARE pcmp;
    LVSORTDATA sortData;

    /* If pfn_user is not NULL, use it as comparision function; otherwise, 
     * use the one associated with nCols column.
     */ 
    if (pfn_user)
	pcmp = pfn_user;
    else {
        if ( !(ph = lvGetHdrByCol (plvdata, nCols)) )
	    return -1;
        pcmp = ph->pfnCmp;
    }

    //FIXME, sortData howto use?
    sortData.ncol = nCols;

    //FIXME, more efficient algorithm
    for (i = 1; i <= plvdata->nRows; i++)
    {
	for (j = 1; j < i; j++) {
	    int ret;

	    if (pcmp) {
		ret = pcmp (i, j, &sortData);
	        if (ret == 0)
		    ret = lvDefCompare (i, j, 1, plvdata);
	    }
	    else {
                ret = lvDefCompare (i, j, nCols, plvdata);
	    }

            if ( (sort == LOSORTED && ret < 0) || (sort == HISORTED && ret > 0))
		break;
	}
	lvMoveItem (i, j, plvdata);
    }

    return 0;
}

/***************************  scroll info  ***********************************/
static int lstSetVScrollInfo (PLVDATA plvdata)
{
    SCROLLINFO si;
    RECT rect;

    GetClientRect (plvdata->hWnd, &rect);

    if ((rect.bottom - rect.top - plvdata->nHeadHeight) > 
		    ((plvdata->nRows) * plvdata->nItemHeight))
    {
        ShowScrollBar (plvdata->hWnd, SB_VERT, FALSE);
        plvdata->nOriginalY = 0;
      
        return 0;
    }

    if (plvdata->nOriginalY < 0)
        plvdata->nOriginalY = 0;

    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMax = plvdata->nRows * plvdata->nItemHeight;
    si.nMin = 0;
    si.nPage = rect.bottom - rect.top - plvdata->nHeadHeight;
    si.nPos = plvdata->nOriginalY;

    if (si.nPos > si.nMax - si.nPage) {
	si.nPos = si.nMax - si.nPage;
	plvdata->nOriginalY = si.nPos;
    }

    SetScrollInfo (plvdata->hWnd, SB_VERT, &si, TRUE);
    ShowScrollBar (plvdata->hWnd, SB_VERT, TRUE);

    return 0;
}

static int lstSetHScrollInfo (PLVDATA plvdata)
{
    SCROLLINFO si;
    RECT rect;

    GetClientRect (plvdata->hWnd, &rect);

    if ( !LV_H_OUTWND(plvdata, rect) )
    {
        ShowScrollBar (plvdata->hWnd, SB_HORZ, FALSE);
        return 0;
    }

    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMax = sGetItemWidth (plvdata);
    si.nMin = 0;
    si.nPage = rect.right - rect.left;
    si.nPos = plvdata->nOriginalX;

    if (si.nPos > si.nMax - si.nPage) {
	si.nPos = si.nMax - si.nPage;
	plvdata->nOriginalX = si.nPos;
    }

    SetScrollInfo (plvdata->hWnd, SB_HORZ, &si, TRUE);
    ShowScrollBar (plvdata->hWnd, SB_HORZ, TRUE);

    return 0;
}

/*************************************  Listview drag border action ***********************/

static void lvBorderDrag (HWND hwnd, int x, int y)
{
    int mouseX = x, mouseY = y;
    PLVDATA plvdata;
    RECT rect, rcClient;
    int offset;
    PLSTHDR pDrag;

    plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);
    pDrag = lvGetHdrByCol (plvdata, plvdata->nItemDraged);

    GetClientRect (hwnd, &rcClient);
    ScreenToClient (hwnd, &mouseX, &mouseY);

    //the column width should not less than the min value
    if ((pDrag->x - plvdata->nOriginalX + COLWIDTHMIN) > mouseX - 1)
        return;

    offset = mouseX - (pDrag->x + pDrag->width-plvdata->nOriginalX);
    lvSetColumnWidth (plvdata->nItemDraged, pDrag->width+offset, plvdata);

    rect.left = rcClient.left;
    rect.right = rcClient.right;
    rect.top = rcClient.top;
    rect.bottom = rect.top + plvdata->nHeadHeight+1;

    InvalidateRect(hwnd, &rect, FALSE);

    if (offset < 0) {
	plvdata->nOriginalX += offset;
        if (plvdata->nOriginalX < 0)
	    plvdata->nOriginalX = 0;
    }

    //lstSetHScrollInfo (plvdata);
}

static void lvToggleSortStatus (PLSTHDR p1)
{
    switch (p1->sort)
    {
    case NOTSORTED:
	p1->sort = LOSORTED;
	break;
    case HISORTED:
	p1->sort = LOSORTED;
	break;
    case LOSORTED:
	p1->sort = HISORTED;
	break;
    }
}

/********************************************** List Report        **********************************************/
static int sListViewProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    PLVDATA plvdata = NULL;
    DWORD dwStyle;

    if (message != MSG_CREATE)
        plvdata = (PLVDATA) GetWindowAdditionalData2 (hwnd);

    switch (message)
    {
        case MSG_CREATE:
        {  
            dwStyle = GetWindowStyle (hwnd);

            if (!(plvdata = (PLVDATA) malloc (sizeof (LVDATA))))
                return -1;

            SetWindowAdditionalData2 (hwnd, (DWORD) plvdata);

            InitListViewData (hwnd);

            lstSetHScrollInfo (plvdata);
            lstSetVScrollInfo (plvdata);

            break;
        }

        case MSG_SETCURSOR:
        {
            int mouseX = LOSWORD (lParam);
            int mouseY = HISWORD (lParam);
            
            if ((LVSTATUS(hwnd) & LVST_BDDRAG) || 
			    (lvInWhichHeadBorder (mouseX, mouseY, NULL, plvdata) > 0))
            {
                SetCursor (GetSystemCursor (IDC_SPLIT_VERT));

                return 0;
            }
            break;
        }

        case MSG_PAINT:
        {
            HDC hdc; /*, mem_dc*/;

            hdc = BeginPaint (hwnd);
            /* mem_dc = CreateCompatibleDC (hdc); */

            /* lvOnDraw (hwnd, mem_dc); */
            lvOnDraw (hwnd, hdc);
            /* BitBlt (mem_dc, 0, 0, 0, 0, hdc, 0, 0, 0); */

            /* DeleteCompatibleDC (mem_dc); */
            EndPaint (hwnd, hdc);
            return 0;
        }

        case MSG_SETFOCUS:
        case MSG_KILLFOCUS:
	    if (message == MSG_SETFOCUS)
    	        plvdata->bkc_selected = LIGHTBLUE;
	    else
    	        plvdata->bkc_selected = GRAYBLUE;
            if (plvdata->nItemSelected > 0) {
		RECT rc;
		LV_GET_ITEM_RECT(plvdata->nItemSelected, rc);
		InvalidateRect(hwnd, &rc, FALSE);
	    }
            break;

        case MSG_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTCHARS;

        case MSG_MOUSEMOVE:
        {
            int mouseX = LOSWORD (lParam);
            int mouseY = HISWORD (lParam);

	    //in head clicked status
	    if (LVSTATUS(hwnd) & LVST_HEADCLICK) {
		RECT rc;
	        PLSTHDR p1;

		if (GetCapture() != hwnd)
		    break;

                ScreenToClient (hwnd, &mouseX, &mouseY);

                p1 = plvdata->pHdrClicked;
		LV_GET_HDR_RECT(p1, rc);

		if (PtInRect(&rc, mouseX, mouseY)) {
		    if (!(LVSTATUS(hwnd) & LVST_INHEAD)) {
			LVSTATUS(hwnd) |= LVST_INHEAD;
			InvalidateRect (hwnd, &rc, FALSE);
		    }
		}
		else if (LVSTATUS(hwnd) & LVST_INHEAD) {
		        rc.left -= 1;
		        rc.right += 1;
			LVSTATUS(hwnd) &= ~LVST_INHEAD;
			InvalidateRect (hwnd, &rc, FALSE);
		}
	    }
	    //in border dragged status
	    else if (LVSTATUS(hwnd) & LVST_BDDRAG)
		lvBorderDrag (hwnd, mouseX, mouseY);

            break;
        }

        case MSG_LBUTTONDOWN:
        {
            int mouseX = LOSWORD (lParam);
            int mouseY = HISWORD (lParam);
            int nCols, nRows;

            RECT rect, rcClient;
            PLSTHDR p1;
            PITEMDATA p2;

            GetClientRect (hwnd, &rcClient);

            nCols = isInListViewHead (mouseX, mouseY, &p1, plvdata);

            if (nCols > 0)  // clicks on the header
            {
		if (GetCapture() == hwnd)
		    break;

		SetCapture (hwnd);
		LVSTATUS(hwnd) |= LVST_HEADCLICK;
		LVSTATUS(hwnd) |= LVST_INHEAD;
                plvdata->pHdrClicked = p1;

		SetRect (&rect, p1->x - plvdata->nOriginalX, LV_HDR_TOP, 
				p1->x - plvdata->nOriginalX+ p1->width, 
				LV_HDR_TOP + LV_HDR_HEIGHT);
                InvalidateRect (hwnd, &rect, FALSE);
            }
            else
            {
                if ((nCols = lvInWhichHeadBorder (mouseX, mouseY, &p1, plvdata)) > 0)
                {
		    LVSTATUS(hwnd) |= LVST_BDDRAG;
                    plvdata->nItemDraged = nCols;
                    SetCapture (hwnd);
                }
                else if ((nRows = isInLVItem (mouseX, mouseY, &p2, plvdata)) > 0)
                {
            	    lvSelectItem (hwnd, nRows, plvdata);
	            lvMakeItemVisible (hwnd, plvdata, nRows);
                    NotifyParent (hwnd, GetDlgCtrlID(hwnd), LVN_SELCHANGE);
                }
            }
            
            break;
        }
        case MSG_LBUTTONUP:
        {
            PLSTHDR p1;
	    int nCols;
            int mouseX = LOSWORD (lParam);
            int mouseY = HISWORD (lParam);

	    if (LVSTATUS(hwnd) & LVST_HEADCLICK)
            {
		if (GetCapture() != hwnd)
		    break;
		ReleaseCapture ();

		LVSTATUS(hwnd) &= ~LVST_HEADCLICK;
		LVSTATUS(hwnd) &= ~LVST_INHEAD;

		ScreenToClient (hwnd, &mouseX, &mouseY);

                nCols = isInListViewHead (mouseX, mouseY, NULL, plvdata);
		p1 = lvGetHdrByCol (plvdata, nCols);
                if (p1 != plvdata->pHdrClicked)
	            break;

		lvToggleSortStatus(p1);
		lvSortItem (NULL, nCols, p1->sort, plvdata);
                plvdata->nItemSelected = sGetItemSeq (plvdata);

                InvalidateRect (hwnd, NULL, FALSE);
            }
	    else if (LVSTATUS(hwnd) & LVST_BDDRAG)
            {
                ReleaseCapture ();
                lstSetHScrollInfo (plvdata);
		LVSTATUS(hwnd) &= ~LVST_BDDRAG;
                InvalidateRect (hwnd, NULL, FALSE);
            }
            
            break;
        }

	/* listview defined messages */

        case LVM_COLSORT:
        {
	    int nCols = (int)wParam;

            if (lvSortItem (NULL, nCols, LOSORTED, plvdata) == 0) {
                plvdata->nItemSelected = sGetItemSeq (plvdata);
                InvalidateRect (hwnd, NULL, FALSE);
		return LV_OKAY;
	    }
	    return LV_ERR;
        }

	case LVM_SORTITEMS:
	{
	    //PLVSORTDATA sortData = (PLVSORTDATA)wParam;
	    PFNLVCOMPARE pfn = (PFNLVCOMPARE)lParam;

            if (lvSortItem (pfn, 0, LOSORTED, plvdata) == 0) {
                plvdata->nItemSelected = sGetItemSeq (plvdata);
                InvalidateRect (hwnd, NULL, FALSE);
		return LV_OKAY;
	    }
	    return LV_ERR;
	}

	case LVM_SETITEMHEIGHT:
	{
	    int height = (int)wParam; 

            plvdata->nItemHeight = height;
	    if (height < LV_ITEMH_DEF(hwnd))
                plvdata->nItemHeight = LV_ITEMH_DEF(hwnd);

            lstSetVScrollInfo (plvdata);
            InvalidateRect (hwnd, NULL, FALSE);

            return TRUE;
	}

	case LVM_SETHEADHEIGHT:
	{
	    int height = (int)wParam; 

            plvdata->nHeadHeight = height;
	    if (height < LV_HDRH_DEF(hwnd))
                plvdata->nHeadHeight = LV_HDRH_DEF(hwnd);

            lstSetVScrollInfo (plvdata);
            InvalidateRect (hwnd, NULL, FALSE);

            return TRUE;
	}
	
        case LVM_ADDITEM:
        {
            PLVITEM p1;
	    PITEMDATA pnew;
	    int index;

            p1 = (PLVITEM) lParam;

    	    pnew = itemNew (plvdata->nCols);
	    pnew->addData = p1->itemData;

            index = sAddItemToList (p1->nItem, pnew, plvdata); 

            lstSetVScrollInfo (plvdata);
            InvalidateRect (hwnd, NULL, FALSE);
	    return index;
        }
        case LVM_SETSUBITEMCOLOR:
        {
            PLVSUBITEM p1;
            RECT rect;
            
            p1 = (PLVSUBITEM) lParam;

            lvSetSubItemTextColor(p1->nItem, p1->subItem, p1->nTextColor, plvdata); 

    	    LV_GET_SUBITEM_RECT(p1->nItem, p1->subItem, rect);
            InvalidateRect (hwnd, &rect, FALSE);
	    return 0;
        }
        case LVM_SETSUBITEMTEXT:
        {
            PLVSUBITEM p1;
            RECT rect;
            
            p1 = (PLVSUBITEM) lParam;
            
            if (lvFillSubItem(p1->nItem, p1->subItem, p1->pszText, plvdata) < 0)
                return LV_ERR;

    	    LV_GET_SUBITEM_RECT(p1->nItem, p1->subItem, rect);
            InvalidateRect (hwnd, &rect, FALSE);
	    return LV_OKAY;
        }

        case LVM_FILLSUBITEM:
        case LVM_SETSUBITEM:
	{
            PLVSUBITEM p1;
            RECT rect;
            
            p1 = (PLVSUBITEM) lParam;
	    if ( (lvSetSubItem(p1, plvdata)) < 0 )
		return LV_ERR;

    	    LV_GET_SUBITEM_RECT(p1->nItem, p1->subItem, rect);
            InvalidateRect (hwnd, &rect, FALSE);
	    return LV_OKAY;
	}

	case LVM_GETITEM:
	{
            PLVITEM p1;
	    PITEMDATA pitem;
            
            p1 = (PLVITEM) lParam;
	    pitem = lvGetItemByRow (plvdata, p1->nItem);
	    if (!pitem)
		return LV_ERR;

	    p1->itemData = pitem->addData;
	    return LV_OKAY;
	}
        case LVM_GETITEMCOUNT:
        {
            return plvdata->nRows;
        }
        case LVM_GETSELECTEDITEM:
        {
            return plvdata->nItemSelected;
        }
	case LVM_GETCOLUMN:
	{
	    int col = (int)wParam;
	    PLVCOLUMN pcol = (PLVCOLUMN)lParam;
	    PLSTHDR ph = lvGetHdrByCol(plvdata, col);
	    if (!ph)
	        return -1;
	    pcol->width = ph->width;
	    strncpy (pcol->pszHeadText, ph->pTitle, pcol->nTextMax);
	    pcol->pszHeadText[pcol->nTextMax] = 0;
	    pcol->image = ph->Image;
	    pcol->pfnCompare = ph->pfnCmp;
	    pcol->colFlags = ph->flags;
	}
	case LVM_GETCOLUMNWIDTH:
        {
	    int col = (int)wParam;
	    PLSTHDR ph = lvGetHdrByCol(plvdata, col);
	    if (ph)
                return ph->width;
	    else
	        return -1;
        }
        case LVM_GETCOLUMNCOUNT:
        {
            return plvdata->nCols;
        }

        case LVM_GETSUBITEMLEN:
        {
            PLVSUBITEM p1;
            PSUBITEMDATA p2;

            p1 = (PLVSUBITEM) lParam;
            p2 = lvGetSubItem (p1->nItem, p1->subItem, plvdata);

            if (p2 == NULL || p2->pszInfo == NULL)
                return LV_ERR;

            return strlen (p2->pszInfo);
        }
        case LVM_GETSUBITEMTEXT:
        {
            PLVSUBITEM p1;
            PSUBITEMDATA p2;

            p1 = (PLVSUBITEM) lParam;
            p2 = lvGetSubItem (p1->nItem, p1->subItem, plvdata);
            strcpy (p1->pszText, p2->pszInfo);

            return 0;
        }
        case LVM_ADDCOLUMN:
        {
            PLVCOLUMN p1;

            if ( !(p1 = (PLVCOLUMN)lParam) )
		return LV_ERR;

            if (!lvAddColumnToList (p1, plvdata))
		return LV_ERR;

            lstSetHScrollInfo (plvdata);
            InvalidateRect (hwnd, NULL, FALSE);

	    return LV_OKAY;
        }
        case LVM_MODIFYHEAD:
        {
            PLVCOLUMN p1 = (PLVCOLUMN) lParam;
	    if (!p1)
		return LV_ERR;
            if (sModifyHeadText (p1->nCols, p1->pszHeadText, plvdata) >= 0) {
                InvalidateRect (hwnd, NULL, FALSE);
		return LV_OKAY;
	    }
	    return LV_ERR;
        }
	case LVM_SETCOLUMN:
	{
            PLVCOLUMN p1 = (PLVCOLUMN) lParam;
	    PLSTHDR ph;

	    if (!p1)
		return LV_ERR;
	    if ( !(ph = lvGetHdrByCol (plvdata, p1->nCols)) )
		return LV_ERR;

            sModifyHeadText (p1->nCols, p1->pszHeadText, plvdata);
	    lvSetColumnWidth (p1->nCols, p1->width, plvdata);
	    ph->Image = p1->image;
	    ph->flags = p1->colFlags;
	    
            InvalidateRect (hwnd, NULL, FALSE);
	    return LV_OKAY;
	}
        case LVM_FINDITEM:
        {
            PLVFINDINFO p1;
            p1 = (PLVFINDINFO) lParam;
            return lvFindItem(p1, plvdata);
        }
        case LVM_DELITEM:
        {
            if (sRemoveItemFromList ((int)wParam, plvdata)) 
		return LV_ERR;
	    return LV_OKAY;
        }
        case LVM_DELALLITEM:
        {
            sRemoveAllItem (plvdata); 
	    return LV_OKAY;
        }

	//TODO
        case LVM_CLEARSUBITEM:
            break;

        case LVM_DELCOLUMN:
        {
            if (lvRemoveColumn ((int)wParam, plvdata))
		return LV_ERR;
	    return LV_OKAY;
        }

        case LVM_SELECTITEM:
            lvSelectItem (hwnd, (int)wParam, plvdata);
	    return 0;
        case LVM_SHOWITEM:
	    lvMakeItemVisible (hwnd, plvdata, (int)wParam);
	    return 0;
        case LVM_CHOOSEITEM:
            lvSelectItem(hwnd, (int)wParam, plvdata); 
	    lvMakeItemVisible (hwnd, plvdata, (int)wParam);
	    return 0;

	case LVM_GETITEMADDDATA:
	{
	    PITEMDATA pitem = lvGetItemByRow (plvdata, (int)wParam);
	    if (pitem)
		return pitem->addData;
	    else
		return LV_ERR;
	}

	case LVM_SETITEMADDDATA:
	{
	    PITEMDATA pitem = lvGetItemByRow (plvdata, (int)wParam);
	    if (pitem) {
		pitem->addData = (DWORD)lParam;
		return LV_OKAY;
	    }
	    else
		return LV_ERR;
	}

	case LVM_SETSTRCMPFUNC:
            if (lParam) {
                plvdata->str_cmp = (STRCMP)lParam;
                return 0;
            }
            return -1;
	break;

        case MSG_VSCROLL:
        {
	    lvVScroll (hwnd, wParam, lParam);
	    return 0;
        }

        case MSG_HSCROLL:
        {
	    lvHScroll (hwnd, wParam, lParam);
	    return 0;
        }

        case MSG_FONTCHANGED:
        {
	    if (plvdata->nItemHeight < LV_ITEMH_DEF(hwnd)) {
                plvdata->nItemHeight = LV_ITEMH_DEF(hwnd);
	    }
	    if (plvdata->nHeadHeight < LV_HDRH_DEF(hwnd)) {
                plvdata->nHeadHeight = LV_HDRH_DEF(hwnd);
	    }

            lstSetVScrollInfo (plvdata);
            InvalidateRect (hwnd, NULL, FALSE);

            return 0;
        }

        case MSG_LBUTTONDBLCLK:
        {
            int mouseX = LOSWORD (lParam);
            int mouseY = HISWORD (lParam);
            PITEMDATA p2;

            if (isInLVItem (mouseX, mouseY, &p2, plvdata) > 0)
                NotifyParent (hwnd, GetDlgCtrlID(hwnd), LVN_ITEMDBCLK);
            
            break;
        }

	case MSG_RBUTTONUP:
	case MSG_RBUTTONDOWN:
	{
            int mouseX = LOSWORD (lParam);
            int mouseY = HISWORD (lParam);
            int nCols, nRows;

            RECT rcClient;
            PLSTHDR p1;
            PITEMDATA p2;

	    LVNM_NORMAL lvnm;
	    lvnm.wParam = wParam;
	    lvnm.lParam = lParam;

            GetClientRect (hwnd, &rcClient);

            // clicks on the header
            if ( (nCols = isInListViewHead (mouseX, mouseY, &p1, plvdata)) > 0 )
	    {
		if (message == MSG_RBUTTONDOWN)
	            NotifyParentEx (hwnd, GetDlgCtrlID(hwnd), LVN_HEADRDOWN, 
				    (DWORD)&lvnm);
		else
	            NotifyParentEx (hwnd, GetDlgCtrlID(hwnd), LVN_HEADRUP, 
				    (DWORD)&lvnm);
	    }
	    // clicks on an item
	    else if ( (nRows = isInLVItem (mouseX, mouseY, &p2, plvdata)) > 0) {
		if (message == MSG_RBUTTONDOWN) {
            	    lvSelectItem (hwnd, nRows, plvdata);
	            lvMakeItemVisible (hwnd, plvdata, nRows);
                    NotifyParent (hwnd, GetDlgCtrlID(hwnd), LVN_SELCHANGE);
	            NotifyParentEx (hwnd, GetDlgCtrlID(hwnd), LVN_ITEMRDOWN, (DWORD)&lvnm);
		}
		else {
	            NotifyParentEx (hwnd, GetDlgCtrlID(hwnd), LVN_ITEMRUP, (DWORD)&lvnm);
		}
            }
	    break;
	}

        case MSG_KEYDOWN:
        {
            int id = LOWORD (wParam);
	    LVNM_NORMAL lvnm;
	    int nItem = 0;

            switch (id)
            {
                case SCANCODE_CURSORBLOCKDOWN:
		    nItem = plvdata->nItemSelected+1;
                    break;
                case SCANCODE_CURSORBLOCKUP:
		    nItem = plvdata->nItemSelected-1;
                    break;
                case SCANCODE_HOME:
		    nItem = 1;
                    break;
                case SCANCODE_END:
		    nItem = plvdata->nRows;
                    break;

                case SCANCODE_CURSORBLOCKLEFT:
                    break;
                case SCANCODE_CURSORBLOCKRIGHT:
                    break;

                case SCANCODE_PAGEUP:
		    SendMessage (hwnd, MSG_VSCROLL, SB_PAGEUP, 0);
                    break;
                case SCANCODE_PAGEDOWN:
		    SendMessage (hwnd, MSG_VSCROLL, SB_PAGEDOWN, 0);
                    break;
            }

	    if (LV_BE_VALID_ROW(nItem)) {
            	lvSelectItem (hwnd, nItem, plvdata);
		lvMakeItemVisible (hwnd, plvdata, nItem);
	    }

	    lvnm.wParam = wParam;
	    lvnm.lParam = lParam;
	    NotifyParentEx (hwnd, GetDlgCtrlID(hwnd), LVN_KEYDOWN, (DWORD)&lvnm);
            
            break;
        }

        case MSG_DESTROY:
        {
            lvDataDestory (plvdata);
            break;
        }
    }

    return DefaultControlProc (hwnd, message, wParam, lParam);
}

BOOL RegisterListViewControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = CTRL_LISTVIEW;
    WndClass.dwStyle = WS_NONE;
    WndClass.dwExStyle = WS_EX_NONE;
    WndClass.hCursor = GetSystemCursor (0);
    WndClass.iBkColor = PIXEL_lightwhite;
    WndClass.WinProc = sListViewProc;

    return RegisterWindowClass (&WndClass);
}

void ListViewControlCleanup (void)
{
    UnregisterWindowClass (CTRL_LISTVIEW);
}

#endif /* _EXT_CTRL_LISTVIEW */
