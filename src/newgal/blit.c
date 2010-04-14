/*
**  $Id: blit.c,v 1.7 2003/11/22 04:44:14 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/06).
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "newgal.h"
#include "sysvideo.h"
#include "blit.h"
#include "RLEaccel_c.h"
#include "pixels_c.h"
#include "memops.h"

/* The general purpose software blit routine */
static int GAL_SoftBlit(GAL_Surface *src, GAL_Rect *srcrect,
            GAL_Surface *dst, GAL_Rect *dstrect)
{
    int okay;
#if 0
    int src_locked;
    int dst_locked;
#endif

    /* Everything is okay at the beginning...  */
    okay = 1;

#if 0
    /* Lock the destination if it's in hardware */
    dst_locked = 0;
    if ( dst->flags & (GAL_HWSURFACE|GAL_ASYNCBLIT) ) {
        GAL_VideoDevice *video = current_video;
        GAL_VideoDevice *this  = current_video;
        if ( video->LockHWSurface(this, dst) < 0 ) {
            okay = 0;
        } else {
            dst_locked = 1;
        }
    }
    /* Lock the source if it's in hardware */
    src_locked = 0;
    if ( src->flags & (GAL_HWSURFACE|GAL_ASYNCBLIT) ) {
        GAL_VideoDevice *video = current_video;
        GAL_VideoDevice *this  = current_video;
        if ( video->LockHWSurface(this, src) < 0 ) {
            okay = 0;
        } else {
            src_locked = 1;
        }
    }
#endif

    /* Unencode the destination if it's RLE encoded */
    if ( dst->flags & GAL_RLEACCEL ) {
        GAL_UnRLESurface(dst, 1);
        dst->flags |= GAL_RLEACCEL;    /* save accel'd state */
    }

    /* Set up source and destination buffer pointers, and BLIT! */
    if ( okay  && srcrect->w && srcrect->h ) {
        GAL_BlitInfo info;
        GAL_loblit RunBlit;

        /* Set up the blit information */
        info.s_pixels = (Uint8 *)src->pixels + src->offset +
                (Uint16)srcrect->y*src->pitch +
                (Uint16)srcrect->x*src->format->BytesPerPixel;
        info.s_width = srcrect->w;
        info.s_height = srcrect->h;
        info.s_skip=src->pitch-info.s_width*src->format->BytesPerPixel;
        info.d_pixels = (Uint8 *)dst->pixels + dst->offset +
                (Uint16)dstrect->y*dst->pitch +
                (Uint16)dstrect->x*dst->format->BytesPerPixel;
        info.d_width = dstrect->w;
        info.d_height = dstrect->h;
        info.d_skip=dst->pitch-info.d_width*dst->format->BytesPerPixel;
        info.aux_data = src->map->sw_data->aux_data;
        info.src = src->format;
        info.table = src->map->table;
        info.dst = dst->format;
        RunBlit = src->map->sw_data->blit;

        /* Run the actual software blit */
        RunBlit(&info);
    }

    /* Re-encode the destination if it's RLE encoded */
    if ( dst->flags & GAL_RLEACCEL ) {
            dst->flags &= ~GAL_RLEACCEL; /* stop lying */
        GAL_RLESurface(dst);
    }

#if 0
    /* We need to unlock the surfaces if they're locked */
    if ( dst_locked ) {
        GAL_VideoDevice *video = current_video;
        GAL_VideoDevice *this  = current_video;
        video->UnlockHWSurface(this, dst);
    } else
    if ( src_locked ) {
        GAL_VideoDevice *video = current_video;
        GAL_VideoDevice *this  = current_video;
        video->UnlockHWSurface(this, src);
    }
#endif

    /* Blit is done! */
    return(okay ? 0 : -1);
}

static void GAL_BlitCopy(GAL_BlitInfo *info)
{
    Uint8 *src, *dst;
    int w, h;
    int srcskip, dstskip;

    w = info->d_width*info->dst->BytesPerPixel;
    h = info->d_height;
    src = info->s_pixels;
    dst = info->d_pixels;
    srcskip = w+info->s_skip;
    dstskip = w+info->d_skip;
    while ( h-- ) {
        GAL_memcpy(dst, src, w);
        src += srcskip;
        dst += dstskip;
    }
}

static void GAL_BlitCopyOverlap(GAL_BlitInfo *info)
{
    Uint8 *src, *dst;
    int w, h;
    int srcskip, dstskip;

    w = info->d_width*info->dst->BytesPerPixel;
    h = info->d_height;
    src = info->s_pixels;
    dst = info->d_pixels;
    srcskip = w+info->s_skip;
    dstskip = w+info->d_skip;
    if ( dst < src ) {
        while ( h-- ) {
            GAL_memcpy(dst, src, w);
            src += srcskip;
            dst += dstskip;
        }
    } else {
        src += ((h-1) * srcskip);
        dst += ((h-1) * dstskip);
        while ( h-- ) {
            GAL_revcpy(dst, src, w);
            src -= srcskip;
            dst -= dstskip;
        }
    }
}

