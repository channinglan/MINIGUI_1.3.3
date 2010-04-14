/*
**  $Id: video.c,v 1.14 2003/11/22 04:44:14 weiym Exp $
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

/* The high-level video driver subsystem */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "newgal.h"
#include "sysvideo.h"
#include "blit.h"
#include "pixels_c.h"

/* Available video drivers */
static VideoBootStrap *bootstrap[] = {
#ifdef ENABLE_DUMMY
    &DUMMY_bootstrap,
#endif
#ifdef ENABLE_FBCON
    &FBCON_bootstrap,
#endif
#ifdef ENABLE_QVFB
    &QVFB_bootstrap,
#endif
#ifdef ENABLE_ECOSLCD
    &ECOSLCD_bootstrap,
#endif
#ifdef ENABLE_X11
    &X11_bootstrap,
#endif
#ifdef ENABLE_DGA
    &DGA_bootstrap,
#endif
#ifdef ENABLE_GGI
    &GGI_bootstrap,
#endif
#ifdef ENABLE_SVGALIB
    &SVGALIB_bootstrap,
#endif
    NULL
};
GAL_VideoDevice *current_video = NULL;

/* Various local functions */
int GAL_VideoInit(const char *driver_name, Uint32 flags);
void GAL_VideoQuit(void);

/*
 * Initialize the video subsystems -- determine native pixel format
 */
int GAL_VideoInit (const char *driver_name, Uint32 flags)
{
    GAL_VideoDevice *video;
    int index;
    int i;
    GAL_PixelFormat vformat;
    Uint32 video_flags;

    /* Check to make sure we don't overwrite 'current_video' */
    if ( current_video != NULL ) {
        GAL_VideoQuit();
    }

    /* Select the proper video driver */
    index = 0;
    video = NULL;
    if ( driver_name != NULL ) {
        for ( i=0; bootstrap[i]; ++i ) {
            if ( strncmp(bootstrap[i]->name, driver_name,
                         strlen(bootstrap[i]->name)) == 0 ) {
                if ( bootstrap[i]->available() ) {
                    video = bootstrap[i]->create(index);
                    break;
                }
            }
        }
    } else {
        for ( i=0; bootstrap[i]; ++i ) {
            if ( bootstrap[i]->available() ) {
                video = bootstrap[i]->create(index);
                if ( video != NULL ) {
                    break;
                }
            }
        }
    }
    if ( video == NULL ) {
        GAL_SetError("No available video device.\n");
        return(-1);
    }
    current_video = video;
    current_video->name = bootstrap[i]->name;

    /* Do some basic variable initialization */
    video->screen = NULL;
#if 0
    video->shadow = NULL;
    video->visible = NULL;
#endif
    video->physpal = NULL;
    video->offset_x = 0;
    video->offset_y = 0;
    memset(&video->info, 0, (sizeof video->info));

    /* Initialize the video subsystem */
    memset(&vformat, 0, sizeof(vformat));
    if ( video->VideoInit(video, &vformat) < 0 ) {
        GAL_VideoQuit();
        return(-1);
    }

    /* Create a zero sized video surface of the appropriate format */
    video_flags = GAL_SWSURFACE;
    GAL_VideoSurface = GAL_CreateRGBSurface(video_flags, 0, 0,
                vformat.BitsPerPixel,
                vformat.Rmask, vformat.Gmask, vformat.Bmask, 0);
    if ( GAL_VideoSurface == NULL ) {
        GAL_VideoQuit();
        return(-1);
    }

    video->info.vfmt = GAL_VideoSurface->format;

    /* We're ready to go! */
    return(0);
}

char *GAL_VideoDriverName(char *namebuf, int maxlen)
{
    if ( current_video != NULL ) {
        strncpy(namebuf, current_video->name, maxlen-1);
        namebuf[maxlen-1] = '\0';
        return(namebuf);
    }
    return(NULL);
}

/*
 * Get the current display surface
 */
