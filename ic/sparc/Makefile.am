## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/sparc/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL 

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
tme_ic_sparc_la_LDFLAGS = -module -version-info 0:0:0
tme_ic_sparc_la_LIBADD = ../ieee754/libtme-ieee754.la
noinst_SCRIPTS = sparc-misc-auto.sh \
	sparc-insns-auto.sh \
	sparc-fpu-auto.sh \
	sparc-vis-auto.sh \
	sparc-bus-auto.sh
BUILT_SOURCES = $(noinst_SCRIPTS) sparc-auto.h sparc-insns-auto.c sparc-bus-auto.c sparc-fpu-auto.c sparc-vis-auto.c
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = sparc-impl.h \
	$(BUILT_SOURCES) \
	$(noinst_SCRIPTS:.sh=.m4)  \
	sparc-execute.c \
	sparc-rc-cc.c \
	sparc-rc-chain.c \
	sparc-rc-insns.c \
	sparc-rc-ls.c

# default rule for regenerating the portable shell scripts from m4 descriptions
$(noinst_SCRIPTS): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make sparc-auto.h:
sparc-auto.h: sparc-misc-auto.sh sparc-insns-auto.sh
	$(SHELL) sparc-misc-auto.sh --header > $@
	$(SHELL) sparc-insns-auto.sh --header >> $@

# target to make sparc-insns-auto.c:
sparc-insns-auto.c: sparc-insns-auto.sh sparc-misc-auto.sh
	$(SHELL) sparc-insns-auto.sh > $@
	$(SHELL) sparc-misc-auto.sh >> $@

# target to make sparc-bus-auto.c:
sparc-bus-auto.c: sparc-bus-auto.sh
	$(SHELL) sparc-bus-auto.sh > $@

# target to make sparc-fpu-auto.c:
sparc-fpu-auto.c: sparc-fpu-auto.sh
	$(SHELL) sparc-fpu-auto.sh > $@

# target to make sparc-vis-auto.c:
sparc-vis-auto.c: sparc-vis-auto.sh
	$(SHELL) sparc-vis-auto.sh > $@

# dependencies on automatically-generated IEEE 754 sources:
$(tme_ic_sparc_la_OBJECTS): ../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h
$(pkglib_LTLIBRARIES): ../ieee754/libtme-ieee754.la
../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h ../ieee754/libtme-ieee754.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;

include $(top_srcdir)/modules
