## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/gtk:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_host_gtk_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(GTK_CFLAGS)

if HAVE_GTK
pkglib_LTLIBRARIES = @PACKAGE@_host_gtk.la
endif
@PACKAGE@_host_gtk_la_SOURCES = gtk-mouse.c gtk-keyboard.c gtk-display.c gtk-display.h
@PACKAGE@_host_gtk_la_LIBADD = ../lib@PACKAGE@-display.la $(GTK_LIBS) $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