GAL_Surface *GAL_GetVideoSurface(void)
{
    GAL_Surface *visible;

    visible = NULL;
    if ( current_video ) {
#if 0
        visible = current_video->visible;
#else
        visible = current_video->screen;
#endif
    }
    return(visible);
}

/*
 * Get the current information about the video hardware
 */
const GAL_VideoInfo *GAL_GetVideoInfo(void)
{
    const GAL_VideoInfo *info;

    info = NULL;
    if ( current_video ) {
        info = &current_video->info;
    }
    return(info);
}

/*
 * Return a pointer to an array of available screen dimensions for the
 * given format, sorted largest to smallest.  Returns NULL if there are
 * no dimensions available for a particular format, or (GAL_Rect **)-1
 * if any dimension is okay for the given format.  If 'format' is NULL,
 * the mode list will be for the format given by GAL_GetVideoInfo()->vfmt
 */
GAL_Rect ** GAL_ListModes (GAL_PixelFormat *format, Uint32 flags)
{
    GAL_VideoDevice *video = current_video;
    GAL_VideoDevice *this  = current_video;
    GAL_Rect **modes;

    modes = NULL;
    if ( GAL_VideoSurface ) {
        if ( format == NULL ) {
            format = GAL_VideoSurface->format;
        }
        modes = video->ListModes(this, format, flags);
    }
    return(modes);
}

/*
 * Check to see if a particular video mode is supported.
 * It returns 0 if the requested mode is not supported under any bit depth,
 * or returns the bits-per-pixel of the closest available mode with the
 * given width and height.  If this bits-per-pixel is different from the
 * one used when setting the video mode, GAL_SetVideoMode() will succeed,
 * but will emulate the requested bits-per-pixel with a shadow surface.
 */
static Uint8 GAL_closest_depths[4][8] = {
    /* 8 bit closest depth ordering */
    { 0, 8, 16, 15, 32, 24, 0, 0 },
    /* 15,16 bit closest depth ordering */
    { 0, 16, 15, 32, 24, 8, 0, 0 },
    /* 24 bit closest depth ordering */
    { 0, 24, 32, 16, 15, 8, 0, 0 },
    /* 32 bit closest depth ordering */
    { 0, 32, 16, 15, 24, 8, 0, 0 }
};

int GAL_VideoModeOK (int width, int height, int bpp, Uint32 flags) 
{
    int table, b, i;
    int supported;
    GAL_PixelFormat format;
    GAL_Rect **sizes;

    /* Currently 1 and 4 bpp are not supported */
    if ( bpp < 8 || bpp > 32 ) {
        return(0);
    }
    if ( (width == 0) || (height == 0) ) {
        return(0);
    }

    /* Search through the list valid of modes */
    memset(&format, 0, sizeof(format));
    supported = 0;
    table = ((bpp+7)/8)-1;
    GAL_closest_depths[table][0] = bpp;
    GAL_closest_depths[table][7] = 0;
    for ( b = 0; !supported && GAL_closest_depths[table][b]; ++b ) {
        format.BitsPerPixel = GAL_closest_depths[table][b];
        sizes = GAL_ListModes(&format, flags);
        if ( sizes == (GAL_Rect **)0 ) {
            /* No sizes supported at this bit-depth */
            continue;
        } else 
        if ( (sizes == (GAL_Rect **)-1) ||
             current_video->handles_any_size ) {
            /* Any size supported at this bit-depth */
            supported = 1;
            continue;
        } else
        for ( i=0; sizes[i]; ++i ) {
            if ((sizes[i]->w == width) && (sizes[i]->h == height)) {
                supported = 1;
                break;
            }
        }
    }
    if ( supported ) {
        --b;
        return(GAL_closest_depths[table][b]);
    } else {
        return(0);
    }
}

/*
 * Get the closest non-emulated video mode to the one requested
 */
