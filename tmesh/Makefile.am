## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator tmesh/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL

if USING_OPENVPN
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
tmesh_LDADD = libtmesh.la $(LDADD)
tmesh_LDFLAGS = -dlpreopen force -dlopen self
include $(top_srcdir)/tme-preopen.txt

install-exec-hook:
	$(SETCAP) $(DESTDIR)$(bindir)/$(TGTPFX)tmesh$(EXEEXT)
