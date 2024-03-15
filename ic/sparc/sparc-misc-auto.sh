#! /bin/sh
# Generated from sparc-misc-auto.m4 by GNU Autoconf 2.72.
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


# $Id: sparc-misc-auto.sh,v 1.4 2010/02/14 00:29:48 fredette Exp $

# ic/sparc/sparc-misc-auto.sh - automatically generates C code
# for miscellaneous SPARC emulation support:

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
EOF

# emit the register mapping macros:
if $header; then

    printf "%s\n" ""
    printf "%s\n" "/* the register mapping: */"
    printf "%s\n" "#define TME_SPARC_IREG_UNDEF		(-1)"
    ireg_next=0

    # all integer registers start from register number zero:
    #
    for regnum in 0 1 2 3 4 5 6 7; do
	printf "%s\n" "#define TME_SPARC_IREG_G${regnum}		(${ireg_next})"
	ireg_next=`expr ${ireg_next} + 1`
    done

    # all other registers start after the last register in the last
    # possible register window:
    #
    ireg_base='(TME_SPARC_WINDOWS_MAX * 16)'

    # the sparc64 alternate, MMU, and interrupt globals:
    #
    printf "%s\n" "#define TME_SPARC64_IREG_AG_G0	(${ireg_base} + ${ireg_next} + (8 * 0))"
    printf "%s\n" "#define TME_SPARC64_IREG_MG_G0	(${ireg_base} + ${ireg_next} + (8 * 1))"
    printf "%s\n" "#define TME_SPARC64_IREG_IG_G0	(${ireg_base} + ${ireg_next} + (8 * 2))"
    ireg_next=`expr ${ireg_next} + 24`

    # the current, next, and next-next program counter:
    #
    printf "%s\n" "#define TME_SPARC_IREG_PC		(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`
    printf "%s\n" "#define TME_SPARC_IREG_PC_NEXT		(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`
    printf "%s\n" "#define TME_SPARC_IREG_PC_NEXT_NEXT		(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`

    # the instruction register:
    #
    printf "%s\n" "#define TME_SPARC_IREG_INSN		(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`

    # some temporary registers:
    #
    printf "%s\n" "#define TME_SPARC_IREG_TMP(x)		(${ireg_base} + ${ireg_next} + (x))"
    ireg_next=`expr ${ireg_next} + 3`

    # the Y multiply/divide register:
    #
    printf "%s\n" "#define TME_SPARC_IREG_Y		(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`

    # the floating-point transfer registers.  since these are often
    # treated as 32-bit parts used to transfer 64- and 128-bit values,
    # this block of registers must be aligned to four.  NB that we
    # assume that ${ireg_base} is aligned to at least four:
    #
    while test `expr ${ireg_next} % 4` != 0; do ireg_next=`expr ${ireg_next} + 1`; done
    printf "%s\n" "#define TME_SPARC_IREG_FPX		(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 4`

    # the sparc32 PSR, and the sparc64 PSTATE:
    #
    printf "%s\n" "#define TME_SPARC32_IREG_PSR		(${ireg_base} + ${ireg_next})"
    printf "%s\n" "#define tme_sparc32_ireg_psr		tme_sparc_ireg_uint32(TME_SPARC32_IREG_PSR)"
    printf "%s\n" "#define tme_sparc64_ireg_pstate	tme_sparc_ireg_uint32((${ireg_base} + ${ireg_next}) << 1)"
    ireg_next=`expr ${ireg_next} + 1`

    # the sparc32 WIM, and the common sparc64 register-window state registers:
    #
    printf "%s\n" "#define tme_sparc32_ireg_wim		tme_sparc_ireg_uint32(${ireg_base} + ${ireg_next})"
    printf "%s\n" "#define tme_sparc64_ireg_winstates	tme_sparc_ireg_uint32(((${ireg_base} + ${ireg_next}) << 1) + 0)"
    printf "%s\n" "#define TME_SPARC64_WINSTATES_CWP(x)		(((x) & 0x3f) << (8 * 0))"
    printf "%s\n" "#define tme_sparc64_ireg_cwp		tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 0)"
    printf "%s\n" "#define TME_SPARC64_WINSTATES_CANRESTORE(x)	(((x) & 0x3f) << (8 * 1))"
    printf "%s\n" "#define tme_sparc64_ireg_canrestore	tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 1)"
    printf "%s\n" "#define TME_SPARC64_WINSTATES_CANSAVE(x)	(((x) & 0x3f) << (8 * 2))"
    printf "%s\n" "#define tme_sparc64_ireg_cansave	tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 2)"
    printf "%s\n" "#define TME_SPARC64_WINSTATES_OTHERWIN(x)	(((x) & 0x3f) << (8 * 3))"
    printf "%s\n" "#define tme_sparc64_ireg_otherwin	tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 3)"
    printf "%s\n" "#define tme_sparc64_ireg_winstates_mask tme_sparc_ireg_uint32(((${ireg_base} + ${ireg_next}) << 1) + 1)"
    ireg_next=`expr ${ireg_next} + 1`

    # the sparc32 TBR register, and the sparc64 TBA register:
    #
    printf "%s\n" "#define tme_sparc32_ireg_tbr		tme_sparc_ireg_uint32(${ireg_base} + ${ireg_next})"
    printf "%s\n" "#define tme_sparc64_ireg_tba		tme_sparc_ireg_uint64(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`

    # the sparc64 trap PC, NPC, state, and type registers:
    #
    printf "%s\n" "#define tme_sparc64_ireg_tpc(tl)	tme_sparc_ireg_uint64(${ireg_base} + (TME_SPARC_TL_MAX * 0) + ${ireg_next} + ((tl) - 1))"
    printf "%s\n" "#define tme_sparc64_ireg_tnpc(tl)	tme_sparc_ireg_uint64(${ireg_base} + (TME_SPARC_TL_MAX * 1) + ${ireg_next} + ((tl) - 1))"
    printf "%s\n" "#define TME_SPARC64_IREG_TSTATE(tl)	(${ireg_base} + (TME_SPARC_TL_MAX * 2) + ${ireg_next} + ((tl) - 1))"
    printf "%s\n" "#define tme_sparc64_ireg_tstate(tl)	tme_sparc_ireg_uint64(TME_SPARC64_IREG_TSTATE(tl))"
    printf "%s\n" "#define tme_sparc64_ireg_tstate_ccr(tl) tme_sparc_ireg_uint8((TME_SPARC64_IREG_TSTATE(tl) << 3) + sizeof(tme_uint32_t))"
    printf "%s\n" "#if TME_SPARC_TL_MAX > 8"
    printf "%s\n" "#error \"TME_SPARC_TL_MAX changed\""
    printf "%s\n" "#endif"
    printf "%s\n" "#define tme_sparc64_ireg_tt(tl)	tme_sparc_ireg_uint8(((${ireg_base} + (TME_SPARC_TL_MAX * 3) + ${ireg_next}) << 3) + ((tl) - 1))"
    ireg_base="(${ireg_base} + (TME_SPARC_TL_MAX * 3) + 1)"

    # the sparc64 TL, PIL, ASI, FPRS, remaining register-window state registers, and TICK.NPT:
    #
    printf "%s\n" "#define tme_sparc64_ireg_tl		tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 0)"
    printf "%s\n" "#define tme_sparc64_ireg_pil		tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 1)"
    printf "%s\n" "#define tme_sparc64_ireg_asi		tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 2)"
    printf "%s\n" "#define tme_sparc64_ireg_fprs		tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 3)"
    printf "%s\n" "#define tme_sparc64_ireg_wstate	tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 4)"
    printf "%s\n" "#define tme_sparc64_ireg_cleanwin	tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 5)"
    printf "%s\n" "#define tme_sparc64_ireg_tick_npt	tme_sparc_ireg_uint8(((${ireg_base} + ${ireg_next}) << 3) + 6)"
    ireg_next=`expr ${ireg_next} + 1`

    # the sparc64 TICK (offset) register:
    #
    printf "%s\n" "#define tme_sparc64_ireg_tick_offset	tme_sparc_ireg_uint64(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`

    # the sparc64 version register:
    #
    printf "%s\n" "#define tme_sparc64_ireg_ver		tme_sparc_ireg_uint64(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`

    # the sparc64 CCR:
    # NB: this is a separate register for recode; can it maybe be combined
    # with the block of 8-bit registers above, as the first byte?
    #
    printf "%s\n" "#define TME_SPARC64_IREG_CCR		(${ireg_base} + ${ireg_next})"
    printf "%s\n" "#define tme_sparc64_ireg_ccr		tme_sparc_ireg_uint8(TME_SPARC64_IREG_CCR << 3)"
    ireg_next=`expr ${ireg_next} + 1`

    # our internal sparc64 RCC register:
    #
    printf "%s\n" "#define TME_SPARC64_IREG_RCC		(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`

    # our internal virtual address hole start:
    #
    printf "%s\n" "#define tme_sparc64_ireg_va_hole_start tme_sparc_ireg_uint64(${ireg_base} + ${ireg_next})"
    ireg_next=`expr ${ireg_next} + 1`
