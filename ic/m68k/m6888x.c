/* $Id: m6888x.c,v 1.4 2007/08/25 20:37:30 fredette Exp $ */

/* ic/m68k/m6888x.c - m68k floating-point implementation */

/*
 * Copyright (c) 2004 Matt Fredette
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Matt Fredette.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tme/common.h>
_TME_RCSID("$Id: m6888x.c,v 1.4 2007/08/25 20:37:30 fredette Exp $");

/* includes: */
#include "m68k-impl.h"

/* macros: */

/* m6888x FPCR bits: */
#define TME_M6888X_FPCR_RND_MASK	(0x00000030)
#define  TME_M6888X_FPCR_RND_RN		 (0x00000000)
#define  TME_M6888X_FPCR_RND_RZ		 (0x00000010)
#define  TME_M6888X_FPCR_RND_RM		 (0x00000020)
#define  TME_M6888X_FPCR_RND_RP		 (0x00000030)
#define TME_M6888X_FPCR_PREC_MASK	(0x000000c0)
#define  TME_M6888X_FPCR_PREC_X		 (0x00000000)
#define  TME_M6888X_FPCR_PREC_S		 (0x00000040)
#define  TME_M6888X_FPCR_PREC_D		 (0x00000080)
#define  TME_M6888X_FPCR_PREC_UNDEF	 (0x000000c0)
#define TME_M6888X_FPCR_ENABLE_INEX1	TME_BIT(8)
#define TME_M6888X_FPCR_ENABLE_INEX2	TME_BIT(9)
#define TME_M6888X_FPCR_ENABLE_DZ	TME_BIT(10)
#define TME_M6888X_FPCR_ENABLE_UNFL	TME_BIT(11)
#define TME_M6888X_FPCR_ENABLE_OVFL	TME_BIT(12)
#define TME_M6888X_FPCR_ENABLE_OPERR	TME_BIT(13)
#define TME_M6888X_FPCR_ENABLE_SNAN	TME_BIT(14)
#define TME_M6888X_FPCR_ENABLE_BSUN	TME_BIT(15)

/* m6888x FPSR bits: */
#define TME_M6888X_FPSR_AEXC_INEX	TME_BIT(3)
#define TME_M6888X_FPSR_AEXC_DZ		TME_BIT(4)
#define TME_M6888X_FPSR_AEXC_UNFL	TME_BIT(5)
#define TME_M6888X_FPSR_AEXC_OVFL	TME_BIT(6)
#define TME_M6888X_FPSR_AEXC_IOP	TME_BIT(7)
#define TME_M6888X_FPSR_EXC_INEX1	TME_M6888X_FPCR_ENABLE_INEX1
#define TME_M6888X_FPSR_EXC_INEX2	TME_M6888X_FPCR_ENABLE_INEX2
#define TME_M6888X_FPSR_EXC_DZ		TME_M6888X_FPCR_ENABLE_DZ
#define TME_M6888X_FPSR_EXC_UNFL	TME_M6888X_FPCR_ENABLE_UNFL
#define TME_M6888X_FPSR_EXC_OVFL	TME_M6888X_FPCR_ENABLE_OVFL
#define TME_M6888X_FPSR_EXC_OPERR	TME_M6888X_FPCR_ENABLE_OPERR
#define TME_M6888X_FPSR_EXC_SNAN	TME_M6888X_FPCR_ENABLE_SNAN
#define TME_M6888X_FPSR_EXC_BSUN	TME_M6888X_FPCR_ENABLE_BSUN
#define TME_M6888X_FPSR_QUOTIENT	(0x00ff0000)
#define TME_M6888X_FPSR_CC_NAN		TME_BIT(24)
#define TME_M6888X_FPSR_CC_I		TME_BIT(25)
#define TME_M6888X_FPSR_CC_Z		TME_BIT(26)
#define TME_M6888X_FPSR_CC_N		TME_BIT(27)

/* m6888x exceptions: */
#define TME_M6888X_VECTOR_BSUN		(0x30)
#define TME_M6888X_VECTOR_INEX		(0x31)
#define TME_M6888X_VECTOR_DZ		(0x32)
#define TME_M6888X_VECTOR_UNFL		(0x33)
#define TME_M6888X_VECTOR_OPERR		(0x34)
#define TME_M6888X_VECTOR_OVFL		(0x35)
#define TME_M6888X_VECTOR_SNAN		(0x36)

/* m6888x frame versions: */
#define TME_M6888X_FRAME_VERSION_NULL		(0x00)
#define TME_M6888X_FRAME_VERSION_IDLE_M68881	(0x1f)
#define TME_M6888X_FRAME_VERSION_IDLE_M68882	(0x21)
#define TME_M6888X_FRAME_VERSION_IDLE_M68040	(0x23)

/* m6888x frame sizes: */
#define TME_M6888X_FRAME_SIZE_NULL		(0x00)
#define TME_M6888X_FRAME_SIZE_IDLE_M68881	(0x18)
#define TME_M6888X_FRAME_SIZE_IDLE_M68882	(0x38)
#define TME_M6888X_FRAME_SIZE_IDLE_M68040	(0x00)

/* bits in a packed decimal real: */
#define TME_M6888X_PACKEDDEC_SM		TME_BIT(31)
#define TME_M6888X_PACKEDDEC_SE		TME_BIT(30)
#define TME_M6888X_PACKEDDEC_YY		(TME_BIT(29) | TME_BIT(28))

/* rounding precisions: */
#define TME_M6888X_ROUNDING_PRECISION_CTL		(0)
#define TME_M6888X_ROUNDING_PRECISION_SINGLE		(32)
#define TME_M6888X_ROUNDING_PRECISION_DOUBLE		(64)
#define TME_M6888X_ROUNDING_PRECISION_EXTENDED80	(80)

/* operation types: */
#define TME_M6888X_OPTYPE_MONADIC		(0)
#define TME_M6888X_OPTYPE_DYADIC_SRC_DST	(1)
#define TME_M6888X_OPTYPE_DYADIC_DST_SRC	(2)

/* special opmodes: */
#define TME_M6888X_FPGEN_OPMODE_FCMP	(0x38)
#define TME_M6888X_FPGEN_OPMODE_FTST	(0x3a)
#define TME_M6888X_FPGEN_OPMODE_OTHER	(0xff)

/* this causes an exception if there is no FPU, or if it isn't enabled: */
#define TME_M68K_INSN_FPU				\
do {							\
  if (!ic->tme_m68k_fpu_enabled) {			\
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);	\
  }							\
} while (/* CONSTCOND */ 0)

/* these declare an m68k FPgen function: */
#define TME_M6888X_FPGEN_DECL(name)		\
  static void name _TME_P((struct tme_m68k *, const struct tme_float *, struct tme_float *))
#ifdef __STDC__
#define TME_M6888X_FPGEN(name)			\
  static void name(struct tme_m68k *ic, const struct tme_float *src, struct tme_float *dst)
#else  /* !__STDC__ */
#define TME_M6888X_FPGEN(name)			\
  static void name(ic, src, dst)		\
  struct tme_m68k *ic;				\
  const struct tme_float *src;	\
  struct tme_float *dst;
#endif /* !__STDC__ */

/* this gets the offset of a function in the IEEE 754 operations structure: */
#define TME_M6888X_IEEE754_OP(func)	((unsigned long) ((char *) &((struct tme_ieee754_ops *) 0)->func))

/* these invoke an IEEE 754 operation: */
#define _TME_M6888X_IEEE754_OP(func, x)			\
do {							\
  if ((func) == NULL) {					\
    if (ic->tme_m68k_fpu_incomplete_abort) {		\
      abort();						\
    }							\
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);	\
  }							\
  (*(func)) x;						\
} while (/* CONSTCOND */ 0)
#define TME_M6888X_IEEE754_OP_MONADIC(func, src, dst) \
  _TME_M6888X_IEEE754_OP(ic->tme_m68k_fpu_ieee754_ops->func, (&ic->tme_m68k_fpu_ieee754_ctl, src, dst))
#define TME_M6888X_IEEE754_OP_DYADIC(func, src0, src1, dst) \
  _TME_M6888X_IEEE754_OP(ic->tme_m68k_fpu_ieee754_ops->func, (&ic->tme_m68k_fpu_ieee754_ctl, src0, src1, dst))
#define TME_M6888X_IEEE754_OP_FUNC(ops_offset) \
  (*((void **) (((char *) ic->tme_m68k_fpu_ieee754_ops) + (ops_offset))))
#define TME_M6888X_IEEE754_OP_RUN(ops_offset, t, x) \
  _TME_M6888X_IEEE754_OP(((void (*) _TME_P(t)) TME_M6888X_IEEE754_OP_FUNC(ops_offset)), x)

/* this gets the Nth raw unsigned 32-bit word from an EA operand: */
#define TME_M6888X_EA_OP32(n)   (ic->tme_m68k_ireg_uint32(op1_ireg32 + (n)))

/* this gets the Nth raw digit from a packed decimal operand: */
#define TME_M6888X_PD_DIGIT(n)  ((TME_M6888X_EA_OP32((n) / 8) >> (4 * ((n) % 8))) & 0xf)

/* types: */

/* the FPgen opmode table: */
struct tme_m6888x_fpgen {

  /* any m6888x-specific function.  this is normally NULL: */
  void (*tme_m6888x_fpgen_func) _TME_P((struct tme_m68k *, 
					const struct tme_float *,
					struct tme_float *));

  /* unless there is an m6888x-specific function, this is the offset
     in the IEEE 754 operations struct of the function: */
  unsigned long tme_m6888x_fpgen_func_ops_offset;

  /* the FPU types that have this function: */
  tme_uint8_t tme_m6888x_fpgen_fpu_types;
  
  /* the operation type: */
  tme_uint8_t tme_m6888x_fpgen_optype;

  /* the rounding mode used by the function: */
  tme_uint8_t tme_m6888x_fpgen_rounding_mode;

  /* the rounding precision used by the function: */
  tme_uint8_t tme_m6888x_fpgen_rounding_precision;
};

/* an m6888x frame: */
struct tme_m6888x_frame {

  /* the frame version: */
  tme_uint8_t tme_m6888x_frame_version;

  /* the frame size: */
  tme_uint8_t tme_m6888x_frame_size;

  /* reserved: */
  tme_uint16_t tme_m6888x_frame_reserved2;

  /* the command/condition register for an IDLE frame: */
  tme_uint16_t tme_m6888x_frame_ccr;

  /* reserved: */
  tme_uint16_t tme_m6888x_frame_reserved6;

  /* additional words: */
  tme_uint32_t tme_m6888x_frame_words[(TME_M6888X_FRAME_SIZE_IDLE_M68882 / sizeof(tme_uint32_t)) - 1];
};

