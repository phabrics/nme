## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/gtk:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL $(GTK_CFLAGS)

pkglib_LTLIBRARIES = tme_host_gtk.la
tme_host_gtk_la_SOURCES = gtk-mouse.c gtk-keyboard.c gtk-screen.c gtk-display.c gtk-display.h
tme_host_gtk_la_LDFLAGS = -module -version-info 0:0:0
tme_host_gtk_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

include $(top_srcdir)/modules
