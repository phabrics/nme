## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/sparc/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

pkglib_LTLIBRARIES = tme_ic_sparc.la
tme_ic_sparc_la_SOURCES = \
	sparc-insns.c \
	sparc-misc.c \
	sparc-fpu.c \
	sparc-timing.c \
	sparc-recode.c \
	stp103x.c \
	cy7c601.c \
	mb86900.c
tme_ic_sparc_la_LIBADD = ../ieee754/libtme-ieee754.la $(TME_LIBS)
noinst_SCRIPTS = sparc-misc-auto.sh \
	sparc-insns-auto.sh \
	sparc-fpu-auto.sh \
	sparc-vis-auto.sh \
	sparc-bus-auto.sh
BUILT_SOURCES = sparc-auto.h sparc-insns-auto.c sparc-bus-auto.c sparc-fpu-auto.c sparc-vis-auto.c
MAINTAINERCLEANFILES = $(noinst_SCRIPTS) $(BUILT_SOURCES)

EXTRA_DIST = sparc-impl.h \
	$(MAINTAINERCLEANFILES) \
	$(noinst_SCRIPTS:.sh=.m4)  \
	sparc-execute.c \
	sparc-rc-cc.c \
	sparc-rc-chain.c \
	sparc-rc-insns.c \
	sparc-rc-ls.c

# default rule for regenerating the portable shell scripts from m4 descriptions
$(addprefix $(srcdir)/,$(noinst_SCRIPTS)): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make sparc-auto.h:
$(srcdir)/sparc-auto.h: sparc-misc-auto.sh sparc-insns-auto.sh
	$(SHELL) $< --header > $@
	$(SHELL) $(srcdir)/sparc-insns-auto.sh --header >> $@

# target to make sparc-insns-auto.c:
$(srcdir)/sparc-insns-auto.c: sparc-insns-auto.sh sparc-misc-auto.sh
	$(SHELL) $< > $@
	$(SHELL) $(srcdir)/sparc-misc-auto.sh >> $@

# target to make sparc-bus-auto.c:
$(srcdir)/sparc-bus-auto.c: sparc-bus-auto.sh
	$(SHELL) $< > $@

# target to make sparc-fpu-auto.c:
$(srcdir)/sparc-fpu-auto.c: sparc-fpu-auto.sh
	$(SHELL) $< > $@

# target to make sparc-vis-auto.c:
$(srcdir)/sparc-vis-auto.c: sparc-vis-auto.sh
	$(SHELL) $< > $@

# dependencies on automatically-generated IEEE 754 sources:
$(tme_ic_sparc_la_OBJECTS): ../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h
$(pkglib_LTLIBRARIES): ../ieee754/libtme-ieee754.la
../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h ../ieee754/libtme-ieee754.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;

include $(top_srcdir)/modules.mk
