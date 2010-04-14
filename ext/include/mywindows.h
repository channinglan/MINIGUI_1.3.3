/**
 * \file mywindows.h
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 2002/01/06
 * 
 *  This file includes interfaces for MyWins module of MiniGUIExt library, 
 *  which provides some useful and convenient user interface functions.
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
 * $Id: mywindows.h,v 1.3 2003/09/04 06:12:04 weiym Exp $
 *
 *             MiniGUI for Linux Version 1.3.x
 *             Copyright (C) 2002 ~ 2003 Feynman Software.
 *             Copyright (C) 1998 ~ 2002 Wei Yongming.
 */

#ifndef _MGUI_MYWINDOWS_H
    #define _MGUI_MYWINDOWS_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

    /**
     * \addtogroup mgext_fns Interfaces of the MiniGUI extension library (libmgext)
     * @{
     */

    /**
     * \addtogroup mywins_fns Interfaces of MyWins module of MiniGUIExt library
     * @{
     */

    /**
     * \defgroup mywins_helpers Useful helpers
     *
     * This module defines some useful helpers, such as tool-tip window,
     * progress window, and so on.
     *
     * @{
     */

/**
 * \fn int myMessageBox (HWND hwnd, DWORD dwStyle, char* title, char* text, ...)
 * \brief Creates a message box.
 *
 * This function creates a message box calling \a MessageBox and passing
 * \a hwnd, \a dwStyle and \a title to it. This function also receives \a printf-like
 * arguments to format a string.
 *
 * \param hwnd The hosting main window.
 * \param dwStyle The style of the message box.
 * \param title The title of the message box.
 * \param text The format string.
 * \return Identifier of the button which close the message box.
 *
 * \sa MessageBox, printf(3)
 */
int myMessageBox (HWND hwnd, DWORD dwStyle, char* title, char* text, ...);

/**
 * \fn int myWinMessage (HWND hwnd, char* title, char* button1, char* text, ...)
 * \brief Creates a message box within only one button.
 *
 * This function creates a message box hosted to the main window \a hwnd,
 * displays a message and an application icon in the message box, and creates
 * a button which can be used to close the box. This function also 
 * receives \a printf-like arguments to format a string.
 *
 * This function can be used to display a information for the user.
 *
 * \param hwnd The hosting main window.
 * \param title The title of the message box.
 * \param button1 The text in the button.
 * \param text The format string.
 * \return 0: indicates the message box was closed by click the only button.
 *
 * \sa myWinChoice, printf(3)
 */
int myWinMessage (HWND hwnd, char* title, char* button1, char* text, ...);

/**
 * \fn int myWinChoice (HWND hwnd, char* title, char* button1, char* button2, char* text, ...)
 * \brief Creates a message box within two buttons.
 *
 * This function creates a message box hosted to the main window \a hwnd, 
 * displays a message and an application icon in the message box, and creates
 * two buttons in it. This function also receives \a printf-like arguments 
 * to format a string.
 *
 * This function can be used to prompt the user to choose one item between two items.
 *
 * \param hwnd The hosting main window.
 * \param title The title of the message box.
 * \param button1 The title of the first button.
 * \param button2 The title of the second button.
 * \param text The format string.
 * \return Either 0 or 1, indicates the message box was closed by 
 *         the first or second button.
 *
 * \sa myWinTernary, printf(3)
 */
int myWinChoice (HWND hwnd, char* title, char* button1, char* button2,
                       char* text, ...);

/**
 * \fn int myWinTernary (HWND hwnd, char* title, char* button1, char* button2, char* button3, char* text, ...)
 * \brief Creates a message box within three buttons.
 *
 * This function creates a message box hosted to the main window \a hwnd, 
 * displays a message and an application icon in the message box, and creates
 * three buttons in it. This function also receives \a printf-like arguments 
 * to format a string.
 *
 * This function can be used to prompt the user to choose one item among three choices.
 *
 * \param hwnd The hosting main window.
 * \param title The title of the message box.
 * \param button1 The title of the first button.
 * \param button2 The title of the second button.
 * \param button3 The title of the third button.
 * \param text The format string.
 * \return 0, 1, or 2, indicates the message box was closed by 
 *         the first, the second or the third button.
 *
 * \sa myWinChoice, printf(3)
 */
int myWinTernary (HWND hwnd, char* title, char* button1, char* button2,
                       char* button3, char* text, ...);

/**
 * \fn void errorWindow (HWND hwnd, char * str, char* title)
 * \brief A MiniGUI edition of \a perror.
 *
 * This function creates a message box by using \a myMessageBox, and display the
 * current system error message. You can consider it as an alternative of \a perror.
 *
 * \param hwnd The hosting main window.
 * \param str The string will be appeared before the system error message.
 * \param title The title of the message box.
 *
 * \sa myMessageBox, perror(3)
 */
