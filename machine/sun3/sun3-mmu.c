/* $Id: sun3-mmu.c,v 1.6 2009/08/30 14:25:21 fredette Exp $ */

/* machine/sun3/sun3-mmu.c - implementation of Sun 3 MMU emulation: */

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
_TME_RCSID("$Id: sun3-mmu.c,v 1.6 2009/08/30 14:25:21 fredette Exp $");

/* includes: */
#include "sun3-impl.h"

/* macros: */

/* real PTE entry bits: */
#define TME_SUN3_PTE_VALID		(0x80000000)
#define TME_SUN3_PTE_WRITE		(0x40000000)
#define TME_SUN3_PTE_SYSTEM		(0x20000000)
#define TME_SUN3_PTE_NC			(0x10000000)
#define TME_SUN3_PTE_PGTYPE		(0x0C000000)
#define TME_SUN3_PTE_REF		(0x02000000)
#define TME_SUN3_PTE_MOD		(0x01000000)
#define TME_SUN3_PTE_PGFRAME		(0x0007FFFF)

/* real PTE page types: */
#define TME_SUN3_PGTYPE_OBMEM		(0)
#define TME_SUN3_PGTYPE_OBIO		(1)
#define TME_SUN3_PGTYPE_VME_D16		(2)
#define TME_SUN3_PGTYPE_VME_D32		(3)

/* real bus error register bits: */
#define TME_SUN3_BUSERR_WATCHDOG	TME_BIT(0)	/* watchdog or user reset */
					/* bit 1 unused */
#define TME_SUN3_BUSERR_FPAENERR	TME_BIT(2)	/* FPA enable error */
#define TME_SUN3_BUSERR_FPABERR		TME_BIT(3)	/* FPA bus error */
#define TME_SUN3_BUSERR_VMEBUSERR	TME_BIT(4)	/* VME bus error */
#define TME_SUN3_BUSERR_TIMEOUT		TME_BIT(5)	/* timeout error */
#define TME_SUN3_BUSERR_PROTERR		TME_BIT(6)	/* MMU protection error */
#define TME_SUN3_BUSERR_INVALID		TME_BIT(7)	/* MMU page invalid error */

/* real context count: */
#define TME_SUN3_CONTEXT_COUNT		(8)

/* this logs a bus error: */
#ifndef TME_NO_LOG
static void
_tme_sun3_bus_fault_log(struct tme_sun3 *sun3, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle)
{
  tme_bus_addr32_t virtual_address;
  struct tme_sun_mmu_pte pte;
  tme_uint32_t pte_sun3;
  const char *bus_name;
  tme_bus_addr32_t physical_address;
  int rc;

  /* this silences gcc -Wuninitialized: */
  bus_name = NULL;

  /* recover the virtual address used: */
  virtual_address = cycle->tme_bus_cycle_address - tlb->tme_bus_tlb_addr_offset;

  /* look up the PTE involved: */
  rc = tme_sun_mmu_pte_get(sun3->tme_sun3_mmu, 
			   sun3->tme_sun3_context,
			   virtual_address,
			   &pte);
  assert(rc == TME_OK);
  pte_sun3 = pte.tme_sun_mmu_pte_raw;
    
  /* form the physical address and get the bus name: */
  physical_address = (((pte_sun3 & TME_SUN3_PTE_PGFRAME) << TME_SUN3_PAGE_SIZE_LOG2)
		      | (virtual_address & (TME_SUN3_PAGE_SIZE - 1)));
  switch (TME_FIELD_MASK_EXTRACTU(pte_sun3, TME_SUN3_PTE_PGTYPE)) {
  case TME_SUN3_PGTYPE_OBMEM: bus_name = "obmem"; break;
  case TME_SUN3_PGTYPE_OBIO: bus_name = "obio"; break;
  case TME_SUN3_PGTYPE_VME_D16:
    bus_name = "VME_D16";
    break;
  case TME_SUN3_PGTYPE_VME_D32:
    bus_name = "VME_D32";
    break;
  }

  /* log this bus error: */
  tme_log(TME_SUN3_LOG_HANDLE(sun3), 1000, TME_OK,
	  (TME_SUN3_LOG_HANDLE(sun3), 
	   _("%s bus error, physical 0x%08x, virtual 0x%08x, buserr = 0x%02x"),
	   bus_name,
	   physical_address,
	   virtual_address,
	   sun3->tme_sun3_buserr));
}
#else  /* TME_NO_LOG */
#define _tme_sun3_bus_fault_log(a, b, c) do { } while (/* CONSTCOND */ 0)
#endif /* TME_NO_LOG */

