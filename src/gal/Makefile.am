INCLUDES = -I../../include -I../include 

if NATIVE_GAL_ENGINE
SUBDIRS = native
endif

EXTRA_DIST=gal.c libggi.c libggi.h svgalib.c svgalib.h vga16.c vga16.h

DIST_SUBDIR = native

noinst_LTLIBRARIES = libgal.la

COMMON_SRCS = gal.c vga16.c vga16.h

if SVGALIB_ENGINE
SVGALIB_SRCS = svgalib.c svgalib.h
else
SVGALIB_SRCS = 
endif

if LIBGGI_ENGINE
LIBGGI_SRCS = libggi.c libggi.h
else
LIBGGI_SRCS =
endif

if NATIVE_GAL_ENGINE
libgal_la_LIBADD = native/libnative.la
endif

libgal_la_SOURCES = $(COMMON_SRCS) $(SVGALIB_SRCS) $(LIBGGI_SRCS)

