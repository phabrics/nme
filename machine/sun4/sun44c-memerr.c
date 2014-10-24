/* $Id: sun44c-memerr.c,v 1.3 2010/06/05 19:29:11 fredette Exp $ */

/* machine/sun4/sun44c-memerr.c - implementation of Sun 4/4c memory error emulation: */

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
_TME_RCSID("$Id: sun44c-memerr.c,v 1.3 2010/06/05 19:29:11 fredette Exp $");

/* includes: */
#include "sun4-impl.h"

/* this calls out a memory error interrupt change: */
static void
_tme_sun44c_memerr_callout(struct tme_sun4 *sun4)
{
  unsigned int int_asserted;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* on a sun4c, asynchronous memory errors can't really be tested, so
     we never need to assert the memory error interrupt: */
  if (TME_SUN4_IS_SUN4C(sun4)) {
    return;
  }

  /* see if the memory error interrupt should be asserted: */
  int_asserted
    = ((sun4->tme_sun44c_memerr_csr[0]
	& (TME_SUN4_MEMERR_X_INT_ACTIVE
	   | TME_SUN4_MEMERR_X_ENABLE_INT))
       == (TME_SUN4_MEMERR_X_INT_ACTIVE
	   | TME_SUN4_MEMERR_X_ENABLE_INT));

  /* if we need to call out an interrupt change: */
  if (!int_asserted != !sun4->tme_sun4_memerr_int_asserted) {

    /* get our bus connection: */
    conn_bus = sun4->tme_sun4_buses[TME_SUN4_32_CONN_REG_MEMERR];
	
    /* call out the bus interrupt signal edge: */
    rc = (*conn_bus->tme_bus_signal)
      (conn_bus,
       TME_BUS_SIGNAL_INT(TME_SPARC_IPL_NMI)
       | (int_asserted
	  ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	  : TME_BUS_SIGNAL_LEVEL_NEGATED));

    /* if this callout was successful, note the new state of the
       interrupt signal: */
    if (rc == TME_OK) {
      sun4->tme_sun4_memerr_int_asserted = int_asserted;
    }

    /* otherwise, abort: */
    else {
      abort();
    }
  }
}

/* this updates bad memory: */
void
_tme_sun44c_memerr_update(struct tme_sun4 *sun4,
			  tme_uint32_t pte,
			  const tme_shared tme_uint8_t *memory,
			  unsigned int cycle_size)
{
  unsigned int address_i;
  unsigned int address_i_free;
  int which_csr;
  int memory_bad;
  int visible_before;

  /* on the SS2, code name "Calvin", the second 64MB of memory is on
     an expansion board, which uses the second memory error register: */
  which_csr = (TME_SUN4_IS_MODEL(sun4, TME_SUN_IDPROM_TYPE_CODE_CALVIN)
	       && (pte & TME_SUN4C_PTE_PGFRAME) >= ((64 * 1024 * 1024) / TME_SUN4C_PAGE_SIZE));

  /* if the memory is being written with bad parity: */
  memory_bad = (sun4->tme_sun44c_memerr_csr[which_csr] & TME_SUN44C_MEMERR_PAR_TEST) != 0;
  
  /* see if the memory error registers are visible before: */
  visible_before = TME_SUN44C_MEMERR_VISIBLE(sun4);

  /* this is obviously a poor implementation, but this is not used
     during normal operation: */

  /* loop over all of the written memory: */
  for (; cycle_size > 0; memory++, cycle_size--) {

    /* search the bad memory list for this memory, and for a free
       entry in the list: */
    address_i = 0;
    address_i_free = TME_ARRAY_ELS(sun4->tme_sun4_memerr_bad_memory);
    do {
      if (sun4->tme_sun4_memerr_bad_memory[address_i] == NULL) {
	address_i_free = address_i;
      }
      else if (sun4->tme_sun4_memerr_bad_memory[address_i] == memory) {
	address_i_free = address_i;
	break;
      }
    } while (++address_i < TME_ARRAY_ELS(sun4->tme_sun4_memerr_bad_memory));

    /* if the memory is being written with bad parity: */
    if (memory_bad) {

      /* if the bad memory list is full: */
      if (address_i_free == TME_ARRAY_ELS(sun4->tme_sun4_memerr_bad_memory)) {
	abort();
      }

      /* if this bad memory isn't already on the list: */
      if (sun4->tme_sun4_memerr_bad_memory[address_i_free] == NULL) {

	/* this memory is now bad: */
	sun4->tme_sun4_memerr_bad_memory[address_i_free] = memory;
	sun4->tme_sun4_memerr_bad_memory_count++;
      }
    }

    /* otherwise, the memory is being written with good parity: */
    else {

      /* if this memory was previously written with bad parity: */
      if (address_i_free < TME_ARRAY_ELS(sun4->tme_sun4_memerr_bad_memory)
	  && sun4->tme_sun4_memerr_bad_memory[address_i] == memory) {

	/* this memory is good again: */
	sun4->tme_sun4_memerr_bad_memory[address_i] = NULL;
	sun4->tme_sun4_memerr_bad_memory_count--;
      }
    }
  }

  /* if the memory error registers were visible before, and are no
     longer visible: */
  if (visible_before
      && !TME_SUN44C_MEMERR_VISIBLE(sun4)) {

    /* unless the cache is visible, update the TLB fill function: */
    if (!sun4->tme_sun4_cache_visible) {
      sun4->tme_sun4_tlb_fill = _tme_sun44c_tlb_fill_mmu;
    }
  }
}

