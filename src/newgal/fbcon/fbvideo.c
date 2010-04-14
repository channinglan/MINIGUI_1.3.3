/*
**  $Id: fbvideo.c,v 1.27 2003/11/23 05:40:52 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
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

/* Framebuffer console based video driver implementation.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/page.h>        /* For definition of PAGE_SIZE */

#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/keyboard.h>

#include "common.h"
#include "minigui.h"
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
#include "client.h"
#endif
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"
#include "fbvideo.h"
#include "fb3dfx.h"
#include "fbmatrox.h"
#include "fbneomagic.h"

/*
#define FBACCEL_DEBUG   1
#define FBCON_DEBUG   1
*/

#if defined(i386) && defined(FB_TYPE_VGA_PLANES)
#if 0
#define VGA16_FBCON_SUPPORT
#include <sys/io.h>
#ifndef FB_AUX_VGA_PLANES_VGA4
#define FB_AUX_VGA_PLANES_VGA4    0
#endif
inline static void outb (unsigned char value, unsigned short port)
{
  __asm__ __volatile__ ("outb %b0,%w1"::"a" (value), "Nd" (port));
} 
#endif
#endif /* FB_TYPE_VGA_PLANES */

/* A list of video resolutions that we query for (sorted largest to smallest) */
static const GAL_Rect checkres[] = {
    {  0, 0, 1600, 1200 },        /* 16 bpp: 0x11E, or 286 */
    {  0, 0, 1408, 1056 },        /* 16 bpp: 0x19A, or 410 */
    {  0, 0, 1280, 1024 },        /* 16 bpp: 0x11A, or 282 */
    {  0, 0, 1152,  864 },        /* 16 bpp: 0x192, or 402 */
    {  0, 0, 1024,  768 },        /* 16 bpp: 0x117, or 279 */
    {  0, 0,  960,  720 },        /* 16 bpp: 0x18A, or 394 */
    {  0, 0,  800,  600 },        /* 16 bpp: 0x114, or 276 */
    {  0, 0,  768,  576 },        /* 16 bpp: 0x182, or 386 */
    {  0, 0,  640,  480 },        /* 16 bpp: 0x111, or 273 */
    {  0, 0,  640,  400 },        /*  8 bpp: 0x100, or 256 */
    {  0, 0,  512,  384 },
    {  0, 0,  320,  240 },
    {  0, 0,  320,  200 },
    {  0, 0,  240,  320 },
    {  0, 0,  160,  160 },
    {  0, 0,  60,  60 }
};
static const struct {
    int xres;
    int yres;
    int pixclock;
    int left;
    int right;
    int upper;
    int lower;
    int hslen;
    int vslen;
    int sync;
    int vmode;
} vesa_timings[] = {
#ifdef USE_VESA_TIMINGS    /* Only tested on Matrox Millenium I */
    {  640,  400, 39771,  48, 16, 39,  8,  96, 2, 2, 0 },    /* 70 Hz */
    {  640,  480, 39683,  48, 16, 33, 10,  96, 2, 0, 0 },    /* 60 Hz */
    {  768,  576, 26101, 144, 16, 28,  6, 112, 4, 0, 0 },    /* 60 Hz */
    {  800,  600, 24038, 144, 24, 28,  8, 112, 6, 0, 0 },    /* 60 Hz */
    {  960,  720, 17686, 144, 24, 28,  8, 112, 4, 0, 0 },    /* 60 Hz */
    { 1024,  768, 15386, 160, 32, 30,  4, 128, 4, 0, 0 },    /* 60 Hz */
    { 1152,  864, 12286, 192, 32, 30,  4, 128, 4, 0, 0 },    /* 60 Hz */
    { 1280, 1024,  9369, 224, 32, 32,  4, 136, 4, 0, 0 },    /* 60 Hz */
    { 1408, 1056,  8214, 256, 40, 32,  5, 144, 5, 0, 0 },    /* 60 Hz */
    { 1600, 1200,/*?*/0, 272, 48, 32,  5, 152, 5, 0, 0 },    /* 60 Hz */
#else
    /* You can generate these timings from your XF86Config file using
       the 'modeline2fb' perl script included with the fbset package.
       These timings were generated for Matrox Millenium I, 15" monitor.
    */
    {  320,  200, 79440,  16, 16, 20,  4,  48, 1, 0, 2 },    /* 70 Hz */
    {  320,  240, 63492,  16, 16, 16,  4,  48, 2, 0, 2 },    /* 72 Hz */
    {  512,  384, 49603,  48, 16, 16,  1,  64, 3, 0, 0 },    /* 78 Hz */
    {  640,  400, 31746,  96, 32, 41,  1,  64, 3, 2, 0 },    /* 85 Hz */
    {  640,  480, 31746, 120, 16, 16,  1,  64, 3, 0, 0 },    /* 75 Hz */
    {  768,  576, 26101, 144, 16, 28,  6, 112, 4, 0, 0 },    /* 60 Hz */
    {  800,  600, 20000,  64, 56, 23, 37, 120, 6, 3, 0 },    /* 72 Hz */
    {  960,  720, 17686, 144, 24, 28,  8, 112, 4, 0, 0 },    /* 60 Hz */
    { 1024,  768, 13333, 144, 24, 29,  3, 136, 6, 0, 0 },    /* 70 Hz */
    { 1152,  864, 12286, 192, 32, 30,  4, 128, 4, 0, 0 },    /* 60 Hz */
    { 1280, 1024,  9369, 224, 32, 32,  4, 136, 4, 0, 0 },    /* 60 Hz */
    { 1408, 1056,  8214, 256, 40, 32,  5, 144, 5, 0, 0 },    /* 60 Hz */
    { 1600, 1200,/*?*/0, 272, 48, 32,  5, 152, 5, 0, 0 },    /* 60 Hz */
#endif
};

/* Initialization/Query functions */
static int FB_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **FB_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags);
static GAL_Surface *FB_SetVideoMode(_THIS, GAL_Surface *current, int width, int height, int bpp, Uint32 flags);
#ifdef VGA16_FBCON_SUPPORT
static GAL_Surface *FB_SetVGA16Mode(_THIS, GAL_Surface *current, int width, int height, int bpp, Uint32 flags);
#endif
static int FB_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors);
static void FB_VideoQuit(_THIS);

/* Hardware surface functions */
static int FB_InitHWSurfaces(_THIS, GAL_Surface *screen, char *base, int size);
static void FB_FreeHWSurfaces(_THIS);
static void FB_RequestHWSurface (_THIS, const REQ_HWSURFACE* request, REP_HWSURFACE* reply);
static int FB_AllocHWSurface(_THIS, GAL_Surface *surface);
static void FB_FreeHWSurface(_THIS, GAL_Surface *surface);
static void FB_WaitVBL(_THIS);
static void FB_WaitIdle(_THIS);

#if 0
static int FB_LockHWSurface(_THIS, GAL_Surface *surface);
static void FB_UnlockHWSurface(_THIS, GAL_Surface *surface);
static int FB_FlipHWSurface(_THIS, GAL_Surface *surface);
#endif

/* Internal palette functions */
static void FB_SavePalette(_THIS, struct fb_fix_screeninfo *finfo,
                                  struct fb_var_screeninfo *vinfo);
static void FB_RestorePalette(_THIS);

/* FB driver bootstrap functions */

static int FB_Available(void)
{
    int console;
    const char *GAL_fbdev;

    GAL_fbdev = getenv("FRAMEBUFFER");
    if ( GAL_fbdev == NULL ) {
        GAL_fbdev = "/dev/fb0";
    }
    console = open(GAL_fbdev, O_RDWR, 0);
    if ( console >= 0 ) {
        close(console);
    }
    return(console >= 0);
}

