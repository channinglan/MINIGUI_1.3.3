INCLUDES = -I../../include -I../include

noinst_LTLIBRARIES = libmisc.la

EXTRA_DIST=about.c drawsemop.c error.c errorlog.c misc.c sockio.c listenfd.c systext.c \
           rwops.c endianrw.c math.c math3d.c

if MISC_ABOUTDLG
ABOUT_SRCS=about.c
else
ABOUT_SRCS=
endif

if LITE_VERSION
if STAND_ALONE
LITE_SRCS=listenfd.c
else
LITE_SRCS=error.c errorlog.c sockio.c drawsemop.c listenfd.c
endif
else
LITE_SRCS=
endif

if USE_NEWGAL
NEWGAL_SRCS=math.c math3d.c
else
NEWGAL_SRCS=
endif

libmisc_la_SOURCES = misc.c systext.c rwops.c endianrw.c nposix.c $(NEWGAL_SRCS) $(LITE_SRCS) $(ABOUT_SRCS)


