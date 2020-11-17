## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator:

AUTOMAKE_OPTIONS = 1.4 gnu
ACLOCAL_AMFLAGS = -I m4

if USE_TAP_WINDOWS
nodist_include_HEADERS = tap-windows.h
endif

tap-windows.h: tap-windows-def.h
	cp $< $@

if USING_OPENVPN
SUBDIRS = . libopenvpn
else
SUBDIRS = 
endif
SUBDIRS += tme lib libltdl libtme generic ic machine host bus serial scsi tmesh tools
EXTRA_DIST = modules edist tap-windows-def.h
pkgdata_DATA = tmeconfig.h tmememory.h

# target to hook into the distribution:
#
dist-hook:
	rm -f ${distdir}/libtme/recode-host.c
	find $(distdir)/edist -name CVS -a -exec rm -r {} ';' -a -prune
