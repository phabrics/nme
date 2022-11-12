## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/rfb:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(srcdir)/.. -I. -D_TME_IMPL $(RFB_CFLAGS) $(SDL_CFLAGS)

bin_PROGRAMS = SDLvncviewer
LDADD = ../libtme-display.la $(RFB_LIBS) $(SDL_LIBS)

pkglib_LTLIBRARIES = tme_host_sdl.la
tme_host_sdl_la_SOURCES = sdl-display.c
tme_host_sdl_la_LIBADD = ../libtme-display.la $(SDL_LIBS) $(TME_LIBS)

include $(top_srcdir)/modules
