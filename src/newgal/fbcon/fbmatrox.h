/*
** $Id: fbmatrox.h,v 1.3 2003/09/04 06:02:53 weiym Exp $
**  
** Matrox hardware acceleration for the SDL framebuffer console driver
**
** Port to MiniGUI by Wei Yongming (2001/10/03).
** Copyright (C) 2001 ~ 2002 Wei Yongming.
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/ 

#include "fbvideo.h"

/* Set up the driver for Matrox acceleration */
extern void FB_MatroxAccel(_THIS, __u32 card);
