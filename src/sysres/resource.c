/*
** $Id: resource.c,v 1.11 2003/11/23 11:42:20 weiym Exp $
**
** resource.c: This file include some functions for system resource loading. 
**           some functions are from misc.c.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Create date: 2003/09/06
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cursor.h"
#include "icon.h"
#include "sysres.h"
#include "misc.h"


BITMAP SystemBitmap [SYSBMP_ITEM_NUMBER];
HICON  LargeSystemIcon [SYSICO_ITEM_NUMBER] = {0};
HICON  SmallSystemIcon [SYSICO_ITEM_NUMBER] = {0};

#ifndef _INCORE_RES

/****************************** System resource support *********************/

#ifdef _CURSOR_SUPPORT
PCURSOR LoadSystemCursor (int i)
{
    PCURSOR tempcsr;

    char szValue[MAX_NAME + 1];
    char szPathName[MAX_PATH + 1];
    char szFileName[MAX_PATH + 1];
    char szKey[10];

    if (GetMgEtcValue (CURSORSECTION, "cursorpath", szPathName, MAX_PATH) < 0)
                 goto error;

    sprintf (szKey, "cursor%d", i);
    if (GetMgEtcValue (CURSORSECTION, szKey, szValue, MAX_NAME) < 0)
                goto error;

    strcpy (szFileName, szPathName);
    strcat (szFileName, szValue);

    if (!(tempcsr = (PCURSOR)LoadCursorFromFile (szFileName)))
                     goto error;

    return tempcsr;

error:
    return 0;
}
#endif

BOOL GUIAPI LoadSystemBitmap (PBITMAP pBitmap, const char* szItemName)
{
    char szPathName[MAX_PATH + 1];
    char szFileName[MAX_PATH + 1];
    char szValue[MAX_NAME + 1];
    
    if (GetMgEtcValue ("bitmapinfo", szItemName,
            szValue, MAX_NAME) < 0 ) {
        fprintf (stderr, "Get bitmap file name error!\n");
        return FALSE;
    }
    
    if (GetMgEtcValue ("bitmapinfo", "bitmappath",
            szPathName, MAX_PATH) < 0 ) {
        fprintf (stderr, "Get bitmap path error!\n");
        return FALSE;
    }

    strcpy(szFileName, szPathName);
    strcat(szFileName, szValue);
    
    if (LoadBitmap (HDC_SCREEN, pBitmap, szFileName) < 0) {
        fprintf (stderr, "Load bitmap error: %s!\n", szFileName);
        return FALSE;
    }
    
    return TRUE;
}

HICON GUIAPI LoadSystemIcon (const char* szItemName, int which)
{
    char szPathName[MAX_PATH + 1];
    char szFileName[MAX_PATH + 1];
    char szValue[MAX_NAME + 1];
    HICON hIcon;
    
    if (GetMgEtcValue ("iconinfo", szItemName,
            szValue, MAX_NAME) < 0 ) {
        fprintf (stderr, "Get icon file name error!\n");
        return 0;
    }
    
    if (GetMgEtcValue ("iconinfo", "iconpath",
            szPathName, MAX_PATH) < 0 ) {
        fprintf (stderr, "Get icon path error!\n");
        return 0;
    }

    strcpy (szFileName, szPathName);
    strcat (szFileName, szValue);
    
    if ((hIcon = LoadIconFromFile (HDC_SCREEN, szFileName, which)) == 0) {
        fprintf (stderr, "Load icon error: %s!\n", szFileName);
        return 0;
    }
    
    return hIcon;
}

