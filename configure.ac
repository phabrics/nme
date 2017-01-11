dnl Process this file with autoconf to produce a configure script.

dnl $Id: configure.ac,v 1.20 2010/06/05 19:56:37 fredette Exp $

dnl Copyright (c) 2001, 2003 Matt Fredette
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
AC_PREREQ([2.68])
m4_include(version.m4)
AC_INIT([tme], [0.12beta34])
AC_CONFIG_SRCDIR([ic/m68k/m68k-impl.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_TARGET
#AC_CONFIG_AUX_DIR([libltdl/config])
#AC_CONFIG_MACRO_DIR([libltdl/m4])
AC_CONFIG_LIBOBJ_DIR([lib])
AM_INIT_AUTOMAKE([subdir-objects])
AM_PROG_CC_C_O

dnl Write configuration out to config.h through config.h.in.
AC_CONFIG_HEADERS([config.h])

dnl Checks for programs.
AC_PROG_AWK
#AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_DEFINE_UNQUOTED([TARGET_ALIAS], ["${host}"], [A string representing our host])
case "${host_os}" in
netbsd*) 
   AC_DEFINE([TARGET_NETBSD], [1], [Are we running NetBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["N"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-} -D_NETBSD_SOURCE"
   DEV_TAP_FILENAME='"\"/dev/tap\""'
   SETCAP="chmod u+s"
   ;;
openbsd*) 
   AC_DEFINE([TARGET_OPENBSD], [1], [Are we running on OpenBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["O"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/tun\""'
   SETCAP="chmod u+s"
   ;;
freebsd*) 
   AC_DEFINE([TARGET_FREEBSD], [1], [Are we running on FreeBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["F"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/tap\""'
   SETCAP="chmod u+s"
   ;;
dragonfly*) 
   AC_DEFINE([TARGET_DRAGONFLY], [1], [Are we running on DragonFlyBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["D"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/tap\""'
   SETCAP="chmod u+s"
   ;;
linux*) 
   AC_DEFINE([TARGET_LINUX], [1], [Are we running on Linux?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["L"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   DEV_TAP_FILENAME='"\"/dev/net/tun\""'
   SETCAP="chmod u+s"
#   SETCAP="setcap cap_sys_nice,cap_net_admin,cap_net_raw+ep"
   ;;
mingw* | msys*)
   AC_DEFINE([TARGET_WIN32], [1], [Are we running WIN32?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["W"], [Target prefix])
   CPPFLAGS="${CPPFLAGS} -DWIN32_LEAN_AND_MEAN"
   CPPFLAGS="${CPPFLAGS} -DNTDDI_VERSION=NTDDI_WINXP -D_WIN32_WINNT=_WIN32_WINNT_WINXP"
   WIN32=yes
   SETCAP="chmod u+s"
   ;;
cygwin*)   
   WIN32=cygwin
   SETCAP="chmod u+s"
   ;;
solaris*)
   AC_DEFINE([TARGET_SOLARIS], [1], [Are we running on Solaris?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["S"], [Target prefix])
   SETCAP="chmod u+s"
   ;;
darwin*)
   AC_DEFINE([TARGET_DARWIN], [1], [Are we running on Mac OS X?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["M"], [Target prefix])
   dnl some Mac OS X tendering (we use vararg macros...)
   CPPFLAGS="$CPPFLAGS -no-cpp-precomp"
   SETCAP="chmod u+s"
   have_tap_header="yes"
   ;;
*)
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["X"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   SETCAP="chmod u+s"
   have_tap_header="yes"
   ;;
esac

AM_CONDITIONAL([WIN32], [test "${WIN32}" = "yes"])

AC_SUBST(DEV_TAP_FILENAME)
AC_SUBST(SETCAP)
AC_SUBST(TGTPFX)

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
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h stdio.h memory.h stdarg.h stdbool.h limits.h float.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/sockio.h sys/socketio.h net/if_dl.h ioctls.h netpacket/packet.h ifaddrs.h])
AC_CHECK_HEADERS([byteswap.h sys/endian.h])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h poll.h sys/socket.h sys/select.h sys/time.h time.h errno.h sys/mman.h sys/uio.h])
AC_CHECK_HEADERS([io.h direct.h \
		  ctype.h sys/types.h sys/socket.h \
		  signal.h unistd.h dlfcn.h])
AC_CHECK_HEADERS([linux/netfilter/nf_tables.h libmnl/libmnl.h libnftnl/table.h libnftnl/chain.h libnftnl/rule.h libnftnl/expr.h])
AC_CHECK_HEADERS([pthread_np.h sched.h])
AC_CHECK_HEADERS([sys/param.h sys/linker.h sys/sysctl.h sys/module.h npf.h net/npf_ncode.h pcap/pcap.h], [], [],
[#ifdef HAVE_SYS_PARAM_H
 # include <sys/param.h>
 #endif
])

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

AC_CHECK_HEADERS([netinet/in_systm.h netinet/tcp.h windows.h winsock2.h ws2tcpip.h])

AC_CHECK_HEADERS([ \
	sys/stat.h \
	sys/file.h sys/wait.h \
	libgen.h stropts.h \
	syslog.h pwd.h grp.h \
	sys/sockio.h linux/sockios.h \
	linux/types.h sys/poll.h sys/epoll.h err.h \
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
AX_TYPE_SOCKLEN_T
AX_EMPTY_ARRAY
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

dnl Checks for library functions and prototypes.
#AC_FUNC_MEMCMP
#AC_FUNC_MALLOC
#AC_FUNC_MMAP
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime memmove memset munmap pow ptsname ptsname_r sqrt strchr strerror strrchr strstr strtoul devname fdevname kldfind isinff])

SOCKET_INCLUDES="
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
"

AC_CHECK_HEADERS(
	[net/if.h netinet/ip.h resolv.h sys/un.h net/if_utun.h sys/kern_control.h],
	,
	,
	[[${SOCKET_INCLUDES}]]
)

AC_CHECK_TYPES(
	[in_addr_t],
	,
	[AC_DEFINE([in_addr_t], [uint32_t], [Workaround missing in_addr_t])],
	[[${SOCKET_INCLUDES}]]
)
AC_CHECK_TYPE(
	[struct iphdr],
	[AC_DEFINE([HAVE_IPHDR], [1], [struct iphdr needed for IPv6 support])],
	,
	[[${SOCKET_INCLUDES}]]
)
AC_CHECK_TYPE(
	[struct sock_extended_err],
	[AC_DEFINE([HAVE_SOCK_EXTENDED_ERR], [1], [struct sock_extended_err needed for extended socket error support])],
	,
	[[${SOCKET_INCLUDES}]]
)
AC_CHECK_TYPE(
	[struct msghdr],
	[AC_DEFINE([HAVE_MSGHDR], [1], [struct msghdr needed for extended socket error support])],
	,
	[[${SOCKET_INCLUDES}]]
)
AC_CHECK_TYPE(
	[struct cmsghdr],
	[AC_DEFINE([HAVE_CMSGHDR], [1], [struct cmsghdr needed for extended socket error support])],
	,
	[[${SOCKET_INCLUDES}]]
)
AC_CHECK_TYPE(
	[struct in_pktinfo],
	[AC_DEFINE([HAVE_IN_PKTINFO], [1], [struct in_pktinfo needed for IP_PKTINFO support])],
	,
	[[${SOCKET_INCLUDES}]]
)
AC_CHECK_TYPE(
	[struct sockaddr_in6],
	,
	[AC_MSG_ERROR([struct sockaddr_in6 not found, needed for ipv6 transport support.])],
	[[${SOCKET_INCLUDES}]]
)
AC_CHECK_DECLS(
	[SO_MARK],
	,
	,
	[[${SOCKET_INCLUDES}]]
)

dnl We emulate signals in Windows
AC_CHECK_DECLS(
	[SIGHUP],
	,
	[AC_DEFINE([SIGHUP], [1], [SIGHUP replacement])],
	[[
		#ifdef HAVE_SIGNAL_H
		#include <signal.h>
		#endif
	]]
)
AC_CHECK_DECLS(
	[SIGINT],
	,
	[AC_DEFINE([SIGINT], [2], [SIGINT replacement])],
	[[
		#ifdef HAVE_SIGNAL_H
		#include <signal.h>
		#endif
	]]
)
AC_CHECK_DECLS(
	[SIGUSR1],
	,
	[AC_DEFINE([SIGUSR1], [10], [SIGUSR1 replacement])],
	[[
		#ifdef HAVE_SIGNAL_H
		#include <signal.h>
		#endif
	]]
)
AC_CHECK_DECLS(
	[SIGUSR2],
	,
	[AC_DEFINE([SIGUSR2], [12], [SIGUSR2 replacement])],
	[[
		#ifdef HAVE_SIGNAL_H
		#include <signal.h>
		#endif
	]]
)
AC_CHECK_DECLS(
	[SIGTERM],
	,
	[AC_DEFINE([SIGTERM], [15], [SIGTERM replacement])],
	[[
		#ifdef HAVE_SIGNAL_H
		#include <signal.h>
		#endif
	]]
)

AC_FUNC_FORK

AC_CHECK_FUNCS([ \
	chroot getpwnam setuid nice system getpid dup dup2 \
	getpass syslog openlog mlockall getgrnam setgid \
	setgroups stat flock readv writev time \
	ctime vsnprintf strdup \
	setsid chdir putenv getpeername unlink \
	chsize ftruncate execve getpeereid umask access \
	epoll_create sendmsg recvmsg
])

AC_CHECK_LIB(
	[dl],
	[dlopen],
	[DL_LIBS="-ldl"]
)
AC_SUBST([DL_LIBS])

AC_CHECK_LIB(
	[nsl],
	[inet_ntoa],
	[SOCKETS_LIBS="${SOCKETS_LIBS} -lnsl"]
)
AC_CHECK_LIB(
	[socket],
	[socket],
	[SOCKETS_LIBS="${SOCKETS_LIBS} -lsocket"]
)
AC_CHECK_LIB(
	[resolv],
	[gethostbyname],
	[SOCKETS_LIBS="${SOCKETS_LIBS} -lresolv"]
)
AC_SUBST([SOCKETS_LIBS])

old_LIBS="${LIBS}"
LIBS="${LIBS} ${SOCKETS_LIBS}"
AC_CHECK_FUNCS([sendmsg recvmsg inet_ntop inet_pton])
# Windows use stdcall for winsock so we cannot auto detect these
m4_define(
	[SOCKET_FUNCS],
[socket recv recvfrom send sendto listen dnl
accept connect bind select gethostbyname inet_ntoa]dnl
)
m4_define(
	[SOCKET_OPT_FUNCS],
	[setsockopt getsockopt getsockname poll]dnl
)
if test "${WIN32}" = "yes"; then
	m4_foreach(
		[F],
		m4_split(SOCKET_FUNCS SOCKET_OPT_FUNCS),
			m4_define([UF], [[m4_join([_], [HAVE], m4_toupper(F))]])
			AC_DEFINE([UF], [1], [Win32 builtin])
	)
else
	AC_CHECK_FUNCS(
		SOCKET_FUNCS,
		,
		[AC_MSG_ERROR([Required library function not found])]
	)
	AC_CHECK_FUNCS(SOCKET_OPT_FUNCS)
fi
LIBS="${old_LIBS}"

# we assume res_init() always exist, but need to find out *where*...
AC_SEARCH_LIBS(__res_init, resolv bind, ,
    AC_SEARCH_LIBS(res_9_init, resolv bind, ,
	AC_SEARCH_LIBS(res_init, resolv bind, , )))

AC_REPLACE_FUNCS([basename daemon dirname gettimeofday inet_ntop inet_pton])
AC_REPLACE_FUNCS([mmap munmap mprotect msync mlock munlock])

if test -n "${SP_PLATFORM_WINDOWS}"; then
	AC_DEFINE_UNQUOTED([PATH_SEPARATOR], ['\\\\'], [Path separator]) #"
	AC_DEFINE_UNQUOTED([PATH_SEPARATOR_STR], ["\\\\"], [Path separator]) #"
else
	AC_DEFINE_UNQUOTED([PATH_SEPARATOR], ['/'], [Path separator])
	AC_DEFINE_UNQUOTED([PATH_SEPARATOR_STR], ["/"], [Path separator])
fi

TAP_WIN_COMPONENT_ID="PRODUCT_TAP_WIN_COMPONENT_ID"
TAP_WIN_MIN_MAJOR="PRODUCT_TAP_WIN_MIN_MAJOR"
TAP_WIN_MIN_MINOR="PRODUCT_TAP_WIN_MIN_MINOR"
AC_DEFINE_UNQUOTED([TAP_WIN_COMPONENT_ID], ["${TAP_WIN_COMPONENT_ID}"], [The tap-windows id])
AC_DEFINE_UNQUOTED([TAP_WIN_MIN_MAJOR], [${TAP_WIN_MIN_MAJOR}], [The tap-windows version number is required for OpenVPN])
AC_DEFINE_UNQUOTED([TAP_WIN_MIN_MINOR], [${TAP_WIN_MIN_MINOR}], [The tap-windows version number is required for OpenVPN])
AC_SUBST([TAP_WIN_COMPONENT_ID])
AC_SUBST([TAP_WIN_MIN_MAJOR])
AC_SUBST([TAP_WIN_MIN_MINOR])

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

dnl Start the list of host support.
TME_HOSTS=

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

if test "x${have_tap_header}" = xyes; then
   TME_HOSTS="${TME_HOSTS} openvpn"
   AC_DEFINE([OPENVPN_VERSION_RESOURCE], [PRODUCT_VERSION_RESOURCE], [Version in windows resource format])
fi

have_tap_header=
AC_CHECK_HEADERS([linux/if_tun.h net/if_tap.h net/tap/if_tap.h net/if_tun.h],
		 [have_tap_header="yes"])

if test "x${have_tap_header}" = xyes; then
   TME_HOSTS="${TME_HOSTS} tun"
fi

AC_CHECK_HEADERS([linux/filter.h net/bpf.h])

dnl Checks for Ethernet.
AC_MSG_CHECKING([for raw Ethernet access method])

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
[TME_HOSTS="${TME_HOSTS} bsd"])

if test "x${TME_HOSTS}" != x; then
   TME_HOSTS="eth ${TME_HOSTS}"
fi

AC_MSG_RESULT($TME_HOSTS)

TME_HOSTS="${TME_HOSTS} posix"

AC_SYS_POSIX_TERMIOS

AM_CONDITIONAL([USING_TERMIOS], [test "x${ac_cv_sys_posix_termios}" = xyes])

AC_MSG_CHECKING([for available network address translation methods])

# specify whether built-in NAT support should be enabled & with what NAT methods
AC_ARG_ENABLE(nat,
[  --enable-nat=NATs  Enable built-in NAT support utilizing the given NAT methods (compiled-in order of usage by platform availability: 'nft','npf','pf','ipf').  Defaults to 'yes', any available.  Turn off NAT with 'no' or --disable-nat.], [tme_nat_types=${enableval}], [enable_nat=yes])

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

dnl Checks for X11.
AC_PATH_XTRA

AM_CONDITIONAL([USING_X], [test "x${no_x}" = x])

dnl Checks for GLIB.
AM_PATH_GLIB_2_0([], have_glib=yes, have_glib=no)
if test "x$have_glib" = xyes; then
  AC_DEFINE(HAVE_GLIB, [], [Define if you are compiling with GLIB.])
fi

dnl Checks for GTK.
AM_PATH_GTK_3_0([], have_gtk=yes, have_gtk=no)
if test "x$have_gtk" = xyes; then
  TME_HOSTS="${TME_HOSTS} gtk"
fi

dnl Characterize any graphics displays and remember the areas of
dnl the smallest and largest screens.
TME_FB_XLAT_DST=
area_smallest=0
area_largest=0

# if --with-tme-host-displays is given, characterize those given displays:
AC_ARG_WITH(tme-host-displays,
[  --with-tme-host-displays=DISPLAYS  support host displays with these display formats])
for dst_key in ${with_tme_host_displays}; do

  dnl Make brackets safe to use.
  changequote(<<, >>)dnl

   this_width=`echo ${dst_key} | sed -e 's/^\([0-9][0-9]*\)x\([0-9][0-9]*\)\(.*\)$/\1/'`
  this_height=`echo ${dst_key} | sed -e 's/^\([0-9][0-9]*\)x\([0-9][0-9]*\)\(.*\)$/\2/'`
      dst_key=`echo ${dst_key} | sed -e 's/^\([0-9][0-9]*\)x\([0-9][0-9]*\)\(.*\)$/\3/'`
  this_area=`expr ${this_width} \* ${this_height}`

  if test ${area_smallest} = 0 || test `expr ${this_area} \< ${area_smallest}` = 1; then
    area_smallest=${this_area}
  fi
  if test `expr ${this_area} \> ${area_largest}` = 1; then
    area_largest=${this_area}
  fi
  
  # add in this destination display key:
  if echo " ${TME_FB_XLAT_DST} " | grep " ${dst_key} " > /dev/null 2>&1; then :; else
    TME_FB_XLAT_DST="${TME_FB_XLAT_DST} ${dst_key}"
  fi

  dnl Make brackets the quote characters again.
  changequote([, ])dnl
done

# if DISPLAY is set and xdpyinfo appears to work, characterize this X display:
if test "x${DISPLAY}" != x && xdpyinfo >/dev/null 2>&1; then
  AC_MSG_CHECKING(characteristics of X display ${DISPLAY})

  dnl Make brackets safe to use.
  changequote(<<, >>)dnl

  # get the format this display uses for bitmaps:
  bitmap_info=`xdpyinfo | grep 'bitmap unit' | sed -e 's/bitmap unit, *bit order, *padding: *\(.*\)$/\1/'`
  bitmap_order=`echo ${bitmap_info} | sed -e 's/\([0-9][0-9]*\), *\([LM]\)SBFirst, *\([0-9][0-9]*\).*/\2/'`
    bitmap_pad=`echo ${bitmap_info} | sed -e 's/\([0-9][0-9]*\), *\([LM]\)SBFirst, *\([0-9][0-9]*\).*/\3/'`
  bitmap_order=`echo ${bitmap_order} | tr A-Z a-z`

  # get the format this display uses for images at its default depth.  we
  # assume that the root window is at the default depth:
  pixmap_order=`xdpyinfo | grep 'image byte order'`
  pixmap_order=`echo ${pixmap_order} | sed -e 's/.*image byte order: *\([LM]\)SBFirst.*/\1/' | tr A-Z a-z`
  pixmap_depth=`xwininfo -root | grep 'Depth:'`
  pixmap_depth=`echo ${pixmap_depth} | sed -e 's/.*Depth: *\([0-9][0-9]*\).*/\1/'`
  pixmap_infos=`xdpyinfo | grep bits_per_pixel | tr '\n' %`
  save_IFS=$IFS
  IFS=%
  for pixmap_info in $pixmap_infos; do
    IFS=$save_IFS
    if test "x${pixmap_info}" = x; then continue; fi
    this_pixmap_depth=`echo ${pixmap_info} | sed -e 's/.*depth \([0-9][0-9]*\), *bits_per_pixel \([0-9][0-9]*\), *scanline_pad \([0-9][0-9]*\).*/\1/'`
     this_pixmap_bipp=`echo ${pixmap_info} | sed -e 's/.*depth \([0-9][0-9]*\), *bits_per_pixel \([0-9][0-9]*\), *scanline_pad \([0-9][0-9]*\).*/\2/'`
      this_pixmap_pad=`echo ${pixmap_info} | sed -e 's/.*depth \([0-9][0-9]*\), *bits_per_pixel \([0-9][0-9]*\), *scanline_pad \([0-9][0-9]*\).*/\3/'`
    if test "x${this_pixmap_depth}" = "x${pixmap_depth}"; then
      pixmap_bipp=${this_pixmap_bipp}
      pixmap_pad=${this_pixmap_pad}
      break
    fi
  done
  IFS=$save_IFS

  # if this display's greatest depth is one, we use its
  # bitmap format:
  if test "x${pixmap_bipp}" = x; then
    pixmap_bipp=1
    pixmap_pad=${bitmap_pad}

    # our translation functions can't handle the case when the bitmap
    # bit order is different from the image byte order - doing so
    # requires dealing with the bitmap unit size.  since this
    # situation is rare, just bail:
    if test ${bitmap_order} != ${pixmap_order}; then
      changequote([, ])dnl
      AC_MSG_WARN([the X display ${DISPLAY} is monochrome and needs bitmaps with a bit order that the generic code doesn't support])
      changequote(<<, >>)dnl
      pixmap_bipp=
    fi
  fi

  # if this display seems usable:
  if test "x${pixmap_bipp}" != x; then

    # check the dimensions of all screens on this display:
    dimensions=`xdpyinfo | grep dimensions | tr '\n' %`
    save_IFS=$IFS
    IFS=%
    for dimension in $dimensions; do
      IFS=$save_IFS
      if test "x${dimension}" = x; then continue; fi
       this_width=`echo ${dimension} | sed -e 's/.*dimensions: *\([0-9][0-9]*\)x\([0-9][0-9]*\).*/\1/'`
      this_height=`echo ${dimension} | sed -e 's/.*dimensions: *\([0-9][0-9]*\)x\([0-9][0-9]*\).*/\2/'`
      this_area=`expr ${this_width} \* ${this_height}`
      if test ${area_smallest} = 0 || test `expr ${this_area} \< ${area_smallest}` = 1; then
        area_smallest=${this_area}
      fi
      if test `expr ${this_area} \> ${area_largest}` = 1; then
        area_largest=${this_area}
      fi
    done
    IFS=$save_IFS

    # assume an unknown set of masks and an unknown mapping type:
    dst_masks=_r0x0_g0x0_b0x0
    dst_map=
    value_visual=", unknown visual"

    # if the depth is greater than one:
    if test "x${pixmap_depth}" != 1; then

	# we assume that the root window uses the default visual:
	visual=`xwininfo -root | grep 'Visual Class:' | sed -e 's/^  *Visual Class: *\([A-Za-z][A-Za-z]*\).*$/\1/'`

	# get any primary masks for this visual:
	case "x${visual}" in
	xDirectColor | xTrueColor)
	    dst_masks=`xdpyinfo | $EGREP '(visual id|class|blue masks):' | $EGREP -C1 $visual | grep 'blue masks' | head -1`
	    dst_masks=`echo ${dst_masks} | sed -e 's/^.*blue masks:  *\(.*\)/\1/'`
	    value_visual=", rgb masks ${dst_masks}"
	    dst_masks=`echo ${dst_masks} | sed -e 's/^/_r/' -e 's/, /_g/' -e 's/, /_b/'`
	    ;;
	xStaticGray | xGrayScale | xStaticColor | xPseudoColor)	
	    dst_masks=
	    value_visual=", no subfields"
	    ;;
	*) ;;
	esac

	# get the mapping type for this visual:
	case "x${visual}" in
	xStaticGray | xStaticColor | xPseudoColor)
	    dst_map="ml"
	    ;;
	xDirectColor)
	    dst_map="mi"
	    value_visual="${value_visual}, indexed"
	    ;;
	xGrayScale | xTrueColor)
	    dst_map="ml"
	    value_visual="${value_visual}, linear"
	    ;;
	*) ;;
	esac
    fi

    dnl Make brackets the quote characters again.
    changequote([, ])dnl

    # finish the characterization:
    if test "x${pixmap_bipp}" = "x${pixmap_depth}"; then
      value=
    else
      value=" (${pixmap_bipp} bits per pixel)"
    fi
    AC_MSG_RESULT([${pixmap_depth}-bit${value} ${pixmap_order}sb-first images with ${pixmap_pad}-bit padding${value_visual}])
    dst_key="d${pixmap_depth}b${pixmap_bipp}s0p${pixmap_pad}o${pixmap_order}${dst_map}${dst_masks}"
    
    # add in this destination display key:
    if echo " ${TME_FB_XLAT_DST} " | grep " ${dst_key} " > /dev/null 2>&1; then :; else
      TME_FB_XLAT_DST="${TME_FB_XLAT_DST} ${dst_key}"
    fi
  fi
