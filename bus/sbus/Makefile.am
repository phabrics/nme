## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/sbus/:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = tme_bus_sbus.la
tme_bus_sbus_la_SOURCES = \
	sun-fbs4.c
tme_bus_sbus_la_LIBADD = $(top_builddir)/machine/sun/libtme-machine-sun.la $(TME_LIBS)

include $(top_srcdir)/modules
