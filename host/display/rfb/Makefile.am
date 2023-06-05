## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/rfb:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(srcdir)/.. -I. -D_TME_IMPL $(RFB_CFLAGS)

pkglib_LTLIBRARIES = tme_host_rfb.la
tme_host_rfb_la_SOURCES = rfb-display.c
tme_host_rfb_la_LIBADD = ../libtme-display.la $(RFB_LIBS) $(TME_LIBS)

if HAVE_GLIB
AM_CPPFLAGS += $(GTK_CFLAGS)
tme_host_rfb_la_LIBADD += $(GTK_LIBS)
else
AM_CPPFLAGS += $(SDL_CFLAGS)
tme_host_rfb_la_LIBADD += $(SDL_LIBS)
endif

include $(top_srcdir)/modules
