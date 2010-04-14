/*
**  $Id: leaks.h,v 1.2 2003/09/04 06:02:53 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/

/* Define this if you want surface leak detection code enabled */
/*#define CHECK_LEAKS*/

/* Global variables used to check leaks in code using SDL */

#ifdef CHECK_LEAKS
extern int surfaces_allocated;
#endif