void errorWindow (HWND hwnd, char * str, char* title);

/**
 * \fn HWND createStatusWin (HWND hParentWnd, int width, int height, char * title, char * text, ...)
 * \brief Creates a status main window.
 *
 * This function creates a status main window and returns the handle to it.
 * You can call \a destroyStatusWin to destroy it. This function also 
 * receives \a printf-like arguments to format a string.
 *
 * \param hParentWnd The hosting main window.
 * \param width The width of the status window. 
 * \param height The height of the status window. 
 * \param title The title of the status window.
 * \param text The format string.
 * \return The handle to the status window on success, HWND_INVALID on error.
 *
 * \sa destroyStatusWin
 */
HWND createStatusWin (HWND hParentWnd, int width, int height, 
                char * title, char * text, ...);

/**
 * \fn void destroyStatusWin (HWND hwnd)
 * \brief Destroies a status window.
 *
 * This function destroies the specified status window \a hwnd, which 
 * is returned by \a createStatusWin.
 *
 * \param hwnd The handle to the status window.
 * 
 * \sa createStatusWin
 */
void destroyStatusWin (HWND hwnd);

/**
 * \fn HWND createToolTipWin (HWND hParentWnd, int x, int y, int timeout_ms, const char * text, ...)
 * \brief Creates a tool tip window.
 *
 * This function creates a tool tip window and returns the handle to it.
 * You can call \a destroyToolTipWin to destroy it. This function also 
 * receives \a printf-like arguments to format a string. 
 *
 * Note that the tool tip window will disappear automatically after the specified 
 * milliseconds by \a timeout_ms if \a timeout_ms is larger than 9 ms.
 *
 * \param hParentWnd The hosting main window.
 * \param x x,y: The position of the tool tip window.
 * \param y x,y: The position of the tool tip window.
 * \param timeout_ms The timeout value of the tool tip window. 
 * \param text The format string.
 * \return The handle to the tool tip window on success, HWND_INVALID on error.
 *
 * \sa resetToolTipWin, destroyToolTipWin
 */
HWND createToolTipWin (HWND hParentWnd, int x, int y, int timeout_ms,
                const char * text, ...);

/**
 * \fn void resetToolTipWin (HWND hwnd, int x, int y, const char* text, ...)
 * \brief Resets a tool tip window.
 *
 * This function resets the tool tip window specified by \a hwnd, including its
 * position, text displayed in it, and the visible status. If the tool tip is
 * invisible, it will become visible.
 *
 * This function also receives \a printf-like arguments to format a string. 
 *
 * Note that the tool tip window will disappear automatically after the specified 
 * milliseconds by \a timeout_ms if \a timeout_ms is larger than 9 ms.
 *
 * \param hwnd The tool tip window handle returned by \a createToolTipWin.
 * \param x x,y: The new position of the tool tip window.
 * \param y x,y: The new position of the tool tip window.
 * \param text The new format string.
 *
 * \sa createToolTipWin, destroyToolTipWin
 */
void resetToolTipWin (HWND hwnd, int x, int y, const char* text, ...);

/**
 * \fn void destroyToolTipWin (HWND hwnd)
 * \brief Destroies a tool tip window.
 *
 * This function destroies the specified tool tip window \a hwnd, which 
 * is returned by \a createToolTipWin.
 *
 * \param hwnd The handle to the tool tip window.
 * 
 * \sa createToolTipWin
 */
void destroyToolTipWin (HWND hwnd);

#ifdef _CTRL_PROGRESSBAR
/**
 * \fn HWND createProgressWin (HWND hParentWnd, char * title, char * label, int id, int range)
 * \brief Creates a main window within a progress bar.
 *
 * This function creates a main window within a progress bar and returns the handle.
 * You can call \a destroyProgressWin to destroy it. 
 *
 * Note that can use \a SendDlgItemMessage to send a message to the 
 * progress bar in the main window in order to update the progress bar.
 *
 * \param hParentWnd The hosting main window.
 * \param title The title of the progress window.
 * \param label The text in the label of the progress bar.
 * \param id The identifier of the progress bar.
 * \param range The maximal value of the progress bar (minimal value is 0).
 * \return The handle to the progress window on success, HWND_INVALID on error.
 *
 * \sa destroyProgressWin
 */
HWND createProgressWin (HWND hParentWnd, char * title, char * label,
        int id, int range);

/**
 * \fn void destroyProgressWin (HWND hwnd)
 * \brief Destroies progress window.
 *
 * This function destroies the specified progress window \a hwnd, which 
 * is returned by \a createProgressWin.
 *
 * \param hwnd The handle to the progress window.
 * 
 * \sa createToolTipWin
 */
