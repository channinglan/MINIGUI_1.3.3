/*
**  $Id: sysvideo.h,v 1.8 2003/11/22 04:32:25 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/

#ifndef _GAL_sysvideo_h
#define _GAL_sysvideo_h

/* This file prototypes the video driver implementation.
   This is designed to be easily converted to C++ in the future.
 */

/* The SDL video driver */
typedef struct GAL_VideoDevice GAL_VideoDevice;
struct REQ_HWSURFACE;
struct REP_HWSURFACE;

/* Define the SDL video driver structure */
#define _THIS	GAL_VideoDevice *_this
#ifndef _STATUS
#define _STATUS	GAL_status *status
#endif
struct GAL_VideoDevice {
	/* * * */
	/* The name of this video driver */
	const char *name;

	/* * * */
	/* Initialization/Query functions */

	/* Initialize the native video subsystem, filling 'vformat' with the 
	   "best" display pixel format, returning 0 or -1 if there's an error.
	 */
	int (*VideoInit)(_THIS, GAL_PixelFormat *vformat);

	/* List the available video modes for the given pixel format, sorted
	   from largest to smallest.
	 */
	GAL_Rect **(*ListModes)(_THIS, GAL_PixelFormat *format, Uint32 flags);

	/* Set the requested video mode, returning a surface which will be
	   set to the GAL_VideoSurface.  The width and height will already
	   be verified by ListModes(), and the video subsystem is free to
	   set the mode to a supported bit depth different from the one
	   specified -- the desired bpp will be emulated with a shadow
	   surface if necessary.  If a new mode is returned, this function
	   should take care of cleaning up the current mode.
	 */
	GAL_Surface *(*SetVideoMode)(_THIS, GAL_Surface *current,
				int width, int height, int bpp, Uint32 flags);

	/* Toggle the fullscreen mode */
	int (*ToggleFullScreen)(_THIS, int on);

#if 0
	/* This is called after the video mode has been set, to get the
	   initial mouse state.  It should queue events as necessary to
	   properly represent the current mouse focus and position.
	 */
	void (*UpdateMouse)(_THIS);
#endif

	/* Create a YUV video surface (possibly overlay) of the given
	   format.  The hardware should be able to perform at least 2x
	   scaling on display.
	 */
	GAL_Overlay *(*CreateYUVOverlay)(_THIS, int width, int height,
	                                 Uint32 format, GAL_Surface *display);

	/* Sets the color entries { firstcolor .. (firstcolor+ncolors-1) }
	   of the physical palette to those in 'colors'. If the device is
	   using a software palette (GAL_HWPALETTE not set), then the
	   changes are reflected in the logical palette of the screen
	   as well.
	   The return value is 1 if all entries could be set properly
	   or 0 otherwise.
	 */
	int (*SetColors)(_THIS, int firstcolor, int ncolors,
			 GAL_Color *colors);

	/* This pointer should exist in the native video subsystem and should
	   point to an appropriate update function for the current video mode
	 */
	void (*UpdateRects)(_THIS, int numrects, GAL_Rect *rects);

	/* Reverse the effects VideoInit() -- called if VideoInit() fails
	   or if the application is shutting down the video subsystem.
	*/
	void (*VideoQuit)(_THIS);

	/* * * */
	/* Hardware acceleration functions */

	/* Information about the video hardware */
	GAL_VideoInfo info;

#ifdef _LITE_VERSION
	/* Request a surface in video memory */
	void (*RequestHWSurface)(_THIS, const REQ_HWSURFACE* request, REP_HWSURFACE* reply);
#endif

	/* Allocates a surface in video memory */
	int (*AllocHWSurface)(_THIS, GAL_Surface *surface);

	/* Sets the hardware accelerated blit function, if any, based
	   on the current flags of the surface (colorkey, alpha, etc.)
	 */
	int (*CheckHWBlit)(_THIS, GAL_Surface *src, GAL_Surface *dst);

	/* Fills a surface rectangle with the given color */
	int (*FillHWRect)(_THIS, GAL_Surface *dst, GAL_Rect *rect, Uint32 color);

	/* Sets video mem colorkey and accelerated blit function */
	int (*SetHWColorKey)(_THIS, GAL_Surface *surface, Uint32 key);

	/* Sets per surface hardware alpha value */
	int (*SetHWAlpha)(_THIS, GAL_Surface *surface, Uint8 value);

#if 0
	/* Returns a readable/writable surface */
	int (*LockHWSurface)(_THIS, GAL_Surface *surface);
	void (*UnlockHWSurface)(_THIS, GAL_Surface *surface);

	/* Performs hardware flipping */
	int (*FlipHWSurface)(_THIS, GAL_Surface *surface);
#endif

	/* Frees a previously allocated video surface */
	void (*FreeHWSurface)(_THIS, GAL_Surface *surface);

	/* * * */
	/* Gamma support */

	Uint16 *gamma;

	/* Set the gamma correction directly (emulated with gamma ramps) */
	int (*SetGamma)(_THIS, float red, float green, float blue);

	/* Get the gamma correction directly (emulated with gamma ramps) */
	int (*GetGamma)(_THIS, float *red, float *green, float *blue);

	/* Set the gamma ramp */
	int (*SetGammaRamp)(_THIS, Uint16 *ramp);

	/* Get the gamma ramp */
	int (*GetGammaRamp)(_THIS, Uint16 *ramp);

	/* * * */
	/* Data common to all drivers */
	GAL_Surface *screen;
//	GAL_Surface *shadow;
//	GAL_Surface *visible;
    GAL_Palette *physpal;	/* physical palette, if != logical palette */
    GAL_Color *gammacols;	/* gamma-corrected colours, or NULL */
	char *wm_title;
	char *wm_icon;
	int offset_x;
	int offset_y;

	/* Driver information flags */
	int handles_any_size;	/* Driver handles any size video mode */

	/* * * */
	/* Data private to this driver */
	struct GAL_PrivateVideoData *hidden;

	/* * * */
	/* The function used to dispose of this structure */
	void (*free)(_THIS);
};
#undef _THIS

typedef struct VideoBootStrap {
	const char *name;
	const char *desc;
	int (*available)(void);
	GAL_VideoDevice *(*create)(int devindex);
} VideoBootStrap;

#ifdef ENABLE_DUMMY
extern VideoBootStrap DUMMY_bootstrap;
#endif
#ifdef ENABLE_FBCON
extern VideoBootStrap FBCON_bootstrap;
#endif
#ifdef ENABLE_QVFB
extern VideoBootStrap QVFB_bootstrap;
#endif
#ifdef ENABLE_ECOSLCD
extern VideoBootStrap ECOSLCD_bootstrap;
#endif
#ifdef ENABLE_X11
extern VideoBootStrap X11_bootstrap;
#endif
#ifdef ENABLE_DGA
extern VideoBootStrap DGA_bootstrap;
#endif
#ifdef ENABLE_GGI
extern VideoBootStrap GGI_bootstrap;
#endif
#ifdef ENABLE_VGL
extern VideoBootStrap VGL_bootstrap;
#endif
#ifdef ENABLE_SVGALIB
extern VideoBootStrap SVGALIB_bootstrap;
#endif
#ifdef ENABLE_AALIB
extern VideoBootStrap AALIB_bootstrap;
#endif

/* This is the current video device */
extern GAL_VideoDevice *current_video;

#define GAL_VideoSurface	(current_video->screen)
#define GAL_PublicSurface	(current_video->screen)
// #define GAL_ShadowSurface	(current_video->shadow)

#endif /* _GAL_sysvideo_h */

