#! /bin/sh
# Generated from m68k-misc-auto.m4 by GNU Autoconf 2.72.
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


# $Id: m68k-misc-auto.sh,v 1.11 2007/02/16 02:50:23 fredette Exp $

# ic/m68k/m68k-misc-auto.sh - automatically generates C code
# for miscellaneous m68k emulation support:

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
EOF

# we need our own inclusion protection, since the instruction word
# fetch macros need to be multiply included:
if $header; then
    printf "%s\n" ""
    printf "%s\n" "#ifndef _IC_M68K_MISC_H"
    printf "%s\n" "#define _IC_M68K_MISC_H"
fi

# emit the register mapping macros:
if $header; then

    printf "%s\n" ""
    printf "%s\n" "/* the register mapping: */"
    printf "%s\n" "#define TME_M68K_IREG_UNDEF		(-1)"
    ireg32_next=0

    # NB: these are in a deliberate order, matching the order of
    # registers in instruction encodings:
    for regtype in d a; do
	capregtype=`printf "%s\n" ${regtype} | tr a-z A-Z`
	for regnum in 0 1 2 3 4 5 6 7; do
	    printf "%s\n" "#define TME_M68K_IREG_${capregtype}${regnum}		(${ireg32_next})"
	    printf "%s\n" "#define tme_m68k_ireg_${regtype}${regnum}		tme_m68k_ireg_uint32(TME_M68K_IREG_${capregtype}${regnum})"
	    ireg32_next=`expr ${ireg32_next} + 1`
	done
    done

    # the current, next, and last program counter:
    printf "%s\n" "#define TME_M68K_IREG_PC		(${ireg32_next})"
    printf "%s\n" "#define tme_m68k_ireg_pc		tme_m68k_ireg_uint32(TME_M68K_IREG_PC)"
    ireg32_next=`expr ${ireg32_next} + 1`
    printf "%s\n" "#define TME_M68K_IREG_PC_NEXT		(${ireg32_next})"
    printf "%s\n" "#define tme_m68k_ireg_pc_next		tme_m68k_ireg_uint32(TME_M68K_IREG_PC_NEXT)"
    ireg32_next=`expr ${ireg32_next} + 1`
    printf "%s\n" "#define TME_M68K_IREG_PC_LAST		(${ireg32_next})"
    printf "%s\n" "#define tme_m68k_ireg_pc_last		tme_m68k_ireg_uint32(TME_M68K_IREG_PC_LAST)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the status register and ccr:
    printf "%s\n" "#define tme_m68k_ireg_sr		tme_m68k_ireg_uint16(${ireg32_next} << 1)"
    printf "%s\n" "#define tme_m68k_ireg_ccr		tme_m68k_ireg_uint8(${ireg32_next} << 2)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the shadow status register and format/offset word:
    printf "%s\n" "#define TME_M68K_IREG_SHADOW_SR	(${ireg32_next} << 1)"
    printf "%s\n" "#define tme_m68k_ireg_shadow_sr	tme_m68k_ireg_uint16(TME_M68K_IREG_SHADOW_SR)"
    printf "%s\n" "#define TME_M68K_IREG_FORMAT_OFFSET	((${ireg32_next} << 1) + 1)"
    printf "%s\n" "#define tme_m68k_ireg_format_offset	tme_m68k_ireg_uint16(TME_M68K_IREG_FORMAT_OFFSET)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the memory buffers:
    for mem_which in x y z; do
	cap_mem_which=`printf "%s\n" ${mem_which} | tr a-z A-Z`
	printf "%s\n" "#define TME_M68K_IREG_MEM${cap_mem_which}32		(${ireg32_next})"
	printf "%s\n" "#define tme_m68k_ireg_mem${mem_which}32		tme_m68k_ireg_uint32(TME_M68K_IREG_MEM${cap_mem_which}32)"
	printf "%s\n" "#define TME_M68K_IREG_MEM${cap_mem_which}16		(${ireg32_next} << 1)"
	printf "%s\n" "#define tme_m68k_ireg_mem${mem_which}16		tme_m68k_ireg_uint16(TME_M68K_IREG_MEM${cap_mem_which}16)"
	printf "%s\n" "#define TME_M68K_IREG_MEM${cap_mem_which}8		(${ireg32_next} << 2)"
	printf "%s\n" "#define tme_m68k_ireg_mem${mem_which}8		tme_m68k_ireg_uint8(TME_M68K_IREG_MEM${cap_mem_which}8)"
	ireg32_next=`expr ${ireg32_next} + 1`
    done

    # the control registers:
    for reg in usp isp msp sfc dfc vbr cacr caar; do
	capreg=`printf "%s\n" $reg | tr a-z A-Z`
	printf "%s\n" "#define TME_M68K_IREG_${capreg}		(${ireg32_next})"
	printf "%s\n" "#define tme_m68k_ireg_${reg}		tme_m68k_ireg_uint32(TME_M68K_IREG_${capreg})"
	ireg32_next=`expr ${ireg32_next} + 1`
    done

    # this is the count of variable 32-bit registers:
    printf "%s\n" "#define TME_M68K_IREG32_COUNT		(${ireg32_next})"

    # the immediate register.  there are actually three 32-bit
    # registers in a row, to allow the fpgen specop to fetch up to a
    # 96-bit immediate:
    printf "%s\n" "#define TME_M68K_IREG_IMM32		(${ireg32_next})"
    printf "%s\n" "#define tme_m68k_ireg_imm32		tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32)"
    ireg32_next=`expr ${ireg32_next} + 3`

    # the effective address register:
    printf "%s\n" "#define TME_M68K_IREG_EA		(${ireg32_next})"
    printf "%s\n" "#define tme_m68k_ireg_ea		tme_m68k_ireg_uint32(TME_M68K_IREG_EA)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the constant registers:
    for reg in zero one two three four five six seven eight; do
	capreg=`printf "%s\n" $reg | tr a-z A-Z`
	printf "%s\n" "#define TME_M68K_IREG_${capreg}		(${ireg32_next})"
	ireg32_next=`expr ${ireg32_next} + 1`
    done
