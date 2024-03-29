#! /bin/sh
# Generated from ieee754-misc-auto.m4 by GNU Autoconf 2.72.
## -------------------- ##
## M4sh Initialization. ##
## -------------------- ##

# Be more Bourne compatible
DUALCASE=1; export DUALCASE # for MKS sh
if test ${ZSH_VERSION+y} && (emulate sh) >/dev/null 2>&1
then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on ${1+"$@"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '${1+"$@"}'='"$@"'
  setopt NO_GLOB_SUBST
else case e in #(
  e) case `(set -o) 2>/dev/null` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
esac ;;
esac
fi



# Reset variables that may have inherited troublesome values from
# the environment.

# IFS needs to be set, to space, tab, and newline, in precisely that order.
# (If _AS_PATH_WALK were called with IFS unset, it would have the
# side effect of setting IFS to empty, thus disabling word splitting.)
# Quoting is to prevent editors from complaining about space-tab.
as_nl='
'
export as_nl
IFS=" ""	$as_nl"

PS1='$ '
PS2='> '
PS4='+ '

# Ensure predictable behavior from utilities with locale-dependent output.
LC_ALL=C
export LC_ALL
LANGUAGE=C
export LANGUAGE

# We cannot yet rely on "unset" to work, but we need these variables
# to be unset--not just set to an empty or harmless value--now, to
# avoid bugs in old shells (e.g. pre-3.0 UWIN ksh).  This construct
# also avoids known problems related to "unset" and subshell syntax
# in other old shells (e.g. bash 2.01 and pdksh 5.2.14).
for as_var in BASH_ENV ENV MAIL MAILPATH CDPATH
do eval test \${$as_var+y} \
  && ( (unset $as_var) || exit 1) >/dev/null 2>&1 && unset $as_var || :
done

# Ensure that fds 0, 1, and 2 are open.
if (exec 3>&0) 2>/dev/null; then :; else exec 0</dev/null; fi
if (exec 3>&1) 2>/dev/null; then :; else exec 1>/dev/null; fi
if (exec 3>&2)            ; then :; else exec 2>/dev/null; fi

# The user is always right.
if ${PATH_SEPARATOR+false} :; then
  PATH_SEPARATOR=:
  (PATH='/bin;/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 && {
    (PATH='/bin:/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 ||
      PATH_SEPARATOR=';'
  }
fi


# Find who we are.  Look in the path if we contain no directory separator.
as_myself=
case $0 in #((
  *[\\/]* ) as_myself=$0 ;;
  *) as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
for as_dir in $PATH
do
  IFS=$as_save_IFS
  case $as_dir in #(((
    '') as_dir=./ ;;
    */) ;;
    *) as_dir=$as_dir/ ;;
  esac
    test -r "$as_dir$0" && as_myself=$as_dir$0 && break
  done
IFS=$as_save_IFS

     ;;
esac
# We did not find ourselves, most probably we were run as 'sh COMMAND'
# in which case we are not to be found in the path.
if test "x$as_myself" = x; then
  as_myself=$0
fi
if test ! -f "$as_myself"; then
  printf "%s\n" "$as_myself: error: cannot find myself; rerun with an absolute file name" >&2
  exit 1
fi


if test "x$CONFIG_SHELL" = x; then
  as_bourne_compatible="if test \${ZSH_VERSION+y} && (emulate sh) >/dev/null 2>&1
