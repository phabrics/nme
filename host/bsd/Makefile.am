## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/bsd:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_host_bsd.la
tme_host_bsd_la_SOURCES = bsd-if.c bsd-bpf.c bsd-impl.h
tme_host_bsd_la_LDFLAGS = -module -version-info 0:0:0
tme_host_bsd_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

include $(top_srcdir)/modules
