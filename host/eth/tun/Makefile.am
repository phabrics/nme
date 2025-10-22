## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/tun:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_host_tun_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. -DDEV_TAP_FILENAME=@DEV_TAP_FILENAME@ $(NATINC)

if HAVE_TUN
pkglib_LTLIBRARIES = @PACKAGE@_host_tun.la
endif
@PACKAGE@_host_tun_la_SOURCES = tun-tap.c
@PACKAGE@_host_tun_la_LIBADD = ../lib@PACKAGE@-eth.la $(NATLIBS) $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
