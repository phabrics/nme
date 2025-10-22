## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/openvpn:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_host_openvpn_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/..

if HAVE_OPENVPN
pkglib_LTLIBRARIES = @PACKAGE@_host_openvpn.la
endif
@PACKAGE@_host_openvpn_la_SOURCES = \
	openvpn-link.c \
	openvpn-tap.c
@PACKAGE@_host_openvpn_la_LIBADD = ../lib@PACKAGE@-eth.la $(@PACKAGE@_LIBS)

if WIN32
@PACKAGE@_host_openvpn_la_LIBADD += -lgdi32 -lws2_32 -lwininet -lcrypt32 -liphlpapi -lwinmm
endif

include $(top_srcdir)/modules.mk
