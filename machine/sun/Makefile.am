## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun:

AUTOMAKE_OPTIONS = 1.4 gnu

lib_LTLIBRARIES = lib@PACKAGE@-machine-sun.la
lib@PACKAGE@_machine_sun_la_SOURCES = sun-mmu.c \
	sun-bwtwo.c \
	sun-cgtwo.c \
	sun-cgsix.c \
	sun-fb.c sun-fb.h \
	sun-si.c \
	sun-obie.c
lib@PACKAGE@_machine_sun_la_LDFLAGS = -version-info 0:0:0
lib@PACKAGE@_machine_sun_la_LIBADD = $(@PACKAGE@_LIBS)

bin_SCRIPTS = nme-sun-idprom nme-sun-eeprom
dist_bin_SCRIPTS = nme-make-machs

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = sun-keyboards.txt my-sun-macros.txt

EXTRA_DIST = nme-sun-idprom.pl nme-sun-eeprom.pl

# target to make nme-sun-idprom:
nme-sun-idprom: nme-sun-idprom.pl
	sed -e s%/usr/pkg/bin/perl%$(PERL)% < $(srcdir)/nme-sun-idprom.pl > $@

# target to make nme-sun-eeprom:
nme-sun-eeprom: nme-sun-eeprom.pl $(top_srcdir)/tools/nme-binary-struct.pl.in
	cat $(top_srcdir)/tools/nme-binary-struct.pl.in $(srcdir)/nme-sun-eeprom.pl | \
	sed -e s%/usr/pkg/bin/perl%$(PERL)% > $@
include $(top_srcdir)/modules.mk