/* our general bus fault handler: */
static int
_tme_sun3_bus_fault_handler(struct tme_sun3 *sun3, 
			    struct tme_bus_tlb *tlb,
			    struct tme_bus_cycle *cycle,
			    int rc)
{
  tme_uint8_t buserr;

  /* dispatch on our fault code: */
  switch (rc) {

    /* bus address nonexistent: */
  case ENOENT:
    buserr = TME_SUN3_BUSERR_TIMEOUT;
    break;

    /* anything else is just a fault: */
  default:
    buserr = 0;
    break;
  }

  /* set the bus error register: */
  sun3->tme_sun3_buserr = buserr;

  /* log the fault: */
  _tme_sun3_bus_fault_log(sun3, tlb, cycle);

  return (rc);
}

/* our obio bus fault handler: */
static int
_tme_sun3_obio_fault_handler(void *_sun3, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{

  /* call the common bus fault handler: */
  return (_tme_sun3_bus_fault_handler((struct tme_sun3 *) _sun3, tlb, cycle, rc));
}

/* our obmem bus fault handler: */
static int
_tme_sun3_obmem_fault_handler(void *_sun3, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{

  /* call the common bus fault handler: */
  return (_tme_sun3_bus_fault_handler((struct tme_sun3 *) _sun3, tlb, cycle, rc));
}

/* our VMEbus fault handler: */
static int
_tme_sun3_vmebus_fault_handler(void *_sun3, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{
  struct tme_sun3 *sun3;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* call the common bus fault handler: */
  rc = _tme_sun3_bus_fault_handler((struct tme_sun3 *) _sun3, tlb, cycle, rc);

  /* this bus fault happened on the VMEbus: */
  sun3->tme_sun3_buserr |= TME_SUN3_BUSERR_VMEBUSERR;

  /* return the fault: */
  return (rc);
}

/* our dummy cycle handler: */
static int
_tme_sun3_cycle_dummy(void *_sun3, struct tme_bus_cycle *cycle)
{
  return (TME_OK);
}

/* our page-invalid cycle handler: */
static int
_tme_sun3_mmu_invalid(void *_sun3, struct tme_bus_cycle *cycle)
{
  struct tme_sun3 *sun3;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* log this bus error: */
  tme_log(TME_SUN3_LOG_HANDLE(sun3), 1000, TME_OK,
	  (TME_SUN3_LOG_HANDLE(sun3), 
	   _("page invalid bus error")));

  /* set the bus error register: */
  sun3->tme_sun3_buserr = TME_SUN3_BUSERR_INVALID;

  /* return the fault: */
  return (EFAULT);
}

/* our protection error cycle handler: */
static int
_tme_sun3_mmu_proterr(void *_sun3, struct tme_bus_cycle *cycle)
{
  struct tme_sun3 *sun3;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* log this bus error: */
  tme_log(TME_SUN3_LOG_HANDLE(sun3), 1000, TME_OK,
	  (TME_SUN3_LOG_HANDLE(sun3),
	   _("page protection bus error")));

  /* set the bus error register: */
  sun3->tme_sun3_buserr = TME_SUN3_BUSERR_PROTERR;

  /* return the fault: */
  return (EFAULT);
}

/* our SDVMA disabled cycle handler: */
static int
_tme_sun3_sdvma_disabled(void *_sun3, struct tme_bus_cycle *cycle)
{
  return (ENOENT);
}

/* our internal TLB filler: */
static int
_tme_sun3_tlb_fill(struct tme_sun3 *sun3, struct tme_bus_tlb *tlb, tme_uint8_t context,
		   unsigned int *_function_code_or_codes, tme_uint32_t address, unsigned int cycles)
{
  unsigned int function_code;
  unsigned int function_code_sp;
  unsigned short access;
  struct tme_bus_tlb tlb_bus;
  unsigned short tlb_flags;

  /* recover the function code: */
  function_code = *_function_code_or_codes;

  /* this must be a user or supervisor program or data function code: */
  assert(function_code == TME_M68K_FC_UD
	 || function_code == TME_M68K_FC_UP
	 || function_code == TME_M68K_FC_SD
	 || function_code == TME_M68K_FC_SP);

  /* assume that if this TLB entry ends up good for the supervisor,
     it's good for the supervisor program function code: */
  function_code_sp = TME_BIT(TME_M68K_FC_SP);

  /* if we're in the boot state: */
  if (__tme_predict_false((sun3->tme_sun3_enable & TME_SUN3_ENA_NOTBOOT) == 0)) {

    /* if this is the supervisor program function code: */
    if (function_code == TME_M68K_FC_SP) {

      /* fill this TLB entry directly from the obmem bus: */
      (*sun3->tme_sun3_obmem->tme_bus_tlb_fill)
	(sun3->tme_sun3_obmem,
	 tlb,
	 TME_SUN3_PROM_BASE | (address & (TME_SUN3_PROM_SIZE - 1)),
	 cycles);
	
      /* create the mapping TLB entry: */
      tlb_bus.tme_bus_tlb_addr_first = address & (((tme_bus_addr32_t) 0) - TME_SUN3_PROM_SIZE);
      tlb_bus.tme_bus_tlb_addr_last = address | (TME_SUN3_PROM_SIZE - 1);
      tlb_bus.tme_bus_tlb_cycles_ok
	= TME_BUS_CYCLE_READ;
  
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, TME_SUN3_PROM_BASE | (address & (TME_SUN3_PROM_SIZE - 1)), &tlb_bus, address);
	
      /* this is good for the supervisor program function code only: */
      *_function_code_or_codes = TME_BIT(TME_M68K_FC_SP);

      /* done: */
      return(TME_OK);
    }

    /* if this TLB entry ends up good for the supervisor, it's not
       good for the supervisor program function code: */
    function_code_sp = 0;
  }

  /* fill this TLB entry from the MMU: */
  access
    = ((cycles & TME_BUS_CYCLE_WRITE)
       ? TME_SUN_MMU_PTE_PROT_RW
       : TME_SUN_MMU_PTE_PROT_RO);
  access
    = ((function_code == TME_M68K_FC_UD
	|| function_code == TME_M68K_FC_UP)
       ? TME_SUN_MMU_PTE_PROT_USER(access)
       : TME_SUN_MMU_PTE_PROT_SYSTEM(access));
  tlb_flags = tme_sun_mmu_tlb_fill(sun3->tme_sun3_mmu,
				   tlb,
				   context,
				   address,
				   access);

  /* this TLB entry is good for the program and data function codes
     for the user and/or the supervisor: */
  *_function_code_or_codes 
    = (((tlb_flags & TME_SUN_MMU_TLB_USER)
	? (TME_BIT(TME_M68K_FC_UD)
	   | TME_BIT(TME_M68K_FC_UP))
	: 0)
       | ((tlb_flags & TME_SUN_MMU_TLB_SYSTEM)
	  ? (TME_BIT(TME_M68K_FC_SD)
	     | function_code_sp)
	  : 0));

  /* if the memory error register is being tested: */
  if (__tme_predict_false(sun3->tme_sun3_memerr_csr & TME_SUN3_MEMERR_PAR_TEST)) {

    /* this must be a supervisor data access: */
    if (function_code != TME_M68K_FC_SD) {
      abort();
    }
    
    /* if this TLB's bus cycle handler isn't for the memory error
       register itself: */
    if (tlb->tme_bus_tlb_cycle != _tme_sun3_memerr_cycle_handler) {

      /* there must be no other TLB entry already involved in the test: */
      if (sun3->tme_sun3_memerr_tlb != NULL) {
	abort();
      }

      /* remember this TLB entry pointer, and its original bus cycle
	 handler: */
      /* XXX FIXME - this assumes that the TLB entry being filled is a
	 real m68k TLB entry, not on the stack: */
      assert (tlb->tme_bus_tlb_token == &((struct tme_m68k_tlb *) tlb)->tme_m68k_tlb_token);
      sun3->tme_sun3_memerr_tlb = tlb;
      assert (sun3->tme_sun3_memerr_tlb != NULL);
      sun3->tme_sun3_memerr_cycle_private = tlb->tme_bus_tlb_cycle_private;
      sun3->tme_sun3_memerr_cycle = tlb->tme_bus_tlb_cycle;
      
      /* this TLB entry does not allow fast reading and writing, and
	 it now uses the memory error test cycle handler: */
      tlb->tme_bus_tlb_emulator_off_read = TME_EMULATOR_OFF_UNDEF;
      tlb->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
      tlb->tme_bus_tlb_rwlock = NULL;
      tlb->tme_bus_tlb_cycle_private = sun3;
      tlb->tme_bus_tlb_cycle = _tme_sun3_memerr_test_cycle_handler;

      /* reset other memory error test values: */
      sun3->tme_sun3_memerr_pending_csr = 0;
    }
  }

  return (TME_OK);
}

/* our m68k TLB filler: */
int
_tme_sun3_m68k_tlb_fill(struct tme_m68k_bus_connection *conn_m68k, struct tme_m68k_tlb *tlb_m68k,
			unsigned int function_code, tme_uint32_t address, unsigned int cycles)
{
  struct tme_sun3 *sun3;
  struct tme_bus_tlb *tlb;
  struct tme_bus_tlb tlb_bus;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) conn_m68k->tme_m68k_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the generic bus TLB: */
  tlb = &tlb_m68k->tme_m68k_tlb_bus_tlb;

  /* if this is function code three: */
  if (function_code == TME_M68K_FC_3) {

    /* if this address is within the UART bypass: */
    if (address >= TME_SUN3_CONTROL_UART_BYPASS
	&& address < TME_SUN3_CONTROL_UART_BYPASS + TME_SUN_Z8530_SIZE) {

      /* fill this TLB entry directly from the obio bus: */
      (*sun3->tme_sun3_obio->tme_bus_tlb_fill)
	(sun3->tme_sun3_obio,
	 tlb,
	 (address - TME_SUN3_CONTROL_UART_BYPASS) + TME_SUN3_OBIO_ZS0,
	 cycles);

      /* create the mapping TLB entry: */
      tlb_bus.tme_bus_tlb_addr_first = TME_SUN3_CONTROL_UART_BYPASS;
      tlb_bus.tme_bus_tlb_addr_last = TME_SUN3_CONTROL_UART_BYPASS + TME_SUN_Z8530_SIZE - 1;
      tlb_bus.tme_bus_tlb_cycles_ok
	= (TME_BUS_CYCLE_READ
	   | TME_BUS_CYCLE_WRITE);
  
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, (address - TME_SUN3_CONTROL_UART_BYPASS) + TME_SUN3_OBIO_ZS0, &tlb_bus, address);
    }

    /* otherwise, this is something else in control space: */
    else {

      /* initialize the TLB entry: */
      tme_bus_tlb_initialize(tlb);

      /* we cover the entire address space up to the UART bypass: */
      tlb->tme_bus_tlb_addr_first = 0;
      tlb->tme_bus_tlb_addr_last = TME_SUN3_CONTROL_UART_BYPASS - 1;

      /* we allow reading and writing: */
      tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

      /* our bus cycle handler: */
      tlb->tme_bus_tlb_cycle_private = sun3;
      tlb->tme_bus_tlb_cycle = _tme_sun3_control_cycle_handler;
    }

    /* this is good for function code three only: */
    tlb_m68k->tme_m68k_tlb_function_codes_mask = TME_BIT(TME_M68K_FC_3);

    /* done: */
    return (TME_OK);
  }

  /* this is a normal function code: */
  tlb_m68k->tme_m68k_tlb_function_codes_mask = function_code;
  return (_tme_sun3_tlb_fill(sun3, tlb, sun3->tme_sun3_context,
			     &tlb_m68k->tme_m68k_tlb_function_codes_mask,
			     address, cycles));
}

