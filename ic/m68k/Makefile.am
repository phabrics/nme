## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/m68k/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

pkglib_LTLIBRARIES = @PACKAGE@_ic_m68k.la
@PACKAGE@_ic_m68k_la_SOURCES = m68k-opmap.c \
	m68k-insns.c \
	m68k-misc.c \
	m68010.c \
	m68020.c \
	m6888x.c
@PACKAGE@_ic_m68k_la_LIBADD = ../ieee754/lib@PACKAGE@-ieee754.la $(@PACKAGE@_LIBS)

noinst_SCRIPTS = m68k-misc-auto.sh \
	m68k-insns-auto.sh \
	m68k-bus-auto.sh \
	m6888x-auto.sh

BUILT_SOURCES = m68k-auto.h m68k-opmap.c m68k-insns-auto.c m68k-bus-auto.c m6888x-auto.c
MAINTAINERCLEANFILES = $(noinst_SCRIPTS) $(BUILT_SOURCES)

EXTRA_DIST = m68k-impl.h \
	$(MAINTAINERCLEANFILES) \
	$(noinst_SCRIPTS:.sh=.m4)  \
	m68k-execute.c \
	m68k-iset-expand.pl \
	m68k-iset.txt \
	m68k-opmap-make.pl

# default rule for regenerating the portable shell scripts from m4 descriptions
$(addprefix $(srcdir)/,$(noinst_SCRIPTS)): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make m68k-auto.h:
$(srcdir)/m68k-auto.h: m68k-misc-auto.sh m68k-insns-auto.sh
	$(SHELL) $< --header > $@
	$(SHELL) $(srcdir)/m68k-insns-auto.sh --header >> $@

# target to make m68k-opmap.c:
$(srcdir)/m68k-opmap.c: m68k-iset.txt m68k-iset-expand.pl m68k-opmap-make.pl
	( $(PERL) m68k-iset-expand.pl m68000 < m68k-iset.txt ; \
	  $(PERL) m68k-iset-expand.pl m68010 < m68k-iset.txt ; \
	  $(PERL) m68k-iset-expand.pl m68020 < m68k-iset.txt ) | \
	  $(PERL) m68k-opmap-make.pl > $@

# target to make m68k-insns-auto.c:
$(srcdir)/m68k-insns-auto.c: m68k-insns-auto.sh m68k-misc-auto.sh
	$(SHELL) $< > $@
	$(SHELL) $(srcdir)/m68k-misc-auto.sh >> $@

# target to make m68k-bus-auto.c:
$(srcdir)/m68k-bus-auto.c: m68k-bus-auto.sh
	$(SHELL) $< > $@

# target to make m6888x-auto.c:
$(srcdir)/m6888x-auto.c: m6888x-auto.sh
	$(SHELL) $< > $@

include $(top_srcdir)/modules.mk
