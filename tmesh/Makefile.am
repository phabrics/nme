## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator tmesh/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL @TME_HOST_CFLAGS@

if HAVE_OPENVPN
AM_CPPFLAGS += -DTME_OPENVPN
endif

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
if WIN32
if HAVE_SDL
tmesh_LDFLAGS += -mwindows
endif
else
install-exec-hook:
	$(SETCAP) $(DESTDIR)$(bindir)/$(TGTPFX)tmesh$(EXEEXT)
endif

include $(top_srcdir)/tme-hosts.txt
include $(top_srcdir)/tme-preopen.txt

SUBDIRS = @TME_HOST_DIRS@
