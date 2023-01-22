#! /bin/sh
# Generated from ../../../ic/ieee754/ieee754-ops-auto.m4 by GNU Autoconf 2.69.
## -------------------- ##
## M4sh Initialization. ##
## -------------------- ##

# Be more Bourne compatible
DUALCASE=1; export DUALCASE # for MKS sh
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on ${1+"$@"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '${1+"$@"}'='"$@"'
  setopt NO_GLOB_SUBST
else
  case `(set -o) 2>/dev/null` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
esac
fi


as_nl='
'
export as_nl
# Printing a long string crashes Solaris 7 /usr/bin/printf.
as_echo='\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\'
as_echo=$as_echo$as_echo$as_echo$as_echo$as_echo
as_echo=$as_echo$as_echo$as_echo$as_echo$as_echo$as_echo
# Prefer a ksh shell builtin over an external printf program on Solaris,
# but without wasting forks for bash or zsh.
if test -z "$BASH_VERSION$ZSH_VERSION" \
    && (test "X`print -r -- $as_echo`" = "X$as_echo") 2>/dev/null; then
  as_echo='print -r --'
  as_echo_n='print -rn --'
elif (test "X`printf %s $as_echo`" = "X$as_echo") 2>/dev/null; then
  as_echo='printf %s\n'
  as_echo_n='printf %s'
else
  if test "X`(/usr/ucb/echo -n -n $as_echo) 2>/dev/null`" = "X-n $as_echo"; then
    as_echo_body='eval /usr/ucb/echo -n "$1$as_nl"'
    as_echo_n='/usr/ucb/echo -n'
  else
    as_echo_body='eval expr "X$1" : "X\\(.*\\)"'
    as_echo_n_body='eval
      arg=$1;
      case $arg in #(
      *"$as_nl"*)
	expr "X$arg" : "X\\(.*\\)$as_nl";
	arg=`expr "X$arg" : ".*$as_nl\\(.*\\)"`;;
      esac;
      expr "X$arg" : "X\\(.*\\)" | tr -d "$as_nl"
    '
    export as_echo_n_body
    as_echo_n='sh -c $as_echo_n_body as_echo'
  fi
  export as_echo_body
  as_echo='sh -c $as_echo_body as_echo'
fi

# The user is always right.
if test "${PATH_SEPARATOR+set}" != set; then
  PATH_SEPARATOR=:
  (PATH='/bin;/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 && {
    (PATH='/bin:/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 ||
      PATH_SEPARATOR=';'
  }
fi


# IFS
# We need space, tab and new line, in precisely that order.  Quoting is
# there to prevent editors from complaining about space-tab.
# (If _AS_PATH_WALK were called with IFS unset, it would disable word
# splitting by setting IFS to empty value.)
IFS=" ""	$as_nl"

# Find who we are.  Look in the path if we contain no directory separator.
as_myself=
case $0 in #((
  *[\\/]* ) as_myself=$0 ;;
  *) as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
for as_dir in $PATH
do
  IFS=$as_save_IFS
  test -z "$as_dir" && as_dir=.
    test -r "$as_dir/$0" && as_myself=$as_dir/$0 && break
  done
IFS=$as_save_IFS

     ;;
esac
# We did not find ourselves, most probably we were run as `sh COMMAND'
# in which case we are not to be found in the path.
if test "x$as_myself" = x; then
  as_myself=$0
fi
if test ! -f "$as_myself"; then
  $as_echo "$as_myself: error: cannot find myself; rerun with an absolute file name" >&2
  exit 1
fi

# Unset variables that we do not need and which cause bugs (e.g. in
# pre-3.0 UWIN ksh).  But do not cause bugs in bash 2.01; the "|| exit 1"
# suppresses any "Segmentation fault" message there.  '((' could
# trigger a bug in pdksh 5.2.14.
for as_var in BASH_ENV ENV MAIL MAILPATH
do eval test x\${$as_var+set} = xset \
  && ( (unset $as_var) || exit 1) >/dev/null 2>&1 && unset $as_var || :
done
PS1='$ '
PS2='> '
PS4='+ '

# NLS nuisances.
LC_ALL=C
export LC_ALL
LANGUAGE=C
export LANGUAGE

# CDPATH.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

