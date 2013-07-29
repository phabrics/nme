/* $Id: sparc-timing.c,v 1.3 2010/02/14 15:57:09 fredette Exp $ */

/* ic/sparc/sparc-timing.c - SPARC instruction timing support: */

/*
 * Copyright (c) 2009 Matt Fredette
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

/* includes: */
#include "sparc-impl.h"

_TME_RCSID("$Id: sparc-timing.c,v 1.3 2010/02/14 15:57:09 fredette Exp $");

/* macros: */

/* at or below this maximum number of microseconds, we will spin
   instead of yield: */
#define TME_SPARC_TIMING_SPIN_USEC_MAX		(4096)

/* normally, when we yield we do a plain yield so we are immediately
   runnable again.  this makes timing loops more accurate, at the
   expense of consuming the host CPU.  if this is nonzero, when we
   yield we will instead do a sleep or wait on an external event: */
#define TME_SPARC_TIMING_YIELD_BLOCK		(FALSE)

/* this does a timing loop update: */
static void
_tme_sparc_timing_loop_update(struct tme_sparc *ic,
			      tme_sparc_ireg_umax_t update_count_m1)
{
  tme_uint32_t insn_update;
  unsigned long opcode;
  unsigned int reg_rd;
  signed int immediate;
  tme_sparc_ireg_umax_t addend_total_m1;

  /* get the update instruction: */
  insn_update = ic->_tme_sparc_insn;

  /* get the opcode: */
  opcode = TME_FIELD_MASK_EXTRACTU(insn_update, (0x3f << 19));

  /* get the rd register: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(insn_update, TME_SPARC_FORMAT3_MASK_RD);
  TME_SPARC_REG_INDEX(ic, reg_rd);

  /* get the immediate: */
  immediate = insn_update & 2;
  immediate = 1 - immediate;

  /* get the total addend: */
  addend_total_m1 = update_count_m1;
  if (ic->tme_sparc_timing_loop_addend < 0) {
    addend_total_m1 = -addend_total_m1;
  }

  /* if this is a v9 CPU: */
  if (TME_SPARC_VERSION(ic) >= 9) {
#ifdef TME_HAVE_INT64_T

    /* save the immediate: */
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_TMP(0)) = immediate;

    /* do all but one of the updates of the rd register directly: */
    ic->tme_sparc_ireg_uint64(reg_rd) += addend_total_m1;

    /* do the final update, including setting any condition codes: */
    (*(ic->_tme_sparc64_execute_opmap[opcode]))
      (ic, 
       &ic->tme_sparc_ireg_uint64(reg_rd),
       &ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_TMP(0)),
       &ic->tme_sparc_ireg_uint64(reg_rd));

#endif /* TME_HAVE_INT64_T */
  }

  /* otherwise, this is a v7 or v8 CPU: */
  else {

    /* save the immediate: */
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_TMP(0)) = immediate;

    /* do all but one of the updates of the rd register directly: */
    ic->tme_sparc_ireg_uint32(reg_rd) += addend_total_m1;

    /* do the final update, including setting any condition codes: */
    (*(ic->_tme_sparc32_execute_opmap[opcode]))
      (ic, 
       &ic->tme_sparc_ireg_uint32(reg_rd),
       &ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_TMP(0)),
       &ic->tme_sparc_ireg_uint32(reg_rd));
  }
}

/* this returns nonzero if the branch to . instruction and the update
   instruction in its delay slot are a supported timing loop: */
