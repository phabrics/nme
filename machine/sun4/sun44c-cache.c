/* $Id: sun44c-cache.c,v 1.3 2009/08/30 14:04:24 fredette Exp $ */

/* machine/sun4/sun44c-cache.c - implementation of Sun 4/4c cache emulation: */

/*
 * Copyright (c) 2006 Matt Fredette
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
_TME_RCSID("$Id: sun44c-cache.c,v 1.3 2009/08/30 14:04:24 fredette Exp $");

/* includes: */
#include "sun4-impl.h"

/* macros: */

/* real sun4/4c cache tag bits: */
#define TME_SUN44C_CACHETAG_CONTEXT	(0x03c00000)
#define TME_SUN44C_CACHETAG_WRITE	(0x00200000)
#define TME_SUN44C_CACHETAG_SYSTEM	(0x00100000)
#define TME_SUN44C_CACHETAG_VALID	(0x00080000)
#define TME_SUN44C_CACHETAG_TAG		(0x0000fffc)

/* actions that the cache may take for an access: */
#define _TME_SUN44C_CACHE_ACTION_NULL		(0)
#define _TME_SUN44C_CACHE_ACTION_FLUSH		TME_BIT(0)
#define _TME_SUN44C_CACHE_ACTION_INVALIDATE	TME_BIT(1)
#define _TME_SUN44C_CACHE_ACTION_ALLOCATE	(TME_BIT(2) | _TME_SUN44C_CACHE_ACTION_FLUSH | _TME_SUN44C_CACHE_ACTION_INVALIDATE)
#define _TME_SUN44C_CACHE_ACTION_READ		TME_BIT(3)
#define _TME_SUN44C_CACHE_ACTION_WRITE_BACK	TME_BIT(4)
#define _TME_SUN44C_CACHE_ACTION_WRITE_THROUGH	TME_BIT(5)
#define _TME_SUN44C_CACHE_ACTION_ERROR_WRITE	TME_BIT(6)
#define _TME_SUN44C_CACHE_ACTION_ERROR_SYSTEM	TME_BIT(7)
#define _TME_SUN44C_CACHE_ACTION_ERROR_MEMERR	TME_BIT(8)

/* each time the cache is enabled or its tag or data memory is
   accessed directly, the cache remains visible for this many more bus
   cycles.  eventually, the cache becomes invisible again, to allow
   for maximum performance - but if this value is too low, diagnostics
   may fail.

   some diagnostics do one bus cycle per cache address, so this must
   be at least the size of the largest system's cache, plus some
   slack: */
#define _TME_SUN44C_CACHE_VISIBLE_BUS_CYCLES	(65536 + 1024)

/* this returns the next TLB fill function: */
#define _tme_sun44c_cache_tlb_fill_next(sun4)	\
  (TME_SUN44C_MEMERR_VISIBLE(sun4)		\
   ? _tme_sun44c_tlb_fill_memerr		\
   : _tme_sun44c_tlb_fill_mmu)

/* this returns a valid cache tag for an address and context: */
static tme_uint32_t
_tme_sun44c_cache_tag(struct tme_sun4 *sun4,
		      tme_uint32_t context,
		      tme_uint32_t address)
{
  tme_uint32_t cache_tag;

  /* start with the address tag: */
  cache_tag
    = (((address >> sun4->tme_sun4_cache_size_log2)
	* _TME_FIELD_MASK_FACTOR(TME_SUN44C_CACHETAG_TAG))
       & TME_SUN44C_CACHETAG_TAG);

  /* add the context: */
  TME_FIELD_MASK_DEPOSITU(cache_tag, TME_SUN44C_CACHETAG_CONTEXT, context);

  /* add the valid bit: */
  cache_tag |= TME_SUN44C_CACHETAG_VALID;

  return (cache_tag);
}  

/* this returns the mask of actions that the cache will take for an
   access: */
