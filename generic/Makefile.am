## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator generic/:

AUTOMAKE_OPTIONS = 1.4 gnu

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I. -D_TME_IMPL

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
	$(SHELL) $(srcdir)/fb-xlat-auto.sh src @TME_FB_XLAT_SRC@ dst @TME_FB_XLAT_DST@ > $@

# fb.lo depends on fb-xlat-auto.c:
fb.lo: fb-xlat-auto.c

# target to make float-auto.c:
float-auto.c: float-auto.sh
	$(SHELL) $(srcdir)/float-auto.sh > $@

# float.lo depends on float-auto.c:
float.lo: float-auto.c

# target to make bus-device-auto.c:
bus-device-auto.c: bus-device-auto.sh
	$(SHELL) $(srcdir)/bus-device-auto.sh > $@

# bus-device.lo depends on bus-device-auto.c:
bus-device.lo: bus-device-auto.c

pkglib_LTLIBRARIES = tme_generic.la
tme_generic_la_SOURCES = bus-el.c
tme_generic_la_LDFLAGS = -module -version-info 0:0:0
tme_generic_la_LIBADD = libtme-generic.la

EXTRA_DIST = fb-xlat-auto.sh \
	float-auto.sh \
	float-auto.c \
	bus-device-auto.sh \
	bus-device-auto.c

include $(top_srcdir)/modules
