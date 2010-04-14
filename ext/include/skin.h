/**
 * \file skin.h
 * \author Feynman Software (http://www.minigui.com).
 * \date 2003/10/10
 * 
 *  This file includes interfaces for skin module of MiniGUIExt library, 
 *  which provides skin support for MiniGUI applications.
 *
 \verbatim
    Copyright (C) 2003 Feynman Software.

    This file is part of MiniGUI, a lightweight Graphics User Interface
    support library for real-time embedded Linux.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 \endverbatim
 */

/*
 * $Id: skin.h,v 1.33 2003/11/23 06:06:14 weiym Exp $
 *
 *             MiniGUI for Linux Version 1.3.x
 *             Copyright (C) 2003 Feynman Software.
 */


#ifndef _MGUI_SKIN_H
#define _MGUI_SKIN_H

#ifdef _EXT_SKIN

    /**
     * \addtogroup mgext_fns Interfaces of the MiniGUI extension library (libmgext)
     * @{
     */

    /**
     * \defgroup mgext_skin_fns Skin related routines
     *
     * @{
     */

/* skin messages (send to items) */
#define SKIN_MSG_LBUTTONDOWN    1
#define SKIN_MSG_LBUTTONUP      2
#define SKIN_MSG_CLICK          3
//#define SKIN_MSG_DBCLICK      4
#define SKIN_MSG_MOUSEMOVE      5
#define SKIN_MSG_MOUSEDRAG      6
#define SKIN_MSG_SETFOCUS       7
#define SKIN_MSG_KILLFOCUS      8

/* type of item */
#define SI_TYPE_NRMLABEL        0x10000000
#define SI_TYPE_BMPLABEL        0x20000000
#define SI_TYPE_CMDBUTTON       0x30000000
#define SI_TYPE_CHKBUTTON       0x40000000
#define SI_TYPE_NRMSLIDER       0x50000000
#define SI_TYPE_ROTSLIDER       0x60000000
#define SI_TYPE_CONTROL         0x70000000

#define SI_TYPE_MASK            0xF0000000

/* type of hittest shape */
#define SI_TEST_SHAPE_RECT      0x01000000

#ifdef _USE_NEWGAL

#define SI_TEST_SHAPE_ELLIPSE   0x02000000
#define SI_TEST_SHAPE_LOZENGE   0x03000000
#define SI_TEST_SHAPE_LTRIANGLE 0x04000000
#define SI_TEST_SHAPE_RTRIANGLE 0x05000000
#define SI_TEST_SHAPE_UTRIANGLE 0x06000000
#define SI_TEST_SHAPE_DTRIANGLE 0x07000000

#endif

#define SI_TEST_SHAPE_MASK      0x0F000000

/* common status of item */
#define SI_STATUS_OK            0x00000001
#define SI_STATUS_VISIBLE       0x00000002
#define SI_STATUS_DISABLED      0x00000004
#define SI_STATUS_HILIGHTED     0x00000008

#define SI_STATUS_MASK          0x0000000F

/* status of button item */
#define SI_BTNSTATUS_CLICKED    0x00000010
#define SI_BTNSTATUS_CHECKED    SI_BTNSTATUS_CLICKED
#define SI_STATUS_CLICKED       SI_BTNSTATUS_CLICKED

#define SI_BTNSTATUS_MASK       0x000000F0

/* style of normal slider item */
#define SI_NRMSLIDER_HORZ       0x00000100
#define SI_NRMSLIDER_VERT       0x00000200
#define SI_NRMSLIDER_STATIC     0x00000400

/* style of rotation slider item */
#define SI_ROTSLIDER_CW         0x00000100
#define SI_ROTSLIDER_ANTICW     0x00000200
#define SI_ROTSLIDER_STATIC     0x00000400

#define SI_STYLE_MASK           0x0000FF00

/* skin contrl class name */
#define CTRL_SKIN               "skin"

