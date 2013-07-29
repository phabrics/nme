/* $Id: sun3-control.c,v 1.3 2009/08/30 14:20:59 fredette Exp $ */

/* machine/sun3/sun3-control.c - implementation of Sun 3 emulation control space: */

/*
 * Copyright (c) 2003, 2004 Matt Fredette
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
_TME_RCSID("$Id: sun3-control.c,v 1.3 2009/08/30 14:20:59 fredette Exp $");

/* includes: */
#include "sun3-impl.h"

/* the bus cycle handler for function code three space: */
int
_tme_sun3_control_cycle_handler(void *_sun3, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun3 *sun3;
  struct tme_bus_cycle cycle_resp;
  tme_uint32_t address, reg;
  tme_uint8_t port_data[sizeof(tme_uint32_t)];
  tme_uint8_t *port_data8;
  tme_uint32_t *port_data32;
  tme_uint32_t value32;
  tme_uint8_t enable_old, enable_new;
  int rc;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* get the address, convert it into a register number, then mask
     the address: */
  address = cycle_init->tme_bus_cycle_address;
  reg = TME_SUN3_CONTROL_REG(address);
  if (reg != TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_IDPROM)) {
    address &= TME_SUN3_CONTROL_MASK_ADDRESS;
  }

  /* dispatch on the register, to get bytes for our port: */
  port_data8 = &port_data[0];
  port_data32 = (tme_uint32_t *) &port_data[0];
  switch (reg) {
    
  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_IDPROM):
    memcpy(port_data, 
	   &sun3->tme_sun3_idprom_contents[(address & (TME_SUN_IDPROM_SIZE - 1))],
	   TME_MIN(sizeof(port_data),
		   TME_SUN_IDPROM_SIZE - (address & (TME_SUN_IDPROM_SIZE - 1))));
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_PGMAP):
    rc = _tme_sun3_mmu_pte_get(sun3, address, &value32);
    assert(rc == TME_OK);
    *port_data32 = tme_htobe_u32(value32);
    break;
      
  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_SEGMAP):
    *port_data8 = tme_sun_mmu_segmap_get(sun3->tme_sun3_mmu, sun3->tme_sun3_context, address);
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_CONTEXT):
    *port_data8 = sun3->tme_sun3_context;
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_ENABLE):
    *port_data8 = sun3->tme_sun3_enable;
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_UDVMA):
    *port_data8 = sun3->tme_sun3_udvma;
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_BUSERR):
    *port_data8 = sun3->tme_sun3_buserr;
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_DIAG):
    *port_data8 = sun3->tme_sun3_diag;
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_VAC_TAGS):
    abort();
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_VAC_DATA):
    abort();
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_VAC_FLUSH):
    abort();
    break;

  case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_COPY):
    abort();
    break;

  default:
    abort();
  }
     
  /* run the bus cycle: */
  cycle_resp.tme_bus_cycle_buffer = &port_data[0];
  cycle_resp.tme_bus_cycle_buffer_increment = 1;
  cycle_resp.tme_bus_cycle_lane_routing = cycle_init->tme_bus_cycle_lane_routing;
  cycle_resp.tme_bus_cycle_address = 0;
  cycle_resp.tme_bus_cycle_type = (cycle_init->tme_bus_cycle_type
				   ^ (TME_BUS_CYCLE_WRITE
				      | TME_BUS_CYCLE_READ));
  cycle_resp.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS32_LOG2);
  tme_bus_cycle_xfer(cycle_init, &cycle_resp);

  /* whenever the bus error register is read or written, it is
     cleared: */
  if (reg == TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_BUSERR)) {
    sun3->tme_sun3_buserr = 0;
  }

  /* these registers only need action taken when they're written: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* dispatch on the register: */
    switch (reg) {

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_PGMAP):
      value32 = tme_betoh_u32(*port_data32);
      rc = _tme_sun3_mmu_pte_set(sun3, address, value32);
      assert (rc == TME_OK);
      break;
      
    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_SEGMAP):
      tme_sun_mmu_segmap_set(sun3->tme_sun3_mmu, sun3->tme_sun3_context, address, *port_data8 % TME_SUN3_PMEGS);
      break;
      
    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_CONTEXT):
      sun3->tme_sun3_context = *port_data8;
      _tme_sun3_mmu_context_set(sun3);
      break;

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_ENABLE):
      enable_old = sun3->tme_sun3_enable;
      enable_new = (enable_old & TME_SUN3_ENA_DIAG) | (*port_data8 & ~TME_SUN3_ENA_DIAG);
      sun3->tme_sun3_enable = enable_new;

      /* if we're changing to or from boot state, make a pseudo-context change: */
      if ((enable_old ^ enable_new) & TME_SUN3_ENA_NOTBOOT) {
	_tme_sun3_mmu_context_set(sun3);
      }

      /* if we're changing the m6888x enable bit, call out the change: */
      if ((enable_old ^ enable_new) & TME_SUN3_ENA_FPP) {
	rc = ((*sun3->tme_sun3_m68k->tme_m68k_bus_m6888x_enable)
	      (sun3->tme_sun3_m68k, (enable_new & TME_SUN3_ENA_FPP)));
	assert (rc == TME_OK);
      }

      /* if we're enabling or disabling system DVMA, call out the change: */
      if ((enable_old ^ enable_new) & TME_SUN3_ENA_SDVMA) {
	_tme_sun3_mmu_sdvma_change(sun3);
      }

      /* certain things can't be enabled: */
      if (enable_new
	  & (TME_SUN3_ENA_COPY
	     | TME_SUN3_ENA_CACHE)) {
	abort();
      }
      break;

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_UDVMA):
      if (sun3->tme_sun3_enable & TME_SUN3_ENA_NOTBOOT) {
	abort();
      }
      sun3->tme_sun3_udvma = *port_data8;
      break;

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_DIAG):
      sun3->tme_sun3_diag = *port_data8;
      /* TBD */
      break;

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_VAC_TAGS):
      abort();
      break;

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_VAC_DATA):
      abort();
      break;

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_VAC_FLUSH):
      abort();
      break;

    case TME_SUN3_CONTROL_REG(TME_SUN3_CONTROL_COPY):
      abort();
      break;
    }
  }

  return (TME_OK);
}

