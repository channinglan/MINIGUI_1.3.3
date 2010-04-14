/**
 * \file control.h
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 2001/12/29
 * 
 * This file includes interfaces of standard controls of MiniGUI.
 *
 \verbatim
    Copyright (C) 1998-2002 Wei Yongming.
    Copyright (C) 2002-2003 Feynman Software.

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
 * $Id: control.h,v 1.106 2003/08/01 10:04:08 weiym Exp $
 *
 *             MiniGUI for Linux Version 1.3.x
 *             Copyright (C) 1999-2002 Wei Yongming.
 *             Copyright (C) 2002-2003 Feynman Software.
 */

#ifndef _MGUI_CONTROL_H
  #define _MGUI_CONTROL_H
 
#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/************************** Control Helpers **********************************/
    /**
     * \addtogroup fns Functions
     * @{
     */

    /**
     * \addtogroup global_fns Global/general functions
     * @{
     */

    /**
     * \addtogroup misc_fns Miscellaneous functions
     * @{
     */
/**
 * \fn Draw3DControlFrame (HDC hdc, int x0, int y0, int x1, int y1, gal_pixel fillc, BOOL updown)
 * \brief Draws a 3D style frame control.           
 *
 * \param hdc The handle to Drawing context.
 * \param x0  Specifies the x-coordinate of the upper-left corner.
 * \param y0  Specifies the y-coordinate of the upper-left corner.
 * \param x1  Specifies the x-coordinate of the lower-right corner.  
 * \param y1  Specifies the y-coordinate of the lower-right corner.
 * \param fillc Specifies the color to fill the frame.
 * \param updown Specifies the state of the frame control, ie. up or down. 
 *
 * \sa Draw3DControlFrame
 */
void GUIAPI Draw3DControlFrame (HDC hdc, int x0, int y0, int x1, int y1, 
            gal_pixel fillc, BOOL updown);

/**
 * \fn DrawFlatControlFrameEx(HDC hdc, int x0, int y0, int x1, int y1, gal_pixel fillc, int corner, BOOL updown)
 * \brief Draws a flat frame control with triangle corner.
 *
 * \param hdc The handle to Drawing context.
 * \param x0  Specifies the x-coordinate of the upper-left corner.
 * \param y0  Specifies the y-coordinate of the upper-left corner.
 * \param x1  Specifies the x-coordinate of the lower-right corner.
 * \param y1  Specifies the y-coordinate of the lower-right corner.
 * \param fillc Specifies the color to fill the frame.
 * \param corner Specifies the length of the edge of the triangle corner.
 * \param updown Specifies the state of the frame control, ie. up or down.
 *
 * \sa Draw3DControlFrame
 */
void GUIAPI DrawFlatControlFrameEx (HDC hdc, int x0, int y0, int x1, int y1, 
            gal_pixel fillc, int corner, BOOL updown);

/**
 * \def DrawFlatControlFrame(HDC hdc, int x0, int y0, int x1, int y1, gal_pixel fillc, BOOL updown)
 * \brief Draws a flat frame control.           
 *
 * \param hdc The handle to drawing context.
 * \param x0  Specifies the x-coordinate of the upper-left corner.
 * \param y0  Specifies the y-coordinate of the upper-left corner.
 * \param x1  Specifies the x-coordinate of the lower-right corner.  
 * \param y1  Specifies the y-coordinate of the lower-right corner.
 * \param fillc Specifies the color to fill the frame.
 * \param updown Specifies the state of the frame control, ie. up or down. 
 *
 * \note This function is actually a macro of DrawFlatControlFrameEx.
 *
 * \sa DrawFlatControlFrameEx
 */
#define DrawFlatControlFrame(hdc, x0, y0, x1, y1, fillc, updown) \
                 DrawFlatControlFrameEx(hdc, x0, y0, x1, y1, fillc, 3, updown);

/**
 * \fn void GUIAPI NotifyParentEx (HWND hwnd, int id, int code, DWORD add_data)
 * \brief Sends a notification message to the parent.
 *
 * By default, the notification from a control will be sent to its parent
 * window within a MSG_COMMAND messsage.
 *
 * Since version 1.2.6, MiniGUI defines the Nofication Callback Procedure 
 * for control. You can specify a callback function for a control by calling
 * \a SetNotificationCallback to receive and handle the notification from 
 * the control.
 *
 * If you have defined the Notificaton Callback Procedure for the control,
 * calling NotifyParentEx will call the notification callback procedure,
 * not send the notification message to the parent.
 *
 * \param hwnd The handle to current control window.
 * \param id The identifier of current control.
 * \param code The notification code.
 * \param add_data The additional data of the notification.
 *
 * \sa SetNotificationCallback
 */
void GUIAPI NotifyParentEx (HWND hwnd, int id, int code, DWORD add_data);

/**
 * \def NotifyParent(hwnd, id, code)
 * \brief Sends a notification message to the parent, 
 *        but without additional data.
 *
 * \param hwnd The handle to current control window.
 * \param id The identifier of current control.
 * \param code The notification code.
 *
 * \note This function is actually a macro of NotifyParentEx with 
 *       \a dwAddData being zero.
 *
 * \sa NotifiyParentEx
 */
#define NotifyParent(hwnd, id, code) \
                NotifyParentEx(hwnd, id, code, 0)

/**
 * \var typedef int (*STRCMP) (const char* s1, const char* s2, size_t n)
 * \brief Type of general strncmp function.
 *
 * The function compares the two strings \a s1 and \a s2. It returns
 * an integer less than, equal to, or greater than zero if  \a s1 is found,
 * respectively, to be less than, to match, or be greater than \a s2.
 *
 * Note that it only compares the first (at most) \a n characters of s1 and s2. 
 */
typedef int (*STRCMP) (const char* s1, const char* s2, size_t n);

    /** @} end of misc_fns */

    /** @} end of global_fns */

    /** @} end of fns */

    /**
     * \defgroup controls Standard controls
     * @{
     */

/****** control messages *****************************************************/

/* NOTE: control messages start from 0xF000 to 0xFFFF */
#define MSG_FIRSTCTRLMSG    0xF000
#define MSG_LASTCTRLMSG     0xFFFF

/****** Static Control ******************************************************/
#ifdef _CTRL_STATIC

    /**
     * \defgroup ctrl_static Static control
     * @{
     */

/**
 * \def CTRL_STATIC
 * \brief The class name of static control.
 */
#define CTRL_STATIC         ("static")

    /**
     * \defgroup ctrl_static_styles Styles of static control
     * @{
     */

/**
 * \def SS_LEFT
 * \brief Displays the given text flush-left.
 */
#define SS_LEFT             0x00000000L

/**
 * \def SS_CENTER 
 * \brief Displays the given text centered in the rectangle. 
 */
#define SS_CENTER           0x00000001L

/**
 * \def SS_RIGHT
 * \brief Displays the given text flush-right.
 */
#define SS_RIGHT            0x00000002L

/**
 * \def SS_ICON 
 * \brief Designates an icon displayed in the static control.
 */
#define SS_ICON             0x00000003L

/**
 * \def SS_BLACKRECT
 * \brief Specifies a rectangle filled with the black color.
 */
#define SS_BLACKRECT        0x00000004L

/**
 * \def SS_GRAYRECT
 * \brief Specifies a rectangle filled with the light gray color.
 */
#define SS_GRAYRECT         0x00000005L

/**
 * \def SS_WHITERECT
 * \brief Specifies a rectangle filled with the light white color.
 */
#define SS_WHITERECT        0x00000006L

/**
 * \def SS_BLACKFRAME
 * \brief Specifies a box with a frame drawn with the black color.
 */
#define SS_BLACKFRAME       0x00000007L

/**
 * \def SS_GRAYFRAME
 * \brief Specifies a box with a frame drawn with the light gray color.
 */
#define SS_GRAYFRAME        0x00000008L

/**
 * \def SS_WHITEFRAME
 * \brief Specifies a box with a frame drawn with the light gray color.
 */
#define SS_WHITEFRAME       0x00000009L

/**
 * \def SS_GROUPBOX
 * \brief Creates a rectangle in which other controls can be grouped.
 */
#define SS_GROUPBOX         0x0000000AL

/**
 * \def SS_SIMPLE 
 * \brief Designates a simple rectangle and displays a single line 
 *        of text flush-left in the rectangle.
 */
#define SS_SIMPLE           0x0000000BL

/**
 * \def SS_LEFTNOWORDWRAP
 * \brief Designates a simple rectangle and displays the given text 
 *        flush-left in the rectangle. 
 *
 * Tabs are expanded, but words are not wrapped. 
 * Text that extends past the end of a line is clipped.
 */
#define SS_LEFTNOWORDWRAP   0x0000000CL

#define SS_OWNERDRAW        0x0000000DL

/**
 * \def SS_BITMAP
 * \brief Specifies that a bitmap will be displayed in the static control.
 */
#define SS_BITMAP           0x0000000EL

#define SS_ENHMETAFILE      0x0000000FL
#define SS_TYPEMASK         0x0000000FL


/**
 * \def SS_NOPREFIX
 * \brief Prevents interpretation of any ampersand (&) characters in 
 *        the control's text as accelerator prefix characters.
 *
 * \note Not implemented so far.
 */
#define SS_NOPREFIX         0x00000080L

/**
 * \def SS_NOTIFY
 * \brief Sends the parent window notification messages when the user 
 *        clicks or double-clicks the control.
 */
#define SS_NOTIFY           0x00000100L

/**
 * \def SS_CENTERIMAGE
 * \brief Puts the image in the center of the static control.
 *        Default is top-left aligned.
 */
#define SS_CENTERIMAGE      0x00000200L

/**
 * \def SS_REALSIZEIMAGE
 * \brief Does not scale the image.
 */
#define SS_REALSIZEIMAGE    0x00000800L

    /** @} end of ctrl_static_styles */
    
    /**
     * \defgroup ctrl_static_msgs Messages of static control
     * @{
     */

#define STM_SETICON         0xF170
#define STM_GETICON         0xF171

/**
 * \def STM_SETIMAGE
 * \brief Associates a new image (icon or bitmap) with a static control.
 *
 * An application sends an STM_SETIMAGE message to 
 * associate a new image (icon or bitmap) with a static control.
 *
 * \code
 * STM_SETIMAGE
 * HICON image;
 *  or 
 * BITMAP* image;
 *
 * wParam = (WPARAM)image;
 * lParam = 0;
 * \endcode
 * 
 * \param image The handle to an icon if the type of static control type 
 *        is SS_ICON, or the pointer to a BITMAP object if the type is SS_BITMAP.
 *
 * \return The old image (handle or pointer).
 */
#define STM_SETIMAGE        0xF172

/**
 * \def STM_GETIMAGE
 * \brief Retrieves a handle to the image.
 *
 * An application sends an STM_GETIMAGE message to retrieve a handle 
 * to the image associated with a static control.
 *
 * \code
 * STM_GETIMAGE
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The handle to the icon if the type of static control type is SS_ICON, 
 *         or the pointer to the BITMAP object if the type is SS_BITMAP.
 */
#define STM_GETIMAGE        0xF173

#define STM_MSGMAX          0xF174

    /** @} end of ctrl_static_msgs */

    /**
     * \defgroup ctrl_static_ncs Notification codes of static control
     * @{
     */

/**
 * \def STN_DBLCLK
 * \brief Notifies a double-click.
 *
 * The STN_DBLCLK notification message is sent when 
 * the user double-clicks a static control that has the SS_NOTIFY style.
 */
#define STN_DBLCLK          1

#define STN_ENABLE          2
#define STN_DISABLE         3

/**
 * \def STN_CLICKED
 * \brief Notifies that a static control is clicked.
 *
 * The STN_CLICKED notification message is sent 
 * when the user clicks a static control that has the SS_NOTIFY style.
 */
#define STN_CLICKED         4

    /** @} end of ctrl_static_ncs */

    /** @} end of ctrl_static */

#endif /* _CTRL_STATIC */

/****** Button Control ******************************************************/
#ifdef _CTRL_BUTTON

    /**
     * \defgroup ctrl_button Button control
     * @{
     */

/**
 * \def CTRL_BUTTON
 * \brief The class name of button control.
 */
#define CTRL_BUTTON         ("button")

    /**
     * \defgroup ctrl_button_styles Styles of button control
     * @{
     */

/**
 * \def BS_PUSHBUTTON
 * \brief Creates a push button.
 */
#define BS_PUSHBUTTON       0x00000000L

/**
 * \def BS_DEFPUSHBUTTON
 * \brief Creates a push button that behaves like a BS_PUSHBUTTON style button.
 
 * Creates a push button that behaves like a BS_PUSHBUTTON style button, 
 * but also has a heavy black border.  If the button is in a dialog box, 
 * the user can select the button by pressing the enter key, 
 * even when the button does not have the input focus. 
 * This style is useful for enabling the user to quickly select 
 * the most likely (default) option.
 */
#define BS_DEFPUSHBUTTON    0x00000001L

/**
 * \def BS_CHECKBOX
 * \brief Creates a small, empty check box with text.
 *
 * By default, the text is displayed to the right of the check box. 
 * To display the text to the left of the check box, combine this flag 
 * with the BS_LEFTTEXT style (or with the equivalent BS_RIGHTBUTTON style).
 */
#define BS_CHECKBOX         0x00000002L

/**
 * \def BS_AUTOCHECKBOX
 * \brief Creates a button that is almost the same as a check box.
 *
 * Creates a button that is the same as a check box, 
 * except that the check state automatically toggles between 
 * checked and unchecked each time the user selects the check box.
 */
#define BS_AUTOCHECKBOX     0x00000003L

/**
 * \def BS_RADIOBUTTON
 * \brief Creates a small circle with text.
 *
 * By default, the text is displayed to the right of the circle. 
 * To display the text to the left of the circle, combine this flag 
 * with the BS_LEFTTEXT style (or with the equivalent BS_RIGHTBUTTON style). 
 * Use radio buttons for groups of related, but mutually exclusive choices.
 */
#define BS_RADIOBUTTON      0x00000004L

/**
 * \def BS_3STATE
 * \brief Creates a button that is almost the same as a check box.
 * 
 * Creates a button that is the same as a check box, except 
 * that the box can be grayed as well as checked or unchecked.
 * Use the grayed state to show that the state of the check box 
 * is not determined.
 */
#define BS_3STATE           0x00000005L

/**
 * \def BS_AUTO3STATE
 * \brief Creates a button that is almost the same as a three-state check box.
 *
 * Creates a button that is the same as a three-state check box, 
 * except that the box changes its state when the user selects it.
 * The state cycles through checked, grayed, and unchecked.
 */
#define BS_AUTO3STATE       0x00000006L

#define BS_GROUPBOX         0x00000007L
#define BS_USERBUTTON       0x00000008L

/**
 * \def BS_AUTORADIOBUTTON
 * \brief Creates a button that is almost the same as a radio button.
 * 
 * Creates a button that is the same as a radio button, 
 * except that when the user selects it, The system automatically 
 * sets the button's check state to checked
 * and automatically sets the check state for all other buttons 
 * in the same group to unchecked.
 */
#define BS_AUTORADIOBUTTON  0x00000009L

/**
 * \def BS_OWNERDRAW
 * \brief Creates an owner-drawn button.
 *
 * \note Not implemented so far.
 */
#define BS_OWNERDRAW        0x0000000BL

#define BS_TYPEMASK         0x0000000FL

/**
 * \def BS_TEXT
 * \brief Specifies that the button displays text.
 */
#define BS_TEXT             0x00000000L

/**
 * \def BS_LEFTTEXT
 * \brief Places text on the left side.
 *
 * Places text on the left side of the radio button 
 * or check box when combined with a radio button or check box style.
 */
#define BS_LEFTTEXT         0x00000020L

/**
 * \def BS_ICON
 * \brief Specifies that the button displays an icon.
 */
#define BS_ICON             0x00000040L