fi
AC_SUBST(TME_FB_XLAT_DST)

AC_MSG_CHECKING([for available threading models])

# specify whether preemptive thread support should be enabled & with what thread models
AC_ARG_ENABLE(threads,
[  --enable-threads=THRs  Enable preemptive thread support utilizing the given threading models (compiled-in order of usage by platform availability: 'pthreads (or posix)','glib').  Defaults to 'yes', any available.  Turn off threads with 'no' or --disable-threads; this will use setjmp/longjmp.], [tme_thread_types=${enableval}], [enable_threads=yes])

if test "x${enable_threads}" = xyes; then
   dnl Make brackets safe to use.
   changequote(<<, >>)dnl

   dnl set the default threads based on platform
   case "${host_os}" in
   mingw* | msys* | cygwin*) 
	tme_thread_types=sjlj
   	;;
   *) 
	tme_thread_types=pthreads
   	;;
   esac

   dnl Make brackets the quote characters again.
   changequote([, ])dnl
fi

# 128-bit integral type required to enable preemptive thread support
if test "x${enable_threads}" = xno; then
   tme_thread_types=sjlj
fi

for tme_thread_type in ${tme_thread_types}; do
case "x${tme_thread_type}" in
xpthreads | xposix)  CPPFLAGS="${CPPFLAGS-} -DTME_THREADS_POSIX"
#	  	     CFLAGS="${CFLAGS-} -pthread"
		     ;;
