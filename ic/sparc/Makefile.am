## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator ic/sparc/:

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

pkglib_LTLIBRARIES = @PACKAGE@_ic_sparc.la
@PACKAGE@_ic_sparc_la_SOURCES = \
	sparc-insns.c \
	sparc-misc.c \
	sparc-fpu.c \
	sparc-timing.c \
	sparc-recode.c \
	stp103x.c \
	cy7c601.c \
	mb86900.c
@PACKAGE@_ic_sparc_la_LIBADD = ../ieee754/lib@PACKAGE@-ieee754.la $(@PACKAGE@_LIBS)
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

include $(top_srcdir)/modules.mk