/* this checks for bad memory during a read: */
int
_tme_sun44c_memerr_check(const struct tme_bus_connection *conn_bus_init,
			 tme_uint32_t address,
			 tme_uint32_t pte,
			 const tme_shared tme_uint8_t *memory,
			 unsigned int cycle_size)
{
  struct tme_sun4 *sun4;
  unsigned int address_i;
  tme_uint32_t csr;
  int write_csr;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* this is obviously a poor implementation, but this is not used
     during normal operation: */

  /* loop over all of the bad memory: */
  csr = 0;
  for (; cycle_size > 0; address++, memory++, cycle_size--) {

    /* see if this byte is bad: */
    address_i = 0;
    do {
      if (sun4->tme_sun4_memerr_bad_memory[address_i] == memory) {
	csr
	  |= (TME_SUN4_IS_SUN4C(sun4)
	      ? TME_SUN44C_MEMERR_PAR_ERR_BL0 << (address % sizeof(tme_uint32_t))
	      : TME_SUN44C_MEMERR_PAR_ERR_BL3 >> (address % sizeof(tme_uint32_t)));
	break;
      }
    } while (++address_i < TME_ARRAY_ELS(sun4->tme_sun4_memerr_bad_memory));
  }

  /* if no bad memory was accessed, return now: */
  if (csr == 0) {
    return (FALSE);
  }

  /* if this is a sun4c: */
  if (TME_SUN4_IS_SUN4C(sun4)) {

    /* on the SS2, code name "Calvin", the second 64MB of memory is on
       an expansion board, which uses the second memory error
       register: */
    write_csr = (TME_SUN4_IS_MODEL(sun4, TME_SUN_IDPROM_TYPE_CODE_CALVIN)
		 && (pte & TME_SUN4C_PTE_PGFRAME) >= ((64 * 1024 * 1024) / TME_SUN4C_PAGE_SIZE));

    sun4->tme_sun44c_memerr_csr[write_csr]
      |= (csr
	  | TME_SUN4C_MEMERR_PAR_ERROR
	  | ((sun4->tme_sun44c_memerr_csr[write_csr] & TME_SUN4C_MEMERR_PAR_ERROR)
	     ? TME_SUN4C_MEMERR_PAR_MULTI
	     : 0));
  }

  /* otherwise, this is a sun4: */
  else {
    abort();
  }

  /* call out an interrupt: */
  _tme_sun44c_memerr_callout(sun4);

  /* a memory error has happened: */
  return (TRUE);
}