typedef struct skin_item_ops_s skin_item_ops_t;
typedef struct skin_head_s skin_head_t;

/** Skin item information structure */
typedef struct skin_item_s
{
    /** The identifier of the item. */
    int id;

    /**
     * The style of the item.
     *
     * The style of an item is composed of five classes of information:
     *
     *  - The type of an item.
     *  - The hit-test shape type.
     *  - The common status.
     *  - The item specific status.
     *  - The item specific style.
     *
     * The type of an item can be one of the following values:
     *
     *  - SI_TYPE_NRMLABEL\n
     *    The item is a normal label.
     *  - SI_TYPE_BMPLABEL\n
     *    The item is a bitmap label.
     *  - SI_TYPE_CMDBUTTON\n
     *    The item is a command button.
     *  - SI_TYPE_CHKBUTTON\n
     *    The item is a check button.
     *  - SI_TYPE_NRMSLIDER\n
     *    The item is a normal slider.
     *  - SI_TYPE_ROTSLIDER\n
     *    The item is a rotation slider.
     *
     * The hit-test shape type can be one of the following values:
     *
     *  - SI_TEST_SHAPE_RECT\n
     *    The hit-test shape is a rect.
     *  - SI_TEST_SHAPE_ELLIPSE\n
     *    The hit-test shape is an ellipse.
     *  - SI_TEST_SHAPE_LOZENGE\n
     *    The hit-test shape is a lozenge.
     *  - SI_TEST_SHAPE_LTRIANGLE\n
     *    The hit-test shape is a left triangle.
     *  - SI_TEST_SHAPE_RTRIANGLE\n
     *    The hit-test shape is a right triangle.
     *  - SI_TEST_SHAPE_UTRIANGLE\n
     *    The hit-test shape is a up triangle.
     *  - SI_TEST_SHAPE_DTRIANGLE\n
     *    The hit-test shape is a down triangle.
     *
     * The common status of an item can be OR'ed with the following values:
     *
     *  - SI_STATUS_OK\n
     *    The basic status of an item, always be there.
     *  - SI_STATUS_VISIBLE\n
     *    The item is visble.
     *  - SI_STATUS_DISABLED\n
     *    The item is disabled.
     *  - SI_STATUS_HILIGHTED\n
     *    The item is hilighted.
     *
     *  The item specific status can be one of the following values:
     *
     *  - SI_BTNSTATUS_CLICKED\n
     *    The three-state button is clicked.
     *  - SI_BTNSTATUS_CHECKED\n
     *    The check button is ckecked.
     *
     *  The item specific style can be a valued OR'ed with the following values:
     *
     *  - SI_NRMSLIDER_HORZ\n
     *    The normal slider is horizontal.
     *  - SI_NRMSLIDER_VERT\n
     *    The normal slider is vertical.
     *  - SI_NRMSLIDER_STATIC\n
     *    The normal slider is static, i.e., the slider acts like a progress bar.
     *  - SI_ROTSLIDER_CW\n
     *    The rotation slider is clockwise.
     *  - SI_ROTSLIDER_ANTICW\n
     *    The rotation slider is anti-clockwise.
     *  - SI_ROTSLIDER_STATIC\n
     *    The rotation slider is static, i.e., the slider acts like a progress bar.
     */
    DWORD style;

    /** X-coordinate of the item in the skin. */
    int x;
    /** Y-coordinate of the item in the skin. */
    int y;

    /** The hit-test rectangle of the item. */
    RECT rc_hittest;

    /** The index of the item bitmap in the skin bitmap array. */
    int bmp_index;

    /** The tip text. */
    char* tip;

    /** The attached private data with the item by application. */
    DWORD attached;

    /** 
     * The type-specific data of the item, 
     * used to define extra information of the item.
     */
    void* type_data;

    /******** start of internal fields ********/
    /* The bounding rectangle of the item. */
    RECT shape;
    /* The region for mouse hittest. */
    CLIPRGN region;
    /* The operation routines of the item. */
    skin_item_ops_t *ops;
    /* The parent skin. */
    skin_head_t *hostskin;
} skin_item_t;

