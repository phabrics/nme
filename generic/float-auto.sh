#! /bin/sh
# Generated from ../../generic/float-auto.m4 by GNU Autoconf 2.69.
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


# $Id: float-auto.sh,v 1.2 2007/08/24 00:55:33 fredette Exp $

# generic/float-auto.sh - automatically generates C code for floating
# point conversion functions:

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

EOF
if $header; then :; else
    cat <<EOF
#include <tme/common.h>
_TME_RCSID("\$Id: float-auto.sh,v 1.2 2007/08/24 00:55:33 fredette Exp $");

/* includes: */
#include <tme/generic/float.h>

EOF
fi

# permute over the builtin types:
#
for _builtin_type in float double long_double; do

    # make the builtin type without underscores, and in all caps:
    #
    builtin_type=`$as_echo ${_builtin_type} | sed -e 's/_/ /g'`
    _BUILTIN_TYPE=`$as_echo ${_builtin_type} | tr 'a-z' 'A-Z'`

    # dispatch on the builtin type to open any protection:
    #
    case ${_builtin_type} in
    long_double)
	$as_echo ; $as_echo "#ifdef _TME_HAVE_${_BUILTIN_TYPE}" ;;
    *) ;;
    esac

    # if we're generating a header:
    #
    if $header; then
	cat <<EOF

/* if possible, this returns a positive or negative infinity
   ${builtin_type}, otherwise, this returns the ${builtin_type} value
   closest to that infinity: */
${builtin_type} tme_float_infinity_${_builtin_type} _TME_P((int));

/* if possible, this returns a negative zero ${builtin_type}.
   otherwise, this returns the negative ${builtin_type} value closest
   to zero: */
${builtin_type} tme_float_negative_zero_${_builtin_type} _TME_P((void));
EOF
    else
	cat <<EOF

/* if possible, this returns a positive or negative infinity
   ${builtin_type}, otherwise, this returns the ${builtin_type} value
   closest to that infinity: */
${builtin_type}
tme_float_infinity_${_builtin_type}(int negative)
{
  static int inf_set_${_builtin_type};
  static ${builtin_type} inf_${_builtin_type}[2];
  ${builtin_type} inf_test;
  int negative_i;

  /* make sure that negative can index the inf_${_builtin_type} array: */
  negative = !!negative;

  /* if the ${builtin_type} infinities have already been set: */
  if (__tme_predict_true(inf_set_${_builtin_type})) {
    return (inf_${_builtin_type}[negative]);
  }

  /* the ${builtin_type} infinities will be set now: */
  inf_set_${_builtin_type} = TRUE;

  /* set the positive and negative infinities: */
  for (negative_i = 0; negative_i < 2; negative_i++) {

    /* start with the limit maximum positive value or limit minimum
       negative value.  double this value until either it doesn't
       change or it isn't closer to the desired infinity, and then
       use the previous value: */
    inf_test = FLOAT_MAX_${_BUILTIN_TYPE};
    if (negative_i) {
      inf_test = -inf_test;
    }
    do {
      memcpy((char *) &inf_${_builtin_type}[negative_i], (char *) &inf_test, sizeof(inf_test));
      inf_test *= 2;
    } while (memcmp((char *) &inf_${_builtin_type}[negative_i], (char *) &inf_test, sizeof(inf_test)) != 0
             && (negative_i
                 ? inf_test < inf_${_builtin_type}[negative_i]
                 : inf_test > inf_${_builtin_type}[negative_i]));

    /* try to generate the actual infinity by dividing one or negative
       one by zero.  if this value is closer to the desired infinity,
       use it: */
    inf_test = (negative_i ? -1.0 : 1.0) / 0.0;
    if (negative_i
        ? inf_test < inf_${_builtin_type}[negative_i]
        : inf_test > inf_${_builtin_type}[negative_i]) {
      inf_${_builtin_type}[negative_i] = inf_test;
    }
  }

  /* return the desired infinity: */
  return (inf_${_builtin_type}[negative]);
}

/* if possible, this returns a negative zero ${builtin_type}.
   otherwise, this returns the negative ${builtin_type} value closest
   to zero: */
