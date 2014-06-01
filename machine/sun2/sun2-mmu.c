/* $Id: sun2-mmu.c,v 1.14 2009/08/30 14:39:47 fredette Exp $ */

/* machine/sun2/sun2-mmu.c - implementation of Sun 2 MMU emulation: */

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
_TME_RCSID("$Id: sun2-mmu.c,v 1.14 2009/08/30 14:39:47 fredette Exp $");

/* includes: */
#include "sun2-impl.h"

/* macros: */

/* real PTE entry bits: */
#define TME_SUN2_PTE_VALID		0x80000000
#define TME_SUN2_PTE_PROT		0x7C000000
#define TME_SUN2_PTE_FOD		0x02000000
#define TME_SUN2_PTE_PGTYPE		0x00C00000
#define  TME_SUN2_PTE_PGTYPE_MASK	 0x00000003
#define TME_SUN2_PTE_REF		0x00200000
#define TME_SUN2_PTE_MOD		0x00100000
#define TME_SUN2_PTE_PGFRAME		0x00000FFF

/* real PTE page types: */
#define TME_SUN2_PGTYPE_OBMEM	(0)
#define TME_SUN2_PGTYPE_OBIO	(1)
#define TME_SUN2_PGTYPE_MBMEM	(2)
#define TME_SUN2_PGTYPE_VME0	(2)
#define TME_SUN2_PGTYPE_MBIO	(3)
#define TME_SUN2_PGTYPE_VME8	(3)

/* real bus error register bits: */
#define TME_SUN2_BUSERR_PARERR_L	TME_BIT(0)	/* parity error, lower byte */
#define TME_SUN2_BUSERR_PARERR_U	TME_BIT(1)	/* parity error, upper byte */
#define TME_SUN2_BUSERR_TIMEOUT		TME_BIT(2)	/* bus access timed out */
#define TME_SUN2_BUSERR_PROTERR		TME_BIT(3)	/* protection error */
#define TME_SUN2_BUSERR_VMEBUSERR	TME_BIT(6)	/* bus error signaled on VMEbus */
#define TME_SUN2_BUSERR_VALID		TME_BIT(7)	/* page map was valid */

/* real context count: */
#define TME_SUN2_CONTEXT_COUNT		(8)

/* this logs a bus error: */
#ifndef TME_NO_LOG
static void
_tme_sun2_bus_fault_log(struct tme_sun2 *sun2, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle)
{
  tme_bus_addr32_t virtual_address;
  struct tme_sun_mmu_pte pte;
  tme_uint32_t pte_sun2;
  const char *bus_name;
  tme_bus_addr32_t physical_address;
  int rc;

  /* this silences gcc -Wuninitialized: */
  bus_name = NULL;

  /* recover the virtual address used: */
  virtual_address = cycle->tme_bus_cycle_address - tlb->tme_bus_tlb_addr_offset;

  /* look up the PTE involved.  since this is a real bus error, and
     not a protection violation or page not present bus error, we
     assume the system context: */
  rc = tme_sun_mmu_pte_get(sun2->tme_sun2_mmu, 
			   sun2->tme_sun2_context_system,
			   virtual_address,
			   &pte);
  assert(rc == TME_OK);
  pte_sun2 = pte.tme_sun_mmu_pte_raw;
    
  /* form the physical address and get the bus name: */
  physical_address = (((pte_sun2 & TME_SUN2_PTE_PGFRAME) << TME_SUN2_PAGE_SIZE_LOG2)
		      | (virtual_address & (TME_SUN2_PAGE_SIZE - 1)));
  switch ((pte_sun2 & TME_SUN2_PTE_PGTYPE) / (TME_SUN2_PTE_PGTYPE / TME_SUN2_PTE_PGTYPE_MASK)) {
  case TME_SUN2_PGTYPE_OBMEM: bus_name = "obmem"; break;
  case TME_SUN2_PGTYPE_OBIO: bus_name = "obio"; break;
  case TME_SUN2_PGTYPE_MBMEM:
    if (sun2->tme_sun2_has_vme) {
      bus_name = "VME";
    }
    else {
      bus_name = "mbmem";
    }
    break;
  case TME_SUN2_PGTYPE_MBIO:
    if (sun2->tme_sun2_has_vme) {
      bus_name = "VME";
      physical_address |= 0x800000;
    }
    else {
      bus_name = "mbio";
    }
    break;
  }

  /* log this bus error: */
  tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	  (TME_SUN2_LOG_HANDLE(sun2), 
	   _("%s bus error, physical 0x%08x, virtual 0x%08x, buserr = 0x%02x"),
	   bus_name,
	   physical_address,
	   virtual_address,
	   sun2->tme_sun2_buserr));
}
#else  /* TME_NO_LOG */
#define _tme_sun2_bus_fault_log(a, b, c) do { } while (/* CONSTCOND */ 0)
#endif /* TME_NO_LOG */

