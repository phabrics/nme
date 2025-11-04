## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator @PACKAGE@sh/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL $(@PACKAGE@_HOST_CFLAGS)

lib_LTLIBRARIES = lib@PACKAGE@sh.la
lib@PACKAGE@sh_la_SOURCES = @PACKAGE@sh-impl.h \
	@PACKAGE@sh-input.y \
	@PACKAGE@sh-util.c \
	@PACKAGE@sh-cmds.c
lib@PACKAGE@sh_la_LIBADD = $(top_builddir)/libtme/lib@PACKAGE@.la

@PACKAGE@sh_SOURCES = @PACKAGE@sh.c @PACKAGE@sh-threads.c
bin_PROGRAMS = @PACKAGE@sh
@PACKAGE@sh_LDADD = lib@PACKAGE@sh.la $(@PACKAGE@_PREOPEN)
@PACKAGE@sh_LDFLAGS = 

EXTRA_DIST = @PACKAGE@sh.sh

if HAVE_EMSCRIPTEN
bin_SCRIPTS = @PACKAGE@sh.sh @PACKAGE@sh.js @PACKAGE@sh.wasm @PACKAGE@sh.data
@PACKAGE@sh_LDADD += $(SDL_LIBS) 
@PACKAGE@sh_LDFLAGS += -sASYNCIFY
if USE_NODEFS
AM_CPPFLAGS += -DUSE_NODEFS
@PACKAGE@sh_LDADD += -lnodefs.js
else
@PACKAGE@sh_LDFLAGS += --emrun --preload-file $(top_builddir)@$(ROOT_DIR)
endif
@PACKAGE@sh.js @PACKAGE@sh.wasm @PACKAGE@sh.data: @PACKAGE@sh.html
	@test -f $@ || $(LN_S) .libs/$@ .
install-exec-hook:
	test -f $(DESTDIR)$(bindir)/@PACKAGE@sh.sh && \
	mv -f $(DESTDIR)$(bindir)/@PACKAGE@sh.sh $(DESTDIR)$(bindir)/@PACKAGE@sh
	mkdir -p $(DESTDIR)$(pkgdatadir)$(PKG_LIB_DIR)
	cp -f $(DESTDIR)$(libdir)/lib@PACKAGE@*.la $(DESTDIR)$(pkgdatadir)$(LIB_DIR)
	cp -f $(DESTDIR)$(libdir)/libltdl.la $(DESTDIR)$(pkgdatadir)$(LIB_DIR)
	cp -f $(DESTDIR)$(pkglibdir)/*.la $(DESTDIR)$(pkgdatadir)$(PKG_LIB_DIR)
	cp -f $(DESTDIR)$(pkglibdir)/*.txt $(DESTDIR)$(pkgdatadir)$(PKG_LIB_DIR)
	$(SETCAP) $(DESTDIR)$(bindir)/$(TGTPFX)@PACKAGE@sh$(EXEEXT)
else
if WIN32
if HAVE_SDL
	@PACKAGE@sh_LDFLAGS += -mwindows
endif
endif
endif

EXTRA_@PACKAGE@sh_DEPENDENCIES = plugins

NME_DIR = $(top_builddir)$(PKG_LIB_DIR)
.PHONY: plugins
plugins:
	mkdir -p $(NME_DIR)
	cp -f $(top_srcdir)/tme/*.txt $(NME_DIR)
	if test -d $(top_srcdir)/machines; then \
		$(LN_S) $(top_srcdir)/machines $(top_builddir) ; \
	fi
#	find $(top_builddir) -type f -name '*.la' -exec cp '{}' $NME_DIR ';'
	for mod in $(@PACKAGE@_PREOPEN); do \
		if test $$mod != -dlopen; then \
			echo $$mod ; \
			cp -f $$mod $(NME_DIR) ; \
		fi \
	done

include $(top_srcdir)/nme-preopen.txt
