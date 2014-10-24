/* $Id: sparc-rc-ls.c,v 1.6 2010/06/05 17:08:57 fredette Exp $ */

/* ic/sparc/sparc-rc-cc.c - SPARC recode ld/st support: */

/*
 * Copyright (c) 2008 Matt Fredette
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

#if TME_SPARC_RECODE_SIZE(ic) == TME_RECODE_SIZE_32

_TME_RCSID("$Id: sparc-rc-ls.c,v 1.6 2010/06/05 17:08:57 fredette Exp $");

/* macros: */

/* rename various things by the architecture size: */
#define _tme_sparc_recode_ls_assist_check _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_ls_assist_check)
#define _tme_sparc_recode_ls_assist_ld _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_ls_assist_ld)
#define _tme_sparc_recode_ls_assist_st _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_ls_assist_st)
#define tme_sparc_recode_ls_tlb_update _TME_SPARC_RECODE_SIZE(tme_sparc,_recode_ls_tlb_update)
#define _tme_sparc_recode_ls_init _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_ls_init)

#endif /* TME_SPARC_RECODE_SIZE(ic) == TME_RECODE_SIZE_32 */

/* this checks whether a load or store needs an assist: */
void
_tme_sparc_recode_ls_assist_check(const struct tme_sparc *ic,
				  tme_recode_uguest_t recode_address,
				  int assisted)
{
  tme_uint32_t sparc_insn;
  unsigned int reg_rs2;
  tme_sparc_ireg_t sparc_address;
  unsigned int reg_rs1;
  tme_uint32_t sparc_opcode;
  tme_bus_context_t context;
  tme_uint32_t asi_mask_flags_assist;
  unsigned int asi;
  tme_uint32_t asi_mask_flags;
  const struct tme_sparc_tlb *dtlb;
  tme_uint32_t asi_mask;
  tme_uint32_t dtlb_page_size;
  tme_sparc_ireg_t sparc_page;

  /* get the instruction: */
  sparc_insn = ic->_tme_sparc_insn;

  /* if the i bit is zero: */
  if ((sparc_insn & TME_BIT(13)) == 0) {

    /* decode rs2: */
    reg_rs2 = TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS2);
    TME_SPARC_REG_INDEX(ic, reg_rs2);
    sparc_address = ic->tme_sparc_ireg(reg_rs2);
  }

  /* otherwise, the i bit is one: */
  else {

    /* decode simm13: */
    sparc_address = TME_FIELD_MASK_EXTRACTS(sparc_insn, (tme_sparc_ireg_t) 0x1fff);
  }

  /* decode rs1: */
  reg_rs1 = TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS1);
  TME_SPARC_REG_INDEX(ic, reg_rs1);
  sparc_address += ic->tme_sparc_ireg(reg_rs1);

  /* mask the addresses: */
  if (TME_SPARC_VERSION(ic) >= 9) {
    sparc_address &= ic->tme_sparc_address_mask;
    recode_address &= ic->tme_sparc_address_mask;
  }

  /* check the address: */
  assert (sparc_address == (tme_sparc_ireg_t) recode_address);

  /* if this instruction is being assisted, return now: */
  if (assisted) {
    return;
  }

  /* the verifier must not be on: */
  assert (!_tme_sparc_recode_verify_on);

  /* get the instruction opcode: */
  sparc_opcode = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x3f << 19));

  /* the address must be aligned: */
  assert ((sparc_address
	   & ((TME_SPARC_VERSION(ic) >= 9
	       && (sparc_opcode == 0x0b /* v9: ldx */
		   || sparc_opcode == 0x0e))
	      ? (sizeof(tme_sparc_ireg_t) - 1)
	      : ((sparc_opcode - 1)
		 & (sizeof(tme_uint32_t) - 1)))) == 0);

  /* assume that this load or store uses the default context: */
  context = ic->tme_sparc_memory_context_default;
  
  /* assume that only DTLB ASI mask flags for special ASIs must be assisted: */
  asi_mask_flags_assist = TME_SPARC_ASI_MASK_FLAG_SPECIAL;

  /* if this is a v9 CPU, assume that a load or store to a no-fault
     address must be assisted: */
  if (TME_SPARC_VERSION(ic) >= 9) {
    asi_mask_flags_assist += TME_SPARC64_ASI_FLAG_NO_FAULT;
  }

  /* if this is a load or store with an alternate space: */
  if (__tme_predict_false(sparc_insn & TME_BIT(19 + 4))) {

    /* get the ASI: */
    asi = ((TME_SPARC_VERSION(ic) >= 9
	    && (sparc_insn & TME_BIT(13)))
	   ? ic->tme_sparc64_ireg_asi
	   : TME_FIELD_MASK_EXTRACTU(sparc_insn, (0xff << 5)));

    /* get the flags for this ASI: */
    asi_mask_flags = ic->tme_sparc_asis[asi].tme_sparc_asi_mask_flags;

    /* if this is a v9 CPU: */
    if (TME_SPARC_VERSION(ic) >= 9) {

      /* get the context: */
      context
	= (((TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS)
	    && (asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS))
	   ? 0
	   : (asi_mask_flags & TME_SPARC64_ASI_FLAG_SECONDARY)
	   ? ic->tme_sparc_memory_context_secondary
	   : ic->tme_sparc_memory_context_primary);
    }

    /* this must be a load from an alternate space: */
    assert ((sparc_insn & TME_BIT(19 + 2)) == 0);

    /* this must be a v9 CPU: */
    assert (TME_SPARC_VERSION(ic) >= 9);

    /* this v9 CPU must not be in nucleus context: */
    /* NB: this implies that when a v9 CPU is in nucleus context, even
       loads that specify an immediate no-fault ASI are assisted: */
    assert ((TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS) == 0
	    || ic->tme_sparc64_ireg_tl == 0);

    /* this ASI must be the default data ASI with the no-fault bit
       set: */
    assert (asi 
	    == (TME_SPARC_ASI_MASK_WHICH(ic->tme_sparc_asi_mask_data)
		+ TME_SPARC64_ASI_FLAG_NO_FAULT));

    /* this load doesn't necessarily need an assist if it's to a
       no-fault address: */
    asi_mask_flags_assist -= TME_SPARC64_ASI_FLAG_NO_FAULT;
  }

  /* get the DTLB entry: */
  dtlb = &ic->tme_sparc_tlbs[TME_SPARC_DTLB_ENTRY(ic, TME_SPARC_TLB_HASH(ic, context, sparc_address))];

  /* the DTLB entry must be valid: */
  assert (!tme_bus_tlb_is_invalid(&dtlb->tme_sparc_tlb_bus_tlb));

  /* the DTLB entry must match the context: */
  assert (dtlb->tme_sparc_tlb_context == context
	  || dtlb->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max);

  /* the DTLB entry must cover the page: */
  dtlb_page_size = (1 << ic->tme_sparc_tlb_page_size_log2);
  sparc_page = sparc_address & (0 - (tme_sparc_ireg_t) dtlb_page_size);
  assert ((((tme_sparc_ireg_t) dtlb->tme_sparc_tlb_addr_first)
	   <= sparc_page)
	  && (((tme_sparc_ireg_t) dtlb->tme_sparc_tlb_addr_last)
	      >= (sparc_page + dtlb_page_size - 1)));

  /* if the DTLB entry doesn't allow this privilege load or store: */
  asi_mask
    = (TME_SPARC_VERSION(ic) >= 9
       ? (TME_SPARC_PRIV(ic)
	  ? TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(!TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
	  : TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER))
       : (TME_SPARC_PRIV(ic)
	  ? TME_SPARC32_ASI_MASK_SD
	  : TME_SPARC32_ASI_MASK_UD));
  assert (TME_SPARC_TLB_ASI_MASK_OK(dtlb, asi_mask));

  /* the DTLB entry ASI flags must not require an assist: */
  assert ((dtlb->tme_sparc_tlb_asi_mask
	   & asi_mask_flags_assist) == 0);

  /* the DTLB entry must allow the fast load or store, and if this is
     a store, it must not allow fast loads, or it must allow both to
     the same memory: */
  assert ((sparc_insn & TME_BIT(19 + 2))
	  ? (dtlb->tme_sparc_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF
	     && (dtlb->tme_sparc_tlb_emulator_off_read == TME_EMULATOR_OFF_UNDEF
		 || dtlb->tme_sparc_tlb_emulator_off_read == dtlb->tme_sparc_tlb_emulator_off_write))
	  : (dtlb->tme_sparc_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF));
}