static void FB_DeleteDevice(GAL_VideoDevice *device)
{
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *FB_CreateDevice(int devindex)
{
    GAL_VideoDevice *this;

    /* Initialize all variables that we clean on shutdown */
    this = (GAL_VideoDevice *)malloc(sizeof(GAL_VideoDevice));
    if ( this ) {
        memset(this, 0, (sizeof *this));
        this->hidden = (struct GAL_PrivateVideoData *)
                malloc((sizeof *this->hidden));
    }
    if ( (this == NULL) || (this->hidden == NULL) ) {
        GAL_OutOfMemory();
        if ( this ) {
            free(this);
        }
        return(0);
    }
    memset(this->hidden, 0, (sizeof *this->hidden));
    wait_vbl = FB_WaitVBL;
    wait_idle = FB_WaitIdle;
    mouse_fd = -1;
    keyboard_fd = -1;

    /* Set the function pointers */
    this->VideoInit = FB_VideoInit;
    this->ListModes = FB_ListModes;
    this->SetVideoMode = FB_SetVideoMode;
    this->SetColors = FB_SetColors;
    this->VideoQuit = FB_VideoQuit;
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    this->RequestHWSurface = FB_RequestHWSurface;
#endif
    this->AllocHWSurface = FB_AllocHWSurface;
    this->CheckHWBlit = NULL;
    this->FillHWRect = NULL;
    this->SetHWColorKey = NULL;
    this->SetHWAlpha = NULL;
    this->UpdateRects = NULL;
#if 0
    this->LockHWSurface = FB_LockHWSurface;
    this->UnlockHWSurface = FB_UnlockHWSurface;
    this->FlipHWSurface = FB_FlipHWSurface;
#endif
    this->FreeHWSurface = FB_FreeHWSurface;

    this->free = FB_DeleteDevice;

    return this;
}

VideoBootStrap FBCON_bootstrap = {
    "fbcon", "Linux Framebuffer Console",
    FB_Available, FB_CreateDevice
};

static int FB_CheckMode(_THIS, struct fb_var_screeninfo *vinfo,
                        int index, unsigned int *w, unsigned int *h)
{
    int mode_okay;

    mode_okay = 0;
    vinfo->bits_per_pixel = (index+1)*8;
    vinfo->xres = *w;
    vinfo->xres_virtual = *w;
    vinfo->yres = *h;
    vinfo->yres_virtual = *h;
    vinfo->activate = FB_ACTIVATE_TEST;
    if ( ioctl(console_fd, FBIOPUT_VSCREENINFO, vinfo) == 0 ) {
#ifdef FBCON_DEBUG
        fprintf(stderr, "Checked mode %dx%d at %d bpp, got mode %dx%d at %d bpp\n", *w, *h, (index+1)*8, vinfo->xres, vinfo->yres, vinfo->bits_per_pixel);
#endif
        if ( (((vinfo->bits_per_pixel+7)/8)-1) == index ) {
            *w = vinfo->xres;
            *h = vinfo->yres;
            mode_okay = 1;
        }
    }
    return mode_okay;
}

static int FB_AddMode(_THIS, int index, unsigned int w, unsigned int h)
{
    GAL_Rect *mode;
    int i;
    int next_mode;

    /* Check to see if we already have this mode */
    if ( GAL_nummodes[index] > 0 ) {
        mode = GAL_modelist[index][GAL_nummodes[index]-1];
        if ( (mode->w == w) && (mode->h == h) ) {
#ifdef FBCON_DEBUG
            fprintf(stderr, "We already have mode %dx%d at %d bytes per pixel\n", w, h, index+1);
#endif
            return(0);
        }
    }

    if (w >= 320) {
        /* Only allow a mode if we have a valid timing for it */
        next_mode = -1;
        for ( i=0; i<(sizeof(vesa_timings)/sizeof(vesa_timings[0])); ++i ) {
            if ( (w == vesa_timings[i].xres) &&
                (h == vesa_timings[i].yres) && vesa_timings[i].pixclock ) {
                next_mode = i;
                break;
            }
        }

        if ( next_mode == -1 ) {
#ifdef FBCON_DEBUG
            fprintf(stderr, "No valid timing line for mode %dx%d\n", w, h);
#endif
            return(0);
        }
    }

    /* Set up the new video mode rectangle */
    mode = (GAL_Rect *)malloc(sizeof *mode);
    if ( mode == NULL ) {
        GAL_OutOfMemory();
        return(-1);
    }
    mode->x = 0;
    mode->y = 0;
    mode->w = w;
    mode->h = h;
#ifdef FBCON_DEBUG
    fprintf(stderr, "Adding mode %dx%d at %d bytes per pixel\n", w, h, index+1);
#endif

    /* Allocate the new list of modes, and fill in the new mode */
    next_mode = GAL_nummodes[index];
    GAL_modelist[index] = (GAL_Rect **)
           realloc(GAL_modelist[index], (1+next_mode+1)*sizeof(GAL_Rect *));
    if ( GAL_modelist[index] == NULL ) {
        GAL_OutOfMemory();
        GAL_nummodes[index] = 0;
        free(mode);
        return(-1);
    }
    GAL_modelist[index][next_mode] = mode;
    GAL_modelist[index][next_mode+1] = NULL;
    GAL_nummodes[index]++;

    return(0);
}

static int ttyfd = -1;
static void FB_LeaveGraphicsMode (_THIS)
{
#ifdef _HAVE_TEXT_MODE
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer) {
#endif
        /* enter text mode*/
        if (ttyfd >= 0) {
            ioctl (ttyfd, KDSETMODE, KD_TEXT);
            close (ttyfd);
            ttyfd = -1;
        }
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    }
#endif
#endif
}

static int FB_EnterGraphicsMode (_THIS)
{
#ifdef _HAVE_TEXT_MODE
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer) {
#endif
        char* tty_dev;
        if (geteuid() == 0)
            tty_dev = "/dev/tty0";
        else    /* not a super user, so try to open the control terminal */
            tty_dev = "/dev/tty";

        /* open tty, enter graphics mode */
        ttyfd = open (tty_dev, O_RDWR);
        if (ttyfd < 0) {
            fprintf (stderr,"GAL ENGINE: Can't open %s: %m\n", tty_dev);
            goto fail;
        }
        if (ioctl (ttyfd, KDSETMODE, KD_GRAPHICS) == -1) {
            fprintf (stderr,"GAL ENGINE: Error when setting the terminal to graphics mode: %m\n");
            fprintf (stderr,"GAL ENGINE: Maybe is not a console.\n");
            goto fail;
        }
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    }
    else {
        ttyfd = 0;
    }
#endif

    return ttyfd;

fail:
    FB_LeaveGraphicsMode (this);
    return -1;

#else
    return 0;
#endif
}

