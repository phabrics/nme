## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator lib@PACKAGE@/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

SUBDIRS = host

AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL $(LTDLINCL) $(LIBOPENVPN_CFLAGS) $(ZLIB_CFLAGS) $(GLIB_CFLAGS) $(SDL_CFLAGS)

lib_LTLIBRARIES = lib@PACKAGE@.la
pkginclude_HEADERS = memory-auto.h
lib@PACKAGE@_la_SOURCES = misc-host.c \
	memory-auto.c \
	openvpn-setup.c \
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
	threads-fiber.c \
	element.c \
	log.c \
	hash.c \
	token.c \
	runlength.c \
	alloc.c

lib@PACKAGE@_la_LIBADD = $(LIBLTDL) $(LIBOPENVPN_LIBS) $(ZLIB_LIBS) $(GLIB_LIBS) $(SDL_LIBS)
lib@PACKAGE@_la_LDFLAGS = -version-info 0:0:0

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
