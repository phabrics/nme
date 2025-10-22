## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator serial/:

AUTOMAKE_OPTIONS = 1.4 gnu

##SUBDIRS = @@PACKAGE@_serial_SUBDIRS@
##DIST_SUBDIRS = 

pkglib_LTLIBRARIES = @PACKAGE@_serial_kb.la

@PACKAGE@_serial_kb_la_SOURCES = \
	serial-sink.c \
	serial-kb.h \
	serial-kb.c \
	kb-sun.c \
	serial-ms.h \
	serial-ms.c \
	ms-mssystems.c
@PACKAGE@_serial_kb_la_LIBADD = $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
