## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator generic/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL -DLIBTME_GENERIC_BUILD

lib_LTLIBRARIES = libtme-generic.la
libtme_generic_la_SOURCES = bus.c \
	bus-device.c \
	serial.c \
	ethernet.c \
	fb.c \
	scsi.c \
	disk.c \
	tape.c \
	mouse.c \
	float.c \
	keyboard.c
libtme_generic_la_LDFLAGS = -version-info 0:0:0
libtme_generic_la_LIBADD = $(top_builddir)/libtme/libtme.la

noinst_SCRIPTS = float-auto.sh \
		bus-device-auto.sh \
		fb-xlat-auto.sh

BUILT_SOURCES =	$(noinst_SCRIPTS:.sh=.c)

CLEANFILES = $(noinst_SCRIPTS) $(BUILT_SOURCES)

EXTRA_DIST = $(CLEANFILES) $(noinst_SCRIPTS:.sh=.m4) fb-xlat-auto.pl

# default rule for regenerating the portable shell scripts from m4 descriptions
$(noinst_SCRIPTS): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make float-auto.c and bus-device-auto.c:
float-auto.c bus-device-auto.c: %.c: %.sh
	$(SHELL) $< > $@

# target to make fb-xlat-auto.c:
if USING_X
fb-xlat-auto.c: %.c: %.sh %.in
	cat $*.in | xargs $(SHELL) $< > $@

fb-xlat-auto.in: %.in: %.pl
	$(PERL) $< src $(TME_FB_XLAT_SRC) dst $(TME_FB_XLAT_DST) > $@ || \
	echo "src $(TME_FB_XLAT_SRC) dst $(TME_FB_XLAT_DST)" > $@
else
fb-xlat-auto.c: %.c: %.sh
	$(SHELL) $< src $(TME_FB_XLAT_SRC) dst $(TME_FB_XLAT_DST) > $@
endif

pkglib_LTLIBRARIES = tme_generic.la
tme_generic_la_SOURCES = bus-el.c
tme_generic_la_LDFLAGS = -module -version-info 0:0:0
tme_generic_la_LIBADD = libtme-generic.la

include $(top_srcdir)/modules
