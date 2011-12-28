## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/ieee754/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

tmeicincludedir = $(pkgincludedir)/ic
tmeicinclude_HEADERS = ieee754-auto.h ieee754-ops-auto.h
lib_LTLIBRARIES = libtme-ieee754.la
libtme_ieee754_la_SOURCES = ieee754-ops-auto.c ieee754-misc.c softfloat-tme.c
libtme_ieee754_la_LDFLAGS = -lm -module -version-info 0:0:0
BUILT_SOURCES = ieee754-auto.h ieee754-ops-auto.h ieee754-misc-auto.c ieee754-ops-auto.c
EXTRA_DIST = ieee754-misc-auto.sh ieee754-ops-auto.sh ieee754-precision.sh softfloat-tme.h
AM_CFLAGS = @CFLAGS_NO_STRICT_ALIASING@

# target to make ieee754-auto.h:
ieee754-auto.h: ieee754-misc-auto.sh
	$(SHELL) $(srcdir)/ieee754-misc-auto.sh --header > $@

# target to make ieee754-misc-auto.c:
ieee754-misc-auto.c: ieee754-misc-auto.sh
	$(SHELL) $(srcdir)/ieee754-misc-auto.sh > $@

# target to make ieee754-ops-auto.h:
ieee754-ops-auto.h: ieee754-ops-auto.sh
	$(SHELL) $(srcdir)/ieee754-ops-auto.sh --header > $@

# target to make ieee754-ops-auto.c:
ieee754-ops-auto.c: ieee754-ops-auto.sh
	$(SHELL) $(srcdir)/ieee754-ops-auto.sh > $@

# ieee754-misc.lo depends on ieee754-misc-auto.c:
ieee754-misc.lo: ieee754-misc-auto.c
