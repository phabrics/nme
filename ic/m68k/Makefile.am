## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/m68k/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL 

pkglib_LTLIBRARIES = tme_ic_m68k.la
tme_ic_m68k_la_SOURCES = m68k-opmap.c \
	m68k-insns.c \
	m68k-misc.c \
	m68010.c \
	m68020.c \
	m6888x.c
tme_ic_m68k_la_LDFLAGS = -module -version-info 0:0:0
tme_ic_m68k_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la \
	$(top_builddir)/ic/ieee754/libtme-ieee754.la
BUILT_SOURCES = m68k-auto.h m68k-opmap.c m68k-insns-auto.c m68k-bus-auto.c m6888x-auto.c
EXTRA_DIST = m68k-impl.h \
	m68k-misc-auto.sh \
	m68k-insns-auto.sh \
	m68k-bus-auto.sh \
	m6888x-auto.sh \
	m68k-execute.c \
	m68k-iset-expand.pl \
	m68k-iset.txt \
	m68k-opmap-make.pl \
	$(BUILT_SOURCES)

# target to make m68k-auto.h:
m68k-auto.h: m68k-misc-auto.sh m68k-insns-auto.sh
	$(SHELL) $(srcdir)/m68k-misc-auto.sh --header > $@
	$(SHELL) $(srcdir)/m68k-insns-auto.sh --header >> $@

# target to make m68k-opmap.c:
m68k-opmap.c: m68k-iset.txt m68k-iset-expand.pl m68k-opmap-make.pl
	( $(PERL) $(srcdir)/m68k-iset-expand.pl m68000 < $(srcdir)/m68k-iset.txt ; \
	  $(PERL) $(srcdir)/m68k-iset-expand.pl m68010 < $(srcdir)/m68k-iset.txt ; \
	  $(PERL) $(srcdir)/m68k-iset-expand.pl m68020 < $(srcdir)/m68k-iset.txt ) | \
	  $(PERL) $(srcdir)/m68k-opmap-make.pl > $@

# target to make m68k-insns-auto.c:
m68k-insns-auto.c: m68k-insns-auto.sh m68k-misc-auto.sh
	$(SHELL) $(srcdir)/m68k-insns-auto.sh > $@
	$(SHELL) $(srcdir)/m68k-misc-auto.sh >> $@

# target to make m68k-bus-auto.c:
m68k-bus-auto.c: m68k-bus-auto.sh
	$(SHELL) $(srcdir)/m68k-bus-auto.sh > $@

# target to make m6888x-auto.c:
m6888x-auto.c: m6888x-auto.sh
	$(SHELL) $(srcdir)/m6888x-auto.sh > $@

# dependencies on automatically-generated IEEE 754 sources:
$(tme_ic_m68k_la_OBJECTS): $(top_builddir)/ic/ieee754/ieee754-auto.h $(top_builddir)/ic/ieee754/ieee754-ops-auto.h
$(pkglib_LTLIBRARIES): $(top_builddir)/ic/ieee754/libtme-ieee754.la
$(top_builddir)/ic/ieee754/ieee754-auto.h $(top_builddir)/ic/ieee754/ieee754-ops-auto.h $(top_builddir)/ic/ieee754/libtme-ieee754.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;

include $(top_srcdir)/modules