/* the bus cycle handler for the interrupt register: */
int
_tme_sun3_intreg_cycle_handler(void *_sun3, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun3 *sun3;
  int rc;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* do the transfer: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    &sun3->tme_sun3_ints,
			    sizeof(sun3->tme_sun3_ints) - 1);

  /* if the interrupt register has been written: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {
    rc = _tme_sun3_ipl_check(sun3);
    assert(rc == TME_OK);
  }

  return (TME_OK);
}

/* this calls out a memory error interrupt change: */
static void
_tme_sun3_memerr_callout(struct tme_sun3 *sun3)
{
  unsigned int int_asserted;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* see if the memory error interrupt should be asserted: */
  int_asserted
    = ((sun3->tme_sun3_memerr_csr
	& (TME_SUN3_MEMERR_X_INT_ACTIVE
	   | TME_SUN3_MEMERR_X_ENABLE_INT))
       == (TME_SUN3_MEMERR_X_INT_ACTIVE
	   | TME_SUN3_MEMERR_X_ENABLE_INT));

  /* if we need to call out an interrupt change: */
  if (!int_asserted != !sun3->tme_sun3_memerr_int_asserted) {

    /* get our bus connection: */
    conn_bus = sun3->tme_sun3_memerr_bus;
	
    /* call out the bus interrupt signal edge: */
    rc = (*conn_bus->tme_bus_signal)
      (conn_bus,
       TME_BUS_SIGNAL_INT_UNSPEC
       | (int_asserted
	  ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	  : TME_BUS_SIGNAL_LEVEL_NEGATED));

    /* if this callout was successful, note the new state of the
       interrupt signal: */
    if (rc == TME_OK) {
      sun3->tme_sun3_memerr_int_asserted = int_asserted;
    }

    /* otherwise, abort: */
    else {
      abort();
    }
  }
}

