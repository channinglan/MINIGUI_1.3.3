/*
** $Id: ourhdr.h,v 1.5 2003/08/12 07:46:18 weiym Exp $
**
** ourhdr.h: the head file from APUE.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming
**
** Create date: 2000/06/11
*/

#ifndef    __ourhdr_h
#define    __ourhdr_h

#include <sys/types.h>    /* required for some of our prototypes */
#include <stdio.h>        /* for convenience */
#include <stdlib.h>        /* for convenience */
#include <string.h>        /* for convenience */
#include <unistd.h>        /* for convenience */
#include <syslog.h>        /* for convenience */
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
    int val;                    /* value for SETVAL */
    struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
    unsigned short int *array;  /* array for GETALL, SETALL */
    struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif

#define MAXLINE             4096        /* max line length */
#define MAXDATALEN          4096        /* max data length */

void err_dump(const char *, ...);    /* {App misc_source} */
void err_msg(const char *, ...);
void err_quit(const char *, ...);
void err_ret(const char *, ...);
void err_sys(const char *, ...);

void log_msg(const char *, ...);        /* {App misc_source} */
void log_open(const char *, int, int);
void log_quit(const char *, ...);
void log_ret(const char *, ...);
void log_sys(const char *, ...);

typedef struct listen_fd {
    int fd;
    int hwnd;
    int type;
    void* context;
} LISTEN_FD;

extern fd_set      mg_rfdset;
extern fd_set*     mg_wfdset;
extern fd_set*     mg_efdset;
extern int         mg_maxfd;
extern LISTEN_FD   mg_listen_fds [];

#endif    /* __ourhdr_h */

