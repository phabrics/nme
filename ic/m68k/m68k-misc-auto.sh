#! /bin/sh
# Generated from ../../../ic/m68k/m68k-misc-auto.m4 by GNU Autoconf 2.69.
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
    $as_echo ""
    $as_echo "#ifndef _IC_M68K_MISC_H"
    $as_echo "#define _IC_M68K_MISC_H"
fi

# emit the register mapping macros:
if $header; then

    $as_echo ""
    $as_echo "/* the register mapping: */"
    $as_echo "#define TME_M68K_IREG_UNDEF		(-1)"
    ireg32_next=0

    # NB: these are in a deliberate order, matching the order of
    # registers in instruction encodings:
    for regtype in d a; do
	capregtype=`$as_echo ${regtype} | tr a-z A-Z`
	for regnum in 0 1 2 3 4 5 6 7; do
	    $as_echo "#define TME_M68K_IREG_${capregtype}${regnum}		(${ireg32_next})"
	    $as_echo "#define tme_m68k_ireg_${regtype}${regnum}		tme_m68k_ireg_uint32(TME_M68K_IREG_${capregtype}${regnum})"
	    ireg32_next=`expr ${ireg32_next} + 1`
	done
    done

    # the current, next, and last program counter:
    $as_echo "#define TME_M68K_IREG_PC		(${ireg32_next})"
    $as_echo "#define tme_m68k_ireg_pc		tme_m68k_ireg_uint32(TME_M68K_IREG_PC)"
    ireg32_next=`expr ${ireg32_next} + 1`
    $as_echo "#define TME_M68K_IREG_PC_NEXT		(${ireg32_next})"
    $as_echo "#define tme_m68k_ireg_pc_next		tme_m68k_ireg_uint32(TME_M68K_IREG_PC_NEXT)"
    ireg32_next=`expr ${ireg32_next} + 1`
    $as_echo "#define TME_M68K_IREG_PC_LAST		(${ireg32_next})"
    $as_echo "#define tme_m68k_ireg_pc_last		tme_m68k_ireg_uint32(TME_M68K_IREG_PC_LAST)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the status register and ccr:
    $as_echo "#define tme_m68k_ireg_sr		tme_m68k_ireg_uint16(${ireg32_next} << 1)"
    $as_echo "#define tme_m68k_ireg_ccr		tme_m68k_ireg_uint8(${ireg32_next} << 2)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the shadow status register and format/offset word:
    $as_echo "#define TME_M68K_IREG_SHADOW_SR	(${ireg32_next} << 1)"
    $as_echo "#define tme_m68k_ireg_shadow_sr	tme_m68k_ireg_uint16(TME_M68K_IREG_SHADOW_SR)"
    $as_echo "#define TME_M68K_IREG_FORMAT_OFFSET	((${ireg32_next} << 1) + 1)"
    $as_echo "#define tme_m68k_ireg_format_offset	tme_m68k_ireg_uint16(TME_M68K_IREG_FORMAT_OFFSET)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the memory buffers:
    for mem_which in x y z; do
	cap_mem_which=`$as_echo ${mem_which} | tr a-z A-Z`
	$as_echo "#define TME_M68K_IREG_MEM${cap_mem_which}32		(${ireg32_next})"
	$as_echo "#define tme_m68k_ireg_mem${mem_which}32		tme_m68k_ireg_uint32(TME_M68K_IREG_MEM${cap_mem_which}32)"
	$as_echo "#define TME_M68K_IREG_MEM${cap_mem_which}16		(${ireg32_next} << 1)"
	$as_echo "#define tme_m68k_ireg_mem${mem_which}16		tme_m68k_ireg_uint16(TME_M68K_IREG_MEM${cap_mem_which}16)"
	$as_echo "#define TME_M68K_IREG_MEM${cap_mem_which}8		(${ireg32_next} << 2)"
	$as_echo "#define tme_m68k_ireg_mem${mem_which}8		tme_m68k_ireg_uint8(TME_M68K_IREG_MEM${cap_mem_which}8)"
	ireg32_next=`expr ${ireg32_next} + 1`
    done

    # the control registers:
    for reg in usp isp msp sfc dfc vbr cacr caar; do
	capreg=`$as_echo $reg | tr a-z A-Z`
	$as_echo "#define TME_M68K_IREG_${capreg}		(${ireg32_next})"
	$as_echo "#define tme_m68k_ireg_${reg}		tme_m68k_ireg_uint32(TME_M68K_IREG_${capreg})"
	ireg32_next=`expr ${ireg32_next} + 1`
    done

    # this is the count of variable 32-bit registers:
    $as_echo "#define TME_M68K_IREG32_COUNT		(${ireg32_next})"

    # the immediate register.  there are actually three 32-bit
    # registers in a row, to allow the fpgen specop to fetch up to a
    # 96-bit immediate:
    $as_echo "#define TME_M68K_IREG_IMM32		(${ireg32_next})"
    $as_echo "#define tme_m68k_ireg_imm32		tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32)"
    ireg32_next=`expr ${ireg32_next} + 3`

    # the effective address register:
    $as_echo "#define TME_M68K_IREG_EA		(${ireg32_next})"
    $as_echo "#define tme_m68k_ireg_ea		tme_m68k_ireg_uint32(TME_M68K_IREG_EA)"
    ireg32_next=`expr ${ireg32_next} + 1`

    # the constant registers:
    for reg in zero one two three four five six seven eight; do
	capreg=`$as_echo $reg | tr a-z A-Z`
	$as_echo "#define TME_M68K_IREG_${capreg}		(${ireg32_next})"
	ireg32_next=`expr ${ireg32_next} + 1`
    done
