## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator host/openvpn:

AUTOMAKE_OPTIONS = 1.4 gnu

include $(top_srcdir)/lib/ltrc.inc

AM_CPPFLAGS = -I$(top_srcdir) -I$(srcdir) -I$(srcdir)/../eth -I$(top_srcdir)/lib -I. -D_TME_IMPL

pkglib_LTLIBRARIES = tme_host_openvpn.la
tme_host_openvpn_la_SOURCES = \
	base64.c base64.h \
	basic.h \
	buffer.c buffer.h \
	circ_list.h \
	clinat.c clinat.h \
	common.h \
	crypto.c crypto.h crypto_backend.h \
	crypto_openssl.c crypto_openssl.h \
	crypto_polarssl.c crypto_polarssl.h \
	dhcp.c dhcp.h \
	errlevel.h \
	error.c error.h \
	event.c event.h \
	fdmisc.c fdmisc.h \
	forward.c forward.h forward-inline.h \
	fragment.c fragment.h \
	gremlin.c gremlin.h \
	helper.c helper.h \
	httpdigest.c httpdigest.h \
	lladdr.c lladdr.h \
	init.c init.h \
	integer.h \
	interval.c interval.h \
	list.c list.h \
	lzo.c lzo.h \
	manage.c manage.h \
	mbuf.c mbuf.h \
	memdbg.h \
	misc.c misc.h \
	platform.c platform.h \
	console.c console.h \
	mroute.c mroute.h \
	mss.c mss.h \
	mstats.c mstats.h \
	mtcp.c mtcp.h \
	mtu.c mtu.h \
	mudp.c mudp.h \
	multi.c multi.h \
	ntlm.c ntlm.h \
	occ.c occ.h occ-inline.h \
	pkcs11.c pkcs11.h pkcs11_backend.h \
	pkcs11_openssl.c \
	pkcs11_polarssl.c \
        openvpn.c openvpn.h \
	openvpn-link.c \
	openvpn-plugin.h \
	openvpn-server.c \
	openvpn-setup.c \
	openvpn-tap.c \
	options.c options.h \
	otime.c otime.h \
	packet_id.c packet_id.h \
	perf.c perf.h \
	pf.c pf.h pf-inline.h \
	ping.c ping.h ping-inline.h \
	plugin.c plugin.h \
	pool.c pool.h \
	proto.c proto.h \
	proxy.c proxy.h \
	ps.c ps.h \
	push.c push.h \
	pushlist.h \
	reliable.c reliable.h \
	route.c route.h \
	schedule.c schedule.h \
	session_id.c session_id.h \
	shaper.c shaper.h \
	sig.c sig.h \
	socket.c socket.h \
	socks.c socks.h \
	ssl.c ssl.h  ssl_backend.h \
	ssl_openssl.c ssl_openssl.h \
	ssl_polarssl.c ssl_polarssl.h \
	ssl_common.h \
	ssl_verify.c ssl_verify.h ssl_verify_backend.h \
	ssl_verify_openssl.c ssl_verify_openssl.h \
	ssl_verify_polarssl.c ssl_verify_polarssl.h \
	status.c status.h \
	syshead.h \
	tun.c tun.h \
	win32.h win32.c \
	cryptoapi.h cryptoapi.c

tme_host_openvpn_la_LIBADD = \
	$(top_builddir)/generic/libtme-generic.la \
	$(SOCKETS_LIBS) \
	$(OPTIONAL_LZO_LIBS) \
	$(OPTIONAL_PKCS11_HELPER_LIBS) \
	$(OPTIONAL_CRYPTO_LIBS) \
	$(OPTIONAL_SELINUX_LIBS) \
	$(OPTIONAL_SYSTEMD_LIBS) \
	$(OPTIONAL_DL_LIBS)

if WIN32
tme_host_openvpn_la_SOURCES += openvpn_win32_resources.rc
tme_host_openvpn_la_LIBADD += -lgdi32 -lws2_32 -lwininet -lcrypt32 -liphlpapi -lwinmm
endif

tme_host_openvpn_la_LDFLAGS = -module -version-info 0:0:0

include $(top_srcdir)/modules

openvpn-link.lo: ../eth/eth-impl.c
openvpn-server.lo: ../eth/eth-impl.c
openvpn-tap.lo: ../eth/eth-impl.c
