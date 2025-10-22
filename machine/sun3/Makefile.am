## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun3:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_machine_sun3.la
@PACKAGE@_machine_sun3_la_SOURCES =	\
	sun3-mainbus.c		\
	sun3-mmu.c		\
	sun3-control.c		\
	sun3-impl.h

@PACKAGE@_machine_sun3_la_LIBADD = ../sun/lib@PACKAGE@-machine-sun.la $(@PACKAGE@_LIBS)

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = SUN3-CARRERA sun3-carrera-eeprom.txt

include $(top_srcdir)/modules.mk