/* our general bus fault handler: */
static int
_tme_sun2_bus_fault_handler(struct tme_sun2 *sun2, 
			    struct tme_bus_tlb *tlb,
			    struct tme_bus_cycle *cycle,
			    int rc)
{
  tme_uint16_t buserr;

  /* dispatch on our fault code: */
  switch (rc) {

    /* bus address nonexistent: */
  case ENOENT:
    buserr = TME_SUN2_BUSERR_VALID | TME_SUN2_BUSERR_TIMEOUT;
    break;

    /* anything else is just a fault: */
  default:
    buserr = TME_SUN2_BUSERR_VALID;
    break;
  }

  /* set the bus error register: */
  sun2->tme_sun2_buserr = buserr;

  /* log the fault: */
  _tme_sun2_bus_fault_log(sun2, tlb, cycle);

  return (rc);
}

/* our obio bus fault handler: */
static int
_tme_sun2_obio_fault_handler(void *_sun2, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{
  tme_uint8_t all_bits_one[sizeof(tme_uint16_t)];

  /* the sun2 obio bus doesn't generate bus errors, it just reads
     all-bits-one: */
  memset(all_bits_one, 0xff, sizeof(all_bits_one));
  tme_bus_cycle_xfer_memory(cycle, 
			    &all_bits_one[0] - cycle->tme_bus_cycle_address,
			    cycle->tme_bus_cycle_address + sizeof(all_bits_one));
  return (TME_OK);
}

/* our obmem bus fault handler: */
static int
_tme_sun2_obmem_fault_handler(void *_sun2, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{
  tme_uint8_t all_bits_one[sizeof(tme_uint16_t)];

  /* the sun2 obmem bus apparently doesn't generate bus errors below
     0x700000, and instead just reads all-bits-one: */
  if (cycle->tme_bus_cycle_address < 0x700000) {
    memset(all_bits_one, 0xff, sizeof(all_bits_one));
    tme_bus_cycle_xfer_memory(cycle, 
			      &all_bits_one[0] - cycle->tme_bus_cycle_address,
			      cycle->tme_bus_cycle_address + sizeof(all_bits_one));
    return (TME_OK);
  }

  /* call the common bus fault handler: */
  return (_tme_sun2_bus_fault_handler((struct tme_sun2 *) _sun2, tlb, cycle, rc));
}

/* our Multibus fault handler: */
static int
_tme_sun2_multibus_fault_handler(void *_sun2, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{

  /* call the common bus fault handler: */
  return (_tme_sun2_bus_fault_handler((struct tme_sun2 *) _sun2, tlb, cycle, rc));
}

/* our VMEbus fault handler: */
static int
_tme_sun2_vmebus_fault_handler(void *_sun2, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{
  struct tme_sun2 *sun2;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) _sun2;

  /* call the common bus fault handler: */
  rc = _tme_sun2_bus_fault_handler((struct tme_sun2 *) _sun2, tlb, cycle, rc);

  /* this bus fault happened on the VMEbus: */
  sun2->tme_sun2_buserr |= TME_SUN2_BUSERR_VMEBUSERR;

  /* return the fault: */
  return (rc);
}

/* our page-invalid cycle handler: */
static int
_tme_sun2_mmu_invalid(void *_sun2, struct tme_bus_cycle *cycle)
{
  struct tme_sun2 *sun2;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) _sun2;

  /* log this bus error: */
  tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	  (TME_SUN2_LOG_HANDLE(sun2), 
	   _("page invalid bus error")));

  /* set the bus error register: */
  sun2->tme_sun2_buserr = TME_SUN2_BUSERR_PROTERR;

  /* return the fault: */
  return (EFAULT);
}

/* our protection error cycle handler: */
static int
_tme_sun2_mmu_proterr(void *_sun2, struct tme_bus_cycle *cycle)
{
  struct tme_sun2 *sun2;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) _sun2;

  /* log this bus error: */
  tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	  (TME_SUN2_LOG_HANDLE(sun2),
	   _("page protection bus error")));

  /* set the bus error register: */
  sun2->tme_sun2_buserr = TME_SUN2_BUSERR_VALID | TME_SUN2_BUSERR_PROTERR;

  /* return the fault: */
  return (EFAULT);
}

