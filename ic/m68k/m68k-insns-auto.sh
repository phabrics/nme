#! /bin/sh
# Generated from m68k-insns-auto.m4 by GNU Autoconf 2.72.
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


# $Id: m68k-insns-auto.sh,v 1.26 2009/08/29 19:38:23 fredette Exp $

# ic/m68k/m68k-insns-auto.sh - automatically generates C code
# for many m68k emulation instructions:

#
# Copyright (c) 2002, 2003 Matt Fredette
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
_TME_RCSID("\$Id: m68k-insns-auto.sh,v 1.26 2009/08/29 19:38:23 fredette Exp $");

EOF
if $header; then :; else
    cat <<EOF
#include "m68k-impl.h"

EOF
fi

# permute for the three different operand sizes we need to handle:
for size in 8 16 32; do

    # the shifts needed to get register contents of a specific size:
    case ${size} in
    8)  reg_size_shift=' << 2' ;;
    16) reg_size_shift=' << 1' ;;
    32) reg_size_shift='' ;;
    esac

    # generate the ALU functions:
    for name in add sub cmp neg or and eor not tst move moveq clr cmpa negx addx subx cmpm; do

	# characterize each operation:
	optype=normal ; src=op0 ; dst=op1 ; res=op1 ; arith=no ; with_x=false ; store_res=true
	case "$name" in
	add) op=' + ' ; arith=add ;;
	sub) op=' - ' ; arith=sub ;;
	cmp) op=' - ' ; arith=sub ; store_res=false ;;
	neg) op=' - ' ; arith=sub ; dst=0 ; src=op1 ;;
	or)  op=' | ' ;;
	and) op=' & ' ;;
	eor) op=' ^ ' ;;
	not) op='~ ' ; dst= ; src=op1 ;;
	tst) op='' ; dst= ; src=op1 ; store_res=false ;;
	move) op='' ; dst= ; src=op1 ; res=op0 ;;
	moveq) op='' ; dst= ; src=opc8 ; if test ${size} != 32; then continue; fi ;;
	clr) op='' ; dst= ; src=0 ;;
	cmpa) op=' - ' ; arith=sub ; src=op0.16s32 ; store_res=false
	    if test $size != 16; then continue; fi ;;
	negx) op=' - ' ; arith=sub ; with_x=true ; dst=0 ; src=op1 ;;
	addx) op=' + ' ; arith=add ; optype=mathx ; with_x=true ;;
	subx) op=' - ' ; arith=sub ; optype=mathx ; with_x=true ;;
	cmpm) op=' - ' ; arith=sub ; optype=mathx ; store_res=false ;;
	*) printf "%s\n" "$0 internal error: unknown ALU function $name" 1>&2 ; exit 1 ;;
	esac

	# placeholder for another permutation:
	:

	    # if we're making the header, just emit a declaration:
	    if $header; then
		printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name}${size});"
		continue
	    fi

	    # open the function:
	    printf "%s\n" ""
	    printf %s "/* this does a ${size}-bit \"$name "
	    case "${src}/${dst}" in *op0*) printf %s "SRC, " ;; esac
	    printf "%s\n" "DST\": */"
	    printf "%s\n" "TME_M68K_INSN(tme_m68k_${name}${size})"
	    printf "%s\n" "{"

	    # declare our locals:
	    if test $name = cmpa; then size=32; fi
	    printf %s "  tme_uint${size}_t res"
	    case "${src}/${dst}" in *op0*) printf %s ", op0" ;; esac
	    case "${src}/${dst}" in *op1*) printf %s ", op1" ;; esac
	    printf "%s\n" ";"
	    printf "%s\n" "  tme_uint8_t flags;"

	    # load the operand(s):
	    printf "%s\n" ""
	    printf "%s\n" "  /* load the operand(s): */"
	    case ${optype} in
	    mathx)
		printf "%s\n" "  unsigned int function_code = TME_M68K_FUNCTION_CODE_DATA(ic);"

		# NB: in my 68000 Programmer's Manual, the description
		# of subx is a little backwards from addx and cmpm. in
		# subx, the reg field at bits 0-2 is called the "x"
		# field, where in addx and cmpm it's called the "y"
		# field, and similarly for the reg field at bits 9-11.
		# fortunately, the meanings of the two reg fields is
		# always the same despite this - the reg field at bits
		# 0-2 always identifies the source operand, and the
		# reg field at bits 9-11 always identifies the
		# destination operand:
		printf "%s\n" "  int ireg_src = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);"
		printf "%s\n" "  int ireg_dst = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 9, 3);"

		# the stack pointer must always be adjusted by a multiple of two.
		# assuming ireg < 8, ((ireg + 1) >> 3) == 1 iff ireg == 7, meaning %a7:
		printf %s "  tme_uint32_t ireg_src_adjust = sizeof(tme_uint${size}_t)";
		if test ${size} = 8; then
		    printf %s " + ((ireg_src + 1) >> 3)"
		fi
		printf "%s\n" ";"
		printf %s "  tme_uint32_t ireg_dst_adjust = sizeof(tme_uint${size}_t)";
		if test ${size} = 8; then
		    printf %s " + ((ireg_dst + 1) >> 3)"
		fi
		printf "%s\n" ";"

		case ${name} in

		# cmpm always uses memory and is always postincrement:
		cmpm)
		    printf "%s\n" ""
		    printf "%s\n" "  TME_M68K_INSN_CANFAULT;"
		    printf "%s\n" ""
		    printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
		    printf "%s\n" "    ic->_tme_m68k_ea_function_code = function_code;"
		    printf "%s\n" "    ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_src);"
		    printf "%s\n" "    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_src) += ireg_src_adjust;"
		    printf "%s\n" "  }"
		    printf "%s\n" "  tme_m68k_read_mem${size}(ic, TME_M68K_IREG_MEMY${size});"
		    printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
		    printf "%s\n" "    ic->_tme_m68k_ea_function_code = function_code;"
		    printf "%s\n" "    ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_dst);"
		    printf "%s\n" "    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_dst) += ireg_dst_adjust;"
		    printf "%s\n" "  }"
		    printf "%s\n" "  tme_m68k_read_memx${size}(ic);"
		    printf "%s\n" "  ${dst} = ic->tme_m68k_ireg_memx${size};"
		    printf "%s\n" "  ${src} = ic->tme_m68k_ireg_memy${size};"
		    ;;

		# addx and subx use either registers or memory.  if they use memory,
		# they always predecrement:
		addx|subx)
		    printf "%s\n" "  tme_uint16_t memory;"
		    printf "%s\n" ""
		    printf "%s\n" "  memory = (TME_M68K_INSN_OPCODE & TME_BIT(3));"
		    printf "%s\n" "  if (memory) {"
		    printf "%s\n" "    TME_M68K_INSN_CANFAULT;"
		    printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
		    printf "%s\n" "      ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_src) -= ireg_src_adjust;"
		    printf "%s\n" "      ic->_tme_m68k_ea_function_code = function_code;"
		    printf "%s\n" "      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_src);"
		    printf "%s\n" "    }"
		    printf "%s\n" "    tme_m68k_read_mem${size}(ic, TME_M68K_IREG_MEMY${size});"
		    printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
		    printf "%s\n" "      ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_dst) -= ireg_dst_adjust;"
		    printf "%s\n" "      ic->_tme_m68k_ea_function_code = function_code;"
		    printf "%s\n" "      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_dst);"
		    printf "%s\n" "    }"
		    printf "%s\n" "    tme_m68k_read_memx${size}(ic);"
		    printf "%s\n" "    ${dst} = ic->tme_m68k_ireg_memx${size};"
		    printf "%s\n" "    ${src} = ic->tme_m68k_ireg_memy${size};"
		    printf "%s\n" "  }"
		    printf "%s\n" "  else {"
		    printf "%s\n" "    ${src} = ic->tme_m68k_ireg_uint${size}((TME_M68K_IREG_D0 + ireg_src)${reg_size_shift});"
		    printf "%s\n" "    ${dst} = ic->tme_m68k_ireg_uint${size}((TME_M68K_IREG_D0 + ireg_dst)${reg_size_shift});"
		    printf "%s\n" "  }"
		    ;;
		*) printf "%s\n" "$0 internal error: unknown mathx ${name}" 1>&2 ; exit 1 ;;
		esac
		;;
	    normal)
		for which in src dst; do
		    eval 'what=$'${which}
		    case "x${what}" in
		    x|x0) ;;
		    xop[01].16s32)
			what=`printf "%s\n" ${what} | sed -e 's/\..*//'`
			eval ${which}"=${what}"
			printf "%s\n" "  ${what} = (tme_uint32_t) ((tme_int32_t) *((tme_int16_t *) _${what}));"
			;;
		    xop[01])
			printf "%s\n" "  ${what} = *((tme_uint${size}_t *) _${what});"
			;;
		    xopc8)
		        eval ${which}"='TME_EXT_S8_U${size}((tme_int8_t) (TME_M68K_INSN_OPCODE & 0xff))'"
			;;
		    *) printf "%s\n" "$0 internal error: unknown what ${what}" 1>&2 ; exit 1 ;;
		    esac
		done
		;;
	    *) printf "%s\n" "$0 internal error: unknown optype ${optype}" 1>&2 ; exit 1 ;;
	    esac

	    # perform the operation:
	    printf "%s\n" ""
	    printf "%s\n" "  /* perform the operation: */"
	    printf %s "  res = ${dst}${op}${src}"
	    if $with_x; then
		printf %s "${op}((ic->tme_m68k_ireg_ccr / TME_M68K_FLAG_X) & 1)"
	    fi
	    printf "%s\n" ";"

	    # store the result:
	    if $store_res; then
		printf "%s\n" ""
		printf "%s\n" "  /* store the result: */"
		case ${optype} in
		mathx)
		    printf "%s\n" "  if (memory) {"
		    printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
		    printf "%s\n" "      ic->tme_m68k_ireg_memx${size} = res;"
		    printf "%s\n" "      ic->_tme_m68k_ea_function_code = function_code;"
		    printf "%s\n" "      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ireg_dst);"
		    printf "%s\n" "    }"
		    printf "%s\n" "    tme_m68k_write_memx${size}(ic);"
		    printf "%s\n" "  }"
		    printf "%s\n" "  else {"
		    printf "%s\n" "    ic->tme_m68k_ireg_uint${size}((TME_M68K_IREG_D0 + ireg_dst)${reg_size_shift}) = res;"
		    printf "%s\n" "  }"
		    ;;
		normal)
		    printf "%s\n" "  *((tme_uint${size}_t *) _${res}) = res;"
		    ;;
		*) printf "%s\n" "$0 internal error: unknown optype ${optype}" 1>&2 ; exit 1 ;;
		esac
	    fi

	    # start the status flags, maybe preserving X:
	    printf "%s\n" ""
	    printf "%s\n" "  /* set the flags: */"
	    case "${name}:${arith}" in
	    cmp*|*:no)
		flag_x=
		;;
	    *)
		flag_x=" | TME_M68K_FLAG_X"
		;;
	    esac

	    # set N.  we cast to tme_uint8_t as soon as we know the
	    # bit we want is within the range of the type, to try
	    # to affect the generated assembly:
	    printf "%s\n" "  flags = ((tme_uint8_t) (((tme_uint${size}_t) res) >> (${size} - 1))) * TME_M68K_FLAG_N;"

	    # set Z:
	    if $with_x; then
		printf "%s\n" "  if (res == 0) flags |= (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_Z);"
	    else
		printf "%s\n" "  if (res == 0) flags |= TME_M68K_FLAG_Z;"
	    fi

	    # set V and C and maybe X:
	    case $arith in
	    add)
		case $size in
		8) ones="0xff" ;;
		16) ones="0xffff" ;;
		32) ones="0xffffffff" ;;
		esac
		# if the operands are the same sign, and the result has
	        # a different sign, set V.   we cast to tme_uint8_t as
		# soon as we know the bit we want is within the range
		# of the type, to try to affect the generated assembly:
		printf "%s\n" "  flags |= ((tme_uint8_t) (((${src} ^ ${dst} ^ ${ones}) & (${dst} ^ res)) >> (${size} - 1))) * TME_M68K_FLAG_V;"
		# if src is greater than the logical inverse of dst, set C:
		printf %s "  if (${src} > (${dst} ^ ${ones})"
		if $with_x; then
		    printf %s " || (${src} == (${dst} ^ ${ones}) && (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X))"
		fi
		printf "%s\n" ") flags |= TME_M68K_FLAG_C${flag_x};"
		;;
	    sub)
		# if the operands are different signs, and the result has
	        # a different sign from the first operand, set V.  we
		# cast to tme_uint8_t as soon as we know the bit we want
		# is within the range of the type, to try to affect the
		# generated assembly:
		printf "%s\n" "  flags |= ((tme_uint8_t) (((${src} ^ ${dst}) & (${dst} ^ res)) >> (${size} - 1))) * TME_M68K_FLAG_V;"
		# if src is greater than dst, set C:
		printf %s "  if (${src} > ${dst}"
		if $with_x; then
		    printf %s " || (${src} == ${dst} && (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X))"
		fi
		printf "%s\n" ") flags |= TME_M68K_FLAG_C${flag_x};"
		;;
	    no) ;;
	    esac

	    # preserve X:
	    if test "x${flag_x}" = x; then
		printf "%s\n" "  flags |= (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X);"
	    fi

	    # set the flags:
	    printf "%s\n" "  ic->tme_m68k_ireg_ccr = flags;"

	    # done:
	    printf "%s\n" ""
	    printf "%s\n" "  TME_M68K_INSN_OK;"
	    printf "%s\n" "}"

	    if test $name = cmpa; then size=16; fi
    done

    # generate the wrapper functions for a move of an address register
    # to a predecrement or postincrement EA with that same address
    # register:
    for name in pd pi; do

	# a move of an address registers are only word and long:
	if test $size = 8; then continue; fi

	# if we're making the header, just emit a declaration:
	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_move_sr${name}${size});"
	    continue
	fi

	printf "%s\n" ""
	printf "%s\n" "/* a move of an address register to a predecrement or"
	printf "%s\n" "   postincrement EA with that same address register, must"
	printf "%s\n" "   store the original value of the address register.  since the"
	printf "%s\n" "   predecrement and postincrement code in the executer updates"
	printf "%s\n" "   the address register before the move has happened, we wrap"
	printf "%s\n" "   the normal move function in this one, that gives an op1"
	printf "%s\n" "   argument that is the original value of the address register: */"
	printf "%s\n" "TME_M68K_INSN(tme_m68k_move_sr${name}${size})"
	printf "%s\n" "{"
	if test ${name} = pd; then op='+'; else op='-'; fi
	printf "%s\n" "  /* NB: both this function and tme_m68k_move${size}()"
	printf "%s\n" "     get the source operand as _op1, and the destination"
	printf "%s\n" "     operand as _op0: */"
	printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
	printf "%s\n" "    *((tme_uint${size}_t *) _op0)"
	printf "%s\n" "      = (*((tme_uint${size}_t *) _op1)"
	printf "%s\n" "         ${op} sizeof(tme_uint${size}_t));"
	printf "%s\n" "  }"
	printf "%s\n" "  tme_m68k_move${size}(ic, _op0, _op0);"
	printf "%s\n" "}"
    done

    # generate the address math functions:
    for name in suba adda movea; do

	# the address math functions don't need an 8-bit version:
	if test $size = 8; then continue; fi

	# if we're making the header, just emit a declaration:
	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name}${size});"
	    continue
	fi

	printf "%s\n" ""
	printf "%s\n" "/* the ${name} function on a ${size}-byte EA: */"
	printf "%s\n" "TME_M68K_INSN(tme_m68k_${name}${size})"
	printf "%s\n" "{"
	case $name in
	suba) op='-' ; src="_op0" ; dst="_op1" ;;
	adda) op='+' ; src="_op0" ; dst="_op1" ;;
	movea) op='' ; src="_op1" ; dst="_op0" ;;
	esac
	printf "%s\n" "  *((tme_int32_t *) ${dst}) ${op}= *((tme_int${size}_t *) ${src});"
	printf "%s\n" "  TME_M68K_INSN_OK;"
	printf "%s\n" "}"
    done

    # generate the bit functions:
    for name in btst bchg bclr bset; do

	# the bit functions don't need a 16-bit version:
	if test $size = 16; then continue; fi

	# if we're making the header, just emit a declaration:
	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name}${size});"
	    continue
	fi

	printf "%s\n" ""
	printf "%s\n" "/* the ${name} function on a ${size}-byte EA: */"
	printf "%s\n" "TME_M68K_INSN(tme_m68k_${name}${size})"
	printf "%s\n" "{"
	printf "%s\n" "  tme_uint${size}_t value, bit;"
	printf "%s\n" "  bit = _TME_BIT(tme_uint${size}_t, TME_M68K_INSN_OP0(tme_uint8_t) & (${size} - 1));"
	printf "%s\n" "  value = TME_M68K_INSN_OP1(tme_uint${size}_t);"
	printf "%s\n" "  if (value & bit) {"
	printf "%s\n" "    ic->tme_m68k_ireg_ccr &= ~TME_M68K_FLAG_Z;"
	printf "%s\n" "  }"
	printf "%s\n" "  else {"
	printf "%s\n" "    ic->tme_m68k_ireg_ccr |= TME_M68K_FLAG_Z;"
	printf "%s\n" "  }"
	case ${name} in
	btst) ;;
	bchg) printf "%s\n" "  TME_M68K_INSN_OP1(tme_uint${size}_t) = value ^ bit;" ;;
	bclr) printf "%s\n" "  TME_M68K_INSN_OP1(tme_uint${size}_t) = value & ~bit;" ;;
	bset) printf "%s\n" "  TME_M68K_INSN_OP1(tme_uint${size}_t) = value | bit;" ;;
	esac
	printf "%s\n" "  TME_M68K_INSN_OK;"
	printf "%s\n" "}"
    done

    # generate the shift/rotate functions:
    for func in as ls ro rox; do
	for dir in l r; do
	    name="${func}${dir}"

	    # if we're making the header, just emit a declaration:
	    if $header; then
		printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name}${size});"
		continue
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "/* the ${name} function on a ${size}-byte EA: */"
	    printf "%s\n" "TME_M68K_INSN(tme_m68k_${name}${size})"
	    printf "%s\n" "{"
	    printf "%s\n" "  unsigned int count;"
	    sign=u
	    case "${name}" in
	    asr) sign= ;;
	    asl) printf "%s\n" "  tme_uint${size}_t sign_bits, sign_bits_mask;" ;;
	    rox[lr]) printf "%s\n" "  tme_uint8_t xbit;" ;;
	    *) ;;
	    esac
	    printf "%s\n" "  tme_${sign}int${size}_t res;"
	    printf "%s\n" "  tme_uint8_t flags;"
	    printf "%s\n" ""
	    printf "%s\n" "  /* get the count and operand: */"
	    printf "%s\n" "  count = TME_M68K_INSN_OP0(tme_uint8_t) & 63;"
	    printf "%s\n" "  res = TME_M68K_INSN_OP1(tme_${sign}int${size}_t);"

	    printf "%s\n" ""
	    printf "%s\n" "  /* generate the X, V, and C flags assuming the count is zero: */"
	    printf "%s\n" "  flags = ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X;"
	    case "${name}" in
	    rox[lr])
		printf "%s\n" "  xbit = (flags / TME_M68K_FLAG_X);"
		printf "%s\n" "  flags |= (xbit * TME_M68K_FLAG_C);"
		;;
	    esac

	    printf "%s\n" ""
	    printf "%s\n" "  /* if the count is nonzero, update the result and"
	    printf "%s\n" "     generate the X, V, and C flags: */"
	    printf "%s\n" "  if (count > 0) {"
	    case "${name}" in
	    lsr)
		printf "%s\n" "    if (63 > SHIFTMAX_INT${size}_T"
		printf "%s\n" "        && count > ${size}) {"
		printf "%s\n" "      res = 0;"
		printf "%s\n" "    }"
		printf "%s\n" "    res >>= (count - 1);"
		printf "%s\n" "    flags = (res & 1);"
		printf "%s\n" "    flags *= TME_M68K_FLAG_C;"
		printf "%s\n" "    flags |= (flags * TME_M68K_FLAG_X);"
		printf "%s\n" "    res >>= 1;"
		;;
	    asr)
		printf "%s\n" "    if (63 > SHIFTMAX_INT${size}_T"
		printf "%s\n" "        && count > ${size}) {"
		printf "%s\n" "      res = 0 - (res < 0);"
		printf "%s\n" "    }"
		printf "%s\n" "#ifdef SHIFTSIGNED_INT${size}_T"
		printf "%s\n" "    res >>= (count - 1);"
		printf "%s\n" "#else  /* !SHIFTSIGNED_INT${size}_T */"
		printf "%s\n" "    for (; --count > 0; ) {"
		printf "%s\n" "      res = (res & ~((tme_${sign}int${size}_t) 1)) / 2;"
		printf "%s\n" "    }"
		printf "%s\n" "#endif /* !SHIFTSIGNED_INT${size}_T */"
		printf "%s\n" "    flags = (res & 1);"
		printf "%s\n" "    flags *= TME_M68K_FLAG_C;"
		printf "%s\n" "    flags |= (flags * TME_M68K_FLAG_X);"
		printf "%s\n" "#ifdef SHIFTSIGNED_INT${size}_T"
		printf "%s\n" "    res >>= 1;"
		printf "%s\n" "#else  /* !SHIFTSIGNED_INT${size}_T */"
		printf "%s\n" "    res = (res & ~((tme_${sign}int${size}_t) 1)) / 2;"
		printf "%s\n" "#endif /* !SHIFTSIGNED_INT${size}_T */"
		;;
	    [al]sl)
		if test ${name} = asl; then
		    printf "%s\n" ""
		    printf "%s\n" "    /* we need to see how the sign of the result will change during"
		    printf "%s\n" "       shifting in order to generate V."
		    printf "%s\n" ""
		    printf "%s\n" "       in general, the idea is to get all of the bits that will ever"
		    printf "%s\n" "       appear in the sign position into sign_bits, with a mask in"
		    printf "%s\n" "       sign_bits_mask.  if (sign_bits & sign_bits_mask) is zero or"
		    printf "%s\n" "       sign_bits_mask, clear V, else set V."
		    printf "%s\n" ""
		    printf "%s\n" "       start by loading the operand into sign_bits and setting"
		    printf "%s\n" "       sign_bits_mask to all-bits-one."
		    printf "%s\n" ""
		    printf "%s\n" "       if the shift count is exactly ${size} - 1, then all of the bits"
		    printf "%s\n" "       of the operand will appear in the sign position."
		    printf "%s\n" ""
		    printf "%s\n" "       if the shift count is less than ${size} - 1, then some of the"
		    printf "%s\n" "       less significant bits of the operand will never appear in the"
		    printf "%s\n" "       sign position, so we can shift sign_bits_mask to ignore them."
		    printf "%s\n" ""
		    printf "%s\n" "       if the shift count is greater than ${size} - 1, then all of the"
		    printf "%s\n" "       bits in the operand, plus at least one zero bit, will appear in"
		    printf "%s\n" "       the sign position.  the only way that the sign bit will never"
		    printf "%s\n" "       change during the shift is if the operand was zero to begin with."
		    printf "%s\n" "       without any changes to sign_bits or sign_bits_mask, the final"
		    printf "%s\n" "       test will always work, except when sign_bits is all-bits-one."
		    printf "%s\n" "       the magic below clears the least-significant bit of sign_bits"
		    printf "%s\n" "       iff sign_bits is all-bits-one: */"
		    printf "%s\n" "    sign_bits = res;"
		fi
		printf "%s\n" "    if (63 > SHIFTMAX_INT${size}_T"
		printf "%s\n" "        && count > ${size}) {"
		printf "%s\n" "      res = 0;"
		printf "%s\n" "    }"
		printf "%s\n" "    res <<= (count - 1);"
		printf "%s\n" "    flags = (res >> (${size} - 1));"
		printf "%s\n" "    flags *= TME_M68K_FLAG_C;"
		printf "%s\n" "    flags |= (flags * TME_M68K_FLAG_X);"
		printf "%s\n" "    res <<= 1;"
		if test ${name} = asl; then
		    printf "%s\n" "    sign_bits_mask = (tme_uint${size}_t) -1;"
		    printf "%s\n" "    if (count != ${size} - 1) {"
		    printf "%s\n" "      if (count < ${size}) {"
		    printf "%s\n" "        sign_bits_mask <<= ((${size} - 1) - count);"
		    printf "%s\n" "      }"
		    printf "%s\n" "      else {"
		    printf "%s\n" "        sign_bits ^= !(sign_bits + 1);"
		    printf "%s\n" "      }"
		    printf "%s\n" "    }"
		    printf "%s\n" "    sign_bits &= sign_bits_mask;"
		    printf "%s\n" "    if (sign_bits != 0 && sign_bits != sign_bits_mask) {"
		    printf "%s\n" "      flags |= TME_M68K_FLAG_V;"
		    printf "%s\n" "    }"
		fi
		;;
	    ro[lr])
		printf "%s\n" "    count &= (${size} - 1);"
		if test $dir = l; then
		    printf "%s\n" "    res = (res << count) | (res >> (${size} - count));"
		    printf "%s\n" "    flags |= ((res & 1) * TME_M68K_FLAG_C);"
		else
		    printf "%s\n" "    res = (res << (${size} - count)) | (res >> count);"
		    printf "%s\n" "    flags |= ((res >> (${size} - 1)) * TME_M68K_FLAG_C);"
		fi
		;;
	    rox[lr])
		printf "%s\n" "    count %= (${size} + 1);"
		printf "%s\n" "    flags = xbit;"
		printf "%s\n" "    if (count > 0) {"
		if test $dir = l; then
		    printf "%s\n" "      flags = (res >> (${size} - count)) & 1;"
		    printf "%s\n" "      if (${size} > SHIFTMAX_INT${size}_T"
		    printf "%s\n" "          && count == ${size}) {"
		    printf "%s\n" "        res = 0 | (xbit << (${size} - 1)) | (res >> ((${size} + 1) - ${size}));"
		    printf "%s\n" "      }"
		    printf "%s\n" "      else if (${size} > SHIFTMAX_INT${size}_T"
		    printf "%s\n" "               && count == 1) {"
		    printf "%s\n" "        res = (res << 1) | (xbit << (1 - 1)) | 0;"
		    printf "%s\n" "      }"
		    printf "%s\n" "      else {"
		    printf "%s\n" "        res = (res << count) | (xbit << (count - 1)) | (res >> ((${size} + 1) - count));"
		    printf "%s\n" "      }"
		else
		    printf "%s\n" "      flags = (res >> (count - 1)) & 1;"
		    printf "%s\n" "      if (${size} > SHIFTMAX_INT${size}_T"
		    printf "%s\n" "          && count == ${size}) {"
		    printf "%s\n" "        res = (res << ((${size} + 1) - ${size})) | (xbit << (${size} - ${size})) | 0;"
		    printf "%s\n" "      }"
		    printf "%s\n" "      else if (${size} > SHIFTMAX_INT${size}_T"
		    printf "%s\n" "               && count == 1) {"
		    printf "%s\n" "        res = 0 | (xbit << (${size} - 1)) | (res >> 1);"
		    printf "%s\n" "      }"
		    printf "%s\n" "      else {"
		    printf "%s\n" "        res = (res << ((${size} + 1) - count)) | (xbit << (${size} - count)) | (res >> count);"
		    printf "%s\n" "      }"
		fi
		printf "%s\n" "    }"
		printf "%s\n" "    flags *= TME_M68K_FLAG_C;"
		printf "%s\n" "    flags |= (flags * TME_M68K_FLAG_X);"
		;;
	    esac
		printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* store the result: */"
	    printf "%s\n" "  TME_M68K_INSN_OP1(tme_${sign}int${size}_t) = res;"

	    printf "%s\n" ""
	    printf "%s\n" "  /* generate the N flag.  we cast to tme_uint8_t as soon as we"
	    printf "%s\n" "     know the bit we want is within the range of the type, to try"
	    printf "%s\n" "     to affect the generated assembly: */"
	    printf "%s\n" "  flags |= ((tme_uint8_t) (((tme_uint${size}_t) res) >> (${size} - 1))) * TME_M68K_FLAG_N;"

	    printf "%s\n" ""
	    printf "%s\n" "  /* generate the Z flag: */"
	    printf "%s\n" "  if (res == 0) flags |= TME_M68K_FLAG_Z;"

	    printf "%s\n" ""
	    printf "%s\n" "  /* store the flags: */"
	    printf "%s\n" "  ic->tme_m68k_ireg_ccr = flags;"
	    printf "%s\n" "  TME_M68K_INSN_OK;"
	    printf "%s\n" "}"
	done
    done

    # movep_rm, movep_mr, movem_rm, and movem_mr:
    for name in rm mr; do

	# movep and movem don't need 8-bit versions:
	if test ${size} = 8; then continue; fi

	# if we're making the header, just emit declarations:
	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_movep_${name}${size});"
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_movem_${name}${size});"
	    continue
	fi

	# emit the movep function:
	printf "%s\n" ""
	printf "%s\n" "/* the movep_${name} function on a ${size}-bit dreg: */"
	printf "%s\n" "TME_M68K_INSN(tme_m68k_movep_${name}${size})"
	printf "%s\n" "{"
	printf "%s\n" "  unsigned int function_code;"
	printf "%s\n" "  tme_uint32_t linear_address;"
	if test $name = rm; then
	    printf "%s\n" "  tme_uint${size}_t value;"
	fi
	printf "%s\n" "  int dreg;"
	printf "%s\n" ""
	printf "%s\n" "  TME_M68K_INSN_CANFAULT;"
	printf "%s\n" ""
	printf "%s\n" "  function_code = TME_M68K_FUNCTION_CODE_DATA(ic);"
	printf "%s\n" "  linear_address = TME_M68K_INSN_OP1(tme_uint32_t);"
	printf "%s\n" "  linear_address += (tme_int32_t) ((tme_int16_t) TME_M68K_INSN_SPECOP);"
	printf "%s\n" "  dreg = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 9, 3);"

	# set value:
	if test $name = rm; then
	    printf "%s\n" "  value = ic->tme_m68k_ireg_uint${size}(dreg${reg_size_shift});"
            value="value"
        else
            value="ic->tme_m68k_ireg_uint${size}(dreg${reg_size_shift})"
	fi

	# transfer the bytes:
	pos=${size}
	while test $pos != 0; do
	    pos=`expr ${pos} - 8`
	    printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
	    printf "%s\n" "    ic->_tme_m68k_ea_function_code = function_code;"
	    printf "%s\n" "    ic->_tme_m68k_ea_address = linear_address;"
	    if test $name = rm; then
		printf "%s\n" "    ic->tme_m68k_ireg_memx8 = TME_FIELD_EXTRACTU(${value}, ${pos}, 8);"
		printf "%s\n" "  }"
		printf "%s\n" "  tme_m68k_write_memx8(ic);"
	    else
		printf "%s\n" "  }"
		printf "%s\n" "  tme_m68k_read_memx8(ic);"
		printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
		printf "%s\n" "    TME_FIELD_DEPOSIT${size}(${value}, ${pos}, 8, ic->tme_m68k_ireg_memx8);"
		printf "%s\n" "  }"
	    fi
	    printf "%s\n" "  linear_address += 2;"
	done

	printf "%s\n" "  TME_M68K_INSN_OK;"
	printf "%s\n" "}"

	# emit the movem function:
	printf "%s\n" ""
	printf "%s\n" "/* the movem_${name} function on ${size}-bit registers: */"
	printf "%s\n" "TME_M68K_INSN(tme_m68k_movem_${name}${size})"
	printf "%s\n" "{"
	printf "%s\n" "  int ireg, direction;"
	printf "%s\n" "  tme_uint16_t mask, bit;"
	printf "%s\n" "  unsigned int ea_mode;"
	printf "%s\n" "  tme_uint32_t addend;"
	printf "%s\n" "  tme_uint32_t total_size;"

	printf "%s\n" "  /* get the register mask, and figure out the total size"
	printf "%s\n" "     of the transfer: */"
	printf "%s\n" "  mask = TME_M68K_INSN_SPECOP;"
	printf "%s\n" "  total_size = 0;"
	printf "%s\n" "  if (mask != 0) {"
	printf "%s\n" "    TME_M68K_INSN_CANFAULT;"
	printf "%s\n" "    bit = mask;"
	printf "%s\n" "    do {"
	printf "%s\n" "      total_size += sizeof(tme_uint${size}_t);"
	printf "%s\n" "      bit &= (bit - 1);"
	printf "%s\n" "    } while (bit != 0);"
	printf "%s\n" "  }"

	printf "%s\n" ""
	printf "%s\n" "  /* figure out what direction to move in, and where to start from: */"
	printf "%s\n" "  ea_mode = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3);"
	printf "%s\n" "  direction = 1;"
	printf "%s\n" "  ireg = TME_M68K_IREG_D0;"
	if test $name = rm; then
	    printf "%s\n" "  if (ea_mode == 4) {"
	    printf "%s\n" "    direction = -1;"
	    printf "%s\n" "    ireg = TME_M68K_IREG_A7;"
	    printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* \"For the MC68020, MC68030, MC68040, and CPU32, if"
	    printf "%s\n" "         the addressing register is also moved to memory, the"
	    printf "%s\n" "         value written is the initial register value decremented "
	    printf "%s\n" "         by the size of the operation. The MC68000 and MC68010 "
	    printf "%s\n" "         write the initial register value (not decremented).\" */"
	    printf "%s\n" "      if (ic->tme_m68k_type >= TME_M68K_M68020) {"
	    printf "%s\n" "        ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0"
	    printf "%s\n" "                                 + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3))"
	    printf "%s\n" "          = (ic->_tme_m68k_ea_address - total_size);"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* predecrement the effective address for the first transfer: */"
	    printf "%s\n" "      ic->_tme_m68k_ea_address -= sizeof(tme_uint${size}_t);"
	    printf "%s\n" "    }"
	    printf "%s\n" "  }"
	fi
	printf "%s\n" "  addend = (tme_uint32_t) (direction * sizeof(tme_uint${size}_t));"

	printf "%s\n" ""
	printf "%s\n" "  /* do the transfer: */"
	printf "%s\n" "  for (bit = 1; bit != 0; bit <<= 1) {"
	printf "%s\n" "    if (mask & bit) {"
	if test $name = rm; then
	    printf "%s\n" "      if (!TME_M68K_SEQUENCE_RESTARTING) {"
	    printf "%s\n" "        ic->tme_m68k_ireg_memx${size} = ic->tme_m68k_ireg_uint${size}(ireg${reg_size_shift});"
	    printf "%s\n" "      }"
	    printf "%s\n" "      tme_m68k_write_memx${size}(ic);"
	    printf "%s\n" "      if (!TME_M68K_SEQUENCE_RESTARTING) {"
	else
	    printf "%s\n" "      tme_m68k_read_memx${size}(ic);"
	    printf "%s\n" "      if (!TME_M68K_SEQUENCE_RESTARTING) {"
	    printf %s "        ic->tme_m68k_ireg_uint32(ireg) = "
	    if test $size = 32; then
		printf "%s\n" "ic->tme_m68k_ireg_memx${size};"
	    else
		printf "%s\n" "TME_EXT_S${size}_U32((tme_int${size}_t) ic->tme_m68k_ireg_memx${size});"
	    fi
	fi
	printf "%s\n" "        ic->_tme_m68k_ea_address += addend;"
	printf "%s\n" "      }"
	printf "%s\n" "    }"
	printf "%s\n" "    ireg += direction;"
	printf "%s\n" "  }"
	printf "%s\n" ""

	# for the predecrement and postincrement modes, update the
	# address register:
	if test $name = rm; then
	    printf "%s\n" "  /* if this is the predecrement mode, update the address register: */"
	    printf "%s\n" "  /* \"For the MC68020, MC68030, MC68040, and CPU32, if"
	    printf "%s\n" "     the addressing register is also moved to memory, the"
	    printf "%s\n" "     value written is the initial register value decremented "
	    printf "%s\n" "     by the size of the operation. The MC68000 and MC68010 "
	    printf "%s\n" "     write the initial register value (not decremented).\" */"
	    printf "%s\n" "  if (ea_mode == 4"
	    printf "%s\n" "      && ic->tme_m68k_type < TME_M68K_M68020) {"
	    printf "%s\n" "    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0"
	    printf "%s\n" "                              + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3))"
	    printf "%s\n" "      = (ic->_tme_m68k_ea_address + sizeof(tme_uint${size}_t));"
	    printf "%s\n" "  }"
	else
	    printf "%s\n" "  /* if this is the postincrement mode, update the address register: */"
	    printf "%s\n" "  if (ea_mode == 3) {"
	    printf "%s\n" "    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0"
	    printf "%s\n" "                              + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3))"
	    printf "%s\n" "      = ic->_tme_m68k_ea_address;"
	    printf "%s\n" "  }"
	fi

	printf "%s\n" "  TME_M68K_INSN_OK;"
	printf "%s\n" "}"
    done

    # chk32 and chk16:
    if test $size != 8; then

	# if we're making the header, just emit a declaration:
	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_chk${size});"
	else
	    printf "%s\n" ""
	    printf "%s\n" "/* chk${size}: */"
	    printf "%s\n" "TME_M68K_INSN(tme_m68k_chk${size})"
	    printf "%s\n" "{"
	    printf "%s\n" "  if (*((tme_int${size}_t *) _op0) < 0) {"
	    printf "%s\n" "    ic->tme_m68k_ireg_ccr |= TME_M68K_FLAG_N;"
	    printf "%s\n" "    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;"
	    printf "%s\n" "    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;"
	    printf "%s\n" "    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_CHK));"
	    printf "%s\n" "  }"
	    printf "%s\n" "  if (*((tme_int${size}_t *) _op0) > *((tme_int${size}_t *) _op1)) {"
	    printf "%s\n" "    ic->tme_m68k_ireg_ccr &= ~TME_M68K_FLAG_N;"
	    printf "%s\n" "    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;"
	    printf "%s\n" "    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;"
	    printf "%s\n" "    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_CHK));"
	    printf "%s\n" "  }"
	    printf "%s\n" "  TME_M68K_INSN_OK;"
	    printf "%s\n" "}"
	fi
    fi

    # cas:
    name=cas
    if $header; then
	printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name}${size});"
    else
	printf "%s\n" ""
	printf "%s\n" "/* ${name}${size}: */"
	printf "%s\n" "TME_M68K_INSN(tme_m68k_${name}${size})"
	printf "%s\n" "{"
	printf "%s\n" "  struct tme_m68k_rmw rmw;"
	printf "%s\n" "  struct tme_m68k_tlb *tlb;"
	printf "%s\n" "  int ireg_dc, ireg_du;"
	printf "%s\n" "  tme_uint${size}_t value_dc, value_du, value_mem;"
	printf "%s\n" ""
	printf "%s\n" "  /* start the read/modify/write cycle: */"
	printf "%s\n" "  rmw.tme_m68k_rmw_addresses[0] = ic->_tme_m68k_ea_address;"
	printf "%s\n" "  rmw.tme_m68k_rmw_address_count = 1;"
	printf "%s\n" "  rmw.tme_m68k_rmw_size = sizeof(tme_uint${size}_t);"
	printf "%s\n" "  if (tme_m68k_rmw_start(ic,"
	printf "%s\n" "                         &rmw)) {"
	printf "%s\n" "    TME_M68K_INSN_OK;"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* get the compare and update registers: */"
	printf "%s\n" "  ireg_dc = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 3);"
	printf "%s\n" "  ireg_du = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 6, 3);"
	printf "%s\n" ""
	printf "%s\n" "  /* if we can do the fast compare-and-exchange: */"
	printf "%s\n" "  if (!rmw.tme_m68k_rmw_slow_reads[0]) {"
	printf "%s\n" ""
	printf "%s\n" "    /* get the compare and update values in big-endian byte order: */"
	printf "%s\n" "    value_dc = ic->tme_m68k_ireg_uint${size}(ireg_dc${reg_size_shift});"
	printf "%s\n" "    value_du = ic->tme_m68k_ireg_uint${size}(ireg_du${reg_size_shift});"
	if test ${size} != 8; then
	    printf "%s\n" "    value_dc = tme_htobe_u${size}(value_dc);"
	    printf "%s\n" "    value_du = tme_htobe_u${size}(value_du);"
	fi
	printf "%s\n" ""
	printf "%s\n" "    /* get this TLB entry: */"
	printf "%s\n" "    tlb = rmw.tme_m68k_rmw_tlbs[0];"
	printf "%s\n" ""
	printf "%s\n" "    /* this TLB entry must allow fast reading and fast writing"
	printf "%s\n" "       to the same memory: */"
	printf "%s\n" "    assert (tlb->tme_m68k_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF"
	printf "%s\n" "            && tlb->tme_m68k_tlb_emulator_off_write == tlb->tme_m68k_tlb_emulator_off_read);"
	printf "%s\n" ""
	printf "%s\n" "    /* do the compare-and-exchange: */"
	printf "%s\n" "    value_mem ="
	printf "%s\n" "      tme_memory_atomic_cx${size}(((tme_shared tme_uint${size}_t *)"
	printf "%s\n" "                                   (tlb->tme_m68k_tlb_emulator_off_read"
	printf "%s\n" "                                    + ic->_tme_m68k_ea_address)),"
	printf "%s\n" "                                  value_dc,"
	printf "%s\n" "                                  value_du,"
	printf "%s\n" "                                  tlb->tme_m68k_tlb_bus_rwlock,"
	printf "%s\n" "                                  sizeof(tme_uint8_t));"
   	printf %s "    ic->tme_m68k_ireg_memx${size} = "
	if test ${size} != 8; then printf %s "tme_betoh_u${size}"; fi
	printf "%s\n" "(value_mem);"
	printf "%s\n" ""
	printf "%s\n" "    /* step the transfer count once for the read, and once for the write: */"
	printf "%s\n" "    TME_M68K_SEQUENCE_TRANSFER_STEP;"
	printf "%s\n" "    TME_M68K_SEQUENCE_TRANSFER_STEP;"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* compare the compare operand to the effective address operand: */"
	printf "%s\n" "  tme_m68k_cmp${size}(ic, &ic->tme_m68k_ireg_uint${size}(ireg_dc${reg_size_shift}), &ic->tme_m68k_ireg_memx${size});"
	printf "%s\n" ""
	printf "%s\n" "  /* if the comparison succeeded: */"
	printf "%s\n" "  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_Z) {"
	printf "%s\n" ""
	printf "%s\n" "    /* write the update operand to the effective address operand: */"
	printf "%s\n" "    ic->tme_m68k_ireg_memx${size} = ic->tme_m68k_ireg_uint${size}(ireg_du${reg_size_shift});"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* otherwise, the comparison failed: */"
	printf "%s\n" "  else {"
	printf "%s\n" ""
	printf "%s\n" "    /* write the effective address operand to the compare operand: */"
	printf "%s\n" "    ic->tme_m68k_ireg_uint${size}(ireg_dc${reg_size_shift}) = ic->tme_m68k_ireg_memx${size};"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  /* finish the read/modify/write cycle: */"
	printf "%s\n" "  tme_m68k_rmw_finish(ic,"
	printf "%s\n" "                      &rmw,"
	printf "%s\n" "                      (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_Z) != 0);"
	printf "%s\n" "  TME_M68K_INSN_OK;"
	printf "%s\n" "}"
    fi

    # cas2:
    name=cas2_
    if test $size != 8; then

	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name}${size});"
	else
	    printf "%s\n" ""
	    printf "%s\n" "/* ${name}${size}: */"
	    printf "%s\n" "TME_M68K_INSN(tme_m68k_${name}${size})"
	    printf "%s\n" "{"
	    printf "%s\n" "  struct tme_m68k_rmw rmw;"
	    printf "%s\n" "  int ireg_dcx, ireg_dux;"
	    printf "%s\n" "  int ireg_dcy, ireg_duy;"
	    printf "%s\n" "  const tme_uint16_t specopx = TME_M68K_INSN_SPECOP;"
	    printf "%s\n" "  const tme_uint16_t specopy = TME_M68K_INSN_OP0(tme_uint16_t);"
	    printf "%s\n" ""
	    printf "%s\n" "  /* start the read/modify/write cycle: */"
	    printf "%s\n" "  ic->_tme_m68k_ea_function_code = TME_M68K_FUNCTION_CODE_DATA(ic);"
	    printf "%s\n" "  rmw.tme_m68k_rmw_addresses[0] = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0"
	    printf "%s\n" "                                                           + TME_FIELD_EXTRACTU(specopx, 12, 4));"
	    printf "%s\n" "  rmw.tme_m68k_rmw_addresses[1] = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0"
	    printf "%s\n" "                                                           + TME_FIELD_EXTRACTU(specopy, 12, 4));"
	    printf "%s\n" "  rmw.tme_m68k_rmw_address_count = 2;"
	    printf "%s\n" "  rmw.tme_m68k_rmw_size = sizeof(tme_uint${size}_t);"
	    printf "%s\n" "  if (tme_m68k_rmw_start(ic,"
	    printf "%s\n" "                         &rmw)) {"
	    printf "%s\n" "    TME_M68K_INSN_OK;"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* do the comparisons: */"
	    printf "%s\n" "  ireg_dcx = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specopx, 0, 3);"
	    printf "%s\n" "  ireg_dcy = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specopy, 0, 3);"
	    printf "%s\n" "  tme_m68k_cmp${size}(ic,"
	    printf "%s\n" "                 &ic->tme_m68k_ireg_uint${size}(ireg_dcx${reg_size_shift}),"
	    printf "%s\n" "                 &ic->tme_m68k_ireg_memx${size});"
	    printf "%s\n" "  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_Z) {"
	    printf "%s\n" "    tme_m68k_cmp${size}(ic,"
	    printf "%s\n" "                   &ic->tme_m68k_ireg_uint${size}(ireg_dcy${reg_size_shift}),"
	    printf "%s\n" "                   &ic->tme_m68k_ireg_memy${size});"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* if the comparisons succeeded: */"
	    printf "%s\n" "  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_Z) {"
	    printf "%s\n" ""
	    printf "%s\n" "    /* write the update operands to the effective address operands: */"
	    printf "%s\n" "    ireg_dux = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specopx, 6, 3);"
	    printf "%s\n" "    ireg_duy = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specopy, 6, 3);"
	    printf "%s\n" "    ic->tme_m68k_ireg_memx${size} = ic->tme_m68k_ireg_uint${size}(ireg_dux${reg_size_shift});"
	    printf "%s\n" "    ic->tme_m68k_ireg_memy${size} = ic->tme_m68k_ireg_uint${size}(ireg_duy${reg_size_shift});"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* otherwise, the comparisons failed: */"
	    printf "%s\n" "  else {"
	    printf "%s\n" ""
	    printf "%s\n" "    /* write the effective address operands to the compare operands."
	    printf "%s\n" "       \"If Dc1 and Dc2 specify the same data register and the comparison"
	    printf "%s\n" "        fails, memory operand 1 is stored in the data register.\" */"
	    printf "%s\n" "    ic->tme_m68k_ireg_uint${size}(ireg_dcy${reg_size_shift}) = ic->tme_m68k_ireg_memy${size};"
	    printf "%s\n" "    ic->tme_m68k_ireg_uint${size}(ireg_dcx${reg_size_shift}) = ic->tme_m68k_ireg_memx${size};"
	    printf "%s\n" "  }"
	    printf "%s\n" ""
	    printf "%s\n" "  /* finish the read/modify/write cycle: */"
	    printf "%s\n" "  tme_m68k_rmw_finish(ic,"
	    printf "%s\n" "                      &rmw,"
	    printf "%s\n" "                      (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_Z) != 0);"
	    printf "%s\n" "  TME_M68K_INSN_OK;"
	    printf "%s\n" "}"
	fi
    fi

    # moves:
    if $header; then
	printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_moves${size});"
    else
	printf "%s\n" ""
	printf "%s\n" "/* moves${size}: */"
	printf "%s\n" "TME_M68K_INSN(tme_m68k_moves${size})"
	printf "%s\n" "{"
	printf "%s\n" "  int ireg;"
	printf "%s\n" "  tme_uint${size}_t ireg_value;"
	printf "%s\n" "  unsigned int ea_reg;"
	printf "%s\n" "  unsigned int increment;"
	printf "%s\n" "  TME_M68K_INSN_PRIV;"
	printf "%s\n" "  TME_M68K_INSN_CANFAULT;"
	printf "%s\n" "  ireg = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 4);"
	printf "%s\n" ""
	printf "%s\n" "  /* in case we're storing the same address register used in a"
	printf "%s\n" "     postincrement or predecrement EA, save the current value"
	printf "%s\n" "     of the register now: */"
	printf "%s\n" "  ireg_value = ic->tme_m68k_ireg_uint${size}(ireg${reg_size_shift});"
	printf "%s\n" ""
	printf "%s\n" "  /* we have to handle postincrement and predecrement ourselves: */"
	printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
	printf "%s\n" "    ea_reg = TME_M68K_IREG_A0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);"
	printf "%s\n" "    increment = TME_M68K_SIZE_${size};"
	printf "%s\n" "    if (increment == TME_M68K_SIZE_8 && ea_reg == TME_M68K_IREG_A7) {"
	printf "%s\n" "      increment = TME_M68K_SIZE_16;"
	printf "%s\n" "    }"
	printf "%s\n" "    switch (TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3)) {"
	printf "%s\n" "    case 3: ic->tme_m68k_ireg_uint32(ea_reg) += increment; break;"
	printf "%s\n" "    case 4: ic->_tme_m68k_ea_address = (ic->tme_m68k_ireg_uint32(ea_reg) -= increment); break;"
	printf "%s\n" "    default: break;"
	printf "%s\n" "    }"
	printf "%s\n" "  }"
	printf "%s\n" ""
	printf "%s\n" "  if (TME_M68K_INSN_SPECOP & TME_BIT(11)) {"
	printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
	printf "%s\n" "      ic->tme_m68k_ireg_memx${size} = ireg_value;"
	printf "%s\n" "      ic->_tme_m68k_ea_function_code = ic->tme_m68k_ireg_dfc;"
	printf "%s\n" "    }"
	printf "%s\n" "    tme_m68k_write_memx${size}(ic);"
	printf "%s\n" "  }"
	printf "%s\n" "  else {"
	printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
	printf "%s\n" "      ic->_tme_m68k_ea_function_code = ic->tme_m68k_ireg_sfc;"
	printf "%s\n" "    }"
	printf "%s\n" "    tme_m68k_read_memx${size}(ic);"
	if test ${size} != 32; then
	    printf "%s\n" "    if (ireg >= TME_M68K_IREG_A0) {"
	    printf "%s\n" "      ic->tme_m68k_ireg_uint32(ireg) = "
	    printf "%s\n" "        TME_EXT_S${size}_U32((tme_int${size}_t) ic->tme_m68k_ireg_memx${size});"
	    printf "%s\n" "    }"
	    printf "%s\n" "    else"
	    printf %s "  "
	fi
	printf "%s\n" "    ic->tme_m68k_ireg_uint${size}(ireg${reg_size_shift}) = ic->tme_m68k_ireg_memx${size};"
	printf "%s\n" "  }"
	printf "%s\n" "  TME_M68K_INSN_OK;"
	printf "%s\n" "}"
    fi
