## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/bsd:

AUTOMAKE_OPTIONS = 1.4 gnu

@PACKAGE@_@SUB_DIR@_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/..

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES = bsd-bpf.c
@PACKAGE@_@SUB_DIR@_la_LIBADD = ../libtme-eth.la $(TME_LIBS)

include $(top_srcdir)/modules.mk