int
tme_sparc_timing_loop_ok(tme_uint32_t insn_branch_dot,
			 tme_uint32_t insn_update)
{
  unsigned int op2;
  tme_uint32_t conds_mask;
  unsigned int cond;

  /* if the update instruction is not an add, addcc, sub, or subcc
     with the i bit set: */
  if ((insn_update
       & ((tme_uint32_t)
	  ((0x3 << 30)		/* format */
	   + (0x2b << 19)	/* op3 (mask addcc to add, sub to add) */
	   + (1 << 13))))	/* i */
      != ((tme_uint32_t)
	  ((0x2 << 30)		/* format */
	   + (0x00 << 19)	/* op3 (add) */
	   + (1 << 13)))) {	/* i */

    /* we only support timing loops with plain add or subtract
       update instructions: */
    return (FALSE);
  }

  /* if the simm13 is not 1 or -1: */
  if (((insn_update
	+ (insn_update & 2))
       & 0x1fff)
      != 1) {

    /* we only support timing loops with plain add or subtract update
       instructions with immediates of 1 or -1: */
    return (FALSE);
  }

  /* if rd is %g0: */
#if TME_SPARC_IREG_G0 != 0
#error "TME_SPARC_IREG_G0 changed"
#endif
  if ((insn_update & TME_SPARC_FORMAT3_MASK_RD) == 0) {

    /* we only support timing loops with plain add or subtract update
       instructions with destination registers other than %g0: */
    return (FALSE);
  }

  /* if rs1 and rd are not the same: */
#if TME_SPARC_FORMAT3_MASK_RD < TME_SPARC_FORMAT3_MASK_RS1
#error "TME_SPARC_FORMAT3_MASK_ values changed"
#endif
  if ((((insn_update
	 / (TME_SPARC_FORMAT3_MASK_RD
	    / TME_SPARC_FORMAT3_MASK_RS1))
	^ insn_update)
       & TME_SPARC_FORMAT3_MASK_RS1) != 0) {

    /* we only support timing loops with plain add or subtract update
       instructions where the source register and destination register
       are the same: */
    return (FALSE);
  }

  /* all branch instructions are format two instructions: */
  assert ((insn_branch_dot & (tme_uint32_t) (0x3 << 30)) == 0);

  /* if this isn't a Bicc or a v9 BPcc instruction: */
  op2 = TME_FIELD_MASK_EXTRACTU(insn_branch_dot, (0x7 << 22));
  if (__tme_predict_false(op2 != 2 && op2 != 1)) {

    /* we support all timing loops with a branch to . instructions
       that don't depend on the integer condition codes: */
    return (TRUE);
  }

  /* otherwise, this is a Bicc or a v9 BPcc instruction: */
  else {

    /* if this is not an addcc or subcc instruction: */
    if (__tme_predict_false((insn_update & (0x10 << 19)) == 0)) {

      /* we support timing loops with Bicc and BPcc instructions even
	 when the update instruction doesn't change the integer
	 condition codes: */
      return (TRUE);
    }

    /* if this is a subcc instruction: */
    if (insn_update & (0x04 << 19)) {

      /* we support timing loops that use subcc with all conditions
	 except for vc and vs (the overflow conditions) and never: */
      conds_mask
	= ((1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_N))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_E))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_LE))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_L))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_LEU))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_CS))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_NEG))
	   + (1 << TME_SPARC_COND_E)
	   + (1 << TME_SPARC_COND_LE)
	   + (1 << TME_SPARC_COND_L)
	   + (1 << TME_SPARC_COND_LEU)
	   + (1 << TME_SPARC_COND_CS)
	   + (1 << TME_SPARC_COND_NEG)
	   );
    }

    /* otherwise, this is an addcc instruction: */
    else {

      /* we support timing loops that use addcc with only these
	 conditions: */
      conds_mask
	= ((1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_N))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_E))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_CS))
	   + (1 << (TME_SPARC_COND_NOT + TME_SPARC_COND_NEG))
	   + (1 << TME_SPARC_COND_E)
	   + (1 << TME_SPARC_COND_CS)
	   + (1 << TME_SPARC_COND_NEG)
	   );
    }

    /* if we don't support the condition: */
    cond = TME_FIELD_MASK_EXTRACTU(insn_branch_dot, (0xf << 25));
    if ((conds_mask & TME_BIT(cond)) == 0) {

      /* we don't support this timing loop: */
      return (FALSE);
    }

    /* otherwise, we support this timing loop: */
    return (TRUE);
  }
}

