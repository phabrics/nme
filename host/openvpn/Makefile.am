## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/openvpn:

AUTOMAKE_OPTIONS = 1.4 gnu

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(srcdir)/../compat -I. -D_TME_IMPL

noinst_LTLIBRARIES = libtme-host-openvpn.la
libtme_host_openvpn_la_SOURCES = \
	base64.c \
	basic.h \
	buffer.c \
	common.h \
	console.c \
	crypto.h \
	error.c \
	event.c \
	fdmisc.c \
	integer.h \
	manage.c \
	memdbg.h \
	misc.c \
	mstats.h \
	mtu.c \
	options.c \
	otime.c \
	perf.c \
	platform.c \
	plugin.c \
	proto.c \
	ps.h \
	route.c \
	sig.c \
	socket.c \
	ssl.h \
	status.c \
	syshead.h \
	tun.c \
	win32.c

libtme_host_openvpn_la_LDFLAGS = -version-info 0:0:0
libtme_host_openvpn_la_LIBADD = $(top_builddir)/generic/libtme-generic.la \
	../compat/libcompat.la

include $(top_srcdir)/modules
$(pkglib_LTLIBRARIES): ../compat/libcompat.la
../compat/libcompat.la: force
	cd `dirname $@` && $(MAKE) `basename $@`
force: ;
