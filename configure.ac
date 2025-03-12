dnl Process this file with autoconf to produce a configure script.

dnl $Id: configure.ac,v 1.20 2010/06/05 19:56:37 fredette Exp $

dnl Copyright (c) 2001, 2003 Matt Fredette
dnl Copyright (c) 2013-2024 Ruben Agin
dnl All rights reserved.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl 3. All advertising materials mentioning features or use of this software
dnl    must display the following acknowledgement:
dnl      This product includes software developed by Matt Fredette.
dnl 4. The name of the author may not be used to endorse or promote products
dnl    derived from this software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
dnl IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
dnl DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
dnl INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
dnl (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
dnl SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
dnl HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
dnl STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
dnl ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.

dnl Checks that we are given a good source directory.
AC_PREREQ([2.69])
AC_INIT([tme],[0.12rc14])
AC_CONFIG_SRCDIR([ic/m68k/m68k-impl.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_CANONICAL_TARGET
#AC_CONFIG_AUX_DIR([libltdl/config])
#AC_CONFIG_MACRO_DIR([libltdl/m4])
AM_INIT_AUTOMAKE([subdir-objects])
AM_EXTRA_RECURSIVE_TARGETS([modules])
AC_PROG_CC
AM_PROG_CC_C_O
AM_SILENT_RULES([yes])
AC_PREFIX_PROGRAM([tmesh])

dnl Write configuration out to config.h through config.h.in.
AC_CONFIG_HEADERS([config.h])

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_EGREP
AC_PROG_LN_S

TME_HOST_TYPES="gtk rfb sdl tun bsd openvpn"
AC_SUBST(TME_HOST_TYPES)

AC_ARG_ENABLE(hosts,
[  --enable-hosts=hosttypes         enable hosts (default="$TME_HOST_TYPES")],
   [tme_enable_hosts=${enableval}], [enable_hosts=yes])

if test "x${enable_hosts}" = xyes; then
   tme_enable_hosts=$TME_HOST_TYPES
fi

for tme_host_type in ${tme_enable_hosts}; do
    eval enable_${tme_host_type}=yes
done

AC_CHECK_DEFINE(__EMSCRIPTEN__, [
				CC=$(basename -s .bat $CC)
				host=$(echo "$host" | sed "s/$host_os/emscripten/")
				host_os=emscripten
				],
				[host_type=$host_os])
AM_CONDITIONAL([HAVE_EMSCRIPTEN], [test "${host_os}" = emscripten])
AM_CONDITIONAL([USE_NODEFS], [test "${enable_nodefs}" = yes])

AC_CHECK_PROG([CYGPATH_M], [cygpath], [cygpath -m], [echo])
AC_CHECK_PROG([CYGPATH_U], [cygpath], [cygpath -u], [echo])

if test "${host_os}" = emscripten; then
   ROOT_DIR="\$(abs_top_builddir)" # /
   EXEEXT=.html
else
   ROOT_DIR=$(${CYGPATH_M} /)
fi
TME_DIR="\`\${CYGPATH_U} \${libdir}\`"
AC_SUBST(TME_DIR)
AC_SUBST(ROOT_DIR)

dnl Configure for libtool.
LT_PREREQ([2.4])
LT_CONFIG_LTDL_DIR([libltdl])
LT_INIT([dlopen,win32-dll])
LT_LANG([Windows Resource])
LTDL_INIT([installable recursive])
CPPFLAGS="${CPPFLAGS-} ${LTDLINCL}" # -DLT_DEBUG_LOADERS"
lto=no
case "${host_os}" in
netbsd*) 
   CPPFLAGS="${CPPFLAGS-} -D_NETBSD_SOURCE"
   DEV_TAP_FILENAME='"\"/dev/tap\""'
   SETCAP="chmod u+s"
   ;;
openbsd*) 
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/tun\""'
   SETCAP="chmod u+s"
   ;;
freebsd*) 
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/tap\""'
   SETCAP="chmod u+s"
   ;;
dragonfly*) 
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/tap\""'
   SETCAP="chmod u+s"
   ;;
linux*) 
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/net/tun\""'
   SETCAP="chmod u+s"
   lto=yes
   LDFLAGS="-Wl,--copy-dt-needed-entries"
#   SETCAP="setcap cap_sys_nice,cap_net_admin,cap_net_raw+ep"
   ;;
mingw* | msys*)
   AC_DEFINE([TME_MODULE_PATH_VAR], ["PKG_CONFIG_SYSTEM_LIBRARY_PATH"], [Use pkgconf environment variable to determine lib path in MSYS2.])
   CPPFLAGS="${CPPFLAGS} -DWIN32_LEAN_AND_MEAN"
   CPPFLAGS="${CPPFLAGS} -DNTDDI_VERSION=NTDDI_WINXP -D_WIN32_WINNT=_WIN32_WINNT_WINXP"
   WIN32=yes
   SETCAP="chmod u+s"
   host_type=
   if test "x${enable_shared}" = "xno" || test "x${host_cpu}" = "xi686"; then
      lto=${GCC}
   fi
   ;;
cygwin*)   
   WIN32=cygwin
   SETCAP="chmod u+s"
   host_type=
   ;;
solaris*)
   SETCAP="chmod u+s"
   ;;
darwin*)
   dnl some Mac OS X tendering (we use vararg macros...)
   CPPFLAGS="$CPPFLAGS -no-cpp-precomp"
   SETCAP="chmod u+s"
   ;;
*)
   CPPFLAGS="${CPPFLAGS-}"
   SETCAP="chmod u+s"
   host_type=
   ;;
esac

AC_MSG_CHECKING([for link-time optimization (LTO)])

AC_ARG_ENABLE(lto,
[  --enable-lto      compile with link-time optimizations (default=supported)],
[ ],[ enable_lto=$lto ])

AC_MSG_RESULT($enable_lto)

if test "x${enable_lto}" = "xyes"; then
  CFLAGS="${CFLAGS-} -flto"
  LDFLAGS="${LDFLAGS-} -flto"
fi

AM_CONDITIONAL([WIN32], [test "${WIN32}" = "yes"])

AC_SUBST(DEV_TAP_FILENAME)
AC_SUBST(SETCAP)

AC_ARG_VAR([TAP_CFLAGS], [C compiler flags for tap])
old_CFLAGS="${CFLAGS}"
CFLAGS="${CFLAGS} ${TAP_CFLAGS}"
AC_CHECK_HEADERS(
	[ \
		net/if_tun.h net/tun/if_tun.h \
		linux/if_tun.h \
		tap-windows.h \
	],
	[have_tap_header="yes"]
)
AC_CHECK_DECLS(
	[TUNSETPERSIST],
	[AC_DEFINE([ENABLE_FEATURE_TUN_PERSIST], [1], [We have persist tun capability])],
	,
	[[
		#ifdef HAVE_LINUX_IF_TUN_H
		#include <linux/if_tun.h>
		#endif
	]]
)
CFLAGS="${old_CFLAGS}"