/* this starts a timing loop: */
static void
_tme_sparc_timing_loop_start(struct tme_sparc *ic,
			     tme_uint32_t insn_update)
{
  unsigned int reg_rd;
  tme_sparc_ireg_umax_t value_rd;
  signed int addend;
  tme_uint32_t insn_branch_dot;
  const struct timeval *sleep;
  unsigned int op2;
  unsigned int cond;
  tme_sparc_ireg_umax_t value_sign;
  tme_sparc_ireg_umax_t value_zero;
  tme_sparc_ireg_umax_t value_true_greatest;
  tme_sparc_ireg_umax_t value_test;
  tme_sparc_ireg_umax_t branch_taken_count_max_m1;
  unsigned int loop_cycles_each;
  tme_sparc_ireg_umax_t cycles_scaled_max;
  union tme_value64 cycles_finish;
  tme_sparc_ireg_umax_t usec;
  tme_uint32_t usec32;
  static struct timeval sleep_buffer;

  /* at this point, the timing loop branch to . has been taken, and
     the PCs have been updated, so both PC and PC_next_next point to
     the timing loop update instruction (in insn_update), and PC_next
     points to the timing loop branch to . instruction again.

     a taken conditional branch never annuls, and sparc-execute.c and
     sparc-rc-insns.c handle a "ba,a ." instruction specially, so we
     know that the update instruction must execute at least as many
     times as the timing loop branch to . is taken.

     the timing loop branch to . has just been taken (this is why
     PC_next_next is the same as PC).  this first take was when the
     branch to . was detected in sparc-execute.c, or when
     tme_sparc_timing_loop_assist() determined that the recode
     instructions thunk that called it did so after a taken branch.

     this very first take is implicit in the taken branch count that
     we compute and store in
     ic->tme_sparc_timing_loop_branch_taken_count_max_m1 and/or pass
     to _tme_sparc_timing_loop_update() - i.e., we always compute the
     taken branch count minus one.

     this is good because it is possible for the timing loop update
     instruction to be executed 2^cc_width times.  if initially %o3 is
     zero and %icc.Z is clear, this bne will be taken 2^32 times:

     bne .
     deccc %o3

     NB that in this specific case, where the timing loop branch to
     . does not annul, the timing loop update instruction will
     actually be run a total of (2^32)+1 times: 2^32 times
     corresponding to the 2^32 times that the branch is taken, plus
     one final time when the branch is *not* taken, but the update
     instruction is not annulled.

     this function only counts and performs the updates corresponding
     to the times that the branch is *taken*.
     _tme_sparc_timing_loop_update() does the count minus one updates
     directly in the destination register, followed by a true
     instruction execution for the last (to update any condition
     codes).

     whether or not the branch to . instruction annuls, and any needed
     "one final time" update instruction will be handled either by
     sparc-execute.c, or by a combination of the recode instructions
     thunk and tme_sparc_timing_loop_assist(): */

  /* NB: our caller has already saved the current host cycles counter
     in ic->tme_sparc_timing_loop_start: */

  /* get the rd register: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(insn_update, TME_SPARC_FORMAT3_MASK_RD);
  TME_SPARC_REG_INDEX(ic, reg_rd);

  /* if this is a v9 CPU: */
  if (TME_SPARC_VERSION(ic) >= 9) {
#ifdef TME_HAVE_INT64_T

    /* get the rd register value: */
    value_rd = ic->tme_sparc_ireg_uint64(reg_rd);

#else  /* !TME_HAVE_INT64_T */

    /* silence uninitialized variable warnings: */
    value_rd = 0;

#endif /* !TME_HAVE_INT64_T */
  }

  /* otherwise, this is not a v9 CPU: */
  else {

    /* get the rd register value: */
    value_rd = (tme_int32_t) ic->tme_sparc_ireg_uint32(reg_rd);
  }

  /* assume that this is an add or addcc instruction: */
  addend = insn_update & 2;
  addend = 1 - addend;

  /* if this is a sub or subcc instruction: */
  if (insn_update & (0x04 << 19)) {

    /* complement the addend: */
    addend = -addend;
  }

  /* get the branch to . instruction: */
  insn_branch_dot = ic->_tme_sparc_insn;

  /* save the update instruction: */
  ic->_tme_sparc_insn = insn_update;

  /* save the addend: */
  ic->tme_sparc_timing_loop_addend = addend;

  /* assume that there isn't a maximum number of times that the branch
     to . can be taken (i.e., that the branch to . doesn't depend on
     the value of rd), as if the branch condition were always: */
  cond = TME_SPARC_COND_NOT + TME_SPARC_COND_N;

  /* assume that if the branch does depend on the value of rd, that
     the sign bit in values of rd is the last bit: */
  value_sign = 1;
  value_sign <<= ((sizeof(value_sign) * 8) - 1);

  /* silence uninitialized variable warnings: */
  value_zero = 0;
  value_true_greatest = 0;

  /* get the op2 field of the branch to . instruction: */
  op2 = TME_FIELD_MASK_EXTRACTU(insn_branch_dot, (0x7 << 22));

  /* if this is a v9 BPr: */
  if (op2 == 3) {

    /* if this BPr tests rd: */
    if (((insn_branch_dot
	  ^ insn_update)
	 & TME_SPARC_FORMAT3_MASK_RS1) == 0) {

      /* get the condition field, and shift the "not" bit from bit two
	 to bit three, to match the other branches: */
      cond = TME_FIELD_MASK_EXTRACTU(insn_branch_dot, (0x7 << 25));
      cond = (cond + 4) & (TME_SPARC_COND_NOT | 3);
      
      /* dispatch on the condition: */
      if ((cond % TME_SPARC_COND_NOT) == TME_SPARC_COND_E) {
	value_zero = -addend;
	value_true_greatest = 0;
      }
      else {
	assert ((cond % TME_SPARC_COND_NOT) == TME_SPARC_COND_LE
		|| (cond % TME_SPARC_COND_NOT) == TME_SPARC_COND_L);
	value_zero = value_sign - addend;
#if (TME_SPARC_COND_L & 1) == 0 || (TME_SPARC_COND_LE & 1) != 0
#error "TME_SPARC_COND_ values changed"
#endif
	value_true_greatest = value_sign - (cond & 1);
      }
    }
  }

  /* otherwise, if this is a Bicc or a v9 BPcc: */
  else if (op2 == 2 || op2 == 1) {

    /* if this is an addcc or subcc instruction: */
    if (insn_update & (0x10 << 19)) {

      /* get the condition: */
      cond = TME_FIELD_MASK_EXTRACTU(insn_branch_dot, (0xf << 25));

      /* if this is a Bicc, or a BPcc with the cc1 bit clear, the
	 sign bit in values of rd is bit 31: */
      if (sizeof(value_sign) > sizeof(tme_uint32_t)
	  && ((insn_branch_dot >> 21) & op2 & 1) == 0) {
	value_sign = (((tme_uint32_t) 1) << 31);
      }

      /* if this is a subcc instruction: */
      if (insn_update & (0x04 << 19)) {

	/* dispatch on the condition: */
	switch (cond % TME_SPARC_COND_NOT) {
	default:
	  /* we should have caught this unsupported condition in
	     tme_sparc_timing_loop_ok(): */
	  assert (FALSE);
	  /* FALLTHROUGH */
	case TME_SPARC_COND_N:
	  /* nothing to do */
	  break;
	case TME_SPARC_COND_E:
	  value_zero = -addend;
	  value_true_greatest = 0;
	  break;
	case TME_SPARC_COND_LE:
	  value_zero = value_sign;
	  value_true_greatest = value_sign - addend;
	  break;
	case TME_SPARC_COND_L:
	  value_zero = value_sign;
	  value_true_greatest = (value_sign - 1) - addend;
	  break;
	case TME_SPARC_COND_LEU:
	  value_zero = 0;
	  value_true_greatest = (value_sign * 2) - addend;
	  break;
	case TME_SPARC_COND_CS:
	  value_zero = 0;
	  value_true_greatest = (value_sign * 2) - (addend + 1);
	  break;
	case TME_SPARC_COND_NEG:
	  value_zero = value_sign - addend;
	  value_true_greatest = value_sign - 1;
	  break;
	}
      }

      /* otherwise, this is an addcc instruction: */
      else {

	/* dispatch on the condition: */
	switch (cond % TME_SPARC_COND_NOT) {
	default:
	  /* we should have caught this unsupported condition in
	     tme_sparc_timing_loop_ok(): */
	  assert (FALSE);
	  /* FALLTHROUGH */
	case TME_SPARC_COND_N:
	  /* nothing to do */
	  break;
	case TME_SPARC_COND_E:
	  value_zero = -addend;
	  value_true_greatest = 0;
	  break;
	case TME_SPARC_COND_CS:
	  value_zero = -addend;
	  value_true_greatest = (value_sign * 2) - (addend - 1);
	  break;
	case TME_SPARC_COND_NEG:
	  value_zero = value_sign - addend;
	  value_true_greatest = value_sign - 1;
	  break;
	}
      }
    }
  }

  /* the condition can't be never: */
  assert (cond != TME_SPARC_COND_N);

  /* assume that, if we block, we will block forever: */
  sleep = (const struct timeval *) NULL;

  /* if the condition is always, there is no maximum number of times
     that the branch to . can be taken: */
#if TME_SPARC_COND_N != 0
#error "TME_SPARC_COND_ values changed"
#endif
  ic->tme_sparc_timing_loop_branch_taken_max = (cond % TME_SPARC_COND_NOT);
  if (cond == (TME_SPARC_COND_NOT + TME_SPARC_COND_N)) {

    /* we may never finish: */
    ic->tme_sparc_timing_loop_finish.tme_value64_uint32_lo = (0 - (tme_uint32_t) 1);
    ic->tme_sparc_timing_loop_finish.tme_value64_uint32_hi = (0 - (tme_uint32_t) 1);
  }

  /* otherwise, the condition isn't always, so there is a maximum
     number of times that the branch to . can be taken: */
  else {

    /* it's not possible for all (adjusted-to-zero) values to be true.
       at least all-bits-one must be false: */
    assert (value_true_greatest <= ((value_sign - 1) * 2));

    /* test the initial value of rd: */
    value_test = (value_rd - value_zero) & ((value_sign * 2) - 1);

    /* if the initial value of rd will make the condition (ignoring
       TME_SPARC_COND_NOT) true after the first rd update
       instruction: */
    if (value_test <= value_true_greatest) {

      /* if this condition has TME_SPARC_COND_NOT: */
      if (cond & TME_SPARC_COND_NOT) {

	/* the branch to . will only be taken the first time: */
	branch_taken_count_max_m1 = 1 - 1;
      }

      /* otherwise, if the addend is -1: */
      else if (addend < 0) {

	/* the branch to . will be taken the first time, followed by
	   at most (value_test + 1) more times when the value of rd
	   makes the condition true: */
	branch_taken_count_max_m1 = (1 + (value_test + 1)) - 1;
      }

      /* otherwise, the addend is 1: */
      else {

	/* the branch to . will be taken the first time, followed by
	   at most ((value_true_greatest - value_test) + 1) more times
	   when the value of rd makes the condition true: */
	branch_taken_count_max_m1 = (1 + ((value_true_greatest - value_test) + 1)) - 1;
      }
    }

    /* otherwise, the initial value of rd will make the condition
       (ignoring TME_SPARC_COND_NOT) false after the first update
       instruction: */
    else {

      /* if this condition doesn't have TME_SPARC_COND_NOT: */
      if ((cond & TME_SPARC_COND_NOT) == 0) {

	/* the branch to . will only be taken the first time: */
	branch_taken_count_max_m1 = 1 - 1;
      }

      /* otherwise, if the addend is -1: */
      else if (addend < 0) {

	/* the branch to . will be taken the first time, followed by
	   at most (value_test - value_true_greatest) more times when
	   the value of rd makes the condition false: */
	branch_taken_count_max_m1 = (1 + (value_test - value_true_greatest)) - 1;
      }

      /* otherwise, the addend is 1: */
      else {

	/* the branch to . will be taken the first time, followed by
	   at most (~value_test + 1) more times when the value of rd
	   makes the condition false: */
	branch_taken_count_max_m1 = ((1 + (~value_test + 1)) - 1) & ((value_sign * 2) - 1);
      }
    }

    /* set the maximum number of times the branch to . can be taken: */
    ic->tme_sparc_timing_loop_branch_taken_count_max_m1 = branch_taken_count_max_m1;

    /* if each loop iteration takes more than one cycle: */
    loop_cycles_each = ic->tme_sparc_timing_loop_cycles_each;
    if (__tme_predict_false(loop_cycles_each != 1)) {

      /* get the maximum number of cycles to loop: */
      /* NB: we try to deal with overflow: */
      if (__tme_predict_false(loop_cycles_each != 2)) {
	cycles_scaled_max
	  = (branch_taken_count_max_m1
	     * loop_cycles_each);
      }
      else {
	cycles_scaled_max = branch_taken_count_max_m1 * 2;
      }
      cycles_scaled_max += loop_cycles_each;
      if (__tme_predict_false(cycles_scaled_max < ic->tme_sparc_timing_loop_branch_taken_count_max_m1)) {
	cycles_scaled_max = 0 - (tme_sparc_ireg_umax_t) 1;
      }
    }

    /* otherwise, each loop iteration takes one cycle: */
    else {

      /* get the maximum number of cycles to loop: */
      /* NB: we try to deal with overflow: */
      cycles_scaled_max = branch_taken_count_max_m1 + 1;
      cycles_scaled_max -= (cycles_scaled_max == 0);
    }

    /* we can't be looping for zero cycles: */
    assert (cycles_scaled_max > 0);

    /* get the latest host cycle counter when the timing loop must
       finish, if it doesn't finish sooner: */
#ifdef TME_HAVE_INT64_T
    cycles_finish.tme_value64_uint = cycles_scaled_max;
#else  /* !TME_HAVE_INT64_T */
    cycles_finish.tme_value64_uint32_lo = cycles_scaled_max;
    cycles_finish.tme_value64_uint32_hi = 0;
#endif /* !TME_HAVE_INT64_T */
    cycles_finish
      = tme_misc_cycles_scaled(&ic->tme_sparc_cycles_unscaling,
			       &cycles_finish);
    (void) tme_value64_add(&cycles_finish, &ic->tme_sparc_timing_loop_start);
    ic->tme_sparc_timing_loop_finish = cycles_finish;

    /* if the number of cycles to spin is small enough that we should
       truly spin, instead of yield: */
    if (cycles_scaled_max
	<= (ic->tme_sparc_cycles_scaled_per_usec
	    * TME_SPARC_TIMING_SPIN_USEC_MAX)) {

      /* spin: */
      tme_misc_cycles_spin_until(&ic->tme_sparc_timing_loop_finish);

      /* do the timing loop update: */
      _tme_sparc_timing_loop_update(ic,
				    ic->tme_sparc_timing_loop_branch_taken_count_max_m1);

      /* unwind back to instruction execution: */
      return;
    }

    /* if we will block until an external event: */
    if (TME_SPARC_TIMING_YIELD_BLOCK) {

      /* if the number of cycles to loop doesn't fit in 32 bits: */
      if (__tme_predict_false(cycles_scaled_max
			      & ~ (tme_sparc_ireg_umax_t) (tme_uint32_t) (0 - (tme_uint32_t) 1))) {

	/* convert cycles into microseconds: */
	usec = cycles_scaled_max / ic->tme_sparc_cycles_scaled_per_usec;

	/* set the sleep time: */
	sleep_buffer.tv_sec = (usec / 1000000);
	sleep_buffer.tv_usec = (usec % 1000000);
      }

      /* otherwise, the number of cycles to loop fits in 32 bits: */
      else {

	/* convert cycles into microseconds: */
	usec32 = ((tme_uint32_t) cycles_scaled_max) / ic->tme_sparc_cycles_scaled_per_usec;

	/* assume that we will sleep for less than one second: */
	sleep_buffer.tv_sec = 0;

	/* if the sleep time is one second or more: */
	if (__tme_predict_false(usec32 >= 1000000)) {

	  /* set the sleep time seconds: */
	  sleep_buffer.tv_sec = (usec32 / 1000000);

	  /* get the microseconds: */
	  usec32 = (usec32 % 1000000);
	}

	/* set the sleep time microseconds: */
	sleep_buffer.tv_usec = usec32;
      }

      /* we won't block forever: */
      sleep = &sleep_buffer;
    }
  }

  /* unbusy the instruction TLB entry: */
  assert (ic->_tme_sparc_itlb_current_token != NULL);
  tme_token_unbusy(ic->_tme_sparc_itlb_current_token);

  /* if threads are cooperative: */
  if (TME_THREADS_COOPERATIVE) {

    /* forget the instruction TLB entry: */
    ic->_tme_sparc_itlb_current_token = NULL;

    /* we will redispatch into timing mode: */
    ic->_tme_sparc_mode = TME_SPARC_MODE_TIMING_LOOP;
  }

  /* if we're blocking: */
  if (TME_SPARC_TIMING_YIELD_BLOCK) {

    /* lock the external mutex: */
    tme_mutex_lock(&ic->tme_sparc_external_mutex);

    /* check one last time for any external signal: */
    if (tme_memory_atomic_read_flag(&ic->tme_sparc_external_flag)) {
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_flag, FALSE);
      (*ic->_tme_sparc_external_check)(ic, TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED);
    }

    /* block on the external signal condition: */
    if (sleep != NULL) {
      tme_cond_sleep_yield(&ic->tme_sparc_external_cond,
			   &ic->tme_sparc_external_mutex,
			   sleep);
    }
    else {
      tme_cond_wait_yield(&ic->tme_sparc_external_cond,
			  &ic->tme_sparc_external_mutex);
    }

    /* unlock the external mutex: */
    tme_mutex_unlock(&ic->tme_sparc_external_mutex);
  }
  
  /* otherwise, we're not blocking: */
  else {

    /* do the simple yield: */
    tme_thread_yield();
  }

  /* finish the timing loop: */
  tme_sparc_timing_loop_finish(ic);

  /* relock the instruction TLB entry: */
  tme_sparc_callout_relock(ic);

  /* unwind back to instruction execution: */
  return;
}

