## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator display/:

AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL
@PACKAGE@_LIBS = $(top_builddir)/generic/lib@PACKAGE@-generic.la $(top_builddir)/libtme/lib@PACKAGE@.la

lib_LTLIBRARIES = lib@PACKAGE@-display.la
lib@PACKAGE@_display_la_SOURCES = display-mouse.c display-keyboard.c display.c display.h keymap.h
lib@PACKAGE@_display_la_LDFLAGS = -version-info 0:0:0
lib@PACKAGE@_display_la_LIBADD = $(@PACKAGE@_LIBS)

EXTRA_DIST = keymap-make.pl keymap-def.h

BUILT_SOURCES = keymap.h
CLEANFILES = keymap.h

keymap.h: keymap-make.pl keymap-def.h
	  $(PERL) $(srcdir)/keymap-make.pl > $@ || $(CP) $(srcdir)/keymap-def.h $@

SUBDIRS=. gtk rfb sdl
