## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/gtk:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL $(GTK_CFLAGS)

pkglib_LTLIBRARIES = tme_host_gtk.la
tme_host_gtk_la_SOURCES = gtk-mouse.c gtk-keyboard.c gtk-screen.c gtk-display.c gtk-display.h
tme_host_gtk_la_LDFLAGS = -module -version-info 0:0:0
tme_host_gtk_la_LIBADD = $(top_builddir)/generic/libtme-generic.la \
	                 $(GTK_LIBS)
EXTRA_DIST = gtk-keymap-make.pl gtk-keymap-def.h

gtk-keyboard.c:  gtk-keymap.h

BUILT_SOURCES = gtk-keymap.h
CLEANFILES = gtk-keymap.h
if USING_X
gtk-keymap.h: gtk-keymap-make.pl
	  $(PERL) $< > $@
else
gtk-keymap.h: gtk-keymap-def.h
	  cp $< $@
endif

include $(top_srcdir)/modules
