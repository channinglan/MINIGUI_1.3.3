/*
** $Id: polygon.h,v 1.3 2003/09/04 06:02:53 weiym Exp $
**
** polygon.h: internals for polygon generator.
**
** Copyright (C) 2003 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
*/

#ifndef GUI_GDI_POLYGON_H
    #define GUI_GDI_POLYGON_H

/* number of fractional bits used by the polygon rasteriser */
#define POLYGON_FIX_SHIFT     18

#ifdef _MATH_3D

/* bitfield specifying which polygon attributes need interpolating */
#define INTERP_FLAT           1      /* no interpolation */
#define INTERP_1COL           2      /* gcol or alpha */
#define INTERP_3COL           4      /* grgb */
#define INTERP_FIX_UV         8      /* atex */
#define INTERP_Z              16     /* always in scene3d */
#define INTERP_FLOAT_UV       32     /* ptex */
#define OPT_FLOAT_UV_TO_FIX   64     /* translate ptex to atex */
#define COLOR_TO_RGB          128    /* grgb to gcol for truecolor */
#define INTERP_ZBUF           256    /* z-buffered */
#define INTERP_THRU           512    /* any kind of transparent */
#define INTERP_NOSOLID        1024   /* non-solid modes for 8-bit flat */
#define INTERP_BLEND          2048   /* lit for truecolor */
#define INTERP_TRANS          4096   /* trans for truecolor */

/* information for polygon scanline fillers */
typedef struct POLYGON_SEGMENT
{
   fixed u, v, du, dv;              /* fixed point u/v coordinates */
   fixed c, dc;                     /* single color gouraud shade values */
   fixed r, g, b, dr, dg, db;       /* RGB gouraud shade values */
   float z, dz;                     /* polygon depth (1/z) */
   float fu, fv, dfu, dfv;          /* floating point u/v coordinates */
   unsigned char *texture;          /* the texture map */
   int umask, vmask, vshift;        /* texture map size information */
   int seg;                         /* destination bitmap selector */
   unsigned long zbuf_addr;	    /* Z-buffer address */
   unsigned long read_addr;	    /* reading address for transparency modes */
} POLYGON_SEGMENT;

#endif

/* an active polygon edge */
typedef struct POLYGON_EDGE 
{
   int top;                         /* top y position */
   int bottom;                      /* bottom y position */
   fixed x, dx;                     /* fixed point x position and gradient */
   fixed w;                         /* width of line segment */

#ifdef _MATH_3D
   POLYGON_SEGMENT dat;             /* texture/gouraud information */
#endif

   struct POLYGON_EDGE *prev;       /* doubly linked list */
   struct POLYGON_EDGE *next;
   struct POLYGON_INFO *poly;	    /* father polygon */
} POLYGON_EDGE;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* polygon helper functions */
void _fill_edge_structure (POLYGON_EDGE *edge, const int *i1, const int *i2);
POLYGON_EDGE* _add_edge (POLYGON_EDGE *list, POLYGON_EDGE *edge, int sort_by_x);
POLYGON_EDGE* _remove_edge (POLYGON_EDGE *list, POLYGON_EDGE *edge);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_GDI_POLYGON_H