/* the assist function for ld instructions: */
static tme_recode_uguest_t
_tme_sparc_recode_ls_assist_ld(struct tme_ic *_ic,
			       tme_recode_uguest_t address)
{
  struct tme_sparc *ic;
  tme_uint32_t sparc_insn;
  tme_uint32_t sparc_opcode;
  int insn_is_ld;
  unsigned int reg_rd;
#if TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic)
  tme_sparc_ireg_t rs1_buffer;
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic) */
  tme_sparc_ireg_t rs2;
  tme_sparc_ireg_t *_rs1;

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_ld);

  /* set PC_next_next from PC_next: */
  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT)
    = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
       + sizeof(tme_uint32_t));

  /* get the instruction: */
  sparc_insn = tme_sparc_recode_insn_current(ic);
  ic->_tme_sparc_insn = sparc_insn;

  /* see if this instruction is a load: */
  sparc_opcode = 0x40 + TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x3f << 19));
  insn_is_ld
    = (sparc_insn >= 0xc0000000
       && _tme_sparc_recode_insn_opmap[sparc_opcode] == TME_SPARC_RECODE_INSN_LD);

  /* if this is a cooperative threading system: */
  if (TME_THREADS_COOPERATIVE) {

    /* this instruction must be the original load instruction from
       when the instructions thunk was recoded.  we have no way to
       confirm this.  if it's not, because this is a cooperative
       threading system, that means that either
       tme_sparc_recode_insn_assist_redispatch(), or
       tme_sparc_recode(), or the host chain in failed to detect that
       the page containing this PC is no longer cache-valid: */
    assert (insn_is_ld);
  }

  /* otherwise, this isn't a cooperative threading system: */
  else {

    /* if this isn't a load instruction: */
    if (__tme_predict_false(!insn_is_ld)) {
      abort();
    }

    /* otherwise, this is a load instruction: */

    /* NB: it's possible that this load instruction is not the
       original load instruction from when the instructions thunk was
       recoded.  we have no way of knowing if it is or isn't.  if it
       isn't, this new load instruction may have different rs1, rs2,
       and rd fields than the original load instruction: */
    abort();
  }

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_opcode[sparc_opcode]);

  /* decode rd: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RD);
  TME_SPARC_REG_INDEX(ic, reg_rd);

  /* run the instruction: */
  rs2 = 0;