fi

# emit the flags->conditions mapping.  note that the nesting of the
# flag variables is deliberate, to make this array indexable with the
# condition code register:
if $header; then :; else
    printf "%s\n" ""
    printf "%s\n" "/* the flags->conditions mapping: */"
    printf "%s\n" "const tme_uint16_t _tme_m68k_conditions[32] = {"
    for xflag in 0 1; do
	for nflag in 0 1; do
	    for zflag in 0 1; do
		for vflag in 0 1; do
		    for cflag in 0 1; do

			# the True condition:
			printf %s "TME_BIT(TME_M68K_C_T)"

			# the High condition:
			if test $cflag != 1 && test $zflag != 1; then
			    printf %s " | TME_BIT(TME_M68K_C_HI)"
			fi

			# the Low or Same condition:
			if test $cflag = 1 || test $zflag = 1; then
			    printf %s " | TME_BIT(TME_M68K_C_LS)"
			fi

			# the Carry Clear and Carry Set conditions:
			if test $cflag != 1; then
			    printf %s " | TME_BIT(TME_M68K_C_CC)"
			else
			    printf %s " | TME_BIT(TME_M68K_C_CS)"
			fi

			# the Not Equal and Equal conditions:
			if test $zflag != 1; then
			    printf %s " | TME_BIT(TME_M68K_C_NE)"
			else
			    printf %s " | TME_BIT(TME_M68K_C_EQ)"
			fi

			# the Overflow Clear and Overflow Set conditions:
			if test $vflag != 1; then
			    printf %s " | TME_BIT(TME_M68K_C_VC)"
			else
			    printf %s " | TME_BIT(TME_M68K_C_VS)"
			fi

			# the Plus and Minus conditions:
			if test $nflag != 1; then
			    printf %s " | TME_BIT(TME_M68K_C_PL)"
			else
			    printf %s " | TME_BIT(TME_M68K_C_MI)"
			fi

			# the Greater or Equal condition:
			if (test $nflag = 1 && test $vflag = 1) || \
			   (test $nflag != 1 && test $vflag != 1); then
			    printf %s " | TME_BIT(TME_M68K_C_GE)"
			fi

			# the Less Than condition:
			if (test $nflag = 1 && test $vflag != 1) || \
			   (test $nflag != 1 && test $vflag = 1); then
			    printf %s " | TME_BIT(TME_M68K_C_LT)"
			fi

			# the Greater Than condition:
			if (test $nflag = 1 && test $vflag = 1 && test $zflag != 1) || \
			   (test $nflag != 1 && test $vflag != 1 && test $zflag != 1); then
			    printf %s " | TME_BIT(TME_M68K_C_GT)"
			fi

			# the Less Than or Equal condition:
			if test $zflag = 1 || \
			   (test $nflag = 1 && test $vflag != 1) || \
			   (test $nflag != 1 && test $vflag = 1); then
			    printf %s " | TME_BIT(TME_M68K_C_LE)"
			fi

			printf "%s\n" ","
		    done
		done
	    done
	done
    done
    printf "%s\n" "};"