/* the bus cycle handler for the memory error register: */
int
_tme_sun44c_memerr_cycle_control(void *_sun4, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun4 *sun4;
  tme_uint32_t memerr_reg[2][TME_SUN44C_MEMERR_SIZ_REG / sizeof(tme_uint32_t)];
  int write_csr, unlatch;
  int write_parctl;
  tme_uint32_t csr_old;
  tme_uint32_t csr_new;
  tme_uint32_t csr_ro;
  int visible_before;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) _sun4;

  /* start filling the memory error register(s): */
  memerr_reg[0][TME_SUN44C_MEMERR_REG_CSR / sizeof(tme_uint32_t)]
    = tme_htobe_u32(sun4->tme_sun44c_memerr_csr[0]);

  /* assume this cycle won't write or unlatch a memory error register: */
  write_csr = -1;
  unlatch = -1;
  write_parctl = -1;

  /* we only tolerate aligned 32-bit accesses: */
  if (!((cycle_init->tme_bus_cycle_address % sizeof(tme_uint32_t)) == 0
	&& cycle_init->tme_bus_cycle_size == sizeof(tme_uint32_t))) {
    abort();
  }

  /* if this is a sun4c: */
  if (TME_SUN4_IS_SUN4C(sun4)) {

    /* finish filling the memory error registers: */
    memerr_reg[0][TME_SUN4C_MEMERR_REG_PARCTL / sizeof(tme_uint32_t)]
      = tme_htobe_u32(sun4->tme_sun4c_memerr_parctl[0]);
    memerr_reg[1][TME_SUN44C_MEMERR_REG_CSR / sizeof(tme_uint32_t)]
      = tme_htobe_u32(sun4->tme_sun44c_memerr_csr[1]);
    memerr_reg[1][TME_SUN4C_MEMERR_REG_PARCTL / sizeof(tme_uint32_t)]
      = tme_htobe_u32(sun4->tme_sun4c_memerr_parctl[1]);

    assert ((cycle_init->tme_bus_cycle_address 
	     + cycle_init->tme_bus_cycle_size
	     - 1) < sizeof(memerr_reg));

    /* if this access is to a parity control register: */
    if (cycle_init->tme_bus_cycle_address & TME_SUN4C_MEMERR_REG_PARCTL) {

      /* if this is a write: */
      if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {
	write_parctl = (cycle_init->tme_bus_cycle_address / TME_SUN44C_MEMERR_SIZ_REG);
      }
    }

    /* otherwise, this access is to a CSR: */
    else {

      /* "The information bits in the memory error register are
         cleared by reading it." */

      /* any write writes either of the CSRs, and any read reads one
         of the CSRs: */
      if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {
	unlatch = (cycle_init->tme_bus_cycle_address / TME_SUN44C_MEMERR_SIZ_REG);
      }
      else {
	write_csr = (cycle_init->tme_bus_cycle_address / TME_SUN44C_MEMERR_SIZ_REG);
      }
    }
  }

  /* otherwise, this is a sun4: */
  else {

    /* finish filling the memory error register: */
    memerr_reg[0][TME_SUN4_MEMERR_REG_VADDR / sizeof(tme_uint32_t)]
      = tme_htobe_u32(sun4->tme_sun4_memerr_vaddr);

    assert ((cycle_init->tme_bus_cycle_address 
	     + cycle_init->tme_bus_cycle_size
	     - 1) < TME_SUN44C_MEMERR_SIZ_REG);

    /* if this is a write: */
    if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

      /* "The interrupt is held pending and the error information in
	 the memory error register is latched (frozen) until it is
	 cleared (unfrozen) by a write to bits <31..24> of the memory
	 error address register." */

      /* see if this writes the vaddr or the CSR: */
      if (cycle_init->tme_bus_cycle_address & TME_SUN4_MEMERR_REG_VADDR) {
	unlatch = 0;
      }
      else {
	write_csr = 0;
      }
    }
  }

  /* do the transfer: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (tme_uint8_t *) memerr_reg,
			    sizeof(memerr_reg) - 1);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* if the sun4c parity control register has been written: */
    if (write_parctl >= 0) {
      sun4->tme_sun4c_memerr_parctl[write_parctl]
	= tme_betoh_u32(memerr_reg[write_parctl][TME_SUN4C_MEMERR_REG_PARCTL / sizeof(tme_uint32_t)]);
    }

    /* if the sun4 vaddr register has been written: */
    if (TME_SUN4_IS_SUN4(sun4)
	&& unlatch) {
      sun4->tme_sun4c_memerr_parctl[0]
	= tme_betoh_u32(memerr_reg[0][TME_SUN4_MEMERR_REG_VADDR / sizeof(tme_uint32_t)]);
    }
  }

  /* assume that there are no CSR changes: */
  csr_old = csr_new = 0;
  csr_ro = ~(TME_SUN44C_MEMERR_PAR_TEST
	     | TME_SUN44C_MEMERR_PAR_ENABLE
	     | (TME_SUN4_IS_SUN4C(sun4)
		? 0
		: TME_SUN4_MEMERR_X_ENABLE_INT));

  /* if a CSR register has been written: */
  if (write_csr >= 0) {

    /* get the new and old CSR values, preserving the read-only bits: */
    csr_old = sun4->tme_sun44c_memerr_csr[write_csr];
    csr_new = tme_betoh_u32(memerr_reg[write_csr][TME_SUN44C_MEMERR_REG_CSR / sizeof(tme_uint32_t)]);
    csr_new = ((csr_old & csr_ro)
	       | (csr_new & ~csr_ro));
  }

  /* otherwise, if a CSR register has been unlatched: */
  else if (unlatch >= 0) {
    
    /* get the old and new CSR values, clearing the read-only bits: */
    csr_old = sun4->tme_sun44c_memerr_csr[unlatch];
    csr_new &= ~csr_ro;
    write_csr = unlatch;
  }

  /* if a CSR register has changed: */
  if (csr_new != csr_old) {

    /* see if the memory error registers were visible before: */
    visible_before = TME_SUN44C_MEMERR_VISIBLE(sun4);

    /* set the new CSR value and call out an interrupt change: */
    sun4->tme_sun44c_memerr_csr[write_csr] = csr_new;
    _tme_sun44c_memerr_callout(sun4);

    /* if the write-inverse-parity testing feature is being enabled or
       disabled: */
    if ((csr_new ^ csr_old) & TME_SUN44C_MEMERR_PAR_TEST) {

      /* if this feature is being enabled: */
      if (csr_new & TME_SUN44C_MEMERR_PAR_TEST) {

	/* if the memory error registers weren't visible before: */
	if (!visible_before) {

	  /* the memory error registers are now visible: */
	  assert (sun4->tme_sun4_memerr_bad_memory_count == 0);

	  /* unless the cache is visible, update the TLB fill function: */
	  if (!sun4->tme_sun4_cache_visible) {
	    sun4->tme_sun4_tlb_fill = _tme_sun44c_tlb_fill_memerr;
	  }

	  /* invalidate all TLBs: */
	  tme_sun_mmu_tlbs_invalidate(sun4->tme_sun44c_mmu);
	}
      }

      /* otherwise, this feature is being disabled: */
      else {

	assert (visible_before);

	/* if the memory error registers are no longer visible: */
	if (!TME_SUN44C_MEMERR_VISIBLE(sun4)) {

	  /* unless the cache is visible, update the TLB fill function: */
	  if (!sun4->tme_sun4_cache_visible) {
	    sun4->tme_sun4_tlb_fill = _tme_sun44c_tlb_fill_mmu;
	  }

	  /* invalidate all TLBs: */
	  tme_sun_mmu_tlbs_invalidate(sun4->tme_sun44c_mmu);
	}
      }	    
    }
  }
    
  return (TME_OK);
}

