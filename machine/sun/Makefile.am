## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator machine/sun:

AUTOMAKE_OPTIONS = 1.4 gnu

lib_LTLIBRARIES = libtme-machine-sun.la
libtme_machine_sun_la_SOURCES = sun-mmu.c \
	sun-bwtwo.c \
	sun-cgtwo.c \
	sun-cgsix.c \
	sun-fb.c sun-fb.h \
	sun-si.c \
	sun-obie.c
libtme_machine_sun_la_LDFLAGS = -version-info 0:0:0
libtme_machine_sun_la_LIBADD = $(TME_LIBS)

bin_SCRIPTS = tme-sun-idprom tme-sun-eeprom
dist_bin_SCRIPTS = tme-make-machs

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = sun-keyboards.txt my-sun-macros.txt

EXTRA_DIST = tme-sun-idprom.pl tme-sun-eeprom.pl

# target to make tme-sun-idprom:
tme-sun-idprom: tme-sun-idprom.pl
	sed -e s%/usr/pkg/bin/perl%$(PERL)% < $(srcdir)/tme-sun-idprom.pl > $@

# target to make tme-sun-eeprom:
tme-sun-eeprom: tme-sun-eeprom.pl $(top_srcdir)/tools/tme-binary-struct.pl.in
	cat $(top_srcdir)/tools/tme-binary-struct.pl.in $(srcdir)/tme-sun-eeprom.pl | \
	sed -e s%/usr/pkg/bin/perl%$(PERL)% > $@
include $(top_srcdir)/modules.mk
