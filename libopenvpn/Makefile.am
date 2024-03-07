## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/openvpn:

AUTOMAKE_OPTIONS = 1.4 gnu
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/lib  -I$(top_srcdir)/$(LIBOBJDIR) -D_OPENVPN_IMPL

include $(top_srcdir)/lib/ltrc.inc

EXTRA_DIST = tap-windows.h

lib_LTLIBRARIES = libopenvpn.la
libopenvpn_la_SOURCES = \
	base64.c \
	buffer.c \
	clinat.c \
	crypto.c \
	crypto_openssl.c \
	crypto_polarssl.c \
	dhcp.c \
	error.c \
	event.c	\
	fdmisc.c \
	forward.c \
	fragment.c \
	gremlin.c \
	helper.c \
	httpdigest.c \
	lladdr.c \
	init.c \
	interval.c \
	link.c \
	list.c \
	lzo.c \
	manage.c \
	mbuf.c \
	misc.c \
	platform.c \
	console.c \
	mroute.c \
	mss.c \
	mstats.c \
	mtcp.c \
	mtu.c \
	mudp.c \
	multi.c \
	ntlm.c \
	occ.c \
	pkcs11.c \
	pkcs11_openssl.c \
	pkcs11_polarssl.c \
        openvpn.c \
	options.c \
	otime.c \
	packet_id.c \
	perf.c \
	pf.c \
	ping.c \
	plugin.c \
	pool.c \
	proto.c \
	proxy.c \
	ps.c \
	push.c \
	reliable.c \
	route.c \
	schedule.c \
	session_id.c \
	shaper.c \
	sig.c \
	socket.c \
	socks.c \
	ssl.c \
	ssl_openssl.c \
	ssl_polarssl.c \
	ssl_verify.c \
	ssl_verify_openssl.c \
	ssl_verify_polarssl.c \
	status.c \
	tun.c \
	win32.c \
	cryptoapi.c

libopenvpn_la_LIBADD = \
	$(SOCKETS_LIBS) \
	$(OPTIONAL_LZO_LIBS) \
	$(OPTIONAL_PKCS11_HELPER_LIBS) \
	$(OPTIONAL_CRYPTO_LIBS) \
	$(OPTIONAL_SELINUX_LIBS) \
	$(OPTIONAL_SYSTEMD_LIBS) \
	$(OPTIONAL_DL_LIBS) \
	$(LTLIBOBJS)
#	$(top_builddir)/lib/libcompat.la

if WIN32
include_HEADERS = tap-windows.h
libopenvpn_la_SOURCES += openvpn_win32_resources.rc
libopenvpn_la_LIBADD += -lgdi32 -lws2_32 -lwininet -lcrypt32 -liphlpapi -lwinmm
endif

pkginclude_HEADERS = \
	base64.h \
	basic.h \
	buffer.h \
	circ_list.h \
	clinat.h \
	common.h \
	console.h \
	cryptoapi.h \
	crypto.h crypto_backend.h \
	crypto_openssl.h \
	crypto_polarssl.h \
	dhcp.h \
	errlevel.h \
	error.h \
	event.h \
	fdmisc.h \
	forward.h forward-inline.h \
	fragment.h \
	gremlin.h \
	helper.h \
	httpdigest.h \
	init.h \
	integer.h \
	interval.h \
	lib/mman.h \
	link.h \
	list.h \
	lladdr.h \
	lzo.h \
	manage.h \
	mbuf.h \
	memdbg.h \
	misc.h \
	mroute.h \
	mss.h \
	mstats.h \
	mtcp.h \
	mtu.h \
	mudp.h \
	multi.h \
	ntlm.h \
	occ.h occ-inline.h \
	openvpn.h \
	openvpn-plugin.h \
	options.h \
	otime.h \
	packet_id.h \
	perf.h \
	pf.h pf-inline.h \
	ping.h ping-inline.h \
	pkcs11.h pkcs11_backend.h \
	platform.h \
	plugin.h \
	pool.h \
	proto.h \
	proxy.h \
	ps.h \
	push.h \
	pushlist.h \
	reliable.h \
	route.h \
	schedule.h \
	session_id.h \
	shaper.h \
	sig.h \
	socket.h \
	socks.h \
	ssl.h  ssl_backend.h \
	ssl_openssl.h \
	ssl_polarssl.h \
	ssl_common.h \
	ssl_verify.h ssl_verify_backend.h \
	ssl_verify_openssl.h \
	ssl_verify_polarssl.h \
	status.h \
	syshead.h \
	tun.h \
	win32.h

SUBDIRS = lib
