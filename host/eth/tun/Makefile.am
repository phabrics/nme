## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/tun:

AUTOMAKE_OPTIONS = 1.4 gnu

tme_host_tun_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/.. -DDEV_TAP_FILENAME=@DEV_TAP_FILENAME@ $(NATINC)

if HAVE_TUN
pkglib_LTLIBRARIES = tme_host_tun.la
endif
tme_host_tun_la_SOURCES = tun-tap.c
tme_host_tun_la_LIBADD = ../libtme-eth.la $(NATLIBS) $(TME_LIBS)

include $(top_srcdir)/modules.mk