dnl Checks for LIBOPENVPN.
PKG_CHECK_MODULES([LIBOPENVPN], [libopenvpn], [AC_DEFINE([HAVE_LIBOPENVPN], [], [Defined if LIBOPENVPN is found.])], [have_libopenvpn=no])
AM_CONDITIONAL([HAVE_LIBOPENVPN], [test "x${have_libopenvpn}" != xno])

dnl Checks for GLIB.
PKG_CHECK_MODULES([GLIB], [glib-2.0], [AC_DEFINE([HAVE_GLIB], [], [Defined if GLIB 2.0 is found.])], [have_glib=no])
AM_CONDITIONAL([HAVE_GLIB], [test "x${have_glib}" != xno])

dnl Checks for display type support
PKG_CHECK_MODULES([GTK], [gtk4], [have_gtk=$enable_gtk], [
   PKG_CHECK_MODULES([GTK], [gtk+-3.0], [have_gtk=$enable_gtk], [have_gtk=no])
])
if test "x${have_gtk}" = xyes; then
   AC_DEFINE([HAVE_GTK], [], [Defined if GTK is found & enabled.])
fi
AM_CONDITIONAL([HAVE_GTK], [test "x${have_gtk}" = xyes])

PKG_CHECK_MODULES([SDL], [sdl2], [have_sdl=$enable_sdl], [have_sdl=no])

AX_CHECK_COMPILE_FLAG(-sUSE_SDL=2, [have_sdl=$enable_sdl])
AX_APPEND_COMPILE_FLAGS(-sUSE_SDL=2, SDL_CFLAGS)
AX_APPEND_LINK_FLAGS(-sUSE_SDL=2, SDL_LIBS)

AM_CONDITIONAL([HAVE_SDL], [test "x${have_sdl}" = xyes])
PKG_CHECK_MODULES([RFB], [libvncclient libvncserver], [have_rfb=$enable_rfb], [have_rfb=no])

if test "x${have_tap_header}" = xyes || test "x${WIN32}" != x; then
   have_openvpn=$enable_openvpn
fi

if test "x${have_openvpn}" = xyes; then
   AC_DEFINE([HAVE_OPENVPN], [], [Defined if openvpn is found.])
fi

AC_CHECK_HEADERS([linux/if_tun.h net/if_tap.h net/tap/if_tap.h net/if_tun.h],
		 [have_tun=$enable_tun])

AC_CHECK_HEADERS([linux/filter.h net/bpf.h])

AC_EGREP_CPP(found_BPFVERSION,
[
#ifdef HAVE_LINUX_FILTER_H
#include <linux/filter.h>
#elif HAVE_NET_BPF_H
#include <net/bpf.h>
#endif
#if BPF_MAJOR_VERSION == 1 && BPF_MINOR_VERSION >= 1
found_BPFVERSION
#endif
],
[have_bsd=$enable_bsd])

for tme_host in $TME_HOST_TYPES; do
    if eval test "x\$have_${tme_host}" = xyes; then
       TME_HOST=`echo ${tme_host} | tr '[a-z]' '[A-Z]'`
       TME_HOST_CFLAGS="\$(${TME_HOST}_CFLAGS) ${TME_HOST_CFLAGS}"
       TME_HOST_LIBS="-dlopen \$(tme_host_${tme_host})/tme_host_${tme_host}.la ${TME_HOST_LIBS}"
       TME_HOST_DIRS="\$(tme_host_${tme_host}) ${TME_HOST_DIRS}"
    fi
done

dnl Checks for host support.
AC_MSG_CHECKING([for enabled host types])

dnl The list of ethernet host support is finished.
AC_SUBST(TME_HOST_CFLAGS)
AC_SUBST(TME_HOST_LIBS)
AC_SUBST(TME_HOST_DIRS)

AC_MSG_RESULT($TME_HOST_DIRS)

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [main])
# Libraries required for network address translation
NATLIBS=
OLDLIBS=$LIBS
AC_SEARCH_LIBS([mnl_socket_open], [mnl])
if test "x$ac_cv_search_mnl_socket_open" != "xnone required" && test "$ac_cv_search_mnl_socket_open" != xno; then
   NATLIBS="$ac_cv_search_mnl_socket_open ${NATLIBS}"
fi

AC_SEARCH_LIBS([nft_rule_alloc], [nftnl])
if test "x$ac_cv_search_nft_rule_alloc" != "xnone required" && test "x$ac_cv_search_nft_rule_alloc" != xno; then
   NATLIBS="$ac_cv_search_nft_rule_alloc ${NATLIBS}"
fi

AC_SEARCH_LIBS([npf_nat_create], [npf])
if test "x$ac_cv_search_npf_nat_create" != "xnone required" && test "x$ac_cv_search_npf_nat_create" != xno; then
   NATLIBS="$ac_cv_search_npf_nat_create ${NATLIBS}"
fi

AC_SEARCH_LIBS([bpf_dump], [pcap])
if test "x$ac_cv_search_bpf_dump" != "xnone required" && test "x$ac_cv_search_bpf_dump" != xno; then
   NATLIBS="$ac_cv_search_bpf_dump ${NATLIBS}"
fi
LIBS=$OLDLIBS
AC_SUBST(NATLIBS)

dnl Checks for header files.
AC_CHECK_HEADERS([unistd.h stdio.h memory.h stdarg.h stdbool.h limits.h float.h curses.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/sockio.h sys/socketio.h net/if_dl.h ioctls.h netpacket/packet.h ifaddrs.h])
AC_CHECK_HEADERS([byteswap.h sys/endian.h])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h poll.h sys/socket.h sys/select.h sys/time.h time.h errno.h sys/mman.h sys/uio.h])
AC_CHECK_HEADERS([linux/netfilter/nf_tables.h libmnl/libmnl.h libnftnl/table.h libnftnl/chain.h libnftnl/rule.h libnftnl/expr.h])
AC_CHECK_HEADERS([pthread_np.h sched.h])
AC_CHECK_HEADERS([X11/keysym.h rfb/keysym.h])
AC_CHECK_HEADERS([sys/param.h sys/linker.h sys/sysctl.h sys/module.h npf.h net/npf_ncode.h pcap/pcap.h], [], [],
[#ifdef HAVE_SYS_PARAM_H
 # include <sys/param.h>
 #endif
])