static unsigned int
_tme_sun44c_cache_actions(const struct tme_bus_connection *conn_bus_init,
			  tme_uint32_t asi_mask,
			  tme_uint32_t address,
			  unsigned int cycles)
{
  struct tme_sun4 *sun4;
  tme_uint32_t context;
  tme_uint32_t cache_line_index;
  tme_uint32_t cache_tag_current;
  tme_uint32_t cache_tag_mask;
  struct tme_sun_mmu_pte pte_mmu;
  tme_uint32_t pte;
  unsigned int cache_actions;
  int rc;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the context: */
  context = TME_SUN44C_BUS_MMU_CONTEXT(sun4, conn_bus_init);

  /* this ASI mask must be a single ASI, for user or supervisor
     instruction or data: */
  assert (asi_mask == TME_SPARC32_ASI_MASK_UD
	  || asi_mask == TME_SPARC32_ASI_MASK_UI
	  || asi_mask == TME_SPARC32_ASI_MASK_SD
	  || asi_mask == TME_SPARC32_ASI_MASK_SI);

  /* this must be a plain read or write: */
  assert (cycles == TME_BUS_CYCLE_READ
	  || cycles == TME_BUS_CYCLE_WRITE);

  /* turn this address into the cache line index: */
  cache_line_index
    = (address >> sun4->tme_sun4_cache_size_line_log2
       & ((1 << (sun4->tme_sun4_cache_size_log2
		 - sun4->tme_sun4_cache_size_line_log2))
	  - 1));

  /* get the current cache tag for this cache line: */
  cache_tag_current = sun4->tme_sun44c_cache_tags[cache_line_index];

  /* NB: apparently the context field of a valid cache tag for system
     memory isn't significant when doing a tag comparison: */
  cache_tag_mask
    = (TME_SUN44C_CACHETAG_VALID
       | TME_SUN44C_CACHETAG_CONTEXT
       | TME_SUN44C_CACHETAG_TAG);
  if (cache_tag_current & TME_SUN44C_CACHETAG_SYSTEM) {
    cache_tag_mask
      = (TME_SUN44C_CACHETAG_VALID
	 | TME_SUN44C_CACHETAG_TAG);
  }

  /* if this address and context hit in the cache: */
  if (((cache_tag_current
	^ _tme_sun44c_cache_tag(sun4, context, address))
       & cache_tag_mask) == 0) {

    /* if the current cache tag only allows system access, but this is
       a user access: */
    if ((cache_tag_current & TME_SUN44C_CACHETAG_SYSTEM)
	&& TME_SPARC_ASI_MASK_OVERLAP(asi_mask,
				      (TME_SPARC32_ASI_MASK_UD
				       | TME_SPARC32_ASI_MASK_UI))) {
      return (_TME_SUN44C_CACHE_ACTION_ERROR_SYSTEM);
    }

    /* if the current cache tag only allows read access, but this is a
       write access: */
    if (!(cache_tag_current & TME_SUN44C_CACHETAG_WRITE)
	&& cycles == TME_BUS_CYCLE_WRITE) {
      return (_TME_SUN44C_CACHE_ACTION_ERROR_WRITE);
    }

    /* this access is a hit in the cache: */
    cache_actions = 0;
  }

  /* otherwise, this address and context miss in the cache: */
  else {

    /* if this ASI mask is for supervisor instructions, and we're in
       the boot state: */
    if (asi_mask == TME_SPARC32_ASI_MASK_SI
	&& (sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT) == 0) {
      return (_TME_SUN44C_CACHE_ACTION_NULL);
    }

    /* get the PTE for this address and context from the MMU: */
    rc = tme_sun_mmu_pte_get(sun4->tme_sun44c_mmu, 
			     context,
			     address,
			     &pte_mmu);
    assert (rc == TME_OK);
    pte = pte_mmu.tme_sun_mmu_pte_raw;

    /* if this PTE is not valid: */
    if (!(pte & TME_SUN44C_PTE_VALID)) {
      return (_TME_SUN44C_CACHE_ACTION_NULL);
    }

    /* if this PTE is not for cacheable space: */
    if (pte & TME_SUN44C_PTE_NC) {
      return (_TME_SUN44C_CACHE_ACTION_NULL);
    }

    /* if this PTE only allows system access, but this is a user
       access: */
    if ((pte & TME_SUN44C_PTE_SYSTEM)
	&& TME_SPARC_ASI_MASK_OVERLAP(asi_mask,
				      (TME_SPARC32_ASI_MASK_UD
				       | TME_SPARC32_ASI_MASK_UI))) {
      return (_TME_SUN44C_CACHE_ACTION_ERROR_SYSTEM);
    }

    /* if this is a write access: */
    if (cycles == TME_BUS_CYCLE_WRITE) {

      /* if this PTE doesn't allow write access: */
      if (!(pte & TME_SUN44C_PTE_WRITE)) {
	return (_TME_SUN44C_CACHE_ACTION_ERROR_WRITE);
      }

      /* on a write miss, we invalidate, but we don't allocate: */
      sun4->tme_sun44c_cache_tags[cache_line_index] &= ~TME_SUN44C_CACHETAG_VALID;
      return (_TME_SUN44C_CACHE_ACTION_NULL);
    }

    /* otherwise, this PTE is valid and cacheable, so it will
       cause an allocation in the cache: */
    cache_actions = _TME_SUN44C_CACHE_ACTION_ALLOCATE;
  }

  /* this access will be handled by the cache: */
  return (cache_actions
	  | (cycles == TME_BUS_CYCLE_READ
	     ? _TME_SUN44C_CACHE_ACTION_READ
	     : (sun4->tme_sun4_cache_writeback
		? _TME_SUN44C_CACHE_ACTION_WRITE_BACK
		: _TME_SUN44C_CACHE_ACTION_WRITE_THROUGH)));
}

