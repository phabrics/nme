/* $Id: sparc-fpu.c,v 1.5 2010/06/05 16:08:44 fredette Exp $ */

/* ic/sparc/sparc-fpu.c - SPARC floating-point unit implementation */

/*
 * Copyright (c) 2005 Matt Fredette
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
_TME_RCSID("$Id: sparc-fpu.c,v 1.5 2010/06/05 16:08:44 fredette Exp $");

/* includes: */
#include "sparc-impl.h"

/* macros: */

/* these invoke an IEEE 754 operation: */
#define _TME_SPARC_FPU_BEGIN				\
  do {							\
    /* at this point, the only possible trap must be	\
       TME_SPARC_FSR_FTT_IEEE754_exception, so we can	\
       clear CEXC: */					\
    ic->tme_sparc_fpu_fsr &= ~TME_SPARC_FSR_CEXC;	\
  } while (/* CONSTCOND */ 0)
#define _TME_SPARC_FPU_OP(func, x)			\
  do {							\
    if (__tme_predict_false((func) == NULL)) {		\
      if (ic->tme_sparc_fpu_incomplete_abort) {		\
        abort();					\
      }							\
      tme_sparc_fpu_exception(ic, TME_SPARC_FSR_FTT_unimplemented_FPop);\
    }							\
    _TME_SPARC_FPU_BEGIN;				\
    (*(func)) x;					\
  } while (/* CONSTCOND */ 0)
#define _TME_SPARC_FPU_OP_MONADIC(func, src, dst) 	\
  _TME_SPARC_FPU_OP(ic->tme_sparc_fpu_ieee754_ops->func, (&ic->tme_sparc_fpu_ieee754_ctl, src, dst))
#define _TME_SPARC_FPU_OP_DYADIC(func, src0, src1, dst)	\
  _TME_SPARC_FPU_OP(ic->tme_sparc_fpu_ieee754_ops->func, (&ic->tme_sparc_fpu_ieee754_ctl, src0, src1, dst))

/* globals: */

/* the floating-point condition codes->conditions mapping.  this
   array is indexed by the fcc value: */
const tme_uint8_t _tme_sparc_conds_fcc[4] = {

  /* E: */
  (0),

  /* L: */
  (TME_BIT(1)		/* fbne */
   | TME_BIT(2)		/* fblg */
   | TME_BIT(3)		/* fbul */
   | TME_BIT(4)),	/* fbl */

  /* G: */
  (TME_BIT(1)		/* fbne */
   | TME_BIT(2)		/* fblg */
   | TME_BIT(5)		/* fbug */
   | TME_BIT(6)),	/* fbg */

  /* U: */
  (TME_BIT(1)		/* fbne */
   | TME_BIT(3)		/* fbul */
   | TME_BIT(5)		/* fbug */
   | TME_BIT(7))	/* fbu */
};

/* this resets the FPU: */
void
tme_sparc_fpu_reset(struct tme_sparc *ic)
{
  unsigned int fp_i;
  
  /* put nonsignaling NaNs in the floating-point data registers: */
  for (fp_i = 0;
       fp_i < TME_ARRAY_ELS(ic->tme_sparc_fpu_fpregs);
       fp_i++) {
    ic->tme_sparc_fpu_fpregs[fp_i].tme_float_format = TME_FLOAT_FORMAT_IEEE754_SINGLE;
    ic->tme_sparc_fpu_fpregs[fp_i].tme_float_value_ieee754_single = ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_default_nan_single;
    ic->tme_sparc_fpu_fpreg_sizes[fp_i] = sizeof(tme_uint32_t) / sizeof(tme_uint32_t);
  }

  /* zero the FSR, except for the version field: */
  ic->tme_sparc_fpu_fsr &= TME_SPARC_FSR_VER;

  /* use the strict compliance operations: */
  ic->tme_sparc_fpu_ieee754_ops = ic->tme_sparc_fpu_ieee754_ops_strict;

  /* the FPU is in execute mode: */
  ic->tme_sparc_fpu_mode = TME_SPARC_FPU_MODE_EXECUTE;
}