/**
 * \def BS_BITMAP
 * \brief Specifies that the button displays a bitmap.
 */
#define BS_BITMAP           0x00000080L

#define BS_CONTENTMASK      0x000000F0L

/**
 * \def BS_LEFT
 * \brief Left-justifies the text in the button rectangle.
 *
 * However, if the button is a check box or radio button that 
 * does not have the BS_RIGHTBUTTON style, the text is left 
 * justified on the right side of the check box or radio button.
 */
#define BS_LEFT             0x00000100L

/**
 * \def BS_RIGHT
 * \brief Right-justifies text in the button rectangle.
 * 
 * However, if the button is a check box or radio button that 
 * does not have the BS_RIGHTBUTTON style, the text is 
 * right justified on the right side of the check box or radio button.
 */
#define BS_RIGHT            0x00000200L

/**
 * \def BS_CENTER
 * \brief Centers text horizontally in the button rectangle.
 */
#define BS_CENTER           0x00000300L

/**
 * \def BS_TOP
 * \brief Places text at the top of the button rectangle.
 */
#define BS_TOP              0x00000400L

/**
 * \def BS_BOTTOM
 * \brief Places text at the bottom of the button rectangle.
 */
#define BS_BOTTOM           0x00000800L

/**
 * \def BS_VCENTER
 * \brief Places text in the middle (vertically) of the button rectangle.
 */
#define BS_VCENTER          0x00000C00L

/**
 * \def BS_REALSIZEIMAGE
 * \brief Does not scale the image.
 */
#define BS_REALSIZEIMAGE    0x00000F00L

#define BS_ALIGNMASK        0x00000F00L

/**
 * \def BS_PUSHLIKE
 * \brief Makes a button look and act like a push button.
 *
 * Makes a button (such as a check box, three-state check box, or radio button) 
 * look and act like a push button. The button looks raised when it isn't 
 * pushed or checked, and sunken when it is pushed or checked.
 */
#define BS_PUSHLIKE         0x00001000L

/**
 * \def BS_MULTLINE
 * \brief Wraps the button text to multiple lines.
 *
 * Wraps the button text to multiple lines if the text string is 
 * too long to fit on a single line in the button rectangle.
 */
#define BS_MULTLINE         0x00002000L

/**
 * \def BS_NOTIFY
 * \brief Enables a button to send notification messages to its parent window.
 */
#define BS_NOTIFY           0x00004000L

/**
 * \def BS_CHECKED
 * \brief Makes a button checked initially.
 */
#define BS_CHECKED          0x00004000L

#define BS_FLAT             0x00008000L
#define BS_NOBORDER            0x00010000L
#define BS_RIGHTBUTTON      BS_LEFTTEXT

    /** @} end of ctrl_button_styles */

    /**
     * \defgroup ctrl_button_states States of button control
     * @{
     */

/**
 * \def BST_UNCHECKED
 * \brief Indicates the button is unchecked.
 */
#define BST_UNCHECKED       0x0000

/**
 * \def BST_CHECKED
 * \brief Indicates the button is checked.
 */
#define BST_CHECKED         0x0001

/**
 * \def BST_INDETERMINATE
 * \brief Indicates the button is grayed because 
 * the state of the button is indeterminate.
 */
#define BST_INDETERMINATE   0x0002

/**
 * \def BST_PUSHED
 * \brief Specifies the highlight state.
 */
#define BST_PUSHED          0x0004

/**
 * \def BST_FOCUS
 * \brief Specifies the focus state.
 */
#define BST_FOCUS           0x0008

    /** @} end of ctrl_button_states */

    /**
     * \defgroup ctrl_button_msgs Messages of button control
     * @{
     */

/**
 * \def BM_GETCHECK
 * \brief Retrieves the check state of a radio button or check box.
 *
 * An application sends a BM_GETCHECK message to retrieve 
 * the check state of a radio button or check box.
 *
 * \code
 * BM_GETCHECK
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return An integer indicates whether the button is checked.
 *
 * \retval BST_UNCHECKED The button is not checked.
 * \retval BST_CHECKED The button is checked.
 * \retval BST_INDETERMINATE The button is grayed because the state of the button is indeterminate.
 *
 * \sa ctrl_button_states
 */
#define BM_GETCHECK             0xF0F0

/**
 * \def BM_SETCHECK
 * \brief Sets the check state of a radio button or check box.
 *
 * An application sends a BM_SETCHECK message to set 
 * the check state of a radio button or check box.
 *
 * \code
 * BM_SETCHECK
 * int check_state;
 *
 * wParam = (WPARAM)check_state;
 * lParam = 0;
 * \endcode
 *
 * \param check_state The check state of button, can be one of the following values:
 *      - BST_UNCHECKED\n
 *        Want the button to be unchecked.
 *      - BST_CHECKED\n
 *        Want the button to be checked.
 *      - BST_INDETERMINATE\n
 *        Want the button to be grayed if it is a three states button.
 * \return The old button state.
 */
#define BM_SETCHECK             0xF0F1

/**
 * \def BM_GETSTATE
 * \brief Gets the state of a button or check box.
 *
 * An application sends a BM_GETSTATE message to 
 * determine the state of a button or check box.
 *
 * \code
 * BM_GETSTATE
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return An integer indicates the button state.
 *
 * \sa ctrl_button_states
 */
#define BM_GETSTATE             0xF0F2

/**
 * \def BM_SETSTATE
 * \brief Sets the state of a button.
 *
 * An application sends a BM_GETSTATE message to set the state of a 
 * button.
 *
 * \code
 * BM_SETSTATE
 * int push_state;
 *
 * wParam = (WPARAM)push_state;
 * lParam = 0;
 * \endcode
 *
 * \param push_state The push state of a button, can be one of the following values:
 *      - Zero\n
 *        Want the button to be unpushed.
 *      - Non zero\n
 *        Want the button to be pushed.
 *
 * \return The old button state.
 */
#define BM_SETSTATE             0xF0F3

/**
 * \def BM_SETSTYLE
 * \brief Changes the style of a button.
 *
 * An application sends a BM_SETSTYLE message to change the style of a button.
 *
 * \code
 * BM_SETSTYLE
 * int button_style;
 *
 * wParam = (WPARAM)button_style;
 * lParam = 0;
 * \endcode
 *
 * \param button_style The styles of a button.
 *
 * \return Always be zero.
 *
 * \sa ctrl_button_styles
 */
#define BM_SETSTYLE             0xF0F4

/**
 * \def BM_CLICK
 * \brief Simulates the user clicking a button.
 *
 * An application sends a BM_CLICK message to simulate the user clicking a button.
 *
 * \code
 * BM_CLICK
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 */
#define BM_CLICK                0xF0F5

/**
 * \def BM_GETIMAGE
 * \brief Retrieves the handle to the image.
 *
 * An application sends a BM_GETIMAGE message to 
 * retrieve a handle to the image (icon or bitmap) associated with the button.
 *
 * \code
 * BM_GETIMAGE
 * int image_type;
 *
 * wParam = (WPARAM)&image_type;
 * lParam = 0;
 * \endcode
 *
 * \param image_type The type of a button image will be returned through this buferr.
 *        It can be one of the following values:
 *      - BM_IMAGE_BITMAP\n
 *        Bitmap of a button.
 *      - BM_IMAGE_ICON\n
 *        Icon of a button.
 *
 * \return A handle of the bitmap or icon of the button, zero when error.
 */
#define BM_GETIMAGE             0xF0F6

#define BM_IMAGE_BITMAP         1
#define BM_IMAGE_ICON           2
    
/**
 * \def BM_SETIMAGE
 * \brief Associates a new image (icon or bitmap) with the button.
 *
 * An application sends a BM_SETIMAGE message to 
 * associate a new image (icon or bitmap) with the button.
 *
 * Please use BM_IMAGE_BITMAP or BM_IMAGE_ICON as the first parameter of the message 
 * to indicate the type of button control image:
 *  - BM_IMAGE_BITMAP\n
 *          Specifies the type of image to associate with the button to be a bitmap.
 *  - BM_IMAGE_ICON\n
 *          Specifies the type of image to associate with the button to be an icon.
 */
#define BM_SETIMAGE             0xF0F7

#define BM_MSGMAX               0xF100

    /** @} end of ctrl_button_msgs */
    
    /**
     * \defgroup ctrl_button_ncs Notification codes of button control
     * @{
     */

/**
 * \def BN_CLICKED
 * \brief The BN_CLICKED notification message is sent when the user clicks a button.
 */
#define BN_CLICKED          0

#define BN_PAINT            1   /* not supported */
#define BN_HILITE           2
#define BN_UNHILITE         3
#define BN_DISABLE          4   /* not supported */
#define BN_DOUBLECLICKED    5

/**
 * \def BN_PUSHED
 * \brief The BN_PUSHED notification message is sent when the user pushes a button.
 */
#define BN_PUSHED           BN_HILITE

/**
 * \def BN_UNPUSHED
 * \brief The BN_UNPUSHED notification message is sent when the user unpushes a button.
 */
#define BN_UNPUSHED         BN_UNHILITE

/**
 * \def BN_DBLCLK
 * \brief The BN_DBLCLK notification message is sent when the user double-clicks a button.
 */
#define BN_DBLCLK           BN_DOUBLECLICKED

/**
 * \def BN_SETFOCUS
 * \brief The BN_SETFOCUS notification message is sent when a button receives the keyboard focus.
 */
#define BN_SETFOCUS         6

/**
 * \def BN_KILLFOCUS
 * \brief The BN_KILLFOCUS notification message is sent when a button loses the keyboard focus.
 */
#define BN_KILLFOCUS        7

    /** @} end of ctrl_button_ncs */

    /** @} end of ctrl_button */

#endif /* _CTRL_BUTTON */

/****** Edit and MEdit Control ***********************************************/
#if defined (_CTRL_SIMEDIT) || defined(_CTRL_SLEDIT) || defined(_CTRL_MLEDIT)

    /**
     * \defgroup ctrl_edit Edit/MEdit control
     *
     * \bug You can not pass caption argument for multi-line edit control
     *      when you create it, just use null string:
     *
     * \code
     *  CreateWindowEx (CTRL_MEDIT, ..., "", ...);
     * \endcode
     *
     * @{
     */

/**
 * \def CTRL_EDIT
 * \brief The class name of simple single-line editor box.
 *
 * This edit control uses the system default fixed logical font.
 */
#define CTRL_EDIT           ("edit")

/**
 * \def CTRL_SLEDIT
 * \brief The class name of single-line editor box.
 *
 * This edit control uses the system logical font for control,
 * which may be variable-width font..
 */
#define CTRL_SLEDIT         ("sledit")

/**
 * \def CTRL_MLEDIT
 * \brief The class name of multiple-line editor box.
 *
 * This edit control uses the system logical font for control,
 * which may be variable-width font..
 */
#define CTRL_MLEDIT         ("mledit")

/**
 * \def CTRL_MEDIT
 * \brief Another class name of multiple-line editor box.
 *
 * This edit control uses the system logical font for control,
 * which may be variable-width font..
 */
#define CTRL_MEDIT          ("medit")

    /**
     * \defgroup ctrl_edit_styles Styles of edit control
     * @{
     */

/**
 * \def ES_LEFT
 * \brief Left-aligns text.
 */
#define ES_LEFT             0x00000000L

#define ES_CENTER           0x00000001L
#define ES_RIGHT            0x00000002L
#define ES_MULTILINE        0x00000004L

/**
 * \def ES_UPPERCASE
 * \brief Converts all characters to uppercase as they are typed into the edit control.
 */
#define ES_UPPERCASE        0x00000008L

/**
 * \def ES_LOWERCASE
 * \brief Converts all characters to lowercase as they are typed into the edit control.
 */
#define ES_LOWERCASE        0x00000010L

/**
 * \def ES_PASSWORD
 * \brief Displays an asterisk (*) for each character typed into the edit control.
 */
#define ES_PASSWORD         0x00000020L

#define ES_AUTOVSCROLL      0x00000040L
#define ES_AUTOHSCROLL      0x00000080L
#define ES_NOHIDESEL        0x00000100L
#define ES_OEMCONVERT       0x00000400L

/**
 * \def ES_READONLY
 * \brief Prevents the user from typing or editing text in the edit control.
 */
#define ES_READONLY         0x00000800L

/**
 * \def ES_BASELINE
 * \brief Draws base line under input area instead of frame border.
 */
#define ES_BASELINE         0x00001000L

/**
 * \def ES_AUTOWRAP
 * \brief Automatically wraps against border when inputting.
 */
#define ES_AUTOWRAP            0x00002000L

    /** @} end of ctrl_edit_styles */

    /**
     * \defgroup ctrl_edit_msgs Messages of edit control
     * @{
     */

#define EM_GETSEL               0xF0B0
#define EM_SETSEL               0xF0B1
#define EM_GETRECT              0xF0B2
#define EM_SETRECT              0xF0B3
#define EM_SETRECTNP            0xF0B4
#define EM_SCROLL               0xF0B5
#define EM_LINESCROLL           0xF0B6

/**
 * \def EM_GETCARETPOS
 * \brief Gets the position of the caret.
 *
 * \code
 * EM_GETCARETPOS
 * int* line_pos;
 * int* char_pos;
 *
 * wParam = (WPARAM)line_pos;
 * lParam = (LPARAM)char_pos;
 * \endcode
 *
 * \param line_pos Pointer to a integer buffer to save the caret line position.
 *        For single line editor, it is always zero.
 * \param char_pos Pointer to a integer buffer to save the caret character position.
 *
 * \return The character position of the caret.
 */
#define EM_GETCARETPOS          0xF0B7

/**
 * \def EM_SETCARETPOS
 * \brief Sets the position of the caret.
 *
 * \code
 * EM_SETCARETPOS
 * int line_pos;
 * int char_pos;
 *
 * wParam = (WPARAM)line_pos;
 * lParam = (LPARAM)char_pos;
 * \endcode
 *
 * \param line_pos The new caret line position. For single line editor, it will be ignored.
 * \param char_pos The new caret character position.
 *
 * \return Zero on success, otherwise -1.
 *
 * \note Not implemented for Multi-Line Edit control.
 */
#define EM_SETCARETPOS          0xF0B8

#define EM_SCROLLCARET          0xF0B9
#define EM_GETMODIFY            0xF0BA
#define EM_SETMODIFY            0xF0BB
#define EM_GETLINECOUNT         0xF0BC
#define EM_LINEINDEX            0xF0BD
#define EM_GETTHUMB             0xF0BE

#define EM_LINELENGTH           0xF0C1
#define EM_REPLACESEL           0xF0C2
#define EM_GETLINE              0xF0C4

/**
 * \def EM_LIMITTEXT
 * \brief Set text limit of an edit control.
 *
 * \code
 * EM_LIMITTEXT
 * int newLimit;
 *
 * wParam = (WPARAM)newLimit;
 * lParam = 0;
 * \endcode
 *
 * \param newLimit The new text limit of an edit control.
 */
#define EM_LIMITTEXT            0xF0C5

#define EM_CANUNDO              0xF0C6
#define EM_UNDO                 0xF0C7
#define EM_FMTLINES             0xF0C8
#define EM_LINEFROMCHAR         0xF0C9
#define EM_SETTABSTOPS          0xF0CB

/**
 * \def EM_SETPASSWORDCHAR
 * \brief Defines the character that edit control uses in conjunction with 
 * the ES_PASSWORD style.
 *
 * \code
 * EM_SETPASSWORDCHAR
 * int passwdChar;
 *
 * wParam = (WPARAM)passwdChar;
 * lParam = 0;
 * \endcode
 */
#define EM_SETPASSWORDCHAR      0xF0CC

#define EM_EMPTYUNDOBUFFER      0xF0CD
#define EM_GETFIRSTVISIBLELINE  0xF0CE