/* our m68k TLB filler: */
int
_tme_sun2_m68k_tlb_fill(struct tme_m68k_bus_connection *conn_m68k, struct tme_m68k_tlb *tlb_m68k,
			unsigned int function_code, tme_uint32_t address, unsigned int cycles)
{
  struct tme_sun2 *sun2;
  struct tme_bus_tlb *tlb;
  unsigned int function_codes_mask;
  struct tme_bus_tlb tlb_mapping;
  tme_uint32_t context;
  tme_uint32_t access;
  unsigned short tlb_flags;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) conn_m68k->tme_m68k_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the generic bus TLB: */
  tlb = &tlb_m68k->tme_m68k_tlb_bus_tlb;

  /* if this is function code three, we handle this ourselves: */
  if (function_code == TME_M68K_FC_3) {

    /* initialize the TLB entry: */
    tme_bus_tlb_initialize(tlb);

    /* we cover the entire address space: */
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = 0 - (tme_bus_addr32_t) 1;

    /* we allow reading and writing: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

    /* our bus cycle handler: */
    tlb->tme_bus_tlb_cycle_private = sun2;
    tlb->tme_bus_tlb_cycle = _tme_sun2_control_cycle_handler;

    /* this is good for function code three only: */
    tlb_m68k->tme_m68k_tlb_function_codes_mask = TME_BIT(TME_M68K_FC_3);

    /* done: */
    return (TME_OK);
  }

  /* this must be or a user or supervisor program or data function
     code: */
  assert(function_code == TME_M68K_FC_UD
	 || function_code == TME_M68K_FC_UP
	 || function_code == TME_M68K_FC_SD
	 || function_code == TME_M68K_FC_SP);

  /* assume that if this TLB entry ends up good for the supervisor,
     it's good for the supervisor program function code: */
  function_codes_mask = TME_BIT(TME_M68K_FC_SP);

  /* if we're in the boot state: */
  if (__tme_predict_false((sun2->tme_sun2_enable & TME_SUN2_ENA_NOTBOOT) == 0)) {

    /* if this is the supervisor program function code: */
    if (function_code == TME_M68K_FC_SP) {

      /* fill this TLB entry directly from the obmem bus: */
      (*sun2->tme_sun2_obmem->tme_bus_tlb_fill)
	(sun2->tme_sun2_obmem,
	 tlb,
	 TME_SUN2_PROM_BASE | (address & (TME_SUN2_PROM_SIZE - 1)),
	 cycles);
	
      /* create the mapping TLB entry: */
      tlb_mapping.tme_bus_tlb_addr_first = address & (((tme_bus_addr32_t) 0) - TME_SUN2_PROM_SIZE);
      tlb_mapping.tme_bus_tlb_addr_last = address | (TME_SUN2_PROM_SIZE - 1);
      tlb_mapping.tme_bus_tlb_cycles_ok
	= TME_BUS_CYCLE_READ;
  
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, TME_SUN2_PROM_BASE | (address & (TME_SUN2_PROM_SIZE - 1)), &tlb_mapping, address);
	
      /* this is good for the supervisor program function code only: */
      tlb_m68k->tme_m68k_tlb_function_codes_mask
	= TME_BIT(TME_M68K_FC_SP);

      /* done: */
      return(TME_OK);
    }

    /* if this TLB entry ends up good for the supervisor, it's not
       good for the supervisor program function code: */
    function_codes_mask = 0;
  }

  /* start the access: */
  access
    = ((cycles & TME_BUS_CYCLE_WRITE)
       ? TME_SUN_MMU_PTE_PROT_RW
       : TME_SUN_MMU_PTE_PROT_RO);

  /* if this is a user program or data function code: */
  if (function_code == TME_M68K_FC_UD
      || function_code == TME_M68K_FC_UP) {
    context = sun2->tme_sun2_context_user;
    access = TME_SUN_MMU_PTE_PROT_USER(access);
    function_codes_mask = (TME_BIT(TME_M68K_FC_UD) + TME_BIT(TME_M68K_FC_UP));
  }

  /* otherwise, this is a supervisor program or data function code: */
  else {
    context = sun2->tme_sun2_context_system;
    access = TME_SUN_MMU_PTE_PROT_SYSTEM(access);
    function_codes_mask += TME_BIT(TME_M68K_FC_SD);
  }

  /* fill this TLB entry from the MMU: */
  tlb_flags
    = tme_sun_mmu_tlb_fill(sun2->tme_sun2_mmu,
			   tlb,
			   context,
			   address,
			   access);

  /* TLB entries are good only for the program and data function
     codes for the user or supervisor, but never both, because
     the two types of accesses go through different contexts: */
  tlb_m68k->tme_m68k_tlb_function_codes_mask = function_codes_mask;

  return (TME_OK);
}

