## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun4:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_machine_sun4.la
@PACKAGE@_machine_sun4_la_SOURCES =	\
	sun4-mainbus.c		\
	sun4-timer.c		\
	sun4-fdc.c		\
	sun44c-cache.c		\
	sun44c-mmu.c		\
	sun44c-control.c	\
	sun44c-memerr.c		\
	sun4-impl.h

@PACKAGE@_machine_sun4_la_LIBADD = ../sun/lib@PACKAGE@-machine-sun.la $(@PACKAGE@_LIBS)

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = SUN4-75.in

include $(top_srcdir)/modules.mk
