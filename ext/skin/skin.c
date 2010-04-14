/*
** $Id: skin.c,v 1.26 2003/11/23 06:09:39 weiym Exp $
**
** skin.c: skin implementation.
**
** Copyright (C) 2003 Feynman Software.
**
** Create date: 2003-10-10
*/

/*
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
** TODO:
** add Raise_Event interface.
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
	#include "mgext.h"
	#include "mywindows.h"
#else
	#include <minigui/common.h>
	#include <minigui/minigui.h>
	#include <minigui/gdi.h>
	#include <minigui/window.h>
	#include <minigui/control.h>
	#include <minigui/mgext.h>
	#include <mywindows.h>
#endif

#ifdef _EXT_SKIN

#include "skin.h"

#include "button.h"
#include "chkbutton.h"
#include "label.h"
#include "bmplabel.h"
#include "slider.h"
#include "rotslider.h"
#include "mgcontrol.h"

static BOOL item_init_ops (skin_item_t *item)
{
    switch (item->style & SI_TYPE_MASK) {

    case SI_TYPE_CMDBUTTON:
        item->ops = BUTTON_OPS;
        break;
    case SI_TYPE_CHKBUTTON:
        item->ops = CHKBUTTON_OPS;
        break;
    case SI_TYPE_NRMLABEL:
        item->ops = NRMLABEL_OPS;
        break;
    case SI_TYPE_BMPLABEL:
        item->ops = BMPLABEL_OPS;
        break;
    case SI_TYPE_NRMSLIDER:
        item->ops = SLIDER_OPS;
        break;
    case SI_TYPE_ROTSLIDER:
        item->ops = ROTSLIDER_OPS;
        break;
    case SI_TYPE_CONTROL:
        item->ops = CONTROL_OPS;
        break;
    default:
        item->ops = NULL;
        return FALSE;
    }

    return TRUE;
}

BOOL skin_init (skin_head_t* skin, skin_event_cb_t event_cb, skin_msg_cb_t msg_cb)
{
    int i;
	const BITMAP* bmp = NULL;
    skin_item_t* item;
#ifdef _USE_NEWGAL
    int pt_num = 0;
	POINT pt[4];
#endif

    InitFreeClipRectList (&skin->rc_heap, skin->nr_items);
    
    item = skin->items;
    for (i = 0; i < skin->nr_items; i++, item++) {
	
		/* init item's #hostskin# */
		item->hostskin = skin;
		
		/* init item's #ops# */
        if ( !item_init_ops (item) ){
        	fprintf (stderr, "Error when init item %d's ops\n", item->id);
	    	return FALSE;
		}
        if (item->ops->init && (item->ops->init (skin, item)==0) )
			continue;
		
		/* init item's #shape# */
		if ( item->bmp_index >= 0 )
			bmp = &item->hostskin->bmps[item->bmp_index];
			
        item->shape.left = item->x;
        item->shape.top = item->y;

		switch (item->style & SI_TYPE_MASK){
		case SI_TYPE_CMDBUTTON:
		case SI_TYPE_CHKBUTTON:
            item->shape.right  = item->x + bmp->bmWidth / 4;
        	item->shape.bottom = item->y + bmp->bmHeight;
			break;
		case SI_TYPE_NRMLABEL:
		case SI_TYPE_BMPLABEL:
            item->shape.right  = item->shape.left+ 1;
        	item->shape.bottom = item->shape.top + 1;
			//item->x = item->y = -10;	// ??
			break;
		default:
            item->shape.right  = item->x + bmp->bmWidth;
        	item->shape.bottom = item->y + bmp->bmHeight;
		}

		/* init item's #region# */
        InitClipRgn (&item->region, &skin->rc_heap);
        if (IsRectEmpty (&item->rc_hittest)) {
            item->rc_hittest = item->shape;
        }
        else {
            OffsetRect (&item->rc_hittest, item->x, item->y);
        }

		switch (item->style & SI_TEST_SHAPE_MASK){
		case SI_TEST_SHAPE_RECT: 
            SetClipRgn (&item->region, &item->rc_hittest);
			break;
#ifdef _USE_NEWGAL
		case SI_TEST_SHAPE_ELLIPSE: 
            InitEllipseRegion (&item->region, 
					(item->rc_hittest.left + item->rc_hittest.right) / 2,
					(item->rc_hittest.top + item->rc_hittest.bottom) / 2,
					(item->rc_hittest.right - item->rc_hittest.left) / 2,
					(item->rc_hittest.bottom - item->rc_hittest.top) / 2);
			break;
		case SI_TEST_SHAPE_LOZENGE: 
			if ( pt_num == 0 ){
				pt_num = 4;
				pt[0].x = item->rc_hittest.left;
				pt[0].y = (item->rc_hittest.top + item->rc_hittest.bottom)/2;
				pt[1].x = (item->rc_hittest.left+ item->rc_hittest.right )/2;
				pt[1].y = item->rc_hittest.top;
				pt[2].x = item->rc_hittest.right;
				pt[2].y = pt[0].y;
				pt[3].x = pt[1].x;
				pt[3].y = item->rc_hittest.bottom;
			}
		case SI_TEST_SHAPE_LTRIANGLE: 
		case SI_TEST_SHAPE_RTRIANGLE: 
		case SI_TEST_SHAPE_UTRIANGLE: 
		case SI_TEST_SHAPE_DTRIANGLE: 
			if ( pt_num == 0 ){
				pt_num = 3;
				pt[0].x = item->rc_hittest.left;	pt[0].y = item->rc_hittest.bottom;
				pt[1].x = item->rc_hittest.left;	pt[1].y = item->rc_hittest.top;
				pt[2].x = item->rc_hittest.right;	pt[2].y = item->rc_hittest.top;
				pt[3].x = item->rc_hittest.right;	pt[3].y = item->rc_hittest.bottom;
				switch (item->style & SI_TEST_SHAPE_MASK){
					case SI_TEST_SHAPE_LTRIANGLE: 
						pt[1] = pt[2];	pt[2] = pt[3];
						pt[0].y = (pt[1].y + pt[2].y) / 2;
						break;
					case SI_TEST_SHAPE_UTRIANGLE: 
						pt[1] = pt[3];	pt[2] = pt[0];
						pt[0].x = (pt[3].x + pt[0].x) / 2;
						pt[0].y = item->rc_hittest.top;
						break;
					case SI_TEST_SHAPE_RTRIANGLE: 
						pt[1] = pt[1];	pt[2] = pt[0];
						pt[0].y = (pt[1].y + pt[2].y) / 2;
						pt[0].x = item->rc_hittest.right;
						break;
					case SI_TEST_SHAPE_DTRIANGLE: 
						pt[1] = pt[1];	pt[2] = pt[2];
						pt[0].x = (pt[1].x + pt[2].x) / 2;
						break;
				}
			}
			InitPolygonRegion (&item->region, pt, pt_num);
			break;
#endif
		default:
            fprintf (stderr, "undefined test rect shape. item->id is %d\n", item->id);
            return FALSE;
        }
    }

    skin->hilighted = NULL;
	skin->tool_tip = HWND_DESKTOP;
    skin->msg_cb 	= msg_cb;
    skin->event_cb 	= event_cb;

    return TRUE;
}

