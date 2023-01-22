#! /bin/sh
# Generated from ../../../ic/sparc/sparc-fpu-auto.m4 by GNU Autoconf 2.69.
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


# $Id: sparc-fpu-auto.sh,v 1.6 2009/11/08 16:27:12 fredette Exp $

# ic/sparc-fpu-auto.sh - automatically generates C code for many SPARC FPU
# emulation instructions:

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
_TME_RCSID("\$Id: sparc-fpu-auto.sh,v 1.6 2009/11/08 16:27:12 fredette Exp $");
EOF

# the precision information helper script:
#
ieee754_precision_sh=`$as_echo $0 | sed -e "s%$PROG%../ieee754/ieee754-precision.sh%"`

# permute for the different precisions:
#
for precision in single double quad; do

    # get information about this precision:
    #
    eval `sh ${ieee754_precision_sh} ${precision}`

    dst_formats="(TME_FLOAT_FORMAT_IEEE754_${capprecision} | TME_FLOAT_FORMAT_IEEE754_${capprecision}_BUILTIN)"

    cat <<EOF

/* this sets the floating-point condition codes after a
   ${precision}-precision operation: */
static inline void
_tme_sparc_fpu_fcc_${precision}(struct tme_sparc *ic, const struct tme_float *dst, int trap_on_nan)
{
  tme_uint32_t fcc;
  unsigned int cc;

  /* set fcc: */
  fcc = (tme_float_is_nan(dst, ${dst_formats})
         ? TME_SPARC_FSR_FCC_UN
	 : tme_float_is_zero(dst, ${dst_formats})
	 ? TME_SPARC_FSR_FCC_EQ
	 : tme_float_is_negative(dst, ${dst_formats})
	 ? TME_SPARC_FSR_FCC_LT
	 : TME_SPARC_FSR_FCC_GT);

  /* if this is an FCMPE and this is a NaN, we always cause an invalid exception: */
  if (trap_on_nan && fcc == TME_SPARC_FSR_FCC_UN) {
    _tme_sparc_fpu_exception_ieee754(&ic->tme_sparc_fpu_ieee754_ctl, TME_FLOAT_EXCEPTION_INVALID);
  }

  /* set the floating-point condition codes: */
  if (TME_SPARC_VERSION(ic) >= 9) {
    cc = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0x3 << 25));
    if (cc != 0) {
      fcc = TME_FIELD_MASK_EXTRACTU(fcc, TME_SPARC_FSR_FCC);
      ic->tme_sparc_fpu_xfsr
	= ((ic->tme_sparc_fpu_xfsr
	    & ~ (tme_uint32_t) (0x3 << (2 * (cc - 1))))
	   | (fcc << (2 * (cc - 1))));
      return;
    }
  }
  ic->tme_sparc_fpu_fsr = (ic->tme_sparc_fpu_fsr & ~TME_SPARC_FSR_FCC) | fcc;
}

/* if the most significant bit of the NaN fraction is zero,
   this is a signaling NaN: */
#define _TME_SPARC_FPU_IS_SNAN_${capprecision}(a) (((*(a))${chunk_member_0} & ((${chunk_mask_0} | (${chunk_mask_0} >> 1)) ^ (${chunk_mask_0} >> 1))) != 0)
static tme_int8_t
_tme_sparc_fpu_is_snan_${precision}(${integral} *value)
{
  return (_TME_SPARC_FPU_IS_SNAN_${capprecision}(value));
}

/* ${precision}-precision NaN propagation: */
static void
_tme_sparc_fpu_nan_from_nans_${precision}(struct tme_ieee754_ctl *ctl,
				     const ${integral} *a,
				     const ${integral} *b,
				     ${integral} *z)
{
  struct tme_sparc *ic;
  int a_is_snan;
  int b_is_snan;

  /* recover our data structure : */
  ic = ctl->tme_ieee754_ctl_private;

  /* see if any of the NaNs are signaling NaNs: */
  a_is_snan = _TME_SPARC_FPU_IS_SNAN_${capprecision}(a);
  b_is_snan = _TME_SPARC_FPU_IS_SNAN_${capprecision}(b);

  /* if either operand is a signaling NaN: */
  if (a_is_snan || b_is_snan) {

    /* signal the signaling NaN: */
    _tme_sparc_fpu_exception_ieee754(ctl, TME_FLOAT_EXCEPTION_INVALID);
  }

  /* if and only if a (corresponding to f[rs1]) is a signaling NaN, do
     we return a. at all other times we return b (corresponding to f[rs2]): */
  if (a_is_snan) {
    b = a;
  }

  /* return the NaN, but make sure it's nonsignaling: */
  *z = *b;
  (*(z))${chunk_member_0} |= ((${chunk_mask_0} | (${chunk_mask_0} >> 1)) ^ (${chunk_mask_0} >> 1));
}