/* Figure out which of many blit routines to set up on a surface */
int GAL_CalculateBlit(GAL_Surface *surface)
{
    int blit_index;

    /* Clean everything out to start */
    if ( (surface->flags & GAL_RLEACCEL) == GAL_RLEACCEL ) {
        GAL_UnRLESurface(surface, 1);
    }
    surface->map->sw_blit = NULL;

    /* Figure out if an accelerated hardware blit is possible */
    surface->flags &= ~GAL_HWACCEL;
    if ( surface->map->identity ) {
        int hw_blit_ok;

        if ( (surface->flags & GAL_HWSURFACE) == GAL_HWSURFACE ) {
            /* We only support accelerated blitting to hardware */
            if ( surface->map->dst->flags & GAL_HWSURFACE ) {
                hw_blit_ok = current_video->info.blit_hw;
            } else {
                hw_blit_ok = 0;
            }
            if (hw_blit_ok && (surface->flags & GAL_SRCCOLORKEY)) {
                hw_blit_ok = current_video->info.blit_hw_CC;
            }
            if ( hw_blit_ok && (surface->flags & GAL_SRCALPHA) ) {
                hw_blit_ok = current_video->info.blit_hw_A;
            }
        } else {
            /* We only support accelerated blitting to hardware */
            if ( surface->map->dst->flags & GAL_HWSURFACE ) {
                hw_blit_ok = current_video->info.blit_sw;
            } else {
                hw_blit_ok = 0;
            }
            if (hw_blit_ok && (surface->flags & GAL_SRCCOLORKEY)) {
                hw_blit_ok = current_video->info.blit_sw_CC;
            }
            if ( hw_blit_ok && (surface->flags & GAL_SRCALPHA) ) {
                hw_blit_ok = current_video->info.blit_sw_A;
            }
        }
        if ( hw_blit_ok ) {
            GAL_VideoDevice *video = current_video;
            GAL_VideoDevice *this  = current_video;
            video->CheckHWBlit(this, surface, surface->map->dst);
        }
    }

    /* Get the blit function index, based on surface mode */
    /* { 0 = nothing, 1 = colorkey, 2 = alpha, 3 = colorkey+alpha } */
    blit_index = 0;
    blit_index |= (!!(surface->flags & GAL_SRCCOLORKEY))      << 0;
    if ( surface->flags & GAL_SRCALPHA
         && (surface->format->alpha != GAL_ALPHA_OPAQUE
         || surface->format->Amask) ) {
            blit_index |= 2;
    }

    /* Check for special "identity" case -- copy blit */
    if ( surface->map->identity && blit_index == 0 ) {
            surface->map->sw_data->blit = GAL_BlitCopy;

        /* Handle overlapping blits on the same surface */
        if ( surface == surface->map->dst ) {
                surface->map->sw_data->blit = GAL_BlitCopyOverlap;
        }
    } else {
        if ( surface->format->BitsPerPixel < 8 ) {
            surface->map->sw_data->blit =
                GAL_CalculateBlit0(surface, blit_index);
        } else {
            switch ( surface->format->BytesPerPixel ) {
                case 1:
                surface->map->sw_data->blit =
                    GAL_CalculateBlit1(surface, blit_index);
                break;
                case 2:
                case 3:
                case 4:
                surface->map->sw_data->blit =
                    GAL_CalculateBlitN(surface, blit_index);
                break;
                default:
                surface->map->sw_data->blit = NULL;
                break;
            }
        }
    }
    /* Make sure we have a blit function */
    if ( surface->map->sw_data->blit == NULL ) {
        GAL_InvalidateMap(surface->map);
        GAL_SetError("Blit combination not supported");
        return(-1);
    }

    /* Choose software blitting function */
    if(surface->flags & GAL_RLEACCELOK
       && (surface->flags & GAL_HWACCEL) != GAL_HWACCEL) {

            if(surface->map->identity
           && (blit_index == 1
               || (blit_index == 3 && !surface->format->Amask))) {
                if ( GAL_RLESurface(surface) == 0 )
                    surface->map->sw_blit = GAL_RLEBlit;
        } else if(blit_index == 2 && surface->format->Amask) {
                if ( GAL_RLESurface(surface) == 0 )
                    surface->map->sw_blit = GAL_RLEAlphaBlit;
        }
    }
    
    if ( surface->map->sw_blit == NULL ) {
        surface->map->sw_blit = GAL_SoftBlit;
    }
    return(0);
}

