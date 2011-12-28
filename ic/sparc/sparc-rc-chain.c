/* $Id: sparc-rc-chain.c,v 1.2 2010/02/14 15:17:56 fredette Exp $ */

/* ic/sparc/sparc-rc-chain.c - SPARC recode chain support: */

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

#if TME_SPARC_RECODE_SIZE(ic) == TME_RECODE_SIZE_32

_TME_RCSID("$Id: sparc-rc-chain.c,v 1.2 2010/02/14 15:17:56 fredette Exp $");

/* macros: */

/* rename various things by the architecture size: */
#define _tme_sparc_recode_chain_src_key _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_chain_src_key)
#define _tme_sparc_recode_chain_insns_thunk _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_chain_insns_thunk)
#define _tme_sparc_recode_chain_fixup _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_chain_fixup)
#define tme_sparc_recode_chain_tlb_update _TME_SPARC_RECODE_SIZE(tme_sparc,_recode_chain_tlb_update)
#define _tme_sparc_recode_chain_init _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_chain_init)

#endif /* TME_SPARC_RECODE_SIZE(ic) == TME_RECODE_SIZE_32 */

/* if the given PC can be converted into a source key on a cache-valid
   page, this returns the source key, otherwise this returns
   TME_SPARC_RECODE_SRC_KEY_UNDEF: */
