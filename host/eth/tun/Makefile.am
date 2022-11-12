## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/tun:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(srcdir)/.. -I$(top_srcdir)/lib -I. -D_TME_IMPL -DDEV_TAP_FILENAME=@DEV_TAP_FILENAME@ $(NATINC)

pkglib_LTLIBRARIES = tme_host_tun.la
tme_host_tun_la_SOURCES = tun-tap.c
tme_host_tun_la_LIBADD = ../libtme-eth.la $(NATLIBS) $(TME_LIBS)

include $(top_srcdir)/modules
