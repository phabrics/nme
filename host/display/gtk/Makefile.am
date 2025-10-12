## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/gtk:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_@SUB_DIR@_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. $(GTK_CFLAGS)

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES = gtk-mouse.c gtk-keyboard.c gtk-display.c gtk-display.h
@PACKAGE@_@SUB_DIR@_la_LIBADD = ../libtme-display.la $(GTK_LIBS) $(TME_LIBS)

include $(top_srcdir)/modules.mk
