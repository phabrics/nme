## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator serial/:

AUTOMAKE_OPTIONS = 1.4 gnu

##SUBDIRS = @@PACKAGE@_@SUB_DIR@_SUBDIRS@
##DIST_SUBDIRS = 

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@_kb.la

@PACKAGE@_@SUB_DIR@_kb_la_SOURCES = \
	serial-sink.c \
	serial-kb.h \
	serial-kb.c \
	kb-sun.c \
	serial-ms.h \
	serial-ms.c \
	ms-mssystems.c
@PACKAGE@_@SUB_DIR@_kb_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules.mk