xglib)   CPPFLAGS="${CPPFLAGS-} -DTME_THREADS_GLIB -DUSE_GLIB_TIME ${GLIB_CFLAGS}"
#	 LDFLAGS="${LDFLAGS-} ${GLIB_LIBS}"
	 ;;
xsjlj)   CPPFLAGS="${CPPFLAGS-} -DTME_THREADS_SJLJ -DUSE_GETTIMEOFDAY" ;;
esac
done

CPPFLAGS="${CPPFLAGS-} -D_GNU_SOURCE" 

AM_CONDITIONAL([THREADS_POSIX], [test "x${tme_thread_types}" = xpthreads || test "x${tme_thread_types}" = xposix])
AM_CONDITIONAL([THREADS_GLIB], [test "x${tme_thread_type}" = xglib])
AM_CONDITIONAL([THREADS_SJLJ], [test "x${tme_thread_types}" = xsjlj])
AC_MSG_RESULT($tme_thread_types)

dnl Configures for the system(s) to emulate.
systems=all
if echo " ${systems} " | grep ' all ' > /dev/null 2>&1; then
  systems="sun2 sun3 sun4 sunultra1"
  bus_size_max=64
  recode_size_guest_max=64
fi
TME_MACHINE_SUBDIRS=
TME_IC_SUBDIRS=ieee754
TME_ICS=
TME_SERIAL_SUBDIRS=
TME_SERIALS=
TME_BUS_SUBDIRS=
TME_FB_XLAT_SRC=
for system in $systems; do

    # dispatch on the machine type to get more machines, ICs, and
    # buses to compile:
    case ${system} in

    # the sun2:
    sun2)
	machines="sun sun2"
	ics="m68k am9513 mm58167 z8530 i825x6"
	buses="multibus"
	fb_xlats="1152x900d1b1s0p32om"
	;;

    # the sun3:
    sun3)
	machines="sun sun3"
	ics="m68k isil7170 z8530 i825x6 ieee754 ncr5380"
	buses="multibus"
	fb_xlats="1152x900d1b1s0p32om 1152x900d8b8s0p32ommi8 1152x900d1b1s0p32omccmi8"
	;;

    # the sun4:
    sun4)
	machines="sun sun4"
	ics="sparc isil7170 z8530 am7930 am7990 ieee754 ncr53c9x mk48txx lsi64854 nec765"
	buses="sbus"
	fb_xlats="1152x900d1b1s0p32om 1152x900d8b8s0p32ommi8 1152x900d1b1s0p32omccmi8"
	;;

    # the sun Ultra-1:
    sunultra1)
	machines="sun sun4u"
	ics="sparc stp22xx z8530 am7990 ieee754 ncr53c9x ncr89c105 mk48txx lsi64854 nec765 stp2024 ad184x"
	buses="sbus"
	fb_xlats="1152x900d1b1s0p32om 1152x900d8b8s0p32ommi8 1152x900d1b1s0p32omccmi8"
	;;

    *)
	AC_MSG_ERROR([don't know how to emulate ${machine}])
	;;
    esac

    # add in the new machines, ICs, serials, and buses to compile:
    for machine in $machines; do
	if echo " ${TME_MACHINE_SUBDIRS} " | grep " ${machine} " > /dev/null 2>&1; then :; else
	    TME_MACHINE_SUBDIRS="${TME_MACHINE_SUBDIRS} ${machine}"
	fi
    done
    for ic in $ics; do
	if test -d $srcdir/ic/$ic; then
	    if echo " ${TME_IC_SUBDIRS} " | grep " ${ic} " > /dev/null 2>&1; then :; else
		TME_IC_SUBDIRS="${TME_IC_SUBDIRS} ${ic}"
	    fi
	else
	    ic="tme_ic_${ic}.la"
	    if echo " ${TME_ICS} " | grep " ${ic} " > /dev/null 2>&1; then :; else
		TME_ICS="${TME_ICS} ${ic}"
	    fi
	fi
    done
    for serial in $serials; do
	if test -d $srcdir/serial/$serial; then
	    if echo " ${TME_SERIAL_SUBDIRS} " | grep " ${serial} " > /dev/null 2>&1; then :; else
		TME_SERIAL_SUBDIRS="${TME_SERIAL_SUBDIRS} ${serial}"
	    fi
	else
	    serial="tme_serial_${serial}.la"
	    if echo " ${TME_SERIALS} " | grep " ${serial} " > /dev/null 2>&1; then :; else
		TME_SERIALS="${TME_SERIALS} ${serial}"
	    fi
	fi
    done
    for bus in $buses; do
	if echo " ${TME_BUS_SUBDIRS} " | grep " ${bus} " > /dev/null 2>&1; then :; else
	    TME_BUS_SUBDIRS="${TME_BUS_SUBDIRS} ${bus}"
	fi
    done
	
    # add in the new frame buffer translations to compile:
    for fb_xlat in $fb_xlats; do
 	if echo " ${TME_FB_XLAT_SRC} " | grep " ${fb_xlat} " > /dev/null 2>&1; then :; else
	    TME_FB_XLAT_SRC="${TME_FB_XLAT_SRC} ${fb_xlat}"
	fi

	dnl Make brackets safe to use.
	changequote(<<, >>)dnl

	# calculate the area of this frame buffer multiplied by 100:
	 this_width=`echo ${fb_xlat} | sed -e 's/^\([0-9][0-9]*\)x\([0-9][0-9]*\)\(.*\)$/\1/'`
	this_height=`echo ${fb_xlat} | sed -e 's/^\([0-9][0-9]*\)x\([0-9][0-9]*\)\(.*\)$/\2/'`
	this_area=`expr ${this_width} \* ${this_height} \* 100`

	dnl Make brackets the quote characters again.
	changequote([, ])dnl

	# if this frame buffer would consume at least 70 percent of
	# the smallest destination screen, make an xlat function that
	# scales this frame buffer down by two:
	if test ${area_smallest} = 0; then
	    percentage=100
	else
	    percentage=`expr ${this_area} / ${area_smallest}`
	fi
	if test `expr ${percentage} \> 70` = 1; then
	    src_key="${fb_xlat}_h"
	    if echo " ${TME_FB_XLAT_SRC} " | grep " ${src_key} " > /dev/null 2>&1; then :; else
		TME_FB_XLAT_SRC="${TME_FB_XLAT_SRC} ${src_key}"
	    fi
	fi
	  
	# if this frame buffer would consume less than 30 percent of
	# the largest destination screen, make an xlat function that
	# scales this frame buffer up by two:
	if test ${area_largest} = 0; then
	    percentage=0
	else
	    percentage=`expr ${this_area} / ${area_largest}`
	fi
	if test `expr ${percentage} \< 30` = 1; then
	    src_key="${fb_xlat}_d"
	    if echo " ${TME_FB_XLAT_SRC} " | grep " ${src_key} " > /dev/null 2>&1; then :; else
		TME_FB_XLAT_SRC="${TME_FB_XLAT_SRC} ${src_key}"
	    fi
	fi
    done
done
AC_SUBST(TME_MACHINE_SUBDIRS)
AC_SUBST(TME_IC_SUBDIRS)
AC_SUBST(TME_ICS)
AC_SUBST(TME_SERIAL_SUBDIRS)
AC_SUBST(TME_SERIALS)
AC_SUBST(TME_BUS_SUBDIRS)
AC_SUBST(TME_FB_XLAT_SRC)

dnl The list of host support is finished.
AC_SUBST(TME_HOSTS)

dnl Configure recode.
dnl Make brackets safe to use.
changequote(<<, >>)dnl

# set the default recode option
case "${host_cpu}" in
i[3456789]86)
recode=yes
;;
*) 
recode=no
;;
esac