static int FB_OpenKeyboard(_THIS)
{
#if 1
    return 0;
#else
    /* Open only if not already opened */
     if ( keyboard_fd < 0 ) {
        static const char * const tty0[] = { "/dev/tty0", "/dev/vc/0", NULL };
        static const char * const vcs[] = { "/dev/vc/%d", "/dev/tty%d", NULL };
        int i, tty0_fd;

        /* Try to query for a free virtual terminal */
        tty0_fd = -1;
        for ( i=0; tty0[i] && (tty0_fd < 0); ++i ) {
            tty0_fd = open(tty0[i], O_WRONLY, 0);
        }
        if ( tty0_fd < 0 ) {
            tty0_fd = dup(0); /* Maybe stdin is a VT? */
        }
        ioctl(tty0_fd, VT_OPENQRY, &current_vt);
        close(tty0_fd);
        if ( (geteuid() == 0) && (current_vt > 0) ) {
            for ( i=0; vcs[i] && (keyboard_fd < 0); ++i ) {
                char vtpath[12];

                sprintf(vtpath, vcs[i], current_vt);
                keyboard_fd = open(vtpath, O_RDWR, 0);
#ifdef DEBUG_KEYBOARD
                fprintf(stderr, "vtpath = %s, fd = %d\n",
                    vtpath, keyboard_fd);
#endif /* DEBUG_KEYBOARD */

                /* This needs to be our controlling tty
                   so that the kernel ioctl() calls work
                */
                if ( keyboard_fd >= 0 ) {
                    tty0_fd = open("/dev/tty", O_RDWR, 0);
                    if ( tty0_fd >= 0 ) {
                        ioctl(tty0_fd, TIOCNOTTY, 0);
                        close(tty0_fd);
                    }
                }
            }
        }
         if ( keyboard_fd < 0 ) {
            /* Last resort, maybe our tty is a usable VT */
            current_vt = 0;
            keyboard_fd = open("/dev/tty", O_RDWR);
         }
#ifdef DEBUG_KEYBOARD
        fprintf(stderr, "Current VT: %d\n", current_vt);
#endif

        /* Make sure that our input is a console terminal */
        { int dummy;
          if ( ioctl(keyboard_fd, KDGKBMODE, &dummy) < 0 ) {
            close(keyboard_fd);
            keyboard_fd = -1;
            GAL_SetError("Unable to open a console terminal");
          }
        }

     }
#endif
     return(keyboard_fd);
}

static int FB_VideoInit(_THIS, GAL_PixelFormat *vformat)
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    int i, j;
    int current_index;
    unsigned int current_w;
    unsigned int current_h;
    const char *GAL_fbdev;

    /* Initialize the library */
    GAL_fbdev = getenv("FRAMEBUFFER");
    if ( GAL_fbdev == NULL ) {
        GAL_fbdev = "/dev/fb0";
    }
    console_fd = open(GAL_fbdev, O_RDWR, 0);
    if ( console_fd < 0 ) {
        GAL_SetError("Unable to open %s", GAL_fbdev);
        return(-1);
    }

    /* Get the type of video hardware */
    if ( ioctl(console_fd, FBIOGET_FSCREENINFO, &finfo) < 0 ) {
        GAL_SetError("Couldn't get console hardware info");
        FB_VideoQuit(this);
        return(-1);
    }
    switch (finfo.type) {
        case FB_TYPE_PACKED_PIXELS:
            /* Supported, no worries.. */
            break;
#ifdef VGA16_FBCON_SUPPORT
        case FB_TYPE_VGA_PLANES:
            /* VGA16 is supported, but that's it */
            if ( finfo.type_aux == FB_AUX_VGA_PLANES_VGA4 ) {
                if ( ioperm(0x3b4, 0x3df - 0x3b4 + 1, 1) < 0 ) {
                    GAL_SetError("No I/O port permissions");
                    FB_VideoQuit(this);
                    return(-1);
                }
                this->SetVideoMode = FB_SetVGA16Mode;
                break;
            }
            /* Fall through to unsupported case */
#endif /* VGA16_FBCON_SUPPORT */
        default:
            GAL_SetError("Unsupported console hardware");
            FB_VideoQuit(this);
            return(-1);
    }
    switch (finfo.visual) {
        case FB_VISUAL_TRUECOLOR:
        case FB_VISUAL_PSEUDOCOLOR:
        case FB_VISUAL_STATIC_PSEUDOCOLOR:
        case FB_VISUAL_DIRECTCOLOR:
            break;
        default:
            GAL_SetError("Unsupported console hardware");
            FB_VideoQuit(this);
            return(-1);
    }

#if 0
    /* Check if the user wants to disable hardware acceleration */
    { const char *fb_accel;
        fb_accel = getenv("GAL_FBACCEL");
        if ( fb_accel ) {
            finfo.accel = atoi(fb_accel);
        }
    }
#endif

    /* Memory map the device, compensating for buggy PPC mmap() */
    mapped_offset = (((long)finfo.smem_start) -
                    (((long)finfo.smem_start)&~(PAGE_SIZE-1)));
    mapped_memlen = finfo.smem_len+mapped_offset;
    mapped_mem = mmap(NULL, mapped_memlen,
                      PROT_READ|PROT_WRITE, MAP_SHARED, console_fd, 0);
    if ( mapped_mem == (char *)-1 ) {
        GAL_SetError("Unable to memory map the video hardware");
        mapped_mem = NULL;
        FB_VideoQuit(this);
        return(-1);
    }

    /* Determine the current screen depth */
    if ( ioctl(console_fd, FBIOGET_VSCREENINFO, &vinfo) < 0 ) {
        GAL_SetError("Couldn't get console pixel format");
        FB_VideoQuit(this);
        return(-1);
    }
    vformat->BitsPerPixel = vinfo.bits_per_pixel;
    if ( vformat->BitsPerPixel < 8 ) {
        /* Assuming VGA16, we handle this via a shadow framebuffer */
        vformat->BitsPerPixel = 8;
    }
    for ( i=0; i<vinfo.red.length; ++i ) {
        vformat->Rmask <<= 1;
        vformat->Rmask |= (0x00000001<<vinfo.red.offset);
    }
    for ( i=0; i<vinfo.green.length; ++i ) {
        vformat->Gmask <<= 1;
        vformat->Gmask |= (0x00000001<<vinfo.green.offset);
    }
    for ( i=0; i<vinfo.blue.length; ++i ) {
        vformat->Bmask <<= 1;
        vformat->Bmask |= (0x00000001<<vinfo.blue.offset);
    }
    saved_vinfo = vinfo;

    /* Save hardware palette, if needed */
    FB_SavePalette(this, &finfo, &vinfo);

    /* If the I/O registers are available, memory map them so we
       can take advantage of any supported hardware acceleration.
     */
    vinfo.accel_flags = 0;    /* Temporarily reserve registers */
    ioctl(console_fd, FBIOPUT_VSCREENINFO, &vinfo);
    if ( finfo.accel && finfo.mmio_len ) {
        mapped_iolen = finfo.mmio_len;
        mapped_io = mmap(NULL, mapped_iolen, PROT_READ|PROT_WRITE,
                         MAP_SHARED, console_fd, mapped_memlen);
        if ( mapped_io == (char *)-1 ) {
            /* Hmm, failed to memory map I/O registers */
            mapped_io = NULL;
        }
    }

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer) {
#endif
        /* Query for the list of available video modes */
        current_w = vinfo.xres;
        current_h = vinfo.yres;
        current_index = ((vinfo.bits_per_pixel+7)/8)-1;
        for ( i=0; i<NUM_MODELISTS; ++i ) {
            GAL_nummodes[i] = 0;
            GAL_modelist[i] = NULL;
            for ( j=0; j<(sizeof(checkres)/sizeof(checkres[0])); ++j ) {
                unsigned int w, h;

                /* See if we are querying for the current mode */
                w = checkres[j].w;
                h = checkres[j].h;
                if ( i == current_index ) {
                    if ( (current_w > w) || (current_h > h) ) {
                        /* Only check once */
                        FB_AddMode(this, i, current_w, current_h);
                        current_index = -1;
                    }
                }
                if ( FB_CheckMode(this, &vinfo, i, &w, &h) ) {
                    FB_AddMode(this, i, w, h);
                }
            }
        }
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    }
#endif

    /* Fill in our hardware acceleration capabilities */
    this->info.hw_available = 1;
    this->info.video_mem = finfo.smem_len/1024;
    if ( mapped_io ) {
        switch (finfo.accel) {
            case FB_ACCEL_MATROX_MGA2064W:
            case FB_ACCEL_MATROX_MGA1064SG:
            case FB_ACCEL_MATROX_MGA2164W:
            case FB_ACCEL_MATROX_MGA2164W_AGP:
            case FB_ACCEL_MATROX_MGAG100:
            /*case FB_ACCEL_MATROX_MGAG200: G200 acceleration broken! */
            case FB_ACCEL_MATROX_MGAG400:
#ifdef FBACCEL_DEBUG
            fprintf(stderr, "Matrox hardware accelerator!\n");
#endif
            FB_MatroxAccel(this, finfo.accel);
            break;
            case FB_ACCEL_3DFX_BANSHEE:
#ifdef FBACCEL_DEBUG
            fprintf(stderr, "3DFX hardware accelerator!\n");
#endif
            FB_3DfxAccel (this, finfo.accel);
            break;
#ifdef FB_ACCEL_NEOMAGIC_NM2070
            case FB_ACCEL_NEOMAGIC_NM2200:
            case FB_ACCEL_NEOMAGIC_NM2230:
            case FB_ACCEL_NEOMAGIC_NM2360:
            case FB_ACCEL_NEOMAGIC_NM2380:
#ifdef FBACCEL_DEBUG
            fprintf(stderr, "NeoMagic hardware accelerator!\n");
#endif
            FB_NeoMagicAccel (this, finfo.accel);
#endif
            break;
            default:
#ifdef FBACCEL_DEBUG
            fprintf(stderr, "Unknown hardware accelerator!\n");
#endif
            break;
        }
    }

    if (FB_OpenKeyboard(this) < 0) {
        FB_VideoQuit(this);
        return(-1);
    }

    /* We're done! */
    return(0);
}