/* prototypes: */
TME_M6888X_FPGEN_DECL(_tme_m6888x_fmovecr);
TME_M6888X_FPGEN_DECL(_tme_m6888x_fsincos);
TME_M6888X_FPGEN_DECL(_tme_m6888x_fcmp);
TME_M6888X_FPGEN_DECL(_tme_m6888x_ftst);
TME_M6888X_FPGEN_DECL(_tme_m6888x_ftwotox);
TME_M6888X_FPGEN_DECL(_tme_m6888x_ftentox);
TME_M6888X_FPGEN_DECL(_tme_m6888x_flog2);
TME_M6888X_FPGEN_DECL(_tme_m6888x_fmod);
TME_M6888X_FPGEN_DECL(_tme_m6888x_frem);
TME_M6888X_FPGEN_DECL(_tme_m6888x_fsgldiv);
TME_M6888X_FPGEN_DECL(_tme_m6888x_fsglmul);

/* globals: */

/* special fpgen structures: */
static const struct tme_m6888x_fpgen _tme_m6888x_fpgen_fmovecr = {
  _tme_m6888x_fmovecr,
  0,
  TME_M68K_FPU_ANY,
  TME_M6888X_OPTYPE_MONADIC,
  TME_FLOAT_ROUND_NULL,
  TME_M6888X_ROUNDING_PRECISION_CTL
};
static const struct tme_m6888x_fpgen _tme_m6888x_fpgen_fmove_rm = {
  NULL,
  0,
  TME_M68K_FPU_ANY,
  TME_M6888X_OPTYPE_MONADIC,
  TME_FLOAT_ROUND_NULL,
  TME_M6888X_ROUNDING_PRECISION_CTL
};

/* include the automatically generated code: */
#include "m6888x-auto.c"

/* this resets the FPU: */
void
tme_m68k_fpu_reset(struct tme_m68k *ic)
{
  unsigned int fp_i;
  
  /* put nonsignaling NaNs in the floating-point data registers: */
  for (fp_i = 0;
       fp_i < (sizeof(ic->tme_m68k_fpu_fpreg) / sizeof(ic->tme_m68k_fpu_fpreg[0]));
       fp_i++) {
    ic->tme_m68k_fpu_fpreg[fp_i].tme_float_format = TME_FLOAT_FORMAT_IEEE754_EXTENDED80;
    ic->tme_m68k_fpu_fpreg[fp_i].tme_float_value_ieee754_extended80 = ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_default_nan_extended80;
  }

  /* put zeroes in the floating-point control register, status
     register, and instruction address register: */
  ic->tme_m68k_fpu_fpcr = 0;
  ic->tme_m68k_fpu_fpsr = 0;
  ic->tme_m68k_fpu_fpiar = 0;
}

/* this handles an exception: */
static void
_tme_m6888x_exception(struct tme_m68k *ic, tme_uint32_t exceptions)
{
  tme_uint8_t vector;

  /* update the EXC byte in the FPSR: */
  ic->tme_m68k_fpu_fpsr |= exceptions;

  /* update the AEXC byte in the FPSR: */
  if (exceptions & (TME_M6888X_FPSR_EXC_SNAN | TME_M6888X_FPSR_EXC_OPERR | TME_M6888X_FPSR_EXC_BSUN)) {
    ic->tme_m68k_fpu_fpsr |= TME_M6888X_FPSR_AEXC_IOP;
  }
  if (exceptions & TME_M6888X_FPSR_EXC_OVFL) {
    ic->tme_m68k_fpu_fpsr |= TME_M6888X_FPSR_AEXC_OVFL;
  }
  if (exceptions & (TME_M6888X_FPSR_EXC_UNFL | TME_M6888X_FPSR_EXC_INEX2)) {
    ic->tme_m68k_fpu_fpsr |= TME_M6888X_FPSR_AEXC_UNFL;
  }
  if (exceptions & TME_M6888X_FPSR_EXC_DZ) {
    ic->tme_m68k_fpu_fpsr |= TME_M6888X_FPSR_AEXC_DZ;
  }
  if (exceptions & (TME_M6888X_FPSR_EXC_INEX1 | TME_M6888X_FPSR_EXC_INEX2 | TME_M6888X_FPSR_EXC_OVFL)) {
    ic->tme_m68k_fpu_fpsr |= TME_M6888X_FPSR_AEXC_INEX;
  }

  /* if any of the new exceptions are unmasked, take the exception: */
  if ((ic->tme_m68k_fpu_fpcr & exceptions)) {

    /* because it's possible for an instruction to cause multiple
       exceptions, the exceptions are prioritized: */
    /* XXX FIXME - when the predecrement or postincrement addressing
       modes are used, are the address registers updated before or
       after any exceptions are generated? */
    if (exceptions & TME_M6888X_FPSR_EXC_BSUN) {
      vector = TME_M6888X_VECTOR_BSUN;
    }
    else if (exceptions & TME_M6888X_FPSR_EXC_SNAN) {
      vector = TME_M6888X_VECTOR_SNAN;
    }
    else if (exceptions & TME_M6888X_FPSR_EXC_OPERR) {
      vector = TME_M6888X_VECTOR_OPERR;
    }
    else if (exceptions & TME_M6888X_FPSR_EXC_OVFL) {
      vector = TME_M6888X_VECTOR_OVFL;
    }
    else if (exceptions & TME_M6888X_FPSR_EXC_UNFL) {
      vector = TME_M6888X_VECTOR_UNFL;
    }
    else if (exceptions & TME_M6888X_FPSR_EXC_DZ) {
      vector = TME_M6888X_VECTOR_DZ;
    }
    else {
      assert (exceptions & (TME_M6888X_FPSR_EXC_INEX2 | TME_M6888X_FPSR_EXC_INEX1));
      vector = TME_M6888X_VECTOR_INEX;
    }

    /* unlock any lock: */
    if (ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_lock_unlock != NULL) {
      (*ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_lock_unlock)();
      ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_lock_unlock = NULL;
    }

    /* take the exception: */
    /* XXX FIXME - we signal all m6888x exceptions as cp
       Postinstruction exceptions.  exceptions generated by a cpGEN
       instruction are probably supposed to be cp Preinstruction
       exceptions, signaled at the time of the next cpGEN instruction: */
    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;
    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(vector));
  }
}

/* the IEEE 754 exception handler: */
static void
_tme_m6888x_exception_ieee754(struct tme_ieee754_ctl *ctl, tme_int8_t exceptions_ieee754)
{
  tme_uint32_t exceptions_m6888x;

  /* map the exceptions: */
  exceptions_m6888x = 0;
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_GENERIC) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_OPERR;
  }
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_INVALID) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_OPERR;
  }
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_DIVBYZERO) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_DZ;
  }
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_OVERFLOW) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_OVFL;
  }
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_UNDERFLOW) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_UNFL;
  }
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_INEXACT) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_INEX2;
  }
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_OVERFLOW_INT) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_OVFL;
  }
  /* XXX FIXME - do denormals count as INEX2? */
  if (exceptions_ieee754 & TME_FLOAT_EXCEPTION_DENORMAL) {
    exceptions_m6888x |= TME_M6888X_FPSR_EXC_INEX2;
  }

  _tme_m6888x_exception((struct tme_m68k *) ctl->tme_ieee754_ctl_private, exceptions_m6888x);
}

/* signaling NaN tests: */
#define _TME_M6888X_IS_SNAN(a) (((a)->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi & TME_BIT(30)) == 0)
static tme_int8_t
_tme_m6888x_is_snan_extended80(struct tme_float_ieee754_extended80 *value)
{
  return (_TME_M6888X_IS_SNAN(value));
}

/* NaN propagation: */
static void
_tme_m6888x_nan_from_nans_extended80(struct tme_ieee754_ctl *ctl, 
				     const struct tme_float_ieee754_extended80 *a,
				     const struct tme_float_ieee754_extended80 *b,
				     struct tme_float_ieee754_extended80 *z)
{
  struct tme_m68k *ic;
  int a_is_snan;
  int b_is_snan;

  /* recover the m68k: */
  ic = ctl->tme_ieee754_ctl_private;    

  /* see if any of the NaNs are signaling NaNs: */
  a_is_snan = _TME_M6888X_IS_SNAN(a);
  b_is_snan = _TME_M6888X_IS_SNAN(b);

  /* if either operand is a signaling NaN: */
  if (a_is_snan || b_is_snan) {

    /* signal the signaling NaN: */
    _tme_m6888x_exception(ic, TME_M6888X_FPSR_EXC_SNAN);
  }

  /* if a and b are different NaNs: */
  if ((a->tme_float_ieee754_extended80_sexp
       != b->tme_float_ieee754_extended80_sexp)
      || (a->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi
	  != b->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi)
      || (a->tme_float_ieee754_extended80_significand.tme_value64_uint32_lo
	  != b->tme_float_ieee754_extended80_significand.tme_value64_uint32_lo)) {
    
    /* we need to return the NaN that is the destination operand: */
    switch (_tme_m6888x_fpgen_opmode_table[TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 7)].tme_m6888x_fpgen_optype) {
    default:
    case TME_M6888X_OPTYPE_MONADIC: assert(FALSE);
    case TME_M6888X_OPTYPE_DYADIC_SRC_DST: a = b; break;
    case TME_M6888X_OPTYPE_DYADIC_DST_SRC: break;
    }
  }

  /* return a as the NaN, but make sure it's nonsignaling: */
  *z = *a;
  z->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi |= TME_BIT(30);
}

/* this prepares to run an fpgen instruction: */
static void inline
_tme_m6888x_fpgen_enter(struct tme_m68k *ic, const struct tme_m6888x_fpgen *fpgen)
{
  tme_int8_t rounding_mode;
  tme_int8_t rounding_precision;

  /* set the rounding mode: */
  rounding_mode = fpgen->tme_m6888x_fpgen_rounding_mode;
  if (__tme_predict_true(rounding_mode == TME_FLOAT_ROUND_NULL)) {
    switch (ic->tme_m68k_fpu_fpcr & TME_M6888X_FPCR_RND_MASK) {
    default: assert(FALSE);
    case TME_M6888X_FPCR_RND_RN: rounding_mode = TME_FLOAT_ROUND_NEAREST_EVEN; break;
    case TME_M6888X_FPCR_RND_RZ: rounding_mode = TME_FLOAT_ROUND_TO_ZERO; break;
    case TME_M6888X_FPCR_RND_RM: rounding_mode = TME_FLOAT_ROUND_DOWN; break;
    case TME_M6888X_FPCR_RND_RP: rounding_mode = TME_FLOAT_ROUND_UP; break;
    }
  }
  ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = rounding_mode;

  /* set the rounding precision: */
  rounding_precision = fpgen->tme_m6888x_fpgen_rounding_precision;
  if (__tme_predict_true(rounding_precision == TME_M6888X_ROUNDING_PRECISION_CTL)) {
    switch (ic->tme_m68k_fpu_fpcr & TME_M6888X_FPCR_PREC_MASK) {
    default: assert(FALSE); /* FALLTHROUGH */
    case TME_M6888X_FPCR_PREC_UNDEF: /* FALLTHROUGH */
    case TME_M6888X_FPCR_PREC_X: rounding_precision = TME_M6888X_ROUNDING_PRECISION_EXTENDED80; break;
    case TME_M6888X_FPCR_PREC_S: rounding_precision = TME_M6888X_ROUNDING_PRECISION_SINGLE; break;
    case TME_M6888X_FPCR_PREC_D: rounding_precision = TME_M6888X_ROUNDING_PRECISION_DOUBLE; break;
    }
  }
  ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_extended80_rounding_precision = rounding_precision;

  /* clear the exception status byte in the FPSR: */
  ic->tme_m68k_fpu_fpsr
    &= ~(TME_M6888X_FPSR_EXC_INEX1
	 | TME_M6888X_FPSR_EXC_INEX2
	 | TME_M6888X_FPSR_EXC_DZ
	 | TME_M6888X_FPSR_EXC_UNFL
	 | TME_M6888X_FPSR_EXC_OVFL
	 | TME_M6888X_FPSR_EXC_OPERR
	 | TME_M6888X_FPSR_EXC_SNAN
	 | TME_M6888X_FPSR_EXC_BSUN);

  /* set the FPIAR: */
  ic->tme_m68k_fpu_fpiar = ic->tme_m68k_ireg_pc;
}