AC_CHECK_HEADERS([netinet/in_systm.h netinet/tcp.h windows.h winsock2.h ws2tcpip.h])

AC_CHECK_HEADERS([net/if.h net/if_var.h net/if_types.h net/if_arp.h netinet/in.h netinet/in_var.h netinet/if_ether.h linux/netfilter.h net/pfvar.h net/pf/pfvar.h netinet/ip_compat.h netinet/ip_fil.h netinet/ip_nat.h netinet/ip_proxy.h], [], [],
[#include <stdio.h>
 #ifdef STDC_HEADERS
 # include <stdlib.h>
 # include <stddef.h>
 #else
 # ifdef HAVE_STDLIB_H
 #  include <stdlib.h>
 # endif
 #endif
 #ifdef HAVE_SYS_SOCKET_H
 # include <sys/socket.h>
 #endif
 #ifdef HAVE_NET_IF_H
 # include <net/if.h>
 #endif
 #ifdef HAVE_NET_IF_VAR_H
 # include <net/if_var.h>
 #endif
 #ifdef HAVE_NET_IF_TYPES_H
 # include <net/if_types.h>
 #endif
 #ifdef HAVE_NET_IF_ARP_H
 # include <net/if_arp.h>
 #endif
 #ifdef HAVE_NETINET_IN_H
 # include <netinet/in.h>
 #endif
 #ifdef HAVE_NETINET_IN_VAR_H
 # include <netinet/in_var.h>
 #endif
 #ifdef HAVE_NETINET_IP_COMPAT_H
 # include <netinet/ip_compat.h>
 #endif
 #ifdef HAVE_NETINET_IP_FIL_H
 # include <netinet/ip_fil.h>
 #endif
 #ifdef HAVE_NETINET_IP_NAT_H
 # include <netinet/ip_nat.h>
 #endif
])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_LONG_DOUBLE_WIDER
ac_cv_c_long_double=$ac_cv_type_long_double_wider
if test $ac_cv_c_long_double = yes; then
  AC_DEFINE([HAVE_LONG_DOUBLE],[1],[Define to 1 if the type `long double' works and has more range or
	 precision than `double'.])
fi
AC_TYPE_LONG_LONG_INT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINTPTR_T
AX_CPP_VARARG_MACRO_ISO
AX_CPP_VARARG_MACRO_GCC
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
if test $ac_cv_c_long_double = yes; then
  AC_CHECK_SIZEOF(long double)
fi
if test $ac_cv_type_long_long_int = yes; then
  AC_CHECK_SIZEOF(long long int)
else
  ac_cv_sizeof_long_long_int=$ac_cv_sizeof_long  
fi
if test $ac_cv_sizeof_int != 4 && test $ac_cv_sizeof_long != 4; then
  AC_MSG_ERROR([can't find a 32-bit type])
fi
if test $ac_cv_sizeof_int != 2 && test $ac_cv_sizeof_short != 2; then
  AC_MSG_ERROR([can't find a 16-bit type])
fi
if test $ac_cv_sizeof_long_long_int != 8 && test $ac_cv_sizeof_long != 8; then
  AC_MSG_WARN([can't find a 64-bit type])
  have_int64=no
else
  AC_CHECK_ALIGNOF(64)
  AC_CHECK_SHIFTMAX(64)
  AC_CHECK_SHIFTSIGNED(64)
  have_int64=yes
fi
AC_CHECK_TYPES([__int128], [
  have_int128=yes
], [ have_int128=no ])
  
AC_CHECK_ALIGNOF(32)
AC_CHECK_ALIGNOF(16)
AC_CHECK_SHIFTMAX(8)
AC_CHECK_SHIFTMAX(16)
AC_CHECK_SHIFTMAX(32)
AC_CHECK_SHIFTSIGNED(8)
AC_CHECK_SHIFTSIGNED(16)
AC_CHECK_SHIFTSIGNED(32)
if test "x$cross_compiling" = xyes; then
   case ${host} in
   	i686-w64-mingw32)
	   ac_cv_float_format_float=IEEE754_SINGLE
	   ac_cv_float_format_double=IEEE754_DOUBLE
	   ac_cv_float_format_long_double=IEEE754_EXTENDED80_I387
	   ;;
   	x86_64-w64-mingw32)
	   ac_cv_float_format_float=IEEE754_SINGLE
	   ac_cv_float_format_double=IEEE754_DOUBLE
	   ac_cv_float_format_long_double=NATIVE
	   ;;
   esac
fi
AC_CHECK_FLOAT_FORMAT(float)
AC_CHECK_FLOAT_LIMITS(float, FLT_MAX/FLT_MIN)
AC_CHECK_FLOAT_FORMAT(double)
AC_CHECK_FLOAT_LIMITS(double, DBL_MAX/DBL_MIN)
if test $ac_cv_c_long_double = yes; then
  AC_CHECK_FLOAT_FORMAT(long double)
  AC_CHECK_FLOAT_LIMITS(long double, LDBL_MAX/LDBL_MIN)
fi

AC_SYS_LARGEFILE

AC_CHECK_MEMBERS([struct stat.st_rdev, struct stat.st_blksize])

AC_CHECK_TYPES([cpuset_t, cpu_set_t],[],[],[
#define _NETBSD_SOURCE
#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
])

AC_CHECK_FUNC_LONG(cpuset_create, [
  cpuset_t *c;

  c = cpuset_create();
], [ 
#define _NETBSD_SOURCE
#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
])

AC_SYS_SOCKADDR_SA_LEN
AX_TYPE_SOCKLEN_T
AX_EMPTY_ARRAY

dnl Checks for library functions and prototypes.
#AC_FUNC_MEMCMP
#AC_FUNC_MALLOC
#AC_FUNC_MMAP
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([gmtime_r gmtime_s gmtime clock_gettime memmove memset munmap pow ptsname ptsname_r sqrt strchr strerror strrchr strstr strtoul devname fdevname kldfind isinff])

AC_CHECK_FUNC_LONG(__builtin_bswap16, [
  unsigned int x;

  x = __builtin_bswap16(0x1122);
])
AC_CHECK_FUNC_LONG(__builtin_bswap32, [
  unsigned int x;

  x = __builtin_bswap32(0x11223344);
])
if test $have_int64 = yes; then
  AC_CHECK_FUNC_LONG(__builtin_bswap64, [
  unsigned long long int x;

  x = __builtin_bswap64(0x1122334455667788);
])
fi

AC_CHECK_FUNC_LONG(bswap16, [
  unsigned int x;

  x = bswap16(0x1122);
], [ 
#include <sys/types.h>
#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif
])
AC_CHECK_FUNC_LONG(bswap32, [
  unsigned int x;

  x = bswap32(0x11223344);
], [ 
#include <sys/types.h>
#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif
])
if test $have_int64 = yes; then
  AC_CHECK_FUNC_LONG(bswap64, [
  unsigned long long int x;

  x = bswap64(0x1122334455667788);
], [ 
#include <sys/types.h>
#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif
  ])
fi

AC_CHECK_FUNC_LONG(bswap_16, [
  unsigned int x;

  x = bswap_16(0x1122);
], [ 
#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif
])
AC_CHECK_FUNC_LONG(bswap_32, [
  unsigned int x;

  x = bswap_32(0x11223344);
], [ 
#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif
])
if test $have_int64 = yes; then
  AC_CHECK_FUNC_LONG(bswap_64, [
  unsigned long long int x;

  x = bswap_64(0x1122334455667788);
], [ 
#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif
  ])
fi

AC_CHECK_FUNC_LONG(swap16, [
  unsigned int x;

  x = swap16(0x1122);
], [ 
#include <sys/types.h>
])
AC_CHECK_FUNC_LONG(swap32, [
  unsigned int x;

  x = swap32(0x11223344);
], [ 
#include <sys/types.h>
])
if test $have_int64 = yes; then
  AC_CHECK_FUNC_LONG(swap64, [
  unsigned long long int x;

  x = swap64(0x1122334455667788);
], [ 
#include <sys/types.h>
  ])
fi

dnl Checks for various library functions
AC_SEARCH_LIBS([pthread_mutex_timedlock], [pthread], [
AC_DEFINE(HAVE_PTHREAD_MUTEX_TIMEDLOCK, [], [Define if you have pthread_mutex_timedlock.])
])

AC_SEARCH_LIBS([pthread_rwlock_timedrdlock], [pthread], [
AC_DEFINE(HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK, [], [Define if you have pthread_rwlock_timedlock.])
])

AC_SEARCH_LIBS([pthread_setaffinity_np], [pthread], [
AC_DEFINE(HAVE_PTHREAD_SETAFFINITY_NP, [], [Define if you have pthread_setaffinity_np.])
])

AC_SEARCH_LIBS([pthread_setschedparam], [pthread], [
AC_DEFINE(HAVE_PTHREAD_SETSCHEDPARAM, [], [Define if you have pthread_setschedparam.])
])

dnl Invoke compatibility library in LIBOBJDIR
AC_CHECK_FUNCS([basename daemon dirname gettimeofday inet_ntop inet_pton])
AC_CHECK_FUNCS([mmap munmap mprotect msync mlock munlock])

dnl Checks for perl.
AC_PATH_PROGS(PERL, perl4.036 perl4 perl perl5, no)
AC_SUBST(PERL)

dnl Checks for AF_LINK.
AC_MSG_CHECKING([for AF_LINK support])
AC_EGREP_CPP(_tme_has_af_link,
[
#include <sys/socket.h>
#ifdef AF_LINK
_tme_has_af_link
#endif
], [ 
AC_MSG_RESULT(yes)
AC_DEFINE(HAVE_AF_LINK, [], [Define if you have AF_LINK.])
], [
AC_MSG_RESULT(no)
])

dnl Checks for AF_PACKET.
AC_MSG_CHECKING([for AF_PACKET support])
AC_EGREP_CPP(_tme_has_af_packet,
[
#include <sys/socket.h>
#ifdef AF_PACKET
_tme_has_af_packet
#endif
], [ 
AC_MSG_RESULT(yes)
AC_DEFINE(HAVE_AF_PACKET, [], [Define if you have AF_PACKET.])
], [
AC_MSG_RESULT(no)
])

dnl Start the list of display host support.
TME_FB_XLAT_DST="d32b32s0p32olml_r0xffff0000_g0xff00ff00_b0xff0000ff d24b32s0p32olml_r0xff0000_g0xff00_b0xff"

# if --with-tme-host-displays is given, characterize those given displays:
AC_ARG_WITH(tme-host-displays,
[  --with-tme-host-displays=DISPLAYS  support host displays with these display formats],
[ TME_FB_XLAT_DST=$withval $TME_FB_XLAT_DST], [])

AC_SUBST(TME_FB_XLAT_DST)

AC_SYS_POSIX_TERMIOS

AM_CONDITIONAL([USING_TERMIOS], [test "x${ac_cv_sys_posix_termios}" = xyes])

AC_MSG_CHECKING([for available network address translation methods])

# specify whether built-in NAT support should be enabled & with what NAT methods
AC_ARG_ENABLE(nat,
[  --enable-nat=NATs  Enable built-in NAT support utilizing the given NAT methods (compiled-in order of usage by platform availability: 'nft','npf','pf','ipf').  Defaults to 'yes', any available.  Turn off NAT with 'no' or --disable-nat.], [tme_nat_types=${enableval}], [enable_nat=no])

if test "x${enable_nat}" = xyes; then
   tme_nat_types="nft npf pf"
fi

if test "x${enable_nat}" = xno; then
   tme_nat_types=
fi

NATINC=

for tme_nat_type in ${tme_nat_types}; do
case "x${tme_nat_type}" in
xnft)   NATINC="${NATINC} -DTME_NAT_NFT" ;;
xnpf)   NATINC="${NATINC} -DTME_NAT_NPF" ;;
xpf)    NATINC="${NATINC} -DTME_NAT_PFV" ;;
xipf)   NATINC="${NATINC} -DTME_NAT_IPF" ;;
esac
done

AC_MSG_RESULT($tme_nat_types)

AC_SUBST(NATINC)

AC_MSG_CHECKING([for available threading models])

# specify whether preemptive thread support should be enabled & with what thread models

AC_ARG_ENABLE(threads,
[  --enable-threads=THRs  Enable preemptive thread support utilizing the given threading models (compiled-in order of usage by platform availability: 'pthreads (or posix)','glib').  Defaults to 'no'; this will use a singled thread with setjmp/longjmp or fibers on platforms that support them.  Turn on threads with --enable-threads=<thr>; no value given is 'yes' which defaults to best threading model for platform.], [tme_thread_types=${enableval}], [enable_threads=no])

if test "x${enable_threads}" = xyes; then
   dnl Make brackets safe to use.
   changequote(<<, >>)dnl

   dnl set the default threads based on platform
   case "${host_os}" in
   mingw* | msys* | cygwin*) 
	tme_thread_types=glib
   	;;
   *) 
	tme_thread_types=pthreads
   	;;
   esac

   dnl Make brackets the quote characters again.
   changequote([, ])dnl
   recode=no