/**
 * \def EM_SETREADONLY
 * \brief Sets or removes the read-only style (ES_READONLY) in an edit control.
 *
 * \code
 * EM_SETREADONLY
 * int readonly;
 *
 * wParam = (WPARAM)readonly;
 * lParam = 0;
 * \endcode
 *
 * \param readonly Indicates whether the edit control is read-only:
 *      - Zero\n
 *        Not read-only.
 *      - Non zero\n
 *        Read-only.
 */
#define EM_SETREADONLY          0xF0CF

#define EM_SETWORDBREAKPROC     0xF0D0
#define EM_GETWORDBREAKPROC     0xF0D1

/**
 * \def EM_GETPASSWORDCHAR
 * \brief Returns the character that edit controls uses in conjunction with 
 * the ES_PASSWORD style.
 *
 * \code
 * EM_GETPASSWORDCHAR
 * int *passwdchar;
 *
 * wParam = 0;
 * lParam = (LPARAM)passwdchar;
 * \endcode
 *
 */
#define EM_GETPASSWORDCHAR      0xF0D2

#define EM_SETMARGINS           0xF0D3
#define EM_GETMARGINS           0xF0D4
#define EM_SETLIMITTEXT         EM_LIMITTEXT
#define EM_GETLIMITTEXT         0xF0D5
#define EM_POSFROMCHAR          0xF0D6
#define EM_CHARFROMPOS          0xF0D7
#define EM_SETIMESTATUS         0xF0D8
#define EM_GETIMESTATUS         0xF0D9
#define MEM_SCROLLCHANGE        0xF0DB
#define MED_STATE_YES           0x0
#define MED_STATE_NOUP          0x1
#define MED_STATE_NODN          0x2
#define MED_STATE_NO            0x3

#define EM_MSGMAX               0xF0E0

    /** @} end of ctrl_edit_msgs */

    /**
     * \defgroup ctrl_edit_ncs Notification codes of edit control
     * @{
     */

#define EN_ERRSPACE         255

/**
 * \def EN_CLICKED
 * \brief Notifies a click in an edit control.
 *
 * An edit control sends the EN_CLICKED notification code when the user clicks
 * in an edit control.
 */
#define EN_CLICKED          0x0001

/**
 * \def EN_DBLCLK
 * \brief Notifies a double click in an edit control.
 *
 * An edit control sends the EN_CLICKED notification code when the user 
 * double clicks in an edit control.
 */
#define EN_DBLCLK           0x0002

/**
 * \def EN_SETFOCUS
 * \brief Notifies the receipt of the input focus.
 *
 * The EN_SETFOCUS notification code is sent when an edit control receives 
 * the input focus.
 */
#define EN_SETFOCUS         0x0100

/**
 * \def EN_KILLFOCUS
 * \brief Notifies the lost of the input focus.
 *
 * The EN_KILLFOCUS notification code is sent when an edit control loses 
 * the input focus.
 */
#define EN_KILLFOCUS        0x0200

/**
 * \def EN_CHANGE
 * \brief Notifies that the text is altered.
 *
 * An edit control sends the EN_CHANGE notification code when the user takes 
 * an action that may have altered text in an edit control.
 */
#define EN_CHANGE           0x0300

#define EN_UPDATE           0x0400

/**
 * \def EN_MAXTEXT
 * \brief Notifies reach of maximum text limitation.
 *
 * The EN_MAXTEXT notification message is sent when the current text 
 * insertion has exceeded the specified number of characters for the edit control.
 */
#define EN_MAXTEXT          0x0501

#define EN_HSCROLL          0x0601
#define EN_VSCROLL          0x0602

/**
 * \def EN_ENTER
 * \brief Notifies the user has type the ENTER key in a single-line edit control.
 */
#define EN_ENTER            0x0700

    /** @} end of ctrl_edit_ncs */

/* Edit control EM_SETMARGIN parameters */
/**
 * \def EC_LEFTMARGIN
 * \brief Value of wParam. Specifies the margins to set.
 */
#define EC_LEFTMARGIN       0x0001
/**
 * \def EC_RIGHTMARGIN
 * \brief Value of wParam. Specifies the margins to set.
 */
#define EC_RIGHTMARGIN      0x0002
/**
 * \def EC_USEFONTINFO
 * \brief Value of wParam. Specifies the margins to set.
 */
#define EC_USEFONTINFO      0xffff

/* wParam of EM_GET/SETIMESTATUS  */
/**
 * \def EMSIS_COMPOSITIONSTRING
 * \brief Indicates the type of status to retrieve.
 */
#define EMSIS_COMPOSITIONSTRING        0x0001

/* lParam for EMSIS_COMPOSITIONSTRING  */
/**
 * \def EIMES_GETCOMPSTRATONCE
 * \brief lParam for EMSIS_COMPOSITIONSTRING.
 */
#define EIMES_GETCOMPSTRATONCE         0x0001
/**
 * \def EIMES_CANCELCOMPSTRINFOCUS
 * \brief lParam for EMSIS_COMPOSITIONSTRING.
 */
#define EIMES_CANCELCOMPSTRINFOCUS     0x0002
/**
 * \def EIMES_COMPLETECOMPSTRKILLFOCUS
 * \brief lParam for EMSIS_COMPOSITIONSTRING.
 */
#define EIMES_COMPLETECOMPSTRKILLFOCUS 0x0004

    /** @} end of ctrl_edit */

#endif /* _CTRL_EDIT || _CTRL_MEDIT */

/****** Progress Bar Control *************************************************/
#ifdef _CTRL_PROGRESSBAR

    /**
     * \defgroup ctrl_progbar ProgressBar control
     * @{
     */

/**
 * \def CTRL_PROGRESSBAR
 * \brief The class name of progressbar control.
 */
#define CTRL_PROGRESSBAR        ("progressbar")

#define PB_OKAY                 0
#define PB_ERR                  -1

    /**
     * \defgroup ctrl_progbar_styles Styles of progressbar control
     * @{
     */

/**
 * \def PBS_NOTIFY
 * \brief Notifies the parent window.
 * 
 * Sends the parent window notification messages when 
 * the user clicks or double-clicks the control.
 */
#define PBS_NOTIFY              0x0001L

/**
 * \def PBS_VERTICAL
 * \brief Creates progressbar vertically.
 */
#define PBS_VERTICAL            0x0002L

    /** @} end of ctrl_progbar_styles */

    /**
     * \defgroup ctrl_progbar_msgs Messages of progressbar control
     * @{
     */

/**
 * \def PBM_SETRANGE
 * \brief Sets the limits of the range.
 *
 * Sets the upper and lower limits of the progress bar control's range, 
 * and redraws the bar to reflect the new ranges.
 *
 * \code
 * PBM_SETRANGE
 * int min, max;
 *
 * wParam = (WPARAM)min;
 * lParam = (LPARAM)max;
 * \endcode
 *
 * \param min The lower limit of the progress bar.
 * \param max The upper limit of the progress bar.
 *
 * \return PB_OKAY on success, else PB_ERR.
 */
#define PBM_SETRANGE            0xF0A0

/**
 * \def PBM_SETSTEP
 * \brief Specifies the step increment for a progress bar control.
 *
 * \code
 * PBM_SETSTEP
 * int stepinc;
 *
 * wParam = (WPARAM)stepinc;
 * lParam = 0;
 * \endcode
 *
 * \param stepinc Step increment for a progress bar control.
 * \return PB_OKAY on success, else PB_ERR.
 */
#define PBM_SETSTEP             0xF0A1

/**
 * \def PBM_SETPOS
 * \brief Sets the progress bar control's current position.
 *
 * Sets the progress bar control's current position as specified by nPos, 
 * and redraw the bar to reflect the new position.
 *
 * \code
 * PBM_SETPOS
 * int nPos;
 *
 * wParam = (WPARAM)nPos;
 * lParam = 0;
 * \endcode
 *
 * \param nPos The progress bar control's current position.
 * \return Always be PB_OKAY.
 */
#define PBM_SETPOS              0xF0A2

/**
 * \def PBM_DELTAPOS
 * \brief Advances the progress bar control's current position. 
 *
 * Advances the progress bar control's current position as specified by posInc, 
 * and redraw the bar to reflect the new position.
 *
 * \code
 * PBM_DELTAPOS
 * int posInc;
 *
 * wParam = (WPARAM)posInc;
 * lParam = 0;
 * \endcode
 *
 * \param posInc The progress bar control's position increment.
 * \return Always be PB_OKAY.
 */
#define PBM_DELTAPOS            0xF0A3

/**
 * \def PBM_STEPIT
 * \brief Advances the current position by the step increment.
 *
 * Advances the current position for a progress bar control by 
 * the step increment, and redraw the bar to reflect the new position.
 *
 * \code
 * PBM_STEPIT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return Always be PB_OKAY.
 */
#define PBM_STEPIT              0xF0A4

#define PBM_MSGMAX              0xF0B0

    /** @} end of ctrl_progbar_msgs */

    /**
     * \defgroup ctrl_progbar_ncs Notification codes of progressbar control
     * @{
     */

/**
 * \def PBN_REACHMAX
 * \brief Notifies reach of maximum limit.
 *
 * The PBN_REACHMAX notification code is sent when the progressbar reachs its maximum limit.
 */
#define PBN_REACHMAX            1

/**
 * \def PBN_REACHMIN
 * \brief Notifies reach of minimum limit.
 *
 * The PBN_REACHMIN notification code is sent when the progressbar reachs its minimum limit.
 */
#define PBN_REACHMIN            2

    /** @} end of ctrl_progbar_ncs */

    /** @} end of ctrl_progbar */

#endif /* _CTRL_PROGRESSBAR */

/****** List Box Control *****************************************************/
#ifdef _CTRL_LISTBOX

    /**
     * \defgroup ctrl_listbox ListBox control
     *
     *
     * @{
     */

/**
 * \def CTRL_LISTBOX
 * \brief The class name of listbox control.
 */
#define CTRL_LISTBOX        ("listbox")

/* Listbox return value */
#define LB_OKAY                 0
#define LB_ERR                  (-1)
#define LB_ERRSPACE             (-2)

#define CMFLAG_BLANK            0x0000
#define CMFLAG_CHECKED          0x0001
#define CMFLAG_PARTCHECKED      0x0002
#define CMFLAG_MASK             0x000F

#define IMGFLAG_BITMAP          0x0010

/** Structrue of the listbox item info */
typedef struct _LISTBOXITEMINFO
{
    /** Item string */
    char* string;

    /** 
     * Check mark and image flag. It can be one of the following values:
     * - CMFLAG_BLANK
     *   The item is blank.
     * - CMFLAG_CHECKED
     *   The item is checked.
     * - CMFLAG_PARTCHECKED
     *   The item is partly checked.
     *
     * For LBS_ICON list box, if you want to display bitmap other than icon, 
     * you can OR'd \a cmFlag whit \a IMGFLAG_BITMAP.
     */
    DWORD   cmFlag;         /* check mark flag */

    /** Handle to the icon (or pointer to bitmap object) of the item */
    HICON   hIcon;          /* handle to icon */
} LISTBOXITEMINFO;

/** 
 * \var typedef LISTBOXITEMINFO* PLISTBOXITEMINFO;
 * \brief Data type of the pointer to a LISTBOXITEMINFO.
 */
typedef LISTBOXITEMINFO* PLISTBOXITEMINFO;

    /**
     * \defgroup ctrl_listbox_styles Styles of listbox control
     * @{
     */

/**
 * \def LBS_NOTIFY
 * \brief Notifies the parent window.
 *
 * Causes the list box to notify the list box parent window 
 * with a notification message when the user clicks or doubleclicks an item.
 */
#define LBS_NOTIFY              0x0001L

/**
 * \def LBS_SORT
 * \brief Sorts strings alphabetically.
 *
 * Causes the list box to sort strings alphabetically that are 
 * added to the list box with an LB_ADDSTRING message.
 */
#define LBS_SORT                0x0002L

/**
 * \def LBS_MULTIPLESEL
 * \brief Causes the list box to allow the user to select multiple items.
 */
#define LBS_MULTIPLESEL         0x0008L

/**
 * \def LBS_CHECKBOX
 * \brief Displays a check box in an item.
 */
#define LBS_CHECKBOX            0x1000L

/**
 * \def LBS_USEICON
 * \brief Displays an icon or bitmap in an item.
 */
#define LBS_USEICON             0x2000L

/**
 * \def LBS_AUTOCHECK
 * \brief If the list box has LBS_CHECKBOX style, this
 *        style tell the box to auto-switch the check box between 
 *        checked or un-checked when the user click the check mark box of an item.
 */
#define LBS_AUTOCHECK           0x4000L

#define LBS_AUTOCHECKBOX        (LBS_CHECKBOX | LBS_AUTOCHECK)

#define LBS_OWNERDRAWFIXED      0x0010L
#define LBS_OWNERDRAWVARIABLE   0x0020L
#define LBS_USETABSTOPS         0x0080L
#define LBS_MULTICOLUMN         0x0200L
#define LBS_WANTKEYBOARDINPUT   0x0400L
#define LBS_NOREDRAW            0x0004L
#define LBS_HASSTRINGS          0x0040L
#define LBS_NOINTEGRALHEIGHT    0x0100L
#define LBS_EXTENDEDSEL         0x0800L

    /** @} end of ctrl_listbox_styles */

    /**
     * \defgroup ctrl_listbox_msgs Messages of listbox control
     * @{
     */

/**
 * \def LB_ADDSTRING
 * \brief Appends the specified string.
 *
 * An application sends an LB_ADDSTRING message to append an item
 * specified in the lParam parameter to a list box.
 *
 * For a text-only list box:
 *
 * \code
 * LB_ADDSTRING
 * const char* text;
 *
 * wParam = 0;
 * lParam = (LPARAM)text;
 * \endcode
 *
 * \param text Pointer to the string of the item to be added.
 *
 * For a list box with check box or icon 
 * (with LBS_CHECKBOX or LBS_USEICON styles):
 *
 * \code
 * LB_ADDSTRING
 * PLISTBOXITEMINFO plbii;
 *
 * wParam = 0;
 * lParam = (LPARAM)plbii;
 * \endcode
 *
 * \param plbii Pointer to the listbox item info to be added.
 *
 * \return The index of the new item on success, else the one of
 *         the following error codes:
 *
 *         - LB_ERRSPACE    No memory can be allocated for new item.
 *         - LB_ERR         Invalid passed arguments.
 *
 */
#define LB_ADDSTRING            0xF180

/**
 * \def LB_INSERTSTRING
 * \brief Inserts an item to the list box.
 *
 * An application sends an LB_INSERTSTRING message to insert an item 
 * into a list box. Unlike LB_ADDSTRING message, the LB_INSERTSTRING
 * message do not cause the list to be sorted.
 *
 * For a text-only list box:
 *
 * \code
 * LB_INSERTSTRING
 * const char* text;
 *
 * wParam = index;
 * lParam = (LPARAM)text;
 * \endcode
 *
 * \param index Specifies the index of the position at which to insert the item.
 * \param text Pointer to the string of the item to be inserted.
 *
 * For a list box with check box or icon 
 * (with LBS_CHECKBOX or LBS_USEICON styles):
 *
 * \code
 * LB_INSERTSTRING
 * int index;
 * PLISTBOXITEMINFO plbii;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)plbii;
 * \endcode
 * 
 * \param index Specifies the index of the position at which to insert the item.
 * \param plbii Pointer to the listbox item info to be inserted.
 *
 * \return The index of the new item on success, else the one of
 *         the following error codes:
 *
 *         - LB_ERRSPACE    No memory can be allocated for new item.
 *         - LB_ERR         Invalid passed arguments.
 *
 */
#define LB_INSERTSTRING         0xF181

/**
 * \def LB_DELETESTRING
 * \brief Removes an item from the list box.
 *
 * An application sends an LB_DELETESTRING message to a list box 
 * to remove from the list box.
 *
 * \code
 * LB_DELETESTRING
 * int delete;
 *
 * wParam = (WPARAM)delete;
 * lParam = 0;
 * \endcode
 *
 * \param delete The index of the listbox item to be deleted.
 *
 * \return LB_OKAY on success, else LB_ERR to indicate you passed an invalid index.
 */