/* for a particular address, this returns the PTE entry for the
   address, and fills the cache internal TLB entry to point to the
   main memory for the address' cache line.  NB that this returns with
   the cache internal TLB entry busy: */
static tme_uint32_t
_tme_sun4_cache_tlb_internal_fill(const struct tme_bus_connection *conn_bus_init,
				  tme_uint32_t address,
				  unsigned int cycle_type)
{
  struct tme_sun4 *sun4;
  int rc;
  struct tme_sun_mmu_pte pte_mmu;
  tme_uint32_t context;
  tme_uint32_t asi_mask;
  tme_uint32_t pte;
  tme_uint32_t cache_size_line;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the context: */
  context = TME_SUN44C_BUS_MMU_CONTEXT(sun4, conn_bus_init);

  /* get the PTE for this address and context from the MMU: */
  rc = tme_sun_mmu_pte_get(sun4->tme_sun44c_mmu, 
			   context,
			   address,
			   &pte_mmu);
  assert (rc == TME_OK);

  /* this PTE must be valid for cacheable obmem space, and, if we're
     writing, it must allow writing: */
  pte = pte_mmu.tme_sun_mmu_pte_raw;
  assert (((pte
	    & (TME_SUN44C_PTE_VALID
	       | TME_SUN44C_PTE_NC))
	   == TME_SUN44C_PTE_VALID)
	  && (cycle_type == TME_BUS_CYCLE_READ
	      || (pte & TME_SUN44C_PTE_WRITE)));
  if ((pte & TME_SUN44C_PTE_PGTYPE) != 0 /* TME_SUN44C_PGTYPE_OBMEM */) {
    abort();
  }

  /* get the size of one cache line: */
  cache_size_line = 1 << sun4->tme_sun4_cache_size_line_log2;

  /* busy the cache internal TLB entry: */
  tme_bus_tlb_busy(&sun4->tme_sun4_cache_tlb_internal);

  /* loop until we can get a valid TLB entry: */
  do {

    /* unbusy the cache internal TLB entry for filling: */
    tme_bus_tlb_unbusy_fill(&sun4->tme_sun4_cache_tlb_internal);

    /* fill the cache internal TLB entry directly from the MMU.  we
       handle memory errors ourselves, so we don't need to call
       _tme_sun44c_tlb_fill_memerr when memory errors are visible: */
    asi_mask = TME_SPARC32_ASI_MASK_SD;
    rc = _tme_sun44c_tlb_fill_mmu(conn_bus_init,
				  &sun4->tme_sun4_cache_tlb_internal,
				  &asi_mask,
				  address & (((tme_uint32_t) 0) - cache_size_line),
				  cycle_type);
    assert (rc == TME_OK);

    /* rebusy the cache internal TLB entry: */
    tme_bus_tlb_busy(&sun4->tme_sun4_cache_tlb_internal);

    /* loop if the cache internal TLB entry is invalid: */
  } while (tme_bus_tlb_is_invalid(&sun4->tme_sun4_cache_tlb_internal));

  /* this TLB entry must allow fast access to the entire cache line.
     this limitation is caused by our poor emulation, but the real
     hardware may not deal well either with addresses that are marked
     cacheable but don't map to memory: */
  assert ((sun4->tme_sun4_cache_tlb_internal.tme_bus_tlb_emulator_off_read
	   != TME_EMULATOR_OFF_UNDEF)
	  && (cycle_type == TME_BUS_CYCLE_READ
	      || (sun4->tme_sun4_cache_tlb_internal.tme_bus_tlb_emulator_off_write
		  != TME_EMULATOR_OFF_UNDEF))
	  && (sun4->tme_sun4_cache_tlb_internal.tme_bus_tlb_addr_first
	      <= (address & -cache_size_line))
	  && (sun4->tme_sun4_cache_tlb_internal.tme_bus_tlb_addr_last
	      >= (address | (cache_size_line - 1))));

  /* done: */
  return (pte_mmu.tme_sun_mmu_pte_raw);
}

/* this flushes a line from the cache: */
static void
_tme_sun44c_cache_line_flush(struct tme_sun4 *sun4,
			     tme_uint32_t cache_line_index)
{

  /* if this is a write-through cache, return now: */
  if (!sun4->tme_sun4_cache_writeback) {
    return;
  }

  /* XXX WRITEME: */
  abort();
}

/* this handles a bus cycle in the cache: */
static int
_tme_sun44c_cache_cycle_bus(void *_conn_bus_init,
			    struct tme_bus_cycle *cycle)
{
  const struct tme_bus_connection *conn_bus_init;
  struct tme_sun4 *sun4;
  tme_uint32_t address;
  tme_uint8_t context;
  tme_uint32_t asi_mask;
  unsigned int cache_actions;
  tme_uint32_t cache_size;
  tme_uint32_t cache_size_line;
  tme_uint32_t cache_data_offset;
  tme_shared tme_uint8_t *cache_data;
  tme_uint32_t cache_word;
  struct tme_bus_tlb *tlb;
  const tme_shared tme_uint8_t *memory_data_read;
  tme_shared tme_uint8_t *memory_data_write;
  tme_uint32_t cache_line_index;
  tme_uint32_t cache_tag;
  tme_uint32_t pte;