fi

if test "x${enable_threads}" = xno; then
   dnl Make brackets safe to use.
   changequote(<<, >>)dnl

   dnl set the default threads based on platform
   case "${host_os}" in
   emscripten)
	recode=no
   	;;
   *) 
   	recode=i686
   	;;
   esac

   dnl Make brackets the quote characters again.
   changequote([, ])dnl
   tme_thread_types=fiber
fi

for tme_thread_type in ${tme_thread_types}; do
case "x${tme_thread_type}" in
xpthreads| xposix)  CPPFLAGS="${CPPFLAGS-} -DTME_THREADS_POSIX"
	  	    CFLAGS="${CFLAGS-} -pthread"
		    LDFLAGS="${LDFLAGS-} -pthread"
		     ;;
xglib)   CPPFLAGS="${CPPFLAGS-} -DTME_THREADS_GLIB -DUSE_GLIB_TIME ${GLIB_CFLAGS}"
	 LDFLAGS="${LDFLAGS-} ${GLIB_LIBS}"
	 ;;
xfiber)   CPPFLAGS="${CPPFLAGS-} -DTME_THREADS_FIBER"
	 ;;
xgiber)   CPPFLAGS="${CPPFLAGS-} -DTME_THREADS_FIBER -DUSE_GLIB_TIME ${GLIB_CFLAGS}"
	 LDFLAGS="${LDFLAGS-} ${GLIB_LIBS}"
	  tme_thread_types=fiber
	 ;;