/* our bus TLB filler: */
int
_tme_sun3_bus_tlb_fill(struct tme_bus_connection *conn_bus, struct tme_bus_tlb *tlb,
		       tme_bus_addr_t address_wider, unsigned int cycles)
{
  tme_bus_addr32_t address;
  struct tme_sun3 *sun3;
  struct tme_sun3_bus_connection *conn_sun3;
  tme_uint32_t base, size;
  struct tme_bus_tlb tlb_bus;
  tme_uint8_t context;
  unsigned int function_code_or_codes;
  unsigned int tlb_i;

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* recover the sun3 internal mainbus connection: */
  conn_sun3 = (struct tme_sun3_bus_connection *) conn_bus;

  /* dispatch on the internal connection.  the bus address into a DVMA
     address, context, and function code, except for the memory error
     register and interrupt register connections, which are handled
     specially: */
  switch (conn_sun3->tme_sun3_bus_connection_which) {

  case TME_SUN3_CONN_OBIO_MASTER:
    base = 0x0f000000;
    size = TME_SUN3_DVMA_SIZE_OBIO;
    context = sun3->tme_sun3_context;
    function_code_or_codes = TME_M68K_FC_SD;
    break;

  case TME_SUN3_CONN_BUS_VME:
    base = 0x0ff00000;
    size = TME_SUN3_DVMA_SIZE_VME;
    context = sun3->tme_sun3_context;
    function_code_or_codes = TME_M68K_FC_SD;
    break;

  case TME_SUN3_CONN_REG_MEMERR:
  case TME_SUN3_CONN_REG_INTREG:
    
    /* initialize the TLB entry: */
    tme_bus_tlb_initialize(tlb);

    /* the address range: */
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last
      = ((conn_sun3->tme_sun3_bus_connection_which == TME_SUN3_CONN_REG_MEMERR
	  ? TME_SUN3_MEMERR_SIZ_REG
	  : sizeof(sun3->tme_sun3_ints))
	 - 1);

    /* we allow reading and writing: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

    /* our bus cycle handler: */
    tlb->tme_bus_tlb_cycle_private = sun3;
    tlb->tme_bus_tlb_cycle
      = (conn_sun3->tme_sun3_bus_connection_which == TME_SUN3_CONN_REG_MEMERR
	 ? _tme_sun3_memerr_cycle_handler
	 : _tme_sun3_intreg_cycle_handler);

    /* done: */
    return (TME_OK);

  default: abort();
  }

  /* update the head pointer for the active SDVMA TLB entry list: */
  tlb_i = sun3->tme_sun3_sdvma_tlb_next
    = ((sun3->tme_sun3_sdvma_tlb_next
	+ 1)
       & (TME_SUN3_SDVMA_TLBS - 1));

  /* if the new head pointer already has a TLB entry, and it doesn't
     happen to be the same as this TLB entry, invalidate it: */
  if (sun3->tme_sun3_sdvma_tlb_tokens[tlb_i] != NULL
      && (sun3->tme_sun3_sdvma_tlb_tokens[tlb_i]
	  != tlb->tme_bus_tlb_token)) {
    tme_token_invalidate(sun3->tme_sun3_sdvma_tlb_tokens[tlb_i]);
  }

  /* add this TLB entry to the active list: */
  sun3->tme_sun3_sdvma_tlb_tokens[tlb_i]
    = tlb->tme_bus_tlb_token;

  /* if system DVMA is disabled: */
  if (__tme_predict_false(!(sun3->tme_sun3_enable & TME_SUN3_ENA_SDVMA))) {

    /* return a TLB entry that will generate a VME bus fault: */
    tme_bus_tlb_initialize(tlb);
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = size - 1;
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
    tlb->tme_bus_tlb_cycle_private = sun3;
    tlb->tme_bus_tlb_cycle = _tme_sun3_sdvma_disabled;
    TME_BUS_TLB_FAULT_HANDLER(tlb, _tme_sun3_vmebus_fault_handler, sun3);
    return (TME_OK);
  }

  assert (!(address & base)
	  && (address < size));

  /* fill this TLB entry from the MMU: */
  _tme_sun3_tlb_fill(sun3, tlb, context,
		     &function_code_or_codes,
		     address | base, cycles);

  /* this bus TLB entry depends on the current context: */
  tme_sun_mmu_context_add(sun3->tme_sun3_mmu, tlb);

  /* create the mapping TLB entry.  we do this even if base == 0,
     because the TLB entry as currently filled may cover more address
     space than DVMA space on this machine is supposed to cover: */
  tlb_bus.tme_bus_tlb_addr_first = 0;
  tlb_bus.tme_bus_tlb_addr_last = size - 1;
  tlb_bus.tme_bus_tlb_cycles_ok
    = (TME_BUS_CYCLE_READ
       | TME_BUS_CYCLE_WRITE);
  
  /* map the filled TLB entry: */
  tme_bus_tlb_map(tlb, address | base, &tlb_bus, address);

  /* XXX FIXME - what happens to a bus cycle to an unmapped DVMA
     address?  is the answer different for obio masters and the VME
     bus?  for now, these bus cycles get ignored in both cases: */
  if (tlb->tme_bus_tlb_cycle == _tme_sun3_mmu_invalid) {
    tlb->tme_bus_tlb_cycle = _tme_sun3_cycle_dummy;
  }

  return (TME_OK);
}