void skin_deinit (skin_head_t* skin)
{
    int i;
    skin_item_t* item;

    item = skin->items;
    for (i = 0; i < skin->nr_items; i++, item++) {
        EmptyClipRgn (&item->region);

        if (item->ops && item->ops->deinit)
            item->ops->deinit (item);
    }

    DestroyFreeClipRectList (&skin->rc_heap);
}

static void draw_item (HDC hdc, skin_item_t* item)
{
    if (!item || !item->ops)
        return;

    /* draw background */
    if (item->ops->draw_bg)
        item->ops->draw_bg (hdc, item);

    /* draw attached element */
    if (item->ops->draw_attached)
        item->ops->draw_attached (hdc, item);
}

/********************************************************************************/
/************   item status functions        *****************/
#define refresh_item(item)	InvalidateRect(item->hostskin->hwnd, &item->rc_hittest, TRUE)
#define item_visible(item)	(item->style & SI_STATUS_VISIBLE)
#define item_clicked(item)	(item->style & SI_BTNSTATUS_CLICKED)
#define item_hilighted(item)(item->style & SI_STATUS_HILIGHTED)
#define item_disabled(item)	(item->style & SI_STATUS_DISABLED)
#define item_enable(item)	!item_disabled(item)

static void set_item_status (skin_item_t* item, DWORD status, BOOL set_status)
{
	if ( set_status)
		item->style |= status;
	else
		item->style &= ~status;
	refresh_item (item);
}

