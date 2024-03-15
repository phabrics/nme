#! /bin/sh
# Generated from m6888x-auto.m4 by GNU Autoconf 2.72.
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


# $Id: m6888x-auto.sh,v 1.2 2007/08/25 20:41:10 fredette Exp $

# ic/m6888x-auto.sh - automatically generates C code for many m6888x
# emulation instructions:

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
_TME_RCSID("\$Id: m6888x-auto.sh,v 1.2 2007/08/25 20:41:10 fredette Exp $");

EOF

# placeholder for a permutation over FPU types:
#
fpu=m6888x

    # generate the FPgen opmode bitmap and table:
    #
    for what in bitmap table; do

	# when we're doing the m68040, we only make the bitmap:
	#
	if test ${fpu} = m68040 && test ${what} != bitmap; then
	    continue
	fi

	# dispatch on the what:
	#
	case ${what} in

	bitmap)
	    cat <<EOF

/* the ${fpu} FPgen opmode bitmap: */
const tme_uint8_t _tme_${fpu}_fpgen_opmode_bitmap[128 / 8] = {
EOF

	# reset the bitmap:
	#
	opmode_bitmap=0
	opmode_bit=1
	printf %s "  "
	;;

	table)
	    cat <<EOF

/* the ${fpu} FPgen opmode table: */
static const struct tme_m6888x_fpgen _tme_${fpu}_fpgen_opmode_table[128] = {
EOF
	    ;;
	esac

	# loop over the FPgen opmodes:
	#
	opmode=-1
	while test ${opmode} != 127; do
	    opmode=`expr \( ${opmode} \) + 1`

	    # if we're generating the opmode bitmap, and this opmode
	    # is in a new byte, emit the previous byte and reset for
	    # this byte:
	    #
	    if test ${what} = bitmap && test ${opmode_bit} = 256; then
		printf %s "${opmode_bitmap}, "
		opmode_bitmap=0
		opmode_bit=1
	    fi

	    # characterize this opmode:
	    #
	    name=''
	    optype=MONADIC
	    rounding_precision=CTL
	    rounding_mode=NULL
	    fpu_types=TME_M68K_FPU_ANY
	    name_ieee754=x
	    case "${opmode}" in

	    # fabs pp 305:
	    #
	    24) name=abs ;;
	    88) name=abs ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    92) name=abs ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    28) name=acos ; fpu_types='TME_M68K_FPU_M6888X' ;;	# facos pp 309

	    # fadd pp 312:
	    #
	    34) name=add ; optype=SRC_DST ;;
	    98) name=add ; optype=SRC_DST ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    102) name=add ; optype=SRC_DST ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    12) name=asin ; fpu_types=TME_M68K_FPU_M6888X ;;	# fasin pp 315
	    10) name=atan ; fpu_types=TME_M68K_FPU_M6888X ;;	# fatan pp 318
	    13) name=atanh ; fpu_types=TME_M68K_FPU_M6888X ;;	# fatanh pp 321
	    56) name=cmp ; name_ieee754= ; optype=DST_SRC ;;	# fcmp pp 326
	    29) name=cos ; fpu_types=TME_M68K_FPU_M6888X ;;	# fcos pp 329
	    25) name=cosh ; fpu_types=TME_M68K_FPU_M6888X ;;	# fcosh pp 332

	    # fdiv pp 337:
	    #
	    32) name=div ; optype=DST_SRC ;;
	    96) name=div ; optype=DST_SRC ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    100) name=div ; optype=DST_SRC ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    16) name=etox ; name_ieee754=exp ; fpu_types=TME_M68K_FPU_M6888X ;;	# fetox pp 341
	    8) name=etoxm1 ; name_ieee754=expm1 ; fpu_types=TME_M68K_FPU_M6888X ;;	# fetoxm1 pp 344
	    30) name=getexp ; fpu_types=TME_M68K_FPU_M6888X ;;	# fgetexp pp 347
	    31) name=getman ; fpu_types=TME_M68K_FPU_M6888X ;;	# fgetman pp 350
	    1) name=int ; name_ieee754=rint ; fpu_types=TME_M68K_FPU_M6888X ;;	# fint pp 353
	    3) name=intrz ; name_ieee754=rint ; rounding_mode=TO_ZERO ; fpu_types=TME_M68K_FPU_M6888X ;; # fintrz pp 356
	    21) name=log10 ; fpu_types=TME_M68K_FPU_M6888X ;;	# flog10 pp 359
	    22) name=log2 ; name_ieee754= ; fpu_types=TME_M68K_FPU_M6888X ;;	# flog2 pp 362
	    20) name=logn ; name_ieee754=log ; fpu_types=TME_M68K_FPU_M6888X ;;	# flogn pp 365
	    6) name=lognp1 ; name_ieee754=log1p ; fpu_types=TME_M68K_FPU_M6888X ;;	# flognp1 pp 368
	    33) name=mod ; name_ieee754= ; fpu_types=TME_M68K_FPU_M6888X ; optype=DST_SRC ;;	# fmod pp 371

	    # fmove pp 374:
	    #
	    0) name=move ;;
	    64) name=move ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    68) name=move ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    # fmul pp 395:
	    #
	    35) name=mul ; optype=SRC_DST ;;
	    99) name=mul ; optype=SRC_DST ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    103) name=mul ; optype=SRC_DST ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    # fneg pp 399:
	    #
	    26) name=neg ;;
	    90) name=neg ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    94) name=neg ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    37) name=rem ; name_ieee754= ; optype=DST_SRC ; fpu_types=TME_M68K_FPU_M6888X ;;	# frem pp 405
	    38) name=scale ; optype=DST_SRC ; fpu_types=TME_M68K_FPU_M6888X ;;	# fscale pp 408
	    36) name=sgldiv ; name_ieee754= ; optype=DST_SRC ;;	# fsgldiv pp 413
	    39) name=sglmul ; name_ieee754= ; optype=DST_SRC ;;	# fsglmul pp 416
	    14) name=sin ;;		# fsin pp 419

	    48|49|50|51|52|53|54|55) name=sincos ; fpu_types=TME_M68K_FPU_M6888X ; name_ieee754= ;;	# fsincos pp 422
	    2) name=sinh ; fpu_types=TME_M68K_FPU_M6888X ;;		# fsinh pp 426

	    # fsqrt pp 429:
	    #
	    4) name=sqrt ;;
	    65) name=sqrt ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    69) name=sqrt ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    # fsub pp 433:
	    #
	    40) name=sub ; optype=DST_SRC ;;
	    104) name=sub ; optype=DST_SRC ; rounding_precision=SINGLE ; fpu_types=TME_M68K_FPU_M68040 ;;
	    108) name=sub ; optype=DST_SRC ; rounding_precision=DOUBLE ; fpu_types=TME_M68K_FPU_M68040 ;;

	    15) name=tan ; fpu_types=TME_M68K_FPU_M6888X ;;		# ftan pp 437
	    9) name=tanh ; fpu_types=TME_M68K_FPU_M6888X ;;		# ftanh pp 440
	    18) name=tentox ; name_ieee754= ; fpu_types=TME_M68K_FPU_M6888X ;;	# ftentox pp 443
	    58) name=tst ; name_ieee754= ;;				# ftst pp 448
	    17) name=twotox ; name_ieee754= ; fpu_types=TME_M68K_FPU_M6888X ;;	# ftwotox pp 451

	    esac

	    # fix up the ieee754 name:
	    #
	    if test "x${name_ieee754}" = xx; then name_ieee754=${name}; fi

	    # dispatch on the what:
	    #
	    case ${what} in

	    bitmap)
		if test ${fpu_types} = TME_M68K_FPU_M68040 && test ${fpu} != m68040; then
		    name=
		fi
		if test "x${name}" != x; then
		    opmode_bitmap=`expr ${opmode_bitmap} + ${opmode_bit}`
		fi
		opmode_bit=`expr ${opmode_bit} \* 2`
		;;

	    table)

		printf "%s\n" ""
		printf "%s\n" "  /* opmode ${opmode}: */"
		printf %s "  { "

		# the function:
		#
		if test "x${name}" = x; then
		    printf %s "NULL"
		    printf %s ", 0"
		    fpu_types=TME_M68K_FPU_NONE
		elif test "x${name_ieee754}" != x; then
		    printf %s "NULL"
		    printf %s ", TME_M6888X_IEEE754_OP(tme_ieee754_ops_extended80_${name_ieee754})"
		else
		    printf %s "_tme_m6888x_f${name}"
		    printf %s ", 0"
		fi

		# the m6888x types:
		#
		printf %s ", ${fpu_types}"

		# the operation type:
		#
		if test ${optype} != MONADIC; then optype="DYADIC_${optype}"; fi
		printf %s ", TME_M6888X_OPTYPE_${optype}"

		# the rounding mode:
		#
		printf %s ", TME_FLOAT_ROUND_${rounding_mode}"

		# the rounding precision:
		#
		printf %s ", TME_M6888X_ROUNDING_PRECISION_${rounding_precision}"

		printf "%s\n" " },"
		;;
	    esac
	done

	# dispatch on the what:
	#
	case ${what} in

	bitmap)
	    printf "%s\n" "${opmode_bitmap}"
	    printf "%s\n" "};"
	    ;;

	table)
	    printf "%s\n" "};"
	    ;;
	esac

    done


# done:
#
exit 0