dnl Make brackets the quote characters again.
changequote([, ])dnl

AC_ARG_ENABLE(recode,
[  --enable-recode         enable recode (dynamic binary translation) (default=if host supports)],
[ ], [enable_recode=$recode])
if test "${recode_size_guest_max}" = 0; then
    enable_recode=no
    recode_size_guest_max=1
fi
if test "x${enable_recode}" = xno; then
    recode_hosts=
else
    recode_hosts="mmap x86"
fi

AC_MSG_CHECKING([for host recode support])

dnl Make brackets safe to use.
changequote(<<, >>)dnl

# get the host maximum guest size:
case "${host_cpu}" in
     i[3456789]86)
     recode_size_guest_max_host=64
     ;;
x86_64)
     recode_size_guest_max_host=128
     ;;
*) 
     recode_hosts=
     recode_size_guest_max_host= ;;
esac

dnl Make brackets the quote characters again.
changequote([, ])dnl

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
*) misc_hosts=no ;;
esac
changequote([, ])dnl
AC_MSG_RESULT([${misc_hosts}])
if test "x${misc_hosts}" = "xno"; then
    misc_hosts=
fi

dnl Configure for libtool.
#AC_LIBLTDL_INSTALLABLE
#AC_SUBST(INCLTDL)
#AC_SUBST(LIBLTDL)
#AC_LIBTOOL_DLOPEN
#AM_PROG_LIBTOOL
#AC_SUBST(LTLIBOBJS)
#AC_CONFIG_SUBDIRS(libltdl)
LT_PREREQ([2.4])
LT_CONFIG_LTDL_DIR([libltdl])
LT_INIT([dlopen,win32-dll])
LT_LANG([Windows Resource])
LTDL_INIT([recursive installable])
CPPFLAGS="${CPPFLAGS-} ${LTDLINCL}"
TME_PREOPEN=
if test "x${enable_static}" = "xyes"; then
  TME_PREOPEN='`sort -u $(top_builddir)/tme-preopen.txt`'