/* this sets the floating-point condition codes: */
static void inline
_tme_m6888x_fpcc(struct tme_m68k *ic, const struct tme_float *dst, unsigned int dst_formats)
{
  tme_uint32_t fpcc;

  /* start with no floating-point condition codes: */
  fpcc = 0;

  /* set N: */
  if (tme_float_is_negative(dst, dst_formats)) {
    fpcc |= TME_M6888X_FPSR_CC_N;
  }

  /* set NAN or I or Z: */
  if (tme_float_is_nan(dst, dst_formats)) {
    fpcc |= TME_M6888X_FPSR_CC_NAN;
  }
  else if (tme_float_is_inf(dst, dst_formats)) {
    fpcc |= TME_M6888X_FPSR_CC_I;
  }
  else if (tme_float_is_zero(dst, dst_formats)) {
    fpcc |= TME_M6888X_FPSR_CC_Z;
  }

  /* set the floating-point condition codes: */
  ic->tme_m68k_fpu_fpsr
    = ((ic->tme_m68k_fpu_fpsr
	& ~(TME_M6888X_FPSR_CC_N
	    | TME_M6888X_FPSR_CC_NAN
	    | TME_M6888X_FPSR_CC_I
	    | TME_M6888X_FPSR_CC_Z))
       | fpcc);
}

