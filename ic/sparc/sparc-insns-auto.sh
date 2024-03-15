#! /bin/sh
# Generated from sparc-insns-auto.m4 by GNU Autoconf 2.72.
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
	printf "%s\n" ""
	printf "%s\n" "#ifdef TME_HAVE_INT64_T"
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
	printf "%s\n" ""
	printf "%s\n" "#undef TME_SPARC_VERSION"
	printf "%s\n" "#define TME_SPARC_VERSION(ic) (${version})"
    fi

    # the alternate ASI function:
    #
    if $header; then :; else
	printf "%s\n" ""
	printf "%s\n" "static tme_uint32_t"
	printf "%s\n" "_tme_sparc${arch}_alternate_asi_mask(struct tme_sparc *ic)"
	printf "%s\n" "{"
	printf "%s\n" "  unsigned int asi_data;"
	printf "%s\n" "  unsigned int asi_mask_flags;"
	printf "%s\n" "  tme_uint32_t asi_mask_data;"
	printf "%s\n" ""
	printf "%s\n" "  /* get the ASI, assuming that the i bit is zero: */"
	printf "%s\n" "  asi_data = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0xff << 5));"
	if test ${arch} = 32; then
	    printf "%s\n" ""
	    printf "%s\n" "  /* this is a privileged instruction: */"
	    printf "%s\n" "  TME_SPARC_INSN_PRIV;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* if the i bit is one, this is an illegal instruction: */"
	    printf "%s\n" "  if (__tme_predict_false(TME_SPARC_INSN & TME_BIT(13))) {"
	    printf "%s\n" "    TME_SPARC_INSN_ILL(ic);"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the flags for this ASI: */"
	    printf "%s\n" "  asi_mask_flags = ic->tme_sparc_asis[asi_data].tme_sparc_asi_mask_flags;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* make the ASI mask: */"
	    printf "%s\n" "  if (asi_mask_flags & TME_SPARC32_ASI_MASK_FLAG_SPECIAL) {"
	    printf "%s\n" "    asi_mask_data"
	    printf "%s\n" "      = TME_SPARC_ASI_MASK_SPECIAL(asi_data, TRUE);"
	    printf "%s\n" "  }"
	    printf "%s\n" "  else {"
	    printf "%s\n" "    asi_mask_data = TME_SPARC32_ASI_MASK(asi_data, asi_data);"
	    printf "%s\n" "  }"
	else
	    printf "%s\n" ""
	    printf "%s\n" "  /* if the i bit is one, use the address space in the ASI register: */"
	    printf "%s\n" "  if (TME_SPARC_INSN & TME_BIT(13)) {"
	    printf "%s\n" "    asi_data = ic->tme_sparc64_ireg_asi;"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the flags for this ASI: */"
	    printf "%s\n" "  asi_mask_flags = ic->tme_sparc_asis[asi_data].tme_sparc_asi_mask_flags;"

	    printf "%s\n" ""
	    printf "%s\n" "  /* if this is a nonprivileged access: */"
	    printf "%s\n" "  if (!TME_SPARC_PRIV(ic)) {"
	    printf "%s\n" ""
	    printf "%s\n" "    /* if this is a restricted ASI: */"
	    printf "%s\n" "    if (__tme_predict_false((asi_data & TME_SPARC64_ASI_FLAG_UNRESTRICTED) == 0)) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* force a slow load or store, which will generate the"
	    printf "%s\n" "         privileged_action trap: */"
	    printf "%s\n" "      asi_mask_flags |= TME_SPARC_ASI_MASK_FLAG_UNDEF;"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* force a nonprivileged access with the ASI: */"
	    printf "%s\n" "    asi_mask_flags |= TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER;"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* make the ASI mask: */"
	    printf "%s\n" "  if (asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_SPECIAL) {"
	    printf "%s\n" "    asi_mask_data"
	    printf "%s\n" "      = (asi_mask_flags"
	    printf "%s\n" "         + TME_SPARC_ASI_MASK_SPECIAL(asi_data,"
	    printf "%s\n" "                                      ((asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER) == 0)));"
	    printf "%s\n" "  }"
	    printf "%s\n" "  else {"
	    printf "%s\n" "    asi_mask_data = TME_SPARC64_ASI_MASK(asi_data, asi_mask_flags);"
	    printf "%s\n" "  }"
	fi
	printf "%s\n" ""
	printf "%s\n" "  /* if this ASI has a special handler: */"
	printf "%s\n" "  if (__tme_predict_false(ic->tme_sparc_asis[TME_SPARC_ASI_MASK_WHICH(asi_mask_data)].tme_sparc_asi_handler != 0)) {"
	printf "%s\n" ""
	printf "%s\n" "    /* force a slow load or store, which will call the special handler: */"
	printf "%s\n" "    asi_mask_data |= TME_SPARC_ASI_MASK_FLAG_UNDEF;"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  return (asi_mask_data);"
	printf "%s\n" "}"
    fi

    # the FPU load and store common function:
    #
    if $header; then :; else
	printf "%s\n" ""
	printf "%s\n" "static struct tme_float *"
	printf "%s\n" "_tme_sparc${arch}_fpu_mem_fpreg(struct tme_sparc *ic,"
	printf "%s\n" "                           tme_uint32_t misaligned,"
	printf "%s\n" "                           struct tme_float *float_buffer)"
	printf "%s\n" "{"
	printf "%s\n" "  unsigned int float_format;"
	printf "%s\n" "  unsigned int fpreg_format;"
	printf "%s\n" "  tme_uint32_t fp_store;"
	printf "%s\n" "  unsigned int fpu_mode;"
	printf "%s\n" "  unsigned int fpreg_number;"
	printf "%s\n" ""
	printf "%s\n" "  /* NB: this checks for various traps by their priority order: */"
	printf "%s\n" ""
	printf "%s\n" "  TME_SPARC_INSN_FPU_ENABLED;"
	printf "%s\n" ""
	printf "%s\n" "  /* get the floating-point format: */"
	printf "%s\n" "  float_format = float_buffer->tme_float_format;"
	printf "%s\n" ""
	printf "%s\n" "  /* convert the floating-point format into the ieee754"
	printf "%s\n" "     floating-point register file format: */"
	printf "%s\n" "#if (TME_FLOAT_FORMAT_NULL | TME_IEEE754_FPREG_FORMAT_NULL) != 0"
	printf "%s\n" "#error \"TME_FLOAT_FORMAT_ or TME_IEEE754_FPREG_FORMAT_ values changed\""
	printf "%s\n" "#endif"
	printf "%s\n" "#if TME_FLOAT_FORMAT_IEEE754_SINGLE < TME_IEEE754_FPREG_FORMAT_SINGLE"
	printf "%s\n" "#error \"TME_FLOAT_FORMAT_ or TME_IEEE754_FPREG_FORMAT_ values changed\""
	printf "%s\n" "#endif"
	printf "%s\n" "#if (TME_FLOAT_FORMAT_IEEE754_SINGLE / TME_IEEE754_FPREG_FORMAT_SINGLE) != (TME_FLOAT_FORMAT_IEEE754_DOUBLE / TME_IEEE754_FPREG_FORMAT_DOUBLE)"
	printf "%s\n" "#error \"TME_FLOAT_FORMAT_ or TME_IEEE754_FPREG_FORMAT_ values changed\""
	printf "%s\n" "#endif"
	printf "%s\n" "  assert (float_format == TME_FLOAT_FORMAT_NULL"
	printf "%s\n" "          || float_format == TME_FLOAT_FORMAT_IEEE754_SINGLE"
	printf "%s\n" "          || float_format == TME_FLOAT_FORMAT_IEEE754_DOUBLE);"
	printf "%s\n" "  fpreg_format = float_format / (TME_FLOAT_FORMAT_IEEE754_SINGLE / TME_IEEE754_FPREG_FORMAT_SINGLE);"
	printf "%s\n" ""
	printf "%s\n" "  /* if the memory address is misaligned, return the"
	printf "%s\n" "     float buffer now.  the eventual load or store will"
	printf "%s\n" "     cause the mem_address_not_aligned trap: */"
	printf "%s\n" ""
	printf "%s\n" "  /* if the memory address is misaligned: */"
	printf "%s\n" "#if TME_IEEE754_FPREG_FORMAT_NULL != 0 || TME_IEEE754_FPREG_FORMAT_SINGLE != 1 || TME_IEEE754_FPREG_FORMAT_DOUBLE != 2 || TME_IEEE754_FPREG_FORMAT_QUAD != 4"
	printf "%s\n" "#error \"TME_IEEE754_FPREG_FORMAT_ values changed\""
	printf "%s\n" "#endif"
	printf "%s\n" "  assert (fpreg_format == TME_IEEE754_FPREG_FORMAT_NULL"
	printf "%s\n" "          || fpreg_format == TME_IEEE754_FPREG_FORMAT_SINGLE"
	printf "%s\n" "          || fpreg_format == TME_IEEE754_FPREG_FORMAT_DOUBLE"
	printf "%s\n" "          || fpreg_format == TME_IEEE754_FPREG_FORMAT_QUAD);"
	printf "%s\n" "  misaligned &= ((sizeof(tme_uint32_t) * fpreg_format) - 1);"
	printf "%s\n" "  if (__tme_predict_false(misaligned)) {"
	printf "%s\n" ""
	if test ${arch} = 32; then
	    printf "%s\n" "    return (float_buffer);"
	else
	    printf "%s\n" "    /* if the memory address is not even 32-bit aligned, or"
	    printf "%s\n" "       if this SPARC doesn't support loads and stores of this"
	    printf "%s\n" "       size at 32-bit alignment: */"
	    printf "%s\n" "    if (misaligned != sizeof(tme_uint32_t)"
	    printf "%s\n" "#if TME_IEEE754_FPREG_FORMAT_SINGLE != 1 || (TME_SPARC_MEMORY_FLAG_HAS_LDDF_STDF_32 * TME_IEEE754_FPREG_FORMAT_DOUBLE) != TME_SPARC_MEMORY_FLAG_HAS_LDQF_STQF_32"
	    printf "%s\n" "#error \"TME_IEEE754_FPREG_FORMAT_ or TME_SPARC_MEMORY_FLAG_ values changed\""
	    printf "%s\n" "#endif"
	    printf "%s\n" "        || (TME_SPARC_MEMORY_FLAGS(ic)"
	    printf "%s\n" "            & (TME_SPARC_MEMORY_FLAG_HAS_LDDF_STDF_32 * fpreg_format)) == 0) {"
	    printf "%s\n" ""
	    printf "%s\n" "      return (float_buffer);"
	    printf "%s\n" "    }"
	fi
        printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* see if this is a floating-point load or store: */"
	printf "%s\n" "  /* NB: all of the floating-point instructions that use"
	printf "%s\n" "     this preamble have bit two of op3 clear for a load,"
	printf "%s\n" "     and set for a store: */"
	printf "%s\n" "  fp_store = (TME_SPARC_INSN & TME_BIT(19 + 2));"
	printf "%s\n" ""
	printf "%s\n" "  /* if the FPU isn't in execute mode: */"
	printf "%s\n" "  fpu_mode = ic->tme_sparc_fpu_mode;"
	printf "%s\n" "  if (__tme_predict_false(fpu_mode != TME_SPARC_FPU_MODE_EXECUTE)) {"
	printf "%s\n" ""
	printf "%s\n" "    /* if this is a floating-point load, or if this is a"
	printf "%s\n" "       floating-point store and a floating-point exception"
	printf "%s\n" "       is pending: */"
	printf "%s\n" "    if (!fp_store"
	printf "%s\n" "        || fpu_mode == TME_SPARC_FPU_MODE_EXCEPTION_PENDING) {"
	printf "%s\n" ""
	printf "%s\n" "      /* do an FPU exception check: */"
	printf "%s\n" "      tme_sparc_fpu_exception_check(ic);"
	printf "%s\n" "    }"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* if this is not a load or store of a floating-point register: */"
	printf "%s\n" "  if (fpreg_format == TME_IEEE754_FPREG_FORMAT_NULL) {"
	printf "%s\n" "    return (float_buffer);"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* decode rd: */"
	printf "%s\n" "  fpreg_number"
	printf "%s\n" "    = tme_sparc_fpu_fpreg_decode(ic,"
	printf "%s\n" "                                 TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN,"
	printf "%s\n" "                                                         TME_SPARC_FORMAT3_MASK_RD),"
	printf "%s\n" "                                 fpreg_format);"
	printf "%s\n" ""
	printf "%s\n" "  /* make sure this floating-point register has the right precision: */"
	printf "%s\n" "  tme_sparc_fpu_fpreg_format(ic, fpreg_number, fpreg_format | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
	printf "%s\n" ""
	printf "%s\n" "  /* if this is a floating-point load: */"
	printf "%s\n" "  if (!fp_store) {"
	printf "%s\n" ""
	printf "%s\n" "    /* mark rd as dirty: */"
	printf "%s\n" "    TME_SPARC_FPU_DIRTY(ic, fpreg_number);"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* return the floating-point register: */"
	printf "%s\n" "  return (&ic->tme_sparc_fpu_fpregs[fpreg_number]);"
	printf "%s\n" "}"
	printf "%s\n" "#define _tme_sparc${arch}_fpu_mem(ic) \\"
	printf "%s\n" "  do { _tme_sparc${arch}_fpu_mem_fpreg(ic, 0, &_tme_sparc_float_null); } while (/* CONSTCOND */ 0)"
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
	    printf "%s\n" "TME_SPARC_FORMAT3_DECL(tme_sparc${arch}_${insn}, tme_uint${arch}_t);"
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
	    *) printf "%s\n" "$0 internal error: unknown ALU function ${insn}" 1>&2 ; exit 1 ;;
	    esac

	    # open the function:
	    #
	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} \"${insn} SRC1, SRC2, DST\": */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"

	    # declare our locals:
	    #
	    printf "%s\n" "  tme_${sign}int${size_src}_t src1;"
	    printf "%s\n" "  tme_${sign}int${size_src}_t src2;"
	    printf "%s\n" "  tme_${sign}int${size_dst}_t dst;"
	    case "${insn}" in
	    umul* | smul* | udiv* | sdiv*)
		printf "%s\n" "  tme_${sign}int64_t val64;"
		;;
	    mulscc)
		printf "%s\n" "  tme_uint32_t y;"
		;;
	    esac
	    if ${cc}; then
		printf "%s\n" "  tme_uint32_t cc;"
	    fi
	    cc_plus=

	    printf "%s\n" ""
	    printf "%s\n" "  /* get the operands: */"
	    printf "%s\n" "  src1 = (tme_${sign}int${arch}_t) TME_SPARC_FORMAT3_RS1;"
	    printf "%s\n" "  src2 = (tme_${sign}int${arch}_t) TME_SPARC_FORMAT3_RS2;"

	    printf "%s\n" ""
	    printf "%s\n" "  /* perform the operation: */"
	    case "${insn}" in
	    umul | umulcc | smul | smulcc)
		printf "%s\n" "  val64 = (((tme_${sign}int64_t) src1) * src2);"
		printf "%s\n" "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift}) = (((tme_uint64_t) val64) >> 32);"
		printf "%s\n" "  dst = ((tme_${sign}int64_t) val64);"
		;;
	    udiv | udivcc | sdiv | sdivcc)
		printf "%s\n" "  val64 = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift});"
		printf "%s\n" "  val64 = (val64 << 32) + (tme_uint32_t) src1;"
	        printf "%s\n" "  if (__tme_predict_false(src2 == 0)) {"
		printf "%s\n" "    tme_sparc${arch}_trap(ic, TME_SPARC${arch}_TRAP_division_by_zero);"
		printf "%s\n" "  }"
		printf "%s\n" "  val64 /= src2;"
		printf "%s\n" "  dst = (tme_${sign}int32_t) val64;"
		printf "%s\n" ""
		printf "%s\n" "  /* if the division overflowed: */"
		printf "%s\n" "  if (dst != val64) {"
		printf "%s\n" ""
		printf "%s\n" "    /* return the largest appropriate value: */"
		if test "x${sign}" = xu; then
		    printf "%s\n" "    dst = 0xffffffff;"
		else
		    printf "%s\n" "    dst = (tme_int32_t) ((val64 < 0) + (tme_uint32_t) 0x7fffffff);"
		fi
		if ${cc}; then
		    printf "%s\n" ""
		    printf "%s\n" "    /* set V: */"
		    printf "%s\n" "    cc = TME_SPARC${ccr}_ICC_V;"
		fi
		printf "%s\n" "  }"
		if ${cc}; then
		    printf "%s\n" ""
		    printf "%s\n" "  /* otherwise, the division didn't overflow: */"
		    printf "%s\n" "  else {"
		    printf "%s\n" ""
		    printf "%s\n" "    /* clear V: */"
		    printf "%s\n" "    cc = !TME_SPARC${ccr}_ICC_V;"
		    printf "%s\n" "  }"
		    cc_plus='+'
		fi
		;;
	    mulscc)
		printf "%s\n" ""
		printf "%s\n" "  /* \"(1) The multiplier is established as r[rs2] if the i field is zero, or "
		printf "%s\n" "     sign_ext(simm13) if the i field is one.\""
		printf "%s\n" ""
		printf "%s\n" "     \"(3) If the least significant bit of the Y register = 1, the shifted"
		printf "%s\n" "     value from step (2) is added to the multiplier. If the LSB of the"
		printf "%s\n" "     Y register = 0, then 0 is added to the shifted value from step (2).\" */"
		printf "%s\n" "  y = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift});"
		printf "%s\n" "  if ((y & 1) == 0) {"
		printf "%s\n" "    src2 = 0;"
		printf "%s\n" "  }"
		printf "%s\n" ""
		printf "%s\n" "  /* \"(6) The Y register is shifted right by one bit, with the LSB of the"
		printf "%s\n" "     unshifted r[rs1] replacing the MSB of Y.\" */"
		printf "%s\n" "  y >>= 1;"
		printf "%s\n" "  if (src1 & 1) {"
		printf "%s\n" "    y += 0x80000000;"
		printf "%s\n" "  }"
		printf "%s\n" "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y${reg32_shift}) = y;"
		printf "%s\n" ""
		printf "%s\n" "  /* \"(2) A 32-bit value is computed by shifting r[rs1] right by one"
		printf "%s\n" "     bit with (N xor V) from the PSR replacing the high-order bit."
		printf "%s\n" "     (This is the proper sign for the previous partial product.)\" */"
		printf "%s\n" "  src1 >>= 1;"
		printf "%s\n" "  if (((ic->${ccr_ireg} ^ (ic->${ccr_ireg} * (TME_SPARC${ccr}_ICC_N / TME_SPARC${ccr}_ICC_V))) & TME_SPARC${ccr}_ICC_N) != 0) {"
		printf "%s\n" "    src1 += 0x80000000;"
		printf "%s\n" "  }"
		printf "%s\n" ""
		printf "%s\n" "  /* \"(4) The sum from step (3) is written into r[rd].\" */"
		printf "%s\n" "  dst = src1 + src2;"
		printf "%s\n" ""
		printf "%s\n" "  /* \"(5) The integer condition codes, icc, are updated according to the"
		printf "%s\n" "     addition performed in step (3).\" */"
		;;
	    *)
		printf "%s\n" "  dst = ${op};"
		if test "x${with_c}" != x; then
		    printf "%s\n" "  dst ${with_c}= ((ic->${ccr_ireg} & TME_SPARC${ccr}_ICC_C) != 0);"
		fi
	    esac

	    # unless this is a tagged-and-trap-on-overflow operation:
	    #
	    if test "x${tagged}" != xtv; then
		printf "%s\n" ""
		printf "%s\n" "  /* store the destination: */"
		printf "%s\n" "  TME_SPARC_FORMAT3_RD = (tme_${sign}int${arch}_t) dst;"
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

		    printf "%s\n" ""
		    printf "%s\n" "  /* set Z if the destination is zero: */"
		    printf "%s\n" "  cc ${cc_plus}= ((((tme_int${arch_cc}_t) dst) == 0) * TME_SPARC${ccr}_${xcc}_Z);"
		    cc_plus='+'

		    if test `expr ${arch_cc} \<= ${size_dst}` = 1; then
			printf "%s\n" ""
			printf "%s\n" "  /* set N if the destination is negative: */"
			printf "%s\n" "  cc += ((((tme_int${arch_cc}_t) dst) < 0) * TME_SPARC${ccr}_${xcc}_N);"
		    fi

		    case $arith in
		    add)
			ones="(((tme_${sign}int${arch_cc}_t) 0) - 1)"

			printf "%s\n" ""
			printf "%s\n" "  /* if the operands are the same sign, and the destination has"
			printf "%s\n" "     a different sign, set V: */"
			printf "%s\n" "  cc += ((((tme_int${arch_cc}_t) ((src2 ^ dst) & (src1 ^ (src2 ^ ${ones})))) < 0) * TME_SPARC${ccr}_${xcc}_V);"

			printf "%s\n" ""
			printf "%s\n" "  /* if src1 and src2 both have the high bit set, or if dst does"
			printf "%s\n" "     not have the high bit set and either src1 or src2 does, set C: */"
			printf "%s\n" "  cc += (((tme_int${arch_cc}_t) (((tme_uint${arch_cc}_t) (src1 & src2)) | ((((tme_uint${arch_cc}_t) dst) ^ ${ones}) & ((tme_uint${arch_cc}_t) (src1 | src2))))) < 0) * TME_SPARC${ccr}_${xcc}_C;"
			;;
		    sub)

			printf "%s\n" ""
			printf "%s\n" "  /* if the operands are different signs, and the destination has"
			printf "%s\n" "     a different sign from the first operand, set V: */"
			printf "%s\n" "  cc += ((((tme_int${arch_cc}_t) ((src1 ^ src2) & (src1 ^ dst))) < 0) * TME_SPARC${ccr}_${xcc}_V);"

			printf "%s\n" ""
			printf "%s\n" "  /* if src2 is greater than src1, set C: */"
			printf %s "  cc += ((((tme_uint${arch_cc}_t) src2) > ((tme_uint${arch_cc}_t) src1))"
			if test "x${with_c}" != x; then
			    printf %s " || (((tme_uint${arch_cc}_t) src2) == ((tme_uint${arch_cc}_t) src1) && (ic->${ccr_ireg} & TME_SPARC${ccr}_ICC_C))"
			fi
			printf "%s\n" ") * TME_SPARC${ccr}_${xcc}_C;"
			;;
		    logical | mul)
			;;
		    udiv | sdiv)
		        # the udivcc and sdivcc V are handled in the operation code
			;;
		    *) printf "%s\n" "$0 internal error: unknown arithmetic type ${arith}" 1>&2 ; exit 1 ;;
		    esac
		done

		# if this is a tagged operation:
		#
		if test "x${tagged}" != x; then

		    printf "%s\n" ""
		    printf "%s\n" "  /* set V if bits zero or one of src1 or src2 are set: */"
		    printf "%s\n" "  cc |= ((((src1 | src2) & 3) != 0) * TME_SPARC${ccr}_ICC_V);"

		    # if this is a tagged-and-trap-on-overflow operation:
		    #
		    if test "x${tagged}" = xtv; then

			printf "%s\n" ""
			printf "%s\n" "  /* trap on a tagged overflow: */"
			printf "%s\n" "  if (cc & TME_SPARC${ccr}_ICC_V) {"
			printf "%s\n" "    tme_sparc${arch}_trap(ic, TME_SPARC${arch}_TRAP_tag_overflow);"
			printf "%s\n" "  }"

			printf "%s\n" "  /* store the destination: */"
			printf "%s\n" "  TME_SPARC_FORMAT3_RD = (tme_${sign}int${arch}_t) dst;"
		    fi
		fi

		printf "%s\n" ""
		printf "%s\n" "  /* set the condition codes: */"
		printf %s "  ic->${ccr_ireg} = "
		if test ${arch} = 32; then
		    printf %s "(ic->${ccr_ireg} & ~TME_SPARC32_PSR_ICC) | "
		fi
		printf "%s\n" "cc;"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# a shift instruction:
	#
	case ${insn} in
	sll | srl | sra)

	    # get the sign of this shift:
	    #
	    if test ${insn} = sra; then sign= ; else sign=u; fi

	    printf "%s\n" ""
	    printf "%s\n" "/* the sparc${arch} ${insn} function: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_${sign}int${arch}_t dst;"
	    printf "%s\n" "  unsigned int count;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the value and the shift count: */"
	    printf "%s\n" "  dst = TME_SPARC_FORMAT3_RS1;"
	    printf "%s\n" "  count = TME_SPARC_FORMAT3_RS2;"

	    # if we're on sparc64:
	    #
	    if test ${arch} = 64; then

		printf "%s\n" ""
		printf "%s\n" "  /* if the X bit is clear: */"
		printf "%s\n" "  if ((TME_SPARC_INSN & TME_BIT(12)) == 0) {"
		printf "%s\n" ""
		printf "%s\n" "    /* limit the count: */"
		printf "%s\n" "    count %= 32;"
		if test ${insn} != sll; then
		    printf "%s\n" ""
		    printf "%s\n" "    /* clip the value to 32 bits: */"
		    printf "%s\n" "    dst = (tme_${sign}int32_t) dst;"
		fi
		printf "%s\n" "  }"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  /* limit the count: */"
	    printf "%s\n" "  count %= ${arch};"
	    printf "%s\n" ""
	    printf "%s\n" "  /* do the shift: */"
	    if test "${insn}" = sra; then
		printf "%s\n" "#ifdef SHIFTSIGNED_INT${arch}_T"
	    fi
	    printf "%s\n" "#if defined(SHIFTMAX_INT${arch}_T) && (SHIFTMAX_INT${arch}_T < (${arch} - 1))"
	    printf "%s\n" "#error \"cannot do full shifts of a tme_int${arch}_t\""
	    printf "%s\n" "#endif /* (SHIFTMAX_INT${arch}_T < (${arch} - 1)) */"
	    if test ${insn} = sll; then
		printf "%s\n" "  dst <<= count;"
	    else
		printf "%s\n" "  dst >>= count;"
	    fi
	    if test "${insn}" = sra; then
		printf "%s\n" "#else  /* !SHIFTSIGNED_INT${arch}_T */"
		printf "%s\n" "  for (; count-- > 0; ) {"
		printf "%s\n" "    dst = (dst & ~((tme_${sign}int${arch}_t) 1)) / 2;"
		printf "%s\n" "  }"
		printf "%s\n" "#endif /* !SHIFTSIGNED_INT${arch}_T */"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  /* store the destination: */"
	    printf "%s\n" "  TME_SPARC_FORMAT3_RD = dst;"
	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
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
	    printf "%s\n" ""
	    printf "%s\n" "/* the sparc${arch} ${insn} function: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_${sign}int64_t src1;"
	    printf "%s\n" "  tme_${sign}int64_t src2;"
	    printf "%s\n" "  tme_${sign}int64_t dst;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the operands: */"
	    printf "%s\n" "  src1 = TME_SPARC_FORMAT3_RS1;"
	    printf "%s\n" "  src2 = TME_SPARC_FORMAT3_RS2;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* do the ${insn}: */"
	    if test ${insn} = mulx; then
		printf "%s\n" "  dst = src1 * src2;"
	    else
		printf "%s\n" "  if (__tme_predict_false(src2 == 0)) {"
		printf "%s\n" "    tme_sparc${arch}_trap(ic, TME_SPARC${arch}_TRAP_division_by_zero);"
		printf "%s\n" "  }"
		if test ${insn} = sdivx; then
		    printf "%s\n" "  dst = (src2 == -1 && src1 == (((tme_int64_t) 1) << 63) ? src1 : src1 / src2);"
		else
		    printf "%s\n" "  dst = src1 / src2;"
		fi
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_FORMAT3_RD = dst;"
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
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
	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"

	    # our locals:
	    #
	    if ${alternate}; then
		printf "%s\n" "  tme_uint32_t asi_mask_data;"
		asi_mask_data=asi_mask_data
	    else
		asi_mask_data="ic->tme_sparc_asi_mask_data"
	    fi
	    printf "%s\n" "  tme_uint${arch}_t address;"
	    if test ${arch} = 64 && ${alternate}; then
		printf "%s\n" "  tme_bus_context_t context;"
		context=context
	    else
		context="ic->tme_sparc_memory_context_default"
	    fi
	    printf "%s\n" "  tme_uint32_t asi_mask_flags_slow;"
	    printf "%s\n" "  struct tme_sparc_tlb *dtlb;"
	    printf %s "  "
	    if test ${slow} = load; then printf %s "const "; fi
	    printf "%s\n" "tme_shared tme_uint8_t *memory;"
	    printf "%s\n" "  tme_bus_context_t dtlb_context;"
	    printf "%s\n" "  tme_uint32_t endian_little;"
	    case "${insn}" in
	    ldd* | std* | swap*) printf "%s\n" "  tme_uint32_t value32;" ;;
	    ldstub*) ;;
	    cas*a)
	        printf "%s\n" "  unsigned int reg_rs2;"
	        printf "%s\n" "  tme_uint${size}_t value_compare${size};"
	        printf "%s\n" "  tme_uint${size}_t value_swap${size};"
	        printf "%s\n" "  tme_uint${size}_t value_read${size};"
		;;
	    ld*)
	        printf "%s\n" "  tme_uint${size}_t value${size};"
		size_extend=${arch}
		if test `expr ${size} \< ${arch}` = 1; then
		    if test `expr ${size} \< 32` = 1; then size_extend=32; fi
		    printf "%s\n" "  tme_uint${size_extend}_t value${size_extend};"
		fi
		;;
	    st*) printf "%s\n" "  tme_uint${size}_t value${size};" ;;
	    esac

	    if ${alternate}; then
		printf "%s\n" ""
		printf "%s\n" "  /* get the alternate ASI mask: */"
		printf "%s\n" "  asi_mask_data = _tme_sparc${arch}_alternate_asi_mask(ic);"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  /* get the address: */"
	    case "${insn}" in
	    cas*a) printf "%s\n" "  address = TME_SPARC_FORMAT3_RS1;" ;;
	    *) printf "%s\n" "  address = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;" ;;
	    esac
	    if test ${arch} = 64; then
		printf "%s\n" "  address &= ic->tme_sparc_address_mask;"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "#ifdef _TME_SPARC_STATS"
	    printf "%s\n" "  /* track statistics: */"
	    printf "%s\n" "  ic->tme_sparc_stats.tme_sparc_stats_memory_total++;"
	    printf "%s\n" "#endif /* _TME_SPARC_STATS */"

	    printf "%s\n" ""
	    printf "%s\n" "  /* verify and maybe replay this transfer: */"
	    if ${double}; then verify_size=32; else verify_size=${size}; fi
	    case "${insn}" in
	    ld*) verify_flags=TME_SPARC_RECODE_VERIFY_MEM_LOAD ;;
	    swap* | cas*a) verify_flags="TME_SPARC_RECODE_VERIFY_MEM_LOAD | TME_SPARC_RECODE_VERIFY_MEM_STORE" ;;
	    st*) verify_flags=TME_SPARC_RECODE_VERIFY_MEM_STORE ;;
	    *) verify_flags= ;;
	    esac
	    printf "%s\n" "  tme_sparc_recode_verify_mem(ic, &TME_SPARC_FORMAT3_RD,"
	    printf "%s\n" "                              ${asi_mask_data}, address,"
	    printf "%s\n" "                              (TME_RECODE_SIZE_${verify_size}"
	    printf "%s\n" "                               | ${verify_flags}));"
	    if ${double}; then
		printf "%s\n" "  tme_sparc_recode_verify_mem(ic, &TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}),"
		printf "%s\n" "                              ${asi_mask_data}, address + sizeof(tme_uint32_t),"
		printf "%s\n" "                              (TME_RECODE_SIZE_${verify_size}"
		printf "%s\n" "                               | ${verify_flags}));"
	    fi
	    printf "%s\n" "  if (tme_sparc_recode_verify_replay_last_pc(ic) != 0) {"
	    printf "%s\n" "    TME_SPARC_INSN_OK;"
	    printf "%s\n" "  }"

	    # if this is some kind of a store, except for an ldstub:
	    #
	    case "${insn}" in
	    std*)
		printf "%s\n" ""
		printf "%s\n" "  /* log the values stored: */"
		printf "%s\n" "  tme_sparc_log(ic, 1000, TME_OK, "
		printf "%s\n" "               (TME_SPARC_LOG_HANDLE(ic),"
		printf "%s\n" "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%08\" TME_PRIx32 \" 0x%08\" TME_PRIx32),"
		printf "%s\n" "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		printf "%s\n" "                address,"
		printf "%s\n" "                (tme_uint32_t) TME_SPARC_FORMAT3_RD,"
		printf "%s\n" "                (tme_uint32_t) TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch})));"
		;;
	    st* | swap* | cas*a)
		printf "%s\n" ""
		printf "%s\n" "  /* log the value stored: */"
		printf "%s\n" "  tme_sparc_log(ic, 1000, TME_OK, "
		printf "%s\n" "               (TME_SPARC_LOG_HANDLE(ic),"
		printf "%s\n" "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${size} / 4`"\" TME_PRIx${size}),"
		printf "%s\n" "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		printf "%s\n" "                address,"
		printf "%s\n" "                (tme_uint${size}_t) TME_SPARC_FORMAT3_RD));"
		;;
	    esac

	    if test "${context}" = context; then
		printf "%s\n" ""
		printf "%s\n" "  /* get the context: */"
		if test ${arch} = 64; then
		    printf "%s\n" "  context = ic->tme_sparc_memory_context_primary;"
		    printf "%s\n" "  if (__tme_predict_false(${asi_mask_data}"
		    printf "%s\n" "                          & (TME_SPARC64_ASI_FLAG_SECONDARY"
		    printf "%s\n" "                             + TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS))) {"
		    printf "%s\n" "    if (${asi_mask_data} & TME_SPARC64_ASI_FLAG_SECONDARY) {"
		    printf "%s\n" "      context = ic->tme_sparc_memory_context_secondary;"
		    printf "%s\n" "    }"
		    printf "%s\n" "    else if (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS) {"
		    printf "%s\n" "      context = 0;"
		    printf "%s\n" "    }"
		    printf "%s\n" "  }"
		fi
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  /* assume that no DTLB ASI mask flags will require a slow ${slow}: */"
	    printf "%s\n" "  asi_mask_flags_slow = 0;"
	    if test ${arch} = 64; then

		printf "%s\n" ""
		if test ${slow} != load || ${atomic}; then
		    printf "%s\n" "  /* a ${insn} traps on no-fault addresses: */"
		else
		    printf "%s\n" "  /* a ${insn} without a no-fault ASI traps on no-fault addresses: */"
		fi
		printf "%s\n" "  asi_mask_flags_slow |= TME_SPARC64_ASI_FLAG_NO_FAULT;"
		if ${atomic}; then
		    printf "%s\n" ""
		    printf "%s\n" "  /* a ${insn} traps on uncacheable addresses with side-effects: */"
		    printf "%s\n" "  asi_mask_flags_slow |= TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE;"
		fi

		if ${alternate}; then
		    printf "%s\n" ""
		    printf "%s\n" "  /* if this ${insn} is using a no-fault ASI: */"
		    printf "%s\n" "  if (__tme_predict_false(${asi_mask_data} & TME_SPARC64_ASI_FLAG_NO_FAULT)) {"
		    printf "%s\n" ""
		    if test ${slow} != load || ${atomic}; then
			printf "%s\n" "    /* a ${insn} with a no-fault ASI traps: */"
			printf "%s\n" "    asi_mask_flags_slow = 0 - (tme_uint32_t) 1;"
		    else
			printf "%s\n" "    /* a ${insn} with a no-fault ASI traps on addresses with side-effects: */"
			printf "%s\n" "    asi_mask_flags_slow = TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS;"
		    fi
		    printf "%s\n" "  }"
		fi
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  /* get and busy the DTLB entry: */"
	    printf "%s\n" "  dtlb = &ic->tme_sparc_tlbs[TME_SPARC_DTLB_ENTRY(ic, TME_SPARC_TLB_HASH(ic, ${context}, address))];"
	    printf "%s\n" "  tme_sparc_tlb_busy(dtlb);"

	    printf "%s\n" ""
	    printf "%s\n" "  /* assume that this DTLB applies and allows fast transfers: */"
	    printf "%s\n" "  memory = dtlb->tme_sparc_tlb_emulator_off_${cycle};"

	    printf "%s\n" ""
	    printf "%s\n" "  /* if this DTLB matches any context, it matches this context: */"
	    printf "%s\n" "  dtlb_context = dtlb->tme_sparc_tlb_context;"
	    printf "%s\n" "  if (dtlb_context > ic->tme_sparc_memory_context_max) {"
	    printf "%s\n" "    dtlb_context = ${context};"
	    printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* we must call the slow ${slow} function if: */"
	    printf "%s\n" "  if (__tme_predict_false("
	    printf "%s\n" ""
	    printf "%s\n" "                          /* the DTLB entry is invalid: */"
	    printf "%s\n" "                          tme_bus_tlb_is_invalid(&dtlb->tme_sparc_tlb_bus_tlb)"
	    printf "%s\n" ""
	    printf "%s\n" "                          /* the DTLB entry does not match the context: */"
	    printf "%s\n" "                          || dtlb_context != ${context}"
	    printf "%s\n" ""
	    printf "%s\n" "                          /* the DTLB entry does not cover the needed addresses: */"
	    printf "%s\n" "                          || (address < (tme_bus_addr${arch}_t) dtlb->tme_sparc_tlb_addr_first)"
	    printf "%s\n" "                          || ((address + ((${size} / 8) - 1)) > (tme_bus_addr${arch}_t) dtlb->tme_sparc_tlb_addr_last)"
	    printf "%s\n" ""
	    printf "%s\n" "                          /* the DTLB entry does not cover the needed address space: */"
	    printf "%s\n" "                          || (!TME_SPARC_TLB_ASI_MASK_OK(dtlb, ${asi_mask_data}))"
	    printf "%s\n" ""
	    printf "%s\n" "                          /* the DTLB entry can't be used for a fast ${insn}: */"
	    printf "%s\n" "                          || (dtlb->tme_sparc_tlb_asi_mask & asi_mask_flags_slow) != 0"
	    printf "%s\n" ""
	    printf "%s\n" "                          /* the DTLB entry does not allow fast transfers: */"
	    if $atomic; then
		printf "%s\n" "                          || (memory != dtlb->tme_sparc_tlb_emulator_off_read)"
	    fi
	    printf "%s\n" "                          || (memory == TME_EMULATOR_OFF_UNDEF)"
	    if test ${size} != 8; then
		printf "%s\n" ""
		printf "%s\n" "                          /* the address is misaligned: */"
		printf "%s\n" "                          || ((address % (${size} / 8)) != 0)"
	    fi
	    if ${double}; then
		printf "%s\n" ""
		printf "%s\n" "                          /* the destination register number is odd: */"
		printf "%s\n" "                          || ((TME_SPARC_INSN & TME_BIT(25)) != 0)"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "                          )) {"

	    printf "%s\n" ""
	    printf "%s\n" "    /* call the slow ${slow} function: */"
	    printf "%s\n" "    memory = tme_sparc${arch}_ls(ic,"
	    printf "%s\n" "                            address,"
	    printf "%s\n" "                            &TME_SPARC_FORMAT3_RD,"
	    printf %s "                            (TME_SPARC_LSINFO_OP_"
	    if ${atomic}; then
		printf "%s\n" "ATOMIC"
	    elif test ${slow} = store; then
		printf "%s\n" "ST"
	    else
		printf "%s\n" "LD"
	    fi
	    printf %s "                             | "
	    case ${insn} in
	    ldd* | std*)
		printf "%s\n" "TME_SPARC_LSINFO_LDD_STD"
		printf %s "                             | "
		;;
	    esac
	    if ${alternate}; then
		printf "%s\n" "TME_SPARC_LSINFO_ASI(TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF))"
		printf "%s\n" "                             | TME_SPARC_LSINFO_A"
		printf %s "                             | "
	    fi
	    printf "%s\n" "(${size} / 8)));"

	    if test ${slow} = store || ${alternate}; then
		printf "%s\n" ""
		printf "%s\n" "    /* if the slow ${slow} function did the transfer: */"
		printf "%s\n" "    if (__tme_predict_false(memory == TME_EMULATOR_OFF_UNDEF)) {"
		printf "%s\n" ""
		printf "%s\n" "      /* unbusy the TLB entry; */"
		printf "%s\n" "      tme_sparc_tlb_unbusy(dtlb);"

	    	# if this is some kind of a load, log the value loaded:
	    	#
		case ${insn} in
		ldd*)
		    printf "%s\n" ""
		    printf "%s\n" "      /* log the value loaded: */"
		    printf "%s\n" "      tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		    printf "%s\n" "      tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}));"
		    printf "%s\n" "      tme_sparc_log(ic, 1000, TME_OK,"
		    printf "%s\n" "                   (TME_SPARC_LOG_HANDLE(ic),"
		    printf "%s\n" "                    _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \" 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \"\"),"
		    printf "%s\n" "                    TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		    printf "%s\n" "                    address,"
		    printf "%s\n" "                    TME_SPARC_FORMAT3_RD,"
		    printf "%s\n" "                    TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch})));"
		    ;;
		ld* | ldstub* | swap* | cas*a)
		    printf "%s\n" ""
		    printf "%s\n" "      /* log the value loaded: */"
		    printf "%s\n" "      tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		    printf "%s\n" "      tme_sparc_log(ic, 1000, TME_OK,"
		    printf "%s\n" "                   (TME_SPARC_LOG_HANDLE(ic),"
		    printf "%s\n" "                    _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${size} / 4`"\" TME_PRIx${arch}),"
		    printf "%s\n" "                    TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		    printf "%s\n" "                    address,"
		    printf "%s\n" "                    TME_SPARC_FORMAT3_RD));"
		    ;;
		esac
		printf "%s\n" ""
		printf "%s\n" "      TME_SPARC_INSN_OK;"
		printf "%s\n" "    }"
	    fi
	    printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* get the byte order of this transfer: */"
	    if test ${arch} = 32; then
		printf "%s\n" "  endian_little = FALSE;"
	    elif test ${arch} = 64; then
		printf "%s\n" "  endian_little = ${asi_mask_data} & TME_SPARC64_ASI_FLAG_LITTLE;"
		printf "%s\n" "  if (__tme_predict_false(dtlb->tme_sparc_tlb_asi_mask & TME_SPARC64_ASI_FLAG_LITTLE)) {"
		printf "%s\n" "    if (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_INVERT_ENDIAN) {"
		printf "%s\n" "      endian_little ^= TME_SPARC64_ASI_FLAG_LITTLE;"
		printf "%s\n" "    }"
		printf "%s\n" "    else {"
		printf "%s\n" "      assert (FALSE);"
		printf "%s\n" "    }"
		printf "%s\n" "  }"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  /* do the fast transfer: */"
	    printf "%s\n" "  memory += address;"

	    # dispatch on the instruction:
	    #
	    case "${insn}" in
	    ldd*)
		printf "%s\n" "  value32 = tme_memory_bus_read32(((const tme_shared tme_uint32_t *) memory) + 0, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 2, sizeof(tme_uint${arch}_t));"
		printf "%s\n" "  value32 = (endian_little ? tme_letoh_u32(value32) : tme_betoh_u32(value32));"
		printf "%s\n" "  TME_SPARC_FORMAT3_RD = value32;"
		printf "%s\n" "  value32 = tme_memory_bus_read32(((const tme_shared tme_uint32_t *) memory) + 1, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 1, sizeof(tme_uint${arch}_t));"
		printf "%s\n" "  value32 = (endian_little ? tme_letoh_u32(value32) : tme_betoh_u32(value32));"
		printf "%s\n" "  TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}) = value32;"
		;;
	    std*)
	        printf "%s\n" "  value32 = TME_SPARC_FORMAT3_RD;"
		printf "%s\n" "  value32 = (endian_little ? tme_htole_u32(value32) : tme_htobe_u32(value32));"
		printf "%s\n" "  tme_memory_bus_write32(((tme_shared tme_uint32_t *) memory) + 0, value32, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 2, sizeof(tme_uint${arch}_t));"
		printf "%s\n" "  value32 = TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch});"
		printf "%s\n" "  value32 = (endian_little ? tme_htole_u32(value32) : tme_htobe_u32(value32));"
		printf "%s\n" "  tme_memory_bus_write32(((tme_shared tme_uint32_t *) memory) + 1, value32, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint32_t) * 1, sizeof(tme_uint${arch}_t));"
		;;
	    ldstub*)
		printf "%s\n" "  TME_SPARC_FORMAT3_RD = tme_memory_atomic_xchg8(memory, 0xff, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint8_t));"
		;;
	    swap*)
		printf "%s\n" "  value32 = TME_SPARC_FORMAT3_RD;"
		printf "%s\n" "  value32 = (endian_little ? tme_htole_u32(value32) : tme_htobe_u32(value32));"
		printf "%s\n" "  value32 = tme_memory_atomic_xchg32((tme_shared tme_uint${size}_t *) memory, value32, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint8_t));"
		printf "%s\n" "  value32 = (endian_little ? tme_letoh_u32(value32) : tme_betoh_u32(value32));"
		printf "%s\n" "  TME_SPARC_FORMAT3_RD = value32;"
		;;
	    cas*a)
	        printf "%s\n" "  reg_rs2 = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RS2);"
		printf "%s\n" "  TME_SPARC_REG_INDEX(ic, reg_rs2);"
		printf "%s\n" "  value_compare${size} = ic->tme_sparc_ireg_uint${arch}(reg_rs2);"
		printf "%s\n" "  value_compare${size} = (endian_little ? tme_htole_u${size}(value_compare${size}) : tme_htobe_u${size}(value_compare${size}));"
		printf "%s\n" "  value_swap${size} = TME_SPARC_FORMAT3_RD;"
		printf "%s\n" "  value_swap${size} = (endian_little ? tme_htole_u${size}(value_swap${size}) : tme_htobe_u${size}(value_swap${size}));"
		printf "%s\n" "  value_read${size} = tme_memory_atomic_cx${size}((tme_shared tme_uint${size}_t *) memory, value_compare${size}, value_swap${size}, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint${size}_t));"
		printf "%s\n" "  value_read${size} = (endian_little ? tme_letoh_u${size}(value_read${size}) : tme_betoh_u${size}(value_read${size}));"
		printf "%s\n" "  TME_SPARC_FORMAT3_RD = value_read${size};"
		;;
	    ld*)
		printf "%s\n" "  value${size} = tme_memory_bus_read${size}((const tme_shared tme_uint${size}_t *) memory, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint${size}_t), sizeof(tme_uint${arch}_t));"
		if test ${size} != 8; then
		    printf "%s\n" "  value${size} = (endian_little ? tme_letoh_u${size}(value${size}) : tme_betoh_u${size}(value${size}));"
		fi
		if test `expr ${size} \< ${arch}` = 1; then
		    printf "%s\n" ""
		    printf "%s\n" "  /* possibly sign-extend the loaded value: */"
		    printf "%s\n" "  value${size_extend} = value${size};"
		    printf "%s\n" "  if (TME_SPARC_INSN & TME_BIT(22)) {"
		    printf "%s\n" "    value${size_extend} = (tme_uint${size_extend}_t) (tme_int${size_extend}_t) (tme_int${size}_t) value${size_extend};"
		    printf "%s\n" "  }"
		fi
		printf "%s\n" "  TME_SPARC_FORMAT3_RD = (tme_uint${arch}_t) (tme_int${arch}_t) (tme_int${size_extend}_t) value${size_extend};"
		;;
	    st*)
		printf "%s\n" "  value${size} = TME_SPARC_FORMAT3_RD;"
		if test ${size} != 8; then
		    printf "%s\n" "  value${size} = (endian_little ? tme_htole_u${size}(value${size}) : tme_htobe_u${size}(value${size}));"
		fi
		printf "%s\n" "  tme_memory_bus_write${size}((tme_shared tme_uint${size}_t *) memory, value${size}, dtlb->tme_sparc_tlb_bus_rwlock, sizeof(tme_uint${size}_t), sizeof(tme_uint${arch}_t));"
		;;
	    *) printf "%s\n" "$PROG internal error: unknown memory insn ${insn}" 1>&2 ; exit 1 ;;
	    esac

	    printf "%s\n" ""
	    printf "%s\n" "  /* unbusy the DTLB entry: */"
	    printf "%s\n" "  tme_sparc_tlb_unbusy(dtlb);"

	    # if this is some kind of a load, log the value loaded:
	    #
	    case ${insn} in
	    ldd*)
		printf "%s\n" ""
		printf "%s\n" "  /* log the value loaded: */"
		printf "%s\n" "  tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		printf "%s\n" "  tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch}));"
		printf "%s\n" "  tme_sparc_log(ic, 1000, TME_OK,"
		printf "%s\n" "               (TME_SPARC_LOG_HANDLE(ic),"
		printf "%s\n" "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \" 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \"\"),"
		printf "%s\n" "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		printf "%s\n" "                address,"
		printf "%s\n" "                TME_SPARC_FORMAT3_RD,"
		printf "%s\n" "                TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch})));"
		;;
	    ld* | ldstub* | swap* | cas*a)
		printf "%s\n" ""
		printf "%s\n" "  /* log the value loaded: */"
		printf "%s\n" "  tme_sparc_recode_verify_mem_load(ic, &TME_SPARC_FORMAT3_RD);"
		printf "%s\n" "  tme_sparc_log(ic, 1000, TME_OK,"
		printf "%s\n" "               (TME_SPARC_LOG_HANDLE(ic),"
		printf "%s\n" "                _(\"${insn}\t0x%02x:0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch} \":\t0x%0"`expr ${size} / 4`"\" TME_PRIx${arch}),"
		printf "%s\n" "                TME_SPARC_ASI_MASK_WHICH(${asi_mask_data} & ~TME_SPARC_ASI_MASK_FLAG_UNDEF),"
		printf "%s\n" "                address,"
		printf "%s\n" "                TME_SPARC_FORMAT3_RD));"
		;;
	    esac

	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# the jmpl instruction:
	#
	if test ${insn} = jmpl; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_uint${arch}_t pc_next_next;"
	    printf "%s\n" "  tme_uint32_t ls_faults;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* \"The JMPL instruction causes a register-indirect delayed control"
	    printf "%s\n" "     transfer to the address given by r[rs1] + r[rs2] if the i field is"
	    printf "%s\n" "     zero, or r[rs1] + sign_ext(simm13) if the i field is one. The JMPL"
	    printf "%s\n" "     instruction copies the PC, which contains the address of the JMPL"
	    printf "%s\n" "     instruction, into register r[rd]. If either of the low-order two"
	    printf "%s\n" "     bits of the jump address is nonzero, a mem_address_not_aligned"
	    printf "%s\n" "     trap occurs.\" */"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the target address: */"
	    printf "%s\n" "  pc_next_next = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;"
	    if test ${arch} = 64; then
		printf "%s\n" "  pc_next_next &= ic->tme_sparc_address_mask;"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* set the delayed control transfer: */"
	    printf "%s\n" "  ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* check the target address: */"
	    printf "%s\n" "  ls_faults = TME_SPARC_LS_FAULT_NONE;"
	    if test ${arch} = 64; then
		printf "%s\n" "  if (__tme_predict_false((pc_next_next"
		printf "%s\n" "                           + ic->tme_sparc${arch}_ireg_va_hole_start)"
		printf "%s\n" "                          > ((ic->tme_sparc${arch}_ireg_va_hole_start * 2) - 1))) {"
		printf "%s\n" "    ls_faults += TME_SPARC64_LS_FAULT_VA_RANGE_NNPC;"
		printf "%s\n" "  }"
	    fi
	    printf "%s\n" "  if (__tme_predict_false((pc_next_next % sizeof(tme_uint32_t)) != 0)) {"
	    printf "%s\n" "    ls_faults += TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;"
	    printf "%s\n" "  }"
	    printf "%s\n" "  if (__tme_predict_false(ls_faults != TME_SPARC_LS_FAULT_NONE)) {"
	    printf "%s\n" "    tme_sparc_nnpc_trap(ic, ls_faults);"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* write the PC of the jmpl into r[rd]: */"
	    printf "%s\n" "  TME_SPARC_FORMAT3_RD = ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_PC);"
	    printf "%s\n" ""
	    printf "%s\n" "  /* log an indirect call instruction, which has 15 (%o7) for rd: */"
	    printf "%s\n" "  if (TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD) == 15) {"
	    printf "%s\n" "    tme_sparc_log(ic, 250, TME_OK,"
	    printf "%s\n" "                  (TME_SPARC_LOG_HANDLE(ic),"
	    printf "%s\n" "                   _(\"call 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch}),"
	    printf "%s\n" "                   pc_next_next));"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* log a ret or retl instruction, which has 0 (%g0) for rd,"
	    printf "%s\n" "     either 31 (%i7) or 15 (%o7) for rs1, and 8 for simm13: */"
	    printf "%s\n" "  else if ((TME_SPARC_INSN | (16 << 14))"
	    printf "%s\n" "           == ((tme_uint32_t) (0x2 << 30) | (0 << 25) | (0x38 << 19) | (31 << 14) | (0x1 << 13) | 8)) {"
	    printf "%s\n" "    tme_sparc_log(ic, 250, TME_OK,"
	    printf "%s\n" "                  (TME_SPARC_LOG_HANDLE(ic),"
	    printf "%s\n" "                   _(\"retl 0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch}),"
	    printf "%s\n" "                   pc_next_next));"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# this may be an alternate-space version of a floating-point
	# load or store instruction:
	#
	case ${insn} in *a) alternate=a ;; *) alternate= ;; esac

	# the ldf instruction:
	#
	if test ${insn} = "ldf${alternate}"; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_uint32_t misaligned;"
	    printf "%s\n" "  struct tme_float float_buffer;"
	    printf "%s\n" "  struct tme_float *fpreg;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the least significant 32 bits of the address: */"
	    printf "%s\n" "  misaligned = TME_SPARC_FORMAT3_RS1;"
	    printf "%s\n" "  misaligned += (tme_uint32_t) TME_SPARC_FORMAT3_RS2;"
	    if test "${arch}${alternate}" = 64a; then
		printf "%s\n" ""
		printf "%s\n" "  /* see if the address is misaligned for the ASI: */"
		printf "%s\n" "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* decode rd: */"
	    printf "%s\n" "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
	    printf "%s\n" "  fpreg"
	    printf "%s\n" "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    printf "%s\n" "                                 misaligned,"
	    printf "%s\n" "                                 &float_buffer);"
	    printf "%s\n" ""
	    printf "%s\n" "  /* do the load: */"
	    printf "%s\n" "  tme_sparc${arch}_ld${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    printf "%s\n" ""
	    printf "%s\n" "  /* set the floating-point register value: */"
	    printf "%s\n" "  assert (fpreg != &float_buffer);"
	    printf "%s\n" "  fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
	    printf "%s\n" "  fpreg->tme_float_value_ieee754_single"
	    printf "%s\n" "    = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift});"
	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# the stf instruction:
	#
	if test ${insn} = "stf${alternate}"; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_uint32_t misaligned;"
	    printf "%s\n" "  struct tme_float float_buffer;"
	    printf "%s\n" "  const struct tme_float *fpreg;"
	    printf "%s\n" "  const tme_uint32_t *value_single;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the least significant 32 bits of the address: */"
	    printf "%s\n" "  misaligned = TME_SPARC_FORMAT3_RS1;"
	    printf "%s\n" "  misaligned += (tme_uint32_t) TME_SPARC_FORMAT3_RS2;"
	    if test "${arch}${alternate}" = 64a; then
		printf "%s\n" ""
		printf "%s\n" "  /* see if the address is misaligned for the ASI: */"
		printf "%s\n" "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* decode rd: */"
	    printf "%s\n" "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
	    printf "%s\n" "  fpreg"
	    printf "%s\n" "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    printf "%s\n" "                                 misaligned,"
	    printf "%s\n" "                                 &float_buffer);"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get this single floating-point register in IEEE754 single-precision format: */"
	    printf "%s\n" "  value_single = tme_ieee754_single_value_get(fpreg, &float_buffer.tme_float_value_ieee754_single);"
	    printf "%s\n" ""
	    printf "%s\n" "  /* set the floating-point register value: */"
	    printf "%s\n" "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}) = *value_single;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* do the store: */"
	    printf "%s\n" "  tme_sparc${arch}_st${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    printf "%s\n" ""
	    printf "%s\n" "  assert (fpreg != &float_buffer);"
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# the lddf instruction:
	#
	if test ${insn} = "lddf${alternate}"; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_uint${arch}_t address;"
	    printf "%s\n" "  tme_uint32_t misaligned;"
	    printf "%s\n" "  struct tme_float float_buffer;"
	    printf "%s\n" "  struct tme_float *fpreg;"
	    if test ${arch} != 32; then
		printf "%s\n" "  tme_uint${arch}_t offset;"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the address: */"
	    printf "%s\n" "  address = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the least significant 32 bits of the address: */"
	    printf "%s\n" "  misaligned = address;"
	    if test "${arch}${alternate}" = 64a; then
		printf "%s\n" ""
		printf "%s\n" "  /* see if the address is misaligned for the ASI: */"
		printf "%s\n" "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* decode rd: */"
	    printf "%s\n" "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
	    printf "%s\n" "  fpreg"
	    printf "%s\n" "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    printf "%s\n" "                                 misaligned,"
	    printf "%s\n" "                                 &float_buffer);"
	    printf "%s\n" ""
	    if test ${arch} = 32; then
		printf "%s\n" "  /* do the load: */"
		printf "%s\n" "  tme_sparc${arch}_ldd(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}));"
		printf "%s\n" ""
		printf "%s\n" "  /* set the double floating-point register value: */"
		printf "%s\n" "  assert (fpreg != &float_buffer);"
		printf "%s\n" "  fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		printf "%s\n" "  fpreg->tme_float_value_ieee754_double.tme_value64_uint32_hi"
		printf "%s\n" "    = ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 0);"
		printf "%s\n" "  fpreg->tme_float_value_ieee754_double.tme_value64_uint32_lo"
		printf "%s\n" "    = ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1);"
	    else
		printf "%s\n" "  /* if bit two of the address is set, and this SPARC supports"
		printf "%s\n" "     32-bit-aligned ${insn} instructions: */"
		printf "%s\n" "  if ((misaligned & sizeof(tme_uint32_t))"
		printf "%s\n" "      && fpreg != &float_buffer) {"
		printf "%s\n" ""
		printf "%s\n" "    /* do two 32-bit loads: */"
		printf "%s\n" "    offset = sizeof(tme_uint32_t) * 0;"
		printf "%s\n" "    tme_sparc${arch}_ld${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 0));"
		printf "%s\n" "    offset = sizeof(tme_uint32_t) * 1;"
		printf "%s\n" "    tme_sparc${arch}_ld${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 1));"
		printf "%s\n" ""
		printf "%s\n" "    /* set the double floating-point register value: */"
		printf "%s\n" "    fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		printf "%s\n" "    fpreg->tme_float_value_ieee754_double.tme_value64_uint32_hi"
		printf "%s\n" "      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX + 0);"
		printf "%s\n" "    fpreg->tme_float_value_ieee754_double.tme_value64_uint32_lo"
		printf "%s\n" "      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX + 1);"
		printf "%s\n" "  }"
		printf "%s\n" ""
		printf "%s\n" "  /* otherwise, bit two of the address is not set, or this SPARC"
		printf "%s\n" "     doesn't support 32-bit-aligned ${insn} instructions: */"
		printf "%s\n" "  else {"
		printf "%s\n" ""
		printf "%s\n" "    /* do an ldx${alternate}-style load: */"
		printf "%s\n" "    tme_sparc${arch}_ldx${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		printf "%s\n" ""
		printf "%s\n" "    /* set the double floating-point register value: */"
		printf "%s\n" "    assert (fpreg != &float_buffer);"
		printf "%s\n" "    fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		printf "%s\n" "    fpreg->tme_float_value_ieee754_double.tme_value64_uint"
		printf "%s\n" "      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX);"
		printf "%s\n" "  }"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# the stdf instruction:
	#
	if test ${insn} = "stdf${alternate}"; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_uint${arch}_t address;"
	    printf "%s\n" "  tme_uint32_t misaligned;"
	    printf "%s\n" "  struct tme_float float_buffer;"
	    printf "%s\n" "  struct tme_float *fpreg;"
	    printf "%s\n" "  const union tme_value64 *value_double;"
	    if test ${arch} != 32; then
		printf "%s\n" "  tme_uint${arch}_t offset;"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the address: */"
	    printf "%s\n" "  address = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the least significant 32 bits of the address: */"
	    printf "%s\n" "  misaligned = address;"
	    if test "${arch}${alternate}" = 64a; then
		printf "%s\n" ""
		printf "%s\n" "  /* see if the address is misaligned for the ASI: */"
		printf "%s\n" "  misaligned = (*ic->_tme_sparc_ls_asi_misaligned)(ic, misaligned);"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  /* decode rd: */"
	    printf "%s\n" "  float_buffer.tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
	    printf "%s\n" "  fpreg"
	    printf "%s\n" "    = _tme_sparc${arch}_fpu_mem_fpreg(ic,"
	    printf "%s\n" "                                 misaligned,"
	    printf "%s\n" "                                 &float_buffer);"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get this double floating-point register in IEEE754 double-precision format: */"
	    printf "%s\n" "  value_double = tme_ieee754_double_value_get(fpreg, &float_buffer.tme_float_value_ieee754_double);"
	    printf "%s\n" ""
	    if test ${arch} = 32; then
		printf "%s\n" "  /* set the floating-point register value: */"
		printf "%s\n" "  ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 0)"
		printf "%s\n" "    = value_double->tme_value64_uint32_hi;"
		printf "%s\n" "  ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1)"
		printf "%s\n" "    = value_double->tme_value64_uint32_lo;"
		printf "%s\n" ""
		printf "%s\n" "  /* do the store: */"
		printf "%s\n" "  tme_sparc${arch}_std(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}));"
	    else
		printf "%s\n" "  /* if bit two of the address is set, and this SPARC supports"
		printf "%s\n" "     32-bit-aligned ${insn} instructions: */"
		printf "%s\n" "  if ((misaligned & sizeof(tme_uint32_t))"
		printf "%s\n" "      && fpreg != &float_buffer) {"
		printf "%s\n" ""
		printf "%s\n" "    /* set the floating-point register value: */"
		printf "%s\n" "    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX + 0)"
		printf "%s\n" "      = value_double->tme_value64_uint32_hi;"
		printf "%s\n" "    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX + 1)"
		printf "%s\n" "      = value_double->tme_value64_uint32_lo;"
		printf "%s\n" ""
		printf "%s\n" "    /* do two 32-bit stores: */"
		printf "%s\n" "    offset = sizeof(tme_uint32_t) * 0;"
		printf "%s\n" "    tme_sparc${arch}_st${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 0));"
		printf "%s\n" "    offset = sizeof(tme_uint32_t) * 1;"
		printf "%s\n" "    tme_sparc${arch}_st${alternate}(ic, &address, &offset, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX + 1));"
		printf "%s\n" "  }"
		printf "%s\n" ""
		printf "%s\n" "  /* otherwise, bit two of the address is not set, or this SPARC"
		printf "%s\n" "     doesn't support 32-bit-aligned ${insn} instructions: */"
		printf "%s\n" "  else {"
		printf "%s\n" ""
		printf "%s\n" "    /* set the floating-point register value: */"
		printf "%s\n" "    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX)"
		printf "%s\n" "      = value_double->tme_value64_uint;"
		printf "%s\n" ""
		printf "%s\n" "    /* do an stx${alternate}-style store: */"
		printf "%s\n" "    tme_sparc${arch}_stx${alternate}(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		printf "%s\n" "  }"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  assert (fpreg != &float_buffer);"
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# the ldfsr instruction:
	#
	if test ${insn} = ldfsr; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  tme_uint32_t fsr;"
	    if test ${arch} != 32; then
		printf "%s\n" "  tme_uint32_t reg_rd;"
		printf "%s\n" ""
		printf "%s\n" "  /* see if this is an ldfsr or an ldxfsr: */"
		printf "%s\n" "  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);"
		printf "%s\n" "  if (__tme_predict_false(reg_rd > 1)) {"
		printf "%s\n" "    TME_SPARC_INSN_ILL(ic);"
		printf "%s\n" "  }"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  _tme_sparc${arch}_fpu_mem(ic);"
	    printf "%s\n" ""
	    if test ${arch} = 32; then
		printf "%s\n" "  /* do the load: */"
	    else
		printf "%s\n" "  /* if this is an ldxfsr: */"
		printf "%s\n" "  if (reg_rd == 1) {"
		printf "%s\n" ""
		printf "%s\n" "    /* do the load: */"
		printf "%s\n" "    tme_sparc${arch}_ldx(ic, _rs1, _rs2,  &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		printf "%s\n" ""
		printf "%s\n" "    /* update the extended FSR: */"
		printf "%s\n" "    ic->tme_sparc_fpu_xfsr"
		printf "%s\n" "      = (ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1)"
		printf "%s\n" "         & 0x3f /* fcc3 .. fcc1 */);"
		printf "%s\n" "  }"
		printf "%s\n" ""
		printf "%s\n" "  /* otherwise, this is an ldfsr.  do the load: */"
		printf "%s\n" "  else"
		printf %s "  "
	    fi
	    printf "%s\n" "  tme_sparc${arch}_ld(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    printf "%s\n" ""
	    printf "%s\n" "  /* update the FSR: */"
	    printf "%s\n" "  fsr = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift});"
	    printf "%s\n" "  /* \"An LDFSR instruction does not affect ftt.\" */"
	    printf "%s\n" "  /* \"The LDFSR instruction does not affect qne.\" */"
	    printf "%s\n" "  fsr &= ~(TME_SPARC_FSR_VER | TME_SPARC_FSR_FTT | TME_SPARC_FSR_QNE);"
	    printf "%s\n" "  ic->tme_sparc_fpu_fsr = (ic->tme_sparc_fpu_fsr & (TME_SPARC_FSR_VER | TME_SPARC_FSR_FTT | TME_SPARC_FSR_QNE)) | fsr;"
	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# the stfsr instruction:
	#
	if test ${insn} = stfsr; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    if test ${arch} = 64; then
		printf "%s\n" "  tme_uint32_t reg_rd;"
		printf "%s\n" ""
		printf "%s\n" "  /* see if this is an stfsr or an stxfsr: */"
		printf "%s\n" "  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);"
		printf "%s\n" "  if (__tme_predict_false(reg_rd > 1)) {"
		printf "%s\n" "    TME_SPARC_INSN_ILL(ic);"
		printf "%s\n" "  }"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "  _tme_sparc${arch}_fpu_mem(ic);"
	    printf "%s\n" ""
	    printf "%s\n" "  /* set the FSR value to store: */"
	    printf "%s\n" "  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX${reg32_shift}) = ic->tme_sparc_fpu_fsr;"
	    printf "%s\n" ""
	    if test ${arch} = 32; then
		printf "%s\n" "  /* do the store: */"
	    else
		printf "%s\n" "  /* if this is an stxfsr: */"
		printf "%s\n" "  if (reg_rd == 1) {"
		printf "%s\n" ""
		printf "%s\n" "    /* set in the extended FSR to store and do the store: */"
		printf "%s\n" "    ic->tme_sparc_ireg_uint32((TME_SPARC_IREG_FPX${reg32_shift}) + 1) = ic->tme_sparc_fpu_xfsr;"
		printf "%s\n" "    tme_sparc${arch}_stx(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
		printf "%s\n" "  }"
		printf "%s\n" ""
		printf "%s\n" "  /* otherwise, this is a stfsr.  do the store: */"
		printf "%s\n" "  else"
		printf %s "  "
	    fi
	    printf "%s\n" "  tme_sparc${arch}_st(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint${arch}(TME_SPARC_IREG_FPX));"
	    printf "%s\n" ""
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

	# the fpop1 and fpop2 instructions:
	#
	if test ${insn} = fpop1 || test ${insn} = fpop2; then

	    printf "%s\n" ""
	    printf "%s\n" "/* this does a sparc${arch} ${insn}: */"
	    printf "%s\n" "TME_SPARC_FORMAT3(tme_sparc${arch}_${insn}, tme_uint${arch}_t)"
	    printf "%s\n" "{"
	    printf "%s\n" "  TME_SPARC_INSN_FPU;"
	    printf "%s\n" "  tme_sparc_fpu_${insn}(ic);"
	    printf "%s\n" "  TME_SPARC_INSN_OK;"
	    printf "%s\n" "}"
	fi

    done

    # the slow atomic function:
    #
    if $header; then
	printf "%s\n" "void tme_sparc${arch}_atomic _TME_P((struct tme_sparc *, struct tme_sparc_ls *));"
    else
	printf "%s\n" ""
	printf "%s\n" "/* this does a slow atomic operation: */"
	printf "%s\n" "void"
	printf "%s\n" "tme_sparc${arch}_atomic(struct tme_sparc *ic, struct tme_sparc_ls *ls)"
	printf "%s\n" "{"
	printf "%s\n" "  tme_uint32_t endian_little;"
	printf "%s\n" "  tme_uint32_t insn;"
	if test ${arch} = 64; then
	    printf "%s\n" "  tme_uint${arch}_t value${arch};"
	    printf "%s\n" "  tme_uint${arch}_t value_swap${arch};"
	    printf "%s\n" "  unsigned int reg_rs2;"
	fi
	printf "%s\n" "  tme_uint32_t value32;"
	printf "%s\n" "  tme_uint32_t value_swap32;"
	printf "%s\n" "  tme_uint32_t size;"
	printf "%s\n" ""
	printf "%s\n" "  /* if this is the beginning of the operation: */"
	printf "%s\n" "  if (ls->tme_sparc_ls_state == 0) {"
	printf "%s\n" ""
	printf "%s\n" "    /* start the load part of the operation: */"
	printf "%s\n" "    ls->tme_sparc_ls_state = ls->tme_sparc_ls_size;"
	printf "%s\n" "    assert (ls->tme_sparc_ls_state != 0"
	printf "%s\n" "            && (ls->tme_sparc_ls_state & TME_BIT(7)) == 0);"
	printf "%s\n" ""
	printf "%s\n" "    /* the load must start at the beginning of the buffer: */"
	printf "%s\n" "    assert (ls->tme_sparc_ls_buffer_offset == 0);"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* if this is the load part of the operation: */"
	printf "%s\n" "  if ((ls->tme_sparc_ls_state & TME_BIT(7)) == 0) {"
	printf "%s\n" ""
	printf "%s\n" "    /* do one slow load cycle: */"
	printf "%s\n" "    tme_sparc${arch}_load(ic, ls);"
	printf "%s\n" ""
	printf "%s\n" "    /* if the slow load cycle did not load all of the data: */"
	printf "%s\n" "    if (__tme_predict_false(ls->tme_sparc_ls_size != 0)) {"
	printf "%s\n" "      return;"
	printf "%s\n" "    }"
	printf "%s\n" ""
	printf "%s\n" "    /* get the byte order of this transfer: */"
	if test ${arch} = 32; then
	    printf "%s\n" "    endian_little = FALSE;"
	else
	    printf "%s\n" "    endian_little = ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_ENDIAN_LITTLE;"
	fi
	printf "%s\n" ""
	printf "%s\n" "    /* dispatch on the op3 of the instruction: */"
	printf "%s\n" "    insn = TME_SPARC_INSN;"
	printf "%s\n" "    switch ((insn >> 19) & 0x3f) {"
	if test ${arch} = 64; then
	    for size in 32 64; do
		printf "%s\n" ""
		if test ${size} = 32; then
		    printf "%s\n" "    case 0x3c: /* casa */"
		else
		    printf "%s\n" "    case 0x3e: /* casxa */"
		fi
		printf "%s\n" ""
		printf "%s\n" "      /* finish the load part of the compare and swap: */"
		printf "%s\n" "      assert (ls->tme_sparc_ls_state == sizeof(tme_uint${size}_t));"
		printf "%s\n" "      value${size} = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer${size}s[0];"
		printf "%s\n" "      value_swap${size} = *ls->tme_sparc_ls_rd64;"
		printf "%s\n" "      if (endian_little) {"
		printf "%s\n" "        value${size} = tme_letoh_u${size}(value${size});"
		printf "%s\n" "        value_swap${size} = tme_htole_u${size}(value_swap${size});"
		printf "%s\n" "      }"
		printf "%s\n" "      else {"
		printf "%s\n" "        value${size} = tme_betoh_u${size}(value${size});"
		printf "%s\n" "        value_swap${size} = tme_htobe_u${size}(value_swap${size});"
		printf "%s\n" "      }"
		printf "%s\n" "      *ls->tme_sparc_ls_rd64 = value${size};"
		printf "%s\n" ""
		printf "%s\n" "      /* if the comparison fails: */"
		printf "%s\n" "      reg_rs2 = TME_FIELD_MASK_EXTRACTU(insn, TME_SPARC_FORMAT3_MASK_RS2);"
		printf "%s\n" "      TME_SPARC_REG_INDEX(ic, reg_rs2);"
		printf "%s\n" "      if (value${size} != (tme_uint${size}_t) ic->tme_sparc_ireg_uint${arch}(reg_rs2)) {"
		printf "%s\n" "        return;"
		printf "%s\n" "      }"
		printf "%s\n" ""
		printf "%s\n" "      /* start the store part of the compare and swap: */"
		printf "%s\n" "      ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer${size}s[0] = value_swap${size};"
		printf "%s\n" "      break;"
	    done
	fi
	printf "%s\n" ""
	printf "%s\n" "    case 0x0d: /* ldstub */"
	printf "%s\n" "    case 0x1d: /* ldstuba */"
	printf "%s\n" ""
	printf "%s\n" "      /* finish the load part of the ldstub: */"
	printf "%s\n" "      assert (ls->tme_sparc_ls_state == sizeof(tme_uint8_t));"
	printf "%s\n" "      *ls->tme_sparc_ls_rd${arch} = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s[0];"
	printf "%s\n" ""
	printf "%s\n" "      /* start the store part of the ldstub: */"
	printf "%s\n" "      ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s[0] = 0xff;"
	printf "%s\n" "      break;"
	printf "%s\n" ""
	printf "%s\n" "    /* otherwise, this must be swap: */"
	printf "%s\n" "    default:"
	printf "%s\n" "      assert (((insn >> 19) & 0x2f) == 0x0f /* swap, swapa */);"
	printf "%s\n" ""
	printf "%s\n" "      /* finish the load part of the swap: */"
	printf "%s\n" "      assert (ls->tme_sparc_ls_state == sizeof(tme_uint32_t));"
	printf "%s\n" "      value32 = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[0];"
	printf "%s\n" "      value_swap32 = *ls->tme_sparc_ls_rd${arch};"
	printf "%s\n" "      if (endian_little) {"
	printf "%s\n" "        value32 = tme_letoh_u32(value32);"
	printf "%s\n" "        value_swap32 = tme_htole_u32(value32);"
	printf "%s\n" "      }"
	printf "%s\n" "      else {"
	printf "%s\n" "        value32 = tme_betoh_u32(value32);"
	printf "%s\n" "        value_swap32 = tme_htobe_u32(value32);"
	printf "%s\n" "      }"
	printf "%s\n" "      *ls->tme_sparc_ls_rd${arch} = value32;"
	printf "%s\n" ""
	printf "%s\n" "      /* start the store part of the swap: */"
	printf "%s\n" "      ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[0] = value_swap32;"
	printf "%s\n" "      break;"
	printf "%s\n" "    }"
	printf "%s\n" ""
	printf "%s\n" "    /* start the store part of the operation: */"
	printf "%s\n" "    size = ls->tme_sparc_ls_state;"
	printf "%s\n" "    ls->tme_sparc_ls_address${arch} -= size;"
	printf "%s\n" "    ls->tme_sparc_ls_size = size;"
	printf "%s\n" "    ls->tme_sparc_ls_buffer_offset = 0;"
	printf "%s\n" "    ls->tme_sparc_ls_state = size | TME_BIT(7);"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* this is the store part of the operation: */"
	printf "%s\n" ""
	printf "%s\n" "  /* do one slow store cycle: */"
	printf "%s\n" "  tme_sparc${arch}_store(ic, ls);"
	printf "%s\n" ""
	printf "%s\n" "  /* if the slow store cycle did not store all of the data: */"
	printf "%s\n" "  if (__tme_predict_false(ls->tme_sparc_ls_size != 0)) {"
	printf "%s\n" "    return;"
	printf "%s\n" "  }"
	printf "%s\n" "}"
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
	    printf "%s\n" "void tme_sparc${arch}_${slow} _TME_P((struct tme_sparc *, struct tme_sparc_ls *));"
	    continue
	fi

	printf "%s\n" ""
	printf "%s\n" "/* this does one slow ${slow} cycle: */"
	printf "%s\n" "void"
	printf "%s\n" "tme_sparc${arch}_${slow}(struct tme_sparc *ic, "
	printf "%s\n" "                  struct tme_sparc_ls *ls)"
	printf "%s\n" "{"

	# our locals:
	#
	printf "%s\n" "  struct tme_sparc_tlb *tlb;"
	printf "%s\n" "  tme_uint${arch}_t address;"
	printf "%s\n" "  unsigned int cycle_size;"
	printf "%s\n" "  tme_bus_addr_t physical_address;"
	printf "%s\n" "  int shift;"
	printf "%s\n" "  int err;"

	printf "%s\n" ""
	printf "%s\n" "  /* get the TLB entry: */"
	printf "%s\n" "  tlb = ls->tme_sparc_ls_tlb;"
	printf "%s\n" ""
	printf "%s\n" "  /* the TLB entry must be busy and valid: */"
	printf "%s\n" "  assert (tme_bus_tlb_is_valid(&tlb->tme_sparc_tlb_bus_tlb));"
	printf "%s\n" ""
	printf "%s\n" "  /* start the bus cycle structure: */"
	printf "%s\n" "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_type = TME_BUS_CYCLE_${capcycle};"
	printf "%s\n" ""
	printf "%s\n" "  /* get the buffer: */"
	printf "%s\n" "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_buffer = &ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s[ls->tme_sparc_ls_buffer_offset];"
	printf "%s\n" "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_buffer_increment = 1;"
	printf "%s\n" ""
	printf "%s\n" "  /* get the current address: */"
	printf "%s\n" "  address = ls->tme_sparc_ls_address${arch};"
	printf "%s\n" "  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address = address;"
	printf "%s\n" ""
	printf "%s\n" "  /* start the cycle size: */"
	printf "%s\n" "  cycle_size = ls->tme_sparc_ls_size;"
	printf "%s\n" "  assert (cycle_size > 0);"
	printf "%s\n" "  cycle_size--;"
	printf "%s\n" "  cycle_size = TME_MIN(cycle_size, (((tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last) - address)) + 1;"
	printf "%s\n" ""
	printf "%s\n" "  /* if this TLB entry allows fast ${cycle}s: */"
	printf "%s\n" "  if (__tme_predict_true(tlb->tme_sparc_tlb_emulator_off_${cycle} != TME_EMULATOR_OFF_UNDEF)) {"
	printf "%s\n" ""
	printf "%s\n" "    /* do a ${cycle}: */"
	printf "%s\n" "    ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size = cycle_size;"
	printf "%s\n" "    tme_memory_bus_${cycle}_buffer((tlb->tme_sparc_tlb_emulator_off_${cycle} + (tme_uint${arch}_t) ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address),"
	printf "%s\n" "                                  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_buffer,"
	printf "%s\n" "                                  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size,"
	printf "%s\n" "                                  tlb->tme_sparc_tlb_bus_rwlock,"
	printf "%s\n" "                                  sizeof(tme_uint8_t),"
	printf "%s\n" "                                  sizeof(tme_uint${arch}_t));"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* otherwise, this TLB entry does not allow fast ${cycle}s: */"
	printf "%s\n" "  else {"
	printf "%s\n" ""
	printf "%s\n" "    /* finish the cycle size: */"
	printf "%s\n" "    cycle_size = TME_MIN(cycle_size, 1 + ((~ (unsigned int) address) % sizeof(tme_uint${arch}_t)));"
	printf "%s\n" "    ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size = cycle_size;"
	printf "%s\n" ""
	printf "%s\n" "    /* form the physical address for the bus cycle handler: */"
	printf "%s\n" "    physical_address = ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address;"
	printf "%s\n" "    physical_address += tlb->tme_sparc_tlb_addr_offset;"
	printf "%s\n" "    shift = tlb->tme_sparc_tlb_addr_shift;"
	printf "%s\n" "    if (shift < 0) {"
	printf "%s\n" "      physical_address <<= (0 - shift);"
	printf "%s\n" "    }"
	printf "%s\n" "    else if (shift > 0) {"
	printf "%s\n" "      physical_address >>= shift;"
	printf "%s\n" "    }"
	printf "%s\n" "    ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address = physical_address;"
	printf "%s\n" ""
	printf "%s\n" "    /* finish the bus cycle structure: */"
	printf "%s\n" "    (*ic->_tme_sparc_ls_bus_cycle)(ic, ls);"

	printf "%s\n" "    tme_sparc_log(ic, 10000, TME_OK,"
	printf "%s\n" "                 (TME_SPARC_LOG_HANDLE(ic),"
	printf "%s\n" "                  _(\"cycle-${slow}%u\t0x%0"`expr ${arch} / 4`"\" TME_PRIx${arch}),"
	printf "%s\n" "                  (unsigned int) (ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size * 8),"
	printf "%s\n" "                  (tme_bus_addr${arch}_t) ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address));"

	printf "%s\n" ""
	printf "%s\n" "    /* callout the bus cycle: */"
	printf "%s\n" "    tme_sparc_tlb_unbusy(tlb);"
	printf "%s\n" "    tme_sparc_callout_unlock(ic);"
	printf "%s\n" "    err = (*tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_cycle)"
	printf "%s\n" "           (tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_cycle_private,"
	printf "%s\n" "            &ls->tme_sparc_ls_bus_cycle);"
	printf "%s\n" "    tme_sparc_callout_relock(ic);"
	printf "%s\n" "    tme_sparc_tlb_busy(tlb);"
	printf "%s\n" ""
	printf "%s\n" "    /* the TLB entry can't have been invalidated before the ${slow}: */"
	printf "%s\n" "    assert (err != EBADF);"
	printf "%s\n" ""
	printf "%s\n" "    /* if the bus cycle didn't complete normally: */"
	printf "%s\n" "    if (err != TME_OK) {"
	printf "%s\n" ""
	printf "%s\n" "      /* if a real bus fault may have happened, instead of"
	printf "%s\n" "         some synchronous event: */"
	printf "%s\n" "      if (err != TME_BUS_CYCLE_SYNCHRONOUS_EVENT) {"
	printf "%s\n" ""
	printf "%s\n" "        /* call the bus fault handlers: */"
	printf "%s\n" "        err = tme_bus_tlb_fault(&tlb->tme_sparc_tlb_bus_tlb, &ls->tme_sparc_ls_bus_cycle, err);"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* if some synchronous event has happened: */"
	printf "%s\n" "      if (err == TME_BUS_CYCLE_SYNCHRONOUS_EVENT) {"
	printf "%s\n" ""
	printf "%s\n" "        /* after the currently executing instruction finishes, check"
	printf "%s\n" "           for external resets, halts, or interrupts: */"
	printf "%s\n" "        ic->_tme_sparc_instruction_burst_remaining = 0;"
	printf "%s\n" "        ic->_tme_sparc_instruction_burst_other = TRUE;"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* otherwise, if no real bus fault happened: */"
	printf "%s\n" "      else if (err == TME_OK) {"
	printf "%s\n" ""
	printf "%s\n" "        /* nothing to do */"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* otherwise, a real bus fault happened: */"
	printf "%s\n" "      else {"
	printf "%s\n" "        (*ic->_tme_sparc_ls_bus_fault)(ic, ls, err);"
	printf "%s\n" "        return;"
	printf "%s\n" "      }"
	printf "%s\n" "    }"
	printf "%s\n" "  }"

	printf "%s\n" ""
	printf "%s\n" "  /* some data must have been transferred: */"
	printf "%s\n" "  assert (ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size > 0);"

	if test ${slow} = store; then
	    printf "%s\n" ""
	    printf "%s\n" "  /* if this was an atomic operation: */"
	    printf "%s\n" "  if (__tme_predict_false(ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ATOMIC)) {"
	    printf "%s\n" ""
	    printf "%s\n" "    /* we do not support atomic operations in TLB entries that"
	    printf "%s\n" "       do not support both fast reads and fast writes.  assuming"
	    printf "%s\n" "       that all atomic operations are to regular memory, we"
	    printf "%s\n" "       should always get fast read and fast write TLBs.  when"
	    printf "%s\n" "       we do not, it should only be because the memory has been"
	    printf "%s\n" "       made read-only in the MMU.  the write above was supposed"
	    printf "%s\n" "       to cause a fault (with the instruction rerun later with"
	    printf "%s\n" "       a fast read and fast write TLB entry), but instead it"
	    printf "%s\n" "       succeeded and transferred some data.  we have modified"
	    printf "%s\n" "       memory and cannot recover: */"
	    printf "%s\n" "    abort();"
	    printf "%s\n" "  }"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* update: */"
	printf "%s\n" "  cycle_size = ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size;"
	printf "%s\n" "  ls->tme_sparc_ls_address${arch} += cycle_size;"
	printf "%s\n" "  ls->tme_sparc_ls_buffer_offset += cycle_size;"
	printf "%s\n" "  ls->tme_sparc_ls_size -= cycle_size;"
	printf "%s\n" "}"
    done

    # the load/store function:
    #
    if $header; then
	printf "%s\n" "tme_shared tme_uint8_t *tme_sparc${arch}_ls _TME_P((struct tme_sparc *, tme_uint${arch}_t, tme_uint${arch}_t *, tme_uint32_t));"
    else

	printf "%s\n" ""
	printf "%s\n" "/* this does a slow load or store: */"
	printf "%s\n" "tme_shared tme_uint8_t *"
	printf "%s\n" "tme_sparc${arch}_ls(struct tme_sparc *ic,"
	printf "%s\n" "               tme_uint${arch}_t const address_first,"
	printf "%s\n" "               tme_uint${arch}_t *_rd,"
	printf "%s\n" "               tme_uint32_t lsinfo)"
	printf "%s\n" "{"
	printf "%s\n" "  struct tme_sparc_ls ls;"
	printf "%s\n" "  tme_uint32_t size;"
	printf "%s\n" "  tme_uint32_t asi;"
	printf "%s\n" "  tme_uint32_t asi_mask_flags;"
	printf "%s\n" "  tme_uint32_t asi_mask;"
	printf "%s\n" "  tme_bus_context_t context;"
	printf "%s\n" "  tme_uint32_t tlb_hash;"
	printf "%s\n" "  unsigned long tlb_i;"
	printf "%s\n" "  unsigned long handler_i;"
	printf "%s\n" "  struct tme_sparc_tlb *tlb;"
	printf "%s\n" "  unsigned int cycle_type;"
	printf "%s\n" "  tme_uint${arch}_t address;"
	printf "%s\n" "  void (*address_map) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));"
	printf "%s\n" "  tme_bus_addr_t address_bus;"
	printf "%s\n" "  int rc;"
	printf "%s\n" "  const tme_shared tme_uint8_t *emulator_off;"
	printf "%s\n" "  unsigned int buffer_offset;"
	printf "%s\n" "  tme_uint${arch}_t value;"
	printf "%s\n" "  tme_uint32_t value32;"
	printf "%s\n" ""
	printf "%s\n" "  /* we must not be replaying instructions: */"
	printf "%s\n" "  assert (tme_sparc_recode_verify_replay_last_pc(ic) == 0);"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the pointer to the rd register: */"
	printf "%s\n" "  ls.tme_sparc_ls_rd${arch} = _rd;"
	printf "%s\n" ""
	printf "%s\n" "#ifndef NDEBUG"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the cycle function: */"
	printf "%s\n" "  ls.tme_sparc_ls_cycle = NULL;"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the TLB entry pointer: */"
	printf "%s\n" "  ls.tme_sparc_ls_tlb = NULL;"
	printf "%s\n" ""
	printf "%s\n" "#endif /* NDEBUG */"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the faults: */"
	printf "%s\n" "  ls.tme_sparc_ls_faults = TME_SPARC_LS_FAULT_NONE;"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the address: */"
	printf "%s\n" "  ls.tme_sparc_ls_address${arch} = address_first;"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the size: */"
	printf "%s\n" "  size = TME_SPARC_LSINFO_WHICH_SIZE(lsinfo);"
	printf "%s\n" "  ls.tme_sparc_ls_size = size;"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the info: */"
	printf "%s\n" "  ls.tme_sparc_ls_lsinfo = lsinfo;"
	printf "%s\n" ""
	printf "%s\n" "  /* if the address is not aligned: */"
	printf "%s\n" "  if (__tme_predict_false(((size - 1) & (tme_uint32_t) address_first) != 0)) {"
	printf "%s\n" "    ls.tme_sparc_ls_faults |= TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* otherwise, the address is aligned: */"
	printf "%s\n" "  else {"
	printf "%s\n" ""
	printf "%s\n" "    /* the transfer must not cross a 32-bit boundary: */"
	printf "%s\n" "    assert ((size - 1) <= (tme_uint32_t) ~address_first);"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* initialize the address map: */"
	printf "%s\n" "  ls.tme_sparc_ls_address_map = ic->_tme_sparc_ls_address_map;"
	printf "%s\n" ""
	printf "%s\n" "  /* if this is a ldd, ldda, std, or stda, or an instruction"
	printf "%s\n" "     that loads or stores in the same way: */"
	printf "%s\n" "  if (lsinfo & TME_SPARC_LSINFO_LDD_STD) {"
	printf "%s\n" ""
	printf "%s\n" "    /* if the rd register is odd: */"
	printf "%s\n" "    /* NB: we don't check the rd field in the instruction,"
	printf "%s\n" "       because the register number there might be encoded"
	printf "%s\n" "       in some way, or the architecture might ignore bit"
	printf "%s\n" "       zero in the rd field (for example, the sparc32 lddf)."
	printf "%s\n" "       instead, we test the rd register pointer: */"
	printf "%s\n" "    if (__tme_predict_false((ls.tme_sparc_ls_rd${arch}"
	printf "%s\n" "                             - ic->tme_sparc_ic.tme_ic_iregs.tme_ic_iregs_uint${arch}s)"
	printf "%s\n" "                            % 2)) {"
	printf "%s\n" "      ls.tme_sparc_ls_faults |= TME_SPARC_LS_FAULT_LDD_STD_RD_ODD;"
	printf "%s\n" "    }"
	printf "%s\n" "  }"

	printf "%s\n" ""
	printf "%s\n" "  /* if the ASI has been specified: */"
	printf "%s\n" "  if (lsinfo & TME_SPARC_LSINFO_A) {"
	printf "%s\n" ""
	printf "%s\n" "    /* get the ASI: */"
	printf "%s\n" "    asi = TME_SPARC_LSINFO_WHICH_ASI(lsinfo);"
	printf "%s\n" ""
	printf "%s\n" "    /* get the flags for this ASI: */"
	printf "%s\n" "    asi_mask_flags = ic->tme_sparc_asis[asi].tme_sparc_asi_mask_flags;"
	printf "%s\n" ""
	if test ${arch} = 32; then
	    printf "%s\n" "    /* make the ASI mask: */"
	    printf "%s\n" "    if (asi_mask_flags & TME_SPARC32_ASI_MASK_FLAG_SPECIAL) {"
	    printf "%s\n" "      asi_mask"
	    printf "%s\n" "        = TME_SPARC_ASI_MASK_SPECIAL(asi, TRUE);"
	    printf "%s\n" "    }"
	    printf "%s\n" "    else {"
	    printf "%s\n" "      asi_mask = TME_SPARC32_ASI_MASK(asi, asi);"
	    printf "%s\n" "    }"
	elif test ${arch} = 64; then
	    printf "%s\n" "    /* if this is a nonprivileged access: */"
	    printf "%s\n" "    if (!TME_SPARC_PRIV(ic)) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* if this is a restricted ASI: */"
	    printf "%s\n" "      if (__tme_predict_false((asi & TME_SPARC64_ASI_FLAG_UNRESTRICTED) == 0)) {"
	    printf "%s\n" "        ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_PRIVILEGED_ASI;"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* force a nonprivileged access with the ASI: */"
	    printf "%s\n" "      asi_mask_flags |= TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER;"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* make the ASI mask: */"
	    printf "%s\n" "    if (asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_SPECIAL) {"
	    printf "%s\n" "      asi_mask"
	    printf "%s\n" "        = (asi_mask_flags"
	    printf "%s\n" "           + TME_SPARC_ASI_MASK_SPECIAL(asi,"
	    printf "%s\n" "                                        (asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER) == 0));"
	    printf "%s\n" "    }"
	    printf "%s\n" "    else {"
	    printf "%s\n" "      asi_mask = TME_SPARC64_ASI_MASK(asi, asi_mask_flags);"
	    printf "%s\n" "    }"
	fi
	printf "%s\n" "    ls.tme_sparc_ls_asi_mask = asi_mask;"
	if test ${arch} = 64; then
	    printf "%s\n" ""
	    printf "%s\n" "    /* if this is a no-fault ASI with a non-load instruction: */"
	    printf "%s\n" "    if (asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT) {"
	    printf "%s\n" "      if (__tme_predict_false(lsinfo & (TME_SPARC_LSINFO_OP_ST | TME_SPARC_LSINFO_OP_ATOMIC))) {"
	    printf "%s\n" "        ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_NO_FAULT_NON_LOAD;"
	    printf "%s\n" "      }"
	    printf "%s\n" "    }"
	fi
	printf "%s\n" ""
	printf "%s\n" "    /* get the context for the alternate address space: */"
	if test ${arch} = 32; then
	    printf "%s\n" "    context = ic->tme_sparc_memory_context_default;"
	elif test ${arch} = 64; then
	    printf "%s\n" "    context = ic->tme_sparc_memory_context_primary;"
	    printf "%s\n" "    if (asi_mask & TME_SPARC64_ASI_FLAG_SECONDARY) {"
	    printf "%s\n" "      context = ic->tme_sparc_memory_context_secondary;"
	    printf "%s\n" "    }"
	    printf "%s\n" "    if (__tme_predict_false(asi_mask & TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS)) {"
	    printf "%s\n" "      if (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS) {"
	    printf "%s\n" "        context = 0;"
	    printf "%s\n" "      }"
	    printf "%s\n" "    }"
	fi
	printf "%s\n" "    ls.tme_sparc_ls_context = context;"
	printf "%s\n" ""
	printf "%s\n" "    /* get the default TLB entry index: */"
	printf "%s\n" "    tlb_hash = TME_SPARC_TLB_HASH(ic, context, address_first);"
	printf "%s\n" "    if (lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	printf "%s\n" "      tlb_i = TME_SPARC_ITLB_ENTRY(ic, tlb_hash);"
	printf "%s\n" "    }"
	printf "%s\n" "    else {"
	printf "%s\n" "      tlb_i = TME_SPARC_DTLB_ENTRY(ic, tlb_hash);"
	printf "%s\n" "    }"
	printf "%s\n" "    ls.tme_sparc_ls_tlb_i = tlb_i;"
	printf "%s\n" ""
	printf "%s\n" "    /* call any special handler for this ASI: */"
	printf "%s\n" "    handler_i = ic->tme_sparc_asis[TME_SPARC_ASI_MASK_WHICH(asi_mask)].tme_sparc_asi_handler;"
	printf "%s\n" "    if (__tme_predict_false(handler_i != 0)) {"
	printf "%s\n" "      (*ic->_tme_sparc_ls_asi_handlers[handler_i])(ic, &ls);"
	printf "%s\n" "    }"
	printf "%s\n" ""
	printf "%s\n" "    /* get the final TLB entry index: */"
	printf "%s\n" "    tlb_i = ls.tme_sparc_ls_tlb_i;"
	printf "%s\n" "  }"

	printf "%s\n" ""
	printf "%s\n" "  /* otherwise, the ASI has not been specified: */"
	printf "%s\n" "  else {"
	printf "%s\n" ""
	printf "%s\n" "    /* get the ASI mask: */"
	printf "%s\n" "    asi_mask = ic->tme_sparc_asi_mask_data;"
	printf "%s\n" ""
	printf "%s\n" "    /* add in any ASI mask flags from the instruction: */"
	if test ${arch} = 64; then
	    printf "%s\n" "    /* NB: initially, TME_SPARC64_ASI_FLAG_NO_FAULT is the"
	    printf "%s\n" "       only flag allowed, and only the flush instruction"
	    printf "%s\n" "       can use it: */"
	fi
	printf "%s\n" "    assert (TME_SPARC_LSINFO_WHICH_ASI_FLAGS(lsinfo) == 0"
	if test ${arch} = 64; then
	    printf "%s\n" "            || (TME_SPARC_LSINFO_WHICH_ASI_FLAGS(lsinfo) == TME_SPARC64_ASI_FLAG_NO_FAULT"
	    printf "%s\n" "                && ((ic->_tme_sparc_insn >> 19) & 0x3f) == 0x3b)"
	fi
	printf "%s\n" "            );"
	printf "%s\n" "    asi_mask |= TME_SPARC_LSINFO_WHICH_ASI_FLAGS(lsinfo);"
	printf "%s\n" ""
	printf "%s\n" "    /* set the ASI mask: */"
	printf "%s\n" "    ls.tme_sparc_ls_asi_mask = asi_mask;"
	printf "%s\n" ""
	printf "%s\n" "    /* get the context: */"
	printf "%s\n" "    context = ic->tme_sparc_memory_context_default;"
	printf "%s\n" "    ls.tme_sparc_ls_context = context;"
	printf "%s\n" ""
	printf "%s\n" "    /* this must not be a fetch: */"
	printf "%s\n" "    assert ((lsinfo & TME_SPARC_LSINFO_OP_FETCH) == 0);"
	printf "%s\n" ""
	printf "%s\n" "    /* get the TLB entry index: */"
	printf "%s\n" "    tlb_hash = TME_SPARC_TLB_HASH(ic, context, address_first);"
	printf "%s\n" "    tlb_i = TME_SPARC_DTLB_ENTRY(ic, tlb_hash);"
	printf "%s\n" "    ls.tme_sparc_ls_tlb_i = tlb_i;"
	printf "%s\n" "  }"

	printf "%s\n" ""
	printf "%s\n" "  /* get the TLB entry pointer: */"
	printf "%s\n" "  tlb = &ic->tme_sparc_tlbs[tlb_i];"
	printf "%s\n" "  ls.tme_sparc_ls_tlb = tlb;"

	printf "%s\n" ""
	printf "%s\n" "  /* get the cycle type: */"
	printf "%s\n" "  /* NB: we deliberately set this once, now, since the lsinfo"
	printf "%s\n" "     may change once we start transferring: */"
	printf "%s\n" "  cycle_type"
	printf "%s\n" "    = ((lsinfo"
	printf "%s\n" "        & (TME_SPARC_LSINFO_OP_ST"
	printf "%s\n" "           | TME_SPARC_LSINFO_OP_ATOMIC))"
	printf "%s\n" "       ? TME_BUS_CYCLE_WRITE"
	printf "%s\n" "       : TME_BUS_CYCLE_READ);"

	printf "%s\n" ""
	printf "%s\n" "  /* loop until the transfer is complete: */"
	printf "%s\n" "  for (;;) {"

	printf "%s\n" ""
	printf "%s\n" "    /* if we have faulted: */"
	printf "%s\n" "    if (__tme_predict_false(ls.tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {"
	printf "%s\n" ""
	printf "%s\n" "      /* unbusy this TLB, since the trap function may not return: */"
	printf "%s\n" "      tme_bus_tlb_unbusy(&tlb->tme_sparc_tlb_bus_tlb);"
	printf "%s\n" ""
	printf "%s\n" "      /* call the trap function, which will not return if it traps: */"
	printf "%s\n" "      (*ic->_tme_sparc_ls_trap)(ic, &ls);"
	printf "%s\n" ""
	printf "%s\n" "      /* rebusy this TLB: */"
	printf "%s\n" "      tme_bus_tlb_busy(&tlb->tme_sparc_tlb_bus_tlb);"
	printf "%s\n" ""
	printf "%s\n" "      /* since the trap function returned, it must have cleared the fault: */"
	printf "%s\n" "      assert (ls.tme_sparc_ls_faults == TME_SPARC_LS_FAULT_NONE);"
	printf "%s\n" "    }"

	printf "%s\n" ""
	printf "%s\n" "    /* if the transfer is complete, stop now: */"
	printf "%s\n" "    if (__tme_predict_false(ls.tme_sparc_ls_size == 0)) {"
	printf "%s\n" "      break;"
	printf "%s\n" "    }"

	printf "%s\n" ""
	printf "%s\n" "    /* get the current address: */"
	printf "%s\n" "    address = ls.tme_sparc_ls_address${arch};"

	printf "%s\n" ""
	printf "%s\n" "    /* if this TLB entry does not apply or is invalid: */"
	printf "%s\n" "    if ((tlb->tme_sparc_tlb_context != ls.tme_sparc_ls_context"
	printf "%s\n" "         && tlb->tme_sparc_tlb_context <= ic->tme_sparc_memory_context_max)"
	printf "%s\n" "        || address < (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_first"
	printf "%s\n" "        || address > (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last"
	printf "%s\n" "        || !TME_SPARC_TLB_ASI_MASK_OK(tlb, ls.tme_sparc_ls_asi_mask)"
	printf "%s\n" "        || ((tlb->tme_sparc_tlb_cycles_ok & cycle_type) == 0"
	printf "%s\n" "            && (cycle_type == TME_BUS_CYCLE_READ"
	printf "%s\n" "                ? tlb->tme_sparc_tlb_emulator_off_read"
	printf "%s\n" "                : tlb->tme_sparc_tlb_emulator_off_write) == TME_EMULATOR_OFF_UNDEF)"
	printf "%s\n" "        || tme_bus_tlb_is_invalid(&tlb->tme_sparc_tlb_bus_tlb)) {"
	printf "%s\n" ""
	printf "%s\n" "      /* unbusy this TLB entry for filling: */"
	printf "%s\n" "      tme_bus_tlb_unbusy_fill(&tlb->tme_sparc_tlb_bus_tlb);"
	printf "%s\n" ""
	printf "%s\n" "      /* if we haven't mapped this address yet: */"
	printf "%s\n" "      address_map = ls.tme_sparc_ls_address_map;"
	printf "%s\n" "      if (address_map != NULL) {"
	printf "%s\n" "        ls.tme_sparc_ls_address_map = NULL;"
	printf "%s\n" ""
	printf "%s\n" "        /* count this mapping: */"
	printf "%s\n" "        if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	printf "%s\n" "          TME_SPARC_STAT(ic, tme_sparc_stats_itlb_map);"
	printf "%s\n" "        }"
	printf "%s\n" "        else {"
	printf "%s\n" "          TME_SPARC_STAT(ic, tme_sparc_stats_dtlb_map);"
	printf "%s\n" "        }"
	printf "%s\n" ""
	printf "%s\n" "        /* initialize the ASI mask and context on this TLB entry: */"
	printf "%s\n" "        /* NB that the ASI mask will likely be updated by either the"
	printf "%s\n" "           address mapping or the TLB fill: */"
	printf "%s\n" "        tlb->tme_sparc_tlb_asi_mask"
	printf "%s\n" "          = (ls.tme_sparc_ls_asi_mask"
	printf "%s\n" "             & ~TME_SPARC_ASI_MASK_FLAGS_AVAIL);"
	printf "%s\n" "        tlb->tme_sparc_tlb_context = ls.tme_sparc_ls_context;"
	printf "%s\n" ""
	printf "%s\n" "        /* NB: if the address mapping traps, we won't get a chance"
	printf "%s\n" "           to finish updating this TLB entry, which is currently in"
	printf "%s\n" "           an inconsistent state - but not necessarily an unusable"
	printf "%s\n" "           state.  poison it to be unusable, including any recode"
	printf "%s\n" "           TLB entry: */"
	printf "%s\n" "        tlb->tme_sparc_tlb_addr_first = 1;"
	printf "%s\n" "        tlb->tme_sparc_tlb_addr_last = 0;"
	printf "%s\n" "#if TME_SPARC_HAVE_RECODE(ic)"
	printf "%s\n" "        if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	printf "%s\n" "          tme_sparc${arch}_recode_chain_tlb_update(ic, &ls);"
	printf "%s\n" "        }"
	printf "%s\n" "        else {"
	printf "%s\n" "          tme_sparc${arch}_recode_ls_tlb_update(ic, &ls);"
	printf "%s\n" "        }"
	printf "%s\n" "#endif /* TME_SPARC_HAVE_RECODE(ic) */"
	printf "%s\n" ""
	printf "%s\n" "#ifndef NDEBUG"
	printf "%s\n" ""
	printf "%s\n" "        /* initialize the mapping TLB entry: */"
	printf "%s\n" "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first = 0 - (tme_bus_addr_t) 1;"
	printf "%s\n" "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last = 0 - (tme_bus_addr_t) 2;"
	printf "%s\n" "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok = 0;"
	printf "%s\n" "        ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset = 0 - (tme_bus_addr_t) 1;"
	printf "%s\n" ""
	printf "%s\n" "#endif /* !NDEBUG */"
	printf "%s\n" ""
	printf "%s\n" "        /* map the address: */"
	printf "%s\n" "        (*address_map)(ic, &ls);"
	printf "%s\n" ""
	printf "%s\n" "        /* the address mapping must do any trapping itself: */"
	printf "%s\n" "        assert (ls.tme_sparc_ls_faults == TME_SPARC_LS_FAULT_NONE);"
	printf "%s\n" ""
	printf "%s\n" "        /* if the address mapping completed the transfer: */"
	printf "%s\n" "        if (__tme_predict_false(ls.tme_sparc_ls_size == 0)) {"
	printf "%s\n" ""
	printf "%s\n" "          /* rebusy the TLB entry: */"
	printf "%s\n" "          tme_sparc_tlb_busy(tlb);"
	printf "%s\n" ""
	printf "%s\n" "          /* stop now: */"
	printf "%s\n" "          break;"
	printf "%s\n" "        }"
	printf "%s\n" ""
	printf "%s\n" "        /* the mapping must have actually made a mapping: */"
	printf "%s\n" "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first != 0 - (tme_bus_addr_t) 1);"
	printf "%s\n" "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last != 0 - (tme_bus_addr_t) 2);"
	printf "%s\n" "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok != 0);"
	printf "%s\n" "        assert (ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset != 0 - (tme_bus_addr_t) 1);"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* count this fill: */"
	printf "%s\n" "      if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	printf "%s\n" "        TME_SPARC_STAT(ic, tme_sparc_stats_itlb_fill);"
	printf "%s\n" "      }"
	printf "%s\n" "      else {"
	printf "%s\n" "        TME_SPARC_STAT(ic, tme_sparc_stats_dtlb_fill);"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* get the bus address: */"
	printf "%s\n" "      address_bus = ls.tme_sparc_ls_address${arch} + ls.tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset;"
	printf "%s\n" ""
	printf "%s\n" "      /* fill the TLB entry: */"
	printf "%s\n" "      tme_sparc_callout_unlock(ic);"
	printf "%s\n" "      rc = (*ic->_tme_sparc_bus_connection->tme_sparc_bus_tlb_fill)"
	printf "%s\n" "        (ic->_tme_sparc_bus_connection,"
	printf "%s\n" "         tlb,"
	printf "%s\n" "         ls.tme_sparc_ls_asi_mask,"
	printf "%s\n" "         address_bus,"
	printf "%s\n" "         cycle_type);"
	printf "%s\n" "      assert (rc == TME_OK);"
	printf "%s\n" "      tme_sparc_callout_relock(ic);"
	printf "%s\n" ""
	printf "%s\n" "      /* map the TLB entry: */"
	printf "%s\n" "      tme_bus_tlb_map(&tlb->tme_sparc_tlb_bus_tlb, address_bus,"
	printf "%s\n" "                      &ls.tme_sparc_ls_tlb_map, ls.tme_sparc_ls_address${arch});"
	printf "%s\n" ""
	printf "%s\n" "      /* update any recode TLB entry: */"
	printf "%s\n" "#if TME_SPARC_HAVE_RECODE(ic)"
	printf "%s\n" "      if (ls.tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {"
	printf "%s\n" "        tme_sparc${arch}_recode_chain_tlb_update(ic, &ls);"
	printf "%s\n" "      }"
	printf "%s\n" "      else {"
	printf "%s\n" "        tme_sparc${arch}_recode_ls_tlb_update(ic, &ls);"
	printf "%s\n" "      }"
	printf "%s\n" "#endif /* TME_SPARC_HAVE_RECODE(ic) */"
	printf "%s\n" ""
	printf "%s\n" "      /* rebusy the TLB entry: */"
	printf "%s\n" "      tme_sparc_tlb_busy(tlb);"
	printf "%s\n" ""
	printf "%s\n" "      /* if this TLB entry is already invalid: */"
	printf "%s\n" "      if (tme_bus_tlb_is_invalid(&tlb->tme_sparc_tlb_bus_tlb)) {"
	printf "%s\n" "        continue;"
	printf "%s\n" "      }"
	printf "%s\n" "    }"

	printf "%s\n" ""
	printf "%s\n" "    /* this TLB entry must apply: */"
	printf "%s\n" "    assert ((tlb->tme_sparc_tlb_context == ls.tme_sparc_ls_context"
	printf "%s\n" "             || tlb->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max)"
	printf "%s\n" "            && ls.tme_sparc_ls_address${arch} >= (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_first"
	printf "%s\n" "            && ls.tme_sparc_ls_address${arch} <= (tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last"
	printf "%s\n" "            && ((tlb->tme_sparc_tlb_cycles_ok & cycle_type)"
	printf "%s\n" "                || (cycle_type == TME_BUS_CYCLE_READ"
	printf "%s\n" "                    ? tlb->tme_sparc_tlb_emulator_off_read"
	printf "%s\n" "                    : tlb->tme_sparc_tlb_emulator_off_write) != TME_EMULATOR_OFF_UNDEF)"
	printf "%s\n" "            && TME_SPARC_TLB_ASI_MASK_OK(tlb, ls.tme_sparc_ls_asi_mask));"

	printf "%s\n" ""
	printf "%s\n" "    /* get the current lsinfo: */"
	printf "%s\n" "    lsinfo = ls.tme_sparc_ls_lsinfo;"

	printf "%s\n" ""
	printf "%s\n" "    /* if we have to check the TLB: */"
	printf "%s\n" "    if (__tme_predict_true((lsinfo & TME_SPARC_LSINFO_NO_CHECK_TLB) == 0)) {"
	printf "%s\n" ""
	printf "%s\n" "      /* get the ASI mask for this TLB entry: */"
	printf "%s\n" "      asi_mask = tlb->tme_sparc_tlb_asi_mask;"
	if test ${arch} = 64; then
	    printf "%s\n" ""
	    printf "%s\n" "      /* if this TLB entry is for no-fault accesses only: */"
	    printf "%s\n" "      if (__tme_predict_false(asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT)) {"
	    printf "%s\n" ""
	    printf "%s\n" "        /* if this access is not using a no-fault ASI: */"
	    printf "%s\n" "        if (__tme_predict_false((ls.tme_sparc_ls_asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT) == 0)) {"
	    printf "%s\n" "          ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_NO_FAULT_FAULT;"
	    printf "%s\n" "          continue;"
	    printf "%s\n" "        }"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* if this TLB entry is for addresses with side effects: */"
	    printf "%s\n" "      if (asi_mask & TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS) {"
	    printf "%s\n" ""
	    printf "%s\n" "        /* if this access is using a no-fault ASI: */"
	    printf "%s\n" "        /* NB: a flush may be implemented as a load with a no-fault ASI: */"
	    printf "%s\n" "        if (__tme_predict_false(ls.tme_sparc_ls_asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT)) {"
	    printf "%s\n" "          ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_SIDE_EFFECTS;"
	    printf "%s\n" "          continue;"
	    printf "%s\n" "        }"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* if this TLB entry is for uncacheable addresses: */"
	    printf "%s\n" "      if (asi_mask & TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE) {"
	    printf "%s\n" ""
	    printf "%s\n" "        /* if this is an atomic access: */"
	    printf "%s\n" "        if (__tme_predict_false(lsinfo & TME_SPARC_LSINFO_OP_ATOMIC)) {"
	    printf "%s\n" "          ls.tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_UNCACHEABLE;"
	    printf "%s\n" "          continue;"
	    printf "%s\n" "        }"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* see if this is a little-endian instruction: */"
	    printf "%s\n" "      lsinfo"
	    printf "%s\n" "        = ((lsinfo"
	    printf "%s\n" "            & ~TME_SPARC_LSINFO_ENDIAN_LITTLE)"
	    printf "%s\n" "           + ((ls.tme_sparc_ls_asi_mask"
	    printf "%s\n" "               & TME_SPARC64_ASI_FLAG_LITTLE)"
	    printf "%s\n" "#if TME_SPARC_LSINFO_ENDIAN_LITTLE < TME_SPARC64_ASI_FLAG_LITTLE"
	    printf "%s\n" "#error \"TME_SPARC_LSINFO_ENDIAN_ values changed\""
	    printf "%s\n" "#endif"
	    printf "%s\n" "              * (TME_SPARC_LSINFO_ENDIAN_LITTLE"
	    printf "%s\n" "                 / TME_SPARC64_ASI_FLAG_LITTLE)));"
	    printf "%s\n" ""
	    printf "%s\n" "      /* if this TLB entry has its little-endian bit set: */"
	    printf "%s\n" "      if (__tme_predict_false(asi_mask & TME_SPARC64_ASI_FLAG_LITTLE)) {"
	    printf "%s\n" "        assert (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_INVERT_ENDIAN);"
	    printf "%s\n" "        if (TRUE) {"
	    printf "%s\n" "          lsinfo ^= TME_SPARC_LSINFO_ENDIAN_LITTLE;"
	    printf "%s\n" "        }"
	    printf "%s\n" "      }"
	fi
	printf "%s\n" "    }"

	printf "%s\n" ""
	printf "%s\n" "    /* if we might not have to call a slow cycle function: */"
	printf "%s\n" "    if (__tme_predict_true((lsinfo & TME_SPARC_LSINFO_SLOW_CYCLES) == 0)) {"
	printf "%s\n" ""
	printf "%s\n" "      /* if this TLB entry allows fast transfer of all of the addresses: */"
	printf "%s\n" "      if (__tme_predict_true(((tme_bus_addr${arch}_t) tlb->tme_sparc_tlb_addr_last) >= (address_first + (ls.tme_sparc_ls_size - 1)))) {"
	printf "%s\n" "        emulator_off = tlb->tme_sparc_tlb_emulator_off_read;"
	printf "%s\n" "        if (lsinfo & TME_SPARC_LSINFO_OP_ST) {"
	printf "%s\n" "          emulator_off = tlb->tme_sparc_tlb_emulator_off_write;"
	printf "%s\n" "        }"
	printf "%s\n" "        if (__tme_predict_true(emulator_off != TME_EMULATOR_OFF_UNDEF"
	printf "%s\n" "                               && (((lsinfo & TME_SPARC_LSINFO_OP_ATOMIC) == 0)"
	printf "%s\n" "                                   || emulator_off == tlb->tme_sparc_tlb_emulator_off_write))) {"
	printf "%s\n" ""
	printf "%s\n" "          /* return and let our caller do the transfer: */"
	printf "%s\n" "          /* NB: we break const here: */"
	printf "%s\n" "          return ((tme_shared tme_uint8_t *) emulator_off);"
	printf "%s\n" "        }"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* we have to call a slow cycle function: */"
	printf "%s\n" "      lsinfo |= TME_SPARC_LSINFO_SLOW_CYCLES;"
	printf "%s\n" "      assert (ls.tme_sparc_ls_cycle == NULL);"
	if test ${arch} = 32; then
	    endian_little="FALSE"
	else
	    endian_little="(lsinfo & TME_SPARC_LSINFO_ENDIAN_LITTLE)"
	fi
	printf "%s\n" ""
	printf "%s\n" "      /* assume that this operation will transfer the start of the buffer: */"
	printf "%s\n" "      buffer_offset = 0;"
	printf "%s\n" ""
	printf "%s\n" "      /* assume that this is a load or a fetch: */"
	printf "%s\n" "      ls.tme_sparc_ls_cycle = tme_sparc${arch}_load;"
	printf "%s\n" ""
	printf "%s\n" "      /* if this is a store: */"
	printf "%s\n" "      if (lsinfo & TME_SPARC_LSINFO_OP_ST) {"
	printf "%s\n" ""
	printf "%s\n" "        /* put the (first) register to store in the memory buffer: */"
	printf "%s\n" "        value = TME_SPARC_FORMAT3_RD;"
	printf "%s\n" "        value = (${endian_little} ? tme_htole_u${arch}(value) : tme_htobe_u${arch}(value));"
	printf "%s\n" "        ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer${arch}s[0] = value;"
	printf "%s\n" ""
	printf "%s\n" "        /* find the offset in the memory buffer corresponding to the"
	printf "%s\n" "           first address: */"
	printf "%s\n" "        buffer_offset = sizeof(tme_uint${arch}_t) - ls.tme_sparc_ls_size;"
	printf "%s\n" "        if (${endian_little}) {"
	printf "%s\n" "          buffer_offset = 0;"
	printf "%s\n" "        }"

	printf "%s\n" ""
	printf "%s\n" "        /* if this is a std or stda: */"
	printf "%s\n" "        if (lsinfo & TME_SPARC_LSINFO_LDD_STD) {"
	printf "%s\n" ""
	printf "%s\n" "          /* put the odd 32-bit register to store in the memory buffer"
	printf "%s\n" "             after the even 32-bit register.  exactly where this is depends"
	printf "%s\n" "             on the architecture and on the byte order of the store: */"
	printf "%s\n" "          value32 = TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint${arch});"
	printf "%s\n" "          if (${endian_little}) {"
	printf "%s\n" "            value32 = tme_htole_u32(value32);"
	printf "%s\n" "            ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[1] = value32;"
	printf "%s\n" "            buffer_offset = 0;"
	printf "%s\n" "          }"
	printf "%s\n" "          else {"
	printf "%s\n" "            value32 = tme_htobe_u32(value32);"
	printf "%s\n" "            ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[(${arch} / 32)] = value32;"
	printf "%s\n" "            buffer_offset = sizeof(tme_uint${arch}_t) - sizeof(tme_uint32_t);"
	printf "%s\n" "          }"
	printf "%s\n" "        }"
	printf "%s\n" ""
	printf "%s\n" "        /* set the cycle function: */"
	printf "%s\n" "        ls.tme_sparc_ls_cycle = tme_sparc${arch}_store;"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* otherwise, if this is an atomic: */"
	printf "%s\n" "      else if (lsinfo & TME_SPARC_LSINFO_OP_ATOMIC) {"
	printf "%s\n" ""
	printf "%s\n" "        /* set the cycle function: */"
	printf "%s\n" "        ls.tme_sparc_ls_cycle = tme_sparc${arch}_atomic;"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* set the buffer offset for the (first) slow cycle: */"
	printf "%s\n" "      ls.tme_sparc_ls_buffer_offset = buffer_offset;"
	printf "%s\n" ""
	printf "%s\n" "      /* clear the state for this operation: */"
	printf "%s\n" "      ls.tme_sparc_ls_state = 0;"
	printf "%s\n" "    }"

	printf "%s\n" ""
	printf "%s\n" "    /* assume that we won't have to check the TLB again: */"
	printf "%s\n" "    ls.tme_sparc_ls_lsinfo = lsinfo | TME_SPARC_LSINFO_NO_CHECK_TLB;"

	printf "%s\n" "    /* call the slow cycle function: */"
	printf "%s\n" "    (*ls.tme_sparc_ls_cycle)(ic, &ls);"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* if this was a load that has already completed, a store,"
	printf "%s\n" "     or an atomic, make sure our caller doesn't try to complete"
	printf "%s\n" "     a fast transfer: */"
	printf "%s\n" "  if (ls.tme_sparc_ls_lsinfo"
	printf "%s\n" "      & (TME_SPARC_LSINFO_LD_COMPLETED"
	printf "%s\n" "         | TME_SPARC_LSINFO_OP_ST"
	printf "%s\n" "         | TME_SPARC_LSINFO_OP_ATOMIC)) {"
	printf "%s\n" "    return (TME_EMULATOR_OFF_UNDEF);"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* otherwise, this was a load that did slow cycles into the"
	printf "%s\n" "     memory buffer and hasn't updated rd yet.  return a pointer"
	printf "%s\n" "     to the memory buffer so our caller can complete the load: */"
	printf "%s\n" "  return (ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s"
	printf "%s\n" "          - address_first);"
	printf "%s\n" "}"
    fi

    # unfix the architecture version:
    #
    if $header; then :; else
	printf "%s\n" ""
	printf "%s\n" "#undef TME_SPARC_VERSION"
	printf "%s\n" "#define TME_SPARC_VERSION(ic) _TME_SPARC_VERSION(ic)"
    fi

    # the sparc64 support depends on a 64-bit integer type:
    #
    if test ${arch} = 64; then
	printf "%s\n" ""
	printf "%s\n" "#endif /* TME_HAVE_INT64_T */"
    fi
done

# done:
#
exit 0