fi
AC_SUBST(TME_PREOPEN)

AC_ARG_VAR([IFCONFIG], [full path to ipconfig utility])
AC_ARG_VAR([ROUTE], [full path to route utility])
AC_ARG_VAR([IPROUTE], [full path to ip utility])
AC_ARG_VAR([NETSTAT], [path to netstat utility]) # tests
AC_ARG_VAR([MAN2HTML], [path to man2html utility])
AC_ARG_VAR([GIT], [path to git utility])
AC_ARG_VAR([SYSTEMD_ASK_PASSWORD], [path to systemd-ask-password utility])
AC_PATH_PROGS([IFCONFIG], [ifconfig],, [$PATH:/usr/local/sbin:/usr/sbin:/sbin])
AC_PATH_PROGS([ROUTE], [route],, [$PATH:/usr/local/sbin:/usr/sbin:/sbin])
AC_PATH_PROGS([IPROUTE], [ip],, [$PATH:/usr/local/sbin:/usr/sbin:/sbin])
AC_PATH_PROGS([SYSTEMD_ASK_PASSWORD], [systemd-ask-password],, [$PATH:/usr/local/bin:/usr/bin:/bin])
AC_CHECK_PROGS([NETSTAT], [netstat], [netstat], [$PATH:/usr/local/sbin:/usr/sbin:/sbin:/etc]) # tests
AC_CHECK_PROGS([MAN2HTML], [man2html])
AC_CHECK_PROGS([GIT], [git]) # optional
AC_DEFINE_UNQUOTED([IFCONFIG_PATH], ["$IFCONFIG"], [Path to ifconfig tool])
AC_DEFINE_UNQUOTED([IPROUTE_PATH], ["$IPROUTE"], [Path to iproute tool])
AC_DEFINE_UNQUOTED([ROUTE_PATH], ["$ROUTE"], [Path to route tool])
AC_DEFINE_UNQUOTED([SYSTEMD_ASK_PASSWORD_PATH], ["$SYSTEMD_ASK_PASSWORD"], [Path to systemd-ask-password tool])

