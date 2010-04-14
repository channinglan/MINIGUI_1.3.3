/*
** $Id: treeview.c,v 1.30 2003/09/29 02:52:46 snig Exp $
** 
** treeview.c: TreeView Control.
** 
** Copyright (C) 2001, 2002 Zheng Yiran, Wei Yongming.
** Copyright (C) 2003 Feynman Software.
**
** Current maintainer: Wei Yongming.
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
** TODO:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

#ifdef _EXT_CTRL_TREEVIEW

#include "mgext.h"
#include "treeview.h"

#define _USE_FIXSTR         1

#define TV_BORDER           2
#define TV_BOXHALFHEIGHT    4
#define TV_BOXGAP           3
#define TV_ICONGAP          3
#define TV_IDENT            5

static HICON icon_fold, icon_unfold;

#define ICON_FOLD       icon_fold
#define ICON_UNFOLD     icon_unfold

static int TreeViewCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam);

BOOL RegisterTreeViewControl (void)
{
    WNDCLASS WndClass;

    if (!(icon_fold = LoadSystemIcon (SYSICON_FOLD, 0))) {
        return FALSE;
    }

    if (!(icon_unfold = LoadSystemIcon (SYSICON_UNFOLD, 0)))
        return FALSE;

    WndClass.spClassName = CTRL_TREEVIEW;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (0);
    WndClass.iBkColor    = PIXEL_lightwhite;
    WndClass.WinProc     = TreeViewCtrlProc;

    return RegisterWindowClass (&WndClass);
}

void TreeViewControlCleanup (void)
{
    UnregisterWindowClass (CTRL_TREEVIEW);
}

/*
 * Initialize Treeview.
 */
static BOOL tvInitTVData (HWND hwnd, PTVDATA pData, DWORD dwStyle, PTVITEMINFO root_ii)
{    
    PTVITEM    tvRoot;
    HDC hdc;
    char* text = root_ii ? root_ii->text : "Root";

    pData->dwStyle = dwStyle;
    pData->id = GetDlgCtrlID (hwnd);

    pData->str_cmp = strncmp;

    if (!(tvRoot = calloc (1, sizeof (TVITEM))))
        return FALSE;

#if _USE_FIXSTR
    tvRoot->text = FixStrAlloc (strlen(text));
    if (tvRoot->text == NULL) {
        free (tvRoot);
        return FALSE;
    }
    strcpy (tvRoot->text, text);
#else
    tvRoot->text = strdup (text);
#endif
    tvRoot->dwFlags = TVIF_ROOT | TVIF_SELECTED;
    if (root_ii && root_ii->dwFlags & TVIF_FOLD) {
        tvRoot->dwFlags |= TVIF_FOLD;
    }

    tvRoot->depth = 0;
    tvRoot->child = tvRoot->next = tvRoot->parent = NULL;
    
    if (pData->dwStyle & TVS_WITHICON) {
        tvRoot->hIconFold = (root_ii ? root_ii->hIconFold : ICON_FOLD);
        tvRoot->hIconUnfold = (root_ii ? root_ii->hIconUnfold : ICON_UNFOLD);
    }

    hdc = GetClientDC (hwnd);

    pData->nItemHeight = GetFontHeight (hdc);
    pData->nVisItemCount = 1;
    pData->nItemCount = 1;
    pData->root = tvRoot;
    pData->pItemSelected = tvRoot;
    pData->nItemTop = 0;
    pData->nLeft = 0;

    GetTextExtent (hdc, tvRoot->text, -1, &tvRoot->text_ext);
    pData->nWidth = TV_BOXGAP + pData->nItemHeight + tvRoot->text_ext.cx;

    if (pData->dwStyle & TVS_WITHICON)
        pData->nWidth += pData->nItemHeight + TV_ICONGAP;
    if (pData->dwStyle & WS_BORDER)
        pData->nWidth += TV_BORDER * 2;

    ReleaseDC (hdc);
    return TRUE;
}

/*
 * Get the parent node of the given node.
 */
static PTVITEM getParent (PTVDATA pData, PTVITEM pChild)
{ 
    if (pChild == pData->root || pChild == NULL)
        return NULL;

    return pChild->parent;
}

/*
 * Get the node whose next sibling is current node.
 */
static PTVITEM getPrev (PTVDATA pData, PTVITEM pCur)
{
    PTVITEM p, t;

    p = getParent (pData, pCur);
    if (p == NULL || p->child == pCur)
        return NULL;

    t = p->child;
    while (t->next != NULL && t->next != pCur)
        t = t->next;

    if (t->next != pCur)
        return NULL;

    return t;
}

#if 0
static void UnfoldAll (PTVITEM p)
{
    PTVITEM t;

    if (p == NULL)
        return;

    p->dwFlags &= ~TVIF_FOLD;
    t = p->child;
    while (t) {
        UnfoldAll (t);
        t = t->next;
    }
}
#endif

/*
 * Mark all ancestor items unfolded
 */
static void unfold_ancestor (HWND hwnd, PTVDATA pData, PTVITEM item)
{
    PTVITEM parent;

    parent = item->parent;
    while (parent) {
        parent->dwFlags &= ~TVIF_FOLD;
        NotifyParentEx (hwnd, pData->id, TVN_UNFOLDED, (DWORD)parent);
        parent = parent->parent;
    }
}

static void vis_item_count (PTVITEM p, int* count)
{
    PTVITEM t;

    (*count)++;
    if (!(p->dwFlags & TVIF_FOLD)) {
        t = p->child;
        while (t) {
            vis_item_count (t, count);
            t = t->next;
        }
    }
}
        
static int getVisItemCount (PTVDATA pData)
{
    int count = 0;

    if (pData->root == NULL)
        return 0;

    vis_item_count (pData->root, &count);
    return count;
}