if test "x$CONFIG_SHELL" = x; then
  as_bourne_compatible="if test -n \"\${ZSH_VERSION+set}\" && (emulate sh) >/dev/null 2>&1; then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on \${1+\"\$@\"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '\${1+\"\$@\"}'='\"\$@\"'
  setopt NO_GLOB_SUBST
else
  case \`(set -o) 2>/dev/null\` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
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
if ( set x; as_fn_ret_success y && test x = \"\$1\" ); then :

else
  exitcode=1; echo positional parameters were not saved.
fi
test x\$exitcode = x0 || exit 1
test -x / || exit 1"
  as_suggested=""
  if (eval "$as_required") 2>/dev/null; then :
  as_have_required=yes
else
  as_have_required=no
fi
  if test x$as_have_required = xyes && (eval "$as_suggested") 2>/dev/null; then :

else
  as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
as_found=false
for as_dir in /bin$PATH_SEPARATOR/usr/bin$PATH_SEPARATOR$PATH
do
  IFS=$as_save_IFS
  test -z "$as_dir" && as_dir=.
  as_found=:
  case $as_dir in #(
	 /*)
	   for as_base in sh bash ksh sh5; do
	     # Try only shells that exist, to save several forks.
	     as_shell=$as_dir/$as_base
	     if { test -f "$as_shell" || test -f "$as_shell.exe"; } &&
		    { $as_echo "$as_bourne_compatible""$as_required" | as_run=a "$as_shell"; } 2>/dev/null; then :
  CONFIG_SHELL=$as_shell as_have_required=yes
		   break 2
fi
	   done;;
       esac
  as_found=false
done
$as_found || { if { test -f "$SHELL" || test -f "$SHELL.exe"; } &&
	      { $as_echo "$as_bourne_compatible""$as_required" | as_run=a "$SHELL"; } 2>/dev/null; then :
  CONFIG_SHELL=$SHELL as_have_required=yes
fi; }
IFS=$as_save_IFS


      if test "x$CONFIG_SHELL" != x; then :
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
# out after a failed `exec'.
$as_echo "$0: could not re-execute with $CONFIG_SHELL" >&2
exit 255
fi

    if test x$as_have_required = xno; then :
  $as_echo "$0: This script requires a shell more modern than all"
  $as_echo "$0: the shells that I found on your system."
  if test x${ZSH_VERSION+set} = xset ; then
    $as_echo "$0: In particular, zsh $ZSH_VERSION has bugs and should"
    $as_echo "$0: be upgraded to zsh 4.3.4 or later."
  else
    $as_echo "$0: Please tell bug-autoconf@gnu.org about your system,
$0: including any error possibly output before this
$0: message. Then install a modern shell, or manually run
$0: the script under such a shell if you do have one."
  fi
  exit 1
fi
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


# $Id: ieee754-ops-auto.sh,v 1.5 2009/08/28 01:34:01 fredette Exp $

# ic/ieee754/ieee754-misc-auto.sh - automatically generates C code
# for IEEE 754 emulation operations:

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
_TME_RCSID("\$Id: ieee754-ops-auto.sh,v 1.5 2009/08/28 01:34:01 fredette Exp $");

EOF
if $header; then
:
else
    cat <<EOF
#include "softfloat-tme.h"
#include <math.h>
EOF
fi

# the precision information helper script:
#
ieee754_precision_sh=`$as_echo $0 | sed -e "s/$PROG/ieee754-precision.sh/"`

# the different compliance levels:
#
levels="strict partial unknown"

