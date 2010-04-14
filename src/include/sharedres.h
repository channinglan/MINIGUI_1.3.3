/*
** $Id: sharedres.h,v 1.11 2003/08/26 04:35:06 weiym Exp $
**
** sharedres.h: structure of shared resource.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Create date: 2000/12/xx
*/

#ifndef GUI_SHAREDRES_H
    #define GUI_SHAREDRES_H

#include <sys/time.h>
#include <sys/termios.h>

#define NR_SRV_WINDOWS  8
#define UBMP_TYPE       BYTE

typedef struct tagG_RES {
    int semid;
    int shmid;
    RECT layer_rc;
    int cli_scr_lx;
    int cli_scr_ty;
    int cli_scr_rx;
    int cli_scr_by;
    GHANDLE topmost_layer;

    unsigned int timer_counter;
    unsigned int tick_on_locksem;
    struct timeval timeout;
    struct termios savedtermio;
    int mousex, mousey;
    int mousebutton;
    int shiftstatus;
    
#ifdef _CURSOR_SUPPORT
    int cursorx, cursory;
    int oldboxleft, oldboxtop;
    HCURSOR csr_current;
    int xhotspot, yhotspot;
	int csr_show_count;
#endif

#ifdef _CURSOR_SUPPORT
	int csrnum;
#endif
	int iconnum;
	int bmpnum;
	int sysfontnum;
	int rbffontnum;
	int varfontnum;

#ifdef _CURSOR_SUPPORT
	unsigned long svdbitsoffset;
	unsigned long csroffset;
#endif
	unsigned long iconoffset;
	unsigned long sfontoffset;
	unsigned long rfontoffset;
	unsigned long vfontoffset;
	unsigned long bmpoffset;

} G_RES;
typedef G_RES* PG_RES;

#define SHAREDRES_TIMER_COUNTER (((PG_RES)mgSharedRes)->timer_counter)
#define SHAREDRES_TICK_ON_LOCKSEM  (((PG_RES)mgSharedRes)->tick_on_locksem)
#define SHAREDRES_TIMEOUT       (((PG_RES)mgSharedRes)->timeout)
#define SHAREDRES_TERMIOS       (((PG_RES)mgSharedRes)->savedtermio)
#define SHAREDRES_MOUSEX        (((PG_RES)mgSharedRes)->mousex)
#define SHAREDRES_MOUSEY        (((PG_RES)mgSharedRes)->mousey)
#define SHAREDRES_BUTTON        (((PG_RES)mgSharedRes)->mousebutton)
#define SHAREDRES_SHIFTSTATUS   (((PG_RES)mgSharedRes)->shiftstatus)
#define SHAREDRES_SEMID         (((PG_RES)mgSharedRes)->semid)
#define SHAREDRES_SHMID         (((PG_RES)mgSharedRes)->shmid)
#define SHAREDRES_LAYER_RC      ((RECT*)(&((PG_RES)mgSharedRes)->layer_rc))
#define SHAREDRES_CLI_SCR_LX    (((PG_RES)mgSharedRes)->cli_scr_lx)
#define SHAREDRES_CLI_SCR_TY    (((PG_RES)mgSharedRes)->cli_scr_ty)
#define SHAREDRES_CLI_SCR_RX    (((PG_RES)mgSharedRes)->cli_scr_rx)
#define SHAREDRES_CLI_SCR_BY    (((PG_RES)mgSharedRes)->cli_scr_by)
#define SHAREDRES_CLI_SCR_RC    ((RECT*)(&((PG_RES)mgSharedRes)->cli_scr_lx))
#define SHAREDRES_TOPMOST_LAYER (((PG_RES)mgSharedRes)->topmost_layer)

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#ifdef __cplusplus

#endif  /* __cplusplus */

#endif // GUI_SHAREDRES_H

