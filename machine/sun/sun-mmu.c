/* $Id: sun-mmu.c,v 1.12 2010/02/15 21:55:55 fredette Exp $ */

/* machine/sun/sun-mmu.c - classic Sun MMU emulation implementation: */

/*
 * Copyright (c) 2003 Matt Fredette
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
_TME_RCSID("$Id: sun-mmu.c,v 1.12 2010/02/15 21:55:55 fredette Exp $");

/* includes: */
#include <tme/machine/sun.h>

/* macros: */
#define TME_SUN_MMU_PMEG_TLBS	(16)
#define TME_SUN_MMU_CONTEXT_TLBS	(8)

/* structures: */

/* an allocated TLB set in a classic two-level Sun MMU: */
struct tme_sun_mmu_tlb_set {

  /* the next allocated TLB set: */
  struct tme_sun_mmu_tlb_set *tme_sun_mmu_tlb_set_next;

  /* the TLB set information: */
  struct tme_bus_tlb_set_info tme_sun_mmu_tlb_set_info;
};

/* one PMEG in a classic two-level Sun MMU: */
struct tme_sun_mmu_pmeg {

  /* the current list of TLBs using a page table entry in this PMEG, and
     the head within that list: */
  struct tme_token *tme_sun_mmu_pmeg_tlb_tokens[TME_SUN_MMU_PMEG_TLBS];
  unsigned int tme_sun_mmu_pmeg_tlbs_head;
};

/* the private structure for a classic two-level Sun MMU: */
struct tme_sun_mmu {

  /* the information provided by the user: */
  struct tme_sun_mmu_info tme_sun_mmu_info;
#define tme_sun_mmu_element tme_sun_mmu_info.tme_sun_mmu_info_element
#define tme_sun_mmu_address_bits tme_sun_mmu_info.tme_sun_mmu_info_address_bits
#define tme_sun_mmu_pgoffset_bits tme_sun_mmu_info.tme_sun_mmu_info_pgoffset_bits
#define tme_sun_mmu_pteindex_bits tme_sun_mmu_info.tme_sun_mmu_info_pteindex_bits
#define tme_sun_mmu_contexts tme_sun_mmu_info.tme_sun_mmu_info_contexts
#define tme_sun_mmu_pmegs_count tme_sun_mmu_info.tme_sun_mmu_info_pmegs
#define _tme_sun_mmu_tlb_fill_private tme_sun_mmu_info.tme_sun_mmu_info_tlb_fill_private
#define _tme_sun_mmu_tlb_fill tme_sun_mmu_info.tme_sun_mmu_info_tlb_fill

  /* if nonzero, this address space has a hole, and this has only the
     last true address bit set: */
  tme_uint32_t tme_sun_mmu_address_hole_bit;

  /* a PTE for addresses in the hole.  this is always all-bits-zero: */
  struct tme_sun_mmu_pte tme_sun_mmu_address_hole_pte;

  /* the number of bits in a segment map index: */
  tme_uint8_t tme_sun_mmu_segment_bits;

  /* the segment map: */
  unsigned short *tme_sun_mmu_segment_map;

  /* the PMEGs: */
  struct tme_sun_mmu_pmeg *tme_sun_mmu_pmegs;

  /* the PTEs: */
  struct tme_sun_mmu_pte *tme_sun_mmu_ptes;

  /* the allocated TLB sets: */
  struct tme_sun_mmu_tlb_set *tme_sun_mmu_tlb_sets;

  /* the current list of TLBs that must be invalidated when the
     context changes: */
  struct tme_token *tme_sun_mmu_context_tlb_tokens[TME_SUN_MMU_CONTEXT_TLBS];
  unsigned int tme_sun_mmu_context_tlbs_head;
};

