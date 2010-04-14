/*
** $Id: drawsemop.h,v 1.10 2003/08/12 07:46:18 weiym Exp $
**
** drawsemop.h: operations for drawing semaphore.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
*/

#ifndef GUI_DRAWSEMOP_H
    #define GUI_DRAWSEMOP_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void unlock_draw_sem (void);
void lock_draw_sem (void);

#ifdef _CURSOR_SUPPORT

#include <errno.h>

static inline void cursor_sem_op (short op)
{
    struct sembuf sb;

again:
    sb.sem_num = 1;
    sb.sem_op = op;
    sb.sem_flg = SEM_UNDO;

    if (semop (SHAREDRES_SEMID, &sb, 1) == -1) {
        if (errno == EINTR) {
            goto again;
        }
    }
}

#define lock_cursor_sem()   cursor_sem_op(-1)
#define unlock_cursor_sem() cursor_sem_op(1)

static inline int get_sem_pid (int num)
{
    int pid;
    union semun ignored;

    pid = semctl (SHAREDRES_SEMID, num, GETPID, ignored);
#ifdef _DEBUG
    if (pid == -1)
        perror ("get_sem_pid");
#endif

    return pid;
}

#define get_cursor_sem_pid() get_sem_pid(1)

inline static int get_hidecursor_sem_val (void)
{
    int val;
    union semun ignored;

    val = semctl (SHAREDRES_SEMID, 2, GETVAL, ignored);
#ifdef _DEBUG
    if (val == -1)
        perror ("get_hidecursor_sem_val");
#endif

    return val;
}

inline static void reset_hidecursor_sem (void)
{
    int ret;
    union semun sunion;

    sunion.val = 0;
    ret = semctl (SHAREDRES_SEMID, 2, SETVAL, sunion) == -1;
#ifdef _DEBUG
    if (ret == -1)
        perror ("reset_hidecursor_sem");
#endif
}

void inc_hidecursor_sem (void);

#endif /* _CURSOR_SUPPORT */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_DRAWSEMOP_H

