/*
** $Id: scr_fb.c,v 1.26 2003/11/22 11:49:29 weiym Exp $
** 
** scr_fb.c: GAL driver for Linux kernel FrameBuffers.
** 
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 Song Lixin.
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

#ifdef _NATIVE_GAL_FBCON

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include "fb.h"

#ifdef _LITE_VERSION
extern BOOL mgIsServer;
#endif

#ifndef FB_TYPE_VGA_PLANES
#define FB_TYPE_VGA_PLANES 4
#endif

/* static variables*/
static int fb;			/* Framebuffer file handle. */
static int status;		/* 0=never inited, 1=once inited, 2=inited. */
static short saved_red[16];	/* original hw palette*/
static short saved_green[16];
static short saved_blue[16];

#ifdef _HAVE_TEXT_MODE
static int tty = -1;
#endif

/* init framebuffer*/
static PSD fb_open(PSD psd)
{
	char *	env;
	int	type, visual;
	PSUBDRIVER subdriver;
	struct fb_fix_screeninfo fb_fix;
	struct fb_var_screeninfo fb_var;

	/* locate and open framebuffer, get info*/
	if(!(env = getenv ("FRAMEBUFFER")))
		env = "/dev/fb0";
	fb = open(env, O_RDWR);
	if(fb < 0) {
		fprintf(stderr,"GAL fbcon engine: Error when opening %s: %m. Please check kernel config.\n", env);
		return NULL;
	}
	if(ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix) == -1 ||
		ioctl(fb, FBIOGET_VSCREENINFO, &fb_var) == -1) {
			fprintf(stderr,"GAL fbcon engine: Error when reading screen info: %m.\n");
			goto fail;
	}
	/* setup screen device from framebuffer info*/
	type = fb_fix.type;
	visual = fb_fix.visual;

	psd->xres = psd->xvirtres = fb_var.xres;
	psd->yres = psd->yvirtres = fb_var.yres;

	/* set planes from fb type*/
	if (type == FB_TYPE_VGA_PLANES)
		psd->planes = 4;
	else if (type == FB_TYPE_PACKED_PIXELS)
		psd->planes = 1;
	else psd->planes = 0;	/* force error later*/

	psd->bpp = fb_var.bits_per_pixel;
	psd->ncolors = (psd->bpp >= 24)? (1 << 24): (1 << psd->bpp);

	/* set linelen to byte length, possibly converted later*/
	psd->linelen = (psd->xres * psd->bpp) >> 3;

    /*
     * Some framebuffer drivers give wrong line_length value.
     * If line_length < xres_virtual * bpp, it's certainly wrong.
     * But how do i know if line_length > xres_virtual * bpp.
     * God bless me!
     *          James Liu
     */
    if (fb_fix.line_length > psd->linelen)
        psd->linelen = fb_fix.line_length;

	psd->size = 0;		/* force subdriver init of size*/

	psd->flags = PSF_SCREEN;

    /*
     * For 1bpp, 2bpp and 4bpp framebuffer, some systems have
     * different bit order. That means the highest bits represent
     * the first pixel or the lowest bites represent first pixel.
     * 
     * For example:
     * 
     * EP7211 2bpp:                    Byte 1                                 Byte 2
     *         | D0  D1 | D2  D3 | D4  D5 | D6  D7 |  | D0  D1 | D2  D3 | D4  D5 | D6  D7 |
     *         | pixel0 | pixel1 | pixel2 | pixel3 |  | pixel4 | pixel5 | pixel6 | pixel7 |
     *         
     * Helio 2bpp:                  Byte 1                                 Byte 2
     *         | D0  D1 | D2  D3 | D4  D5 | D6  D7 |  | D0  D1 | D2  D3 | D4  D5 | D6  D7 |
     *         | pixel3 | pixel2 | pixel1 | pixel0 |  | pixel7 | pixel6 | pixel5 | pixel4 |
     */
    if (fb_var.red.msb_right)
        psd->flags |= PSF_MSBRIGHT;

	/* set pixel format*/
	if (visual == FB_VISUAL_TRUECOLOR || visual == FB_VISUAL_DIRECTCOLOR) {
		switch(psd->bpp) {
		case 8:
			psd->pixtype = PF_TRUECOLOR332;
		break;
		case 16:
			psd->pixtype = PF_TRUECOLOR565;
		break;
		case 24:
			psd->pixtype = PF_TRUECOLOR888;
		break;
		case 32:
			psd->pixtype = PF_TRUECOLOR0888;
		break;
		default:
			fprintf(stderr, "GAL fbcon engine: Unsupported FrameBuffer type\n");
			goto fail;
		}
	}
    else 
        psd->pixtype = PF_PALETTE;

	/* select a framebuffer subdriver based on planes and bpp*/
	subdriver = select_fb_subdriver(psd);
	if (!subdriver) {
		fprintf(stderr,"GAL fbcon engine: No driver for screen type %d visual %d bpp %d\n",
			type, visual, psd->bpp);
		goto fail;
	}

	/*
	 * set and initialize subdriver into screen driver
	 * psd->size is calculated by subdriver init
	 */
	if(!set_subdriver(psd, subdriver, TRUE)) {
		fprintf(stderr,"GAL fbcon engine: Driver initialize failed type %d visual %d bpp %d\n",
			type, visual, psd->bpp);
		goto fail;
	}

#ifdef _HAVE_TEXT_MODE
#ifdef _LITE_VERSION
    if (mgIsServer)