/* the bus cycle handler for memory error testing: */
int
_tme_sun44c_memerr_cycle_bus(void *_conn_bus_init,
			     struct tme_bus_cycle *cycle_init)
{
  const struct tme_bus_connection *conn_bus_init;
  struct tme_sun4 *sun4;
  struct tme_bus_tlb *tlb;
  tme_uint32_t address;
  unsigned int cycle_size;
  union {
    tme_uint8_t memory_buffer_8s[sizeof(tme_uint32_t) / sizeof(tme_uint8_t)];
    tme_uint16_t memory_buffer_16s[sizeof(tme_uint32_t) / sizeof(tme_uint16_t)];
    tme_uint32_t memory_buffer_32s[sizeof(tme_uint32_t) / sizeof(tme_uint32_t)];
  } memory_buffer;
  struct tme_sun_mmu_pte pte_mmu;
  const tme_shared tme_uint8_t *memory_data_read;
  tme_shared tme_uint8_t *memory_data_write;
  int rc;

  /* recover our initiator's bus connection and sun4: */
  conn_bus_init = (struct tme_bus_connection *) _conn_bus_init;
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the address and size for this cycle: */
  address = cycle_init->tme_bus_cycle_address;
  cycle_size = cycle_init->tme_bus_cycle_size;

  /* recover the TLB entry for this cycle: */
  tlb = sun4->tme_sun4_memtest_tlb;
  assert (tlb != NULL);

  /* busy the TLB entry that triggered this cycle: */
  tme_bus_tlb_busy(tlb);