/* our bus TLB filler: */
int
_tme_sun2_bus_tlb_fill(struct tme_bus_connection *conn_bus, struct tme_bus_tlb *tlb,
		       tme_bus_addr_t address_wider, unsigned int cycles)
{
  struct tme_sun2 *sun2;
  tme_bus_addr32_t address;
  struct tme_sun2_bus_connection *conn_sun2;
  tme_uint32_t base, size;
  struct tme_bus_tlb tlb_bus;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* recover the sun2 internal mainbus connection: */
  conn_sun2 = (struct tme_sun2_bus_connection *) conn_bus;

  /* turn the bus address into a DVMA address: */
  switch (conn_sun2->tme_sun2_bus_connection_which) {

    /* obio devices can actually see the whole address space: */
  case TME_SUN2_BUS_OBIO:
    base = 0x000000;
    size = 0x1000000;
    break;

  case TME_SUN2_BUS_MBMEM:
    base = 0xf00000;
    size = TME_SUN2_DVMA_SIZE_MBMEM;
    break;

  case TME_SUN2_BUS_VME:
    base = 0xf00000;
    size = TME_SUN2_DVMA_SIZE_VME;
    break;

  default: abort();
  }

  assert (!(address & base)
	  && (address < size));

  /* fill this TLB entry from the MMU: */
  tme_sun_mmu_tlb_fill(sun2->tme_sun2_mmu,
		       tlb,
		       sun2->tme_sun2_context_system,
		       address | base,
		       ((cycles & TME_BUS_CYCLE_WRITE)
			? TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RW)
			: TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RO)));

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

  return (TME_OK);
}

