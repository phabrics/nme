## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun4:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES =	\
	sun4-mainbus.c		\
	sun4-timer.c		\
	sun4-fdc.c		\
	sun44c-cache.c		\
	sun44c-mmu.c		\
	sun44c-control.c	\
	sun44c-memerr.c		\
	sun4-impl.h

@PACKAGE@_@SUB_DIR@_la_LIBADD = ../sun/libtme-machine-sun.la $(TME_LIBS)

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = SUN4-75

include $(top_srcdir)/modules.mk