static void TVWidth (PTVDATA pData, PTVITEM p, int* width)
{
    PTVITEM t;
    int wtemp, h;

    h = pData->nItemHeight;
    wtemp = TV_BOXGAP + h + TV_BOXGAP + p->depth * h + p->text_ext.cx;
    if (pData->dwStyle & TVS_WITHICON)
        wtemp += h + TV_ICONGAP;
    if (pData->dwStyle & WS_BORDER)
        wtemp += TV_BORDER * 2;
    *width = MAX (wtemp, *width);
    if (!(p->dwFlags & TVIF_FOLD)) {
        t = p->child;
        while (t) {
            TVWidth (pData, t, width);
            t = t->next;
        }
    }
}
            
static int getTVWidth (PTVDATA pData)
{
    int width;

    TVWidth (pData, pData->root, &width);

    return width;
}

/*
 * Used by RemoveTree(), recursive function
 */
static void RemoveSubTree (PTVDATA pData, PTVITEM p)
{
    PTVITEM q, n;
    if (p == NULL )
        return;

    q = p->child;
    while (q != NULL) {
        n = q->next;
        RemoveSubTree (pData, q);
        q = n;
    }

    /* free a node which hasn't any child */
#if _USE_FIXSTR
    FreeFixStr (p->text);
#else
    free (p->text);
#endif
    free (p);
    pData->nItemCount--;    
}

/*
 * Remove a tree or subtree
 */
static void RemoveTree (HWND hwnd, PTVDATA pData, PTVITEM pChild)
{
    PTVITEM p, q;

    if (pChild == NULL)
        return;

    if (pChild == pData->root) {
        pData->root = NULL;
        pData->pItemSelected = NULL;
        pData->nVisItemCount = 1;
        pData->nWidth = 1;
        pData->nItemTop = pData->nLeft = 0;
        RemoveSubTree (pData, pChild);
    }
    else {
        p = pData->pItemSelected = getParent (pData, pChild);
        if (pChild == p->child)
            p->child = pChild->next;
        else {
            q = p->child;
            while (q !=NULL && q->next != pChild)
                q = q->next;
            q->next = pChild->next;
        }
        RemoveSubTree (pData, pChild);
        pData->nWidth = getTVWidth (pData);
        pData->nVisItemCount = getVisItemCount (pData);
    }
}

static void tvCleanTVData (HWND hwnd, PTVDATA pData)
{
    RemoveTree (hwnd, pData, pData->root);

#if 0
    printf ("Item count should be zero now: %d\n", pData->nItemCount);
#endif

    free (pData);
}

static int is_descendant (PTVITEM root, PTVITEM item, int* depth)
{
    int i;
    int d;
   
    if (root == NULL || item == NULL)
        return -1;

    d = item->depth - root->depth;

    for (i = 0; i < d; i++) {
        item = item->parent;
    }

    if (depth) *depth = d;

    if (item == root)
        return 0;

    return -1;
}

static PTVITEM findStringDepthFirst (PTVDATA pData, PTVITEM p, const char* string)
{
    PTVITEM q, t = NULL;

    if (!pData->str_cmp (p->text, string, (size_t)-1))
        return p;
    else {
        q = p->child;
        while (q && !(t = findStringDepthFirst (pData, q, string)))
            q = q->next;
    }

    return t;
}

static PTVITEM findStringInChildren (PTVDATA pData, PTVITEM item, const char* string)
{
    PTVITEM p;

    p = item->child;
    while (p) {
        if (!pData->str_cmp (p->text, string, (size_t)-1))
            return p;
        p = p->next;
    }

    return NULL;
}

static void countPrevNext (PTVDATA pData, PTVITEM prev, int* count)
{
    PTVITEM t;

    (*count)++;
    if (!(prev->dwFlags & TVIF_FOLD)) {
        t = prev->child;
        while (t) {
            countPrevNext (pData, t, count);
            t = t->next;
        }
    }
}

static int getItemsWithPrev (PTVDATA pData, PTVITEM pCur)
{
    int count = 0;
    PTVITEM prev = getPrev (pData, pCur);

    if (prev == NULL)
        return 0;

    countPrevNext (pData, prev, &count);
    return count - 1;
}

static int getItemsWithNext (PTVDATA pData, PTVITEM pCur)
{
    int count = 0;

    if (pCur == NULL || pCur->next == NULL)
        return 0;

    countPrevNext (pData, pCur, &count);
    return count - 1;
}

static PTVITEM ItemFromCount (PTVITEM p, int count, int* cnt)
{
    PTVITEM q, t = NULL;

    (*cnt)++;

    if (*cnt == count)
        return p;
    else if (p->child != NULL && !(p->dwFlags & TVIF_FOLD)) {
        q = p->child;
        while (q != NULL && !(t = ItemFromCount (q, count, cnt)))
            q = q->next;
    } else 
        return NULL;

    return t;
}

static PTVITEM getItemFromCount (PTVDATA pData, int count)
{
    int cnt = 0;

    if (pData->root == NULL)
        return NULL;

    return ItemFromCount (pData->root, count, &cnt);
}

static int CountFromItem (PTVITEM p, PTVITEM pTgt, int* count)
{
    int t = 0;
    PTVITEM q;

    (*count)++;
    if (p == pTgt)
        return *count;
    else if (p->child != NULL && !(p->dwFlags & TVIF_FOLD)) {
        q = p->child;
        while (q != NULL && !(t = CountFromItem (q, pTgt, count)))
            q = q->next;
    } else
        return 0;

    return t;
}

static int getCountFromItem (PTVDATA pData, PTVITEM pTgt)
{
    int count = 0;

    if (pTgt == NULL)
        return 0;

    return CountFromItem (pData->root, pTgt, &count);
}

/*
 * Add a node
 */