  /* recover our initiator's bus connection and sun4: */
  conn_bus_init = (struct tme_bus_connection *) _conn_bus_init;
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the context, address, and ASI mask: */
  context = TME_SUN44C_BUS_MMU_CONTEXT(sun4, conn_bus_init);
  address = cycle->tme_bus_cycle_address;
  asi_mask = sun4->tme_sun4_memtest_tlb_asi_mask;

  /* invalidate the TLB entry that triggered this cycle.  this hurts
     performance, since it keeps TLB entries valid only for one cycle
     at a time, but it's simple and handles the problem of PTE changes
     while the cache is visible (addresses can suddenly become
     cacheable): */
  assert (sun4->tme_sun4_memtest_tlb != NULL);
  tme_token_invalidate(sun4->tme_sun4_memtest_tlb->tme_bus_tlb_token);
  sun4->tme_sun4_memtest_tlb = NULL;

  /* get the cache actions for this access: */
  cache_actions = _tme_sun44c_cache_actions(conn_bus_init,
					    asi_mask,
					    address,
					    cycle->tme_bus_cycle_type);
  assert (cache_actions != _TME_SUN44C_CACHE_ACTION_NULL);

  /* if the cache actions include any involving the cache line for
     this address: */
  if (cache_actions
      & ~(_TME_SUN44C_CACHE_ACTION_ERROR_WRITE
	  | _TME_SUN44C_CACHE_ACTION_ERROR_SYSTEM)) {

    /* get the total size of the cache, and the size of one line: */
    cache_size = 1 << sun4->tme_sun4_cache_size_log2;
    cache_size_line = 1 << sun4->tme_sun4_cache_size_line_log2;

    /* turn this address into a pointer to the cache data for its
       cache line, and into the cache line index: */
    cache_data_offset = address & (cache_size - cache_size_line);
    cache_data = sun4->tme_sun4_cache_data + cache_data_offset;
    cache_line_index = cache_data_offset >> sun4->tme_sun4_cache_size_line_log2;

    /* if this cache line needs to be flushed: */
    if (cache_actions & _TME_SUN44C_CACHE_ACTION_FLUSH) {
      _tme_sun44c_cache_line_flush(sun4, cache_line_index);
    }

    /* if this cache line needs to be invalidated: */
    if (cache_actions & _TME_SUN44C_CACHE_ACTION_INVALIDATE) {
      sun4->tme_sun44c_cache_tags[cache_line_index] &= ~TME_SUN44C_CACHETAG_VALID;
    }

    /* assume that we won't access main memory: */
    pte = 0;
    tlb = NULL;

    /* if this cache line needs to be allocated or written-through: */
    if (((cache_actions & _TME_SUN44C_CACHE_ACTION_ALLOCATE)
	 == _TME_SUN44C_CACHE_ACTION_ALLOCATE)
	|| (cache_actions & _TME_SUN44C_CACHE_ACTION_WRITE_THROUGH)) {

      /* we will access main memory, so get this address' PTE entry
         and fill the cache internal TLB: */
      pte = _tme_sun4_cache_tlb_internal_fill(conn_bus_init,
					      address,
					      ((cache_actions & _TME_SUN44C_CACHE_ACTION_WRITE_THROUGH)
					       ? TME_BUS_CYCLE_WRITE
					       : TME_BUS_CYCLE_READ));
      tlb = &sun4->tme_sun4_cache_tlb_internal;
    }

    /* if this cache line needs to be allocated: */
    if ((cache_actions & _TME_SUN44C_CACHE_ACTION_ALLOCATE) == _TME_SUN44C_CACHE_ACTION_ALLOCATE) {
    
      /* make the new cache tag: */
      cache_tag = _tme_sun44c_cache_tag(sun4, context, address);
      if (pte & TME_SUN44C_PTE_SYSTEM) {
	cache_tag |= TME_SUN44C_CACHETAG_SYSTEM;
      }
      if (pte & TME_SUN44C_PTE_WRITE) {
	cache_tag |= TME_SUN44C_CACHETAG_WRITE;
      }

      /* fill the cache line: */
      memory_data_read
	= (tlb->tme_bus_tlb_emulator_off_read
	   + (address & -cache_size_line));
      cache_data_offset = 0;
      do {
	cache_word 
	  = tme_memory_bus_read32(((const tme_shared tme_uint32_t *)
				   (memory_data_read
				    + cache_data_offset)),
				  tlb->tme_bus_tlb_rwlock,
				  sizeof(cache_word),
				  sizeof(cache_word));
	tme_memory_bus_write32(((tme_shared tme_uint32_t *)
				(cache_data
				 + cache_data_offset)),
			       cache_word,
			       &sun4->tme_sun4_cache_rwlock,
			       sizeof(cache_word),
			       sizeof(cache_word));
	cache_data_offset += sizeof(cache_word);
      } while (cache_data_offset < cache_size_line);

      /* update the cache tag: */
      sun4->tme_sun44c_cache_tags[cache_line_index] = cache_tag;

      /* check for memory errors on this cache line fill: */
      if (_tme_sun44c_memerr_check(conn_bus_init,
				   (address & -cache_size_line),
				   pte,
				   memory_data_read,
				   cache_size_line) != TME_OK) {
	cache_actions |= _TME_SUN44C_CACHE_ACTION_ERROR_MEMERR;
      }
    }

    /* if we're reading or writing this cache line: */
    if (cache_actions
	& (_TME_SUN44C_CACHE_ACTION_READ
	   | _TME_SUN44C_CACHE_ACTION_WRITE_BACK
	   | _TME_SUN44C_CACHE_ACTION_WRITE_THROUGH)) {

      /* the address must be size-aligned, and the transfer must
	 fit within the cache line: */
      assert ((address % cycle->tme_bus_cycle_size) == 0
	      && ((address
		   ^ (address
		      + cycle->tme_bus_cycle_size
		      - 1))
		  < cache_size_line));

      /* get a pointer to the cache data to transfer: */
      cache_data += (address & (cache_size_line - 1));

      /* run the bus cycle against the cache line data: */
      /* XXX FIXME this is not thread-safe: */
      tme_bus_cycle_xfer_memory(cycle, 
				((tme_uint8_t *) cache_data) - address,
				address + cycle->tme_bus_cycle_size - 1);

      /* if we're writing through this cache line: */
      if (cache_actions & _TME_SUN44C_CACHE_ACTION_WRITE_THROUGH) {

	/* write the data through the cache: */
	memory_data_write
	  = (tlb->tme_bus_tlb_emulator_off_write
	     + address);
	switch (cycle->tme_bus_cycle_size) {
	default:
	  assert (FALSE);
	  /* FALLTHROUGH */
	case sizeof(tme_uint8_t):
	  tme_memory_bus_write8(memory_data_write,
				tme_memory_bus_read8(cache_data,
						     &sun4->tme_sun4_cache_rwlock,
						     sizeof(tme_uint8_t),
						     sizeof(tme_uint32_t)),
				tlb->tme_bus_tlb_rwlock,
				sizeof(tme_uint8_t),
				sizeof(tme_uint32_t));
	  break;
	case sizeof(tme_uint16_t):
	  tme_memory_bus_write16((tme_shared tme_uint16_t *) memory_data_write,
				 tme_memory_bus_read16((const tme_shared tme_uint16_t *) cache_data,
						       &sun4->tme_sun4_cache_rwlock,
						       sizeof(tme_uint16_t),
						       sizeof(tme_uint32_t)),
				 tlb->tme_bus_tlb_rwlock,
				 sizeof(tme_uint16_t),
				 sizeof(tme_uint32_t));
	  break;
	case sizeof(tme_uint32_t):
	  tme_memory_bus_write32((tme_shared tme_uint32_t *) memory_data_write,
				 tme_memory_bus_read32((const tme_shared tme_uint32_t *) cache_data,
						       &sun4->tme_sun4_cache_rwlock,
						       sizeof(tme_uint32_t),
						       sizeof(tme_uint32_t)),
				 tlb->tme_bus_tlb_rwlock,
				 sizeof(tme_uint32_t),
				 sizeof(tme_uint32_t));
	  break;
	}

	/* update any memory errors caused by this write: */
	_tme_sun44c_memerr_update(sun4,
				  pte,
				  memory_data_write,
				  cycle->tme_bus_cycle_size);
      }
    }

    /* if we accessed main memory: */
    if (tlb != NULL) {

      /* unbusy the cache internal TLB entry: */
      tme_bus_tlb_unbusy(tlb);
    }
  }

