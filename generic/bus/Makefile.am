## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator generic/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

@PACKAGE@_generic_bus_la_CPPFLAGS = $(AM_CPPFLAGS) -DLIB@PACKAGE@_GENERIC_BUILD

pkglib_LTLIBRARIES = @PACKAGE@_generic_bus.la
@PACKAGE@_generic_bus_la_SOURCES = bus-el.c
@PACKAGE@_generic_bus_la_LIBADD = $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
