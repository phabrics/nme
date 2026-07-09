## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator libtme/host/x86:

EXTRA_DIST = recode-x86.c \
	rc-x86-flags.c \
	rc-x86-conds.c \
	rc-x86-tlb.c \
	rc-x86-chain.c \
	rc-x86-rws.c \
	rc-x86-insns.c \
	rc-x86-regs.c \
	rc-x86-subs.S \
	misc-x86.c

noinst_SCRIPTS = rc-x86-subs

BUILT_SOURCES = rc-x86-subs.c

MAINTAINERCLEANFILES = $(noinst_SCRIPTS) $(BUILT_SOURCES)

EXTRA_DIST += $(noinst_SCRIPTS)

rc-x86-subs.c: rc-x86-subs rc-x86-subs.S
	$(SHELL) $(srcdir)/rc-x86-subs > $@