/* this enables or disables FPU strict compliance: */
void
tme_sparc_fpu_strict(struct tme_sparc_bus_connection *conn_sparc, unsigned int strict)
{
  struct tme_sparc *ic;

  /* recover our IC: */
  ic = conn_sparc->tme_sparc_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  ic->tme_sparc_fpu_ieee754_ops
    = (strict
       ? ic->tme_sparc_fpu_ieee754_ops_strict
       : ic->tme_sparc_fpu_ieee754_ops_user);
}

/* this handles a sparc FPU exception: */
static void
tme_sparc_fpu_exception(struct tme_sparc *ic, tme_uint32_t ftt)
{

  /* the FPU must be in execute mode, and the FQ must be empty: */
  assert (ic->tme_sparc_fpu_mode == TME_SPARC_FPU_MODE_EXECUTE
	  && (ic->tme_sparc_fpu_fsr & TME_SPARC_FSR_QNE) == 0);

  /* put the trapping instruction in the FQ: */
  ic->tme_sparc_fpu_fq[0].tme_sparc_trapqueue_address
    = (TME_SPARC_VERSION(ic) < 9
       ? ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
       : ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC));
  ic->tme_sparc_fpu_fq[0].tme_sparc_trapqueue_insn
    = TME_SPARC_INSN;

  /* set QNE and the FTT field in the FSR: */
  ic->tme_sparc_fpu_fsr
    = ((ic->tme_sparc_fpu_fsr & ~TME_SPARC_FSR_FTT)
       | TME_SPARC_FSR_QNE
       | ftt);

  /* enter pending exception mode and redispatch to run the next
     instruction: */
  ic->tme_sparc_fpu_mode = TME_SPARC_FPU_MODE_EXCEPTION_PENDING;
  tme_sparc_redispatch(ic);
}

/* this checks for a pending sparc FPU trap: */
void
tme_sparc_fpu_exception_check(struct tme_sparc *ic)
{
  
  /* if the FPU is in pending exception mode: */
  if (ic->tme_sparc_fpu_mode == TME_SPARC_FPU_MODE_EXCEPTION_PENDING) {
    
    /* enter exception mode and start IU trap processing: */
    ic->tme_sparc_fpu_mode = TME_SPARC_FPU_MODE_EXCEPTION;
    TME_SPARC_INSN_TRAP(TME_SPARC_VERSION(ic) < 9
			? TME_SPARC32_TRAP_fp_exception
			: (ic->tme_sparc_fpu_fsr & TME_SPARC_FSR_FTT) != TME_SPARC_FSR_FTT_IEEE754_exception
			? TME_SPARC64_TRAP_fp_exception_other
			: TME_SPARC64_TRAP_fp_exception_ieee_754);
  }

  /* otherwise, the FPU must be in exception mode: */
  assert (ic->tme_sparc_fpu_mode == TME_SPARC_FPU_MODE_EXCEPTION);

  /* "If an FPop, floating-point load instruction, or floating-point
     branch instruction is executed while the FPU is in fp_exception
     state, the FPU returns to fp_exception_pending state and also
     sets the FSR ftt field to sequence_error. The instruction that
     caused the sequence_error is not entered into the FQ." */
  /* XXX FIXME - apparently such an instruction does not trap
     immediately, but since it can't run either, I interpret this to
     mean that the instruction is simply ignored: */
  ic->tme_sparc_fpu_fsr = (ic->tme_sparc_fpu_fsr & ~TME_SPARC_FSR_FTT) | TME_SPARC_FSR_FTT_sequence_error;
  ic->tme_sparc_fpu_mode = TME_SPARC_FPU_MODE_EXCEPTION;
  tme_sparc_redispatch(ic);
}

