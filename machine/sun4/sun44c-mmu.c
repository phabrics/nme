/* $Id: sun44c-mmu.c,v 1.4 2009/08/30 14:05:10 fredette Exp $ */

/* machine/sun4/sun44c-mmu.c - implementation of Sun 4/4c MMU emulation: */

/*
 * Copyright (c) 2005, 2006 Matt Fredette
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
_TME_RCSID("$Id: sun44c-mmu.c,v 1.4 2009/08/30 14:05:10 fredette Exp $");

/* includes: */
#include "sun4-impl.h"

/* macros: */

/* real sun4/4c PTE page types: */
#define TME_SUN44C_PGTYPE_OBMEM		(0)
#define TME_SUN44C_PGTYPE_OBIO		(1)
#define TME_SUN4_PGTYPE_VME_D16		(2)
#define TME_SUN4_PGTYPE_VME_D32		(3)

/* real sun4 bus error register bits: */
#define TME_SUN4_BUSERR_WATCHDOG	TME_BIT(0)	/* watchdog or user reset */
#define TME_SUN4_BUSERR_SIZE		TME_BIT(1)	/* size error */
					/* bit 2 unused */
					/* bit 3 unused */
#define TME_SUN4_BUSERR_VMEBUSERR	TME_BIT(4)	/* VME bus error */
#define TME_SUN4_BUSERR_TIMEOUT		TME_BIT(5)	/* timeout error */
#define TME_SUN4_BUSERR_PROTERR		TME_BIT(6)	/* MMU protection error */
#define TME_SUN4_BUSERR_INVALID		TME_BIT(7)	/* MMU page invalid error */

/* real sun4c synchronous error register bits: */
#define TME_SUN4C_SYNC_ERR_WATCHDOG	TME_BIT(0)	/* watchdog or user reset */
#define TME_SUN4C_SYNC_ERR_SIZE		TME_BIT(1)	/* size error */
					/* bit 2 unused */
#define TME_SUN4C_SYNC_ERR_MEMORY	TME_BIT(3)	/* memory error */
#define TME_SUN4C_SYNC_ERR_SBUS		TME_BIT(4)	/* SBus error */
#define TME_SUN4C_SYNC_ERR_TIMEOUT	TME_BIT(5)	/* timeout error */
#define TME_SUN4C_SYNC_ERR_PROTERR	TME_BIT(6)	/* MMU protection error */
#define TME_SUN4C_SYNC_ERR_INVALID	TME_BIT(7)	/* MMU page invalid error */
#define TME_SUN4C_SYNC_ERR_WRITE	TME_BIT(15)	/* error happened on write */

/* real sun4c asynchronous error register bits: */
#define TME_SUN4C_ASYNC_ERR_MULTIPLE	TME_BIT(0)	/* multiple errors detected */
#define TME_SUN4C_ASYNC_ERR_SBUS	TME_BIT(1)	/* SBus error */
					/* bit 2 unused */
#define TME_SUN4C_ASYNC_ERR_MEMORY	TME_BIT(3)	/* memory error */
#define TME_SUN4C_ASYNC_ERR_DVMA	TME_BIT(4)	/* DVMA error */
#define TME_SUN4C_ASYNC_ERR_TIMEOUT	TME_BIT(5)	/* timeout error */
#define TME_SUN4C_ASYNC_ERR_PROTERR	TME_BIT(6)	/* MMU protection error */
#define TME_SUN4C_ASYNC_ERR_INVALID	TME_BIT(7)	/* MMU page invalid error (not 4/60?) */
#define TME_SUN4C_ASYNC_ERR_SIZE_MASK	(0x0300)	/* log2 of access size */

/* the real maximum number of contexts a sun4/4c MMU can have: */
#define TME_SUN44C_CONTEXT_COUNT_MAX	(16)

/* common bus error bits: */
#define TME_SUN44C_BUSERR_COMMON_INVALID	TME_BIT(0)
#define TME_SUN44C_BUSERR_COMMON_PROTERR	TME_BIT(1)
#define TME_SUN44C_BUSERR_COMMON_TIMEOUT	TME_BIT(2)
#define TME_SUN44C_BUSERR_COMMON_MEMORY		TME_BIT(3)
#define TME_SUN4C_BUSERR_COMMON_SBUS		TME_BIT(4)
#define TME_SUN4_BUSERR_COMMON_VMEBUS		TME_BIT(5)
#define TME_SUN4C_BUSERR_COMMON_PGTYPE		TME_BIT(6)

