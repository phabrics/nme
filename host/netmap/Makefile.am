## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/netmap:

@PACKAGE@_host_netmap_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/..

if HAVE_NETMAP
pkglib_LTLIBRARIES = @PACKAGE@_host_netmap.la
endif
@PACKAGE@_host_netmap_la_SOURCES = netmap-vale.c
@PACKAGE@_host_netmap_la_LIBADD = ../lib@PACKAGE@-eth.la $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
