/*
** $Id: sharedres.c,v 1.44 2003/09/29 02:52:46 snig Exp $
** 
** sharedres.c: Load and init shared resource.
** 
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming
**
** Create date: 2000/12/22
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
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/mman.h>

#include "common.h"

#ifndef _STAND_ALONE

#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "cursor.h"
#include "icon.h"
#include "menu.h"
#include "ial.h"
#include "ourhdr.h"
#include "client.h"
#include "server.h"
#include "sharedres.h"
#include "misc.h"

#define SHM_PARAM 0644
#define SEM_PARAM 0666

// define to use mmap instead of shared memory.
#undef  _USE_MMAP 
// #define _USE_MMAP 1

#define _NR_SEM     1

#ifdef _CURSOR_SUPPORT
#undef _NR_SEM
#define _NR_SEM     3

#define CURSORSECTION   "cursorinfo"

static BOOL LoadCursorRes (void)
{
    int number;
    int i;
    PCURSOR tempcsr;
    char *temp;
    char szValue [12];

#ifdef _USE_NEWGAL
    __mg_csrimgsize = GAL_GetBoxSize (__gal_screen, CURSORWIDTH, CURSORHEIGHT, &__mg_csrimgpitch);
#else
    __mg_csrimgsize = CURSORWIDTH * CURSORHEIGHT * BYTESPERPHYPIXEL;
#endif

    if (GetMgEtcValue (CURSORSECTION, "cursornumber", szValue, 10) < 0)
        goto error;

    number = atoi (szValue);
    if (number < 0) goto error;
    number = number < (MAX_SYSCURSORINDEX + 1) ? number : (MAX_SYSCURSORINDEX + 1);

    // realloc for shared resource
    mgSharedRes = realloc (mgSharedRes, mgSizeRes + __mg_csrimgsize +
                    number * (sizeof (HCURSOR) + sizeof (CURSOR) + 2*__mg_csrimgsize));
    if (mgSharedRes == NULL) {
        perror ("realloc shared memory for system cursor");
        return FALSE;
    }

    // set cursor number
    ((PG_RES)mgSharedRes)->csrnum    = number;
    // set cursor data offset
    ((PG_RES)mgSharedRes)->svdbitsoffset = mgSizeRes;
    mgSizeRes += __mg_csrimgsize;
    ((PG_RES)mgSharedRes)->csroffset = mgSizeRes;

    // pointer to begin of cursor struct, 
    // and reserve a space for handles for system cursors.
    temp = (char*)mgSharedRes + mgSizeRes + sizeof (PCURSOR) * number;

    for (i = 0; i < number; i++) {
        if ( !(tempcsr = LoadSystemCursor (i)) )
            goto error;

        memcpy (temp, tempcsr, sizeof(CURSOR));        
        temp += sizeof(CURSOR);
        memcpy (temp, tempcsr->AndBits, __mg_csrimgsize);
        temp += __mg_csrimgsize; 
        memcpy (temp, tempcsr->XorBits, __mg_csrimgsize);
        temp += __mg_csrimgsize; 
        free (tempcsr->AndBits);
        free (tempcsr->XorBits);
        free (tempcsr);
    }

    mgSizeRes += (sizeof (HCURSOR) + sizeof(CURSOR) + 2 * __mg_csrimgsize) * number;
    return TRUE;

error:
    return FALSE;
}

#endif /* _CURSOR_SUPPORT */

