## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun3:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_machine_sun3.la
tme_machine_sun3_la_SOURCES =	\
	sun3-mainbus.c		\
	sun3-mmu.c		\
	sun3-control.c		\
	sun3-impl.h
tme_machine_sun3_la_LDFLAGS = -module -version-info 0:0:0
tme_machine_sun3_la_LIBADD = ../sun/libtme-machine-sun.la

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = SUN3-CARRERA sun3-carrera-eeprom.txt

include $(top_srcdir)/modules
