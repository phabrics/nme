## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/stp22xx/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_ic_stp22xx.la
tme_ic_stp22xx_la_SOURCES = \
  stp220x.c \
  stp222x-main.c \
  stp222x-asearch.c \
  stp222x-aspace.c \
  stp222x-mdu.c \
  stp222x-timer.c \
  stp222x-iommu.c \
  stp222x-stc.c \
  stp22xx.c
tme_ic_stp22xx_la_LIBADD = $(TME_LIBS)

EXTRA_DIST = stp222x-impl.h \
	stp22xx-impl.h

include $(top_srcdir)/modules
