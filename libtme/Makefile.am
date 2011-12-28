## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator libtme/:

AUTOMAKE_OPTIONS = 1.4 gnu

SUBDIRS = host

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. $(INCLTDL) $(GTK_CFLAGS) -D_TME_IMPL

lib_LTLIBRARIES = libtme.la
pkginclude_HEADERS = memory-auto.h
libtme_la_SOURCES = misc-host.c \
	threads-sjlj.c	\
	memory-auto.c \
	recode-flags.c \
	recode-conds.c \
	recode-address.c \
	recode-rws.c \
	recode-regs.c \
	recode-insns.c \
	recode-ic.c \
	recode-host.c \
	recode-impl.h \
	module.c \
	element.c \
	log.c \
	hash.c \
	token.c \
	runlength.c \
	alloc.c
libtme_la_LDFLAGS = -version-info 0:0:0
libtme_la_LIBADD = $(LIBLTDL)
BUILT_SOURCES = memory-auto.h memory-auto.c
EXTRA_DIST = log-prf.c \
	misc.c \
	memory-auto.sh

# target to make memory-auto.h:
memory-auto.h: memory-auto.sh
	$(SHELL) $(srcdir)/memory-auto.sh --header > $@

# target to make memory-auto.c:
memory-auto.c: memory-auto.sh
	$(SHELL) $(srcdir)/memory-auto.sh > $@
memory-auto.lo: memory-auto.h

# module.c needs to know the LTDL_SHLIBPATH_VAR that libltdl
# is using:
module.lo: shlibvar.h
shlibvar.h: $(top_builddir)/libltdl/config.h
	grep LTDL_SHLIBPATH_VAR $(top_builddir)/libltdl/config.h > $@
