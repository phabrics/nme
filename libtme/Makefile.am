## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator libtme/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

SUBDIRS = host

AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL $(LTDLINCL)

if THREADS_GLIB
AM_CPPFLAGS += $(GLIB_CFLAGS)
endif
lib_LTLIBRARIES = libtme.la
pkginclude_HEADERS = memory-auto.h
libtme_la_SOURCES = misc-host.c \
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
        openvpn-setup.c \
	module.c \
	threads.c \
	element.c \
	log.c \
	hash.c \
	token.c \
	runlength.c \
	alloc.c

if THREADS_FIBER
libtme_la_SOURCES += threads-fiber.c
endif
libtme_la_LDFLAGS = -version-info 0:0:0
libtme_la_LIBADD = $(LIBLTDL) $(top_builddir)/libopenvpn/libopenvpn.la

if THREADS_GLIB
libtme_la_LIBADD += $(GLIB_LIBS)
endif

noinst_SCRIPTS = memory-auto.sh

BUILT_SOURCES = memory-auto.h memory-auto.c

MAINTAINERCLEANFILES = $(noinst_SCRIPTS) $(BUILT_SOURCES)

EXTRA_DIST = log-prf.c \
	misc.c \
	$(MAINTAINERCLEANFILES) \
	$(noinst_SCRIPTS:.sh=.m4) 

# default rule for regenerating the portable shell scripts from m4 descriptions
$(addprefix $(srcdir)/,$(noinst_SCRIPTS)): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make memory-auto.h:
$(srcdir)/memory-auto.h: memory-auto.sh
	$(SHELL) $< --header > $@

# target to make memory-auto.c:
$(srcdir)/memory-auto.c: memory-auto.sh
	$(SHELL) $< > $@

module.c: $(top_builddir)/tme/shlibvar.h