/* this logs a bus error: */
static inline void
_tme_sun44c_buserr_log(struct tme_sun4 *sun4,
		       tme_uint32_t vaddr,
		       const struct tme_bus_cycle *cycle,
		       unsigned int async,
		       tme_uint32_t common_err,
		       tme_uint32_t spec_err)
{
  struct tme_sun_mmu_pte pte;
  tme_uint32_t pte_sun44c;
  tme_uint32_t paddr;
  const char *bus_name;
  const char *err_type;
  const char *err_name;
  int rc;

  /* get the PTE involved.  NB we wrap this call so this entire
     function will get optimized away under TME_NO_LOG: */
  /* XXX FIXME - this uses the system context register, which may not
     be right for DVMA? */
#ifndef TME_NO_LOG
  rc = tme_sun_mmu_pte_get(sun4->tme_sun44c_mmu, 
			   sun4->tme_sun44c_context,
			   vaddr,
			   &pte);
#else  /* TME_NO_LOG */
  rc = TME_OK;
  pte.tme_sun_mmu_pte_raw = 0;
#endif /* TME_NO_LOG */
  assert (rc == TME_OK);
  pte_sun44c = pte.tme_sun_mmu_pte_raw;

  /* get the physical address: */
  if (TME_SUN4_IS_SUN4C(sun4)) {
    paddr = (((pte_sun44c & TME_SUN4C_PTE_PGFRAME) * TME_SUN4C_PAGE_SIZE)
	     | (vaddr % TME_SUN4C_PAGE_SIZE));
  }
  else {
    paddr = (((pte_sun44c & TME_SUN4_PTE_PGFRAME) * TME_SUN4_PAGE_SIZE)
	     | (vaddr % TME_SUN4_PAGE_SIZE));
  }

  /* this silences gcc -Wuninitialized: */
  bus_name = NULL;

  /* get the bus name: */
  switch (TME_FIELD_MASK_EXTRACTU(pte_sun44c, TME_SUN44C_PTE_PGTYPE)) {
  case TME_SUN44C_PGTYPE_OBMEM:
    bus_name = "obmem";
    break;
  case TME_SUN44C_PGTYPE_OBIO:
    if (TME_SUN4_IS_SUN4C(sun4)) {
      paddr |= 0xf0000000;
      bus_name = (paddr >= TME_SUN4C_OBIO_SBUS
		  ? "SBus"
		  : "mainbus");
    }
    else {
      bus_name = "obio";
    }
    break;
  case TME_SUN4_PGTYPE_VME_D16:
    bus_name = (TME_SUN4_IS_SUN4C(sun4) ? "TYPE_2" : "VME_D16");
    break;
  case TME_SUN4_PGTYPE_VME_D32:
    bus_name = (TME_SUN4_IS_SUN4C(sun4) ? "TYPE_3" : "VME_D32");
    break;
  }

  /* get the error type and name: */
  err_type = (TME_SUN4_IS_SUN4C(sun4)
	      ? (async
		 ? "async "
		 : "sync ")
	      : "");
  err_name = "other";
  if (common_err & TME_SUN44C_BUSERR_COMMON_TIMEOUT) err_name = "timeout";
  if (common_err & TME_SUN44C_BUSERR_COMMON_MEMORY) err_name = "memory";
  if (common_err & TME_SUN44C_BUSERR_COMMON_INVALID) err_name = "page invalid";
  if (common_err & TME_SUN44C_BUSERR_COMMON_PROTERR) err_name = "page protection";

  /* log this bus error: */
  tme_log(TME_SUN4_LOG_HANDLE(sun4), 500, TME_OK,
	  (TME_SUN4_LOG_HANDLE(sun4), 
	   _("%s%s buserr, virtual 0x%08x, %s 0x%08x, %serr = 0x%02x"),
	   err_type,
	   err_name,
	   vaddr,
	   bus_name,
	   paddr,
	   err_type,
	   spec_err));
}

