## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator serial/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

##SUBDIRS = @TME_SERIAL_SUBDIRS@
##DIST_SUBDIRS = 

pkglib_LTLIBRARIES = tme_serial_kb.la

tme_serial_kb_la_SOURCES = \
	serial-sink.c \
	serial-kb.h \
	serial-kb.c \
	kb-sun.c \
	serial-ms.h \
	serial-ms.c \
	ms-mssystems.c
tme_serial_kb_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_serial_kb_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la 

include $(top_srcdir)/modules
