## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/sparc/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL 

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
tme_ic_sparc_la_LIBADD = $(top_builddir)/libtme/libtme.la \
	$(top_builddir)/generic/libtme-generic.la \
	../ieee754/libtme-ieee754.la
BUILT_SOURCES = sparc-auto.h sparc-insns-auto.c sparc-bus-auto.c sparc-fpu-auto.c sparc-vis-auto.c
EXTRA_DIST = sparc-impl.h \
	sparc-misc-auto.sh \
	sparc-insns-auto.sh \
	sparc-fpu-auto.sh \
	sparc-vis-auto.sh \
	sparc-bus-auto.sh \
	sparc-execute.c \
	sparc-rc-cc.c \
	sparc-rc-chain.c \
	sparc-rc-insns.c \
	sparc-rc-ls.c \
	$(BUILT_SOURCES)

# target to make sparc-auto.h:
sparc-auto.h: sparc-misc-auto.sh sparc-insns-auto.sh
	$(SHELL) $(srcdir)/sparc-misc-auto.sh --header > $@
	$(SHELL) $(srcdir)/sparc-insns-auto.sh --header >> $@

# target to make sparc-insns-auto.c:
sparc-insns-auto.c: sparc-insns-auto.sh sparc-misc-auto.sh
	$(SHELL) $(srcdir)/sparc-insns-auto.sh > $@
	$(SHELL) $(srcdir)/sparc-misc-auto.sh >> $@

# target to make sparc-bus-auto.c:
sparc-bus-auto.c: sparc-bus-auto.sh
	$(SHELL) $(srcdir)/sparc-bus-auto.sh > $@

# target to make sparc-fpu-auto.c:
sparc-fpu-auto.c: sparc-fpu-auto.sh
	$(SHELL) $(srcdir)/sparc-fpu-auto.sh > $@

# target to make sparc-vis-auto.c:
sparc-vis-auto.c: sparc-vis-auto.sh
	$(SHELL) $(srcdir)/sparc-vis-auto.sh > $@

# dependencies on automatically-generated IEEE 754 sources:
$(tme_ic_sparc_la_OBJECTS): ../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h
$(pkglib_LTLIBRARIES): ../ieee754/libtme-ieee754.la
../ieee754/ieee754-auto.h ../ieee754/ieee754-ops-auto.h ../ieee754/libtme-ieee754.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;

include $(top_srcdir)/modules