static void InsertChild (HWND hwnd, PTVDATA pData, PTVITEM pParent, PTVITEM pChild)
{
    PTVITEM p, t = NULL;
    int h, w;
    HDC hdc;

    pChild->depth = pParent->depth + 1;
    pChild->parent = pParent;
    pChild->child = pChild->next = NULL;
    if (pParent->child == NULL)
        pParent->child = pChild;
    else if (pData->dwStyle & TVS_SORT) {
        p = pParent->child;
        while (pData->str_cmp (pChild->text, p->text, (size_t)-1) > 0 && p->next != NULL) {
            t = p;
            p = p->next;
        }
        if (pData->str_cmp (pChild->text, p->text, (size_t)-1) > 0)
            p->next = pChild;
        else if (t == NULL) {
            pParent->child = pChild;
            pChild->next = p;
        } else {
            t->next = pChild;
            pChild->next = p;
        }
    }
    else {
        p = pParent->child;
        while (p->next != NULL)
            p = p->next;
        p->next = pChild;
    }
    pData->nItemCount++;
    pData->nVisItemCount++;

    hdc = GetClientDC (hwnd);
    GetTextExtent (hdc, pChild->text, -1, &pChild->text_ext);
    h = pData->nItemHeight;
    w = TV_BOXGAP + h + TV_BOXGAP + pChild->text_ext.cx + pChild->depth * h;
    if (pData->dwStyle & TVS_WITHICON)
        w += h + TV_ICONGAP;
    if (pData->dwStyle & WS_BORDER)
        w += TV_BORDER * 2;
    pData->nWidth = MAX (w, pData->nWidth);
    ReleaseDC (hdc);
}
    
static void tvDrawItem (HDC hdc, PTVDATA pData, int centerX, int centerY, 
                PTVITEM p, int up, int down)
{
    int h = pData->nItemHeight;

    SetPenColor (hdc, PIXEL_black);
    SetBkColor (hdc, PIXEL_lightwhite);

    if (p->child != NULL) {
        Rectangle (hdc, centerX - TV_BOXHALFHEIGHT, centerY - TV_BOXHALFHEIGHT, 
                centerX + TV_BOXHALFHEIGHT, centerY + TV_BOXHALFHEIGHT);
        MoveTo (hdc, centerX - TV_BOXHALFHEIGHT + 2, centerY);
        LineTo (hdc, centerX + TV_BOXHALFHEIGHT - 1, centerY);
        if (p->dwFlags & TVIF_FOLD) {
            MoveTo (hdc, centerX, centerY - TV_BOXHALFHEIGHT + 2);
            LineTo (hdc, centerX, centerY + TV_BOXHALFHEIGHT - 2);
        }        
        MoveTo (hdc, centerX, centerY - (h>>1) - h * up);
        LineTo (hdc, centerX, centerY - TV_BOXHALFHEIGHT);
        MoveTo (hdc, centerX + TV_BOXHALFHEIGHT, centerY);
        LineTo (hdc, centerX + (h>>1) + TV_BOXGAP, centerY);
        if (p->next != NULL) {
            MoveTo (hdc, centerX, centerY + TV_BOXHALFHEIGHT);
            LineTo (hdc, centerX, centerY + (h>>1) + h * down);
        }
    } else {
        MoveTo (hdc, centerX, centerY - (h>>1) - h * up);
        LineTo (hdc, centerX, centerY);
        LineTo (hdc, centerX + (h>>1) + TV_BOXGAP, centerY);
        if (p->next != NULL) {
            MoveTo (hdc, centerX, centerY);
            LineTo (hdc, centerX, centerY + (h>>1) + h * down);
        }
    }
    centerX += (h>>1) + TV_BOXGAP;

    if (pData->dwStyle & TVS_WITHICON) {
        if (!(pData->dwStyle & TVS_ICONFORSELECT)
                        && (!p->child || (p->dwFlags & TVIF_FOLD))) {
            if ((p->hIconFold)) {
                DrawIcon (hdc, centerX, centerY - (h>>1), 
                        h, h, p->hIconFold);
            }
        } else if ((pData->dwStyle & TVS_ICONFORSELECT) && p->dwFlags & TVIF_SELECTED) {
            if ((p->hIconFold)) {
                DrawIcon (hdc, centerX, centerY - (h>>1), 
                        h, h, p->hIconFold);
            }
        } else 
            if ((p->hIconUnfold)) {
                DrawIcon (hdc, centerX, centerY - (h>>1), 
                        h, h, p->hIconUnfold);
            }
        centerX += h + TV_ICONGAP;
    }

    if (p->dwFlags & TVIF_SELECTED) {
#ifndef _GRAY_SCREEN
        SetBrushColor (hdc, PIXEL_darkblue);
        SetBkColor (hdc, PIXEL_darkblue);
#else
        SetBrushColor (hdc, PIXEL_black);
        SetBkColor (hdc, PIXEL_black);
#endif

        SetTextColor (hdc, PIXEL_lightwhite);
        TextOut (hdc, centerX, centerY - (h>>1), p->text);
#ifndef _GRAY_SCREEN
        if (pData->dwStyle & TVS_FOCUS) {
            FocusRect (hdc, centerX, centerY - (h>>1), 
                        centerX + p->text_ext.cx - 1,
                        centerY + (h>>1) - 1);
        }
#endif
    } else {
        SetBrushColor (hdc, PIXEL_lightwhite);
        SetBkColor (hdc, PIXEL_lightwhite);
        SetTextColor (hdc, PIXEL_black);
        TextOut (hdc, centerX, centerY - (h>>1), p->text);
    }
}

/*
 * draw all items.
 */
static void doAll (HDC hdc, PTVDATA pData, PTVITEM p, int cenX, int cenY, int* count, int* depth)
{
    int up, down, h = pData->nItemHeight;
    PTVITEM t;

    up = getItemsWithPrev (pData, p);
    down = getItemsWithNext (pData, p);
    tvDrawItem (hdc, pData, cenX + *depth * h, cenY + (*count - pData->nItemTop) * h, 
                    p, up, down);
    (*count)++;
    if (!(p->dwFlags & TVIF_FOLD)) {
        t = p->child;
        (*depth)++;
        while (t) {
            doAll (hdc, pData, t, cenX, cenY, count, depth);
            if (*count > pData->nItemTop + pData->nVisCount) 
                return;
            t = t->next;
        }
        (*depth)--;
    }
}

