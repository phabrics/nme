## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/stp22xx/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

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
tme_ic_stp22xx_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_stp22xx_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la
EXTRA_DIST = stp222x-impl.h \
	stp22xx-impl.h

include $(top_srcdir)/modules
