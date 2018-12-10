## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/display:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL $(GTK_CFLAGS)

noinst_LTLIBRARIES = libdisplay.la
libdisplay_la_SOURCES = display-mouse.c display-keyboard.c display-thread.c display.c display.h
libdisplay_la_LIBADD = 	$(top_builddir)/generic/libtme-generic.la

EXTRA_DIST = keymap-make.pl keymap-def.h

display-keyboard.c:  keymap.h

BUILT_SOURCES = keymap.h
CLEANFILES = keymap.h
if USING_X
keymap.h: keymap-make.pl
	  $(PERL) $< > $@
else
keymap.h: keymap-def.h
	  cp $< $@
endif

include $(top_srcdir)/modules
