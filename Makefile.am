## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator:

AUTOMAKE_OPTIONS = 1.4 gnu
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = tme lib libltdl libopenvpn libtme generic ic machine host bus serial scsi tmesh tools
EXTRA_DIST = modules edist
pkgdata_DATA = tmeconfig.h tmememory.h

# target to hook into the distribution:
#
dist-hook:
	rm -f ${distdir}/libtme/recode-host.c
	find $(distdir)/edist -name CVS -a -exec rm -r {} ';' -a -prune
