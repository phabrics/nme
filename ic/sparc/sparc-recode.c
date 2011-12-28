/* $Id: sparc-recode.c,v 1.6 2010/06/05 18:42:35 fredette Exp $ */

/* ic/sparc/sparc-recode.c - recodes SPARC instructions: */

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

/* includes: */
#include "sparc-impl.h"

_TME_RCSID("$Id: sparc-recode.c,v 1.6 2010/06/05 18:42:35 fredette Exp $");

#if TME_HAVE_RECODE

/* macros: */

/* this gives the number of recode page bitmap bits needed to cover
   the given number of bytes.  this is always a multiple of eight: */
#define TME_SPARC_RECODE_PAGE_BITMAP_BITS(ic, size_bytes)	\
  ((((size_bytes) >> (ic)->tme_sparc_tlb_page_size_log2)	\
    + 7)							\
   & (0 - (unsigned long) 8))

/* this returns a read/write thunk index for an opcode: */
#define TME_SPARC_RECODE_RW_THUNK_INDEX(ic, address_size, endian, no_fault, opcode_0_3) \
  ((opcode_0_3)							\
   + ((TME_SPARC_VERSION(ic) >= 9)				\
      * ((16 * ((no_fault) != 0))				\
	 + (32 * ((endian) == TME_ENDIAN_LITTLE))		\
	 + (64 * (TME_SPARC_RECODE_SIZE(ic) - (address_size))))))

/* how many source address hash positions are probed: */
#define TME_SPARC_RECODE_SRC_HASH_SIZE_PROBE	(TME_SPARC_RECODE_SRC_HASH_SIZE_SET * 2)

/* the undefined source address hash key: */
#define TME_SPARC_RECODE_SRC_KEY_UNDEF		((tme_sparc_recode_src_key_t) (0 - (tme_sparc_recode_src_key_t) 1))

/* flags in a source address hash key: */
/* NB: an architecture can't have more than two flags, since those are
   the only bits available in a source address hash key (because PCs
   are 32-bit aligned): */
#define TME_SPARC64_RECODE_SRC_KEY_FLAG_AM	(1 << 0)
#define TME_SPARC64_RECODE_SRC_KEY_FLAG_CLE	(1 << 1)

/* the hit count threshold for recoding a source address: */
#define TME_SPARC_RECODE_HIT_COUNT_THRESHOLD	(512)

/* this returns the recode size for the architecture size: */
#define TME_SPARC_RECODE_SIZE(ic) (TME_RECODE_SIZE_32 + (TME_SPARC_VERSION(ic) >= 9))

/* fix certain things based on the architecture size: */
#undef TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) (8 + (_TME_SPARC_RECODE_SIZE(/**/,/**/) == 64))
#define tme_sparc_ireg_t _TME_SPARC_RECODE_SIZE(tme_uint,_t)
#define tme_sparc_ireg _TME_SPARC_RECODE_SIZE(tme_sparc_ireg_uint,/**/)
#define TME_SPARC_IREG_MSBIT (((tme_sparc_ireg_t) 1) << (sizeof(tme_sparc_ireg_t) * 8 - 1))

/* the current verify state: */
#ifdef _TME_SPARC_RECODE_VERIFY
static int _tme_sparc_recode_off;
static int _tme_sparc_recode_verify_on;
#else  /* !_TME_SPARC_RECODE_VERIFY */
#define _tme_sparc_recode_off (FALSE)
#define _tme_sparc_recode_verify_on (FALSE)
#endif /* !_TME_SPARC_RECODE_VERIFY */

/* make the sparc32 recode support: */
#define _TME_SPARC_RECODE_SIZE(x,y) _TME_CONCAT(_TME_CONCAT(x,32),y)
#include "sparc-rc-cc.c"
#include "sparc-rc-chain.c"
#include "sparc-rc-insns.c"
#include "sparc-rc-ls.c"
#undef _TME_SPARC_RECODE_SIZE

#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32

/* make the sparc64 recode support: */
#define _TME_SPARC_RECODE_SIZE(x,y) _TME_CONCAT(_TME_CONCAT(x,64),y)
#include "sparc-rc-cc.c"
#include "sparc-rc-chain.c"
#include "sparc-rc-insns.c"
#include "sparc-rc-ls.c"
#undef _TME_SPARC_RECODE_SIZE

#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */

/* unfix certain things: */
#undef  TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) _TME_SPARC_VERSION(ic)
#undef tme_sparc_ireg_t
#undef tme_sparc_ireg
#undef TME_SPARC_IREG_MSBIT