/* our post-MMU TLB filler: */
static int
_tme_sun2_tlb_fill_mmu(void *_sun2, struct tme_bus_tlb *tlb, 
		       struct tme_sun_mmu_pte *pte,
		       tme_uint32_t *_address,
		       unsigned int cycles)
{
  struct tme_sun2 *sun2;
  tme_uint32_t address;
  unsigned int bus_type;
  struct tme_bus_connection *conn_bus;
  tme_bus_fault_handler bus_fault_handler;
  int rc;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) _sun2;

  /* get the physical page frame and bus type: */
  address = ((pte->tme_sun_mmu_pte_raw & TME_SUN2_PTE_PGFRAME) << TME_SUN2_PAGE_SIZE_LOG2);
  bus_type = (pte->tme_sun_mmu_pte_raw & TME_SUN2_PTE_PGTYPE) / (TME_SUN2_PTE_PGTYPE / TME_SUN2_PTE_PGTYPE_MASK);

  /* any mapping of the *first* page of obio space means the PROM.
     the virtual page frame is actually used to form the physical
     address: */
  if (address == 0
      && bus_type == TME_SUN2_PGTYPE_OBIO) {
    address = TME_SUN2_PROM_BASE | (*_address & ((TME_SUN2_PROM_SIZE - 1) & ~(TME_SUN2_PAGE_SIZE - 1)));
    bus_type = TME_SUN2_PGTYPE_OBMEM;
  }

  /* add in the page offset to finish the address: */
  address |= *_address & (TME_SUN2_PAGE_SIZE - 1);
  *_address = address;

  /* if this is obio: */
  if (bus_type == TME_SUN2_PGTYPE_OBIO) {
    conn_bus = sun2->tme_sun2_obio;
    bus_fault_handler = _tme_sun2_obio_fault_handler;
  }
  
  /* if this is obmem: */
  else if (bus_type == TME_SUN2_PGTYPE_OBMEM) {
    conn_bus = sun2->tme_sun2_obmem;
    bus_fault_handler = _tme_sun2_obmem_fault_handler;
  }

  /* if this is the VME bus: */
  else if (sun2->tme_sun2_has_vme) {
    
    if (bus_type == TME_SUN2_PGTYPE_VME8) {
      address |= 0x800000;
    }
    else {
      assert(bus_type == TME_SUN2_PGTYPE_VME0);
    }

    bus_fault_handler = _tme_sun2_vmebus_fault_handler;

    /* TBD: */
    abort();
  }

  /* if this is mbmem: */
  else if (bus_type == TME_SUN2_PGTYPE_MBMEM) {
    conn_bus = sun2->tme_sun2_mbmem;
    bus_fault_handler = _tme_sun2_multibus_fault_handler;
  }

  /* otherwise, this is mbio: */
  else {
    assert(bus_type == TME_SUN2_PGTYPE_MBIO);
    conn_bus = sun2->tme_sun2_mbio;
    bus_fault_handler = _tme_sun2_multibus_fault_handler;
  }

  /* call the bus TLB filler: */
  rc = ((*conn_bus->tme_bus_tlb_fill)
	(conn_bus, tlb, address, cycles));

  /* if the bus TLB filler succeeded, add our bus fault handler: */
  if (rc == TME_OK) {
    TME_BUS_TLB_FAULT_HANDLER(tlb, bus_fault_handler, sun2);
  }

  return (rc);
}

/* this gets a PTE from the MMU: */
int
_tme_sun2_mmu_pte_get(struct tme_sun2 *sun2, tme_uint32_t address, tme_uint32_t *_pte_sun2)
{
  struct tme_sun_mmu_pte pte;
  tme_uint32_t pte_sun2;
  unsigned int pte_flags;
  int rc;

  /* get the PTE from the MMU: */
  rc = tme_sun_mmu_pte_get(sun2->tme_sun2_mmu, 
			   sun2->tme_sun2_context_user,
			   address,
			   &pte);
  assert(rc == TME_OK);
    
  /* form the Sun-2 PTE: */
  pte_sun2 = pte.tme_sun_mmu_pte_raw;
  pte_flags = pte.tme_sun_mmu_pte_flags;
  if (pte_flags & TME_SUN_MMU_PTE_REF) {
    pte_sun2 |= TME_SUN2_PTE_REF;
  }
  if (pte_flags & TME_SUN_MMU_PTE_MOD) {
    pte_sun2 |= TME_SUN2_PTE_MOD;
  }

  /* done: */
  *_pte_sun2 = pte_sun2;
  tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	  (TME_SUN2_LOG_HANDLE(sun2),
	   _("pte_get: PGMAP[%d:0x%08x] -> 0x%08x"), 
	   sun2->tme_sun2_context_user,
	   address,
	   pte_sun2));
  return (TME_OK);
}

/* this sets a PTE into the MMU: */
int
_tme_sun2_mmu_pte_set(struct tme_sun2 *sun2, tme_uint32_t address, tme_uint32_t pte_sun2)
{
  struct tme_sun_mmu_pte pte;
  unsigned int pte_flags;
#ifndef TME_NO_LOG
  const char *bus_name;
  tme_bus_addr32_t physical_address;
      
  /* this silences gcc -Wuninitialized: */
  bus_name = NULL;

  /* log this setting: */
  physical_address = ((pte_sun2 & TME_SUN2_PTE_PGFRAME) << TME_SUN2_PAGE_SIZE_LOG2);
  switch ((pte_sun2 & TME_SUN2_PTE_PGTYPE) / (TME_SUN2_PTE_PGTYPE / TME_SUN2_PTE_PGTYPE_MASK)) {
  case TME_SUN2_PGTYPE_OBMEM: bus_name = "obmem"; break;
  case TME_SUN2_PGTYPE_OBIO: bus_name = "obio"; break;
  case TME_SUN2_PGTYPE_MBMEM:
    if (sun2->tme_sun2_has_vme) {
      bus_name = "VME";
    }
    else {
      bus_name = "mbmem";
    }
    break;
  case TME_SUN2_PGTYPE_MBIO:
    if (sun2->tme_sun2_has_vme) {
      bus_name = "VME";
      physical_address |= 0x800000;
    }
    else {
      bus_name = "mbio";
    }
    break;
  }
  tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	  (TME_SUN2_LOG_HANDLE(sun2),
	   _("pte_set: PGMAP[%d:0x%08x] <- 0x%08x (%s 0x%08x)"), 
	   sun2->tme_sun2_context_user,
	   address,
	   pte_sun2,
	   bus_name,
	   physical_address));
