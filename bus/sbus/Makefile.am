## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/sbus/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_bus_sbus.la
tme_bus_sbus_la_SOURCES = \
	sun-fbs4.c
tme_bus_sbus_la_LDFLAGS = -module -version-info 0:0:0
tme_bus_sbus_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/machine/sun/libtme-machine-sun.la

include $(top_srcdir)/modules