/* this creates a classic two-level Sun MMU: */
void *
tme_sun_mmu_new(struct tme_sun_mmu_info *info)
{
  struct tme_sun_mmu *mmu;
  unsigned int segmap_count;
  unsigned int segmap_i;

  /* allocate the new private structure: */
  mmu = tme_new0(struct tme_sun_mmu, 1);

  /* copy the user-provided information: */
  mmu->tme_sun_mmu_info = *info;

  /* if there is an address hole: */
  if (mmu->tme_sun_mmu_info.tme_sun_mmu_info_topindex_bits < 0) {

    /* there must be 32 address bits: */
    assert (mmu->tme_sun_mmu_address_bits == 32);

    /* adjust the number of address bits for the hole: */
    mmu->tme_sun_mmu_address_bits += (mmu->tme_sun_mmu_info.tme_sun_mmu_info_topindex_bits + 1);

    /* make the hole address bit: */
    mmu->tme_sun_mmu_address_hole_bit = TME_BIT(mmu->tme_sun_mmu_address_bits - 1);

    /* zero the number of top index bits: */
    mmu->tme_sun_mmu_info.tme_sun_mmu_info_topindex_bits = 0;
  }

  /* allocate the segment map and initialize it to all invalid: */
  mmu->tme_sun_mmu_segment_bits = (mmu->tme_sun_mmu_address_bits
				   - (mmu->tme_sun_mmu_pteindex_bits
				      + mmu->tme_sun_mmu_pgoffset_bits));
  segmap_count = (mmu->tme_sun_mmu_contexts
		  * (1 << mmu->tme_sun_mmu_segment_bits));
  mmu->tme_sun_mmu_segment_map = tme_new(unsigned short, segmap_count);
  for (segmap_i = 0; segmap_i < segmap_count; segmap_i++) {
    mmu->tme_sun_mmu_segment_map[segmap_i] = mmu->tme_sun_mmu_pmegs_count - 1;
  }

  /* allocate the PMEGs: */
  mmu->tme_sun_mmu_pmegs = tme_new0(struct tme_sun_mmu_pmeg, mmu->tme_sun_mmu_pmegs_count);

  /* allocate the PTEs: */
  mmu->tme_sun_mmu_ptes = 
    tme_new0(struct tme_sun_mmu_pte, 
	     mmu->tme_sun_mmu_pmegs_count
	     * (1 << mmu->tme_sun_mmu_pteindex_bits));

  /* done: */
  return (mmu);
}

/* given a context and an address, returns the segment map index and
   PTE: */
static unsigned short
_tme_sun_mmu_lookup(struct tme_sun_mmu *mmu, tme_uint8_t context, tme_uint32_t address,
		    struct tme_sun_mmu_pte **_pte)
{
  unsigned short pteindex;
  unsigned short segment;
  unsigned short segment_map_index;
  unsigned short pmeg;

  /* if there is an address hole, and this address is in it: */
  if (__tme_predict_false(((address
			    + (address & mmu->tme_sun_mmu_address_hole_bit))
			   & (((tme_uint32_t) 0)
			      - mmu->tme_sun_mmu_address_hole_bit)) != 0)) {

    /* return the hole PTE, and zero for the segment map index: */
    *_pte = &mmu->tme_sun_mmu_address_hole_pte;
    return (0);
  }

  /* lose the page offset bits: */
  address >>= mmu->tme_sun_mmu_pgoffset_bits;
    
  /* get the PTE index: */
  pteindex = (address
	      & (TME_BIT(mmu->tme_sun_mmu_pteindex_bits) - 1));
  address >>= mmu->tme_sun_mmu_pteindex_bits;

  /* get the segment number: */
  segment = (address
	     & (TME_BIT(mmu->tme_sun_mmu_segment_bits) - 1));

  /* get the segment map index: */
  segment_map_index = ((context << mmu->tme_sun_mmu_segment_bits)
		       | segment);

  /* get the PMEG: */
  pmeg = mmu->tme_sun_mmu_segment_map[segment_map_index];

  /* return the segment map index and the PTE: */
  *_pte = (mmu->tme_sun_mmu_ptes + (pmeg << mmu->tme_sun_mmu_pteindex_bits) + pteindex);
  return (segment_map_index);
}

/* this invalidates all TLB entries that may be affected by changes to
   a PMEG: */
static void
_tme_sun_mmu_pmeg_invalidate(struct tme_sun_mmu *mmu, unsigned short segment_map_index)
{
  struct tme_sun_mmu_pmeg *pmeg;
  int tlb_i;
  struct tme_token *token;

  /* get the PMEG: */
  pmeg = mmu->tme_sun_mmu_pmegs + mmu->tme_sun_mmu_segment_map[segment_map_index];

  /* invalidate all of the TLBs: */
  for (tlb_i = 0; tlb_i < TME_SUN_MMU_PMEG_TLBS; tlb_i++) {
    token = pmeg->tme_sun_mmu_pmeg_tlb_tokens[tlb_i];
    pmeg->tme_sun_mmu_pmeg_tlb_tokens[tlb_i] = NULL;
    if (token != NULL) {
      tme_token_invalidate(token);
    }
  }
}

