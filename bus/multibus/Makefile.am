## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator bus/multibus/:

AUTOMAKE_OPTIONS = 1.4 gnu

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES = 3c400.c \
	sun-mie.c \
	sun-sc.c
@PACKAGE@_@SUB_DIR@_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules.mk