BOOL InitSystemRes (void)
{
    int i;
    int nBmpNr, nIconNr;
    char szValue [12];
    
    /*
     * Load system bitmaps here.
     */
    if (GetMgEtcValue ("bitmapinfo", "bitmapnumber", 
                            szValue, 10) < 0)
        return FALSE;
    nBmpNr = atoi (szValue);
    if (nBmpNr <= 0) return FALSE;
    nBmpNr = nBmpNr < SYSBMP_ITEM_NUMBER ? nBmpNr : SYSBMP_ITEM_NUMBER;

    for (i = 0; i < nBmpNr; i++) {
        sprintf (szValue, "bitmap%d", i);

        if (!LoadSystemBitmap (SystemBitmap + i, szValue))
                return FALSE;
    }

    /*
     * Load system icons here.
     */
    if (GetMgEtcValue ("iconinfo", "iconnumber", 
                            szValue, 10) < 0 )
        return FALSE;
    nIconNr = atoi(szValue);
    if (nIconNr <= 0) return FALSE;
    nIconNr = nIconNr < SYSICO_ITEM_NUMBER ? nIconNr : SYSICO_ITEM_NUMBER;

    for (i = 0; i < nIconNr; i++) {
        sprintf(szValue, "icon%d", i);
        
        SmallSystemIcon [i] = LoadSystemIcon (szValue, 1);
        LargeSystemIcon [i] = LoadSystemIcon (szValue, 0);

        if (SmallSystemIcon [i] == 0 || LargeSystemIcon [i] == 0)
            return FALSE;
    }

    return TRUE;
}

#else /* _INCORE_RES */

#include "cursors.c"

#ifdef _FLAT_WINDOW_STYLE

#ifdef _CTRL_BUTTON
#include "button-flat-bmp.c"
#endif

#ifdef _CTRL_COMBOBOX
#include "downarrow-flat-bmp.c"
#include "updownarrow-flat-bmp.c"
#endif

#ifdef _CTRL_LISTBOX
#include "checkmark-flat-bmp.c"
#endif

#ifdef _EXT_CTRL_SPINBOX
#include "spinbox-flat-bmp.c"
#endif

#ifdef __ECOS
#include "bmps-flat-ecos.c"
#else
#include "bmps-flat.c"
#endif

#include "icons-flat.c"

#else

#ifdef _CTRL_BUTTON
#include "button-3d-bmp.c"
#endif

#ifdef _CTRL_COMBOBOX
#include "downarrow-3d-bmp.c"
#include "updownarrow-3d-bmp.c"
#endif

#ifdef _CTRL_LISTBOX
#include "checkmark-3d-bmp.c"
#endif

#ifdef _EXT_CTRL_SPINBOX
#include "spinbox-3d-bmp.c"
#endif

#ifdef __ECOS
#include "bmps-3d-ecos.c"
#else
#include "bmps-3d.c"
#endif

#include "icons-3d.c"

#endif /* _FLAT_WINDOW_STYLE */

#ifdef _EXT_CTRL_TREEVIEW
#include "fold-ico.c"
#include "unfold-ico.c"
#endif

SYSRES sysres_data [] = {
#ifdef _CTRL_BUTTON
        {"button", (void*)button_bmp_data, sizeof(button_bmp_data), 0},
#endif
#ifdef _CTRL_COMBOBOX
        {"downarrow", (void*)downarrow_bmp_data, sizeof(downarrow_bmp_data), 0},
        {"updownarrow", (void*)updownarrow_bmp_data, sizeof(updownarrow_bmp_data), 0},
#endif
#ifdef _CTRL_LISTBOX
        {"checkmark", (void*)checkmark_bmp_data, sizeof(checkmark_bmp_data), 0},
#endif
#ifdef _IME_GB2312
        {"ime", NULL, 0, 0},
#endif
        {"logo", NULL, 0, 0},
#ifdef _EXT_CTRL_SPINBOX
        {"spinbox", (void*)spinbox_bmp_data, sizeof(spinbox_bmp_data), 0},
#endif
#ifdef _EXT_CTRL_TREEVIEW
        {"fold", (void*)fold_ico_data, sizeof(fold_ico_data), 0},
        {"unfold", (void*)unfold_ico_data, sizeof(unfold_ico_data), 0},
#endif
        {"icons", (void*)icons_data, SZ_ICON, 0},
        {"bitmap", (void*)bmps_data, (int)sz_bmps, 1}
};