/* skin item operations */
struct skin_item_ops_s
{
    /* operations for initialization and deinitialization. */
    int (*init) (skin_head_t*, skin_item_t*);
    int (*deinit) (skin_item_t*);

    /* operations when creating and destroying the skin window. */
    int (*on_create) (skin_item_t*);
    int (*on_destroy) (skin_item_t*);

    /* operations for item drawing. */
    void (*draw_bg) (HDC, skin_item_t*);
    void (*draw_attached) (HDC, skin_item_t*);

    /* 
     * operations for getting or setting item value.
     * for button item, value is the click status, non-zero for clicked or checked.
     * for slider, value is the position.
     * for label item, value is the label text.
     * for control item, value is the handle to control.
     */
    DWORD (*get_value) (skin_item_t *item);
    DWORD (*set_value) (skin_item_t *item, DWORD status);
    
    /* item msg proc */
    int (*item_msg_proc) (skin_item_t* item, int message, WPARAM wparam, LPARAM lparam);
};

/** Slider information structure */
typedef struct sie_slider_s
{
    /** The minimal position of the thumb. */
    int min_pos;
    /* The maximal position of the thumb. */
    int max_pos;
    /* The current position of the thumb. */
    int cur_pos;
} sie_slider_t;

/** Normal label item info structure */
typedef struct si_nrmlabel_s
{
    /** The label string. */
    char* label;        

    /** The normal text color. */
    DWORD color;        
    /** The focus text color. */
    DWORD color_focus;  
    /** The click text color. */
    DWORD color_click;  
    /** The index of the logical font in the skin fonts array. */
    int font_index;
} si_nrmlabel_t;

/** Bitmap label item info structure */
typedef struct si_bmplabel_s
{
    /** The label string. */
    char* label;
    /** All label characters in the bitmap. */
    const char* label_chars;
} si_bmplabel_t;

/** Normal slider item info structure */
typedef struct si_nrmslider_s
{
    /** The slider information */
    sie_slider_t    slider_info;

    /** The index of the thumb bitmap in the skin bitmaps array. */
    int thumb_bmp_index;
} si_nrmslider_t;

/** Rotation slider item info structure */
typedef struct si_rotslider_s
{
    /** The rotation radius. */
    int radius;
    /** The start degree. */
    int start_deg;
    /** The end degree. */
    int end_deg;
    /** The current degree. */
    int cur_pos;

    /** The index of the thumb bitmap in the skin bitmaps array. */
    int thumb_bmp_index;
} si_rotslider_t;

#define SIE_BUTTON_CLICKED      0x00000001
#define SIE_SLIDER_CHANGED      0x00000001

#define SIE_GAIN_FOCUS          0x00010000
#define SIE_LOST_FOCUS          0x00020000

/**
 * \var int (* skin_event_cb_t) (HWND hwnd, skin_item_t* item, int event, void* data)
 *
 * Event callback of skin item.
 *
 * The event can be one of the following values:
 *
 *  - SIE_GAIN_FOCUS\n
 *    The item gained the focus.
 *  - SIE_LOST_FOCUS\n
 *    The item losted the focus.
 *  - SIE_BUTTON_CLICKED\n
 *    The button item has been clicked.
 *  - SIE_SLIDER_CHNAGED\n
 *    The position of the slider item has changed.
 */
typedef int (* skin_event_cb_t) (HWND hwnd, skin_item_t* item, int event, void* data);

#define MSG_CB_GOON     0
#define MSG_CB_DEF_GOON 1
#define MSG_CB_STOP     2