#define LB_DELETESTRING         0xF182

#define LB_SELITEMRANGEEX       0xF183

/**
 * \def LB_RESETCONTENT
 * \brief Removes the contents of a list box.
 *
 * An application sends an LB_RESETCONTENT message to remove the all items
 * in a list box.
 *
 * \code
 * LB_RESETCONTENT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return Always be zero.
 */
#define LB_RESETCONTENT         0xF184

/**
 * \def LB_GETSEL
 * \brief Gets the selected state for an specified item.
 *
 * An application sends an LB_GETSEL message to a list box to get the selected 
 * state for an item specified in the wParam parameter.
 *
 * \code
 * LB_GETSEL
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the specified item.
 *
 * \return The state of the specified item:
 *         - 0\n        Not selected.
 *         - >0\n       Selected.
 *         - LB_ERR\n   Invalid index.
 */
#define LB_GETSEL               0xF187

/**
 * \def LB_SETSEL
 * \brief Selects an item in a multiple-selection list box.
 *
 * An application sends an LB_SETSEL message to select an item 
 * in a multiple-selection list box and scroll it into view if necessary.
 *
 * \code
 * LB_SETSEL
 * int index, sel
 *
 * wParam = (WPARAM)sel;
 * lParam = (LPARAM)index;
 * \endcode
 *
 * \param sel Indicates the changes to be made to the listbox item, 
 *        can be one of the following values:
 *             - -1\n      If the item has been selected, makes it unselected, vice versa.
 *             - 0\n       Makes the item unselected. 
 *             - other\n   Makes the item selected. 
 * \param index The index of the item.
 *
 * \return LB_OKAY on success, else LB_ERR to indicate you passed an invalid index
 *         or the list box has no LBS_MULTIPLESEL style.
 */
#define LB_SETSEL               0xF185

/**
 * \def LB_GETCURSEL
 * \brief Gets the index of the currently selected or highlighted item.
 *
 * An application sends an LB_GETCURSEL message to a list box to get the index of 
 * the currently selected item, if there is one, in a single-selection list box.
 * For multiple-selection list box, appliction send an LB_GETCURSEL message to a 
 * list box to get the index of the current highlighted item.
 *
 * \code
 * LB_GETCURSEL
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The index of the currently selected item for single-selection list box;
 *         Eles the index of the highlighted item for multiple-selection list box.
 */
#define LB_GETCURSEL            0xF188

/**
 * \def LB_SETCURSEL
 * \brief Selects an item.
 *
 * An application sends an LB_SETCURSEL message to a list box to 
 * select an item and scroll it into view, if necessary.
 *
 * \code
 * LB_SETCURSEL
 * int cursel;
 *
 * wParam = (WPARAM)cursel;
 * lParam = 0;
 * \endcode
 *
 * \param cursel The index of the item to be selected and hilighted.
 *
 * \return The old index of the item selected on error, else LB_ERR to
 *         indicate an error occurred.
 */
#define LB_SETCURSEL            0xF186

/**
 * \def LB_GETTEXT
 * \brief Retrieves the text of an item in list box.
 *
 * An application sends an LB_GETTEXT message to a list box to retrieve the text
 * of an item.
 *
 * \code
 * LB_GETTEXT
 * int index;
 * char *string;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param index The index of the selected item.
 * \param string Pointer to the string buffer. The buffer should be large enough
 *        to contain the text of the item.
 *
 * \return One of the following values:
 *         - LB_OKAY\n  Success.
 *         - LB_ERR\n   Invalid item index.
 *
 * \sa LB_GETTEXTLEN
 */
#define LB_GETTEXT              0xF189

/**
 * \def LB_GETTEXTLEN
 * \brief Gets the length of text of item specified in a list box.
 *
 * An application sends an LB_GETTEXTLEN message to a list box to get the length 
 * of text of the item specified in the \a wParam parameter.
 *
 * \code
 * LB_GETTEXTLEN
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the specified item.
 *
 * \return The length of the strings on success, else LB_ERR to indicate invalid index.
 */
#define LB_GETTEXTLEN           0xF18A

/**
 * \def LB_GETCOUNT
 * \brief Gets the number of items in the list box.
 *
 * An application sends an LB_GETCOUNT message to a list box to get the number 
 * of items in the list box.
 *
 * \code
 * LB_GETCOUNT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The number of items in the listbox.
 */
#define LB_GETCOUNT             0xF18B

#define LB_SELECTSTRING         0xF18C
#define LB_DIR                  0xF18D

/**
 * \def LB_GETTOPINDEX
 * \brief Gets the index to the first visible item in the list box.
 *
 * An application sends an LB_GETTOPINDEX message to get the index to the first 
 * visible item in the list box. Initially, the first visible item is item 0, but 
 * this changes as the list box is scrolled. 
 *
 * \code
 * LB_GETTOPINDEX
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The index of the first visible item in the listbox.
 */
#define LB_GETTOPINDEX          0xF18E

/**
 * \def LB_FINDSTRING
 * \brief Searchs a specified string.
 *
 * An application sends an LB_FINDSTRING message to search a list box for an item 
 * that begins with the characters specified in the lParam parameter. The wParam 
 * parameter specifies the zero-based index of the item before the first item to 
 * be searched; The lParam parameter specifies a pointer to a null-terminated 
 * string that contains the prefix to search for.
 *
 * \code
 * LB_FINDSTRING
 * int index;
 * char *string;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param index The index of the item to be searched.
 * \param string The string of the item to be searched.
 *
 * \return The index of the matched item; LB_ERR for not found.
 */
#define LB_FINDSTRING           0xF18F

/**
 * \def LB_GETSELCOUNT
 * \brief Gets the number of selected items in a multiple-selection list box.
 *
 * An application sends an LB_GETSELCOUNT message to a list box to get the number 
 * of selected items in a multiple-selection list box.
 *
 * \code
 * LB_GETSELCOUNT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The number of selected items in the multiple-selection listbox.
 */
#define LB_GETSELCOUNT          0xF190

/**
 * \def LB_GETSELITEMS
 * \brief Gets the numbers of selected items.
 *
 * An application sends an LB_GETSELITEMS message to a list box to fill a buffer 
 * with an array of integers that specify the item numbers of selected items in 
 * a multiple-selection list box.
 *
 * \code
 * LB_GETSELITEMS
 * int nItem;
 * int *pInt;
 *
 * wParam = (WPARAM)nItem;
 * lParam = (LPARAM)pInt;
 * \endcode
 *
 * \param nItem The maximum integer numbers wanted.
 * \param pInt The buffer of an array of integers to save the indices of selected items.
 *
 * \return The number of selected items.
 */
#define LB_GETSELITEMS          0xF191

#define LB_SETTABSTOPS          0xF192
#define LB_GETHORIZONTALEXTENT  0xF193
#define LB_SETHORIZONTALEXTENT  0xF194
#define LB_SETCOLUMNWIDTH       0xF195
#define LB_ADDFILE              0xF196

/**
 * \def LB_SETTOPINDEX
 * \brief Ensures that a particular item in it is visible.
 *
 * An application sends an LB_SETTOPINDEX message to a list box to ensure that a 
 * particular item in it is visible. The item is specified in the wParam parameter. 
 * The list box scrolls so that either the specified item appears at the top of 
 * the list box or the maximum scroll range has been reached.
 * 
 * \code
 * LB_SETTOPINDEX
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the particular item to be set.
 *
 * \return Always be zero.
 */
#define LB_SETTOPINDEX          0xF197

/**
 * \def LB_GETITEMRECT
 * \brief Retrieves the dimensions of the rectangle.
 *
 * An application sends an LB_GETITEMRECT message to a list box to retrieve 
 * the dimensions of the rectangle that bounds an item as it is currently 
 * displayed in the list box window. The item is specified in the wParam 
 * parameter, and a pointer to a RECT structure is given in the lParam parameter.
 *
 * \code
 * LB_GETITEMRECT
 * int index;
 * RECT *rcItem;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)rcItem;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param rcItem Pointer to the buffer used for storing the item rect;
 *
 * \return LB_OKAY on success; LB_ERR on error.
 */
#define LB_GETITEMRECT          0xF198

/**
 * \def LB_GETITEMDATA
 * \brief Gets item data in a list box if the box has LBS_CHECKBOX
 *          and/or LBS_USEICON styles.
 * 
 * An application sends LB_GETITEMDATA message to a list box to retrive the
 * check box flag and the handle of icon. Note that the text of the item
 * will not be returned, i.e., the field of \a string of LISTBOXITEMINFO
 * structure will be ignored.
 *
 * \code
 * LB_GETITEMDATA
 * int index;
 * PLISTBOXITEMINFO plbii;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)plbii;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param plbii Pointer to the buffer used for storing the item data of the 
 *         specified item.
 *
 * \return LB_OKAY on success; LB_ERR on error.
 *
 * \sa LISTBOXITEMINFO
 */
#define LB_GETITEMDATA          0xF199

/**
 * \def LB_SETITEMDATA
 * \brief Sets item data in a list box if the box has LBS_CHECKBOX
 *          and/or LBS_USEICON styles.
 * 
 * An application sends LB_SETITEMDATA message to a list box to set the
 * check box flag and the handle of icon. Note that the text of the item
 * will not be changed, i.e., the field of \a string of LISTBOXITEMINFO
 * structure will be ignored.
 *
 * \code
 * LB_SETITEMDATA
 * int index;
 * PLISTBOXITEMINFO plbii;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)plbii;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param plbii Pointer to the buffer used for setting the item info of the 
 *         specified item.
 *
 * \return LB_OKAY on success; LB_ERR on error.
 *
 * \sa LISTBOXITEMINFO
 */
#define LB_SETITEMDATA          0xF19A

#define LB_SELITEMRANGE         0xF19B
#define LB_SETANCHORINDEX       0xF19C
#define LB_GETANCHORINDEX       0xF19D

/**
 * \def LB_SETCARETINDEX
 * \brief Sets the focus rectangle to the item at the specified index.
 * 
 * An application sends an LB_SETCARETINDEX message to set the focus rectangle 
 * to the item at the specified index in a multiple-selection list box.
 *
 * \code
 * LB_SETCARETINDEX
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 */
#define LB_SETCARETINDEX        0xF19E

/**
 * \def LB_GETCARETINDEX
 * \brief Determines the index of the item that has the focus rectangle.
 * 
 * An application sends an LB_GETCARETINDEX message to a list box to determine 
 * the index of the item that has the focus rectangle in a multiple-selection 
 * list box.
 *
 * \code
 * LB_GETCARETINDEX
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The index of the item that has the focus rectangle.
 */
#define LB_GETCARETINDEX        0xF19F

/**
 * \def LB_SETITEMHEIGHT
 * \brief Sets the height of all items.
 * 
 * An application sends an LB_SETITEMHEIGHT message to set the height of 
 * all items in a list box. 
 *
 * \code
 * LB_SETITEMHEIGHT
 * int itemHeight;
 *
 * wParam = 0;
 * lParam = (LPARAM)itemHeight;
 * \endcode
 *
 * \param itemHeight New height of item.
 *
 * \return The effective height of item.
 */
#define LB_SETITEMHEIGHT        0xF1A0

/**
 * \def LB_GETITEMHEIGHT
 * \brief Gets the height in pixels of an item specified in the wParam parameter.
 * 
 * An application sends an LB_GETITEMHEIGHT message to a list box to get the 
 * height in pixels of an item specified in the wParam parameter.
 *
 * \code
 * LB_GETITEMHEIGHT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The height of item in the listbox.
 */
#define LB_GETITEMHEIGHT        0xF1A1

/**
 * \def LB_FINDSTRINGEXACT
 * \brief Searchs for an item that exactly matches the characters specified.
 * 
 * An application sends an LB_FINDSTRINGEXACT message to a list box to search 
 * it for an item that exactly matches the characters specified in the lParam parameter.
 *
 * \code
 * LB_FINDSTRINGEXACT
 * int index;
 * const char *string;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param string The string of the item to be searched for.
 *
 * \return The index of the found item on success, else LB_ERR.
 */
#define LB_FINDSTRINGEXACT      0xF1A2

#define LB_SETLOCALE            0xF1A5
#define LB_GETLOCALE            0xF1A6
#define LB_SETCOUNT             0xF1A7
#define LB_INITSTORAGE          0xF1A8
#define LB_ITEMFROMPOINT        0xF1A9

/**
 * \def LB_SETTEXT
 * \brief Sets text of the specified item.
 *
 * \code
 * LB_SETTEXT
 * int index;
 * const char *string;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param string The string of the item to be set.
 *
 * \return One of the following values:
 *          - LB_OKAY\n Success
 *          - LB_ERR\n  Invalid item index or memory allocation error.
 */
#define LB_SETTEXT              0xF1AA

/**
 * \def LB_GETCHECKMARK
 * \brief Gets check mark status of an item.
 *
 * \code
 * LB_GETCHECKMARK
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the specified item.
 *
 * \return The check mark status of specified item on success; LB_ERR on error.
 *
 * \retval LB_ERR               Invalid item index or the list box have no LBS_CHECKBOX style.
 * \retval CMFLAG_CHECKED       The item is checked.
 * \retval CMFLAG_PARTCHECKED   The item is partly checked.
 * \retval CMFLAG_BLANK         The item is not checked.
 */
#define LB_GETCHECKMARK         0xF1AB

/**
 * \def LB_SETCHECKMARK
 * \brief Sets check mark status of an item.
 *
 * \code
 * LB_SETCHECKMARK
 * int index, status;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)status;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param status The new check mark status, can be one of the followings.
 *             - CMFLAG_CHECKED\n The item is checked.
 *             - CMFLAG_PARTCHECKED\n The item is partly checked.
 *
 * \return One of the following values:
 *          - LB_OKAY\n
 *              Success
 *          - LB_ERR\n
 *              Invalid item index or this list box have no LBS_CHECKBOX style.
 */
#define LB_SETCHECKMARK         0xF1AC

/**
 * \def LB_GETITEMADDDATA
 * \brief Gets the 32-bit data value associated with an item.
 * 
 * An application sends an LB_GETITEMADDDATA message to a list box to get the 
 * 32-bit data value the list box has stored for the item with index of 
 * \a wParam; By default this is zero. An application must set the 
 * item data value by sending an LB_SETITEMADDDATA message to the list box for 
 * this value to have meaning.
 *
 * \code
 * LB_GETITEMADDDATA
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the specified item.
 *
 * \return The 32-bit data value associated with an item on success, otherwise
 *         LB_ERR to indicate an error.
 */
#define LB_GETITEMADDDATA       0xF1AD

/**
 * \def LB_SETITEMADDDATA
 * \brief Associates a 32-bit data value with an item.
 *
 * An application sends an LB_SETITEMADDDATA message to associate a 32-bit data 
 * value specified in the \a lParam parameter with an item in the list box.
 *
 * \code
 * LB_SETITEMADDDATA
 * int index;
 * DWORD addData;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)addData;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param addData the 32-bit data value which will associated with the item.
 *
 * \return One of the following values:
 *          - LB_OKAY\n         Success
 *          - LB_ERR\n          Invalid item index
 */
#define LB_SETITEMADDDATA       0xF1AE

/**
 * \def LB_SETSTRCMPFUNC
 * \brief Sets the STRCMP function used to sort items.
 *
 * An application sends an LB_SETSTRCMPFUNC message to set a 
 * new STRCMP function to sort items.
 *
 * Note that you should send this message before adding 
 * any item to the list box control.
 *
 * \code
 * static int my_strcmp (const char* s1, const char* s2, size_t n)
 * {
 *      ...
 *      return 0;
 * }
 *
 * LB_SETSTRCMPFUNC
 *
 * wParam = 0;
 * lParam = (LPARAM)my_strcmp;
 * \endcode
 *
 * \param my_strcmp Your own function to compare two strings.
 *
 * \return One of the following values:
 *          - LB_OKAY\n         Success
 *          - LB_ERR\n          Not an empty list box.
 */
