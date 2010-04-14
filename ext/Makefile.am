SUBDIRS = control mywins skin image include res

lib_LTLIBRARIES = libmgext.la

COMMON_LADD =               \
    control/libcontrol.la   \
    mywins/libmywins.la     \
    skin/libskin.la         \
	image/libimage.la

libmgext_la_LIBADD = $(COMMON_LADD)
libmgext_la_SOURCES = template.c template.h
libmgext_la_LDFLAGS = -release $(LT_RELEASE) -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
