## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun2:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_machine_sun2.la
@PACKAGE@_machine_sun2_la_SOURCES =	\
	sun2-mainbus.c		\
	sun2-mmu.c		\
	sun2-control.c		\
	sun2-impl.h

@PACKAGE@_machine_sun2_la_LIBADD = ../sun/lib@PACKAGE@-machine-sun.la $(@PACKAGE@_LIBS)

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = SUN2-MULTIBUS.in

include $(top_srcdir)/modules.mk