fi

# emit the integer condition codes->conditions mapping.  note that the
# nesting of the flag variables is deliberate, to make this array
# indexable with the condition codes value:
#
if $header; then :; else
    printf "%s\n" ""
    printf "%s\n" "/* the icc->conditions mapping: */"
    printf "%s\n" "const tme_uint8_t _tme_sparc_conds_icc[16] = {"
    for nflag in 0 1; do
	for zflag in 0 1; do
	    for vflag in 0 1; do
		for cflag in 0 1; do

		    # the Never condition:
		    #
		    printf %s "  0"

		    # the Equal condition:
		    #
		    if test $zflag = 1; then
			printf %s "  | TME_BIT(1)"
		    fi

		    # the Less or Equal condition:
		    #
		    if test $zflag = 1 || test $nflag != $vflag; then
			printf %s "  | TME_BIT(2)"
		    fi

		    # the Less condition:
		    #
		    if test $nflag != $vflag; then
			printf %s "  | TME_BIT(3)"
		    fi

		    # the Less or Equal Unsigned condition:
		    #
		    if test $cflag = 1 || test $zflag = 1; then
			printf %s "  | TME_BIT(4)"
		    fi

		    # the Carry Set condition:
		    #
		    if test $cflag = 1; then
			printf %s "  | TME_BIT(5)"
		    fi

		    # the Negative condition:
		    #
		    if test $nflag = 1; then
			printf %s "  | TME_BIT(6)"
		    fi

		    # the Overflow Set condition:
		    #
		    if test $vflag = 1; then
			printf %s "  | TME_BIT(7)"
		    fi

		    printf "%s\n" ","
		done
	    done
	done
    done
    printf "%s\n" "};"
fi

# done:
#
exit 0