#define LB_SETSTRCMPFUNC        0xF1AF

#define LB_MSGMAX               0xF1B0

    /** @} end of ctrl_listbox_msgs */

    /**
     * \defgroup ctrl_listbox_ncs Notification codes of listbox control
     * @{
     */

/**
 * \def LBN_ERRSPACE
 * \brief Indicates that memory is not enough.
 * 
 * A list box sends an LBN_ERRSPACE notification message to its parent window 
 * when it cannot allocate enough memory to complete the current operation.
 */
#define LBN_ERRSPACE            255

/**
 * \def LBN_SELCHANGE
 * \brief Indicates change due to mouse or keyboard user input.
 * 
 * A list box created with the LBS_NOTIFY style sends an LBN_SELCHANGE 
 * notification message to its parent window when the selection is about to 
 * change due to mouse or keyboard user input.
 */
#define LBN_SELCHANGE           1

/**
 * \def LBN_DBLCLK
 * \brief Indicates double click on an item.
 * 
 * A list box created with the LBS_NOTIFY style sends an LBN_DBLCLK notification 
 * message to its parent window when the user double-clicks a string in it.
 */
#define LBN_DBLCLK              2

/**
 * \def LBN_SELCANCEL
 * \brief Indicates cancel of the selection in the list box.
 * 
 * A list box created with the LBS_NOTIFY style sends an LBN_SELCANCEL 
 * notification message to its parent window when the user cancels the selection 
 * in the list box.
 */
#define LBN_SELCANCEL           3

/**
 * \def LBN_SETFOCUS
 * \brief Indicates gain of input focus.
 * 
 * A list box sends an LBN_SETFOCUS notification message to its parent window 
 * when the list box gains the input focus.
 */
#define LBN_SETFOCUS            4

/**
 * \def LBN_KILLFOCUS
 * \brief Indicates loss of input focus.
 * 
 * A list box sends an LBN_KILLFOCUS notification message to its parent window 
 * when the list box loses the input focus.
 */
#define LBN_KILLFOCUS           5

/**
 * \def LBN_CLICKCHECKMARK
 * \brief Indicates click on the check mark.
 */
#define LBN_CLICKCHECKMARK      6

/**
 * \def LBN_CLICKED
 * \brief Indicates click on the string.
 */
#define LBN_CLICKED             7

/**
 * \def LBN_ENTER
 * \brief Indicates the user has pressed the ENTER key.
 */
#define LBN_ENTER               8

    /** @} end of ctrl_listbox_ncs */

    /** @} end of ctrl_listbox */

#endif /* _CTRL_LISTBOX */

/****** Property Sheet Control ***********************************************/
#ifdef _CTRL_PROPSHEET

    /**
     * \defgroup ctrl_propsheet PropertySheet control
     * @{
     */

/**
 * \def CTRL_PROPSHEET
 * \brief The class name of propsheet control.
 */
#define CTRL_PROPSHEET       ("propsheet")

    /**
     * \defgroup ctrl_propsheet_styles Styles of propertysheet control
     * @{
     */

/**
 * \def PSS_SIMPLE
 * \brief A simple property sheet control. All tabs of the control
 *        will have the same width.
 */
#define PSS_SIMPLE                0x0000L

/**
 * \def PSS_COMPACTTAB
 * \brief Compact tab style. The width of a tab is adaptive to the tab title.
 */
#define PSS_COMPACTTAB            0x0001L

    /** @} end of ctrl_propsheet_styles */

    /**
     * \defgroup ctrl_propsheet_msgs Messages of propertysheet control
     * @{
     */

#define PS_OKAY         0
#define PS_ERR          (-1)

/**
 * \def PSM_GETACTIVEPAGE
 * \brief Gets the handle of current active page.
 *
 * Sends this message to retreive the propsheet window's active page.
 *
 * \code
 * PSM_GETACTIVEPAGE
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The handle to the active page; HWND_INVALID if no such a page.
 */
#define PSM_GETACTIVEPAGE         0xF1C0L

/**
 * \def PSM_SETACTIVEINDEX
 * \brief Changes the active page by index.
 *
 * Sends this message to change the propsheet window's active page.
 *
 * \code
 * PSM_SETACTIVEINDEX
 * int page;
 *
 * wParam = (WPARAM)page;
 * lParam = 0;
 * \endcode
 *
 * \param page Index of the page to set.
 *
 * \return PS_OKAY on success, otherwise PS_ERR.
 */
#define PSM_SETACTIVEINDEX         0xF1C2L

/**
 * \def PSM_GETPAGE
 * \brief Gets the handle of a page by index.
 *
 * Sends this message to retreive the handle to a page by index.
 *
 * \code
 * PSM_GETPAGE
 * int index;
 *
 * wParam = index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the page.
 *
 * \return The handle to the page; HWND_INVALID if no such a page.
 */
#define PSM_GETPAGE               0xF1C3L

/**
 * \def PSM_GETACTIVEINDEX
 * \brief Gets the index of the current active page.
 *
 * Sends this message to retreive the index of the propsheet window's active page.
 *
 * \code
 * PSM_GETACTIVEINDEX
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The index number of the active page.
 */
#define PSM_GETACTIVEINDEX        0xF1C4L

/**
 * \def PSM_GETPAGEINDEX
 * \brief Gets the index of a page by handle.
 *
 * Sends this message to retreive the index to a page by handle.
 *
 * \code
 * PSM_GETPAGEINDEX
 * HWND hwnd;
 *
 * wParam = hwnd;
 * lParam = 0;
 * \endcode
 *
 * \param hwnd The handle of the page.
 *
 * \return The index of the page; PS_ERR if no such a page.
 */
#define PSM_GETPAGEINDEX          0xF1C5L

/**
 * \def PSM_GETPAGECOUNT
 * \brief Gets the number of pages of the propsheet.
 *
 * Sends this message to retreive the number of pages currently in the propsheet.
 *
 * \code
 * PSM_GETPAGECOUNT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The number of pages in the propsheet.
 */
#define PSM_GETPAGECOUNT          0xF1C6L

/**
 * \def PSM_GETTITLELENGTH
 * \brief Gets the length of a page title.
 *
 * Sends this message to retreive the title length of a page.
 *
 * \code
 * PSM_GETTITLELENGTH
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index number of the page in the propsheet.
 *
 * \return The length of the page in the propsheet; PS_ERR if no such a page.
 */
#define PSM_GETTITLELENGTH        0xF1C7L

/**
 * \def PSM_GETTITLE
 * \brief Gets a page title.
 *
 * Sends this message to retreive the title of a page.
 *
 * \code
 * PSM_GETTITLE
 * int index;
 * char *buffer;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)buffer;
 * \endcode
 *
 * \param index The index number of the page in the propsheet.
 * \param buffer The buffer storing the title string.
 *
 * \return PS_OKAY on success; PS_ERR if no such a page.
 */
#define PSM_GETTITLE              0xF1C8L

/**
 * \def PSM_SETTITLE
 * \brief Sets a page title.
 *
 * Sends this message to specify the title of a page.
 *
 * \code
 * PSM_SETTITLE
 * int index;
 * char *buffer;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)buffer;
 * \endcode
 *
 * \param index The index number of the page in the propsheet.
 * \param buffer The string buffer storing the title.
 *
 * \return PS_OKAY on success; PS_ERR if no such a page.
 */
#define PSM_SETTITLE              0xF1C9L

/**
 * \def PSM_ADDPAGE
 * \brief Adds a page to the propsheet.
 *
 * Sends this message to add a page to the propsheet.
 *
 * \code
 * PSM_ADDPAGE
 * DLGTEMPLATE *dlg_tmpl;
 * WNDPROC proc;
 *
 * wParam = (WPARAM)dlg_tmpl;
 * lParam = (LPARAM)proc;
 * \endcode
 *
 * \param hdlg The handle of the page window to be added in the propsheet.
 * \param proc The window callback procedure of the page window.
 *        Note that the procedure should call DefaultPageProc function
 *        by default.
 *
 * \return The index of the page added on success; PS_ERR on error.
 */
#define PSM_ADDPAGE               0xF1D0L

/**
 * \def PSM_REMOVEPAGE
 * \brief Removes a page from the propsheet.
 *
 * Sends this message to remove a page from the propsheet and destroys the 
 * associated controls.
 *
 * \code
 * PSM_REMOVEPAGE
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index number of the page to be removed from the propsheet.
 *
 * \return If success, return PS_OKAY, otherwise PS_ERR.
 */
#define PSM_REMOVEPAGE            0xF1D1L

/**
 * \def PSM_SHEETCMD
 * \brief Sends a MSG_SHEETCMD message to all pages in the propsheet.
 *
 * If you send MSG_SHEETCMD message to the propsheet control, the control
 * will broadcast the message to all pages it contains. The page callback 
 * procedure will receive the message and handle it. If one page return non-zero
 * value, the broadcast will be broken and the message will return a value 
 * indicating which page returned error. The value will be equal to the page index 
 * plus one.
 *
 * The PSM_SHEETCMD can be used by property sheet window, i.e., the container
 * of the property pages. The sheet can create three buttons, like
 * "Ok", "Cancel", and "Apply". When the user clicked the "Apply" or "Ok"
 * button, it can send a PSM_SHEETCMD message to the propsheet control, the
 * control will then send the message to all pages to notify pages to apply
 * the changes made by the user. If there are some errors, the page can return 
 * a non-zero value to indicate an invalid chage so that the sheet can stop
 * to close the sheet window. You can tell the pages which action should
 * be taken by passing a value through the WPARAM parameter of the message.
 * 
 * \code
 * PSM_SHEETCMD
 * WPARAM wParam;
 * LPARAM lParam;
 *
 * wParam = (WPARAM)wParam;
 * lParam = (LPARAM)lParam;
 * \endcode
 *
 * \param wParam The WPARAM parameter of the MSG_SHEETCMD message.
 * \param lParam The LPARAM parameter of the MSG_SHEETCMD message.
 *
 * \return The message has been broken by a page, the value will be
 *         (page_index + 1); Zero indicates no page asserts an error.
 */
#define PSM_SHEETCMD              0xF1D2L

#define PSM_MSGMAX              0xF1E0L

    /** @} end of ctrl_propsheet_msgs */

    /**
     * \defgroup ctrl_propsheet_ncs Notification codes of propertysheet control
     * @{
     */

/**
 * \def PSN_ACTIVE_CHANGED
 * \brief Notifies the parent window that the active page of 
 *        the propsheet has been changed.
 */
#define PSN_ACTIVE_CHANGED        0x01

    /** @} end of ctrl_propsheet_ncs */

int GUIAPI DefaultPageProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

    /** @} end of ctrl_propsheet */

#endif /* _CTRL_PROPSHEET*/

/****** Combo Box Control *****************************************************/
#ifdef _CTRL_COMBOBOX

    /**
     * \defgroup ctrl_combobox ComboBox control
     * @{
     */

/**
 * \def CTRL_COMBOBOX
 * \brief The class name of combobox control.
 */
#define CTRL_COMBOBOX       ("combobox")

    /**
     * \defgroup ctrl_combobox_styles Styles of combobox control
     * @{
     */

/**
 * \def CBS_SIMPLE
 * \brief Displays the list box at all times.
 *
 * The current selection in the list box is displayed in the edit control.
 *
 * \note The argument \a dwAddData of \a CreateWindowEx function should be the
 * expected height of the list box.
 *
 * \code
 * int listbox_height = 100;
 *
 * CreateWindowEx (CTRL_COMBOBOX, ..., listbox_height);
 * \endcode
 */
#define CBS_SIMPLE              0x0000L

/**
 * \def CBS_AUTOSPIN
 * \brief Creates AutoSpin box.
 *
 * AutoSpin box has an input field with a spin button. The input field
 * displays an integer, and you can click spin button to increase or
 * decrease the value of the integer.
 */
#define CBS_AUTOSPIN            0x0001L

/**
 * \def CBS_SPINLIST
 * \brief Creates SpinList box.
 *
 * SpinList box has an input field with a spin button. The input field
 * displays some text, and you can click spin button to change the text.
 * The candidate text string comes from the strings you add to the box.
 */
#define CBS_SPINLIST            0x0002L

/**
 * \def CBS_DROPDOWNLIST
 * \brief Specifies a drop-down list box in the selection filed.
 *
 * \note The argument \a dwAddData of \a CreateWindowEx function should be the
 * expected height of the list box.
 *
 * \code
 * int listbox_height = 100;
 *
 * CreateWindowEx (CTRL_COMBOBOX, ..., listbox_height));
 * \endcode
 */
#define CBS_DROPDOWNLIST        0x0003L

#define CBS_TYPEMASK            0x0007L

/**
 * \def CBS_NOTIFY
 * \brief Notifies the parent window.
 *
 * Causes the combo box to notify the parent window 
 * with a notification message.
 */
#define CBS_NOTIFY              0x0008L

#define CBS_OWNERDRAWFIXED      0x0010L
#define CBS_OWNERDRAWVARIABLE   0x0020L
#define CBS_AUTOHSCROLL         0x0040L
#define CBS_DISABLENOSCROLL     0x0080L

/**
 * \def CBS_SORT
 * \brief Automatically sorts strings entered in the list box.
 */
#define CBS_SORT                0x0100L

/**
 * \def CBS_AUTOLOOP
 * \brief Loops the value automatically if the type of combobox is CBS_AUTOSPIN.
 */
#define CBS_AUTOLOOP            0x0200L

/**
 * \def CBS_EDITNOBORDER
 * \brief The edit box has no border.
 */
#define CBS_EDITNOBORDER        0x0400L

/**
 * \def CBS_EDITBASELINE
 * \brief The edit box has base line.
 */
#define CBS_EDITBASELINE        0x0800L

/**
 * \def CBS_READONLY
 * \brief The edit field is read-only.
 */
#define CBS_READONLY            0x1000L

/**
 * \def CBS_UPPERCASE
 * \brief The edit field is uppercase.
 */
#define CBS_UPPERCASE           0x2000L

/**
 * \def CBS_LOWERCASE
 * \brief The edit field is lowercase.
 */
#define CBS_LOWERCASE           0x4000L

/**
 * \def CBS_AUTOFOCUS
 * \brief The edit field will gain the focus automatically.
 */
#define CBS_AUTOFOCUS           0x8000L

    /** @} end of ctrl_combobox_styles */

    /**
     * \defgroup ctrl_combobox_msgs Messages of combobox control
     * @{
     */

/**
 * \def CB_GETEDITSEL
 * \brief Gets the starting and ending character positions of the current 
 *        selection.
 *
 * An application sends a CB_GETEDITSEL message to get the starting and ending 
 * character positions of the current selection in the edit control of a combo box.
 *
 * \code
 * CB_GETEDITSEL
 * int start;
 * int end;
 *
 * wParam = (WPARAM)&start;
 * lParam = (LPARAM)&end;
 * \endcode
 *
 * \param start Pointer to a 32-bit value that receives the starting
 *           position of the selection.
 * \param end Pointer to a 32-bit value that receives the ending
 *           position of the selection.
 *
 * \note Not implemented yet.
 */
#define CB_GETEDITSEL               0xF140

/**
 * \def CB_LIMITTEXT
 * \brief Limits the length of text in the edit control.
 *
 * An application sends a CB_LIMITTEXT message to limit the length of the text
 * the user may type into the edit control of a combo box. 
 *
 * \code
 * CB_LIMITTEXT
 * int newLimit;
 *
 * wParam = (WPARAM)newLimit;
 * lParam = 0;
 * \endcode
 *
 * \param newLimit Specifies the maximum number of characters the user can enter.
 *
 * \return The return value is always zero.
 */
