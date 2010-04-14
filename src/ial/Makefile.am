INCLUDES = -I../../include -I../include

noinst_LTLIBRARIES = libial.la

if NATIVE_IAL_ENGINE
SUBDIRS=native
endif

DIST_SUBDIR=native
EXTRA_DIST=spec_general_ts_drv.html ggi_input.c libggi.c svgaggi_internal.h svgalib.h libggi.h svgalib.c 

COMMON_SRCS = ial.c

if SVGALIB_ENGINE
SVGALIB_SRCS = svgalib.c svgalib.h
else
SVGALIB_SRCS = 
endif

if LIBGGI_ENGINE
LIBGGI_SRCS = libggi.c libggi.h ggi_input.c svgaggi_internal.h
else
LIBGGI_SRCS =
endif

EP7211_SRCS = ep7211.c ep7211.h ep7211_internal.h
ADS_SRCS = ads.c ads.h ads_internal.h
IPAQ_SRCS = ipaq.c ipaq.h
MPC823_SRCS = mpc823.c mpc823.h
PX255B_SRCS = px255b.c px255b.h
VR4181_SRCS = vr4181.c vr4181.h
TPHELIO_SRCS = helio.c helio.h
TFSTB_SRCS = tf-stb.c tf-stb.h tf-code.h
DUMMY_SRCS = dummy.c dummy.h
QVFB_SRCS = qvfb.c qvfb.h
T800_SRCS = t800.c t800.h
MC68X328_SRCS = mc68x328.c mc68x328.h
SMDK2410_SRCS = 2410.c 2410.h

if NATIVE_IAL_ENGINE
libial_la_LIBADD = native/libnative.la
endif

libial_la_SOURCES = $(COMMON_SRCS) $(SVGALIB_SRCS) $(LIBGGI_SRCS) \
                    $(EP7211_SRCS) $(ADS_SRCS) $(TPHELIO_SRCS) $(IPAQ_SRCS) \
                    $(VR4181_SRCS) $(TFSTB_SRCS) $(DUMMY_SRCS) $(QVFB_SRCS) \
                    $(T800_SRCS) $(MPC823_SRCS) $(PX255B_SRCS) $(MC68X328_SRCS) $(SMDK2410_SRCS)

