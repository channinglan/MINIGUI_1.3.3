/**
 * \file filedlg.h
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 2001/12/29
 * 
 *  This file includes interfaces of Open File Dialog Box.
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
 * $Id: filedlg.h,v 1.3 2003/09/04 06:12:04 weiym Exp $
 *
 *             MiniGUI for Linux Version 1.3.x
 *             Copyright (C) 2002 ~ 2003 Feynman Software.
 *             Copyright (C) 2000 ~ 2002 Wei Yongming and others.
 */

#ifndef _MGUI_FILEDLG_H
#define _MGUI_FILEDLG_H

#include <sys/types.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

    /**
     * \addtogroup mywins_fns Interfaces of MyWins module of MiniGUIExt library
     * @{
     */

    /**
     * \defgroup mywins_filedlg Open File Dialog Box
     * @{
     */

#define FILE_ERROR_OK           0
#define FILE_ERROR_PARAMERR     -1
#define FILE_ERROR_PATHNOTEXIST -2

/**
 * The file dialog box structure used by \a OpenFileDialogEx.
 * \sa OpenFileDialogEx.
 */
typedef struct _FILEDLGDATA
{
    /** indicates to create a Save File or an Open File dialog box. */
	BOOL   	IsSave;
    /** the full path name of the file returned. */
    char   	filefullname[NAME_MAX + PATH_MAX + 1];
    /** the name of the file to be opened. */
    char   	filename[NAME_MAX + 1];
    /** the initial path of the dialog box. */
    char   	filepath[PATH_MAX + 1];
} FILEDLGDATA;

typedef FILEDLGDATA* PFILEDLGDATA;

/**
 * \fn int OpenFileDialogEx (HWND hWnd, int lx, int ty, int w, int h, PFILEDLGDATA pmwfi)
 * \brief Creates an Open File Dialog Box.
 *
 * This function creates an Open File Dialog Box, and returns the full path name of 
 * the file selected by user.
 *
 * \param hWnd The hosting main window.
 * \param lx lx,ty,w,h: The default position and size of the dialog box.
 * \param ty lx,ty,w,h: The default position and size of the dialog box.
 * \param w  lx,ty,w,h: The default position and size of the dialog box.
 * \param h  lx,ty,w,h: The default position and size of the dialog box.
 * \param pmwfi The pointer to the FILEDLGDATA structure.
 * \return A positive integer on success, negative on error.
 *
 * \retval IDOK The user choosed a file and OK button clicked.
 * \retval IDCANCLE CANCEL button clicked.
 * \retval FILE_ERROR_PARAMERR You passed a wrong parameters.
 * \retval FILE_ERROR_PATHNOTEXIST The path specified in \a pmwfi does not exist.
 * 
 * \sa FILEDLGDATA, OpenFileDialog
 */
int OpenFileDialogEx (HWND hWnd, int lx, int ty, int w, int h, PFILEDLGDATA pmwfi);

/**
 * \fn OpenFileDialog (HWND hWnd, BOOL isSave, PFILEDLGDATA pmwfi)
 * \brief Creates an Open File Dialog Box.
 *
 * This function creates an Open File Dialog Box, and returns the full path name of 
 * the file selected by user. 
 *
 * It is defined an inline function calling \a OpenFileDialogEx.
 *
 * \param hWnd The hosting main window.
 * \param isSave Indicates to create a Save File or an Open File dialog box.
 * \param pmwfi The pointer to the FILEDLGDATA structure.
 * \return A positive integer on success, negative on error.
 *
 * \retval IDOK The user choosed a file and OK button clicked.
 * \retval IDCANCLE CANCEL button clicked.
 * \retval FILE_ERROR_PARAMERR You passed a wrong parameters.
 * \retval FILE_ERROR_PATHNOTEXIST The path specified in \a pmwfi does not exist.
 * 
 * \sa FILEDLGDATA, OpenFileDialogEx
 */
static inline int OpenFileDialog (HWND hWnd, BOOL isSave, PFILEDLGDATA pmwfi)
{
    pmwfi->IsSave = isSave;
    return OpenFileDialogEx (hWnd, 0, 0, g_rcScr.right, g_rcScr.bottom, pmwfi);
//    return OpenFileDialogEx (hWnd, 50, 50, 330, 280, pmwfi);
}

    /** @} end of mywins_filedlg */

    /** @} end of mywins_fns */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _MGUI_FILEDLG_H */