static BOOL LoadIconRes (void)
{
    int i, nIconNr;
    size_t size_bits16, size_bits32;
    PICON picon1,picon2;
    char* temp;
    char szValue [12];

    size_bits16 = BYTESPERPHYPIXEL * 16 * 16;
    size_bits32 = BYTESPERPHYPIXEL * 32 * 32;

    if (GetMgEtcValue ("iconinfo", "iconnumber", szValue, 10) < 0 )
        return FALSE;
    nIconNr = atoi(szValue);
    if (nIconNr <= 0) return FALSE;
    nIconNr = nIconNr < SYSICO_ITEM_NUMBER ? nIconNr : SYSICO_ITEM_NUMBER;

    //realloc for shared resource
    mgSharedRes = realloc (mgSharedRes, 
            mgSizeRes + nIconNr * (sizeof(ICON) + size_bits16 + size_bits32) * 2);
    if (mgSharedRes == NULL) {
        perror ("realloc shared memory for system icon");
        return FALSE;
    }

    //set icon number
    ((PG_RES)mgSharedRes)->iconnum    = nIconNr;
    //set icon data offset
    ((PG_RES)mgSharedRes)->iconoffset = mgSizeRes;
    //point to begin of icon struct
    temp= (char*)mgSharedRes + mgSizeRes;

    for (i = 0; i < nIconNr; i++) {
        sprintf (szValue, "icon%d", i);
        picon1 = (PICON)LoadSystemIcon (szValue, 0);
        picon2 = (PICON)LoadSystemIcon (szValue, 1);

        if (picon1 == 0 || picon2 == 0)
            return FALSE;

        memcpy (temp, picon1, sizeof(ICON));
        temp += sizeof(ICON);
        memcpy (temp, picon1->AndBits, size_bits32);
        temp += size_bits32;
        memcpy (temp, picon1->XorBits, size_bits32);
        temp += size_bits32;

        memcpy (temp, picon2, sizeof(ICON));
        temp += sizeof(ICON);
        memcpy (temp, picon2->AndBits, size_bits16);
        temp += size_bits16;
        memcpy (temp, picon2->XorBits, size_bits16);
        temp += size_bits16;

        DestroyIcon ((HICON)picon1);
        DestroyIcon ((HICON)picon2);
    }

    //set mgSizeRes to new
    mgSizeRes += (sizeof(ICON) + size_bits32 + size_bits16) * nIconNr * 2;
    return TRUE;
}

/***************************Bitmap Support***************************/
static BOOL LoadBitmapRes (void)
{
    int i, nBmpNr, size;
    char *temp;
    BITMAP bmp;
    char szValue [12];

    if (GetMgEtcValue ("bitmapinfo", "bitmapnumber", szValue, 10) < 0)
        return FALSE;

    nBmpNr = atoi(szValue);
    if (nBmpNr <= 0) return FALSE;
    nBmpNr = nBmpNr < SYSBMP_ITEM_NUMBER ? nBmpNr : SYSBMP_ITEM_NUMBER;

    ((PG_RES)mgSharedRes)->bmpnum    = nBmpNr;
    ((PG_RES)mgSharedRes)->bmpoffset = mgSizeRes;

    for (i = 0; i < nBmpNr; i++) {
        sprintf (szValue, "bitmap%d", i);
        if (!LoadSystemBitmap (&bmp, szValue))
            return FALSE;

#ifdef _USE_NEWGAL
        size = bmp.bmHeight * bmp.bmPitch;
#else
        size = bmp.bmHeight * bmp.bmWidth * BYTESPERPHYPIXEL;
#endif
        if ((mgSharedRes = realloc (mgSharedRes, mgSizeRes + size + sizeof (BITMAP))) == NULL) {
            UnloadBitmap (&bmp);
            perror ("realloc shared memory for system bitmap");
            return FALSE;
        }

        temp= (char*)mgSharedRes + mgSizeRes;
        memcpy (temp, &bmp, sizeof(BITMAP));
        temp += sizeof (BITMAP);
        memcpy (temp, bmp.bmBits, size);

        mgSizeRes += size;
        mgSizeRes += sizeof (BITMAP);

        UnloadBitmap (&bmp);
    }

    return TRUE;
}

inline static key_t get_shm_key (void)
{
    return ftok ("/etc/passwd", 'm');
}

inline static key_t get_sem_key (void)
{
    return ftok ("/etc/passwd", 'g');
}

BOOL IsOnlyMe (void)
{
    int fd;

    if ((fd = open (LOCKFILE, O_RDONLY)) == -1)
        return TRUE;

    close (fd);
    return FALSE;
}

void delete_sem (void)
{
    union semun ignored;
    if (semctl (SHAREDRES_SEMID, 0, IPC_RMID, ignored) < 0)
        goto error;

    return;

error:
    perror("remove semaphore");
}