static GAL_Rect **FB_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags)
{
    return(GAL_modelist[((format->BitsPerPixel+7)/8)-1]);
}

/* Various screen update functions available */
#if 0
static void FB_DirectUpdate(_THIS, int numrects, GAL_Rect *rects);
#endif

#ifdef VGA16_FBCON_SUPPORT
static void FB_VGA16Update(_THIS, int numrects, GAL_Rect *rects);
#endif

#ifdef FBCON_DEBUG
static void print_vinfo(struct fb_var_screeninfo *vinfo)
{
    fprintf(stderr, "Printing vinfo:\n");
    fprintf(stderr, "txres: %d\n", vinfo->xres);
    fprintf(stderr, "tyres: %d\n", vinfo->yres);
    fprintf(stderr, "txres_virtual: %d\n", vinfo->xres_virtual);
    fprintf(stderr, "tyres_virtual: %d\n", vinfo->yres_virtual);
    fprintf(stderr, "txoffset: %d\n", vinfo->xoffset);
    fprintf(stderr, "tyoffset: %d\n", vinfo->yoffset);
    fprintf(stderr, "tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
    fprintf(stderr, "tgrayscale: %d\n", vinfo->grayscale);
    fprintf(stderr, "tnonstd: %d\n", vinfo->nonstd);
    fprintf(stderr, "tactivate: %d\n", vinfo->activate);
    fprintf(stderr, "theight: %d\n", vinfo->height);
    fprintf(stderr, "twidth: %d\n", vinfo->width);
    fprintf(stderr, "taccel_flags: %d\n", vinfo->accel_flags);
    fprintf(stderr, "tpixclock: %d\n", vinfo->pixclock);
    fprintf(stderr, "tleft_margin: %d\n", vinfo->left_margin);
    fprintf(stderr, "tright_margin: %d\n", vinfo->right_margin);
    fprintf(stderr, "tupper_margin: %d\n", vinfo->upper_margin);
    fprintf(stderr, "tlower_margin: %d\n", vinfo->lower_margin);
    fprintf(stderr, "thsync_len: %d\n", vinfo->hsync_len);
    fprintf(stderr, "tvsync_len: %d\n", vinfo->vsync_len);
    fprintf(stderr, "tsync: %d\n", vinfo->sync);
    fprintf(stderr, "tvmode: %d\n", vinfo->vmode);
    fprintf(stderr, "tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
    fprintf(stderr, "tgreen: %d/%d\n", vinfo->green.length, vinfo->green.offset);
    fprintf(stderr, "tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
    fprintf(stderr, "talpha: %d/%d\n", vinfo->transp.length, vinfo->transp.offset);
}
static void print_finfo(struct fb_fix_screeninfo *finfo)
{
    fprintf(stderr, "Printing finfo:\n");
    fprintf(stderr, "tsmem_start = %p\n", (char *)finfo->smem_start);
    fprintf(stderr, "tsmem_len = %d\n", finfo->smem_len);
    fprintf(stderr, "ttype = %d\n", finfo->type);
    fprintf(stderr, "ttype_aux = %d\n", finfo->type_aux);
    fprintf(stderr, "tvisual = %d\n", finfo->visual);
    fprintf(stderr, "txpanstep = %d\n", finfo->xpanstep);
    fprintf(stderr, "typanstep = %d\n", finfo->ypanstep);
    fprintf(stderr, "tywrapstep = %d\n", finfo->ywrapstep);
    fprintf(stderr, "tline_length = %d\n", finfo->line_length);
    fprintf(stderr, "tmmio_start = %p\n", (char *)finfo->mmio_start);
    fprintf(stderr, "tmmio_len = %d\n", finfo->mmio_len);
    fprintf(stderr, "taccel = %d\n", finfo->accel);
}
#endif

static int choose_fbmodes_mode(struct fb_var_screeninfo *vinfo)
{
    /* FIXME */
    return 0;

#if 0
    int matched;
    FILE *fbmodes;

    matched = 0;
    fbmodes = fopen("/etc/fb.modes", "r");
    if ( fbmodes ) {
        /* FIXME: Parse the mode definition file */
        fclose(fbmodes);
    }
    return(matched);
#endif
}

static int choose_vesa_mode(struct fb_var_screeninfo *vinfo)
{
    int matched;
    int i;

    /* Check for VESA timings */
    matched = 0;
    for ( i=0; i<(sizeof(vesa_timings)/sizeof(vesa_timings[0])); ++i ) {
        if ( (vinfo->xres == vesa_timings[i].xres) &&
             (vinfo->yres == vesa_timings[i].yres) ) {
#ifdef FBCON_DEBUG
            fprintf(stderr, "Using VESA timings for %dx%d\n",
                        vinfo->xres, vinfo->yres);
#endif
            if ( vesa_timings[i].pixclock ) {
                vinfo->pixclock = vesa_timings[i].pixclock;
            }
            vinfo->left_margin = vesa_timings[i].left;
            vinfo->right_margin = vesa_timings[i].right;
            vinfo->upper_margin = vesa_timings[i].upper;
            vinfo->lower_margin = vesa_timings[i].lower;
            vinfo->hsync_len = vesa_timings[i].hslen;
            vinfo->vsync_len = vesa_timings[i].vslen;
            vinfo->sync = vesa_timings[i].sync;
            vinfo->vmode = vesa_timings[i].vmode;
            matched = 1;
            break;
        }
    }
    return(matched);
}

#ifdef VGA16_FBCON_SUPPORT
static GAL_Surface *FB_SetVGA16Mode(_THIS, GAL_Surface *current,
                int width, int height, int bpp, Uint32 flags)
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;

    /* Set the terminal into graphics mode */
    if ( FB_EnterGraphicsMode(this) < 0 ) {
        return(NULL);
    }

    /* Restore the original palette */
    FB_RestorePalette(this);

    /* Set the video mode and get the final screen format */
    if ( ioctl(console_fd, FBIOGET_VSCREENINFO, &vinfo) < 0 ) {
        GAL_SetError("Couldn't get console screen info");
        return(NULL);
    }
    cache_vinfo = vinfo;
#ifdef FBCON_DEBUG
    fprintf(stderr, "Printing actual vinfo:\n");
    print_vinfo(&vinfo);
#endif
    if ( ! GAL_ReallocFormat(current, bpp, 0, 0, 0, 0) ) {
        return(NULL);
    }
    current->format->palette->ncolors = 16;

    /* Get the fixed information about the console hardware.
       This is necessary since finfo.line_length changes.
     */
    if ( ioctl(console_fd, FBIOGET_FSCREENINFO, &finfo) < 0 ) {
        GAL_SetError("Couldn't get console hardware info");
        return(NULL);
    }
#ifdef FBCON_DEBUG
    fprintf(stderr, "Printing actual finfo:\n");
    print_finfo(&finfo);
#endif

    /* Save hardware palette, if needed */
    FB_SavePalette(this, &finfo, &vinfo);

    /* Set up the new mode framebuffer */
    current->flags = GAL_FULLSCREEN;
    current->w = vinfo.xres;
    current->h = vinfo.yres;
    current->pitch = current->w;
    current->pixels = malloc(current->h*current->pitch);

    /* Set the update rectangle function */
    this->UpdateRects = FB_VGA16Update;

    /* We're done */
    return(current);
}
#endif /* VGA16_FBCON_SUPPORT */

static GAL_Surface *FB_SetVideoMode(_THIS, GAL_Surface *current,
                int width, int height, int bpp, Uint32 flags)
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    int i;
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    char *surfaces_mem;
    int surfaces_len;

    /* Set the terminal into graphics mode */
    if (FB_EnterGraphicsMode(this) < 0) {
        return(NULL);
    }

    /* Restore the original palette */
    FB_RestorePalette (this);

    /* Set the video mode and get the final screen format */
    if ( ioctl(console_fd, FBIOGET_VSCREENINFO, &vinfo) < 0 ) {
        GAL_SetError("Couldn't get console screen info");
        return(NULL);
    }
#ifdef FBCON_DEBUG
    fprintf(stderr, "Printing original vinfo:\n");
    print_vinfo(&vinfo);
#endif

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (!mgIsServer) {
        vinfo.xres = width;
        vinfo.yres = height;
    }
    if ( mgIsServer && ((vinfo.xres != width) || (vinfo.yres != height) ||
         (vinfo.bits_per_pixel != bpp) /* || (flags & GAL_DOUBLEBUF) */) ) {
#else
    if ( ((vinfo.xres != width) || (vinfo.yres != height) ||
         (vinfo.bits_per_pixel != bpp) /* || (flags & GAL_DOUBLEBUF) */) ) {
#endif
        vinfo.activate = FB_ACTIVATE_NOW;
        vinfo.accel_flags = 0;
        vinfo.bits_per_pixel = bpp;
        vinfo.xres = width;
        vinfo.xres_virtual = width;
        vinfo.yres = height;
#if 0
        if ( flags & GAL_DOUBLEBUF ) {
            vinfo.yres_virtual = height*2;
        } else {
            vinfo.yres_virtual = height;
        }
#else
        vinfo.yres_virtual = height;
#endif
        vinfo.xoffset = 0;
        vinfo.yoffset = 0;
        vinfo.red.length = vinfo.red.offset = 0;
        vinfo.green.length = vinfo.green.offset = 0;
        vinfo.blue.length = vinfo.blue.offset = 0;
        vinfo.transp.length = vinfo.transp.offset = 0;
        if ( ! choose_fbmodes_mode(&vinfo) && vinfo.xres >= 320 ) {
            choose_vesa_mode(&vinfo);
        }
#ifdef FBCON_DEBUG
        fprintf(stderr, "Printing wanted vinfo:\n");
        print_vinfo(&vinfo);
#endif
        if ( ioctl(console_fd, FBIOPUT_VSCREENINFO, &vinfo) < 0 ) {
            vinfo.yres_virtual = height;
            if ( ioctl(console_fd, FBIOPUT_VSCREENINFO, &vinfo) < 0 ) {
                GAL_SetError("Couldn't set console screen info");
                return(NULL);
            }
        }
    } else {
        int maxheight;

        /* Figure out how much video memory is available */
#if 0
        if ( flags & GAL_DOUBLEBUF ) {
            maxheight = height*2;
        } else {
            maxheight = height;
        }
#else
        maxheight = height;
#endif
        if ( vinfo.yres_virtual > maxheight ) {
            vinfo.yres_virtual = maxheight;
        }
    }
    cache_vinfo = vinfo;
#ifdef FBCON_DEBUG
    fprintf (stderr, "Printing actual vinfo:\n");
    print_vinfo(&vinfo);
#endif
    Rmask = 0;
    for ( i=0; i<vinfo.red.length; ++i ) {
        Rmask <<= 1;
        Rmask |= (0x00000001<<vinfo.red.offset);
    }
    Gmask = 0;
    for ( i=0; i<vinfo.green.length; ++i ) {
        Gmask <<= 1;
        Gmask |= (0x00000001<<vinfo.green.offset);
    }
    Bmask = 0;
    for ( i=0; i<vinfo.blue.length; ++i ) {
        Bmask <<= 1;
        Bmask |= (0x00000001<<vinfo.blue.offset);
    }

    if (!GAL_ReallocFormat(current, vinfo.bits_per_pixel,
                                      Rmask, Gmask, Bmask, 0) ) {
        return(NULL);
    }

    /* Get the fixed information about the console hardware.
       This is necessary since finfo.line_length changes.
     */
    if ( ioctl(console_fd, FBIOGET_FSCREENINFO, &finfo) < 0 ) {
        GAL_SetError("Couldn't get console hardware info");
        return(NULL);
    }

    /* Save hardware palette, if needed */
    FB_SavePalette(this, &finfo, &vinfo);

    /* Set up the new mode framebuffer */
    current->flags = (GAL_FULLSCREEN|GAL_HWSURFACE);
    current->w = vinfo.xres;
    current->h = vinfo.yres;
    current->pitch = finfo.line_length;
    current->pixels = mapped_mem+mapped_offset;

    /* Set up the information for hardware surfaces */
    surfaces_mem = (char *)current->pixels +
                            vinfo.yres_virtual*current->pitch;
    surfaces_len = (mapped_memlen-(surfaces_mem-mapped_mem));

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer)
#endif
    {
        FB_FreeHWSurfaces(this);
        FB_InitHWSurfaces(this, current, surfaces_mem, surfaces_len);
    }
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    else {
        current->hwdata = NULL;
    }
#endif

    /* Let the application know we have a hardware palette */
    switch (finfo.visual) {
        case FB_VISUAL_PSEUDOCOLOR:
        current->flags |= GAL_HWPALETTE;
        break;
        default:
        break;
    }

#if 0
    /* Update for double-buffering, if we can */
    if ( flags & GAL_DOUBLEBUF ) {
        if ( vinfo.yres_virtual == (height*2) ) {
            current->flags |= GAL_DOUBLEBUF;
            flip_page = 0;
            flip_address[0] = (char *)current->pixels;
            flip_address[1] = (char *)current->pixels+
                                      current->h*current->pitch;
            this->screen = current;
            FB_FlipHWSurface(this, current);
            this->screen = NULL;
        }
    }

    /* Set the update rectangle function */
    this->UpdateRects = FB_DirectUpdate;

#endif

    /* We're done */
    return(current);
}

#ifdef FBCON_DEBUG
static void FB_DumpHWSurfaces(_THIS)
{
    vidmem_bucket *bucket;

    fprintf(stderr, "Memory left: %d (%d total)\n", surfaces_memleft, surfaces_memtotal);
    fprintf(stderr, "\n");
    fprintf(stderr, "         Base  Size\n");
    for ( bucket=&surfaces; bucket; bucket=bucket->next ) {
        fprintf(stderr, "Bucket:  %p, %d (%s)\n", bucket->base, bucket->size, bucket->used ? "used" : "free");
        if ( bucket->prev ) {
            if ( bucket->base != bucket->prev->base+bucket->prev->size ) {
                fprintf(stderr, "Warning, corrupt bucket list! (prev)\n");
            }
        } else {
            if ( bucket != &surfaces ) {
                fprintf(stderr, "Warning, corrupt bucket list! (!prev)\n");
            }
        }
        if ( bucket->next ) {
            if ( bucket->next->base != bucket->base+bucket->size ) {
                fprintf(stderr, "Warning, corrupt bucket list! (next)\n");
            }
        }
    }
    fprintf(stderr, "\n");
}
#endif

static int FB_InitHWSurfaces(_THIS, GAL_Surface *screen, char *base, int size)
{
    vidmem_bucket *bucket;

    surfaces_memtotal = size;
    surfaces_memleft = size;

    if ( surfaces_memleft > 0 ) {
        bucket = (vidmem_bucket *)malloc(sizeof(*bucket));
        if ( bucket == NULL ) {
            GAL_OutOfMemory();
            return(-1);
        }
        bucket->prev = &surfaces;
        bucket->used = 0;
        bucket->dirty = 0;
        bucket->base = base;
        bucket->size = size;
        bucket->next = NULL;
    } else {
        bucket = NULL;
    }

    surfaces.prev = NULL;
    surfaces.used = 1;
    surfaces.dirty = 0;
    surfaces.base = screen->pixels;
    surfaces.size = (unsigned int)((long)base - (long)surfaces.base);
    surfaces.next = bucket;
    screen->hwdata = (struct private_hwdata *)&surfaces;
    return(0);
}

static void FB_FreeHWSurfaces(_THIS)
{
    vidmem_bucket *bucket, *freeable;

    bucket = surfaces.next;
    while ( bucket ) {
        freeable = bucket;
        bucket = bucket->next;
        free(freeable);
    }
    surfaces.next = NULL;
}

static void FB_RequestHWSurface (_THIS, const REQ_HWSURFACE* request, REP_HWSURFACE* reply)
{
    if (request->bucket == NULL) {     /* alloc hw surface */
        vidmem_bucket *bucket;
        int size;
        int extra;

        reply->bucket = NULL;
        /* Temporarily, we only allow surfaces the same width as display.
           Some blitters require the pitch between two hardware surfaces
           to be the same.  Others have interesting alignment restrictions.
           Until someone who knows these details looks at the code...
        */
        if (request->pitch > GAL_VideoSurface->pitch) {
            GAL_SetError("Surface requested wider than screen");
            return;
        }

        reply->pitch = GAL_VideoSurface->pitch;
        size = request->h * reply->pitch;
#ifdef FBCON_DEBUG
        fprintf(stderr, "Allocating bucket of %d bytes\n", size);
#endif

        /* Quick check for available mem */
        if ( size > surfaces_memleft ) {
            GAL_SetError("Not enough video memory");
            return;
        }

        /* Search for an empty bucket big enough */
        for ( bucket=&surfaces; bucket; bucket=bucket->next ) {
            if ( ! bucket->used && (size <= bucket->size) ) {
                break;
            }
        }
        if ( bucket == NULL ) {
            GAL_SetError("Video memory too fragmented");
            return;
        }

        /* Create a new bucket for left-over memory */
        extra = (bucket->size - size);
        if ( extra ) {
            vidmem_bucket *newbucket;

#ifdef FBCON_DEBUG
            fprintf(stderr, "Adding new free bucket of %d bytes\n", extra);
#endif
            newbucket = (vidmem_bucket *)malloc(sizeof(*newbucket));
            if ( newbucket == NULL ) {
                GAL_OutOfMemory();
                return;
            }
            newbucket->prev = bucket;
            newbucket->used = 0;
            newbucket->base = bucket->base + size;
            newbucket->size = extra;
            newbucket->next = bucket->next;
            if ( bucket->next ) {
                bucket->next->prev = newbucket;
            }
            bucket->next = newbucket;
        }

        /* Set the current bucket values and return it! */
        bucket->used = 1;
        bucket->size = size;
        bucket->dirty = 0;
#ifdef FBCON_DEBUG
        fprintf(stderr, "Allocated %d bytes at %p\n", bucket->size, bucket->base);
#endif
        surfaces_memleft -= size;

        reply->bucket = bucket;
        reply->offset = bucket->base - mapped_mem;
    }
    else {  /* free hw surface */
        vidmem_bucket *bucket, *freeable;

        /* Look for the bucket in the current list */
        for ( bucket=&surfaces; bucket; bucket=bucket->next ) {
            if ( bucket == (vidmem_bucket *)request->bucket) {
                break;
            }
        }
        if ( bucket && bucket->used ) {
        /* Add the memory back to the total */
#ifdef FBCON_DEBUG
            fprintf(stderr, "Freeing bucket of %d bytes\n", bucket->size);
#endif
            surfaces_memleft += bucket->size;

            /* Can we merge the space with surrounding buckets? */
            bucket->used = 0;
            if ( bucket->next && ! bucket->next->used ) {
#ifdef FBCON_DEBUG
                fprintf(stderr, "Merging with next bucket, for %d total bytes\n", bucket->size+bucket->next->size);
#endif
                freeable = bucket->next;
                bucket->size += bucket->next->size;
                bucket->next = bucket->next->next;
                if ( bucket->next ) {
                    bucket->next->prev = bucket;
                }
                free(freeable);
            }
            if ( bucket->prev && ! bucket->prev->used ) {
#ifdef FBCON_DEBUG
                fprintf(stderr, "Merging with previous bucket, for %d total bytes\n", bucket->prev->size+bucket->size);
#endif
                freeable = bucket;
                bucket->prev->size += bucket->size;
                bucket->prev->next = bucket->next;
                if ( bucket->next ) {
                    bucket->next->prev = bucket->prev;
                }
                free(freeable);
            }
        }
    }
}

static int FB_AllocHWSurface (_THIS, GAL_Surface *surface)
{
    REQ_HWSURFACE request = {surface->w, surface->h, surface->pitch, 0, NULL};
    REP_HWSURFACE reply = {0, 0, NULL};

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer)
        FB_RequestHWSurface (this, &request, &reply);
    else {
        REQUEST req;

        req.id = REQID_HWSURFACE;
        req.data = &request;
        req.len_data = sizeof (REQ_HWSURFACE);

        cli_request (&req, &reply, sizeof (REP_HWSURFACE));
    }
#else
    FB_RequestHWSurface (this, &request, &reply);
#endif

    if (reply.bucket == NULL)
        return -1;

    surface->flags |= GAL_HWSURFACE;
    surface->pitch = reply.pitch;
    surface->pixels = mapped_mem + reply.offset;
    surface->hwdata = (struct private_hwdata *)reply.bucket;
    return 0;
}

