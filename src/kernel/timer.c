/*
** $Id: timer.c,v 1.28 2003/11/22 08:01:37 weiym Exp $
**
** timer.c: The Timer module for MiniGUI-Threads.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 1999/04/21
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

/*
** TODO:
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "timer.h"

unsigned int __mg_timer_counter = 0;

static struct sigaction old_alarm_handler;
static TIMER *timerstr[MAX_TIMERS];

#ifndef __ECOS

static struct itimerval old_timer;

static void sig_handler (int v)
{
    int sem_value;

    __mg_timer_counter++;

    // alert desktop
    __mg_dsk_msgs.dwState |= 0x01;
    sem_getvalue (&__mg_dsk_msgs.wait, &sem_value);
    if (sem_value == 0)
        sem_post (&__mg_dsk_msgs.wait);
}

BOOL InitTimer (void)
{
    struct itimerval timerv;
    struct sigaction siga;
    
    siga.sa_handler = sig_handler;
    siga.sa_flags = 0;
    
    memset (&siga.sa_mask, 0, sizeof (sigset_t));

    sigaction (SIGALRM, &siga, &old_alarm_handler);

    timerv.it_interval.tv_sec = 0;
    timerv.it_interval.tv_usec = 10000;     // 10 ms
    timerv.it_value = timerv.it_interval;

    if (setitimer (ITIMER_REAL, &timerv, &old_timer)) {
        fprintf(stderr, "TIMER: setitimer call failed!\n");
        perror("setitimer");
		return FALSE;
    }

    __mg_timer_counter = 0;

    return TRUE;
}

void TerminateTimer ()
{
    int i;

    if (setitimer (ITIMER_REAL, &old_timer, 0) == -1) {
        fprintf( stderr, "TIMER: setitimer call failed!\n");
        perror("setitimer");
        return;
    }

    if (sigaction (SIGALRM, &old_alarm_handler, NULL) == -1) {
        fprintf( stderr, "TIMER: sigaction call failed!\n");
        perror("sigaction");
    	return;
    }

    for (i=0; i<MAX_TIMERS; i++) {
        if (timerstr[i] != NULL)
            free ( timerstr[i] );
        timerstr[i] = NULL;
    }
}

#else
#include "timer_posix.c"
#endif

/************************* Functions run in desktop thread *******************/
void DispatchTimerMessage (unsigned int inter)
{
    PMAINWIN pWin;
    PMSGQUEUE pMsgQueue;
    int i, slot;
    int sem_value;

    for ( i=0; i<MAX_TIMERS; i++ ) {
        if ( timerstr[i] ) {
#if _TIMER_UNIT_10MS
    	    timerstr[i]->count += inter;
#else
    	    timerstr[i]->count += 1<<7;
#endif
            if ( timerstr[i]->count >= timerstr[i]->speed ) {

                pWin = GetMainWindowPtrOfControl (timerstr[i]->hWnd);
                if (pWin == NULL) continue;

                pMsgQueue = pWin->pMessages;

                pthread_mutex_lock (&pMsgQueue->lock);
                for (slot=0; slot<8; slot++) {
                    if (pMsgQueue->TimerID[slot] == timerstr[i]->id
                        && pMsgQueue->TimerOwner[slot] == timerstr[i]->hWnd)
                        break;
                }
                if (slot != 8) {
                    pMsgQueue->dwState |= (0x01 << slot);
                    sem_getvalue (&pMsgQueue->wait, &sem_value);
                    if (sem_value == 0)
                        sem_post(&pMsgQueue->wait);
                }
                pthread_mutex_unlock (&pMsgQueue->lock);
                
                timerstr[i]->count -= timerstr[i]->speed;
            }
        }
    }
}