/* this possibly starts a timing loop from the instruction
   executor: */
void
tme_sparc_timing_loop_start(struct tme_sparc *ic)
{
  tme_uint32_t insn_update;
  tme_uint32_t insn_branch_dot;
  tme_sparc_ireg_umax_t pc;

  /* save the current host cycles counter: */
  ic->tme_sparc_timing_loop_start = tme_misc_cycles();

  /* get the update instruction from the branch delay slot: */
  insn_update = tme_sparc_fetch_nearby(ic, 1);

  /* get the branch to . instruction: */
  insn_branch_dot = ic->_tme_sparc_insn;

  /* if we don't support this timing loop: */
  if (!tme_sparc_timing_loop_ok(insn_branch_dot,
				insn_update)) {
    return;
  }

  /* at this point, PC and PC_next_next both point to the branch to .,
     and PC_next points to the update instruction.  we have to advance
     the PCs, because _tme_sparc_timing_loop_update() expects PC and
     PC_next_next to point to the update instruction, PC_next to point
     to the branch to .: */

  /* if this is a v9 CPU: */
  if (TME_SPARC_VERSION(ic) >= 9) {
#ifdef TME_HAVE_INT64_T

    /* advance the PCs: */
    pc = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT);
    assert (ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)
	    == ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT));
    assert (((ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)
	      + sizeof(tme_uint32_t))
	     & ic->tme_sparc_address_mask)
	    == pc);
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT)
      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT);
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC) = pc;
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT) = pc;

#endif /* TME_HAVE_INT64_T */
  }

  /* otherwise, this is a v7 or v8 CPU: */
  else {

    /* advance the PCs: */
    pc = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT);
    assert (ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
	    == ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT));
    assert ((ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
	      + sizeof(tme_uint32_t))
	    == pc);
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT)
      = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT);
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC) = pc;
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT) = pc;
  }

  /* start the timing loop: */
  _tme_sparc_timing_loop_start(ic,
			       insn_update);
}

#if TME_HAVE_RECODE

/* the recode assist function for timing loops: */
tme_recode_uguest_t
tme_sparc_timing_loop_assist(struct tme_ic *_ic,
			     tme_recode_uguest_t insn_branch_dot,
			     tme_recode_uguest_t junk)
{
  struct tme_sparc *ic;
  tme_sparc_ireg_umax_t pc_next_next;
  int branch_dot_taken;
  tme_uint32_t insn_update;

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  /* save the branch to . instruction in the normal instruction
     position: */
  /* NB: we do this even though PC currently points to the timing loop
     update instruction: */
  ic->_tme_sparc_insn = insn_branch_dot;

  /* save the current host cycles counter: */
  ic->tme_sparc_timing_loop_start = tme_misc_cycles();

  /* NB: unlike tme_sparc_timing_loop_start(), this function may be
     called after the branch to . has *not* been taken.  this happens
     when the branch to . is conditional and does not annul - this is
     the "one final time" update instruction discussed in
     _tme_sparc_timing_loop_start().

     at this point, PC points to the update instruction, PC_next
     points to the branch to . (if the branch to . was taken) or to
     the instruction following the update instruction (if the branch
     to . was not taken and does not annul): */

  /* if this is a v9 CPU: */
  if (TME_SPARC_VERSION(ic) >= 9) {
#ifdef TME_HAVE_INT64_T

    /* set PC_next_next from PC_next: */
    pc_next_next
      = ((ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT)
	  + sizeof(tme_uint32_t))
	 & ic->tme_sparc_address_mask);
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;

    /* see if the timing loop branch to . instruction was taken: */
    branch_dot_taken = (ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC) == pc_next_next);

    /* get the timing loop update instruction: */
    insn_update = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_INSN);

#else  /* !TME_HAVE_INT64_T */

    /* silence uninitialized variable warnings: */
    branch_dot_taken = 0;
    insn_update = 0;

#endif /* !TME_HAVE_INT64_T */
  }

  /* otherwise, this is not a v9 CPU: */
  else {

    /* set PC_next_next from PC_next: */
    pc_next_next
      = (ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT)
	 + sizeof(tme_uint32_t));
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;

    /* see if the timing loop branch to . instruction was taken: */
    branch_dot_taken = (ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC) == (tme_uint32_t) pc_next_next);

    /* get the timing loop update instruction: */
    insn_update = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_INSN);
  }

  /* if the timing loop branch to . instruction was taken: */
  if (branch_dot_taken) {

    /* end any recode verifying: */
    tme_sparc_recode_verify_end_preinstruction(ic);

    /* start the timing loop: */
    _tme_sparc_timing_loop_start(ic,
				 insn_update);
  }

  /* otherwise, the timing loop branch to . instruction was not
     taken, and it does not annul: */
  else {

    /* do the one final update: */
    ic->_tme_sparc_insn = insn_update;
    _tme_sparc_timing_loop_update(ic, 0);
  }

  /* unwind back to instruction execution: */
  return (0);
}

#endif /* TME_HAVE_RECODE */

/* this finishes a timing loop: */
void
tme_sparc_timing_loop_finish(struct tme_sparc *ic)
{
  union tme_value64 cycles_finish;
  union tme_value64 cycles_scaled_u;
  tme_sparc_ireg_umax_t cycles_scaled;
  unsigned int loop_cycles_each;
  tme_sparc_ireg_umax_t branch_taken_count_m1;

  /* loop forever: */
  for (;;) {

    /* get the current host cycle counter: */
    cycles_finish = tme_misc_cycles();

    /* if the timing loop has finished: */
    if (tme_value64_cmp(&cycles_finish, >=, &ic->tme_sparc_timing_loop_finish)) {
      break;
    }

    /* if an external event has happened: */
    if (tme_memory_atomic_read_flag(&ic->tme_sparc_external_flag)) {
      break;
    }

    /* if we block, we were supposed to block until an external event
       happened: */
    assert (!TME_SPARC_TIMING_YIELD_BLOCK);

    /* yield: */
    tme_thread_yield();
  }

  /* get the number of cycles elapsed: */
  /* NB: we try to deal with overflow: */
  (void) tme_value64_sub(&cycles_finish, &ic->tme_sparc_timing_loop_start);
  cycles_scaled_u
    = tme_misc_cycles_scaled(&ic->tme_sparc_cycles_scaling,
			     &cycles_finish);
#ifdef TME_HAVE_INT64_T
  cycles_scaled = cycles_scaled_u.tme_value64_uint;
#else  /* !TME_HAVE_INT64_T */
  cycles_scaled
    = (cycles_scaled_u.tme_value64_uint32_hi
       ? (tme_uint32_t) (0 - (tme_uint32_t) 1)
       : cycles_scaled_u.tme_value64_uint32_lo);
#endif /* !TME_HAVE_INT64_T */

  /* NB: it's unusual, but actually okay if no cycles have elapsed.
     this just means that the branch to . will only be taken that
     first time.  since we need the count of times the branch to .
     was taken, minus one, dividing the elapsed cycles by the number
     of cycles per loop gets exactly what we need: */

  /* get the count of times the branch to . was taken, minus one: */
  loop_cycles_each = ic->tme_sparc_timing_loop_cycles_each;
  if (__tme_predict_false(loop_cycles_each != 1)) {
    if (__tme_predict_false(loop_cycles_each != 2)) {
      branch_taken_count_m1 = cycles_scaled / loop_cycles_each;
    }
    else {
      branch_taken_count_m1 = cycles_scaled / 2;
    }
  }
  else {
    branch_taken_count_m1 = cycles_scaled;
  }

  /* if there is a maximum count of times the branch to . could be taken: */
  if (ic->tme_sparc_timing_loop_branch_taken_max) {

    /* make sure that the branch to . isn't taken any more than the
       maximum: */
    if (branch_taken_count_m1 > ic->tme_sparc_timing_loop_branch_taken_count_max_m1) {
      branch_taken_count_m1 = ic->tme_sparc_timing_loop_branch_taken_count_max_m1;
    }
  }

  /* do the timing loop update: */
  _tme_sparc_timing_loop_update(ic,
				branch_taken_count_m1);

  /* zero the instruction burst: */
  ic->_tme_sparc_instruction_burst_remaining = 0;
  ic->_tme_sparc_instruction_burst_other = TRUE;

  /* if threads are cooperative: */
  if (TME_THREADS_COOPERATIVE) {

    /* we will chain into execution mode: */
    ic->_tme_sparc_mode = TME_SPARC_MODE_EXECUTION;

    /* save a redispatch and resume execution directly: */
    (*ic->_tme_sparc_execute)(ic);
    abort();
  }

  /* otherwise, threads are preemptive: */

  /* unwind back to instruction execution: */
  return;
}
