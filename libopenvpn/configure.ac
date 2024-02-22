#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([libopenvpn], [2.3.8])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_SRCDIR([base64.c])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET
AC_CONFIG_LIBOBJ_DIR([lib])
AM_INIT_AUTOMAKE([subdir-objects])

m4_include([version.m4])
AC_DEFINE([OPENVPN_VERSION_RESOURCE], [PRODUCT_VERSION_RESOURCE], [Version in windows resource format])

# Checks for programs.
AC_PROG_CC

# Platform checks
AC_CHECK_DEFINE(__EMSCRIPTEN__, [
				CC=$(basename -s .bat $CC)
				host=$(echo "$host" | sed "s/$host_os/emscripten/")
				host_os=emscripten
				],
				[host_type=$host_os])

AC_DEFINE_UNQUOTED([TARGET_ALIAS], ["${host}"], [A string representing our host])
case "${host_os}" in
netbsd*) 
   AC_DEFINE([TARGET_NETBSD], [1], [Are we running NetBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["N"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-} -D_NETBSD_SOURCE"
   ;;
openbsd*) 
   AC_DEFINE([TARGET_OPENBSD], [1], [Are we running on OpenBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["O"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   ;;
freebsd*) 
   AC_DEFINE([TARGET_FREEBSD], [1], [Are we running on FreeBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["F"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   ;;
dragonfly*) 
   AC_DEFINE([TARGET_DRAGONFLY], [1], [Are we running on DragonFlyBSD?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["D"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   ;;
linux*) 
   AC_DEFINE([TARGET_LINUX], [1], [Are we running on Linux?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["L"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   ;;
mingw* | msys*)
   AC_DEFINE([TARGET_WIN32], [1], [Are we running WIN32?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["W"], [Target prefix])
   CPPFLAGS="${CPPFLAGS} -DWIN32_LEAN_AND_MEAN"
   CPPFLAGS="${CPPFLAGS} -DNTDDI_VERSION=NTDDI_WINXP -D_WIN32_WINNT=_WIN32_WINNT_WINXP"
   WIN32=yes
   host_type=
   ;;
cygwin*)   
   WIN32=cygwin
   host_type=
   ;;
solaris*)
   AC_DEFINE([TARGET_SOLARIS], [1], [Are we running on Solaris?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["S"], [Target prefix])
   ;;
darwin*)
   AC_DEFINE([TARGET_DARWIN], [1], [Are we running on Mac OS X?])
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["M"], [Target prefix])
   dnl some Mac OS X tendering (we use vararg macros...)
   CPPFLAGS="$CPPFLAGS -no-cpp-precomp"
   ;;
*)
   AC_DEFINE_UNQUOTED([TARGET_PREFIX], ["X"], [Target prefix])
   CPPFLAGS="${CPPFLAGS-}"
   host_type=
   ;;
esac

AM_CONDITIONAL([WIN32], [test "${WIN32}" = "yes"])

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

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h fcntl.h errno.h inttypes.h netdb.h netinet/in.h stdint.h strings.h sys/file.h sys/ioctl.h sys/mman.h sys/socket.h sys/time.h syslog.h unistd.h])

AC_CHECK_HEADERS([io.h direct.h ctype.h sys/types.h signal.h])

AC_CHECK_HEADERS([stdio.h stdarg.h stdbool.h limits.h])

AC_CHECK_HEADERS([netinet/in_systm.h netinet/tcp.h windows.h winsock2.h ws2tcpip.h])

AC_CHECK_HEADERS([ \
	sys/stat.h \
	sys/file.h sys/wait.h \
	libgen.h stropts.h \
	syslog.h pwd.h grp.h \
	sys/sockio.h linux/sockios.h \
	linux/types.h sys/poll.h sys/epoll.h err.h \
])

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

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AX_CPP_VARARG_MACRO_ISO
AX_CPP_VARARG_MACRO_GCC
AX_TYPE_SOCKLEN_T
AX_EMPTY_ARRAY

# Checks for libraries.
# FIXME: Replace 'main' with a function in '-lcrypt32':
#AC_CHECK_LIB([crypt32], [main])
# FIXME: Replace 'main' with a function in '-lgdi32':
#AC_CHECK_LIB([gdi32], [main])
# FIXME: Replace 'main' with a function in '-liphlpapi':
#AC_CHECK_LIB([iphlpapi], [main])
# FIXME: Replace 'main' with a function in '-lwininet':
#AC_CHECK_LIB([wininet], [main])
# FIXME: Replace 'main' with a function in '-lwinmm':
#AC_CHECK_LIB([winmm], [main])
# FIXME: Replace 'main' with a function in '-lws2_32':
#AC_CHECK_LIB([ws2_32], [main])

# Checks for library functions.
AC_FUNC_FORK
#AC_FUNC_MALLOC
#AC_FUNC_MMAP
#AC_CHECK_FUNCS([bzero dup2 ftruncate getpass gettimeofday inet_ntoa isascii memmove memset munmap select socket strcasecmp strchr strcspn strdup strerror strrchr strstr strtol])

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
	if test -n "${host_type}"; then
		test -z "${ROUTE}" && AC_MSG_ERROR([route utility is required but missing])
		test -z "${IFCONFIG}" && AC_MSG_ERROR([ifconfig utility is required but missing])
	fi
fi

#
# Libtool
#
ifdef(
	[LT_INIT],
	[
		LT_INIT([win32-dll])
		LT_LANG([Windows Resource])
	],
	[
		AC_LIBTOOL_WIN32_DLL
		AC_LIBTOOL_RC
		AC_PROG_LIBTOOL
	]
)

CPPFLAGS="${CPPFLAGS-} -D_GNU_SOURCE" 

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
  CPPFLAGS="${CPPFLAGS-} -DENABLE_DEBUG"
  if test "x${have_Og}" = xyes; then
     CFLAGS="${CFLAGS-} -Og"
     CXXFLAGS="${CXXFLAGS-} -g3 -Og"
  fi			   
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

AC_ARG_ENABLE(lto,
[  --enable-lto      compile with link-time optimizations (default=supported)],
[ ],[ enable_lto=yes ])

if test "x${enable_lto}" = "xyes"; then
  CFLAGS="${CFLAGS-} -flto"
  LDFLAGS="${LDFLAGS-} -flto"
fi

dnl Some code isn't alias-clean.
CFLAGS_NO_STRICT_ALIASING=
if test "x${GCC}" = "xyes"; then
  CFLAGS_NO_STRICT_ALIASING=" -fno-strict-aliasing"
fi
AC_SUBST(CFLAGS_NO_STRICT_ALIASING)

dnl For platforms, like Windows, where libraries must have all their symbols resolved
LDFLAGS="${LDFLAGS-} -no-undefined"

AC_CONFIG_FILES([Makefile
          lib/Makefile
])
AC_OUTPUT
