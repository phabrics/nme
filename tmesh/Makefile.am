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
tmesh_LDFLAGS += --emrun --preload-file $(NME_DIR)@$(ROOT_DIR)
EXTRA_tmesh_DEPENDENCIES = $(NME_DIR)$(TME_DIR)
$(NME_DIR)$(TME_DIR):
	mkdir -p $@
	cp -f $(top_srcdir)/tme/*.txt $@
	find $(top_builddir) -type f -name '*.la' -exec cp '{}' $@ ';'
endif
tmesh.js tmesh.wasm tmesh.data: tmesh.html
	@test -f $@ || $(LN_S) .libs/$@ .
install-exec-hook:
	mv -f $(DESTDIR)$(bindir)/tmesh.sh $(DESTDIR)$(bindir)/tmesh
else
if WIN32
if HAVE_SDL
	tmesh_LDFLAGS += -mwindows
endif
else
install-exec-hook:
	$(SETCAP) $(DESTDIR)$(bindir)/$(TGTPFX)tmesh$(EXEEXT)
endif
endif

include $(top_srcdir)/tme-hosts.txt
include $(top_srcdir)/tme-preopen.txt

SUBDIRS = @TME_HOST_DIRS@