done

# generate the memory read and write functions:

# permute on size:
for size in 8 16 32 any; do

    # permute on read or write:
    for name in read write; do
	capname=`printf "%s\n" $name | tr a-z A-Z`
	if test $name = read; then
	    from="from"
	else
	    from="to"
	fi

	# permute on the special-purpose what:
	for what in memx mem inst stack; do

	    # placeholder for another permutation:
	    :

		# dispatch on the size:
		_first=_first ; _last=_last
		case "$size" in
		8) _first= ; _last= ;;
		esac

		# set up the details of each special purpose:
		rval="void"
		args=""
		args_proto=""
		fc=""
		addr=""
		count=""
		tlb="TME_M68K_DTLB_ENTRY(ic, bus_context, function_code, linear_address${_first})"
		flags="TME_M68K_BUS_CYCLE_NORMAL"
		case "${name}-${what}-${size}" in
		*-memx-8 | *-memx-16 | *-memx-32)
		    action="${name}_${what}${size}"
		    fcptr="&ic->_tme_m68k_ea_function_code"
		    addrptr="&ic->_tme_m68k_ea_address"
		    reg="ic->tme_m68k_ireg_memx${size}"
		    regptr="&${reg}"
		    ;;
		*-mem-any)
		    action="${name}_${what}"
		    args_proto=", tme_uint8_t *, unsigned int"
		    args=", tme_uint8_t *buffer, unsigned int count"
		    fcptr="&ic->_tme_m68k_ea_function_code"
		    addrptr="&ic->_tme_m68k_ea_address"
		    _last=
		    reg=
		    regptr="buffer"
		    ;;
		*-mem-8 | *-mem-16 | *-mem-32)
		    action="${name}_${what}${size}"
		    args_proto=", int"
		    args="${args_proto} ireg"
		    fcptr="&ic->_tme_m68k_ea_function_code"
		    addrptr="&ic->_tme_m68k_ea_address"
		    reg="ic->tme_m68k_ireg_uint${size}(ireg)"
		    regptr="&${reg}"
		    ;;
		read-stack-16 | read-stack-32)
		    action="pop${size}"
		    args_proto=", tme_uint${size}_t *"
		    args="${args_proto}_value"
		    fc="TME_M68K_FUNCTION_CODE_DATA(ic)"
		    addrptr="&ic->tme_m68k_ireg_a7"
		    regptr="_value"
		    reg="*${regptr}"
		    ;;
		write-stack-16 | write-stack-32)
		    action="push${size}"
		    args_proto=", tme_uint${size}_t "
		    args="${args_proto}value"
		    fc="TME_M68K_FUNCTION_CODE_DATA(ic)"
		    addr="ic->tme_m68k_ireg_a7 - sizeof(tme_uint${size}_t)"
		    reg="value"
		    regptr="&${reg}"
		    ;;
		read-inst-16 | read-inst-32)
		    rval="tme_uint${size}_t"
		    action="fetch${size}"
		    args_proto=", tme_uint32_t"
		    args="${args_proto} pc"
		    fc="TME_M68K_FUNCTION_CODE_PROGRAM(ic)"
		    addrptr="&pc"
		    tlb="&ic->_tme_m68k_itlb"
		    flags="TME_M68K_BUS_CYCLE_FETCH"
		    ;;
		*)
		    continue
		    ;;
		esac

		# if we're making the header, just emit a declaration:
		if $header; then
		    printf "%s\n" "${rval} tme_m68k_${action} _TME_P((struct tme_m68k *${args_proto}));"
		    continue
		fi

		# start the function:
		printf "%s\n" ""
		printf "%s\n" "/* this ${name}s a ${size}-bit ${what} value: */"
		printf "%s\n" "${rval}"
		printf "%s\n" "tme_m68k_${action}(struct tme_m68k *ic${args}) "
		printf "%s\n" "{"

		# our locals:
		printf "%s\n" "  tme_bus_context_t bus_context = ic->_tme_m68k_bus_context;"
		printf %s "  unsigned int function_code = "
		if test "x${fc}" != x; then
		    printf "%s\n" "${fc};"
		    fc="function_code"
		    fcptr="&function_code"
		else
		    fc=`printf "%s\n" ${fcptr} | sed -e 's,^&,,'`
		    printf "%s\n" "${fc};"
		fi
		printf %s "  tme_uint32_t linear_address${_first} = "
		if test "x${addr}" != x; then
		    printf "%s\n" "${addr};"
		    addr="linear_address${_first}"
		    addrptr="&linear_address${_first}"
		else
		    addr=`printf "%s\n" ${addrptr} | sed -e 's,^&,,'`
		    printf "%s\n" "${addr};"
		fi
		if test "x${count}" = x; then
		    if test $size = any; then count=count; else count="sizeof(tme_uint${size}_t)"; fi
		fi
		if test x$_last != x; then
		    printf "%s\n" "  tme_uint32_t linear_address${_last} = linear_address_first + ${count} - 1;";
		fi
		printf "%s\n" "  struct tme_m68k_tlb *tlb = ${tlb};"
		if test $size != any; then
		    memtype="tme_uint${size}_t"
		    printf "%s\n" "  ${memtype} mem_value;"
		    memtype="tme_shared ${memtype} *"
		    if test $name = read; then memtype="const ${memtype}"; fi
		    printf "%s\n" "  ${memtype}mem;"
		fi
		case "$what" in
		inst)
		    printf "%s\n" "  unsigned int fetch_slow_next = ic->_tme_m68k_insn_fetch_slow_next;"
		    regptr="((tme_uint${size}_t *) (((tme_uint8_t *) &ic->_tme_m68k_insn_fetch_buffer[0]) + fetch_slow_next))"
		    reg="*${regptr}"
		    ;;
		esac

		# track statistics:
		printf "%s\n" ""
		printf "%s\n" "#ifdef _TME_M68K_STATS"
		printf "%s\n" "  ic->tme_m68k_stats.tme_m68k_stats_memory_total++;"
		printf "%s\n" "#endif /* _TME_M68K_STATS */"

		# if this is a write, log the value written:
		if test $name = write; then
		    printf "%s\n" ""
		    printf "%s\n" "  /* log the value written: */"
		    if test $size != any; then
			printf "%s\n" "  tme_m68k_verify_mem${size}(ic, ${fc}, ${addr}, ${reg}, TME_BUS_CYCLE_WRITE);"
			printf "%s\n" "  tme_m68k_log(ic, 1000, TME_OK, "
			printf "%s\n" "               (TME_M68K_LOG_HANDLE(ic),"
			printf "%s\n" "                _(\"${action}\t%d:0x%08x:\t0x%0"`expr ${size} / 4`"x\"),"
			printf "%s\n" "                ${fc},"
			printf "%s\n" "                ${addr},"
			printf "%s\n" "                ${reg}));"
		    else
			printf "%s\n" "  tme_m68k_verify_mem_any(ic, ${fc}, ${addr}, ${regptr}, ${count}, TME_BUS_CYCLE_WRITE);"
			printf "%s\n" "  tme_m68k_log_start(ic, 1000, TME_OK) {"
			printf "%s\n" "    unsigned int byte_i;"
			printf "%s\n" "    tme_log_part(TME_M68K_LOG_HANDLE(ic),"
			printf "%s\n" "                 _(\"${action} %d:0x%08x count %d:\"),"
			printf "%s\n" "                 ${fc},"
			printf "%s\n" "                 ${addr},"
			printf "%s\n" "                 ${count});"
			printf "%s\n" "    for (byte_i = 0; byte_i < count ; byte_i++) {"
			printf "%s\n" "      tme_log_part(TME_M68K_LOG_HANDLE(ic), \" 0x%02x\", (${regptr})[byte_i]);"
			printf "%s\n" "    }"
			printf "%s\n" "  } tme_m68k_log_finish(ic);"
		    fi
		fi

		printf "%s\n" ""
		printf "%s\n" "  /* busy this TLB entry: */"
		printf "%s\n" "  tme_m68k_tlb_busy(tlb);"

		# if this is an any-transfer:
		#
		if test $size = any; then
		    printf "%s\n" ""
		    printf "%s\n" "  /* call the full ${name} function: */"
		    printf "%s\n" "  tme_m68k_${name}(ic, tlb, ${fcptr}, ${addrptr}, ${regptr}, ${count}, TME_M68K_BUS_CYCLE_RAW);"

		# otherwise, this is not an any-transfer:
		#
		else

		    # dispatch on the what:
		    #
		    i=
		    case "$what" in
		    inst)
			printf "%s\n" ""
			printf "%s\n" "  /* if this fetch was done by the fast executor: */"
			printf "%s\n" "  if (__tme_predict_true(fetch_slow_next < ic->_tme_m68k_insn_fetch_slow_count_fast)) {"
			printf "%s\n" ""
			printf "%s\n" "    /* the entire fetch must be in the instruction buffer, and"
			printf "%s\n" "       we must be restarting: */"
			printf "%s\n" "    assert ((fetch_slow_next + sizeof(tme_uint${size}_t))"
			printf "%s\n" "            <= ic->_tme_m68k_insn_fetch_slow_count_fast);"
			printf "%s\n" "    assert (TME_M68K_SEQUENCE_RESTARTING);"
			printf "%s\n" "    mem_value = tme_memory_read${size}(${regptr}, sizeof(tme_uint16_t));"
			printf "%s\n" "  }"
			printf "%s\n" ""
			printf "%s\n" "  /* otherwise, this fetch was not done by the fast executor: */"
			printf "%s\n" "  else {"
			printf "%s\n" ""
			printf "%s\n" "    /* if we're restarting, but the offset in the instruction buffer"
			printf "%s\n" "       to fetch into is at the instruction buffer total, this must be"
			printf "%s\n" "       a fake fault caused by the fast executor.  we confirm this by"
			printf "%s\n" "       checking that this transfer \"caused\" the fault, and that this"
			printf "%s\n" "       transfer will be the first slow one after any fast fetches."
			printf "%s\n" "       in this case, we can cancel the restart for now: */"
			printf "%s\n" "    if (TME_M68K_SEQUENCE_RESTARTING"
			printf "%s\n" "        && (fetch_slow_next"
			printf "%s\n" "            == ic->_tme_m68k_insn_fetch_slow_count_total)) {"
			printf "%s\n" "      assert ((ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next"
			printf "%s\n" "               == ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted)"
			printf "%s\n" "              && (fetch_slow_next"
			printf "%s\n" "                  == ic->_tme_m68k_insn_fetch_slow_count_fast));"
			printf "%s\n" "      ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted--;"
			printf "%s\n" "    }"
			printf "%s\n" ""
			printf "%s\n" "    /* if we're not restarting: */"
			printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
			printf "%s\n" ""
			printf "%s\n" "      /* we advance the instruction buffer total *before* we do"
			printf "%s\n" "         what may be a slow fetch, because we may transfer a few"
			printf "%s\n" "         bytes and then fault.  without this, those few bytes"
			printf "%s\n" "         would not get saved in the exception stack frame and"
			printf "%s\n" "         restored later before the continuation of the fetch: */"
			printf "%s\n" "      ic->_tme_m68k_insn_fetch_slow_count_total += sizeof(tme_uint${size}_t);"
			printf "%s\n" "    }"
			printf "%s\n" ""
			printf "%s\n" "    /* make sure that if this is a new transfer or if this"
			printf "%s\n" "       transfer faulted, that we're fetching for the current"
			printf "%s\n" "       last positions in the instruction buffer: */"
			printf "%s\n" "    assert ((ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next"
			printf "%s\n" "             < ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted)"
			printf "%s\n" "            || ((fetch_slow_next + sizeof(tme_uint${size}_t))"
			printf "%s\n" "                == ic->_tme_m68k_insn_fetch_slow_count_total));"
			i="  "
			;;
		    esac

		    printf "%s\n" ""
		    printf "%s\n" "${i}  /* if we aren't restarting, and this address is properly aligned,"
		    printf "%s\n" "${i}     and this TLB entry covers the operand and allows fast ${name}s: */"
		    printf "%s\n" "${i}  if (__tme_predict_true(!TME_M68K_SEQUENCE_RESTARTING"
		    align_min="sizeof(tme_uint8_t)"
		    if test $size != 8; then
			printf %s "${i}                         && ("
			if test $what = inst; then
			    align_min="sizeof(tme_uint16_t)"
			    printf %s "(${align_min} - 1)"
			else
			    printf %s "ic->_tme_m68k_bus_16bit"
			fi
			printf "%s\n" " & linear_address${_first}) == 0"
		    fi
		    printf "%s\n" "${i}                         && tme_m68k_tlb_is_valid(tlb)"
		    printf "%s\n" "${i}                         && tlb->tme_m68k_tlb_bus_context == bus_context"
		    printf "%s\n" "${i}                         && (tlb->tme_m68k_tlb_function_codes_mask"
		    printf "%s\n" "${i}                             & TME_BIT(function_code))"
		    printf "%s\n" "${i}                         && linear_address${_first} >= (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first"
		    printf "%s\n" "${i}                         && linear_address${_last} <= (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last"
		    printf "%s\n" "${i}                         && tlb->tme_m68k_tlb_emulator_off_${name} != TME_EMULATOR_OFF_UNDEF)) {"

		    printf "%s\n" ""
		    printf "%s\n" "${i}    /* make the emulator memory pointer: */"
		    printf "%s\n" "${i}    mem = (${memtype}) (tlb->tme_m68k_tlb_emulator_off_${name} + linear_address${_first});"

		    if test $name = write; then
			if test $size = 8; then
			    printf "%s\n" ""
			    printf "%s\n" "${i}    /* get the value to write: */"
			    printf "%s\n" "${i}    mem_value = ${reg};"
			else
			    printf "%s\n" ""
			    printf "%s\n" "${i}    /* get the value to write, in big-endian byte order: */"
			    printf "%s\n" "${i}    mem_value = tme_htobe_u${size}(${reg});"
			fi
		    fi

		    printf "%s\n" ""
		    printf "%s\n" "${i}    /* do the ${size}-bit bus ${name}: */"
		    if test $name = read; then
			printf %s "${i}    mem_value = tme_memory_bus_${name}${size}(mem"
		    else
			printf %s "${i}    tme_memory_bus_${name}${size}(mem, mem_value"
		    fi
		    printf "%s\n" ", tlb->tme_m68k_tlb_bus_rwlock, ${align_min}, sizeof(tme_uint32_t));"

		    if test $name = read; then
			if test $what = inst; then
			    printf "%s\n" ""
			    printf "%s\n" "${i}    /* put the value read, in host byte order: */"
			    printf "%s\n" "${i}    mem_value = tme_betoh_u${size}(mem_value);"
			    printf "%s\n" "${i}    tme_memory_write${size}(${regptr}, mem_value, sizeof(tme_uint16_t));"
			elif test $size = 8; then
			    printf "%s\n" ""
			    printf "%s\n" "${i}    /* put the value read: */"
			    printf "%s\n" "${i}    ${reg} = mem_value;"
			else
			    printf "%s\n" ""
			    printf "%s\n" "${i}    /* put the value read, in host byte order: */"
			    printf "%s\n" "${i}    ${reg} = tme_betoh_u${size}(mem_value);"
			fi
		    fi

		    printf "%s\n" ""
		    printf "%s\n" "${i}    /* step the transfer count: */"
		    printf "%s\n" "${i}    TME_M68K_SEQUENCE_TRANSFER_STEP;"
		    printf "%s\n" "${i}  }"

		    printf "%s\n" ""
		    printf "%s\n" "${i}  /* otherwise, do the bus cycles the slow way: */"
		    printf "%s\n" "${i}  else {"
		    printf "%s\n" "${i}    tme_m68k_${name}${size}(ic, tlb,"
		    printf "%s\n" "${i}                    ${fcptr},"
		    printf "%s\n" "${i}                    ${addrptr},"
		    printf "%s\n" "${i}                    ${regptr},"
		    printf "%s\n" "${i}                    ${flags});"
		    if test ${what} = inst; then
			printf "%s\n" "${i}    mem_value = tme_memory_read${size}(${regptr}, sizeof(tme_uint16_t));"
		    fi
		    printf "%s\n" "${i}  }"
		fi
		if test "x${i}" != x; then
		    printf "%s\n" "  }"
		fi

		printf "%s\n" ""
		printf "%s\n" "  /* unbusy this TLB entry: */"
		printf "%s\n" "  tme_m68k_tlb_unbusy(tlb);"

		# if this is a read, log the value read:
		if test $name = read; then
		    printf "%s\n" ""
		    printf "%s\n" "  /* log the value read: */"
		    if test $size != any; then
			printf "%s\n" "  tme_m68k_verify_mem${size}(ic, ${fc}, ${addr}, ${reg}, TME_BUS_CYCLE_READ);"
			printf "%s\n" "  tme_m68k_log(ic, 1000, TME_OK,"
			printf "%s\n" "               (TME_M68K_LOG_HANDLE(ic),"
			printf "%s\n" "                _(\"${action}\t%d:0x%08x:\t0x%0"`expr ${size} / 4`"x\"),"
			printf "%s\n" "                ${fc},"
			printf "%s\n" "                ${addr},"
			printf "%s\n" "                ${reg}));"
		    else
			printf "%s\n" "  tme_m68k_verify_mem_any(ic, ${fc}, ${addr}, ${regptr}, ${count}, TME_BUS_CYCLE_READ);"
			printf "%s\n" "  tme_m68k_log_start(ic, 1000, TME_OK) {"
			printf "%s\n" "    unsigned int byte_i;"
			printf "%s\n" "    tme_log_part(TME_M68K_LOG_HANDLE(ic),"
			printf "%s\n" "                 _(\"${action} %d:0x%08x count %d:\"),"
			printf "%s\n" "                 ${fc},"
			printf "%s\n" "                 ${addr},"
			printf "%s\n" "                 ${count});"
			printf "%s\n" "    for (byte_i = 0; byte_i < count ; byte_i++) {"
			printf "%s\n" "      tme_log_part(TME_M68K_LOG_HANDLE(ic), \" 0x%02x\", (${regptr})[byte_i]);"
			printf "%s\n" "    }"
			printf "%s\n" "  } tme_m68k_log_finish(ic);"
		    fi
		fi

		# perform any updating and value returning:
		case "$what" in
		stack)
		    if test $name = read; then dir="+"; else dir="-"; fi
		    printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
		    printf "%s\n" "    ic->tme_m68k_ireg_a7 ${dir}= sizeof(tme_uint${size}_t);"
		    printf "%s\n" "  }"
		    ;;
		inst)
		    printf "%s\n" ""
		    printf "%s\n" "  /* advance the offset in the instruction buffer for the next slow fetch: */"
		    printf "%s\n" "  fetch_slow_next += sizeof(tme_uint${size}_t);"
		    printf "%s\n" "  ic->_tme_m68k_insn_fetch_slow_next = fetch_slow_next;"
		    printf "%s\n" ""
		    printf "%s\n" "  /* return the fetched value: */"
		    printf "%s\n" "  return(mem_value);"
		    ;;
		esac

		printf "%s\n" "}"
	    :
	done

	# the general-purpose cycle-making read and write macros:
	if test ${size} != any; then

	    # if we're making the header, emit a macro:
	    if $header; then
		printf "%s\n" "#define tme_m68k_${name}${size}(ic, t, fc, la, _v, f) \\"
		printf "%s\n" "  tme_m68k_${name}(ic, t, fc, la, (tme_uint8_t *) (_v), sizeof(tme_uint${size}_t), f)"
	    fi
	else

	    # if we're making the header, just emit a declaration:
	    if $header; then
		printf "%s\n" "void tme_m68k_${name} _TME_P((struct tme_m68k *, struct tme_m68k_tlb *, unsigned int *, tme_uint32_t *, tme_uint8_t *, unsigned int, unsigned int));"
		continue
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "/* this ${name}s a region of address space using actual bus cycles: */"
	    printf "%s\n" "void"
	    printf "%s\n" "tme_m68k_${name}(struct tme_m68k *ic, "
	    printf "%s\n" "              struct tme_m68k_tlb *tlb,"
	    printf "%s\n" "              unsigned int *_function_code, "
	    printf "%s\n" "              tme_uint32_t *_linear_address, "
	    printf "%s\n" "              tme_uint8_t *reg,"
	    printf "%s\n" "              unsigned int reg_size,"
	    printf "%s\n" "              unsigned int flags)"
	    printf "%s\n" "{"

	    # our locals:
	    printf "%s\n" "  unsigned int function_code;"
	    printf "%s\n" "  tme_uint32_t linear_address;"
	    printf "%s\n" "  tme_bus_addr_t physical_address;"
	    printf "%s\n" "  int shift;"
	    printf "%s\n" "  struct tme_bus_cycle cycle;"
	    printf "%s\n" "  unsigned int transferred, resid, cycle_size;"
	    printf "%s\n" "  int exception;"
	    printf "%s\n" "  int err;"
	    printf "%s\n" "  tme_uint8_t *reg_p;"
	    printf "%s\n" "  unsigned int buffer_i;"
	    printf "%s\n" "  tme_uint8_t reg_buffer[sizeof(tme_uint32_t) * 2];"
	    if test ${name} = read; then name_const_mem="const "; else name_const_mem= ; fi
	    printf "%s\n" "  ${name_const_mem}tme_shared tme_uint8_t *mem;"

	    printf "%s\n" ""
	    printf "%s\n" "  /* if we're not restarting, everything is fresh: */"
	    printf "%s\n" "  if (!TME_M68K_SEQUENCE_RESTARTING) {"
	    printf "%s\n" "    function_code = *_function_code;"
	    printf "%s\n" "    linear_address = *_linear_address;"
	    printf "%s\n" "    transferred = 0;"
	    printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* otherwise, if this is the transfer that faulted, restore"
	    printf "%s\n" "     our state to the cycle that faulted, then take into account"
	    printf "%s\n" "     any data provided by a software rerun of the faulted cycle: */"
	    printf "%s\n" "  else if (ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted"
	    printf "%s\n" "           == ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next) {"
	    printf "%s\n" "    function_code = *_function_code = ic->_tme_m68k_group0_function_code;"
	    printf "%s\n" "    linear_address = ic->_tme_m68k_group0_address;"
	    printf "%s\n" "    transferred = ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted_after;"
	    printf "%s\n" "    if (transferred >= reg_size) abort();"
	    printf "%s\n" "    *_linear_address = linear_address - transferred;"
	    printf "%s\n" "    resid = reg_size - transferred;"
	    printf "%s\n" "    if (ic->_tme_m68k_group0_buffer_${name}_size > resid) abort();"
	    printf "%s\n" "    if (ic->_tme_m68k_group0_buffer_${name}_softrr > resid) abort();"
	    if test $name = read; then cmp=">"; else cmp="=="; fi
	    printf "%s\n" "    if (ic->_tme_m68k_group0_buffer_${name}_softrr ${cmp} 0) {"
	    printf "%s\n" "#ifdef WORDS_BIGENDIAN"
	    printf "%s\n" "      memcpy(reg + transferred, "
	    printf "%s\n" "             ic->_tme_m68k_group0_buffer_${name},"
	    printf "%s\n" "             ic->_tme_m68k_group0_buffer_${name}_size);"
	    printf "%s\n" "#else  /* !WORDS_BIGENDIAN */"
	    printf "%s\n" "      reg_p = (reg + reg_size - 1) - transferred;"
	    printf "%s\n" "      for (buffer_i = 0;"
	    printf "%s\n" "           buffer_i < ic->_tme_m68k_group0_buffer_${name}_size;"
	    printf "%s\n" "           buffer_i++) {"
	    printf "%s\n" "        *(reg_p--) = ic->_tme_m68k_group0_buffer_${name}[buffer_i];"
	    printf "%s\n" "      }"
	    printf "%s\n" "#endif /* !WORDS_BIGENDIAN */"
	    printf "%s\n" "    }"
	    printf "%s\n" "    transferred += ic->_tme_m68k_group0_buffer_${name}_softrr;"
	    printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* otherwise, a later transfer has faulted.  just step the"
	    printf "%s\n" "     transfer number and return: */"
	    printf "%s\n" "  else {"
	    printf "%s\n" "    TME_M68K_SEQUENCE_TRANSFER_STEP;"
	    printf "%s\n" "    return;"
	    printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* do as many bus cycles as needed to complete the transfer: */"
	    printf "%s\n" "  exception = TME_M68K_EXCEPTION_NONE;"
	    printf "%s\n" "  cycle_size = 0;"
	    printf "%s\n" "  for(; transferred < reg_size; ) {"
	    printf "%s\n" "    resid = reg_size - transferred;"

	    printf "%s\n" ""
	    printf "%s\n" "    /* start the bus cycle structure: */"
	    printf "%s\n" "    cycle.tme_bus_cycle_type = TME_BUS_CYCLE_${capname};"
	    printf "%s\n" "    if (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG"
	    printf "%s\n" "        || (flags & TME_M68K_BUS_CYCLE_RAW)) {"
	    printf "%s\n" "      cycle.tme_bus_cycle_buffer = reg + transferred;"
	    printf "%s\n" "      cycle.tme_bus_cycle_buffer_increment = 1;"
	    printf "%s\n" "    }"
	    printf "%s\n" "    else {"
	    printf "%s\n" "      cycle.tme_bus_cycle_buffer = reg + reg_size - (1 + transferred);"
	    printf "%s\n" "      cycle.tme_bus_cycle_buffer_increment = -1;"
	    printf "%s\n" "    }"

	    printf "%s\n" ""
	    printf "%s\n" "    /* if we're emulating a CPU with a 16-bit bus interface: */"
	    printf "%s\n" "    if (ic->_tme_m68k_bus_16bit) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* if we're trying to transfer a non-power-of-two"
	    printf "%s\n" "         number of bytes, either the CPU is broken (no"
	    printf "%s\n" "         instructions ever transfer a non-power-of-two"
	    printf "%s\n" "         number of bytes), or this function allowed an"
	    printf "%s\n" "         unaligned transfer: */"
	    printf "%s\n" "      assert((resid & (resid - 1)) == 0"
	    printf "%s\n" "             || (flags & TME_M68K_BUS_CYCLE_RAW));"
	    printf "%s\n" ""
	    printf "%s\n" "      /* only byte transfers can be unaligned: */"
	    printf "%s\n" "      if (resid > sizeof(tme_uint8_t)"
	    printf "%s\n" "          && (linear_address & 1)) {"
	    printf "%s\n" "          exception = TME_M68K_EXCEPTION_AERR;"
	    printf "%s\n" "          break;"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* set the bus-size specific parts of the bus cycle structure: */"
	    printf "%s\n" "      cycle_size = TME_MIN(resid, sizeof(tme_uint16_t));"
	    printf "%s\n" "      cycle.tme_bus_cycle_size = cycle_size;"
	    printf "%s\n" "      cycle.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS16_LOG2);"
	    printf "%s\n" "      cycle.tme_bus_cycle_lane_routing = "
	    printf "%s\n" "        &tme_m68k_router_16[TME_M68K_BUS_ROUTER_INDEX(TME_BUS16_LOG2, cycle_size, linear_address)];"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* otherwise we're emulating a CPU with a 32-bit bus interface: */"
	    printf "%s\n" "    else {"
	    if test $name = read; then
		printf "%s\n" ""
		printf "%s\n" "      /* an instruction fetch must be aligned: */"
		printf "%s\n" "      if (flags & TME_M68K_BUS_CYCLE_FETCH) {"
		printf "%s\n" "        if (linear_address & 1) {"
		printf "%s\n" "          exception = TME_M68K_EXCEPTION_AERR;"
		printf "%s\n" "          break;"
		printf "%s\n" "        }"
		printf "%s\n" "        assert(!(resid & 1));"
		printf "%s\n" "      }"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "      /* set the bus-size specific parts of the bus cycle structure: */"
	    printf "%s\n" "      cycle_size = TME_MIN(resid, sizeof(tme_uint32_t) - (linear_address & (sizeof(tme_uint32_t) - 1)));"
	    printf "%s\n" "      cycle.tme_bus_cycle_size = cycle_size;"
	    printf "%s\n" "      cycle.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS32_LOG2);"
	    printf "%s\n" "      cycle.tme_bus_cycle_lane_routing = "
	    printf "%s\n" "        &tme_m68k_router_32[TME_M68K_BUS_ROUTER_INDEX(TME_BUS32_LOG2, cycle_size, linear_address)];"
	    printf "%s\n" "    }"

	    printf "%s\n" ""
	    printf "%s\n" "    /* loop while this TLB entry is invalid or does not apply: */"
	    printf "%s\n" "    for (; __tme_predict_false(tme_m68k_tlb_is_invalid(tlb)"
	    printf "%s\n" "                               || tlb->tme_m68k_tlb_bus_context != ic->_tme_m68k_bus_context"
	    printf "%s\n" "                               || (tlb->tme_m68k_tlb_function_codes_mask & TME_BIT(function_code)) == 0"
	    printf "%s\n" "                               || linear_address < (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first"
	    printf "%s\n" "                               || linear_address > (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last"
	    printf "%s\n" "                               || (tlb->tme_m68k_tlb_emulator_off_${name} == TME_EMULATOR_OFF_UNDEF"
	    printf "%s\n" "                                   && (tlb->tme_m68k_tlb_cycles_ok & TME_BUS_CYCLE_${capname}) == 0)); ) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* this must not be part of a read/modify/write cycle: */"
	    printf "%s\n" "      assert(!(flags & TME_M68K_BUS_CYCLE_RMW));"
	    printf "%s\n" ""
	    printf "%s\n" "      /* fill this TLB entry: */"
	    printf "%s\n" "      tme_m68k_tlb_fill(ic, tlb,"
	    printf "%s\n" "                        function_code,"
	    printf "%s\n" "                        linear_address,"
	    printf "%s\n" "                        TME_BUS_CYCLE_${capname});"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* if this TLB entry allows for fast ${name}s: */"
	    printf "%s\n" "    mem = tlb->tme_m68k_tlb_emulator_off_${name};"
	    printf "%s\n" "    if (__tme_predict_true(mem != TME_EMULATOR_OFF_UNDEF)) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* make the emulator memory pointer: */"
	    printf "%s\n" "      mem += linear_address;"
	    printf "%s\n" ""
	    printf "%s\n" "      /* limit the cycle size to addresses covered by the TLB entry: */"
	    printf "%s\n" "      if (__tme_predict_false((cycle_size - 1)"
	    printf "%s\n" "                              > (((tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last) - linear_address))) {"
	    printf "%s\n" "        cycle_size = (((tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last) - linear_address) + 1;"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* if this is a little-endian host, and this isn't a raw ${name}: */"
	    printf "%s\n" "      if (TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE"
	    printf "%s\n" "          && (flags & TME_M68K_BUS_CYCLE_RAW) == 0) {"
	    if test ${name} = write; then
		printf "%s\n" ""
		printf "%s\n" "        /* byteswap the data to write in the intermediate buffer: */"
		printf "%s\n" "        reg_p = cycle.tme_bus_cycle_buffer;"
		printf "%s\n" "        buffer_i = 0;"
		printf "%s\n" "        do {"
		printf "%s\n" "          reg_buffer[buffer_i] = *(reg_p--);"
		printf "%s\n" "        } while (++buffer_i != cycle_size);"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "        /* use the intermediate buffer for the ${name}: */"
	    printf "%s\n" "        cycle.tme_bus_cycle_buffer = &reg_buffer[0];"
	    printf "%s\n" "      }"
	    printf "%s\n" ""
	    printf "%s\n" "      /* do the bus ${name}: */"
	    printf "%s\n" "      tme_memory_bus_${name}_buffer(mem,"
	    printf "%s\n" "                                 cycle.tme_bus_cycle_buffer,"
	    printf "%s\n" "                                 cycle_size,"
	    printf "%s\n" "                                 tlb->tme_m68k_tlb_bus_rwlock,"
	    printf "%s\n" "                                 sizeof(tme_uint8_t),"
	    printf "%s\n" "                                 sizeof(tme_uint32_t));"
	    if test ${name} = read; then
		printf "%s\n" ""
		printf "%s\n" "      /* if this is a little-endian host, and this isn't a raw ${name}: */"
		printf "%s\n" "      if (TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE"
		printf "%s\n" "          && (flags & TME_M68K_BUS_CYCLE_RAW) == 0) {"
		printf "%s\n" ""
		printf "%s\n" "        /* byteswap the read data in the intermediate buffer: */"
		printf "%s\n" "        reg_p = reg + reg_size - (1 + transferred);"
		printf "%s\n" "        buffer_i = 0;"
		printf "%s\n" "        do {"
		printf "%s\n" "          *(reg_p--) = reg_buffer[buffer_i];"
		printf "%s\n" "        } while (++buffer_i != cycle_size);"
		printf "%s\n" "      }"
	    fi
	    printf "%s\n" ""
	    printf "%s\n" "      /* update: */"
	    printf "%s\n" "      linear_address += cycle_size;"
	    printf "%s\n" "      transferred += cycle_size;"
	    printf "%s\n" "      continue;"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* otherwise, this TLB entry does not allow for fast ${name}s: */"
	    printf "%s\n" ""
	    printf "%s\n" "    /* if this is a part of a read/modify/write cycle: */"
	    printf "%s\n" "    if (flags & TME_M68K_BUS_CYCLE_RMW) {"
	    printf "%s\n" ""
	    if test ${name} = read; then
		printf "%s\n" "      /* if this is the first cycle in this read,"
		printf "%s\n" "         we will establish the new lock, otherwise"
		printf "%s\n" "         we will continue using the existing lock: */"
	    else
		printf "%s\n" "      /* we will continue using the existing lock."
		printf "%s\n" "         the device will automatically unlock after"
		printf "%s\n" "         the last cycle of this write: */"
	    fi
	    printf "%s\n" "      cycle.tme_bus_cycle_type"
	    printf "%s\n" "        |= (TME_BUS_CYCLE_LOCK"
	    printf %s "            | ("
	    if test ${name} = read; then
		printf %s "transferred == 0 ? 0 : "
	    fi
	    printf "%s\n" "TME_BUS_CYCLE_UNLOCK));"
	    printf "%s\n" "    }"

	    printf "%s\n" ""
	    printf "%s\n" "    /* form the physical address for the bus cycle handler: */"
	    printf "%s\n" "    physical_address = tlb->tme_m68k_tlb_addr_offset + linear_address;"
	    printf "%s\n" "    shift = tlb->tme_m68k_tlb_addr_shift;"
	    printf "%s\n" "    if (shift < 0) {"
	    printf "%s\n" "      physical_address <<= (0 - shift);"
	    printf "%s\n" "    }"
	    printf "%s\n" "    else if (shift > 0) {"
	    printf "%s\n" "      physical_address >>= shift;"
	    printf "%s\n" "    }"
	    printf "%s\n" "    cycle.tme_bus_cycle_address = physical_address;"

	    printf "%s\n" ""
	    printf "%s\n" "    /* run the bus cycle: */"
	    printf "%s\n" "    tme_m68k_tlb_unbusy(tlb);"
	    printf "%s\n" "    tme_m68k_callout_unlock(ic);"
	    printf "%s\n" "    err = (*tlb->tme_m68k_tlb_bus_tlb.tme_bus_tlb_cycle)"
	    printf "%s\n" "         (tlb->tme_m68k_tlb_bus_tlb.tme_bus_tlb_cycle_private, &cycle);"
	    printf "%s\n" "    tme_m68k_callout_relock(ic);"
	    printf "%s\n" "    tme_m68k_tlb_busy(tlb);"
	    printf "%s\n" ""
	    printf "%s\n" "    /* if the TLB entry was invalidated before the ${name}: */"
	    printf "%s\n" "    if (err == EBADF"
	    printf "%s\n" "        && tme_m68k_tlb_is_invalid(tlb)) {"
	    printf "%s\n" "      cycle.tme_bus_cycle_size = 0;"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* otherwise, if we didn't get a bus error, but some"
	    printf "%s\n" "       synchronous event has happened: */"
	    printf "%s\n" "    else if (err == TME_BUS_CYCLE_SYNCHRONOUS_EVENT) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* after the currently executing instruction finishes, check"
	    printf "%s\n" "         for external resets, halts, or interrupts: */"
	    printf "%s\n" "      ic->_tme_m68k_instruction_burst_remaining = 0;"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* otherwise, any other error might be a bus error: */"
	    printf "%s\n" "    else if (err != TME_OK) {"
	    printf "%s\n" "      err = tme_bus_tlb_fault(&tlb->tme_m68k_tlb_bus_tlb, &cycle, err);"
	    printf "%s\n" "      if (err != TME_OK) {"
	    printf "%s\n" "        exception = TME_M68K_EXCEPTION_BERR;"
	    printf "%s\n" "        break;"
	    printf "%s\n" "      }"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* update: */"
	    printf "%s\n" "    linear_address += cycle.tme_bus_cycle_size;"
	    printf "%s\n" "    transferred += cycle.tme_bus_cycle_size;"
	    printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* NB: there is no need to explicitly unlock"
	    printf "%s\n" "     a device.  if a locked bus cycle to a device"
	    printf "%s\n" "     faults, the lock must be automatically unlocked: */"

	    printf "%s\n" ""
	    printf "%s\n" "  /* if we faulted, stash the information the fault stacker"
	    printf "%s\n" "     will need and start exception processing: */"
	    printf "%s\n" "  if (exception != TME_M68K_EXCEPTION_NONE) {"
	    printf %s "    ic->_tme_m68k_group0_flags = flags"
	    if test $name = read; then
		printf %s " | TME_M68K_BUS_CYCLE_READ"
	    fi
	    printf "%s\n" ";"
	    printf "%s\n" "    ic->_tme_m68k_group0_function_code = function_code;"
	    printf "%s\n" "    ic->_tme_m68k_group0_address = linear_address;"
	    printf "%s\n" "    ic->_tme_m68k_group0_sequence = ic->_tme_m68k_sequence;"
	    printf "%s\n" "    ic->_tme_m68k_group0_sequence._tme_m68k_sequence_transfer_faulted_after = transferred;"
	    printf "%s\n" "    ic->_tme_m68k_group0_buffer_${name}_size = cycle_size;"
	    if test $name = write; then
		printf "%s\n" "#ifdef WORDS_BIGENDIAN"
		printf "%s\n" "    memcpy(ic->_tme_m68k_group0_buffer_${name},"
		printf "%s\n" "           reg + transferred,"
		printf "%s\n" "           ic->_tme_m68k_group0_buffer_${name}_size);"
		printf "%s\n" "#else  /* !WORDS_BIGENDIAN */"
		printf "%s\n" "      reg_p = (reg + reg_size - 1) - transferred;"
		printf "%s\n" "      for (buffer_i = 0;"
		printf "%s\n" "           buffer_i < ic->_tme_m68k_group0_buffer_${name}_size;"
		printf "%s\n" "           buffer_i++) {"
		printf "%s\n" "        ic->_tme_m68k_group0_buffer_${name}[buffer_i] = *(reg_p--);"
		printf "%s\n" "      }"
		printf "%s\n" "#endif /* !WORDS_BIGENDIAN */"
	    fi
	    printf "%s\n" "    if (ic->_tme_m68k_group0_hook != NULL) {"
	    printf "%s\n" "      (*ic->_tme_m68k_group0_hook)(ic);"
	    printf "%s\n" "    }"
	    printf "%s\n" "    ic->_tme_m68k_group0_sequence._tme_m68k_sequence_transfer_faulted = ";
	    printf "%s\n" "      ic->_tme_m68k_group0_sequence._tme_m68k_sequence_transfer_next;"
	    printf "%s\n" "    tme_m68k_tlb_unbusy(tlb);"
	    printf "%s\n" "    tme_m68k_exception(ic, exception);"
	    printf "%s\n" "  }"

	    printf "%s\n" ""
	    printf "%s\n" "  /* otherwise, this transfer has now completed: */"
	    printf "%s\n" "  TME_M68K_SEQUENCE_TRANSFER_STEP;"

	    printf "%s\n" "}"
	fi
    done

