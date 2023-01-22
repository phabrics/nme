#! /bin/sh
# Generated from ../../../ic/sparc/sparc-insns-auto.m4 by GNU Autoconf 2.69.
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


# $Id: sparc-insns-auto.sh,v 1.10 2010/06/05 16:13:41 fredette Exp $

# ic/sparc/sparc-insns-auto.sh - automatically generates C code
# for many SPARC emulation instructions:

#
# Copyright (c) 2005 Matt Fredette
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
_TME_RCSID("\$Id: sparc-insns-auto.sh,v 1.10 2010/06/05 16:13:41 fredette Exp $");

EOF
if $header; then :; else
    cat <<EOF
#include "sparc-impl.h"

/* an all-bits-zero float for use with _tme_sparc*_fpu_mem_fpreg(): */
#if TME_FLOAT_FORMAT_NULL != 0
#error "TME_FLOAT_FORMAT_NULL changed"
#endif
static struct tme_float _tme_sparc_float_null;
EOF
fi

# permute over architecture:
#
for arch in 32 64; do

    # the sparc64 support depends on a 64-bit integer type:
    #
    if test ${arch} = 64; then
	$as_echo ""
	$as_echo "#ifdef TME_HAVE_INT64_T"
    fi

    # get the name of the register with the integer condition codes, a
    # shift value for 32-bit registers, and the architecture version:
    #
    if test ${arch} = 32; then
	ccr=32_PSR
	ccr_ireg='tme_sparc32_ireg_psr'
	reg32_shift=''
	version=8
    else
	ccr=64_CCR
	ccr_ireg='tme_sparc64_ireg_ccr'
	reg32_shift=' << 1'
	version=9
    fi

    # fix the architecture version:
    #
    if $header; then :; else
	$as_echo ""
	$as_echo "#undef TME_SPARC_VERSION"
	$as_echo "#define TME_SPARC_VERSION(ic) (${version})"
    fi

    # the alternate ASI function:
    #
    if $header; then :; else
	$as_echo ""
	$as_echo "static tme_uint32_t"
	$as_echo "_tme_sparc${arch}_alternate_asi_mask(struct tme_sparc *ic)"
	$as_echo "{"
	$as_echo "  unsigned int asi_data;"
	$as_echo "  unsigned int asi_mask_flags;"
	$as_echo "  tme_uint32_t asi_mask_data;"
	$as_echo ""
	$as_echo "  /* get the ASI, assuming that the i bit is zero: */"
	$as_echo "  asi_data = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0xff << 5));"
	if test ${arch} = 32; then
	    $as_echo ""
	    $as_echo "  /* this is a privileged instruction: */"
	    $as_echo "  TME_SPARC_INSN_PRIV;"
	    $as_echo ""
	    $as_echo "  /* if the i bit is one, this is an illegal instruction: */"
	    $as_echo "  if (__tme_predict_false(TME_SPARC_INSN & TME_BIT(13))) {"
	    $as_echo "    TME_SPARC_INSN_ILL(ic);"
	    $as_echo "  }"
	    $as_echo ""
	    $as_echo "  /* get the flags for this ASI: */"
	    $as_echo "  asi_mask_flags = ic->tme_sparc_asis[asi_data].tme_sparc_asi_mask_flags;"
	    $as_echo ""
	    $as_echo "  /* make the ASI mask: */"
	    $as_echo "  if (asi_mask_flags & TME_SPARC32_ASI_MASK_FLAG_SPECIAL) {"
	    $as_echo "    asi_mask_data"
	    $as_echo "      = TME_SPARC_ASI_MASK_SPECIAL(asi_data, TRUE);"
	    $as_echo "  }"
	    $as_echo "  else {"
	    $as_echo "    asi_mask_data = TME_SPARC32_ASI_MASK(asi_data, asi_data);"
	    $as_echo "  }"
	else
	    $as_echo ""
	    $as_echo "  /* if the i bit is one, use the address space in the ASI register: */"
	    $as_echo "  if (TME_SPARC_INSN & TME_BIT(13)) {"
	    $as_echo "    asi_data = ic->tme_sparc64_ireg_asi;"
	    $as_echo "  }"
	    $as_echo ""
	    $as_echo "  /* get the flags for this ASI: */"
	    $as_echo "  asi_mask_flags = ic->tme_sparc_asis[asi_data].tme_sparc_asi_mask_flags;"

	    $as_echo ""
	    $as_echo "  /* if this is a nonprivileged access: */"
	    $as_echo "  if (!TME_SPARC_PRIV(ic)) {"
	    $as_echo ""
	    $as_echo "    /* if this is a restricted ASI: */"
	    $as_echo "    if (__tme_predict_false((asi_data & TME_SPARC64_ASI_FLAG_UNRESTRICTED) == 0)) {"
	    $as_echo ""
	    $as_echo "      /* force a slow load or store, which will generate the"
	    $as_echo "         privileged_action trap: */"
	    $as_echo "      asi_mask_flags |= TME_SPARC_ASI_MASK_FLAG_UNDEF;"
	    $as_echo "    }"
	    $as_echo ""
	    $as_echo "    /* force a nonprivileged access with the ASI: */"
	    $as_echo "    asi_mask_flags |= TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER;"
	    $as_echo "  }"
	    $as_echo ""
	    $as_echo "  /* make the ASI mask: */"
	    $as_echo "  if (asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_SPECIAL) {"
	    $as_echo "    asi_mask_data"
	    $as_echo "      = (asi_mask_flags"
	    $as_echo "         + TME_SPARC_ASI_MASK_SPECIAL(asi_data,"
	    $as_echo "                                      ((asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER) == 0)));"
	    $as_echo "  }"
	    $as_echo "  else {"
	    $as_echo "    asi_mask_data = TME_SPARC64_ASI_MASK(asi_data, asi_mask_flags);"
	    $as_echo "  }"
	fi
	$as_echo ""
	$as_echo "  /* if this ASI has a special handler: */"
	$as_echo "  if (__tme_predict_false(ic->tme_sparc_asis[TME_SPARC_ASI_MASK_WHICH(asi_mask_data)].tme_sparc_asi_handler != 0)) {"
	$as_echo ""
	$as_echo "    /* force a slow load or store, which will call the special handler: */"
	$as_echo "    asi_mask_data |= TME_SPARC_ASI_MASK_FLAG_UNDEF;"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  return (asi_mask_data);"
	$as_echo "}"
    fi

    # the FPU load and store common function:
    #
    if $header; then :; else
	$as_echo ""
	$as_echo "static struct tme_float *"
	$as_echo "_tme_sparc${arch}_fpu_mem_fpreg(struct tme_sparc *ic,"
	$as_echo "                           tme_uint32_t misaligned,"
	$as_echo "                           struct tme_float *float_buffer)"
	$as_echo "{"
	$as_echo "  unsigned int float_format;"
	$as_echo "  unsigned int fpreg_format;"
	$as_echo "  tme_uint32_t fp_store;"
	$as_echo "  unsigned int fpu_mode;"
	$as_echo "  unsigned int fpreg_number;"
	$as_echo ""
	$as_echo "  /* NB: this checks for various traps by their priority order: */"
	$as_echo ""
	$as_echo "  TME_SPARC_INSN_FPU_ENABLED;"
	$as_echo ""
	$as_echo "  /* get the floating-point format: */"
	$as_echo "  float_format = float_buffer->tme_float_format;"
	$as_echo ""
	$as_echo "  /* convert the floating-point format into the ieee754"
	$as_echo "     floating-point register file format: */"
	$as_echo "#if (TME_FLOAT_FORMAT_NULL | TME_IEEE754_FPREG_FORMAT_NULL) != 0"
	$as_echo "#error \"TME_FLOAT_FORMAT_ or TME_IEEE754_FPREG_FORMAT_ values changed\""
	$as_echo "#endif"
	$as_echo "#if TME_FLOAT_FORMAT_IEEE754_SINGLE < TME_IEEE754_FPREG_FORMAT_SINGLE"
	$as_echo "#error \"TME_FLOAT_FORMAT_ or TME_IEEE754_FPREG_FORMAT_ values changed\""
	$as_echo "#endif"
	$as_echo "#if (TME_FLOAT_FORMAT_IEEE754_SINGLE / TME_IEEE754_FPREG_FORMAT_SINGLE) != (TME_FLOAT_FORMAT_IEEE754_DOUBLE / TME_IEEE754_FPREG_FORMAT_DOUBLE)"
	$as_echo "#error \"TME_FLOAT_FORMAT_ or TME_IEEE754_FPREG_FORMAT_ values changed\""
	$as_echo "#endif"
	$as_echo "  assert (float_format == TME_FLOAT_FORMAT_NULL"
	$as_echo "          || float_format == TME_FLOAT_FORMAT_IEEE754_SINGLE"
	$as_echo "          || float_format == TME_FLOAT_FORMAT_IEEE754_DOUBLE);"
	$as_echo "  fpreg_format = float_format / (TME_FLOAT_FORMAT_IEEE754_SINGLE / TME_IEEE754_FPREG_FORMAT_SINGLE);"
	$as_echo ""
	$as_echo "  /* if the memory address is misaligned, return the"
	$as_echo "     float buffer now.  the eventual load or store will"
	$as_echo "     cause the mem_address_not_aligned trap: */"
	$as_echo ""
	$as_echo "  /* if the memory address is misaligned: */"
	$as_echo "#if TME_IEEE754_FPREG_FORMAT_NULL != 0 || TME_IEEE754_FPREG_FORMAT_SINGLE != 1 || TME_IEEE754_FPREG_FORMAT_DOUBLE != 2 || TME_IEEE754_FPREG_FORMAT_QUAD != 4"
	$as_echo "#error \"TME_IEEE754_FPREG_FORMAT_ values changed\""
	$as_echo "#endif"
	$as_echo "  assert (fpreg_format == TME_IEEE754_FPREG_FORMAT_NULL"
	$as_echo "          || fpreg_format == TME_IEEE754_FPREG_FORMAT_SINGLE"
	$as_echo "          || fpreg_format == TME_IEEE754_FPREG_FORMAT_DOUBLE"
	$as_echo "          || fpreg_format == TME_IEEE754_FPREG_FORMAT_QUAD);"
	$as_echo "  misaligned &= ((sizeof(tme_uint32_t) * fpreg_format) - 1);"
	$as_echo "  if (__tme_predict_false(misaligned)) {"
	$as_echo ""
	if test ${arch} = 32; then
	    $as_echo "    return (float_buffer);"
	else
	    $as_echo "    /* if the memory address is not even 32-bit aligned, or"
	    $as_echo "       if this SPARC doesn't support loads and stores of this"
	    $as_echo "       size at 32-bit alignment: */"
	    $as_echo "    if (misaligned != sizeof(tme_uint32_t)"
	    $as_echo "#if TME_IEEE754_FPREG_FORMAT_SINGLE != 1 || (TME_SPARC_MEMORY_FLAG_HAS_LDDF_STDF_32 * TME_IEEE754_FPREG_FORMAT_DOUBLE) != TME_SPARC_MEMORY_FLAG_HAS_LDQF_STQF_32"
	    $as_echo "#error \"TME_IEEE754_FPREG_FORMAT_ or TME_SPARC_MEMORY_FLAG_ values changed\""
	    $as_echo "#endif"
	    $as_echo "        || (TME_SPARC_MEMORY_FLAGS(ic)"
	    $as_echo "            & (TME_SPARC_MEMORY_FLAG_HAS_LDDF_STDF_32 * fpreg_format)) == 0) {"
	    $as_echo ""
	    $as_echo "      return (float_buffer);"
	    $as_echo "    }"
	fi
        $as_echo "  }"
	$as_echo ""
	$as_echo "  /* see if this is a floating-point load or store: */"
	$as_echo "  /* NB: all of the floating-point instructions that use"
	$as_echo "     this preamble have bit two of op3 clear for a load,"
	$as_echo "     and set for a store: */"
	$as_echo "  fp_store = (TME_SPARC_INSN & TME_BIT(19 + 2));"
	$as_echo ""
	$as_echo "  /* if the FPU isn't in execute mode: */"
	$as_echo "  fpu_mode = ic->tme_sparc_fpu_mode;"
	$as_echo "  if (__tme_predict_false(fpu_mode != TME_SPARC_FPU_MODE_EXECUTE)) {"
	$as_echo ""
	$as_echo "    /* if this is a floating-point load, or if this is a"
	$as_echo "       floating-point store and a floating-point exception"
	$as_echo "       is pending: */"
	$as_echo "    if (!fp_store"
	$as_echo "        || fpu_mode == TME_SPARC_FPU_MODE_EXCEPTION_PENDING) {"
	$as_echo ""
	$as_echo "      /* do an FPU exception check: */"
	$as_echo "      tme_sparc_fpu_exception_check(ic);"
	$as_echo "    }"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* if this is not a load or store of a floating-point register: */"
	$as_echo "  if (fpreg_format == TME_IEEE754_FPREG_FORMAT_NULL) {"
	$as_echo "    return (float_buffer);"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* decode rd: */"
	$as_echo "  fpreg_number"
	$as_echo "    = tme_sparc_fpu_fpreg_decode(ic,"
	$as_echo "                                 TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN,"
	$as_echo "                                                         TME_SPARC_FORMAT3_MASK_RD),"
	$as_echo "                                 fpreg_format);"
	$as_echo ""
	$as_echo "  /* make sure this floating-point register has the right precision: */"
	$as_echo "  tme_sparc_fpu_fpreg_format(ic, fpreg_number, fpreg_format | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
	$as_echo ""
	$as_echo "  /* if this is a floating-point load: */"
	$as_echo "  if (!fp_store) {"
	$as_echo ""
	$as_echo "    /* mark rd as dirty: */"
	$as_echo "    TME_SPARC_FPU_DIRTY(ic, fpreg_number);"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* return the floating-point register: */"
	$as_echo "  return (&ic->tme_sparc_fpu_fpregs[fpreg_number]);"
	$as_echo "}"
	$as_echo "#define _tme_sparc${arch}_fpu_mem(ic) \\"
	$as_echo "  do { _tme_sparc${arch}_fpu_mem_fpreg(ic, 0, &_tme_sparc_float_null); } while (/* CONSTCOND */ 0)"
    fi

    # permute over instruction:
    #
    case ${arch} in
    32) insns_arch= ;;
    64) insns_arch="ldx stx ldqf stqf ldxa stxa ldfa lddfa stfa stdfa ldqfa stqfa casa casxa mulx sdivx udivx" ;;
    esac
    for insn in \
	add \
	addcc \
	sub \
	subcc \
	or \
	orcc \
	orn \
	orncc \
	and \
	andcc \
	andn \
	andncc \
	xor \
	xorcc \
	xnor \
	xnorcc \
	addx \
	addxcc \
	subx \
	subxcc \
	taddcc taddcctv \
	tsubcc tsubcctv \
	umul umulcc smul smulcc \
	udiv udivcc sdiv sdivcc \
	sll \
	srl \
	sra \
	ldb stb \
	ldh sth \
	ld st \
	ldd std \
	ldstub ldstuba swap swapa \
	ldba stba \
	ldha stha \
	lda sta \
	ldda stda \
	jmpl \
	ldf lddf ldfsr \
	stf stdf stfsr \
	fpop1 fpop2 \
	mulscc \
	${insns_arch} \
	; do

	# if we're making the header, just emit declarations:
	#
	if $header; then
	    $as_echo "TME_SPARC_FORMAT3_DECL(tme_sparc${arch}_${insn}, tme_uint${arch}_t);"
	    continue
	fi

	# an ALU instruction:
	#
	case ${insn} in
	add | sub | or | orn | and | andn | xor | xnor | \
	addx | subx | umul | smul | udiv | sdiv)
	    cc=false
	    ;;
	addcc | subcc | orcc | orncc | andcc | andncc | xorcc | xnorcc | \
	addxcc | subxcc | umulcc | smulcc | udivcc | sdivcc | \
	taddcc | tsubcc | taddcctv | tsubcctv | \
	mulscc)
	    cc=true
	    ;;
	*)
	    cc=
	    ;;
	esac
	if test "x${cc}" != x; then

	    # characterize each function:
	    #
	    sign=u ; size_src=${arch} ; size_dst=${arch} ; arith=logical ; with_c= ; tagged=
	    case "${insn}" in
	    add  | addcc)   op='src1 + src2' ; arith=add ;;
	    sub  | subcc)   op='src1 - src2' ; arith=sub ;;
	    or   | orcc)    op='src1 | src2' ;;
	    orn  | orncc)   op='src1 | ~src2' ;;
	    and  | andcc)   op='src1 & src2' ;;
	    andn | andncc)  op='src1 & ~src2' ;;
	    xor  | xorcc)   op='src1 ^ src2' ;;
	    xnor | xnorcc)  op='src1 ^ ~src2' ;;
	    addx | addxcc)  op='src1 + src2' ; arith=add ; with_c=+ ;;
	    subx | subxcc)  op='src1 - src2' ; arith=sub ; with_c=- ;;
	    taddcc)         op='src1 + src2' ; arith=add ; tagged=x ;;
	    taddcctv)       op='src1 + src2' ; arith=add ; tagged=tv ;;
	    tsubcc)         op='src1 - src2' ; arith=sub ; tagged=x ;;
	    tsubcctv)       op='src1 - src2' ; arith=sub ; tagged=tv ;;
	    umul | umulcc)  arith=mul ; size_src=32 ;;
	    smul | smulcc)  arith=mul ; size_src=32 ; sign= ;;
	    udiv | udivcc)  arith=udiv ; size_src=32 ; size_dst=32 ;;
	    sdiv | sdivcc)  arith=sdiv ; size_src=32 ; sign= ;;
	    mulscc)	    arith=add ; size_src=32 ; size_dst=32 ;;
	    *) $as_echo "$0 internal error: unknown ALU function ${insn}" 1>&2 ; exit 1 ;;
	    esac

	    # open the function:
	    #
	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} \"${insn} SRC1, SRC2, DST\": */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"

	    # declare our locals:
	    #
	    $as_echo "  tme_${sign}int${size_src}_t src1;"
	    $as_echo "  tme_${sign}int${size_src}_t src2;"
	    $as_echo "  tme_${sign}int${size_dst}_t dst;"
	    case "${insn}" in
	    umul* | smul* | udiv* | sdiv*)
		$as_echo "  tme_${sign}int64_t val64;"
		;;
	    mulscc)
		$as_echo "  tme_uint32_t y;"
		;;
	    esac
	    if ${cc}; then
		$as_echo "  tme_uint32_t cc;"
	    fi
	    cc_plus=

	    $as_echo ""
	    $as_echo "  /* get the operands: */"
	    $as_echo "  src1 = (tme_${sign}int${arch}_t) TME_SPARC_FORMAT3_RS1;"
	    $as_echo "  src2 = (tme_${sign}int${arch}_t) TME_SPARC_FORMAT3_RS2;"

	    $as_echo ""
	    $as_echo "  /* perform the operation: */"
	    case "${insn}" in
	    umul | umulcc | smul | smulcc)
		$as_echo "  val64 = (((tme_${sign}int64_t) src1) * src2);"
		$as_echo "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift}) = (((tme_uint64_t) val64) >> 32);"
		$as_echo "  dst = ((tme_${sign}int64_t) val64);"
		;;
	    udiv | udivcc | sdiv | sdivcc)
		$as_echo "  val64 = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift});"
		$as_echo "  val64 = (val64 << 32) + (tme_uint32_t) src1;"
	        $as_echo "  if (__tme_predict_false(src2 == 0)) {"
		$as_echo "    tme_sparc${arch}_trap(ic, TME_SPARC${arch}_TRAP_division_by_zero);"
		$as_echo "  }"
		$as_echo "  val64 /= src2;"
		$as_echo "  dst = (tme_${sign}int32_t) val64;"
		$as_echo ""
		$as_echo "  /* if the division overflowed: */"
		$as_echo "  if (dst != val64) {"
		$as_echo ""
		$as_echo "    /* return the largest appropriate value: */"
		if test "x${sign}" = xu; then
		    $as_echo "    dst = 0xffffffff;"
		else
		    $as_echo "    dst = (tme_int32_t) ((val64 < 0) + (tme_uint32_t) 0x7fffffff);"
		fi
		if ${cc}; then
		    $as_echo ""
		    $as_echo "    /* set V: */"
		    $as_echo "    cc = TME_SPARC${ccr}_ICC_V;"
		fi
		$as_echo "  }"
		if ${cc}; then
		    $as_echo ""
		    $as_echo "  /* otherwise, the division didn't overflow: */"
		    $as_echo "  else {"
		    $as_echo ""
		    $as_echo "    /* clear V: */"
		    $as_echo "    cc = !TME_SPARC${ccr}_ICC_V;"
		    $as_echo "  }"
		    cc_plus='+'
		fi
		;;
	    mulscc)
		$as_echo ""
		$as_echo "  /* \"(1) The multiplier is established as r[rs2] if the i field is zero, or "
		$as_echo "     sign_ext(simm13) if the i field is one.\""
		$as_echo ""
		$as_echo "     \"(3) If the least significant bit of the Y register = 1, the shifted"
		$as_echo "     value from step (2) is added to the multiplier. If the LSB of the"
		$as_echo "     Y register = 0, then 0 is added to the shifted value from step (2).\" */"
		$as_echo "  y = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift});"
		$as_echo "  if ((y & 1) == 0) {"
		$as_echo "    src2 = 0;"
		$as_echo "  }"
		$as_echo ""
		$as_echo "  /* \"(6) The Y register is shifted right by one bit, with the LSB of the"
		$as_echo "     unshifted r[rs1] replacing the MSB of Y.\" */"
		$as_echo "  y >>= 1;"
		$as_echo "  if (src1 & 1) {"
		$as_echo "    y += 0x80000000;"
		$as_echo "  }"
		$as_echo "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift}) = y;"
		$as_echo ""
		$as_echo "  /* \"(2) A 32-bit value is computed by shifting r[rs1] right by one"
		$as_echo "     bit with (N xor V) from the PSR replacing the high-order bit."
		$as_echo "     (This is the proper sign for the previous partial product.)\" */"
		$as_echo "  src1 >>= 1;"
		$as_echo "  if (((ic->${ccr_ireg} ^ (ic->${ccr_ireg} * (TME_SPARC${ccr}_ICC_N / TME_SPARC${ccr}_ICC_V))) & TME_SPARC${ccr}_ICC_N) != 0) {"
		$as_echo "    src1 += 0x80000000;"
		$as_echo "  }"
		$as_echo ""
		$as_echo "  /* \"(4) The sum from step (3) is written into r[rd].\" */"
		$as_echo "  dst = src1 + src2;"
		$as_echo ""
		$as_echo "  /* \"(5) The integer condition codes, icc, are updated according to the"
		$as_echo "     addition performed in step (3).\" */"
		;;
	    *)
		$as_echo "  dst = ${op};"
		if test "x${with_c}" != x; then
		    $as_echo "  dst ${with_c}= ((ic->${ccr_ireg} & TME_SPARC${ccr}_ICC_C) != 0);"
		fi
	    esac

	    # unless this is a tagged-and-trap-on-overflow operation:
	    #
	    if test "x${tagged}" != xtv; then
		$as_echo ""
		$as_echo "  /* store the destination: */"
		$as_echo "  TME_SPARC_FORMAT3_RD = (tme_${sign}int${arch}_t) dst;"
	    fi

	    # if this instruction modifies the condition codes:
	    #
	    if ${cc}; then

		# set the 32-bit, and possibly the 64-bit, condition codes:
		#
		arch_cc=16
		while test `expr ${arch_cc} \< ${arch}` = 1; do
		    arch_cc=`expr ${arch_cc} \* 2`

		    case ${arch_cc} in
		    32) xcc=ICC ;;
		    64) xcc=XCC ;;
		    esac

		    $as_echo ""
		    $as_echo "  /* set Z if the destination is zero: */"
		    $as_echo "  cc ${cc_plus}= ((((tme_int${arch_cc}_t) dst) == 0) * TME_SPARC${ccr}_${xcc}_Z);"
		    cc_plus='+'

		    if test `expr ${arch_cc} \<= ${size_dst}` = 1; then
			$as_echo ""
			$as_echo "  /* set N if the destination is negative: */"
			$as_echo "  cc += ((((tme_int${arch_cc}_t) dst) < 0) * TME_SPARC${ccr}_${xcc}_N);"
		    fi

		    case $arith in
		    add)
			ones="(((tme_${sign}int${arch_cc}_t) 0) - 1)"

			$as_echo ""
			$as_echo "  /* if the operands are the same sign, and the destination has"
			$as_echo "     a different sign, set V: */"
			$as_echo "  cc += ((((tme_int${arch_cc}_t) ((src2 ^ dst) & (src1 ^ (src2 ^ ${ones})))) < 0) * TME_SPARC${ccr}_${xcc}_V);"

			$as_echo ""
			$as_echo "  /* if src1 and src2 both have the high bit set, or if dst does"
			$as_echo "     not have the high bit set and either src1 or src2 does, set C: */"
			$as_echo "  cc += (((tme_int${arch_cc}_t) (((tme_uint${arch_cc}_t) (src1 & src2)) | ((((tme_uint${arch_cc}_t) dst) ^ ${ones}) & ((tme_uint${arch_cc}_t) (src1 | src2))))) < 0) * TME_SPARC${ccr}_${xcc}_C;"
			;;
		    sub)

			$as_echo ""
			$as_echo "  /* if the operands are different signs, and the destination has"
			$as_echo "     a different sign from the first operand, set V: */"
			$as_echo "  cc += ((((tme_int${arch_cc}_t) ((src1 ^ src2) & (src1 ^ dst))) < 0) * TME_SPARC${ccr}_${xcc}_V);"

			$as_echo ""
			$as_echo "  /* if src2 is greater than src1, set C: */"
			$as_echo_n "  cc += ((((tme_uint${arch_cc}_t) src2) > ((tme_uint${arch_cc}_t) src1))"
			if test "x${with_c}" != x; then
			    $as_echo_n " || (((tme_uint${arch_cc}_t) src2) == ((tme_uint${arch_cc}_t) src1) && (ic->${ccr_ireg} & TME_SPARC${ccr}_ICC_C))"
			fi
			$as_echo ") * TME_SPARC${ccr}_${xcc}_C;"
			;;
		    logical | mul)
			;;
		    udiv | sdiv)
		        # the udivcc and sdivcc V are handled in the operation code
			;;
		    *) $as_echo "$0 internal error: unknown arithmetic type ${arith}" 1>&2 ; exit 1 ;;
		    esac
		done

		# if this is a tagged operation:
		#
		if test "x${tagged}" != x; then

		    $as_echo ""
		    $as_echo "  /* set V if bits zero or one of src1 or src2 are set: */"
		    $as_echo "  cc |= ((((src1 | src2) & 3) != 0) * TME_SPARC${ccr}_ICC_V);"

		    # if this is a tagged-and-trap-on-overflow operation:
		    #
		    if test "x${tagged}" = xtv; then

			$as_echo ""
			$as_echo "  /* trap on a tagged overflow: */"
			$as_echo "  if (cc & TME_SPARC${ccr}_ICC_V) {"
			$as_echo "    tme_sparc${arch}_trap(ic, TME_SPARC${arch}_TRAP_tag_overflow);"
			$as_echo "  }"

			$as_echo "  /* store the destination: */"
			$as_echo "  TME_SPARC_FORMAT3_RD = (tme_${sign}int${arch}_t) dst;"
		    fi
		fi

		$as_echo ""
		$as_echo "  /* set the condition codes: */"
		$as_echo_n "  ic->${ccr_ireg} = "
		if test ${arch} = 32; then
		    $as_echo_n "(ic->${ccr_ireg} & ~TME_SPARC32_PSR_ICC) | "
		fi
		$as_echo "cc;"
	    fi

	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# a shift instruction:
	#
	case ${insn} in
	sll | srl | sra)

	    # get the sign of this shift:
	    #
	    if test ${insn} = sra; then sign= ; else sign=u; fi

	    $as_echo ""
	    $as_echo "/* the sparc${arch} ${insn} function: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_${sign}int${arch}_t dst;"
	    $as_echo "  unsigned int count;"
	    $as_echo ""
	    $as_echo "  /* get the value and the shift count: */"
	    $as_echo "  dst = TME_SPARC_FORMAT3_RS1;"
	    $as_echo "  count = TME_SPARC_FORMAT3_RS2;"

	    # if we're on sparc64:
	    #
	    if test ${arch} = 64; then

		$as_echo ""
		$as_echo "  /* if the X bit is clear: */"
		$as_echo "  if ((TME_SPARC_INSN & TME_BIT(12)) == 0) {"
		$as_echo ""
		$as_echo "    /* limit the count: */"
		$as_echo "    count %= 32;"
		if test ${insn} != sll; then
		    $as_echo ""
		    $as_echo "    /* clip the value to 32 bits: */"
		    $as_echo "    dst = (tme_${sign}int32_t) dst;"
		fi
		$as_echo "  }"
	    fi

	    $as_echo ""
	    $as_echo "  /* limit the count: */"
	    $as_echo "  count %= ${arch};"
	    $as_echo ""
	    $as_echo "  /* do the shift: */"
	    if test "${insn}" = sra; then
		$as_echo "#ifdef SHIFTSIGNED_INT${arch}_T"
	    fi
	    $as_echo "#if defined(SHIFTMAX_INT${arch}_T) && (SHIFTMAX_INT${arch}_T < (${arch} - 1))"
	    $as_echo "#error \"cannot do full shifts of a tme_int${arch}_t\""
	    $as_echo "#endif /* (SHIFTMAX_INT${arch}_T < (${arch} - 1)) */"
	    if test ${insn} = sll; then
		$as_echo "  dst <<= count;"
	    else
		$as_echo "  dst >>= count;"
	    fi
	    if test "${insn}" = sra; then
		$as_echo "#else  /* !SHIFTSIGNED_INT${arch}_T */"
		$as_echo "  for (; count-- > 0; ) {"
		$as_echo "    dst = (dst & ~((tme_${sign}int${arch}_t) 1)) / 2;"
		$as_echo "  }"
		$as_echo "#endif /* !SHIFTSIGNED_INT${arch}_T */"
	    fi

	    $as_echo ""
	    $as_echo "  /* store the destination: */"
	    $as_echo "  TME_SPARC_FORMAT3_RD = dst;"
	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	    ;;
	esac

	# the sdivx, udivx, and mulx instructions:
	#
	case ${insn} in
	sdivx) sign= ;;
	udivx | mulx) sign=u ;;
	*) sign=x ;;
	esac
	if test "x${sign}" != xx; then
	    $as_echo ""
	    $as_echo "/* the sparc${arch} ${insn} function: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_${sign}int64_t src1;"
	    $as_echo "  tme_${sign}int64_t src2;"
	    $as_echo "  tme_${sign}int64_t dst;"
	    $as_echo ""
	    $as_echo "  /* get the operands: */"
	    $as_echo "  src1 = TME_SPARC_FORMAT3_RS1;"
	    $as_echo "  src2 = TME_SPARC_FORMAT3_RS2;"
	    $as_echo ""
	    $as_echo "  /* do the ${insn}: */"
	    if test ${insn} = mulx; then
		$as_echo "  dst = src1 * src2;"
	    else
		$as_echo "  if (__tme_predict_false(src2 == 0)) {"
		$as_echo "    tme_sparc${arch}_trap(ic, TME_SPARC${arch}_TRAP_division_by_zero);"
		$as_echo "  }"
		if test ${insn} = sdivx; then
		    $as_echo "  dst = (src2 == -1 && src1 == (((tme_int64_t) 1) << 63) ? src1 : src1 / src2);"
		else
		    $as_echo "  dst = src1 / src2;"
		fi
	    fi
	    $as_echo ""
	    $as_echo "  TME_SPARC_FORMAT3_RD = dst;"
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# a load or store instruction:
	#
	size=
	case "${insn}" in
	ldb | stb | ldstub | ldba | stba | ldstuba) size=8 ;;
	ldh | sth | ldha | stha) size=16 ;;
	ld | st | swap | lda | sta | swapa) size=32 ;;
	ldd | std | ldda | stda) size=32 ;;
	ldx | stx | ldxa | stxa | casxa) size=64 ;;
	casa) size=32 ;;
	esac
	if test "x${size}" != x; then

	    # set the alternate space indication:
	    #
	    case "${insn}" in
	    *a) alternate=true ;;
	    *) alternate=false ;;
	    esac

	    # set the atomic and double indications:
	    #
	    atomic=false
	    double=false
	    case "${insn}" in
	    ldstub | ldstuba | swap | swapa | casa | casxa) atomic=true ;;
	    ldd* | std*) double=true ; size=64 ;;
	    esac

	    # if this is only a load, we are reading, otherwise we are writing:
	    #
	    cycle=write
	    capcycle=WRITE
	    slow=store
	    case "${insn}" in
	    ldstub*) ;;
	    ld*)
		cycle=read
		capcycle=READ
		slow=load
		;;
	    esac

	    # start the instruction:
	    #
	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"

	    # our locals:
	    #
	    if ${alternate}; then
		$as_echo "  tme_uint32_t asi_mask_data;"
		asi_mask_data=asi_mask_data
	    else
		asi_mask_data="ic->tme_sparc_asi_mask_data"
	    fi
	    $as_echo "  tme_uint${arch}_t address;"
	    if test ${arch} = 64 && ${alternate}; then
		$as_echo "  tme_bus_context_t context;"
		context=context
	    else
		context="ic->tme_sparc_memory_context_default"
	    fi
	    $as_echo "  tme_uint32_t asi_mask_flags_slow;"
	    $as_echo "  struct tme_sparc_tlb *dtlb;"
	    $as_echo_n "  "
	    if test ${slow} = load; then $as_echo_n "const "; fi
	    $as_echo "tme_shared tme_uint8_t *memory;"
	    $as_echo "  tme_bus_context_t dtlb_context;"
	    $as_echo "  tme_uint32_t endian_little;"
	    case "${insn}" in
	    ldd* | std* | swap*) $as_echo "  tme_uint32_t value32;" ;;
	    ldstub*) ;;
	    cas*a)
	        $as_echo "  unsigned int reg_rs2;"
	        $as_echo "  tme_uint${size}_t value_compare${size};"
	        $as_echo "  tme_uint${size}_t value_swap${size};"
	        $as_echo "  tme_uint${size}_t value_read${size};"
		;;
	    ld*)
	        $as_echo "  tme_uint${size}_t value${size};"
		size_extend=${arch}
		if test `expr ${size} \< ${arch}` = 1; then
		    if test `expr ${size} \< 32` = 1; then size_extend=32; fi
		    $as_echo "  tme_uint${size_extend}_t value${size_extend};"
		fi
		;;
	    st*) $as_echo "  tme_uint${size}_t value${size};" ;;
	    esac

	    if ${alternate}; then
		$as_echo ""
		$as_echo "  /* get the alternate ASI mask: */"
		$as_echo "  asi_mask_data = _tme_sparc${arch}_alternate_asi_mask(ic);"
	    fi

	    $as_echo ""
	    $as_echo "  /* get the address: */"
	    case "${insn}" in
	    cas*a) $as_echo "  address = TME_SPARC_FORMAT3_RS1;" ;;
	    *) $as_echo "  address = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;" ;;
	    esac
	    if test ${arch} = 64; then
		$as_echo "  address &= ic->tme_sparc_address_mask;"
	    fi

	    $as_echo ""
	    $as_echo "#ifdef _TME_SPARC_STATS"
	    $as_echo "  /* track statistics: */"
	    $as_echo "  ic->tme_sparc_stats.tme_sparc_stats_memory_total++;"
	    $as_echo "#endif /* _TME_SPARC_STATS */"

	    $as_echo ""
	    $as_echo "  /* verify and maybe replay this transfer: */"
	    if ${double}; then verify_size=32; else verify_size=${size}; fi
	    case "${insn}" in
	    ld*) verify_flags=TME_SPARC_RECODE_VERIFY_MEM_LOAD ;;
	    swap* | cas*a) verify_flags="TME_SPARC_RECODE_VERIFY_MEM_LOAD | TME_SPARC_RECODE_VERIFY_MEM_STORE" ;;
	    st*) verify_flags=TME_SPARC_RECODE_VERIFY_MEM_STORE ;;
	    *) verify_flags= ;;
	    esac
	    $as_echo "  tme_sparc_recode_verify_mem(ic, &TME_SPARC_FORMAT3_RD,"
	    $as_echo "                              ${asi_mask_data}, address,"
	    $as_echo "                              (TME_RECODE_SIZE_${verify_size}"
	    $as_echo "                               | ${verify_flags}));"
	    if ${double}; then
		$as_echo "  tme_sparc_recode_verify_mem(ic, &TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}),"
		$as_echo "                              ${asi_mask_data}, address + sizeof(tme_uint32_t),"
		$as_echo "                              (TME_RECODE_SIZE_${verify_size}"
		$as_echo "                               | ${verify_flags}));"
	    fi
	    $as_echo "  if (tme_sparc_recode_verify_replay_last_pc(ic) != 0) {"
	    $as_echo "    TME_SPARC_INSN_OK;"
	    $as_echo "  }"

	    # if this is some kind of a store, except for an ldstub:
	    #
	    case "${insn}" in
	    std*)
		$as_echo ""
		$as_echo "  /* log the values stored: */"
		$as_echo "  tme_sparc_log(ic, 1000, TME_OK, "
		$as_echo "               (TME_SPARC_LOG_HANDLE(ic),"
		$as_echo "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%08\" TME_PRIx32 \" 0x%08\" TME_PRIx32),"
		$as_echo "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		$as_echo "                address,"
		$as_echo "                (tme_uint32_t) TME_SPARC_FORMAT3_RD,"
		$as_echo "                (tme_uint32_t) TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch})));"
		;;
	    st* | swap* | cas*a)
		$as_echo ""
		$as_echo "  /* log the value stored: */"
		$as_echo "  tme_sparc_log(ic, 1000, TME_OK, "
		$as_echo "               (TME_SPARC_LOG_HANDLE(ic),"
		$as_echo "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${size} / 4`"\" TME_PRIx${size}),"
		$as_echo "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		$as_echo "                address,"
		$as_echo "                (tme_uint${size}_t) TME_SPARC_FORMAT3_RD));"
		;;
	    esac

	    if test "${context}" = context; then
		$as_echo ""
		$as_echo "  /* get the context: */"
		if test ${arch} = 64; then
		    $as_echo "  context = ic->tme_sparc_memory_context_primary;"
		    $as_echo "  if (__tme_predict_false(${asi_mask_data}"
		    $as_echo "                          & (TME_SPARC64_ASI_FLAG_SECONDARY"
		    $as_echo "                             + TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS))) {"
		    $as_echo "    if (${asi_mask_data} & TME_SPARC64_ASI_FLAG_SECONDARY) {"
		    $as_echo "      context = ic->tme_sparc_memory_context_secondary;"
		    $as_echo "    }"
		    $as_echo "    else if (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS) {"
		    $as_echo "      context = 0;"
		    $as_echo "    }"
		    $as_echo "  }"
		fi
	    fi

	    $as_echo ""
	    $as_echo "  /* assume that no DTLB ASI mask flags will require a slow ${slow}: */"
	    $as_echo "  asi_mask_flags_slow = 0;"
	    if test ${arch} = 64; then

		$as_echo ""
		if test ${slow} != load || ${atomic}; then
		    $as_echo "  /* a ${insn} traps on no-fault addresses: */"
		else
		    $as_echo "  /* a ${insn} without a no-fault ASI traps on no-fault addresses: */"
		fi
		$as_echo "  asi_mask_flags_slow |= TME_SPARC64_ASI_FLAG_NO_FAULT;"
		if ${atomic}; then
		    $as_echo ""
		    $as_echo "  /* a ${insn} traps on uncacheable addresses with side-effects: */"
		    $as_echo "  asi_mask_flags_slow |= TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE;"
		fi

		if ${alternate}; then
		    $as_echo ""
		    $as_echo "  /* if this ${insn} is using a no-fault ASI: */"
		    $as_echo "  if (__tme_predict_false(${asi_mask_data} & TME_SPARC64_ASI_FLAG_NO_FAULT)) {"
		    $as_echo ""
		    if test ${slow} != load || ${atomic}; then
			$as_echo "    /* a ${insn} with a no-fault ASI traps: */"
			$as_echo "    asi_mask_flags_slow = 0 - (tme_uint32_t) 1;"
		    else
			$as_echo "    /* a ${insn} with a no-fault ASI traps on addresses with side-effects: */"
			$as_echo "    asi_mask_flags_slow = TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS;"
		    fi
		    $as_echo "  }"
		fi
	    fi

	    $as_echo ""
	    $as_echo "  /* get and busy the DTLB entry: */"
	    $as_echo "  dtlb = &ic->tme_sparc_tlbs[TME_SPARC_DTLB_ENTRY(ic, TME_SPARC_TLB_HASH(ic, ${context}, address))];"
	    $as_echo "  tme_sparc_tlb_busy(dtlb);"

	    $as_echo ""
	    $as_echo "  /* assume that this DTLB applies and allows fast transfers: */"
	    $as_echo "  memory = dtlb->tme_sparc_tlb_emulator_off_${cycle};"

	    $as_echo ""
	    $as_echo "  /* if this DTLB matches any context, it matches this context: */"
	    $as_echo "  dtlb_context = dtlb->tme_sparc_tlb_context;"
	    $as_echo "  if (dtlb_context > ic->tme_sparc_memory_context_max) {"
	    $as_echo "    dtlb_context = ${context};"
	    $as_echo "  }"

	    $as_echo ""
	    $as_echo "  /* we must call the slow ${slow} function if: */"
	    $as_echo "  if (__tme_predict_false("
	    $as_echo ""
	    $as_echo "                          /* the DTLB entry is invalid: */"
	    $as_echo "                          tme_bus_tlb_is_invalid(&dtlb->tme_sparc_tlb_bus_tlb)"
	    $as_echo ""
	    $as_echo "                          /* the DTLB entry does not match the context: */"
	    $as_echo "                          || dtlb_context != ${context}"
	    $as_echo ""
	    $as_echo "                          /* the DTLB entry does not cover the needed addresses: */"
	    $as_echo "                          || (address < (tme_bus_addr${arch}_t) dtlb->tme_sparc_tlb_addr_first)"
	    $as_echo "                          || ((address + ((${size} / 8) - 1)) > (tme_bus_addr${arch}_t) dtlb->tme_sparc_tlb_addr_last)"
	    $as_echo ""
	    $as_echo "                          /* the DTLB entry does not cover the needed address space: */"
	    $as_echo "                          || (!TME_SPARC_TLB_ASI_MASK_OK(dtlb, ${asi_mask_data}))"
	    $as_echo ""
	    $as_echo "                          /* the DTLB entry can't be used for a fast ${insn}: */"
	    $as_echo "                          || (dtlb->tme_sparc_tlb_asi_mask & asi_mask_flags_slow) != 0"
	    $as_echo ""
	    $as_echo "                          /* the DTLB entry does not allow fast transfers: */"
	    if $atomic; then
		$as_echo "                          || (memory != dtlb->tme_sparc_tlb_emulator_off_read)"
	    fi
	    $as_echo "                          || (memory == TME_EMULATOR_OFF_UNDEF)"
	    if test ${size} != 8; then
		$as_echo ""
		$as_echo "                          /* the address is misaligned: */"
		$as_echo "                          || ((address % (${size} / 8)) != 0)"
	    fi
	    if ${double}; then
		$as_echo ""
		$as_echo "                          /* the destination register number is odd: */"
		$as_echo "                          || ((TME_SPARC_INSN & TME_BIT(25)) != 0)"
	    fi
	    $as_echo ""
	    $as_echo "                          )) {"

	    $as_echo ""
	    $as_echo "    /* call the slow ${slow} function: */"
	    $as_echo "    memory = tme_sparc${arch}_ls(ic,"
	    $as_echo "                            address,"
	    $as_echo "                            &TME_SPARC_FORMAT3_RD,"
	    $as_echo_n "                            (TME_SPARC_LSINFO_OP_"
	    if ${atomic}; then
		$as_echo "ATOMIC"
	    elif test ${slow} = store; then
		$as_echo "ST"
	    else
		$as_echo "LD"
	    fi
	    $as_echo_n "                             | "
	    case ${insn} in
	    ldd* | std*)
		$as_echo "TME_SPARC_LSINFO_LDD_STD"
		$as_echo_n "                             | "
		;;
	    esac
	    if ${alternate}; then
		$as_echo "TME_SPARC_LSINFO_ASI(TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF))"
		$as_echo "                             | TME_SPARC_LSINFO_A"
		$as_echo_n "                             | "
	    fi
	    $as_echo "(${size} / 8)));"

	    if test ${slow} = store || ${alternate}; then
		$as_echo ""
		$as_echo "    /* if the slow ${slow} function did the transfer: */"
		$as_echo "    if (__tme_predict_false(memory == TME_EMULATOR_OFF_UNDEF)) {"
		$as_echo ""
		$as_echo "      /* unbusy the TLB entry; */"
		$as_echo "      tme_sparc_tlb_unbusy(dtlb);"

	    	# if this is some kind of a load, log the value loaded:
	    	#
		case ${insn} in
		ldd*)
		    $as_echo ""
		    $as_echo "      /* log the value loaded: */"
		    $as_echo "      tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		    $as_echo "      tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}));"
		    $as_echo "      tme_sparc_log(ic, 1000, TME_OK,"
		    $as_echo "                   (TME_SPARC_LOG_HANDLE(ic),"
		    $as_echo "                    _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \" 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \"\"),"
		    $as_echo "                    TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		    $as_echo "                    address,"
		    $as_echo "                    TME_SPARC_FORMAT3_RD,"
		    $as_echo "                    TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch})));"
		    ;;
		ld* | ldstub* | swap* | cas*a)
		    $as_echo ""
		    $as_echo "      /* log the value loaded: */"
		    $as_echo "      tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		    $as_echo "      tme_sparc_log(ic, 1000, TME_OK,"
		    $as_echo "                   (TME_SPARC_LOG_HANDLE(ic),"
		    $as_echo "                    _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${size} / 4`"\" TME_PRIx${arch}),"
		    $as_echo "                    TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		    $as_echo "                    address,"
		    $as_echo "                    TME_SPARC_FORMAT3_RD));"
		    ;;
		esac
		$as_echo ""
		$as_echo "      TME_SPARC_INSN_OK;"
		$as_echo "    }"
	    fi
	    $as_echo "  }"

	    $as_echo ""
	    $as_echo "  /* get the byte order of this transfer: */"
	    if test ${arch} = 32; then
		$as_echo "  endian_little = FALSE;"
	    elif test ${arch} = 64; then
		$as_echo "  endian_little = ${asi_mask_data} & TME_SPARC64_ASI_FLAG_LITTLE;"
		$as_echo "  if (__tme_predict_false(dtlb->tme_sparc_tlb_asi_mask & TME_SPARC64_ASI_FLAG_LITTLE)) {"
		$as_echo "    if (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_INVERT_ENDIAN) {"
		$as_echo "      endian_little ^= TME_SPARC64_ASI_FLAG_LITTLE;"
		$as_echo "    }"
		$as_echo "    else {"
		$as_echo "      assert (FALSE);"
		$as_echo "    }"
		$as_echo "  }"
	    fi

	    $as_echo ""
	    $as_echo "  /* do the fast transfer: */"
	    $as_echo "  memory += address;"

	    # dispatch on the instruction:
	    #
	    case "${insn}" in
	    ldd*)
		$as_echo "  value32 = tme_memory_bus_read32(((const tme_shared tme_uint32_t *) memory) + 0, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 2, sizeof(tme_uint${arch}_t));"
		$as_echo "  value32 = (endian_little ? tme_letoh_u32(value32) : tme_betoh_u32(value32));"
		$as_echo "  TME_SPARC_FORMAT3_RD = value32;"
		$as_echo "  value32 = tme_memory_bus_read32(((const tme_shared tme_uint32_t *) memory) + 1, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 1, sizeof(tme_uint${arch}_t));"
		$as_echo "  value32 = (endian_little ? tme_letoh_u32(value32) : tme_betoh_u32(value32));"
		$as_echo "  TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}) = value32;"
		;;
	    std*)
	        $as_echo "  value32 = TME_SPARC_FORMAT3_RD;"
		$as_echo "  value32 = (endian_little ? tme_htole_u32(value32) : tme_htobe_u32(value32));"
		$as_echo "  tme_memory_bus_write32(((tme_shared tme_uint32_t *) memory) + 0, value32, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 2, sizeof(tme_uint${arch}_t));"
		$as_echo "  value32 = TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch});"
		$as_echo "  value32 = (endian_little ? tme_htole_u32(value32) : tme_htobe_u32(value32));"
		$as_echo "  tme_memory_bus_write32(((tme_shared tme_uint32_t *) memory) + 1, value32, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 1, sizeof(tme_uint${arch}_t));"
		;;
	    ldstub*)
		$as_echo "  TME_SPARC_FORMAT3_RD = tme_memory_atomic_xchg8(memory, 0xff, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint8_t));"
		;;
	    swap*)
		$as_echo "  value32 = TME_SPARC_FORMAT3_RD;"
		$as_echo "  value32 = (endian_little ? tme_htole_u32(value32) : tme_htobe_u32(value32));"
		$as_echo "  value32 = tme_memory_atomic_xchg32((tme_shared tme_uint${size}_t *) memory, value32, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint8_t));"
		$as_echo "  value32 = (endian_little ? tme_letoh_u32(value32) : tme_betoh_u32(value32));"
		$as_echo "  TME_SPARC_FORMAT3_RD = value32;"
		;;
	    cas*a)
	        $as_echo "  reg_rs2 = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RS2);"
		$as_echo "  TME_SPARC_REG_INDEX(ic, reg_rs2);"
		$as_echo "  value_compare${size} = ic->tme_sparc_ireg_uint${arch}(reg_rs2);"
		$as_echo "  value_compare${size} = (endian_little ? tme_htole_u${size}(value_compare${size}) : tme_htobe_u${size}(value_compare${size}));"
		$as_echo "  value_swap${size} = TME_SPARC_FORMAT3_RD;"
		$as_echo "  value_swap${size} = (endian_little ? tme_htole_u${size}(value_swap${size}) : tme_htobe_u${size}(value_swap${size}));"
		$as_echo "  value_read${size} = tme_memory_atomic_cx${size}((tme_shared tme_uint${size}_t *) memory, value_compare${size}, value_swap${size}, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint${size}_t));"
		$as_echo "  value_read${size} = (endian_little ? tme_letoh_u${size}(value_read${size}) : tme_betoh_u${size}(value_read${size}));"
		$as_echo "  TME_SPARC_FORMAT3_RD = value_read${size};"
		;;
	    ld*)
		$as_echo "  value${size} = tme_memory_bus_read${size}((const tme_shared tme_uint${size}_t *) memory, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint${size}_t), sizeof(tme_uint${arch}_t));"
		if test ${size} != 8; then
		    $as_echo "  value${size} = (endian_little ? tme_letoh_u${size}(value${size}) : tme_betoh_u${size}(value${size}));"
		fi
		if test `expr ${size} \< ${arch}` = 1; then
		    $as_echo ""
		    $as_echo "  /* possibly sign-extend the loaded value: */"
		    $as_echo "  value${size_extend} = value${size};"
		    $as_echo "  if (TME_SPARC_INSN & TME_BIT(22)) {"
		    $as_echo "    value${size_extend} = (tme_uint${size_extend}_t) (tme_int${size_extend}_t) (tme_int${size}_t) value${size_extend};"
		    $as_echo "  }"
		fi
		$as_echo "  TME_SPARC_FORMAT3_RD = (tme_uint${arch}_t) (tme_int${arch}_t) (tme_int${size_extend}_t) value${size_extend};"
		;;
	    st*)
		$as_echo "  value${size} = TME_SPARC_FORMAT3_RD;"
		if test ${size} != 8; then
		    $as_echo "  value${size} = (endian_little ? tme_htole_u${size}(value${size}) : tme_htobe_u${size}(value${size}));"
		fi
		$as_echo "  tme_memory_bus_write${size}((tme_shared tme_uint${size}_t *) memory, value${size}, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint${size}_t), sizeof(tme_uint${arch}_t));"
		;;
	    *) $as_echo "$PROG internal error: unknown memory insn ${insn}" 1>&2 ; exit 1 ;;
	    esac

	    $as_echo ""
	    $as_echo "  /* unbusy the DTLB entry: */"
	    $as_echo "  tme_sparc_tlb_unbusy(dtlb);"

	    # if this is some kind of a load, log the value loaded:
	    #
	    case ${insn} in
	    ldd*)
		$as_echo ""
		$as_echo "  /* log the value loaded: */"
		$as_echo "  tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		$as_echo "  tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}));"
		$as_echo "  tme_sparc_log(ic, 1000, TME_OK,"
		$as_echo "               (TME_SPARC_LOG_HANDLE(ic),"
		$as_echo "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \" 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \"\"),"
		$as_echo "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		$as_echo "                address,"
		$as_echo "                TME_SPARC_FORMAT3_RD,"
		$as_echo "                TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch})));"
		;;
	    ld* | ldstub* | swap* | cas*a)
		$as_echo ""
		$as_echo "  /* log the value loaded: */"
		$as_echo "  tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		$as_echo "  tme_sparc_log(ic, 1000, TME_OK,"
		$as_echo "               (TME_SPARC_LOG_HANDLE(ic),"
		$as_echo "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${size} / 4`"\" TME_PRIx${arch}),"
		$as_echo "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		$as_echo "                address,"
		$as_echo "                TME_SPARC_FORMAT3_RD));"
		;;
	    esac

	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# the jmpl instruction:
	#
	if test ${insn} = jmpl; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_uint${arch}_t pc_next_next;"
	    $as_echo "  tme_uint32_t ls_faults;"
	    $as_echo ""
	    $as_echo "  /* \"The JMPL instruction causes a register-indirect delayed control"
	    $as_echo "     transfer to the address given by r[rs1] + r[rs2] if the i field is"
	    $as_echo "     zero, or r[rs1] + sign_ext(simm13) if the i field is one. The JMPL"
	    $as_echo "     instruction copies the PC, which contains the address of the JMPL"
	    $as_echo "     instruction, into register r[rd]. If either of the low-order two"
	    $as_echo "     bits of the jump address is nonzero, a mem_address_not_aligned"
	    $as_echo "     trap occurs.\" */"
	    $as_echo ""
	    $as_echo "  /* get the target address: */"
	    $as_echo "  pc_next_next = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;"
	    if test ${arch} = 64; then
		$as_echo "  pc_next_next &= ic->tme_sparc_address_mask;"
	    fi
	    $as_echo ""
	    $as_echo "  /* set the delayed control transfer: */"
	    $as_echo "  ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;"
	    $as_echo ""
	    $as_echo "  /* check the target address: */"
	    $as_echo "  ls_faults = TME_SPARC_LS_FAULT_NONE;"
	    if test ${arch} = 64; then
		$as_echo "  if (__tme_predict_false((pc_next_next"
		$as_echo "                           + ic->tme_sparc${arch}_ireg_va_hole_start)"
		$as_echo "                          > ((ic->tme_sparc${arch}_ireg_va_hole_start * 2) - 1))) {"
		$as_echo "    ls_faults += TME_SPARC64_LS_FAULT_VA_RANGE_NNPC;"
		$as_echo "  }"
	    fi
	    $as_echo "  if (__tme_predict_false((pc_next_next % sizeof(tme_uint32_t)) != 0)) {"
	    $as_echo "    ls_faults += TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;"
	    $as_echo "  }"
	    $as_echo "  if (__tme_predict_false(ls_faults != TME_SPARC_LS_FAULT_NONE)) {"
	    $as_echo "    tme_sparc_nnpc_trap(ic, ls_faults);"
	    $as_echo "  }"
	    $as_echo ""
	    $as_echo "  /* write the PC of the jmpl into r[rd]: */"
	    $as_echo "  TME_SPARC_FORMAT3_RD = ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_PC);"
	    $as_echo ""
	    $as_echo "  /* log an indirect call instruction, which has 15 (%o7) for rd: */"
	    $as_echo "  if (TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD) == 15) {"
	    $as_echo "    tme_sparc_log(ic, 250, TME_OK,"
	    $as_echo "                  (TME_SPARC_LOG_HANDLE(ic),"
	    $as_echo "                   _(\"call 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch}),"
	    $as_echo "                   pc_next_next));"
	    $as_echo "  }"
	    $as_echo ""
	    $as_echo "  /* log a ret or retl instruction, which has 0 (%g0) for rd,"
	    $as_echo "     either 31 (%i7) or 15 (%o7) for rs1, and 8 for simm13: */"
	    $as_echo "  else if ((TME_SPARC_INSN | (16 << 14))"
	    $as_echo "           == ((tme_uint32_t) (0x2 << 30) | (0 << 25) | (0x38 << 19) | (31 << 14) | (0x1 << 13) | 8)) {"
	    $as_echo "    tme_sparc_log(ic, 250, TME_OK,"
	    $as_echo "                  (TME_SPARC_LOG_HANDLE(ic),"
	    $as_echo "                   _(\"retl 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch}),"
	    $as_echo "                   pc_next_next));"
	    $as_echo "  }"
	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# this may be an alternate-space version of a floating-point
	# load or store instruction:
	#
	case ${insn} in *a) alternate=a ;; *) alternate= ;; esac

	# the ldf instruction:
	#
	if test ${insn} = "ldf${alternate}"; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_uint32_t misaligned;"
	    $as_echo "  struct tme_float float_buffer;"
	    $as_echo "  struct tme_float *fpreg;"
	    $as_echo ""
	    $as_echo "  /* get the least significant 32 bits of the address: */"
	    $as_echo "  misaligned = TME_SPARC_FORMAT3_RS1;"
	    $as_echo "  misaligned += (tme_uint32_t) TME_SPARC_FORMAT3_RS2;"
	    if test "${arch}${alternate}" = 64a; then
		$as_echo ""
		$as_echo "  /* see if the address is misaligned for the ASI: */"
		$as_echo "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    $as_echo ""
	    $as_echo "  /* decode rd: */"
	    $as_echo "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
	    $as_echo "  fpreg"
	    $as_echo "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    $as_echo "                                 misaligned,"
	    $as_echo "                                 &float_buffer);"
	    $as_echo ""
	    $as_echo "  /* do the load: */"
	    $as_echo "  tme_sparc${arch}_ld${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    $as_echo ""
	    $as_echo "  /* set the floating-point register value: */"
	    $as_echo "  assert (fpreg != &float_buffer);"
	    $as_echo "  fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
	    $as_echo "  fpreg->tme_float_value_ieee754_single"
	    $as_echo "    = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift});"
	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# the stf instruction:
	#
	if test ${insn} = "stf${alternate}"; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_uint32_t misaligned;"
	    $as_echo "  struct tme_float float_buffer;"
	    $as_echo "  const struct tme_float *fpreg;"
	    $as_echo "  const tme_uint32_t *value_single;"
	    $as_echo ""
	    $as_echo "  /* get the least significant 32 bits of the address: */"
	    $as_echo "  misaligned = TME_SPARC_FORMAT3_RS1;"
	    $as_echo "  misaligned += (tme_uint32_t) TME_SPARC_FORMAT3_RS2;"
	    if test "${arch}${alternate}" = 64a; then
		$as_echo ""
		$as_echo "  /* see if the address is misaligned for the ASI: */"
		$as_echo "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    $as_echo ""
	    $as_echo "  /* decode rd: */"
	    $as_echo "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
	    $as_echo "  fpreg"
	    $as_echo "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    $as_echo "                                 misaligned,"
	    $as_echo "                                 &float_buffer);"
	    $as_echo ""
	    $as_echo "  /* get this single floating-point register in IEEE754 single-precision format: */"
	    $as_echo "  value_single = tme_ieee754_single_value_get(fpreg, &float_buffer.tme_float_value_ieee754_single);"
	    $as_echo ""
	    $as_echo "  /* set the floating-point register value: */"
	    $as_echo "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}) = *value_single;"
	    $as_echo ""
	    $as_echo "  /* do the store: */"
	    $as_echo "  tme_sparc${arch}_st${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    $as_echo ""
	    $as_echo "  assert (fpreg != &float_buffer);"
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# the lddf instruction:
	#
	if test ${insn} = "lddf${alternate}"; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_uint${arch}_t address;"
	    $as_echo "  tme_uint32_t misaligned;"
	    $as_echo "  struct tme_float float_buffer;"
	    $as_echo "  struct tme_float *fpreg;"
	    if test ${arch} != 32; then
		$as_echo "  tme_uint${arch}_t offset;"
	    fi
	    $as_echo ""
	    $as_echo "  /* get the address: */"
	    $as_echo "  address = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;"
	    $as_echo ""
	    $as_echo "  /* get the least significant 32 bits of the address: */"
	    $as_echo "  misaligned = address;"
	    if test "${arch}${alternate}" = 64a; then
		$as_echo ""
		$as_echo "  /* see if the address is misaligned for the ASI: */"
		$as_echo "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    $as_echo ""
	    $as_echo "  /* decode rd: */"
	    $as_echo "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
	    $as_echo "  fpreg"
	    $as_echo "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    $as_echo "                                 misaligned,"
	    $as_echo "                                 &float_buffer);"
	    $as_echo ""
	    if test ${arch} = 32; then
		$as_echo "  /* do the load: */"
		$as_echo "  tme_sparc${arch}_ldd(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}));"
		$as_echo ""
		$as_echo "  /* set the double floating-point register value: */"
		$as_echo "  assert (fpreg != &float_buffer);"
		$as_echo "  fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		$as_echo "  fpreg->tme_float_value_ieee754_double.tme_value64_uint32_hi"
		$as_echo "    = ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 0);"
		$as_echo "  fpreg->tme_float_value_ieee754_double.tme_value64_uint32_lo"
		$as_echo "    = ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1);"
	    else
		$as_echo "  /* if bit two of the address is set, and this SPARC supports"
		$as_echo "     32-bit-aligned ${insn} instructions: */"
		$as_echo "  if ((misaligned & sizeof(tme_uint32_t))"
		$as_echo "      && fpreg != &float_buffer) {"
		$as_echo ""
		$as_echo "    /* do two 32-bit loads: */"
		$as_echo "    offset = sizeof(tme_uint32_t) * 0;"
		$as_echo "    tme_sparc${arch}_ld${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 0));"
		$as_echo "    offset = sizeof(tme_uint32_t) * 1;"
		$as_echo "    tme_sparc${arch}_ld${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 1));"
		$as_echo ""
		$as_echo "    /* set the double floating-point register value: */"
		$as_echo "    fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		$as_echo "    fpreg->tme_float_value_ieee754_double.tme_value64_uint32_hi"
		$as_echo "      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX + 0);"
		$as_echo "    fpreg->tme_float_value_ieee754_double.tme_value64_uint32_lo"
		$as_echo "      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX + 1);"
		$as_echo "  }"
		$as_echo ""
		$as_echo "  /* otherwise, bit two of the address is not set, or this SPARC"
		$as_echo "     doesn't support 32-bit-aligned ${insn} instructions: */"
		$as_echo "  else {"
		$as_echo ""
		$as_echo "    /* do an ldx${alternate}-style load: */"
		$as_echo "    tme_sparc${arch}_ldx${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		$as_echo ""
		$as_echo "    /* set the double floating-point register value: */"
		$as_echo "    assert (fpreg != &float_buffer);"
		$as_echo "    fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		$as_echo "    fpreg->tme_float_value_ieee754_double.tme_value64_uint"
		$as_echo "      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX);"
		$as_echo "  }"
	    fi
	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# the stdf instruction:
	#
	if test ${insn} = "stdf${alternate}"; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_uint${arch}_t address;"
	    $as_echo "  tme_uint32_t misaligned;"
	    $as_echo "  struct tme_float float_buffer;"
	    $as_echo "  struct tme_float *fpreg;"
	    $as_echo "  const union tme_value64 *value_double;"
	    if test ${arch} != 32; then
		$as_echo "  tme_uint${arch}_t offset;"
	    fi
	    $as_echo ""
	    $as_echo "  /* get the address: */"
	    $as_echo "  address = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;"
	    $as_echo ""
	    $as_echo "  /* get the least significant 32 bits of the address: */"
	    $as_echo "  misaligned = address;"
	    if test "${arch}${alternate}" = 64a; then
		$as_echo ""
		$as_echo "  /* see if the address is misaligned for the ASI: */"
		$as_echo "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    $as_echo ""
	    $as_echo "  /* decode rd: */"
	    $as_echo "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
	    $as_echo "  fpreg"
	    $as_echo "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    $as_echo "                                 misaligned,"
	    $as_echo "                                 &float_buffer);"
	    $as_echo ""
	    $as_echo "  /* get this double floating-point register in IEEE754 double-precision format: */"
	    $as_echo "  value_double = tme_ieee754_double_value_get(fpreg, &float_buffer.tme_float_value_ieee754_double);"
	    $as_echo ""
	    if test ${arch} = 32; then
		$as_echo "  /* set the floating-point register value: */"
		$as_echo "  ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 0)"
		$as_echo "    = value_double->tme_value64_uint32_hi;"
		$as_echo "  ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1)"
		$as_echo "    = value_double->tme_value64_uint32_lo;"
		$as_echo ""
		$as_echo "  /* do the store: */"
		$as_echo "  tme_sparc${arch}_std(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}));"
	    else
		$as_echo "  /* if bit two of the address is set, and this SPARC supports"
		$as_echo "     32-bit-aligned ${insn} instructions: */"
		$as_echo "  if ((misaligned & sizeof(tme_uint32_t))"
		$as_echo "      && fpreg != &float_buffer) {"
		$as_echo ""
		$as_echo "    /* set the floating-point register value: */"
		$as_echo "    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX + 0)"
		$as_echo "      = value_double->tme_value64_uint32_hi;"
		$as_echo "    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX + 1)"
		$as_echo "      = value_double->tme_value64_uint32_lo;"
		$as_echo ""
		$as_echo "    /* do two 32-bit stores: */"
		$as_echo "    offset = sizeof(tme_uint32_t) * 0;"
		$as_echo "    tme_sparc${arch}_st${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 0));"
		$as_echo "    offset = sizeof(tme_uint32_t) * 1;"
		$as_echo "    tme_sparc${arch}_st${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 1));"
		$as_echo "  }"
		$as_echo ""
		$as_echo "  /* otherwise, bit two of the address is not set, or this SPARC"
		$as_echo "     doesn't support 32-bit-aligned ${insn} instructions: */"
		$as_echo "  else {"
		$as_echo ""
		$as_echo "    /* set the floating-point register value: */"
		$as_echo "    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX)"
		$as_echo "      = value_double->tme_value64_uint;"
		$as_echo ""
		$as_echo "    /* do an stx${alternate}-style store: */"
		$as_echo "    tme_sparc${arch}_stx${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		$as_echo "  }"
	    fi
	    $as_echo ""
	    $as_echo "  assert (fpreg != &float_buffer);"
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# the ldfsr instruction:
	#
	if test ${insn} = ldfsr; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  tme_uint32_t fsr;"
	    if test ${arch} != 32; then
		$as_echo "  tme_uint32_t reg_rd;"
		$as_echo ""
		$as_echo "  /* see if this is an ldfsr or an ldxfsr: */"
		$as_echo "  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);"
		$as_echo "  if (__tme_predict_false(reg_rd > 1)) {"
		$as_echo "    TME_SPARC_INSN_ILL(ic);"
		$as_echo "  }"
	    fi
	    $as_echo ""
	    $as_echo "  _tme_sparc${arch}_fpu_mem(ic);"
	    $as_echo ""
	    if test ${arch} = 32; then
		$as_echo "  /* do the load: */"
	    else
		$as_echo "  /* if this is an ldxfsr: */"
		$as_echo "  if (reg_rd == 1) {"
		$as_echo ""
		$as_echo "    /* do the load: */"
		$as_echo "    tme_sparc${arch}_ldx(ic, _rs1, _rs2,  &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		$as_echo ""
		$as_echo "    /* update the extended FSR: */"
		$as_echo "    ic->tme_sparc_fpu_xfsr"
		$as_echo "      = (ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1)"
		$as_echo "         & 0x3f /* fcc3 .. fcc1 */);"
		$as_echo "  }"
		$as_echo ""
		$as_echo "  /* otherwise, this is an ldfsr.  do the load: */"
		$as_echo "  else"
		$as_echo_n "  "
	    fi
	    $as_echo "  tme_sparc${arch}_ld(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    $as_echo ""
	    $as_echo "  /* update the FSR: */"
	    $as_echo "  fsr = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift});"
	    $as_echo "  /* \"An LDFSR instruction does not affect ftt.\" */"
	    $as_echo "  /* \"The LDFSR instruction does not affect qne.\" */"
	    $as_echo "  fsr &= ~(TME_SPARC_FSR_VER | TME_SPARC_FSR_FTT | TME_SPARC_FSR_QNE);"
	    $as_echo "  ic->tme_sparc_fpu_fsr = (ic->tme_sparc_fpu_fsr & (TME_SPARC_FSR_VER | TME_SPARC_FSR_FTT | TME_SPARC_FSR_QNE)) | fsr;"
	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# the stfsr instruction:
	#
	if test ${insn} = stfsr; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    if test ${arch} = 64; then
		$as_echo "  tme_uint32_t reg_rd;"
		$as_echo ""
		$as_echo "  /* see if this is an stfsr or an stxfsr: */"
		$as_echo "  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);"
		$as_echo "  if (__tme_predict_false(reg_rd > 1)) {"
		$as_echo "    TME_SPARC_INSN_ILL(ic);"
		$as_echo "  }"
	    fi
	    $as_echo ""
	    $as_echo "  _tme_sparc${arch}_fpu_mem(ic);"
	    $as_echo ""
	    $as_echo "  /* set the FSR value to store: */"
	    $as_echo "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}) = ic->tme_sparc_fpu_fsr;"
	    $as_echo ""
	    if test ${arch} = 32; then
		$as_echo "  /* do the store: */"
	    else
		$as_echo "  /* if this is an stxfsr: */"
		$as_echo "  if (reg_rd == 1) {"
		$as_echo ""
		$as_echo "    /* set in the extended FSR to store and do the store: */"
		$as_echo "    ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1) = ic->tme_sparc_fpu_xfsr;"
		$as_echo "    tme_sparc${arch}_stx(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		$as_echo "  }"
		$as_echo ""
		$as_echo "  /* otherwise, this is a stfsr.  do the store: */"
		$as_echo "  else"
		$as_echo_n "  "
	    fi
	    $as_echo "  tme_sparc${arch}_st(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    $as_echo ""
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

	# the fpop1 and fpop2 instructions:
	#
	if test ${insn} = fpop1 || test ${insn} = fpop2; then

	    $as_echo ""
	    $as_echo "/* this does a sparc${arch} ${insn}: */"
	    $as_echo "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    $as_echo "{"
	    $as_echo "  TME_SPARC_INSN_FPU;"
	    $as_echo "  tme_sparc_fpu_${insn}(ic);"
	    $as_echo "  TME_SPARC_INSN_OK;"
	    $as_echo "}"
	fi

    done

    # the slow atomic function:
    #
    if $header; then
	$as_echo "void tme_sparc${arch}_atomic _TME_P((struct tme_sparc *, struct tme_sparc_ls *));"
    else
	$as_echo ""
	$as_echo "/* this does a slow atomic operation: */"
	$as_echo "void"
	$as_echo "tme_sparc${arch}_atomic(struct tme_sparc *ic, struct tme_sparc_ls *ls)"
	$as_echo "{"
	$as_echo "  tme_uint32_t endian_little;"
	$as_echo "  tme_uint32_t insn;"
	if test ${arch} = 64; then
	    $as_echo "  tme_uint${arch}_t value${arch};"
	    $as_echo "  tme_uint${arch}_t value_swap${arch};"
	    $as_echo "  unsigned int reg_rs2;"
	fi
	$as_echo "  tme_uint32_t value32;"
	$as_echo "  tme_uint32_t value_swap32;"
	$as_echo "  tme_uint32_t size;"
	$as_echo ""
	$as_echo "  /* if this is the beginning of the operation: */"
	$as_echo "  if (ls->tme_sparc_ls_state == 0) {"
	$as_echo ""
	$as_echo "    /* start the load part of the operation: */"
	$as_echo "    ls->tme_sparc_ls_state = ls->tme_sparc_ls_size;"
	$as_echo "    assert (ls->tme_sparc_ls_state != 0"
	$as_echo "            && (ls->tme_sparc_ls_state & TME_BIT(7)) == 0);"
	$as_echo ""
	$as_echo "    /* the load must start at the beginning of the buffer: */"
	$as_echo "    assert (ls->tme_sparc_ls_buffer_offset == 0);"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* if this is the load part of the operation: */"
	$as_echo "  if ((ls->tme_sparc_ls_state & TME_BIT(7)) == 0) {"
	$as_echo ""
	$as_echo "    /* do one slow load cycle: */"
	$as_echo "    tme_sparc${arch}_load(ic, ls);"
	$as_echo ""
	$as_echo "    /* if the slow load cycle did not load all of the data: */"
	$as_echo "    if (__tme_predict_false(ls->tme_sparc_ls_size != 0)) {"
	$as_echo "      return;"
	$as_echo "    }"
	$as_echo ""
	$as_echo "    /* get the byte order of this transfer: */"
	if test ${arch} = 32; then
	    $as_echo "    endian_little = FALSE;"
	else
	    $as_echo "    endian_little = ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_ENDIAN_LITTLE;"
	fi
	$as_echo ""
	$as_echo "    /* dispatch on the op3 of the instruction: */"
	$as_echo "    insn = TME_SPARC_INSN;"
	$as_echo "    switch ((insn >> 19) & 0x3f) {"
	if test ${arch} = 64; then
	    for size in 32 64; do
		$as_echo ""
		if test ${size} = 32; then
		    $as_echo "    case 0x3c: /* casa */"
		else
		    $as_echo "    case 0x3e: /* casxa */"
		fi
		$as_echo ""
		$as_echo "      /* finish the load part of the compare and swap: */"
		$as_echo "      assert (ls->tme_sparc_ls_state == sizeof(tme_uint${size}_t));"
		$as_echo "      value${size} = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer${size}s[0];"
		$as_echo "      value_swap${size} = *ls->tme_sparc_ls_rd64;"
		$as_echo "      if (endian_little) {"
		$as_echo "        value${size} = tme_letoh_u${size}(value${size});"
		$as_echo "        value_swap${size} = tme_htole_u${size}(value_swap${size});"
		$as_echo "      }"
		$as_echo "      else {"
		$as_echo "        value${size} = tme_betoh_u${size}(value${size});"
		$as_echo "        value_swap${size} = tme_htobe_u${size}(value_swap${size});"
		$as_echo "      }"
		$as_echo "      *ls->tme_sparc_ls_rd64 = value${size};"
		$as_echo ""
		$as_echo "      /* if the comparison fails: */"
		$as_echo "      reg_rs2 = TME_FIELD_MASK_EXTRACTU(insn, TME_SPARC_FORMAT3_MASK_RS2);"
		$as_echo "      TME_SPARC_REG_INDEX(ic, reg_rs2);"
		$as_echo "      if (value${size} != (tme_uint${size}_t) ic->tme_sparc_ireg_uint${arch}(reg_rs2)) {"
		$as_echo "        return;"
		$as_echo "      }"
		$as_echo ""
		$as_echo "      /* start the store part of the compare and swap: */"
		$as_echo "      ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer${size}s[0] = value_swap${size};"
		$as_echo "      break;"
	    done
	fi
	$as_echo ""
	$as_echo "    case 0x0d: /* ldstub */"
	$as_echo "    case 0x1d: /* ldstuba */"
	$as_echo ""
	$as_echo "      /* finish the load part of the ldstub: */"
	$as_echo "      assert (ls->tme_sparc_ls_state == sizeof(tme_uint8_t));"
	$as_echo "      *ls->tme_sparc_ls_rd${arch} = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s[0];"
	$as_echo ""
	$as_echo "      /* start the store part of the ldstub: */"
	$as_echo "      ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s[0] = 0xff;"
	$as_echo "      break;"
	$as_echo ""
	$as_echo "    /* otherwise, this must be swap: */"
	$as_echo "    default:"
	$as_echo "      assert (((insn >> 19) & 0x2f) == 0x0f /* swap, swapa */);"
	$as_echo ""
	$as_echo "      /* finish the load part of the swap: */"
	$as_echo "      assert (ls->tme_sparc_ls_state == sizeof(tme_uint32_t));"
	$as_echo "      value32 = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[0];"
	$as_echo "      value_swap32 = *ls->tme_sparc_ls_rd${arch};"
	$as_echo "      if (endian_little) {"
	$as_echo "        value32 = tme_letoh_u32(value32);"
	$as_echo "        value_swap32 = tme_htole_u32(value32);"
	$as_echo "      }"
	$as_echo "      else {"
	$as_echo "        value32 = tme_betoh_u32(value32);"
	$as_echo "        value_swap32 = tme_htobe_u32(value32);"
	$as_echo "      }"
	$as_echo "      *ls->tme_sparc_ls_rd${arch} = value32;"
	$as_echo ""
	$as_echo "      /* start the store part of the swap: */"
	$as_echo "      ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[0] = value_swap32;"
	$as_echo "      break;"
	$as_echo "    }"
	$as_echo ""
	$as_echo "    /* start the store part of the operation: */"
	$as_echo "    size = ls->tme_sparc_ls_state;"
	$as_echo "    ls->tme_sparc_ls_address${arch} -= size;"
	$as_echo "    ls->tme_sparc_ls_size = size;"
	$as_echo "    ls->tme_sparc_ls_buffer_offset = 0;"
	$as_echo "    ls->tme_sparc_ls_state = size | TME_BIT(7);"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* this is the store part of the operation: */"
	$as_echo ""
	$as_echo "  /* do one slow store cycle: */"
	$as_echo "  tme_sparc${arch}_store(ic, ls);"
	$as_echo ""
	$as_echo "  /* if the slow store cycle did not store all of the data: */"
	$as_echo "  if (__tme_predict_false(ls->tme_sparc_ls_size != 0)) {"
	$as_echo "    return;"
	$as_echo "  }"
	$as_echo "}"
    fi

    # the slow load and store functions:
    #
    for slow in load store; do

	if test ${slow} = load; then
	    cycle="read"
	    capcycle="READ"
	else
	    cycle="write"
	    capcycle="WRITE"
	fi
	if $header; then
	    $as_echo "void tme_sparc${arch}_${slow} _TME_P((struct tme_sparc *, struct tme_sparc_ls *));"
	    continue
	fi

	$as_echo ""
	$as_echo "/* this does one slow ${slow} cycle: */"
	$as_echo "void"
	$as_echo "tme_sparc${arch}_${slow}(struct tme_sparc *ic, "
	$as_echo "                  struct tme_sparc_ls *ls)"
	$as_echo "{"

	# our locals:
	#
	$as_echo "  struct tme_sparc_tlb *tlb;"
	$as_echo "  tme_uint${arch}_t address;"
	$as_echo "  unsigned int cycle_size;"
	$as_echo "  tme_bus_addr_t physical_address;"
	$as_echo "  int shift;"
	$as_echo "  int err;"

	$as_echo ""
	$as_echo "  /* get the TLB entry: */"
	$as_echo "  tlb = ls->tme_sparc_ls_tlb;"
	$as_echo ""
	$as_echo "  /* the TLB entry must be busy and valid: */"
	$as_echo "  assert (tme_bus_tlb_is_valid(&tlb->tme_sparc_tlb_bus_tlb));"
	$as_echo ""
	$as_echo "  /* start the bus cycle structure: */"
	$as_echo "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_type = TME_BUS_CYCLE_${capcycle};"
	$as_echo ""
	$as_echo "  /* get the buffer: */"
	$as_echo "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_buffer = &ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s[ls->tme_sparc_ls_buffer_offset];"
	$as_echo "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_buffer_increment = 1;"
	$as_echo ""
	$as_echo "  /* get the current address: */"
	$as_echo "  address = ls->tme_sparc_ls_address${arch};"
	$as_echo "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address = address;"
	$as_echo ""
	$as_echo "  /* start the cycle size: */"
	$as_echo "  cycle_size = ls->tme_sparc_ls_size;"
	$as_echo "  assert (cycle_size > 0);"
	$as_echo "  cycle_size--;"
	$as_echo "  cycle_size = TME_MIN(cycle_size, (((tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last) - address)) + 1;"
	$as_echo ""
	$as_echo "  /* if this TLB entry allows fast ${cycle}s: */"
	$as_echo "  if (__tme_predict_true(tlb->tme_sparc_tlb_emulator_off_${cycle} != TME_EMULATOR_OFF_UNDEF)) {"
	$as_echo ""
	$as_echo "    /* do a ${cycle}: */"
	$as_echo "    ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size = cycle_size;"
	$as_echo "    tme_memory_bus_${cycle}_buffer((tlb->tme_sparc_tlb_emulator_off_${cycle} + (tme_uint${arch}_t) ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address),"
	$as_echo "                                  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_buffer,"
	$as_echo "                                  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size,"
	$as_echo "                                  tlb->tme_sparc_tlb_bus_rwlock,"
	$as_echo "                                  sizeof(tme_uint8_t),"
	$as_echo "                                  sizeof(tme_uint${arch}_t));"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* otherwise, this TLB entry does not allow fast ${cycle}s: */"
	$as_echo "  else {"
	$as_echo ""
	$as_echo "    /* finish the cycle size: */"
	$as_echo "    cycle_size = TME_MIN(cycle_size, 1 + ((~ (unsigned int) address) % sizeof(tme_uint${arch}_t)));"
	$as_echo "    ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size = cycle_size;"
	$as_echo ""
	$as_echo "    /* form the physical address for the bus cycle handler: */"
	$as_echo "    physical_address = ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address;"
	$as_echo "    physical_address += tlb->tme_sparc_tlb_addr_offset;"
	$as_echo "    shift = tlb->tme_sparc_tlb_addr_shift;"
	$as_echo "    if (shift < 0) {"
	$as_echo "      physical_address <<= (0 - shift);"
	$as_echo "    }"
	$as_echo "    else if (shift > 0) {"
	$as_echo "      physical_address >>= shift;"
	$as_echo "    }"
	$as_echo "    ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address = physical_address;"
	$as_echo ""
	$as_echo "    /* finish the bus cycle structure: */"
	$as_echo "    (*ic->_tme_sparc_ls_bus_cycle)(ic, ls);"

	$as_echo "    tme_sparc_log(ic, 10000, TME_OK,"
	$as_echo "                 (TME_SPARC_LOG_HANDLE(ic),"
	$as_echo "                  _(\"cycle-${slow}%u\t0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch}),"
	$as_echo "                  (unsigned int) (ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size * 8),"
	$as_echo "                  (tme_bus_addr${arch}_t) ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address));"

	$as_echo ""
	$as_echo "    /* callout the bus cycle: */"
	$as_echo "    tme_sparc_tlb_unbusy(tlb);"
	$as_echo "    tme_sparc_callout_unlock(ic);"
	$as_echo "    err = (*tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_cycle)"
	$as_echo "           (tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_cycle_private,"
	$as_echo "            &ls->tme_sparc_ls_bus_cycle);"
	$as_echo "    tme_sparc_callout_relock(ic);"
	$as_echo "    tme_sparc_tlb_busy(tlb);"
	$as_echo ""
	$as_echo "    /* the TLB entry can't have been invalidated before the ${slow}: */"
	$as_echo "    assert (err != EBADF);"
	$as_echo ""
	$as_echo "    /* if the bus cycle didn't complete normally: */"
	$as_echo "    if (err != TME_OK) {"
	$as_echo ""
	$as_echo "      /* if a real bus fault may have happened, instead of"
	$as_echo "         some synchronous event: */"
	$as_echo "      if (err != TME_BUS_CYCLE_SYNCHRONOUS_EVENT) {"
	$as_echo ""
	$as_echo "        /* call the bus fault handlers: */"
	$as_echo "        err = tme_bus_tlb_fault(&tlb->tme_sparc_tlb_bus_tlb, &ls->tme_sparc_ls_bus_cycle, err);"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* if some synchronous event has happened: */"
	$as_echo "      if (err == TME_BUS_CYCLE_SYNCHRONOUS_EVENT) {"
	$as_echo ""
	$as_echo "        /* after the currently executing instruction finishes, check"
	$as_echo "           for external resets, halts, or interrupts: */"
	$as_echo "        ic->_tme_sparc_instruction_burst_remaining = 0;"
	$as_echo "        ic->_tme_sparc_instruction_burst_other = TRUE;"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* otherwise, if no real bus fault happened: */"
	$as_echo "      else if (err == TME_OK) {"
	$as_echo ""
	$as_echo "        /* nothing to do */"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* otherwise, a real bus fault happened: */"
	$as_echo "      else {"
	$as_echo "        (*ic->_tme_sparc_ls_bus_fault)(ic, ls, err);"
	$as_echo "        return;"
	$as_echo "      }"
	$as_echo "    }"
	$as_echo "  }"

	$as_echo ""
	$as_echo "  /* some data must have been transferred: */"
	$as_echo "  assert (ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size > 0);"

	if test ${slow} = store; then
	    $as_echo ""
	    $as_echo "  /* if this was an atomic operation: */"
	    $as_echo "  if (__tme_predict_false(ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ATOMIC)) {"
	    $as_echo ""
	    $as_echo "    /* we do not support atomic operations in TLB entries that"
	    $as_echo "       do not support both fast reads and fast writes.  assuming"
	    $as_echo "       that all atomic operations are to regular memory, we"
	    $as_echo "       should always get fast read and fast write TLBs.  when"
	    $as_echo "       we do not, it should only be because the memory has been"
	    $as_echo "       made read-only in the MMU.  the write above was supposed"
	    $as_echo "       to cause a fault (with the instruction rerun later with"
	    $as_echo "       a fast read and fast write TLB entry), but instead it"
	    $as_echo "       succeeded and transferred some data.  we have modified"
	    $as_echo "       memory and cannot recover: */"
	    $as_echo "    abort();"
	    $as_echo "  }"
	fi

	$as_echo ""
	$as_echo "  /* update: */"
	$as_echo "  cycle_size = ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size;"
	$as_echo "  ls->tme_sparc_ls_address${arch} += cycle_size;"
	$as_echo "  ls->tme_sparc_ls_buffer_offset += cycle_size;"
	$as_echo "  ls->tme_sparc_ls_size -= cycle_size;"
	$as_echo "}"
    done

    # the load/store function:
    #
    if $header; then
	$as_echo "tme_shared tme_uint8_t *tme_sparc${arch}_ls _TME_P((struct tme_sparc *, tme_uint${arch}_t, tme_uint${arch}_t *, tme_uint32_t));"
    else

	$as_echo ""
	$as_echo "/* this does a slow load or store: */"
	$as_echo "tme_shared tme_uint8_t *"
	$as_echo "tme_sparc${arch}_ls(struct tme_sparc *ic,"
	$as_echo "               tme_uint${arch}_t const address_first,"
	$as_echo "               tme_uint${arch}_t *_rd,"
	$as_echo "               tme_uint32_t lsinfo)"
	$as_echo "{"
	$as_echo "  struct tme_sparc_ls ls;"
	$as_echo "  tme_uint32_t size;"
	$as_echo "  tme_uint32_t asi;"
	$as_echo "  tme_uint32_t asi_mask_flags;"
	$as_echo "  tme_uint32_t asi_mask;"
	$as_echo "  tme_bus_context_t context;"
	$as_echo "  tme_uint32_t tlb_hash;"
	$as_echo "  unsigned long tlb_i;"
	$as_echo "  unsigned long handler_i;"
	$as_echo "  struct tme_sparc_tlb *tlb;"
	$as_echo "  unsigned int cycle_type;"
	$as_echo "  tme_uint${arch}_t address;"
	$as_echo "  void (*address_map) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));"
	$as_echo "  tme_bus_addr_t address_bus;"
	$as_echo "  int rc;"
	$as_echo "  const tme_shared tme_uint8_t *emulator_off;"
	$as_echo "  unsigned int buffer_offset;"
	$as_echo "  tme_uint${arch}_t value;"
	$as_echo "  tme_uint32_t value32;"
	$as_echo ""
	$as_echo "  /* we must not be replaying instructions: */"
	$as_echo "  assert (tme_sparc_recode_verify_replay_last_pc(ic) == 0);"
	$as_echo ""
	$as_echo "  /* initialize the pointer to the rd register: */"
	$as_echo "  ls.tme_sparc_ls_rd${arch} = _rd;"
	$as_echo ""
	$as_echo "#ifndef NDEBUG"
	$as_echo ""
	$as_echo "  /* initialize the cycle function: */"
	$as_echo "  ls.tme_sparc_ls_cycle = NULL;"
	$as_echo ""
	$as_echo "  /* initialize the TLB entry pointer: */"
	$as_echo "  ls.tme_sparc_ls_tlb = NULL;"
	$as_echo ""
	$as_echo "#endif /* NDEBUG */"
	$as_echo ""
	$as_echo "  /* initialize the faults: */"
	$as_echo "  ls.tme_sparc_ls_faults = TME_SPARC_LS_FAULT_NONE;"
	$as_echo ""
	$as_echo "  /* initialize the address: */"
	$as_echo "  ls.tme_sparc_ls_address${arch} = address_first;"
	$as_echo ""
	$as_echo "  /* initialize the size: */"
	$as_echo "  size = TME_SPARC_LSINFO_WHICH_SIZE(lsinfo);"
	$as_echo "  ls.tme_sparc_ls_size = size;"
	$as_echo ""
	$as_echo "  /* initialize the info: */"
	$as_echo "  ls.tme_sparc_ls_lsinfo = lsinfo;"
	$as_echo ""
	$as_echo "  /* if the address is not aligned: */"
	$as_echo "  if (__tme_predict_false(((size - 1) & (tme_uint32_t) address_first) != 0)) {"
	$as_echo "    ls.tme_sparc_ls_faults |= TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* otherwise, the address is aligned: */"
	$as_echo "  else {"
	$as_echo ""
	$as_echo "    /* the transfer must not cross a 32-bit boundary: */"
	$as_echo "    assert ((size - 1) <= (tme_uint32_t) ~address_first);"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* initialize the address map: */"
	$as_echo "  ls.tme_sparc_ls_address_map = ic->_tme_sparc_ls_address_map;"
	$as_echo ""
	$as_echo "  /* if this is a ldd, ldda, std, or stda, or an instruction"
	$as_echo "     that loads or stores in the same way: */"
	$as_echo "  if (lsinfo & TME_SPARC_LSINFO_LDD_STD) {"
	$as_echo ""
	$as_echo "    /* if the rd register is odd: */"
	$as_echo "    /* NB: we don't check the rd field in the instruction,"
	$as_echo "       because the register number there might be encoded"
	$as_echo "       in some way, or the architecture might ignore bit"
	$as_echo "       zero in the rd field (for example, the sparc32 lddf)."
	$as_echo "       instead, we test the rd register pointer: */"
	$as_echo "    if (__tme_predict_false((ls.tme_sparc_ls_rd${arch}"
	$as_echo "                             - ic->tme_sparc_ic.tme_ic_iregs.tme_ic_iregs_uint${arch}s)"
	$as_echo "                            % 2)) {"
	$as_echo "      ls.tme_sparc_ls_faults |= TME_SPARC_LS_FAULT_LDD_STD_RD_ODD;"
	$as_echo "    }"
	$as_echo "  }"

	$as_echo ""
	$as_echo "  /* if the ASI has been specified: */"
	$as_echo "  if (lsinfo & TME_SPARC_LSINFO_A) {"
	$as_echo ""
	$as_echo "    /* get the ASI: */"
	$as_echo "    asi = TME_SPARC_LSINFO_WHICH_ASI(lsinfo);"
	$as_echo ""
	$as_echo "    /* get the flags for this ASI: */"
	$as_echo "    asi_mask_flags = ic->tme_sparc_asis[asi].tme_sparc_asi_mask_flags;"
	$as_echo ""
	if test ${arch} = 32; then
	    $as_echo "    /* make the ASI mask: */"
	    $as_echo "    if (asi_mask_flags & TME_SPARC32_ASI_MASK_FLAG_SPECIAL) {"
	    $as_echo "      asi_mask"
	    $as_echo "        = TME_SPARC_ASI_MASK_SPECIAL(asi, TRUE);"
	    $as_echo "    }"
	    $as_echo "    else {"
	    $as_echo "      asi_mask = TME_SPARC32_ASI_MASK(asi, asi);"
	    $as_echo "    }"
	elif test ${arch} = 64; then
	    $as_echo "    /* if this is a nonprivileged access: */"
	    $as_echo "    if (!TME_SPARC_PRIV(ic)) {"
	    $as_echo ""
	    $as_echo "      /* if this is a restricted ASI: */"
	    $as_echo "      if (__tme_predict_false((asi & TME_SPARC64_ASI_FLAG_UNRESTRICTED) == 0)) {"
	    $as_echo "        ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_PRIVILEGED_ASI;"
	    $as_echo "      }"
	    $as_echo ""
	    $as_echo "      /* force a nonprivileged access with the ASI: */"
	    $as_echo "      asi_mask_flags |= TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER;"
	    $as_echo "    }"
	    $as_echo ""
	    $as_echo "    /* make the ASI mask: */"
	    $as_echo "    if (asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_SPECIAL) {"
	    $as_echo "      asi_mask"
	    $as_echo "        = (asi_mask_flags"
	    $as_echo "           + TME_SPARC_ASI_MASK_SPECIAL(asi,"
	    $as_echo "                                        (asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER) == 0));"
	    $as_echo "    }"
	    $as_echo "    else {"
	    $as_echo "      asi_mask = TME_SPARC64_ASI_MASK(asi, asi_mask_flags);"
	    $as_echo "    }"
	fi
	$as_echo "    ls.tme_sparc_ls_asi_mask = asi_mask;"
	if test ${arch} = 64; then
	    $as_echo ""
	    $as_echo "    /* if this is a no-fault ASI with a non-load instruction: */"
	    $as_echo "    if (asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT) {"
	    $as_echo "      if (__tme_predict_false(lsinfo & (TME_SPARC_LSINFO_OP_ST | TME_SPARC_LSINFO_OP_ATOMIC))) {"
	    $as_echo "        ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_NO_FAULT_NON_LOAD;"
	    $as_echo "      }"
	    $as_echo "    }"
	fi
	$as_echo ""
	$as_echo "    /* get the context for the alternate address space: */"
	if test ${arch} = 32; then
	    $as_echo "    context = ic->tme_sparc_memory_context_default;"
	elif test ${arch} = 64; then
	    $as_echo "    context = ic->tme_sparc_memory_context_primary;"
	    $as_echo "    if (asi_mask & TME_SPARC64_ASI_FLAG_SECONDARY) {"
	    $as_echo "      context = ic->tme_sparc_memory_context_secondary;"
	    $as_echo "    }"
	    $as_echo "    if (__tme_predict_false(asi_mask & TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS)) {"
	    $as_echo "      if (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS) {"
	    $as_echo "        context = 0;"
	    $as_echo "      }"
	    $as_echo "    }"
	fi
	$as_echo "    ls.tme_sparc_ls_context = context;"
	$as_echo ""
	$as_echo "    /* get the default TLB entry index: */"
	$as_echo "    tlb_hash = TME_SPARC_TLB_HASH(ic, context, address_first);"
	$as_echo "    if (lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	$as_echo "      tlb_i = TME_SPARC_ITLB_ENTRY(ic, tlb_hash);"
	$as_echo "    }"
	$as_echo "    else {"
	$as_echo "      tlb_i = TME_SPARC_DTLB_ENTRY(ic, tlb_hash);"
	$as_echo "    }"
	$as_echo "    ls.tme_sparc_ls_tlb_i = tlb_i;"
	$as_echo ""
	$as_echo "    /* call any special handler for this ASI: */"
	$as_echo "    handler_i = ic->tme_sparc_asis[TME_SPARC_ASI_MASK_WHICH(asi_mask)].tme_sparc_asi_handler;"
	$as_echo "    if (__tme_predict_false(handler_i != 0)) {"
	$as_echo "      (*ic->_tme_sparc_ls_asi_handlers[handler_i])(ic, &ls);"
	$as_echo "    }"
	$as_echo ""
	$as_echo "    /* get the final TLB entry index: */"
	$as_echo "    tlb_i = ls.tme_sparc_ls_tlb_i;"
	$as_echo "  }"

	$as_echo ""
	$as_echo "  /* otherwise, the ASI has not been specified: */"
	$as_echo "  else {"
	$as_echo ""
	$as_echo "    /* get the ASI mask: */"
	$as_echo "    asi_mask = ic->tme_sparc_asi_mask_data;"
	$as_echo ""
	$as_echo "    /* add in any ASI mask flags from the instruction: */"
	if test ${arch} = 64; then
	    $as_echo "    /* NB: initially, TME_SPARC64_ASI_FLAG_NO_FAULT is the"
	    $as_echo "       only flag allowed, and only the flush instruction"
	    $as_echo "       can use it: */"
	fi
	$as_echo "    assert (TME_SPARC_LSINFO_WHICH_ASI_FLAGS(lsinfo) == 0"
	if test ${arch} = 64; then
	    $as_echo "            || (TME_SPARC_LSINFO_WHICH_ASI_FLAGS(lsinfo) == TME_SPARC64_ASI_FLAG_NO_FAULT"
	    $as_echo "                && ((ic->_tme_sparc_insn >> 19) & 0x3f) == 0x3b)"
	fi
	$as_echo "            );"
	$as_echo "    asi_mask |= TME_SPARC_LSINFO_WHICH_ASI_FLAGS(lsinfo);"
	$as_echo ""
	$as_echo "    /* set the ASI mask: */"
	$as_echo "    ls.tme_sparc_ls_asi_mask = asi_mask;"
	$as_echo ""
	$as_echo "    /* get the context: */"
	$as_echo "    context = ic->tme_sparc_memory_context_default;"
	$as_echo "    ls.tme_sparc_ls_context = context;"
	$as_echo ""
	$as_echo "    /* this must not be a fetch: */"
	$as_echo "    assert ((lsinfo & TME_SPARC_LSINFO_OP_FETCH) == 0);"
	$as_echo ""
	$as_echo "    /* get the TLB entry index: */"
	$as_echo "    tlb_hash = TME_SPARC_TLB_HASH(ic, context, address_first);"
	$as_echo "    tlb_i = TME_SPARC_DTLB_ENTRY(ic, tlb_hash);"
	$as_echo "    ls.tme_sparc_ls_tlb_i = tlb_i;"
	$as_echo "  }"

	$as_echo ""
	$as_echo "  /* get the TLB entry pointer: */"
	$as_echo "  tlb = &ic->tme_sparc_tlbs[tlb_i];"
	$as_echo "  ls.tme_sparc_ls_tlb = tlb;"

	$as_echo ""
	$as_echo "  /* get the cycle type: */"
	$as_echo "  /* NB: we deliberately set this once, now, since the lsinfo"
	$as_echo "     may change once we start transferring: */"
	$as_echo "  cycle_type"
	$as_echo "    = ((lsinfo"
	$as_echo "        & (TME_SPARC_LSINFO_OP_ST"
	$as_echo "           | TME_SPARC_LSINFO_OP_ATOMIC))"
	$as_echo "       ? TME_BUS_CYCLE_WRITE"
	$as_echo "       : TME_BUS_CYCLE_READ);"

	$as_echo ""
	$as_echo "  /* loop until the transfer is complete: */"
	$as_echo "  for (;;) {"

	$as_echo ""
	$as_echo "    /* if we have faulted: */"
	$as_echo "    if (__tme_predict_false(ls.tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {"
	$as_echo ""
	$as_echo "      /* unbusy this TLB, since the trap function may not return: */"
	$as_echo "      tme_bus_tlb_unbusy(&tlb->tme_sparc_tlb_bus_tlb);"
	$as_echo ""
	$as_echo "      /* call the trap function, which will not return if it traps: */"
	$as_echo "      (*ic->_tme_sparc_ls_trap)(ic, &ls);"
	$as_echo ""
	$as_echo "      /* rebusy this TLB: */"
	$as_echo "      tme_bus_tlb_busy(&tlb->tme_sparc_tlb_bus_tlb);"
	$as_echo ""
	$as_echo "      /* since the trap function returned, it must have cleared the fault: */"
	$as_echo "      assert (ls.tme_sparc_ls_faults == TME_SPARC_LS_FAULT_NONE);"
	$as_echo "    }"

	$as_echo ""
	$as_echo "    /* if the transfer is complete, stop now: */"
	$as_echo "    if (__tme_predict_false(ls.tme_sparc_ls_size == 0)) {"
	$as_echo "      break;"
	$as_echo "    }"

	$as_echo ""
	$as_echo "    /* get the current address: */"
	$as_echo "    address = ls.tme_sparc_ls_address${arch};"

	$as_echo ""
	$as_echo "    /* if this TLB entry does not apply or is invalid: */"
	$as_echo "    if ((tlb->tme_sparc_tlb_context != ls.tme_sparc_ls_context"
	$as_echo "         && tlb->tme_sparc_tlb_context <= ic->tme_sparc_memory_context_max)"
	$as_echo "        || address < (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_first"
	$as_echo "        || address > (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last"
	$as_echo "        || !TME_SPARC_TLB_ASI_MASK_OK(tlb, ls.tme_sparc_ls_asi_mask)"
	$as_echo "        || ((tlb->tme_sparc_tlb_cycles_ok & cycle_type) == 0"
	$as_echo "            && (cycle_type == TME_BUS_CYCLE_READ"
	$as_echo "                ? tlb->tme_sparc_tlb_emulator_off_read"
	$as_echo "                : tlb->tme_sparc_tlb_emulator_off_write) == TME_EMULATOR_OFF_UNDEF)"
	$as_echo "        || tme_bus_tlb_is_invalid(&tlb->tme_sparc_tlb_bus_tlb)) {"
	$as_echo ""
	$as_echo "      /* unbusy this TLB entry for filling: */"
	$as_echo "      tme_bus_tlb_unbusy_fill(&tlb->tme_sparc_tlb_bus_tlb);"
	$as_echo ""
	$as_echo "      /* if we haven't mapped this address yet: */"
	$as_echo "      address_map = ls.tme_sparc_ls_address_map;"
	$as_echo "      if (address_map != NULL) {"
	$as_echo "        ls.tme_sparc_ls_address_map = NULL;"
	$as_echo ""
	$as_echo "        /* count this mapping: */"
	$as_echo "        if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	$as_echo "          TME_SPARC_STAT(ic, tme_sparc_stats_itlb_map);"
	$as_echo "        }"
	$as_echo "        else {"
	$as_echo "          TME_SPARC_STAT(ic, tme_sparc_stats_dtlb_map);"
	$as_echo "        }"
	$as_echo ""
	$as_echo "        /* initialize the ASI mask and context on this TLB entry: */"
	$as_echo "        /* NB that the ASI mask will likely be updated by either the"
	$as_echo "           address mapping or the TLB fill: */"
	$as_echo "        tlb->tme_sparc_tlb_asi_mask"
	$as_echo "          = (ls.tme_sparc_ls_asi_mask"
	$as_echo "             & ~TME_SPARC_ASI_MASK_FLAGS_AVAIL);"
	$as_echo "        tlb->tme_sparc_tlb_context = ls.tme_sparc_ls_context;"
	$as_echo ""
	$as_echo "        /* NB: if the address mapping traps, we won't get a chance"
	$as_echo "           to finish updating this TLB entry, which is currently in"
	$as_echo "           an inconsistent state - but not necessarily an unusable"
	$as_echo "           state.  poison it to be unusable, including any recode"
	$as_echo "           TLB entry: */"
	$as_echo "        tlb->tme_sparc_tlb_addr_first = 1;"
	$as_echo "        tlb->tme_sparc_tlb_addr_last = 0;"
	$as_echo "#if TME_SPARC_HAVE_RECODE(ic)"
	$as_echo "        if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	$as_echo "          tme_sparc${arch}_recode_chain_tlb_update(ic, &ls);"
	$as_echo "        }"
	$as_echo "        else {"
	$as_echo "          tme_sparc${arch}_recode_ls_tlb_update(ic, &ls);"
	$as_echo "        }"
	$as_echo "#endif /* TME_SPARC_HAVE_RECODE(ic) */"
	$as_echo ""
	$as_echo "#ifndef NDEBUG"
	$as_echo ""
	$as_echo "        /* initialize the mapping TLB entry: */"
	$as_echo "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first = 0 - (tme_bus_addr_t) 1;"
	$as_echo "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last = 0 - (tme_bus_addr_t) 2;"
	$as_echo "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok = 0;"
	$as_echo "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset = 0 - (tme_bus_addr_t) 1;"
	$as_echo ""
	$as_echo "#endif /* !NDEBUG */"
	$as_echo ""
	$as_echo "        /* map the address: */"
	$as_echo "        (*address_map)(ic, &ls);"
	$as_echo ""
	$as_echo "        /* the address mapping must do any trapping itself: */"
	$as_echo "        assert (ls.tme_sparc_ls_faults == TME_SPARC_LS_FAULT_NONE);"
	$as_echo ""
	$as_echo "        /* if the address mapping completed the transfer: */"
	$as_echo "        if (__tme_predict_false(ls.tme_sparc_ls_size == 0)) {"
	$as_echo ""
	$as_echo "          /* rebusy the TLB entry: */"
	$as_echo "          tme_sparc_tlb_busy(tlb);"
	$as_echo ""
	$as_echo "          /* stop now: */"
	$as_echo "          break;"
	$as_echo "        }"
	$as_echo ""
	$as_echo "        /* the mapping must have actually made a mapping: */"
	$as_echo "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first != 0 - (tme_bus_addr_t) 1);"
	$as_echo "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last != 0 - (tme_bus_addr_t) 2);"
	$as_echo "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok != 0);"
	$as_echo "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset != 0 - (tme_bus_addr_t) 1);"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* count this fill: */"
	$as_echo "      if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	$as_echo "        TME_SPARC_STAT(ic, tme_sparc_stats_itlb_fill);"
	$as_echo "      }"
	$as_echo "      else {"
	$as_echo "        TME_SPARC_STAT(ic, tme_sparc_stats_dtlb_fill);"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* get the bus address: */"
	$as_echo "      address_bus = ls.tme_sparc_ls_address${arch} + ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset;"
	$as_echo ""
	$as_echo "      /* fill the TLB entry: */"
	$as_echo "      tme_sparc_callout_unlock(ic);"
	$as_echo "      rc = (*ic->_tme_sparc_bus_connection->tme_sparc_bus_tlb_fill)"
	$as_echo "        (ic->_tme_sparc_bus_connection,"
	$as_echo "         tlb,"
	$as_echo "         ls.tme_sparc_ls_asi_mask,"
	$as_echo "         address_bus,"
	$as_echo "         cycle_type);"
	$as_echo "      assert (rc == TME_OK);"
	$as_echo "      tme_sparc_callout_relock(ic);"
	$as_echo ""
	$as_echo "      /* map the TLB entry: */"
	$as_echo "      tme_bus_tlb_map(&tlb->tme_sparc_tlb_bus_tlb, address_bus,"
	$as_echo "                      &ls.tme_sparc_ls_tlb_map, ls.tme_sparc_ls_address${arch});"
	$as_echo ""
	$as_echo "      /* update any recode TLB entry: */"
	$as_echo "#if TME_SPARC_HAVE_RECODE(ic)"
	$as_echo "      if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	$as_echo "        tme_sparc${arch}_recode_chain_tlb_update(ic, &ls);"
	$as_echo "      }"
	$as_echo "      else {"
	$as_echo "        tme_sparc${arch}_recode_ls_tlb_update(ic, &ls);"
	$as_echo "      }"
	$as_echo "#endif /* TME_SPARC_HAVE_RECODE(ic) */"
	$as_echo ""
	$as_echo "      /* rebusy the TLB entry: */"
	$as_echo "      tme_sparc_tlb_busy(tlb);"
	$as_echo ""
	$as_echo "      /* if this TLB entry is already invalid: */"
	$as_echo "      if (tme_bus_tlb_is_invalid(&tlb->tme_sparc_tlb_bus_tlb)) {"
	$as_echo "        continue;"
	$as_echo "      }"
	$as_echo "    }"

	$as_echo ""
	$as_echo "    /* this TLB entry must apply: */"
	$as_echo "    assert ((tlb->tme_sparc_tlb_context == ls.tme_sparc_ls_context"
	$as_echo "             || tlb->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max)"
	$as_echo "            && ls.tme_sparc_ls_address${arch} >= (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_first"
	$as_echo "            && ls.tme_sparc_ls_address${arch} <= (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last"
	$as_echo "            && ((tlb->tme_sparc_tlb_cycles_ok & cycle_type)"
	$as_echo "                || (cycle_type == TME_BUS_CYCLE_READ"
	$as_echo "                    ? tlb->tme_sparc_tlb_emulator_off_read"
	$as_echo "                    : tlb->tme_sparc_tlb_emulator_off_write) != TME_EMULATOR_OFF_UNDEF)"
	$as_echo "            && TME_SPARC_TLB_ASI_MASK_OK(tlb, ls.tme_sparc_ls_asi_mask));"

	$as_echo ""
	$as_echo "    /* get the current lsinfo: */"
	$as_echo "    lsinfo = ls.tme_sparc_ls_lsinfo;"

	$as_echo ""
	$as_echo "    /* if we have to check the TLB: */"
	$as_echo "    if (__tme_predict_true((lsinfo & TME_SPARC_LSINFO_NO_CHECK_TLB) == 0)) {"
	$as_echo ""
	$as_echo "      /* get the ASI mask for this TLB entry: */"
	$as_echo "      asi_mask = tlb->tme_sparc_tlb_asi_mask;"
	if test ${arch} = 64; then
	    $as_echo ""
	    $as_echo "      /* if this TLB entry is for no-fault accesses only: */"
	    $as_echo "      if (__tme_predict_false(asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT)) {"
	    $as_echo ""
	    $as_echo "        /* if this access is not using a no-fault ASI: */"
	    $as_echo "        if (__tme_predict_false((ls.tme_sparc_ls_asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT) == 0)) {"
	    $as_echo "          ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_NO_FAULT_FAULT;"
	    $as_echo "          continue;"
	    $as_echo "        }"
	    $as_echo "      }"
	    $as_echo ""
	    $as_echo "      /* if this TLB entry is for addresses with side effects: */"
	    $as_echo "      if (asi_mask & TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS) {"
	    $as_echo ""
	    $as_echo "        /* if this access is using a no-fault ASI: */"
	    $as_echo "        /* NB: a flush may be implemented as a load with a no-fault ASI: */"
	    $as_echo "        if (__tme_predict_false(ls.tme_sparc_ls_asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT)) {"
	    $as_echo "          ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_SIDE_EFFECTS;"
	    $as_echo "          continue;"
	    $as_echo "        }"
	    $as_echo "      }"
	    $as_echo ""
	    $as_echo "      /* if this TLB entry is for uncacheable addresses: */"
	    $as_echo "      if (asi_mask & TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE) {"
	    $as_echo ""
	    $as_echo "        /* if this is an atomic access: */"
	    $as_echo "        if (__tme_predict_false(lsinfo & TME_SPARC_LSINFO_OP_ATOMIC)) {"
	    $as_echo "          ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_UNCACHEABLE;"
	    $as_echo "          continue;"
	    $as_echo "        }"
	    $as_echo "      }"
	    $as_echo ""
	    $as_echo "      /* see if this is a little-endian instruction: */"
	    $as_echo "      lsinfo"
	    $as_echo "        = ((lsinfo"
	    $as_echo "            & ~TME_SPARC_LSINFO_ENDIAN_LITTLE)"
	    $as_echo "           + ((ls.tme_sparc_ls_asi_mask"
	    $as_echo "               & TME_SPARC64_ASI_FLAG_LITTLE)"
	    $as_echo "#if TME_SPARC_LSINFO_ENDIAN_LITTLE < TME_SPARC64_ASI_FLAG_LITTLE"
	    $as_echo "#error \"TME_SPARC_LSINFO_ENDIAN_ values changed\""
	    $as_echo "#endif"
	    $as_echo "              * (TME_SPARC_LSINFO_ENDIAN_LITTLE"
	    $as_echo "                 / TME_SPARC64_ASI_FLAG_LITTLE)));"
	    $as_echo ""
	    $as_echo "      /* if this TLB entry has its little-endian bit set: */"
	    $as_echo "      if (__tme_predict_false(asi_mask & TME_SPARC64_ASI_FLAG_LITTLE)) {"
	    $as_echo "        assert (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_INVERT_ENDIAN);"
	    $as_echo "        if (TRUE) {"
	    $as_echo "          lsinfo ^= TME_SPARC_LSINFO_ENDIAN_LITTLE;"
	    $as_echo "        }"
	    $as_echo "      }"
	fi
	$as_echo "    }"

	$as_echo ""
	$as_echo "    /* if we might not have to call a slow cycle function: */"
	$as_echo "    if (__tme_predict_true((lsinfo & TME_SPARC_LSINFO_SLOW_CYCLES) == 0)) {"
	$as_echo ""
	$as_echo "      /* if this TLB entry allows fast transfer of all of the addresses: */"
	$as_echo "      if (__tme_predict_true(((tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last) >= (address_first + (ls.tme_sparc_ls_size - 1)))) {"
	$as_echo "        emulator_off = tlb->tme_sparc_tlb_emulator_off_read;"
	$as_echo "        if (lsinfo & TME_SPARC_LSINFO_OP_ST) {"
	$as_echo "          emulator_off = tlb->tme_sparc_tlb_emulator_off_write;"
	$as_echo "        }"
	$as_echo "        if (__tme_predict_true(emulator_off != TME_EMULATOR_OFF_UNDEF"
	$as_echo "                               && (((lsinfo & TME_SPARC_LSINFO_OP_ATOMIC) == 0)"
	$as_echo "                                   || emulator_off == tlb->tme_sparc_tlb_emulator_off_write))) {"
	$as_echo ""
	$as_echo "          /* return and let our caller do the transfer: */"
	$as_echo "          /* NB: we break const here: */"
	$as_echo "          return ((tme_shared tme_uint8_t *) emulator_off);"
	$as_echo "        }"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* we have to call a slow cycle function: */"
	$as_echo "      lsinfo |= TME_SPARC_LSINFO_SLOW_CYCLES;"
	$as_echo "      assert (ls.tme_sparc_ls_cycle == NULL);"
	if test ${arch} = 32; then
	    endian_little="FALSE"
	else
	    endian_little="(lsinfo & TME_SPARC_LSINFO_ENDIAN_LITTLE)"
	fi
	$as_echo ""
	$as_echo "      /* assume that this operation will transfer the start of the buffer: */"
	$as_echo "      buffer_offset = 0;"
	$as_echo ""
	$as_echo "      /* assume that this is a load or a fetch: */"
	$as_echo "      ls.tme_sparc_ls_cycle = tme_sparc${arch}_load;"
	$as_echo ""
	$as_echo "      /* if this is a store: */"
	$as_echo "      if (lsinfo & TME_SPARC_LSINFO_OP_ST) {"
	$as_echo ""
	$as_echo "        /* put the (first) register to store in the memory buffer: */"
	$as_echo "        value = TME_SPARC_FORMAT3_RD;"
	$as_echo "        value = (${endian_little} ? tme_htole_u${arch}(value) : tme_htobe_u${arch}(value));"
	$as_echo "        ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer${arch}s[0] = value;"
	$as_echo ""
	$as_echo "        /* find the offset in the memory buffer corresponding to the"
	$as_echo "           first address: */"
	$as_echo "        buffer_offset = sizeof(tme_uint${arch}_t) - ls.tme_sparc_ls_size;"
	$as_echo "        if (${endian_little}) {"
	$as_echo "          buffer_offset = 0;"
	$as_echo "        }"

	$as_echo ""
	$as_echo "        /* if this is a std or stda: */"
	$as_echo "        if (lsinfo & TME_SPARC_LSINFO_LDD_STD) {"
	$as_echo ""
	$as_echo "          /* put the odd 32-bit register to store in the memory buffer"
	$as_echo "             after the even 32-bit register.  exactly where this is depends"
	$as_echo "             on the architecture and on the byte order of the store: */"
	$as_echo "          value32 = TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch});"
	$as_echo "          if (${endian_little}) {"
	$as_echo "            value32 = tme_htole_u32(value32);"
	$as_echo "            ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[1] = value32;"
	$as_echo "            buffer_offset = 0;"
	$as_echo "          }"
	$as_echo "          else {"
	$as_echo "            value32 = tme_htobe_u32(value32);"
	$as_echo "            ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[(${arch} / 32)] = value32;"
	$as_echo "            buffer_offset = sizeof(tme_uint${arch}_t) - sizeof(tme_uint32_t);"
	$as_echo "          }"
	$as_echo "        }"
	$as_echo ""
	$as_echo "        /* set the cycle function: */"
	$as_echo "        ls.tme_sparc_ls_cycle = tme_sparc${arch}_store;"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* otherwise, if this is an atomic: */"
	$as_echo "      else if (lsinfo & TME_SPARC_LSINFO_OP_ATOMIC) {"
	$as_echo ""
	$as_echo "        /* set the cycle function: */"
	$as_echo "        ls.tme_sparc_ls_cycle = tme_sparc${arch}_atomic;"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* set the buffer offset for the (first) slow cycle: */"
	$as_echo "      ls.tme_sparc_ls_buffer_offset = buffer_offset;"
	$as_echo ""
	$as_echo "      /* clear the state for this operation: */"
	$as_echo "      ls.tme_sparc_ls_state = 0;"
	$as_echo "    }"

	$as_echo ""
	$as_echo "    /* assume that we won't have to check the TLB again: */"
	$as_echo "    ls.tme_sparc_ls_lsinfo = lsinfo | TME_SPARC_LSINFO_NO_CHECK_TLB;"

	$as_echo "    /* call the slow cycle function: */"
	$as_echo "    (*ls.tme_sparc_ls_cycle)(ic, &ls);"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* if this was a load that has already completed, a store,"
	$as_echo "     or an atomic, make sure our caller doesn't try to complete"
	$as_echo "     a fast transfer: */"
	$as_echo "  if (ls.tme_sparc_ls_lsinfo"
	$as_echo "      & (TME_SPARC_LSINFO_LD_COMPLETED"
	$as_echo "         | TME_SPARC_LSINFO_OP_ST"
	$as_echo "         | TME_SPARC_LSINFO_OP_ATOMIC)) {"
	$as_echo "    return (TME_EMULATOR_OFF_UNDEF);"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  /* otherwise, this was a load that did slow cycles into the"
	$as_echo "     memory buffer and hasn't updated rd yet.  return a pointer"
	$as_echo "     to the memory buffer so our caller can complete the load: */"
	$as_echo "  return (ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s"
	$as_echo "          - address_first);"
	$as_echo "}"
    fi

    # unfix the architecture version:
    #
    if $header; then :; else
	$as_echo ""
	$as_echo "#undef TME_SPARC_VERSION"
	$as_echo "#define TME_SPARC_VERSION(ic) _TME_SPARC_VERSION(ic)"
    fi

    # the sparc64 support depends on a 64-bit integer type:
    #
    if test ${arch} = 64; then
	$as_echo ""
	$as_echo "#endif /* TME_HAVE_INT64_T */"
    fi
done

# done:
#
exit 0
