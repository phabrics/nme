## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/posix:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_host_posix.la
tme_host_posix_la_SOURCES = posix-memory.c \
	posix-disk.c \
	posix-tape.c \
	posix-serial.c
tme_host_posix_la_LDFLAGS = -module -version-info 0:0:0
tme_host_posix_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

include $(top_srcdir)/modules
