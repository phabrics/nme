## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/bsd:

AUTOMAKE_OPTIONS = 1.4 gnu

tme_host_bsd_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/..

if HAVE_BSD
pkglib_LTLIBRARIES = tme_host_bsd.la
endif
tme_host_bsd_la_SOURCES = bsd-bpf.c
tme_host_bsd_la_LIBADD = ../libtme-eth.la $(TME_LIBS)

include $(top_srcdir)/modules.mk