#endif /* !TME_NO_LOG */

  pte.tme_sun_mmu_pte_raw = pte_sun2;
      
  pte_flags = 0;
  if (pte_sun2 & TME_SUN2_PTE_MOD) {
    pte_flags |= TME_SUN_MMU_PTE_MOD;
  }
  if (pte_sun2 & TME_SUN2_PTE_REF) {
    pte_flags |= TME_SUN_MMU_PTE_REF;
  }
  switch (pte_sun2 & TME_SUN2_PTE_PROT) {

    /* with this protection, the system can read and write,
       and the user gets a protection error: */
  case 0x70000000:
  case 0x74000000:
  case 0x60000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RW)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_ERROR));
    break;

    /* with this protection, the system gets a protection error,
       and the user gets a protection error: */
  case 0x30000000:
  case 0x20000000:
  case 0x10000000:
  case 0x00000000:
  case 0x04000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_ERROR)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_ERROR));
    break;

    /* with this protection, the system can read and write,
       and the user can read and write: */
  case 0x7C000000:
  case 0x6C000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RW)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RW));
    break;

    /* with this protection, the system can read and write,
       and the user can read: */
  case 0x78000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RW)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RO));
    break;

    /* with this protection, the system can read,
       and the user can read: */
  case 0x58000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RO)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RO));
    break;

    /* with this protection, the system can read,
       and the user can read and write: */
  case 0x5C000000:
  case 0x4C000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RO)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RW));
    break;

    /* with this protection, the system can read,
       and the user gets a protection error: */
  case 0x50000000:
  case 0x40000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_RO)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_ERROR));
    break;

    /* with this protection, the system gets a protection error,
       and the user can read and write: */
  case 0x3c000000:
  case 0x0c000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_ERROR)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RW));
    break;

    /* with this protection, the system gets a protection error,
       and the user can read: */
  case 0x08000000:
    pte_flags |= 
      (TME_SUN_MMU_PTE_PROT_SYSTEM(TME_SUN_MMU_PTE_PROT_ERROR)
       | TME_SUN_MMU_PTE_PROT_USER(TME_SUN_MMU_PTE_PROT_RO));
    break;

  default: abort();
  }
  if (pte_sun2 & TME_SUN2_PTE_VALID) {
    pte_flags |= TME_SUN_MMU_PTE_VALID;
  }
  pte.tme_sun_mmu_pte_flags = pte_flags;
  
  return (tme_sun_mmu_pte_set(sun2->tme_sun2_mmu, 
			      sun2->tme_sun2_context_user,
			      address,
			      &pte));
}

/* this is called when the system context register is set: */
void
_tme_sun2_mmu_context_system_set(struct tme_sun2 *sun2)
{
  /* system context register changes are assumed to be rare.  if they
     were frequent, we'd have to allocate 64 TLB sets for each TLB
     user - one for each possible combination of user context and
     system context.  instead, when the system context register
     changes, we simply invalidate all TLB entries everywhere: */
  tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	  (TME_SUN2_LOG_HANDLE(sun2),
	   _("system context now #%d"),
	   sun2->tme_sun2_context_system));
  tme_sun_mmu_tlbs_invalidate(sun2->tme_sun2_mmu);
}