then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on \${1+\"\$@\"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '\${1+\"\$@\"}'='\"\$@\"'
  setopt NO_GLOB_SUBST
else case e in #(
  e) case \`(set -o) 2>/dev/null\` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
esac ;;
esac
fi
"
  as_required="as_fn_return () { (exit \$1); }
as_fn_success () { as_fn_return 0; }
as_fn_failure () { as_fn_return 1; }
as_fn_ret_success () { return 0; }
as_fn_ret_failure () { return 1; }

exitcode=0
as_fn_success || { exitcode=1; echo as_fn_success failed.; }
as_fn_failure && { exitcode=1; echo as_fn_failure succeeded.; }
as_fn_ret_success || { exitcode=1; echo as_fn_ret_success failed.; }
as_fn_ret_failure && { exitcode=1; echo as_fn_ret_failure succeeded.; }
if ( set x; as_fn_ret_success y && test x = \"\$1\" )
then :

else case e in #(
  e) exitcode=1; echo positional parameters were not saved. ;;
esac
fi
test x\$exitcode = x0 || exit 1
blah=\$(echo \$(echo blah))
test x\"\$blah\" = xblah || exit 1
test -x / || exit 1"
  as_suggested=""
  if (eval "$as_required") 2>/dev/null
then :
  as_have_required=yes
else case e in #(
  e) as_have_required=no ;;
esac
fi
  if test x$as_have_required = xyes && (eval "$as_suggested") 2>/dev/null
then :

else case e in #(
  e) as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
as_found=false
for as_dir in /bin$PATH_SEPARATOR/usr/bin$PATH_SEPARATOR$PATH
do
  IFS=$as_save_IFS
  case $as_dir in #(((
    '') as_dir=./ ;;
    */) ;;
    *) as_dir=$as_dir/ ;;
  esac
  as_found=:
  case $as_dir in #(
	 /*)
	   for as_base in sh bash ksh sh5; do
	     # Try only shells that exist, to save several forks.
	     as_shell=$as_dir$as_base
	     if { test -f "$as_shell" || test -f "$as_shell.exe"; } &&
		    as_run=a "$as_shell" -c "$as_bourne_compatible""$as_required" 2>/dev/null
then :
  CONFIG_SHELL=$as_shell as_have_required=yes
		   break 2
fi
	   done;;
       esac
  as_found=false
done
IFS=$as_save_IFS
if $as_found
then :

else case e in #(
  e) if { test -f "$SHELL" || test -f "$SHELL.exe"; } &&
	      as_run=a "$SHELL" -c "$as_bourne_compatible""$as_required" 2>/dev/null
then :
  CONFIG_SHELL=$SHELL as_have_required=yes
fi ;;
esac
fi


      if test "x$CONFIG_SHELL" != x
then :
  export CONFIG_SHELL
             # We cannot yet assume a decent shell, so we have to provide a
# neutralization value for shells without unset; and this also
# works around shells that cannot unset nonexistent variables.
# Preserve -v and -x to the replacement shell.
BASH_ENV=/dev/null
ENV=/dev/null
(unset BASH_ENV) >/dev/null 2>&1 && unset BASH_ENV ENV
case $- in # ((((
  *v*x* | *x*v* ) as_opts=-vx ;;
  *v* ) as_opts=-v ;;
  *x* ) as_opts=-x ;;
  * ) as_opts= ;;
esac
exec $CONFIG_SHELL $as_opts "$as_myself" ${1+"$@"}
# Admittedly, this is quite paranoid, since all the known shells bail
# out after a failed 'exec'.
printf "%s\n" "$0: could not re-execute with $CONFIG_SHELL" >&2
exit 255
fi

    if test x$as_have_required = xno
then :
  printf "%s\n" "$0: This script requires a shell more modern than all"
  printf "%s\n" "$0: the shells that I found on your system."
  if test ${ZSH_VERSION+y} ; then
    printf "%s\n" "$0: In particular, zsh $ZSH_VERSION has bugs and should"
    printf "%s\n" "$0: be upgraded to zsh 4.3.4 or later."
  else
    printf "%s\n" "$0: Please tell bug-autoconf@gnu.org about your system,
$0: including any error possibly output before this
$0: message. Then install a modern shell, or manually run
$0: the script under such a shell if you do have one."
  fi
  exit 1
fi ;;
esac
fi
fi
SHELL=${CONFIG_SHELL-/bin/sh}
export SHELL
# Unset more variables known to interfere with behavior of common tools.
CLICOLOR_FORCE= GREP_OPTIONS=
unset CLICOLOR_FORCE GREP_OPTIONS

## --------------------- ##
## M4sh Shell Functions. ##
## --------------------- ##
# as_fn_unset VAR
# ---------------
# Portably unset VAR.
as_fn_unset ()
{
  { eval $1=; unset $1;}
}
as_unset=as_fn_unset

## -------------------- ##
## Main body of script. ##
## -------------------- ##


# $Id: ieee754-misc-auto.sh,v 1.6 2007/08/24 01:05:43 fredette Exp $

# ic/ieee754/ieee754-misc-auto.sh - automatically generates C code
# for miscellaneous IEEE 754 emulation support:

#
# Copyright (c) 2004 Matt Fredette
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Matt Fredette.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

header=false

for option
do
    case $option in
    --header) header=true ;;
    esac
done

PROG=`basename $0`
cat <<EOF
/* automatically generated by $PROG, do not edit! */
#include <tme/common.h>
_TME_RCSID("\$Id: ieee754-misc-auto.sh,v 1.6 2007/08/24 01:05:43 fredette Exp $");

EOF
if $header; then
    :
else
    cat <<EOF
#include <tme/ic/ieee754.h>
#include "softfloat-tme.h"
#include <math.h>
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
EOF
fi

# permute for the different precisions we want to support:
#
for precision in single double extended80 quad; do

    # get information about this precision:
    #
    _precision=`printf "%s\n" $0 | sed -e "s/$PROG/ieee754-precision.sh/"`
    eval `sh ${_precision} ${precision}`

    # if we're generating a header:
    #
    if $header; then
	cat <<EOF

/* decide which builtin C floating-point type is the best match for
   the IEEE 754 ${precision} precision format.  if a builtin type matches
   this format exactly, use that type, otherwise we assume that the
   smallest builtin type that is at least ${size} bytes wide is the best
   match.  if no builtin type is at least that wide, we use long
   double, or double if long double is not available: */
#if ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_${capprecision}) != 0)
#define TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN TME_FLOAT_FORMAT_IEEE754_${capprecision}
#elif (_TME_SIZEOF_FLOAT >= ${size})
#define TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN TME_FLOAT_FORMAT_FLOAT
#elif (_TME_SIZEOF_DOUBLE >= ${size} || !defined(_TME_HAVE_LONG_DOUBLE))
#define TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN TME_FLOAT_FORMAT_DOUBLE
#else
#define TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN TME_FLOAT_FORMAT_LONG_DOUBLE
#endif

/* typedef the builtin C floating-point type that is the best match
   for the IEEE 754 ${precision} precision format: */
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)
typedef float tme_ieee754_${precision}_builtin_t;
#define tme_float_value_ieee754_${precision}_builtin tme_float_value_float
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_DOUBLE)
typedef double tme_ieee754_${precision}_builtin_t;
#define tme_float_value_ieee754_${precision}_builtin tme_float_value_double
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_LONG_DOUBLE))
typedef long double tme_ieee754_${precision}_builtin_t;
#define tme_float_value_ieee754_${precision}_builtin tme_float_value_long_double
#endif

/* this asserts that the float is either in IEEE 754 ${precision}
   precision format, or in the best-match builtin type format.  it
   evaluates to nonzero if the float is in IEEE 754 ${precision}
   precision format: */
#define tme_ieee754_${precision}_is_format(x) \\
  (tme_float_assert_formats(x, \\
                            TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN) \\
   && tme_float_is_format(x, \\
                          TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN, \\
		          TME_FLOAT_FORMAT_IEEE754_${capprecision}))

/* this asserts that the float is either in IEEE 754 ${precision}
   precision format, or in the best-match builtin type format.  it
   evaluates to nonzero if the float is in the best-match builtin
   type format: */
#define tme_ieee754_${precision}_is_format_builtin(x) \\
  (tme_float_assert_formats(x, \\
                            TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN) \\
   && tme_float_is_format(x, \\
                          TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN, \\
		          TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN))

/* this asserts that the float is either in IEEE 754 ${precision}
   precision format, or in the best-match builtin type format.  it
   evaluates to nonzero if the float is a NaN: */
#define tme_ieee754_${precision}_is_nan(x) \\
  (tme_float_assert_formats(x, \\
                            TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN) \\
   && tme_float_is_nan(x, \\
                       TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN))

/* this asserts that the float is either in IEEE 754 ${precision}
   precision format, or in the best-match builtin type format.  it
   evaluates to nonzero if the float is an infinity: */
#define tme_ieee754_${precision}_is_inf(x) \\
  (tme_float_assert_formats(x, \\
                            TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN) \\
   && tme_float_is_inf(x, \\
                       TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN))

/* this asserts that the float is either in IEEE 754 ${precision}
   precision format, or in the best-match builtin type format.  it
   evaluates to nonzero if the float is a zero: */
#define tme_ieee754_${precision}_is_zero(x) \\
  (tme_float_assert_formats(x, \\
                            TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN) \\
   && tme_float_is_zero(x, \\
                        TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN))

/* tme_ieee754_${precision}_value_get(x, buffer) returns a pointer to
   the value of x in IEEE 754 ${precision} precision format (i.e., it
   returns a pointer to ${integral}).  if x isn't already in this
   format, it is converted into that format in the given buffer: */
#define tme_ieee754_${precision}_value_get(x, buffer) \\
  (tme_ieee754_${precision}_is_format(x) \\
   ? &(x)->tme_float_value_ieee754_${precision} \\
   : tme_ieee754_${precision}_value_from_builtin((x)->tme_float_value_ieee754_${precision}_builtin, buffer))

/* tme_ieee754_${precision}_value_set(x, y) sets the value of x to
   y, in IEEE 754 ${precision} precision format (i.e., y is a ${integral}).
   (the internal function _tme_ieee754_${precision}_value_set(x, t, y)
   takes the type of y, which must be compatible with ${integral}): */
#define tme_ieee754_${precision}_value_set(x, y) \\
  do { \\
    (x)->tme_float_value_ieee754_${precision} = (y); \\
    (x)->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision}; \\
  } while (/* CONSTCOND */ 0)
#define _tme_ieee754_${precision}_value_set(x, t, y) \\
  do { \\
    *((t *) &(x)->tme_float_value_ieee754_${precision}) = (y); \\
    (x)->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision}; \\
  } while (/* CONSTCOND */ 0)

/* tme_ieee754_${precision}_value_set_constant(x, y) sets the value of
   x to the constant y (i.e., y is a const ${constant} *): */
#define tme_ieee754_${precision}_value_set_constant(x, y) \\
  do { \\
EOF
	x_value="(x)->tme_float_value_ieee754_${precision}"
	case "${precision}" in
	single)
	    printf "%s\n" "    ${x_value} = *(y); \\"
	    ;;
	double)
	    printf "%s\n" "    ${x_value}.tme_value64_uint32_hi = (y)->tme_ieee754_double_constant_hi; \\"
	    printf "%s\n" "    ${x_value}.tme_value64_uint32_lo = (y)->tme_ieee754_double_constant_lo; \\"
	    ;;
	extended80)
	    printf "%s\n" "    ${x_value}.tme_float_ieee754_extended80_sexp = (y)->tme_ieee754_extended80_constant_sexp; \\"
	    printf "%s\n" "    ${x_value}.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi = (y)->tme_ieee754_extended80_constant_significand_hi; \\"
	    printf "%s\n" "    ${x_value}.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = (y)->tme_ieee754_extended80_constant_significand_lo; \\"
	    ;;
	quad)
	    printf "%s\n" "    ${x_value}.tme_float_ieee754_quad_hi.tme_value64_uint32_hi = (y)->tme_ieee754_quad_constant_hi_hi; \\"
	    printf "%s\n" "    ${x_value}.tme_float_ieee754_quad_hi.tme_value64_uint32_lo = (y)->tme_ieee754_quad_constant_hi_lo; \\"
	    printf "%s\n" "    ${x_value}.tme_float_ieee754_quad_lo.tme_value64_uint32_hi = (y)->tme_ieee754_quad_constant_lo_hi; \\"
	    printf "%s\n" "    ${x_value}.tme_float_ieee754_quad_lo.tme_value64_uint32_lo = (y)->tme_ieee754_quad_constant_lo_lo; \\"
	    ;;
	esac
	cat <<EOF
    (x)->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision}; \\
  } while (/* CONSTCOND */ 0)

/* tme_ieee754_${precision}_value_builtin_get(x) returns the value of
   x as the builtin C type that is the best match for the IEEE 754
   ${precision} precision format: */
#define tme_ieee754_${precision}_value_builtin_get(x) \\
  (tme_ieee754_${precision}_is_format_builtin(x) \\
   ? (x)->tme_float_value_ieee754_${precision}_builtin \\
   : tme_ieee754_${precision}_value_to_builtin(&(x)->tme_float_value_ieee754_${precision}))

/* tme_ieee754_${precision}_value_builtin_set(x, format, y) sets the value of
   x to y, whose type is a builtin C type with format format.  if the value of
   y is a NaN or an infinity, y is stored in x in IEEE 754 ${precision}
   precision format, otherwise y is stored in x as the builtin C type
   that is the best match for the IEEE 754 ${precision} precision format: */
#define tme_ieee754_${precision}_value_builtin_set(x, format, y) \\
  do { \\
    /* set the value: */ \\
    tme_float_value_builtin_set(x, format, y); \\
    \\
    /* if the result is a NaN: */ \\
    if (tme_float_is_nan(x, format)) { \\
      \\
      /* use the proper default IEEE 754 ${precision} precision NaN: */ \\
      (x)->tme_float_value_ieee754_${precision} = ieee754_ctl->tme_ieee754_ctl_default_nan_${precision}; \\
      (x)->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision}; \\
    } \\
    \\
    /* otherwise, if the result isn't already in IEEE 754 ${precision} precision format: */ \\
    else if ((format) != TME_FLOAT_FORMAT_IEEE754_${capprecision}) { \\
      \\
      /* if the result is infinite: */ \\
      if (tme_float_is_inf(x, format)) { \\
        \\
	/* use the IEEE 754 ${precision} precision infinity: */ \\
	(x)->tme_float_value_ieee754_${precision}${sexp} = ${mask_exp} | (tme_float_is_negative(x, (format)) ? ${mask_sign} : 0); \\
EOF
	case "${precision}" in
	single) ;;
	double)
	    printf "%s\n" "	(x)->tme_float_value_ieee754_double.tme_value64_uint32_lo = 0; \\"
	    ;;
	extended80)
	    printf "%s\n" "	(x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi = 0; \\"
	    printf "%s\n" "	(x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = 0; \\"
	    ;;
	quad)
	    printf "%s\n" "	(x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_hi.tme_value64_uint32_lo = 0; \\"
	    printf "%s\n" "	(x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_lo.tme_value64_uint32_hi = 0; \\"
	    printf "%s\n" "	(x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_lo.tme_value64_uint32_lo = 0; \\"
	    ;;
	esac
	cat <<EOF
        (x)->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision}; \\
      } \\
      \\
      /* otherwise, if the result isn't already the builtin C type that \\
         is the best match for the IEEE 754 ${precision} precision format: */ \\
      else if ((format) != TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN) { \\
        \\
	/* convert the result: */ \\
	if ((format) == TME_FLOAT_FORMAT_FLOAT) { \\
	  (x)->tme_float_value_ieee754_${precision}_builtin = (x)->tme_float_value_float; \\
	} \\
        TME_FLOAT_IF_LONG_DOUBLE(else if ((format) == TME_FLOAT_FORMAT_LONG_DOUBLE) { \\
	  (x)->tme_float_value_ieee754_${precision}_builtin = (x)->tme_float_value_long_double; \\
	}) \\
	else { \\
	  assert((format) == TME_FLOAT_FORMAT_DOUBLE); \\
	  (x)->tme_float_value_ieee754_${precision}_builtin = (x)->tme_float_value_double; \\
	} \\
        (x)->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN; \\
      } \\
    } \\
  } while (/* CONSTCOND */ 0)

/* this converts a value from IEEE 754 ${precision} precision format
   into the builtin C type that is the best match for that format: */
tme_ieee754_${precision}_builtin_t tme_ieee754_${precision}_value_to_builtin _TME_P((const ${integral} *));

/* this converts a value from the builtin C type that is the best
   match for the IEEE 754 ${precision} precision format, into that
   format: */
const ${integral} *tme_ieee754_${precision}_value_from_builtin _TME_P((tme_ieee754_${precision}_builtin_t, ${integral} *));

/* this does a NaN check for an IEEE 754 ${precision} precision monadic function: */
int tme_ieee754_${precision}_check_nan_monadic _TME_P((struct tme_ieee754_ctl *, const struct tme_float *, struct tme_float *));

/* this does a NaN check for an IEEE 754 ${precision} precision dyadic function: */
int tme_ieee754_${precision}_check_nan_dyadic _TME_P((struct tme_ieee754_ctl *, const struct tme_float *, const struct tme_float *, struct tme_float *));
EOF

	# emit the prototypes for the from-integer conversion
	# functions:
	#
	for convert in int32 int64; do

	    cond=1
	    case ${convert} in
	    int32) convert_builtin="tme_uint32_t" ;;
	    int64) convert_builtin="tme_uint64_t" ; cond="defined(TME_HAVE_INT64_T)" ;;
	    esac

	    if test "${cond}" != 1; then
		printf "%s\n" ""
		printf "%s\n" "#if ${cond}"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "/* this converts a ${convert_builtin} to a ${precision}: */"
	    printf "%s\n" "void tme_ieee754_${precision}_from_${convert} _TME_P((${convert_builtin}, struct tme_float *));"

	    if test "${cond}" != 1; then
		printf "%s\n" ""
		printf "%s\n" "#endif /* ${cond} */"
	    fi
	done

	# permute over the radices:
	#
	for radix in 10; do

	    cat <<EOF

/* this converts an in-range IEEE 754 ${precision} precision value into its
   radix ${radix} mantissa and exponent.  the mantissa is either zero, or
   in the range incl 1 to ${radix} excl: */
void tme_ieee754_${precision}_radix${radix}_mantissa_exponent _TME_P((struct tme_ieee754_ctl *, const struct tme_float *, struct tme_float *, struct tme_float *));

/* this scales an IEEE 754 ${precision} precision value by adding n to its
   radix ${radix} exponent: */
void tme_ieee754_${precision}_radix${radix}_scale _TME_P((struct tme_ieee754_ctl *, const struct tme_float *, const struct tme_float *, struct tme_float *));

EOF
	done

    # otherwise, we're not generating a header:
    #
    else

	cat <<EOF

/* this converts a value from IEEE 754 ${precision} precision format
   to the builtin C type that is the best match for that format: */
tme_ieee754_${precision}_builtin_t
tme_ieee754_${precision}_value_to_builtin(const ${integral} *x_ieee754)
{
  tme_ieee754_${precision}_builtin_t x_builtin;
  tme_uint32_t exponent;
  tme_uint32_t sign;
  tme_uint32_t chunk;
  tme_uint32_t fracor;

  /* get x's biased exponent: */
  exponent = TME_FIELD_MASK_EXTRACTU((*x_ieee754)${sexp}, ${mask_exp});

  /* convert the fraction one 16-bit chunk at a time, and track
     a bitwise-or of all of the fraction bits: */
EOF
	chunk_i=0
	while true; do
	    eval "chunk_member=\$chunk_member_${chunk_i} ; chunk_mask=\$chunk_mask_${chunk_i}"
	    if test "x${chunk_member}" = xx; then
		break
	    fi
	    printf "%s\n" "  chunk = TME_FIELD_MASK_EXTRACTU((*x_ieee754)${chunk_member}, ${chunk_mask});"
	    if test ${chunk_i} = 0; then
		printf "%s\n" "  fracor = chunk;"
		if ${implicit}; then
		    printf "%s\n" "  /* if the exponent is nonzero, add the implicit integer bit: */"
		    printf "%s\n" "  if (exponent != 0) chunk |= ((${chunk_mask} / _TME_FIELD_MASK_FACTOR(${chunk_mask})) + 1);"
		else
		    printf "%s\n" "  /* if the exponent is the biased maximum, clear the explicit integer bit: */"
		    printf "%s\n" "  if (exponent == ${exp_biased_max}) fracor &= ~_TME_FIELD_MASK_MSBIT(${chunk_mask} / _TME_FIELD_MASK_FACTOR(${chunk_mask}));"
		fi
		printf "%s\n" "  x_builtin = chunk;"
	    else
		printf "%s\n" "  fracor |= chunk;"
		printf "%s\n" "  x_builtin = (x_builtin * 65536) + chunk;"
	    fi
	    chunk_i=`expr ${chunk_i} + 1`
	done
	cat <<EOF

  /* get x's sign bit: */
  sign = ((*x_ieee754)${sexp} & ${mask_sign});

  /* if the exponent is the biased maximum, x is either an infinity or a NaN: */
  if (exponent == ${exp_biased_max}) {

    /* if the fraction is nonzero, x is a NaN.  x must not be a NaN,
       because we were supposed to catch this earlier: */
    assert (fracor == 0);

    /* x is an infinity.  construct a builtin infinity: */
    x_builtin =
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)
      tme_float_infinity_float
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_DOUBLE)
      tme_float_infinity_double
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_LONG_DOUBLE))
      tme_float_infinity_long_double
#endif
      (sign);
  }

  /* if the exponent is the biased minimum and the fraction is
     all-bits-zero, x is a zero: */
  else if (exponent == 0
	   && fracor == 0) {

    /* construct a builtin zero: */
    x_builtin =
      (sign ?
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)
       tme_float_negative_zero_float()
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_DOUBLE)
       tme_float_negative_zero_double()
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_LONG_DOUBLE))
       tme_float_negative_zero_long_double()
#endif
       : 0.0);
  }

  /* otherwise, x is an in-range value that needs to be converted: */
  else {

    /* scale the result by the unbiased exponent, adjusted by the
       number of fraction bits (which are currently to the left of the
       floating point in x_builtin): */
    x_builtin =
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)
      tme_float_radix2_scale_float
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_DOUBLE)
      tme_float_radix2_scale_double
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_LONG_DOUBLE))
      tme_float_radix2_scale_long_double
#endif
      (x_builtin, (exponent - ${exp_bias}) - ${fracbits});

    if (sign) {
      x_builtin = 0 - x_builtin;
    }
  }

  /* done: */
  return (x_builtin);
}

/* this converts a value from the builtin C type that is the best
   match for the IEEE 754 ${precision} precision format, to that
   format: */
const ${integral} *
tme_ieee754_${precision}_value_from_builtin(tme_ieee754_${precision}_builtin_t x_builtin, ${integral} *x_ieee754)
{
  tme_int32_t exponent;
  tme_uint32_t chunk;
  tme_ieee754_${precision}_builtin_t x_builtin_buffer;
  const tme_ieee754_${precision}_builtin_t pzero_builtin = +0.0;

  /* x must not be a NaN or an infinity: */
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)
  assert (!isnan(x_builtin));
  assert (!isinf(x_builtin));
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_DOUBLE)
  assert (!isnan(x_builtin));
  assert (!isinf(x_builtin));
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_LONG_DOUBLE))
  assert (!isnan(x_builtin));
  assert (!isinf(x_builtin));
#endif

  /* get the mantissa and exponent of x: */
  x_builtin =
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)
    tme_float_radix2_mantissa_exponent_float
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_DOUBLE)
    tme_float_radix2_mantissa_exponent_double
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_LONG_DOUBLE))
    tme_float_radix2_mantissa_exponent_long_double
#endif
    (x_builtin, &exponent);

  /* zero the IEEE 754 version: */
  memset((char *) x_ieee754, 0, sizeof((*x_ieee754)));

  /* if x is positive or negative zero: */
  if (x_builtin == 0
      || -x_builtin == 0) {

    /* set x's sign bit if x is not positive zero: */
    memcpy((char *) &x_builtin_buffer, (char *) &pzero_builtin, sizeof(pzero_builtin));
    x_builtin_buffer = x_builtin;
    if (x_builtin < 0
        || memcmp((char *) &x_builtin_buffer, (char *) &pzero_builtin, sizeof(pzero_builtin)) != 0) {
      (*x_ieee754)${sexp} |= ${mask_sign};
    }

    /* return the zero: */
    return (x_ieee754);
  }

  /* set x's sign bit: */
  if (x_builtin < 0) {
    (*x_ieee754)${sexp} |= ${mask_sign};
    x_builtin = -x_builtin;
  }

  /* bias the exponent: */
  exponent += ${exp_bias};

  /* if the biased exponent is greater than or equal to the biased
     maximum, we must represent x as an infinity: */
  if (exponent >= (tme_int32_t) ${exp_biased_max}) {

    /* we do this by just setting the biased exponent to the biased
       maximum: */
    exponent = ${exp_biased_max};
  }

  /* otherwise, x will be either a normalized number, a denormalized
     number, or possibly a zero: */
  else {

    /* if the biased exponent is less than or equal to the biased
       minimum, x will be a denormalized number (possibly so
       denormalized that it becomes a zero): */
    if (exponent <= 0) {

      /* scale x into a denormalized number: */
      assert (x_builtin >= 1);
      x_builtin =
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)
	tme_float_radix2_scale_float
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_DOUBLE)
	tme_float_radix2_scale_double
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_LONG_DOUBLE))
	tme_float_radix2_scale_long_double
#endif
	(x_builtin, exponent - 1);
      assert (x_builtin < 1);
      exponent = 0;
    }

    /* convert the mantissa, one 16-bit chunk at a time: */
EOF
	chunk_i=0
	while true; do
	    eval "chunk_member=\$chunk_member_${chunk_i} ; chunk_mask=\$chunk_mask_${chunk_i}"
	    if test "x${chunk_member}" = xx; then
		break
	    fi
	    factor="((${chunk_mask} / _TME_FIELD_MASK_FACTOR(${chunk_mask})) + 1)"
	    if test ${chunk_i} = 0; then
		if ${implicit}; then
		    printf "%s\n" "    /* remove any implicit integer bit: */"
		    printf "%s\n" "    if (x_builtin >= 1) {"
		    printf "%s\n" "      x_builtin -= 1;"
		    printf "%s\n" "    }"
		else
		    factor="(${factor} / 2)"
		fi
	    fi
	    printf "%s\n" "    x_builtin = x_builtin * ${factor};"
	    printf "%s\n" "    chunk = x_builtin;"
	    printf "%s\n" "    chunk -= (chunk > x_builtin);"
	    printf "%s\n" "    x_builtin -= chunk;"
	    printf "%s\n" "    TME_FIELD_MASK_DEPOSITU((*x_ieee754)${chunk_member}, ${chunk_mask}, chunk);"
	    chunk_i=`expr ${chunk_i} + 1`
	done
	cat <<EOF
  }

  /* set x's biased exponent: */
  TME_FIELD_MASK_DEPOSITU((*x_ieee754)${sexp}, ${mask_exp}, exponent);

  /* done: */
  return (x_ieee754);
}
EOF

	# emit the NaN check functions:
	#
	for monadic in true false; do
	    if ${monadic}; then type=monadic; else type=dyadic; fi

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a NaN check for an IEEE 754 ${precision} precision ${type} function: */"
	    printf "%s\n" "int"
	    printf %s "tme_ieee754_${precision}_check_nan_${type}(struct tme_ieee754_ctl *ieee754_ctl, const struct tme_float *src0"
	    if ${monadic}; then :; else
		printf %s ", const struct tme_float *src1"
	    fi
	    printf "%s\n" ", struct tme_float *dst)"
	    printf "%s\n" "{"
	    printf "%s\n" "  const ${integral} *nan0;"
	    if $monadic; then :; else
		printf "%s\n" "  const ${integral} *nan1;"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* check for a NaN operand: */"
	    printf "%s\n" "  nan0 = NULL;"
	    printf "%s\n" "  if (tme_ieee754_${precision}_is_nan(src0)) {"
	    printf "%s\n" "    assert (src0->tme_float_format == TME_FLOAT_FORMAT_IEEE754_${capprecision});"
	    printf "%s\n" "    nan0 = &src0->tme_float_value_ieee754_${precision};"
	    printf "%s\n" "  }"
	    if $monadic; then nan1=nan0; else
		nan1=nan1
		printf "%s\n" "  nan1 = nan0;"
		printf "%s\n" "  if (tme_ieee754_${precision}_is_nan(src1)) {"
		printf "%s\n" "    assert (src1->tme_float_format == TME_FLOAT_FORMAT_IEEE754_${capprecision});"
		printf "%s\n" "    nan1 = &src1->tme_float_value_ieee754_${precision};"
		printf "%s\n" "    if (nan0 == NULL) {"
		printf "%s\n" "      nan0 = nan1;"
		printf "%s\n" "    }"
		printf "%s\n" "  }"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* if we have a NaN operand: */"
	    printf "%s\n" "  if (__tme_predict_false(nan0 != NULL)) {"
	    printf "%s\n" ""
	    printf "%s\n" "    /* propagate a NaN: */"
	    printf "%s\n" "    dst->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision};"
	    printf "%s\n" "    (*ieee754_ctl->tme_ieee754_ctl_nan_from_nans_${precision})"
	    printf "%s\n" "      (ieee754_ctl, nan0, ${nan1}, &dst->tme_float_value_ieee754_${precision});"
	    printf "%s\n" "    return (TRUE);"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  return (FALSE);"
	    printf "%s\n" "}"
	done

	# emit the from-integer conversion functions:
	#
	for convert in int32 int64; do

	    cond=1
	    case ${convert} in
	    int32) convert_builtin="tme_uint32_t" ;;
	    int64) convert_builtin="tme_uint64_t" ; cond="defined(TME_HAVE_INT64_T)" ;;
	    esac

	    if test "${cond}" != 1; then
		printf "%s\n" ""
		printf "%s\n" "#if ${cond}"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "/* this converts a ${convert_builtin} to a ${precision}: */"
	    printf "%s\n" "void"
	    printf "%s\n" "tme_ieee754_${precision}_from_${convert}(${convert_builtin} src, struct tme_float *dst)"
	    printf "%s\n" "{"
	    printf "%s\n" "  _tme_ieee754_${precision}_value_set(dst, ${precision_sf}, ${convert}_to_${precision_sf}(src));"
	    printf "%s\n" "}"

	    if test "${cond}" != 1; then
		printf "%s\n" ""
		printf "%s\n" "#endif /* ${cond} */"
	    fi
	done

	# permute over the radices:
	#
	for radix in 2 10; do

	    # XXX FIXME - for now, skip quad precisions.  this is just
	    # laziness over generating the constants:
	    #
	    if test "${precision}" = quad; then continue; fi

	    cat <<EOF

/* this converts an in-range IEEE 754 ${precision} precision value into its
   radix ${radix} mantissa and exponent.  the mantissa is either zero, or
   in the range incl 1 to ${radix} excl: */
void
tme_ieee754_${precision}_radix${radix}_mantissa_exponent(struct tme_ieee754_ctl *ieee754_ctl, const struct tme_float *src, struct tme_float *_mantissa, struct tme_float *_exponent)
{
  tme_int32_t exponent;
EOF
	    if test ${radix} != 2; then
		cat <<EOF
#if ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_${capprecision}) == 0)
  ${integral} value_ieee754_buffer;
  struct tme_float value_buffer;
  struct tme_float zero_buffer;
  struct tme_float one_buffer;
  struct tme_float constant_buffer;
  struct tme_float radix_buffer;
  ${precision_sf} * const value = (${precision_sf} *) &value_buffer.tme_float_value_ieee754_${precision};
  const ${precision_sf} * const zero = (${precision_sf} *) &zero_buffer.tme_float_value_ieee754_${precision};
  const ${precision_sf} * const one = (${precision_sf} *) &one_buffer.tme_float_value_ieee754_${precision};
  const ${precision_sf} * const constant = (${precision_sf} *) &constant_buffer.tme_float_value_ieee754_${precision};
  const ${precision_sf} * const radix = (${precision_sf} *) &radix_buffer.tme_float_value_ieee754_${precision};
  tme_uint32_t exponent_bit;
  int negate;
#endif /* ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_${capprecision}) == 0) */
EOF
	    fi
	    cat <<EOF

  /* check for a NaN operand: */
  if (__tme_predict_false(tme_ieee754_${precision}_check_nan_monadic(ieee754_ctl, src, _mantissa))) {
    if (_exponent != NULL) {
      *_exponent = *_mantissa;
    }
    return;
  }

  /* if this is an infinity: */
  if (tme_ieee754_${precision}_is_inf(src)) {

    /* return a NaN: */
    _mantissa->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision};
    _mantissa->tme_float_value_ieee754_${precision} = ieee754_ctl->tme_ieee754_ctl_default_nan_${precision};
    if (_exponent != NULL) {
      *_exponent = *_mantissa;
    }
    return;
  }
EOF
	    if test ${radix} = 2; then
		printf "%s\n" ""
		printf "%s\n" "  /* extract the unbiased exponent: */"
		printf "%s\n" "  exponent = TME_FIELD_MASK_EXTRACTU(src->tme_float_value_ieee754_${precision}${sexp}, ${mask_exp});"
		printf "%s\n" "  exponent -= ${exp_bias};"
		printf "%s\n" ""
		printf "%s\n" "  /* the mantissa is the source with a biased zero for the exponent: */"
		printf "%s\n" "  *_mantissa = *src;"
		printf "%s\n" "  TME_FIELD_MASK_DEPOSITU(_mantissa->tme_float_value_ieee754_${precision}${sexp}, ${mask_exp}, ${exp_bias});"
	    else
		cat <<EOF

  /* if a builtin type matches the IEEE 754 ${precision} format exactly,
     use the corresponding mantissa-exponent function: */
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision} == TME_FLOAT_FORMAT_FLOAT)
  tme_ieee754_${precision}_value_builtin_set
    (_mantissa,
     TME_FLOAT_FORMAT_FLOAT,
     tme_float_radix${radix}_mantissa_exponent_float(tme_ieee754_${precision}_value_builtin_get(src), &exponent));
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision} == TME_FLOAT_FORMAT_DOUBLE)
  tme_ieee754_${precision}_value_builtin_set
    (_mantissa,
     TME_FLOAT_FORMAT_DOUBLE,
     tme_float_radix${radix}_mantissa_exponent_double(tme_ieee754_${precision}_value_builtin_get(src), &exponent));
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision} == TME_FLOAT_FORMAT_LONG_DOUBLE))
  tme_ieee754_${precision}_value_builtin_set
    (_mantissa,
     TME_FLOAT_FORMAT_LONG_DOUBLE,
     tme_float_radix${radix}_mantissa_exponent_long_double(tme_ieee754_${precision}_value_builtin_get(src), &exponent));
#else

  /* get this value and some constants: */
  tme_ieee754_${precision}_value_set(&value_buffer, *tme_ieee754_${precision}_value_get(src, &value_ieee754_buffer));
  tme_ieee754_${precision}_value_set_constant(&zero_buffer, &tme_ieee754_${precision}_constant_zero);
  tme_ieee754_${precision}_value_set_constant(&one_buffer, &tme_ieee754_${precision}_constant_one);
  tme_ieee754_${precision}_value_set_constant(&radix_buffer, &tme_ieee754_${precision}_constant_${radix}e2ex[0]);

  /* take the magnitude of the value, but remember if it was negative: */
  negate = ${precision_sf}_lt(*value, *zero);
  if (negate) {
    *value = ${precision_sf}_sub(*zero, *value);
  }

  /* start with an exponent of zero: */
  exponent = 0;

  /* if the value is nonzero: */
  if (!${precision_sf}_eq(*value, *zero)) {

    /* while the value is less than one: */
    exponent_bit = TME_ARRAY_ELS(tme_ieee754_${precision}_constant_${radix}e_minus_2ex) - 1;
    tme_ieee754_${precision}_value_set_constant(&constant_buffer, &tme_ieee754_${precision}_constant_${radix}e_minus_2ex[exponent_bit]);
    for (; ${precision_sf}_lt(*value, *one); ) {

      /* if value is less than or equal to ${radix}^-(2^exponent_bit),
         divide value by ${radix}^-(2^exponent_bit), and subtract 2^exponent_bit
         from exponent: */
      if (${precision_sf}_le(*value, *constant)
          || exponent_bit == 0) {
        *value = ${precision_sf}_div(*value, *constant);
        exponent -= (1 << exponent_bit);
      }

      /* otherwise, move to the next exponent bit: */
      else {
        exponent_bit--;
        tme_ieee754_${precision}_value_set_constant(&constant_buffer, &tme_ieee754_${precision}_constant_${radix}e_minus_2ex[exponent_bit]);
      }
    }

    /* while the value is greater than ${radix}: */
    exponent_bit = TME_ARRAY_ELS(tme_ieee754_${precision}_constant_${radix}e2ex) - 1;
    tme_ieee754_${precision}_value_set_constant(&constant_buffer, &tme_ieee754_${precision}_constant_${radix}e2ex[exponent_bit]);
    for (; !(${precision_sf}_le(*value, *radix)) ; ) {

      /* if value is greater than or equal to ${radix}^(2^exponent_bit),
         divide value by ${radix}^(2^exponent_bit), and add 2^exponent_bit
         to exponent: */
      if (!(${precision_sf}_lt(*value, *constant))
          || exponent_bit == 0) {
        *value = ${precision_sf}_div(*value, *constant);
        exponent += (1 << exponent_bit);
      }

      /* otherwise, move to the next exponent bit: */
      else {
        exponent_bit--;
        tme_ieee754_${precision}_value_set_constant(&constant_buffer, &tme_ieee754_${precision}_constant_${radix}e2ex[exponent_bit]);
      }
    }

    /* if the value was originally negative, negate the mantissa: */
    if (negate) {
      *value = ${precision_sf}_sub(*zero, *value);
    }

    /* return the mantissa: */
    _tme_ieee754_${precision}_value_set(_mantissa, ${precision_sf}, *value);
  }
#endif
EOF
	    fi
	    cat <<EOF

  /* return the exponent: */
  if (_exponent != NULL) {
    _tme_ieee754_${precision}_value_set(_exponent, ${precision_sf}, int32_to_${precision_sf}(exponent));
  }
}

/* this scales an IEEE 754 ${precision} precision value by adding n to its
   radix ${radix} exponent: */
void
tme_ieee754_${precision}_radix${radix}_scale(struct tme_ieee754_ctl *ieee754_ctl, const struct tme_float *src0, const struct tme_float *src1, struct tme_float *dst)
{
  ${integral} src_buffer;
  tme_int8_t rounding_mode;
  tme_int32_t _n;
#if ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_${capprecision}) == 0)
  tme_int32_t exponent;
  tme_uint32_t exponent_bit, n;
  ${precision_sf} * const value = (${precision_sf} *) &dst->tme_float_value_ieee754_${precision};
  struct tme_float constant_buffer;
  const ${precision_sf} * const constant = (${precision_sf} *) &constant_buffer.tme_float_value_ieee754_${precision};
#endif /* ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_${capprecision}) == 0) */

  /* check for a NaN operand: */
  if (__tme_predict_false(tme_ieee754_${precision}_check_nan_dyadic(ieee754_ctl, src0, src1, dst))) {
    return;
  }

  /* if the exponent is an infinity: */
  if (tme_ieee754_${precision}_is_inf(src1)) {

    /* return a NaN: */
    dst->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision};
    dst->tme_float_value_ieee754_${precision} = ieee754_ctl->tme_ieee754_ctl_default_nan_${precision};
    return;
  }

  /* if the operand is a zero or an infinity: */
  if (tme_ieee754_${precision}_is_zero(src1)
      || tme_ieee754_${precision}_is_inf(src1)) {

    /* return the operand unchanged: */
    *dst = *src0;
    return;
  }

  /* truncate the exponent to an integer, using the round-to-zero mode: */
  rounding_mode = ieee754_ctl->tme_ieee754_ctl_rounding_mode;
  ieee754_ctl->tme_ieee754_ctl_rounding_mode = TME_FLOAT_ROUND_TO_ZERO;
  _n = ${precision_sf}_to_int32(*((const ${precision_sf} *) tme_ieee754_${precision}_value_get(src1, &src_buffer)));
  ieee754_ctl->tme_ieee754_ctl_rounding_mode = rounding_mode;

  /* if a builtin type matches the IEEE 754 ${precision} format exactly,
     use the corresponding mantissa-exponent function: */
#if (TME_FLOAT_FORMAT_IEEE754_${capprecision} == TME_FLOAT_FORMAT_FLOAT)
  tme_ieee754_${precision}_value_builtin_set
    (dst,
     TME_FLOAT_FORMAT_FLOAT,
     tme_float_radix${radix}_scale_float(tme_ieee754_${precision}_value_builtin_get(src0), _n));
#elif (TME_FLOAT_FORMAT_IEEE754_${capprecision} == TME_FLOAT_FORMAT_DOUBLE)
  tme_ieee754_${precision}_value_builtin_set
    (dst,
     TME_FLOAT_FORMAT_DOUBLE,
     tme_float_radix${radix}_scale_double(tme_ieee754_${precision}_value_builtin_get(src0), _n));
#elif (defined(_TME_HAVE_LONG_DOUBLE) && (TME_FLOAT_FORMAT_IEEE754_${capprecision} == TME_FLOAT_FORMAT_LONG_DOUBLE))
  tme_ieee754_${precision}_value_builtin_set
    (dst,
     TME_FLOAT_FORMAT_LONG_DOUBLE,
     tme_float_radix${radix}_scale_long_double(tme_ieee754_${precision}_value_builtin_get(src0), _n));
#else

  /* start this value: */
  tme_ieee754_${precision}_value_set(dst, *tme_ieee754_${precision}_value_get(src0, &src_buffer));

  /* start with the most significant exponent bit: */
  exponent_bit = TME_ARRAY_ELS(tme_ieee754_${precision}_constant_${radix}e2ex) - 1;
  exponent = (1 << exponent_bit);
  tme_ieee754_${precision}_value_set_constant(&constant_buffer, &tme_ieee754_${precision}_constant_${radix}e2ex[exponent_bit]);

  /* if n is negative: */
  if (_n < 0) {

    for (n = 0 - _n; n > 0;) {
      if (n >= exponent || exponent == 1) {
        *value = ${precision_sf}_div(*value, *constant);
        n -= exponent;
      }
      else {
        exponent >>= 1;
        exponent_bit--;
	tme_ieee754_${precision}_value_set_constant(&constant_buffer, &tme_ieee754_${precision}_constant_${radix}e2ex[exponent_bit]);
      }
    }
  }

  /* otherwise, n is positive: */
  else {
    for (n = _n; n > 0;) {
      if (n >= exponent || exponent == 1) {
        *value = ${precision_sf}_mul(*value, *constant);
        n -= exponent;
      }
      else {
        exponent >>= 1;
        exponent_bit--;
	tme_ieee754_${precision}_value_set_constant(&constant_buffer, &tme_ieee754_${precision}_constant_${radix}e2ex[exponent_bit]);
      }
    }
  }
#endif
}
EOF
	done

    fi
done

# done:
#
exit 0;
