INCLUDES = -I../../include -I../include

noinst_LTLIBRARIES = libmain.la

EXTRA_DIST=main-lite.c main.c

if LITE_VERSION
libmain_la_SOURCES = main-lite.c
else
libmain_la_SOURCES = main.c
endif