${builtin_type}
tme_float_negative_zero_${_builtin_type}(void)
{
  static int nzero_set_${_builtin_type};
  static ${builtin_type} nzero_${_builtin_type};
  ${builtin_type} constant_pzero;
  ${builtin_type} constant_nzero;
  ${builtin_type} nzero_test;

  /* if the ${builtin_type} negative zero has already been set: */
  if (__tme_predict_true(nzero_set_${_builtin_type})) {
    return (nzero_${_builtin_type});
  }

  /* the ${builtin_type} negative zero will be set now: */
  nzero_set_${_builtin_type} = TRUE;

  /* make a +0.0 and a -0.0, that we can do bit-for-bit comparisons with.
     NB that sizeof(${builtin_type}) may cover more bits than are actually
     used by a ${builtin_type}: */
  memset((char *) &constant_pzero, 0, sizeof(constant_pzero));
  memset((char *) &constant_nzero, 0, sizeof(constant_nzero));
  constant_pzero = +0.0;
  constant_nzero = -0.0;

  /* if -0.0 * -0.0 is bit-for-bit different from -0.0 and is
     bit-for-bit identical to +0.0, use -0.0: */
  memset((char *) &nzero_test, 0, sizeof(nzero_test));
  nzero_test = constant_nzero * constant_nzero;
  if (memcmp((char *) &constant_nzero, (char *) &nzero_test, sizeof(nzero_test)) != 0
      && memcmp((char *) &constant_pzero, (char *) &nzero_test, sizeof(nzero_test)) == 0) {
    return (nzero_${_builtin_type} = constant_nzero);
  }

  /* otherwise, start with the limit maximum negative value (which is
     zero minus the limit minimum positive value).  halve this value
     until either it doesn't change or it becomes positive zero, and
     then use the previous value: */
  nzero_test = 0 - FLOAT_MIN_${_BUILTIN_TYPE};
  do {
    memcpy((char *) &nzero_${_builtin_type}, (char *) &nzero_test, sizeof(nzero_test));
    nzero_test = nzero_test / 2;
  } while (memcmp((char *) &nzero_${_builtin_type}, (char *) &nzero_test, sizeof(nzero_test)) != 0
	   && memcmp((char *) &constant_pzero, (char *) &nzero_test, sizeof(nzero_test)) != 0);
  return (nzero_${_builtin_type});
}
EOF
    fi


    # permute over the radices:
    #
    for radix in 2 10; do

	# if we're generating a header:
	#
	if $header; then
	    cat <<EOF

/* this returns the radix ${radix} mantissa and exponent of an in-range ${builtin_type}.
   the mantissa is either zero, or in the range incl 1 to ${radix} excl: */
${builtin_type} tme_float_radix${radix}_mantissa_exponent_${_builtin_type} _TME_P((${builtin_type}, tme_int32_t *));

/* this scales a value by adding n to its radix ${radix} exponent: */
${builtin_type} tme_float_radix${radix}_scale_${_builtin_type} _TME_P((${builtin_type}, tme_int32_t));
EOF
	    continue
	fi

	# permute over the sign of the exponent:
	#
	for _sign in pos neg; do

	    # make the sign into two operators:
	    #
	    if test ${_sign} = pos; then sign= ; combine='*' ; else sign=- ; combine='/' ; fi

	    $as_echo ""
	    $as_echo "/* a series of ${builtin_type} values of the form ${radix}^${sign}x, where x is a power of two: */"
	    $as_echo "static const ${builtin_type} _tme_float_radix${radix}_exponent_bits_${_builtin_type}_${_sign}[] = {"
	    exponent=1
	    formats_last=

	    while true; do

		# dispatch on the radix to get the largest factor we will
		# use, its exponent, and a coarse upper bound on this
		# value's exponent in the worst-case radix of two:
		#
		case ${radix} in
		2)  exponent_radix2=${exponent} ; x=16777216 ; exponent_x=24 ;;
		10) exponent_radix2=`expr ${exponent} \* 4` ; x=10000 ; exponent_x=4 ;;
		*)
		    $as_echo "$PROG internal error: can't handle radix ${radix}" 1>&2
		    exit 1
		    ;;
		esac

		# we assume that all floating-point formats that use a
		# radix of two support at least positive and negative
		# exponents of magnitude 16.  if this exponent's
		# magnitude is greater than that, dispatch to get the
		# list of floating-point formats that support it:
		#
		formats=
		if test `expr ${exponent_radix2} \> 16` != 0; then

		    # the IEEE 754 types:
		    #
		    if test `expr ${exponent_radix2} \< 16384` != 0; then
			formats="${formats} | TME_FLOAT_FORMAT_IEEE754_EXTENDED80"
		    fi
		    if test `expr ${exponent_radix2} \< 1024` != 0; then
			formats="${formats} | TME_FLOAT_FORMAT_IEEE754_DOUBLE"
		    fi
		    if test `expr ${exponent_radix2} \< 128` != 0; then
			formats="${formats} | TME_FLOAT_FORMAT_IEEE754_SINGLE"
		    fi

		    # if we don't know any formats that support this
		    # exponent, stop now:
		    #
		    if test "x${formats}" = x; then
			break
		    fi

		    # clean up the formats:
		    #
		    formats="((TME_FLOAT_FORMAT_${_BUILTIN_TYPE} & ("`$as_echo "${formats}" | sed -e 's%^ | %%'`")) != 0)"
		fi

		# if the formats have changed:
		#
		if test "x${formats}" != "x${formats_last}"; then

		    # close any old #if first:
		    #
		    if test "x${formats_last}" != x; then
			$as_echo ""
			$as_echo "#endif /* ${formats_last} */"
		    fi

		    # open the new #if:
		    #
		    $as_echo ""
		    $as_echo "#if ${formats}"
		    formats_last=${formats}
		fi

		# compute this value:
		#
		$as_echo ""
		$as_echo "  /* ${radix}^${sign}${exponent}: */"
		exponent_remaining=${exponent}
		value=1
		while test ${exponent_remaining} != 0; do
		    if test `expr ${exponent_remaining} \>= ${exponent_x}` = 1; then
			value="(${value} ${combine} ((${builtin_type}) ((tme_uint32_t) ${x})))"
			exponent_remaining=`expr ${exponent_remaining} - ${exponent_x}`
		    else
			x=`expr ${x} / ${radix}`
			exponent_x=`expr ${exponent_x} - 1`
		    fi
		done
		$as_echo "  ${value},"

		# double the exponent:
		#
		exponent=`expr ${exponent} \* 2`
	    done

	    # close any #if:
	    #
	    if test "x${formats_last}" != x; then
		$as_echo ""
		$as_echo "#endif /* ${formats_last} */"
	    fi

	    $as_echo "};"
	done