static int GAL_GetVideoMode (int *w, int *h, int *BitsPerPixel, Uint32 flags)
{
    int table, b, i;
    int supported;
    int native_bpp;
    GAL_PixelFormat format;
    GAL_Rect **sizes;

    /* Try the original video mode, get the closest depth */
    native_bpp = GAL_VideoModeOK(*w, *h, *BitsPerPixel, flags);
    if ( native_bpp == *BitsPerPixel ) {
        return(1);
    }
    if ( native_bpp > 0 ) {
        *BitsPerPixel = native_bpp;
        return(1);
    }

    /* No exact size match at any depth, look for closest match */
    memset(&format, 0, sizeof(format));
    supported = 0;
    table = ((*BitsPerPixel+7)/8)-1;
    GAL_closest_depths[table][0] = *BitsPerPixel;
    GAL_closest_depths[table][7] = GAL_VideoSurface->format->BitsPerPixel;
    for ( b = 0; !supported && GAL_closest_depths[table][b]; ++b ) {
        format.BitsPerPixel = GAL_closest_depths[table][b];
        sizes = GAL_ListModes(&format, flags);
        if ( sizes == (GAL_Rect **)0 ) {
            /* No sizes supported at this bit-depth */
            continue;
        }
        for ( i=0; sizes[i]; ++i ) {
            if ((sizes[i]->w < *w) || (sizes[i]->h < *h)) {
                if ( i > 0 ) {
                    --i;
                    *w = sizes[i]->w;
                    *h = sizes[i]->h;
                    *BitsPerPixel = GAL_closest_depths[table][b];
                    supported = 1;
                } else {
                    /* Largest mode too small... */;
                }
                break;
            }
        }
        if ( (i > 0) && ! sizes[i] ) {
            /* The smallest mode was larger than requested, OK */
            --i;
            *w = sizes[i]->w;
            *h = sizes[i]->h;
            *BitsPerPixel = GAL_closest_depths[table][b];
            supported = 1;
        }
    }
    if ( ! supported ) {
        GAL_SetError("No video mode large enough for the resolution specified.\n");
    }
    return(supported);
}

/* This should probably go somewhere else -- like GAL_surface.c */
static void GAL_ClearSurface(GAL_Surface *surface)
{
    Uint32 black;

    black = GAL_MapRGB(surface->format, 0, 0, 0);
    GAL_FillRect(surface, NULL, black);
#if 0
    if ((surface->flags&GAL_HWSURFACE) && (surface->flags&GAL_DOUBLEBUF)) {
        GAL_Flip(surface);
        GAL_FillRect(surface, NULL, black);
    }
    GAL_Flip(surface);
#endif
}

#if 0
/*
 * Create a shadow surface suitable for fooling the app. :-)
 */
static void GAL_CreateShadowSurface(int depth)
{
    Uint32 Rmask, Gmask, Bmask;

    /* Allocate the shadow surface */
    if ( depth == (GAL_VideoSurface->format)->BitsPerPixel ) {
        Rmask = (GAL_VideoSurface->format)->Rmask;
        Gmask = (GAL_VideoSurface->format)->Gmask;
        Bmask = (GAL_VideoSurface->format)->Bmask;
    } else {
        Rmask = Gmask = Bmask = 0;
    }
    GAL_ShadowSurface = GAL_CreateRGBSurface(GAL_SWSURFACE,
                GAL_VideoSurface->w, GAL_VideoSurface->h,
                        depth, Rmask, Gmask, Bmask, 0);
    if ( GAL_ShadowSurface == NULL ) {
        return;
    }

    /* 8-bit shadow surfaces report that they have exclusive palette */
    if ( GAL_ShadowSurface->format->palette ) {
        GAL_ShadowSurface->flags |= GAL_HWPALETTE;
        if ( depth == (GAL_VideoSurface->format)->BitsPerPixel ) {
            memcpy(GAL_ShadowSurface->format->palette->colors,
                GAL_VideoSurface->format->palette->colors,
                GAL_VideoSurface->format->palette->ncolors*
                            sizeof(GAL_Color));
        } else {
            GAL_DitherColors(
            GAL_ShadowSurface->format->palette->colors, depth);
        }
    }

    /* If the video surface is fullscreen, the shadow should say so */
    if ( (GAL_VideoSurface->flags & GAL_FULLSCREEN) == GAL_FULLSCREEN ) {
        GAL_ShadowSurface->flags |= GAL_FULLSCREEN;
    }
    /* If the video surface is flippable, the shadow should say so */
    if ( (GAL_VideoSurface->flags & GAL_DOUBLEBUF) == GAL_DOUBLEBUF ) {
        GAL_ShadowSurface->flags |= GAL_DOUBLEBUF;
    }
    return;
}
#endif

