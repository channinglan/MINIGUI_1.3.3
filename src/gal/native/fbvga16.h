/*
** $Id: fbvga16.h,v 1.2 2003/08/01 09:50:43 weiym Exp $
**
** fbvga16.h: Header file for EGA/VGA 16 color 4 planes screen driver.
**
** Copyright (C) 2003 Feynman Software.
 */

#ifndef GUI_GAL_FBVGA16_H
    #define GUI_GAL_FBVGA16_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus  */

#define SLOWVGA        0    /* =1 for outb rather than outw instructions  */

#include <sys/io.h>         /* for outb def's, requires -O option of gcc  */

/* get byte at address  */
#define GETBYTE(addr)       (*(gal_uint8*)(addr))

/* put byte at address  */
#define PUTBYTE(addr,val)   ((*(gal_uint8*)(addr)) = (val))

/* read-modify-write at address  */
#define RMW(addr)           ((*(gal_uint8*)(addr)) |= 1)

/* or byte at address  */
#define ORBYTE(addr,val)    ((*(gal_uint8*)(addr)) |= (val))

/* and byte at address  */
#define ANDBYTE(addr,val)   ((*(gal_uint8*)(addr)) &= (val))

#if SLOWVGA
/* use outb rather than outw instructions for older, slower VGA's */

/* Program the Set/Reset Register for drawing in color COLOR for write
   mode 0.  */
#define set_color(c)        { outb (0, 0x3ce); outb (c, 0x3cf); }

/* Set the Enable Set/Reset Register.  */
#define set_enable_sr(mask) { outb (1, 0x3ce); outb (mask, 0x3cf); }

/* Select the Bit Mask Register on the Graphics Controller.  */
#define select_mask()       { outb (8, 0x3ce); }

/* Program the Bit Mask Register to affect only the pixels selected in
   MASK.  The Bit Mask Register must already have been selected with
   select_mask ().  */
#define set_mask(mask)      { outb (mask, 0x3cf); }

#define select_and_set_mask(mask)   { outb (8, 0x3ce); outb (mask, 0x3cf); }

/* Set the Data Rotate Register.  Bits 0-2 are rotate count, bits 3-4
   are logical operation (0=NOP, 1=AND, 2=OR, 3=XOR).  */
#define set_op(op)          { outb (3, 0x3ce); outb (op, 0x3cf); }

/* Set the Memory Plane Write Enable register.  */
#define set_write_planes(mask)  { outb (2, 0x3c4); outb (mask, 0x3c5); }

/* Set the Read Map Select register.  */
#define set_read_plane(plane)   { outb (4, 0x3ce); outb (plane, 0x3cf); }

/* Set the Graphics Mode Register.  The write mode is in bits 0-1, the
   read mode is in bit 3.  */
#define set_mode(mode)          { outb (5, 0x3ce); outb (mode, 0x3cf); }

#else /* !SLOWVGA  */

/* use outw rather than outb instructions for new VGAs */

/* Program the Set/Reset Register for drawing in color COLOR for write
   mode 0.  */
#define set_color(c)            { outw ((c)<<8, 0x3ce); }

/* Set the Enable Set/Reset Register.  */
#define set_enable_sr(mask)     { outw (1|((mask)<<8), 0x3ce); }

/* Select the Bit Mask Register on the Graphics Controller.  */
#define select_mask()           { outb (8, 0x3ce); }

/* Program the Bit Mask Register to affect only the pixels selected in
   MASK.  The Bit Mask Register must already have been selected with
   select_mask ().  */
#define set_mask(mask)          { outb (mask, 0x3cf); }

#define select_and_set_mask(mask)   { outw (8|((mask)<<8), 0x3ce); }

/* Set the Data Rotate Register.  Bits 0-2 are rotate count, bits 3-4
   are logical operation (0=NOP, 1=AND, 2=OR, 3=XOR).  */
#define set_op(op)              { outw (3|((op)<<8), 0x3ce); }

/* Set the Memory Plane Write Enable register.  */
#define set_write_planes(mask)  { outw (2|((mask)<<8), 0x3c4); }

/* Set the Read Map Select register.  */
#define set_read_plane(plane)   { outw (4|((plane)<<8), 0x3ce); }

/* Set the Graphics Mode Register.  The write mode is in bits 0-1, the
   read mode is in bit 3.  */
#define set_mode(mode)          { outw (5|((mode)<<8), 0x3ce); }

#endif /* SLOWVGA */

#ifdef __cplusplus
}
#endif  /* __cplusplus  */

#endif  /* GUI_GAL_FBVGA16_H  */

