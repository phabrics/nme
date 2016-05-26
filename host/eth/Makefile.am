## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/eth:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL

noinst_LTLIBRARIES = libeth.la
libeth_la_SOURCES = eth-impl.c eth-if.h
libeth_la_LIBADD = $(top_builddir)/generic/libtme-generic.la

include $(top_srcdir)/modules
