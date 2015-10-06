## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/tun:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(srcdir)/../eth -I. -D_TME_IMPL -DDEV_TAP_FILENAME=@DEV_TAP_FILENAME@ $(NATINC)
if USING_OPENVPN
AM_CPPFLAGS += -I$(srcdir)/../openvpn
endif

pkglib_LTLIBRARIES = tme_host_tun.la
tme_host_tun_la_SOURCES = tun-tap.c
tme_host_tun_la_LDFLAGS = -module -version-info 0:0:0
tme_host_tun_la_LIBADD = ../eth/libtme-host-eth.la $(NATLIBS)
if USING_OPENVPN
tme_host_tun_la_LIBADD += ../openvpn/libtme-host-openvpn.la
endif

include $(top_srcdir)/modules
$(pkglib_LTLIBRARIES): ../eth/libtme-host-eth.la
../eth/libtme-host-eth.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;
