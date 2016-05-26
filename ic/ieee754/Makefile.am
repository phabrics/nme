## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/ieee754/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL

tmeicincludedir = $(pkgincludedir)/ic
tmeicinclude_HEADERS = ieee754-auto.h ieee754-ops-auto.h

lib_LTLIBRARIES = libtme-ieee754.la
libtme_ieee754_la_SOURCES = ieee754-ops-auto.c ieee754-misc.c softfloat-tme.c
libtme_ieee754_la_LDFLAGS = -lm -version-info 0:0:0
libtme_ieee754_la_LIBADD = $(top_builddir)/generic/libtme-generic.la

AM_CFLAGS = @CFLAGS_NO_STRICT_ALIASING@

noinst_SCRIPTS = ieee754-misc-auto.sh ieee754-ops-auto.sh ieee754-precision.sh 
BUILT_SOURCES = $(noinst_SCRIPTS) ieee754-auto.h ieee754-ops-auto.h
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(noinst_SCRIPTS) $(noinst_SCRIPTS:.sh=.m4) softfloat-tme.h

# default rule for regenerating the portable shell scripts from m4 descriptions
$(noinst_SCRIPTS): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make ieee754-auto.h:
ieee754-auto.h: ieee754-misc-auto.sh ieee754-precision.sh
	$(SHELL) $< --header > $@

# target to make ieee754-misc-auto.c:
ieee754-misc-auto.c: ieee754-misc-auto.sh ieee754-precision.sh
	$(SHELL) $< > $@

# target to make ieee754-ops-auto.h:
ieee754-ops-auto.h: ieee754-ops-auto.sh ieee754-precision.sh
	$(SHELL) $< --header > $@

# target to make ieee754-ops-auto.c:
ieee754-ops-auto.c: ieee754-ops-auto.sh ieee754-precision.sh
	$(SHELL) $< > $@

# ieee754-misc.lo depends on ieee754-misc-auto.c:
ieee754-misc.lo: ieee754-misc-auto.c
