## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator tmesh/:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I. $(GTK_CFLAGS) -D_TME_IMPL

lib_LTLIBRARIES = libtmesh.la
libtmesh_la_SOURCES = \
	tmesh-impl.h \
	tmesh-input.y \
	tmesh-util.c \
	tmesh-cmds.c
libtmesh_la_LDFLAGS = -version-info 0:0:0
#libtmesh_la_LIBADD = $(top_builddir)/libtme/libtme.la

bin_PROGRAMS = tmesh
tmesh_LDADD = $(top_builddir)/libtme/libtme.la libtmesh.la $(GTK_LIBS) -lX11
tmesh_LDFLAGS = -dlpreopen self @TME_PREOPEN@

install-exec-hook:
	$(SETCAP) $(DESTDIR)$(bindir)/tmesh$(EXEEXT)