/* the IEEE 754 exception handler: */
static void
_tme_sparc_fpu_exception_ieee754(struct tme_ieee754_ctl *ctl, tme_int8_t exception_ieee754)
{
  struct tme_sparc *ic;
  tme_uint32_t exception_cexc;

  /* map the IEEE754 exception(s) to CEXC bit(s): */
  exception_cexc = 0;
#define _TME_SPARC_FPU_EXCEPTION_MAP(e_ieee754, e_sparc)	\
  if (((tme_uint8_t) exception_ieee754) & (e_ieee754))		\
    exception_cexc |= (e_sparc)
  _TME_SPARC_FPU_EXCEPTION_MAP(TME_FLOAT_EXCEPTION_INVALID,   TME_SPARC_FSR_CEXC_NVC);
  _TME_SPARC_FPU_EXCEPTION_MAP(TME_FLOAT_EXCEPTION_DIVBYZERO, TME_SPARC_FSR_CEXC_DZC);
  _TME_SPARC_FPU_EXCEPTION_MAP(TME_FLOAT_EXCEPTION_OVERFLOW,  TME_SPARC_FSR_CEXC_OFC);
  _TME_SPARC_FPU_EXCEPTION_MAP(TME_FLOAT_EXCEPTION_UNDERFLOW, TME_SPARC_FSR_CEXC_UFC);
  _TME_SPARC_FPU_EXCEPTION_MAP(TME_FLOAT_EXCEPTION_INEXACT,   TME_SPARC_FSR_CEXC_NXC);
  if (exception_cexc == 0) {
    abort();
  }

  /* recover our data structure: */
  ic = (struct tme_sparc *) ctl->tme_ieee754_ctl_private;
  
  /* set the CEXC field in the FSR.  "When a floating-point trap
     occurs ... The value of aexc is unchanged", which is why we don't
     update it until we're sure a trap is not going to occur: */
  TME_FIELD_MASK_DEPOSITU(ic->tme_sparc_fpu_fsr, TME_SPARC_FSR_CEXC, exception_cexc); 

  /* if any of the new exceptions are unmasked, take the exception: */
  if (TME_FIELD_MASK_EXTRACTU(ic->tme_sparc_fpu_fsr, TME_SPARC_FSR_TEM) & exception_cexc) {

    /* unlock any lock: */
    if (ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_lock_unlock != NULL) {
      (*ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_lock_unlock)();
      ic->tme_sparc_fpu_ieee754_ctl.tme_ieee754_ctl_lock_unlock = NULL;
    }

    /* take the exception: */
    tme_sparc_fpu_exception(ic, TME_SPARC_FSR_FTT_IEEE754_exception);
  }

  /* now that we're sure a trap isn't going to happen, update the AEXC
     field in the FSR: */
  ic->tme_sparc_fpu_fsr |= exception_cexc * (TME_SPARC_FSR_AEXC / TME_SPARC_FSR_CEXC);
}

TME_SPARC_FORMAT3(tme_sparc32_stdfq, tme_uint32_t)
{
  TME_SPARC_INSN_PRIV;
  TME_SPARC_INSN_FPU_ENABLED;

  /* "An attempt to execute STDFQ on an implementation without a
     floating-point queue causes an fp_exception trap with FSR.ftt set
     to 4 (sequence_error). On an implementation with a floating-point
     queue, an attempt to execute STDFQ when the FQ is empty (FSR.qne
     = 0) should cause an fp_exception trap with FSR.ftt set to 4
     (sequence_error)." */
  if ((ic->tme_sparc_fpu_fsr & TME_SPARC_FSR_QNE) == 0) {
    assert (ic->tme_sparc_fpu_mode == TME_SPARC_FPU_MODE_EXECUTE);
    tme_sparc_fpu_exception(ic, TME_SPARC_FSR_FTT_sequence_error);
  }
  assert (ic->tme_sparc_fpu_mode == TME_SPARC_FPU_MODE_EXCEPTION);

  /* store the FQ entry: */
  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX + 0)
    = ic->tme_sparc_fpu_fq[0].tme_sparc_trapqueue_address;
  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX + 1)
    = ic->tme_sparc_fpu_fq[0].tme_sparc_trapqueue_insn;
  tme_sparc32_std(ic, _rs1, _rs2, &ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_FPX));

  /* clear the QNE bit and return to execute mode: */
  ic->tme_sparc_fpu_fsr &= ~TME_SPARC_FSR_QNE;
  ic->tme_sparc_fpu_mode = TME_SPARC_FPU_MODE_EXECUTE;

  TME_SPARC_INSN_OK;
}

/* this decodes a floating point register number and checks that it is
   aligned for a certain format: */
