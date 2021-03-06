/*
** $Id: menubutton.h,v 1.5 2003/09/04 03:40:35 weiym Exp $
**
** menubutton.h: the head file of menu button control.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Create date: 2000/11/16
*/

#ifndef __MENUBUTTON_H_
#define __MENUBUTTON_H_

#ifdef  __cplusplus
extern  "C" {
#endif

#ifndef __MINIGUI_LIB__

/* styles of menu button control */
#define MBS_SORT                0x0001

/* struct used by parent to add/retrive item */
#define MB_WHICH_TEXT           0x01
#define MB_WHICH_BMP            0x02
#define MB_WHICH_ATTDATA        0x04
typedef struct _MenuButtonItem
{
    DWORD           which;          // which fields are valid, ignored when add item.
    const char*     text;           // item string
    PBITMAP         bmp;            // item bitmap
    DWORD           data;           // attached data
} MENUBUTTONITEM;
typedef MENUBUTTONITEM* PMENUBUTTONITEM;

/* notification code */
#define MBN_CHANGED             1   // send when selected item changed.
#define MBN_SELECTED            2   // send when item selected.
#define MBN_ERRSPACE            3   // send when memory space error occurred.
#define MBN_STARTMENU           4   // send when start tracking popup menu.
#define MBN_ENDMENU             5   // send when end tracking popup menu.

/* messages used by parent to control menubutton */
#define MBM_ADDITEM             0xF200  // send to add item
#define MBM_DELITEM             0xF201  // send to delete item
#define MBM_RESETCTRL           0xF202  // send to reset control
#define MBM_SETITEMDATA         0xF203  // send to set item data
#define MBM_GETITEMDATA         0xF204  // send to retrive the data of specific item
#define MBM_GETCURITEM          0xF206  // send to retrive the index of current selected item
#define MBM_SETCURITEM          0xF207  // send to set the current selected item based on index

#define MB_INV_ITEM             -1
#define MB_ERR_SPACE            -2

#define MENUBUTTON              ("menubutton")

#endif /* !__MINIGUI_LIB__ */

typedef struct _MBITEM
{
    int             index;          // index of item
    char*           text;           // item string
    PBITMAP         bmp;            // item bitmap
    DWORD           data;           // attached data
    struct _MBITEM* next;           // next item
} MBITEM;
typedef MBITEM* PMBITEM;

#define DEF_MB_BUFFER_LEN       5

typedef struct tagMenuBtnData
{
    STRCMP  str_cmp;        // function to compare two strings.

    int     item_count;     // items count.
    int     cur_sel;        // index of current selected item,
                            // -1 for none.
    MBITEM* first_item;     // first item.

    HMENU   hmnu;           // handle of popup menu.

    int     buff_len;       // buffer length.
    MBITEM* buff_head;      // buffer head.
    MBITEM* buff_tail;      // buffer tail.
    MBITEM* free_list;      // free list in buffer.

} MENUBTNDATA;
typedef MENUBTNDATA* PMENUBTNDATA;   

BOOL RegisterMenuButtonControl (void);
void MenuButtonControlCleanup (void);

#ifdef  __cplusplus
}
#endif

#endif  /* __MENUBUTTON_H_ */

