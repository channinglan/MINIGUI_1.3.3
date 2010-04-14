/*
 ** $Id: mgetc.c,v 1.6 2003/11/22 13:40:44 weiym Exp $
 **
 ** mgetc.c: definitions for incore resource.
 **
 ** Copyright (C) 2003 Feynman Software.
 **
 ** Create date: 2003/09/22
 */

#include "common.h"

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


#ifdef _INCORE_RES

static char *SYSTEM_KEYS[] = {"gal_engine", "ial_engine", "mdev", "mtype"};

#ifdef __ECOS
static char *SYSTEM_VALUES[] = {"ecoslcd", "ipaq", "/dev/ts", "none"};
#else
static char *SYSTEM_VALUES[] = {"qvfb", "qvfb", "/dev/ts", "none"};
#endif

static char *FBCON_KEYS[] = {"defaultmode"};
static char *FBCON_VALUES[] = {"240x320-16bpp"};

static char *QVFB_KEYS[] = {"defaultmode", "display"};
static char *QVFB_VALUES[] = {"640x480-16bpp", "0"};

static char *SYSTEMFONT_KEYS[] = 
{"font_number", "font0", "font1", "font2", "default", "wchar_def", "fixed", "caption", "menu", "control"};

static char *SYSTEMFONT_VALUES[] = 
{
    "3","rbf-fixed-rrncnn-6-12-ISO8859-1", "*-fixed-rrncnn-*-12-GB2312", "*-SansSerif-rrncnn-*-12-GB2312", 
    "0", "1", "1", "1", "1", "1"
};

static char *CURSORINFO_KEYS[] = {"cursornumber"};
static char *CURSORINFO_VALUES[] = {"2"};

static char *ICONINFO_KEYS[] = {"iconnumber"};
static char *ICONINFO_VALUES[] = {"5"};

static char *BITMAPINFO_KEYS[] = {"bitmapnumber"};
static char *BITMAPINFO_VALUES[] = {"3"};

/*
static char *BGPICTURE_KEYS[] = {"position"};
static char *BGPICTURE_VALUES[] = {"center"};

static char *MOUSE_KEYS[] = {"dblclicktime"};
static char *MOUSE_VALUES[] = {"300"};

static char *EVENT_KEYS[] = {"timeoutusec", "repeatusec"};
static char *EVENT_VALUES[] = {"300000", "50000"};
*/

static ETCSECTION mgetc_sections [] =
{
  {4, "system",       SYSTEM_KEYS,     SYSTEM_VALUES},
  {1, "fbcon",        FBCON_KEYS,      FBCON_VALUES},
  {2, "qvfb",         QVFB_KEYS,       QVFB_VALUES},
  {10,"systemfont",   SYSTEMFONT_KEYS, SYSTEMFONT_VALUES},
  {1, "cursorinfo",   CURSORINFO_KEYS, CURSORINFO_VALUES},
  {1, "iconinfo",     ICONINFO_KEYS,   ICONINFO_VALUES},
  {1, "bitmapinfo",   BITMAPINFO_KEYS, BITMAPINFO_VALUES},
/* optional sections */
  /*
  {1, "bgpicture", BGPICTURE_KEYS, BGPICTURE_VALUES},
  {1, "mouse", MOUSE_KEYS, MOUSE_VALUES},
  {2, "event", EVENT_KEYS, EVENT_VALUES},
  */
};

ETC_S MGETC = { 7, mgetc_sections };

#endif /* _INCORE_RES */
