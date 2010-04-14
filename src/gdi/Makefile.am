INCLUDES = -I../../include -I../include

noinst_LTLIBRARIES = libgdi.la

EXTRA_DIST = attr.c clip-lite.c draw-lite.c drawtext.h map.c rect.c \
             text-lite.c bitmap-lite.c clip.c draw.c gdi-lite.c icon-lite.c \
             screen-lite.c text.c bitmap.c coor.c drawtext.c gdi.c icon.c logfont.c \
             readbmp.c screen.c bitmap-comm.c palette.c cliprect.c loadicon.c

if LITE_VERSION
COMMON_SRCS = attr.c coor.c bitmap-lite.c clip-lite.c draw-lite.c drawtext.c   \
    drawtext.h gdi-lite.c icon-lite.c map.c rect.c screen-lite.c text-lite.c        \
    logfont.c readbmp.c palette.c cliprect.c
else
COMMON_SRCS = attr.c bitmap.c clip.c coor.c draw.c drawtext.c   \
    drawtext.h gdi.c icon.c map.c rect.c screen.c text.c        \
    logfont.c readbmp.c palette.c cliprect.c
endif

libgdi_la_SOURCES = $(COMMON_SRCS)

