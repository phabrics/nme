## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/ieee754/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

@PACKAGE@icincludedir = $(pkgincludedir)/ic
@PACKAGE@icinclude_HEADERS = ieee754-auto.h ieee754-ops-auto.h

lib_LTLIBRARIES = lib@PACKAGE@-ieee754.la
lib@PACKAGE@_ieee754_la_SOURCES = ieee754-ops-auto.c ieee754-misc.c softfloat-tme.c
lib@PACKAGE@_ieee754_la_LDFLAGS = -lm -version-info 0:0:0
lib@PACKAGE@_ieee754_la_LIBADD = $(@PACKAGE@_LIBS)

AM_CFLAGS = @CFLAGS_NO_STRICT_ALIASING@

noinst_SCRIPTS = ieee754-misc-auto.sh ieee754-ops-auto.sh ieee754-precision.sh 
BUILT_SOURCES = ieee754-auto.h ieee754-ops-auto.h ieee754-misc-auto.c
MAINTAINERCLEANFILES = $(noinst_SCRIPTS) $(BUILT_SOURCES) ieee754-ops-auto.c

EXTRA_DIST = $(MAINTAINERCLEANFILES) $(noinst_SCRIPTS:.sh=.m4) softfloat-tme.h

# default rule for regenerating the portable shell scripts from m4 descriptions
$(addprefix $(srcdir)/,$(noinst_SCRIPTS)): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make ieee754-auto.h:
$(srcdir)/ieee754-auto.h: ieee754-misc-auto.sh ieee754-precision.sh
	$(SHELL) $< --header > $@

# target to make ieee754-misc-auto.c:
$(srcdir)/ieee754-misc-auto.c: ieee754-misc-auto.sh ieee754-precision.sh
	$(SHELL) $< > $@

# target to make ieee754-ops-auto.h:
$(srcdir)/ieee754-ops-auto.h: ieee754-ops-auto.sh ieee754-precision.sh
	$(SHELL) $< --header > $@

# target to make ieee754-ops-auto.c:
$(srcdir)/ieee754-ops-auto.c: ieee754-ops-auto.sh ieee754-precision.sh
	$(SHELL) $< > $@

include $(top_srcdir)/modules.mk