unsigned int
tme_sparc_fpu_fpreg_decode(struct tme_sparc *ic,
			   unsigned int fpreg_number_encoded,
			   unsigned int fpreg_format)
{
  unsigned int fpreg_number;

  /* assume that the register number and its encoding are the same: */
  fpreg_number = fpreg_number_encoded;
 
  /* if this is a double or quad-precision register number encoding: */
#if (TME_IEEE754_FPREG_FORMAT_SINGLE != 1 || TME_IEEE754_FPREG_FORMAT_DOUBLE != 2 || TME_IEEE754_FPREG_FORMAT_QUAD != 4)
#error "bad TME_IEEE754_FPREG_FORMAT_ macros"
#endif
  if (fpreg_format
      & (TME_IEEE754_FPREG_FORMAT_DOUBLE
	 | TME_IEEE754_FPREG_FORMAT_QUAD)) {

    /* on a v9 CPU, bit zero of a double- or quad-precision register
       number encoding is bit five of the register number: */
    if (TME_SPARC_VERSION(ic) >= 9) {
      fpreg_number
	= ((fpreg_number_encoded
	    + (fpreg_number_encoded * 32))
	   & (64 - 2));
    }
  }

  /* if the register number is misaligned for the format: */
#if (TME_IEEE754_FPREG_FORMAT_SINGLE != 1 || TME_IEEE754_FPREG_FORMAT_DOUBLE != 2 || TME_IEEE754_FPREG_FORMAT_QUAD != 4)
#error "bad TME_IEEE754_FPREG_FORMAT_ macros"
#endif
  if (__tme_predict_false(fpreg_number
			  & ((fpreg_format
			      & (TME_IEEE754_FPREG_FORMAT_SINGLE
				 | TME_IEEE754_FPREG_FORMAT_DOUBLE
				 | TME_IEEE754_FPREG_FORMAT_QUAD))
			     - 1))) {

    /* if this CPU allows misaligned register numbers: */
    if (ic->tme_sparc_fpu_flags & TME_SPARC_FPU_FLAG_OK_REG_MISALIGNED) {

      /* force the register number to be aligned: */
      fpreg_number
	&= (0 - (fpreg_format
		 & (TME_IEEE754_FPREG_FORMAT_SINGLE
		    | TME_IEEE754_FPREG_FORMAT_DOUBLE
		    | TME_IEEE754_FPREG_FORMAT_QUAD)));
    }

    /* otherwise, this CPU does not allow misaligned register numbers: */
    else {
      tme_sparc_fpu_exception(ic, TME_SPARC_FSR_FTT_invalid_fp_register);
    }
  }

  return (fpreg_number);
}

/* this forces the given floating point register to assume the given
   format (width, really) in the register file: */
void
tme_sparc_fpu_fpreg_format(struct tme_sparc *ic,
			   unsigned int fpreg_number,
			   unsigned int fpreg_format)
{

  /* make sure the register is in the given format: */
  tme_ieee754_fpreg_format(ic->tme_sparc_fpu_fpregs,
			   ic->tme_sparc_fpu_fpreg_sizes,
			   fpreg_number,
			   (fpreg_format
			    | TME_IEEE754_FPREG_FORMAT_ENDIAN_BIG));
}

/* this forces the given floating point register to assume the given
   format (width, really) in the register file, then returns a pointer
   to the register: */
static const struct tme_float *
tme_sparc_fpu_fpreg_read(struct tme_sparc *ic,
			 tme_uint32_t fpreg_number_mask,
			 unsigned int fpreg_format)
{
  tme_uint32_t fpreg_number_encoded;
  unsigned int fpreg_number;

  /* extract and decode the register number: */
  fpreg_number_encoded = TME_SPARC_INSN;
#if TME_SPARC_FORMAT3_MASK_RD < TME_SPARC_FORMAT3_MASK_RS1 || TME_SPARC_FORMAT3_MASK_RS1 < TME_SPARC_FORMAT3_MASK_RS2
#error "TME_SPARC_FORMAT3_MASK_RS values changed"
#endif
  assert (fpreg_number_mask == TME_SPARC_FORMAT3_MASK_RD
	  || fpreg_number_mask == TME_SPARC_FORMAT3_MASK_RS1
	  || fpreg_number_mask == TME_SPARC_FORMAT3_MASK_RS2);
  if (fpreg_number_mask == TME_SPARC_FORMAT3_MASK_RD) {
    fpreg_number_encoded /= (TME_SPARC_FORMAT3_MASK_RD / TME_SPARC_FORMAT3_MASK_RS2);
  }    
  if (fpreg_number_mask == TME_SPARC_FORMAT3_MASK_RS1) {
    fpreg_number_encoded /= (TME_SPARC_FORMAT3_MASK_RS1 / TME_SPARC_FORMAT3_MASK_RS2);
  }
  fpreg_number_encoded = TME_FIELD_MASK_EXTRACTU(fpreg_number_encoded, TME_SPARC_FORMAT3_MASK_RS2);
  fpreg_number
    = tme_sparc_fpu_fpreg_decode(ic,
				 fpreg_number_encoded,
				 fpreg_format);

  /* make sure the register is in the given format: */
  tme_sparc_fpu_fpreg_format(ic, fpreg_number, fpreg_format);

  /* return a pointer to the register: */
  return (&ic->tme_sparc_fpu_fpregs[fpreg_number]);
}