/* this gets a PTE: */
int
tme_sun_mmu_pte_get(void *_mmu, tme_uint8_t context, tme_uint32_t address, 
		    struct tme_sun_mmu_pte *_pte)
{
  struct tme_sun_mmu *mmu;
  unsigned short segment_map_index;
  struct tme_sun_mmu_pte *pte;

  /* lookup this address: */
  mmu = (struct tme_sun_mmu *) _mmu;
  segment_map_index = _tme_sun_mmu_lookup(mmu, context, address, &pte);

  /* otherwise, copy the PTE: */
  *_pte = *pte;
  return (TME_OK);
}

/* this sets a PTE: */
int
tme_sun_mmu_pte_set(void *_mmu, tme_uint8_t context, tme_uint32_t address, 
		    struct tme_sun_mmu_pte *_pte)
{
  struct tme_sun_mmu *mmu;
  unsigned short segment_map_index;
  struct tme_sun_mmu_pte *pte;

  /* lookup this address: */
  mmu = (struct tme_sun_mmu *) _mmu;
  segment_map_index = _tme_sun_mmu_lookup(mmu, context, address, &pte);
  if (__tme_predict_false(pte == &mmu->tme_sun_mmu_address_hole_pte)) {
    return (TME_OK);
  }

  /* invalidate all TLB entries that are affected by changes to this PMEG: */
  _tme_sun_mmu_pmeg_invalidate(mmu, segment_map_index);

  /* otherwise, copy the PTE: */
  *pte = *_pte;
  return (TME_OK);
}

/* this gets a segment map entry: */
unsigned short
tme_sun_mmu_segmap_get(void *_mmu, tme_uint8_t context, tme_uint32_t address)
{
  struct tme_sun_mmu *mmu;
  struct tme_sun_mmu_pte *pte;
  unsigned short segment_map_index, pmeg;

  /* lookup this address: */
  mmu = (struct tme_sun_mmu *) _mmu;
  segment_map_index = _tme_sun_mmu_lookup(mmu, context, address, &pte);
  if (__tme_predict_false(pte == &mmu->tme_sun_mmu_address_hole_pte)) {
    return (mmu->tme_sun_mmu_pmegs_count - 1);
  }
  pmeg = mmu->tme_sun_mmu_segment_map[segment_map_index];
  tme_log(&mmu->tme_sun_mmu_element->tme_element_log_handle, 1000, TME_OK,
	  (&mmu->tme_sun_mmu_element->tme_element_log_handle,
	   "segmap_get: SEGMAP[%d:0x%08x] -> 0x%04x", 
	   context,
	   address,
	   pmeg));
  return (pmeg);
}

/* this sets a segment map entry: */
void
tme_sun_mmu_segmap_set(void *_mmu, tme_uint8_t context, tme_uint32_t address, unsigned short pmeg)
{
  struct tme_sun_mmu *mmu;
  unsigned short segment_map_index;
  struct tme_sun_mmu_pte *pte;

  /* lookup this address: */
  mmu = (struct tme_sun_mmu *) _mmu;
  segment_map_index = _tme_sun_mmu_lookup(mmu, context, address, &pte);
  if (__tme_predict_false(pte == &mmu->tme_sun_mmu_address_hole_pte)) {
    return;
  }

  /* invalidate all TLB entries that are affected by changes to this
     PMEG - losing a spot in the segment map counts as such a change: */
  _tme_sun_mmu_pmeg_invalidate(mmu, segment_map_index);

  /* set the new segment: */
  mmu->tme_sun_mmu_segment_map[segment_map_index] = pmeg;
  tme_log(&mmu->tme_sun_mmu_element->tme_element_log_handle, 1000, TME_OK,
	  (&mmu->tme_sun_mmu_element->tme_element_log_handle,
	   "segmap_set: SEGMAP[%d:0x%08x] <- 0x%04x", 
	   context,
	   address,
	   pmeg));
}

