## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/m68k/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL 

pkglib_LTLIBRARIES = tme_ic_m68k.la
tme_ic_m68k_la_SOURCES = m68k-opmap.c \
	m68k-insns.c \
	m68k-misc.c \
	m68010.c \
	m68020.c \
	m6888x.c
tme_ic_m68k_la_LDFLAGS = -module -version-info 0:0:0
tme_ic_m68k_la_LIBADD = ../ieee754/libtme-ieee754.la

noinst_SCRIPTS = m68k-misc-auto.sh \
	m68k-insns-auto.sh \
	m68k-bus-auto.sh \
	m6888x-auto.sh

BUILT_SOURCES = $(noinst_SCRIPTS) m68k-auto.h m68k-opmap.c m68k-insns-auto.c m68k-bus-auto.c m6888x-auto.c
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = m68k-impl.h \
	$(BUILT_SOURCES) \
	$(noinst_SCRIPTS:.sh=.m4)  \
	m68k-execute.c \
	m68k-iset-expand.pl \
	m68k-iset.txt \
	m68k-opmap-make.pl

# default rule for regenerating the portable shell scripts from m4 descriptions
$(noinst_SCRIPTS): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make m68k-auto.h:
m68k-auto.h: m68k-misc-auto.sh m68k-insns-auto.sh
	$(SHELL) m68k-misc-auto.sh --header > $@
	$(SHELL) m68k-insns-auto.sh --header >> $@

# target to make m68k-opmap.c:
m68k-opmap.c: m68k-iset.txt m68k-iset-expand.pl m68k-opmap-make.pl
	( $(PERL) $(srcdir)/m68k-iset-expand.pl m68000 < $(srcdir)/m68k-iset.txt ; \
	  $(PERL) $(srcdir)/m68k-iset-expand.pl m68010 < $(srcdir)/m68k-iset.txt ; \
	  $(PERL) $(srcdir)/m68k-iset-expand.pl m68020 < $(srcdir)/m68k-iset.txt ) | \
	  $(PERL) $(srcdir)/m68k-opmap-make.pl > $@

# target to make m68k-insns-auto.c:
m68k-insns-auto.c: m68k-insns-auto.sh m68k-misc-auto.sh
	$(SHELL) m68k-insns-auto.sh > $@
	$(SHELL) m68k-misc-auto.sh >> $@

# target to make m68k-bus-auto.c:
m68k-bus-auto.c: m68k-bus-auto.sh
	$(SHELL) m68k-bus-auto.sh > $@

# target to make m6888x-auto.c:
m6888x-auto.c: m6888x-auto.sh
	$(SHELL) m6888x-auto.sh > $@

# dependencies on automatically-generated IEEE 754 sources:
$(tme_ic_m68k_la_OBJECTS): ../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h
$(pkglib_LTLIBRARIES): ../ieee754/libtme-ieee754.la
../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h ../ieee754/libtme-ieee754.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;

include $(top_srcdir)/modules