TME_M68K_INSN(tme_m68k_fpgen)
{
  struct tme_ieee754_ctl *ieee754_ctl;
  tme_uint16_t command;
  tme_uint16_t opmode;
  const struct tme_m6888x_fpgen *fpgen;
  unsigned int src_ea;
  const struct tme_float *src;
  struct tme_float *dst;
  struct tme_float src_buffer;
  struct tme_float dst_buffer;
  struct tme_float conv_buffer;
  union tme_value64 value64_buffer;
  struct tme_float_ieee754_extended80 extended80_buffer;
  unsigned int ea_mode;
  unsigned int ea_reg;
  unsigned int ea_size;
  unsigned int op1_ireg32;
  unsigned int src_specifier;
  unsigned int digit_i;
  tme_int32_t packed_value_int32;
  struct tme_float packed_value_float;
  tme_int32_t exponent;

  /* get the IEEE 754 ctl: */
  ieee754_ctl = &ic->tme_m68k_fpu_ieee754_ctl;

  /* this is an FPU instruction: */
  TME_M68K_INSN_FPU;

  /* get the coprocessor-dependent command word: */
  command = TME_M68K_INSN_SPECOP;

  /* if this is an FMOVECR instruction
     (command word pattern 0101 11dd dooo oooo): */
  if ((command & 0xfc00) == 0x5c00
      && TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 6) == 0) {

    /* use the FMOVECR opmode and FPgen structure: */
    opmode = TME_M6888X_FPGEN_OPMODE_OTHER;
    fpgen = &_tme_m6888x_fpgen_fmovecr;

    /* the source operand does not use the EA: */
    src_ea = FALSE;
  }

  /* otherwise, this is a generic FPgen instruction: */
  else {

    /* get the opmode: */
    opmode = TME_FIELD_EXTRACTU(command, 0, 7);

    /* decode this instruction: */
    fpgen = &_tme_m6888x_fpgen_opmode_table[opmode];

    /* the source operand uses the EA if this is an EA-to-register
       operation: */
    src_ea = (command & TME_BIT(14)) != 0;
  }

  /* catch illegal instructions: */
  switch (fpgen->tme_m6888x_fpgen_fpu_types) {

  case TME_M68K_FPU_M6888X:
    /* instructions not supported in hardware by the m68040 are caught
       later: */
  case TME_M68K_FPU_ANY:
    break;
    
  case TME_M68K_FPU_M68040:
    if (ic->tme_m68k_fpu_type == TME_M68K_FPU_M68040) {
      break;
    }
    /* FALLTHROUGH */
  case TME_M68K_FPU_NONE:
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
    break;
  default:
    abort();
  }
  
  /* get the source specifier: */
  src_specifier = TME_FIELD_EXTRACTU(command, 10, 3);

  /* if the source operand uses the EA: */
  if (src_ea) {

    /* assume that the most-significant first 32-bit part of the
       source operand will end up in the internal memx register: */
    op1_ireg32 = TME_M68K_IREG_MEMX32;

    /* get the EA mode and register fields: */
    ea_mode = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3);
    ea_reg = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);

    /* if this is a data register direct EA: */
    if (ea_mode == 0) {
    
      /* dispatch on the source specifier, since we need to
	 sign-extend a byte or word to long, and we need to check that
	 only a byte, word, long, or single precision source is
	 specified: */
      switch (src_specifier) {
      case TME_M6888X_TYPE_LONG:
      case TME_M6888X_TYPE_SINGLE:
	op1_ireg32 = TME_M68K_IREG_D0 + ea_reg;
	break;
      case TME_M6888X_TYPE_WORD:
	ic->tme_m68k_ireg_int32(TME_M68K_IREG_MEMX32) = (tme_int16_t) TME_M68K_INSN_OP1(tme_int32_t);
	break;
      case TME_M6888X_TYPE_BYTE:
	ic->tme_m68k_ireg_int32(TME_M68K_IREG_MEMX32) = (tme_int8_t) TME_M68K_INSN_OP1(tme_int32_t);
	break;
      default:
	TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
	break;
      }
    }

    /* otherwise, if this is an address register direct EA: */
    else if (ea_mode == 1) {
      TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
    }

    /* otherwise, if this is an immediate EA: */
    else if (ea_mode == 7
	     && ea_reg == 4) {

      /* _op1 already points to the operand as one or more 32-bit
         words: */
      assert (_op1 == &ic->tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32 + 0));
      op1_ireg32 = TME_M68K_IREG_IMM32;
    }

    /* otherwise, this is a memory EA: */
    else {

      /* this instruction can fault: */
      TME_M68K_INSN_CANFAULT;

      /* adjust ea_reg to reference the address register: */
      ea_reg += TME_M68K_IREG_A0;

      /* dispatch on the source specifier to size the operand: */
      switch (src_specifier) {
      case TME_M6888X_TYPE_LONG:
      case TME_M6888X_TYPE_SINGLE:
	ea_size = TME_M68K_SIZE_32;
	break;

      case TME_M6888X_TYPE_PACKEDDEC:
      case TME_M6888X_TYPE_EXTENDED80:
	ea_size = TME_M68K_SIZE_96;
	break;

      case TME_M6888X_TYPE_WORD:
	ea_size = TME_M68K_SIZE_16;
	break;

      case TME_M6888X_TYPE_DOUBLE:
	ea_size = TME_M68K_SIZE_64;
	break;

      case TME_M6888X_TYPE_BYTE:
	ea_size = TME_M68K_SIZE_8;
	break;

      default:
	abort();
      }

      /* for the effective address predecrement and postincrement
	 modes, we require that these size macros correspond exactly
	 to the number of bytes: */
#if TME_M68K_SIZE_8 != 1
#error "TME_M68K_SIZE_8 must be 1"
#endif
#if TME_M68K_SIZE_16 != 2
#error "TME_M68K_SIZE_16 must be 2"
#endif
#if TME_M68K_SIZE_32 != 4
#error "TME_M68K_SIZE_32 must be 4"
#endif
#if TME_M68K_SIZE_64 != 8
#error "TME_M68K_SIZE_64 must be 8"
#endif
#if TME_M68K_SIZE_96 != 12
#error "TME_M68K_SIZE_96 must be 12"
#endif
#define TME_M68K_AREG_INCREMENT(areg, size) \
  ((size) + (((size) == TME_M68K_SIZE_8 && (areg) == TME_M68K_IREG_A7) ? 1 : 0))

      /* address register indirect postincrement: */
      if (ea_mode == 3) {
	/* if we are not restarting, set the effective address: */
	if (!TME_M68K_SEQUENCE_RESTARTING) {
	  ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ea_reg);
	  ic->tme_m68k_ireg_uint32(ea_reg) += TME_M68K_AREG_INCREMENT(ea_reg, ea_size);
	}
      }

      /* address register indirect predecrement: */
      else if (ea_mode == 4) {
	/* if we are not restarting, set the effective address: */
	if (!TME_M68K_SEQUENCE_RESTARTING) {
	  ic->tme_m68k_ireg_uint32(ea_reg) -= TME_M68K_AREG_INCREMENT(ea_reg, ea_size);
	  ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ea_reg);
	}
      }

      /* dispatch on the operand size to read in the operand as one or
         more 32-bit words.  we will read up to three 32-bit words
         into memx, memy, and memz: */
      assert ((TME_M68K_IREG_MEMX32 + 1) == TME_M68K_IREG_MEMY32
	      && (TME_M68K_IREG_MEMY32 + 1) == TME_M68K_IREG_MEMZ32);
      switch (ea_size) {

	/* this can only happen when the source operand is a byte.  we
	   sign-extend the byte to a long: */
      case TME_M68K_SIZE_8:
	tme_m68k_read_memx8(ic);
	assert (!TME_M68K_SEQUENCE_RESTARTING);
	ic->tme_m68k_ireg_memx32 = TME_EXT_S8_S32((tme_int8_t) ic->tme_m68k_ireg_memx8);
	break;

	/* this can only happen when the source operand is a word.  we
	   sign-extend the word to a long: */
      case TME_M68K_SIZE_16:
	tme_m68k_read_memx16(ic);
	assert (!TME_M68K_SEQUENCE_RESTARTING);
	ic->tme_m68k_ireg_memx32 = TME_EXT_S16_S32((tme_int16_t) ic->tme_m68k_ireg_memx16);
	break;

	/* everything else is one or more 32-bit words: */
      default:

	/* read the first 32 bits into the memx register: */
	tme_m68k_read_memx32(ic);
	if (ea_size == TME_M68K_SIZE_32) {
	  break;
	}

	/* read the second 32 bits into the memy register: */
	if (!TME_M68K_SEQUENCE_RESTARTING) {
	  ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
	}
	tme_m68k_read_mem32(ic, TME_M68K_IREG_MEMY32);
	if (ea_size == TME_M68K_SIZE_64) {
	  break;
	}
	  
	/* read the third 32 bits into the memz register: */
	if (!TME_M68K_SEQUENCE_RESTARTING) {
	  ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
	}
	tme_m68k_read_mem32(ic, TME_M68K_IREG_MEMZ32);
	break;
      }
    }

    /* convert the operand from one or more raw 32-bit words into the
       internal extended precision format: */
    switch (src_specifier) {
      
      /* convert a 32-bit integral value.  all of these integral types
	 have already been converted into 32-bit signed integers: */
    case TME_M6888X_TYPE_BYTE:
    case TME_M6888X_TYPE_WORD:
    case TME_M6888X_TYPE_LONG:
      tme_ieee754_extended80_from_int32((tme_int32_t) TME_M6888X_EA_OP32(0), &src_buffer);
      break;

      /* convert a single-precision value: */
    case TME_M6888X_TYPE_SINGLE:
      tme_ieee754_single_value_set(&conv_buffer, TME_M6888X_EA_OP32(0));
      TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_extended80_from_single,
				    &conv_buffer, 
				    &src_buffer);
      break;

      /* convert a double-precision value: */
    case TME_M6888X_TYPE_DOUBLE:
      /* NB that TME_M6888X_EA_OP32(0) is always the most significant
	 32 bits of the double, regardless of the endianness of the
	 host.  this is how both the executer fetches an immediate
	 double, and how the memory code above reads a double: */
      value64_buffer.tme_value64_uint32_hi = TME_M6888X_EA_OP32(0);
      value64_buffer.tme_value64_uint32_lo = TME_M6888X_EA_OP32(1);
      tme_ieee754_double_value_set(&conv_buffer, value64_buffer);
      TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_extended80_from_double,
				    &conv_buffer, 
				    &src_buffer);
      break;

      /* assign an extended-precision value: */
    case TME_M6888X_TYPE_EXTENDED80:
      /* NB that TME_M6888X_EA_OP32(0) is always the most significant
	 32 bits of the extended80, regardless of the endianness of
	 the host.  this is how both the executer fetches an immediate
	 extended80, and how the memory code above reads a extended80: */
      extended80_buffer.tme_float_ieee754_extended80_sexp = TME_M6888X_EA_OP32(0) >> 16;
      extended80_buffer.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi = TME_M6888X_EA_OP32(1);
      extended80_buffer.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = TME_M6888X_EA_OP32(2);
      tme_ieee754_extended80_value_set(&src_buffer, extended80_buffer);
      break;
      
    case TME_M6888X_TYPE_PACKEDDEC:

      /* if this value's SE and YY bits are all set, and the exponent
	 is 0xFFF, the value is either an infinity or a NaN: */
      if ((TME_M6888X_EA_OP32(0)
	   & (TME_M6888X_PACKEDDEC_SE
	      | TME_M6888X_PACKEDDEC_YY))
	  == (TME_M6888X_PACKEDDEC_SE
	      | TME_M6888X_PACKEDDEC_YY)
	  && TME_M6888X_PD_DIGIT(22) == 0xf
	  && TME_M6888X_PD_DIGIT(21) == 0xf
	  && TME_M6888X_PD_DIGIT(20) == 0xf) {

	/* "A packed decimal real data format with the SE and both Y
	   bits set, an exponent of $FFF and a nonzero 16-bit [sic]
	   decimal fraction is a NAN.  When the FPU uses this format,
	   the fraction of the NAN is moved bit- by-bit into the
	   extended-precision mantissa of a floating-point data
	   register."
	   
	   moving the fraction bit-by-bit works for the infinities,
	   too, since both the packed decimal and the extended
	   precision infinities have all-bits-zero fractions: */
	extended80_buffer.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi = TME_M6888X_EA_OP32(1);
	extended80_buffer.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = TME_M6888X_EA_OP32(2);
	  
	/* "The exponent of the register is set to signify a NAN,
	   and no conversion occurs.  The MSB of the most
	   significant digit in the decimal fraction (the MSB of
	   digit 15) is a don't care, as in extended-precision NANs,
	   and the MSB of minus one of digit 15 is the SNAN bit. If
	   the NAN bit is a zero, then it is an SNAN."

	   the biased exponent for NaNs and infinities is the same,
	   and the sign bit is a don't care for a NaN: */
	extended80_buffer.tme_float_ieee754_extended80_sexp
	  = (0x7fff
	     | (TME_M6888X_EA_OP32(0) & TME_M6888X_PACKEDDEC_SM
		? 0x8000
		: 0));

	/* finally create the source operand: */
	tme_ieee754_extended80_value_set(&src_buffer, extended80_buffer);
      }

      /* otherwise, this should be an in-range value: */
      else {

	/* "The FPU does not detect non-decimal digits in the exponent,
	   integer, or fraction digits of an in-range packed decimal real data
	   format.  These non-decimal digits are converted to binary in the
	   same manner as decimal digits; however, the result is probably
	   useless although it is repeatable." */

	/* convert the significand: */
	tme_ieee754_extended80_from_int32(TME_M6888X_PD_DIGIT(16), &src_buffer);
	tme_ieee754_extended80_from_int32(100000000, &conv_buffer);
	packed_value_int32 = 0;
	digit_i = 15;
	do {
	  packed_value_int32 = (packed_value_int32 * 10) + TME_M6888X_PD_DIGIT(digit_i);
	  if ((digit_i % 8) == 0) {
	    TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_mul,
					 &src_buffer, 
					 &conv_buffer,
					 &src_buffer);
	    tme_ieee754_extended80_from_int32(packed_value_int32, &packed_value_float);
	    TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_add,
					 &src_buffer, 
					 &packed_value_float,
					 &src_buffer);
	    packed_value_int32 = 0;
	  }
	} while (digit_i-- > 0);
	if (TME_M6888X_EA_OP32(0) & TME_M6888X_PACKEDDEC_SM) {
	  tme_ieee754_extended80_from_int32(-1, &conv_buffer);
	  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_mul,
				       &src_buffer, 
				       &conv_buffer,
				       &src_buffer);
	}
	  
	/* convert the exponent: */
	exponent = 0;
	digit_i = 22;
	do {
	  exponent = (exponent * 10) + TME_M6888X_PD_DIGIT(digit_i);
	} while (digit_i-- > 21);
	if (TME_M6888X_EA_OP32(0) & TME_M6888X_PACKEDDEC_SE) {
	  exponent = -exponent;
	}

	/* adjust the exponent, since we ignored the implicit decimal
	   point when converting the significand: */
	exponent -= 16;

	/* scale the significand: */
	tme_ieee754_extended80_from_int32(exponent, &conv_buffer);
	tme_ieee754_extended80_radix10_scale(&ic->tme_m68k_fpu_ieee754_ctl, &src_buffer, &conv_buffer, &src_buffer);
      }
      break;

    default:
      abort();
    }

    /* the source operand is in the buffer: */
    src = &src_buffer;
  }

  /* otherwise, the source operand is in a register: */
  else {
    src = &ic->tme_m68k_fpu_fpreg[src_specifier];
  }

  /* XXX FIXME - a check for operand types not implemented on the
     m68040 would go here: */

  /* do the common fpgen setup: */
  _tme_m6888x_fpgen_enter(ic, fpgen);

  /* get the destination operand: */
  dst = &ic->tme_m68k_fpu_fpreg[TME_FIELD_EXTRACTU(command, 7, 3)];

  /* dispatch on the opmode to handle any special cases: */
  switch (opmode) {

    /* these instructions don't modify the destination register: */
  case TME_M6888X_FPGEN_OPMODE_FCMP:
  case TME_M6888X_FPGEN_OPMODE_FTST:
    dst_buffer = *dst;
    dst = &dst_buffer;
    break;

  default:
    break;
  }

  /* if this instruction is m6888x specific: */
  if (fpgen->tme_m6888x_fpgen_func != NULL) {

    /* run the function: */
    (*fpgen->tme_m6888x_fpgen_func)(ic, src, dst);
  }

  /* otherwise, this instruction has an IEEE 754 operation: */
  else {

    /* run the function: */
    switch (fpgen->tme_m6888x_fpgen_optype) {
    default: assert(FALSE);
    case TME_M6888X_OPTYPE_MONADIC:
      TME_M6888X_IEEE754_OP_RUN(fpgen->tme_m6888x_fpgen_func_ops_offset, (struct tme_ieee754_ctl *, const struct tme_float *, struct tme_float *), (&ic->tme_m68k_fpu_ieee754_ctl, src, dst));
      break;
    case TME_M6888X_OPTYPE_DYADIC_SRC_DST:
      TME_M6888X_IEEE754_OP_RUN(fpgen->tme_m6888x_fpgen_func_ops_offset, (struct tme_ieee754_ctl *, const struct tme_float *, const struct tme_float *, struct tme_float *), (&ic->tme_m68k_fpu_ieee754_ctl, src, dst, dst));
      break;
    case TME_M6888X_OPTYPE_DYADIC_DST_SRC:
      TME_M6888X_IEEE754_OP_RUN(fpgen->tme_m6888x_fpgen_func_ops_offset, (struct tme_ieee754_ctl *, const struct tme_float *, const struct tme_float *, struct tme_float *), (&ic->tme_m68k_fpu_ieee754_ctl, dst, src, dst));
      break;
    }
  }

  /* set the floating-point condition codes: */
  _tme_m6888x_fpcc(ic, dst, TME_FLOAT_FORMAT_IEEE754_EXTENDED80 | TME_FLOAT_FORMAT_IEEE754_EXTENDED80_BUILTIN);

#undef TME_M68K_AREG_INCREMENT
}

TME_M6888X_FPGEN(_tme_m6888x_fsincos)
{
  /* "If FPs and FPc are specified to be the same register, the cosine
     result is first loaded into the register and then is overwritten
     with the sine result." */
  TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_extended80_cos,
				src, 
				&ic->tme_m68k_fpu_fpreg[TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 3)]);
  TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_extended80_sin,
				src, 
				dst);
}

