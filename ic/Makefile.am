## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/:

AUTOMAKE_OPTIONS = 1.4 gnu

SUBDIRS = ieee754 m68k sparc stp22xx

pkglib_LTLIBRARIES = @PACKAGE@_ic_am9513.la \
	@PACKAGE@_ic_z8530.la \
	@PACKAGE@_ic_i825x6.la \
	@PACKAGE@_ic_isil7170.la \
	@PACKAGE@_ic_ncr5380.la \
	@PACKAGE@_ic_ncr53c9x.la \
	@PACKAGE@_ic_ncr89c105.la \
	@PACKAGE@_ic_stp2024.la \
	@PACKAGE@_ic_ad184x.la \
	@PACKAGE@_ic_mk48txx.la \
	@PACKAGE@_ic_nec765.la \
	@PACKAGE@_ic_am7930.la \
	@PACKAGE@_ic_am7990.la \
	@PACKAGE@_ic_lsi64854.la \
	@PACKAGE@_ic_mm58167.la

@PACKAGE@_ic_am9513_la_SOURCES = am9513.c
@PACKAGE@_ic_am9513_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_z8530_la_SOURCES = z8530.c z8530reg.h
@PACKAGE@_ic_z8530_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_mm58167_la_SOURCES = mm58167.c
@PACKAGE@_ic_mm58167_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_i825x6_la_SOURCES = i825x6.c i825x6reg.h
@PACKAGE@_ic_i825x6_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_isil7170_la_SOURCES = isil7170.c
@PACKAGE@_ic_isil7170_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_ncr5380_la_SOURCES = ncr5380.c
@PACKAGE@_ic_ncr5380_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_ncr53c9x_la_SOURCES = ncr53c9x.c
@PACKAGE@_ic_ncr53c9x_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_ncr89c105_la_SOURCES = ncr89c105.c
@PACKAGE@_ic_ncr89c105_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_stp2024_la_SOURCES = stp2024.c
@PACKAGE@_ic_stp2024_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_ad184x_la_SOURCES = ad184x.c
@PACKAGE@_ic_ad184x_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_mk48txx_la_SOURCES = mk48txx.c
@PACKAGE@_ic_mk48txx_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_nec765_la_SOURCES = nec765.c
@PACKAGE@_ic_nec765_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_am7930_la_SOURCES = am7930.c
@PACKAGE@_ic_am7930_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_am7990_la_SOURCES = am7990.c
@PACKAGE@_ic_am7990_la_LIBADD = $(@PACKAGE@_LIBS)

@PACKAGE@_ic_lsi64854_la_SOURCES = lsi64854.c
@PACKAGE@_ic_lsi64854_la_LIBADD = $(@PACKAGE@_LIBS)

include $(top_srcdir)/modules.mk