done

# generate the BCD math functions:
for name in abcd sbcd nbcd; do

    # if we're making the header, just emit a declaration:
    if $header; then
	printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name});"
	continue
    fi

    # emit the function:
    printf "%s\n" ""
    printf "%s\n" "TME_M68K_INSN(tme_m68k_${name})"
    printf "%s\n" "{"
    printf "%s\n" "  tme_uint8_t dst, dst_msd, dst_lsd;"
    printf "%s\n" "  tme_uint8_t src, src_msd, src_lsd;"
    printf "%s\n" "  tme_uint8_t res, res_msd, res_lsd;"
    printf "%s\n" "  tme_uint8_t flags;"

    # get the operands:
    if test $name != nbcd; then
	printf "%s\n" "  int memory;"
	printf "%s\n" "  int rx, ry, function_code;"
	printf "%s\n" ""
	printf "%s\n" "  /* load the operands: */"
	printf "%s\n" "  rx = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);"
	printf "%s\n" "  ry = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 9, 3);"
	printf "%s\n" "  memory = (TME_M68K_INSN_OPCODE & TME_BIT(3)) != 0;"
	printf "%s\n" "  function_code = TME_M68K_FUNCTION_CODE_DATA(ic);"
	printf "%s\n" "  if (memory) {"
	printf "%s\n" "    TME_M68K_INSN_CANFAULT;"
	printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
	# the stack pointer must always be decremented by a multiple of two.
	# assuming rx < 8, ((rx + 1) >> 3) == 1 iff rx == 7, meaning %a7:
	printf "%s\n" "      ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + rx) -= sizeof(tme_uint8_t) + ((rx + 1) >> 3);"
	printf "%s\n" "      ic->_tme_m68k_ea_function_code = function_code;"
	printf "%s\n" "      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + rx);"
	printf "%s\n" "    }"
	printf "%s\n" "    tme_m68k_read_memx8(ic);"
	printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
	# the stack pointer must always be incremented by a multiple of two.
	# assuming rx < 8, ((rx + 1) >> 3) == 1 iff rx == 7, meaning %a7:
	printf "%s\n" "      ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ry) -= sizeof(tme_uint8_t) + ((ry + 1) >> 3);"
	printf "%s\n" "      ic->_tme_m68k_ea_function_code = function_code;"
	printf "%s\n" "      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ry);"
	printf "%s\n" "    }"
	printf "%s\n" "    tme_m68k_read_mem8(ic, TME_M68K_IREG_MEMY32);"
	printf "%s\n" "    src = ic->tme_m68k_ireg_memx8;"
	printf "%s\n" "    dst = ic->tme_m68k_ireg_memy8;"
	printf "%s\n" "  }"
	printf "%s\n" "  else {"
	printf "%s\n" "    src = ic->tme_m68k_ireg_uint8(rx << 2);"
	printf "%s\n" "    dst = ic->tme_m68k_ireg_uint8(ry << 2);"
	printf "%s\n" "  }"
    else
	printf "%s\n" ""
	printf "%s\n" "  dst = 0x00;"
	printf "%s\n" "  src = TME_M68K_INSN_OP1(tme_uint8_t);"
    fi
    printf "%s\n" "  dst_lsd = TME_FIELD_EXTRACTU(dst, 0, 4);"
    printf "%s\n" "  dst_msd = TME_FIELD_EXTRACTU(dst, 4, 4);"
    printf "%s\n" "  src_lsd = TME_FIELD_EXTRACTU(src, 0, 4);"
    printf "%s\n" "  src_msd = TME_FIELD_EXTRACTU(src, 4, 4);"

    # perform the operation:
    printf "%s\n" ""
    printf "%s\n" "  /* perform the operation: */"
    if test $name = abcd; then op='+' ; opc='-' ; else op='-' ; opc='+' ; fi
    printf "%s\n" "  res_lsd = dst_lsd ${op} src_lsd ${op} ((ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X) != 0);"
    printf "%s\n" "  res_msd = dst_msd ${op} src_msd;"
    printf "%s\n" "  flags = 0;"
    printf "%s\n" "  if (res_lsd > 9) {"
    printf "%s\n" "    res_lsd ${opc}= 10;"
    printf "%s\n" "    res_msd ${op}= 1;"
    printf "%s\n" "  }"
    printf "%s\n" "  if (res_msd > 9) {"
    printf "%s\n" "    res_msd ${opc}= 10;"
    printf "%s\n" "    flags |= TME_M68K_FLAG_C | TME_M68K_FLAG_X;"
    printf "%s\n" "  }"
    printf "%s\n" "  res = (res_msd << 4) + (res_lsd & 0xf);"
    printf "%s\n" "  if (res == 0) flags |= TME_M68K_FLAG_N;"
    printf "%s\n" ""

    # store the result
    printf "%s\n" "  /* store the result and set the flags: */"
    if test $name != nbcd; then
	printf "%s\n" "  if (memory) {"
	printf "%s\n" "    if (!TME_M68K_SEQUENCE_RESTARTING) {"
	printf "%s\n" "      ic->tme_m68k_ireg_memx8 = res;"
	printf "%s\n" "      ic->_tme_m68k_ea_function_code = function_code;"
	printf "%s\n" "      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ry);"
	printf "%s\n" "      ic->tme_m68k_ireg_ccr = flags;"
	printf "%s\n" "     }"
	printf "%s\n" "     tme_m68k_write_memx8(ic);"
	printf "%s\n" "  }"
	printf "%s\n" "  else {"
	printf "%s\n" "    ic->tme_m68k_ireg_uint8(ry << 2) = res;"
	printf "%s\n" "    ic->tme_m68k_ireg_ccr = flags;"
	printf "%s\n" "  }"
    else
	printf "%s\n" "  TME_M68K_INSN_OP1(tme_uint8_t) = res;"
	printf "%s\n" "  ic->tme_m68k_ireg_ccr = flags;"
    fi
    printf "%s\n" ""
    printf "%s\n" "  TME_M68K_INSN_OK;"
    printf "%s\n" "}"