/* include the automatically generated code: */
#include "sparc-fpu-auto.c"
#ifdef TME_HAVE_INT64_T
#include "sparc-vis-auto.c"
#endif /* TME_HAVE_INT64_T */

/* this checks for an FPU argument: */
int
tme_sparc_fpu_new(struct tme_sparc *ic, const char * const *args, int *_arg_i, int *_usage, char **_output)
{
  int arg_i;
  const char *compliance;
  int complete;
  struct tme_ieee754_ctl *ctl;
  tme_uint32_t ver;

  /* get the argument index: */
  arg_i = *_arg_i;
  
  /* if this is not an FPU type, this is not an sparc FPU argument: */
  if (!TME_ARG_IS(args[arg_i + 0], "fpu-type")) {
    return (FALSE);
  }

  /* you can't specify more than one FPU type: */
  if ((ic->tme_sparc_fpu_fsr & TME_SPARC_FSR_VER) != TME_SPARC_FSR_VER_missing) {
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
  ver = (*ic->_tme_sparc_fpu_ver)(ic, args[arg_i + 1], NULL);
  if (ver == TME_SPARC_FSR_VER_missing) {
    tme_output_append_error(_output, 
			    "%s fpu-type %s",
			    _("bad"),
			    args[arg_i + 1]);
    *_usage = TRUE;
    return (TRUE);
  }
  ic->tme_sparc_fpu_fsr = (ic->tme_sparc_fpu_fsr & ~TME_SPARC_FSR_VER) | ver; 
  arg_i += 2;

  /* the next argument must be a compliance level: */
  compliance = args[arg_i + 1];
  if (!TME_ARG_IS(args[arg_i + 0], "fpu-compliance")
      || compliance == NULL) {
    *_usage = TRUE;
    return (TRUE);
  }
  ic->tme_sparc_fpu_ieee754_ops_user = tme_ieee754_ops_lookup(compliance);
  if (ic->tme_sparc_fpu_ieee754_ops_user == NULL) {
    tme_output_append_error(_output, 
			    "%s fpu-compliance %s",
			    _("bad"),
			    compliance);
    *_usage = TRUE;
    return (TRUE);
  }
  arg_i += 2;

  /* see if the operations for this compliance level are complete: */
#define _TME_SPARC_FPU_OP_CHECK(func) (ic->tme_sparc_fpu_ieee754_ops_user->func != NULL)
  complete
    = (_TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_add)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_div)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_from_double)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_mul)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_mul)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_sub)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_sub)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_sub)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_add)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_div)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_from_single)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_from_single)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_from_single)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_mul)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_sub)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_sub)
       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_sub)
       && ((ic->tme_sparc_fpu_flags & TME_SPARC_FPU_FLAG_NO_FSQRT) != 0
	   || (_TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_sqrt)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_sqrt)
	       && ((ic->tme_sparc_fpu_flags & TME_SPARC_FPU_FLAG_NO_QUAD) != 0
		   || _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_sqrt))))
       && ((ic->tme_sparc_fpu_flags & TME_SPARC_FPU_FLAG_NO_QUAD) != 0
	   || (_TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_double_from_quad)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_add)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_div)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_from_double)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_from_double)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_from_double)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_from_single)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_mul)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_mul)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_sub)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_sub)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_quad_sub)
	       && _TME_SPARC_FPU_OP_CHECK(tme_ieee754_ops_single_from_quad))));