/* the bus cycle handler for the memory error register: */
int
_tme_sun3_memerr_cycle_handler(void *_sun3, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun3 *sun3;
  tme_uint8_t memerr_reg[TME_SUN3_MEMERR_SIZ_REG];
  int write_csr, unlatch;
  tme_uint8_t csr_old, csr_new;

  /* the read-only bits: */
#define TME_SUN3_MEMERR_RO (TME_SUN3_MEMERR_X_INT_ACTIVE \
			    | TME_SUN3_MEMERR_PAR_ERR_BL3 \
			    | TME_SUN3_MEMERR_PAR_ERR_BL2 \
			    | TME_SUN3_MEMERR_PAR_ERR_BL1 \
			    | TME_SUN3_MEMERR_PAR_ERR_BL0)

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* fill the memory error register: */
  memerr_reg[TME_SUN3_MEMERR_REG_CSR] = sun3->tme_sun3_memerr_csr;
  *((tme_uint32_t *) &memerr_reg[TME_SUN3_MEMERR_SIZ_VADDR]) = tme_htobe_u32(sun3->tme_sun3_memerr_vaddr);

  /* assume this cycle won't write the CSR or unlatch the register: */
  write_csr = FALSE;
  unlatch = FALSE;

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* see if this writes the CSR: */
    write_csr = TME_RANGES_OVERLAP(cycle_init->tme_bus_cycle_address,
				   (cycle_init->tme_bus_cycle_address
				    + cycle_init->tme_bus_cycle_size
				    - 1),
				   TME_SUN3_MEMERR_REG_CSR,
				   (TME_SUN3_MEMERR_REG_CSR
				    + TME_SUN3_MEMERR_SIZ_CSR
				    - 1));

    /* see if this write unlatches the register: */
    unlatch = TME_RANGES_OVERLAP(cycle_init->tme_bus_cycle_address,
				 (cycle_init->tme_bus_cycle_address
				  + cycle_init->tme_bus_cycle_size
				  - 1),
				 TME_SUN3_MEMERR_REG_VADDR,
				 TME_SUN3_MEMERR_REG_VADDR);
  }

  /* do the transfer: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    memerr_reg,
			    sizeof(memerr_reg) - 1);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* get the old CSR value, and assume that the new CSR value is the
       same: */
    csr_old = sun3->tme_sun3_memerr_csr;
    csr_new = csr_old;

    /* if the CSR register has been written: */
    if (write_csr) {

      /* get the new CSR value, but preserve the read-only bits: */
      csr_new = ((csr_new & TME_SUN3_MEMERR_RO)
		 | (memerr_reg[TME_SUN3_MEMERR_REG_CSR] & ~TME_SUN3_MEMERR_RO));
    }

    /* if the memory error register has been unlatched: */
    if (unlatch) {

      /* clear the memory error: */
      csr_new &= ~TME_SUN3_MEMERR_RO;
      sun3->tme_sun3_memerr_vaddr = 0;
    }

    /* if the CSR register has changed: */
    if (csr_new != csr_old) {

      /* set the new CSR value and call out an interrupt change: */
      sun3->tme_sun3_memerr_csr = csr_new;
      _tme_sun3_memerr_callout(sun3);

      /* if the memory error register is being tested: */
      if (csr_new & TME_SUN3_MEMERR_PAR_TEST) {

	/* if the test is just beginning, invalidate all TLB entries: */
	if (!(csr_old & TME_SUN3_MEMERR_PAR_TEST)) {
	  tme_sun_mmu_tlbs_invalidate(sun3->tme_sun3_mmu);
	}
      }
    }
  }
    
  return (TME_OK);
}