fi

# emit the flags->conditions mapping.  note that the nesting of the
# flag variables is deliberate, to make this array indexable with the
# condition code register:
if $header; then :; else
    $as_echo ""
    $as_echo "/* the flags->conditions mapping: */"
    $as_echo "const tme_uint16_t _tme_m68k_conditions[32] = {"
    for xflag in 0 1; do
	for nflag in 0 1; do
	    for zflag in 0 1; do
		for vflag in 0 1; do
		    for cflag in 0 1; do

			# the True condition:
			$as_echo_n "TME_BIT(TME_M68K_C_T)"

			# the High condition:
			if test $cflag != 1 && test $zflag != 1; then
			    $as_echo_n " | TME_BIT(TME_M68K_C_HI)"
			fi

			# the Low or Same condition:
			if test $cflag = 1 || test $zflag = 1; then
			    $as_echo_n " | TME_BIT(TME_M68K_C_LS)"
			fi

			# the Carry Clear and Carry Set conditions:
			if test $cflag != 1; then
			    $as_echo_n " | TME_BIT(TME_M68K_C_CC)"
			else
			    $as_echo_n " | TME_BIT(TME_M68K_C_CS)"
			fi

			# the Not Equal and Equal conditions:
			if test $zflag != 1; then
			    $as_echo_n " | TME_BIT(TME_M68K_C_NE)"
			else
			    $as_echo_n " | TME_BIT(TME_M68K_C_EQ)"
			fi

			# the Overflow Clear and Overflow Set conditions:
			if test $vflag != 1; then
			    $as_echo_n " | TME_BIT(TME_M68K_C_VC)"
			else
			    $as_echo_n " | TME_BIT(TME_M68K_C_VS)"
			fi

			# the Plus and Minus conditions:
			if test $nflag != 1; then
			    $as_echo_n " | TME_BIT(TME_M68K_C_PL)"
			else
			    $as_echo_n " | TME_BIT(TME_M68K_C_MI)"
			fi

			# the Greater or Equal condition:
			if (test $nflag = 1 && test $vflag = 1) || \
			   (test $nflag != 1 && test $vflag != 1); then
			    $as_echo_n " | TME_BIT(TME_M68K_C_GE)"
			fi

			# the Less Than condition:
			if (test $nflag = 1 && test $vflag != 1) || \
			   (test $nflag != 1 && test $vflag = 1); then
			    $as_echo_n " | TME_BIT(TME_M68K_C_LT)"
			fi

			# the Greater Than condition:
			if (test $nflag = 1 && test $vflag = 1 && test $zflag != 1) || \
			   (test $nflag != 1 && test $vflag != 1 && test $zflag != 1); then
			    $as_echo_n " | TME_BIT(TME_M68K_C_GT)"
			fi

			# the Less Than or Equal condition:
			if test $zflag = 1 || \
			   (test $nflag = 1 && test $vflag != 1) || \
			   (test $nflag != 1 && test $vflag = 1); then
			    $as_echo_n " | TME_BIT(TME_M68K_C_LE)"
			fi

			$as_echo ","
		    done
		done
	    done
	done
    done
    $as_echo "};"
fi