#if TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic)
  rs1_buffer = address;
  _rs1 = &rs1_buffer;
#else  /* TME_RECODE_SIZE_GUEST_MAX == TME_SPARC_RECODE_SIZE(ic) */
  _rs1 = &address;
#endif /* TME_RECODE_SIZE_GUEST_MAX == TME_SPARC_RECODE_SIZE(ic) */
#ifdef _TME_SPARC_RECODE_VERIFY
  _tme_sparc_recode_ls_assist_check(ic, *_rs1, TRUE);
#endif /* _TME_SPARC_RECODE_VERIFY */
  (*ic->_tme_sparc_execute_opmap[sparc_opcode])
    (ic,
     _rs1,
     &rs2,
     &ic->tme_sparc_ireg(reg_rd));

  /* set %g0 to zero, in case the instruction changed it: */
  ic->tme_sparc_ireg(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G0) = 0;

  /* do any required redispatch: */
  /* NB: in a noncooperative threading system, this will do any needed
     redispatch after we executed a new load instruction that is
     different from the original load instruction from when the
     instructions thunk was recoded.  in that case, we can't return to
     the instructions thunk since it will take our return value for
     the rd register in the original instruction, not for the rd
     register in the new instruction: */
  /* NB: in any threading system, this will do any needed redispatch
     after a load instruction changes the state of the system such
     that we can't return to the instructions thunk.  this is very
     unusual: */
  tme_sparc_recode_insn_assist_redispatch(ic);

  /* return the result: */
  return (ic->tme_sparc_ireg(reg_rd));
}

