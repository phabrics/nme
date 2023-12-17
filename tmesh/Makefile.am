## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator tmesh/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL @TME_HOST_CFLAGS@

noinst_LTLIBRARIES = libtmesh.la
libtmesh_la_SOURCES = \
	tmesh-impl.h \
	tmesh-input.y \
	tmesh-util.c \
	tmesh-cmds.c \
	tmesh-threads.c
libtmesh_la_LIBADD = $(top_builddir)/libtme/libtme.la

bin_PROGRAMS = tmesh
tmesh_LDADD = libtmesh.la $(tme_preopen) @TME_HOST_LIBS@
tmesh_LDFLAGS = -dlpreopen force -dlopen self

EXTRA_DIST = tmesh.sh

if HAVE_EMSCRIPTEN
bin_SCRIPTS = tmesh.sh tmesh.js tmesh.wasm tmesh.data
tmesh_LDADD += $(SDL_LIBS) 
tmesh_LDFLAGS += -sASYNCIFY
if USE_NODEFS
AM_CPPFLAGS += -DUSE_NODEFS
tmesh_LDADD += -lnodefs.js
else
tmesh_LDFLAGS += --emrun --preload-file $(pkgdatadir)@$(ROOT_DIR)
endif
tmesh.js tmesh.wasm tmesh.data: tmesh.html
	@test -f $@ || $(LN_S) .libs/$@ .
else
if WIN32
if HAVE_SDL
	tmesh_LDFLAGS += -mwindows
endif
endif
endif

install-exec-hook:
#	test -f $(DESTDIR)$(bindir)/tmesh.sh && \
#	mv -f $(DESTDIR)$(bindir)/tmesh.sh $(DESTDIR)$(bindir)/tmesh
	mkdir -p  $(DESTDIR)$(pkgdatadir)$(TME_DIR)
	cp -f $(DESTDIR)$(libdir)/libtme*.la $(DESTDIR)$(pkgdatadir)$(TME_DIR)
	cp -f $(DESTDIR)$(libdir)/libltdl.la $(DESTDIR)$(pkgdatadir)$(TME_DIR)
	cp -fr $(DESTDIR)$(pkglibdir) $(DESTDIR)$(pkgdatadir)$(TME_DIR)
	$(SETCAP) $(DESTDIR)$(bindir)/$(TGTPFX)tmesh$(EXEEXT)

EXTRA_tmesh_DEPENDENCIES = $(top_builddir)/nme/tme
$(EXTRA_tmesh_DEPENDENCIES):
	mkdir -p $@
	cp -f $(top_srcdir)/tme/*.txt $@
	find $(top_builddir) -type f -name '*.la' -exec cp '{}' $@ ';'

include $(top_srcdir)/tme-hosts.txt
include $(top_srcdir)/tme-preopen.txt

SUBDIRS = @TME_HOST_DIRS@
