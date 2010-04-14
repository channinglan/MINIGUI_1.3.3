/*
** $Id: fbneomagic.h,v 1.2 2003/09/04 06:02:53 weiym Exp $
**
** Port from DirectFB by Wei Yongming. 
**
** Copyright (C) 2001 ~ 2002 Wei Yongming.
** Copyright (C) 2003 Feynman Software.
**
** NeoMagic hardware acceleration for the MiniGUI framebuffer console driver
*/ 

#include "fbvideo.h"

/* Set up the driver for NeoMagic acceleration */
extern void FB_NeoMagicAccel(_THIS, __u32 card);