/* our post-MMU TLB filler: */
static int
_tme_sun3_tlb_fill_mmu(void *_sun3, struct tme_bus_tlb *tlb, 
		       struct tme_sun_mmu_pte *pte,
		       tme_uint32_t *_address,
		       unsigned int cycles)
{
  struct tme_sun3 *sun3;
  tme_uint32_t address;
  unsigned int bus_type;
  struct tme_bus_connection *conn_bus;
  tme_bus_fault_handler bus_fault_handler;
  int rc;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) _sun3;

  /* get the physical page frame and bus type: */
  address = ((pte->tme_sun_mmu_pte_raw & TME_SUN3_PTE_PGFRAME) << TME_SUN3_PAGE_SIZE_LOG2);
  bus_type = TME_FIELD_MASK_EXTRACTU(pte->tme_sun_mmu_pte_raw, TME_SUN3_PTE_PGTYPE);

  /* any mapping of any address in a PROM-sized region starting at
     0x100000 in obio space means the PROM.  the virtual page frame is
     actually used to form the physical address: */
  if ((address & -TME_SUN3_PROM_SIZE) == TME_SUN3_OBIO_PROM
      && bus_type == TME_SUN3_PGTYPE_OBIO) {
    address = TME_SUN3_PROM_BASE | (*_address & ((TME_SUN3_PROM_SIZE - 1) & ~(TME_SUN3_PAGE_SIZE - 1)));
    bus_type = TME_SUN3_PGTYPE_OBMEM;
  }

