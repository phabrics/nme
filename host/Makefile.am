## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/:

AUTOMAKE_OPTIONS = 1.4 gnu
AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I. -D_TME_IMPL $(GTK_CFLAGS)

lib_LTLIBRARIES = libtme-host.la
libtme_host_la_SOURCES = eth-impl.c eth-if.h display-mouse.c display-keyboard.c display.c display.h
libtme_host_la_LDFLAGS = -version-info 0:0:0
libtme_host_la_LIBADD = $(top_builddir)/generic/libtme-generic.la $(GTK_LIBS)

EXTRA_DIST = keymap-make.pl keymap-def.h

display-keyboard.lo:  keymap.h

BUILT_SOURCES = keymap.h
CLEANFILES = keymap.h
if USING_X
keymap.h: keymap-make.pl
	  $(PERL) $< > $@
else
keymap.h: keymap-def.h
	  cp $< $@
endif

include $(top_srcdir)/modules

SUBDIRS = . @TME_HOSTS@
DIST_SUBDIRS = posix gtk bsd tun openvpn rfb
