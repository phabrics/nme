## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/rfb:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_@SUB_DIR@_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(RFB_CFLAGS)

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES = rfb-display.c
@PACKAGE@_@SUB_DIR@_la_LIBADD = ../libtme-display.la $(RFB_LIBS) $(TME_LIBS)

if HAVE_GTK
@PACKAGE@_@SUB_DIR@_la_CPPFLAGS += $(GTK_CFLAGS)
@PACKAGE@_@SUB_DIR@_la_LIBADD += $(GTK_LIBS)
else
@PACKAGE@_@SUB_DIR@_la_CPPFLAGS += $(SDL_CFLAGS)
@PACKAGE@_@SUB_DIR@_la_LIBADD += $(SDL_LIBS)
endif

include $(top_srcdir)/modules.mk