static void FB_FreeHWSurface(_THIS, GAL_Surface *surface)
{
    REQ_HWSURFACE request = {surface->w, surface->h, surface->pitch, 0, surface->hwdata};
    REP_HWSURFACE reply = {0, 0};

    request.offset = (char*)surface->pixels - (char*)mapped_mem;

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer)
        FB_RequestHWSurface (this, &request, &reply);
    else {
        REQUEST req;

        if (surface->hwdata == NULL) {
            surface->pixels = NULL;
            return;
        }

        request.bucket = surface->hwdata;

        req.id = REQID_HWSURFACE;
        req.data = &request;
        req.len_data = sizeof (REQ_HWSURFACE);

        cli_request (&req, &reply, sizeof (REP_HWSURFACE));
    }
#else
    FB_RequestHWSurface (this, &request, &reply);
#endif

    surface->pixels = NULL;
    surface->hwdata = NULL;
}

#if 0
static int FB_LockHWSurface(_THIS, GAL_Surface *surface)
{
    if ( surface == this->screen ) {
        GAL_mutexP(hw_lock);
        if ( FB_IsSurfaceBusy(surface) ) {
            FB_WaitBusySurfaces(this);
        }
    } else {
        if ( FB_IsSurfaceBusy(surface) ) {
            FB_WaitBusySurfaces(this);
        }
    }
    return(0);
}
static void FB_UnlockHWSurface(_THIS, GAL_Surface *surface)
{
    if ( surface == this->screen ) {
        GAL_mutexV(hw_lock);
    }
}