  /* if we're returning a protection error: */
  if (cache_actions
      & (_TME_SUN44C_CACHE_ACTION_ERROR_SYSTEM
	 | _TME_SUN44C_CACHE_ACTION_ERROR_WRITE)) {
    return (_tme_sun44c_mmu_proterr((void *) conn_bus_init, cycle));
  }

  /* if we're returning a memory error: */
  if (cache_actions & _TME_SUN44C_CACHE_ACTION_ERROR_MEMERR) {
    return (_tme_sun44c_ob_fault_handler((void *) conn_bus_init, NULL, cycle, EIO));
  }

  /* return success: */
  return (TME_OK);
}

/* this fills TLBs when the cache is visible: */
int
_tme_sun44c_tlb_fill_cache(const struct tme_bus_connection *conn_bus_init,
			   struct tme_bus_tlb *tlb,
			   tme_uint32_t *_asi_mask,
			   tme_uint32_t address,
			   unsigned int cycles)
{
  struct tme_sun4 *sun4;
  tme_uint32_t asi_mask;
  tme_uint32_t cache_size_line;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* recover the ASI mask: */
  asi_mask = *_asi_mask;

  /* this ASI mask must be a single ASI, for user or supervisor
     instruction or data: */
  assert (asi_mask == TME_SPARC32_ASI_MASK_UD
	  || asi_mask == TME_SPARC32_ASI_MASK_UI
	  || asi_mask == TME_SPARC32_ASI_MASK_SD
	  || asi_mask == TME_SPARC32_ASI_MASK_SI);

  /* invalidate any other memory test TLB entry: */
  if (sun4->tme_sun4_memtest_tlb != NULL
      && sun4->tme_sun4_memtest_tlb != tlb) {
    tme_token_invalidate(sun4->tme_sun4_memtest_tlb->tme_bus_tlb_token);
  }
  sun4->tme_sun4_memtest_tlb = NULL;

  /* the cache must be visible: */
  assert (sun4->tme_sun4_cache_visible > 0);

  /* if the cache is no longer visible: */
  if ((--sun4->tme_sun4_cache_visible) == 0) {

    /* update the TLB fill function: */
    sun4->tme_sun4_tlb_fill = _tme_sun44c_cache_tlb_fill_next(sun4);
  }

  /* otherwise, if this access will be handled by the cache: */
  else if (_tme_sun44c_cache_actions(conn_bus_init,
				     asi_mask,
				     address,
				     cycles)
	   != _TME_SUN44C_CACHE_ACTION_NULL) {

    /* fill this TLB entry: */
    tme_bus_tlb_initialize(tlb);
    cache_size_line = 1 << sun4->tme_sun4_cache_size_line_log2;
    tlb->tme_bus_tlb_addr_first = address & -cache_size_line;
    tlb->tme_bus_tlb_addr_last = address | (cache_size_line - 1);
    tlb->tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE);
    tlb->tme_bus_tlb_emulator_off_read = TME_EMULATOR_OFF_UNDEF;
    tlb->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
    tlb->tme_bus_tlb_cycle_private = (void *) conn_bus_init;
    tlb->tme_bus_tlb_cycle = _tme_sun44c_cache_cycle_bus;

    /* remember this TLB entry and ASI mask: */
    sun4->tme_sun4_memtest_tlb = tlb;
    sun4->tme_sun4_memtest_tlb_asi_mask = asi_mask;

    /* return success: */
    return (TME_OK);
  }

  /* chain to the next TLB fill function: */
  return ((*_tme_sun44c_cache_tlb_fill_next(sun4))(conn_bus_init,
						   tlb,
						   _asi_mask,
						   address,
						   cycles));
}

