## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator generic/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

@PACKAGE@_@SUB_DIR@_la_CPPFLAGS = $(AM_CPPFLAGS) -DLIBTME_GENERIC_BUILD

pkglib_LTLIBRARIES = @PACKAGE@_@SUB_DIR@.la
@PACKAGE@_@SUB_DIR@_la_SOURCES = bus-el.c
@PACKAGE@_@SUB_DIR@_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules.mk
