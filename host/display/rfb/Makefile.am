## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/rfb:

AUTOMAKE_OPTIONS = 1.4 gnu

tme_host_rfb_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(RFB_CFLAGS)

pkglib_LTLIBRARIES = tme_host_rfb.la
tme_host_rfb_la_SOURCES = rfb-display.c
tme_host_rfb_la_LIBADD = ../libtme-display.la $(RFB_LIBS) $(TME_LIBS)

if HAVE_GTK
tme_host_rfb_la_CPPFLAGS += $(GTK_CFLAGS)
tme_host_rfb_la_LIBADD += $(GTK_LIBS)
else
tme_host_rfb_la_CPPFLAGS += $(SDL_CFLAGS)
tme_host_rfb_la_LIBADD += $(SDL_LIBS)
endif

include $(top_srcdir)/modules.mk