# emit the instruction word fetch macros:
if $header; then

    # emit the simple signed and unsigned fetch macros:
    #
    $as_echo ""
    $as_echo "/* the simple signed and unsigned fetch macros: */"

    # permute over size:
    #
    for size in 16 32; do

	# permute for signed or unsigned:
	#
	for capsign in U S; do
	    if test $capsign = U; then sign=u ; un=un ; else sign= ; un= ; fi

	    $as_echo "#define _TME_M68K_EXECUTE_FETCH_${capsign}${size}(v) \\"
	    $as_echo "  _TME_M68K_EXECUTE_FETCH_${size}(tme_${sign}int${size}_t, v)"
	    if test ${size} = 16 && test ${capsign} = U; then
		$as_echo "#define _TME_M68K_EXECUTE_FETCH_${capsign}${size}_FIXED(v, field) \\"
		$as_echo "  _TME_M68K_EXECUTE_FETCH_${size}_FIXED(tme_${sign}int${size}_t, v, field)"
	    fi
	done
    done

    $as_echo ""
    $as_echo "#endif /* _IC_M68K_MISC_H */"

    # permute for the fast vs. slow executors:
    for executor in fast slow; do

	$as_echo ""
	$as_echo_n "#if"
	if test $executor = slow; then $as_echo_n "n"; fi
	$as_echo "def _TME_M68K_EXECUTE_FAST"
	$as_echo ""
	$as_echo "/* these macros are for the ${executor} executor: */"

	# permute over size:
	#
	for size in 16 32; do

	    $as_echo ""
	    $as_echo "/* this fetches a ${size}-bit value for the ${executor} executor: */"
	    $as_echo "#undef _TME_M68K_EXECUTE_FETCH_${size}"
	    $as_echo "#define _TME_M68K_EXECUTE_FETCH_${size}(type, v) \\"

	    if test $executor = slow; then

		# this expression gives the current fetch offset in the instruction:
		#
		offset_fetch="ic->_tme_m68k_insn_fetch_slow_next"

		$as_echo "  /* macros for the ${executor} executor are simple, because \\"
		$as_echo "     tme_m68k_fetch${size}() takes care of all endianness, alignment, \\"
		$as_echo "     and atomic issues, and also stores the fetched value in the \\"
		$as_echo "     instruction fetch buffer (if a previous fetch before a fault \\"
		$as_echo "     didn't store all or part of it there already): */ \\"
		$as_echo "  (v) = (type) tme_m68k_fetch${size}(ic, linear_pc); \\"
		$as_echo "  linear_pc += sizeof(tme_uint${size}_t)"

	    else

		# this expression gives the current fetch offset in the instruction:
		#
		offset_fetch="(fetch_fast_next - ic->_tme_m68k_insn_fetch_fast_start)"

		$as_echo "  /* use the raw fetch macro to fetch the value into the variable, \\"
		$as_echo "     and then save it in the instruction buffer.  the save doesn't \\"
		$as_echo "     need to be atomic; no one else can see the instruction buffer. \\"
		$as_echo "     however, the raw fetch macro has already advanced fetch_fast_next, \\"
		$as_echo "     so we need to compensate for that here: */ \\"
		$as_echo "  __TME_M68K_EXECUTE_FETCH_${size}(type, v); \\"
		$as_echo "  tme_memory_write${size}(((tme_uint${size}_t *) ((((tme_uint8_t *) &ic->_tme_m68k_insn_fetch_buffer[0]) - sizeof(tme_uint${size}_t)) + ${offset_fetch})), (tme_uint${size}_t) (v), sizeof(tme_uint16_t))"

		$as_echo ""
		$as_echo "/* this does a raw fetch of a ${size}-bit value for the ${executor} executor: */"
		$as_echo "#undef __TME_M68K_EXECUTE_FETCH_${size}"
		$as_echo "#define __TME_M68K_EXECUTE_FETCH_${size}(type, v) \\"
		$as_echo "  /* if we can't do the fast read, we need to redispatch: */ \\"
		$as_echo "  /* NB: checks in tme_m68k_go_slow(), and proper setting of \\"
		$as_echo "     ic->_tme_m68k_insn_fetch_fast_last in _TME_M68K_EXECUTE_NAME(), \\"
		$as_echo "     allow  us to do a simple pointer comparison here, for \\"
		$as_echo "     any fetch size: */ \\"
		$as_echo "  if (__tme_predict_false(fetch_fast_next > ic->_tme_m68k_insn_fetch_fast_last)) \\"
		$as_echo "    goto _tme_m68k_fast_fetch_failed; \\"
		$as_echo "  (v) = ((type) \\"
		$as_echo "         tme_betoh_u${size}(tme_memory_bus_read${size}((const tme_shared tme_uint${size}_t *) fetch_fast_next, \\"
		$as_echo "                                             tlb->tme_m68k_tlb_bus_rwlock, \\"
		$as_echo "                                             sizeof(tme_uint16_t), \\"
		$as_echo "                                             sizeof(tme_uint32_t)))); \\"
		$as_echo "  fetch_fast_next += sizeof(tme_uint${size}_t)"
	    fi

	    # if this size doesn't get a fixed fetch macro, continue now:
	    #
	    if test ${size} != 16; then
		continue
	    fi

	    $as_echo ""
	    $as_echo "/* this fetches a ${size}-bit value at a fixed instruction position"
	    $as_echo "   for the ${executor} executor: */"
	    $as_echo "#undef _TME_M68K_EXECUTE_FETCH_${size}_FIXED"
	    $as_echo "#define _TME_M68K_EXECUTE_FETCH_${size}_FIXED(type, v, field) \\"
	    $as_echo "  assert(&((struct tme_m68k *) 0)->field \\"
	    $as_echo "         == (type *) (((tme_uint8_t *) &((struct tme_m68k *) 0)->_tme_m68k_insn_fetch_buffer[0]) + ${offset_fetch})); \\"
	    $as_echo_n "  "
	    if test ${executor} = fast; then $as_echo_n _ ; fi
	    $as_echo "_TME_M68K_EXECUTE_FETCH_${size}(type, v)"

	done

	$as_echo ""
	$as_echo_n "#endif /* "
	if test $executor = slow; then $as_echo_n "!"; fi
	$as_echo "_TME_M68K_EXECUTE_FAST */"
    done
fi

# done:
exit 0
