/*
**  $Id: ecoslcd.c,v 1.2 2003/11/22 04:44:14 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyring (C) 2001 ~ 2002 Wei Yongming.
**  Copyring (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
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

#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>  
#include <cyg/hal/lcd_support.h>

#include "common.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"

#include "ecoslcd.h"

#define ECOSLCDVID_DRIVER_NAME "ecoslcd"

/* Initialization/Query functions */
static int ECOSLCD_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **ECOSLCD_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags);
static GAL_Surface *ECOSLCD_SetVideoMode(_THIS, GAL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int ECOSLCD_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors);
static void ECOSLCD_VideoQuit(_THIS);

/* Hardware surface functions */
static int ECOSLCD_AllocHWSurface(_THIS, GAL_Surface *surface);
static void ECOSLCD_FreeHWSurface(_THIS, GAL_Surface *surface);

/* ECOSLCD driver bootstrap functions */

static int ECOSLCD_Available(void)
{
    return(1);
}

static void ECOSLCD_DeleteDevice(GAL_VideoDevice *device)
{
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *ECOSLCD_CreateDevice(int devindex)
{
    GAL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (GAL_VideoDevice *)malloc(sizeof(GAL_VideoDevice));
    if ( device ) {
        memset(device, 0, (sizeof *device));
        device->hidden = (struct GAL_PrivateVideoData *)
                malloc((sizeof *device->hidden));
    }
    if ( (device == NULL) || (device->hidden == NULL) ) {
        GAL_OutOfMemory();
        if ( device ) {
            free(device);
        }
        return(0);
    }
    memset(device->hidden, 0, (sizeof *device->hidden));

    /* Set the function pointers */
    device->VideoInit = ECOSLCD_VideoInit;
    device->ListModes = ECOSLCD_ListModes;
    device->SetVideoMode = ECOSLCD_SetVideoMode;
    device->CreateYUVOverlay = NULL;
    device->SetColors = ECOSLCD_SetColors;
    device->VideoQuit = ECOSLCD_VideoQuit;
#ifdef _LITE_VERSION
    device->RequestHWSurface = NULL;
#endif
    device->AllocHWSurface = ECOSLCD_AllocHWSurface;
    device->CheckHWBlit = NULL;
    device->FillHWRect = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->FreeHWSurface = ECOSLCD_FreeHWSurface;

    device->free = ECOSLCD_DeleteDevice;

    return device;
}

VideoBootStrap ECOSLCD_bootstrap = {
    ECOSLCDVID_DRIVER_NAME, "eCos LCD video driver",
    ECOSLCD_Available, ECOSLCD_CreateDevice
};


static int ECOSLCD_VideoInit(_THIS, GAL_PixelFormat *vformat)
{
    struct lcd_info li;

    fprintf(stderr, "WARNING: You are using the eCos LCD video driver!\n");

    /* Initialize LCD screen */
    lcd_init (16);
    lcd_getinfo (&li);

    this->hidden->w = li.width;
    this->hidden->h = li.height;
    this->hidden->pitch = li.rlen;
    this->hidden->fb = li.fb;

    switch (li.type) {
        case FB_TRUE_RGB565:
            vformat->BitsPerPixel = li.bpp;
            vformat->BytesPerPixel = 2;
            vformat->Rmask = 0x0000F800;
            vformat->Gmask = 0x000007E0;
            vformat->Bmask = 0x0000001F;
            break;
        default:
            GAL_SetError ("Not supported depth: %d.\n", vformat->BitsPerPixel);
            return -1;
    }

    /* We're done! */
    return(0);
}

static GAL_Rect **ECOSLCD_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags)
{
        return (GAL_Rect **) -1;
}

static GAL_Surface *ECOSLCD_SetVideoMode(_THIS, GAL_Surface *current,
                int width, int height, int bpp, Uint32 flags)
{
    /* Set up the new mode framebuffer */
    current->flags = GAL_HWSURFACE | GAL_FULLSCREEN;
    current->w = this->hidden->w;
    current->h = this->hidden->h;
    current->pitch = this->hidden->pitch;
    current->pixels = this->hidden->fb;

    /* We're done */
    return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int ECOSLCD_AllocHWSurface(_THIS, GAL_Surface *surface)
{
    return(-1);
}
static void ECOSLCD_FreeHWSurface(_THIS, GAL_Surface *surface)
{
    surface->pixels = NULL;
}

static int ECOSLCD_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors)
{
    /* do nothing of note. */
    return(1);
}

static void ECOSLCD_VideoQuit(_THIS)
{
    /* do nothing of note. */
    return;
}

