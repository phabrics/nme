## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/sdl:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_@SUB_DIR@_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(SDL_CFLAGS)

if HAVE_SDL
pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
endif
@PACKAGE@_@SUB_DIR@_la_SOURCES = sdl-display.c
@PACKAGE@_@SUB_DIR@_la_LIBADD = ../libtme-display.la $(SDL_LIBS) $(TME_LIBS)

@PACKAGE@_@SUB_DIR@_la_CPPFLAGS += $(GTK_CFLAGS)
@PACKAGE@_@SUB_DIR@_la_LIBADD += $(GTK_LIBS)

include $(top_srcdir)/modules.mk