#define CB_LIMITTEXT                0xF141

/**
 * \def CB_SETEDITSEL
 * \brief Sets the starting and ending character positions of the current 
 *        selection.
 *
 * An application sends a CB_SETEDITSEL message to set the starting and ending 
 * character positions of the current selection in the edit control of a combo box.
 *
 * \code
 * CB_SETEDITSEL
 * int start;
 * int end;
 *
 * wParam = (WPARAM)start;
 * lParam = (LPARAM)end;
 * \endcode
 *
 * \param start The starting position of the selection.
 * \param end The ending position of the selection.
 *
 * \note Not implemented yet.
 */
#define CB_SETEDITSEL               0xF142

/**
 * \def CB_ADDSTRING
 * \brief Adds a string to the list box of a combo box.
 *
 * An application sends a CB_ADDSTRING message to add a string to the list box
 * of a combo box. 
 *
 * \code
 * CB_ADDSTRING
 * char* string;
 *
 * wParam = 0;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param string Pointer to the null-terminated string to be added.
 *
 * \return The index of the new item on success, else the one of
 *         the following error codes:
 *
 *         - CB_ERRSPACE    No memory can be allocated for new item.
 *         - CB_ERR         Invalid passed arguments.
 */
#define CB_ADDSTRING                0xF143

/**
 * \def CB_DELETESTRING
 * \brief Deletes a string in the list box of a combo box.
 *
 * An application sends a CB_DELETESTRING message to delete a string in the list box
 * of a combo box. 
 *
 * \code
 * CB_DELETESTRING
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index Specifies the index of the string to delete.
 *
 * \return If succeed, return zero; otherwise CB_ERR.
 */
#define CB_DELETESTRING             0xF144

#define CB_DIR                      0xF145

/**
 * \def CB_GETCOUNT
 * \brief Retreives the number of items in the list box of a combo box.
 *
 * An application sends a CB_GETCOUNT message to retreive the number of items 
 * in the list box of a combo box. 
 *
 * \code
 * CB_GETCOUNT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The number of items in the list box.
 */
#define CB_GETCOUNT                 0xF146

/**
 * \def CB_GETCURSEL
 * \brief Retreives the index of the currently selected item in the list box.
 *
 * An application sends a CB_GETCURSEL message to retreive the index of the 
 * currently selected item in the list box of a combo box. 
 *
 * \code
 * CB_GETCURSEL
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The index of currently selected item in the list box if have one;
 *         otherwise CB_ERR.
 */
#define CB_GETCURSEL                0xF147

/**
 * \def CB_GETLBTEXT
 * \brief Retreives the string of an item in the list box.
 *
 * An application sends a CB_GETLBTEXT message to retreive the string of 
 * a specified item in the list box of a combo box. 
 *
 * \code
 * CB_GETLBTEXT
 * int index;
 * char* string;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param index The index of the selected item.
 * \param string Pointer to the string buffer.
 *
 * \return One of the following values:
 *         - CB_OKAY\n  Success.
 *         - CB_ERR\n   Invalid item index.
 */
#define CB_GETLBTEXT                0xF148

/**
 * \def CB_GETLBTEXTLEN
 * \brief Gets the string length of an item in the list box.
 *
 * An application sends a CB_GETLBTEXTLEN message to get the string length
 * of a specified item in the list box of a combo box. 
 *
 * \code
 * CB_GETLBTEXTLEN
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the specified item.
 *
 * \return The length of the string on success; otherwise CB_ERR.
 */
#define CB_GETLBTEXTLEN             0xF149

/**
 * \def CB_INSERTSTRING
 * \brief Inserts a string to the list box of a combo box.
 *
 * An application sends a CB_INSERTSTRING message to insert a string to the list 
 * box of a combo box. Unlike the CB_ADDSTRING message, the CB_INSERTSTRING message
 * do not cause a list to be sorted.
 *
 * \code
 * CB_INSERTSTRING
 * int index;
 * char* string;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param index The index of the position at which to insert the string.
 * \param string Pointer to the null-terminated string to be added.
 *
 * \return The index of the new item on success, else the one of
 *         the following error codes:
 *
 *         - CB_ERRSPACE    No memory can be allocated for new item.
 *         - CB_ERR         Invalid passed arguments.
 */
#define CB_INSERTSTRING             0xF14A

/**
 * \def CB_RESETCONTENT
 * \brief Removes all items from the list box and edit control.
 *
 * An application sends a CB_RESETCONTENT message remove all items from the list
 * box and edit control of a combo box. 
 *
 * \code
 * CB_RESETCONTENT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return Always be zero.
 */
#define CB_RESETCONTENT             0xF14B

/**
 * \def CB_FINDSTRING
 * \brief Searchs the list box for an item beginning with the characters in a 
 *         specified string.
 *
 * An application sends a CB_FINDSTRING message to search the list box for an 
 * item beginning with the characters in a specified string.
 *
 * \code
 * CB_FINDSTRING
 * int indexStart;
 * char* string;
 *
 * wParam = (WPARAM)indexStart;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param indexStart Index of the item preceding the first item to be searched.
 * \param string Pointer to the null-terminated string that contains the prefix
 *             to search for.
 *
 * \return The index of the matching item or CB_ERR to indicate not found.
 */
#define CB_FINDSTRING               0xF14C

#define CB_SELECTSTRING             0xF14D

/**
 * \def CB_SETCURSEL
 * \brief Selects a string in the list of a combo box.
 *
 * \code
 * CB_SETCURLSEL
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the string to select. 
 *
 * \return CB_OKAY on success; otherwise CB_ERR to indicate an invalid index.
 */
#define CB_SETCURSEL                0xF14E

#define CB_SHOWDROPDOWN             0xF14F

/**
 * \def CB_GETITEMADDDATA
 * \brief Retreives the application-supplied 32-bit value associated with the 
 *         specified item.
 *
 * An application sends an CB_GETITEMADDDATA message to retrive the 32-bit data 
 * value associated with with an item in the list box of the combo box.
 *
 * \code
 * CB_GETITEMADDDATA
 * int index;
 *
 * wParam = (WPARAM)index;
 * lParam = 0;
 * \endcode
 *
 * \param index The index of the item.
 *
 * \return The 32-bit data value associated with an item on success, otherwise
 *         CB_ERR to indicate an error.
 */
#define CB_GETITEMADDDATA           0xF150

/**
 * \def CB_SETITEMADDDATA
 * \brief Sets a 32-bit data value with the specified item.
 * 
 * An application sends an CB_SETITEMADDDATA message to associate a 32-bit data 
 * value specified in the lParam parameter with an item in the list box that 
 * is specified in the wParam parameter.
 *
 * \code
 * CB_SETITEMADDDATA
 * int index;
 * DWORD addData;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)addData;
 * \endcode
 *
 * \param index The index of the specified item.
 * \param addData the 32-bit data value which will associated with the item.
 *
 * \return One of the following values:
 *          - CB_OKAY\n         Success
 *          - CB_ERR\n          Invalid item index
 */
#define CB_SETITEMADDDATA              0xF151

/**
 * \def CB_GETDROPPEDCONTROLRECT
 * \brief Retreives the screen coordinates of the dropdown list box of a combo box. 
 *
 * \code
 * CB_GETDROPPEDCONTROLRECT
 * RECT *rect;
 *
 * wParam = 0;
 * lParam = (LPARAM)rect;
 * \endcode
 *
 * \param rect Pointer to the RECT structure used to save the coordinates.
 */
#define CB_GETDROPPEDCONTROLRECT    0xF152

/**
 * \def CB_SETITEMHEIGHT
 * \brief Sets the height of all items of the list box in a combo box.
 * 
 * An application sends an CB_SETITEMHEIGHT message to set the height of 
 * all items of the list box in a combo box.
 *
 * \code
 * CB_SETITEMHEIGHT
 * int itemHeight;
 *
 * wParam = 0;
 * lParam = (LPARAM)itemHeight;
 * \endcode
 *
 * \param itemHeight New height of item of the list box.
 *
 * \return The effective height of item of the list box.
 */
#define CB_SETITEMHEIGHT            0xF153

/**
 * \def CB_GETITEMHEIGHT
 * \brief Gets the height of items of the list box in a combo box.
 *
 * \code
 * CB_GETITEMHEIGHT
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The height of item in the list box.
 */
#define CB_GETITEMHEIGHT            0xF154

#define CB_SETEXTENDEDUI            0xF155
#define CB_GETEXTENDEDUI            0xF156

/**
 * \def CB_GETDROPPEDSTATE
 * \brief Determines whether the list box of a combo box is dropped down.
 *
 * \code
 * CB_GETIDROPSTATE
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return If the list box is visible, the return value is TRUE; 
 *         otherwise, it is FALSE.
 */
#define CB_GETDROPPEDSTATE          0xF157

/**
 * \def CB_FINDSTRINGEXACT
 * \brief Searchs the list box for an item that matches the specified string. 
 *
 * An application sends a CB_FINDSTRINGEXACT message to search the list box for an 
 * item that matches a specified string.
 *
 * \code
 * CB_FINDSTRINGEXACT
 * int indexStart;
 * char* string;
 *
 * wParam = (WPARAM)indexStart;
 * lParam = (LPARAM)string;
 * \endcode
 *
 * \param indexStart Index of the item preceding the first item to be searched.
 * \param string Pointer to the null-terminated string to search for.
 *
 * \return The index of the matching item; otherwise CB_ERR to indicate not found.
 */
#define CB_FINDSTRINGEXACT          0xF158

#define CB_SETLOCALE                0xF159
#define CB_GETLOCALE                0xF15A
#define CB_GETTOPINDEX              0xF15b
#define CB_SETTOPINDEX              0xF15c
#define CB_GETHORIZONTALEXTENT      0xF15d
#define CB_SETHORIZONTALEXTENT      0xF15e
#define CB_GETDROPPEDWIDTH          0xF15f
#define CB_SETDROPPEDWIDTH          0xF160
#define CB_INITSTORAGE              0xF161

/**
 * \def CB_SETSPINFORMAT
 * \brief Sets the format string of value for CBS_AUTOSPIN type.
 *
 * \code
 * CB_SETSPINFORMAT
 * const char* format;
 *
 * wParam = 0;
 * lParam = (LPARAM)format;
 * \endcode
 *
 * \param format A format string can be used by \a sprintf function
 *        to format an interger.
 *
 * \return CB_OKAY on success; otherwise CB_ERR.
 */
#define CB_SETSPINFORMAT             0xF162

/**
 * \def CB_SETSPINRANGE
 * \brief Determines the range of the spin box in a combo box.
 *
 * \code
 * CB_SETSPINRANGE
 * int new_min;
 * int new_max;
 *
 * wParam = (WPARAM)new_min;
 * lParam = (LPARAM)new_max;
 * \endcode
 *
 * \param new_min The new minimum value of the spin box.
 * \param new_max The new maximum value of the spin box.
 *
 * \return CB_OKAY on success; otherwise CB_ERR to indicate invalid parameters.
 */
#define CB_SETSPINRANGE             0xF163

/**
 * \def CB_GETSPINRANGE
 * \brief Gets the range of the spin box in a combo box.
 *
 * \code
 * CB_GETSPINRANGE
 * int *spin_min;
 * int *spin_max;
 *
 * wParam = (WPARAM)spin_min;
 * lParam = (LPARAM)spin_max;
 * \endcode
 *
 * \param spin_min The minimum value of the spin box.
 * \param spin_max The maximum value of the spin box.
 *
 * \return Always be CB_OKAY.
 */
#define CB_GETSPINRANGE             0xF164

/**
 * \def CB_SETSPINVALUE
 * \brief Determines the value of the spin box in a combo box.
 *
 * \code
 * CB_SETSPINVALUE
 * int new_value;
 *
 * wParam = (WPARAM)new_value;
 * lParam = 0;
 * \endcode
 *
 * \param new_value The new value of the spin box.
 *
 * \return CB_OKAY on success; otherwise CB_ERR to indicate invalid parameters.
 */
#define CB_SETSPINVALUE             0xF165

/**
 * \def CB_GETSPINVALUE
 * \brief Gets the current value of the spin box in a combo box.
 *
 * \code
 * CB_GETSPINVALUE
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The current value of the spin box.
 */
#define CB_GETSPINVALUE             0xF166

/**
 * \def CB_SETSPINPACE
 * \brief Determines the pace and the fast pace of the spin box in a combo box.
 *
 * \code
 * CB_SETSPINPACE
 * int new_pace;
 * int new_fastpace;
 *
 * wParam = (WPARAM)new_pace;
 * lParam = (LPARAM)new_fastpace;
 * \endcode
 *
 * \param new_pace The new pace value of the spin box.
 * \param new_fastpace The new fast pace value of the spin box.
 *
 * \return Always be CB_OKAY.
 */
#define CB_SETSPINPACE              0xF167

/**
 * \def CB_GETSPINPACE
 * \brief Gets the pace and the fast pace of the spin box in a combo box.
 *
 * \code
 * CB_GETSPINPACE
 * int *spin_pace;
 * int *spin_fastpace;
 *
 * wParam = (WPARAM)spin_pace;
 * lParam = (LPARAM)spin_fastpace;
 * \endcode
 *
 * \param spin_pace Pointer to the data to retreive the new pace value of the spin box.
 * \param spin_fastpace Pointer to the data to retreive the new fast pace value of the spin box.
 *
 * \return Always be CB_OKAY.
 */
#define CB_GETSPINPACE              0xF168

/**
 * \def CB_SPIN
 * \brief Spins the value of the spin box or auto spin box.
 *
 * \code
 * CB_SPIN
 *
 * int direct;
 *
 * wParam = direct;
 * lParam = 0;
 * \endcode
 *
 * \param direct Indicats the direct of the spin. Zero means up, non-zero down.
 *
 * \return Always be CB_OKAY.
 */
#define CB_SPIN                     0xF170

/**
 * \def CB_FASTSPIN
 * \brief Fast spins the value of the spin box or auto spin box.
 *
 * \code
 * CB_FASTSPIN
 *
 * int direct
 *
 * wParam = direct;
 * lParam = 0;
 * \endcode
 *
 * \param direct Indicats the direct of the spin. Zero means up, non-zero down.
 *
 * \return Always be CB_OKAY.
 */
#define CB_FASTSPIN                 0xF171

/**
 * \def CB_SETSTRCMPFUNC
 * \brief Sets the STRCMP function used to sort items.
 *
 * An application sends a CB_SETSTRCMPFUNC message to set a 
 * new STRCMP function to sort items in the combo-box.
 *
 * Note that you should send this message before adding 
 * any item to the combo-box control.
 *
 * \code
 * static int my_strcmp (const char* s1, const char* s2, size_t n)
 * {
 *      ...
 *      return 0;
 * }
 *
 * CB_SETSTRCMPFUNC
 *
 * wParam = 0;
 * lParam = (LPARAM) my_strcmp;
 * \endcode
 *
 * \param my_strcmp Your own function to compare two strings.
 *
 * \return One of the following values:
 *          - CB_OKAY\n
 *              Success
 *          - CB_ERR\n
 *              This combobox has no list box or it is not an empty list box.
 */
#define CB_SETSTRCMPFUNC            0xF172

/**
 * \def CB_GETCHILDREN
 * \brief Gets the handles to the children of a ComboBox control.
 *
 * An application sends a CB_GETCHILDREN message to get the handles 
 * to the children of a ComboBox control.
 *
 * \code
 * CB_GETCHILDREN
 *
 * HWND *wnd_edit, *wnd_listbox;
 *
 * wParam = (WPARAM)wnd_edit;
 * lParam = (LPARAM)wnd_listbox;
 * \endcode
 *
 * \param wnd_edit The buffer saving the handle to the edit box of the ComboBox control.
 * \param wnd_list The buffer saving the handle to the list box of the ComboBox control.
 *        If the ComboBox have type of CBS_AUTOSPIN, handle to the list box will be 0.
 *
 * \return Always be CB_OKAY.
 */