/* the assist function for st instructions: */
static void
_tme_sparc_recode_ls_assist_st(struct tme_ic *_ic,
			       tme_recode_uguest_t address,
			       tme_recode_uguest_t rd)
{
  struct tme_sparc *ic;
  tme_uint32_t sparc_insn;
  tme_uint32_t sparc_opcode;
  int insn_is_st;
#if TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic)
  tme_sparc_ireg_t rs1_buffer;
  tme_sparc_ireg_t rd_buffer;
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic) */
  tme_sparc_ireg_t *_rs1;
  tme_sparc_ireg_t rs2;
  tme_sparc_ireg_t *_rd;

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_st);

  /* set PC_next_next from PC_next: */
  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT)
    = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
       + sizeof(tme_uint32_t));

  /* get the instruction: */
  sparc_insn = tme_sparc_recode_insn_current(ic);
  ic->_tme_sparc_insn = sparc_insn;

  /* see if this instruction is a store: */
  sparc_opcode = (0x40 + TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x3f << 19)));
  insn_is_st
    = (sparc_insn >= 0xc0000000
       && _tme_sparc_recode_insn_opmap[sparc_opcode] == TME_SPARC_RECODE_INSN_ST);

  /* if this is a cooperative threading system: */
  if (TME_THREADS_COOPERATIVE) {

    /* this instruction must be the original store instruction from
       when the instructions thunk was recoded.  we have no way to
       confirm this.  if it's not, because this is a cooperative
       threading system, that means that either
       tme_sparc_recode_insn_assist_redispatch(), or
       tme_sparc_recode(), or the host chain in failed to detect that
       the page containing this PC is no longer cache-valid: */
    assert (insn_is_st);
  }

  /* otherwise, this isn't a cooperative threading system: */
  else {

    /* if this isn't a store instruction: */
    if (__tme_predict_false(!insn_is_st)) {
      abort();
    }

    /* otherwise, this is a store instruction: */

    /* NB: it's possible that this store instruction is not the
       original store instruction from when the instructions thunk was
       recoded.  we have no way of knowing if it is or isn't.  if it
       isn't, this new store instruction may have different rs1, rs2,
       and rd fields than the original store instruction: */
    abort();
  }

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_opcode[sparc_opcode]);

  /* run the store instruction: */
  rs2 = 0;
#if TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic)
  rs1_buffer = address;
  rd_buffer = rd;
  _rs1 = &rs1_buffer;
  _rd = &rd_buffer;
#else  /* TME_RECODE_SIZE_GUEST_MAX == TME_SPARC_RECODE_SIZE(ic) */
  _rs1 = &address;
  _rd = &rd;
#endif /* TME_RECODE_SIZE_GUEST_MAX == TME_SPARC_RECODE_SIZE(ic) */
#ifdef _TME_SPARC_RECODE_VERIFY
  _tme_sparc_recode_ls_assist_check(ic, *_rs1, TRUE);
#endif /* _TME_SPARC_RECODE_VERIFY */
  (*ic->_tme_sparc_execute_opmap[sparc_opcode])
    (ic, _rs1, &rs2, _rd);

  /* return no result: */
  tme_sparc_recode_insn_assist_redispatch(ic);
}