/* this fills a TLB entry: */
unsigned short
tme_sun_mmu_tlb_fill(void *_mmu, struct tme_bus_tlb *tlb, 
		     tme_uint8_t context, tme_uint32_t address, unsigned short access)
{
  struct tme_sun_mmu *mmu;
  unsigned short segment_map_index;
  struct tme_sun_mmu_pte *pte;
  tme_bus_addr32_t addr_first, addr_last;
  unsigned short protection, protection_other, tlb_valid_for;
  tme_uint32_t physical_address;
  struct tme_sun_mmu_pmeg *pmeg;
  struct tme_bus_tlb tlb_virtual;
  struct tme_token *token_old;
  int tlb_i;

  /* the access must be a read or write by the system or user: */
  assert(access != 0
	 && (access == TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RO)
	     || access == TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RW)
	     || access == TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RO)
	     || access == TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RW)));

  /* lookup this address: */
  mmu = (struct tme_sun_mmu *) _mmu;
  segment_map_index = _tme_sun_mmu_lookup(mmu, context, address, &pte);
  addr_first = (address & ~(TME_BIT(mmu->tme_sun_mmu_pgoffset_bits) - 1));
  addr_last = (address | (TME_BIT(mmu->tme_sun_mmu_pgoffset_bits) - 1));

  /* remember this TLB entry in the PMEG: */
  if (__tme_predict_true(pte != &mmu->tme_sun_mmu_address_hole_pte)) {
    pmeg = mmu->tme_sun_mmu_pmegs + mmu->tme_sun_mmu_segment_map[segment_map_index];
    tlb_i = pmeg->tme_sun_mmu_pmeg_tlbs_head;
    token_old = pmeg->tme_sun_mmu_pmeg_tlb_tokens[tlb_i];
    if (token_old != NULL
	&& token_old != tlb->tme_bus_tlb_token) {
      tme_token_invalidate(token_old);
    }
    pmeg->tme_sun_mmu_pmeg_tlb_tokens[tlb_i]
      = tlb->tme_bus_tlb_token;
    pmeg->tme_sun_mmu_pmeg_tlbs_head = (tlb_i + 1) & (TME_SUN_MMU_PMEG_TLBS - 1);
  }

  /* if this page is invalid, return the page-invalid cycle handler,
     which is valid for reading and writing for the user and system: */
  if (!(pte->tme_sun_mmu_pte_flags & TME_SUN_MMU_PTE_VALID)) {
    tme_bus_tlb_initialize(tlb);
    tlb->tme_bus_tlb_addr_first = addr_first;
    tlb->tme_bus_tlb_addr_last = addr_last;
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
    tlb->tme_bus_tlb_cycle_private = mmu->tme_sun_mmu_info.tme_sun_mmu_info_invalid_private;
    tlb->tme_bus_tlb_cycle = mmu->tme_sun_mmu_info.tme_sun_mmu_info_invalid;
    return (TME_SUN_MMU_TLB_SYSTEM | TME_SUN_MMU_TLB_USER);
  }

  /* otherwise, this page is valid.  get the relevant part of the
     protection for this accessor (system or user), the part of the
     protection covering the other accessor (system or user), adjust
     "access" to be an unshifted TME_SUN_MMU_PTE_PROT_ value, and get
     the accessor (user or system) that this TLB entry will definitely
     be valid for: */
  protection = pte->tme_sun_mmu_pte_flags;
  if (access & TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_MASK)) {
    protection_other = protection / TME_SUN_MMU_PTE_PROT_USER(1);
    access /= TME_SUN_MMU_PTE_PROT_SYSTEM(1);
    protection /= TME_SUN_MMU_PTE_PROT_SYSTEM(1);
    tlb_valid_for = TME_SUN_MMU_TLB_SYSTEM;
  }
  else {
    protection_other = protection / TME_SUN_MMU_PTE_PROT_SYSTEM(1);
    access /= TME_SUN_MMU_PTE_PROT_USER(1);
    protection /= TME_SUN_MMU_PTE_PROT_USER(1);
    tlb_valid_for = TME_SUN_MMU_TLB_USER;
  }

  /* NB that the following code assumes a particular ordering of
     TME_SUN_MMU_PTE_PROT_ values.  specifically, it assumes that
     ABORT < ERROR < RO < RW: */

  /* if the part of the protection covering the other accessor (system
     or user) allows at least as much access as the relevant part of
     the protection, this TLB entry will be valid for that other
     successor as well.  we rely on particular definitions of the
     TME_SUN_MMU_TLB_ macros to make this fast: */