/* the bus cycle handler for the memory error register test: */
int
_tme_sun3_memerr_test_cycle_handler(void *_sun3, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun3 *sun3;
  struct tme_bus_tlb *tlb;
  tme_uint32_t vaddr, cycle_align;
  tme_uint8_t csr;
  int rc;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* get the memory error test TLB entry: */
  tlb = sun3->tme_sun3_memerr_tlb;
  if (tlb == NULL) {
    abort();
  }

  /* recover the virtual address from the TLB entry.  this TLB entry
     should be for normal memory, which means it should have no shift: */
  if (tlb->tme_bus_tlb_addr_shift != 0) {
    abort();
  }
  vaddr = (cycle_init->tme_bus_cycle_address
	   - tlb->tme_bus_tlb_addr_offset);

  /* calculate the number of bytes after the last byte of the cycle
     until the next 32-bit boundary.  this bus cycle must not cross a
     32-bit boundary: */
  cycle_align = ((vaddr & (sizeof(tme_uint32_t) - 1))
		 + cycle_init->tme_bus_cycle_size);
  if (cycle_align > sizeof(tme_uint32_t)) {
    abort();
  }
  cycle_align = sizeof(tme_uint32_t) - cycle_align;
  
  /* make the pending csr value.  this calculates a mask of
     TME_SUN3_MEMERR_PAR_ERR_BLx bits corresponding to the byte lanes
     being read or written in this cycle: */
  csr = (1 << cycle_init->tme_bus_cycle_size) - 1;
  csr <<= cycle_align;

  /* if this cycle is a read: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {

    /* parity checking must be enabled and there must be no memory
       error pending: */
    if ((sun3->tme_sun3_memerr_csr
	 & (TME_SUN3_MEMERR_X_INT_ACTIVE
	    | TME_SUN3_MEMERR_PAR_ENABLE))
	!= TME_SUN3_MEMERR_PAR_ENABLE) {
      abort();
    }

    /* this must be a read of the written address: */
    if ((sun3->tme_sun3_memerr_pending_csr & csr) == 0
	|| ((sun3->tme_sun3_memerr_pending_vaddr ^ vaddr) & -sizeof(tme_uint32_t)) != 0) {
      abort();
    }

    /* do the read: */
    rc = ((*sun3->tme_sun3_memerr_cycle)
	  (sun3->tme_sun3_memerr_cycle_private, 
	   cycle_init));

    /* set the memory error control register: */
    sun3->tme_sun3_memerr_csr
      = ((sun3->tme_sun3_memerr_csr
	  & ~TME_SUN3_MEMERR_RO)
	 | TME_SUN3_MEMERR_X_INT_ACTIVE
	 | (sun3->tme_sun3_memerr_pending_csr & csr));

    /* set the memory error address register: */
    sun3->tme_sun3_memerr_vaddr
      = ((sun3->tme_sun3_context << 28)
	 | vaddr);
    
    /* call out an interrupt: */
    _tme_sun3_memerr_callout(sun3);

    /* invalidate the memory test TLB entry: */
    tme_token_invalidate(tlb->tme_bus_tlb_token);
    sun3->tme_sun3_memerr_tlb = NULL;

    /* tell the CPU that a synchronous event of some kind has happened
       on the bus: */
    return (rc == TME_OK
	    ? TME_BUS_CYCLE_SYNCHRONOUS_EVENT
	    : rc);
  }

  /* otherwise, this must be a write: */
  if (cycle_init->tme_bus_cycle_type != TME_BUS_CYCLE_WRITE) {
    abort();
  }

  /* this must be the first write: */
  if (sun3->tme_sun3_memerr_pending_csr != 0) {
    abort();
  }

  /* remember the pending memory error address and CSR value: */
  sun3->tme_sun3_memerr_pending_csr = csr;
  sun3->tme_sun3_memerr_pending_vaddr = vaddr;

  /* run the cycle normally: */
  return ((*sun3->tme_sun3_memerr_cycle)
	  (sun3->tme_sun3_memerr_cycle_private, 
	   cycle_init));

#undef TME_SUN3_MEMERR_RO
}

#if 1
#include <stdio.h>

/* this dumps out the sun3 state: */
void
tme_sun3_dump(struct tme_sun3 *sun3)
{
  
  /* dump out the page map register: */
  fprintf(stderr, "CONTEXT = 0x%02x\n", sun3->tme_sun3_context);
  fprintf(stderr, "\n");
  fprintf(stderr, "DIAG = 0x%02x\n", sun3->tme_sun3_diag);
  fprintf(stderr, "UDVMA = 0x%02x\n", sun3->tme_sun3_udvma);
  fprintf(stderr, "BUSERR = 0x%04x\n", sun3->tme_sun3_buserr);
  fprintf(stderr, "ENABLE = 0x%02x\n", sun3->tme_sun3_enable);
  fprintf(stderr, "INTS = 0x%02x\n", sun3->tme_sun3_ints);
  fprintf(stderr, "MEMERR = 0x%02x 0x%04x\n", sun3->tme_sun3_memerr_csr, sun3->tme_sun3_memerr_vaddr);
}
#endif /* 1 */
