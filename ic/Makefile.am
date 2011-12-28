## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

SUBDIRS = @TME_IC_SUBDIRS@
DIST_SUBDIRS = m68k ieee754 sparc stp22xx

pkglib_LTLIBRARIES = @TME_ICS@
EXTRA_LTLIBRARIES = tme_ic_am9513.la \
	tme_ic_z8530.la \
	tme_ic_i825x6.la \
	tme_ic_isil7170.la \
	tme_ic_ncr5380.la \
	tme_ic_ncr53c9x.la \
	tme_ic_ncr89c105.la \
	tme_ic_stp2024.la \
	tme_ic_ad184x.la \
	tme_ic_mk48txx.la \
	tme_ic_nec765.la \
	tme_ic_am7930.la \
	tme_ic_am7990.la \
	tme_ic_lsi64854.la \
	tme_ic_mm58167.la

tme_ic_am9513_la_SOURCES = am9513.c
tme_ic_am9513_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_am9513_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la 

tme_ic_z8530_la_SOURCES = z8530.c z8530reg.h
tme_ic_z8530_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_z8530_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_mm58167_la_SOURCES = mm58167.c
tme_ic_mm58167_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_mm58167_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_i825x6_la_SOURCES = i825x6.c i825x6reg.h
tme_ic_i825x6_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_i825x6_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_isil7170_la_SOURCES = isil7170.c
tme_ic_isil7170_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_isil7170_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_ncr5380_la_SOURCES = ncr5380.c
tme_ic_ncr5380_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_ncr5380_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_ncr53c9x_la_SOURCES = ncr53c9x.c
tme_ic_ncr53c9x_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_ncr53c9x_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_ncr89c105_la_SOURCES = ncr89c105.c
tme_ic_ncr89c105_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_ncr89c105_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_stp2024_la_SOURCES = stp2024.c
tme_ic_stp2024_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_stp2024_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_ad184x_la_SOURCES = ad184x.c
tme_ic_ad184x_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_ad184x_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_mk48txx_la_SOURCES = mk48txx.c
tme_ic_mk48txx_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_mk48txx_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_nec765_la_SOURCES = nec765.c
tme_ic_nec765_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_nec765_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_am7930_la_SOURCES = am7930.c
tme_ic_am7930_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_am7930_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_am7990_la_SOURCES = am7990.c
tme_ic_am7990_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_am7990_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

tme_ic_lsi64854_la_SOURCES = lsi64854.c
tme_ic_lsi64854_la_LDFLAGS = -rpath $(pkglibdir) -module -version-info 0:0:0
tme_ic_lsi64854_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la

include $(top_srcdir)/modules
