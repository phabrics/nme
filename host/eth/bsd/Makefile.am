## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/bsd:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_host_bsd_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/..

if HAVE_BSD
pkglib_LTLIBRARIES = @PACKAGE@_host_bsd.la
endif
@PACKAGE@_host_bsd_la_SOURCES = bsd-bpf.c
@PACKAGE@_host_bsd_la_LIBADD = ../lib@PACKAGE@-eth.la $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
