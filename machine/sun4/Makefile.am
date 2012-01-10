## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun4:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_machine_sun4.la
tme_machine_sun4_la_SOURCES =	\
	sun4-mainbus.c		\
	sun4-timer.c		\
	sun4-fdc.c		\
	sun44c-cache.c		\
	sun44c-mmu.c		\
	sun44c-control.c	\
	sun44c-memerr.c		\
	sun4-impl.h
tme_machine_sun4_la_LDFLAGS = -module -version-info 0:0:0
tme_machine_sun4_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	../sun/libtme-machine-sun.la

examplesdir = $(datadir)/examples/tme
examples_DATA = SUN4-75

EXTRA_DIST = $(examples_DATA)

include $(top_srcdir)/modules