#if (3 - TME_SUN_MMU_TLB_SYSTEM) != TME_SUN_MMU_TLB_USER
#error "TME_SUN_MMU_TLB_USER and TME_SUN_MMU_TLB_SYSTEM are incompatible"
#endif
  protection &= TME_SUN_MMU_PTE_PROT_MASK;
  protection_other &= TME_SUN_MMU_PTE_PROT_MASK;
  if (protection_other >= protection) {
    tlb_valid_for |= (3 - tlb_valid_for);
  }

  /* if the access is protected, return the protection-error cycle
     handler: */
  if (protection < access) {
    if (protection == TME_SUN_MMU_PTE_PROT_ABORT) {
      abort();
    }
    tme_bus_tlb_initialize(tlb);
    tlb->tme_bus_tlb_addr_first = addr_first;
    tlb->tme_bus_tlb_addr_last = addr_last;
    tlb->tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_WRITE
				  | (protection == TME_SUN_MMU_PTE_PROT_ERROR
				     ? TME_BUS_CYCLE_READ
				     : 0));
    tlb->tme_bus_tlb_cycle_private = mmu->tme_sun_mmu_info.tme_sun_mmu_info_proterr_private;
    tlb->tme_bus_tlb_cycle = mmu->tme_sun_mmu_info.tme_sun_mmu_info_proterr;
    return (tlb_valid_for);
  }

  /* this access is OK.  fill the TLB with physical bus information.
     we pass in the virtual address as the initial physical address
     because sometimes the virtual part of the address can influence
     the physical address (as in the Sun-2 PROM mapping): */
  physical_address = address;
  (*mmu->_tme_sun_mmu_tlb_fill)
    (mmu->_tme_sun_mmu_tlb_fill_private, 
     tlb, 
     pte,
     &physical_address,
     ((access == TME_SUN_MMU_PTE_PROT_RW)
      ? TME_BUS_CYCLE_WRITE
      : TME_BUS_CYCLE_READ));

  /* create the mapping TLB entry, and update the PTE flags: */
  tlb_virtual.tme_bus_tlb_addr_first = addr_first;
  tlb_virtual.tme_bus_tlb_addr_last = addr_last;
  pte->tme_sun_mmu_pte_flags |= TME_SUN_MMU_PTE_REF;
  tlb_virtual.tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ;
  if (access == TME_SUN_MMU_PTE_PROT_RW) {
    pte->tme_sun_mmu_pte_flags |= TME_SUN_MMU_PTE_MOD;
  }
  if (protection == TME_SUN_MMU_PTE_PROT_RW
      && (pte->tme_sun_mmu_pte_flags & TME_SUN_MMU_PTE_MOD)) {
    tlb_virtual.tme_bus_tlb_cycles_ok |= TME_BUS_CYCLE_WRITE;
  }
  
  /* map the filled TLB entry: */
  tme_bus_tlb_map(tlb, physical_address, &tlb_virtual, address);

  /* return who this TLB entry is good for: */
  return (tlb_valid_for);
}

/* this invalidates all TLB entries in all TLB sets: */
void
tme_sun_mmu_tlbs_invalidate(void *_mmu)
{
  struct tme_sun_mmu *mmu;
  struct tme_sun_mmu_tlb_set *tlb_set;

  /* recover our MMU: */
  mmu = (struct tme_sun_mmu *) _mmu;

  /* invalidate all TLB entries in all sets: */
  for (tlb_set = mmu->tme_sun_mmu_tlb_sets;
       tlb_set != NULL;
       tlb_set = tlb_set->tme_sun_mmu_tlb_set_next) {
    tme_bus_tlb_set_invalidate(&tlb_set->tme_sun_mmu_tlb_set_info);
  }
}

