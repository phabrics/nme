## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator tmesh/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -D_TME_IMPL $(@PACKAGE@_HOST_CFLAGS)

lib_LTLIBRARIES = libtmesh.la
libtmesh_la_SOURCES = tmesh-impl.h \
	tmesh-input.y \
	tmesh-util.c \
	tmesh-cmds.c
libtmesh_la_LIBADD = $(top_builddir)/libtme/lib@PACKAGE@.la

tmesh_SOURCES = tmesh.c tmesh-threads.c
bin_PROGRAMS = tmesh
tmesh_LDADD = libtmesh.la $(@PACKAGE@_PREOPEN)
tmesh_LDFLAGS = 

EXTRA_DIST = tmesh.sh

if HAVE_EMSCRIPTEN
bin_SCRIPTS = tmesh.sh tmesh.js tmesh.wasm tmesh.data
tmesh_LDADD += $(SDL_LIBS) 
tmesh_LDFLAGS += -sASYNCIFY
if USE_NODEFS
AM_CPPFLAGS += -DUSE_NODEFS
tmesh_LDADD += -lnodefs.js
else
tmesh_LDFLAGS += --emrun --preload-file $(top_builddir)@$(ROOT_DIR)
endif
tmesh.js tmesh.wasm tmesh.data: tmesh.html
	@test -f $@ || $(LN_S) .libs/$@ .
install-exec-hook:
	test -f $(DESTDIR)$(bindir)/tmesh.sh && \
	mv -f $(DESTDIR)$(bindir)/tmesh.sh $(DESTDIR)$(bindir)/tmesh
	mkdir -p $(DESTDIR)$(pkgdatadir)$(PKG_LIB_DIR)
	cp -f $(DESTDIR)$(libdir)/lib@PACKAGE@*.la $(DESTDIR)$(pkgdatadir)$(LIB_DIR)
	cp -f $(DESTDIR)$(libdir)/libltdl.la $(DESTDIR)$(pkgdatadir)$(LIB_DIR)
	cp -f $(DESTDIR)$(pkglibdir)/*.la $(DESTDIR)$(pkgdatadir)$(PKG_LIB_DIR)
	cp -f $(DESTDIR)$(pkglibdir)/*.txt $(DESTDIR)$(pkgdatadir)$(PKG_LIB_DIR)
	$(SETCAP) $(DESTDIR)$(bindir)/$(TGTPFX)tmesh$(EXEEXT)
else
if WIN32
if HAVE_SDL
	tmesh_LDFLAGS += -mwindows
endif
endif
endif

EXTRA_tmesh_DEPENDENCIES = plugins

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
