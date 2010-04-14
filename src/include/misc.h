/*
** $Id: misc.h,v 1.13 2003/09/26 08:45:14 snig Exp $
**
** misc.h: the head file for Miscellous module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Create date: 1999/01/03
*/

#ifndef GUI_MISC_H
    #define GUI_MISC_H

/* Function definitions */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


typedef struct _ETCSECTION
{
    int key_nr;               /* key number in the section */
    char *name;               /* name of the section */
    char **keys;              /* key string arrays */
    char **values;            /* value string arrays */
} ETCSECTION;
typedef ETCSECTION* PETCSECTION;

typedef struct _ETC_S
{
    int section_nr;           /* number of sections */
    PETCSECTION sections;     /* pointer to section arrays */
} ETC_S;


extern GHANDLE hMgEtc;

#ifndef _INCORE_RES

BOOL InitMisc (void);
static inline void TerminateMisc (void) {}

/* Initialize MiniGUI etc file object, call before accessing MiniGUI etc value */
static inline BOOL InitMgEtc (void)
{
    if (hMgEtc)
        return TRUE;

    if ( !(hMgEtc = LoadEtcFile (ETCFILEPATH)) )
        return FALSE;
    return TRUE;
}

/* Terminate MiniGUI etc file object */
static inline void TerminateMgEtc (void)
{
    UnloadEtcFile (hMgEtc);
    hMgEtc = 0;
}

#else

extern ETC_S MGETC;

static inline void TerminateMisc (void) {}

static inline BOOL InitMgEtc (void)
{
    extern ETC_S MGETC;
    hMgEtc = (GHANDLE) &MGETC;
    return TRUE;
}

static inline BOOL InitMisc (void)
{
    return InitMgEtc();
}

static inline void TerminateMgEtc (void)
{
    //hMgEtc = 0;
}

#endif /* _INCORE_RES */

BOOL InitSystemRes (void);
void TerminateSysRes (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_MISC_H */