# permute for the different compliance levels:
#
for level in ${levels}; do

    # permute for functions or for a set:
    #
    for what in funcs set; do

	# if we're generating headers:
	#
	if $header; then

	    # if we're doing the strict-level functions, start the
	    # operations struct type:
	    #
	    if test "${level}-${what}" = strict-funcs; then
		$as_echo "/* the IEEE 754 operations: */"
		$as_echo "struct tme_ieee754_ops {"
		$as_echo ""
		$as_echo "  /* the version of this structure: */"
		$as_echo "  tme_uint32_t tme_ieee754_ops_version;"


	    # otherwise, if we're doing a set, just declare the set
	    # and continue:
	    #
	    elif test ${what} = set; then
		$as_echo ""
		$as_echo "/* the ${level} compliance operations: */"
		$as_echo "extern _tme_const struct tme_ieee754_ops tme_ieee754_ops_${level};"
		continue
	    fi

	# otherwise, if we're doing a set:
	#
	elif test ${what} = set; then

	    # start the operations set for this level:
	    #
	    $as_echo ""
	    $as_echo "/* the ${level} compliance operations: */"
	    $as_echo "_tme_const struct tme_ieee754_ops tme_ieee754_ops_${level} = {"
	    $as_echo ""
	    $as_echo "  /* the version of this structure: */"
	    $as_echo "  TME_X_VERSION(0, 0),"
	fi

	# permute for the different precisions:
	#
	for precision in single double extended80 quad; do

	    # get information about this precision:
	    #
	    eval `sh ${ieee754_precision_sh} ${precision}`

	    # generate the operations:
	    #
	    for name in add sub mul div \
			rem sqrt abs neg move \
			rint \
			cos acos cosh \
			sin asin sinh \
			tan atan tanh atanh \
			exp expm1 log10 log log1p \
			getexp getman scale \
			pow \
			from_single from_double from_extended80 from_quad \
			from_int32 from_int64 \
			to_int32 to_int64 \
			; do

		# get the characteristics of this operation that are the
		# same at all compliance levels:
		#
		monadic=true
		case "${name}" in
		add | sub | mul | div | rem | pow | scale)
		    monadic=false
		    ;;
		esac
		src_type="struct tme_float *"
		dst_type="struct tme_float"
		case "${name}" in
		from_int32) src_type="tme_int32_t " ;;
		from_int64) src_type="tme_int64_t " ;;
		to_int32) dst_type="tme_int32_t" ;;
		to_int64) dst_type="tme_int64_t" ;;
		esac

		# we don't need a function to convert from the same
		# precision to this precision:
		#
		from_precision=
		case "${name}" in
		from_*)
		    from_precision=`$as_echo ${name} | sed -e 's/^from_//'`
		    if test ${from_precision} = ${precision}; then
			continue
		    fi
		    ;;
		esac

		# if we're generating headers:
		#
		if $header; then

		    # only emit this header information if we're
		    # doing the strict level functions:
		    #
		    if test "${level}-${what}" = strict-funcs; then
			$as_echo ""
			$as_echo "  /* this does a ${precision}-precision "`$as_echo ${name} | tr _ -`": */"
			$as_echo_n "  void (*tme_ieee754_ops_${precision}_${name}) _TME_P((struct tme_ieee754_ctl *, "
			if $monadic; then :; else
			    $as_echo_n "_tme_const struct tme_float *, "
			fi
			$as_echo "_tme_const ${src_type}, ${dst_type} *));"
		    fi

		    continue
		fi

		# start with no function for this set:
		#
		func_set='  NULL,'

		# permute for the stricter compliance levels:
		#
		for level_stricter in ${levels}; do

		    # form this function name:
		    #
		    func="_tme_ieee754_${level_stricter}_${precision}_${name}"

		    # the function type is normally determined by
		    # whether or not a softfloat function, libm
		    # function, or C operator is given.  it can be
		    # overridden:
		    #
		    type=
		    func_softfloat=
		    func_libm=
		    func_libm_has_f=true
		    op_builtin=

		    # any preprocessor condition controlling whether or not
		    # the function can be emitted is normally determined by
		    # the function type.  it can be overridden:
		    #
		    cond=
		    cond_int64=
		    cond_builtin_match=

		    # assume that we will use the operands as passed
		    # in.  the precisions will later default to the
		    # precision of the result, if they are not
		    # overridden:
		    #
		    op0=src0
		    op0_precision=
		    if $monadic; then op1= ; else op1=src1 ; fi
		    op1_precision=

		    # miscellaneous attributes:
		    #
		    src0_buffer=
		    src0_precision=
		    src1_buffer=
		    src1_precision=
		    check_nan=
		    check_inf_src0=
		    enter_softfloat=
		    enter_native=

		    # characterize this operation at this compliance level:
		    #
		    case "${level_stricter}-${name}" in
		    strict-add | strict-sub | strict-mul | strict-div | strict-rem | strict-sqrt)
			func_softfloat="${name}"
			;;
		    *-add) op_builtin='+' ;;
		    *-sub) op_builtin='-' ;;
		    *-mul) op_builtin='*' ;;
		    *-div) op_builtin='/' ;;
		    *-sqrt) func_libm=sqrt ;;
		    partial-abs | unknown-abs) func_libm=fabs ;;
		    strict-neg)	op0=-1 ; func_softfloat=mul ; op1=src0 ;;
		    partial-neg | unknown-neg) op0=-1 ; op_builtin='*'; op1=src0 ;;
		    strict-move) func_softfloat=add ; op1=0 ;;
		    *-move) type="${level_stricter}-move" ; src0_buffer=false ;;
		    strict-rint) func_softfloat=round_to_int ;;
		    partial-pow | unknown-pow) func_libm="${name}" ;;
		    partial-log | unknown-log) func_libm="${name}" ;;
		    partial-exp | unknown-exp) func_libm="${name}" ;;
		    partial-log10 | unknown-log10) func_libm="${name}" ;;
		    partial-scale | unknown-scale) func_libm=scalbn ;;
		    strict-getexp | strict-getman) type="${level_stricter}-${name}" ; check_nan=true ; check_inf_src0=return-nan ;;
		    strict-from_*) func_softfloat="OP0_PRECISION_SF_to_${precision_sf}" ; op0_precision="${from_precision}" ;;
		    strict-to_int32) type="${level_stricter}-${name}" ; func_softfloat="${precision_sf}_${name}" ;;
		    strict-to_int64) type="${level_stricter}-${name}" ; func_softfloat="${precision_sf}_${name}" ;;
		    esac

		    # finish typing this function:
		    #
		    if test "x${type}" = x; then
			if test "x${func_softfloat}" != x; then
			    type=softfloat
			elif test "x${func_libm}" != x; then
			    type=libm
			elif test "x${op_builtin}" != x; then
			    type=builtin
			else
			    type=none
			fi
		    fi

		    # finish the preprocessor condition controlling
		    # whether or not the function can be emitted:
		    #
		    if test "x${cond}" = x; then

			# assume that this function can be unconditionally emitted:
			#
			cond=1

			case "${precision}-${level_stricter}-${type}" in

			# a function with a none type can't be emitted:
			#
			*-*-none) cond=0 ;;

			# extended80 or quad precision softfloat
			# functions are conditional on a 64-bit
			# integral type:
			#
			extended80-*-softfloat | quad-*-softfloat) cond_int64=true ;;

			# partial compliance functions are conditional
			# on the builtin type being an exact match for
			# the IEEE 754 type:
			#
			*-partial-*) cond_builtin_match=true ;;
			esac
		    fi
		    case "${src_type}" in tme_int64_t*) cond_int64=true ;; esac
		    case "${dst_type}" in tme_int64_t*) cond_int64=true ;; esac
		    if test "x${cond_int64}" != x; then
			cond="${cond} && defined(TME_HAVE_INT64_T)"
		    fi
		    if test "x${cond_builtin_match}" != x; then
			cond="${cond} && (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_IEEE754_${capprecision})"
		    fi
		    eval "cond=\`$as_echo_n '${cond}' | sed -e 's%^1 && %%'\`"
		    case "${cond}" in 0*) cond=0 ;; esac

		    # finish the operands:
		    #
		    for _opn in 0 1; do
			eval "opn=\$op${_opn}"
			if test "x${opn}" = x; then
			    continue
			fi
			eval "opn_precision=\$op${_opn}_precision"
			if test "x${opn_precision}" = x; then
			    opn_precision="${precision}"
			fi
			case "${opn_precision}" in
			int32 | int64)
			    check_nan=false
			    eval "op${_opn}_precision_sf=\$opn_precision"
			    continue
			    ;;
			esac
			eval `sh ${ieee754_precision_sh} ${opn_precision} opn_`
			case "${opn}" in
			src[01])
			    eval "${opn}_precision=\$opn_precision"
			    if test "x${func_softfloat}" != x || test "x${func_libm}${op_builtin}" = x; then
				eval "srcn_buffer=\$${opn}_buffer"
				if test "x${srcn_buffer}" = x; then
				    eval "${opn}_buffer=true"
				fi
				opn="tme_ieee754_${opn_precision}_value_get(${opn}, &${opn}_buffer)"
				if test "x${func_softfloat}" != x; then
				    opn="((const ${opn_precision_sf} *) ${opn})"
				fi
				opn="(*${opn})"
			    else
				opn="tme_ieee754_${opn_precision}_value_builtin_get(${opn})"
			    fi
			    ;;
			[0-9] | -[0-9])
			    if test "x${func_softfloat}" != x; then
				opn="int32_to_${opn_precision_sf}(${opn})"
			    fi
			    ;;
			esac
			eval "op${_opn}=\$opn"
			eval "op${_opn}_precision=\$opn_precision"
			eval "op${_opn}_precision_sf=\$opn_precision_sf"
			eval "op${_opn}_integral=\$opn_integral"
		    done

		    # finish the miscellaneous attributes:
		    #
		    if test "x${src0_buffer}" = x; then
			src0_buffer=false
		    fi
		    if test "x${src1_buffer}" = x; then
			src1_buffer=false
		    fi
		    if test "x${check_nan}" = x; then
			if test "${level_stricter}" = partial; then
			    check_nan=true
			else
			    check_nan=false
			fi
		    fi
		    if test "x${enter_softfloat}" = x; then
			if test "x${func_softfloat}" != x; then
			    enter_softfloat=true
			else
			    enter_softfloat=false
			fi
		    fi
		    if test "x${enter_native}" = x; then
			if test "${level_stricter}" = partial; then
			    enter_native=true
			else
			    enter_native=false
			fi
		    fi

		    # if we're making functions, and we're at the
		    # right level to emit this function, and this
		    # function can be emitted:
		    #
		    if test ${what} = funcs && test ${level_stricter} = ${level} && test "${cond}" != 0; then

			# start any conditional:
			#
			if test "${cond}" != 1; then
			    $as_echo ""
			    $as_echo "#if ${cond}"
			fi

			# start the function:
			#
			$as_echo ""
			$as_echo "/* this does a ${level} compliance ${precision}-precision "`$as_echo ${name} | tr _ -`": */"
			$as_echo "static void"
			$as_echo_n "${func}(struct tme_ieee754_ctl *ieee754_ctl, const ${src_type}src0, "
			if $monadic; then :; else
			    $as_echo_n "const struct tme_float *src1, "
			fi
			$as_echo "${dst_type} *dst)"
			$as_echo "{"

			# emit locals:
			#
			if ${src0_buffer}; then
			    $as_echo "  ${op0_integral} src0_buffer;"
			fi
			if ${src1_buffer}; then
			    $as_echo "  ${op1_integral} src1_buffer;"
			fi
			$as_echo "  int exceptions;"

			# check the operand(s):
			#
			if ${check_nan}; then
			    $as_echo ""
			    $as_echo "  /* check for a NaN operand: */"
			    if $monadic; then nanf=monadic; src1= ; else nanf=dyadic; src1=", src1"; fi
			    $as_echo "  if (__tme_predict_false(tme_ieee754_${src0_precision}_check_nan_${nanf}(ieee754_ctl, src0${src1}, dst))) {"
			    $as_echo "    return;"
			    $as_echo "  }"
			fi
			if test "x${check_inf_src0}" != x; then
			    $as_echo ""
			    $as_echo "  /* if the operand is an infinity: */"
			    $as_echo "  if (tme_ieee754_${precision}_is_inf(src0)) {"
			    $as_echo ""
			    case "${check_inf_src0}" in
			    return-nan)
				$as_echo "    /* return a NaN: */"
				$as_echo "    dst->tme_float_format = TME_FLOAT_FORMAT_IEEE754_${capprecision};"
				$as_echo "    dst->tme_float_value_ieee754_${precision} = ieee754_ctl->tme_ieee754_ctl_default_nan_${precision};"
				$as_echo "    return;"
				;;
			    esac
			    $as_echo "  }"
			fi

			# enter the operation mode:
			#
			if ${enter_softfloat}; then
			    $as_echo ""
			    $as_echo "  /* enter softfloat operation: */"
			    $as_echo "  tme_mutex_lock(&tme_ieee754_global_mutex);"
			    $as_echo "  tme_ieee754_global_ctl = ieee754_ctl;"
			    $as_echo "  tme_ieee754_global_exceptions = 0;"
			    $as_echo "  ieee754_ctl->tme_ieee754_ctl_lock_unlock = tme_ieee754_unlock_softfloat;"
			fi
			if ${enter_native}; then
			    $as_echo ""
			    $as_echo "  /* enter native floating-point operation: */"
			    $as_echo "  tme_float_enter(ieee754_ctl->tme_ieee754_ctl_rounding_mode, tme_ieee754_exception_float, ieee754_ctl);"
			    $as_echo "  ieee754_ctl->tme_ieee754_ctl_lock_unlock = tme_float_leave;"
			fi

			# assume that this operation raises no exceptions:
			#
			$as_echo ""
			$as_echo "  /* assume that this operation raises no exceptions: */"
			$as_echo "  exceptions = 0;"

			# the operation:
			#
			$as_echo ""
			$as_echo "  /* the operation: */"
			case "${type}" in

			# a move operation:
			#
			*-move)
			    $as_echo "  *dst = *src0;"
			    ;;

			# a getexp operation:
			#
			strict-getexp)
			    $as_echo ""
			    $as_echo "  /* if the operand is a zero, return a zero: */"
			    $as_echo "  if (tme_ieee754_${precision}_is_zero(src0)) {"
			    $as_echo "    tme_ieee754_${precision}_value_builtin_set(dst, TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN, 0);"
			    $as_echo "  }"
			    $as_echo ""
			    $as_echo "  /* otherwise, return the unbiased exponent: */"
			    $as_echo "  else {"
			    $as_echo "    tme_ieee754_${precision}_value_builtin_set(dst, TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN, TME_FIELD_MASK_EXTRACTU(${op0}${sexp}, ${mask_exp}) - ${exp_bias});"
			    $as_echo "  }"
			    ;;

			# a getman operation:
			#
			strict-getman)
			    $as_echo ""
			    $as_echo "  /* if the operand is a zero, return it: */"
			    $as_echo "  if (tme_ieee754_${precision}_is_zero(src0)) {"
			    $as_echo "    *dst = *src0;"
			    $as_echo "  }"
			    $as_echo ""
			    $as_echo "  /* otherwise, return the operand, with its exponent set to biased zero: */"
			    $as_echo "  else {"
			    $as_echo "    tme_ieee754_${precision}_value_set(dst, ${op0});"
			    $as_echo "    TME_FIELD_MASK_DEPOSITU(dst->tme_float_value_ieee754_${precision}${sexp}, ${mask_exp}, ${exp_bias});"
			    $as_echo "  }"
			    ;;

			# a strict to-integer conversion operation:
			#
			strict-to_int32 | strict-to_int64)
			    $as_echo "  *dst = ${precision_sf}_${name}(${op0});"
			    ;;

			# a softfloat operation:
			#
			softfloat)
			    $as_echo "  _tme_ieee754_${precision}_value_set(dst, ${precision_sf},"
			    func_softfloat_raw="${func_softfloat}"
			    func_softfloat=`$as_echo ${func_softfloat} | sed -e "s/OP0_PRECISION_SF/${op0_precision_sf}/g"`
			    func_softfloat=`$as_echo ${func_softfloat} | sed -e "s/OP1_PRECISION_SF/${op1_precision_sf}/g"`
			    if test "${func_softfloat}" = "${func_softfloat_raw}"; then
				func_softfloat="${precision_sf}_${func_softfloat}"
			    fi
			    $as_echo_n "    ${func_softfloat}(${op0}"
			    if test "x${op1}" != x; then
				$as_echo ","
				$as_echo_n "                ${op1}"
			    fi
			    $as_echo "));"
			    ;;

			# a libm operation:
			#
			libm)
			    if test "x${op1}" = x; then ops="${op0}"; else ops="${op0}, ${op1}"; fi
			    # if there is a float variant of this libm function:
			    #
			    if ${func_libm_has_f}; then
				$as_echo "#if (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN == TME_FLOAT_FORMAT_FLOAT)"
				$as_echo "  tme_ieee754_${precision}_value_builtin_set(dst, TME_FLOAT_FORMAT_FLOAT, ${func_libm}f(${ops}));"
				$as_echo "#else  /* (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN != TME_FLOAT_FORMAT_FLOAT) */"
			    fi
			    $as_echo "  tme_ieee754_${precision}_value_builtin_set(dst, TME_FLOAT_FORMAT_DOUBLE, ${func_libm}(${ops}));"
			    if ${func_libm_has_f}; then
				$as_echo "#endif /* (TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN != TME_FLOAT_FORMAT_FLOAT) */"
			    fi
			    ;;

			# a builtin operation:
			#
			builtin)
			    $as_echo "  tme_ieee754_${precision}_value_builtin_set(dst, TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN, ${op0} ${op_builtin} ${op1});"
			    ;;

			*)
			    $as_echo "$PROG internal error: don't know how to generate a ${type} type operation" 1>&2
			    exit 1
			    ;;
			esac

			# leave the operation mode:
			#
			if ${enter_native}; then
			    $as_echo ""
			    $as_echo "  /* leave native floating-point operation: */"
			    $as_echo "  exceptions |= tme_float_leave();"
			fi
			if ${enter_softfloat}; then
			    $as_echo ""
			    $as_echo "  /* leave softfloat operation: */"
			    $as_echo "  tme_ieee754_global_ctl = NULL;"
			    $as_echo "  exceptions |= tme_ieee754_global_exceptions;"
			    $as_echo "  tme_mutex_unlock(&tme_ieee754_global_mutex);"
			fi
			$as_echo "  ieee754_ctl->tme_ieee754_ctl_lock_unlock = NULL;"

			# signal any exceptions:
			#
			$as_echo ""
			$as_echo "  /* signal any exceptions: */"
			$as_echo "  if (exceptions != 0) {"
			$as_echo "    (*ieee754_ctl->tme_ieee754_ctl_exception)(ieee754_ctl, exceptions);"
			$as_echo "  }"

			# end the function:
			#
			$as_echo "}"

			# close any conditional:
			#
			if test "${cond}" != 1; then
			    $as_echo ""
			    $as_echo "#endif /* ${cond} */"
			fi
		    fi

		    # update the function for this set:
		    #
		    case "${cond}" in
		    0) ;;
		    1) func_set="  ${func}," ;;
		    *) func_set="#if (${cond})@  ${func},@#else  /* !(${cond}) */@${func_set}@#endif /* !(${cond}) */" ;;
		    esac

		    # stop now if we just did this level:
		    #
		    if test ${level_stricter} = ${level}; then
			break
		    fi
		done

		# if we're making a set:
		#
		if test ${what} = set; then
		    $as_echo ""
		    $as_echo "  /* this does a ${level} compliance ${precision}-precision ${name}: */"
		    $as_echo "${func_set}" | tr '@' '\n'
		fi

	    done
	done

	# if we're generating headers:
	#
	if $header; then

	    # if we're doing the strict-level functions, close the
	    # operations struct type:
	    #
	    if test "${level}-${what}" = strict-funcs; then
		$as_echo "};"
	    fi

	# otherwise, if we're doing a set:
	#
	elif test ${what} = set; then

	    # close the operations set for this level:
	    #
	    $as_echo "};"
	fi

    done

done

# if we're not generating headers:
#
if $header; then :; else

    $as_echo ""
    $as_echo "/* this looks up an operations structure: */"
    $as_echo "const struct tme_ieee754_ops *"
    $as_echo "tme_ieee754_ops_lookup(const char *compliance)"
    $as_echo "{"
    $as_echo ""
    for level in ${levels}; do
	$as_echo "  if (TME_ARG_IS(compliance, \"${level}\")) { "
	$as_echo "    return (&tme_ieee754_ops_${level});"
	$as_echo "  }"
    done
    $as_echo "  return (NULL);"
    $as_echo "}"

    $as_echo ""
    $as_echo "/* this is a compliance options string: */"
    $as_echo_n "const char * const tme_ieee754_compliance_options = \"{ ";
    sep=
    for level in ${levels}; do
	$as_echo_n "${sep}${level}"
	sep=' | '
    done
    $as_echo " }\";"
fi

# done:
#
exit 0;