AC_ARG_ENABLE(
	[iproute2],
	[AS_HELP_STRING([--enable-iproute2], [enable support for iproute2 @<:@default=no@:>@])],
	,
	[test "${IPROUTE}" && enable_iproute2="yes" || enable_iproute2="no"]
)

if test "${enable_iproute2}" = "yes"; then
	test -z "${IPROUTE}" && AC_MSG_ERROR([ip utility is required but missing])
	AC_DEFINE([ENABLE_IPROUTE], [1], [enable iproute2 support])
else
	if test -z "${WIN32}"; then
		test -z "${ROUTE}" && AC_MSG_ERROR([route utility is required but missing])
		test -z "${IFCONFIG}" && AC_MSG_ERROR([ifconfig utility is required but missing])
	fi
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
  CPPFLAGS="${CPPFLAGS-} -DENABLE_DEBUG -DTME_NO_DEBUG_LOCKS -DTME_RECODE_DEBUG -D_TME_SPARC_STATS -DGTK_DISABLE_SINGLE_INCLUDES -DNDEBUG"
  if test "x${have_Og}" = xyes; then
     CFLAGS="${CFLAGS-} -Og"
     CXXFLAGS="${CXXFLAGS-} -g3 -Og"
  fi			   
else
  CPPFLAGS="${CPPFLAGS-} -DTME_NO_LOG -DTME_NO_DEBUG_LOCKS -DTME_NO_AUDIT_ATOMICS -DNDEBUG"
  CFLAGS="${CFLAGS-}"
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
		       -Wno-implicit-function-declaration -Wno-\#warnings \
		       -fbracket-depth=1024"
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
  echo '#if defined(USE_GLIB_TIME) && defined(_TME_HAVE_GLIB)' >> ${outfile}-tmp
  echo '#include <glib.h>' >> ${outfile}-tmp
  echo 'typedef gint64 tme_time_t;' >> ${outfile}-tmp
  echo '#define TME_FRAC_PER_SEC G_USEC_PER_SEC' >> ${outfile}-tmp
  echo '#elif defined(WIN32) && !defined(USE_GETTIMEOFDAY)' >> ${outfile}-tmp
  echo '#include <windows.h>' >> ${outfile}-tmp
  echo 'typedef tme_uint64_t tme_time_t;' >> ${outfile}-tmp
  echo '#define TME_FRAC_PER_SEC 10000000' >> ${outfile}-tmp
  echo '#elif defined(USE_GETTIMEOFDAY) || !defined(_TME_HAVE_CLOCK_GETTIME)' >> ${outfile}-tmp
  echo 'typedef struct timeval tme_time_t;' >> ${outfile}-tmp
  echo '#else' >> ${outfile}-tmp
  echo 'typedef struct timespec tme_time_t;' >> ${outfile}-tmp
  echo '#endif' >> ${outfile}-tmp
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
	  tme/Makefile
	  tme/ic/Makefile
	  tme/machine/Makefile
	  tme/generic/Makefile
	  tme/scsi/Makefile
	  tme/host/Makefile
	  tme/bus/Makefile
	  tme/libopenvpn/Makefile
          lib/Makefile
	  libltdl/Makefile
	  libopenvpn/Makefile
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
          host/openvpn/Makefile
          host/posix/Makefile
          host/eth/Makefile
          host/bsd/Makefile
          host/tun/Makefile
          host/gtk/Makefile
          bus/Makefile
          bus/multibus/Makefile
          bus/sbus/Makefile
          serial/Makefile
          scsi/Makefile
          generic/Makefile
	  tmesh/Makefile
	  tools/Makefile])
AC_OUTPUT