/* this updates a recode DTLB entry: */
void
tme_sparc_recode_ls_tlb_update(struct tme_sparc *ic,
			       const struct tme_sparc_ls *ls)
{
  const struct tme_sparc_tlb *dtlb;
  tme_uint32_t dtlb_page_size;
  struct _TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,/**/) *recode_tlb;
  tme_uint32_t rw_flags_tlb;
  tme_sparc_ireg_t recode_tlb_page;

  /* get the sparc DTLB entry: */
  dtlb = ls->tme_sparc_ls_tlb;

  /* get the recode DTLB entry: */
  recode_tlb = &ic->_TME_SPARC_RECODE_SIZE(tme_sparc_recode_tlb,s)[ls->tme_sparc_ls_tlb_i];

  /* assume that this recode DTLB entry will assist all loads and
     stores (this mask includes at least
     TME_SPARC_RECODE_TLB_FLAG_LD_USER(ic),
     TME_SPARC_RECODE_TLB_FLAG_LD_PRIV(ic),
     TME_SPARC_RECODE_TLB_FLAG_ST_USER(ic), and
     TME_SPARC_RECODE_TLB_FLAG_ST_PRIV(ic)): */
  rw_flags_tlb = TME_RECODE_TLB_FLAGS_MASK(ic->tme_sparc_recode_ic);

  /* get our DTLB page size: */
  dtlb_page_size = (1 << ic->tme_sparc_tlb_page_size_log2);

  /* update the recode DTLB entry page: */
  recode_tlb_page = ls->_TME_SPARC_RECODE_SIZE(tme_sparc_ls_address,/**/) & (0 - (tme_sparc_ireg_t) dtlb_page_size);
  recode_tlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_page) = recode_tlb_page;

  /* if the sparc DTLB entry covers an entire page for one or more
     normal ASIs, and verification isn't on: */
  if ((((tme_sparc_ireg_t) dtlb->tme_sparc_tlb_addr_first)
       <= recode_tlb_page)
      && ((recode_tlb_page | (dtlb_page_size - 1))
	  <= ((tme_sparc_ireg_t) dtlb->tme_sparc_tlb_addr_last))
      && (dtlb->tme_sparc_tlb_asi_mask & TME_SPARC_ASI_MASK_FLAG_SPECIAL) == 0
      && !_tme_sparc_recode_verify_on) {

    /* if the sparc DTLB entry allows fast reading: */
    if (dtlb->tme_sparc_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF) {

      /* update the recode DTLB entry page memory pointer: */
      recode_tlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_memory)
	= (dtlb->tme_sparc_tlb_emulator_off_read
	   + recode_tlb_page);

      /* if this sparc DTLB entry allows user loads: */
      if (TME_SPARC_TLB_ASI_MASK_OK(dtlb, 
				    (TME_SPARC_VERSION(ic) >= 9
				     ? TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
				     : TME_SPARC32_ASI_MASK_UD))) {

	/* this recode DTLB entry probably won't need an assist for
	   user loads (a sparc DTLB entry with side-effects will
	   override this): */
	rw_flags_tlb
	  -= TME_SPARC_RECODE_TLB_FLAG_LD_USER(ic);
      }

      /* if this sparc DTLB entry allows privileged/supervisor
	 loads: */
      if (TME_SPARC_TLB_ASI_MASK_OK(dtlb, 
				    (TME_SPARC_VERSION(ic) >= 9
				     ? TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(!TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
				     : TME_SPARC32_ASI_MASK_SD))) {

	/* this recode DTLB entry probably won't need an assist for
	   privileged/supervisor loads (a sparc DTLB entry with
	   side-effects will override this): */
	rw_flags_tlb
	  -= TME_SPARC_RECODE_TLB_FLAG_LD_PRIV(ic);
      }
    }

    /* if the sparc DTLB entry allows fast writing, and it either
       doesn't allow fast reading or the fast read and write pointers
       are the same: */
    if (dtlb->tme_sparc_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF
	&& (dtlb->tme_sparc_tlb_emulator_off_read == TME_EMULATOR_OFF_UNDEF
	    || dtlb->tme_sparc_tlb_emulator_off_write == dtlb->tme_sparc_tlb_emulator_off_read)) {

      /* update the recode DTLB entry page memory pointer: */
      recode_tlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_memory)
	= (dtlb->tme_sparc_tlb_emulator_off_write
	   + recode_tlb_page);

      /* if this sparc DTLB entry allows user stores: */
      if (TME_SPARC_TLB_ASI_MASK_OK(dtlb, 
				    (TME_SPARC_VERSION(ic) >= 9
				     ? TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
				     : TME_SPARC32_ASI_MASK_UD))) {

	/* this recode DTLB entry probably won't need an assist for
	   user stores (a sparc DTLB entry with side-effects will
	   override this): */
	rw_flags_tlb
	  -= TME_SPARC_RECODE_TLB_FLAG_ST_USER(ic);
      }

      /* if this sparc DTLB entry allows privileged/supervisor
	 stores: */
      if (TME_SPARC_TLB_ASI_MASK_OK(dtlb, 
				    (TME_SPARC_VERSION(ic) >= 9
				     ? TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(!TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
				     : TME_SPARC32_ASI_MASK_SD))) {

	/* this recode DTLB entry probably won't need an assist for
	   privileged/supervisor stores (a sparc DTLB entry with
	   side-effects will override this): */
	rw_flags_tlb
	  -= TME_SPARC_RECODE_TLB_FLAG_ST_PRIV(ic);
      }
    }

    /* this recode DTLB entry needs an assist for no-fault loads when
       the ASI register is not loaded with the correct default
       no-fault ASI: */
    assert (rw_flags_tlb & TME_SPARC_RECODE_TLB_FLAG_LD_NF(ic));

    /* if this sparc DTLB entry does not only allow no-fault loads: */
    if (TME_SPARC_VERSION(ic) < 9
	|| (dtlb->tme_sparc_tlb_asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT) == 0) {

      /* this recode DTLB entry doesn't need an assist for faulting
	 loads: */
      rw_flags_tlb -= TME_SPARC_RECODE_TLB_FLAG_LD_F(ic);
    }

    /* if this sparc DTLB entry is for addresses with side-effects
       (regardless of whether or not the sparc DTLB entry allows fast
       reads and/or writes): */
    if (TME_SPARC_VERSION(ic) >= 9
	&& (dtlb->tme_sparc_tlb_asi_mask & TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS)) {

      /* this recode DTLB entry needs an assist for all loads and stores: */
      rw_flags_tlb
	|= (TME_SPARC_RECODE_TLB_FLAG_LD_USER(ic)
	    | TME_SPARC_RECODE_TLB_FLAG_LD_PRIV(ic)
	    | TME_SPARC_RECODE_TLB_FLAG_ST_USER(ic)
	    | TME_SPARC_RECODE_TLB_FLAG_ST_PRIV(ic));
    }

    /* if this is sparc32, or if this sparc64 MMU has an invert-endian bit: */
    if (TME_SPARC_VERSION(ic) < 9
	|| (TME_SPARC_MEMORY_FLAGS(ic) & TME_SPARC_MEMORY_FLAG_HAS_INVERT_ENDIAN)) {

      /* this recode DTLB entry doesn't need an assist for loads and
	 stores based on their explicit endianness: */
      rw_flags_tlb
	-= (TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_BIG(ic)
	    + TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_LITTLE(ic));

      /* if this is sparc32, or if this sparc64 DTLB entry's
	 invert-endian bit is not set: */
      if (TME_SPARC_VERSION(ic) < 9
	  || (dtlb->tme_sparc_tlb_asi_mask & TME_SPARC64_ASI_FLAG_LITTLE) == 0) {

	/* this recode DTLB entry doesn't need an assist for loads and
	   stores to invert their endianness: */
	rw_flags_tlb
	  -= TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_INVERT(ic);
      }
    }

    /* otherwise, we don't know how this v9 MMU affects endianness: */
    else {
      assert (FALSE);
    }

    /* update the recode DTLB entry context: */
    recode_tlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_context)
      = dtlb->tme_sparc_tlb_context;

    /* if this sparc DTLB entry matches any context: */
    if (dtlb->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max) {

      /* this recode DTLB entry doesn't need an assist for loads and
	 stores whose context mismatches the recode DTLB entry: */
      rw_flags_tlb
	-= TME_RECODE_TLB_FLAG_CONTEXT_MISMATCH(ic);
    }
  }

  /* update the recode DTLB entry read/write flags: */
  recode_tlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_flags) = rw_flags_tlb;
}