#define CB_GETCHILDREN              0xF173

#define CB_MSGMAX                   0xF180

    /** @} end of ctrl_combobox_msgs */

/* Combo Box return value */
#define CB_OKAY                 LB_OKAY
#define CB_ERR                  LB_ERR
#define CB_ERRSPACE             LB_ERRSPACE

    /**
     * \defgroup ctrl_combobox_ncs Notification codes of combobox control
     * @{
     */

#define CBN_ERRSPACE            255

/**
 * \def CBN_SELCHANGE
 * \brief Notifies the change of the current selection.
 *
 * The CBN_SELCHANGE notification code is sent when the user changes the current
 * selection in the list box of a combo box.
 */
#define CBN_SELCHANGE           1

/**
 * \def CBN_DBLCLK
 * \brief Notifies the user has double clicked an item.
 *
 * A combo box created with the CBS_NOTIFY style sends an CBN_DBLCLK notification 
 * message to its parent window when the user double-clicks a string in its listbox.
 */
#define CBN_DBLCLK              2

/**
 * \def CBN_SETFOCUS
 * \brief Notifies the box has gained the input focus.
 */
#define CBN_SETFOCUS            3

/**
 * \def CBN_KILLFOCUS
 * \brief Notifies the box has lost the input focus.
 */
#define CBN_KILLFOCUS           4

/**
 * \def CBN_EDITCHANGE
 * \brief Notifies the change of the text in the edit control.
 *
 * The CBN_EDITCHANGE notification code is sent when the user has taken an action
 * that may have altered the text in the edit control portion of a combo box.
 */
#define CBN_EDITCHANGE          5

#define CBN_EDITUPDATE          6

/**
 * \def CBN_DROPDOWN
 * \brief Notifies the list box has been dropped down.
 */
#define CBN_DROPDOWN            7

/**
 * \def CBN_CLOSEUP
 * \brief Notifies the list box has been closed up.
 */
#define CBN_CLOSEUP             8

/**
 * \def CBN_SELENDOK
 * \brief Notifies the selection of a list item.
 *
 * The CBN_SELENDOK notification code is sent when the user has 
 * selected a list item.
 */
#define CBN_SELENDOK            9

/**
 * \def CBN_SELENDCANCEL
 * \brief Notifies that the selection of a list item is ignored.
 *
 * The CBN_SELENDCANCEL notification code is sent when the user has selected a list
 * item but then selects another control or closes the dialog box.
 */
#define CBN_SELENDCANCEL        10

    /** @} end of ctrl_combobox_ncs */

    /** @} end of ctrl_combobox */

#endif /* _CTRL_COMBOBOX */
  
/************************** Tool Bar control **********************************/
#ifdef _CTRL_TOOLBAR

    /**
     * \defgroup ctrl_toolbar ToolBar control
     *
     * \note The argument \a dwAddData of \a CreateWindowEx function should specify
     *       the height and the width of item on toolbar. 
     * \note This control is reserved only for compatibility, new applications 
     *       should use the new tool bar control instead (NewToolBar).
     *
     * \code
     * int item_height = 16;
     * int item_width = 16;
     *
     * CreateWindowEx (CTRL_TOOLBAR, ..., MAKELONG (item_height, item_width));
     * \endcode
     *
     * \sa ctrl_newtoolbar
     *
     * @{
     */

/**
 * \def CTRL_TOOLBAR
 * \brief The class name of toolbar control.
 */
#define CTRL_TOOLBAR            ("toolbar")

/** Structure of the toolbar item info */
typedef struct _TOOLBARITEMINFO
{
    /** Reserved, do not use. */
    int    insPos;

    /**
     * Identifier of the item. When the user clicked the item, this control
     * will send a notification message to the parent window
     * with the notification code to be equal to this identifier.
     */
    int    id;

    /** Path of the bitmap for normal state of the item */
    char   NBmpPath[MAX_PATH+10];
    /** Path of the bitmap for hilighted state of the item */
    char   HBmpPath[MAX_PATH+10];
    /** Path of the bitmap for down state of the item */
    char   DBmpPath[MAX_PATH+10];

    /** Additional data of the item */
    DWORD  dwAddData;
} TOOLBARITEMINFO;
/**
 * \var typedef TOOLBARITEMINFO* PTOOLBARITEMINFO;
 * \brief Data type of the pointer to a TOOLBARITEMINFO.
 */
typedef TOOLBARITEMINFO* PTOOLBARITEMINFO;

    /**
     * \defgroup ctrl_toolbar_msgs Messages of toolbar control
     *
     * @{
     */

/**
 * \def TBM_ADDITEM
 * \brief Adds an item to a toolbar.
 *
 * \code
 * TBM_ADDITEM
 * TOOLBARITEMINFO *ptbii;
 *
 * wParam = 0;
 * lParam = (LPARAM)ptbii;
 * \endcode
 *
 * \param ptbii Pointer to the data storing the toolbar item info.
 */
#define TBM_ADDITEM             0xFE00

#define TBM_MSGMAX              0xFE10

    /** @} end of ctrl_toolbar_msgs */

    /** @} end of ctrl_toolbar */

#endif /* _CTRL_TOOLBAR */

/************************** New Tool Bar control **********************************/
#ifdef _CTRL_NEWTOOLBAR

    /**
     * \defgroup ctrl_newtoolbar NewToolBar control
     *
     * \note You should pass information of the control through 
     * the argument \a dwAddData of \a CreateWindowEx function.
     *
     * \code
     * NTBINFO ntb_info;
     *
     * CreateWindowEx (CTRL_TOOLBAR, ..., (DWORD) &ntb_info);
     * \endcode
     *
     * \sa NTBINFO
     *
     * @{
     */

/**
 * \def CTRL_NEWTOOLBAR
 * \brief The class name of newtoolbar control.
 */
#define CTRL_NEWTOOLBAR            ("newtoolbar")

/** Structure of the newtoolbar control information */
typedef struct _NTBINFO
{
    /** Images for displaying tool bar item. */
    PBITMAP image;

    /** Number of bitmap cells in the image. */
    int nr_cells;

    /**
     * Number of cell columns in the image.
     * It should be one of the following values:
     *  - 0, 4\n
     *  Four rows, the cells have normal, highlight, pushed, and disabled states.
     *  - 1\n
     *  The cells have only normal state.
     *  - 2\n
     *  The cells have normal and highlight states.
     *  - 3\n
     *  The cells have normal, highlight, and pushed states.
     */
    int nr_cols;

    /**
     * Width of bitmap cell. If w_cell is zero, it will be 
     * equal to (width_of_image / nr_cols).
     */
    int w_cell;

    /**
     * Height of bitmap cell. If h_cell is zero, it will be 
     * equal to (height_of_image / nr_cells).
     */
    int h_cell;
} NTBINFO;

/**
 * \var typedef void (* HOTSPOTPROC)(HWND hwnd, int id, const RECT* cell, int x, int y)
 * \brief Type of the hotspot-clicked callback procedure.
 *
 * \param hwnd  The handle to the control.
 * \param id    The identifier of the button in the NewToolBar control.
 * \param cell  The rectangle of the button in screen coordinates.
 * \param x     The x-coordinate of the mouse down point.
 * \param y     THe y-coordinate of the mouse down point.
 */
typedef void (* HOTSPOTPROC)(HWND hwnd, int id, const RECT* cell, int x, int y);

#define NTB_TEXT_LEN        15
#define NTB_TIP_LEN         255

#define NTBIF_PUSHBUTTON    0x0001
#define NTBIF_HOTSPOTBUTTON  0x0002
#define NTBIF_SEPARATOR     0x0003
#define NTBIF_TYPEMASK      0x000F

#define NTBIF_DISABLED      0x0010

#define MTB_WHICH_FLAGS     0x0001
#define MTB_WHICH_ID        0x0002
#define MTB_WHICH_TEXT      0x0004
#define MTB_WHICH_TIP       0x0008
#define MTB_WHICH_CELL      0x0010
#define MTB_WHICH_HOTSPOT   0x0020
#define MTB_WHICH_ADDDATA   0x0040

/** Structure of the new toolbar item info */
typedef struct _NTBITEMINFO
{
    /**
     * Which fields are valid when sets/retrives the item information 
     * (ignored when add item). It can be OR'd with the following values:
     *
     *  - MTB_WHICH_FLAGS\n     The flags is valid.
     *  - MTB_WHICH_ID\n        The identifier is valid.
     *  - MTB_WHICH_TEXT\n      The text is valid.
     *  - MTB_WHICH_TIP\n       The tip text is valid.
     *  - MTB_WHICH_CELL\n      The bitmap cell index is valid.
     *  - MTB_WHICH_HOTSPOT\n   The hotspot rect is valid.
     *  - MTB_WHICH_ADDDATA\n   The additional data is valid.
     */
    DWORD       which;

    /**
     * Flags of the item. It can be OR'd with the following values:
     *
     *  - NTBIF_PUSHBUTTON\n    The item is a normal push button.
     *  - NTBIF_HOTSPOTBUTTON\n The item is a menu button with hotspot.
     *  - NTBIF_SEPARATOR\n     The item is a separator.
     *  - NTBIF_DISABLED\n      The item is disabled.
     */
    DWORD       flags;

    /**
     * Identifier of the item. When the user clicked the item, this control
     * will send a notification message to the parent window
     * with the notification code to be equal to this identifier.
     */
    int         id;

    /**
     * Text of the item. This text will be displayed under the bitmap
     * if the control have NTBS_WITHTEXT style.
     */
    char*       text;

    /** Tooltip of the item, not used, reserved. */
    char*       tip;

    /** Index of bitmap cell. */
    int         bmp_cell;

    /** Hotpot-clicked callback procedure for menubutton. */
    HOTSPOTPROC  hotspot_proc;

    /** Rectangle region of hotspot in the cell. */
    RECT        rc_hotspot;

    /** Additional data of the item */
    DWORD       add_data;
} NTBITEMINFO;

/**
 * \var PNTBITEMINFO
 * \brief Data type of pointer to a NTBITEMINFO;
 */
typedef NTBITEMINFO* PNTBITEMINFO;

    /**
     * \defgroup ctrl_newtoolbar_styles Styles of NewToolBar control
     * @{
     */

/**
 * \def NTBS_WITHTEXT 
 * \brief Displays text with the item bitmap. 
 */
#define NTBS_WITHTEXT               0x000000001L

/**
 * \def NTBS_TEXTRIGHT
 * \brief Displays text at the right side of the item bitmap. 
 */
#define NTBS_TEXTRIGHT              0x000000002L

/**
 * \def NTBS_DRAWSTATES
 * \brief Draws the button states with 3D frame, and does not
 *        use the highlight, pushed and disabled bitmap cell.
 */
#define NTBS_DRAWSTATES             0x000000004L

/**
 * \def NTBS_DRAWSEPARATOR
 * \brief Draws the separator bar.
 */
#define NTBS_DRAWSEPARATOR          0x000000008L

    /** @} end of ctrl_newtoolbar_styles */

    /**
     * \defgroup ctrl_newtoolbar_msgs Messages of NewToolBar control
     * @{
     */

#define NTB_OKAY        0
#define NTB_ERR         (-1)
#define NTB_ERR_SPACE   (-2)
#define NTB_ERR_DUPID   (-3)

/**
 * \def NTBM_ADDITEM
 * \brief Adds an item to a newtoolbar.
 *
 * \code
 * TBM_ADDITEM
 * NTBITEMINFO *ntbii;
 *
 * wParam = 0;
 * lParam = (LPARAM)ntbii;
 * \endcode
 *
 * \param ntbii Pointer to the data storing the newtoobar item info.
 *
 * \return NTB_OKAY on success, else one of the following values:
 *      - NTB_ERR_SPACE\n No enongh space to allocate memory for new item.
 *      - NTB_ERR_DUPID\n Duplicated identifier with an existed item.
 */
#define NTBM_ADDITEM             0xFE00

/**
 * \def NTBM_GETITEM
 * \brief Retrives the information of an item in a newtoolbar control.
 *
 * \code
 * TBM_GETITEM
 * int id;
 * NTBITEMINFO *ntbii;
 *
 * wParam = id;
 * lParam = (LPARAM)ntbii;
 * \endcode
 *
 * \param id The identifier of the item.
 * \param ntbii Pointer to the data storing the newtoobar item info.
 *
 * \return NTB_OKAY on success, else NTB_ERR.
 */
#define NTBM_GETITEM             0xFE01

/**
 * \def NTBM_SETITEM
 * \brief Sets the information of an item in a newtoolbar control.
 *
 * \code
 * TBM_SETITEM
 * int id;
 * NTBITEMINFO ntbii;
 *
 * wParam = id;
 * lParam = (LPARAM)&ntbii;
 * \endcode
 *
 * \param id The identifier of the item.
 * \param ntbii The structure for storing the newtoobar item info.
 *
 * \return NTB_OKAY on success, else NTB_ERR.
 */
#define NTBM_SETITEM             0xFE02

/**
 * \def NTBM_ENABLEITEM
 * \brief Enables/Disables an item in a newtoolbar control.
 *
 * \code
 * NTBM_ENABLEITEM
 * int id;
 * BOOL enable;
 *
 * wParam = id;
 * lParam = enable;
 * \endcode
 *
 * \param id The identifier of the item.
 * \param enable True to enable item; false to disable item.
 *
 * \return NTB_OKAY on success, else NTB_ERR.
 */
#define NTBM_ENABLEITEM         0xFE03

#define NTBM_MSGMAX             0xFE10

    /** @} end of ctrl_newtoolbar_msgs */

    /** @} end of ctrl_newtoolbar */

#endif /* _CTRL_NEWTOOLBAR */

/************************** Menu Button control ********************************/
#ifdef _CTRL_MENUBUTTON

    /**
     * \defgroup ctrl_menubutton MenuButton control
     * @{
     */

/**
 * \def CTRL_MENUBTN
 * \brief The class name of menubutton control.
 */
#define CTRL_MENUBTN        ("menubutton")

/**
 * \def CTRL_MENUBUTTON
 * \brief The class name of menubutton control.
 */
#define CTRL_MENUBUTTON     ("menubutton")

/* Menu Button return value */
#define MB_OKAY                 0
#define MB_ERR                  -1
#define MB_INV_ITEM             -2
#define MB_ERR_SPACE            -3

/* struct used by parent to add/retrive item */
#define MB_WHICH_TEXT           0x01
#define MB_WHICH_BMP            0x02
#define MB_WHICH_ATTDATA        0x04

/** Structure of the menubutton item */
typedef struct _MENUBUTTONITEM
{
    /**
     * Which fields are valid when sets/retrives the item information 
     * (ignored when add item). It can be OR'd with the following values:
     *
     *  - MB_WHICH_TEXT The \a text field is valid.
     *  - MB_WHICH_BMP The \a bmp field is valid.
     *  - MB_WHICH_ATTDATA The \a data field is valid.
     */
    DWORD           which;
    /** Item string */
    const char*     text;
    /** Item bitmap */
    PBITMAP         bmp;
    /** Attached data */
    DWORD           data;
} MENUBUTTONITEM;

/**
 * \var typedef MENUBUTTONITEM* PMENUBUTTONITEM;
 * \brief Data type of the pointer to a MENUBUTTONITEM.
 */
typedef MENUBUTTONITEM* PMENUBUTTONITEM;

    /**
     * \defgroup ctrl_menubutton_styles Styles of menubutton control
     * @{
     */

/**
 * \def MBS_SORT
 * \brief If this bit is set, the items listed in the control 
 * are displayed in a specified order.  
 */
#define MBS_SORT                0x0001

    /** @} end of ctrl_menubutton_styles */

    /**
     * \defgroup ctrl_menubutton_msgs Messages of menubutton control
     * @{
     */

