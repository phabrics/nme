## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/posix:

AUTOMAKE_OPTIONS = 1.4 gnu

tme_host_posix_la_CPPFLAGS = $(AM_CPPFLAGS)

pkglib_LTLIBRARIES = tme_host_posix.la
tme_host_posix_la_SOURCES = posix-memory.c \
	posix-disk.c \
	posix-tape.c \
	posix-serial.c

tme_host_posix_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules.mk

SUBDIRS = display eth