/* this updates the visibility of the cache: */
void
_tme_sun44c_cache_enable_change(struct tme_sun4 *sun4)
{

  /* if the cache is enabled in the boot state, it is visible: */
  if ((sun4->tme_sun44c_enable
       & (TME_SUN44C_ENA_CACHE
	  | TME_SUN44C_ENA_NOTBOOT)) == TME_SUN44C_ENA_CACHE) {

    /* if the cache is currently invisible: */
    if (!sun4->tme_sun4_cache_visible) {

      /* update the TLB fill function: */
      sun4->tme_sun4_tlb_fill = _tme_sun44c_tlb_fill_cache;

      /* invalidate all TLBs: */
      tme_sun_mmu_tlbs_invalidate(sun4->tme_sun44c_mmu);
    }

    /* refresh the cache visibility: */
    sun4->tme_sun4_cache_visible = _TME_SUN44C_CACHE_VISIBLE_BUS_CYCLES;
  }

  /* otherwise, the cache is not visible: */
  else {

    /* if the cache is currently visible: */
    if (sun4->tme_sun4_cache_visible) {

      /* the next TLB fill will not see the cache: */
      sun4->tme_sun4_cache_visible = 1;
    }
  }
}

/* this does a control bus cycle with either the tag or data cache
   memory: */
int
_tme_sun44c_cache_cycle_control(struct tme_sun4 *sun4,
				struct tme_bus_cycle *cycle)
{
  tme_uint32_t address;
  tme_uint32_t cache_size;
  tme_uint32_t cache_line_index;
  tme_uint32_t value32;

  /* get the total size of the cache: */
  cache_size = 1 << sun4->tme_sun4_cache_size_log2;

  /* get the address: */
  address = cycle->tme_bus_cycle_address;