#endif
    {
	    /* open tty, enter graphics mode*/
        char* tty_dev;
        if (geteuid() == 0)
            tty_dev = "/dev/tty0";
        else    /* not a super user, so try to open the control terminal */
            tty_dev = "/dev/tty";

	    tty = open (tty_dev, O_RDWR);
	    if(tty < 0) {
		    fprintf(stderr,"GAL fbcon engine: Can't open /dev/tty0: %m\n");
		    goto fail;
	    }
	    if(ioctl (tty, KDSETMODE, KD_GRAPHICS) == -1) {
		    fprintf(stderr,"GAL fbcon engine: Error when setting console to graphics mode: %m\n");
		    fprintf(stderr,"GAL fbcon engine: Maybe have no enough permission.\n");
		    goto fail;
	    }
    }
#endif

	/* mmap framebuffer into this address space*/
	psd->size = (psd->size + getpagesize () - 1)
			/ getpagesize () * getpagesize ();
#ifdef __uClinux__
	psd->addr = mmap(NULL, psd->size, PROT_READ | PROT_WRITE, 0, fb, 0);
#else
	psd->addr = mmap(NULL, psd->size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
#endif
	if(psd->addr == NULL || psd->addr == (unsigned char *)-1) {
		fprintf(stderr,"GAL fbcon engine: Error when mmaping %s: %m\n", env);
		goto fail;
	}
	/* save original palette*/
	ioctl_getpalette(0, 16, saved_red, saved_green, saved_blue);
	status = 2;
	psd->gr_mode = MODE_SET;
	return psd;	/* success*/

fail:

#ifdef _HAVE_TEXT_MODE
#ifdef _LITE_VERSION
    if (mgIsServer) {
#endif
	    /* enter text mode*/
        if (tty >= 0) {
	        ioctl (tty, KDSETMODE, KD_TEXT);
	        close (tty);
            tty = -1;
        }
#ifdef _LITE_VERSION
    }
#endif
#endif
	close(fb);
	return NULL;
}

/* close framebuffer*/
static void fb_close(PSD psd)
{
	/* if not opened, return*/
	if(status != 2)
		return;
	status = 1;

  	/* reset hw palette*/
	ioctl_setpalette(0, 16, saved_red, saved_green, saved_blue);
  
	/* unmap framebuffer*/
	munmap(psd->addr, psd->size);
  
#ifdef _HAVE_TEXT_MODE
#ifdef _LITE_VERSION
    if (mgIsServer) {
#endif
	    /* enter text mode*/
        if (tty >= 0) {
	        ioctl (tty, KDSETMODE, KD_TEXT);
	        close (tty);
            tty = -1;
        }
#ifdef _LITE_VERSION
    }
#endif
#endif
	/* close framebuffer*/
	close(fb);
}


static int fade = 100;

static void fb_setpalette(PSD psd,int first, int count, GAL_Color *palette)
{
	int 	i;
	unsigned short 	red[count];
	unsigned short 	green[count];
	unsigned short 	blue[count];

	/* convert palette to framebuffer format*/
	for(i=0; i < count; i++) {
		GAL_Color *p = &palette[i];

		/* grayscale computation:
		 * red[i] = green[i] = blue[i] =
		 *	(p->r * 77 + p->g * 151 + p->b * 28);
		 */
		red[i] = (p->r * fade / 100) << 8;
		green[i] = (p->g * fade / 100) << 8;
		blue[i] = (p->b * fade / 100) << 8;
	}
	ioctl_setpalette(first, count, red, green, blue);
}

static void fb_getpalette(PSD psd,int first, int count, GAL_Color *palette)
{
	int 	i;
	unsigned short 	red[count];
	unsigned short 	green[count];
	unsigned short 	blue[count];

	ioctl_getpalette(first,count,red,green,blue);
	for(i=0; i < count; i++) {
		GAL_Color *p = &palette[i];

		/* grayscale computation:
		 * red[i] = green[i] = blue[i] =
		 *	(p->r * 77 + p->g * 151 + p->b * 28);
		 */
		p->r = (red[i] >> 8) * 100 / fade;
		p->g = (green[i] >>8) * 100 / fade;
		p->b = (blue[i] >>8) * 100 / fade;
	}
}

/* get framebuffer palette*/
void ioctl_getpalette(int start, int len, short *red, short *green, short *blue)
{
	struct fb_cmap cmap;

	cmap.start = start;
	cmap.len = len;
	cmap.red = red;
	cmap.green = green;
	cmap.blue = blue;
	cmap.transp = NULL;

	ioctl(fb, FBIOGETCMAP, &cmap);
}

/* set framebuffer palette*/
void ioctl_setpalette(int start, int len, short *red, short *green, short *blue)
{
	struct fb_cmap cmap;

	cmap.start = start;
	cmap.len = len;
	cmap.red = red;
	cmap.green = green;
	cmap.blue = blue;
	cmap.transp = NULL;

	ioctl(fb, FBIOPUTCMAP, &cmap);
}


SCREENDEVICE scrdev = {
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
	NULL,			/* DrawPixel subdriver*/
	NULL,			/* ReadPixel subdriver*/
	NULL,			/* DrawHLine subdriver*/
	NULL,			/* DrawVLine subdriver*/
	NULL,			/* Blit subdriver*/
	NULL,			/* PutBox subdriver*/
	NULL			/* CopyBox subdriver*/
};

#endif /* _NATIVE_GAL_FBCON */