static DWORD get_item_status (skin_item_t* item)
{
    return item ? item->style & SI_STATUS_MASK : 0;
}

#define set_item_visible(item,visible)	   set_item_status (item,SI_STATUS_VISIBLE, visible)
#define set_item_clicked(item,clicked)	   set_item_status (item,SI_BTNSTATUS_CLICKED,clicked)
#define set_item_hilighted(item,hilighted) set_item_status (item, SI_STATUS_HILIGHTED, hilighted)
#define set_item_disabled(item,disabled)   set_item_status (item, SI_STATUS_DISABLED, disabled)
/********************************************************************************/

static void show_item_hint (HWND hwnd, skin_head_t* skin, skin_item_t* item, int xo, int yo)
{
	int x = xo, y = yo;
	if ( !(skin->style & SKIN_STYLE_TOOLTIP) )
		return;
	if (item->tip == NULL || item->tip [0] == '\0')
		return;

	ClientToScreen (hwnd, &x, &y);

	if (skin->tool_tip == HWND_DESKTOP) {
		skin->tool_tip = createToolTipWin (hwnd, x, y, 1000, item->tip);
	}
	else {
		resetToolTipWin (skin->tool_tip, x, y, item->tip);
	}
}

static void on_paint (HWND hwnd, skin_head_t* skin)
{
    int i;
    skin_item_t* item;

    HDC hdc = BeginPaint (hwnd);

	/* draw skin window background */
    //FillBoxWithBitmap (hdc, 0, 0, 0, 0, &skin->bmps[skin->bk_bmp_index]);
	
	/* draw items */
	item = skin->items;
    for (i = 0; i < skin->nr_items; i++, item++) {
        //if (RectVisible (hdc, &item->rc_hittest))
        if ( item_visible(item) )
            draw_item (hdc, item);
    }

    EndPaint (hwnd, hdc);
}

static skin_item_t* find_item (skin_head_t* skin, int x, int y)
{
    int i;
    skin_item_t* item;
	
 	item = skin->items;
    for ( i = 0; i < skin->nr_items; i++, item++) {
		if ( (item->style & SI_TYPE_MASK) == SI_TYPE_CONTROL )	continue;
        if ( PtInRegion (&item->region, x, y) && item_visible(item) )
            return item;
    }

    return NULL;
}

#define ITEM_MSG_PROC(item, msg, wparam, lparam) \
(item->ops->item_msg_proc && item->ops->item_msg_proc(item, msg, wparam, lparam))

static void on_lbuttondown (HWND hwnd, skin_head_t* skin, int x, int y)
{
    skin_item_t* item = find_item (skin, x, y);

	/* if an enabled item is mousedowned, call it's msg proc : LBUTTONDOWN */
	if ( item && item_enable(item) ){
		if ( ITEM_MSG_PROC (item, SKIN_MSG_LBUTTONDOWN, x, y) ){
			set_item_hilighted (item, FALSE);
    		if ( GetCapture() != hwnd )
				SetCapture (hwnd);
		}
	}
}

static void on_lbuttonup (HWND hwnd, skin_head_t* skin, int x, int y)
{
    skin_item_t* item = find_item (skin, x, y);

    if ( GetCapture() == hwnd ) {
		if (skin->hilighted){	/*skin->hilighted must be NON-NULL*/
			/* call item msg proc : LBUTTONUP */
			ITEM_MSG_PROC (skin->hilighted, SKIN_MSG_LBUTTONUP, x, y);
		}
        if ( item && skin->hilighted && (item->id == skin->hilighted->id) ){
			set_item_hilighted (item, TRUE);
			/* call item msg proc : CLICK */
			ITEM_MSG_PROC (item, SKIN_MSG_CLICK, x, y);
		}
		else{
			/* send hilighted item KILLFOCUS msg*/
			ITEM_MSG_PROC (skin->hilighted, SKIN_MSG_KILLFOCUS, 0, 0);
			skin->hilighted = NULL;
		}
        ReleaseCapture ();
    }
}