  /* if the TLB entry is invalid, return now: */
  if (tme_bus_tlb_is_invalid(tlb)) {
    
    /* unbusy the TLB entry and return now: */
    tme_bus_tlb_unbusy(tlb);
    return (EBADF);
  }

  /* get the PTE for this address and context from the MMU: */
  rc = tme_sun_mmu_pte_get(sun4->tme_sun44c_mmu, 
			   TME_SUN44C_BUS_MMU_CONTEXT(sun4, conn_bus_init),
			   address,
			   &pte_mmu);
  assert (rc == TME_OK);

  /* if this cycle is a read: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {

    /* get a pointer to the data to read: */
    memory_data_read
      = (sun4->tme_sun4_memerr_tlb_emulator_off_read
	 + address);

    /* dispatch on the cycle size, to transfer the data from memory
       into the memory buffer: */
    switch (cycle_size) {
    default:
      assert (FALSE);
      /* FALLTHROUGH */
    case sizeof(tme_uint8_t):
      memory_buffer.memory_buffer_8s[0]
	= tme_memory_bus_read8(memory_data_read,
			       tlb->tme_bus_tlb_rwlock,
			       sizeof(tme_uint8_t),
			       sizeof(tme_uint32_t));
      break;
    case sizeof(tme_uint16_t):
      memory_buffer.memory_buffer_16s[0]
	= tme_memory_bus_read16((const tme_shared tme_uint16_t *) memory_data_read,
				tlb->tme_bus_tlb_rwlock,
				sizeof(tme_uint16_t),
				sizeof(tme_uint32_t));
      break;
    case sizeof(tme_uint32_t):
      memory_buffer.memory_buffer_32s[0]
	= tme_memory_bus_read32((const tme_shared tme_uint32_t *) memory_data_read,
				tlb->tme_bus_tlb_rwlock,
				sizeof(tme_uint32_t),
				sizeof(tme_uint32_t));
      break;
    }

    /* run the bus cycle against the memory buffer: */
    tme_bus_cycle_xfer_memory(cycle_init,
			      &memory_buffer.memory_buffer_8s[0] - address, 
			      address + cycle_size - 1);
    assert (cycle_init->tme_bus_cycle_size == cycle_size);

    /* check for memory errors on this cache line fill: */
    rc = (_tme_sun44c_memerr_check(conn_bus_init,
				   address,
				   pte_mmu.tme_sun_mmu_pte_raw,
				   memory_data_read,
				   cycle_size)
	  ? EIO
	  : TME_OK);
  }

  /* otherwise, this cycle must be a write: */
  else {
    assert (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE);

    /* run the bus cycle against the memory buffer: */
    tme_bus_cycle_xfer_memory(cycle_init,
			      &memory_buffer.memory_buffer_8s[0] - address, 
			      address + cycle_size - 1);
    assert (cycle_init->tme_bus_cycle_size == cycle_size);

    /* get a pointer to the data to write: */
    memory_data_write
      = (sun4->tme_sun4_memerr_tlb_emulator_off_write
	 + address);

    /* dispatch on the cycle size, to transfer the data from the
       memory buffer to memory: */
    switch (cycle_size) {
    default:
      assert (FALSE);
      /* FALLTHROUGH */
    case sizeof(tme_uint8_t):
      tme_memory_bus_write8(memory_data_write,
			    memory_buffer.memory_buffer_8s[0],
			    tlb->tme_bus_tlb_rwlock,
			    sizeof(tme_uint8_t),
			    sizeof(tme_uint32_t));
      break;
    case sizeof(tme_uint16_t):
      tme_memory_bus_write16((tme_shared tme_uint16_t *) memory_data_write,
			     memory_buffer.memory_buffer_16s[0],
			     tlb->tme_bus_tlb_rwlock,
			     sizeof(tme_uint16_t),
			     sizeof(tme_uint32_t));
      break;
    case sizeof(tme_uint32_t):
      tme_memory_bus_write32((tme_shared tme_uint32_t *) memory_data_write,
			     memory_buffer.memory_buffer_32s[0],
			     tlb->tme_bus_tlb_rwlock,
			     sizeof(tme_uint32_t),
			     sizeof(tme_uint32_t));
      break;
    }

    /* add any memory errors caused by this write: */
    _tme_sun44c_memerr_update(sun4,
			      pte_mmu.tme_sun_mmu_pte_raw,
			      memory_data_write,
			      cycle_size);
    rc = TME_OK;
  }
      
  /* unbusy the TLB entry and invalidate it.  this hurts performance,
     since it keeps TLB entries valid only for one cycle at a time,
     but it's simple and handles the problem of memory error testing
     in the presence of virtual address aliases (addresses that have
     never been written to can suddenly have read errors): */
  tme_bus_tlb_unbusy(tlb);
  tme_token_invalidate(tlb->tme_bus_tlb_token);
  sun4->tme_sun4_memtest_tlb = NULL;

  return (rc);
}

