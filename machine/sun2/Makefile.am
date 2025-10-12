## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun2:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES =	\
	sun2-mainbus.c		\
	sun2-mmu.c		\
	sun2-control.c		\
	sun2-impl.h

@PACKAGE@_@SUB_DIR@_la_LIBADD = ../sun/libtme-machine-sun.la $(TME_LIBS)

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = SUN2-MULTIBUS

include $(top_srcdir)/modules.mk