static tme_sparc_recode_src_key_t
_tme_sparc_recode_chain_src_key(const struct tme_sparc * const ic,
				tme_sparc_ireg_t address)
{
  tme_uint32_t tlb_hash;
  const struct tme_sparc_tlb *itlb;
  const tme_shared tme_uint8_t *src;
  tme_bus_context_t context;
  const struct tme_sparc_recode_cacheable *cacheable;
  tme_sparc_recode_src_key_t src_key;
  tme_uint32_t pstate;
  unsigned long page_index;

  /* hash the address into an instruction TLB entry: */
  tlb_hash
    = TME_SPARC_TLB_HASH(ic,
			 ic->tme_sparc_memory_context_default,
			 address);
  itlb = &ic->tme_sparc_tlbs[TME_SPARC_ITLB_ENTRY(ic, tlb_hash)];

  /* if this instruction TLB entry doesn't cover the address: */
  if (__tme_predict_false(address < (tme_sparc_ireg_t) itlb->tme_sparc_tlb_addr_first
			  || address > (tme_sparc_ireg_t) itlb->tme_sparc_tlb_addr_last)) {

    /* this PC has no source key: */
    return (TME_SPARC_RECODE_SRC_KEY_UNDEF);
  }

  /* if this instruction TLB entry doesn't allow fast reading: */
  src = itlb->tme_sparc_tlb_emulator_off_read;
  if (__tme_predict_false(src == TME_EMULATOR_OFF_UNDEF)) {

    /* this PC has no source key: */
    return (TME_SPARC_RECODE_SRC_KEY_UNDEF);
  }

  /* assume that this instruction TLB entry is valid and covers, and
     make a pointer to the instruction: */
  src += address;

  /* if this instruction TLB entry isn't valid: */
  /* NB: here, we check validity without busying the TLB entry first,
     to keep things simple (since this may be the current instruction
     TLB entry, and already busy).  this is OK because we don't really
     depend on the entry being valid - we only check for validity on
     the off chance that an invalid TLB entry appears to cover this
     address, context and ASI.

     the instruction TLB entry could become immediately invalid after
     this point, without affecting correctness, since we're only using
     the translated address to check the cache-valid bit and maybe do
     a lookup in the source key hash.  if this instruction TLB entry
     isn't already busy, before doing anything with the result of this
     lookup, the instruction TLB entry will be busied and checked for
     validity as normal: */
  if (tme_bus_tlb_is_invalid(&itlb->tme_sparc_tlb_bus_tlb)) {

    /* this PC has no source key: */
    return (TME_SPARC_RECODE_SRC_KEY_UNDEF);
  }

  /* if this instruction TLB entry doesn't cover this context or
     ASI: */
  context = itlb->tme_sparc_tlb_context;
  if (__tme_predict_false((context <= ic->tme_sparc_memory_context_max
			   && context != ic->tme_sparc_memory_context_default)
			  || !TME_SPARC_TLB_ASI_MASK_OK(itlb, ic->tme_sparc_asi_mask_insn))) {

    /* this PC has no source key: */
    return (TME_SPARC_RECODE_SRC_KEY_UNDEF);
  }

  /* search for a cacheable that contains this source address: */
  cacheable = ic->tme_sparc_recode_cacheable_first;
  for (;;) {

    /* assume that this cacheable contains this source address, and get the
       source address key relative to the start of the cacheable: */
    src_key = src - cacheable->tme_sparc_recode_cacheable_contents;

    /* if this cacheable doesn't contain this source address: */
    if (__tme_predict_false(src_key
			    >= cacheable->tme_sparc_recode_cacheable_size)) {

      /* if we have run out of cacheables: */
      if (__tme_predict_false(cacheable != &ic->tme_sparc_recode_cacheables[0])) {

	/* this PC has no source key: */
	return (TME_SPARC_RECODE_SRC_KEY_UNDEF);
      }

      /* try the next cacheable: */
      cacheable--;
      continue;
    }

    break;
  }

  /* make the absolute source address key by adding in the first
     source address key in this cacheable: */
  src_key += cacheable->tme_sparc_recode_cacheable_src_key_first;
  assert ((src_key % sizeof(tme_uint32_t)) == 0);

  /* make the index for the page containing this source address: */
  page_index = src_key >> ic->tme_sparc_tlb_page_size_log2;

  /* if the valid bit for this page is no longer set: */
  /* NB: this isn't needed by _tme_sparc_recode_chain_fixup(), because
     each instructions thunk checks its own page valid bit, but this
     is needed by tme_sparc_recode_insn_assist_redispatch(): */
  if (__tme_predict_false(((cacheable->tme_sparc_recode_cacheable_valids[page_index / 8])
			   & TME_BIT(page_index % 8)) == 0)) {

    /* this PC has no source key: */
    return (TME_SPARC_RECODE_SRC_KEY_UNDEF);
  }

  /* if this is a v9 CPU: */
  if (TME_SPARC_VERSION(ic) >= 9) {

    /* add PSTATE.AM and PSTATE.CLE to the source address key: */
    pstate = ic->tme_sparc64_ireg_pstate;
    if (pstate & TME_SPARC64_PSTATE_AM) {
      src_key += TME_SPARC64_RECODE_SRC_KEY_FLAG_AM;
    }
    if (pstate & TME_SPARC64_PSTATE_CLE) {
      src_key += TME_SPARC64_RECODE_SRC_KEY_FLAG_CLE;
    }
  }

  /* the source address key must be defined: */
  assert (src_key != TME_SPARC_RECODE_SRC_KEY_UNDEF);
  return (src_key);
}