/* our sun4/4c common bus error handler: */
static int
_tme_sun44c_buserr_common(const void *_conn_bus_init,
			  const struct tme_bus_tlb *tlb,
			  const struct tme_bus_cycle *cycle,
			  unsigned int common_err)
{
  const struct tme_bus_connection *conn_bus_init;
  struct tme_sun4 *sun4;
  tme_uint32_t vaddr;
  unsigned int log2_size;
  tme_uint32_t async_err;
  tme_uint32_t sync_err;

  /* recover the initiator's bus connection and sun4: */
  conn_bus_init = (struct tme_bus_connection *) _conn_bus_init;
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the virtual address.  certain errors, like memory errors,
     still allow the cycle to complete, and for those we have to
     subtract the cycle size from the post-cycle address: */
  vaddr = cycle->tme_bus_cycle_address;
  if (tlb != NULL) {
    vaddr -= tlb->tme_bus_tlb_addr_offset;
  }
  if (common_err & TME_SUN44C_BUSERR_COMMON_MEMORY) {
    vaddr -= cycle->tme_bus_cycle_size;
  }

  /* calculate the log2 of the cycle size: */
  for (log2_size = 0;
       (1 << log2_size) < cycle->tme_bus_cycle_size;
       log2_size++);

  /* if this is a sun4c: */
  if (TME_SUN4_IS_SUN4C(sun4)) {

    /* if this is any cycle not initiated by the CPU, or if this is a
       CPU write cycle that was not faulted by the MMU: */
    if (conn_bus_init->tme_bus_connection.tme_connection_type != TME_CONNECTION_BUS_SPARC
	|| (cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
	    && !(common_err
		 & (TME_SUN44C_BUSERR_COMMON_INVALID
		    | TME_SUN44C_BUSERR_COMMON_PROTERR
		    | TME_SUN4C_BUSERR_COMMON_PGTYPE)))) {

      /* this is an asynchronous error: */
      async_err = 0;
      if (common_err & TME_SUN44C_BUSERR_COMMON_TIMEOUT) async_err |= TME_SUN4C_ASYNC_ERR_TIMEOUT;
      if (common_err & TME_SUN44C_BUSERR_COMMON_MEMORY) async_err |= TME_SUN4C_ASYNC_ERR_MEMORY;
      if (common_err & TME_SUN4C_BUSERR_COMMON_SBUS) async_err |= TME_SUN4C_ASYNC_ERR_SBUS;
      if (common_err & TME_SUN44C_BUSERR_COMMON_INVALID) async_err |= TME_SUN4C_ASYNC_ERR_INVALID;
      if (common_err & TME_SUN44C_BUSERR_COMMON_PROTERR) async_err |= TME_SUN4C_ASYNC_ERR_PROTERR;
      if (conn_bus_init->tme_bus_connection.tme_connection_type != TME_CONNECTION_BUS_SPARC) {
	async_err |= TME_SUN4C_ASYNC_ERR_DVMA;
      }

      /* if this is the first asynchronous error: */
      if (sun4->tme_sun4c_async_err == 0) {

	/* set the asynchronous virtual address register: */
	sun4->tme_sun4c_async_vaddr = vaddr;

	/* add the cycle size to the asynchronous error register value: */
	TME_FIELD_MASK_DEPOSITU(async_err, TME_SUN4C_ASYNC_ERR_SIZE_MASK, log2_size);
      }

      /* otherwise, this is not the first asynchronous error: */
      else {

	/* there are multiple asynchronous errors: */
	async_err |= TME_SUN4C_ASYNC_ERR_MULTIPLE;
      }

      /* update the asynchronous error register: */
      sun4->tme_sun4c_async_err |= async_err;

      /* send an NMI to the CPU: */
      sun4->tme_sun4_int_signals[TME_SPARC_IPL_NMI / 8] |= TME_BIT(TME_SPARC_IPL_NMI % 8);
      _tme_sun4_ipl_check(sun4);

      /* log this bus error: */
      _tme_sun44c_buserr_log(sun4,
			     vaddr,
			     cycle,
			     TRUE,
			     common_err,
			     async_err);

      /* asynchronous errors aren't reported to the CPU as faults, but
	 they are reported as faults to another bus master (for whom
	 the error is really synchronous): */
      return (conn_bus_init->tme_bus_connection.tme_connection_type == TME_CONNECTION_BUS_SPARC
	      ? TME_OK
	      : (common_err & TME_SUN44C_BUSERR_COMMON_MEMORY)
	      ? EIO
	      : (common_err & TME_SUN44C_BUSERR_COMMON_TIMEOUT)
	      ? ENOENT
	      : EFAULT);
    }

    /* this is a synchronous error.  NB that the cycle is only
       considered a write if it's only a write cycle; read cycles and
       all parts of read/modify/write cycles are considered reads: */
    sync_err = 0;
    if (common_err & TME_SUN44C_BUSERR_COMMON_TIMEOUT) sync_err |= TME_SUN4C_SYNC_ERR_TIMEOUT;
    if (common_err & TME_SUN44C_BUSERR_COMMON_MEMORY) sync_err |= TME_SUN4C_SYNC_ERR_MEMORY;
    if (common_err & TME_SUN4C_BUSERR_COMMON_SBUS) sync_err |= TME_SUN4C_SYNC_ERR_SBUS;
    if (common_err & TME_SUN44C_BUSERR_COMMON_INVALID) sync_err |= TME_SUN4C_SYNC_ERR_INVALID;
    if (common_err & TME_SUN44C_BUSERR_COMMON_PROTERR) sync_err |= TME_SUN4C_SYNC_ERR_PROTERR;
    if (cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {
      sync_err |= TME_SUN4C_SYNC_ERR_WRITE;
    }
    
    /* set the synchronous virtual address register: */
    sun4->tme_sun4c_sync_vaddr = vaddr;

    /* update the synchronous error register: */
    sun4->tme_sun4c_sync_err
      = ((sun4->tme_sun4c_sync_err
	  & ~TME_SUN4C_SYNC_ERR_WRITE)
	 | sync_err);
    sync_err = sun4->tme_sun4c_sync_err;
  }

  /* otherwise, this is a sun4: */
  else {

    /* this is a synchronous bus error: */
    sync_err = 0;
    if (common_err & TME_SUN44C_BUSERR_COMMON_TIMEOUT) sync_err |= TME_SUN4_BUSERR_TIMEOUT;
    if (common_err & TME_SUN4_BUSERR_COMMON_VMEBUS) sync_err |= TME_SUN4_BUSERR_VMEBUSERR;
    if (common_err & TME_SUN44C_BUSERR_COMMON_INVALID) sync_err |= TME_SUN4_BUSERR_INVALID;
    if (common_err & TME_SUN44C_BUSERR_COMMON_PROTERR) sync_err |= TME_SUN4_BUSERR_PROTERR;

    /* set the bus error register: */
    sun4->tme_sun4_buserr = sync_err;
  }

  /* log this bus error: */
  _tme_sun44c_buserr_log(sun4,
			 vaddr,
			 cycle,
			 FALSE,
			 common_err,
			 sync_err);

  /* return a bus fault code: */
  return ((common_err & TME_SUN44C_BUSERR_COMMON_MEMORY)
	  ? EIO
	  : (common_err & TME_SUN44C_BUSERR_COMMON_TIMEOUT)
	  ? ENOENT
	  : EFAULT);
}

/* this maps a bus fault code to a common bus error: */
static inline unsigned int
_tme_sun44c_bus_fault_error(int rc)
{
  switch (rc) {
  default: abort();
  case ENOENT: return (TME_SUN44C_BUSERR_COMMON_TIMEOUT);
  case EIO: return (TME_SUN44C_BUSERR_COMMON_MEMORY);
  }
}

/* our page-invalid cycle handler: */
static int
_tme_sun44c_mmu_invalid(void *_conn_bus_init, struct tme_bus_cycle *cycle)
{

  /* call the common bus error handler: */
  return (_tme_sun44c_buserr_common(_conn_bus_init,
				    NULL,
				    cycle,
				    TME_SUN44C_BUSERR_COMMON_INVALID));
}

/* our protection error cycle handler: */
int
_tme_sun44c_mmu_proterr(void *_conn_bus_init, struct tme_bus_cycle *cycle)
{

  /* call the common bus error handler: */
  return (_tme_sun44c_buserr_common(_conn_bus_init,
				    NULL,
				    cycle,
				    TME_SUN44C_BUSERR_COMMON_PROTERR));
}

/* the sun4/4c obio and obmem bus fault handler: */
int
_tme_sun44c_ob_fault_handler(void *_conn_bus_init, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{

  /* call the common bus error handler: */
  return (_tme_sun44c_buserr_common(_conn_bus_init,
				    tlb,
				    cycle,
				    _tme_sun44c_bus_fault_error(rc)));
}

/* the sun4c obmem bus fault handler: */
static int
_tme_sun4c_obmem_fault_handler(void *_conn_bus_init, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{
  tme_uint8_t *buffer;
  unsigned int bytes;

  /* sun4c obmem (at least on an SS2) apparently doesn't give timeout
     errors, because while an SS2 PROM's memory probe code seems to
     tolerate faults, it never clears the synchronous error register
     when they happen, which causes problems in later self tests that
     check that register: */
  if (rc == ENOENT) {

    /* nonexistent obmem discards writes and reads as all-bits-one: */
    if (cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {
      for (bytes = cycle->tme_bus_cycle_size, buffer = cycle->tme_bus_cycle_buffer;
	   bytes > 0;
	   bytes--, buffer += cycle->tme_bus_cycle_buffer_increment) {
	*buffer = 0xff;
      }
    }

    return (TME_OK);
  }

  /* call the common bus error handler: */
  return (_tme_sun44c_buserr_common(_conn_bus_init,
				    tlb,
				    cycle,
				    _tme_sun44c_bus_fault_error(rc)));
}

/* the sun4c sbus fault handler: */
static int
_tme_sun4c_sbus_fault_handler(void *_conn_bus_init, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{

  /* call the common bus error handler: */
  return (_tme_sun44c_buserr_common(_conn_bus_init,
				    tlb,
				    cycle,
				    (TME_SUN4C_BUSERR_COMMON_SBUS
				     | _tme_sun44c_bus_fault_error(rc))));
}

/* the sun4c page type (type-2 and type-3) fault handler: */
static int
_tme_sun4c_pgtype_fault_handler(void *_conn_bus_init, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{

  /* call the common bus error handler: */
  return (_tme_sun44c_buserr_common(_conn_bus_init,
				    tlb,
				    cycle,
				    (TME_SUN4C_BUSERR_COMMON_PGTYPE
				     | _tme_sun44c_bus_fault_error(rc))));
}

/* the sun4 VMEbus fault handler: */
static int
_tme_sun4_vmebus_fault_handler(void *_conn_bus_init, struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{

  /* call the common bus error handler: */
  return (_tme_sun44c_buserr_common(_conn_bus_init,
				    tlb,
				    cycle,
				    (TME_SUN4_BUSERR_COMMON_VMEBUS
				     | _tme_sun44c_bus_fault_error(rc))));
}

/* our bus timeout cycle handler: */
static int
_tme_sun44c_bus_timeout(void *_sun4, struct tme_bus_cycle *cycle)
{
  return (ENOENT);
}

/* this fills memory TLBs from the MMU: */
int
_tme_sun44c_tlb_fill_mmu(const struct tme_bus_connection *conn_bus_init,
			 struct tme_bus_tlb *tlb,
			 tme_uint32_t *_asi_mask,
			 tme_uint32_t address,
			 unsigned int cycles)
{
  struct tme_sun4 *sun4;
  tme_uint32_t asi_mask;
  tme_uint32_t asi_mask_si;
  unsigned short access;
  struct tme_bus_tlb tlb_bus;
  unsigned short tlb_flags;

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

  /* assume that if this TLB entry ends up good for the supervisor,
     it's good for the supervisor instruction ASI mask: */
  asi_mask_si = TME_SPARC32_ASI_MASK_SI;

  /* if we're in the boot state: */
  if (__tme_predict_false((sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT) == 0)) {

    /* if this is the supervisor instruction ASI: */
    if (asi_mask == TME_SPARC32_ASI_MASK_SI) {

      /* fill this TLB entry directly from the obio (sun4c, sbus) or
         obmem (sun4) bus: */
      if (TME_SUN4_IS_SUN4C(sun4)) {
	(*sun4->tme_sun4_32_obio->tme_bus_tlb_fill)
	  (sun4->tme_sun4_32_obio,
	   tlb,
	   TME_SUN44C_PROM_BASE | (address & (TME_SUN44C_PROM_SIZE - 1)),
	   cycles);
      }
      else {
	(*sun4->tme_sun4_32_obmem->tme_bus_tlb_fill)
	  (sun4->tme_sun4_32_obmem,
	   tlb,
	   TME_SUN44C_PROM_BASE | (address & (TME_SUN44C_PROM_SIZE - 1)),
	   cycles);
      }
	
      /* create the mapping TLB entry: */
      tlb_bus.tme_bus_tlb_addr_first = address & (((tme_bus_addr32_t) 0) - TME_SUN44C_PROM_SIZE);
      tlb_bus.tme_bus_tlb_addr_last = address | (TME_SUN44C_PROM_SIZE - 1);
      tlb_bus.tme_bus_tlb_cycles_ok
	= TME_BUS_CYCLE_READ;
  
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, TME_SUN44C_PROM_BASE | (address % TME_SUN44C_PROM_SIZE), &tlb_bus, address);
	
      /* this is good for the supervisor instruction ASI only: */
      *_asi_mask = TME_SPARC32_ASI_MASK_SI;

      /* done: */
      return(TME_OK);
    }

    /* this should be the supervisor data ASI only: */
    assert (asi_mask == TME_SPARC32_ASI_MASK_SD);

    /* if this TLB entry ends up good for the supervisor, it's not
       good for the supervisor instruction ASI: */
    asi_mask_si = 0;
  }

  /* thread the initiator's bus connection down to
     _tme_sun44c_tlb_fill_pte(): */
  tlb->tme_bus_tlb_fault_handlers[0]
    .tme_bus_tlb_fault_handler_private = (void *) conn_bus_init;

  /* fill this TLB entry from the MMU: */
  access
    = ((cycles & TME_BUS_CYCLE_WRITE)
       ? TME_SUN_MMU_PTE_PROT_RW
       : TME_SUN_MMU_PTE_PROT_RO);
  access
    = ((asi_mask == TME_SPARC32_ASI_MASK_UD
	|| asi_mask == TME_SPARC32_ASI_MASK_UI)
       ? TME_SUN_MMU_PTE_PROT_USER(access)
       : TME_SUN_MMU_PTE_PROT_SYSTEM(access));
  tlb_flags = tme_sun_mmu_tlb_fill(sun4->tme_sun44c_mmu,
				   tlb,
				   TME_SUN44C_BUS_MMU_CONTEXT(sun4, conn_bus),
				   address,
				   access);

  /* this TLB entry is good for the program and instruction ASIs
     for the user and/or the supervisor: */
  *_asi_mask 
    = (((tlb_flags & TME_SUN_MMU_TLB_USER)
	? (TME_SPARC32_ASI_MASK_UD
	   | TME_SPARC32_ASI_MASK_UI)
	: 0)
       | ((tlb_flags & TME_SUN_MMU_TLB_SYSTEM)
	  ? (TME_SPARC32_ASI_MASK_SD
	     | asi_mask_si)
	  : 0));

  return (TME_OK);
}

/* our sparc TLB filler: */
int
_tme_sun44c_tlb_fill_sparc(struct tme_sparc_bus_connection *conn_sparc,
			   struct tme_sparc_tlb *tlb_sparc,
			   tme_uint32_t asi_mask,
			   tme_bus_addr_t address_wider,
			   unsigned int cycles)
{
  tme_uint32_t address;
  struct tme_sun4 *sun4;
  struct tme_bus_tlb *tlb;
  struct tme_bus_tlb tlb_bus;
 
  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_sparc->tme_sparc_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the generic bus TLB: */
  tlb = &tlb_sparc->tme_sparc_tlb_bus_tlb;

  /* if this is the for user or supervisor data or instruction address
     spaces: */
  if (__tme_predict_true(TME_SPARC_ASI_MASK_OVERLAP(asi_mask,
						    (TME_SPARC32_ASI_MASK_UI
						     | TME_SPARC32_ASI_MASK_SI
						     | TME_SPARC32_ASI_MASK_UD
						     | TME_SPARC32_ASI_MASK_SD)))) {

    /* call the current TLB filler: */
    tlb_sparc->tme_sparc_tlb_asi_mask = asi_mask;
    return ((*sun4->tme_sun4_tlb_fill)(&conn_sparc->tme_sparc_bus_connection,
				       tlb,
				       &tlb_sparc->tme_sparc_tlb_asi_mask,
				       address,
				       cycles));
  }


  /* assume that we need a TLB entry that allows reading and writing
     over the entire address space, using the control cycle handler: */
  tme_bus_tlb_initialize(tlb);
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = (((tme_uint32_t) 0) - 1);
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
  tlb_sparc->tme_sparc_tlb_asi_mask = asi_mask;
  tlb->tme_bus_tlb_cycle = _tme_sun44c_control_cycle_handler;
  tlb->tme_bus_tlb_cycle_private = &sun4->tme_sun4_asis[TME_SPARC_ASI_MASK_WHICH(asi_mask)];

  /* if this address space isn't defined: */
  if (__tme_predict_false(sun4->tme_sun4_asis[TME_SPARC_ASI_MASK_WHICH(asi_mask)].tme_sun4_asi_sun4 == NULL)) {
    abort();
  }

  /* if this is for control space: */
  if (__tme_predict_false(asi_mask == TME_SPARC_ASI_MASK_SPECIAL(TME_SUN4_32_ASI_CONTROL, TRUE))) {

    /* if this address is before the UART bypass: */
    if (__tme_predict_true(address < TME_SUN44C_CONTROL_UART_BYPASS)) {

      /* we cover the address space before the UART bypass: */
      tlb->tme_bus_tlb_addr_last = TME_SUN44C_CONTROL_UART_BYPASS - 1;
    }

    /* otherwise, this address is within the UART bypass: */
    else {

      /* fill this TLB entry directly from the obio bus: */
      (*sun4->tme_sun4_32_obio->tme_bus_tlb_fill)
	(sun4->tme_sun4_32_obio,
	 tlb,
	 (address % TME_SUN_Z8530_SIZE) + TME_SUN44C_OBIO_ZS0,
	 cycles);

      /* create the mapping TLB entry: */
      tlb_bus.tme_bus_tlb_addr_first = address & (((tme_uint32_t) 0) - TME_SUN_Z8530_SIZE);
      tlb_bus.tme_bus_tlb_addr_last = address | (TME_SUN_Z8530_SIZE - 1);
      tlb_bus.tme_bus_tlb_cycles_ok
	= (TME_BUS_CYCLE_READ
	   | TME_BUS_CYCLE_WRITE);
  
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, (address % TME_SUN_Z8530_SIZE) + TME_SUN44C_OBIO_ZS0, &tlb_bus, address);
    }
  }

  /* done: */
  return (TME_OK);
}

/* our bus TLB filler: */
int
_tme_sun44c_tlb_fill_bus(struct tme_bus_connection *conn_bus_init,
			 struct tme_bus_tlb *tlb,
			 tme_bus_addr_t address_wider,
			 unsigned int cycles)
{
  tme_uint32_t address;
  struct tme_sun4 *sun4;
  struct tme_sun4_bus_connection *conn_sun4;
  tme_uint32_t base, mask;
  tme_uint32_t asi_mask;
  struct tme_bus_tlb tlb_bus;
  unsigned int tlb_i;

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_bus_init->tme_bus_connection.tme_connection_element->tme_element_private;

  /* recover the internal sun4 mainbus, or sun4c board, connection: */
  conn_sun4 = (struct tme_sun4_bus_connection *) conn_bus_init;

  /* dispatch on the internal connection.  this turns the bus address
     into a DVMA base address and size, except for the register
     connections, which are handled specially: */
  switch (conn_sun4->tme_sun4_bus_connection_which) {

  case TME_SUN4_32_CONN_BUS_OBIO:
    if (TME_SUN4_IS_SUN4C(sun4)) {
      base = 0x00000000;
      mask = ((tme_uint32_t) 0) - 1;
    }
    else {
      abort();
    }
    break;

  case TME_SUN4_32_CONN_REG_TIMER:

    /* return a TLB entry that allows reading and writing the two timers: */
    tme_bus_tlb_initialize(tlb);
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = (TME_SUN44C_TIMER_SIZ_REG * 2) - 1;
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
    tlb->tme_bus_tlb_cycle_private = sun4;
    tlb->tme_bus_tlb_cycle = _tme_sun4_timer_cycle_control;
    return (TME_OK);

  case TME_SUN4_32_CONN_REG_INTREG:
  case TME_SUN4C4M_CONN_REG_AUXREG:

    /* return a TLB entry that allows reading and writing these 8-bit
       registers: */
    tme_bus_tlb_initialize(tlb);
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = sizeof(tme_uint8_t) - 1;
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
    tlb->tme_bus_tlb_cycle_private = sun4;
    tlb->tme_bus_tlb_cycle
      = (conn_sun4->tme_sun4_bus_connection_which == TME_SUN4C4M_CONN_REG_AUXREG
	 ? _tme_sun4c_auxreg_cycle_control
	 : _tme_sun44c_intreg_cycle_control);
    return (TME_OK);


  case TME_SUN4_32_CONN_REG_MEMERR:
    
    /* return a TLB entry that allows reading and writing the memory
       error register(s): */
    tme_bus_tlb_initialize(tlb);
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
    tlb->tme_bus_tlb_cycle_private = sun4;
    tlb->tme_bus_tlb_cycle = _tme_sun44c_memerr_cycle_control;

    /* the size of the memory error register(s) depends on
       the model: */
    tlb->tme_bus_tlb_addr_last
      = (TME_SUN4_IS_MODEL(sun4, TME_SUN_IDPROM_TYPE_CODE_CALVIN)
	 ? (TME_SUN44C_MEMERR_SIZ_REG * 2)
	 : TME_SUN44C_MEMERR_SIZ_REG) - 1;

    return (TME_OK);

  default: abort();
  }

  /* update the head pointer for the active SDVMA TLB entry list: */
  tlb_i = sun4->tme_sun44c_sdvma_tlb_next
    = ((sun4->tme_sun44c_sdvma_tlb_next
	+ 1)
       & (TME_SUN44C_SDVMA_TLBS - 1));

  /* if the new head pointer already has a TLB entry, and it doesn't
     happen to be the same as this TLB entry, invalidate it: */
  if (sun4->tme_sun44c_sdvma_tlb_tokens[tlb_i] != NULL
      && (sun4->tme_sun44c_sdvma_tlb_tokens[tlb_i]
	  != tlb->tme_bus_tlb_token)) {
    tme_token_invalidate(sun4->tme_sun44c_sdvma_tlb_tokens[tlb_i]);
  }

  /* add this TLB entry to the active list: */
  sun4->tme_sun44c_sdvma_tlb_tokens[tlb_i] = tlb->tme_bus_tlb_token;

  /* if system DVMA is disabled: */
  if (__tme_predict_false(!(sun4->tme_sun44c_enable & TME_SUN44C_ENA_SDVMA))) {

    /* return a TLB entry that will generate a bus fault: */
    tme_bus_tlb_initialize(tlb);
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = mask;
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
    tlb->tme_bus_tlb_cycle_private = sun4;
    tlb->tme_bus_tlb_cycle = _tme_sun44c_bus_timeout;
    TME_BUS_TLB_FAULT_HANDLER(tlb, 
			      (TME_SUN4_IS_SUN4C(sun4)
			       ? _tme_sun4c_sbus_fault_handler
			       : _tme_sun4_vmebus_fault_handler),
			      conn_bus_init);
    return (TME_OK);
  }

  assert (!(address & base)
	  && (address <= mask));

  /* call the current TLB filler: */
  asi_mask = TME_SPARC32_ASI_MASK_SD;
  (*sun4->tme_sun4_tlb_fill)(conn_bus_init,
			     tlb,
			     &asi_mask,
			     address,
			     cycles);

  /* this bus TLB entry depends on the current context: */
  tme_sun_mmu_context_add(sun4->tme_sun44c_mmu, tlb);

  /* create the mapping TLB entry.  we do this even if base == 0,
     because the TLB entry as currently filled may cover more address
     space than DVMA space on this machine is supposed to cover: */
  tlb_bus.tme_bus_tlb_addr_first = 0;
  tlb_bus.tme_bus_tlb_addr_last = mask;
  tlb_bus.tme_bus_tlb_cycles_ok
    = (TME_BUS_CYCLE_READ
       | TME_BUS_CYCLE_WRITE);
  
  /* map the filled TLB entry: */
  tme_bus_tlb_map(tlb, address | base, &tlb_bus, address);

  return (TME_OK);
}

/* our post-MMU TLB filler: */
static int
_tme_sun44c_tlb_fill_pte(void *_sun4,
			 struct tme_bus_tlb *tlb, 
			 struct tme_sun_mmu_pte *pte,
			 tme_uint32_t *_address,
			 unsigned int cycles)
{
  struct tme_sun4 *sun4;
  tme_uint32_t address;
  unsigned int bus_type;
  void *_conn_bus_init;
  struct tme_bus_connection *conn_bus_resp;
  tme_bus_fault_handler bus_fault_handler;
  int rc;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) _sun4;

  /* recover the initiator's bus connection.  this is threaded down
     from _tme_sun44c_tlb_fill_mmu(): */
  _conn_bus_init = 
    tlb->tme_bus_tlb_fault_handlers[0]
    .tme_bus_tlb_fault_handler_private;

  /* get the initial physical address and bus type: */
  address = pte->tme_sun_mmu_pte_raw;
  if (TME_SUN4_IS_SUN4C(sun4)) {
    address = (address & TME_SUN4C_PTE_PGFRAME) * TME_SUN4C_PAGE_SIZE;
    address += *_address % TME_SUN4C_PAGE_SIZE;
  }
  else {
    address = (address & TME_SUN4_PTE_PGFRAME) * TME_SUN4_PAGE_SIZE;
    address += *_address % TME_SUN4_PAGE_SIZE;
  }
  bus_type = TME_FIELD_MASK_EXTRACTU(pte->tme_sun_mmu_pte_raw, TME_SUN44C_PTE_PGTYPE);

  /* if this is obio: */
  if (bus_type == TME_SUN44C_PGTYPE_OBIO) {
    conn_bus_resp = sun4->tme_sun4_32_obio;
    bus_fault_handler = _tme_sun44c_ob_fault_handler;
    if (TME_SUN4_IS_SUN4C(sun4)) {
      address |= 0xf0000000;
      if (address >= TME_SUN4C_OBIO_SBUS) {
	bus_fault_handler = _tme_sun4c_sbus_fault_handler;
      }
    }
    else {
      abort();
    }
  }
  
  /* if this is obmem: */
  else if (bus_type == TME_SUN44C_PGTYPE_OBMEM) {
    if (TME_SUN4_IS_SUN4C(sun4)) {
      conn_bus_resp = sun4->tme_sun4_32_obio;
      bus_fault_handler = _tme_sun4c_obmem_fault_handler;
    }
    else {
      conn_bus_resp = sun4->tme_sun4_32_obmem;
      bus_fault_handler = _tme_sun44c_ob_fault_handler;
    }
  }

  /* if this is the VME bus: */
  else {
    assert ((bus_type == TME_SUN4_PGTYPE_VME_D16
	     || bus_type == TME_SUN4_PGTYPE_VME_D32));
    conn_bus_resp = sun4->tme_sun4_vmebus;
    bus_fault_handler = _tme_sun4_vmebus_fault_handler;

    /* SS2 PROMs will try to map type-2 and type-3 space to test
       synchronous timeouts: */
    if (TME_SUN4_IS_SUN4C(sun4)) {

      /* return the real physical address: */
      *_address = address;

      /* return a TLB entry that will generate a bus fault: */
      tme_bus_tlb_initialize(tlb);
      tlb->tme_bus_tlb_addr_first = 0;
      tlb->tme_bus_tlb_addr_last = (((tme_uint32_t) 0) - 1);
      tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
      tlb->tme_bus_tlb_cycle_private = sun4;
      tlb->tme_bus_tlb_cycle = _tme_sun44c_bus_timeout;
      TME_BUS_TLB_FAULT_HANDLER(tlb, _tme_sun4c_pgtype_fault_handler, _conn_bus_init);
      return (TME_OK);
    }
  }

  /* return the real physical address: */
  *_address = address;

  /* call the bus TLB filler: */
  rc = ((*conn_bus_resp->tme_bus_tlb_fill)
	(conn_bus_resp, tlb, address, cycles));

  /* if the bus TLB filler succeeded, add our bus fault handler: */
  if (rc == TME_OK) {
    TME_BUS_TLB_FAULT_HANDLER(tlb, bus_fault_handler, _conn_bus_init);
  }

  return (rc);
}

/* this gets a PTE from the MMU: */
int
_tme_sun44c_mmu_pte_get(struct tme_sun4 *sun4, tme_uint32_t address, tme_uint32_t *_pte_sun44c)
{
  struct tme_sun_mmu_pte pte;
  tme_uint32_t pte_sun44c;
  unsigned int pte_flags;
  int rc;

  /* get the PTE from the MMU: */
  rc = tme_sun_mmu_pte_get(sun4->tme_sun44c_mmu, 
			   sun4->tme_sun44c_context,
			   address,
			   &pte);
  assert(rc == TME_OK);
    
  /* form the Sun 4/4c PTE: */
  pte_sun44c = pte.tme_sun_mmu_pte_raw;
  pte_flags = pte.tme_sun_mmu_pte_flags;
  if (pte_flags & TME_SUN_MMU_PTE_REF) {
    pte_sun44c |= TME_SUN44C_PTE_REF;
  }
  if (pte_flags & TME_SUN_MMU_PTE_MOD) {
    pte_sun44c |= TME_SUN44C_PTE_MOD;
  }

  /* done: */
  *_pte_sun44c = pte_sun44c;
  tme_log(TME_SUN4_LOG_HANDLE(sun4), 1000, TME_OK,
	  (TME_SUN4_LOG_HANDLE(sun4),
	   _("pte_get: PGMAP[%d:0x%08x] -> 0x%08x"), 
	   sun4->tme_sun44c_context,
	   address,
	   pte_sun44c));
  return (TME_OK);
}

/* this sets a PTE into the MMU: */
int
_tme_sun44c_mmu_pte_set(struct tme_sun4 *sun4, tme_uint32_t address, tme_uint32_t pte_sun44c)
{
  struct tme_sun_mmu_pte pte;
  unsigned int pte_flags;
#ifndef TME_NO_LOG
  const char *bus_name;
  tme_bus_addr32_t physical_address;
      
  /* this silences gcc -Wuninitialized: */
  bus_name = NULL;

  /* log this setting: */
  if (TME_SUN4_IS_SUN4C(sun4)) {
    physical_address = (pte_sun44c & TME_SUN4C_PTE_PGFRAME) * TME_SUN4C_PAGE_SIZE;
  }
  else {
    physical_address = (pte_sun44c & TME_SUN4_PTE_PGFRAME) * TME_SUN4_PAGE_SIZE;
  }
  switch (TME_FIELD_MASK_EXTRACTU(pte_sun44c, TME_SUN44C_PTE_PGTYPE)) {
  case TME_SUN44C_PGTYPE_OBMEM: bus_name = "obmem"; break;
  case TME_SUN44C_PGTYPE_OBIO:
    if (TME_SUN4_IS_SUN4C(sun4)) {
      physical_address |= 0xf0000000;
      bus_name = (physical_address >= TME_SUN4C_OBIO_SBUS
		  ? "SBus"
		  : "mainbus");
    }
    else {
      bus_name = "obio";
    }
    break;
  case TME_SUN4_PGTYPE_VME_D16: bus_name = "VME_D16"; break;
  case TME_SUN4_PGTYPE_VME_D32: bus_name = "VME_D32"; break;
  }
  tme_log(TME_SUN4_LOG_HANDLE(sun4), 1000, TME_OK,
	  (TME_SUN4_LOG_HANDLE(sun4),
	   _("pte_set: PGMAP[%d:0x%08x] <- 0x%08x (%s 0x%08x)"), 
	   sun4->tme_sun44c_context,
	   address,
	   pte_sun44c,
	   bus_name,
	   physical_address));
#endif /* !TME_NO_LOG */

  /* store only the bits that the real hardware stores: */
  pte_sun44c
    &= (TME_SUN44C_PTE_VALID
	| TME_SUN44C_PTE_WRITE
	| TME_SUN44C_PTE_SYSTEM
	| TME_SUN44C_PTE_NC
	| TME_SUN44C_PTE_REF
	| TME_SUN44C_PTE_MOD
	| (TME_SUN4_IS_SUN4C(sun4)
	   ? (TME_SUN44C_PTE_PGTYPE
	      | TME_SUN4C_PTE_PGFRAME)
	   : (TME_SUN44C_PTE_PGTYPE
	      | TME_SUN4_PTE_PGFRAME)));

  pte.tme_sun_mmu_pte_raw = pte_sun44c;
      
  pte_flags = (pte_sun44c & TME_SUN44C_PTE_WRITE
	       ? TME_SUN_MMU_PTE_PROT_RW
	       : TME_SUN_MMU_PTE_PROT_RO);
  pte_flags = (TME_SUN_MMU_PTE_PROT_SYSTEM(pte_flags)
	       | TME_SUN_MMU_PTE_PROT_USER(pte_sun44c & TME_SUN44C_PTE_SYSTEM
					   ? TME_SUN_MMU_PTE_PROT_ERROR
					   : pte_flags));
  if (pte_sun44c & TME_SUN44C_PTE_MOD) {
    pte_flags |= TME_SUN_MMU_PTE_MOD;
  }
  if (pte_sun44c & TME_SUN44C_PTE_REF) {
    pte_flags |= TME_SUN_MMU_PTE_REF;
  }
  if (pte_sun44c & TME_SUN44C_PTE_VALID) {
    pte_flags |= TME_SUN_MMU_PTE_VALID;
  }
  pte.tme_sun_mmu_pte_flags = pte_flags;
  
  return (tme_sun_mmu_pte_set(sun4->tme_sun44c_mmu, 
			      sun4->tme_sun44c_context,
			      address,
			      &pte));
}

/* this is called when the SDVMA bit is changed in the enable register: */
void
_tme_sun44c_mmu_sdvma_change(struct tme_sun4 *sun4)
{
  unsigned int tlb_i;

  /* whenever the SDVMA bit changes, we have to invalidate all SDVMA
     TLB entries: */
  for (tlb_i = 0; tlb_i < TME_SUN44C_SDVMA_TLBS; tlb_i++) {
    if (sun4->tme_sun44c_sdvma_tlb_tokens[tlb_i] != NULL) {
      tme_token_invalidate(sun4->tme_sun44c_sdvma_tlb_tokens[tlb_i]);
      sun4->tme_sun44c_sdvma_tlb_tokens[tlb_i] = NULL;
    }
  }
}

/* this is called when the context register is set: */
void
_tme_sun44c_mmu_context_set(struct tme_sun4 *sun4)
{
  tme_bus_context_t context_base;

  /* there are up to (TME_SUN44C_CONTEXT_COUNT_MAX * 2) total
     contexts.  contexts zero through an implementation's last context
     number are the not-boot (normal) contexts.  the same number of
     contexts starting at TME_SUN44C_CONTEXT_COUNT_MAX are the same
     contexts, but in the boot state.
     
     in the boot state, TLB fills for supervisor program references
     bypass the MMU and are filled to reference the PROM, and data
     fills are filled as normal using the current context: */

  /* in the not-boot (i.e., normal, state): */
  if (__tme_predict_true(sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT)) {

    tme_log(TME_SUN4_LOG_HANDLE(sun4), 1000, TME_OK,
	    (TME_SUN4_LOG_HANDLE(sun4),
	     _("context now #%d"),
	     sun4->tme_sun44c_context));

    /* the normal state contexts are numbered from zero: */
    context_base = 0;
  }

  /* in the boot state: */
  else {

    tme_log(TME_SUN4_LOG_HANDLE(sun4), 1000, TME_OK,
	    (TME_SUN4_LOG_HANDLE(sun4),
	     _("context now #%d (boot state)"),
	     sun4->tme_sun44c_context));

    /* the boot state contexts are numbered from
       TME_SUN44C_CONTEXT_COUNT_MAX: */
    context_base = TME_SUN44C_CONTEXT_COUNT_MAX;
  }

  /* update the sparc bus context register: */
  *sun4->tme_sun44c_sparc_bus_context
    = (context_base
       + sun4->tme_sun44c_context);

  /* invalidate all DVMA TLBs that depended on the previous context: */
  tme_sun_mmu_context_switched(sun4->tme_sun44c_mmu);
}


/* this adds a new TLB set: */
int
_tme_sun44c_mmu_tlb_set_add(struct tme_bus_connection *conn_bus_asker,
			    struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_sun4 *sun4;
  int rc;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_bus_asker->tme_bus_connection.tme_connection_element->tme_element_private;

  /* add the TLB set to the MMU: */
  rc = tme_sun_mmu_tlb_set_add(sun4->tme_sun44c_mmu,
			       tlb_set_info);
  assert (rc == TME_OK);

  /* if this is the TLB set from the sparc: */
  if (conn_bus_asker->tme_bus_connection.tme_connection_type == TME_CONNECTION_BUS_SPARC) {

    /* the sparc must be a v7, which must expose a bus context register: */
    assert (tlb_set_info->tme_bus_tlb_set_info_bus_context != NULL);

    /* save the pointer to the sparc bus context register, and
       initialize it: */
    sun4->tme_sun44c_sparc_bus_context
      = tlb_set_info->tme_bus_tlb_set_info_bus_context;
    _tme_sun44c_mmu_context_set(sun4);

    /* return the maximum context number.  there are up to
       (TME_SUN44C_CONTEXT_COUNT_MAX * 2) contexts, as discussed
       above: */
    tlb_set_info->tme_bus_tlb_set_info_bus_context_max
      = ((TME_SUN44C_CONTEXT_COUNT_MAX * 2)
	 - 1);
  }

  return (rc);
}

/* this creates a sun4/4c MMU: */
void
_tme_sun44c_mmu_new(struct tme_sun4 *sun4)
{
  struct tme_sun_mmu_info mmu_info;

  memset(&mmu_info, 0, sizeof(mmu_info));
  mmu_info.tme_sun_mmu_info_element = sun4->tme_sun4_element;
  mmu_info.tme_sun_mmu_info_address_bits = 32;
  if (TME_SUN4_IS_SUN4C(sun4)) {
    mmu_info.tme_sun_mmu_info_pgoffset_bits = TME_SUN4C_PAGE_SIZE_LOG2;
    mmu_info.tme_sun_mmu_info_topindex_bits = -3; /* the address hole makes the top 3 address bits the same */
  }
  else {
    mmu_info.tme_sun_mmu_info_pgoffset_bits = TME_SUN4_PAGE_SIZE_LOG2;
  }
  mmu_info.tme_sun_mmu_info_pteindex_bits = 18 - mmu_info.tme_sun_mmu_info_pgoffset_bits;
  if (TME_SUN4_IS_MODEL(sun4, TME_SUN_IDPROM_TYPE_CODE_CALVIN)) {
    mmu_info.tme_sun_mmu_info_contexts = 16;
    mmu_info.tme_sun_mmu_info_pmegs = 256;
  }
  else if (TME_SUN4_IS_SUN4C(sun4)) {
    mmu_info.tme_sun_mmu_info_contexts = 8;
    mmu_info.tme_sun_mmu_info_pmegs = 128;
  }
  else {
    abort();
  }
  mmu_info.tme_sun_mmu_info_tlb_fill_private = sun4;
  mmu_info.tme_sun_mmu_info_tlb_fill = _tme_sun44c_tlb_fill_pte;
  mmu_info.tme_sun_mmu_info_proterr_private = &sun4->tme_sun4_dummy_connection_sparc;
  mmu_info.tme_sun_mmu_info_proterr = _tme_sun44c_mmu_proterr;
  mmu_info.tme_sun_mmu_info_invalid_private = &sun4->tme_sun4_dummy_connection_sparc;
  mmu_info.tme_sun_mmu_info_invalid = _tme_sun44c_mmu_invalid;
  sun4->tme_sun44c_mmu = tme_sun_mmu_new(&mmu_info);
  sun4->tme_sun44c_mmu_pmegs = mmu_info.tme_sun_mmu_info_pmegs;
  sun4->tme_sun4_dummy_connection_sparc.tme_connection_type = TME_CONNECTION_BUS_SPARC;
  sun4->tme_sun4_dummy_connection_sparc.tme_connection_element = sun4->tme_sun4_element;
}