TME_M6888X_FPGEN(_tme_m6888x_fcmp)
{
  int dst_is_negative;
  int src_is_negative;

  /* check for a NaN operand: */
  if (__tme_predict_false(tme_ieee754_extended80_check_nan_dyadic(&ic->tme_m68k_fpu_ieee754_ctl, src, dst, dst))) {
    return;
  }

  /* see if the destination is negative: */
  dst_is_negative
    = (tme_float_is_negative(dst,
			     (TME_FLOAT_FORMAT_IEEE754_EXTENDED80
			      | TME_FLOAT_FORMAT_IEEE754_EXTENDED80_BUILTIN))
       != 0);

  /* if the source operand is an infinity: */
  if (tme_ieee754_extended80_is_inf(src)) {

    /* see if the source operand is negative infinity: */
    src_is_negative
      = (tme_float_is_negative(src,
			       (TME_FLOAT_FORMAT_IEEE754_EXTENDED80
				| TME_FLOAT_FORMAT_IEEE754_EXTENDED80_BUILTIN))
	 != 0);

    /* if the destination operand is the same infinity as the source operand: */
    if (tme_ieee754_extended80_is_inf(dst)
	&& dst_is_negative == src_is_negative) {

      /* return a zero, to set Z, with the same sign as the source
	 operand, to set N appropriately: */
      tme_ieee754_extended80_value_set_constant(dst, &tme_ieee754_extended80_constant_zero);
      if (src_is_negative) {
	assert (dst->tme_float_format == TME_FLOAT_FORMAT_IEEE754_EXTENDED80);
	dst->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_sexp |= 0x8000;
      }
    }

    /* otherwise, either the destination operand is not an infinity
       or it is the other infinity: */
    else {

      /* return a one with the opposite sign as the source operand, to
	 set N appropriately: */
      tme_ieee754_extended80_value_set_constant(dst, &tme_ieee754_extended80_constant_one);
      if (!src_is_negative) {
	assert (dst->tme_float_format == TME_FLOAT_FORMAT_IEEE754_EXTENDED80);
	dst->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_sexp |= 0x8000;
      }
    }
    return;
  }

  /* otherwise, if the destination operand is an infinity: */
  else if (tme_ieee754_extended80_is_inf(dst)) {

    /* return a one with the same sign as the destination operand, to
       set N appropriately: */
    tme_ieee754_extended80_value_set_constant(dst, &tme_ieee754_extended80_constant_one);
    if (dst_is_negative) {
      assert (dst->tme_float_format == TME_FLOAT_FORMAT_IEEE754_EXTENDED80);
      dst->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_sexp |= 0x8000;
    }
    return;
  }

  /* do the subtraction: */
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_sub,
			       dst,
			       src,
			       dst);
}

TME_M6888X_FPGEN(_tme_m6888x_ftst)
{
  *dst = *src;
}

TME_M6888X_FPGEN(_tme_m6888x_ftwotox)
{
  struct tme_float two;

  tme_ieee754_extended80_value_set_constant(&two, &tme_ieee754_extended80_constant_2e2ex[0]);
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_pow,
			       src, 
			       &two,
			       dst);
}

TME_M6888X_FPGEN(_tme_m6888x_ftentox)
{
  struct tme_float ten;

  tme_ieee754_extended80_value_set_constant(&ten, &tme_ieee754_extended80_constant_10e2ex[0]);
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_pow,
			       src, 
			       &ten,
			       dst);
}

TME_M6888X_FPGEN(_tme_m6888x_flog2)
{
  struct tme_float log_two;

  /* 2^log2(x) = e^log(x) */
  /* log(2^log2(x)) = log(e^log(x)) */
  /* log2(x) * log(2) = log(x) * log(e) */
  /* log2(x) = log(x) / log(2) */

  TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_extended80_log,
				src,
				dst);
  tme_ieee754_extended80_value_set_constant(&log_two, &tme_ieee754_extended80_constant_ln_2);
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_div,
			       dst,
			       &log_two,
			       dst);
}

/* this internal function handles fmod and frem: */
static void
_tme_m6888x_fmodrem(struct tme_m68k *ic, const struct tme_float *src, struct tme_float *dst, int rounding)
{
  struct tme_float quotient;
  struct tme_float quotient_divisor;
  tme_int32_t quotient_byte;
  struct tme_float two_hundred_fifty_six;

  /* check for a NaN operand: */
  if (__tme_predict_false(tme_ieee754_extended80_check_nan_dyadic(&ic->tme_m68k_fpu_ieee754_ctl, src, dst, dst))) {
    return;
  }

  /* if the source operand is zero, or if the destination operand is infinity: */
  if (tme_ieee754_extended80_is_zero(src)
      || tme_ieee754_extended80_is_inf(dst)) {

    /* return a NaN: */
    dst->tme_float_format = TME_FLOAT_FORMAT_IEEE754_EXTENDED80;
    dst->tme_float_value_ieee754_extended80 = ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_default_nan_extended80;
    return;
  }

  /* do the division.  the quotient must not be a NaN: */
  ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_rounding_mode = rounding;
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_div, dst, src, &quotient);
  assert (!tme_ieee754_extended80_is_nan(&quotient));

  /* round the quotient to an integer: */
  /* XXX FIXME we assume that the rounding mode is the same as the division: */
  TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_extended80_rint, &quotient, &quotient);

  /* get the remainder: */
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_mul, src, &quotient, &quotient_divisor);
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_sub, dst, &quotient_divisor, dst);

  /* get the quotient's least significant eight bits, eventually
     truncating them to seven: */
  tme_ieee754_extended80_from_int32(256, &two_hundred_fifty_six);
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_rem, &quotient, &two_hundred_fifty_six, &quotient);
  quotient_byte = tme_ieee754_extended80_value_builtin_get(&quotient);
  if (quotient_byte >= 0) {
    quotient_byte &= 0x7f;
  }
  else {
    quotient_byte = ((-quotient_byte) & 0x7f) | 0x80;
  }

  /* update the quotient byte in the FPSR: */
  TME_FIELD_MASK_DEPOSITU(ic->tme_m68k_fpu_fpsr, TME_M6888X_FPSR_QUOTIENT, ((tme_uint32_t) quotient_byte));
}

TME_M6888X_FPGEN(_tme_m6888x_fmod)
{
  _tme_m6888x_fmodrem(ic, src, dst, TME_FLOAT_ROUND_TO_ZERO);
}

TME_M6888X_FPGEN(_tme_m6888x_frem)
{
  _tme_m6888x_fmodrem(ic, src, dst, TME_FLOAT_ROUND_NEAREST_EVEN);  
}

TME_M6888X_FPGEN(_tme_m6888x_fsgldiv)
{
  struct tme_float src_trunc, dst_trunc;
  struct tme_float_ieee754_extended80 src_buffer, dst_buffer;

  /* check for a NaN operand: */
  if (__tme_predict_false(tme_ieee754_extended80_check_nan_dyadic(&ic->tme_m68k_fpu_ieee754_ctl, src, dst, dst))) {
    return;
  }

  /* if the source and destination operands are both zero or both
     infinity: */
  if ((tme_ieee754_extended80_is_zero(src)
       && tme_ieee754_extended80_is_zero(dst))
      || (tme_ieee754_extended80_is_inf(src)
	  && tme_ieee754_extended80_is_inf(dst))) {

    /* return a NaN: */
    dst->tme_float_format = TME_FLOAT_FORMAT_IEEE754_EXTENDED80;
    dst->tme_float_value_ieee754_extended80 = ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_default_nan_extended80;

    /* set OPERR: */
    _tme_m6888x_exception(ic, TME_M6888X_FPSR_EXC_OPERR);
    return;
  }

  /* truncate the significands of the source and destination to no
     more than 24 bits to the right of the point.  24 becomes 25
     because the extended80 format includes the explicit integer bit: */
  tme_ieee754_extended80_value_set(&src_trunc, *tme_ieee754_extended80_value_get(src, &src_buffer));
  src_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi &= 0xffff8000;
  src_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = 0x00000000;
  tme_ieee754_extended80_value_set(&dst_trunc, *tme_ieee754_extended80_value_get(dst, &dst_buffer));
  dst_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi &= 0xffff8000;
  dst_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = 0x00000000;

  /* do the division: */
  ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_extended80_rounding_precision = 32;
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_div, &dst_trunc, &src_trunc, dst);
}

TME_M6888X_FPGEN(_tme_m6888x_fsglmul)
{
  struct tme_float src_trunc, dst_trunc;
  struct tme_float_ieee754_extended80 src_buffer, dst_buffer;

  /* check for a NaN operand: */
  if (__tme_predict_false(tme_ieee754_extended80_check_nan_dyadic(&ic->tme_m68k_fpu_ieee754_ctl, src, dst, dst))) {
    return;
  }

  /* if the source is a zero and the destination is a NaN, or vice
     versa: */
  if ((tme_ieee754_extended80_is_zero(src)
       && tme_ieee754_extended80_is_inf(dst))
      || (tme_ieee754_extended80_is_inf(src)
	  && tme_ieee754_extended80_is_zero(dst))) {

    /* return a NaN: */
    dst->tme_float_format = TME_FLOAT_FORMAT_IEEE754_EXTENDED80;
    dst->tme_float_value_ieee754_extended80 = ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_default_nan_extended80;

    /* if the destination is a zero, set OPERR: */
    if (tme_ieee754_extended80_is_zero(dst)) {
      _tme_m6888x_exception(ic, TME_M6888X_FPSR_EXC_OPERR);
    }
    return;
  }

  /* truncate the significands of the source and destination to no
     more than 24 bits to the right of the point.  24 becomes 25
     because the extended80 format includes the explicit integer bit: */
  tme_ieee754_extended80_value_set(&src_trunc, *tme_ieee754_extended80_value_get(src, &src_buffer));
  src_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi &= 0xffff8000;
  src_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = 0x00000000;
  tme_ieee754_extended80_value_set(&dst_trunc, *tme_ieee754_extended80_value_get(dst, &dst_buffer));
  dst_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi &= 0xffff8000;
  dst_trunc.tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = 0x00000000;

  /* do the multiplication: */
  ic->tme_m68k_fpu_ieee754_ctl.tme_ieee754_ctl_extended80_rounding_precision = 32;
  TME_M6888X_IEEE754_OP_DYADIC(tme_ieee754_ops_extended80_mul, &src_trunc, &dst_trunc, dst);
}

TME_M6888X_FPGEN(_tme_m6888x_fmovecr)
{
  const struct tme_ieee754_extended80_constant *constant;
  tme_uint16_t offset;

  offset = TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 7);
  
  /* the binary powers of 10 offsets: */
  if (offset >= 0x33
      && offset <= 0x3f) {
    constant = &tme_ieee754_extended80_constant_10e2ex[offset - 0x33];
  }

  /* anything else: */
  else {
    switch (offset) {
    case 0x00: constant = &tme_ieee754_extended80_constant_pi; break;
    case 0x0b: constant = &tme_ieee754_extended80_constant_log10_2; break;
    case 0x0c: constant = &tme_ieee754_extended80_constant_e; break;
    case 0x0d: constant = &tme_ieee754_extended80_constant_log2_e; break;
    case 0x0e: constant = &tme_ieee754_extended80_constant_log10_e; break;
    default:
    case 0x0f: constant = &tme_ieee754_extended80_constant_zero; break;
    case 0x30: constant = &tme_ieee754_extended80_constant_ln_2; break;
    case 0x31: constant = &tme_ieee754_extended80_constant_ln_10; break;
    case 0x32: constant = &tme_ieee754_extended80_constant_one; break;
    }
  }

  /* return the result: */
  tme_ieee754_extended80_value_set_constant(dst, constant);
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_fmove_rm)
{
  unsigned int ea_mode;
  unsigned int ea_reg;
  unsigned int ea_size;
  unsigned int destination_format;
  const struct tme_float *src;
  struct tme_float src_buffer;
  const struct tme_float *dst;
  struct tme_float dst_buffer;
  unsigned int dst_formats;
  int src_is_nan;
  tme_int32_t value_int32_raw;
  tme_int32_t value_int32;
  tme_uint32_t single_buffer;
  const union tme_value64 *value64;
  union tme_value64 value64_buffer;
  const struct tme_float_ieee754_extended80 *extended80;
  struct tme_float_ieee754_extended80 extended80_buffer;

  /* get the EA mode and register fields: */
  ea_mode = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3);
  ea_reg = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);

  /* get the destination format: */
  destination_format = TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 10, 3);

  /* if this is an address register direct EA, or this is a data
     register direct EA and the destination format isn't byte, word,
     long, or single, this is an illegal instruction: */
  if (ea_mode == 1
      || (ea_mode == 0
	  && destination_format != TME_M6888X_TYPE_BYTE
	  && destination_format != TME_M6888X_TYPE_WORD
	  && destination_format != TME_M6888X_TYPE_LONG
	  && destination_format != TME_M6888X_TYPE_SINGLE)) {
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
  }

  /* for the effective address predecrement and postincrement modes,
     and for the integer conversions, we require that these size
     macros correspond exactly to the number of bytes: */