/**
 * \def MBM_ADDITEM
 * \brief Sends to the control to add an item to the menu list.
 *
 * \code
 * MBM_ADDITEM
 * int pos;
 * MENUBUTTONITEM newitem;
 *
 * wParam = (WPARAM)pos;
 * lParam = (LPARAM)&newitem;
 * \endcode
 *
 * \param pos The position at which to add the item. If the control
 *        was created with the style of \a MBS_SORT, this parameter
 *        will be ignored. If this parameter is less than 0, 
 *        the new item will be append to the tail of the menu list.
 * \param newitem Pointer to the menubutton item info structure.
 *
 * \return The position at which the item has been added, i.e.,
 *         the index of the added item if success. Otherwise, 
 *         the following error code will be returned:
 *
 *        - MB_ERR_SPACE\n  No memory can be allocated for new item.
 */
#define MBM_ADDITEM                 0xF200

/**
 * \def MBM_DELITEM
 * \brief Sends to the control to delete an item in the menu list.
 *
 * \code
 * MBM_DELETEITEM
 * int delete;
 *
 * wParam = (WPARAM)delete;
 * lParam = 0;
 * \endcode
 *
 * \param delete The index of the item to be deleted.
 *
 * \return MB_OKAY if success, else MB_INV_ITEM to indicate valid index.
 */
#define MBM_DELITEM                 0xF201

/**
 * \def MBM_RESETCTRL
 * \brief Sends to the control to remove all items in the menu list.
 *
 * \code
 * MBM_RESETCTRL
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return Always be zero.
 */
#define MBM_RESETCTRL               0xF202

/**
 * \def MBM_SETITEMDATA
 * \brief Sends to the control to set the data of a specific item.
 *
 * \code
 * MBM_SETITEMDATA
 * int index;
 * PMENUBUTTONITEM pmbi;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)pmbi;
 * \endcode
 *
 * \param index The index of the item to be altered.
 * \param pmbi Pointer to the MENUBUTTONITEM structure that stores the new 
 *         menubutton item data.
 *
 * \return MB_OKAY if success, otherwise will be one of the following error codes:
 *
 *          - MB_INV_ITEM\n     Indicate that the index you passed is valid.
 *          - MB_ERR_SPACE\n    No memory can be allocated for new item data.
 * \return 
 */
#define MBM_SETITEMDATA             0xF203

/**
 * \def MBM_GETITEMDATA
 * \brief Sends to the control to retrive the data of a specific item.
 *
 * \code
 * MBM_GETITEMDATA
 * int index;
 * PMENUBUTTONITEM pmbi;
 *
 * wParam = (WPARAM)index;
 * lParam = (LPARAM)pmbi;
 * \endcode
 *
 * \param index The index of the specific item.
 * \param pmbi Pointer to the MENUBUTTONITEM structure for storing the 
 *         menubutton item data.
 *
 * \return MB_OKAY if success, otherwise MB_INV_ITEM to indicate invalid index.
 */
#define MBM_GETITEMDATA             0xF204

/**
 * \def MBM_GETCURITEM
 * \brief Sends to get the index of the current selected item.
 *
 * \code
 * MBM_GETCURITEM
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The index of the current selected item. If there is no seleted
 *         item, MB_INV_ITEM will be returned.
 */
#define MBM_GETCURITEM              0xF206

/**
 * \def MBM_SETCURITEM
 * \brief Sends to Set the the current selected item based on index.
 *
 * \code
 * MBM_SETCURITEM
 * int new;
 *
 * wParam = (WPARAM)new;
 * lParam = 0;
 * \endcode
 *
 * \param new The index to be the current item.
 *
 * \return The index of the old selected item.
 */
#define MBM_SETCURITEM              0xF207

/**
 * \def MBM_SETSTRCMPFUNC
 * \brief Sets the STRCMP function used to sort items.
 *
 * An application sends a MBM_SETSTRCMPFUNC message to set a 
 * new STRCMP function to sort items in the menubutton.
 *
 * Note that you should send this message before adding 
 * any item to the menubutton control.
 *
 * \code
 * static int my_strcmp (const char* s1, const char* s2, size_t n)
 * {
 *      ...
 *      return 0;
 * }
 *
 * MBM_SETSTRCMPFUNC
 *
 * wParam = 0;
 * lParam = (LPARAM) my_strcmp;
 * \endcode
 *
 * \param my_strcmp Your own function to compare two strings.
 *
 * \return One of the following values:
 *          - MB_OKAY\n     Success
 *          - MB_ERR\n      Not an empty menubutton
 */
#define MBM_SETSTRCMPFUNC           0xF208

#define MBM_MSGMAX                  0xF210

    /** @} end of ctrl_menubutton_msgs */

    /**
     * \defgroup ctrl_menubutton_ncs Notification codes of menubutton control
     * @{
     */

/**
 * \def MBN_ERRSPACE
 * \brief Sends when memory space error occures.
 */
#define MBN_ERRSPACE            255

/**
 * \def MBN_CHANGED
 * \brief Sends when selected item changes.
 */
#define MBN_CHANGED             1

/**
 * \def MBN_SELECTED
 * \brief Sends when an item is selected.
 */
#define MBN_SELECTED            2

/**
 * \def MBN_STARTMENU
 * \brief Sends when starting tracking popup menu.
 */
#define MBN_STARTMENU           4

/**
 * \def MBN_ENDMENU
 * \brief Sends when ending tracking popup menu.
 */
#define MBN_ENDMENU             5

    /** @} end of ctrl_menubutton_ncs */

    /** @} end of ctrl_menubutton */

#endif /* _CTRL_MENUBUTTON */

/************************** Scroll Bar Control *********************************/
#ifdef _CTRL_SCROLLBAR

    /**
     * \defgroup ctrl_scrollbar ScrollBar control
     *
     * \note The scrollbar is not implemented as a control in current version.
     * @{
     */

/**
 * \def CTRL_SCROLLBAR
 * \brief The class name of scrollbar control.
 */
#define CTRL_SCROLLBAR              ("scrollbar")

    /**
     * \defgroup ctrl_scrollbar_msgs Messages of scrollbar control
     * @{
     */

#define SBM_SETPOS                  0xF0E0
#define SBM_GETPOS                  0xF0E1
#define SBM_SETRANGE                0xF0E2
#define SBM_SETRANGEREDRAW          0xF0E6
#define SBM_GETRANGE                0xF0E3
#define SBM_ENABLE_ARROWS           0xF0E4
#define SBM_SETSCROLLINFO           0xF0E9
#define SBM_GETSCROLLINFO           0xF0EA

#define SBM_MSGMAX                  0xF0F0

    /** @} end of ctrl_scrollbar_msgs */

    /** @} end of ctrl_scrollbar */

#endif /* _CTRL_SCROLLBAR */

/************************** TrackBar Control ***********************************/
#ifdef _CTRL_TRACKBAR

    /**
     * \defgroup ctrl_trackbar TrackBar control
     * @{
     */

/**
 * \def CTRL_TRACKBAR
 * \brief The class name of trackbar control.
 */
#define CTRL_TRACKBAR              ("trackbar")

/**
 * \def TBLEN_TIP
 * \brief The maximum length of the trackbar tip string.
 */
#define TBLEN_TIP                   31

    /**
     * \defgroup ctrl_trackbar_styles Styles of trackbar control
     * @{
     */

/**
 * \def TBS_NOTIFY
 * \brief Causes the trackbar to notify the parent window with a notification message 
 *         when the user clicks or doubleclicks the trackbar.
 */
#define TBS_NOTIFY                 0x0001L

/**
 * \def TBS_VERTICAL
 * \brief The trackbar control will be oriented vertically.
 */
#define TBS_VERTICAL               0x0002L

/**
 * \def TBS_BORDER
 * \brief The trackbar control will have border.
 */
#define TBS_BORDER                 0x0004L

/**
 * \def TBS_TIP
 * \brief The trackbar control will display tip string above the control.
 */
#define TBS_TIP                    0x0008L

/**
 * \def TBS_NOTICK
 * \brief The trackbar control will not display tick line beside the control.
 */
#define TBS_NOTICK                 0x0010L

/* internally used style */
#define TBS_FOCUS                  0x1000L
#define TBS_DRAGGED                0x2000L

    /** @} end of ctrl_trackbar_styles */

    /**
     * \defgroup ctrl_trackbar_msgs Messages of trackbar control
     * @{
     */

/**
 * \def TBM_SETRANGE
 * \brief Sets the range of minimum and maximum logical positions for the 
 *         slider in a trackbar.
 *
 * \code
 * TBM_SETRANGE
 * int min;
 * int max;
 *
 * wParam = (WPARAM)min;
 * lParam = (LPARAM)max;
 * \endcode
 *
 * \param min Minimum position for the slider.
 * \param max Maximum position for the slider.
 *
 * \return Zero on success; otherwise -1.
 */
#define TBM_SETRANGE               0xF090

/**
 * \def TBM_GETMIN
 * \brief Gets the minimum logical position for the slider.
 *
 * \code
 * TBM_GETMIN
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The minimum logical position for the slider.
 */
#define TBM_GETMIN                 0xF091

/**
 * \def TBM_SETPOS
 * \brief Sets the current logical position of the slider.
 *
 * \code
 * TBM_SETPOS
 * int pos;
 *
 * wParam = (WPARAM)pos;
 * lParam = 0;
 * \endcode
 *
 * \param pos New logical position of the slider.
 *
 * \return Always be zero.
 */
#define TBM_SETPOS                 0xF092

/**
 * \def TBM_GETPOS
 * \brief Gets the current logical position of the slider.
 *
 * \code
 * TBM_GETPOS
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The current logical position of the slider.
 */
#define TBM_GETPOS                 0xF093

/**
 * \def TBM_SETLINESIZE
 * \brief Sets the number of logical positions moved in response to keyboard
 *     input from the arrow keys.
 *
 * Sets the number of logical positions the trackbar's slider moves in response
 * to keyboard input from the arrow keys. The logical positions are the integer
 * increments in the trackbar's range of minimum to maximum slider positions.
 *
 * \code
 * TBM_SETLINESIZE
 * int linesize;
 *
 * wParam = (WPARAM)linesize;
 * lParam = 0;
 * \endcode
 *
 * \param linesize New line size.
 *
 * \return Zero on success; otherwise -1.
 */
#define TBM_SETLINESIZE            0xF094

/**
 * \def TBM_GETLINESIZE
 * \brief Gets the number of logical positions moved in response to keyboard
 *     input from the arrow keys.
 *
 * Gets the number of logical positions the trackbar's slider moves in response
 * to keyboard input from the arrow keys. The logical positions are the integer
 * increments in the trackbar's range of minimum to maximum slider positions.
 *
 * \code
 * TBM_GETLINESIZE
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The current line size.
 */
#define TBM_GETLINESIZE            0xF095

/**
 * \def TBM_SETPAGESIZE
 * \brief Sets the number of logical positions moved in response to keyboard
 *         input from page keys..
 *
 * Sets the number of logical positions the trackbar's slider moves in response
 * to keyboard input form page keys, such as PAGE DOWN or PAGE UP keys. The 
 * logical positions are the integer increments in the trackbar's range of 
 * minimum to maximum slider positions.
 *
 * \code
 * TBM_SETPAGESIZE
 * int pagesize;
 *
 * wParam = (WPARAM)pagesize;
 * lParam = 0;
 * \endcode
 *
 * \param pagesize New page size.
 *
 * \return Zero on success; otherwise -1.
 */
#define TBM_SETPAGESIZE            0xF096

/**
 * \def TBM_GETPAGESIZE
 * \brief Gets the number of logical positions moved in response to keyboard
 *        input from page keys..
 *
 * Gets the number of logical positions the trackbar's slider moves in response
 * to keyboard input form page keys, such as PAGE DOWN or PAGE UP keys. The 
 * logical positions are the integer increments in the trackbar's range of 
 * minimum to maximum slider positions.
 *
 * \code
 * TBM_GETPAGESIZE
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The current page size.
 */
#define TBM_GETPAGESIZE            0xF097

/**
 * \def TBM_SETTIP
 * \brief Sets the start and end tip strings.
 *
 * \code
 * TBM_SETTIP
 * char* starttip;
 * char* endtip;
 *
 * wParam = (WPARAM)starttip;
 * lParam = (LPARAM)endtip;
 * \endcode
 *
 * \param starttip New start tip string.
 * \param endtip New end tip tip string.
 *
 * \return Always be zero.
 */
#define TBM_SETTIP            0xF098

/**
 * \def TBM_GETTIP
 * \brief Gets the start and end tip strings.
 *
 * \code
 * TBM_GETTIP
 * char starttip [TBLEN_TIP + 1];
 * char endtip [TBLEN_TIP + 1];
 *
 * wParam = (WPARAM)starttip;
 * lParam = (LPARAM)endtip;
 * \endcode
 *
 * \param starttip Buffer receives the start tip string. It should
 *        be length enough to save (TBLEN_TIP + 1) characters.
 * \param endtip Buffer receives the end tip string. It should
 *        be length enough to save (TBLEN_TIP + 1) characters.
 *
 * \return Always be zero.
 */
#define TBM_GETTIP                 0xF09A

/**
 * \def TBM_SETTICKFREQ
 * \brief Sets the interval frequency for tick marks in a trackbar.
 *
 * \code
 * TBM_SETTICKFREQ
 * int tickfreq;
 *
 * wParam = (WPARAM)tickfreq;
 * lParam = 0;
 * \endcode
 *
 * \param tickfreq New interval frequency for tick marks in a trackbar;
 *
 * \return Zero on success; otherwise -1.
 */
#define TBM_SETTICKFREQ            0xF09B

/**
 * \def TBM_GETTICKFREQ
 * \brief Gets the interval frequency for tick marks in a trackbar.
 *
 * \code
 * TBM_GETTICKFREQ
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The current interval frequency for tick marks in a trackbar;
 */
#define TBM_GETTICKFREQ            0xF09C

/**
 * \def TBM_SETMIN
 * \brief Sets the minimum logical position for the slider in a trackbar.
 *
 * \code
 * TBM_SETMIN
 * int min;
 *
 * wParam = (WPARAM)min;
 * lParam = 0;
 * \endcode
 *
 * \param min The new minimum logical position for the slider in a trackbar.
 *
 * \return Zero on success; otherwise -1.
 */
#define TBM_SETMIN                 0xF09D

/**
 * \def TBM_SETMAX
 * \brief Sets the maximum logical position for the slider in a trackbar.
 *
 * \code
 * TBM_SETMAX
 * int max;
 *
 * wParam = (WPARAM)max;
 * lParam = 0;
 * \endcode
 *
 * \param min The new maximum logical position for the slider in a trackbar.
 *
 * \return Zero on success; otherwise -1.
 */
#define TBM_SETMAX                 0xF09E

/**
 * \def TBM_GETMAX
 * \brief Gets the maximum logical position for the slider in a trackbar.
 *
 * \code
 * TBM_GETMAX
 *
 * wParam = 0;
 * lParam = 0;
 * \endcode
 *
 * \return The current maximum logical position for the slider in a trackbar.
 */
#define TBM_GETMAX                 0xF09F

#define TBR_MSGMAX                 0xF010

    /** @} end of ctrl_trackbar_msgs */

    /**
     * \defgroup ctrl_trackbar_ncs Notification codes of trackbar control
     * @{
     */

/**
 * \def TBN_REACHMIN
 * \brief Notifies that the slider has reached the minimum position.
 */
#define TBN_REACHMIN                1

/**
 * \def TBN_REACHMAX
 * \brief Notifies that the slider has reached the maximum position.
 */
#define TBN_REACHMAX                2

/**
 * \def TBN_CHANGE
 * \brief Notifies that the position of the slider has changed.
 */
#define TBN_CHANGE                  3

    /** @} end of ctrl_trackbar_ncs */

    /** @} end of ctrl_trackbar */

#endif /* _CTRL_TRACKBAR */

    /** @} end of controls */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _MGUI_CONTROL_H */