void *LoadSharedResource (void)
{
    key_t sem_key;
#ifndef _USE_MMAP
    key_t shm_key;
    void *memptr;
#endif
    int lockfd, semid;
#ifndef _USE_MMAP
    int shmid;
#endif
    union semun sunion;
    PG_RES pG_res = (PG_RES) calloc (1, sizeof(G_RES));

    mgSharedRes = pG_res;
    mgSizeRes = sizeof (G_RES);

#ifdef _CURSOR_SUPPORT
    if (!LoadCursorRes()) {
        perror ("InitCursor"); 
        return NULL;
    }
#endif

    if (!LoadIconRes()) {
        perror ("InitIcon"); 
        return NULL;
    }

    if (!LoadBitmapRes()) {
        perror ("InitBitmap"); 
        return NULL;
    }

#ifndef _USE_MMAP
    if ((shm_key = get_shm_key ()) == -1) {
        goto error;
    }
    shmid = shmget (shm_key, mgSizeRes, SHM_PARAM | IPC_CREAT | IPC_EXCL); 
    if (shmid == -1) { 
        goto error;
    } 

    // Attach to the share memory. 
    memptr = shmat (shmid, 0, 0);
    if (memptr == (char*)-1) 
        goto error;
    else {
        memcpy (memptr, mgSharedRes, mgSizeRes);
        free (mgSharedRes);
    }

    if (shmctl (shmid, IPC_RMID, NULL) < 0) 
        goto error;
#endif

    // Write shmid into the lock file.
    if ((lockfd = open (LOCKFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
        goto error;

#ifdef _USE_MMAP
    if (write (lockfd, mgSharedRes, mgSizeRes) < mgSizeRes)
        goto error;
    else
    {
        free(mgSharedRes);
        mgSharedRes = mmap( 0, mgSizeRes, PROT_READ|PROT_WRITE, MAP_SHARED, lockfd, 0);
    }
#else
    if (write (lockfd, &shmid, sizeof (shmid)) < sizeof (shmid))
        goto error;
#endif
#if 0
    if (flock (lockfd, LOCK_EX | LOCK_NB) == -1)
        goto error;
#endif

    close (lockfd);

    // Obtain the semophore syncing drawing.
    if ((sem_key = get_sem_key ()) == -1) {
        goto error;
    }
    semid = semget (sem_key, _NR_SEM, SEM_PARAM | IPC_CREAT | IPC_EXCL); 
    if (semid == -1) { 
        goto error;
    }
    atexit (delete_sem);

    // Initially drawing and cursor all should be available.
    sunion.val = 1;
    semctl (semid, 0, SETVAL, sunion);
#ifdef _CURSOR_SUPPORT
    sunion.val = 1;
    semctl (semid, 1, SETVAL, sunion);
    sunion.val = 0;
    semctl (semid, 2, SETVAL, sunion);
#endif /* _CURSOR_SUPPORT */

#ifndef _USE_MMAP
    mgSharedRes = memptr;
    SHAREDRES_SHMID = shmid;
#endif
    SHAREDRES_SEMID = semid;

    return mgSharedRes; 

error:
    perror ("LoadSharedResource"); 
    return NULL;
} 

void UnloadSharedResource (void)
{
    unlink (LOCKFILE);
}

void* AttachSharedResource (void)
{
#ifndef _USE_MMAP
    int shmid;
#endif
    int lockfd;
    void* memptr;

    if ((lockfd = open (LOCKFILE, O_RDONLY)) == -1)
        goto error;

#ifdef _USE_MMAP
    mgSizeRes = lseek (lockfd, 0, SEEK_END );
    memptr = mmap( 0, mgSizeRes, PROT_READ, MAP_SHARED, lockfd, 0);
#else
    if (read (lockfd, &shmid, sizeof (shmid)) < sizeof (shmid))
        goto error;
    close (lockfd);

    memptr = shmat (shmid, 0, SHM_RDONLY);
#endif
    if (memptr == (char*)-1) 
        goto error;
    return memptr;

error:
    perror ("AttachSharedResource"); 
    return NULL;
}

void UnattachSharedResource (void)
{
#ifdef _USE_MMAP
    if (munmap(mgSharedRes, mgSizeRes))
        perror("detaches shared resource");        
#else
    if (shmdt (mgSharedRes) < 0)
        perror("detaches shared resource");        
#endif
}

#endif /* !_STAND_ALONE */
