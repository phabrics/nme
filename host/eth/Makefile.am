## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator eth/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL
@PACKAGE@_LIBS = $(top_builddir)/generic/lib@PACKAGE@-generic.la $(top_builddir)/libtme/lib@PACKAGE@.la

lib_LTLIBRARIES = lib@PACKAGE@-eth.la
lib@PACKAGE@_eth_la_SOURCES = eth-impl.c eth-if.h
lib@PACKAGE@_eth_la_LDFLAGS = -version-info 0:0:0
lib@PACKAGE@_eth_la_LIBADD = $(@PACKAGE@_LIBS)

SUBDIRS=. bsd openvpn tun
