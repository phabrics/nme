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

noinst_SCRIPTS = fb-xlat-auto.sh \
	 	float-auto.sh \
		bus-device-auto.sh

CLEANFILES = $(noinst_SCRIPTS)

BUILT_SOURCES =	$(noinst_SCRIPTS)

EXTRA_DIST = $(noinst_SCRIPTS) \
	$(noinst_SCRIPTS:.sh=.m4) \
	float-auto.c \
	bus-device-auto.c

# default rule for regenerating the portable shell scripts from m4 descriptions
$(noinst_SCRIPTS): %.sh: %.m4
	$(AUTOM4TE) -l m4sh $< > $@

# target to make float-auto.c and bus-device-auto.c:
float-auto.c bus-device-auto.c: %.c: %.sh
	$(SHELL) $< > $@

# target to make fb-xlat-auto.c:
fb-xlat-auto.c: %.c: %.sh
	$(SHELL) $< src @TME_FB_XLAT_SRC@ dst @TME_FB_XLAT_DST@ > $@

# fb.lo depends on fb-xlat-auto.c:
fb.lo: fb-xlat-auto.c

# float.lo depends on float-auto.c:
float.lo: float-auto.c

# bus-device.lo depends on bus-device-auto.c:
bus-device.lo: bus-device-auto.c

pkglib_LTLIBRARIES = tme_generic.la
tme_generic_la_SOURCES = bus-el.c
tme_generic_la_LDFLAGS = -module -version-info 0:0:0
tme_generic_la_LIBADD = libtme-generic.la

include $(top_srcdir)/modules
