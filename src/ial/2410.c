/*
** $Id: 2410.c,v 1.4 2003/11/21 12:15:37 weiym Exp $
**
** 2410.c: Low Level Input Engine for SMDK2410 Dev Board.
** 
** Copyright (C) 2003 Feynman Software.
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
#include <unistd.h>
#include <fcntl.h>

#include "common.h"

#ifdef _SMDK2410_IAL

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/kd.h>

#include <linux/input.h>

#include "ial.h"
#include "2410.h"

//#define _2410DEBUG


/* for data reading from /dev/ts */
typedef struct {
    unsigned int pressure;
    int x;
    int y;
    unsigned short pad;
} TS_EVENT;


static int ts = -1;
static int mousex = 0;
static int mousey = 0;
static TS_EVENT ts_event;


/* Interface to Keyboard Device Driver*/
typedef struct _kbddevice {
    int  (*Open)(void);
    void (*Close)(void);
    void (*GetModifierInfo)(int *modifiers);
    int  (*Read)(unsigned char *buf,int *modifiers);
    void (*Suspend) (void);
    int  (*Resume) (void);
} KBDDEVICE;

extern KBDDEVICE kbddev_tty;

static unsigned char state [NR_KEYS];
static int kbd_fd;
static KBDDEVICE * kbddev = &kbddev_tty;


////////////////////////////////////////////////////////////////////////////////////
/* for ts calibration*/

struct ts_sample {
	int		x;
	int		y;
	unsigned int	pressure;
	struct timeval	tv;
};

static struct ts_sample cur_samp;


static struct ts_sample ts_position[4];//={{230,245,0},{230,15,0},{25,15,0},{15,245,0}};
static struct ts_sample display_position[4];//={{0,0,0},{319,0,0},{319,239,0},{0,239,0}};

static struct matrix {
/* This arrangement of values facilitates
*  calculations within get_display_point()
*/
	int An,     /* A = An/Divider */
		Bn,     /* B = Bn/Divider */
		Cn,     /* C = Cn/Divider */
		Dn,     /* D = Dn/Divider */
		En,     /* E = En/Divider */
		Fn,     /* F = Fn/Divider */
		Divider ;
} matrix;

