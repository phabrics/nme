## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator generic/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

tme_generic_bus_la_CPPFLAGS = $(AM_CPPFLAGS) -DLIBTME_GENERIC_BUILD

pkglib_LTLIBRARIES = tme_generic_bus.la
tme_generic_bus_la_SOURCES = bus-el.c
tme_generic_bus_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules.mk
