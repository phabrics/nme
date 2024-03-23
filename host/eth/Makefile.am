## Process this file with automake to produce Makefile.in
# Makefile.am for The Machine Emulator eth/:

AUTOMAKE_OPTIONS = 1.4 gnu

lib_LTLIBRARIES = libtme-eth.la
libtme_eth_la_SOURCES = eth-impl.c eth-if.h
libtme_eth_la_LDFLAGS = -version-info 0:0:0
libtme_eth_la_LIBADD = $(TME_LIBS)

include $(top_srcdir)/modules.mk

DIST_SUBDIRS=bsd openvpn tun