static void on_mousemove (HWND hwnd, skin_head_t* skin, int x, int y)
{
    skin_item_t* item = find_item (skin, x, y);

	/* normal move */
    if ( GetCapture() != hwnd ) {
		/* if item == hilighted != NULL, call it's msg proc : MOUSEMOVE */
		if ( item && item_enable(item) && skin->hilighted && (skin->hilighted->id == item->id) ){
			ITEM_MSG_PROC ( skin->hilighted, SKIN_MSG_MOUSEMOVE, x, y );
			return;
		}

		/* first, should we KILLFOCUS form hilighted item? */
		/* if hilight exist, and mouse moves out of it, kill focus from it */
		if ( skin->hilighted && !(item && (item->id == skin->hilighted->id)) ){
			ITEM_MSG_PROC (skin->hilighted, SKIN_MSG_KILLFOCUS, 0, 0);
			
			set_item_hilighted (skin->hilighted, FALSE);
			skin->hilighted = NULL;
		}

		/* second, should we SETFOCUS on current item? (item isn't hilighted) */
		if ( item && item_enable (item) ){
			/* hilighted it */
			skin->hilighted = item;
			set_item_hilighted (item, TRUE);
			ITEM_MSG_PROC (item, SKIN_MSG_SETFOCUS, 0, 0);

			/* show tool tip here */
			show_item_hint (hwnd, skin, item, x, y);
		}
	}
	else{	/* item is being DRAGED. skin->hilighted must be non-null */
		ITEM_MSG_PROC (skin->hilighted, SKIN_MSG_MOUSEDRAG, x, y);
	}
}

#define ITEM_OPS(skin,op)	\
{	\
    int i;	\
    skin_item_t* item;	\
	item = skin->items;	\
    for (i = 0; i < skin->nr_items; i++, item++) {	\
        if ( item->ops && item->ops->op )	\
            item->ops->op (item);	\
    }	\
}	

static int SkinWndProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    int x, y, result = MSG_CB_GOON;
    skin_head_t *skin = NULL;

    if (message == MSG_NCCREATE)
        SetWindowAdditionalData2 (hwnd, GetWindowAdditionalData(hwnd));

    skin = (skin_head_t*) GetWindowAdditionalData2 (hwnd);
	
	/* msg_cb */
	if ( skin && skin->msg_cb ){
		skin->msg_cb (hwnd, message, wParam, lParam, &result);
	}
	switch (result){
	case MSG_CB_GOON: /* continue */ 
		switch (message) {
		case MSG_CREATE:
			skin->hwnd = hwnd;
			ITEM_OPS(skin,on_create);
			break;
			
		case MSG_DESTROY:
			ITEM_OPS(skin,on_destroy);
			break;

		case MSG_ERASEBKGND:
			{
			/* draw skin window background */
			HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*) lParam;
			BOOL fGetDC = FALSE;
			RECT rcTemp;

			if (hdc == 0) {
				hdc = GetClientDC (hwnd);
				fGetDC = TRUE;
			}

			if (clip) {
				rcTemp = *clip;
				ScreenToClient (hwnd, &rcTemp.left, &rcTemp.top);
				ScreenToClient (hwnd, &rcTemp.right, &rcTemp.bottom);
				IncludeClipRect (hdc, &rcTemp);
			}

    		FillBoxWithBitmap (hdc, 0, 0, 0, 0, &skin->bmps[skin->bk_bmp_index]);

			if (fGetDC)
				ReleaseDC (hdc);

			return 0;
			}

		case MSG_PAINT:
			on_paint (hwnd, skin);
			break;

		case MSG_MOUSEMOVE:
			x = LOSWORD (lParam); 
			y = HISWORD (lParam);
			on_mousemove (hwnd, skin, x, y);
			break;

		case MSG_LBUTTONDOWN:
			x = LOSWORD (lParam); 
			y = HISWORD (lParam);
			on_lbuttondown (hwnd, skin, x, y);
			break;

		case MSG_LBUTTONUP:
			x = LOSWORD (lParam);
			y = HISWORD (lParam);
			ScreenToClient (hwnd, &x, &y);
			on_lbuttonup (hwnd, skin, x, y);
			break;

		case MSG_CLOSE:
			DestroyAllControls (hwnd);
			DestroyMainWindow (hwnd);
			if (GetHosting(hwnd) == HWND_DESKTOP)
				PostQuitMessage (hwnd);
			else
				MainWindowCleanup (hwnd);
			return 0;
		}
	case MSG_CB_DEF_GOON:	/* continue default only */ 
		if (IsControl(hwnd))
		return DefaultControlProc (hwnd, message, wParam, lParam);
		return DefaultMainWinProc (hwnd, message, wParam, lParam);
		break;

	case MSG_CB_STOP: /* stop proc */
	default:	/* error if here */
		return 0;
	}
}