/* this looks up an instructions thunk for a PC: */
static tme_recode_thunk_off_t
_tme_sparc_recode_chain_insns_thunk(const struct tme_sparc * const ic,
				    tme_sparc_ireg_t address)
{
  tme_sparc_recode_src_key_t src_key;
  unsigned long src_hash_i;
  signed int src_hash_probe;
  tme_sparc_recode_src_key_t src_key_other;
  tme_recode_thunk_off_t insns_thunk;

  /* if this PC has no source key: */
  src_key = _tme_sparc_recode_chain_src_key(ic, address);
  if (__tme_predict_false(src_key == TME_SPARC_RECODE_SRC_KEY_UNDEF)) {

    /* the lookup fails: */
    return (0);
  }

  /* hash the source address key: */
  src_hash_i
    = ((((src_key
	  / sizeof(tme_uint32_t))
	 % TME_SPARC_RECODE_SRC_HASH_MODULUS)
	* TME_SPARC_RECODE_SRC_HASH_SIZE_SET)
       + (TME_SPARC_RECODE_SRC_HASH_SIZE_SET
	  - 1));

  /* search for the source address key in the hash: */
  src_hash_probe = TME_SPARC_RECODE_SRC_HASH_SIZE_PROBE;
  for (;;) {

    /* get the source address key at this position: */
    src_key_other
      = (ic->tme_sparc_recode_src_hash
	 [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
	 .tme_sparc_recode_src_hash_keys
	 [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]);

    /* if we found the source address key, stop now: */
    if (src_key_other == src_key) {
      break;
    }

    /* if this position in the hash is free: */
    if (src_key_other == TME_SPARC_RECODE_SRC_KEY_UNDEF) {

      /* the lookup fails: */
      return (0);
    }

    /* if we have searched enough: */
    if (__tme_predict_false(--src_hash_probe < 0)) {

      /* XXX FIXME - add a counter here? */

      /* the lookup fails: */
      return (0);
    }

    /* move to the next position to search: */
    if (__tme_predict_false(((signed long) --src_hash_i) < 0)) {
      src_hash_i
	= ((TME_SPARC_RECODE_SRC_HASH_MODULUS
	    * TME_SPARC_RECODE_SRC_HASH_SIZE_SET)
	   - 1);
    }
  }

  /* assume that this source address has been recoded, and get the
     instructions thunk: */
  insns_thunk
    = (ic->tme_sparc_recode_src_hash
       [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
       .tme_sparc_recode_src_hash_values
       [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]);

  /* if the source address has not been recoded yet: */
  if (__tme_predict_false(insns_thunk & TME_BIT(0))) {

    /* the lookup fails: */
    return (0);
  }

  /* the lookup succeeds: */
  return (insns_thunk);
}

/* this tries to fix up a chain: */
static tme_recode_thunk_off_t
_tme_sparc_recode_chain_fixup(struct tme_ic * const _ic,
			      tme_recode_thunk_off_t const chain_fixup,
			      tme_uint32_t const chain_info)
{
  struct tme_sparc *ic;
  tme_recode_thunk_off_t insns_thunk_next;
  tme_recode_thunk_off_t insns_thunk_return;

  /* we can't chain if we're verifying: */
  if (_tme_sparc_recode_verify_on) {

    /* the fixup fails: */
    return (0);
  }

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  /* if the next PC has not been recoded: */
  insns_thunk_next
    = _tme_sparc_recode_chain_insns_thunk(ic,
					  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT));
  if (insns_thunk_next == 0) {

    /* the fixup fails: */
    return (0);
  }

  /* if this chain is a call: */
  if (chain_info & TME_RECODE_CHAIN_INFO_CALL) {

    /* if the standard return address has not been recoded: */
    /* NB: PC currently points to the branch delay slot after the
       call.  the standard return address is the next instruction
       after that: */
    insns_thunk_return
      = _tme_sparc_recode_chain_insns_thunk(ic,
					    (ic->tme_sparc_ireg(TME_SPARC_IREG_PC)
					     + sizeof(tme_uint32_t)));
    if (insns_thunk_return == 0) {

      /* the fixup fails: */
      return (0);
    }
  }

  /* otherwise, this chain is not a call: */
  else {

    /* silence uninitialized variable warnings: */
    insns_thunk_return = 0;
  }

  /* fix up the chain: */
  return (tme_recode_chain_fixup(ic->tme_sparc_recode_ic,
				 chain_fixup,
				 chain_info,
				 insns_thunk_next,
				 insns_thunk_return));
}

/* this updates a recode ITLB entry: */
void
tme_sparc_recode_chain_tlb_update(struct tme_sparc *ic,
				  const struct tme_sparc_ls *ls)
{
  struct tme_sparc_tlb *itlb;
  struct _TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,/**/) *recode_itlb;
  tme_uint32_t tlb_flags_chain;
  tme_uint32_t itlb_page_size;
  tme_sparc_ireg_t recode_itlb_page;

  /* get the sparc ITLB entry: */
  itlb = ls->tme_sparc_ls_tlb;

  /* get the recode ITLB entry: */
  recode_itlb = &ic->_TME_SPARC_RECODE_SIZE(tme_sparc_recode_tlb,s)[ls->tme_sparc_ls_tlb_i];

  /* assume that this recode ITLB entry will assist all fetches
     (this mask includes at least
     TME_SPARC_RECODE_TLB_FLAG_CHAIN_USER(ic), and
     TME_SPARC_RECODE_TLB_FLAG_CHAIN_PRIV(ic): */
  tlb_flags_chain = TME_RECODE_TLB_FLAGS_MASK(ic->tme_sparc_recode_ic);

  /* get our ITLB page size: */
  itlb_page_size = (1 << ic->tme_sparc_tlb_page_size_log2);

  /* update the recode ITLB entry page: */
  recode_itlb_page = ls->_TME_SPARC_RECODE_SIZE(tme_sparc_ls_address,/**/) & (0 - (tme_sparc_ireg_t) itlb_page_size);
  recode_itlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_page) = recode_itlb_page;

  /* if the sparc ITLB entry covers an entire page for one or more
     normal ASIs and allows fast reading: */
  /* NB: we don't check if verification is on, like
     tme_sparc_recode_ls_tlb_update() does, because instruction
     fetches aren't verified: */
  if ((((tme_sparc_ireg_t) itlb->tme_sparc_tlb_addr_first)
       <= recode_itlb_page)
      && ((recode_itlb_page | (itlb_page_size - 1))
	  <= ((tme_sparc_ireg_t) itlb->tme_sparc_tlb_addr_last))
      && (itlb->tme_sparc_tlb_asi_mask & TME_SPARC_ASI_MASK_FLAG_SPECIAL) == 0
      && itlb->tme_sparc_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF) {

    /* limit the sparc ITLB entry to covering only the single recode
       page, like the recode ITLB entry does.  this only affects
       large-page sparc ITLB entries.

       if we didn't do this, and left a large-page sparc ITLB entry
       covering more than the one recode page, when a chain far would
       take a recode ITLB miss on an address actually covered by the
       large-page corresponding sparc ITLB entry (i.e., miss for an
       address that aliases to the same sparc/recode ITLB entries for
       the address we're handling now, where both are in the large
       page), _TME_SPARC_EXECUTE_NAME() would refuse to do an ITLB
       refill, and would just try to run the recode instructions thunk
       again, which would miss again, leading to an endless loop.

       limiting the sparc ITLB entry like this somewhat hurts the
       performance of the regular _TME_SPARC_EXECUTE_NAME() executor,
       which normally would stick with a large-page ITLB entry for as
       long as possible (even for PCs that wouldn't hash to that
       entry), but this will only be felt when not running recode
       instruction thunks, which hopefully won't be often: */
    itlb->tme_sparc_tlb_addr_first = recode_itlb_page;
    itlb->tme_sparc_tlb_addr_last = (recode_itlb_page | (itlb_page_size - 1));

    /* update the recode ITLB entry page memory pointer: */
    recode_itlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_memory)
      = (itlb->tme_sparc_tlb_emulator_off_read
	 + recode_itlb_page);

    /* if this sparc ITLB entry allows user fetches: */
    if (TME_SPARC_TLB_ASI_MASK_OK(itlb, 
				  (TME_SPARC_VERSION(ic) >= 9
				   ? TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
				   : TME_SPARC32_ASI_MASK_UI))) {

      /* this recode ITLB entry won't need an assist for user
	 fetches: */
      tlb_flags_chain -= TME_SPARC_RECODE_TLB_FLAG_CHAIN_USER(ic);
    }

    /* if this sparc ITLB entry allows privileged/supervisor
       fetches: */
    if (TME_SPARC_TLB_ASI_MASK_OK(itlb, 
				  (TME_SPARC_VERSION(ic) >= 9
				   ? TME_SPARC64_ASI_MASK_REQUIRED_UNRESTRICTED(!TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER)
				   : TME_SPARC32_ASI_MASK_SI))) {

      /* this recode ITLB entry won't need an assist for
	 privileged/supervisor fetches: */
      tlb_flags_chain -= TME_SPARC_RECODE_TLB_FLAG_CHAIN_PRIV(ic);
    }

    /* update the recode ITLB entry context: */
    recode_itlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_context)
      = itlb->tme_sparc_tlb_context;

    /* if this sparc ITLB entry matches any context: */
    if (itlb->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max) {

      /* this recode ITLB entry doesn't need an assist for fetches
	 whose context mismatches the recode ITLB entry: */
      tlb_flags_chain -= TME_RECODE_TLB_FLAG_CONTEXT_MISMATCH(ic);
    }
  }

  /* update the recode ITLB entry flags: */
  recode_itlb->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_flags) = tlb_flags_chain;
}

