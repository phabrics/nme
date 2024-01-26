## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/openvpn:

AUTOMAKE_OPTIONS = 1.4 gnu

include $(top_srcdir)/lib/ltrc.inc

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/tme/libopenvpn -I. -D_TME_IMPL

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
	$(top_builddir)/lib/libcompat.la

if WIN32
libopenvpn_la_SOURCES += openvpn_win32_resources.rc
libopenvpn_la_LIBADD += -lgdi32 -lws2_32 -lwininet -lcrypt32 -liphlpapi -lwinmm
endif
