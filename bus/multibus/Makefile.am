## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/multibus/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_bus_multibus.la
tme_bus_multibus_la_SOURCES = 3c400.c \
	sun-mie.c \
	sun-sc.c
tme_bus_multibus_la_LDFLAGS = -module -version-info 0:0:0
tme_bus_multibus_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

include $(top_srcdir)/modules
