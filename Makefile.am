## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator:

AUTOMAKE_OPTIONS = 1.4 gnu
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = tme libltdl libtme generic ic machine host bus serial scsi tmesh tools
EXTRA_DIST = modules dist
pkgdata_DATA = tmeconfig.h

# target to hook into the distribution:
#
dist-hook:
	rm -f ${distdir}/libtme/recode-host.c
	find $(distdir)/dist -name CVS -a -exec rm -r {} ';' -a -prune
