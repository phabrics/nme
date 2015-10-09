## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/tun:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(srcdir)/../eth -I$(top_srcdir)/lib -I. -D_TME_IMPL -DDEV_TAP_FILENAME=@DEV_TAP_FILENAME@ $(NATINC)
if USING_OPENVPN
AM_CPPFLAGS += -I$(srcdir)/../openvpn
endif

pkglib_LTLIBRARIES = tme_host_tun.la
tme_host_tun_la_SOURCES = tun-tap.c
tme_host_tun_la_LDFLAGS = -module -version-info 0:0:0
tme_host_tun_la_LIBADD = $(NATLIBS)
if USING_OPENVPN
tme_host_tun_la_LIBADD += ../openvpn/libopenvpn.la
else
tme_host_tun_la_LIBADD += ../eth/libeth.la
endif

include $(top_srcdir)/modules

if USING_OPENVPN
$(pkglib_LTLIBRARIES): ../openvpn/libopenvpn.la
../openvpn/libopenvpn.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
else
$(pkglib_LTLIBRARIES): ../eth/libeth.la
../eth/libeth.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
endif
force: ;