/* this invalidates pages in the source address hash: */
static void
_tme_sparc_recode_src_hash_invalidate(struct tme_sparc *ic,
				      tme_sparc_recode_src_key_t src_key_mask,
				      tme_sparc_recode_src_key_t src_key_match)
{
  signed long src_hash_i;
  tme_sparc_recode_src_key_t src_key;
  tme_recode_thunk_off_t insns_thunk;

  /* walk all of the positions in the source address hash: */
  src_hash_i
    = ((TME_SPARC_RECODE_SRC_HASH_MODULUS
	* TME_SPARC_RECODE_SRC_HASH_SIZE_SET)
       - 1);
  do {

    /* if the source address key in this position matches: */
    src_key
      = (ic->tme_sparc_recode_src_hash
	 [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
	 .tme_sparc_recode_src_hash_keys
	 [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]);
    if (((src_key ^ src_key_match) & src_key_mask) == 0) {

      /* invalidate this source address key: */
      (ic->tme_sparc_recode_src_hash
       [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
       .tme_sparc_recode_src_hash_keys
       [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT])
	= TME_SPARC_RECODE_SRC_KEY_UNDEF;

      /* if this source address key has an instructions thunk: */
      insns_thunk
	= (ic->tme_sparc_recode_src_hash
	   [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
	   .tme_sparc_recode_src_hash_values
	   [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]);
      if ((insns_thunk & TME_BIT(0)) == 0
	  && insns_thunk != 0) {

	/* invalidate the instructions thunk: */
	tme_recode_insns_thunk_invalidate(ic->tme_sparc_recode_ic,
					  insns_thunk);
      }

      /* reset the value for this position to a hit count of zero: */
      (ic->tme_sparc_recode_src_hash
       [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
       .tme_sparc_recode_src_hash_values
       [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT])
	= TME_BIT(0);
    }
  } while (--src_hash_i >= 0);
}

/* this invalidates our entire source hash and all instructions
   thunks: */
void
tme_sparc_recode_invalidate_all(struct tme_sparc *ic)
{

  /* invalidate our entire source hash: */
  _tme_sparc_recode_src_hash_invalidate(ic, 0, 0);

  /* invalidate all instructions thunks: */
  tme_recode_insns_thunk_invalidate_all(ic->tme_sparc_recode_ic);

  /* clear the return address stack: */
  tme_recode_chain_ras_clear(ic->tme_sparc_recode_ic,
			     &ic->tme_sparc_ic);
}

/* include the verify support: */
#ifdef _TME_SPARC_RECODE_VERIFY
#include "sparc-rc-verify.c"
#endif /* _TME_SPARC_RECODE_VERIFY */

/* this adds a new cacheable: */
static struct tme_sparc_recode_cacheable *
_tme_sparc_recode_cacheable_new(struct tme_sparc *ic,
				const struct tme_sparc_tlb *itlb,
				const tme_shared tme_uint32_t *src)
{
  unsigned int page_size_log2;
  tme_uint32_t page_size;
  tme_uint32_t page_byte_offset;
  struct tme_sparc_recode_cacheable *cacheable;
  const struct tme_bus_cacheable *bus_cacheable;
  unsigned long bitmap_bytes_old;
  unsigned long bitmap_bytes;

  /* get the page size: */
  page_size_log2 = ic->tme_sparc_tlb_page_size_log2;
  page_size = (((tme_uint32_t) 1) << page_size_log2);

  /* get the offset of the current PC into this page: */
  page_byte_offset
    = (
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
       TME_SPARC_VERSION(ic) >= 9
       ? ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)
       : 
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
       ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC));
  page_byte_offset %= page_size;

  /* if we have run out of cacheables, return failure: */
  cacheable = ic->tme_sparc_recode_cacheable_first;
  if (cacheable == &ic->tme_sparc_recode_cacheables[TME_SPARC_RECODE_CACHEABLES_MAX - 1]) {
    return (NULL);
  }
  cacheable += (ic->tme_sparc_recode_cacheables[0].tme_sparc_recode_cacheable_size != 0);

  /* if this instruction TLB entry isn't for cacheable memory, return
     failure: */
  bus_cacheable = itlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_cacheable;
  if (bus_cacheable == NULL) {
    return (NULL);
  }

  /* if the cacheable memory doesn't cover some whole number of
     aligned pages, return failure: */
  assert (bus_cacheable->tme_bus_cacheable_size > 0);
  if ((((((tme_uint8_t *) src)
	 - bus_cacheable->tme_bus_cacheable_contents)
	% page_size)
       != page_byte_offset)
      || ((bus_cacheable->tme_bus_cacheable_size
	   % page_size)
	  != 0)) {
    return (NULL);
  }

  /* initialize our cacheable: */
  /* NB: we round up the first source address key for this cacheable
     so that its page index is a multiple of eight, which will be used
     to index the least-significant bit in the first byte of the
     valids bitmap we allocate below: */
  cacheable->tme_sparc_recode_cacheable_contents = bus_cacheable->tme_bus_cacheable_contents;
  cacheable->tme_sparc_recode_cacheable_size = bus_cacheable->tme_bus_cacheable_size;
  cacheable->tme_sparc_recode_cacheable_src_key_first
    = (cacheable == &ic->tme_sparc_recode_cacheables[0]
       ? 0
       : ((cacheable - 1)->tme_sparc_recode_cacheable_src_key_first

	  + (TME_SPARC_RECODE_PAGE_BITMAP_BITS(ic, (cacheable - 1)->tme_sparc_recode_cacheable_size)
	     * page_size)));

  /* allocate the valids bitmap, and adjust the pointer to account
     for the first source key in this cacheable: */
  cacheable->tme_sparc_recode_cacheable_valids
    = ((*bus_cacheable->tme_bus_cacheable_valids_new)
       (bus_cacheable->tme_bus_cacheable_private,
	page_size_log2)
       - ((cacheable->tme_sparc_recode_cacheable_src_key_first
	   / page_size)
	  / 8));

  /* allocate or reallocate the active recode pages bitmap, and zero
     the new parts of the bitmap: */
  bitmap_bytes_old
    = (cacheable->tme_sparc_recode_cacheable_src_key_first
       / (page_size * 8));
  bitmap_bytes
    = (bitmap_bytes_old
       + (TME_SPARC_RECODE_PAGE_BITMAP_BITS(ic, cacheable->tme_sparc_recode_cacheable_size)
	  / 8));
  ic->tme_sparc_recode_cacheable_actives
    = (cacheable == &ic->tme_sparc_recode_cacheables[0]
       ? tme_new(tme_uint8_t, bitmap_bytes)
       : tme_renew(tme_uint8_t, ic->tme_sparc_recode_cacheable_actives, bitmap_bytes));
  memset((ic->tme_sparc_recode_cacheable_actives + bitmap_bytes_old),
	 0,
	 (bitmap_bytes - bitmap_bytes_old));

  /* finish allocating this cacheable: */
  ic->tme_sparc_recode_cacheable_first = cacheable;

  /* return success: */
  return (cacheable);
}

tme_recode_thunk_off_t
tme_sparc_recode(struct tme_sparc *ic,
		 const struct tme_sparc_tlb *itlb,
		 const tme_shared tme_uint32_t *src)
{
  
  const struct tme_sparc_recode_cacheable *cacheable;
  tme_sparc_recode_src_key_t src_key;
  tme_uint32_t pstate;
  unsigned long page_index;
  tme_uint8_t page_invalid;
  const tme_shared tme_uint8_t *cacheable_valid_byte;
  unsigned int cacheable_valid_mask;
  signed long src_hash_i;
  signed int src_hash_probe;
  tme_sparc_recode_src_key_t src_key_other;
  tme_recode_thunk_off_t insns_thunk;
  tme_recode_thunk_off_t hit_count;

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_calls);

  /* if recoding is off: */
  if (_tme_sparc_recode_off) {

    /* we can't recode this source address: */
    return (0);
  }

  /* if this instruction TLB entry doesn't extend to the end of the
     page: */
  /* XXX FIXME - this isn't right, since it will reject all itlb
     entries that don't end right at a recode page boundary, instead
     of only those that end before the end of the page containing the
     current PC.  maybe this check should move to sparc-rc-insns.c or
     sparc-execute.c, where we have the current PC? */
  if (__tme_predict_false((~itlb->tme_sparc_tlb_addr_last)
			  & ((1 << ic->tme_sparc_tlb_page_size_log2)
			     - sizeof(tme_uint32_t)))) {

    /* we can't recode this source address: */
    return (0);
  }

  /* assume that we will recode at this source address, and save it: */
  ic->tme_sparc_recode_insns_group.tme_recode_insns_group_src
    = (const tme_shared tme_uint8_t *) src;

  /* search for a cacheable that contains this source address: */
  cacheable = ic->tme_sparc_recode_cacheable_first;
  for (;;) {

    /* assume that this cacheable contains this source address, and get the
       source address key relative to the start of the cacheable: */
    src_key = ((tme_uint8_t *) src) - cacheable->tme_sparc_recode_cacheable_contents;

    /* if this cacheable contains this source address: */
    if (__tme_predict_true(src_key
			   < cacheable->tme_sparc_recode_cacheable_size)) {

      /* stop now: */
      break;
    }

    /* if we have not run out of cacheables: */
    if (__tme_predict_true(--cacheable != (&ic->tme_sparc_recode_cacheables[0] - 1))) {
      continue;
    }

    /* if this instruction TLB entry isn't for a cacheable, or if we
       can't add this cacheable: */
    cacheable = _tme_sparc_recode_cacheable_new(ic, itlb, src);
    if (cacheable == NULL) {

      /* we can't recode this source address: */
      return (0);
    }
  }

  /* make the absolute source address key by adding in the first
     source address key in this cacheable: */
  src_key += cacheable->tme_sparc_recode_cacheable_src_key_first;
  assert ((src_key % sizeof(tme_uint32_t)) == 0);

  /* if this is a v9 CPU: */
  if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
      && TME_SPARC_VERSION(ic) >= 9) {

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

  /* make the index for the page containing this source address: */
  page_index = src_key >> ic->tme_sparc_tlb_page_size_log2;

  /* assume that this recode page is still valid: */
  page_invalid = FALSE;

  /* get the cacheable valid byte and mask for this page: */
  cacheable_valid_byte = &cacheable->tme_sparc_recode_cacheable_valids[page_index / 8];
  cacheable_valid_mask = TME_BIT(page_index % 8);

  /* assume that we will recode at this source address, and save the
     cacheable valid byte and mask for this page: */
  ic->tme_sparc_recode_insns_group.tme_recode_insns_group_valid_byte = cacheable_valid_byte;
  ic->tme_sparc_recode_insns_group.tme_recode_insns_group_valid_mask = cacheable_valid_mask;

  /* if the valid bit for this page is no longer set: */
  if (__tme_predict_false(((*cacheable_valid_byte) & cacheable_valid_mask) == 0)) {

    /* this page is invalid: */
    page_invalid = TRUE;

    /* if this page was active: */
    if (__tme_predict_false(ic->tme_sparc_recode_cacheable_actives[page_index / 8]
			    & TME_BIT(page_index % 8))) {

      TME_SPARC_STAT(ic, tme_sparc_stats_recode_page_invalids);

      /* mark this page as inactive: */
      ic->tme_sparc_recode_cacheable_actives[page_index / 8] &= ~TME_BIT(page_index % 8);

      /* re-set the valid bit for the page containing this
	 source address: */
      /* NB: we cheat and pass our
	 page-index-base-greater-than-zero-relative valids bitmap
	 pointer instead of the original pointer returned by the
	 allocator.  this only works if the re-set function will work
	 given arbitrary valids bitmap pointers and page indices: */
      (*itlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_cacheable->tme_bus_cacheable_valids_set)
	(itlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_cacheable->tme_bus_cacheable_private,
	 cacheable->tme_sparc_recode_cacheable_valids,
	 page_index);

      /* invalidate this page in the source address hash: */
      _tme_sparc_recode_src_hash_invalidate(ic, 
					    (0
					     - ((tme_sparc_recode_src_key_t)
						(1 << ic->tme_sparc_tlb_page_size_log2))),
					    src_key);
    }
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

    TME_SPARC_STAT(ic, tme_sparc_stats_recode_source_hash_probes);

    /* get the source address key at this position: */
    src_key_other
      = (ic->tme_sparc_recode_src_hash
	 [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
	 .tme_sparc_recode_src_hash_keys
	 [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]);

    /* if we found the source address key, stop now: */
    if (__tme_predict_true(src_key_other == src_key)) {
      break;
    }

    /* if this position in the hash is free: */
    if (src_key_other == TME_SPARC_RECODE_SRC_KEY_UNDEF) {

      /* allocate this position in the hash: */
      (ic->tme_sparc_recode_src_hash
       [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
       .tme_sparc_recode_src_hash_keys
       [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT])
	= src_key;

      /* stop now: */
      break;
    }

    /* if we have searched enough: */
    if (__tme_predict_false(--src_hash_probe < 0)) {

      TME_SPARC_STAT(ic, tme_sparc_stats_recode_source_hash_misses);

      /* we won't recode this source address: */
      return (0);
    }

    /* move to the next position to search: */
    if (__tme_predict_false(--src_hash_i < 0)) {
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

    /* update the hit count for this source address: */
    hit_count = insns_thunk + 2;

    /* if the hit count for this source address is still less than the
       recode threshold: */
    if (__tme_predict_true(hit_count
			   < ((TME_SPARC_RECODE_HIT_COUNT_THRESHOLD * 2)
			      + 1))) {

      /* update the hit count for this source address: */
      (ic->tme_sparc_recode_src_hash
       [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
       .tme_sparc_recode_src_hash_values
       [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT])
	= hit_count;

      /* we won't recode this source address now: */
      return (0);
    }

    /* mark this page as active: */
    ic->tme_sparc_recode_cacheable_actives[page_index / 8] |= TME_BIT(page_index % 8);

    /* if this page is invalid: */
    if (page_invalid) {

      /* return now and interpret instructions normally - the next
	 time we encounter this page again, we will mark this
	 page as inactive, re-set the page's valid bit and
	 invalidate the page in the source address hash.

	 this somewhat limits the work we do for pages that are
	 both executed from and written to frequently.  we will only
	 recode an instructions thunk when its page was valid for the
	 entire time it took for its hit count to go from zero to the
	 threshold, and we will only invalidate an invalid page in the
	 source address hash when any hit count in the page reaches
	 the threshold: */
      return (0);
    }

    /* recode starting from this source address: */
    insns_thunk
      = (
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
	 TME_SPARC_VERSION(ic) >= 9
	 ? _tme_sparc64_recode_recode(ic, itlb)
	 :
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
	 _tme_sparc32_recode_recode(ic, itlb));

    /* if we couldn't recode at this source address: */
    if (__tme_predict_false(insns_thunk < 0)) {

      /* the only time we can't recode is when the host runs out of
	 memory for instructions thunks.  when this happens, the host
	 invalidates all previous instructions thunks, so we have to
	 invalidate our entire source hash: */
      tme_sparc_recode_invalidate_all(ic);
      return (0);
    }

    /* set the instructions thunk for this source address: */
    assert ((insns_thunk & TME_BIT(0)) == 0);
    (ic->tme_sparc_recode_src_hash
     [src_hash_i / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT]
     .tme_sparc_recode_src_hash_values
     [src_hash_i % TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT])
      = insns_thunk;

    /* we can't run the new instructions thunk without re-checking the
       valid bit for the page containing this source address
       first - if it wasn't set for the entire time we were recoding,
       the instructions thunk might not be consistent with the source
       memory.

       to keep things simple, we just return now and interpret
       instructions normally - the next time we get to this source
       address, we'll check the valid bit and either run the
       instructions thunk for the first time, or we'll do the
       invalidate: */
    return (0);
  }

  /* return the instructions thunk offset: */
  return (insns_thunk);
}

#ifdef TME_RECODE_DEBUG
#include <stdio.h>

void tme_sparc_recode_dump_ic _TME_P((const struct tme_sparc *));
void
tme_sparc_recode_dump_ic(const struct tme_sparc *ic)
{
  const char *conds_integer[] = {
    "n", "e", "le", "l", "leu", "cs", "neg", "vs",
    NULL
  };

  printf("sparc%u add flags thunk %p:\n",
	 (1 << TME_SPARC_RECODE_SIZE(ic)),
	 ic->tme_sparc_recode_flags_thunk_add);
  tme_recode_flags_thunk_dump(ic->tme_sparc_recode_ic,
			      ic->tme_sparc_recode_flags_thunk_add);
  printf("sparc%u sub flags thunk %p:\n",
	 (1 << TME_SPARC_RECODE_SIZE(ic)),
	 ic->tme_sparc_recode_flags_thunk_sub);
  tme_recode_flags_thunk_dump(ic->tme_sparc_recode_ic,
			      ic->tme_sparc_recode_flags_thunk_sub);
  printf("sparc%u logical flags thunk %p:\n",
	 (1 << TME_SPARC_RECODE_SIZE(ic)),
	 ic->tme_sparc_recode_flags_thunk_logical);
  tme_recode_flags_thunk_dump(ic->tme_sparc_recode_ic,
			      ic->tme_sparc_recode_flags_thunk_logical);

  if (TME_SPARC_VERSION(ic) >= 9) {
    printf("sparc%u rcc flags thunk %p:\n",
	   (1 << TME_SPARC_RECODE_SIZE(ic)),
	   ic->tme_sparc_recode_flags_thunk_rcc);
    tme_recode_flags_thunk_dump(ic->tme_sparc_recode_ic,
				ic->tme_sparc_recode_flags_thunk_rcc);
  }
  printf("sparc%u %%icc conds thunk %p:\n",
	 (1 << TME_SPARC_RECODE_SIZE(ic)),
	 ic->tme_sparc_recode_conds_thunk_icc);
  tme_recode_conds_thunk_dump(ic->tme_sparc_recode_ic, 
			      ic->tme_sparc_recode_conds_thunk_icc,
			      conds_integer);
  if (TME_SPARC_VERSION(ic) >= 9) {
    printf("sparc%u %%xcc conds thunk %p:\n",
	   (1 << TME_SPARC_RECODE_SIZE(ic)),
	   ic->tme_sparc_recode_conds_thunk_xcc);
    tme_recode_conds_thunk_dump(ic->tme_sparc_recode_ic, 
				ic->tme_sparc_recode_conds_thunk_xcc,
				conds_integer);
    printf("sparc%u %%rcc conds thunk %p:\n",
	   (1 << TME_SPARC_RECODE_SIZE(ic)),
	   ic->tme_sparc_recode_conds_thunk_rcc);
    tme_recode_conds_thunk_dump(ic->tme_sparc_recode_ic, 
				ic->tme_sparc_recode_conds_thunk_rcc,
				conds_integer);
  }
}

void tme_sparc_recode_dump_insns _TME_P((const struct tme_sparc *));
void
tme_sparc_recode_dump_insns(const struct tme_sparc *ic)
{
  const struct tme_recode_insn *insns;
  const struct tme_recode_insn *insns_end;
  unsigned int insn_i;
  const char *s;
  unsigned int insn_size;
  unsigned int operands_mask;
  unsigned int thunk_i;
  unsigned int operand_i;
  int delim;
  const char *regcs = "goli";
  const char *conds_integer[] = {
    "n", "e", "le", "l", "leu", "cs", "neg", "vs",
    "a", "ne", "g", "ge", "gu", "cc", "pos", "vc",
  };
  tme_uint32_t chain_info;

  printf("%%pc = ");
  tme_recode_uguest_dump(
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
			 TME_SPARC_VERSION(ic) >= 9
			 ? ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)
			 :
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
			 ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC));
  printf("\n");

  insns = ic->tme_sparc_recode_insns_group.tme_recode_insns_group_insns;
  insns_end = ic->tme_sparc_recode_insns_group.tme_recode_insns_group_insns_end;
  for (insn_i = 0; insns < insns_end; insns++, insn_i++) {

    printf("%2u: ", insn_i);

    s = tme_recode_opcode_dump(insns->tme_recode_insn_opcode);
    insn_size = insns->tme_recode_insn_size;
    switch (insns->tme_recode_insn_opcode) {
    case TME_RECODE_OPCODE_EXTZ:
    case TME_RECODE_OPCODE_EXTS:
      printf("ext%u%c",
	     (1 << insns->tme_recode_insn_operand_src[1]),
	     (insns->tme_recode_insn_opcode == TME_RECODE_OPCODE_EXTZ ? 'z' : 's'));
      s = "";
      operands_mask = (1 << 0) | (1 << 2);
      break;
    case TME_RECODE_OPCODE_DEFC:
      printf("defc\t %%%ccc:%s, %%c%u\n",
	     (insns->tme_recode_insn_conds_thunk == ic->tme_sparc_recode_conds_thunk_icc
	      ? 'i'
	      : insns->tme_recode_insn_conds_thunk == ic->tme_sparc_recode_conds_thunk_xcc
	      ? 'x'
	      : 'r'),
	     conds_integer
	     [insns->tme_recode_insn_operand_src[0]
	      + (TME_SPARC_COND_NOT
		 * (insns->tme_recode_insn_operand_src[1] != 0))],
	     insns->tme_recode_insn_operand_dst);
      continue;
    case TME_RECODE_OPCODE_IF:
      printf("%s\t %%c%u\n", s, insns->tme_recode_insn_operand_src[0]);
      continue;
    case TME_RECODE_OPCODE_ELSE:
    case TME_RECODE_OPCODE_ENDIF:
      printf("%s\n", s);
      continue;
    case TME_RECODE_OPCODE_RW:
      for (thunk_i = 0;; thunk_i++) {
	assert (thunk_i < TME_ARRAY_ELS(ic->tme_sparc_recode_rw_thunks));
	if (insns->tme_recode_insn_rw_thunk == ic->tme_sparc_recode_rw_thunks[thunk_i]) {
	  break;
	}
      }
      s = "?rw?";
      switch (thunk_i % 0x10) {
      case 0x0: s = (TME_SPARC_VERSION(ic) >= 9 ? "lduw" : "ld"); break;
      case 0x1: s = "ldub"; break;
      case 0x2: s = "lduh"; break;
      case 0x4: s = (TME_SPARC_VERSION(ic) >= 9 ? "stw" : "st"); break;
      case 0x5: s = "stb"; break;
      case 0x6: s = "sth"; break;
      case 0x8: if (TME_SPARC_VERSION(ic) >= 9) { s = "ldsw"; } break;
      case 0x9: s = "ldsb"; break;
      case 0xa: s = "ldsh"; break;
      case 0xb: if (TME_SPARC_VERSION(ic) >= 9) { s = "ldx"; } break;
      case 0xe: if (TME_SPARC_VERSION(ic) >= 9) { s = "stx"; } break;
      }
      if (thunk_i & 16) {
	printf("%s", s);
	s = ".nf";
      }
      if (thunk_i & 32) {
	printf("%s", s);
	s = ".el";
      }
      if (thunk_i & 64) {
	printf("%s", s);
	s = ".am";
      }
      insn_size = 0;
      operands_mask = (1 << 0);
      if (insns->tme_recode_insn_rw_thunk->tme_recode_rw_thunk_write) {
	operands_mask |= (1 << 1);
      }
      else {
	operands_mask |= (1 << 2);
      }
      break;
    default:
      operands_mask = (1 << 0) | (1 << 1) | (1 << 2);
      break;
    }

    printf("%s", s);
    if (insn_size > 0) {
      printf("%u", (1 << insns->tme_recode_insn_size));
    }
    delim = '\t';
    for (operand_i = 0; operand_i <= 2; operand_i++) {
      if (operands_mask & (1 << operand_i)) {
	putchar(delim);
	putchar(' ');
	delim = ',';
	switch (insns->tme_recode_insn_operands[operand_i]) {
	case TME_RECODE_OPERAND_UNDEF:
	  s = "undef";
	  break;
	case TME_RECODE_OPERAND_IMM:
	  tme_recode_uguest_dump(insns->tme_recode_insn_imm_uguest);
	  continue;
#if TME_RECODE_OPERAND_NULL != TME_RECODE_OPERAND_ZERO
#error "TME_RECODE_OPERAND_ values changed"
#endif
	case TME_RECODE_OPERAND_ZERO: 
	  s = "g0";
	  if (operand_i == 2
	      && insns->tme_recode_insn_opcode == TME_RECODE_OPCODE_GUEST) {
	    s = "null";
	  }
	  break;
	case TME_SPARC_IREG_PC: s = "pc"; break;
	case TME_SPARC_IREG_PC_NEXT: s = "pc_next"; break;
	case TME_SPARC_IREG_PC_NEXT_NEXT: s = "pc_next_next"; break;
	case TME_SPARC_IREG_INSN: s = "insn"; break;
	case TME_SPARC_IREG_TMP(0): s = "tmp0"; break;
	case TME_SPARC_IREG_TMP(1): s = "tmp1"; break;
	case TME_SPARC_IREG_TMP(2): s = "tmp2"; break;
	default:
	  if (insns->tme_recode_insn_operands[operand_i] < 32) {
	    printf("%%%c%u",
		   regcs[insns->tme_recode_insn_operands[operand_i] / 8],
		   (insns->tme_recode_insn_operands[operand_i] % 8));
	    continue;
	  }
	  s = "??";
	  break;
	}
	printf("%%%s", s);
      }
    }

    if (insns->tme_recode_insn_opcode < TME_RECODE_OPCODES_INTEGER
	&& insns->tme_recode_insn_flags_thunk != NULL) {
      printf(", %%%s",
	     (TME_SPARC_VERSION(ic) < 9
	      ? "icc"
	      : insns->tme_recode_insn_flags_thunk == ic->tme_sparc_recode_flags_thunk_rcc
	      ? "rcc"
	      : "ccr"));
    }	     

    putchar('\n');
  }
  
  chain_info = ic->tme_sparc_recode_insns_group.tme_recode_insns_group_chain_info;
  printf("chain %s %s ",
	 ((chain_info & TME_RECODE_CHAIN_INFO_JUMP)
	  ? "jump"
	  : (chain_info & TME_RECODE_CHAIN_INFO_RETURN)
	  ? "return"
	  : "call"),
	 (((chain_info
	    & (TME_RECODE_CHAIN_INFO_NEAR
	       | TME_RECODE_CHAIN_INFO_FAR))
	   == TME_RECODE_CHAIN_INFO_NEAR)
	  ? "near"
	  : "far"));
  if ((chain_info
       & (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
	  | TME_RECODE_CHAIN_INFO_CONDITIONAL))
      == TME_RECODE_CHAIN_INFO_UNCONDITIONAL) {
    printf("unconditional\n");
  }
  else {
    printf("conditional alternate %s\n",
	   (((chain_info
	      & (TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR
		 | TME_RECODE_CHAIN_INFO_ALTERNATE_FAR))
	     == TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR)
	    ? "near"
	    : "far"));
  }
}

#endif /* TME_RECODE_DEBUG */

#endif /* TME_HAVE_RECODE */

/* this initializes sparc recoding: */
void
tme_sparc_recode_init(struct tme_sparc *ic)
{
#if TME_HAVE_RECODE
  unsigned long reg_guest_count;
  unsigned long reg_guest;
  struct tme_recode_ic *recode_ic;

  /* get the number of guest registers: */
  reg_guest_count = TME_SPARC_IREG_TMP(3);

  /* allocate the recode ic: */
  recode_ic
    = tme_malloc0(sizeof(struct tme_recode_ic)
		  + (sizeof(union tme_recode_reginfo)
		     * reg_guest_count));
  ic->tme_sparc_recode_ic = recode_ic;

  /* set the register size: */
  recode_ic->tme_recode_ic_reg_size = TME_SPARC_RECODE_SIZE(ic);

  /* set the register count: */
  recode_ic->tme_recode_ic_reg_count = reg_guest_count;

  /* allocate some number of read-uses records: */
  /* XXX FIXME - this can be anything, and doesn't affect correctness,
     just performance.  should it be tunable? */
  recode_ic->tme_recode_ic_reg_guest_ruses_record_count = 512;
  recode_ic->tme_recode_ic_reg_guest_ruses_records
    = tme_new(tme_uint16_t, 
	      recode_ic->tme_recode_ic_reg_guest_ruses_record_count + 1);
  recode_ic->tme_recode_ic_reg_guest_ruses_records
    [recode_ic->tme_recode_ic_reg_guest_ruses_record_count]
    = TME_RECODE_REG_RUSES_RECORD_UNDEF;

  /* assume that all registers use flat addressing: */
  for (reg_guest = 0;
       reg_guest < reg_guest_count;
       reg_guest++) {
    (recode_ic->tme_recode_ic_reginfo
     + reg_guest)->tme_recode_reginfo_all = TME_RECODE_REGINFO_TYPE_FLAT;
  }

  /* %g0 is fixed: */
  (recode_ic->tme_recode_ic_reginfo
   + TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0))->tme_recode_reginfo_all
    |= TME_RECODE_REGINFO_TYPE_FIXED;

  /* %r8 through %r23 are addressed through window zero, and %r24
     through %r31 are addressed through window one: */
  reg_guest = TME_RECODE_REG_GUEST(8);
  do {
    (recode_ic->tme_recode_ic_reginfo
     + reg_guest)->tme_recode_reginfo_all = TME_RECODE_REGINFO_TYPE_WINDOW(0);
  } while (++reg_guest <= TME_RECODE_REG_GUEST(23));
  do {
    (recode_ic->tme_recode_ic_reginfo
     + reg_guest)->tme_recode_reginfo_all = TME_RECODE_REGINFO_TYPE_WINDOW(1);
  } while (++reg_guest <= TME_RECODE_REG_GUEST(31));

  /* on a v9 CPU, %r1 through %r7 are addressed through window two: */
  if (TME_SPARC_VERSION(ic) >= 9) {
    reg_guest = TME_RECODE_REG_GUEST(1);
    do {
      (recode_ic->tme_recode_ic_reginfo
       + reg_guest)->tme_recode_reginfo_all = TME_RECODE_REGINFO_TYPE_WINDOW(2);
    } while (++reg_guest <= TME_RECODE_REG_GUEST(7));
  }

  /* the temporary registers are temporary: */
  reg_guest = TME_RECODE_REG_GUEST(TME_SPARC_IREG_TMP(0));
  do {
    (recode_ic->tme_recode_ic_reginfo
     + reg_guest)->tme_recode_reginfo_all
      |= TME_RECODE_REGINFO_TYPE_TEMPORARY;
  } while (++reg_guest <= TME_RECODE_REG_GUEST(TME_SPARC_IREG_TMP(2)));

  /* set the window base offsets: */
  recode_ic->tme_recode_ic_window_base_offsets[0]
    = (((char *) &ic->tme_sparc_recode_window_base_offsets[0]) - ((char *) ic));
  recode_ic->tme_recode_ic_window_base_offsets[1]
    = (((char *) &ic->tme_sparc_recode_window_base_offsets[1]) - ((char *) ic));
  recode_ic->tme_recode_ic_window_base_offsets[2]
    = (((char *) &ic->tme_sparc_recode_window_base_offsets[2]) - ((char *) ic));

  /* initialize the cacheables: */
  ic->tme_sparc_recode_cacheables[0].tme_sparc_recode_cacheable_size = 0;
  ic->tme_sparc_recode_cacheable_first = &ic->tme_sparc_recode_cacheables[0];

  /* invalidate the entire source hash: */
  _tme_sparc_recode_src_hash_invalidate(ic, 0, 0);

  /* set the TLB page size: */
  recode_ic->tme_recode_ic_tlb_page_size
    = (((tme_uint32_t) 1) << ic->tme_sparc_tlb_page_size_log2);

  /* set the pointer to the first instruction in a group: */
  ic->tme_sparc_recode_insns_group.tme_recode_insns_group_insns
    = &ic->tme_sparc_recode_insns[0];

  /* initialize the recode ic: */
  /* XXX FIXME - this run size should be passed in as an argument.  it
     needs to be big enough for the maximum number of non-variable
     thunks (flags, conds, chain, etc.) that we will make, probably
     TME_RECODE_HOST_THUNK_SIZE_MAX times some n? */
  tme_recode_ic_new(recode_ic, 4 * 1024 * 1024);

  /* call the architecture-specific init functions: */
  if (TME_SPARC_VERSION(ic) >= 9) {
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
    _tme_sparc64_recode_cc_init(ic);
    _tme_sparc64_recode_chain_init(ic);
    _tme_sparc64_recode_ls_init(ic);
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
  }
  else {
    _tme_sparc32_recode_cc_init(ic);
    _tme_sparc32_recode_chain_init(ic);
    _tme_sparc32_recode_ls_init(ic);
  }

#ifdef _TME_SPARC_RECODE_VERIFY

  /* initialize the verifier: */
  _tme_sparc_recode_verify_init(ic);

#endif /* _TME_SPARC_RECODE_VERIFY */

#else  /* !TME_HAVE_RECODE */
  /* unused: */
  ic = 0;
#endif /* !TME_HAVE_RECODE */
}