/* this initializes for chaining: */
static void
_tme_sparc_recode_chain_init(struct tme_sparc *ic)
{
  struct tme_recode_chain chain;
  struct tme_recode_address_type *chain_address_type;

  /* set the offset of the pointer to the token for the current
     instruction TLB entry: */
  ic->tme_sparc_recode_ic->tme_recode_ic_itlb_current_token_offset
    = (((char *) &ic->_tme_sparc_itlb_current_token)
       - (char *) ic);

  /* set the chain fixup function: */
  ic->tme_sparc_recode_ic->tme_recode_ic_chain_fixup
    = _tme_sparc_recode_chain_fixup;

  /* set the offset of the chain counter: */
  ic->tme_sparc_recode_ic->tme_recode_ic_chain_counter_offset
    = (((char *) &ic->_tme_sparc_instruction_burst_remaining)
       - (char *) ic);

  /* set the size of the chain return address stack: */
  ic->tme_sparc_recode_ic->tme_recode_ic_chain_ras_size
    = TME_ARRAY_ELS(ic->_tme_sparc_recode_chain_ras);

  /* set the offset of the chain return address stack: */
  ic->tme_sparc_recode_ic->tme_recode_ic_chain_ras_offset
    = (((char *) &ic->_tme_sparc_recode_chain_ras)
       - (char *) ic);

  /* set the offset of the chain return address stack pointer: */
  ic->tme_sparc_recode_ic->tme_recode_ic_chain_ras_pointer_offset
    = (((char *) &ic->_tme_sparc_recode_chain_ras_pointer)
       - (char *) ic);

  /* make the chain thunk: */
  chain.tme_recode_chain_reg_guest = TME_SPARC_IREG_PC_NEXT;
  chain_address_type = &chain.tme_recode_chain_address_type;
  chain_address_type->tme_recode_address_type_context_ic_offset
    = (((char *) &ic->tme_sparc_memory_context_default) - ((char *) ic));
  chain_address_type->tme_recode_address_type_context_size = TME_RECODE_SIZE_16;
  chain_address_type->tme_recode_address_type_size = TME_SPARC_RECODE_SIZE(ic);
  chain_address_type->tme_recode_address_type_signed = FALSE;
  chain_address_type->tme_recode_address_type_align_min = sizeof(tme_uint32_t);
  /* NB: TME_RECODE_RW_FLAG_CONTEXT_MISMATCH() will get set
     automatically as needed by the chain thunk; there's no need to
     set it here: */
  chain_address_type->tme_recode_address_type_tlb_flags
    = (TME_SPARC_RECODE_TLB_FLAG_CHAIN_USER(ic)
       | TME_SPARC_RECODE_TLB_FLAG_CHAIN_PRIV(ic));
  chain_address_type->tme_recode_address_type_tlb_flags_ic_offset
    = (((char *) &ic->tme_sparc_recode_chain_tlb_flags) - ((char *) ic));
  chain_address_type->tme_recode_address_type_mask_tlb_index
    = ((_TME_SPARC_ITLB_HASH_SIZE - 1)
       * ic->tme_sparc_recode_ic->tme_recode_ic_tlb_page_size);
  chain_address_type->tme_recode_address_type_tlb0_ic_offset
    = (((char *) &ic->_TME_SPARC_RECODE_SIZE(tme_sparc_recode_tlb,s)[TME_SPARC_ITLB_ENTRY(ic, 0)])
       - (char *) ic);
  ic->tme_sparc_recode_insns_group.tme_recode_insns_group_chain_thunk
    = tme_recode_chain_thunk(ic->tme_sparc_recode_ic, &chain);

  /* clear the return address stack: */
  tme_recode_chain_ras_clear(ic->tme_sparc_recode_ic,
			     &ic->tme_sparc_ic);
}
