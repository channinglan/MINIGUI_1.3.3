/*
**  $Id: 3dfx_mmio.h,v 1.3 2003/09/04 06:02:53 weiym Exp $
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

/* 3Dfx register definitions */

#include "3dfx_regs.h"

/* 3Dfx control macros */

#define tdfx_in8(reg)		*(volatile Uint8  *)(mapped_io + (reg))
#define tdfx_in32(reg)		*(volatile Uint32 *)(mapped_io + (reg))

#define tdfx_out8(reg,v)	*(volatile Uint8  *)(mapped_io + (reg)) = v;
#define tdfx_out32(reg,v)	*(volatile Uint32 *)(mapped_io + (reg)) = v;


/* Wait for fifo space */
#define tdfx_wait(space)						\
{									\
	while ( (tdfx_in8(TDFX_STATUS) & 0x1F) < space )		\
		;							\
}


/* Wait for idle accelerator */
#define tdfx_waitidle()							\
{									\
	int i = 0;							\
									\
	tdfx_wait(1);							\
	tdfx_out32(COMMAND_3D, COMMAND_3D_NOP);				\
	do {								\
		i = (tdfx_in32(TDFX_STATUS) & STATUS_BUSY) ? 0 : i + 1;	\
	} while ( i != 3 );						\
}

