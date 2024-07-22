## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/sdl:

AUTOMAKE_OPTIONS = 1.4 gnu

tme_host_sdl_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(SDL_CFLAGS)

pkglib_LTLIBRARIES = tme_host_sdl.la
tme_host_sdl_la_SOURCES = sdl-display.c
tme_host_sdl_la_LIBADD = ../libtme-display.la $(SDL_LIBS) $(TME_LIBS)

if HAVE_GTK
tme_host_sdl_la_CPPFLAGS += $(GTK_CFLAGS)
tme_host_sdl_la_LIBADD += $(GTK_LIBS)
endif

include $(top_srcdir)/modules.mk
