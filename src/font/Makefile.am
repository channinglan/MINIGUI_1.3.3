
SUBDIRS = in-core

noinst_LTLIBRARIES = libfont.la

COMMON_SRCS = charset.c rawbitmap.c sysfont.c charset.h rawbitmap.h \
              devfont.c fontname.c

EXTRA_DIST = charset.c  convgbmap.c  fontname.c  freetype.h  jisunimap.c  rawbitmap.h  type1.c  varbitmap.c \
             charset.h  devfont.c    freetype.c  gbunimap.c  gbkunimap.c  gb18030unimap.c       rawbitmap.c  \
             sysfont.c  type1.h      varbitmap.h qpf.h       qpf.c

if VBF_SUPPORT
VBF_SRCS = varbitmap.c varbitmap.h
else
VBF_SRCS = 
endif

if HAS_FREETYPE2
INCLUDES = `freetype-config --cflags` -I../../include -I../include
else 
INCLUDES = -I../../include -I../include
endif

OTHER_SRCS= freetype.c freetype.h type1.c type1.h freetype2.c freetype2.h qpf.h qpf.c \
            gbunimap.c gbkunimap.c gb18030unimap.c big5unimap.c ujisunimap.c sjisunimap.c euckrunimap.c

libfont_la_LIBADD = in-core/libincore.la
libfont_la_SOURCES = $(COMMON_SRCS) $(VBF_SRCS) $(OTHER_SRCS)

