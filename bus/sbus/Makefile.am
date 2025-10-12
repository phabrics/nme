## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/sbus/:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES = \
	sun-fbs4.c
@PACKAGE@_@SUB_DIR@_la_LIBADD = $(top_builddir)/machine/sun/libtme-machine-sun.la $(TME_LIBS)

include $(top_srcdir)/modules.mk