/* this is called when the user context register is set: */
void
_tme_sun2_mmu_context_user_set(struct tme_sun2 *sun2)
{
  tme_bus_context_t context_base;

  /* NB that even though user and supervisor references use the two
     different context registers simultaneously, TLB entries for one
     are never usable by the other.  because of this, and because we
     assume that the system context register rarely changes, we choose
     not to factor the system context into the context seen by the
     m68k: */

  /* there are sixteen total contexts.  contexts zero through seven
     are the not-boot (normal) contexts.  contexts eight through
     fifteen are the same contexts, but in the boot state.

     in the boot state, TLB fills for supervisor program references
     bypass the MMU and are filled to reference the PROM, and data
     fills are filled as normal using the current context:  */

  /* in the not-boot (i.e., normal, state): */
  if (__tme_predict_true(sun2->tme_sun2_enable & TME_SUN2_ENA_NOTBOOT)) {

    tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	    (TME_SUN2_LOG_HANDLE(sun2),
	     _("user context now #%d"),
	     sun2->tme_sun2_context_user));

    /* the normal state contexts are numbered from zero: */
    context_base = 0;
  }

  /* in the boot state: */
  else {

    tme_log(TME_SUN2_LOG_HANDLE(sun2), 1000, TME_OK,
	    (TME_SUN2_LOG_HANDLE(sun2),
	     _("user context now #%d (boot state)"),
	     sun2->tme_sun2_context_user));

    /* the boot state contexts are numbered from eight: */
    context_base = TME_SUN2_CONTEXT_COUNT;
  }

  /* update the m68k bus context register: */
  *sun2->tme_sun2_m68k_bus_context
    = (context_base
       + sun2->tme_sun2_context_user);

  /* NB: unlike the sun3, sun2 DVMA TLBS are always filled using the
     system context, meaning they don't need to be invalidated when
     the user context changes, so we don't need to call
     tme_sun_mmu_context_switched() here: */
}

/* this adds a new TLB set: */
int
_tme_sun2_mmu_tlb_set_add(struct tme_bus_connection *conn_bus_asker,
			  struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_sun2 *sun2;
  int rc;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) conn_bus_asker->tme_bus_connection.tme_connection_element->tme_element_private;

  /* add the TLB set to the MMU: */
  rc = tme_sun_mmu_tlb_set_add(sun2->tme_sun2_mmu,
			       tlb_set_info);
  assert (rc == TME_OK);

  /* if this is the TLB set from the m68k: */
  if (conn_bus_asker->tme_bus_connection.tme_connection_type == TME_CONNECTION_BUS_M68K) {

    /* the m68k must expose a bus context register: */
    assert (tlb_set_info->tme_bus_tlb_set_info_bus_context != NULL);

    /* save the pointer to the m68k bus context register, and
       initialize it: */
    sun2->tme_sun2_m68k_bus_context
      = tlb_set_info->tme_bus_tlb_set_info_bus_context;
    _tme_sun2_mmu_context_user_set(sun2);

    /* return the maximum context number.  there are eight
       contexts in the not-boot (normal) state, and each of
       them has a boot state counterpart: */
    tlb_set_info->tme_bus_tlb_set_info_bus_context_max
      = (TME_SUN2_CONTEXT_COUNT
	 + TME_SUN2_CONTEXT_COUNT
	 - 1);
  }

  return (rc);
}

/* this creates a Sun-2 MMU: */
void
_tme_sun2_mmu_new(struct tme_sun2 *sun2)
{
  struct tme_sun_mmu_info mmu_info;

  memset(&mmu_info, 0, sizeof(mmu_info));
  mmu_info.tme_sun_mmu_info_element = sun2->tme_sun2_element;
  mmu_info.tme_sun_mmu_info_address_bits = 24;
  mmu_info.tme_sun_mmu_info_pgoffset_bits = TME_SUN2_PAGE_SIZE_LOG2;
  mmu_info.tme_sun_mmu_info_pteindex_bits = 4;
  mmu_info.tme_sun_mmu_info_contexts = TME_SUN2_CONTEXT_COUNT;
  mmu_info.tme_sun_mmu_info_pmegs = 256;
  mmu_info.tme_sun_mmu_info_tlb_fill_private = sun2;
  mmu_info.tme_sun_mmu_info_tlb_fill = _tme_sun2_tlb_fill_mmu;
  mmu_info.tme_sun_mmu_info_proterr_private = sun2;
  mmu_info.tme_sun_mmu_info_proterr = _tme_sun2_mmu_proterr;
  mmu_info.tme_sun_mmu_info_invalid_private = sun2;
  mmu_info.tme_sun_mmu_info_invalid = _tme_sun2_mmu_invalid;
  sun2->tme_sun2_mmu = tme_sun_mmu_new(&mmu_info);
}