#if TME_M68K_SIZE_8 != 1
#error "TME_M68K_SIZE_8 must be 1"
#endif
#if TME_M68K_SIZE_16 != 2
#error "TME_M68K_SIZE_16 must be 2"
#endif
#if TME_M68K_SIZE_32 != 4
#error "TME_M68K_SIZE_32 must be 4"
#endif
#if TME_M68K_SIZE_64 != 8
#error "TME_M68K_SIZE_64 must be 8"
#endif
#if TME_M68K_SIZE_96 != 12
#error "TME_M68K_SIZE_96 must be 12"
#endif
#define TME_M68K_AREG_INCREMENT(areg, size) \
  ((size) + (((size) == TME_M68K_SIZE_8 && (areg) == TME_M68K_IREG_A7) ? 1 : 0))

  /* dispatch on the destination format to get the size of the destination: */
  switch (destination_format) {
  case TME_M6888X_TYPE_BYTE: ea_size = TME_M68K_SIZE_8; break;
  case TME_M6888X_TYPE_WORD: ea_size = TME_M68K_SIZE_16; break;
  case TME_M6888X_TYPE_LONG: /* FALLTHROUGH */
  case TME_M6888X_TYPE_SINGLE: ea_size = TME_M68K_SIZE_32; break;
  case TME_M6888X_TYPE_DOUBLE: ea_size = TME_M68K_SIZE_64; break;
  default: assert(FALSE);
  case TME_M6888X_TYPE_PACKEDDEC: /* FALLTHROUGH */
  case TME_M6888X_TYPE_PACKEDDEC_DK: /* FALLTHROUGH */
  case TME_M6888X_TYPE_EXTENDED80: ea_size = TME_M68K_SIZE_96; break;
  }

  /* if we're not restarting: */
  if (!TME_M68K_SEQUENCE_RESTARTING) {

    /* do the common fpgen setup: */
    _tme_m6888x_fpgen_enter(ic, &_tme_m6888x_fpgen_fmove_rm);

    /* get the source register: */
    src = &ic->tme_m68k_fpu_fpreg[TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 7, 3)];

    /* check for a NaN operand: */
    src_is_nan = tme_ieee754_extended80_check_nan_monadic(&ic->tme_m68k_fpu_ieee754_ctl, src, &src_buffer);
    if (src_is_nan) {
      src = &src_buffer;
    }

    /* assume that the source is the destination: */
    dst = src;
    dst_formats = TME_FLOAT_FORMAT_IEEE754_EXTENDED80 | TME_FLOAT_FORMAT_IEEE754_EXTENDED80_BUILTIN;

    /* dispatch on the destination format: */
    switch (destination_format) {

    case TME_M6888X_TYPE_BYTE:
    case TME_M6888X_TYPE_WORD:
    case TME_M6888X_TYPE_LONG:
      if (src_is_nan) {
	/* XXX how is a NaN converted into an integer? */
	value_int32 = -1;
	_tme_m6888x_exception(ic, TME_M6888X_FPSR_EXC_OPERR);
      }
      else {
	TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_extended80_to_int32, src, &value_int32_raw);
	value_int32 = TME_MIN(value_int32_raw, (2147483647 / (1L << (8 * (TME_M68K_SIZE_32 - ea_size)))));
	value_int32 = TME_MAX(value_int32, ((-1073741824 * 2) / (1L << (8 * (TME_M68K_SIZE_32 - ea_size)))));
	if (tme_ieee754_extended80_is_inf(src)
	    || value_int32 != value_int32_raw) {
	  _tme_m6888x_exception(ic, TME_M6888X_FPSR_EXC_OPERR);
	}
      }
      ic->tme_m68k_ireg_memx32 = value_int32;
      break;

    case TME_M6888X_TYPE_SINGLE:
      TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_single_from_extended80, src, &dst_buffer);
      ic->tme_m68k_ireg_memx32 = *tme_ieee754_single_value_get(&dst_buffer, &single_buffer);
      dst = &dst_buffer;
      dst_formats = TME_FLOAT_FORMAT_IEEE754_SINGLE | TME_FLOAT_FORMAT_IEEE754_SINGLE_BUILTIN;
      break;

    case TME_M6888X_TYPE_DOUBLE:
      TME_M6888X_IEEE754_OP_MONADIC(tme_ieee754_ops_double_from_extended80, src, &dst_buffer);
      value64 = tme_ieee754_double_value_get(&dst_buffer, &value64_buffer);
      ic->tme_m68k_ireg_memx32 = value64->tme_value64_uint32_hi;
      ic->tme_m68k_ireg_memy32 = value64->tme_value64_uint32_lo;
      dst = &dst_buffer;
      dst_formats = TME_FLOAT_FORMAT_IEEE754_DOUBLE | TME_FLOAT_FORMAT_IEEE754_DOUBLE_BUILTIN;
      break;

    case TME_M6888X_TYPE_EXTENDED80:
      extended80 = tme_ieee754_extended80_value_get(src, &extended80_buffer);
      ic->tme_m68k_ireg_memx32 = extended80->tme_float_ieee754_extended80_sexp << 16;
      ic->tme_m68k_ireg_memy32 = extended80->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi;
      ic->tme_m68k_ireg_memz32 = extended80->tme_float_ieee754_extended80_significand.tme_value64_uint32_lo;
      break;

    default:
      assert(FALSE);
      /* FALLTHROUGH */

    case TME_M6888X_TYPE_PACKEDDEC:
    case TME_M6888X_TYPE_PACKEDDEC_DK:

      /* we punt on the packed-decimal format for now: */
      TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
      break;    
    }

    /* set the floating-point condition codes: */
    _tme_m6888x_fpcc(ic, dst, dst_formats);
  }
      
  /* if this is a data register direct EA: */
  if (ea_mode == 0) {

    switch (ea_size) {
    case TME_M68K_SIZE_8:
      ic->tme_m68k_ireg_uint8(ea_reg << 2) = ic->tme_m68k_ireg_memx32;
      break;

    case TME_M68K_SIZE_16:
      ic->tme_m68k_ireg_uint8(ea_reg << 1) = ic->tme_m68k_ireg_memx32;
      break;

    default:
      assert (FALSE);
      /* FALLTHROUGH */

    case TME_M68K_SIZE_32:
      ic->tme_m68k_ireg_uint32(ea_reg) = ic->tme_m68k_ireg_memx32;
      break;
    }
  }

  /* otherwise, this is a memory EA: */
  else {

    /* this instruction can fault: */
    TME_M68K_INSN_CANFAULT;

    /* adjust ea_reg to reference the address register: */
    ea_reg += TME_M68K_IREG_A0;

    /* address register indirect postincrement: */
    if (ea_mode == 3) {
      /* if we are not restarting, set the effective address: */
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ea_reg);
	ic->tme_m68k_ireg_uint32(ea_reg) += TME_M68K_AREG_INCREMENT(ea_reg, ea_size);
      }
    }

    /* address register indirect predecrement: */
    else if (ea_mode == 4) {
      /* if we are not restarting, set the effective address: */
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->tme_m68k_ireg_uint32(ea_reg) -= TME_M68K_AREG_INCREMENT(ea_reg, ea_size);
	ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ea_reg);
      }
    }

    /* dispatch on the operand size to write in the destination as one
       or more 32-bit words.  we will write up to three 32-bit words
       from memx, memy, and memz: */
    switch (ea_size) {

      /* this can only happen when the source operand is a byte: */
    case TME_M68K_SIZE_8:
      tme_m68k_write_memx8(ic);
      assert (!TME_M68K_SEQUENCE_RESTARTING);
      break;

      /* this can only happen when the source operand is a word: */
    case TME_M68K_SIZE_16:
      tme_m68k_write_memx16(ic);
      assert (!TME_M68K_SEQUENCE_RESTARTING);
      break;

      /* everything else is one or more 32-bit words: */
    default:

      /* write the first 32 bits from the memx register: */
      tme_m68k_write_memx32(ic);
      if (ea_size == TME_M68K_SIZE_32) {
	break;
      }

      /* write the second 32 bits from the memy register: */
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
      }
      tme_m68k_write_mem32(ic, TME_M68K_IREG_MEMY32);
      if (ea_size == TME_M68K_SIZE_64) {
	break;
      }
	  
      /* write the third 32 bits from the memz register: */
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
      }
      tme_m68k_write_mem32(ic, TME_M68K_IREG_MEMZ32);
      break;
    }
  }

  TME_M68K_INSN_OK;

#undef TME_M68K_AREG_INCREMENT
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_fmovem)
{  
  unsigned int ea_mode;
  unsigned int ea_reg;
  unsigned int register_to_memory;
  tme_uint16_t mask;
  unsigned int bit;
  unsigned int first_register;
  struct tme_float *fpreg;
  const struct tme_float_ieee754_extended80 *extended80;
  struct tme_float_ieee754_extended80 extended80_buffer;

  TME_M68K_INSN_FPU;

  /* get the EA mode and register fields: */
  ea_mode = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3);
  ea_reg = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);

  /* get the register-to-memory flag: */
  register_to_memory = (TME_M68K_INSN_SPECOP & TME_BIT(13)) != 0;

  /* immediate EAs must have already been caught as illegal instructions: */
  assert (!(ea_mode == 7 && ea_reg == 4));

  /* if this is a data register direct EA or an address register
     direct EA, or if this is a predecrement EA and this is a
     memory-to-register operation, or if this is a postincrement EA
     and this is a register-to-memory operation, this is an illegal
     instruction: */
  if (ea_mode == 0
      || ea_mode == 1
      || (ea_mode == 4
	  && !register_to_memory)
      || (ea_mode == 3
	  && register_to_memory)) {
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
  }

  /* get the register list: */
  mask = TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 8);

  /* if the register list is dynamic: */
  if (TME_M68K_INSN_SPECOP & TME_BIT(11)) {

    /* the mask field is supposed to contain only a data register
       number: */
    if (mask & 0x8f) {
      TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
    }

    /* get the dynamic register list: */
    mask = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(mask, 4, 3));
  }

  /* get the FP register corresponding to bit 7 in the mask: */
  if (TME_M68K_INSN_SPECOP & TME_BIT(12)) {
    first_register = 0;
  }
  else {

    /* this must be a predecrement EA: */
    if (ea_mode != 4) {
      TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
    }

    first_register = 7;
  }

  /* if the mask is empty, return now: */
  if (mask == 0) {
    TME_M68K_INSN_OK;
  }

  /* this instruction can fault: */
  TME_M68K_INSN_CANFAULT;

  /* we require that TME_M68K_SIZE_96 be 12: */
