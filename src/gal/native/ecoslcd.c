/*
** $Id: ecoslcd.c,v 1.1 2003/11/22 12:00:19 weiym Exp $
** 
** qvfb.c: GAL driver for Qt Virtual FrameBuffer.
** 
** Copyright (C) 2003 Feynman Software.
**
** Create Date: 2003/07/04 by Wei Yongming
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
#include <stdarg.h>
#include <unistd.h>

#include "native.h"

#ifdef _NATIVE_GAL_ECOSLCD

#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>  
#include <cyg/hal/lcd_support.h>

#include "fb.h"

/* init framebuffer*/
static PSD fb_open(PSD psd)
{
    struct lcd_info li;
    PSUBDRIVER subdriver;

    /* Initialize LCD screen */
    lcd_init (16);
    lcd_getinfo (&li);

    psd->planes = 1;
    psd->xres = li.width;
    psd->yres = li.height;
    psd->bpp = li.bpp;
    psd->linelen = li.rlen;
    psd->ncolors = (psd->bpp >= 24) ? (1 << 24) : (1 << psd->bpp);

    psd->addr = li.fb;
    psd->gr_mode = MODE_SET;

    psd->size = 0;                /* force subdriver init of size */
    psd->flags = PSF_MEMORY;
    psd->flags |= PSF_MSBRIGHT; /* the most significant bit is right */

    /* set pixel format*/
    switch (psd->bpp) {
        case 16:
            psd->pixtype = PF_TRUECOLOR565;
            break;
        default:
            fprintf(stderr, "GAL eCos LCD engine: Unsupported FrameBuffer\n");
            goto fail;
    }

    /* select a framebuffer subdriver based on planes and bpp*/
    subdriver = select_fb_subdriver (psd);
    if (!subdriver) {
        fprintf(stderr,"GAL eCos LCD engine: No driver for bpp %d\n", psd->bpp);
        goto fail;
    }

    /*
     * set and initialize subdriver into screen driver
     * psd->size is calculated by subdriver init
     */
    if (!set_subdriver (psd, subdriver, TRUE)) {
        fprintf(stderr,"GAL eCos LCD engine: Driver initialize failed for bpp %d\n", psd->bpp);
        goto fail;
    }

    return psd;

fail:
    return NULL;
}

/* close framebuffer*/
static void fb_close (PSD psd)
{
}

static void fb_setpalette (PSD psd, int first, int count, GAL_Color *palette)
{
}

static void fb_getpalette (PSD psd, int first, int count, GAL_Color *palette)
{
}

SCREENDEVICE ecoslcd = {
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 
    NULL, NULL,
    fb_open,
    fb_close,
    fb_setpalette,
    fb_getpalette,
    native_gen_allocatememgc,
    fb_mapmemgc,
    native_gen_freememgc,
    native_gen_clippoint,
    native_gen_fillrect,
    NULL,           /* DrawPixel subdriver */
    NULL,           /* ReadPixel subdriver */
    NULL,           /* DrawHLine subdriver */
    NULL,           /* DrawVLine subdriver */
    NULL,           /* Blit subdriver */
    NULL,           /* PutBox subdriver */
    NULL,           /* GetBox subdriver */
    NULL,           /* PutBoxMask subdriver */
    NULL,           /* CopyBox subdriver */
    NULL            /* UpdateRect */
};

#endif /* _NATIVE_GAL_QVFB */