HWND create_skin_control (skin_head_t* skin, HWND parent, int id, int x, int y, int w, int h)
{
    skin->hwnd =  CreateWindow ( CTRL_SKIN, NULL,
								WS_VISIBLE | WS_CHILD,
								id,
								x, y, w, h,
								parent,
								(DWORD)skin);
    return skin->hwnd;
}

HWND create_skin_main_window (skin_head_t* skin, HWND hosting, int x, int y, int w, int h, BOOL modal)
{
	MSG Msg;
	MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle 		= WS_VISIBLE;
	CreateInfo.dwExStyle 	= 0;
	CreateInfo.spCaption 	= "Main Window with Skin";
	CreateInfo.hMenu 		= 0;
	CreateInfo.hCursor 		= GetSystemCursor (IDC_ARROW);
	CreateInfo.hIcon 		= 0;
	CreateInfo.lx 			= x;
	CreateInfo.ty 			= y;
	CreateInfo.rx 			= w;
	CreateInfo.by 			= h;
	CreateInfo.hHosting 	= hosting;
	CreateInfo.iBkColor 	= PIXEL_lightwhite;
    CreateInfo.dwAddData 	= (DWORD)skin;
    CreateInfo.MainWindowProc = SkinWndProc;

    skin->hwnd = CreateMainWindow (&CreateInfo);
    if (skin->hwnd == HWND_INVALID)
        return HWND_INVALID;
				
	ShowWindow (skin->hwnd, SW_SHOWNORMAL);

	/* modal window */
	if ( modal ) {
	    if (hosting)
	        EnableWindow (hosting, FALSE);
				
		while( GetMessage (&Msg, skin->hwnd) ) {
			TranslateMessage (&Msg);
			DispatchMessage (&Msg);
		}
		DestroyMainWindow (skin->hwnd);
		ThrowAwayMessages (skin->hwnd);
		MainWindowThreadCleanup (skin->hwnd);

	    if (hosting) {
	        EnableWindow (hosting, TRUE);
	        ShowWindow (hosting, SW_SHOWNORMAL);
	        SetActiveWindow (hosting);
	    }
										
	}

   	return skin->hwnd;
}

void destroy_skin_window (HWND hwnd)
{
    if (IsControl(hwnd))
        DestroyWindow (hwnd);
    else
        SendNotifyMessage (hwnd, MSG_CLOSE, 0, 0);
}

skin_head_t* set_window_skin (HWND hwnd, skin_head_t *new_skin)
{
    skin_head_t *old_skin = (skin_head_t*) GetWindowAdditionalData2 (hwnd);

    if (!new_skin || !old_skin || old_skin == new_skin)
        return NULL;

    new_skin->hwnd = hwnd;
    old_skin->hwnd = 0;
    SetWindowAdditionalData2 (hwnd, (DWORD)new_skin);
    InvalidateRect (hwnd, NULL, FALSE);

    return old_skin;
}

skin_head_t* get_window_skin (HWND hwnd)
{
    return (skin_head_t*) GetWindowAdditionalData2 (hwnd);
}

skin_item_t* skin_get_item (skin_head_t* skin, int id)
{
    int i;
    skin_item_t* item = skin->items;

    for ( i = 0 ; i < skin->nr_items ; i++, item++ ) {
        if ( item->id == id )
            return item;
    }
    return NULL;
}

/* ------------------------- operation interfaces -------------------------- */

#define CHECK_OPS(func, error) \
    skin_item_t *item = skin_get_item (skin, id); \
    if (!item || !item->ops || !item->ops->func)   \
        return error;
    
const char* skin_get_item_label (skin_head_t* skin, int id)
{
    CHECK_OPS(get_value, NULL);

    return (const char*)( item->ops->get_value (item) );
}