#if TME_M68K_SIZE_96 != 12
#error "TME_M68K_SIZE_96 must be 12"
#endif

  /* loop over the bits in the mask: */
  for (bit = 0; bit < 8; bit++, mask <<= 1) {

    /* skip this register if its bit isn't set in the mask: */
    if (!(mask & 0x80)) {
      continue;
    }

    /* get this register: */
    fpreg = &ic->tme_m68k_fpu_fpreg[bit ^ first_register];

    /* if this is a register-to-memory operation: */
    if (register_to_memory) {

      /* if this is a predecrement EA, and we're not restarting,
         predecrement the EA: */
      if (!TME_M68K_SEQUENCE_RESTARTING
	  && ea_mode == 4) {
	ic->_tme_m68k_ea_address = (ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ea_reg) -= TME_M68K_SIZE_96);
      }

      /* write out the register: */
      extended80 = tme_ieee754_extended80_value_get(fpreg, &extended80_buffer);
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->tme_m68k_ireg_memx32 = extended80->tme_float_ieee754_extended80_sexp << 16;
      }
      tme_m68k_write_memx32(ic);
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
	ic->tme_m68k_ireg_memx32 = extended80->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi;
      }
      tme_m68k_write_memx32(ic);
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
	ic->tme_m68k_ireg_memx32 = extended80->tme_float_ieee754_extended80_significand.tme_value64_uint32_lo;
      }
      tme_m68k_write_memx32(ic);
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
      }
    }

    /* otherwise, this is a memory-to-register operation: */
    else {

      /* read in this register: */
      tme_m68k_read_memx32(ic);
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	fpreg->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_sexp = (ic->tme_m68k_ireg_memx32 >> 16);
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
      }
      tme_m68k_read_memx32(ic);
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	fpreg->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi = ic->tme_m68k_ireg_memx32;
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
      }
      tme_m68k_read_memx32(ic);
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	fpreg->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = ic->tme_m68k_ireg_memx32;
	ic->_tme_m68k_ea_address += TME_M68K_SIZE_32;
	fpreg->tme_float_format = TME_FLOAT_FORMAT_IEEE754_EXTENDED80;
      }
    }
  }

  /* if this is the postincrement addressing mode: */
  if (ea_mode == 3) {

    /* update the address register: */
    assert (!TME_M68K_SEQUENCE_RESTARTING);
    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ea_reg) = ic->_tme_m68k_ea_address;
  }

  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_fmovemctl)
{
  tme_uint16_t mask;
  unsigned int ea_mode;
  unsigned int ea_reg;
  unsigned int register_to_memory;
  unsigned int bit;
  tme_uint32_t *value;

  TME_M68K_INSN_FPU;

  /* get the register mask: */
  mask = TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 10, 3);

  /* get the EA mode and register fields: */
  ea_mode = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3);
  ea_reg = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);

  /* get the register-to-memory flag: */
  register_to_memory = (TME_M68K_INSN_SPECOP & TME_BIT(13)) != 0;

  /* if no registers have been selected, or if this is a data register
     direct EA and multiple registers have been selected, or if this
     is an address register direct EA and the floating point
     instruction address register is not the single register selected,
     this is an illegal instruction: */
  if (mask == 0
      || (ea_mode == 0
	  && ((mask & (mask - 1)) != 0))
      || (ea_mode == 1
	  && mask != 1)) {
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
  }

  /* if this isn't a data register direct EA or an address register
     direct EA, this instruction can fault: */
  if (ea_mode != 0
      && ea_mode != 1) {
    TME_M68K_INSN_CANFAULT;
  }

  /* if we're not restarting, and this is the predecrement addressing mode: */
  if (!TME_M68K_SEQUENCE_RESTARTING
      && ea_mode == 4) {

    /* update the effective address: */
    for (; mask != 0; ic->_tme_m68k_ea_address -= sizeof(tme_uint32_t), mask &= (mask - 1));

    /* update the address register: */
    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ea_reg) = ic->_tme_m68k_ea_address;
  }

  /* get the register mask: */
  mask = TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 10, 3);

  /* loop over the register mask bits: */
  for (bit = 3; bit-- > 0; ) {

    /* ignore this register if its bit isn't set: */
    if (!(mask & (1 << bit))) {
      continue;
    }

    /* get a pointer to this register's value: */
    value = (bit == 2
	     ? &ic->tme_m68k_fpu_fpcr
	     : bit == 1
	     ? &ic->tme_m68k_fpu_fpsr
	     : &ic->tme_m68k_fpu_fpiar);

    /* transfer this register's value: */

    /* if this is a data register direct EA: */
    if (ea_mode == 0) {
      if (register_to_memory) {
	ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0 + ea_reg) = *value;
      }
      else {
	*value = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0 + ea_reg);
      }
    }

    /* if this is an address register direct EA: */
    else if (ea_mode == 1) {
      if (register_to_memory) {
	ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ea_reg) = *value;
      }
      else {
	*value = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ea_reg);
      }
    }

    /* otherwise, this is a memory EA: */
    else {
      if (register_to_memory) {
	if (!TME_M68K_SEQUENCE_RESTARTING) {
	  ic->tme_m68k_ireg_memx32 = *value;
	}
	tme_m68k_write_memx32(ic);
	if (!TME_M68K_SEQUENCE_RESTARTING) {
	  ic->_tme_m68k_ea_address += sizeof(tme_uint32_t);
	}	
      }
      else {
	tme_m68k_read_memx32(ic);
	if (!TME_M68K_SEQUENCE_RESTARTING) {
	  *value = ic->tme_m68k_ireg_memx32;
	  ic->_tme_m68k_ea_address += sizeof(tme_uint32_t);
	}
      }
    }
  }

  /* if this is the postincrement addressing mode: */
  if (ea_mode == 3) {

    /* update the address register: */
    assert (!TME_M68K_SEQUENCE_RESTARTING);
    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0 + ea_reg) = ic->_tme_m68k_ea_address;
  }

  TME_M68K_INSN_OK;
}

/* this evaluates a floating-point predicate: */
static int
_tme_m6888x_predicate_true(struct tme_m68k *ic, tme_uint16_t predicate)
{
  unsigned int cc_nan;
  unsigned int cc_i;
  unsigned int cc_z;
  unsigned int cc_n;

  /* get the condition codes: */
  cc_nan = (ic->tme_m68k_fpu_fpsr & TME_M6888X_FPSR_CC_NAN) != 0;
  cc_i = (ic->tme_m68k_fpu_fpsr & TME_M6888X_FPSR_CC_I) != 0;
  cc_z = (ic->tme_m68k_fpu_fpsr & TME_M6888X_FPSR_CC_Z) != 0;
  cc_n = (ic->tme_m68k_fpu_fpsr & TME_M6888X_FPSR_CC_N) != 0;

  /* if this predicate is greater than 0x1f, this is an illegal instruction: */
  if (predicate > 0x1f) {
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
  }

  /* if this predicate sets BSUN when NaN is set: */
  if (predicate > 0x0f) {

    /* if NaN is set, set BSUN: */
    if (cc_nan) {
      _tme_m6888x_exception(ic, TME_M6888X_FPSR_EXC_BSUN);
    }

    /* adjust predicate to be its non-BSUN-setting version: */
    predicate -= 0x10;
  }

  /* dispatch on the predicate: */
  switch (predicate) {
  default: assert(FALSE);
  case 0x00: predicate = FALSE; break;				/* F, SF */
  case 0x01: predicate =  cc_z; break;				/* EQ, SEQ */
  case 0x02: predicate = !(cc_nan || cc_z || cc_n); break;	/* OGT, GT */
  case 0x03: predicate = cc_z || !(cc_nan || cc_n); break;	/* OGE, GE */
  case 0x04: predicate = cc_n && !(cc_nan || cc_z); break;	/* OLT, LT */
  case 0x05: predicate = cc_z || (cc_n && !cc_nan); break;	/* OLE, LE */
  case 0x06: predicate = !(cc_nan || cc_z); break;		/* OGL, GL */
  case 0x07: predicate = !cc_nan; break;			/* OR, GLE */
  case 0x08: predicate =  cc_nan; break;			/* UN, NGLE */
  case 0x09: predicate =  (cc_nan || cc_z); break;		/* UEQ, NGL */
  case 0x0a: predicate = cc_nan || !(cc_n || cc_z); break;	/* UGT, NLE */
  case 0x0b: predicate = cc_nan || cc_z || !cc_n; break;	/* UGE, NLT */
  case 0x0c: predicate = cc_nan || (cc_n && !cc_z); break;	/* ULT, NGE */
  case 0x0d: predicate =  (cc_nan || cc_z || cc_n); break;	/* ULE, NGT */
  case 0x0e: predicate = !cc_z; break;				/* NE, SNE */
  case 0x0f: predicate = FALSE; break;				/* T, ST */
  }

  return (predicate);
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_fdbcc)
{
  TME_M68K_INSN_FPU;

  if (_tme_m6888x_predicate_true(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 6))) {
    if (--TME_M68K_INSN_OP0(tme_int16_t) != -1) {
      TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_pc
			   + 4
			   + TME_EXT_S16_U32(TME_M68K_INSN_OP1(tme_int16_t)));
    }
  }
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_ftrapcc)
{
  TME_M68K_INSN_FPU;
  if (_tme_m6888x_predicate_true(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 6))) {
    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;
    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_TRAP));
  }
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_fscc)
{
  TME_M68K_INSN_FPU;
  TME_M68K_INSN_OP1(tme_uint8_t) =
    (_tme_m6888x_predicate_true(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 6))
     ? 0xff
     : 0x00);
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_fbcc)
{
  TME_M68K_INSN_FPU;

  if (_tme_m6888x_predicate_true(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 6))) {
    TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_pc
			 + sizeof(tme_uint16_t)
			 + TME_M68K_INSN_OP0(tme_uint32_t));
  }
  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_fsave)
{
  struct tme_m6888x_frame frame;
  tme_uint32_t frame_size;

  TME_M68K_INSN_FPU;
  TME_M68K_INSN_PRIV;
  TME_M68K_INSN_CANFAULT;

  /* zero the frame: */
  memset(&frame, 0, sizeof(frame));

  /* dispatch on the FPU type: */
  switch (ic->tme_m68k_fpu_type) {
  default: assert (FALSE);
  case TME_M68K_FPU_M68881:
    frame.tme_m6888x_frame_version = TME_M6888X_FRAME_VERSION_IDLE_M68881;
    frame.tme_m6888x_frame_size = TME_M6888X_FRAME_SIZE_IDLE_M68881;
    break;
  case TME_M68K_FPU_M68882:
    frame.tme_m6888x_frame_version = TME_M6888X_FRAME_VERSION_IDLE_M68882;
    frame.tme_m6888x_frame_size = TME_M6888X_FRAME_SIZE_IDLE_M68882;
    break;
  case TME_M68K_FPU_M68040:
    frame.tme_m6888x_frame_version = TME_M6888X_FRAME_VERSION_IDLE_M68040;
    frame.tme_m6888x_frame_size = TME_M6888X_FRAME_SIZE_IDLE_M68040;
    break;
  }

  /* if this is the m68881 or m68882: */
  if (ic->tme_m68k_fpu_type & TME_M68K_FPU_M6888X) {

    /* fill in a minimal BIU flags field: */
    frame.tme_m6888x_frame_words[(frame.tme_m6888x_frame_size / sizeof(tme_uint32_t)) - 2] = tme_htobe_u32(0x70000000);
  }

  /* get the total size of the frame: */
  frame_size
    = (sizeof(frame.tme_m6888x_frame_version)
       + sizeof(frame.tme_m6888x_frame_size)
       + sizeof(frame.tme_m6888x_frame_reserved2)
       + frame.tme_m6888x_frame_size);

  /* if we're not restarting, and this is the predecrement addressing
     mode, update the effective address and the address register: */
  if (!TME_M68K_SEQUENCE_RESTARTING
      && TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3) == 4) {
    ic->_tme_m68k_ea_address -= frame_size;
    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0
			     + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3))
      = ic->_tme_m68k_ea_address;
  }

  /* write out the saved frame: */
  tme_m68k_write_mem(ic, (tme_uint8_t *) &frame, frame_size);
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_frestore)
{
  tme_uint8_t frame_version;
  tme_uint8_t frame_size;
  int format_error;

  TME_M68K_INSN_FPU;
  TME_M68K_INSN_PRIV;
  TME_M68K_INSN_CANFAULT;

  /* read in the format word: */
  tme_m68k_read_memx32(ic);
  frame_version = (ic->tme_m68k_ireg_memx32 >> 24) & 0xff;
  frame_size = (ic->tme_m68k_ireg_memx32 >> 16) & 0xff;
  
  /* determine if we have a format error: */
  if (frame_version == TME_M6888X_FRAME_VERSION_NULL) {
    format_error = (frame_size != TME_M6888X_FRAME_SIZE_NULL);
  }
  else {
    switch (ic->tme_m68k_fpu_type) {
    default: assert (FALSE);
    case TME_M68K_FPU_M68881:
      format_error = (frame_version != TME_M6888X_FRAME_VERSION_IDLE_M68881
		      || frame_size != TME_M6888X_FRAME_SIZE_IDLE_M68881);
      break;
    case TME_M68K_FPU_M68882:
      format_error = (frame_version != TME_M6888X_FRAME_VERSION_IDLE_M68882
		      || frame_size != TME_M6888X_FRAME_SIZE_IDLE_M68882);
      break;
    case TME_M68K_FPU_M68040:
      format_error = (frame_version != TME_M6888X_FRAME_VERSION_IDLE_M68040
		      || frame_size != TME_M6888X_FRAME_SIZE_IDLE_M68040);
      break;
    }
  }
  
  /* if we have a format error: */
  if (format_error) {
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_FORMAT));
  }

  /* XXX FIXME - we don't bother reading in the rest of the frame.
     this gives an incomplete emulation: */

  /* if this is the postincrement addressing mode, update the address
     register: */
  if (TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 3, 3) == 3) {
    ic->tme_m68k_ireg_uint32(TME_M68K_IREG_A0
			     + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3))
      += (sizeof(ic->tme_m68k_ireg_memx32)
	  + frame_size);
  }

  /* if this was a NULL frame, reset the FPU: */
  if (frame_version == TME_M6888X_FRAME_VERSION_NULL) {
    tme_m68k_fpu_reset(ic);
  }
}

