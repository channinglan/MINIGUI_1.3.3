INCLUDES = -I../../include -I../include

noinst_LTLIBRARIES = libkernel.la

EXTRA_DIST= blockheap.c cursor.c desktop.c event.c message.c timer-lite.c \
            cursor-lite.c      desktop-comm.c       fixstr.c  init.c \
            sharedres.c desktop-lite.c \
            init-lite.c timer.c timer_posix.c cursor-sa.c

if LITE_VERSION
if STAND_ALONE
libkernel_la_SOURCES = timer-lite.c init-lite.c desktop-lite.c \
                message.c blockheap.c fixstr.c event.c sharedres.c cursor-sa.c 
else
libkernel_la_SOURCES = timer-lite.c init-lite.c desktop-lite.c \
                message.c blockheap.c fixstr.c event.c sharedres.c cursor-lite.c 
endif
else
libkernel_la_SOURCES = timer.c init.c desktop.c cursor.c \
                message.c blockheap.c fixstr.c event.c 
endif