/*
 * Set the requested video mode, allocating a shadow buffer if necessary.
 */
GAL_Surface * GAL_SetVideoMode (int width, int height, int bpp, Uint32 flags)
{
    GAL_VideoDevice *video, *this;
    GAL_Surface *prev_mode, *mode;
    int video_w;
    int video_h;
    int video_bpp;

    this = video = current_video;

    /* Default to the current video bpp */
    if ( bpp == 0 ) {
        flags |= GAL_ANYFORMAT;
        bpp = GAL_VideoSurface->format->BitsPerPixel;
    }

    /* Get a good video mode, the closest one possible */
    video_w = width;
    video_h = height;
    video_bpp = bpp;
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if ( mgIsServer && !GAL_GetVideoMode(&video_w, &video_h, &video_bpp, flags) ) {
#else
    if ( !GAL_GetVideoMode(&video_w, &video_h, &video_bpp, flags) ) {
#endif
        return(NULL);
    }

    /* Check the requested flags */
    /* There's no palette in > 8 bits-per-pixel mode */
    if ( video_bpp > 8 ) {
        flags &= ~GAL_HWPALETTE;
    }

#if 0
    if ( (flags&GAL_DOUBLEBUF) == GAL_DOUBLEBUF ) {
        /* Use hardware surfaces when double-buffering */
        flags |= GAL_HWSURFACE;
    }

    /* Clean up any previous video mode */
    if ( GAL_PublicSurface != NULL ) {
        GAL_PublicSurface = NULL;
    }
    if ( GAL_ShadowSurface != NULL ) {
        GAL_Surface *ready_to_go;
        ready_to_go = GAL_ShadowSurface;
        GAL_ShadowSurface = NULL;
        GAL_FreeSurface(ready_to_go);
    }
#endif

    if ( video->physpal ) {
        free(video->physpal->colors);
        free(video->physpal);
        video->physpal = NULL;
    }

    /* Try to set the video mode, along with offset and clipping */
    prev_mode = GAL_VideoSurface;
    GAL_VideoSurface = NULL;    /* In case it's freed by driver */
    mode = video->SetVideoMode(this, prev_mode,video_w,video_h,video_bpp,flags);

    /*
     * rcg11292000
     * If you try to set an GAL_OPENGL surface, and fail to find a
     * matching  visual, then the next call to GAL_SetVideoMode()
     * will segfault, since  we no longer point to a dummy surface,
     * but rather NULL.
     * Sam 11/29/00
     * WARNING, we need to make sure that the previous mode hasn't
     * already been freed by the video driver.  What do we do in
     * that case?  Should we call GAL_VideoInit() again?
     */
    GAL_VideoSurface = (mode != NULL) ? mode : prev_mode;

    if ((mode != NULL)) {
        /* Sanity check */
        if ( (mode->w < width) || (mode->h < height) ) {
            GAL_SetError("Video mode smaller than requested");
            return(NULL);
        }

        /* If we have a palettized surface, create a default palette */
        if ( mode->format->palette ) {
            GAL_PixelFormat *vf = mode->format;
            GAL_DitherColors(vf->palette->colors, vf->BitsPerPixel);
            vf->DitheredPalette = TRUE;
            video->SetColors(this, 0, vf->palette->ncolors,
                                       vf->palette->colors);
        }

        /* Clear the surface to black */
        video->offset_x = 0;
        video->offset_y = 0;
        mode->offset = 0;
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
        if (mgIsServer) {
#endif
            GAL_SetClipRect(mode, NULL);
            GAL_ClearSurface(mode);
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
        }
#endif

#if 0
        /* Now adjust the offsets to match the desired mode */
        video->offset_x = (mode->w-width)/2;
        video->offset_y = (mode->h-height)/2;
        mode->offset = video->offset_y*mode->pitch +
                video->offset_x*mode->format->BytesPerPixel;
#endif

#ifdef DEBUG_VIDEO
        fprintf(stderr,
            "Requested mode: %dx%dx%d, obtained mode %dx%dx%d (offset %d)\n",
            width, height, bpp,
            mode->w, mode->h, mode->format->BitsPerPixel, mode->offset);
#endif
        mode->w = width;
        mode->h = height;
        GAL_SetClipRect(mode, NULL);
    }

    /* If we failed setting a video mode, return NULL... (Uh Oh!) */
    if ( mode == NULL ) {
        return(NULL);
    }

#if 0
    /* Create a shadow surface if necessary */
    /* There are three conditions under which we create a shadow surface:
        1.  We need a particular bits-per-pixel that we didn't get.
        2.  We need a hardware palette and didn't get one.
        3.  We need a software surface and got a hardware surface.
    */
    if ( (
         (  !(flags&GAL_ANYFORMAT) &&
            (GAL_VideoSurface->format->BitsPerPixel != bpp)) ||
         (   (flags&GAL_HWPALETTE) && 
                !(GAL_VideoSurface->flags&GAL_HWPALETTE)) ||
        /* If the surface is in hardware, video writes are visible
           as soon as they are performed, so we need to buffer them
         */
         (   ((flags&GAL_HWSURFACE) == GAL_SWSURFACE) &&
                (GAL_VideoSurface->flags&GAL_HWSURFACE))
         ) ) {
        GAL_CreateShadowSurface(bpp);
        if ( GAL_ShadowSurface == NULL ) {
            GAL_SetError("Couldn't create shadow surface");
            return(NULL);
        }
        GAL_PublicSurface = GAL_ShadowSurface;
    } else {
        GAL_PublicSurface = GAL_VideoSurface;
    }
#endif

    video->info.vfmt = GAL_VideoSurface->format;

    /* We're done! */
    return(GAL_PublicSurface);
}

/* 
 * Convert a surface into the video pixel format.
 */
GAL_Surface * GAL_DisplayFormat (GAL_Surface *surface)
{
    Uint32 flags;

    if (!GAL_PublicSurface) {
        GAL_SetError("No video mode has been set");
        return(NULL);
    }
    /* Set the flags appropriate for copying to display surface */
    flags  = (GAL_PublicSurface->flags&GAL_HWSURFACE);
#ifdef AUTORLE_DISPLAYFORMAT
    flags |= (surface->flags & (GAL_SRCCOLORKEY|GAL_SRCALPHA));
    flags |= GAL_RLEACCELOK;
#else
    flags |= surface->flags & (GAL_SRCCOLORKEY|GAL_SRCALPHA|GAL_RLEACCELOK);
#endif
    return(GAL_ConvertSurface(surface, GAL_PublicSurface->format, flags));
}

/*
 * Convert a surface into a format that's suitable for blitting to
 * the screen, but including an alpha channel.
 */
GAL_Surface *GAL_DisplayFormatAlpha(GAL_Surface *surface)
{
    GAL_PixelFormat *vf;
    GAL_PixelFormat *format;
    GAL_Surface *converted;
    Uint32 flags;
    /* default to ARGB8888 */
    Uint32 amask = 0xff000000;
    Uint32 rmask = 0x00ff0000;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x000000ff;

    if (!GAL_PublicSurface) {
        GAL_SetError("No video mode has been set");
        return(NULL);
    }
    vf = GAL_PublicSurface->format;

    switch(vf->BytesPerPixel) {
        case 2:
        /* For XGY5[56]5, use, AXGY8888, where {X, Y} = {R, B}.
           For anything else (like ARGB4444) it doesn't matter
           since we have no special code for it anyway */
        if ( (vf->Rmask == 0x1f) &&
             (vf->Bmask == 0xf800 || vf->Bmask == 0x7c00)) {
            rmask = 0xff;
            bmask = 0xff0000;
        }
        break;

        case 3:
        case 4:
        /* Keep the video format, as long as the high 8 bits are
           unused or alpha */
        if ( (vf->Rmask == 0xff) && (vf->Bmask == 0xff0000) ) {
            rmask = 0xff;
            bmask = 0xff0000;
        }
        break;

        default:
        /* We have no other optimised formats right now. When/if a new
           optimised alpha format is written, add the converter here */
        break;
    }
    format = GAL_AllocFormat(32, rmask, gmask, bmask, amask);
    flags = GAL_PublicSurface->flags & GAL_HWSURFACE;
    flags |= surface->flags & (GAL_SRCALPHA | GAL_RLEACCELOK);
    converted = GAL_ConvertSurface(surface, format, flags);
    GAL_FreeFormat(format);
    return(converted);
}

/*
 * Update a specific portion of the physical screen
 */
void GAL_UpdateRect(GAL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
    GAL_VideoDevice *video = current_video;

    if ( screen && video->UpdateRects ) {
        GAL_Rect rect;

        /* Perform some checking */
        if ( w == 0 )
            w = screen->w;
        if ( h == 0 )
            h = screen->h;
        if ( (int)(x+w) > screen->w )
            return;
        if ( (int)(y+h) > screen->h )
            return;

        /* Fill the rectangle */
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        GAL_UpdateRects(screen, 1, &rect);
    }
}

void GAL_UpdateRects (GAL_Surface *screen, int numrects, GAL_Rect *rects)
{
    GAL_VideoDevice *video = current_video;
    GAL_VideoDevice *this = current_video;

#if 0
    int i;
    if ( screen == GAL_ShadowSurface ) {
        /* Blit the shadow surface using saved mapping */
            GAL_Palette *pal = screen->format->palette;
        GAL_Color *saved_colors = NULL;
            if ( pal && !(GAL_VideoSurface->flags & GAL_HWPALETTE) ) {
            /* simulated 8bpp, use correct physical palette */
            saved_colors = pal->colors;
            if ( video->physpal ) {
                /* physical palette different from logical */
                pal->colors = video->physpal->colors;
            }
        }
        for ( i=0; i<numrects; ++i ) {
                GAL_LowerBlit(GAL_ShadowSurface, &rects[i], 
                        GAL_VideoSurface, &rects[i]);
        }
        if ( saved_colors )
            pal->colors = saved_colors;

        /* Fall through to video surface update */
        screen = GAL_VideoSurface;
    }
#endif

    if ( screen == GAL_VideoSurface && video->UpdateRects ) {
#if 0
        /* Update the video surface */
        if ( screen->offset ) {
            for ( i=0; i<numrects; ++i ) {
                rects[i].x += video->offset_x;
                rects[i].y += video->offset_y;
            }
            video->UpdateRects(this, numrects, rects);
            for ( i=0; i<numrects; ++i ) {
                rects[i].x -= video->offset_x;
                rects[i].y -= video->offset_y;
            }
        } else {
            video->UpdateRects(this, numrects, rects);
        }
#else
        video->UpdateRects(this, numrects, rects);
#endif
    }
}

#if 0
/*
 * Performs hardware double buffering, if possible, or a full update if not.
 */
int GAL_Flip(GAL_Surface *screen)
{
    GAL_VideoDevice *video = current_video;
    /* Copy the shadow surface to the video surface */
    if ( screen == GAL_ShadowSurface ) {
        GAL_Rect rect;
            GAL_Palette *pal = screen->format->palette;
        GAL_Color *saved_colors = NULL;
            if ( pal && !(GAL_VideoSurface->flags & GAL_HWPALETTE) ) {
            /* simulated 8bpp, use correct physical palette */
            saved_colors = pal->colors;
            if ( video->physpal ) {
                /* physical palette different from logical */
                pal->colors = video->physpal->colors;
            }
        }

        rect.x = 0;
        rect.y = 0;
        rect.w = screen->w;
        rect.h = screen->h;
        GAL_LowerBlit(GAL_ShadowSurface,&rect, GAL_VideoSurface,&rect);

        if ( saved_colors )
            pal->colors = saved_colors;
        screen = GAL_VideoSurface;
    }
    if ( (screen->flags & GAL_DOUBLEBUF) == GAL_DOUBLEBUF ) {
        GAL_VideoDevice *this  = current_video;
        return(video->FlipHWSurface(this, GAL_VideoSurface));
    } else {
        GAL_UpdateRect(screen, 0, 0, 0, 0);
    }
    return(0);
}
#endif

static void SetPalette_logical(GAL_Surface *screen, GAL_Color *colors,
                   int firstcolor, int ncolors)
{
    GAL_Palette *pal = screen->format->palette;
#if 0
    GAL_Palette *vidpal;
#endif

    if ( colors != (pal->colors + firstcolor) ) {
            memcpy(pal->colors + firstcolor, colors,
               ncolors * sizeof(*colors));
    }

#if 0
    vidpal = GAL_VideoSurface->format->palette;
    if ( (screen == GAL_ShadowSurface) && vidpal ) {
            /*
         * This is a shadow surface, and the physical
         * framebuffer is also indexed. Propagate the
         * changes to its logical palette so that
         * updates are always identity blits
         */
        memcpy(vidpal->colors + firstcolor, colors,
               ncolors * sizeof(*colors));
    }
#endif
    GAL_FormatChanged(screen);
}

static int SetPalette_physical(GAL_Surface *screen,
                               GAL_Color *colors, int firstcolor, int ncolors)
{
    GAL_VideoDevice *video = current_video;
    int gotall = 1;

    if ( video->physpal ) {
        /* We need to copy the new colors, since we haven't
         * already done the copy in the logical set above.
         */
        memcpy(video->physpal->colors + firstcolor,
               colors, ncolors * sizeof(*colors));
    }

#if 0
    if ( screen == GAL_ShadowSurface ) {
        if ( GAL_VideoSurface->flags & GAL_HWPALETTE ) {
            /*
             * The real screen is also indexed - set its physical
             * palette. The physical palette does not include the
             * gamma modification, we apply it directly instead,
             * but this only happens if we have hardware palette.
             */
            screen = GAL_VideoSurface;
        } else {
            /*
             * The video surface is not indexed - invalidate any
             * active shadow-to-video blit mappings.
             */
            if ( screen->map->dst == GAL_VideoSurface ) {
                GAL_InvalidateMap(screen->map);
            }
            GAL_UpdateRect(screen, 0, 0, 0, 0);
        }
    }
#endif

    if ( screen == GAL_VideoSurface ) {
        GAL_Color gcolors[256];

        colors = gcolors;
        gotall = video->SetColors(video, firstcolor, ncolors, colors);
        if ( ! gotall ) {
            /* The video flags shouldn't have GAL_HWPALETTE, and
               the video driver is responsible for copying back the
               correct colors into the video surface palette.
            */
            ;
        }
    }
    return gotall;
}

/*
 * Set the physical and/or logical colormap of a surface:
 * Only the screen has a physical colormap. It determines what is actually
 * sent to the display.
 * The logical colormap is used to map blits to/from the surface.
 * 'which' is one or both of GAL_LOGPAL, GAL_PHYSPAL
 *
 * Return nonzero if all colours were set as requested, or 0 otherwise.
 */
int GAL_SetPalette(GAL_Surface *screen, int which,
           GAL_Color *colors, int firstcolor, int ncolors)
{
    GAL_Palette *pal;
    int gotall;
    int palsize;

    if ( ! current_video ) {
        return 0;
    }
    if ( screen != GAL_PublicSurface ) {
        /* only screens have physical palettes */
        which &= ~GAL_PHYSPAL;
    } else if( (screen->flags & GAL_HWPALETTE) != GAL_HWPALETTE ) {
        /* hardware palettes required for split colormaps */
        which |= GAL_PHYSPAL | GAL_LOGPAL;
    }

    /* Verify the parameters */
    pal = screen->format->palette;
    if( !pal ) {
            return 0;    /* not a palettized surface */
    }
    gotall = 1;
    palsize = 1 << screen->format->BitsPerPixel;
    if ( ncolors > (palsize - firstcolor) ) {
            ncolors = (palsize - firstcolor);
        gotall = 0;
    }

    if ( which & GAL_LOGPAL ) {
        /*
         * Logical palette change: The actual screen isn't affected,
         * but the internal colormap is altered so that the
         * interpretation of the pixel values (for blits etc) is
         * changed.
         */
         SetPalette_logical(screen, colors, firstcolor, ncolors);
    }
    if ( which & GAL_PHYSPAL ) {
        GAL_VideoDevice *video = current_video;
            /*
         * Physical palette change: This doesn't affect the
         * program's idea of what the screen looks like, but changes
         * its actual appearance.
         */
        if(!video)
                return gotall;    /* video not yet initialized */
        if(!video->physpal && !(which & GAL_LOGPAL) ) {
            /* Lazy physical palette allocation */
                int size;
            GAL_Palette *pp = malloc(sizeof(*pp));
            current_video->physpal = pp;
            pp->ncolors = pal->ncolors;
            size = pp->ncolors * sizeof(GAL_Color);
            pp->colors = malloc(size);
            memcpy(pp->colors, pal->colors, size);
        }
        if ( ! SetPalette_physical(screen,
                                   colors, firstcolor, ncolors) ) {
            gotall = 0;
        }
    }
    screen->format->DitheredPalette = FALSE;

    return gotall;
}

int GAL_SetColors(GAL_Surface *screen, GAL_Color *colors, int firstcolor,
          int ncolors)
{
        return GAL_SetPalette(screen, GAL_LOGPAL | GAL_PHYSPAL,
                  colors, firstcolor, ncolors);
}

/*
 * Clean up the video subsystem
 */
void GAL_VideoQuit (void)
{
    GAL_Surface *ready_to_go;

    if ( current_video ) {
        GAL_VideoDevice *video = current_video;
        GAL_VideoDevice *this  = current_video;

#if 0
        /* Clean up allocated window manager items */
        if ( GAL_PublicSurface ) {
            GAL_PublicSurface = NULL;
        }
#endif

        /* Clean up the system video */
        video->VideoQuit(this);

#if 0
        /* Free any lingering surfaces */
        ready_to_go = GAL_ShadowSurface;
        GAL_ShadowSurface = NULL;
        GAL_FreeSurface(ready_to_go);
#endif

        if (GAL_VideoSurface != NULL) {
            ready_to_go = GAL_VideoSurface;
            GAL_VideoSurface = NULL;
            GAL_FreeSurface(ready_to_go);
        }
        GAL_PublicSurface = NULL;

        /* Clean up miscellaneous memory */
        if ( video->physpal ) {
            free(video->physpal->colors);
            free(video->physpal);
            video->physpal = NULL;
        }

        /* Finish cleaning up video subsystem */
        video->free(this);
        current_video = NULL;
    }
    return;
}