/* this checks for an FPU argument: */
int
tme_m68k_fpu_new(struct tme_m68k *ic, const char * const *args, int *_arg_i, int *_usage, char **_output)
{
  int arg_i;
  int fpu_type;
  const char *compliance;
  int complete;
  unsigned int opmode_i;
  struct tme_ieee754_ctl *ctl;

  /* get the argument index: */
  arg_i = *_arg_i;
  
  /* if this is not an FPU type, this is not an m6888x argument: */
  if (!TME_ARG_IS(args[arg_i + 0], "fpu-type")) {
    return (FALSE);
  }

  /* you can't specify more than one FPU type: */
  if (ic->tme_m68k_fpu_type != TME_M68K_FPU_NONE) {
    tme_output_append_error(_output, 
			    "%s fpu-type %s",
			    _("multiple"),
			    _("unexpected"));
    *_usage = TRUE;
    return (TRUE);
  }

  /* get the FPU type: */
  if (args[arg_i + 1] == NULL) {
    *_usage = TRUE;
    return (TRUE);
  }
  if (TME_ARG_IS(args[arg_i + 1], "m68881")) {
    fpu_type = TME_M68K_FPU_M68881;
  }
  else if (TME_ARG_IS(args[arg_i + 1], "m68882")) {
    fpu_type = TME_M68K_FPU_M68882;
  }
  else if (TME_ARG_IS(args[arg_i + 1], "m68040")) {
    fpu_type = TME_M68K_FPU_M68040;
  }
  else {
    tme_output_append_error(_output, 
			    "%s fpu-type %s",
			    _("bad"),
			    args[arg_i + 1]);
    *_usage = TRUE;
    return (TRUE);
  }
  ic->tme_m68k_fpu_type = fpu_type;
  arg_i += 2;

  /* the next argument must be a compliance level: */
  compliance = args[arg_i + 1];
  if (!TME_ARG_IS(args[arg_i + 0], "fpu-compliance")
      || compliance == NULL) {
    *_usage = TRUE;
    return (TRUE);
  }
  ic->tme_m68k_fpu_ieee754_ops = tme_ieee754_ops_lookup(compliance);
  if (ic->tme_m68k_fpu_ieee754_ops == NULL) {
    tme_output_append_error(_output, 
			    "%s fpu-compliance %s",
			    _("bad"),
			    compliance);
    *_usage = TRUE;
    return (TRUE);
  }
  arg_i += 2;

  /* see if the operations for this compliance level are complete: */
  complete = TRUE;
  for (opmode_i = 0;
       opmode_i < (sizeof(_tme_m6888x_fpgen_opmode_table) / sizeof(_tme_m6888x_fpgen_opmode_table[0]));
       opmode_i++) {
    if (_tme_m6888x_fpgen_opmode_table[opmode_i].tme_m6888x_fpgen_func_ops_offset != 0
	&& TME_M6888X_IEEE754_OP_FUNC(_tme_m6888x_fpgen_opmode_table[opmode_i].tme_m6888x_fpgen_func_ops_offset) == NULL) {
      complete = FALSE;
      break;
    }
  }

  /* if the next argument is an incomplete disposition: */
  if (TME_ARG_IS(args[arg_i + 0], "fpu-incomplete")) {
    
    if (TME_ARG_IS(args[arg_i + 1], "abort")) {
      ic->tme_m68k_fpu_incomplete_abort = TRUE;
    }
    else if (TME_ARG_IS(args[arg_i + 1], "line-f")) {
      ic->tme_m68k_fpu_incomplete_abort = FALSE;
    }
    else {
      tme_output_append_error(_output, 
			      "%s fpu-incomplete %s",
			      _("bad"),
			      args[arg_i + 1]);
      *_usage = TRUE;
      return (TRUE);
    }
    arg_i += 2;
  }

  /* otherwise, no incomplete disposition is given.  if this
     compliance is incomplete: */
  else if (!complete) {
    tme_output_append_error(_output, 
			    "%s %s %s fpu-incomplete",
			    _("compliance"),
			    compliance,
			    _("is incomplete, needs"));
    *_usage = TRUE;
    return (TRUE);
  }

  /* initialize the IEEE 754 control: */
  ctl = &ic->tme_m68k_fpu_ieee754_ctl;

  /* a private data structure: */
  ctl->tme_ieee754_ctl_private = ic;

  /* the underflow tininess-detection mode: */
  /* XXX FIXME - is this right for the m6888x? */
  ctl->tme_ieee754_ctl_detect_tininess = TME_IEEE754_CTL_DETECT_TININESS_BEFORE_ROUNDING;

  /* the exception function: */
  ctl->tme_ieee754_ctl_exception = _tme_m6888x_exception_ieee754;

  /* we don't check whether or not a value is a NaN when converting it
     from one precision to another: */
  ctl->tme_ieee754_ctl_check_snan_on_conversion = FALSE;

  /* the default generated NaN patterns: */
  ctl->tme_ieee754_ctl_default_nan_single = 0x7fffffff;
  ctl->tme_ieee754_ctl_default_nan_double.tme_value64_uint32_hi = 0x7fffffff;
  ctl->tme_ieee754_ctl_default_nan_double.tme_value64_uint32_lo = 0xffffffff;
  ctl->tme_ieee754_ctl_default_nan_extended80.tme_float_ieee754_extended80_sexp = 0x7fff;
  ctl->tme_ieee754_ctl_default_nan_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi = 0xffffffff;
  ctl->tme_ieee754_ctl_default_nan_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo = 0xffffffff;

  /* NaN tests: */
  ctl->tme_ieee754_ctl_is_snan_extended80 = _tme_m6888x_is_snan_extended80;

  /* NaN canonicalization: */
  ctl->tme_ieee754_ctl_nan_single_to_common = tme_ieee754_default_nan_single_to_common;
  ctl->tme_ieee754_ctl_nan_common_to_single = tme_ieee754_default_nan_common_to_single;
  ctl->tme_ieee754_ctl_nan_double_to_common = tme_ieee754_default_nan_double_to_common;
  ctl->tme_ieee754_ctl_nan_common_to_double = tme_ieee754_default_nan_common_to_double;
  ctl->tme_ieee754_ctl_nan_extended80_to_common = tme_ieee754_default_nan_extended80_to_common;
  ctl->tme_ieee754_ctl_nan_common_to_extended80 = tme_ieee754_default_nan_common_to_extended80;

  /* NaN propagation: */
  ctl->tme_ieee754_ctl_nan_from_nans_extended80 = _tme_m6888x_nan_from_nans_extended80;

  /* done: */
  *_arg_i = arg_i;
  return (TRUE);
}

/* this returns the FPU usage: */
void
tme_m68k_fpu_usage(char **_output)
{
  tme_output_append_error(_output, 
			  "[ fpu-type { m68881 | m68882 | m68040 } fpu-compliance %s [ fpu-incomplete { abort | line-f } ] ]",
			  tme_ieee754_compliance_options);
}