/**
 * \var int (* skin_msg_cb_t) (HWND hwnd, int message, WPARAM wparam, LPARAM lparam, int* result)
 *
 * This is the type of message callback function of a skin window. 
 *
 * Before a skin window processes a message, it will call the message 
 * callback function if the application defined it.
 *
 * The application defined message callback function can process the message 
 * as a normal window procedure and return the result through the parameter \a result. 
 * However, the return value of a message callback function can be used
 * to control the behavior of the skin window's message handling procedure. 
 * It can be one of the following values:
 *
 *  - MSG_CB_GOON
 *    The message should be processed by skin window procedure, and
 *    the result of the callback will be ignored.
 *  - MSG_CB_DEF_GOON
 *    The message should be processed by the default window procedure, and
 *    the result of the callback will be ignored.
 *  - MSG_CB_STOP
 *    The message should not be processed, and the result is valid.
 */
typedef int (* skin_msg_cb_t) (HWND hwnd, int message, WPARAM wparam, LPARAM lparam, int* result);

#define SKIN_STYLE_NONE     0x00000000
#define SKIN_STYLE_TOOLTIP  0x00000001

/** Skin header information structure */
struct skin_head_s
{
    /** The name of the skin. */
    char* name;

    /**
     * The style of the skin, can be OR'ed by the following values:
     *  - SKIN_STYLE_TOOLTIP
     *    The skin has tooltip window.
     */
    DWORD style;

    /** The pointer to the array of the bitmaps used by the skin. */
    const BITMAP* bmps;
    /** The pointer to the array of the logical fonts used by the skin. */
    const LOGFONT* fonts;

    /** The index of the background bitmap in the bitmap array. */
    int bk_bmp_index;

    /** The number of the items in this skin. */
    int nr_items;
    /** The pointer to the array of skin items. */
    skin_item_t* items;

    /** The attached private data with the skin by application. */
    DWORD attached;

    /******** start of internal fields ********/

    /* The event callback function of the skin. */
    skin_event_cb_t event_cb;

    /* The window message procedure of the skin window. */
    skin_msg_cb_t msg_cb;

    /**** The fields initialized when skin_init called. ****/
    /* The rectangle heap for hit-test regions. */
    BLOCKHEAP rc_heap;

    /**** The fields initialized when create_skin_main_window or create_skin_control called. ****/
    /* The handle of window in which the skin locates. */
    HWND hwnd;
    /* The handle of tool tip window. */
    HWND tool_tip;

    /* The old x and y. */
    int oldx, oldy;
    /* The current hilighted item. */
    skin_item_t* hilighted;

    /* cached identifier. */
    int cached_id;
    /* the item whose identifier is cached_id. */
    skin_item_t* cached_item;
};

/** Skin header information structure */
typedef struct skin_set_s
{
    /** The version of the skin, currently is 3. */
    int version;
    /** The author of the skin. */
    char* author;

    /** The pointer to the array of the bitmaps used by the skin set. */
    BITMAP* bmps;
    /** The pointer to the array of the logical fonts used by the skin set. */
    LOGFONT* fonts;

    /** The number of the skins in the set. */
    int nr_skins;
    /** The pointer to the skin objects array. */
    skin_head_t* skins;
} skin_set_t;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * \fn skin_set_t* skin_set_load (MG_RWops* source)
 * \brief Loads a set of skins from a MG_RWops object which defines the skin set.
 *
 * This function loads and returns a skin objects set from the MG_RWops 
 * object \a source which defines the skin.
 *
 * This function will load the bitmap objects and create the logical
 * fonts for the skin set, then create the skin_set_t object and
 * its skin and skin items.
 *
 * The source should have the following layout:
 *
 * \code
 *
 * {
 *  global {
 *      version: 3
 *      author: <the name of the author>
 *  }
 *  bitmaps {
 *      <id of bitmap 1> {
 *          name: <file name of the bitmap>
 *      }
 *      <id of bitmap 2> {
 *          name: <file name of the bitmap>
 *      }
 *      ...
 *  }
 *  fonts {
 *      <id of font 1> {
 *          name: <name of the font>
 *      }
 *      <id of font 2> {
 *          name: <name of the font>
 *      }
 *      ...
 *  }
 *  skins {
 *      <name of skin1> {
 *          items {
 *              <id of item1> {
 *                  style: 
 *                  x:
 *                  y:
 *                  test_rect:
 *                  bmp_id:
 *                  tip:
 *                  type_data {
 *                      ...
 *                  }
 *              }
 *              <id of item2> {
 *                  style: 
 *                  x:
 *                  y:
 *                  test_rect:
 *                  bmp_id:
 *                  tip:
 *                  type_data {
 *                      ...
 *                  }
 *              }
 *              ...
 *          }
 *      }
 *      <name of skin2> {
 *          items {
 *              ...
 *          }
 *      }
 *      <name of skin3> {
 *          items {
 *              ...
 *          }
 *      }
 *  }
 * }
 *
 * \endcode
 *
 * 
 * \return This function returns the pointer to the skin set
 *         when success, otherwise NULL.
 *
 * \sa general_rw_fns, skin_set_unload, skin_init
 */