#if 1 /* NetBSD/sun3 cgtwo bug */
  /* XXX FIXME - this hack works around a bug in NetBSD/sun3, present
     since revision 1.49 of src/sys/arch/sun3/conf/GENERIC (when the
     sun3x port was merged into the sun3 port).  in this revision, the
     declaration for cgtwo0 changed:

-cgtwo0 at vmes0 addr 0xff400000 level 4 vect 0xA8
+cgtwo0 at vme2 addr 0x400000 ipl 4 vect 0xA8

     because the cg2mmap() function in src/sys/arch/sun3/dev/cg2.c
     doesn't add the 0xff000000 mask to the configured physical
     address (needed because the cgtwo is an A24 device), when Xsun
     mmap()s the cgtwo it gets a mapping of physical address 0x400000
     in VME space instead of the correct 0xff400000.  the sparc cgtwo
     driver gets this right.

     so for now we force all accesses to VME D16 address 0x400000 to
     0xff400000.  once the NetBSD bug has been fixed this code should
     be removed: */
  if (bus_type == TME_SUN3_PGTYPE_VME_D16
      && ((address & 0xff400000) == 0x400000)) {
    address |= 0xff000000;
  }
#endif /* NetBSD/sun3 cgtwo bug */

  /* add in the page offset to finish the address: */
  address |= *_address & (TME_SUN3_PAGE_SIZE - 1);
  *_address = address;

  /* if this is obio: */
  if (bus_type == TME_SUN3_PGTYPE_OBIO) {
    conn_bus = sun3->tme_sun3_obio;
    bus_fault_handler = _tme_sun3_obio_fault_handler;
  }
  
  /* if this is obmem: */
  else if (bus_type == TME_SUN3_PGTYPE_OBMEM) {
    conn_bus = sun3->tme_sun3_obmem;
    bus_fault_handler = _tme_sun3_obmem_fault_handler;
  }

  /* if this is the VME bus: */
  else {
    assert(bus_type == TME_SUN3_PGTYPE_VME_D16
	   || bus_type == TME_SUN3_PGTYPE_VME_D32);
    conn_bus = sun3->tme_sun3_vmebus;
    bus_fault_handler = _tme_sun3_vmebus_fault_handler;
  }

  /* call the bus TLB filler: */
  rc = ((*conn_bus->tme_bus_tlb_fill)
	(conn_bus, tlb, address, cycles));

  /* if the bus TLB filler succeeded, add our bus fault handler: */
  if (rc == TME_OK) {
    TME_BUS_TLB_FAULT_HANDLER(tlb, bus_fault_handler, sun3);
  }

  return (rc);
}