static void tvOnDraw (HWND hwnd, HDC hdc, PTVDATA pData)
{
    RECT    rcClient;
    int     x, y, w, h;
    int    centerX, centerY;
    int    count = 0, depth = 0;

    GetClientRect (hwnd, &rcClient);

    x = rcClient.left;
    y = rcClient.top;
    w = RECTW (rcClient);
    h = RECTH (rcClient);
    if (pData->dwStyle & WS_BORDER) {
        x += TV_BORDER;
        y += TV_BORDER;
        w -= (TV_BORDER<<1);
        h -= (TV_BORDER<<1);
    }        
    SetBrushColor (hdc, PIXEL_lightwhite);
    FillBox (hdc, rcClient.left, rcClient.top, RECTW (rcClient), RECTH (rcClient));
    
    centerX = x - pData->nLeft + TV_BOXGAP + (pData->nItemHeight>>1);
    centerY = y + (pData->nItemHeight >> 1);
    doAll (hdc, pData, pData->root, centerX, centerY, &count, &depth);
}

static void tvSetVScrollInfo (HWND hwnd, PTVDATA pData, BOOL fRedraw)
{
    SCROLLINFO si;

    if (pData->nVisCount >= pData->nVisItemCount) {
        SetScrollPos (hwnd, SB_VERT, 0);
        EnableScrollBar (hwnd, SB_VERT, FALSE);
        ShowScrollBar (hwnd, SB_VERT, FALSE);
        return;
    }

    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMax = pData->nVisItemCount - 1;
    si.nMin = 0;
    si.nPage = MIN (pData->nVisCount, (pData->nVisItemCount - pData->nItemTop));
    si.nPos = pData->nItemTop;
    SetScrollInfo (hwnd, SB_VERT, &si, fRedraw);
    EnableScrollBar (hwnd, SB_VERT, TRUE);
    ShowScrollBar (hwnd, SB_VERT, TRUE);
}
                
static void tvSetHScrollInfo (HWND hwnd, PTVDATA pData, BOOL fRedraw)
{
    SCROLLINFO si;

    if (pData->nVisWidth >= pData->nWidth) {
        SetScrollPos (hwnd, SB_HORZ, 0);
        ShowScrollBar (hwnd, SB_HORZ, FALSE);
        EnableScrollBar (hwnd, SB_HORZ, FALSE);
        return;
    }

    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMax = pData->nWidth - 1;
    si.nMin = 0;
    si.nPage = MIN (pData->nVisWidth, (pData->nWidth - pData->nLeft));
    si.nPos = pData->nLeft;
    SetScrollInfo (hwnd, SB_HORZ, &si, fRedraw);
    ShowScrollBar (hwnd, SB_HORZ, TRUE);
    EnableScrollBar (hwnd, SB_HORZ, TRUE);
}

static void tvRecalcWidthsHelper (HDC hdc, PTVITEM p)
{
    while (p) {

        GetTextExtent (hdc, p->text, -1, &p->text_ext);

        if (p->child)
            tvRecalcWidthsHelper (hdc, p->child);

        p = p->next;
    }
}

static void tvRecalcWidths (HWND hwnd, PTVDATA pData)
{
    HDC hdc = GetClientDC (hwnd);
    tvRecalcWidthsHelper (hdc, pData->root);
    ReleaseDC (hdc);

    pData->nWidth = getTVWidth (pData);
    pData->nVisItemCount = getVisItemCount (pData);
    InvalidateRect (hwnd, NULL, FALSE);
    tvSetVScrollInfo (hwnd, pData, TRUE);
    tvSetHScrollInfo (hwnd, pData, TRUE);
}

/*
 * Recalculate and redraw the control
 */
static void recalc_redraw (HWND hwnd, PTVDATA pData)
{
    pData->nVisItemCount = getVisItemCount (pData);
    pData->nWidth = getTVWidth (pData);
    InvalidateRect (hwnd, NULL, FALSE);
    tvSetVScrollInfo (hwnd, pData, TRUE);
    tvSetHScrollInfo (hwnd, pData, TRUE);
}

static void change_selected (HWND hwnd, PTVDATA pData, PTVITEM new_sel)
{
    PTVITEM old_sel = pData->pItemSelected;

    if (new_sel != old_sel) {
        if (old_sel) old_sel->dwFlags &= ~TVIF_SELECTED;
        if (new_sel) new_sel->dwFlags |= TVIF_SELECTED;
        pData->pItemSelected = new_sel;

        NotifyParent (hwnd, pData->id, TVN_SELCHANGE);
    }
}

