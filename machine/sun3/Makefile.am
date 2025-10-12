## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun3:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES =	\
	sun3-mainbus.c		\
	sun3-mmu.c		\
	sun3-control.c		\
	sun3-impl.h

@PACKAGE@_@SUB_DIR@_la_LIBADD = ../sun/libtme-machine-sun.la $(TME_LIBS)

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = SUN3-CARRERA sun3-carrera-eeprom.txt

include $(top_srcdir)/modules.mk
