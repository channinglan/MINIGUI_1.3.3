/*
**  $Id: matrox_mmio.h,v 1.3 2003/09/04 06:02:53 weiym Exp $
**  
**  Port to MiniGUI by Wei Yongming (2001/10/03).
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
**  Copyright (C) 2003 Feynman Software.
**
**  SDL - Simple DirectMedia Layer
**  Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga
*/

/* MGA register definitions */

#include "matrox_regs.h"

/* MGA control macros */

#define mga_in8(reg)		*(volatile Uint8  *)(mapped_io + (reg))
#define mga_in32(reg)		*(volatile Uint32 *)(mapped_io + (reg))

#define mga_out8(reg,v)		*(volatile Uint8  *)(mapped_io + (reg)) = v;
#define mga_out32(reg,v)	*(volatile Uint32 *)(mapped_io + (reg)) = v;


/* Wait for fifo space */
#define mga_wait(space)							\
{									\
	while ( mga_in8(MGAREG_FIFOSTATUS) < space )			\
		;							\
}


/* Wait for idle accelerator */
#define mga_waitidle()							\
{									\
	while ( mga_in32(MGAREG_STATUS) & 0x10000 )			\
		;							\
}