EOF

done

if $header; then :; else
    $as_echo "#define _TME_SPARC_FPU_UNIMPL tme_sparc_fpu_exception(ic, TME_SPARC_FSR_FTT_unimplemented_FPop)"
    $as_echo "#define _TME_SPARC_FPU_UNIMPL_IF(flags) do { if ((ic->tme_sparc_fpu_flags & (flags)) != 0) { _TME_SPARC_FPU_UNIMPL; } } while (/* CONSTCOND */ 0)"
fi
quad="_TME_SPARC_FPU_UNIMPL_IF(TME_SPARC_FPU_FLAG_NO_QUAD);"

# permute over fpop1/fpop2:
#
for fpop in fpop1 fpop2; do

    # placeholder for another permutation:
    #
    :

	# open the function:
	#
	$as_echo ""
	$as_echo "void"
	$as_echo "tme_sparc_fpu_${fpop}(struct tme_sparc *ic)"
	$as_echo "{"
	$as_echo "  tme_uint8_t rounding_mode;"
	$as_echo "  unsigned int opf;"
	$as_echo "  unsigned int fpreg_rd_number_encoded;"
	$as_echo "  unsigned int fpreg_rd_number;"
	$as_echo "  const struct tme_float *fpreg_rs1;"
	$as_echo "  const struct tme_float *fpreg_rs2;"
	if test ${fpop} = fpop1; then
	    $as_echo "  struct tme_float fpreg_rs1_buffer;"
	    $as_echo "  struct tme_float fpreg_rs2_buffer;"
	else
	    $as_echo "  unsigned int cc;"
	    $as_echo "  tme_uint32_t conds_mask;"
	    $as_echo "  unsigned int cc_i;"
	    $as_echo "  tme_uint32_t cond;"
	fi
	$as_echo "  struct tme_float fpreg_rd;"
	$as_echo "  unsigned int fpreg_rd_format;"

	$as_echo ""
	$as_echo "  /* set the rounding mode: */"
	$as_echo "  switch (ic->tme_sparc_fpu_fsr & TME_SPARC_FSR_RND) {"
	$as_echo "  default: assert(FALSE);"
	$as_echo "  case TME_SPARC_FSR_RND_RN: rounding_mode = TME_FLOAT_ROUND_NEAREST_EVEN; break;"
	$as_echo "  case TME_SPARC_FSR_RND_RZ: rounding_mode = TME_FLOAT_ROUND_TO_ZERO; break;"
	$as_echo "  case TME_SPARC_FSR_RND_RM: rounding_mode = TME_FLOAT_ROUND_DOWN; break;"
	$as_echo "  case TME_SPARC_FSR_RND_RP: rounding_mode = TME_FLOAT_ROUND_UP; break;"
	$as_echo "  }"
	$as_echo "  ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = rounding_mode;"

	$as_echo ""
	$as_echo "  /* decode the rd and opf fields: */"
	$as_echo "  fpreg_rd_number_encoded = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);"
	$as_echo "  opf = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0x1ff << 5));"

	$as_echo ""
	$as_echo "  /* silence uninitialized variable warnings: */"
	$as_echo "  fpreg_rd_number = 0;"

	$as_echo ""
	$as_echo "#ifdef _TME_SPARC_RECODE_VERIFY"
	$as_echo "  /* clear the rd buffer: */"
	$as_echo "  memset(&fpreg_rd, 0, sizeof(fpreg_rd));"
	$as_echo "#endif /* _TME_SPARC_RECODE_VERIFY */"

	fmovcc=
	if test ${fpop} = fpop2; then
	    fmovcc=cc
	    $as_echo ""
	    $as_echo "  /* if this is an FMOVcc: */"
	    $as_echo "  if (((opf - 1) & 0x3f) < 3) {"
	    $as_echo ""
	    $as_echo "    /* if opf bit eight is set, this uses integer condition codes: */"
	    $as_echo "    if (opf & TME_BIT(8)) {"
	    $as_echo ""
	    $as_echo "      /* if opf bit six is set, this is unimplemented: */"
	    $as_echo "      if (__tme_predict_false(opf & TME_BIT(6))) {"
	    $as_echo "        _TME_SPARC_FPU_UNIMPL;"
	    $as_echo "      }"
	    $as_echo ""
	    $as_echo "      /* get %icc or %xcc, depending on opf bit seven: */"
	    $as_echo "      cc = ic->tme_sparc64_ireg_ccr;"
	    $as_echo "      if (opf & TME_BIT(7)) {"
	    $as_echo "        cc /= (TME_SPARC64_CCR_XCC / TME_SPARC64_CCR_ICC);"
	    $as_echo "      }"
	    $as_echo "      cc = TME_FIELD_MASK_EXTRACTU(cc, TME_SPARC64_CCR_ICC);"
	    $as_echo ""
	    $as_echo "      /* get the conditions mask: */"
	    $as_echo "      conds_mask = _tme_sparc_conds_icc[cc];"
	    $as_echo "    }"
	    $as_echo ""
	    $as_echo "    /* otherwise, this uses floating-point condition codes: */"
	    $as_echo "    else {"
	    $as_echo ""
	    $as_echo "      /* get the right %fcc: */"
	    $as_echo "      cc_i = TME_FIELD_MASK_EXTRACTU(opf, (0x3 << 6));"
	    $as_echo "      if (cc_i == 0) {"
	    $as_echo "        cc = TME_FIELD_MASK_EXTRACTU(ic->tme_sparc_fpu_fsr, TME_SPARC_FSR_FCC);"
	    $as_echo "      }"
	    $as_echo "      else {"
	    $as_echo "        cc = (ic->tme_sparc_fpu_xfsr >> (2 * (cc_i - 1))) & 0x3;"
	    $as_echo "      }"
	    $as_echo ""
	    $as_echo "      /* get the conditions mask: */"
	    $as_echo "      conds_mask = _tme_sparc_conds_fcc[cc];"
	    $as_echo "    }"
	    $as_echo ""
	    $as_echo "    /* add the not-conditions to the conditions mask: */"
	    $as_echo "    conds_mask += ((~conds_mask) << 8);"
	    $as_echo ""
	    $as_echo "    /* get the cond field: */"
	    $as_echo "    cond = TME_BIT(TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0xf << 14)));"
  	    $as_echo ""
	    $as_echo "    /* if the condition is not true: */"
	    $as_echo "    if (!(conds_mask & cond)) {"
	    $as_echo ""
	    $as_echo "      /* return now: */"
	    $as_echo "      /* NB that this may expose us to guests, since we do not check"
	    $as_echo "         that the floating-point register numbers are valid: */"
	    $as_echo "      return;"
	    $as_echo "    }"
	    $as_echo ""
	    $as_echo "    /* clear bits six, seven, and eight in opf: */"
	    $as_echo "    opf &= 0x3f;"
	    $as_echo "  }"
	fi

	$as_echo ""
	$as_echo "  /* dispatch on the opf field: */"
	$as_echo "  switch (opf) {"
	$as_echo "#define _TME_SPARC_FPU_FORMAT_RS1(format) fpreg_rs1 = tme_sparc_fpu_fpreg_read(ic, TME_SPARC_FORMAT3_MASK_RS1, (format))"
	$as_echo "#define _TME_SPARC_FPU_FORMAT_RS2(format) fpreg_rs2 = tme_sparc_fpu_fpreg_read(ic, TME_SPARC_FORMAT3_MASK_RS2, (format))"
	$as_echo "#define _TME_SPARC_FPU_FORMAT_RD(format) do { fpreg_rd_format = (format) | TME_IEEE754_FPREG_FORMAT_BUILTIN; fpreg_rd_number = tme_sparc_fpu_fpreg_decode(ic, fpreg_rd_number_encoded, fpreg_rd_format); } while (/* CONSTCOND */ 0)"
	$as_echo ""

	# permute over the opf field:
	#
	opf_decimal=-1
	while test ${opf_decimal} != 511; do
	    opf_decimal=`expr \( ${opf_decimal} \) + 1`

	    # make the binary version of the opf field:
	    #
	    bits=9
	    opf=
	    opf_shifted=${opf_decimal}
	    while test ${bits} != 0; do
		bits=`expr ${bits} - 1`
		opf_shifted_next=`expr ${opf_shifted} / 2`
		opf_test=`expr ${opf_shifted_next} \* 2`
		if test ${opf_test} = ${opf_shifted}; then
		    opf="0${opf}"
		else
		    opf="1${opf}"
		fi
		opf_shifted=${opf_shifted_next}
	    done

	    # dispatch on the fpop/opf combination:
	    #
	    default=false
	    case "${fpop}:${opf}" in
	    fpop1:011000100)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FiTOs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_single_from_int32,"
		$as_echo "                              fpreg_rs2->tme_float_value_ieee754_single, &fpreg_rd);"
		;;
	    fpop1:011001000)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FiTOd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_from_int32,"
		$as_echo "                              fpreg_rs2->tme_float_value_ieee754_single, &fpreg_rd);"
		;;
	    fpop1:011001100)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FiTOq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_from_int32,"
		$as_echo "                              fpreg_rs2->tme_float_value_ieee754_single, &fpreg_rd);"
		;;
	    fpop1:010000100)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FxTOs: */"
		$as_echo "#ifdef TME_HAVE_INT64_T"
		$as_echo "    if (__tme_predict_true(TME_SPARC_VERSION(ic) >= 9)) {"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "      _TME_SPARC_FPU_BEGIN;"
		$as_echo "      _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_single_from_int64,"
		$as_echo "                                fpreg_rs2->tme_float_value_ieee754_double.tme_value64_int, &fpreg_rd);"
		$as_echo "      break;"
		$as_echo "    }"
		$as_echo "#endif /* TME_HAVE_INT64_T */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL;"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop1:010001000)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FxTOd: */"
		$as_echo "#ifdef TME_HAVE_INT64_T"
		$as_echo "    if (__tme_predict_true(TME_SPARC_VERSION(ic) >= 9)) {"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "      _TME_SPARC_FPU_BEGIN;"
		$as_echo "      _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_from_int64,"
		$as_echo "                                fpreg_rs2->tme_float_value_ieee754_double.tme_value64_int, &fpreg_rd);"
		$as_echo "      break;"
		$as_echo "    }"
		$as_echo "#endif /* TME_HAVE_INT64_T */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL;"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop1:010001100)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FxTOq: */"
		$as_echo "#ifdef TME_HAVE_INT64_T"
		$as_echo "    if (__tme_predict_true(TME_SPARC_VERSION(ic) >= 9)) {"
		$as_echo "      ${quad}"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "      _TME_SPARC_FPU_BEGIN;"
		$as_echo "      _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_from_int64,"
		$as_echo "                                fpreg_rs2->tme_float_value_ieee754_double.tme_value64_int, &fpreg_rd);"
		$as_echo "      break;"
		$as_echo "    }"
		$as_echo "#endif /* TME_HAVE_INT64_T */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL;"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop1:011010001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FsTOi: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd.tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
		$as_echo "    ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = TME_FLOAT_ROUND_TO_ZERO;"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_single_to_int32,"
		$as_echo "                              fpreg_rs2, (tme_int32_t *) &fpreg_rd.tme_float_value_ieee754_single);"
		;;
	    fpop1:011010010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FdTOi: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd.tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
		$as_echo "    ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = TME_FLOAT_ROUND_TO_ZERO;"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_to_int32,"
		$as_echo "                              fpreg_rs2, (tme_int32_t *) &fpreg_rd.tme_float_value_ieee754_single);"
		;;
	    fpop1:011010011)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FqTOi: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd.tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;"
		$as_echo "    ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = TME_FLOAT_ROUND_TO_ZERO;"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_to_int32,"
		$as_echo "                              fpreg_rs2, (tme_int32_t *) &fpreg_rd.tme_float_value_ieee754_single);"
		;;
	    fpop1:010000001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FsTOx: */"
		$as_echo "#ifdef TME_HAVE_INT64_T"
		$as_echo "    if (__tme_predict_true(TME_SPARC_VERSION(ic) >= 9)) {"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "      _TME_SPARC_FPU_BEGIN;"
		$as_echo "      fpreg_rd.tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		$as_echo "      ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = TME_FLOAT_ROUND_TO_ZERO;"
		$as_echo "      _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_single_to_int64,"
		$as_echo "                                fpreg_rs2, &fpreg_rd.tme_float_value_ieee754_double.tme_value64_int);"
		$as_echo "      break;"
		$as_echo "    }"
		$as_echo "#endif /* TME_HAVE_INT64_T */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL;"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop1:010000010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FdTOx: */"
		$as_echo "#ifdef TME_HAVE_INT64_T"
		$as_echo "    if (__tme_predict_true(TME_SPARC_VERSION(ic) >= 9)) {"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "      _TME_SPARC_FPU_BEGIN;"
		$as_echo "      fpreg_rd.tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		$as_echo "      ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = TME_FLOAT_ROUND_TO_ZERO;"
		$as_echo "      _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_to_int64,"
		$as_echo "                                fpreg_rs2, &fpreg_rd.tme_float_value_ieee754_double.tme_value64_int);"
		$as_echo "      break;"
		$as_echo "    }"
		$as_echo "#endif /* TME_HAVE_INT64_T */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL;"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop1:010000011)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FqTOx: */"
		$as_echo "#ifdef TME_HAVE_INT64_T"
		$as_echo "    if (__tme_predict_true(TME_SPARC_VERSION(ic) >= 9)) {"
		$as_echo "      ${quad}"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "      _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "      _TME_SPARC_FPU_BEGIN;"
		$as_echo "      fpreg_rd.tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;"
		$as_echo "      ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = TME_FLOAT_ROUND_TO_ZERO;"
		$as_echo "      _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_to_int64,"
		$as_echo "                                fpreg_rs2, &fpreg_rd.tme_float_value_ieee754_double.tme_value64_int);"
		$as_echo "      break;"
		$as_echo "    }"
		$as_echo "#endif /* TME_HAVE_INT64_T */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL;"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop1:011001001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FsTOd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_from_single,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:011001101)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FsTOq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_from_single,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:011000110)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FdTOs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_single_from_double,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:011001110)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FdTOq: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_from_double,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:011000111)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FqTOs: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_single_from_quad,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:011001011)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FqTOd: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_from_quad,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:000000001 | fpop2:000000001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FMOVs${fmovcc}: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd = *fpreg_rs2;"
		;;
	    fpop1:000000010 | fpop2:000000010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FMOVd${fmovcc}: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd = *fpreg_rs2;"
		;;
	    fpop1:000000101)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FNEGs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd = *fpreg_rs2;"
		$as_echo "    fpreg_rd.tme_float_value_ieee754_single ^= 0x80000000;"
		;;
	    fpop1:000000110)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FNEGd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd = *fpreg_rs2;"
		$as_echo "    fpreg_rd.tme_float_value_ieee754_double.tme_value64_uint32_hi ^= 0x80000000;"
		;;
	    fpop1:000001001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FABSs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd = *fpreg_rs2;"
		$as_echo "    fpreg_rd.tme_float_value_ieee754_single &= ~0x80000000;"
		;;
	    fpop1:000001010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FABSd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_BEGIN;"
		$as_echo "    fpreg_rd = *fpreg_rs2;"
		$as_echo "    fpreg_rd.tme_float_value_ieee754_double.tme_value64_uint32_hi &= ~0x80000000;"
		;;
	    fpop1:000101001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FSQRTs: */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL_IF(TME_SPARC_FPU_FLAG_NO_FSQRT);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_single_sqrt,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:000101010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FSQRTd: */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL_IF(TME_SPARC_FPU_FLAG_NO_FSQRT);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_sqrt,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:000101011)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FSQRTq: */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL_IF(TME_SPARC_FPU_FLAG_NO_FSQRT | TME_SPARC_FPU_FLAG_NO_QUAD);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_sqrt,"
		$as_echo "                              fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001000001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FADDs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_single_add,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001000010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FADDd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_double_add,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001000011)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FADDq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_quad_add,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001000101)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FSUBs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_single_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001000110)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FSUBd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_double_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001000111)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FSUBq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_quad_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001001001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FMULs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_single_mul,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001001010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FMULd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_double_mul,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001001011)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FMULq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_quad_mul,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001101001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FsMULd: */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL_IF(TME_SPARC_FPU_FLAG_NO_FMUL_WIDER);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_from_single,"
		$as_echo "                              fpreg_rs1, &fpreg_rs1_buffer);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_double_from_single,"
		$as_echo "                              fpreg_rs2, &fpreg_rs2_buffer);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_double_mul,"
		$as_echo "                             &fpreg_rs1_buffer, &fpreg_rs2_buffer, &fpreg_rd);"
		;;
	    fpop1:001101110)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FdMULq: */"
		$as_echo "    _TME_SPARC_FPU_UNIMPL_IF(TME_SPARC_FPU_FLAG_NO_FMUL_WIDER | TME_SPARC_FPU_FLAG_NO_QUAD);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_from_double,"
		$as_echo "                              fpreg_rs1, &fpreg_rs1_buffer);"
		$as_echo "    _TME_SPARC_FPU_OP_MONADIC(tme_ieee754_ops_quad_from_double,"
		$as_echo "                              fpreg_rs2, &fpreg_rs2_buffer);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_quad_mul,"
		$as_echo "                             &fpreg_rs1_buffer, &fpreg_rs2_buffer, &fpreg_rd);"
		;;
	    fpop1:001001101)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FDIVs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_SINGLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_single_div,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001001110)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FDIVd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_DOUBLE);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_double_div,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop1:001001111)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FDIVq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RD(TME_IEEE754_FPREG_FORMAT_QUAD);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_quad_div,"
		$as_echo "                              fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		;;
	    fpop2:001010001)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FCMPs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_single_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		$as_echo "    _tme_sparc_fpu_fcc_single(ic, &fpreg_rd, FALSE);"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop2:001010010)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FCMPd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_double_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		$as_echo "    _tme_sparc_fpu_fcc_double(ic, &fpreg_rd, FALSE);"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop2:001010011)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FCMPq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_quad_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		$as_echo "    _tme_sparc_fpu_fcc_quad(ic, &fpreg_rd, FALSE);"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop2:001010101)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FCMPEs: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_SINGLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_single_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		$as_echo "    _tme_sparc_fpu_fcc_single(ic, &fpreg_rd, TRUE);"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop2:001010110)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FCMPEd: */"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_DOUBLE | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_double_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		$as_echo "    _tme_sparc_fpu_fcc_double(ic, &fpreg_rd, TRUE);"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    fpop2:001010111)
		$as_echo "  case ${opf_decimal}:  /* ${opf} FCMPEq: */"
		$as_echo "    ${quad}"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS1(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_FORMAT_RS2(TME_IEEE754_FPREG_FORMAT_QUAD | TME_IEEE754_FPREG_FORMAT_BUILTIN);"
		$as_echo "    _TME_SPARC_FPU_OP_DYADIC(tme_ieee754_ops_quad_sub,"
		$as_echo "                             fpreg_rs1, fpreg_rs2, &fpreg_rd);"
		$as_echo "    _tme_sparc_fpu_fcc_quad(ic, &fpreg_rd, TRUE);"
		$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
		;;
	    *) default=true ;;
	    esac
	    if $default; then :; else $as_echo "    break;"; $as_echo ""; fi
	done
	$as_echo "  default:"
	$as_echo "    _TME_SPARC_FPU_UNIMPL;"
	$as_echo "    fpreg_rd_format = TME_IEEE754_FPREG_FORMAT_NULL;"
	$as_echo "    break;"
	$as_echo ""
	$as_echo "#undef _TME_SPARC_FPU_FORMAT_RS1"
	$as_echo "#undef _TME_SPARC_FPU_FORMAT_RS2"
	$as_echo "#undef _TME_SPARC_FPU_FORMAT_RD"
	$as_echo "  }"

	$as_echo ""
	$as_echo "  /* store any destination: */"
	$as_echo "  if (fpreg_rd_format != TME_IEEE754_FPREG_FORMAT_NULL) {"
	$as_echo "    tme_sparc_fpu_fpreg_format(ic, fpreg_rd_number, fpreg_rd_format);"
	$as_echo "    ic->tme_sparc_fpu_fpregs[fpreg_rd_number] = fpreg_rd;"
	$as_echo "    TME_SPARC_FPU_DIRTY(ic, fpreg_rd_number);"
	$as_echo "  }"

	$as_echo ""
	$as_echo "}"
   :
done

# done:
#
exit 0
