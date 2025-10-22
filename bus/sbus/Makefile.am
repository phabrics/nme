## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/sbus/:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_bus_sbus.la
@PACKAGE@_bus_sbus_la_SOURCES = \
	sun-fbs4.c
@PACKAGE@_bus_sbus_la_LIBADD = $(top_builddir)/machine/sun/lib@PACKAGE@-machine-sun.la $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
