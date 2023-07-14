## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator tme/:

AUTOMAKE_OPTIONS = 1.4 gnu

SUBDIRS = generic ic machine scsi host bus libopenvpn

pkginclude_HEADERS = memory.h \
	common.h \
	connection.h \
	element.h \
	threads.h \
	threads-posix.h \
	threads-glib.h \
	threads-sjlj.h \
	module.h \
	log.h \
	tme.h \
	misc.h \
	hash.h \
	recode.h \
	token.h \
	completion.h \
	tmesh.h \
	runlength.h
info_TEXINFOS = tme.texi
tme_TEXINFOS = fdl.texi

tmedir = $(pkglibdir)
dist_tme_DATA = tme-plugins.txt

BUILT_SOURCES = shlibvar.h

CLEANFILES = $(BUILT_SOURCES)

# module.c needs to know the LTDL_SHLIBPATH_VAR that libltdl
# is using:
shlibvar.h: $(top_builddir)/config.h Makefile
	grep LT_MODULE_PATH_VAR $(top_builddir)/config.h > $@
	grep LT_DLSEARCH_PATH $(top_builddir)/config.h >> $@
	echo "#define TME_PACKAGE_PATH \"$(libdir)\"" >> $@
	echo -n "#define TME_PREFIX_PATH \"" >> $@
	echo -n @ROOT_DIR@ | tr -d "\n\r\f" >> $@
#	$(CYGPATH_M) / | tr -d "\n\r\f" >> $@
	echo \" >> $@