static int FB_FlipHWSurface(_THIS, GAL_Surface *surface)
{
    /* Wait for vertical retrace and then flip display */
    cache_vinfo.yoffset = flip_page*surface->h;
    if ( FB_IsSurfaceBusy(this->screen) ) {
        FB_WaitBusySurfaces(this);
    }
    wait_vbl(this);
    if ( ioctl(console_fd, FBIOPAN_DISPLAY, &cache_vinfo) < 0 ) {
        GAL_SetError("ioctl(FBIOPAN_DISPLAY) failed");
        return(-1);
    }
    flip_page = !flip_page;

    surface->pixels = flip_address[flip_page];
    return(0);
}

static void FB_DirectUpdate(_THIS, int numrects, GAL_Rect *rects)
{
    /* The application is already updating the visible video memory */
    return;
}

#endif

static void FB_WaitVBL(_THIS)
{
#ifdef FBIOWAITRETRACE /* Heheh, this didn't make it into the main kernel */
    ioctl(console_fd, FBIOWAITRETRACE, 0);
#endif
    return;
}

static void FB_WaitIdle(_THIS)
{
    return;
}

#ifdef VGA16_FBCON_SUPPORT
/* Code adapted with thanks from the XFree86 VGA16 driver! :) */
#define writeGr(index, value)   \
outb(index, 0x3CE);             \
outb(value, 0x3CF);

