## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/rfb:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(srcdir)/.. -I. -D_TME_IMPL $(RFB_CFLAGS) $(SDL_CFLAGS)

bin_PROGRAMS = SDLvncviewer
LDADD = ../libtme-display.la $(RFB_LIBS) $(SDL_LIBS)

include $(top_srcdir)/modules