/* this gets a PTE from the MMU: */
int
_tme_sun3_mmu_pte_get(struct tme_sun3 *sun3, tme_uint32_t address, tme_uint32_t *_pte_sun3)
{
  struct tme_sun_mmu_pte pte;
  tme_uint32_t pte_sun3;
  unsigned int pte_flags;
  int rc;

  /* get the PTE from the MMU: */
  rc = tme_sun_mmu_pte_get(sun3->tme_sun3_mmu, 
			   sun3->tme_sun3_context,
			   address,
			   &pte);
  assert(rc == TME_OK);
    
  /* form the Sun-3 PTE: */
  pte_sun3 = pte.tme_sun_mmu_pte_raw;
  pte_flags = pte.tme_sun_mmu_pte_flags;
  if (pte_flags & TME_SUN_MMU_PTE_REF) {
    pte_sun3 |= TME_SUN3_PTE_REF;
  }
  if (pte_flags & TME_SUN_MMU_PTE_MOD) {
    pte_sun3 |= TME_SUN3_PTE_MOD;
  }

  /* done: */
  *_pte_sun3 = pte_sun3;
  tme_log(TME_SUN3_LOG_HANDLE(sun3), 1000, TME_OK,
	  (TME_SUN3_LOG_HANDLE(sun3),
	   _("pte_get: PGMAP[%d:0x%08x] -> 0x%08x"), 
	   sun3->tme_sun3_context,
	   address,
	   pte_sun3));
  return (TME_OK);
}

