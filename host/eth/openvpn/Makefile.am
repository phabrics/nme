## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/openvpn:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(srcdir)/.. -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_host_openvpn.la
tme_host_openvpn_la_SOURCES = \
	openvpn-link.c \
	openvpn-tap.c
tme_host_openvpn_la_LIBADD = ../libtme-eth.la
tme_host_openvpn_la_LDFLAGS = -module -version-info 0:0:0

include $(top_srcdir)/modules
