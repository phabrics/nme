## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/bsd:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(srcdir)/.. -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_host_bsd.la
tme_host_bsd_la_SOURCES = bsd-bpf.c
tme_host_bsd_la_LIBADD = ../libtme-eth.la $(TME_LIBS)

include $(top_srcdir)/modules