/* this sets a PTE into the MMU: */
int
_tme_sun3_mmu_pte_set(struct tme_sun3 *sun3, tme_uint32_t address, tme_uint32_t pte_sun3)
{
  struct tme_sun_mmu_pte pte;
  unsigned int pte_flags;
#ifndef TME_NO_LOG
  const char *bus_name;
  tme_bus_addr32_t physical_address;
      
  /* this silences gcc -Wuninitialized: */
  bus_name = NULL;

  /* log this setting: */
  physical_address = ((pte_sun3 & TME_SUN3_PTE_PGFRAME) << TME_SUN3_PAGE_SIZE_LOG2);
  switch (TME_FIELD_MASK_EXTRACTU(pte_sun3, TME_SUN3_PTE_PGTYPE)) {
  case TME_SUN3_PGTYPE_OBMEM: bus_name = "obmem"; break;
  case TME_SUN3_PGTYPE_OBIO: bus_name = "obio"; break;
  case TME_SUN3_PGTYPE_VME_D16: bus_name = "VME_D16"; break;
  case TME_SUN3_PGTYPE_VME_D32: bus_name = "VME_D32"; break;
  }
  tme_log(TME_SUN3_LOG_HANDLE(sun3), 1000, TME_OK,
	  (TME_SUN3_LOG_HANDLE(sun3),
	   _("pte_set: PGMAP[%d:0x%08x] <- 0x%08x (%s 0x%08x)"), 
	   sun3->tme_sun3_context,
	   address,
	   pte_sun3,
	   bus_name,
	   physical_address));
#endif /* !TME_NO_LOG */

  pte.tme_sun_mmu_pte_raw = pte_sun3;
      
  pte_flags = (pte_sun3 & TME_SUN3_PTE_WRITE
	       ? TME_SUN_MMU_PTE_PROT_RW
	       : TME_SUN_MMU_PTE_PROT_RO);
  pte_flags = (TME_SUN_MMU_PTE_PROT_SYSTEM(pte_flags)
	       | TME_SUN_MMU_PTE_PROT_USER(pte_sun3 & TME_SUN3_PTE_SYSTEM
					   ? TME_SUN_MMU_PTE_PROT_ERROR
					   : pte_flags));
  if (pte_sun3 & TME_SUN3_PTE_MOD) {
    pte_flags |= TME_SUN_MMU_PTE_MOD;
  }
  if (pte_sun3 & TME_SUN3_PTE_REF) {
    pte_flags |= TME_SUN_MMU_PTE_REF;
  }
  if (pte_sun3 & TME_SUN3_PTE_VALID) {
    pte_flags |= TME_SUN_MMU_PTE_VALID;
  }
  pte.tme_sun_mmu_pte_flags = pte_flags;
  
  return (tme_sun_mmu_pte_set(sun3->tme_sun3_mmu, 
			      sun3->tme_sun3_context,
			      address,
			      &pte));
}

/* this is called when the SDVMA bit is changed in the enable register: */
void
_tme_sun3_mmu_sdvma_change(struct tme_sun3 *sun3)
{
  unsigned int tlb_i;

  /* whenever the SDVMA bit changes, we have to invalidate all SDVMA
     TLB entries: */
  for (tlb_i = 0; tlb_i < TME_SUN3_SDVMA_TLBS; tlb_i++) {
    if (sun3->tme_sun3_sdvma_tlb_tokens[tlb_i] != NULL) {
      tme_token_invalidate(sun3->tme_sun3_sdvma_tlb_tokens[tlb_i]);
      sun3->tme_sun3_sdvma_tlb_tokens[tlb_i] = NULL;
    }
  }
}

