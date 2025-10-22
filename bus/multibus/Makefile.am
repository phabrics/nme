## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/multibus/:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_bus_multibus.la
@PACKAGE@_bus_multibus_la_SOURCES = 3c400.c \
	sun-mie.c \
	sun-sc.c
@PACKAGE@_bus_multibus_la_LIBADD = $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
