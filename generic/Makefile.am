## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator generic/:

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te

AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL -DLIBTME_GENERIC_BUILD

lib_LTLIBRARIES = lib@PACKAGE@-generic.la
lib@PACKAGE@_generic_la_SOURCES = bus.c \
	bus-device.c \
	serial.c \
	ethernet.c \
	fb.c \
	scsi.c \
	disk.c \
	tape.c \
	mouse.c \
	float.c \
	keyboard.c \
	display-mouse.c display-keyboard.c display.c \
	eth-impl.c 

lib@PACKAGE@_generic_la_LDFLAGS = -version-info 0:0:0
lib@PACKAGE@_generic_la_LIBADD = $(top_builddir)/libtme/lib@PACKAGE@.la

noinst_SCRIPTS = float-auto.sh \
		bus-device-auto.sh \
		fb-xlat-auto.sh

BUILT_SOURCES =	$(noinst_SCRIPTS:.sh=.c)

MAINTAINERCLEANFILES = $(noinst_SCRIPTS) $(BUILT_SOURCES)

EXTRA_DIST = $(noinst_SCRIPTS) $(noinst_SCRIPTS:.sh=.m4) fb-xlat-auto.pl float-auto.c bus-device-auto.c

# default rule for regenerating the portable shell scripts from m4 descriptions
.m4.sh:
	$(AUTOM4TE) -l m4sh $< > $@

# default rule for regenerating source files from portable shell scripts
.sh.c:
	$(SHELL) $< > $@

# target to make fb-xlat-auto.c:
fb-xlat-auto.c: fb-xlat-auto.in fb-xlat-auto.sh
	cat fb-xlat-auto.in | xargs $(SHELL) $(srcdir)/fb-xlat-auto.sh > $@

.pl.in:
if USING_X
	$(PERL) $< src $(NME_FB_XLAT_SRC) dst $(NME_FB_XLAT_DST) > $@
else
	echo "src $(NME_FB_XLAT_SRC) dst $(NME_FB_XLAT_DST)" > $@
endif

SUBDIRS = . bus