/* this is called when the context register is set: */
void
_tme_sun3_mmu_context_set(struct tme_sun3 *sun3)
{
  tme_bus_context_t context_base;

  /* there are sixteen total contexts.  contexts zero through seven
     are the not-boot (normal) contexts.  contexts eight through
     fifteen are the same contexts, but in the boot state.

     in the boot state, TLB fills for supervisor program references
     bypass the MMU and are filled to reference the PROM, and data
     fills are filled as normal using the current context:  */

  /* in the not-boot (i.e., normal, state): */
  if (__tme_predict_true(sun3->tme_sun3_enable & TME_SUN3_ENA_NOTBOOT)) {

    tme_log(TME_SUN3_LOG_HANDLE(sun3), 1000, TME_OK,
	    (TME_SUN3_LOG_HANDLE(sun3),
	     _("context now #%d"),
	     sun3->tme_sun3_context));

    /* the normal state contexts are numbered from zero: */
    context_base = 0;
  }

  /* in the boot state: */
  else {

    tme_log(TME_SUN3_LOG_HANDLE(sun3), 1000, TME_OK,
	    (TME_SUN3_LOG_HANDLE(sun3),
	     _("context now #%d (boot state)"),
	     sun3->tme_sun3_context));

    /* the boot state contexts are numbered from eight: */
    context_base = TME_SUN3_CONTEXT_COUNT;
  }

  /* update the m68k bus context register: */
  *sun3->tme_sun3_m68k_bus_context
    = (context_base
       + sun3->tme_sun3_context);

  /* invalidate all DVMA TLBs that depended on the previous context: */
  tme_sun_mmu_context_switched(sun3->tme_sun3_mmu);
}

/* this adds a new TLB set: */
int
_tme_sun3_mmu_tlb_set_add(struct tme_bus_connection *conn_bus_asker,
			  struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_sun3 *sun3;
  int rc;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) conn_bus_asker->tme_bus_connection.tme_connection_element->tme_element_private;

  /* add the TLB set to the MMU: */
  rc = tme_sun_mmu_tlb_set_add(sun3->tme_sun3_mmu,
			       tlb_set_info);
  assert (rc == TME_OK);

  /* if this is the TLB set from the m68k: */
  if (conn_bus_asker->tme_bus_connection.tme_connection_type == TME_CONNECTION_BUS_M68K) {

    /* the m68k must expose a bus context register: */
    assert (tlb_set_info->tme_bus_tlb_set_info_bus_context != NULL);

    /* save the pointer to the m68k bus context register, and
       initialize it: */
    sun3->tme_sun3_m68k_bus_context
      = tlb_set_info->tme_bus_tlb_set_info_bus_context;
    _tme_sun3_mmu_context_set(sun3);

    /* return the maximum context number.  there are eight
       contexts in the not-boot (normal) state, and each of
       them has a boot state counterpart: */
    tlb_set_info->tme_bus_tlb_set_info_bus_context_max
      = (TME_SUN3_CONTEXT_COUNT
	 + TME_SUN3_CONTEXT_COUNT
	 - 1);
  }

  return (rc);
}

/* this creates a Sun-3 MMU: */
void
_tme_sun3_mmu_new(struct tme_sun3 *sun3)
{
  struct tme_sun_mmu_info mmu_info;

  memset(&mmu_info, 0, sizeof(mmu_info));
  mmu_info.tme_sun_mmu_info_element = sun3->tme_sun3_element;
  mmu_info.tme_sun_mmu_info_address_bits = 28;
  mmu_info.tme_sun_mmu_info_pgoffset_bits = TME_SUN3_PAGE_SIZE_LOG2;
  mmu_info.tme_sun_mmu_info_pteindex_bits = 4;
  mmu_info.tme_sun_mmu_info_contexts = TME_SUN3_CONTEXT_COUNT;
  mmu_info.tme_sun_mmu_info_pmegs = TME_SUN3_PMEGS;
  mmu_info.tme_sun_mmu_info_tlb_fill_private = sun3;
  mmu_info.tme_sun_mmu_info_tlb_fill = _tme_sun3_tlb_fill_mmu;
  mmu_info.tme_sun_mmu_info_proterr_private = sun3;
  mmu_info.tme_sun_mmu_info_proterr = _tme_sun3_mmu_proterr;
  mmu_info.tme_sun_mmu_info_invalid_private = sun3;
  mmu_info.tme_sun_mmu_info_invalid = _tme_sun3_mmu_invalid;
  sun3->tme_sun3_mmu = tme_sun_mmu_new(&mmu_info);
}