skin_set_t* skin_set_load (MG_RWops* skin_info);

/**
 * \fn void skin_set_unload (skin_set_t* skin_set)
 * \brief Unloads a skin set.
 *
 * This function unloads the skin set \a skin_set. It destroies
 * the bitmap objects, the logical fonts, and the skin objects.
 *
 * \sa skin_set_load, skin_destroy
 */
void skin_set_unload (skin_set_t* skin_set);

/**
 * \fn skin_head_t* skin_set_find_skin (skin_set_t* skin_set, const char* skin_name)
 * \brief Finds a skin object in a skin set.
 *
 * This function finds a skin object which matches the name \a skin_name in a skin set.
 *
 * \sa skin_set_load
 */
skin_head_t* skin_set_find_skin (skin_set_t* skin_set, const char* skin_name);

/**
 * \fn BOOL skin_init (skin_head_t* skin, skin_event_cb_t event_cb, skin_msg_cb_t msg_cb)
 * \brief Initializes a skin.
 *
 * This function initializes a skin \a skin before it can be used.
 * The initialization of a skin includes skin items initialization, etc, 
 * such as creating the hit-test region, allocating space for label, and so on.
 *
 * \param skin The pointer to the skin object. The skin can be
 *        a object returned by \a skin_set_find_skin or a hard-coded
 *        skin_head_t structure.
 * \param event_cb The item event callback of the skin.
 * \param msg_cb The window message callback of the skin window.
 *
 * \return TRUE for success, otherwise FALSE.
 *
 * \sa skin_set_load, skin_destroy, skin_set_find_skin
 */
BOOL skin_init (skin_head_t* skin, skin_event_cb_t event_cb, skin_msg_cb_t msg_cb);

/**
 * \fn void skin_deinit (skin_head_t* skin)
 * \brief Deinitializes an initialized skin.
 *
 * This function deinitializes a skin, opposed to skin_init.
 *
 * \param skin The pointer to the skin object.
 *
 * \sa skin_set_load, skin_init
 */
void skin_deinit (skin_head_t* skin);

/**
 * \fn HWND create_skin_main_window (skin_head_t* skin, HWND hosting, int x, int y, int w, int h, BOOL modal)
 * \brief Creates a main window for a skin.
 *
 * This function creates a main window for the skin pointed to by \a skin.
 * The main window will have no caption, no menu, and no frame. However,
 * the window's caption string will be the name of the skin object.
 *
 * \param skin The pointer to the initialized skin object.
 * \param hosting The the hosting main window.
 * \param x X-coordinate of the expected main window.
 * \param y Y-coordinate of the expected main window.
 * \param w The width of the expected main window.
 * \param h The height of the expected main window.
 * \param modal Whether to be a modal or modeless main window.
 *
 * \return The handle to the main window.
 *
 * \note The skin will store itself as the main window's additional data,
 *       so application should not call SetWindowAdditionalData to store
 *       other value.
 *
 * \sa skin_set_load, create_skin_control
 */
