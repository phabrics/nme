## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/bsd:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(srcdir)/../eth -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_host_bsd.la
tme_host_bsd_la_SOURCES = bsd-bpf.c
tme_host_bsd_la_LDFLAGS = -module -version-info 0:0:0
tme_host_bsd_la_LIBADD = ../eth/libeth.la
include $(top_srcdir)/modules
$(pkglib_LTLIBRARIES): ../eth/libeth.la
../eth/libeth.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;
