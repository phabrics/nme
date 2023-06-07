## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator:

AUTOMAKE_OPTIONS = 1.4 gnu
ACLOCAL_AMFLAGS = -I m4

if USE_TAP_WINDOWS
nodist_include_HEADERS = tap-windows.h
endif

tap-windows.h: tap-windows-def.h
	cp $< $@

SUBDIRS = . libopenvpn tme lib libltdl libtme generic ic machine host bus serial scsi tmesh tools

EXTRA_DIST = modules edist tap-windows-def.h
pkgdata_DATA = tmeconfig.h tmememory.h

# target to hook into the distribution:
#
dist-hook:
	rm -f ${distdir}/libtme/recode-host.c
	find $(distdir)/edist -name CVS -a -exec rm -r {} ';' -a -prune

# The target to update the list of modules to preopen on platforms that don't support dlopen.
# This will update this list to be automatically included by automake
# so that future builds will track all module dependencies.
# If new modules are added, "make modules" must be run to update this list in the source.
modules-local:
	@if test ! -e .modules; then \
		rm -f ${top_srcdir}/tme/tme-plugins.txt ${top_srcdir}/tme-hosts.txt ; \
		echo -n tme_preopen=> $(top_srcdir)/tme-preopen.txt ; \
		chmod +x tme-modules.sh ; \
		touch .modules ; \
	fi