  /* if this is an access to the cache tags: */
  if ((address ^ TME_SUN44C_CONTROL_CACHE_TAGS) < cache_size) {

    /* if the address isn't 32-bit aligned, we can't emulate this
       access: */
    if (address % sizeof(tme_uint32_t)) {
      abort();
    }

    /* get the cache line index: */
    cache_line_index
      = ((address
	  ^ TME_SUN44C_CONTROL_CACHE_TAGS)
	 >> sun4->tme_sun4_cache_size_line_log2);

    /* transfer the cache tag: */
    value32 = tme_htobe_u32(sun4->tme_sun44c_cache_tags[cache_line_index]);
    tme_bus_cycle_xfer_memory(cycle, 
			      (((tme_uint8_t *) &value32)
			       - cycle->tme_bus_cycle_address),
			      (cycle->tme_bus_cycle_address
			       + cycle->tme_bus_cycle_size
			       - 1));

    /* if this was a write: */
    if (cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

      /* only certain bits in a cache tag are writable: */
      value32 = tme_betoh_u32(value32);
      value32
	&= (TME_SUN44C_CACHETAG_CONTEXT
	    | TME_SUN44C_CACHETAG_WRITE
	    | TME_SUN44C_CACHETAG_SYSTEM
	    | TME_SUN44C_CACHETAG_VALID
	    | TME_SUN44C_CACHETAG_TAG);

      /* if the cache is enabled but not visible, and the tag being
	 written is not zero, we can't emulate this access: */
      if ((sun4->tme_sun44c_enable & TME_SUN44C_ENA_CACHE)
	  && !sun4->tme_sun4_cache_visible
	  && value32 != 0) {
	abort();
      }

      /* write the cache tag: */
      sun4->tme_sun44c_cache_tags[cache_line_index] = value32;
    }
  }

  /* otherwise, if this is an access to the cache data: */
  else if ((address ^ TME_SUN44C_CONTROL_CACHE_DATA) < cache_size) {

    /* if the cache is enabled but not visible, we can't emulate this
       access: */
    if ((sun4->tme_sun44c_enable & TME_SUN44C_ENA_CACHE)
	&& !sun4->tme_sun4_cache_visible) {
      abort();
    }

    /* transfer the cache data: */
    /* XXX FIXME this is not thread-safe: */
    tme_bus_cycle_xfer_memory(cycle, 
			      (tme_uint8_t *) 
			      (sun4->tme_sun4_cache_data
			       - TME_SUN44C_CONTROL_CACHE_DATA),
			      (TME_SUN44C_CONTROL_CACHE_DATA
			       + cache_size
			       - 1));
  }

  /* otherwise, we can't emulate this cycle: */
  else {
    abort();
  }

  /* refresh the cache visibility: */
  _tme_sun44c_cache_enable_change(sun4);
  return (TME_OK);
}

