## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/gtk:

AUTOMAKE_OPTIONS = 1.4 gnu

tme_host_gtk_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(GTK_CFLAGS)

pkglib_LTLIBRARIES = tme_host_gtk.la
tme_host_gtk_la_SOURCES = gtk-mouse.c gtk-keyboard.c gtk-display.c gtk-display.h
tme_host_gtk_la_LIBADD = ../libtme-display.la $(GTK_LIBS) $(TME_LIBS)

include $(top_srcdir)/modules