/* this adds a TLB entry as dependent on the current context: */
void
tme_sun_mmu_context_add(void *_mmu,
			const struct tme_bus_tlb *tlb)
{
  struct tme_sun_mmu *mmu;
  tme_uint32_t address;
  tme_uint32_t segment_bits;
  tme_uint32_t segments_per_context;
  signed long segment_map_index;
  tme_uint32_t pmeg;
  unsigned long tlb_i;
  struct tme_token *token_old;

  /* recover our MMU: */
  mmu = (struct tme_sun_mmu *) _mmu;

  /* get the address used with the MMU: */
  /* NB: if there is an address hole, this address could be in it.  if
     it is, it doesn't matter if we decide that this TLB entry is
     valid in all contexts or not - this TLB entry never needs to be
     invalidated because of a context change: */
  address = tlb->tme_bus_tlb_addr_first;

  /* get the number of bits in a segment number: */
  segment_bits = mmu->tme_sun_mmu_segment_bits;

  /* get the number of segment map entries per context: */
  segments_per_context = 1 << segment_bits;

  /* get the segment map index for this address in the last
     context: */
  segment_map_index
    = (((address
	 >> (mmu->tme_sun_mmu_pgoffset_bits
	     + mmu->tme_sun_mmu_pteindex_bits))
	& (segments_per_context - 1))
       + ((mmu->tme_sun_mmu_contexts - 1)
	  << segment_bits));

  /* get the PMEG for this address in the last context: */
  pmeg = mmu->tme_sun_mmu_segment_map[segment_map_index];

  /* there must be at least two contexts: */
  assert (mmu->tme_sun_mmu_contexts >= 2);

  /* loop over the segment map indices for this address in all other
     contexts: */
  segment_map_index -= segments_per_context;
  do {

    /* if the PMEG for this address in this context is different: */
    if (__tme_predict_false(mmu->tme_sun_mmu_segment_map[segment_map_index] != pmeg)) {

      /* this address doesn't have the same mapping in all contexts,
	 so we must invalidate this TLB entry when the context
	 changes: */
      tlb_i = mmu->tme_sun_mmu_context_tlbs_head;
      token_old = mmu->tme_sun_mmu_context_tlb_tokens[tlb_i];
      if (token_old != NULL
	  && token_old != tlb->tme_bus_tlb_token) {
	tme_token_invalidate(token_old);
      }
      mmu->tme_sun_mmu_context_tlb_tokens[tlb_i] = tlb->tme_bus_tlb_token;
      mmu->tme_sun_mmu_context_tlbs_head = (tlb_i + 1) % TME_SUN_MMU_CONTEXT_TLBS;

      return;
    }

  } while ((segment_map_index -= segments_per_context) >= 0);

  /* this address has the same mapping in all contexts, so we don't
     need to invalidate this TLB entry when the context changes: */
}

/* this is called after a context switch, to invalidate TLB entries
   that were dependent on the previous context: */
void
tme_sun_mmu_context_switched(void *_mmu)
{
  struct tme_sun_mmu *mmu;
  signed long tlb_i;
  struct tme_token *token;

  /* recover our MMU: */
  mmu = (struct tme_sun_mmu *) _mmu;

  /* invalidate all of the TLBs that depended on the previous
     context: */
  tlb_i = TME_SUN_MMU_CONTEXT_TLBS - 1;
  do {
    token = mmu->tme_sun_mmu_context_tlb_tokens[tlb_i];
    mmu->tme_sun_mmu_context_tlb_tokens[tlb_i] = NULL;
    if (token != NULL) {
      tme_token_invalidate(token);
    }
  } while (--tlb_i >= 0);
}

/* this adds a new TLB set: */
int
tme_sun_mmu_tlb_set_add(void *_mmu,
			struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_sun_mmu *mmu;
  struct tme_sun_mmu_tlb_set *tlb_set;

  /* recover our mmu: */
  mmu = (struct tme_sun_mmu *) _mmu;

  /* remember this set: */
  tlb_set = tme_new0(struct tme_sun_mmu_tlb_set, 1);
  tlb_set->tme_sun_mmu_tlb_set_next = mmu->tme_sun_mmu_tlb_sets;
  tlb_set->tme_sun_mmu_tlb_set_info = *tlb_set_info;
  mmu->tme_sun_mmu_tlb_sets = tlb_set;
  
  return (TME_OK);
}

