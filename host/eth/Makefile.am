## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/eth:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

lib_LTLIBRARIES = libtme-host-eth.la
libtme_host_eth_la_SOURCES = eth-if.c eth-impl.h
libtme_host_eth_la_LDFLAGS = -version-info 0:0:0
libtme_host_eth_la_LIBADD = $(top_builddir)/libtme/libtme.la \
		$(top_builddir)/generic/libtme-generic.la

include $(top_srcdir)/modules
