## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator:

AUTOMAKE_OPTIONS = 1.4 gnu
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . $(subdirs) tme libltdl libtme generic ic machine host bus serial scsi tmesh tools

EXTRA_DIST = modules.mk edist
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
		rm -f ${top_srcdir}/tme/tme-plugins.txt ${top_srcdir}/tme-hosts.txt ${top_srcdir}/tme-modules.txt ; \
		echo -n tme_preopen=> $(top_srcdir)/tme-preopen.txt ; \
		chmod +x tme-modules.sh ; \
		touch .modules ; \
	fi