done

# generate the ccr and sr functions:
for reg in ccr sr; do
    for name in ori andi eori move_to; do
	if test $reg = ccr; then size=8 ; else size=16 ; fi

	# if we're making the header, just emit a declaration:
	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_${name}_${reg});"
	    continue
	fi

	# emit the function:
	printf "%s\n" ""
	printf "%s\n" "TME_M68K_INSN(tme_m68k_${name}_${reg})"
	printf "%s\n" "{"
	printf "%s\n" "  tme_uint${size}_t reg;"

	# form the new register value:
	src=0
	printf %s "  reg = "
	case $name in
	ori) printf %s "ic->tme_m68k_ireg_${reg} | " ;;
	andi) printf %s "ic->tme_m68k_ireg_${reg} & " ;;
	eori) printf %s "ic->tme_m68k_ireg_${reg} ^ " ;;
	move_to) size=16 ; src=1 ;;
	esac
	printf "%s\n" "(TME_M68K_INSN_OP${src}(tme_uint${size}_t) & TME_M68K_FLAG_"`printf "%s\n" $reg | tr a-z A-Z`");"

	# sr changes are special:
	if test $reg = sr; then
	    printf "%s\n" "  TME_M68K_INSN_PRIV;"
	    printf "%s\n" "  TME_M68K_INSN_CHANGE_SR(reg);"
	else
	    printf "%s\n" "  ic->tme_m68k_ireg_${reg} = reg;"
	fi

	printf "%s\n" "  TME_M68K_INSN_OK;"
	printf "%s\n" "}"
    done
