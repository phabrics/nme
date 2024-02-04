## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/openvpn:

AUTOMAKE_OPTIONS = 1.4 gnu

tme_host_openvpn_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/..

pkglib_LTLIBRARIES = tme_host_openvpn.la
tme_host_openvpn_la_SOURCES = \
	openvpn-link.c \
	openvpn-tap.c
tme_host_openvpn_la_LIBADD = ../libtme-eth.la $(TME_LIBS)

if WIN32
tme_host_openvpn_la_LIBADD += -lgdi32 -lws2_32 -lwininet -lcrypt32 -liphlpapi -lwinmm
endif

include $(top_srcdir)/modules