/* this fills TLBs when memory error testing is visible: */
int
_tme_sun44c_tlb_fill_memerr(const struct tme_bus_connection *conn_bus_init,
			    struct tme_bus_tlb *tlb,
			    tme_uint32_t *_asi_mask,
			    tme_uint32_t address,
			    unsigned int cycle_type)
{
  struct tme_sun4 *sun4;
  tme_uint32_t asi_mask;
  struct tme_sun_mmu_pte pte_mmu;
  int rc;

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
#ifndef NDEBUG
  sun4->tme_sun4_memerr_tlb_emulator_off_read = TME_EMULATOR_OFF_UNDEF;
  sun4->tme_sun4_memerr_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
#endif /* !NDEBUG */

  /* memory error testing must be active: */
  assert (TME_SUN44C_MEMERR_VISIBLE(sun4));

  /* fill this TLB entry from the MMU: */
  rc = _tme_sun44c_tlb_fill_mmu(conn_bus_init,
				tlb,
				_asi_mask,
				address,
				cycle_type);
  assert (rc == TME_OK);

  /* get the PTE for this address and context from the MMU: */
  rc = tme_sun_mmu_pte_get(sun4->tme_sun44c_mmu, 
			   TME_SUN44C_BUS_MMU_CONTEXT(sun4, conn_bus_init),
			   address,
			   &pte_mmu);
  assert (rc == TME_OK);

  /* if this PTE is valid and for onboard memory: */
  if ((pte_mmu.tme_sun_mmu_pte_raw
       & (TME_SUN44C_PTE_VALID
	  | TME_SUN44C_PTE_PGTYPE))
      == (TME_SUN44C_PTE_VALID
	  | 0 /* TME_SUN44C_PGTYPE_OBMEM */)) {

    /* this TLB entry should have no shift: */
    if (tlb->tme_bus_tlb_addr_shift != 0) {
      abort();
    }

    /* if this is a read: */
    if (cycle_type == TME_BUS_CYCLE_READ) {

      /* invalidate this TLB entry for writes: */
      tlb->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;

      /* if this TLB entry allows fast reading, and there are bad
	 memory addresses: */
      if (tlb->tme_bus_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF
	  && sun4->tme_sun4_memerr_bad_memory_count > 0) {

	/* we will handle this read: */
	sun4->tme_sun4_memerr_tlb_emulator_off_read = tlb->tme_bus_tlb_emulator_off_read;
	tlb->tme_bus_tlb_emulator_off_read = TME_EMULATOR_OFF_UNDEF;
	tlb->tme_bus_tlb_cycle_private = (void *) conn_bus_init;
	tlb->tme_bus_tlb_cycle = _tme_sun44c_memerr_cycle_bus;
      }
    }

    /* otherwise, this must be a write: */
    else {
      if (cycle_type != TME_BUS_CYCLE_WRITE) {
	abort();
      }

      /* invalidate this TLB entry for reads: */
      tlb->tme_bus_tlb_emulator_off_read = TME_EMULATOR_OFF_UNDEF;

      /* if this TLB entry allows fast writing: */
      if (tlb->tme_bus_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF) {

	/* we will handle this write: */
	sun4->tme_sun4_memerr_tlb_emulator_off_write = tlb->tme_bus_tlb_emulator_off_write;
	tlb->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
	tlb->tme_bus_tlb_cycle_private = (void *) conn_bus_init;
	tlb->tme_bus_tlb_cycle = _tme_sun44c_memerr_cycle_bus;
      }
    }

    /* finish invalidating this TLB entry for the other cycle type: */
    assert (tlb->tme_bus_tlb_cycles_ok & cycle_type);
    tlb->tme_bus_tlb_cycles_ok = cycle_type;
    tlb->tme_bus_tlb_addr_offset = 0;
  }

  /* remember this TLB entry: */
  sun4->tme_sun4_memtest_tlb = tlb;

  /* return success: */
  return (TME_OK);
}