HWND create_skin_main_window (skin_head_t* skin, HWND hosting, int x, int y, int w, int h, BOOL modal);

/**
 * \fn HWND create_skin_control (skin_head_t* skin, HWND parent, int id, int x, int y, int w, int h)
 * \brief Creates a control for a skin.
 *
 * This function creates a control for the skin pointed to by \a skin.
 *
 * \param skin The pointer to the initialized skin object.
 * \param parent The handle to the parent window of the expected control.
 * \param id The identifier of the expected control.
 * \param x X-coordinate of the expected control in the parent window's client area.
 * \param y Y-coordinate of the expected control in the parent window's client area.
 * \param w The width of the expected control.
 * \param h The height of the expected control.
 *
 * \return The handle to the control.
 *
 * \note The skin will store itself as the control's additional data,
 *       so application should not call SetWindowAdditionalData to store other value.
 *
 *       You can also create a skin control by calling CreateWindowEx in the following manner:
 * \code
 *     CreateWindowEx (CTRL_SKIN, "", WS_VISIBLE | WS_CHILD, WS_EX_NONE, id, x, y, w, h, parent, (DWORD) skin);
 * \endcode
 *
 * \sa skin_set_load, create_skin_main_window
 */
HWND create_skin_control (skin_head_t* skin, HWND parent, int id, int x, int y, int w, int h);

/**
 * \fn void destroy_skin_window (HWND hwnd)
 * \brief Destroies a skin window.
 *
 * The function destroies the skin main window or control.
 *  
 * \param hwnd The handle to the skin window.
 *
 * \sa create_skin_main_window, create_skin_control
 */
void destroy_skin_window (HWND hwnd);

/**
 * \fn skin_head_t* set_window_skin (HWND hwnd, skin_head_t *new_skin)
 * \brief Sets new skin.
 *
 * The function sets a new skin pointed to by \a new_skin for
 * the skin window \a hwnd.
 *
 * \param hwnd The handle to the skin window.
 * \param new_skin The pointer to the new skin object.
 *
 * \return The pointer to the old skin object.
 *
 * \sa skin_set_event_cb, skin_set_msg_cb
 */
skin_head_t* set_window_skin (HWND hwnd, skin_head_t *new_skin);

/**
 * \fn skin_head_t* get_window_skin (HWND hwnd)
 * \brief Gets new skin.
 *
 * The function gets the skin of skin window \a hwnd.
 *
 * \param hwnd The handle to the skin window.
 *
 * \return The pointer to the skin object.
 *
 * \sa skin_set_event_cb, skin_set_msg_cb
 */
skin_head_t* get_window_skin (HWND hwnd);

/**
 * \fn skin_event_cb_t skin_set_event_cb (skin_head_t* skin, skin_event_cb_t event_cb)
 * \brief Sets new event callback of a skin.
 *
 * This function sets \a event_cb as the event callback of the skin \a skin.
 *
 * \param skin The pointer to the skin object.
 * \param event_cb The new event callback.
 *
 * \return The old event callback.
 *
 * \sa skin_set_msg_cb
 */
static inline skin_event_cb_t skin_set_event_cb (skin_head_t* skin, skin_event_cb_t event_cb)
{
    skin_event_cb_t old = skin->event_cb;
    skin->event_cb = event_cb;
    return old;
}

/**
 * \fn skin_msg_cb_t skin_set_msg_cb (skin_head_t* skin, skin_msg_cb_t msg_cb)
 * \brief Sets new message callback of a skin window.
 *
 * This function sets \a msg_cb as the message callback of the skin \a skin.
 *
 * \param skin The pointer to the skin object.
 * \param msg_cb The new message callback.
 *
 * \return The old message callback.
 *
 * \sa skin_set_event_cb
 */
