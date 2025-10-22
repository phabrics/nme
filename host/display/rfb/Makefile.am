## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/rfb:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_host_rfb_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(RFB_CFLAGS)

if HAVE_RFB
pkglib_LTLIBRARIES = @PACKAGE@_host_rfb.la
endif
@PACKAGE@_host_rfb_la_SOURCES = rfb-display.c
@PACKAGE@_host_rfb_la_LIBADD = ../lib@PACKAGE@-display.la $(RFB_LIBS) $(@PACKAGE@_LIBS)

if HAVE_GTK
@PACKAGE@_host_rfb_la_CPPFLAGS += $(GTK_CFLAGS)
@PACKAGE@_host_rfb_la_LIBADD += $(GTK_LIBS)
else
@PACKAGE@_host_rfb_la_CPPFLAGS += $(SDL_CFLAGS)
@PACKAGE@_host_rfb_la_LIBADD += $(SDL_LIBS)
endif

include $(top_srcdir)/modules.mk