fi

# emit the instruction word fetch macros:
if $header; then

    # emit the simple signed and unsigned fetch macros:
    #
    printf "%s\n" ""
    printf "%s\n" "/* the simple signed and unsigned fetch macros: */"

    # permute over size:
    #
    for size in 16 32; do

	# permute for signed or unsigned:
	#
	for capsign in U S; do
	    if test $capsign = U; then sign=u ; un=un ; else sign= ; un= ; fi

	    printf "%s\n" "#define _TME_M68K_EXECUTE_FETCH_${capsign}${size}(v) \\"
	    printf "%s\n" "  _TME_M68K_EXECUTE_FETCH_${size}(tme_${sign}int${size}_t, v)"
	    if test ${size} = 16 && test ${capsign} = U; then
		printf "%s\n" "#define _TME_M68K_EXECUTE_FETCH_${capsign}${size}_FIXED(v, field) \\"
		printf "%s\n" "  _TME_M68K_EXECUTE_FETCH_${size}_FIXED(tme_${sign}int${size}_t, v, field)"
	    fi
	done
    done

    printf "%s\n" ""
    printf "%s\n" "#endif /* _IC_M68K_MISC_H */"

    # permute for the fast vs. slow executors:
    for executor in fast slow; do

	printf "%s\n" ""
	printf %s "#if"
	if test $executor = slow; then printf %s "n"; fi
	printf "%s\n" "def _TME_M68K_EXECUTE_FAST"
	printf "%s\n" ""
	printf "%s\n" "/* these macros are for the ${executor} executor: */"

	# permute over size:
	#
	for size in 16 32; do

	    printf "%s\n" ""
	    printf "%s\n" "/* this fetches a ${size}-bit value for the ${executor} executor: */"
	    printf "%s\n" "#undef _TME_M68K_EXECUTE_FETCH_${size}"
	    printf "%s\n" "#define _TME_M68K_EXECUTE_FETCH_${size}(type, v) \\"

	    if test $executor = slow; then

		# this expression gives the current fetch offset in the instruction:
		#
		offset_fetch="ic->_tme_m68k_insn_fetch_slow_next"

		printf "%s\n" "  /* macros for the ${executor} executor are simple, because \\"
		printf "%s\n" "     tme_m68k_fetch${size}() takes care of all endianness, alignment, \\"
		printf "%s\n" "     and atomic issues, and also stores the fetched value in the \\"
		printf "%s\n" "     instruction fetch buffer (if a previous fetch before a fault \\"
		printf "%s\n" "     didn't store all or part of it there already): */ \\"
		printf "%s\n" "  (v) = (type) tme_m68k_fetch${size}(ic, linear_pc); \\"
		printf "%s\n" "  linear_pc += sizeof(tme_uint${size}_t)"

	    else

		# this expression gives the current fetch offset in the instruction:
		#
		offset_fetch="(fetch_fast_next - ic->_tme_m68k_insn_fetch_fast_start)"

		printf "%s\n" "  /* use the raw fetch macro to fetch the value into the variable, \\"
		printf "%s\n" "     and then save it in the instruction buffer.  the save doesn't \\"
		printf "%s\n" "     need to be atomic; no one else can see the instruction buffer. \\"
		printf "%s\n" "     however, the raw fetch macro has already advanced fetch_fast_next, \\"
		printf "%s\n" "     so we need to compensate for that here: */ \\"
		printf "%s\n" "  __TME_M68K_EXECUTE_FETCH_${size}(type, v); \\"
		printf "%s\n" "  tme_memory_write${size}(((tme_uint${size}_t *) ((((tme_uint8_t *) &ic->_tme_m68k_insn_fetch_buffer[0]) - sizeof(tme_uint${size}_t)) + ${offset_fetch})), (tme_uint${size}_t) (v), sizeof(tme_uint16_t))"

		printf "%s\n" ""
		printf "%s\n" "/* this does a raw fetch of a ${size}-bit value for the ${executor} executor: */"
		printf "%s\n" "#undef __TME_M68K_EXECUTE_FETCH_${size}"
		printf "%s\n" "#define __TME_M68K_EXECUTE_FETCH_${size}(type, v) \\"
		printf "%s\n" "  /* if we can't do the fast read, we need to redispatch: */ \\"
		printf "%s\n" "  /* NB: checks in tme_m68k_go_slow(), and proper setting of \\"
		printf "%s\n" "     ic->_tme_m68k_insn_fetch_fast_last in _TME_M68K_EXECUTE_NAME(), \\"
		printf "%s\n" "     allow  us to do a simple pointer comparison here, for \\"
		printf "%s\n" "     any fetch size: */ \\"
		printf "%s\n" "  if (__tme_predict_false(fetch_fast_next > ic->_tme_m68k_insn_fetch_fast_last)) \\"
		printf "%s\n" "    goto _tme_m68k_fast_fetch_failed; \\"
		printf "%s\n" "  (v) = ((type) \\"
		printf "%s\n" "         tme_betoh_u${size}(tme_memory_bus_read${size}((const tme_shared tme_uint${size}_t *) fetch_fast_next, \\"
		printf "%s\n" "                                             tlb->tme_m68k_tlb_bus_rwlock, \\"
		printf "%s\n" "                                             sizeof(tme_uint16_t), \\"
		printf "%s\n" "                                             sizeof(tme_uint32_t)))); \\"
		printf "%s\n" "  fetch_fast_next += sizeof(tme_uint${size}_t)"
	    fi

	    # if this size doesn't get a fixed fetch macro, continue now:
	    #
	    if test ${size} != 16; then
		continue
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "/* this fetches a ${size}-bit value at a fixed instruction position"
	    printf "%s\n" "   for the ${executor} executor: */"
	    printf "%s\n" "#undef _TME_M68K_EXECUTE_FETCH_${size}_FIXED"
	    printf "%s\n" "#define _TME_M68K_EXECUTE_FETCH_${size}_FIXED(type, v, field) \\"
	    printf "%s\n" "  assert(&((struct tme_m68k *) 0)->field \\"
	    printf "%s\n" "         == (type *) (((tme_uint8_t *) &((struct tme_m68k *) 0)->_tme_m68k_insn_fetch_buffer[0]) + ${offset_fetch})); \\"
	    printf %s "  "
	    if test ${executor} = fast; then printf %s _ ; fi
	    printf "%s\n" "_TME_M68K_EXECUTE_FETCH_${size}(type, v)"

	done

	printf "%s\n" ""
	printf %s "#endif /* "
	if test $executor = slow; then printf %s "!"; fi
	printf "%s\n" "_TME_M68K_EXECUTE_FAST */"
    done
fi

# done:
exit 0
