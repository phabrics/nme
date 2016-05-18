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

install-exec-local:
	sort -u < tme-plugins.txt > tme-plugins.txt.final
	mv tme-plugins.txt.final tme-plugins.txt
	$(mkinstalldirs) $(DESTDIR)$(pkglibdir)
	$(INSTALL_DATA) tme-plugins.txt $(DESTDIR)$(pkglibdir)