void destroyProgressWin (HWND hwnd);
#endif

/**
 * Button info structure used by \a myWinMenu and \a myWinEntries function.
 * \sa myWinMenu, myWinEntries
 */
typedef struct _myWinButton
{
    /** text of the button. */
    char*   text;
    /** identifier of the button. */
    int     id;
    /** styles of the button. */
    DWORD   flags;
} myWINBUTTON;

/* This is an internal structure. */
typedef struct _myWinMenuItems
{
    /* the pointer to the array of the item strings. */
    char**      items;
    /* the identifier of the listbox display the menu items. */
    int         listboxid;
    /* the pointer to the array of the selection status of the items. */
    int*        selected;
    /* the minimal button identifier. */
    int         minbuttonid;
    /* the maximal button identifier. */
    int         maxbuttonid;
} myWINMENUITEMS;

/**
 * \fn int myWinMenu (HWND hParentWnd, const char* title, const char* label, int width, int listboxheight, char ** items, int * listItem, myWINBUTTON* buttons)
 * \brief Creates a menu main window for the user to select an item.
 *
 * This function creates a menu main window including a few buttons, 
 * and a list box with checkable item.
 * 
 * When the user click one of the buttons, this function will return the 
 * identifier of the button which leads to close the menu window, and 
 * the selections of the items via \a listItem.
 *
 * \param hParentWnd The hosting main window.
 * \param title The title of the menu main window.
 * \param label The label of the list box.
 * \param width The width of the menu main window.
 * \param listboxheight The height of the list box.
 * \param items The pointer to the array of the item strings.
 * \param listItem The pointer to the array of the check status of the items, initial and retured.
 * \param buttons The buttons will be created.
 * \return Returns the identifier of the button leading to close the menu window on success, else on errors.
 *
 * \sa myWINBUTTON
 */
int myWinMenu (HWND hParentWnd, const char* title, const char* label, 
                int width, int listboxheight, char ** items, int * listItem, 
                myWINBUTTON* buttons);

/**
 * Entry info structure used by \a myWinEntries function.
 * \sa myWinEntries
 */
typedef struct _myWinEntry
{
    /** the label of the entry. */
    char*   text;
    /** the pointer to the string of the entry. */
    char**  value;
    /** the maximal length of the entry in bytes. */
    int     maxlen;
    /** the styles of the entry. */
    DWORD   flags;
} myWINENTRY;

/* This is an internal structure. */
typedef struct _myWinEntryItems
{
    myWINENTRY* entries;
    int         entrycount;
    int         firstentryid;
    int         minbuttonid;
    int         maxbuttonid;
} myWINENTRYITEMS;

/**
 * \fn int myWinEntries (HWND hParentWnd, const char* title, const char* label, int width, int editboxwidth, BOOL fIME, myWINENTRY* items, myWINBUTTON* buttons)
 * \brief Creates a entry main window for the user to enter something.
 *
 * This function creates a entry main window including a few buttons
 * and a few entries.
 * 
 * When the user click one of the buttons, this function will return the 
 * identifier of the button which leads to close the menu window, and 
 * the entered strings.
 *
 * \param hParentWnd The hosting main window.
 * \param title The title of the menu main window.
 * \param label The label of the entries.
 * \param width The width of the menu main window.
 * \param editboxwidth The width of the edit boxes.
 * \param fIME Whether active the IME window.
 * \param items The pointer to the array of the entries, initial and returned.
 * \param buttons The buttons will be created.
 * \return Returns the identifier of the button leading to close the menu window on success, else on errors.
 *
 * \sa myWINBUTTON, myWINENTRY
 */
int myWinEntries (HWND hParentWnd, const char* title, const char* label,
                int width, int editboxwidth, BOOL fIME, myWINENTRY* items, 
                myWINBUTTON* buttons);

/**
 * \fn int myWinHelpMessage (HWND hwnd, int width, int height, const char* help_title, const char* help_msg)
 * \brief Creates a help message window.
 *
 * This function creates a help message window including a scrollable help message and a spin box.
 * When the user click the OK button, this function will return.
 *
 * \param hwnd The hosting main window.
 * \param width The width of the help message window.
 * \param height The height of the help message window.
 * \param help_title The title of the window.
 * \param help_msg The help message.
 * \return 0 on success, -1 on error.
 */
int myWinHelpMessage (HWND hwnd, int width, int height,
                const char* help_title, const char* help_msg);

    /** @} end of mywins_helpers */

    /** @} end of mywins_fns */

    /** @} end of mgext_fns */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _MGUI_MYWINDOWS_H */