BOOL skin_set_item_label (skin_head_t* skin, int id, const char* label)
{
	BOOL ret;	
    CHECK_OPS(set_value, FALSE);

    ret = (BOOL)( item->ops->set_value (item, (DWORD)label) );
	//refresh_item (item);

    return ret;
}

DWORD skin_get_item_status ( skin_head_t* skin, int id )
{
    return get_item_status ( skin_get_item (skin, id) );
}

DWORD skin_set_item_status ( skin_head_t* skin, int id, DWORD status )
{
    skin_item_t *item = skin_get_item (skin, id);

	DWORD old_status =  get_item_status (item); 
	if ( old_status ){
		item->style |= status;
		refresh_item (item);
	}
	return old_status; 
}

DWORD skin_show_item (skin_head_t* skin, int id, BOOL show)
{
    skin_item_t *item = skin_get_item (skin, id);
	
	set_item_visible ( item, show );

    return get_item_status ( item );
}

DWORD skin_enable_item (skin_head_t* skin, int id, BOOL enable)
{
    skin_item_t *item = skin_get_item (skin, id);
	
	set_item_disabled( item, !enable );

    return get_item_status ( item );
}

skin_item_t* skin_get_hilited_item (skin_head_t* skin)
{
	return skin->hilighted;
}

skin_item_t* skin_set_hilited_item (skin_head_t* skin, int id)
{
	skin_item_t *old_hilighted = skin->hilighted;
	skin->hilighted = skin_get_item (skin, id);
	return old_hilighted;
}

BOOL skin_get_check_status (skin_head_t* skin, int id)
{
	return item_clicked ( skin_get_item (skin, id) );
}

DWORD skin_set_check_status (skin_head_t* skin, int id, BOOL check)
{
	set_item_clicked ( skin_get_item (skin, id), check );
	return skin_get_item_status (skin, id);
}

int skin_get_thumb_pos (skin_head_t* skin, int id)
{
    CHECK_OPS(get_value, -1);

    return (int) (item->ops->get_value (item) ) ;
}

BOOL skin_set_thumb_pos (skin_head_t* skin, int id, int pos)
{
    BOOL ret;
    CHECK_OPS(set_value, FALSE);

    ret = (BOOL) (item->ops->set_value (item, pos) );
	
	refresh_item (item);
    return ret; 
}

BOOL skin_get_slider_info (skin_head_t* skin, int id, sie_slider_t* sie)
{
	skin_item_t *item = skin_get_item (skin, id);
	si_nrmslider_t* slider = (si_nrmslider_t*) item->type_data;

	memcpy ( sie, &slider->slider_info, sizeof(sie_slider_t));
	return TRUE;
}

BOOL skin_set_slider_info (skin_head_t* skin, int id, const sie_slider_t* sie)
{
	skin_item_t *item = skin_get_item (skin, id);
	si_nrmslider_t* slider = (si_nrmslider_t*) item->type_data;

	memcpy ( &slider->slider_info, sie, sizeof(sie_slider_t));
	return TRUE;
}

int skin_scale_slider_pos (const sie_slider_t* org, int new_min, int new_max)
{
	return (.5 + new_min + 1. * (new_max - new_min) * 
		(org->cur_pos - org->min_pos) / (org->max_pos - org->min_pos) );
}

HWND skin_get_control_hwnd (skin_head_t* skin, int id)
{
	skin_item_t *item = skin_get_item (skin, id);

	if ( (item->style & SI_TYPE_MASK) == SI_TYPE_CONTROL )
		return item->ops->get_value(item);

	return HWND_INVALID;
}

BOOL RegisterSkinControl (void)
{
    WNDCLASS SkinClass;

    SkinClass.spClassName    = CTRL_SKIN;
    SkinClass.dwStyle        = 0;
    SkinClass.dwExStyle      = 0;
    SkinClass.hCursor        = GetSystemCursor (IDC_ARROW);
    SkinClass.iBkColor       = COLOR_lightwhite;
    SkinClass.WinProc        = SkinWndProc;
    SkinClass.dwAddData      = 0;

    return RegisterWindowClass (&SkinClass);
}

void SkinControlCleanup (void)
{
    UnregisterWindowClass (CTRL_SKIN);
}

#endif /* _EXT_SKIN */
