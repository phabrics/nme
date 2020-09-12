## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/rfb:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(srcdir)/.. -I. -D_TME_IMPL $(RFB_CFLAGS)

pkglib_LTLIBRARIES = tme_host_rfb.la
tme_host_rfb_la_SOURCES = rfb-display.c
tme_host_rfb_la_LDFLAGS = -module -version-info 0:0:0
tme_host_rfb_la_LIBADD = ../libtme-host.la $(RFB_LIBS)

include $(top_srcdir)/modules
$(pkglib_LTLIBRARIES): ../libtme-host.la
../libtme-host.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;