/* this initializes for ld and st instructions: */
static void
_tme_sparc_recode_ls_init(struct tme_sparc *ic)
{
  signed long rw_thunk_index;
  struct tme_recode_rw rw;
  tme_uint32_t sparc_opcode_ldu;
  tme_uint32_t sparc_opcode_st;
  tme_uint32_t rw_flags_insn_common;
  tme_uint32_t rw_flags_insn;
  unsigned int other_signed;
  unsigned int no_fault;

  /* initialize the array of read/write thunks: */
  for (rw_thunk_index = 0;
       rw_thunk_index < TME_ARRAY_ELS(ic->tme_sparc_recode_rw_thunks);
       rw_thunk_index++) {
    ic->tme_sparc_recode_rw_thunks[rw_thunk_index] = NULL;
  }

  /* make the common read/write structure for all sizes: */
  rw.tme_recode_rw_bus_boundary = TME_BIT(TME_SPARC_RECODE_SIZE(ic) - TME_RECODE_SIZE_8);
  rw.tme_recode_rw_address_type.tme_recode_address_type_context_ic_offset
    = (((char *) &ic->tme_sparc_memory_context_default) - (char *) ic);
  rw.tme_recode_rw_address_type.tme_recode_address_type_context_size
    = TME_RECODE_SIZE_16;
  rw.tme_recode_rw_address_type.tme_recode_address_type_signed = FALSE;
  rw.tme_recode_rw_address_type.tme_recode_address_type_tlb_flags_ic_offset
    = (((char *) &ic->tme_sparc_recode_rw_tlb_flags) - (char *) ic);
  rw.tme_recode_rw_address_type.tme_recode_address_type_mask_tlb_index
    = ((_TME_SPARC_DTLB_HASH_SIZE - 1)
       * ic->tme_sparc_recode_ic->tme_recode_ic_tlb_page_size);
  rw.tme_recode_rw_address_type.tme_recode_address_type_tlb0_ic_offset
    = (((char *) &ic->_tme_sparc_tlb_tokens_u) - (char *) ic);

  /* initialize the read/write structure for the first ld and st
     instructions: */
  rw.tme_recode_rw_address_type.tme_recode_address_type_size = TME_SPARC_RECODE_SIZE(ic);
  rw.tme_recode_rw_memory_endian = TME_ENDIAN_BIG;
  rw.tme_recode_rw_memory_size = TME_RECODE_SIZE_8;
  for (;;) {

    /* get the sparc ld/st opcodes for this size: */
    sparc_opcode_ldu = TME_BIT(rw.tme_recode_rw_memory_size - TME_RECODE_SIZE_8) % sizeof(tme_uint32_t);
    sparc_opcode_st = sparc_opcode_ldu + 4;
    if (TME_SPARC_VERSION(ic) >= 9
	&& rw.tme_recode_rw_memory_size == TME_SPARC_RECODE_SIZE(ic)) {
      sparc_opcode_ldu = 0x0b; /* v9: ldx */
      sparc_opcode_st = 0x0e; /* v9: stx */
    }

    /* make the common read/write structure for this size: */
    rw.tme_recode_rw_address_type.tme_recode_address_type_align_min
      = TME_BIT(rw.tme_recode_rw_memory_size - TME_RECODE_SIZE_8);

    /* start the common read/write structure flags: */
    rw_flags_insn_common = 0;

    /* all loads and stores will need an assist when they encounter a
       recode TLB entry from the wrong context: */
    /* NB: TME_RECODE_TLB_FLAG_CONTEXT_MISMATCH() will get set
       automatically as needed by the read/write thunk; there's no
       need to set it here: */

    /* all loads and stores will need an assist when they encounter a
       recode TLB entry that inverts their endianness: */
    rw_flags_insn_common |= TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_INVERT(ic);

    /* all loads and stores will need an assist when they encounter a
       recode TLB entry that doesn't allow their endianness: */
    rw_flags_insn_common
      |= (rw.tme_recode_rw_memory_endian == TME_ENDIAN_BIG
	  ? TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_BIG(ic)
	  : TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_LITTLE(ic));

    /* make the common structure for loads: */
    rw.tme_recode_rw_write = FALSE;
    rw.tme_recode_rw_reg_size = TME_SPARC_RECODE_SIZE(ic);
    rw.tme_recode_rw_guest_func_read = _tme_sparc_recode_ls_assist_ld;

    /* set the signedness of the (first) ld instructions at this size
       that we will make a read thunk for: */
    /* NB: we assume that 8- and 32-bit loads are signed more often
       than not (because we assume that the C char and int types are
       used more often than the unsigned char and unsigned int types),
       so we make their thunks first, assuming that if we made them
       second, that they might reuse an unsigned thunk and then
       require extra sign-extension instructions in an instructions
       thunk: */
    rw.tme_recode_rw_memory_signed
      = (rw.tme_recode_rw_memory_size < TME_SPARC_RECODE_SIZE(ic)
	 && (rw.tme_recode_rw_memory_size == TME_RECODE_SIZE_8
	     || rw.tme_recode_rw_memory_size == TME_RECODE_SIZE_32));

    /* permute over signedness and no-fault, making the read thunk(s)
       for the ld instruction(s) with this size and endianness: */
    for (other_signed = 0;
	 other_signed <= (rw.tme_recode_rw_memory_size < TME_SPARC_RECODE_SIZE(ic));
	 other_signed++, rw.tme_recode_rw_memory_signed = !rw.tme_recode_rw_memory_signed) {
      for (no_fault = 0;
	   no_fault <= (TME_SPARC_VERSION(ic) >= 9);
	   no_fault++) {

	/* start the read structure flags: */
	rw_flags_insn = rw_flags_insn_common;

	/* this load will need an assist when it encounters a recode
	   TLB entry that needs an assist on all loads (because the
	   sparc TLB entry doesn't allow fast reads, or has
	   side-effects) or doesn't allow loads at the current
	   privilege level (at load instruction time, the and of the
	   recode ic read/write flags will leave only one of these
	   flags set): */
	rw_flags_insn
	  |= (TME_SPARC_RECODE_TLB_FLAG_LD_USER(ic)
	      | TME_SPARC_RECODE_TLB_FLAG_LD_PRIV(ic));

	/* if this is a no-fault load: */
	if (no_fault) {

	  /* this no-fault load will need an assist when the ASI
	     register is not loaded with the correct default no-fault
	     ASI (at load instruction time, the and of the recode ic
	     read/write flags will clear this bit if the ASI register
	     is correct, and recode TLB entries always need this
	     assist): */
	  rw_flags_insn |= TME_SPARC_RECODE_TLB_FLAG_LD_NF(ic);
	}

	/* otherwise, this is a faulting load: */
	else {

	  /* this faulting load will need an assist when it encounters
	     a recode TLB entry that doesn't allow faulting loads
	     (i.e., it only allows non-faulting loads): */
	  rw_flags_insn |= TME_SPARC_RECODE_TLB_FLAG_LD_F(ic);
	}

	/* make the read thunk for this ld instruction: */
	rw.tme_recode_rw_address_type.tme_recode_address_type_tlb_flags = rw_flags_insn;
	ic->tme_sparc_recode_rw_thunks
	  [TME_SPARC_RECODE_RW_THUNK_INDEX(ic,
					   rw.tme_recode_rw_address_type.tme_recode_address_type_size,
					   rw.tme_recode_rw_memory_endian,
					   no_fault,
					   (sparc_opcode_ldu
					    + (8 * rw.tme_recode_rw_memory_signed)))]
	  /* NB: for an 8-bit little-endian ld instructions, we reuse
	     the 8-bit big-endian read thunk: */
	  = ((rw.tme_recode_rw_memory_size == TME_RECODE_SIZE_8
	      && rw.tme_recode_rw_memory_endian == TME_ENDIAN_LITTLE)
	     ? (ic->tme_sparc_recode_rw_thunks
		[TME_SPARC_RECODE_RW_THUNK_INDEX(ic,
						 rw.tme_recode_rw_address_type.tme_recode_address_type_size,
						 TME_ENDIAN_BIG,
						 no_fault,
						 (sparc_opcode_ldu
						  + (8 * rw.tme_recode_rw_memory_signed)))])
	     : tme_recode_rw_thunk(ic->tme_sparc_recode_ic, &rw));
      }
    }

    /* make the common structure for reads: */
    rw.tme_recode_rw_write = TRUE;
    rw.tme_recode_rw_reg_size = rw.tme_recode_rw_memory_size;
    rw.tme_recode_rw_guest_func_write = _tme_sparc_recode_ls_assist_st;

    /* start the write structure flags: */
    rw_flags_insn = rw_flags_insn_common;

    /* this store will need an assist when it encounters a recode TLB
       entry that needs an assist on all store (because the sparc TLB
       entry doesn't allow fast stores, or has side-effects, or only
       allows no-fault loads) or doesn't allow stores at the current
       privilege level (at store instruction time, the and of the
       recode ic read/write flags will leave only one of these flags
       set): */
    rw_flags_insn
      |= (TME_SPARC_RECODE_TLB_FLAG_ST_USER(ic)
	  | TME_SPARC_RECODE_TLB_FLAG_ST_PRIV(ic));

    /* make the write thunk for this st instruction: */
    rw.tme_recode_rw_address_type.tme_recode_address_type_tlb_flags = rw_flags_insn;
    ic->tme_sparc_recode_rw_thunks
      [TME_SPARC_RECODE_RW_THUNK_INDEX(ic,
				       rw.tme_recode_rw_address_type.tme_recode_address_type_size,
				       rw.tme_recode_rw_memory_endian,
				       FALSE,
				       sparc_opcode_st)]
      /* NB: for an 8-bit little-endian st instruction, we reuse the
	 8-bit big-endian write thunk: */
      = ((rw.tme_recode_rw_memory_size == TME_RECODE_SIZE_8
	  && rw.tme_recode_rw_memory_endian == TME_ENDIAN_LITTLE)
	 ? (ic->tme_sparc_recode_rw_thunks
	    [TME_SPARC_RECODE_RW_THUNK_INDEX(ic,
					     rw.tme_recode_rw_address_type.tme_recode_address_type_size,
					     TME_ENDIAN_BIG,
					     FALSE,
					     sparc_opcode_st)])
	 : tme_recode_rw_thunk(ic->tme_sparc_recode_ic, &rw));
  
    /* advance: */
    if (++rw.tme_recode_rw_memory_size > TME_SPARC_RECODE_SIZE(ic)) {
      rw.tme_recode_rw_memory_size = TME_RECODE_SIZE_8;
      if (rw.tme_recode_rw_memory_endian == TME_ENDIAN_BIG
	  && TME_SPARC_VERSION(ic) >= 9) {
	rw.tme_recode_rw_memory_endian = TME_ENDIAN_LITTLE;
      }
      else {
	rw.tme_recode_rw_memory_endian = TME_ENDIAN_BIG;
	if (--rw.tme_recode_rw_address_type.tme_recode_address_type_size
	    < (TME_SPARC_RECODE_SIZE(ic) - (TME_SPARC_VERSION(ic) >= 9))) {
	  break;
	}
      }
    }
  }
}