esac
done

CPPFLAGS="${CPPFLAGS-} -D_GNU_SOURCE" 

AM_CONDITIONAL([THREADS_POSIX], [test "x${tme_thread_types}" = xpthreads || test "x${tme_thread_types}" = xposix])
AM_CONDITIONAL([THREADS_GLIB], [test "x${tme_thread_type}" = xglib])
AM_CONDITIONAL([THREADS_FIBER], [test "x${tme_thread_types}" = xfiber])
AC_MSG_RESULT($tme_thread_types)

dnl Checks for ZLIB.
AS_IF([test "x${WIN32}" != xyes], [PKG_CHECK_MODULES([ZLIB], [zlib], [have_zlib=yes], [have_zlib=no])])

if test "x${have_zlib}" = xyes; then
   AC_DEFINE([HAVE_ZLIB], [], [Defined if ZLIB is found and can be used.])
fi
AM_CONDITIONAL([HAVE_ZLIB], [test "x${have_zlib}" = xyes])

dnl Configures for the system(s) to emulate.
bus_size_max=64
recode_size_guest_max=64

TME_FB_XLAT_SRC="1152x900d1b1s0p32om 1152x900d8b8s0p32ommi8 1152x900d1b1s0p32omccmi8"
AC_SUBST(TME_FB_XLAT_SRC)

dnl Checks for X11.
AC_PATH_XTRA
AX_PERL_MODULE_VERSION([X11::Protocol 0.56],[],[no_x=yes])
AM_CONDITIONAL([USING_X], [test "x${no_x}" = x])

dnl Configure recode.
dnl Make brackets safe to use.
changequote(<<, >>)dnl

dnl Make brackets the quote characters again.
changequote([, ])dnl

AC_ARG_ENABLE(recode,
[  --enable-recode         enable recode (dynamic binary translation) (default=if host supports)],
[ ], [enable_recode=$recode])

AC_MSG_CHECKING([for host recode support])

dnl Make brackets safe to use.
changequote(<<, >>)dnl

# get the host maximum guest size:
case "${host_cpu}" in
i[3456789]86)
     recode_hosts="mmap x86"
     recode_size_guest_max_host=64
     ;;
x86_64)
     recode_hosts="mmap x86"
     recode_size_guest_max_host=128
     ;;
*) 
     recode_hosts=
     recode_size_guest_max_host=1 ;;
esac

dnl Make brackets the quote characters again.
changequote([, ])dnl

keep_recode=$enable_recode

for tme_recode_type in ${enable_recode}; do
    if test "x${host_cpu}" = "x${tme_recode_type}"; then
       keep_recode=yes
       break
    fi
done

if test "x${keep_recode}" != xyes; then
   recode_hosts=
   recode_size_guest_max=1
fi

if test "x${recode_hosts}" = x; then
   AC_MSG_RESULT([no])
   if test "x${enable_recode}" = xrequired; then
      AC_MSG_ERROR([recode required but not supported on ${host}])
   fi
else
   if test "x${recode_size_guest_max_host}" = x; then
      AC_MSG_ERROR([internal error - no maximum recode guest size for ${host_cpu}])
   fi
   if test `expr ${recode_size_guest_max} \< ${recode_size_guest_max_host}` = 1; then
      recode_size_guest_max=$recode_size_guest_max_host
   fi
   AC_MSG_RESULT([up to ${recode_size_guest_max}-bit guests using ${recode_hosts}])

   # get the host maximum guest size:
   changequote(<<, >>)dnl
   case "${host_cpu}" in
   i[3456789]86)
     bus_size_max_host=32
     ;;
   x86_64)
     bus_size_max_host=64
     ;;
   *) 
     bus_size_max_host=
     ;;
   esac
   changequote([, ])dnl

   dnl Configure miscellaneous host support.
   AC_MSG_CHECKING([for host recode bus size])

   if test `expr ${bus_size_max} \< ${bus_size_max_host}` = 1; then
      bus_size_max=$bus_size_max_host
   fi
   AC_MSG_RESULT([using ${bus_size_max}-bit bus for guests using ${host_cpu}])
fi

dnl Configure miscellaneous host support.
AC_MSG_CHECKING([for host miscellaneous support])
changequote(<<, >>)dnl
case "${host_cpu}" in
i[3456789]86 | x86_64) misc_hosts="x86" ;;
*) misc_hosts= ;;
esac
changequote([, ])dnl

if test "${host_os}" = emscripten; then
    misc_hosts=
fi
AC_MSG_RESULT([${misc_hosts}])

if test "x${misc_hosts}" = "xx86"; then
  AC_CHECK_FUNC_LONG(__rdtsc, [
  unsigned long long int x;

  // _mm_lfence();  // optionally wait for earlier insns to retire before reading the clock
  x = __rdtsc();
  // _mm_lfence();  // optionally block later instructions until rdtsc retires
],[
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
])
fi

dnl Configure debugging and/or warnings.
AC_MSG_CHECKING([for gcc --version >= 4.8])
AC_EGREP_CPP(_gcc_has_Og,
[
#if (__GNUC__ > 3 && __GNUC_MINOR__ > 7) || __GNUC__ > 4
_gcc_has_Og
#endif
], [
AC_MSG_RESULT(yes)
have_Og=yes
], [ AC_MSG_RESULT(no) ])

