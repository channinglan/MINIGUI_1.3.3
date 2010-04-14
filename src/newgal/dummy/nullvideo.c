/*
**  $Id: nullvideo.c,v 1.7 2003/11/23 05:40:52 weiym Exp $
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

/* Dummy SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call GAL_Init(GAL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@linuxgames.com). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "DUMMY" by Sam Lantinga.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"

#include "nullvideo.h"

#define DUMMYVID_DRIVER_NAME "dummy"

/* Initialization/Query functions */
static int DUMMY_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **DUMMY_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags);
static GAL_Surface *DUMMY_SetVideoMode(_THIS, GAL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int DUMMY_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors);
static void DUMMY_VideoQuit(_THIS);

/* Hardware surface functions */
static int DUMMY_AllocHWSurface(_THIS, GAL_Surface *surface);
static void DUMMY_FreeHWSurface(_THIS, GAL_Surface *surface);

/* DUMMY driver bootstrap functions */

static int DUMMY_Available(void)
{
    return(1);
}

static void DUMMY_DeleteDevice(GAL_VideoDevice *device)
{
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *DUMMY_CreateDevice(int devindex)
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
    device->VideoInit = DUMMY_VideoInit;
    device->ListModes = DUMMY_ListModes;
    device->SetVideoMode = DUMMY_SetVideoMode;
    device->CreateYUVOverlay = NULL;
    device->SetColors = DUMMY_SetColors;
    device->VideoQuit = DUMMY_VideoQuit;
#ifdef _LITE_VERSION
    device->RequestHWSurface = NULL;
#endif
    device->AllocHWSurface = DUMMY_AllocHWSurface;
    device->CheckHWBlit = NULL;
    device->FillHWRect = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->FreeHWSurface = DUMMY_FreeHWSurface;

    device->free = DUMMY_DeleteDevice;

    return device;
}

VideoBootStrap DUMMY_bootstrap = {
    DUMMYVID_DRIVER_NAME, "Dummy video driver",
    DUMMY_Available, DUMMY_CreateDevice
};


static int DUMMY_VideoInit(_THIS, GAL_PixelFormat *vformat)
{
    fprintf(stderr, "WARNING: You are using the dummy video driver!\n");

    /* Determine the screen depth (use default 8-bit depth) */
    /* we change this during the GAL_SetVideoMode implementation... */
    vformat->BitsPerPixel = 8;
    vformat->BytesPerPixel = 1;

    /* We're done! */
    return(0);
}

static GAL_Rect **DUMMY_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags)
{
        return (GAL_Rect **) -1;
}

static GAL_Surface *DUMMY_SetVideoMode(_THIS, GAL_Surface *current,
                int width, int height, int bpp, Uint32 flags)
{
    if ( this->hidden->buffer ) {
        free( this->hidden->buffer );
    }

    this->hidden->buffer = malloc(width * height * (bpp / 8));
    if ( ! this->hidden->buffer ) {
        GAL_SetError("Couldn't allocate buffer for requested mode");
        return(NULL);
    }

    /* printf("Setting mode %dx%d\n", width, height); */

    memset(this->hidden->buffer, 0, width * height * (bpp / 8));

    /* Allocate the new pixel format for the screen */
    if ( ! GAL_ReallocFormat(current, bpp, 0, 0, 0, 0) ) {
        free(this->hidden->buffer);
        this->hidden->buffer = NULL;
        GAL_SetError("Couldn't allocate new pixel format for requested mode");
        return(NULL);
    }

    /* Set up the new mode framebuffer */
    current->flags = flags & GAL_FULLSCREEN;
    this->hidden->w = current->w = width;
    this->hidden->h = current->h = height;
    current->pitch = current->w * (bpp / 8);
    current->pixels = this->hidden->buffer;

    /* We're done */
    return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int DUMMY_AllocHWSurface(_THIS, GAL_Surface *surface)
{
    return(-1);
}
static void DUMMY_FreeHWSurface(_THIS, GAL_Surface *surface)
{
    surface->pixels = NULL;
}

static int DUMMY_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors)
{
    /* do nothing of note. */
    return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another video routine -- notably UpdateRects.
*/
static void DUMMY_VideoQuit(_THIS)
{
    if (this->screen->pixels != NULL)
    {
        free(this->screen->pixels);
        this->screen->pixels = NULL;
    }
}