#define SYSRES_NR (sizeof(sysres_data) / sizeof(SYSRES))

static void* get_res_position (const char* szItemName, int *len)
{
    int i = 0;
    int idx_len = 0;
    int name_len;
    int item_idx = 0;
    const char *pidx;

    if (!szItemName || szItemName[0] == '\0')
        return NULL;

    name_len = strlen (szItemName);
    pidx = szItemName + name_len - 1;
    idx_len = 0;
    while ( isdigit(*pidx) )
    {
        idx_len ++;
        if (idx_len == name_len)
            return NULL;
        pidx --;
    }
    name_len -= idx_len;

    if (idx_len > 0)
        item_idx = atoi (szItemName+name_len);

    while ( strncmp(sysres_data[i].name, szItemName, name_len) != 0 && i < SYSRES_NR) i++;
    if (i >= SYSRES_NR)
        return NULL;

    if (sysres_data[i].flag > 0) {
        void *pos = sysres_data[i].res_data;
        int j;
        for (j=0; j<item_idx; j++) {
            pos += *( (int*)sysres_data[i].data_len + j );
        }
        if (len)
            *len = *( (int*)sysres_data[i].data_len + item_idx );
        return pos;
    }
    if (len)
        *len = sysres_data[i].data_len;
    return sysres_data[i].res_data + sysres_data[i].data_len *item_idx;
}

HICON GUIAPI LoadSystemIcon (const char* szItemName, int which)
{
    void *icon;

    icon = get_res_position (szItemName, NULL);
    if (!icon)
        return 0;

    return LoadIconFromMem (HDC_SCREEN, icon, which);
}

BOOL GUIAPI LoadSystemBitmap (PBITMAP pBitmap, const char* szItemName)
{
    Uint8* bmpdata;
    int len;

    bmpdata = get_res_position (szItemName, &len);
    if (!bmpdata)
        return FALSE;

    if (LoadBitmapFromMemory (HDC_SCREEN, pBitmap,
                              bmpdata, len, "bmp"))
        return FALSE;

    return TRUE;
}

BOOL InitSystemRes (void)
{
    int i;
    const Uint8* bmp = bmps_data;
    const Uint8* icon = icons_data;

    for (i = 0; i < NR_BMPS; i++) {
        if (LoadBitmapFromMemory (HDC_SCREEN, SystemBitmap + i, bmp, 
                                sz_bmps [i], "bmp")) {
            fprintf (stderr, "error when loading %d system bitmap.\n", i);
            return FALSE;
        }
        bmp += sz_bmps [i];
    }

    for (i = 0; i < NR_ICONS; i++) {
        SmallSystemIcon [i] = LoadIconFromMem (HDC_SCREEN, icon, 1);
        LargeSystemIcon [i] = LoadIconFromMem (HDC_SCREEN, icon, 0);

        icon += SZ_ICON;

        if (SmallSystemIcon [i] == 0 || LargeSystemIcon [i] == 0) {
            fprintf (stderr, "error when loading %d system icon.\n", i);
            return FALSE;
        }
    }

    return TRUE;
}

#ifdef _CURSOR_SUPPORT
PCURSOR LoadSystemCursor (int i)
{
    return (PCURSOR)LoadCursorFromMem (cursors_data + SZ_CURSOR*i);
}
#endif

#endif /* _INCORE_RES */


void TerminateSysRes (void)
{
    int i;
    
    for (i=0; i<SYSBMP_ITEM_NUMBER; i++)
        UnloadBitmap (SystemBitmap + i);

    for (i=0; i<SYSICO_ITEM_NUMBER; i++) {
        if (SmallSystemIcon [i])
            DestroyIcon (SmallSystemIcon [i]);

        if (LargeSystemIcon [i])
            DestroyIcon (LargeSystemIcon [i]);
    }
}