AC_MSG_CHECKING([for clang])
AC_EGREP_CPP(_gcc_is_clang,
[
#if __clang__
_gcc_is_clang
#endif
], [
AC_MSG_RESULT(yes)
have_clang=yes
], [ AC_MSG_RESULT(no) ])

AC_ARG_ENABLE(debug,
[  --enable-debug          compile debuggable libraries and programs (default=don't)],
[ ], [enable_debug=no])
if test "x${enable_debug}" = "xyes"; then
  CPPFLAGS="${CPPFLAGS-} -DENABLE_DEBUG -DTME_NO_DEBUG_LOCKS -D_TME_SPARC_STATS -DGTK_DISABLE_SINGLE_INCLUDES -DNDEBUG" # -DTME_RECODE_DEBUG
  if test "x${have_Og}" = xyes; then
     CFLAGS="${CFLAGS-} -Og"
     CXXFLAGS="${CXXFLAGS-} -g3 -Og"
  fi			   
else
  CPPFLAGS="${CPPFLAGS-} -DTME_NO_LOG -DTME_NO_DEBUG_LOCKS -DTME_NO_AUDIT_ATOMICS -DNDEBUG"
fi

AC_ARG_ENABLE(warnings,
[  --disable-warnings      don't compile with warnings turned on (default=don't)],
[ ], [enable_warnings=no])
if test "x${enable_warnings}" = "xyes" -a "x${GCC}" = "xyes"; then
  CFLAGS="${CFLAGS-} -Wno-unused -Wno-sign-compare -Wundef -Wall -Wextra"
  CXXFLAGS="${CXXFLAGS-} -W"
else
  CFLAGS="${CFLAGS-} -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable"
  if test "x${have_clang}" = xyes; then
    CFLAGS="${CFLAGS-} -Wno-tautological-compare -Wno-sometimes-uninitialized \
    		       -Wno-sign-compare -Wno-incompatible-pointer-types \
		       -Wno-implicit-function-declaration -Wno-int-conversion \
		       -Wno-cpp -Wno-compound-token-split-by-macro -fbracket-depth=1024"
  fi
  if test "x${have_Og}" = xyes; then
     CFLAGS="${CFLAGS-} -Wno-unused-but-set-variable -Wno-array-bounds -Wno-type-limits -Wno-incompatible-pointer-types \
     			-Wno-discarded-qualifiers -Wno-implicit-function-declaration -Wno-cpp -fno-strict-aliasing"
  fi
  CXXFLAGS="${CXXFLAGS-} -W"
fi

dnl Some code isn't alias-clean.
CFLAGS_NO_STRICT_ALIASING=
if test "x${GCC}" = "xyes"; then
  CFLAGS_NO_STRICT_ALIASING=" -fno-strict-aliasing"
fi
AC_SUBST(CFLAGS_NO_STRICT_ALIASING)

dnl For platforms, like Windows, where libraries must have all their symbols resolved
LDFLAGS="${LDFLAGS-} -no-undefined"

dnl Generate tmeconfig.h.  The whether-to-generate logic is cribbed 
dnl from glib-1.2.1's configure.ac.
dnl See that file for an explanation.
AC_CONFIG_COMMANDS([default-1],[[
          
## Generate our configure-time sources in two 
## cases:
## 1. `config.status' is run either explicitly, or via configure.
##     Esp. not when it is run in `Makefile' to generate makefiles and
##     config.h
## 2. CONFIG_OTHER is set explicitly
##
## Case 1 is difficult.  We know that `automake' sets one of
## CONFIG_FILES or CONFIG_HEADERS to empty.  This heuristic works
## only when AM_CONFIG_HEADER is set, however.

case "x${CONFIG_OTHER}" in
*tmeconfig.h) gen_tmeconfig_h=yes
;;
esac
if test -n "${CONFIG_FILES}" && test -n "${CONFIG_HEADERS}"; then
  # Both CONFIG_FILES and CONFIG_HEADERS are non-empty ==> Case 1
  if test "x${CONFIG_OTHER}" = x; then
    gen_tmeconfig_h=yes
  fi
fi

## If we're generating tmeconfig.h:
if test "x${gen_tmeconfig_h}" = xyes; then
  outfile=tmeconfig.h
  echo "$as_me: creating $outfile"
  cat <<TMEEOF > ${outfile}-tmp
/* tmeconfig.h
 * 
 * This is an automatically generated file - please modify 'configure.ac'.
 */

#ifndef _TMECONFIG_H
#define _TMECONFIG_H

TMEEOF
  (for word in HAVE_ TIME_ PROTO_ ALIGNOF_ SIZEOF_ AC_APPLE_UNIVERSAL_BUILD; do \
    grep $word config.h | sed -e 's/#define[ 	]\{1,\}/&_TME_/' -e 's/#if.*//g' ; \
  done) | sort | uniq >> ${outfile}-tmp
  echo '#if defined _TME_AC_APPLE_UNIVERSAL_BUILD' >> ${outfile}-tmp
  echo '# if defined __BIG_ENDIAN__' >> ${outfile}-tmp
  echo '#  define _TME_WORDS_BIGENDIAN 1' >> ${outfile}-tmp
  echo '# endif' >> ${outfile}-tmp
  echo '#else' >> ${outfile}-tmp
  echo '# ifndef _TME_WORDS_BIGENDIAN' >> ${outfile}-tmp
  words_be_num=`grep -c "undef WORDS_BIGENDIAN" config.h`
  if test $words_be_num = 0; then
    echo '#  define _TME_WORDS_BIGENDIAN 1' >> ${outfile}-tmp
  fi
  echo '# endif' >> ${outfile}-tmp
  echo '#endif' >> ${outfile}-tmp
  grep FLOAT_FORMAT config.h | \
    sed 's/#define[ 	]\{1,\}\([A-Za-z0-9_]\{1,\}\)[ 	]\{1,\}\([A-Za-z0-9_]\{1,\}\)/#define TME_\1 TME_FLOAT_FORMAT_\2/' >> ${outfile}-tmp
  echo '#define TME_BUSMAX_LOG2 TME_BUS'${bus_size_max}'_LOG2' >> ${outfile}-tmp
  echo '#define TME_HAVE_RECODE (' \
    `if test "x${recode_hosts}" = x; then echo 0; else echo 1; fi` ')' >> ${outfile}-tmp
  echo '#define TME_RECODE_SIZE_GUEST_MAX TME_RECODE_SIZE_'${recode_size_guest_max} >> ${outfile}-tmp
  echo '#define TME_RELEASE_MAJOR (' \
    `echo $VERSION | sed -e 's,^\([0-9][0-9]*\)\.[0-9][0-9]*$,\1,'` ')' >> ${outfile}-tmp
  echo '#define TME_RELEASE_MINOR (' \
    `echo $VERSION | sed -e 's,^[0-9][0-9]*\.\([0-9][0-9]*\)$,\1,'` ')' >> ${outfile}-tmp
  echo '#define TME_BUILD_HOST "'$host'"' >> ${outfile}-tmp
  echo '#ifdef _TME_HAVE___INT128' >> ${outfile}-tmp
  echo '#define TME_HAVE_INT128_T' >> ${outfile}-tmp
  echo '#define _TME_ALIGNOF_INT128_T 16' >> ${outfile}-tmp
  echo 'typedef __int128 tme_int128_t;' >> ${outfile}-tmp
  echo 'typedef unsigned __int128 tme_uint128_t;' >> ${outfile}-tmp
  echo '#endif' >> ${outfile}-tmp
  if test $ac_cv_sizeof_long = 8; then
    echo '#define TME_HAVE_INT64_T' >> ${outfile}-tmp
    echo '#define _TME_PRI64 "l"' >> ${outfile}-tmp
    echo 'typedef signed long tme_int64_t;' >> ${outfile}-tmp
    echo 'typedef unsigned long tme_uint64_t;' >> ${outfile}-tmp
  elif test $ac_cv_sizeof_long_long_int = 8; then
    echo '#define TME_HAVE_INT64_T' >> ${outfile}-tmp
    echo '#define _TME_PRI64 "ll"' >> ${outfile}-tmp
    echo 'typedef signed long long int tme_int64_t;' >> ${outfile}-tmp
    echo 'typedef unsigned long long int tme_uint64_t;' >> ${outfile}-tmp
  fi
  if test $ac_cv_sizeof_int = 4; then
    echo '#define _TME_PRI32 ""' >> ${outfile}-tmp
    echo 'typedef signed int tme_int32_t;' >> ${outfile}-tmp
    echo 'typedef unsigned int tme_uint32_t;' >> ${outfile}-tmp
  elif test $ac_cv_sizeof_long = 4; then
    echo '#define _TME_PRI32 "l"' >> ${outfile}-tmp
    echo 'typedef signed long tme_int32_t;' >> ${outfile}-tmp
    echo 'typedef unsigned long tme_uint32_t;' >> ${outfile}-tmp
  fi
  if test $ac_cv_sizeof_int = 2; then
    echo 'typedef signed int tme_int16_t;' >> ${outfile}-tmp
    echo 'typedef unsigned int tme_uint16_t;' >> ${outfile}-tmp
  elif test $ac_cv_sizeof_short = 2; then
    echo 'typedef signed short tme_int16_t;' >> ${outfile}-tmp
    echo 'typedef unsigned short tme_uint16_t;' >> ${outfile}-tmp
  fi
  echo 'typedef signed char tme_int8_t;' >> ${outfile}-tmp
  echo 'typedef unsigned char tme_uint8_t;' >> ${outfile}-tmp
  echo 'typedef tme_int'`expr $ac_cv_sizeof_void_p \* 8`'_t tme_intptr_t;' >> ${outfile}-tmp
  echo 'typedef tme_uint'`expr $ac_cv_sizeof_void_p \* 8`'_t tme_uintptr_t;' >> ${outfile}-tmp
  cat <<TMEEOF >> ${outfile}-tmp

/* features that aren't currently controlled by a configure option: */

#endif /* !_TMECONFIG_H */
TMEEOF
  if cmp -s ${outfile}-tmp $outfile; then
    echo "$as_me: $outfile is unchanged"
    rm -f ${outfile}-tmp
  else
    mv ${outfile}-tmp ${outfile}
  fi

  outfile=tme/recode-host.h
  echo "$as_me: creating $outfile"
  cat <<TMEEOF > ${outfile}-tmp
/* recode-host.h
 * 
 * This is an automatically generated file - please modify 'configure.ac'.
 */

TMEEOF
  for host in ${recode_hosts}; do
    echo '#include <tme/host/recode-'${host}'.h>' >> ${outfile}-tmp
  done
  delim='#define TME_RECODE_HOST_IC \'
  for host in ${recode_hosts}; do
    echo ${delim} >> ${outfile}-tmp
    echo -n '  TME_RECODE_'`echo ${host} | tr '[a-z]' '[A-Z]'`'_IC' >> ${outfile}-tmp
    delim='; \'
  done
  echo '' >> ${outfile}-tmp
  if cmp -s ${outfile}-tmp $outfile; then
    echo "$as_me: $outfile is unchanged"
    rm -f ${outfile}-tmp
  else
    mv ${outfile}-tmp ${outfile}
  fi

  outfile=libtme/recode-host.c
  echo "$as_me: creating $outfile"
  cat <<TMEEOF > ${outfile}-tmp
/* recode-host.c
 * 
 * This is an automatically generated file - please modify 'configure.ac'.
 */

TMEEOF
  for host in ${recode_hosts}; do
    if test -d ${srcdir}/libtme/host/${host}; then
      echo '#include "libtme/host/'${host}'/recode-'${host}'.c"' >> ${outfile}-tmp
    else
      echo '#include "libtme/host/recode-'${host}'.c"' >> ${outfile}-tmp
    fi
  done
  if cmp -s ${outfile}-tmp $outfile; then
    echo "$as_me: $outfile is unchanged"
    rm -f ${outfile}-tmp
  else
    mv ${outfile}-tmp ${outfile}
  fi

  outfile=libtme/misc-host.c
  echo "$as_me: creating $outfile"
  cat <<TMEEOF > ${outfile}-tmp
/* misc-host.c
 * 
 * This is an automatically generated file - please modify 'configure.ac'.
 */

TMEEOF
  for host in ${misc_hosts}; do
    if test -d ${srcdir}/libtme/host/${host}; then
      echo '#include "libtme/host/'${host}'/misc-'${host}'.c"' >> ${outfile}-tmp
    else
      echo '#include "libtme/host/misc-'${host}'.c"' >> ${outfile}-tmp
    fi
  done
  echo '#include "libtme/misc.c"' >> ${outfile}-tmp
  if cmp -s ${outfile}-tmp $outfile; then
    echo "$as_me: $outfile is unchanged"
    rm -f ${outfile}-tmp
  else
    mv ${outfile}-tmp ${outfile}
  fi

  outfile=tmememory.h
  echo "$as_me: creating $outfile"
  cat <<TMEEOF > ${outfile}-tmp
/* tmememory.h
 * 
 * This is an automatically generated file - please modify 'configure.ac'.
 */
#ifndef _TMEMEMORY_H
#define _TMEMEMORY_H
#define TME_MEMORY_TYPE_COMMON long
#ifdef TME_HAVE_INT128_T
#define TME_MEMORY_BUS_BOUNDARY sizeof(tme_uint128_t)
#else
#define TME_MEMORY_BUS_BOUNDARY sizeof(tme_uint64_t)
#endif

#define TME_MEMORY_BARRIER_READ_BEFORE_READ	(0)
#define TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE	(0)
#define TME_MEMORY_BARRIER_READ_BEFORE_WRITE	(0)
#define TME_MEMORY_BARRIER_WRITE_BEFORE_READ	(0)

#define check_align_min(mem,type,align_min) \
	(__tme_predict_true(TME_MEMORY_ALIGNMENT_ATOMIC(TME_MEMORY_TYPE_COMMON) != 0 \
	      && TME_MEMORY_ALIGNMENT_ATOMIC(type) != 0 \
              && _tme_memory_address_test(mem, TME_MEMORY_ALIGNMENT_ATOMIC(type) - 1, align_min) == 0))

#if __GNUC__ > 3 && __GNUC_MINOR__ > 7
#define tme_memory_barrier(address, size, barrier) __atomic_thread_fence(__ATOMIC_SEQ_CST)

#define TME_MEMORY_ALIGNMENT_ATOMIC(type) ((__atomic_always_lock_free(sizeof(type), 0)) ? (sizeof(type)) : (0))

TMEEOF

for sz in 8 16 32 64 128; do

if test $sz = 128; then
echo '#ifdef TME_HAVE_INT128_T' >> ${outfile}-tmp
fi

for op in add sub and or xor; do

echo '#define tme_memory_atomic_'${op}${sz}'(mem,val,lock,align_min) (check_align_min(mem,tme_uint'${sz}'_t,align_min) ? (__atomic_fetch_'${op}'(mem,val,__ATOMIC_SEQ_CST)) : (tme_memory_atomic_'${op}${sz}'(mem,val,lock,align_min)))' >> ${outfile}-tmp
done

echo '#define tme_memory_atomic_not'${sz}'(mem,lock,align_min) (check_align_min(mem,tme_uint'${sz}'_t,align_min) ? (__atomic_fetch_nand(mem,(uint'${sz}'_t)-1,__ATOMIC_SEQ_CST)) : (tme_memory_atomic_not'${sz}'(mem,lock,align_min)))' >> ${outfile}-tmp

echo '#define tme_memory_atomic_xchg'${sz}'(mem,val,lock,align_min) (check_align_min(mem,tme_uint'${sz}'_t,align_min) ? (__atomic_exchange_n(mem,val,__ATOMIC_SEQ_CST)) : (tme_memory_atomic_xchg'${sz}'(mem,val,lock,align_min)))' >> ${outfile}-tmp

cat <<TMEEOF >> ${outfile}-tmp

static inline
tme_uint${sz}_t _tme_memory_atomic_cx${sz}(tme_shared tme_uint${sz}_t *mem, tme_uint${sz}_t value_read, tme_uint${sz}_t value_written) {
  tme_uint${sz}_t tmp;

  tmp=value_read;
  __atomic_compare_exchange_n(mem, &tmp, value_written, FALSE, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

  return tmp;
}

#define tme_memory_atomic_cx${sz}(mem,valr,valw,lock,align_min) (check_align_min(mem,tme_uint${sz}_t,align_min) ? (_tme_memory_atomic_cx${sz}(mem,valr,valw)) : (tme_memory_atomic_cx${sz}(mem,valr,valw,lock,align_min)))

TMEEOF

if test $sz = 128; then
echo '#endif // TME_HAVE_INT128_T' >> ${outfile}-tmp
fi

done

echo '#else // __GNUC__' >> ${outfile}-tmp
echo '#define tme_memory_barrier(address, size, barrier) __sync_synchronize()' >> ${outfile}-tmp
echo '#define TME_MEMORY_ALIGNMENT_ATOMIC(type) ((sizeof(type) <= TME_MEMORY_BUS_BOUNDARY) ? (sizeof(type)) : (0))' >> ${outfile}-tmp
echo '#endif // !__GNUC__' >> ${outfile}-tmp
echo '#endif /* !_TMEMEMORY_H */' >> ${outfile}-tmp

  if cmp -s ${outfile}-tmp $outfile; then
    echo "$as_me: $outfile is unchanged"
    rm -f ${outfile}-tmp
  else
    mv ${outfile}-tmp ${outfile}
  fi
fi
]],[[
recode_hosts="$recode_hosts"
recode_size_guest_max="$recode_size_guest_max"
misc_hosts="$misc_hosts"
srcdir="$srcdir"
PACKAGE="$PACKAGE"
VERSION="$VERSION"
host="$host"
ac_cv_sizeof_void_p="$ac_cv_sizeof_void_p"
ac_cv_sizeof_int="$ac_cv_sizeof_int"
ac_cv_sizeof_long="$ac_cv_sizeof_long"
ac_cv_sizeof_long_long_int="$ac_cv_sizeof_long_long_int"
ac_cv_sizeof_short="$ac_cv_sizeof_short"
bus_size_max="$bus_size_max"
]])

dnl Writes files.
AC_CONFIG_FILES([Makefile
	  tme-modules.sh
	  tme/Makefile
	  tme/ic/Makefile
	  tme/machine/Makefile
	  tme/generic/Makefile
	  tme/scsi/Makefile
	  tme/host/Makefile
	  tme/bus/Makefile
	  libltdl/Makefile
	  libtme/Makefile
	  libtme/host/Makefile
	  libtme/host/x86/Makefile
	  ic/Makefile
	  ic/m68k/Makefile
	  ic/ieee754/Makefile
	  ic/sparc/Makefile
	  ic/stp22xx/Makefile
	  machine/Makefile
	  machine/sun/Makefile
	  machine/sun2/Makefile
	  machine/sun3/Makefile
	  machine/sun4/Makefile
	  machine/sun4u/Makefile
          host/Makefile
          host/display/Makefile
          host/display/gtk/Makefile
          host/display/rfb/Makefile
          host/display/sdl/Makefile
          host/eth/Makefile
          host/eth/openvpn/Makefile
          host/eth/bsd/Makefile
          host/eth/tun/Makefile
          bus/Makefile
          bus/multibus/Makefile
          bus/sbus/Makefile
          serial/Makefile
          scsi/Makefile
          generic/Makefile
          generic/bus/Makefile
	  tmesh/Makefile
	  tools/Makefile])
AC_OUTPUT
