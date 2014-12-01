## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator generic/:

AUTOMAKE_OPTIONS = 1.4 gnu

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

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

# target to make fb-xlat-auto.c:
fb-xlat-auto.c: fb-xlat-auto.sh
	$(SHELL) fb-xlat-auto.sh src @TME_FB_XLAT_SRC@ dst @TME_FB_XLAT_DST@ > $@

fb-xlat-auto.sh: fb-xlat-auto.m4
	$(AUTOM4TE) -l m4sh $(srcdir)/fb-xlat-auto.m4 > $@

# fb.lo depends on fb-xlat-auto.c:
fb.lo: fb-xlat-auto.c

# target to make float-auto.c:
float-auto.c: float-auto.sh
	$(SHELL) float-auto.sh > $@

float-auto.sh: float-auto.m4
	$(AUTOM4TE) -l m4sh $(srcdir)/float-auto.m4 > $@

# float.lo depends on float-auto.c:
float.lo: float-auto.c

# target to make bus-device-auto.c:
bus-device-auto.c: bus-device-auto.sh
	$(SHELL) bus-device-auto.sh > $@

bus-device-auto.sh: bus-device-auto.m4
	$(AUTOM4TE) -l m4sh $(srcdir)/bus-device-auto.m4 > $@

# bus-device.lo depends on bus-device-auto.c:
bus-device.lo: bus-device-auto.c

pkglib_LTLIBRARIES = tme_generic.la
tme_generic_la_SOURCES = bus-el.c
tme_generic_la_LDFLAGS = -module -version-info 0:0:0
tme_generic_la_LIBADD = libtme-generic.la

EXTRA_DIST = fb-xlat-auto.sh \
	fb-xlat-auto.m4 \
	float-auto.sh \
	float-auto.m4 \
	float-auto.c \
	bus-device-auto.sh \
	bus-device-auto.m4 \
	bus-device-auto.c

include $(top_srcdir)/modules