static inline skin_msg_cb_t skin_set_msg_cb (skin_head_t* skin, skin_msg_cb_t msg_cb)
{
    skin_msg_cb_t old = skin->msg_cb;
    skin->msg_cb = msg_cb;
    return old;
}

/**
 * \fn skin_item_t* skin_get_item (skin_head_t* skin, int id)
 * \brief Retrives a skin item through its identifier.
 *
 * This function retrives the pointer to a skin item by its identifier.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 *
 * \return The pointer to the skin item, NULL when error.
 *
 * \sa skin_item_s
 */
skin_item_t* skin_get_item (skin_head_t* skin, int id);

/**
 * \fn DWORD skin_get_item_status (skin_head_t* skin, int id)
 * \brief Retrives the common status of a skin item.
 *
 * This function retrives the common status of a skin item by its identifier.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 *
 * \return The common status of the skin items, zero when error.
 *
 * \sa skin_item_s, skin_show_item, skin_enable_item, skin_set_hilited_item
 */
DWORD skin_get_item_status (skin_head_t* skin, int id);

/**
 * \fn skin_item_t* skin_get_hilited_item (skin_head_t* skin)
 * \brief Gets the current highlighted skin item.
 *
 * \param skin The pointer to the skin.
 *
 * \return The pointer to the current highlighted skin item.
 *
 * \sa skin_item_s, skin_set_hilited_item
 */
skin_item_t* skin_get_hilited_item (skin_head_t* skin);

/**
 * \fn DWORD skin_show_item (skin_head_t* skin, int id, BOOL show)
 * \brief Showes or hides a skin item.
 *
 * This function showes or hides a skin item whose identifier is \a id.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 * \param show TRUE for show, FALSE for hide.
 *
 * \return The current common status of the skin items, zero when error.
 *
 * \sa skin_item_s, skin_get_item_status
 */
DWORD skin_show_item (skin_head_t* skin, int id, BOOL show);

/**
 * \fn BOOL skin_enable_item (skin_head_t* skin, int id, BOOL enable)
 * \brief Enables or disables a skin item.
 *
 * This function enables or disables a skin item whose identifier is \a id.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 * \param enable TRUE for enable, FALSE for disable.
 *
 * \return The current common status of the skin items, zero when error.
 *
 * \sa skin_set_load
 */
DWORD skin_enable_item (skin_head_t* skin, int id, BOOL enable);

/**
 * \fn skin_item_t* skin_set_hilited_item (skin_head_t* skin, int id)
 * \brief Sets the current highlighted skin item.
 *
 * This function sets the current highlighted skin item.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 *
 * \return The pointer to the old highlighted skin item.
 *
 * \sa skin_item_s, skin_get_hilited_item
 */
skin_item_t* skin_set_hilited_item (skin_head_t* skin, int id);

/**
 * \fn BOOL skin_get_check_status (skin_head_t* skin, int id)
 * \brief Gets the check status of a check button item.
 *
 * This function gets the check status of a check button item,
 * checked or unchecked.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 *
 * \return TRUE for checked, FALSE for unchecked.
 *
 * \sa skin_item_s, skin_set_check_status
 */
BOOL skin_get_check_status (skin_head_t* skin, int id);

/**
 * \fn DWORD skin_set_check_status (skin_head_t* skin, int id, BOOL check)
 * \brief Sets the check status of a check button item.
 *
 * This function sets the check status of a check button item.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 * \param check Checks it or not.
 *
 * \return The current common status of the check button item.
 *
 * \sa skin_item_s, skin_get_check_status
 */
DWORD skin_set_check_status (skin_head_t* skin, int id, BOOL check);

/**
 * \fn const char* skin_get_item_label (skin_head_t* skin, int id)
 * \brief Gets the label string of a label item.
 *
 * This function returns the label string of a label item whose
 * identifier is \a id.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 *
 * \return The pointer to the label string. You should not
 *         modify the content of the string directly.
 *
 * \sa skin_item_s, skin_set_item_label
 */
