## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/multibus/:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = tme_bus_multibus.la
tme_bus_multibus_la_SOURCES = 3c400.c \
	sun-mie.c \
	sun-sc.c
tme_bus_multibus_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules
