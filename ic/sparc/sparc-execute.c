/* $Id: sparc-execute.c,v 1.10 2010/02/20 21:58:15 fredette Exp $ */

/* ic/sparc/sparc-execute.c - executes SPARC instructions: */

/*
 * Copyright (c) 2005, 2009 Matt Fredette
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

_TME_RCSID("$Id: sparc-execute.c,v 1.10 2010/02/20 21:58:15 fredette Exp $");

/* includes: */
#include "sparc-auto.h"

#if (TME_SPARC_VERSION(ic) < 9)
#define tme_sparc_ireg_t tme_uint32_t
#define tme_sparc_ireg(x)  tme_sparc_ireg_uint32(x)
#define tme_sparc_idle_pcs tme_sparc_idle_pcs_32
#define TME_PRIxSPARCREG "0x%08" TME_PRIx32
#else  /* TME_SPARC_VERSION(ic) >= 9 */
#define tme_sparc_ireg_t tme_uint64_t
#define tme_sparc_ireg(x)  tme_sparc_ireg_uint64(x)
#define tme_sparc_idle_pcs tme_sparc_idle_pcs_64
#define TME_PRIxSPARCREG "0x%016" TME_PRIx64
#endif /* TME_SPARC_VERSION(ic) >= 9 */

/* the sparc instruction executor: */
static void
_TME_SPARC_EXECUTE_NAME(struct tme_sparc *ic)
{
  tme_uint32_t asi_mask_insn;
  tme_uint32_t asi_mask_data;
  struct tme_sparc_tlb *itlb_current;
  struct tme_sparc_tlb itlb_invalid;
  struct tme_token token_invalid;
  tme_sparc_ireg_t pc_previous;
  tme_sparc_ireg_t pc;
  tme_uint32_t insn;
  tme_uint32_t tlb_hash;
  const tme_shared tme_uint8_t *emulator_off;
  unsigned int opcode;
  unsigned int reg_rs1;
  unsigned int reg_rs2;
  unsigned int reg_rd;
  int annulled;
  int branch_dot;
  tme_uint32_t branch_dot_burst;
#if TME_SPARC_VERSION(ic) >= 9
  unsigned int cc;
  tme_uint64_t value_rs1;
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  tme_uint8_t conds_mask_icc;
  tme_uint8_t conds_mask_fcc;
  tme_uint16_t conds_mask;
  unsigned int cond;
  tme_int32_t disp;
  tme_sparc_ireg_t pc_next_next;
  unsigned int reg_o0;

  /* get the default address space identifiers and masks: */
  if (TME_SPARC_VERSION(ic) < 9) {
    if (TME_SPARC_PRIV(ic)) {
      asi_mask_insn = TME_SPARC32_ASI_MASK_SI;
      asi_mask_data = TME_SPARC32_ASI_MASK_SD;
    }
    else {
      asi_mask_insn = TME_SPARC32_ASI_MASK_UI;
      asi_mask_data = TME_SPARC32_ASI_MASK_UD;
    }
  }
  else {
    if (__tme_predict_false((TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS)
			    && ic->tme_sparc64_ireg_tl > 0)) {
      asi_mask_insn
	= TME_SPARC64_ASI_MASK_NUCLEUS(!TME_SPARC64_ASI_FLAG_LITTLE);
      ic->tme_sparc_memory_context_default = 0;
    }
    else {
      asi_mask_insn
	= TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED((TME_SPARC_PRIV(ic)
						      ? !TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER
						      : TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
						     + !TME_SPARC64_ASI_FLAG_SECONDARY
						     + !TME_SPARC64_ASI_FLAG_NO_FAULT
						     + !TME_SPARC64_ASI_FLAG_LITTLE);
      ic->tme_sparc_memory_context_default = ic->tme_sparc_memory_context_primary;
    }
    asi_mask_data = asi_mask_insn;
    if (__tme_predict_false(ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_CLE)) {
      assert ((TME_SPARC64_ASI_MASK_NUCLEUS(!TME_SPARC64_ASI_FLAG_LITTLE)
	       ^ TME_SPARC64_ASI_MASK_NUCLEUS(TME_SPARC64_ASI_FLAG_LITTLE))
	      == (TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(!TME_SPARC64_ASI_FLAG_LITTLE)
		  ^ TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(TME_SPARC64_ASI_FLAG_LITTLE)));
      asi_mask_data
	^= (TME_SPARC64_ASI_MASK_NUCLEUS(!TME_SPARC64_ASI_FLAG_LITTLE)
	    ^ TME_SPARC64_ASI_MASK_NUCLEUS(TME_SPARC64_ASI_FLAG_LITTLE));
    }
  }
  ic->tme_sparc_asi_mask_insn = asi_mask_insn;
  ic->tme_sparc_asi_mask_data = asi_mask_data;

#if TME_SPARC_HAVE_RECODE(ic)

  /* set the recode read/write TLB flags mask to and with the flags
     from a read/write thunk, before being tested against the flags in
     a recode DTLB entry.  this TLB flags mask must clear flags that
     do not apply, based on the current state: */
  ic->tme_sparc_recode_rw_tlb_flags
    = (TME_RECODE_TLB_FLAGS_MASK(ic->tme_sparc_recode_ic)
       - (

	  /* the load and store flags for the other privilege level do
	     not apply, because we're not at that privilege level: */
	  (TME_SPARC_PRIV(ic)
	   ? (TME_SPARC_RECODE_TLB_FLAG_LD_USER(ic)
	      + TME_SPARC_RECODE_TLB_FLAG_ST_USER(ic))
	   : (TME_SPARC_RECODE_TLB_FLAG_LD_PRIV(ic)
	      + TME_SPARC_RECODE_TLB_FLAG_ST_PRIV(ic)))

	  /* on a v9 CPU, if the ASI register has the default data
	     ASI, but with the no-fault bit set, the ASI register is
	     correct for no-fault loads, and the no-fault load bit
	     doesn't apply: */
	  + ((TME_SPARC_VERSION(ic) >= 9
	      && ((TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS) == 0
		  || ic->tme_sparc64_ireg_tl == 0)
	      && (ic->tme_sparc64_ireg_asi
		  == (TME_SPARC_ASI_MASK_WHICH(asi_mask_data)
		      + TME_SPARC64_ASI_FLAG_NO_FAULT)))
	     ? TME_SPARC_RECODE_TLB_FLAG_LD_NF(ic)
	     : 0)));

  /* set the recode chain TLB flags mask to and with the flags from
     the chain thunk, before being tested against the flags in a
     recode ITLB entry.  this TLB flags mask must clear flags that do
     not apply, based on the current state: */
  ic->tme_sparc_recode_chain_tlb_flags
    = (TME_RECODE_TLB_FLAGS_MASK(ic->tme_sparc_recode_ic)
       - (

	  /* the fetch flags for the other privilege level do not
	     apply, because we're not at that privilege level: */
	  (TME_SPARC_PRIV(ic)
	   ? TME_SPARC_RECODE_TLB_FLAG_CHAIN_USER(ic)
	   : TME_SPARC_RECODE_TLB_FLAG_CHAIN_PRIV(ic))
	  ));

#endif /* TME_SPARC_HAVE_RECODE(ic) */

  /* create an invalid instruction TLB entry, and use it as the initial
     current instruction TLB entry: */
  tme_token_init(&token_invalid);
  itlb_invalid.tme_sparc_tlb_addr_first = 1;
  itlb_invalid.tme_sparc_tlb_addr_last = 0;
  itlb_invalid.tme_sparc_tlb_bus_tlb.tme_bus_tlb_token = &token_invalid;
  itlb_current = &itlb_invalid;

  /* busy the invalid instruction TLB entry: */
  assert (ic->_tme_sparc_itlb_current_token == NULL);
  tme_token_busy(&token_invalid);
  ic->_tme_sparc_itlb_current_token = &token_invalid;

  /* the first instruction will not be annulled: */
  annulled = FALSE;

  /* the last instruction was not a taken branch to .: */
  branch_dot = FALSE;
  branch_dot_burst = 0;

  for (;;) {

    /* if we have used up our instruction burst: */
    if (__tme_predict_false(ic->_tme_sparc_instruction_burst_remaining == 0)) {

      /* if the last instruction was a taken branch to .: */
      if (__tme_predict_false(branch_dot)) {

	/* clear the taken branch to . flag and restore the
	   instruction burst that had been remaining: */
	branch_dot = FALSE;
	ic->_tme_sparc_instruction_burst_remaining = branch_dot_burst;

	/* if the next instruction will be annulled: */
	if (__tme_predict_false(annulled)) {

	  /* a taken branch to . that annuls its branch delay slot
	     must be a "ba,a .", since taken conditional branches
	     never annul.  "ba,a ." makes an infinite loop.

	     we can just go idle here, but we must make sure that any
	     trap sees %pc on the branch to ., and not its branch
	     delay slot (since we don't track the annulled bit in the
	     processor structure), and we must make sure that %pc_next
	     is the branch to . delay slot (because otherwise it would
	     look like we didn't loop even once): */
	  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
	    = ic->tme_sparc_ireg(TME_SPARC_IREG_PC);
	  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT)
	    = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC)
	       + sizeof(tme_uint32_t));
	  if (TME_SPARC_VERSION(ic) >= 9) {
	    ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT) &= ic->tme_sparc_address_mask;
	  }
	  tme_sparc_do_idle(ic);
	  /* NOTREACHED */
	}

	/* if the branch delay instruction immediately follows the
	   branch to .: */
	if (ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
	    == (ic->tme_sparc_ireg(TME_SPARC_IREG_PC)
		+ sizeof(tme_uint32_t))) {

	  /* if this branch to . is not a timing loop, this will
	     return.  if it's a timing loop that doesn't sleep, this
	     will return.  otherwise, this won't return: */
	  tme_sparc_timing_loop_start(ic);
	}

	/* continue now, to finish the instruction burst that
	   had been remaining: */
	continue;
      }

      /* if this was a full instruction burst: */
      if (!ic->_tme_sparc_instruction_burst_other) {

	/* if it's time to update the runlength: */
	if (ic->tme_sparc_runlength_update_next == 0) {

	  /* update the runlength: */
#ifndef _TME_SPARC_RECODE_VERIFY
	  tme_runlength_update(&ic->tme_sparc_runlength);
#endif /* !_TME_SPARC_RECODE_VERIFY */

	  /* start another runlength update period: */
	  ic->tme_sparc_runlength_update_next = ic->tme_sparc_runlength_update_period;
	}

	/* advance in the runlength update period: */
	ic->tme_sparc_runlength_update_next--;

	/* we are not in a full instruction burst: */
	ic->_tme_sparc_instruction_burst_other = TRUE;
      }

      /* if the next instruction will be annulled: */
      if (__tme_predict_false(annulled)) {

	/* NB that we have to handle the next instruction now, in the
	   immediate next iteration of the execution loop, since we
	   don't track the annulled bit in the processor structure,
	   and we want to do good emulation and actually fetch the
	   instruction (as opposed to just advancing the PCs now).
	   start an instruction burst of one instruction: */
	ic->_tme_sparc_instruction_burst_remaining = 1;
	continue;
      }

      /* if we need to do an external check: */
      if (tme_memory_atomic_read_flag(&ic->tme_sparc_external_flag)) {

	/* do an external check: */
	tme_memory_atomic_write_flag(&ic->tme_sparc_external_flag, FALSE);
	tme_memory_barrier(ic, sizeof(*ic), TME_MEMORY_BARRIER_READ_BEFORE_READ);
	(*ic->_tme_sparc_external_check)(ic, TME_SPARC_EXTERNAL_CHECK_NULL);
      }

      /* start a new instruction burst: */
      ic->_tme_sparc_instruction_burst_remaining
	= ic->_tme_sparc_instruction_burst;
      ic->_tme_sparc_instruction_burst_other = FALSE;

      /* if the next PC might be in an idle PC range: */
      pc = ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT);
      if (__tme_predict_false(pc < ic->tme_sparc_idle_pcs[1])) {

	/* if we haven't detected the idle PC yet: */
	if (__tme_predict_false(TME_SPARC_IDLE_TYPE_PC_STATE(ic->tme_sparc_idle_pcs[0]) != 0)) {
	  /* nothing to do */
	}

	/* if the next PC and the delay PC are both in the idle PC
	   range, and this idle type has an idle PC range: */
	else if (__tme_predict_false(pc >= ic->tme_sparc_idle_pcs[0])) {
	  if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPES_PC_RANGE)
	      && ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT) >= ic->tme_sparc_idle_pcs[0]
	      && ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT) < ic->tme_sparc_idle_pcs[1]) {

	    /* if we haven't marked any idles yet, or if we have
	       marked one and the next PC is at or behind that PC: */
	    if (ic->tme_sparc_idle_marks == 0
		|| (ic->tme_sparc_idle_marks == 1
		    && pc <= ic->tme_sparc_idle_pcs[2])) {

	      /* mark the idle: */
	      ic->tme_sparc_idle_marks++;

	      /* we won't mark another idle until we detect a
		 backwards control transfer in the idle PC range,
		 indicating another iteration of the idle loop: */
	      ic->tme_sparc_idle_pcs[2] = pc;
	    }
	  }
	}
      }

      /* if we have marked any idles: */
      if (__tme_predict_false(ic->tme_sparc_idle_marks != 0)) {

	/* if we have marked one idle: */
	if (ic->tme_sparc_idle_marks == 1) {

	  /* start a new idle instruction burst: */
	  ic->_tme_sparc_instruction_burst_remaining
	    = ic->_tme_sparc_instruction_burst_idle;
	  ic->_tme_sparc_instruction_burst_other = TRUE;
	}

	/* otherwise, we have marked two consecutive idles without a
	   trap: */
	else {
	  assert (ic->tme_sparc_idle_marks == 2);

	  /* idle: */
	  tme_sparc_do_idle(ic);
	}
      }

      /* if this is a cooperative threading system: */
#if TME_THREADS_COOPERATIVE

      /* unbusy the current instruction TLB entry: */
      assert (ic->_tme_sparc_itlb_current_token
	      == itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
      tme_sparc_tlb_unbusy(itlb_current);
      ic->_tme_sparc_itlb_current_token = NULL;

      /* yield: */
      tme_thread_yield();
#endif /* TME_THREADS_COOPERATIVE */

      /* if we may update the runlength with this instruction burst,
	 note its start time: */
      if (ic->tme_sparc_runlength_update_next == 0) {
	ic->tme_sparc_runlength.tme_runlength_cycles_start = tme_misc_cycles();
      }
    }

    /* we can't know that this instruction is a taken branch to .: */
    assert (!branch_dot);

    /* we are going to use one instruction in the burst: */
    ic->_tme_sparc_instruction_burst_remaining--;
#ifdef _TME_SPARC_STATS
    ic->tme_sparc_stats.tme_sparc_stats_insns_total++;
#endif /* _TME_SPARC_STATS */

    /* save the previous PC: */
    pc_previous = ic->tme_sparc_ireg(TME_SPARC_IREG_PC);

    /* if we're replaying recoded instructions: */
    if (tme_sparc_recode_verify_replay_last_pc(ic) != 0) {

      /* if the previous instruction was the last instruction to
	 verify, return now: */
      if (__tme_predict_false(tme_sparc_recode_verify_replay_last_pc(ic) == pc_previous)) {
	assert (ic->_tme_sparc_itlb_current_token != &token_invalid);
	return;
      }

      /* poison pc_previous to prevent all recoding: */
      pc_previous = ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT) - sizeof(tme_uint32_t);
    }

    /* update the PCs and get the PC of the instruction to execute: */
    pc = ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT);
    ic->tme_sparc_ireg(TME_SPARC_IREG_PC) = pc;
    pc_next_next = ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT);
    ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT) = pc_next_next;
    pc_next_next += sizeof(tme_uint32_t);
    if (TME_SPARC_VERSION(ic) >= 9) {
      pc_next_next &= ic->tme_sparc_address_mask;
      assert ((pc | ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT))
	      < ic->tme_sparc_address_mask);
    }
    ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;

    /* NB that we only save instruction TLB entries that allow fast
       reading, and we also change tme_sparc_tlb_addr_last to be the
       last PC covered by the entry (it's normally the last address
       covered by the entry).  this allows us to do minimal checking
       of the current instruction TLB entry at itlb_current: */

    /* if the current instruction TLB entry covers this address: */
    if (__tme_predict_true(((tme_sparc_ireg_t) itlb_current->tme_sparc_tlb_addr_first) <= pc
			   && pc <= ((tme_sparc_ireg_t) itlb_current->tme_sparc_tlb_addr_last))) {

      /* the current instruction TLB entry must cover this
	 address and allow reading: */
      assert (TME_SPARC_TLB_ASI_MASK_OK(itlb_current, asi_mask_insn)
	      && (itlb_current->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max
		  || itlb_current->tme_sparc_tlb_context == ic->tme_sparc_memory_context_default)
	      && itlb_current->tme_sparc_tlb_addr_first <= pc
	      && pc <= itlb_current->tme_sparc_tlb_addr_last);

      /* fetch the instruction: */
      insn = tme_memory_bus_read32((const tme_shared tme_uint32_t *) (itlb_current->tme_sparc_tlb_emulator_off_read + pc),
				   itlb_current->tme_sparc_tlb_bus_rwlock,
				   sizeof(tme_uint32_t),
				   sizeof(tme_sparc_ireg_t));
      insn = tme_betoh_u32(insn);
    }

    /* otherwise, our current TLB entry doesn't cover this address: */
    else {

      /* unbusy the current instruction TLB entry: */
      assert (ic->_tme_sparc_itlb_current_token
	      == itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
      tme_sparc_tlb_unbusy(itlb_current);

      /* rehash the current instruction TLB entry: */
      tlb_hash = TME_SPARC_TLB_HASH(ic, ic->tme_sparc_memory_context_default, pc);
      itlb_current = &ic->tme_sparc_tlbs[TME_SPARC_ITLB_ENTRY(ic, tlb_hash)];

      /* busy the current instruction TLB entry: */
      tme_sparc_tlb_busy(itlb_current);
      ic->_tme_sparc_itlb_current_token = itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token;

      /* if the new current instruction TLB entry is valid and covers
         this address: */
      if (tme_bus_tlb_is_valid(&itlb_current->tme_sparc_tlb_bus_tlb)
	  && __tme_predict_true(TME_SPARC_TLB_ASI_MASK_OK(itlb_current, asi_mask_insn)
				&& (itlb_current->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max
				    || itlb_current->tme_sparc_tlb_context == ic->tme_sparc_memory_context_default)
				&& pc >= (tme_sparc_ireg_t) itlb_current->tme_sparc_tlb_addr_first
				&& pc <= (tme_sparc_ireg_t) itlb_current->tme_sparc_tlb_addr_last)) {

	/* fetch the instruction: */
	insn = tme_memory_bus_read32((const tme_shared tme_uint32_t *) (itlb_current->tme_sparc_tlb_emulator_off_read + pc),
				     itlb_current->tme_sparc_tlb_bus_rwlock,
				     sizeof(tme_uint32_t),
				     sizeof(tme_sparc_ireg_t));
	insn = tme_betoh_u32(insn);
      }

      /* otherwise, the new current instruction TLB entry is not valid
         or does not cover this address: */
      else {

	/* the slow fetch will manage unbusying and busying the
	   current instruction TLB entry, so make sure that doesn't
	   happen at unlock and relock time: */
	ic->_tme_sparc_itlb_current_token = NULL;

	/* fetch the instruction: */
	emulator_off =
#if TME_SPARC_VERSION(ic) < 9
	  tme_sparc32_ls
#else  /* TME_SPARC_VERSION(ic) >= 9 */
	  tme_sparc64_ls
#endif /* TME_SPARC_VERSION(ic) >= 9 */
	  (ic,
	   pc,
	   (tme_sparc_ireg_t *) NULL,
	   (TME_SPARC_LSINFO_SIZE(sizeof(tme_uint32_t))
	    + TME_SPARC_LSINFO_ASI(TME_SPARC_ASI_MASK_WHICH(asi_mask_insn))
	    + TME_SPARC_LSINFO_A
	    + TME_SPARC_LSINFO_OP_FETCH
	    + (annulled
	       ? TME_SPARC_LSINFO_NO_FAULT
	       : 0)));
	assert (emulator_off != TME_EMULATOR_OFF_UNDEF);

	/* unbusy and busy the current instruction TLB entry at unlock
	   and relock time again: */
	ic->_tme_sparc_itlb_current_token = itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token;

	/* if this current instruction TLB entry covers the entire
	   instruction and allows fast reading: */
	if (__tme_predict_true(emulator_off == itlb_current->tme_sparc_tlb_emulator_off_read)) {

	  /* the current instruction TLB entry must now cover this
	     address and allow reading: */
	  /* NB that tme_sparc_tlb_addr_last has not been changed yet: */
	  assert (TME_SPARC_TLB_ASI_MASK_OK(itlb_current, asi_mask_insn)
		  && (itlb_current->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max
		      || itlb_current->tme_sparc_tlb_context == ic->tme_sparc_memory_context_default)
		  && itlb_current->tme_sparc_tlb_addr_first <= pc
		  && (pc + sizeof(tme_uint32_t) - 1) <= itlb_current->tme_sparc_tlb_addr_last);

	  /* fetch the instruction: */
	  insn = tme_memory_bus_read32((const tme_shared tme_uint32_t *) (itlb_current->tme_sparc_tlb_emulator_off_read + pc),
				       itlb_current->tme_sparc_tlb_bus_rwlock,
				       sizeof(tme_uint32_t),
				       sizeof(tme_sparc_ireg_t));
	  insn = tme_betoh_u32(insn);

	  /* modify tme_sparc_tlb_addr_last of this first to represent the last valid
	     PC covered by the entry: */
	  itlb_current->tme_sparc_tlb_addr_last
	    &= (((tme_bus_addr_t) 0) - sizeof(tme_uint32_t));
	}

	/* otherwise, this instruction TLB entry does not cover the
	   entire instruction and/or it does not allow fast reading.
	   the instruction has already been loaded into the memory
	   buffer: */
	else {

	  /* unbusy the current instruction TLB entry and poison it,
	     so we won't try to do any fast fetches with it: */
	  assert (ic->_tme_sparc_itlb_current_token
		  == itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
	  tme_sparc_tlb_unbusy(itlb_current);
	  itlb_current->tme_sparc_tlb_addr_first = 1;
	  itlb_current->tme_sparc_tlb_addr_last = 0;
	  ic->_tme_sparc_itlb_current_token = NULL;

	  /* fetch the instruction from the memory buffer: */
	  assert ((emulator_off + pc) == ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s);
	  insn = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer32s[0];
	  insn = tme_betoh_u32(insn);
#ifdef _TME_SPARC_STATS
	  ic->tme_sparc_stats.tme_sparc_stats_insns_slow++;
#endif /* _TME_SPARC_STATS */

	  /* busy the invalid instruction TLB entry: */
	  itlb_current = &itlb_invalid;
	  assert (ic->_tme_sparc_itlb_current_token == NULL);
	  tme_sparc_tlb_busy(itlb_current);
	  ic->_tme_sparc_itlb_current_token = itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token;
	}
      }

      /* if this instruction has been annulled: */
      if (__tme_predict_false(annulled)) {

	/* make this instruction a nop: */
	insn = 0x01000000;

	/* when an annulled instruction also happens to be a branch
	   target, we can't run or make an instructions thunk
	   associated with its PC, since instructions thunks don't
	   take the annulled bit as any kind of parameter.  we poison
	   pc_previous to prevent this from happening.  annulled
	   instructions that are also branch targets should be pretty
	   rare anyways: */
	pc_previous = pc - sizeof(tme_uint32_t);
      }

      /* the next instruction will not be annulled: */
      annulled = FALSE;
    }

    /* start this instruction: */
    ic->_tme_sparc_insn = insn;

    /* set %g0 to zero: */
    ic->tme_sparc_ireg(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G0) = 0;

#if TME_SPARC_HAVE_RECODE(ic)

    /* if this is the idle PC, and the idle type marks the idle when
       control reaches the idle PC: */
    if (__tme_predict_false(pc == ic->tme_sparc_idle_pcs[0])) {
      if (TME_SPARC_IDLE_TYPE_IS(ic,
				 (TME_SPARC_IDLE_TYPES_TARGET_CALL
				  | TME_SPARC_IDLE_TYPES_TARGET_BRANCH
				  ))) {

	/* mark the idle: */
	TME_SPARC_IDLE_MARK(ic);

	/* poison the previous PC to prevent all recoding, to
	   guarantee that we always see the idle PC (if we allowed the
	   idle PC to be recoded, it might get chained to): */
	pc_previous = pc - sizeof(tme_uint32_t);
      }
    }

    /* if this PC does not follow the previous PC, but the next PC
       follows this PC, this PC is a simple control transfer target: */
    if (__tme_predict_false(((tme_sparc_ireg_t) (pc - sizeof(tme_uint32_t)))
			    != pc_previous)) {
      if (__tme_predict_true(((tme_sparc_ireg_t) (pc + sizeof(tme_uint32_t)))
			     == ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT))) {
	tme_recode_thunk_off_t insns_thunk;

	/* if the current instruction TLB entry is not the invalid TLB
	   entry, and there is an instructions thunk for this PC: */
	if (__tme_predict_true(itlb_current != &itlb_invalid
			       && (insns_thunk
				   = tme_sparc_recode(ic,
						      itlb_current,
						      ((const tme_shared tme_uint32_t *) 
						       (itlb_current->tme_sparc_tlb_emulator_off_read
							+ pc)))) != 0)) {

	  /* begin verifying this instructions thunk: */
	  tme_sparc_recode_verify_begin(ic);

	  /* like this execution loop, the recode instructions thunks
	     expect PC_next to be the next instruction to execute.
	     we've already updated the PCs above, so we have to undo
	     the update of PC_next.  NB that we don't have to undo PC
	     or PC_next_next, since the instructions thunks don't read
	     them: */
	  pc = ic->tme_sparc_ireg(TME_SPARC_IREG_PC);
	  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT) = pc;

	  /* run the recode instructions thunk: */
	  TME_SPARC_STAT_N(ic, tme_sparc_stats_insns_total, -1);
	  tme_recode_insns_thunk_run(&ic->tme_sparc_ic,
				     ic->tme_sparc_recode_insns_group.tme_recode_insns_group_chain_thunk,
				     insns_thunk);

	  /* set PC_next_next from PC_next, since the recode
	     instructions thunks usually don't.  (this won't destroy
	     any specially set PC_next_next, because any instruction
	     that sets one is supposed to redispatch.) */
	  pc = ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT);
	  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT) = pc + sizeof(tme_uint32_t);

	  /* end verifying this instructions thunk: */
	  tme_sparc_recode_verify_end(ic, TME_SPARC_TRAP_none);

	  /* we force a PC to make it look like a control transfer has
	     happened (one probably has), to encourage creation of
	     another instructions thunk.  this is something like the
	     opposite of poisoning: */
	  ic->tme_sparc_ireg(TME_SPARC_IREG_PC) = pc;

	  /* instead of figuring out what the currently busy
	     instruction TLB entry is, we simply unbusy the currently
	     busy instruction TLB token and make the current
	     instruction TLB entry invalid: */
	  assert (ic->_tme_sparc_itlb_current_token != NULL);
	  tme_token_unbusy(ic->_tme_sparc_itlb_current_token);
	  itlb_current = &itlb_invalid;
	  tme_token_busy(&token_invalid);
	  ic->_tme_sparc_itlb_current_token = &token_invalid;

	  /* restart the loop: */
	  continue;
	}
      }
    }

#endif /* TME_SPARC_HAVE_RECODE(ic) */

    /* if this is a format three instruction (op is two or three): */
    if (__tme_predict_true(insn >= 0x80000000)) {

      /* if the i bit is zero: */
      if (__tme_predict_true((insn & TME_BIT(13)) == 0)) {

	/* decode rs2: */
	reg_rs2 = TME_FIELD_MASK_EXTRACTU(insn, TME_SPARC_FORMAT3_MASK_RS2);
	TME_SPARC_REG_INDEX(ic, reg_rs2);
      }

      /* otherwise, the i bit is one: */
      else {

	/* decode simm13: */
	ic->tme_sparc_ireg(TME_SPARC_IREG_TMP(0)) = TME_FIELD_MASK_EXTRACTS(insn, (tme_sparc_ireg_t) 0x1fff);
	reg_rs2 = TME_SPARC_IREG_TMP(0);
      }

      /* decode rs1: */
      reg_rs1 = TME_FIELD_MASK_EXTRACTU(insn, TME_SPARC_FORMAT3_MASK_RS1);
      TME_SPARC_REG_INDEX(ic, reg_rs1);

      /* decode rd: */
      reg_rd = TME_FIELD_MASK_EXTRACTU(insn, TME_SPARC_FORMAT3_MASK_RD);
      TME_SPARC_REG_INDEX(ic, reg_rd);
    
      /* form the opcode index: */
      opcode = TME_FIELD_MASK_EXTRACTU(insn, (0x3f << 19));
      opcode += ((insn >> (30 - 6)) & 0x40);
      
      /* run the instruction: */
      (*_TME_SPARC_EXECUTE_OPMAP[opcode])
	(ic, 
	 &ic->tme_sparc_ireg(reg_rs1),
	 &ic->tme_sparc_ireg(reg_rs2),
	 &ic->tme_sparc_ireg(reg_rd));
    }

    /* otherwise, if this is a format two instruction: */
    else if (__tme_predict_true(insn < 0x40000000)) {

      /* dispatch on op2: */
      switch (TME_FIELD_MASK_EXTRACTU(insn, (0x7 << 22))) {

#if TME_SPARC_VERSION(ic) >= 9
      case 1: /* BPcc */

	/* if cc0 is set, this is an illegal instruction: */
	if (__tme_predict_false(insn & TME_BIT(20))) {
	  TME_SPARC_INSN_TRAP(TME_SPARC_TRAP(ic,illegal_instruction));
	}

	/* get %icc or %xcc: */
	cc = ic->tme_sparc64_ireg_ccr;
	if (insn & TME_BIT(21)) {
	  cc /= (TME_SPARC64_CCR_XCC / TME_SPARC64_CCR_ICC);
	}
	cc = TME_FIELD_MASK_EXTRACTU(cc, TME_SPARC64_CCR_ICC);

	/* get the conditions mask: */
	conds_mask = _tme_sparc_conds_icc[cc];

	/* add the not-conditions to the conditions mask: */
	conds_mask += ((conds_mask ^ 0xff) << 8);

	/* clear cc1, cc0, and p: */
	insn &= ~(TME_BIT(21) + TME_BIT(20) + TME_BIT(19));

	/* flip the most significant bit of the disp19: */
	insn ^= TME_BIT(18);

	/* sign-extend the disp19 to a disp22: */
	/* NB: this potentially destroys op2: */
	insn += TME_BIT(22) - TME_BIT(18);
	break;

      case 3: /* BPr */

	/* if bit 28 is set, or if the least significant two bits of
	   cond are clear, this is an illegal instruction: */
	if (__tme_predict_false((insn & TME_BIT(28))
				|| (insn & (0x3 << 25)) == TME_SPARC_COND_N)) {
	  TME_SPARC_INSN_TRAP(TME_SPARC_TRAP(ic,illegal_instruction));
	}

	/* decode rs1: */
	reg_rs1 = TME_FIELD_MASK_EXTRACTU(insn, TME_SPARC_FORMAT3_MASK_RS1);
	TME_SPARC_REG_INDEX(ic, reg_rs1);

	/* make a conditions mask, with the E and LE conditions if the
	   register is zero, and with the L and LE conditions if the
	   register is less than zero: */
	value_rs1 = ic->tme_sparc_ireg(reg_rs1);
	conds_mask
	  = (((value_rs1 == 0)
	      * (TME_BIT(TME_SPARC_COND_E)
		 + TME_BIT(TME_SPARC_COND_LE)))
	     | ((((tme_int64_t) value_rs1) < 0)
		* (TME_BIT(TME_SPARC_COND_L)
		   + TME_BIT(TME_SPARC_COND_LE))));

	/* add the not-conditions to the conditions mask: */
	conds_mask += ((conds_mask ^ 0xf) << 4);

	/* clear rs1 and p, move d16hi down, and clear d16hi: */
	insn
	  = ((insn & ~((2 << 21) - (1 << 14)))
	     + ((insn & (3 << 20)) >> (20 - 14)));

	/* flip the most significant bit of the disp16: */
	insn ^= TME_BIT(15);

	/* sign-extend the disp16 to a disp22: */
	/* NB: this potentially destroys op2: */
	insn += TME_BIT(22) - TME_BIT(15);
	break;

      case 5: /* FBPfcc: */
	TME_SPARC_INSN_FPU;

	/* get the right %fcc: */
	cc = TME_FIELD_MASK_EXTRACTU(insn, (0x3 << 20));
	if (cc == 0) {
	  cc = ic->tme_sparc_fpu_fsr / _TME_FIELD_MASK_FACTOR(TME_SPARC_FSR_FCC);
	}
	else {
	  cc = ic->tme_sparc_fpu_xfsr >> (2 * (cc - 1));
	}
	cc &= (TME_SPARC_FSR_FCC / _TME_FIELD_MASK_FACTOR(TME_SPARC_FSR_FCC));

	/* get the conditions mask: */
	conds_mask = _tme_sparc_conds_fcc[cc];

	/* add the not-conditions to the conditions mask: */
	conds_mask += ((conds_mask ^ 0xff) << 8);

	/* clear cc1, cc0, and p: */
	insn &= ~(TME_BIT(21) + TME_BIT(20) + TME_BIT(19));

	/* flip the most significant bit of the disp19: */
	insn ^= TME_BIT(18);

	/* sign-extend the disp19 to a disp22: */
	/* NB: this potentially destroys op2: */
	insn += TME_BIT(22) - TME_BIT(18);
	break;
	
#endif /* TME_SPARC_VERSION(ic) >= 9 */

      default:

      case 0: /* UNIMP: */
	TME_SPARC_INSN_TRAP(TME_SPARC_TRAP(ic,illegal_instruction));
	continue;

      case 2: /* Bicc: */
	conds_mask_icc = _tme_sparc_conds_icc[
#if TME_SPARC_VERSION(ic) < 9
	  TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_ICC)
#else  /* TME_SPARC_VERSION(ic) >= 9 */
	  TME_FIELD_MASK_EXTRACTU(ic->tme_sparc64_ireg_ccr, TME_SPARC64_CCR_ICC)
#endif /* TME_SPARC_VERSION(ic) >= 9 */
	];

	/* add the not-conditions to the conditions mask: */
	conds_mask = conds_mask_icc ^ 0xff;
	conds_mask = (conds_mask << 8) | conds_mask_icc;
	break;

      case 4: /* SETHI: */

	/* decode rd: */
	reg_rd = TME_FIELD_MASK_EXTRACTU(insn, TME_SPARC_FORMAT3_MASK_RD);
	TME_SPARC_REG_INDEX(ic, reg_rd);
	ic->tme_sparc_ireg(reg_rd) = (insn << 10);
	continue;

      case 6: /* FBfcc: */
	TME_SPARC_INSN_FPU;
	conds_mask_fcc = _tme_sparc_conds_fcc[TME_FIELD_MASK_EXTRACTU(ic->tme_sparc_fpu_fsr, TME_SPARC_FSR_FCC)];

	/* add the not-conditions to the conditions mask: */
	conds_mask = conds_mask_fcc ^ 0xff;
	conds_mask = (conds_mask << 8) | conds_mask_fcc;
	break;
      }

      /* get the condition field: */
      cond = TME_FIELD_MASK_EXTRACTU(insn, (0xf << 25));

      /* if this conditional branch is taken: */
      if (conds_mask & TME_BIT(cond)) {

	/* get the raw displacement: */
	disp = TME_FIELD_MASK_EXTRACTS(insn, 0x003fffff);

	/* if there is no recode support, and the raw displacement is zero: */
	if (__tme_predict_false(!TME_SPARC_HAVE_RECODE(ic)
				&& disp == 0)) {

	  /* a taken branch to . is probably a timing loop.  instead
	     of handling that here, which would involve function calls
	     that would probably hurt register allocation, instead we
	     just set a flag and pretend that this is the last
	     instruction in the burst.  when we start a new burst
	     above, we will find the flag set and do the handling
	     then: */
	  branch_dot = TRUE;
	  branch_dot_burst = ic->_tme_sparc_instruction_burst_remaining;
	  ic->_tme_sparc_instruction_burst_remaining = 0;

	  /* the raw displacement is zero: */
	  /* NB: this is not necessary for correctness, but is an
	     attempt to encourage better register allocation: */
	  disp = 0;
	}

	/* do the delayed control transfer: */
	pc_next_next
	  = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC)
	     + (disp << 2));
	if (TME_SPARC_VERSION(ic) >= 9) {
	  pc_next_next &= ic->tme_sparc_address_mask;
	}
	ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;

	/* if there is no recode support, and the delayed control
	   transfer target is the idle PC, and this idle type marks
	   the idle on a branch to the idle PC: */
	if (__tme_predict_false(!TME_SPARC_HAVE_RECODE(ic)
				&& pc_next_next == ic->tme_sparc_idle_pcs[0])) {
	  if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPES_TARGET_BRANCH)) {

	    /* mark the idle: */
	    TME_SPARC_IDLE_MARK(ic);
	  }
	}

	/* if this was a conditional branch, clear the annul bit in
           the instruction image: */
	if (cond & 7) {
	  insn &= ~TME_BIT(29);
	}
      }

      /* if the annul bit it set: */
      if (insn & TME_BIT(29)) {

	/* the next instruction will be annulled.  to get the
	   execution loop to pay attention to the annulled bit,
	   make the current instruction TLB entry invalid: */
	annulled = TRUE;
	assert (ic->_tme_sparc_itlb_current_token
		== itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
	tme_sparc_tlb_unbusy(itlb_current);
	itlb_current = &itlb_invalid;
	tme_token_busy(&token_invalid);
	ic->_tme_sparc_itlb_current_token = &token_invalid;
      }
    }

    /* otherwise, this is a format one instruction: */
    else {

      /* get the current PC: */
      pc = ic->tme_sparc_ireg(TME_SPARC_IREG_PC);

      /* write the PC of the CALL into r[15]: */
      ic->tme_sparc_ireg(((ic)->tme_sparc_reg8_offset[15 / 8] * 8) + 15) = pc;

      /* get the delayed control transfer target: */
      pc_next_next = pc + (tme_int32_t) (insn << 2);
      if (TME_SPARC_VERSION(ic) >= 9) {
	pc_next_next &= ic->tme_sparc_address_mask;
      }

      /* if there is no recode support, and the delayed control
	 transfer target is the idle PC, and this idle type marks
	 the idle on a call to the idle PC: */
      if (__tme_predict_false(!TME_SPARC_HAVE_RECODE(ic)
			      && pc_next_next == ic->tme_sparc_idle_pcs[0])) {
	if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPES_TARGET_CALL)) {

	  /* mark the idle: */
	  TME_SPARC_IDLE_MARK(ic);
	}
      }

      /* log the call: */
      reg_o0 = 8;
      TME_SPARC_REG_INDEX(ic, reg_o0);
      tme_sparc_log(ic, 250, TME_OK,
		    (TME_SPARC_LOG_HANDLE(ic),
		     _("call " TME_PRIxSPARCREG " %%o0 " TME_PRIxSPARCREG " %%o1 " TME_PRIxSPARCREG " %%o2 " TME_PRIxSPARCREG " %%o3 " TME_PRIxSPARCREG " %%o4 " TME_PRIxSPARCREG " %%o5 " TME_PRIxSPARCREG),
		     pc_next_next,
		     ic->tme_sparc_ireg(reg_o0 + 0),
		     ic->tme_sparc_ireg(reg_o0 + 1),
		     ic->tme_sparc_ireg(reg_o0 + 2),
		     ic->tme_sparc_ireg(reg_o0 + 3),
		     ic->tme_sparc_ireg(reg_o0 + 4),
		     ic->tme_sparc_ireg(reg_o0 + 5)));

      /* do the delayed control transfer: */
      ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;
    }
  }
  
  /* NOTREACHED */
}
