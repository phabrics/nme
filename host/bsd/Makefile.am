## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/bsd:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(srcdir)/../eth -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_host_bsd.la
tme_host_bsd_la_SOURCES = bsd-bpf.c
tme_host_bsd_la_LDFLAGS = -module -version-info 0:0:0
tme_host_bsd_la_LIBADD = ../eth/libtme-host-eth.la
include $(top_srcdir)/modules
$(pkglib_LTLIBRARIES): ../eth/libtme-host-eth.la
../eth/libtme-host-eth.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;