cat <<EOF

/* this returns the radix ${radix} mantissa and exponent of an in-range ${builtin_type}.
   the mantissa is either zero, or in the range incl 1 to ${radix} excl: */
${builtin_type}
tme_float_radix${radix}_mantissa_exponent_${_builtin_type}(${builtin_type} value, tme_int32_t *_exponent)
{
  tme_int32_t exponent;
  tme_uint32_t exponent_bit;
  int negate;

  /* start with an exponent of zero: */
  exponent = 0;

  /* if the value is positive or negative zero, return the value: */
  if (value == 0.0
      || -value == 0.0) {
    *_exponent = exponent;
    return (value);
  }

  /* take the magnitude of the value, but remember if it was negative: */
  negate = (value < 0);
  if (negate) {
    value = 0 - value;
  }

  /* while the value is less than one: */
  exponent_bit = TME_ARRAY_ELS(_tme_float_radix${radix}_exponent_bits_${_builtin_type}_neg) - 1;
  for (; value < 1; ) {

    /* if value is less than or equal to ${radix}^-(2^exponent_bit),
       divide value by ${radix}^-(2^exponent_bit), and subtract 2^exponent_bit
       from exponent: */
    if (value <= _tme_float_radix${radix}_exponent_bits_${_builtin_type}_neg[exponent_bit]
        || exponent_bit == 0) {
      value /= _tme_float_radix${radix}_exponent_bits_${_builtin_type}_neg[exponent_bit];
      exponent -= (1 << exponent_bit);
    }

    /* otherwise, move to the next exponent bit: */
    else {
      exponent_bit--;
    }
  }

  /* while the value is greater than or equal to ${radix}: */
  exponent_bit = TME_ARRAY_ELS(_tme_float_radix${radix}_exponent_bits_${_builtin_type}_pos) - 1;
  for (; value >= ${radix}; ) {

    /* if value is greater than or equal to ${radix}^(2^exponent_bit),
       divide value by ${radix}^(2^exponent_bit), and add 2^exponent_bit
       to exponent: */
    if (value >= _tme_float_radix${radix}_exponent_bits_${_builtin_type}_pos[exponent_bit]
        || exponent_bit == 0) {
      value /= _tme_float_radix${radix}_exponent_bits_${_builtin_type}_pos[exponent_bit];
      exponent += (1 << exponent_bit);
    }

    /* otherwise, move to the next exponent bit: */
    else {
      exponent_bit--;
    }
  }

  /* done: */
  *_exponent = exponent;
  return (negate ? 0 - value : value);
}

/* this scales a value by adding n to its exponent: */
${builtin_type}
tme_float_radix${radix}_scale_${_builtin_type}(${builtin_type} value, tme_int32_t _n)
{
  tme_uint32_t exponent_bit, exponent;
  tme_uint32_t n;

  /* start with the most significant exponent bit: */
  exponent_bit = TME_ARRAY_ELS(_tme_float_radix${radix}_exponent_bits_${_builtin_type}_pos) - 1;
  exponent = (1 << exponent_bit);

  /* if n is negative: */
  if (_n < 0) {

    for (n = 0 - _n; n > 0;) {
      if (n >= exponent || exponent == 1) {
        value /= _tme_float_radix${radix}_exponent_bits_${_builtin_type}_pos[exponent_bit];
        n -= exponent;
      }
      else {
        exponent >>= 1;
        exponent_bit--;
      }
    }
  }

  /* otherwise, n is positive: */
  else {
    for (n = _n; n > 0;) {
      if (n >= exponent || exponent == 1) {
        value *= _tme_float_radix${radix}_exponent_bits_${_builtin_type}_pos[exponent_bit];
        n -= exponent;
      }
      else {
        exponent >>= 1;
        exponent_bit--;
      }
    }
  }

  return (value);
}
EOF
    done

    # dispatch on the type to close any protection:
    #
    case ${_builtin_type} in
    long_double)
	$as_echo ; $as_echo "#endif /* _TME_HAVE_${_BUILTIN_TYPE} */" ;;
    *) ;;
    esac

done

# done:
#
exit 0