const char* skin_get_item_label (skin_head_t* skin, int id);

/**
 * \fn BOOL skin_set_item_label (skin_head_t* skin, int id, const char* label)
 * \brief Sets the label string of a label item.
 *
 * This function sets the label string of a label item and reflects
 * the appearance of the item if it is visible.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 * \param label The pointer to the new label string.
 *
 * \return TRUE for success, FALSE on error.
 *
 * \sa skin_item_s, skin_get_item_label
 */
BOOL skin_set_item_label (skin_head_t* skin, int id, const char* label);

/**
 * \fn HWND skin_get_control_hwnd (skin_head_t* skin, int id)
 * \brief Gets the handle to a skin control item in the skin.
 *
 * This function returns the handle to a skin control item whose
 * identifier is \a id.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin control item.
 *
 * \return The handle to the control.
 *
 * \sa skin_item_s
 */
HWND skin_get_control_hwnd (skin_head_t* skin, int id);

/**
 * \fn int skin_get_thumb_pos (skin_head_t* skin, int id)
 * \brief Gets the thumb position of a slider item.
 *
 * This function gets the thumb position of a slider item.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 *
 * \return The current thumb position.
 *
 * \sa skin_item_s, skin_set_thumb_pos
 */
int skin_get_thumb_pos (skin_head_t* skin, int id);

/**
 * \fn BOOL skin_set_thumb_pos (skin_head_t* skin, int id, int pos)
 * \brief Sets the thumb position of a slider item.
 *
 * This function sets the thumb position of a slider item whose identifier
 * is \a id to the value of \a pos. It will redraw the item if the position
 * changed.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 * \param pos The new position value.
 *
 * \return TRUE for success, FALSE on error.
 *
 * \sa skin_item_s, skin_get_thumb_pos
 */
BOOL skin_set_thumb_pos (skin_head_t* skin, int id, int pos);

/**
 * \fn BOOL skin_get_slider_info (skin_head_t* skin, int id, sie_slider_t* sie)
 * \brief Retrives the sliding information of a slider item.
 *
 * This function retrives the sliding information of a slider item whose
 * identifier is \a id.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 * \param sie The pointer to a buffer which will be used to return the
 *        sliding information.
 *
 * \return TRUE for success, FALSE on error.
 *
 * \sa skin_item_s, skin_set_slider_info
 */
BOOL skin_get_slider_info (skin_head_t* skin, int id, sie_slider_t* sie);

/**
 * \fn BOOL skin_set_slider_info (skin_head_t* skin, int id, const sie_slider_t* sie)
 * \brief Sets the sliding information of a slider item.
 *
 * This function sets the sliding information of a slider item whose
 * identifier is \a id.
 *
 * \param skin The pointer to the skin.
 * \param id The identifier of the skin item.
 * \param sie The pointer to a sliding information structure.
 *
 * \return TRUE for success, FALSE on error.
 *
 * \sa skin_set_load
 */
BOOL skin_set_slider_info (skin_head_t* skin, int id, const sie_slider_t* sie);

/**
 * \fn int skin_scale_slider_pos (const sie_slider_t* org, int new_min, int new_max)
 * \brief Scales the sliding position.
 *
 * This function is a helper function, it calculates and returns 
 * the new position value of a sliding information in the new sliding range
 * defined by \a new_max and \a new_min.
 * 
 * \param org The original sliding information, its range and current position value.
 * \param new_min The new minimal value.
 * \param new_max The new maximal value.
 *
 * \return The new position value.
 *
 * \sa sie_slider_t
 */
int skin_scale_slider_pos (const sie_slider_t* org, int new_min, int new_max);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

    /** @} end of mgext_skin_fns */

    /** @} end of mgext_fns */

#endif /* _EXT_SKIN */

#endif /* _MGUI_SKIN_H */