#undef _TME_SPARC_FPU_OP_CHECK

  /* if the next argument is an incomplete disposition: */
  if (TME_ARG_IS(args[arg_i + 0], "fpu-incomplete")) {
    
    if (TME_ARG_IS(args[arg_i + 1], "abort")) {
      ic->tme_sparc_fpu_incomplete_abort = TRUE;
    }
    else if (TME_ARG_IS(args[arg_i + 1], "trap")) {
      ic->tme_sparc_fpu_incomplete_abort = FALSE;
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
  ctl = &ic->tme_sparc_fpu_ieee754_ctl;

  /* a private data structure: */
  ctl->tme_ieee754_ctl_private = ic;

  /* the underflow tininess-detection mode.  Appendix N.5 of my V8
     manual says that this is the mode used on the SPARC: */
  ctl->tme_ieee754_ctl_detect_tininess = TME_IEEE754_CTL_DETECT_TININESS_BEFORE_ROUNDING;

  /* the exception function: */
  ctl->tme_ieee754_ctl_exception = _tme_sparc_fpu_exception_ieee754;

  /* we do check whether or not a value is a sNaN when converting it
     from one precision to another: */
  ctl->tme_ieee754_ctl_check_snan_on_conversion = TRUE;

  /* the default generated NaN patterns: */
  ctl->tme_ieee754_ctl_default_nan_single = 0x7fffffff;
  ctl->tme_ieee754_ctl_default_nan_double.tme_value64_uint32_hi = 0x7fffffff;
  ctl->tme_ieee754_ctl_default_nan_double.tme_value64_uint32_lo = 0xffffffff;
  ctl->tme_ieee754_ctl_default_nan_quad.tme_float_ieee754_quad_hi.tme_value64_uint32_hi = 0x7fffffff;
  ctl->tme_ieee754_ctl_default_nan_quad.tme_float_ieee754_quad_hi.tme_value64_uint32_lo = 0xffffffff;
  ctl->tme_ieee754_ctl_default_nan_quad.tme_float_ieee754_quad_lo.tme_value64_uint32_hi = 0xffffffff;
  ctl->tme_ieee754_ctl_default_nan_quad.tme_float_ieee754_quad_lo.tme_value64_uint32_lo = 0xffffffff;

  /* NaN tests: */
  ctl->tme_ieee754_ctl_is_snan_single = _tme_sparc_fpu_is_snan_single;
  ctl->tme_ieee754_ctl_is_snan_double = _tme_sparc_fpu_is_snan_double;
  ctl->tme_ieee754_ctl_is_snan_quad = _tme_sparc_fpu_is_snan_quad;

  /* NaN canonicalization: */
  ctl->tme_ieee754_ctl_nan_single_to_common = tme_ieee754_default_nan_single_to_common;
  ctl->tme_ieee754_ctl_nan_common_to_single = tme_ieee754_default_nan_common_to_single;
  ctl->tme_ieee754_ctl_nan_double_to_common = tme_ieee754_default_nan_double_to_common;
  ctl->tme_ieee754_ctl_nan_common_to_double = tme_ieee754_default_nan_common_to_double;
  ctl->tme_ieee754_ctl_nan_quad_to_common = tme_ieee754_default_nan_quad_to_common;
  ctl->tme_ieee754_ctl_nan_common_to_quad = tme_ieee754_default_nan_common_to_quad;

  /* NaN propagation: */
  ctl->tme_ieee754_ctl_nan_from_nans_single = _tme_sparc_fpu_nan_from_nans_single;
  ctl->tme_ieee754_ctl_nan_from_nans_double = _tme_sparc_fpu_nan_from_nans_double;
  ctl->tme_ieee754_ctl_nan_from_nans_quad = _tme_sparc_fpu_nan_from_nans_quad;

  /* look up the strict compliance operations: */
  ic->tme_sparc_fpu_ieee754_ops_strict = tme_ieee754_ops_lookup("strict");
  assert (ic->tme_sparc_fpu_ieee754_ops_strict != NULL);

  /* done: */
  *_arg_i = arg_i;
  return (TRUE);
}

/* this returns the FPU usage: */
void
tme_sparc_fpu_usage(struct tme_sparc *ic, char **_output)
{
  tme_output_append_error(_output, 
			  "[ fpu-type ");
  (*ic->_tme_sparc_fpu_ver)(ic, NULL, _output);
  tme_output_append_error(_output,
			  " ] fpu-compliance %s [ fpu-incomplete { abort | trap } ] ]",
			  tme_ieee754_compliance_options);
}