#define writeSeq(index, value)  \
outb(index, 0x3C4);             \
outb(value, 0x3C5);

static void FB_VGA16Update(_THIS, int numrects, GAL_Rect *rects)
{
    GAL_Surface *screen;
    int width, height, FBPitch, left, i, j, SRCPitch, phase;
    register Uint32 m;
    Uint8  s1, s2, s3, s4;
    Uint32 *src, *srcPtr;
    Uint8  *dst, *dstPtr;

    screen = this->screen;
    FBPitch = screen->w >> 3;
    SRCPitch = screen->pitch >> 2;

    writeGr(0x03, 0x00);
    writeGr(0x05, 0x00);
    writeGr(0x01, 0x00);
    writeGr(0x08, 0xFF);

    while(numrects--) {
    left = rects->x & ~7;
        width = (rects->w + 7) >> 3;
        height = rects->h;
        src = (Uint32*)screen->pixels + (rects->y * SRCPitch) + (left >> 2); 
        dst = (Uint8*)mapped_mem + (rects->y * FBPitch) + (left >> 3);

    if((phase = (long)dst & 3L)) {
        phase = 4 - phase;
        if(phase > width) phase = width;
        width -= phase;
    }

        while(height--) {
        writeSeq(0x02, 1 << 0);
        dstPtr = dst;
        srcPtr = src;
        i = width;
        j = phase;
        while(j--) {
        m = (srcPtr[1] & 0x01010101) | ((srcPtr[0] & 0x01010101) << 4);
         *dstPtr++ = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
        srcPtr += 2;
        }
        while(i >= 4) {
        m = (srcPtr[1] & 0x01010101) | ((srcPtr[0] & 0x01010101) << 4);
         s1 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
        m = (srcPtr[3] & 0x01010101) | ((srcPtr[2] & 0x01010101) << 4);
         s2 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
        m = (srcPtr[5] & 0x01010101) | ((srcPtr[4] & 0x01010101) << 4);
         s3 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
        m = (srcPtr[7] & 0x01010101) | ((srcPtr[6] & 0x01010101) << 4);
         s4 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
        *((Uint32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
        srcPtr += 8;
        dstPtr += 4;
        i -= 4;
        }
        while(i--) {
        m = (srcPtr[1] & 0x01010101) | ((srcPtr[0] & 0x01010101) << 4);
         *dstPtr++ = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
        srcPtr += 2;
        }

        writeSeq(0x02, 1 << 1);
        dstPtr = dst;
        srcPtr = src;
        i = width;
        j = phase;
        while(j--) {
        m = (srcPtr[1] & 0x02020202) | ((srcPtr[0] & 0x02020202) << 4);
         *dstPtr++ = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
        srcPtr += 2;
        }
        while(i >= 4) {
        m = (srcPtr[1] & 0x02020202) | ((srcPtr[0] & 0x02020202) << 4);
         s1 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
        m = (srcPtr[3] & 0x02020202) | ((srcPtr[2] & 0x02020202) << 4);
         s2 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
        m = (srcPtr[5] & 0x02020202) | ((srcPtr[4] & 0x02020202) << 4);
         s3 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
        m = (srcPtr[7] & 0x02020202) | ((srcPtr[6] & 0x02020202) << 4);
         s4 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
        *((Uint32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
        srcPtr += 8;
        dstPtr += 4;
        i -= 4;
        }
        while(i--) {
        m = (srcPtr[1] & 0x02020202) | ((srcPtr[0] & 0x02020202) << 4);
         *dstPtr++ = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
        srcPtr += 2;
        }

        writeSeq(0x02, 1 << 2);
        dstPtr = dst;
        srcPtr = src;
        i = width;
        j = phase;
        while(j--) {
        m = (srcPtr[1] & 0x04040404) | ((srcPtr[0] & 0x04040404) << 4);
         *dstPtr++ = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
        srcPtr += 2;
        }
        while(i >= 4) {
        m = (srcPtr[1] & 0x04040404) | ((srcPtr[0] & 0x04040404) << 4);
         s1 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
        m = (srcPtr[3] & 0x04040404) | ((srcPtr[2] & 0x04040404) << 4);
         s2 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
        m = (srcPtr[5] & 0x04040404) | ((srcPtr[4] & 0x04040404) << 4);
         s3 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
        m = (srcPtr[7] & 0x04040404) | ((srcPtr[6] & 0x04040404) << 4);
         s4 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
        *((Uint32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
        srcPtr += 8;
        dstPtr += 4;
        i -= 4;
        }
        while(i--) {
        m = (srcPtr[1] & 0x04040404) | ((srcPtr[0] & 0x04040404) << 4);
         *dstPtr++ = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
        srcPtr += 2;
        }
        
        writeSeq(0x02, 1 << 3);
        dstPtr = dst;
        srcPtr = src;
        i = width;
        j = phase;
        while(j--) {
        m = (srcPtr[1] & 0x08080808) | ((srcPtr[0] & 0x08080808) << 4);
         *dstPtr++ = (m >> 27) | (m >> 18) | (m >> 9) | m;
        srcPtr += 2;
        }
        while(i >= 4) {
        m = (srcPtr[1] & 0x08080808) | ((srcPtr[0] & 0x08080808) << 4);
         s1 = (m >> 27) | (m >> 18) | (m >> 9) | m;
        m = (srcPtr[3] & 0x08080808) | ((srcPtr[2] & 0x08080808) << 4);
         s2 = (m >> 27) | (m >> 18) | (m >> 9) | m;
        m = (srcPtr[5] & 0x08080808) | ((srcPtr[4] & 0x08080808) << 4);
         s3 = (m >> 27) | (m >> 18) | (m >> 9) | m;
        m = (srcPtr[7] & 0x08080808) | ((srcPtr[6] & 0x08080808) << 4);
         s4 = (m >> 27) | (m >> 18) | (m >> 9) | m;
        *((Uint32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
        srcPtr += 8;
        dstPtr += 4;
        i -= 4;
        }
        while(i--) {
        m = (srcPtr[1] & 0x08080808) | ((srcPtr[0] & 0x08080808) << 4);
         *dstPtr++ = (m >> 27) | (m >> 18) | (m >> 9) | m;
        srcPtr += 2;
        }

            dst += FBPitch;
            src += SRCPitch;
        }
        rects++;
    }
}
#endif /* VGA16_FBCON_SUPPORT */

void FB_SavePaletteTo(_THIS, int palette_len, __u16 *area)
{
    struct fb_cmap cmap;

    cmap.start = 0;
    cmap.len = palette_len;
    cmap.red = &area[0*palette_len];
    cmap.green = &area[1*palette_len];
    cmap.blue = &area[2*palette_len];
    cmap.transp = NULL;
    ioctl(console_fd, FBIOGETCMAP, &cmap);
}

void FB_RestorePaletteFrom(_THIS, int palette_len, __u16 *area)
{
    struct fb_cmap cmap;

    cmap.start = 0;
    cmap.len = palette_len;
    cmap.red = &area[0*palette_len];
    cmap.green = &area[1*palette_len];
    cmap.blue = &area[2*palette_len];
    cmap.transp = NULL;
    ioctl(console_fd, FBIOPUTCMAP, &cmap);
}

static void FB_SavePalette(_THIS, struct fb_fix_screeninfo *finfo,
                                  struct fb_var_screeninfo *vinfo)
{
    int i;

    /* Save hardware palette, if needed */
    if ( finfo->visual == FB_VISUAL_PSEUDOCOLOR ) {
        saved_cmaplen = 1<<vinfo->bits_per_pixel;
        saved_cmap=(__u16 *)malloc(3*saved_cmaplen*sizeof(*saved_cmap));
        if ( saved_cmap != NULL ) {
            FB_SavePaletteTo(this, saved_cmaplen, saved_cmap);
        }
    }

    /* Added support for FB_VISUAL_DIRECTCOLOR.
       With this mode pixel information is passed through the palette...
       Neat fading and gamma correction effects can be had by simply
       fooling around with the palette instead of changing the pixel
       values themselves... Very neat!

       Adam Meyerowitz 1/19/2000
       ameyerow@optonline.com
    */
    if ( finfo->visual == FB_VISUAL_DIRECTCOLOR ) {
        __u16 new_entries[3*256];

        /* Save the colormap */
        saved_cmaplen = 256;
        saved_cmap=(__u16 *)malloc(3*saved_cmaplen*sizeof(*saved_cmap));
        if ( saved_cmap != NULL ) {
            FB_SavePaletteTo(this, saved_cmaplen, saved_cmap);
        }

        /* Allocate new identity colormap */
        for ( i=0; i<256; ++i ) {
                  new_entries[(0*256)+i] =
            new_entries[(1*256)+i] =
            new_entries[(2*256)+i] = (i<<8)|i;
        }
        FB_RestorePaletteFrom(this, 256, new_entries);
    }
}

static void FB_RestorePalette(_THIS)
{
    /* Restore the original palette */
    if ( saved_cmap ) {
        FB_RestorePaletteFrom(this, saved_cmaplen, saved_cmap);
        free(saved_cmap);
        saved_cmap = NULL;
    }
}

static int FB_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors)
{
    int i;
    __u16 r[256];
    __u16 g[256];
    __u16 b[256];
    struct fb_cmap cmap;

    /* Set up the colormap */
    for (i = 0; i < ncolors; i++) {
        r[i] = colors[i].r << 8;
        g[i] = colors[i].g << 8;
        b[i] = colors[i].b << 8;
    }
    cmap.start = firstcolor;
    cmap.len = ncolors;
    cmap.red = r;
    cmap.green = g;
    cmap.blue = b;
    cmap.transp = NULL;

    if( (ioctl(console_fd, FBIOPUTCMAP, &cmap) < 0) ||
        !(this->screen->flags & GAL_HWPALETTE) ) {
            colors = this->screen->format->palette->colors;
        ncolors = this->screen->format->palette->ncolors;
        cmap.start = 0;
        cmap.len = ncolors;
        memset(r, 0, sizeof(r));
        memset(g, 0, sizeof(g));
        memset(b, 0, sizeof(b));
        if ( ioctl(console_fd, FBIOGETCMAP, &cmap) == 0 ) {
            for ( i=ncolors-1; i>=0; --i ) {
                colors[i].r = (r[i]>>8);
                colors[i].g = (g[i]>>8);
                colors[i].b = (b[i]>>8);
            }
        }
        return(0);
    }
    return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
static void FB_VideoQuit(_THIS)
{
    int i, j;

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if ( mgIsServer && this->screen ) {
#else
    if ( this->screen ) {
#endif
        /* Clear screen and tell SDL not to free the pixels */
        if ( this->screen->pixels ) {
#ifdef __powerpc__    /* SIGBUS when using memset() ?? */
            Uint8 *rowp = (Uint8 *)this->screen->pixels;
            int left = this->screen->pitch*this->screen->h;
            while ( left-- ) { *rowp++ = 0; }
#else
            memset(this->screen->pixels,0,this->screen->h*this->screen->pitch);
#endif
        }
        /* This test fails when using the VGA16 shadow memory */
        if ( ((char *)this->screen->pixels >= mapped_mem) &&
             ((char *)this->screen->pixels < (mapped_mem+mapped_memlen)) ) {
            this->screen->pixels = NULL;
        }
    }

    /* Clear the lock mutex */
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if ( mgIsServer && hw_lock ) {
#else
    if ( hw_lock ) {
#endif
        GAL_DestroyMutex(hw_lock);
        hw_lock = NULL;
    }

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer) {
#endif
    /* Clean up defined video modes */
        for ( i=0; i<NUM_MODELISTS; ++i ) {
            if ( GAL_modelist[i] != NULL ) {
                for ( j=0; GAL_modelist[i][j]; ++j ) {
                    free(GAL_modelist[i][j]);
                }
                free(GAL_modelist[i]);
                GAL_modelist[i] = NULL;
            }
        }
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    }
#endif

    /* Clean up the memory bucket list */
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (mgIsServer)
#endif
    {
        FB_FreeHWSurfaces(this);
    }

    /* Close console and input file descriptors */
    if ( console_fd > 0 ) {
        /* Unmap the video framebuffer and I/O registers */
        if ( mapped_mem ) {
            munmap(mapped_mem, mapped_memlen);
            mapped_mem = NULL;
        }
        if ( mapped_io ) {
            munmap(mapped_io, mapped_iolen);
            mapped_io = NULL;
        }

        /* Restore the original video mode and palette */
        if (ttyfd >= 0) {
            FB_RestorePalette(this);
            ioctl(console_fd, FBIOPUT_VSCREENINFO, &saved_vinfo);
        }

        FB_LeaveGraphicsMode (this);

        /* We're all done with the framebuffer */
        close(console_fd);
        console_fd = -1;
    }
}