BOOL AddTimer (HWND hWnd, int id, unsigned int speed)
{
#if 0
    sigset_t sa_mask;
#endif

    int i;
    PMAINWIN pWin;
    PMSGQUEUE pMsgQueue;
    int slot;

    if (!(pWin = GetMainWindowPtrOfControl (hWnd))) return FALSE;

#if 0
    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif

    pMsgQueue = pWin->pMessages;
    // Is there a empty timer slot?
    for (slot=0; slot<8; slot++) {
        if ((pMsgQueue->TimerMask >> slot) & 0x01)
            break;
    }
    if (slot == 8) goto badret;

    for (i=0; i<MAX_TIMERS; i++)
        if (timerstr[i] != NULL)
            if (timerstr[i]->hWnd == hWnd && timerstr[i]->id == id)
                goto badret;

    for (i=0; i<MAX_TIMERS; i++)
        if (timerstr[i] == NULL)
            break;

    if (i == MAX_TIMERS)
        goto badret ;

    timerstr[i] = malloc (sizeof (TIMER));

#if _TIMER_UNIT_10MS
    timerstr[i]->speed = speed;
#else
    timerstr[i]->speed = (1000<<7)/speed;
#endif
    timerstr[i]->hWnd = hWnd;
    timerstr[i]->id = id;
    timerstr[i]->count = 0;

    pMsgQueue->TimerOwner[slot] = hWnd;
    pMsgQueue->TimerID[slot] = id;
    pMsgQueue->TimerMask &= ~(0x01 << slot);

#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return TRUE;
    
badret:

#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

BOOL RemoveTimer (HWND hWnd, int id)
{
#if 0
    sigset_t sa_mask;
#endif

    int i;
    PMAINWIN pWin;
    PMSGQUEUE pMsgQueue;
    int slot;
    void* temp;

    if (!(pWin = GetMainWindowPtrOfControl (hWnd))) return FALSE;

#if 0
    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif
    
    pMsgQueue = pWin->pMessages;
    for (slot=0; slot<8; slot++) {
        if (pMsgQueue->TimerID[slot] == id
                    && pMsgQueue->TimerOwner[slot] == hWnd)
            break;
    }
    if (slot == 8) goto badret;

    for (i=0; i<MAX_TIMERS; i++)
        if (timerstr[i] != NULL)
            if (timerstr[i]->hWnd == hWnd && timerstr[i]->id == id)
                break;

    if (i == MAX_TIMERS) goto badret;
    
    temp = timerstr[i];
    timerstr[i] = NULL;
    free (temp);

    pMsgQueue->TimerMask |= (0x01 << slot);

#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif
    
    return TRUE;

badret:
#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

BOOL GUIAPI IsTimerInstalled (HWND hWnd, int id)
{
    int i;

#if 0
    sigset_t sa_mask;

    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif

    for (i=0; i<MAX_TIMERS; i++) {
        if ( timerstr[i] != NULL ) {
            if ( timerstr[i]->hWnd == hWnd && timerstr[i]->id == id) {
#if 0
                pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif
                return TRUE;
            }
        }
    }

#if 0
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

BOOL SetTimerSpeed (HWND hWnd, int id, unsigned int speed)
{
    int i;
    
#if 0
    sigset_t sa_mask;

    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif

    for (i=0; i<MAX_TIMERS; i++)
	if (timerstr[i]->hWnd == hWnd && timerstr[i]->id == id) {
#if _TIMER_UNIT_10MS
		timerstr[i]->speed = speed;
#else
		timerstr[i]->speed = (1000<<7)/speed;
#endif
		timerstr[i]->count = 0;
#if 0
        pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif
        return TRUE;
	}

#if 0
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

/****************** Timer Interfaces for applications ************************/
unsigned int GUIAPI GetTickCount ()
{
    return __mg_timer_counter;
}

BOOL GUIAPI SetTimer (HWND hWnd, int id, unsigned int speed)
{
    TIMER timer;
    
    timer.hWnd = hWnd;
    timer.id = id;
    timer.speed = speed;
    
    return SendMessage (HWND_DESKTOP, MSG_ADDTIMER, 0, (LPARAM)&timer);
}

BOOL GUIAPI KillTimer (HWND hWnd, int id)
{
    TIMER timer;
    
    timer.hWnd = hWnd;
    timer.id = id;
    
    return SendMessage (HWND_DESKTOP, MSG_REMOVETIMER, 0, (LPARAM)&timer);
}

BOOL GUIAPI ResetTimer (HWND hWnd, int id, unsigned int speed)
{
    TIMER timer;
    
    timer.hWnd = hWnd;
    timer.id = id;
    timer.speed = speed;
    
    return SendMessage (HWND_DESKTOP, MSG_RESETTIMER, 0, (LPARAM)&timer);
}