static int TreeViewCtrlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    PTVDATA pData;
    DWORD dwStyle;
    
    switch (message) {
        case MSG_CREATE:
        {
            PTVITEMINFO root_ii = (PTVITEMINFO)lParam;

            dwStyle = GetWindowStyle (hwnd);

            if (!(pData = (PTVDATA) calloc (1, sizeof (TVDATA))))
                return -1;
            SetWindowAdditionalData2 (hwnd, (DWORD) pData);

            if (!tvInitTVData (hwnd, pData, dwStyle, root_ii)) {
                free (pData);
                return -1;
            }

            break;
        }
    
        case MSG_SIZECHANGED:
        {
            RECT* prc = (RECT*) lParam;
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            pData->nVisCount = (RECTHP (prc)) / pData->nItemHeight;
            pData->nVisWidth = RECTWP (prc);
        }
        break;

        case MSG_DESTROY:
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            tvCleanTVData (hwnd, pData);
        break;

        case MSG_GETTEXTLENGTH:
        case MSG_GETTEXT:
        case MSG_SETTEXT:
            return -1;

        case MSG_PAINT:
        {
            HDC hdc;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            hdc = BeginPaint (hwnd);
            tvOnDraw (hwnd, hdc, pData);
            EndPaint (hwnd, hdc);
            return 0;
        }
        
        case TVM_SETSTRCMPFUNC:
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            if (pData->nItemCount == 1 && lParam) {
                pData->str_cmp = (STRCMP)lParam;
                return 0;
            }
            return -1;

        case TVM_GETROOT:
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            return (int) pData->root;
        break;

        case TVM_SEARCHITEM:
        {
            const char *text;
            PTVITEM root, matched;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            root = (PTVITEM)wParam;
            text = (const char *)lParam;

            if (root == NULL)
                root = pData->root;
            if (text == NULL || is_descendant (pData->root, root, NULL))
                return 0;

            matched = findStringDepthFirst (pData, root, text);
            return (int)matched;
        }
        
        case TVM_FINDCHILD:
        {
            const char *text;
            PTVITEM item, matched;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            item = (PTVITEM)wParam;
            text = (const char *)lParam;

            if (item == NULL)
                item = pData->root;
            if (text == NULL || is_descendant (pData->root, item, NULL))
                return 0;

            matched = findStringInChildren (pData, item, text);
            return (int)matched;
        }
        
        case TVM_GETSELITEM:
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            return (int)pData->pItemSelected;

        case TVM_SETSELITEM:
        {
            PTVITEM old_sel, new_sel = (PTVITEM)wParam;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            old_sel = pData->pItemSelected;

            if (old_sel != new_sel) {
                if (is_descendant (pData->root, new_sel, NULL))
                    return -1;

                old_sel->dwFlags &= ~TVIF_SELECTED;
                new_sel->dwFlags |= TVIF_SELECTED;
                pData->pItemSelected = new_sel;
                NotifyParent (hwnd, pData->id, TVN_SELCHANGE);

                unfold_ancestor (hwnd, pData, new_sel);
                recalc_redraw (hwnd, pData);
            }

            return (int)old_sel;
        }

        case TVM_GETRELATEDITEM:
        {
            int related = (int) wParam;
            PTVITEM item = (PTVITEM) lParam;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            if (is_descendant (pData->root, item, NULL))
                return 0;

            switch (related) {
                    case TVIR_PARENT:
                        return (int)item->parent;
                    case TVIR_FIRSTCHILD:
                        return (int)item->child;
                    case TVIR_NEXTSIBLING:
                        return (int)item->next;
                    case TVIR_PREVSIBLING:
                        return (int)getPrev (pData, item);
            }

            return 0;
        }

        case TVM_GETITEMTEXTLEN:
        {
            PTVITEM item = (PTVITEM) wParam;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            if (is_descendant (pData->root, item, NULL))
                return -1;

            return strlen (item->text);
        }

        case TVM_GETITEMTEXT:
        {
            PTVITEM item = (PTVITEM) wParam;
            char* buffer = (char*) lParam;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            if (is_descendant (pData->root, item, NULL))
                return -1;

            strcpy (buffer, item->text);
            return strlen (buffer);
        }

        case TVM_GETITEMINFO:
        {
            PTVITEM item = (PTVITEM)wParam;
            PTVITEMINFO info = (PTVITEMINFO)lParam;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            if (is_descendant (pData->root, item, NULL))
                return -1;

            if (info) {
                if (info->text)
                    strcpy (info->text, item->text);
                info->dwFlags = item->dwFlags;
                info->hIconFold = item->hIconFold;
                info->hIconUnfold = item->hIconUnfold;
                info->dwAddData = item->dwAddData;
            }

            return 0;
        }

        case TVM_SETITEMINFO:
        {
            PTVITEM item = (PTVITEM)wParam;
            PTVITEMINFO info = (PTVITEMINFO)lParam;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            if (is_descendant (pData->root, item, NULL))
                return -1;

            if (info) {
                if (info->text) {
#if _USE_FIXSTR
                    FreeFixStr (item->text);
                    item->text = FixStrAlloc (strlen (info->text));
                    if (item->text == NULL) {
                        NotifyParent (hwnd, pData->id, TVN_ERRSPACE);
                        return -1;
                    }
                    strcpy (item->text, info->text);
#else
                    free (item->text);
                    item->text = strdup (info->text);
                    if (item->text == NULL) {
                        NotifyParent (hwnd, pData->id, TVN_ERRSPACE);
                        return -1;
                    }
#endif
                }

                if (info->hIconFold)
                    item->hIconFold = info->hIconFold;
                if (info->hIconUnfold)
                    item->hIconFold = info->hIconUnfold;

                item->dwAddData = info->dwAddData;

                recalc_redraw (hwnd, pData);
            }
            return 0;
        }

        case TVM_DELTREE:
        {
            PTVITEM item = (PTVITEM)wParam;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            if (is_descendant (pData->root, item, NULL))
                return -1;

            if (is_descendant (item, pData->pItemSelected, NULL) == 0) {
                change_selected (hwnd, pData, item->parent);
            }

            RemoveTree (hwnd, pData, item);
            recalc_redraw (hwnd, pData);
            return 0;
        }
        
        case TVM_ADDITEM:
        case TVM_INSERTITEM:
        {
            PTVITEMINFO pTVItemInfo;
            PTVITEM parent, newItem = NULL;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            pTVItemInfo = (PTVITEMINFO) lParam;
            parent = (PTVITEM) wParam;

            if (parent == NULL) parent = pData->root;

            newItem = calloc (1, sizeof(TVITEM));
            if (newItem == NULL) {
                NotifyParent (hwnd, pData->id, TVN_ERRSPACE);
                return 0;
            }
            
#if _USE_FIXSTR
            newItem->text = FixStrAlloc (strlen(pTVItemInfo->text));
            if (newItem->text == NULL) {
                free (newItem);
                NotifyParent (hwnd, pData->id, TVN_ERRSPACE);
                return 0;
            }
            strcpy (newItem->text, pTVItemInfo->text);
#else
            newItem->text = strdup (pTVItemInfo->text);
            if (newItem->text == NULL) {
                free (newItem);
                NotifyParent (hwnd, pData->id, TVN_ERRSPACE);
                return 0;
            }
#endif
            if (pData->dwStyle & TVS_WITHICON) {
                if (pTVItemInfo->hIconFold)
                    newItem->hIconFold = (DWORD) pTVItemInfo->hIconFold;
                else 
                    newItem->hIconFold = (DWORD) ICON_FOLD;
                if (pTVItemInfo->hIconUnfold)
                    newItem->hIconUnfold = (DWORD) pTVItemInfo->hIconUnfold;
                else 
                    newItem->hIconUnfold = (DWORD) ICON_UNFOLD;
            } else {
                newItem->hIconFold = 0L;
                newItem->hIconUnfold = 0L;
            }

            if (pTVItemInfo->dwFlags & TVIF_FOLD) {
                newItem->dwFlags = TVIF_FOLD;
            }

            newItem->dwAddData = pTVItemInfo->dwAddData;

            InsertChild (hwnd, pData, parent, newItem);
            tvSetVScrollInfo (hwnd, pData, TRUE);
            tvSetHScrollInfo (hwnd, pData, TRUE);
            InvalidateRect (hwnd, NULL, FALSE);
            return (int) newItem;    
        }

        case MSG_KEYDOWN:
        {
            PTVITEM p, t;
            RECT rc;
            int count, c, rctop;
            GetClientRect (hwnd, &rc);
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            switch (LOWORD (wParam)) {
                case SCANCODE_CURSORBLOCKRIGHT:
                    p = pData->pItemSelected;
                    count = getCountFromItem (pData, p);
                    rctop = rc.top;
                    if (p->child == NULL)
                        return 0;
                    else if (p->dwFlags & TVIF_FOLD) {

                        p->dwFlags &= ~TVIF_FOLD;
                        NotifyParentEx (hwnd, pData->id, TVN_UNFOLDED, (DWORD)p);

                        pData->nWidth = getTVWidth (pData);
                        pData->nVisItemCount = getVisItemCount (pData);
                        count = count - pData->nItemTop - 1;
                        rc.top = rctop + count * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        InvalidateRect (hwnd, &rc, FALSE);
                        tvSetVScrollInfo (hwnd, pData, TRUE);
                        tvSetHScrollInfo (hwnd, pData, TRUE);
                    } else {
                        t = p->child;
                        change_selected (hwnd, pData, t);
                        c = getCountFromItem (pData, t);
                        if (c > pData->nItemTop + pData->nVisCount) {
                            pData->nItemTop = c - pData->nVisCount;
                            InvalidateRect (hwnd, NULL, FALSE);
                            tvSetVScrollInfo (hwnd, pData, TRUE);
                            break;
                        }
                        c = c - pData->nItemTop - 1;
                        rc.top = rctop + c * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                        count = count - pData->nItemTop - 1;
                        rc.top = rctop + count * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                    }            
                    break;

                case SCANCODE_CURSORBLOCKLEFT:
                    p = pData->pItemSelected;
                    count = getCountFromItem (pData, p);
                    rctop = rc.top;
                    if (p == NULL)
                        return 0;
                    if (p->child && !(p->dwFlags & TVIF_FOLD)) {

                        p->dwFlags |= TVIF_FOLD;
                        NotifyParentEx (hwnd, pData->id, TVN_FOLDED, (DWORD)p);

                        pData->nWidth = getTVWidth (pData);
                        pData->nVisItemCount = getVisItemCount (pData);
                        count = count - pData->nItemTop - 1;
                        rc.top = rctop + count * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        InvalidateRect (hwnd, &rc, FALSE);
                        tvSetVScrollInfo (hwnd, pData, TRUE);
                        tvSetHScrollInfo (hwnd, pData, TRUE);
                    } else {
                        t = getParent (pData, p);
                        if (t == NULL)
                            return 0;
                        change_selected (hwnd, pData, t);
                        c = getCountFromItem (pData, t);
                        if (c <= pData->nItemTop) {
                            pData->nItemTop = c - 1;
                            InvalidateRect (hwnd, NULL, FALSE);
                            tvSetVScrollInfo (hwnd, pData, TRUE);
                            break;
                        }
                        c = c - pData->nItemTop - 1;
                        rc.top = rctop + c * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                        count = count - pData->nItemTop - 1;
                        rc.top = rctop + count * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                    }
                    break;
            
                case SCANCODE_CURSORBLOCKUP:
                    p = pData->pItemSelected;
                    count = getCountFromItem (pData, p);
                    rctop = rc.top;
                    if (count <= 1)
                        break;
                    c = count - 1;
                    t = getItemFromCount (pData, c);
                    if (t == NULL) break;
                    change_selected (hwnd, pData, t);
                    if (c > pData->nItemTop && c <= pData->nItemTop + pData->nVisCount) {
                        c = c - pData->nItemTop - 1;
                        rc.top = rctop + c * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                        count = count - pData->nItemTop - 1;
                        rc.top = rctop + count * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                    } else {
                        if (c <= pData->nItemTop)
                            pData->nItemTop = c - 1;
                        else if (c >= pData->nItemTop + pData->nVisCount)
                            pData->nItemTop = c - pData->nVisCount;
                        InvalidateRect (hwnd, NULL, FALSE);
                        tvSetVScrollInfo (hwnd, pData, TRUE);
                    }
                    break;
                    
                case SCANCODE_CURSORBLOCKDOWN:
                    p = pData->pItemSelected;
                    count = getCountFromItem (pData, p);
                    rctop = rc.top;
                    if (count >= pData->nVisItemCount)
                        break;
                    c = count + 1;
                    t = getItemFromCount (pData, c);
                    if (t == NULL) break;
                    change_selected (hwnd, pData, t);
                    if (c > pData->nItemTop && c <= pData->nItemTop + pData->nVisCount) {
                        c = c - pData->nItemTop - 1;
                        rc.top = rctop + c * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                        count = count - pData->nItemTop - 1;
                        rc.top = rctop + count * pData->nItemHeight;
                        if (pData->dwStyle & WS_BORDER)
                            rc.top += TV_BORDER;
                        rc.bottom = rc.top + pData->nItemHeight;
                        InvalidateRect (hwnd, &rc, FALSE);
                    } else {
                        if (c <= pData->nItemTop)
                            pData->nItemTop = c - 1;
                        else if (c >= pData->nItemTop + pData->nVisCount)
                            pData->nItemTop = c - pData->nVisCount;
                        InvalidateRect (hwnd, NULL, FALSE);
                        tvSetVScrollInfo (hwnd, pData, TRUE);
                    }
                    break;
                
                case SCANCODE_PAGEDOWN:
                    p = pData->pItemSelected;
                    count = getCountFromItem (pData, p);
                    c = count + pData->nVisCount;
                    if (c > pData->nVisItemCount)
                        c = pData->nVisItemCount;
                    if (c <= pData->nItemTop)
                        pData->nItemTop = c - 1;
                    else if (c > pData->nItemTop + pData->nVisCount * 2)
                        pData->nItemTop = c - pData->nVisCount;
                    else if (c > pData->nItemTop + pData->nVisCount) {
                        pData->nItemTop += pData->nVisCount;
                        if (pData->nItemTop > pData->nVisItemCount - pData->nVisCount)
                            pData->nItemTop = pData->nVisItemCount - pData->nVisCount;
                    }
                    t = getItemFromCount (pData, c);
                    change_selected (hwnd, pData, t);
                    InvalidateRect (hwnd, NULL, FALSE);
                    tvSetVScrollInfo (hwnd, pData, TRUE);
                    break;
            
                case SCANCODE_PAGEUP:
                    p = pData->pItemSelected;
                    count = getCountFromItem (pData, p);
                    c = count - pData->nVisCount;
                    if (c < 1)
                        c = 1;
                    if (c <= pData->nItemTop - pData->nVisCount)
                        pData->nItemTop = c - 1;
                    else if (c <= pData->nItemTop) {
                        pData->nItemTop -= pData->nVisCount;
                        if (pData->nItemTop < 0) pData->nItemTop = 0;
                    }
                    else if (c > pData->nItemTop + pData->nVisCount)
                        pData->nItemTop = c - pData->nVisCount;
                    t = getItemFromCount (pData, c);
                    change_selected (hwnd, pData, t);
                    InvalidateRect (hwnd, NULL, FALSE);
                    tvSetVScrollInfo (hwnd, pData, TRUE);
                    break;
            
                case SCANCODE_HOME:
                    pData->nItemTop = 0;
                    change_selected (hwnd, pData, pData->root);
                    InvalidateRect (hwnd, NULL, FALSE);
                    tvSetVScrollInfo (hwnd, pData, TRUE);
                    break;
            
                case SCANCODE_END:
                    pData->nItemTop = pData->nVisItemCount - pData->nVisCount;
                    t = getItemFromCount (pData, pData->nVisItemCount);
                    change_selected (hwnd, pData, t);
                    InvalidateRect (hwnd, NULL, FALSE);
                    tvSetVScrollInfo (hwnd, pData, TRUE);
                    break;
            
                case SCANCODE_ENTER:
                    NotifyParent (hwnd, pData->id, TVN_ENTER);
                    break;

                default:
                    break;
            }
        }
        break;
        
        case MSG_LBUTTONDOWN:
        {
            int mouseX = LOSWORD(lParam);
            int mouseY = HISWORD(lParam);
            int right, left, count, h, c, rctop;
            RECT rc;
            PTVITEM p;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            count = mouseY / pData->nItemHeight + 1;
            count += pData->nItemTop;
            if (count > pData->nVisItemCount)
                break;

            p = getItemFromCount(pData, count);
            if (p == NULL) 
                break;

            h = pData->nItemHeight;
            left = - pData->nLeft + TV_BOXGAP + p->depth * h;
            if (pData->dwStyle & WS_BORDER)
                left += TV_BORDER;
            right = left + h + TV_BOXGAP + p->text_ext.cx;
            if (pData->dwStyle & TVS_WITHICON)
                right += h + TV_ICONGAP;
            if (mouseX < left || mouseX > right)
                break;

            GetClientRect (hwnd, &rc);
            rctop = rc.top;
            if (mouseX <= left + h && mouseX >= left) {
                if (p->child == NULL)
                    break;
                else {
                    int count;
                    CountFromItem (p, pData->pItemSelected, &count);
                    if (count != 0)
                        change_selected (hwnd, pData, p);
                }

                if (p->dwFlags & TVIF_FOLD) {
                    p->dwFlags &= ~TVIF_FOLD;
                    NotifyParentEx (hwnd, pData->id, TVN_UNFOLDED, (DWORD)p);
                }
                else {
                    p->dwFlags |= TVIF_FOLD;
                    NotifyParentEx (hwnd, pData->id, TVN_FOLDED, (DWORD)p);
                }

#if 0
                p->dwFlags = (p->dwFlags & ~TVIF_FOLD) | (~p->dwFlags & TVIF_FOLD);
#endif
                pData->nWidth = getTVWidth (pData);
                pData->nVisItemCount = getVisItemCount (pData);
                rc.top = rctop + (count - pData->nItemTop - 1) * pData->nItemHeight;
                if (pData->dwStyle & WS_BORDER)
                    rc.top += TV_BORDER;
                tvSetVScrollInfo (hwnd, pData, TRUE);
                tvSetHScrollInfo (hwnd, pData, TRUE);
                InvalidateRect (hwnd, &rc, FALSE);
                break;
            }
            left = left + h;
            if (mouseX <= right && mouseX >= left) {
                if (p == pData->pItemSelected)
                    break;

                c = getCountFromItem (pData, pData->pItemSelected);
                c = c - pData->nItemTop - 1;
                if (c < 0)
                    c = 0;

                change_selected (hwnd, pData, p);

                rc.top = rctop + c * pData->nItemHeight;
                if (pData->dwStyle & WS_BORDER)
                    rc.top += TV_BORDER;
                rc.bottom = rc.top + pData->nItemHeight;
                InvalidateRect (hwnd, &rc, FALSE);

                rc.top = rctop + (count - pData->nItemTop - 1) * pData->nItemHeight;
                if (pData->dwStyle & WS_BORDER)
                    rc.top += TV_BORDER;
                rc.bottom = rc.top + pData->nItemHeight;
                InvalidateRect (hwnd, &rc, FALSE);
            }
        }
        break;
        
        case MSG_LBUTTONDBLCLK:
        {
            int mouseX = LOSWORD(lParam);
            int mouseY = HISWORD(lParam);
            int right, left, count, h;
            PTVITEM p;
            RECT rc;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            count = mouseY / pData->nItemHeight + 1;
            count += pData->nItemTop;
            if (count > pData->nVisItemCount)
                break;

            p = getItemFromCount(pData, count);
            if (p == NULL) 
                break;

            h = pData->nItemHeight;
            left = - pData->nLeft + TV_BOXGAP + h + p->depth * h;
            if (pData->dwStyle & WS_BORDER)
                left += TV_BORDER;
            right = left + TV_BOXGAP + p->text_ext.cx;
            if (pData->dwStyle & TVS_WITHICON)
                right += h + TV_ICONGAP;
            if (p->child && mouseX <= right && mouseX >= left) {

                if (p->dwFlags & TVIF_FOLD) {
                    p->dwFlags &= ~TVIF_FOLD;
                    NotifyParentEx (hwnd, pData->id, TVN_UNFOLDED, (DWORD)p);
                }
                else {
                    p->dwFlags |= TVIF_FOLD;
                    NotifyParentEx (hwnd, pData->id, TVN_FOLDED, (DWORD)p);
                }
#if 0
                p->dwFlags = (p->dwFlags & ~TVIF_FOLD) | (~p->dwFlags & TVIF_FOLD);
#endif
                pData->nWidth = getTVWidth (pData);
                pData->nVisItemCount = getVisItemCount (pData);
                tvSetVScrollInfo (hwnd, pData, TRUE);
                tvSetHScrollInfo (hwnd, pData, TRUE);
                GetClientRect (hwnd, &rc);
                rc.top += (count - pData->nItemTop - 1) * pData->nItemHeight;
                if (pData->dwStyle & WS_BORDER)
                    rc.top += TV_BORDER;
                InvalidateRect (hwnd, &rc, FALSE);
                break;
            }
        }
        break;
        
        case MSG_VSCROLL:
        {
            int newTop;
            int scrollHeight = 0;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            newTop = pData->nItemTop;
            switch(wParam)
            {
                case SB_LINEDOWN:
                    if ((pData->nItemTop + pData->nVisCount) < (pData->nVisItemCount)) {
                        newTop ++;
                        scrollHeight = - pData->nItemHeight;
                    }
                break;
                
                case SB_LINEUP:
                    if (pData->nItemTop > 0) {
                        newTop --;
                        scrollHeight = pData->nItemHeight;
                    }
                break;
                
                case SB_PAGEDOWN:
                    if ((pData->nItemTop + (pData->nVisCount << 1)) <= pData->nItemCount)
                        newTop += pData->nVisCount;
                    else
                        newTop = pData->nVisItemCount - pData->nVisCount;
                    scrollHeight = - (newTop - pData->nItemTop) * pData->nItemHeight;
                    if (newTop < 0)
                        return 0;

                break;

                case SB_PAGEUP:
                    if (pData->nItemTop >= pData->nVisCount)
                        newTop -= pData->nVisCount;
                    else
                        newTop = 0;

                    scrollHeight = (pData->nItemTop - newTop) * pData->nItemHeight;
                break;

                case SB_THUMBTRACK:
                    newTop = (int)lParam;
                    scrollHeight = (pData->nItemTop - newTop) * pData->nItemHeight;
                break;
            }
            
            if (scrollHeight) {
                pData->nItemTop = newTop;
                tvSetVScrollInfo (hwnd, pData, TRUE);
                UpdateWindow (hwnd, FALSE); /* SendMessage (hwnd, MSG_PAINT, 0, 0); */
                return 0;
            }
        }
        break;
        
        case MSG_HSCROLL:
        {
            int newLeft;
            int scrollWidth = 0;

            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            newLeft = pData->nLeft;
            switch(wParam)
            {
                case SB_LINERIGHT:
                    if ((pData->nLeft + pData->nVisWidth) < (pData->nWidth)) {
                        newLeft += GetSysCharWidth ();
                        scrollWidth = - GetSysCharWidth ();
                    }
                break;
                
                case SB_LINELEFT:
                    if (pData->nLeft > 0) {
                        newLeft -= GetSysCharWidth ();
                        scrollWidth = GetSysCharWidth ();
                    }
                break;
                
                case SB_PAGERIGHT:
                    if ((pData->nLeft + (pData->nVisWidth << 1)) <= pData->nWidth)
                        newLeft += pData->nVisWidth;
                    else
                        newLeft = pData->nWidth - pData->nVisWidth;
                    scrollWidth = - (newLeft - pData->nLeft) * GetSysCharWidth ();
                    if (newLeft < 0)
                        return 0;
                break;

                case SB_PAGELEFT:
                    if (pData->nLeft >= pData->nVisWidth)
                        newLeft -= pData->nVisWidth;
                    else
                        newLeft = 0;

                    scrollWidth = (pData->nLeft - newLeft) * GetSysCharWidth ();
                break;

                case SB_THUMBTRACK:
                    newLeft = (int)lParam;
                    scrollWidth = (pData->nLeft - newLeft) * GetSysCharWidth ();
                break;
            }
            
            if (scrollWidth) {
                pData->nLeft = newLeft;
                tvSetHScrollInfo (hwnd, pData, TRUE);
                UpdateWindow (hwnd, FALSE); /* SendMessage (hwnd, MSG_PAINT, 0, 0); */
                return 0;
            }
        }
        break;

        case MSG_SETFOCUS:
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            NotifyParent (hwnd, pData->id, TVN_SETFOCUS);
            if (pData->dwStyle & TVS_FOCUS)
                break;
            pData->dwStyle |= TVS_FOCUS;
            InvalidateRect (hwnd, NULL, FALSE);
            break;

        case MSG_KILLFOCUS:
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);

            NotifyParent (hwnd, pData->id, TVN_KILLFOCUS);
            if (!(pData->dwStyle & TVS_FOCUS))
                break;
            pData->dwStyle &= ~TVS_FOCUS;
            InvalidateRect (hwnd, NULL, FALSE);
            break;
        
        case MSG_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTCHARS;

        case MSG_FONTCHANGED:
            pData = (PTVDATA) GetWindowAdditionalData2 (hwnd);
            tvRecalcWidths (hwnd, pData);
            return 0;

        default:
            break;
    }
    
    return DefaultControlProc (hwnd, message, wParam, lParam);
}

#endif /* _EXT_CTRL_TREEVIEW */

