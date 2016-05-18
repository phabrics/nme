## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator libtme/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

SUBDIRS = host

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I. $(LTDLINCL) -I$(top_srcdir)/lib -D_TME_IMPL
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
	module.c \
	threads.c \
	element.c \
	log.c \
	hash.c \
	token.c \
	runlength.c \
	alloc.c

if THREADS_SJLJ
libtme_la_SOURCES += threads-sjlj.c
endif
libtme_la_LDFLAGS = -version-info 0:0:0
libtme_la_LIBADD = $(LIBLTDL) $(LTLIBOBJS) $(top_builddir)/libopenvpn/libopenvpn.la
if THREADS_GLIB
libtme_la_LIBADD += $(GLIB_LIBS)
endif

noinst_SCRIPTS = memory-auto.sh

BUILT_SOURCES = $(noinst_SCRIPTS)
CLEANFILES = $(noinst_SCRIPTS)

EXTRA_DIST = log-prf.c \
	misc.c \
	$(noinst_SCRIPTS) \
	$(noinst_SCRIPTS:.sh=.m4) 

# default rule for regenerating the portable shell scripts from m4 descriptions
$(noinst_SCRIPTS): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make memory-auto.h:
memory-auto.h: memory-auto.sh
	$(SHELL) $< --header > $@

# target to make memory-auto.c:
memory-auto.c: memory-auto.sh
	$(SHELL) $< > $@

memory-auto.lo: memory-auto.h

# module.c needs to know the LTDL_SHLIBPATH_VAR that libltdl
# is using:
module.lo: shlibvar.h
shlibvar.h: $(top_builddir)/config.h Makefile
	grep LT_MODULE_PATH_VAR $(top_builddir)/config.h > $@
	grep LT_DLSEARCH_PATH $(top_builddir)/config.h >> $@
	echo "#define TME_PACKAGE_PATH \"$(libdir)\"" >> $@