done

# generate the multiply and divide instructions:

# permute on signed vs. unsigned:
for _sign in u s; do
    if test $_sign = u; then sign=u; else sign=; fi

    # permute on short vs. long:
    for size in s l; do
	if test $size = s; then
	    _size=
	    small=16
	    large=32
	    reg_size_shift=' << 1'
	else
	    _size=l
	    small=32
	    large=64
	    reg_size_shift=
	fi

	# if we're making the header, just emit declarations:
	if $header; then
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_mul${_sign}${_size});"
	    printf "%s\n" "TME_M68K_INSN_DECL(tme_m68k_div${_sign}${_size});"
	    continue
	fi

	# emit the multiply function:
	printf "%s\n" ""
	printf "%s\n" "TME_M68K_INSN(tme_m68k_mul${_sign}${_size})"
	printf "%s\n" "{"
 	if test $large = 64; then
	    printf "%s\n" "#ifndef TME_HAVE_INT${large}_T"
	    printf "%s\n" "  abort();"
	    printf "%s\n" "#else /* TME_HAVE_INT${large}_T */"
	    printf "%s\n" "  unsigned int flag_v;"
	    printf "%s\n" "  int ireg_dh;"
	fi
	printf "%s\n" "  int ireg_dl;"
	printf "%s\n" "  tme_${sign}int${large}_t res;"
	printf "%s\n" "  tme_uint8_t flags;"

	printf "%s\n" ""
	printf "%s\n" "  /* get the register containing the factor: */"
	printf %s "  ireg_dl = TME_M68K_IREG_D0 + "
	if test $size = s; then
	    printf "%s\n" "TME_M68K_INSN_OP0(tme_uint32_t);"
	else
	    printf "%s\n" "TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 3);"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* perform the multiplication: */"
	printf "%s\n" "  res = (((tme_${sign}int${large}_t) ic->tme_m68k_ireg_${sign}int${small}(ireg_dl${reg_size_shift}))"
	printf "%s\n" "         * TME_M68K_INSN_OP1(tme_${sign}int${small}_t));"

	printf "%s\n" ""
	printf "%s\n" "  /* store the result: */"
	printf "%s\n" "  ic->tme_m68k_ireg_${sign}int32(ireg_dl) = (tme_${sign}int32_t) res;"
	if test $large = 64; then
	    printf "%s\n" "  flag_v = TME_M68K_FLAG_V;"
	    printf "%s\n" "  if (TME_M68K_INSN_SPECOP & TME_BIT(10)) {"
	    printf "%s\n" "    flag_v = 0;"
	    printf "%s\n" "    ireg_dh = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 3);"
	    printf "%s\n" "    ic->tme_m68k_ireg_${sign}int32(ireg_dh) = (tme_${sign}int32_t) (res >> 32);"
	    printf "%s\n" "  }"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* set the flags: */"
	printf "%s\n" "  flags = ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X;"
	printf "%s\n" "  if (((tme_int${large}_t) res) < 0) flags |= TME_M68K_FLAG_N;"
	printf "%s\n" "  if (res == 0) flags |= TME_M68K_FLAG_Z;"
	if test $large = 64; then
	    if test $_sign = s; then
		printf %s "  if (res > 0x7fffffffL || res < ((0L - 0x7fffffffL) - 1L)"
	    else
		printf %s "  if (res > 0xffffffffUL"
	    fi
	    printf "%s\n" ") flags |= flag_v;"
	fi
	printf "%s\n" "  ic->tme_m68k_ireg_ccr = flags;"

	printf "%s\n" ""
	printf "%s\n" "  TME_M68K_INSN_OK;"
 	if test $large = 64; then
	    printf "%s\n" "#endif /* TME_HAVE_INT${large}_T */"
	fi
	printf "%s\n" "}"

	# emit the divide function:
	printf "%s\n" ""
	printf "%s\n" "TME_M68K_INSN(tme_m68k_div${_sign}${_size})"
	printf "%s\n" "{"
 	if test $large = 64; then
	    printf "%s\n" "#ifndef TME_HAVE_INT${large}_T"
	    printf "%s\n" "  abort();"
	    printf "%s\n" "#else /* TME_HAVE_INT${large}_T */"
	    printf "%s\n" "  int ireg_dr;"
	fi
	printf "%s\n" "  int ireg_dq;"
	printf "%s\n" "  tme_${sign}int${large}_t dividend, quotient;"
	printf "%s\n" "  tme_${sign}int${small}_t divisor, remainder;"
	printf "%s\n" "  tme_uint8_t flags;"

	printf "%s\n" ""
	printf "%s\n" "  /* get the register(s): */"
	printf %s "  ireg_dq = TME_M68K_IREG_D0 + "
	if test $size = s; then
	    printf "%s\n" "TME_M68K_INSN_OP0(tme_uint32_t);"
	else
	    printf "%s\n" "TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 3);"
	    printf "%s\n" "  ireg_dr = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 3);"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* form the dividend and the divisor: */"
	if test $large = 64; then
	    printf "%s\n" "  if (TME_M68K_INSN_SPECOP & TME_BIT(10)) {"
	    printf "%s\n" "    dividend = (tme_${sign}int${large}_t)"
	    printf "%s\n" "               ((((tme_uint${large}_t) ic->tme_m68k_ireg_uint32(ireg_dr)) << 32)"
	    printf "%s\n" "                | ic->tme_m68k_ireg_uint32(ireg_dq));"
	    printf "%s\n" "  }"
	    printf "%s\n" "  else"
	    printf %s "  "
	fi
	printf "%s\n" "  dividend = (tme_${sign}int${large}_t) ic->tme_m68k_ireg_${sign}int32(ireg_dq);"
	printf "%s\n" "  divisor = TME_M68K_INSN_OP1(tme_${sign}int${small}_t);"
	printf "%s\n" "  if (divisor == 0) {"
	printf "%s\n" "    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;"
	printf "%s\n" "    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;"
	printf "%s\n" "    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_DIV0));"
	printf "%s\n" "  }"

	printf "%s\n" ""
	printf "%s\n" "  /* do the division: */"
	printf "%s\n" "  quotient = dividend / divisor;"
	printf "%s\n" "  remainder = dividend % divisor;"

	printf "%s\n" ""
	printf "%s\n" "  /* set the flags and return the quotient and remainder: */"
	printf "%s\n" "  flags = ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X;"
	printf %s "  if ("
	case "${small}${_sign}" in
	16s) printf %s "quotient > 0x7fff || quotient < -32768" ;;
	16u) printf %s "quotient > 0xffff" ;;
	32s) printf %s "quotient > 0x7fffffffL || quotient < ((0L - 0x7fffffffL) - 1L)" ;;
	32u) printf %s "quotient > 0xffffffffUL" ;;
	esac
	printf "%s\n" ") {"
	printf "%s\n" "    flags |= TME_M68K_FLAG_V;"
	printf "%s\n" "  }"
	printf "%s\n" "  else {"
	printf "%s\n" "    if (((tme_int${small}_t) quotient) < 0) flags |= TME_M68K_FLAG_N;"
	printf "%s\n" "    if (quotient == 0) flags |= TME_M68K_FLAG_Z;"
	printf "%s\n" "    ic->tme_m68k_ireg_${sign}int${small}(ireg_dq${reg_size_shift}) = (tme_${sign}int${small}_t) quotient;"
	if test $small = 16; then
	    printf "%s\n" "    ic->tme_m68k_ireg_${sign}int${small}((ireg_dq${reg_size_shift}) + 1) = remainder;"
	else
	    printf "%s\n" "    if (ireg_dr != ireg_dq) {"
	    printf "%s\n" "      ic->tme_m68k_ireg_${sign}int${small}(ireg_dr) = remainder;"
	    printf "%s\n" "    }"
	fi
	printf "%s\n" "  }"
	printf "%s\n" "  ic->tme_m68k_ireg_ccr = flags;"

	printf "%s\n" ""
	printf "%s\n" "  TME_M68K_INSN_OK;"
 	if test $large = 64; then
	    printf "%s\n" "#endif /* TME_HAVE_INT${large}_T */"
	fi
	printf "%s\n" "}"

    done
done

# done:
exit 0
