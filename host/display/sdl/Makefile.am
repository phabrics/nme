## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/sdl:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_host_sdl_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(SDL_CFLAGS)

if HAVE_SDL
pkglib_LTLIBRARIES = @PACKAGE@_host_sdl.la
endif
@PACKAGE@_host_sdl_la_SOURCES = sdl-display.c
@PACKAGE@_host_sdl_la_LIBADD = ../lib@PACKAGE@-display.la $(SDL_LIBS) $(@PACKAGE@_LIBS)

@PACKAGE@_host_sdl_la_CPPFLAGS += $(GTK_CFLAGS)
@PACKAGE@_host_sdl_la_LIBADD += $(GTK_LIBS)

include $(top_srcdir)/modules.mk