static int
set_calibration_matrix(struct ts_sample *displayPtr,struct ts_sample *screenPtr, struct matrix *matrixPtr)
{
	int retvalue = 0;

	matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
							((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y));

	if (matrixPtr->Divider == 0) {
		retvalue = -1 ;
	} else {
		matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
						((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y));

		matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) -
						((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x));

		matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
						(screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
						(screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y;

		matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) -
						((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y));
 
		matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) -
						((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x));

		matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
						(screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
						(screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y;
	}

	return retvalue;

} /* end of set_calibration_matrix() */

static int
get_display_point(struct ts_sample *displayPtr, struct ts_sample *screenPtr, struct matrix *matrixPtr)
{
	int retvalue = 0;


#ifdef _2410DEBUG
	fprintf(stderr, "Time = %d.%d\n",
			 (int)screenPtr->tv.tv_sec, (int)screenPtr->tv.tv_usec);
	fprintf(stderr, "screenPtr: X = %d, Y = %d, Pressure = %d\n",
			screenPtr->x, screenPtr->y, screenPtr->pressure);
#endif	/*_DEBUG*/
	screenPtr->x >>= 4;	//driver is 12bit
	screenPtr->y >>= 4;
//#ifdef _2410DEBUG
	fprintf(stderr, "screenPtr: X = %d, Y = %d, Pressure = %d\n",
			screenPtr->x, screenPtr->y, screenPtr->pressure);
//#endif	/*_DEBUG*/

	if (matrixPtr->Divider != 0) {

		/* Operation order is important since we are doing integer */
		/*  math. Make sure you add all terms together before      */
		/*  dividing, so that the remainder is not rounded off     */
		/*  prematurely.                                           */
		memcpy(displayPtr, screenPtr, sizeof(*screenPtr));
		displayPtr->x = ( (matrixPtr->An * screenPtr->x) + (matrixPtr->Bn * screenPtr->y) + matrixPtr->Cn ) / matrixPtr->Divider;
		displayPtr->y = ( (matrixPtr->Dn * screenPtr->x) + (matrixPtr->En * screenPtr->y) + matrixPtr->Fn ) / matrixPtr->Divider;
	} else {
		retvalue = -1 ;
	}

#ifdef _2410DEBUG
	fprintf(stderr, "get_display_point: X = %d, Y = %d, Pressure = %d\n",
			displayPtr->x, displayPtr->y, displayPtr->pressure);
#endif	/*_DEBUG*/

	return retvalue;

} /* end of get_display_point() */


static int
ts_read_cal(void)
{
	FILE *file;
	int i;

	char cal_file[] = "/usr/local/etc/tscal";
	
#ifdef _2410DEBUG
    fprintf (stderr, "ts_read_cal .\n");
#endif 
	
	if ((file = fopen (cal_file, "r+")) == NULL) {
		printf("Can not open calibration file %s\n", cal_file);
		return -1;
	}

	for (i = 0; i < 4; i++) {
		int ret = fscanf(file, "%d %d\n", &display_position[i].x, &display_position[i].y);
#ifdef _2410DEBUG
		printf("read %d %d\n", display_position[i].x, display_position[i].y);
#endif
		if (ret < 0) {
			printf("Read display_position data fail!\n");
			fclose(file);
			return -1;
		}
	}

	for (i = 0; i < 4; i++) {
		int ret = fscanf(file, "%d %d\n", &ts_position[i].x, &ts_position[i].y);
#ifdef _2410DEBUG
		printf("read %d %d\n", ts_position[i].x, ts_position[i].y);
#endif
		if (ret < 0) {
			printf("Read ts_position data fail!\n");
			fclose(file);
			return -1;
		}
	}

	fclose(file);
	return 0;
}

extern int 
ts_open(const char *name, int nonblock)
{
	int ts, flags = O_RDONLY;

#ifdef _2410DEBUG
    fprintf (stderr, "ts_open .\n");
#endif 	
	
	if (ts_read_cal())
		return -1;

	if (nonblock)
		flags |= O_NONBLOCK;
	
	ts = open(name, flags);

	if (ts < 0) {
		fprintf (stderr, "Can not open touch screen! %s\n", name);
		goto fail;
	}

	set_calibration_matrix(display_position, ts_position, &matrix);

fail:
	return ts;
}

extern int
ts_close(int ts)
{
	return close(ts);
}

extern int
ts_read_raw(int ts, struct ts_sample *get_input)
{
	int ret = 0;
	int total = 0;
	struct input_event ev;
	struct ts_sample *cur = &cur_samp;

	while (1) {
//		printf("total = %d\n", total);
		ret = read(ts, &ev, sizeof(ev));
		
		if (ret < (int)sizeof(ev)) {
			printf("Error input device!\n");
			total = -1;
			break;
		}


		switch (ev.type) {
		case EV_SYN:
			/* Fill out a new complete event */
			get_input->x = cur->x;
			get_input->y = cur->y;
			get_input->pressure = cur->pressure;
			get_input->tv = ev.time;
#ifdef _2410DEBUG
			fprintf(stderr, "ts_read_raw: X = %d, Y = %d, Pressure = %d, Time = %d.%d\n",
					get_input->x, get_input->y, get_input->pressure, (int)get_input->tv.tv_sec, (int)get_input->tv.tv_usec);
#endif	/*_DEBUG*/
			return total;
//			break;
		case EV_ABS:
			switch (ev.code) {
			case ABS_X:
				cur->x = ev.value;
				break;
			case ABS_Y:
				cur->y = ev.value;
				break;
			case ABS_PRESSURE:
				cur->pressure = ev.value;
				break;
			}
			break;
		}
	}
	ret = total;

	return ret;
}

extern int
ts_read(int ts, struct ts_sample *get_input)
{
	struct ts_sample cal_input;
//	static int last_pressure = 0;
	static int old_x, old_y, last_pressure = 0;

//	if (last_pressure == 0)
//		ts_read_raw(ts, get_input);
	ts_read_raw(ts, get_input);
	get_display_point(&cal_input, get_input, &matrix);
	memcpy(get_input, &cal_input, sizeof(cal_input));
//	last_pressure = get_input->pressure;
#if 1
	if ((last_pressure == 0) || (abs(get_input->x - old_x) < 7) || (abs(get_input->y - old_y) < 7)) {
//		printf("< 7\n");
		old_x = get_input->x;
		old_y = get_input->y;
	}
	
	get_input->x = old_x;
	get_input->y = old_y;
	last_pressure = get_input->pressure;
#endif
#ifdef _2410DEBUG
	fprintf(stderr, "ts_read: X = %d, Y = %d, Pressure = %d, Time = %d.%d\n",
			get_input->x, get_input->y, get_input->pressure, (int)get_input->tv.tv_sec, (int)get_input->tv.tv_usec);
#endif	/*_DEBUG*/
	return 0;
}

/************************  Low Level Input Operations **********************/
/*
 * Mouse operations -- Event
 */
static int mouse_update(void)
{
//	printf("mouse_update\n");
    return 1;
}

static void mouse_getxy(int *x, int* y)
{
    extern RECT g_rcScr;

#ifdef _2410DEBUG
    printf ("mouse_getxy g_rcScr.bottom=%d, g_rcScr.right=%d, mousex = %d, mousey = %d\n", g_rcScr.bottom,  g_rcScr.right, mousex, mousey);
#endif

    if (mousex < 0)
		mousex = 0;
    if (mousey < 0)
		mousey = 0;

#ifdef _COOR_TRANS
#if _ROT_DIR_CCW
    if (mousex >= g_rcScr.bottom)
		mousex = g_rcScr.bottom - 1;
    if (mousey >= g_rcScr.right)
		mousey = g_rcScr.right - 1;
    *x = g_rcScr.right - mousey;
    *y = mousex;
#else
    if (mousex >= g_rcScr.bottom)
		mousex = g_rcScr.bottom - 1;
    if (mousey >= g_rcScr.right)
		mousey = g_rcScr.right - 1;
    *x = mousey;
    *y = g_rcScr.bottom - mousex;
#endif

#else	//_COOR_TRANS

    if (mousex >= g_rcScr.right)
		mousex = g_rcScr.right - 1;
    if (mousey >=  g_rcScr.bottom)
		mousey = g_rcScr.bottom - 1;
    *x = mousex;
    *y = mousey;

#endif
}


static int mouse_getbutton(void)
{
#ifdef _2410DEBUG
	printf("mouse_getbutton %d\n", ts_event.pressure);
#endif
    return ts_event.pressure;
}

#ifdef _LITE_VERSION 
static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
#else
static int wait_event (int which, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
#endif
{
    fd_set rfds;
    int    retvalue = 0;
    int    e;

    if (!in) {
        in = &rfds;
        FD_ZERO (in);
    }

    if ((which & IAL_MOUSEEVENT) && ts >= 0) {
        FD_SET (ts, in);
#ifdef _LITE_VERSION
        if (ts > maxfd) maxfd = ts;
#endif
    }

    if (which & IAL_KEYEVENT && kbd_fd >= 0){
        FD_SET (kbd_fd, in);
#ifdef _LITE_VERSION
        if (kbd_fd > maxfd) maxfd = kbd_fd;
#endif
    }

#ifdef _LITE_VERSION
    e = select (maxfd + 1, in, out, except, timeout) ;
#else
    e = select (FD_SETSIZE, in, out, except, timeout) ;
#endif

    if (e > 0) {
        if (ts >= 0 && FD_ISSET (ts, in)) {
            FD_CLR (ts, in);

            {
	            struct ts_sample get_input;

	            ts_read(ts, &get_input);

	            ts_event.x = get_input.x;
	            ts_event.y = get_input.y;
	            ts_event.pressure = get_input.pressure;

#ifdef _2410DEBUG
	            printf ("ts_event.x = %d, ts_event.y = %d, ts_event.pressure = %d\n", ts_event.x, ts_event.y, ts_event.pressure);
#endif
            }

            if (ts_event.pressure > 0) {
                mousex = ts_event.x;
                mousey = ts_event.y;
            }

			ts_event.pressure = ( ts_event.pressure > 0 ? 4 : 0);

//#ifdef _2410DEBUG
            if (ts_event.pressure > 0) {
                printf ("down: x = %4d, y = %4d\n", mousex, mousey);                
            }
//#endif   

            retvalue |= IAL_MOUSEEVENT;
        }

        if (kbd_fd >= 0 && FD_ISSET (kbd_fd, in)) {
            FD_CLR (kbd_fd, in);
            retvalue |= IAL_KEYEVENT;
        }

    } 
    else if (e < 0) {
        return -1;
    }

    return retvalue;
}

/*
 * 0 : no data
 * > 0 : the possible maximal key scancode
 */
static int keyboard_update(void)
{
    unsigned char buf;
    int modifier;
    int ch;
    int is_pressed;
    int retvalue;

    retvalue = kbddev->Read (&buf, &modifier); 

    if ((retvalue == -1) || (retvalue == 0))
        return 0;

    else { /* retvalue > 0 */
        is_pressed = !(buf & 0x80);
        ch         = buf & 0x7f;
        if (is_pressed) {
#ifdef _LITE_VERSION
            if ((ch >= SCANCODE_F1) && (ch <= SCANCODE_F6) && 
                            (state[SCANCODE_RIGHTCONTROL]==1)) 
            {
                vtswitch_try (ch - SCANCODE_F1 + 1);
                state [SCANCODE_RIGHTCONTROL] = 0;
                return 0;
            }
#endif
            state[ch] = 1;
        }
        else 
            state[ch] = 0;
    }

    return NR_KEYS;
}

static const char* keyboard_getstate(void)
{
    return state;
}

BOOL Init2410Input (INPUT* input, const char* mdev, const char* mtype)
{
    kbd_fd = kbddev->Open();

//#ifdef _2410DEBUG
    fprintf (stderr, "Init2410Input .\n");
//#endif    
    
    if (kbd_fd < 0) {
        perror ("Can not open tty!");
        return FALSE;
    }

//    ts = open ("/dev/ts1", O_RDONLY);
    ts = ts_open(mdev, 0);
    if (ts < 0) {
        kbddev->Close();
        return FALSE;
    }

    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;

    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_getstate;
    
    input->wait_event = wait_event;

    mousex = 0;
    mousey = 0;
    ts_event.x = ts_event.y = ts_event.pressure = 0;
    
    return TRUE;
}

void Term2410Input(void) 
{
    if (ts >= 0)
    	ts_close(ts);

	kbddev->Close();
}


#endif /* _SMDK2410_IAL */

