#! /bin/sh
# Generated from ../../../ic/ieee754/ieee754-precision.m4 by GNU Autoconf 2.69.
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


# $Id: ieee754-precision.sh,v 1.2 2007/08/26 14:02:04 fredette Exp $

# ic/ieee754/ieee754-precision.sh - emits information about IEEE 754 types
# in a form usable by scripts that automatically generate code:

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

# get the precision's SoftFloat name, its integral type, its size
# in bytes, any member name for its sign+exponent word, the
# exponent mask for the sign+exponent word, the number of fraction
# bits, and the member names and masks for its fraction chunks:
#
precision=$1
prefix=$2
case $1 in
single)
    cat <<EOF
${prefix}precision_sf=float32 ;
${prefix}integral=tme_uint32_t ;
${prefix}constant=tme_uint32_t ;
${prefix}size=4 ;
${prefix}sexp= ;
${prefix}mask_exp=0x7f800000 ;
${prefix}fracbits=23 ;
${prefix}implicit=true ;
${prefix}chunk_member_0= ; chunk_mask_0=0x007f0000 ;
${prefix}chunk_member_1= ; chunk_mask_1=0x0000ffff ;
${prefix}chunk_member_2=x ;
EOF
    ;;
double)
    cat <<EOF
${prefix}precision_sf=float64 ;
${prefix}integral='union tme_value64' ;
${prefix}constant='struct tme_ieee754_double_constant' ;
${prefix}size=8 ;
${prefix}sexp=.tme_value64_uint32_hi ;
${prefix}mask_exp=0x7ff00000 ;
${prefix}fracbits=52 ;
${prefix}implicit=true ;
${prefix}chunk_member_0=.tme_value64_uint32_hi ; chunk_mask_0=0x000f0000 ;
${prefix}chunk_member_1=.tme_value64_uint32_hi ; chunk_mask_1=0x0000ffff ;
${prefix}chunk_member_2=.tme_value64_uint32_lo ; chunk_mask_2=0xffff0000 ;
${prefix}chunk_member_3=.tme_value64_uint32_lo ; chunk_mask_3=0x0000ffff ;
${prefix}chunk_member_4=x ;
EOF
    ;;
extended80)
    cat <<EOF
${prefix}precision_sf=floatx80 ;
${prefix}integral='struct tme_float_ieee754_extended80' ;
${prefix}constant='struct tme_ieee754_extended80_constant' ;
${prefix}size=12 ;
${prefix}sexp=.tme_float_ieee754_extended80_sexp ;
${prefix}mask_exp=0x7fff ;
${prefix}fracbits=63 ;
${prefix}implicit=false ;
${prefix}chunk_member_0=.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi ; chunk_mask_0=0xffff0000 ;
${prefix}chunk_member_1=.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi ; chunk_mask_1=0x0000ffff ;
${prefix}chunk_member_2=.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo ; chunk_mask_2=0xffff0000 ;
${prefix}chunk_member_3=.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo ; chunk_mask_3=0x0000ffff ;
${prefix}chunk_member_4=x ;
EOF
    ;;
quad)
    cat <<EOF
${prefix}precision_sf=float128 ;
${prefix}integral='struct tme_float_ieee754_quad' ;
${prefix}constant='struct tme_ieee754_quad_constant' ;
${prefix}size=16 ;
${prefix}sexp=.tme_float_ieee754_quad_hi.tme_value64_uint32_hi ;
${prefix}mask_exp=0x7fff0000 ;
${prefix}fracbits=112 ;
${prefix}implicit=true ;
${prefix}chunk_member_0=.tme_float_ieee754_quad_hi.tme_value64_uint32_hi ; chunk_mask_0=0x0000ffff ;
${prefix}chunk_member_1=.tme_float_ieee754_quad_hi.tme_value64_uint32_lo ; chunk_mask_1=0xffff0000 ;
${prefix}chunk_member_2=.tme_float_ieee754_quad_hi.tme_value64_uint32_lo ; chunk_mask_2=0x0000ffff ;
${prefix}chunk_member_3=.tme_float_ieee754_quad_lo.tme_value64_uint32_hi ; chunk_mask_3=0xffff0000 ;
${prefix}chunk_member_4=.tme_float_ieee754_quad_lo.tme_value64_uint32_hi ; chunk_mask_4=0x0000ffff ;
${prefix}chunk_member_5=.tme_float_ieee754_quad_lo.tme_value64_uint32_lo ; chunk_mask_5=0xffff0000 ;
${prefix}chunk_member_6=.tme_float_ieee754_quad_lo.tme_value64_uint32_lo ; chunk_mask_6=0x0000ffff ;
${prefix}chunk_member_7=x ;
EOF
    ;;
esac

# to avoid integer overflow warnings, make sure that the exponent
# mask is always unsigned:
#
$as_echo ${prefix}'mask_exp="((tme_uint32_t) ${'${prefix}'mask_exp})" ;'

# a mask for the sign bit can be derived from the exponent mask:
#
$as_echo ${prefix}'mask_sign="(${'${prefix}'mask_exp} + _TME_FIELD_MASK_FACTOR(${'${prefix}'mask_exp}))" ; '

# the maximum biased exponent can be derived from the exponent mask:
#
$as_echo ${prefix}'exp_biased_max="(${'${prefix}'mask_exp} / _TME_FIELD_MASK_FACTOR(${'${prefix}'mask_exp}))" ; '

# the exponent bias can be derived from the maximum biased exponent:
#
$as_echo ${prefix}'exp_bias="(${'${prefix}'exp_biased_max} >> 1)" ; '

# make a capitalized version of the precision name:
#
$as_echo ${prefix}'capprecision=`$as_echo ${precision} | tr a-z A-Z` ; '

# done:
#
exit 0;