/* this does a flush bus cycle: */
void
_tme_sun44c_cache_cycle_flush(struct tme_sun4 *sun4,
			      tme_uint32_t asi,
			      tme_uint32_t address)
{
  tme_uint32_t cache_tag_mask;
  tme_uint32_t cache_tag_value;
  tme_uint32_t address_mask;
  tme_uint32_t cache_size;
  tme_uint32_t cache_line_index;
  tme_uint32_t cache_line_count;

  /* assume that this is not a hardware-assisted flush: */
  cache_line_count = 1;

  /* if this is a sun4c: */
  if (TME_SUN4_IS_SUN4C(sun4)) {

    /* if this is a hardware-assisted flush: */
#if ((TME_SUN44C_ASI_FLUSH_SEG + 1) != TME_SUN44C_ASI_FLUSH_PG) || ((TME_SUN44C_ASI_FLUSH_PG + 1) != TME_SUN44C_ASI_FLUSH_CONTEXT)
#error "bad sun4c unassisted flushing ASIs"
#endif
    if (asi < TME_SUN44C_ASI_FLUSH_SEG
	|| asi > TME_SUN44C_ASI_FLUSH_CONTEXT) {

      /* XXX FIXME - what happens when the address is not page-aligned? */
      if (address % TME_SUN4C_PAGE_SIZE) {
	abort();
      }

      /* we flush one page's worth of cache lines: */
      cache_line_count = TME_SUN4C_PAGE_SIZE >> sun4->tme_sun4_cache_size_line_log2;
    }
  }
  
  /* otherwise, this is a sun4: */
  else {
    assert(asi != TME_SUN4C_ASI_HW_FLUSH_SEG
	   && asi != TME_SUN4C_ASI_HW_FLUSH_PG);
  }

  /* the sun4c hw-flush-all and sun4 flush-user are handled specially: */
  if (asi == TME_SUN4C_ASI_HW_FLUSH_ALL /* TME_SUN4_ASI_FLUSH_USER */) {

    /* on the sun4c, this flushes any valid cache line.
       on the sun4, this flushes any valid user cache line: */
    cache_tag_value = TME_SUN44C_CACHETAG_VALID;
    cache_tag_mask
      = (TME_SUN4_IS_SUN4C(sun4)
	 ? TME_SUN44C_CACHETAG_VALID
	 : TME_SUN44C_CACHETAG_VALID | TME_SUN44C_CACHETAG_SYSTEM);
  }

  /* otherwise, this is flushing some subset of the context/address
     combinations that might be in the cache: */
  else {

    /* dispatch on the flush type, to get an mask of bits that we will
       ignore in the address when we make the tag: */
    address_mask = 0;
    cache_tag_mask = 0;
    switch(asi) {
    default:
      assert(FALSE);
      /* FALLTHROUGH */
    case TME_SUN4_ASI_FLUSH_REG: /* TME_SUN4C_ASI_HW_FLUSH_CONTEXT */
      if (!TME_SUN4_IS_SUN4C(sun4)) {
	abort();
	break;
      }
      /* FALLTHROUGH */
    case TME_SUN44C_ASI_FLUSH_CONTEXT: 
      address_mask -= 1;
      cache_tag_mask = TME_SUN44C_CACHETAG_SYSTEM;
      break;
    case TME_SUN44C_ASI_FLUSH_SEG:
    case TME_SUN4C_ASI_HW_FLUSH_SEG:
      address_mask -= TME_SUN4_32_SEGMENT_SIZE;
      break;
    case TME_SUN44C_ASI_FLUSH_PG:
    case TME_SUN4C_ASI_HW_FLUSH_PG:
      address_mask -= TME_SUN4C_PAGE_SIZE;
      break;
    }

    /* turn this address and mask into a cache tag value and mask: */
    cache_tag_value = _tme_sun44c_cache_tag(sun4, sun4->tme_sun44c_context, address & address_mask);
    cache_tag_mask |= _tme_sun44c_cache_tag(sun4, 0xff, address_mask);
  }

  /* get the total size of the cache: */
  cache_size = 1 << sun4->tme_sun4_cache_size_log2;

  /* turn this address into its cache line index: */
  cache_line_index = (address & (cache_size - 1)) >> sun4->tme_sun4_cache_size_line_log2;

  /* flush cache lines: */
  do {

    /* if this cache line tag matches what we're supposed to flush: */
    if (((sun4->tme_sun44c_cache_tags[cache_line_index]
	  ^ cache_tag_value)
	 & cache_tag_mask) == 0) {

      /* flush this cache line: */
      _tme_sun44c_cache_line_flush(sun4, cache_line_index);

      /* invalidate this cache line: */
      sun4->tme_sun44c_cache_tags[cache_line_index] &= ~TME_SUN44C_CACHETAG_VALID;
    }

    /* loop: */
    cache_line_index++;
  } while (--cache_line_count > 0);
}

/* this creates a sun4/4c cache: */
void
_tme_sun44c_cache_new(struct tme_sun4 *sun4)
{
  tme_uint32_t cache_size;
  tme_uint32_t cache_size_line;
  tme_uint32_t cache_size_log2;
  tme_uint32_t cache_size_line_log2;
  int cache_writeback;

  /* assume that if this is a sun4c, the cache is write-through.
     otherwise, assume the cache is write-back: */
  cache_writeback = !TME_SUN4_IS_SUN4C(sun4);

  /* SS2, code name "Calvin": */
  if (TME_SUN4_IS_MODEL(sun4, TME_SUN_IDPROM_TYPE_CODE_CALVIN)) {
    cache_size = 64 * 1024;
    cache_size_line = 32;
  }
  else {
    abort();
  }

  /* get the log2 of the cache sizes: */
  assert (cache_size >= cache_size_line && cache_size_line > sizeof(tme_uint32_t));
  assert ((cache_size & (cache_size - 1)) == 0);
  assert ((cache_size_line & (cache_size_line - 1)) == 0);
  for (cache_size_log2 = 0; (tme_uint32_t) (1 << cache_size_log2) < cache_size; cache_size_log2++);
  for (cache_size_line_log2 = 0; (tme_uint32_t) (1 << cache_size_line_log2) < cache_size_line; cache_size_line_log2++);

  /* create the cache: */
  sun4->tme_sun4_cache_size_log2 = cache_size_log2;
  sun4->tme_sun4_cache_size_line_log2 = cache_size_line_log2;
  sun4->tme_sun4_cache_writeback = cache_writeback;  
  sun4->tme_sun4_cache_data = (tme_uint8_t *) tme_new(tme_uint32_t, (cache_size / sizeof(tme_uint32_t)));
  sun4->tme_sun44c_cache_tags = tme_new(tme_uint32_t, (cache_size / cache_size_line));
  tme_rwlock_init(&sun4->tme_sun4_cache_rwlock);
  sun4->tme_sun4_cache_tlb_internal.tme_bus_tlb_token = &sun4->tme_sun4_cache_tlb_internal_token;
  tme_token_init(sun4->tme_sun4_cache_tlb_internal.tme_bus_tlb_token);
}
