/* $Id: sun-si.c,v 1.7 2010/06/05 19:25:16 fredette Exp $ */

/* machine/sun/sun-si.c - Sun ncr5380-based SCSI implementation: */

/*
 * Copyright (c) 2004 Matt Fredette
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
_TME_RCSID("$Id: sun-si.c,v 1.7 2010/06/05 19:25:16 fredette Exp $");

/* includes: */
#include <tme/element.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>

/* macros: */

/* controller types: */
#define TME_SUN_SI_TYPE_NULL			(0)
#define TME_SUN_SI_TYPE_VME			(1)
#define TME_SUN_SI_TYPE_ONBOARD			(2)
#define TME_SUN_SI_TYPE_3E			(3)
#define TME_SUN_SI_TYPE_COBRA			(4)

/* register offsets and sizes: */

/* all controllers: */
#define TME_SUN_SI_REG_NCR5380			(0)
#define TME_SUN_SI_SIZ_NCR5380			(8)

/* all controllers except onboard, and the Sun 3/E limits these to 16 bits: */
#define TME_SUN_SI_REG_DMA_ADDRESS		(8)
#define TME_SUN_SI_REG_DMA_COUNT		(12)

/* Cobra controller only: */
#define TME_SUN_SI_REG_COBRA_FIFO_COUNT		(16)
#define TME_SUN_SI_REG_COBRA_CSR		(20)
#define TME_SIN_SI_REG_COBRA_BPR		(24)

/* onboard controller only: */
#define TME_SUN_SI_REG_ONBOARD_UDC_DATA		(16)
#define TME_SUN_SI_REG_ONBOARD_UDC_ADDRESS	(18)
#define TME_SUN_SI_REG_ONBOARD_FIFO_DATA	(20)

/* onboard and VME controllers only: */
#define TME_SUN_SI_REG_FIFO_COUNT_L		(22)

/* all controllers except for the Cobra: */
#define TME_SUN_SI_REG_CSR			(24)

/* 3/E controller only: */
#define TME_SUN_SI_REG_3E_IVEC			(27)

/* VME controller only: */
#define TME_SUN_SI_REG_VME_BPR_H		(26)
#define TME_SUN_SI_REG_VME_BPR_L		(28)
#define TME_SUN_SI_REG_VME_IV_AM		(30)
#define TME_SUN_SI_REG_VME_FIFO_COUNT_H		(32)

#define TME_SUN_SI_SIZ_REGS			(34)

/* the 3/E includes a 64KB DMA buffer: */
#define TME_SUN_SI_3E_SIZ_DMA			(0x10000)

/* the bits in the Control/Status Register: */
#define TME_SUN_SI_CSR_ONBOARD_DMA_ACTIVE	TME_BIT(15)	/* onboard controller only */
#define TME_SUN_SI_CSR_DMA_CONFLICT		TME_BIT(14)	/* all controllers except for the 3/E */
#define TME_SUN_SI_CSR_DMA_BUS_ERROR		TME_BIT(13)	/* all controllers except for the 3/E */
#define TME_SUN_SI_CSR_VME_MODIFIED		TME_BIT(12)	/* VME controller only */
#define TME_SUN_SI_CSR_FIFO_FULL		TME_BIT(11)	/* all controllers except for the 3/E (and Cobra?) */
#define TME_SUN_SI_CSR_FIFO_EMPTY		TME_BIT(10)	/* all controllers except for the 3/E (and Cobra?) */
#define TME_SUN_SI_CSR_INT_NCR5380		TME_BIT(9)	/* all controllers */
#define TME_SUN_SI_CSR_INT_DMA			TME_BIT(8)	/* all controllers except for the 3/E (and Cobra?) */
#define TME_SUN_SI_CSR_VME_LOB_MASK		(0x00c0)	/* VME controller only */
#define TME_SUN_SI_CSR_VME_BPCON		TME_BIT(5)	/* VME controller only */
#define TME_SUN_SI_CSR_DMA_ENABLE		TME_BIT(4)	/* all controllers except for onboard */
#define TME_SUN_SI_CSR_DMA_SEND			TME_BIT(3)	/* all controllers */
#define TME_SUN_SI_CSR_INT_ENABLE		TME_BIT(2)	/* all controllers */
#define TME_SUN_SI_CSR_RESET_FIFO		TME_BIT(1)	/* all controllers except for the 3/E */
#define TME_SUN_SI_CSR_3E_VCC			TME_BIT(1)	/* 3/E controller only */
#define TME_SUN_SI_CSR_RESET_CONTROLLER		TME_BIT(0)	/* all controllers */

/* the read-only bits in the Control/Status Register: */
/* all controllers except for the 3/E: */
#define TME_SUN_SI_CSR_READONLY		(~(TME_SUN_SI_CSR_VME_BPCON		\
					   | TME_SUN_SI_CSR_DMA_ENABLE		\
					   | TME_SUN_SI_CSR_DMA_SEND		\
					   | TME_SUN_SI_CSR_INT_ENABLE		\
					   | TME_SUN_SI_CSR_RESET_FIFO		\
					   | TME_SUN_SI_CSR_RESET_CONTROLLER))
/* 3/E controller only: */
#define TME_SUN_SI_CSR_3E_READONLY	(TME_SUN_SI_CSR_READONLY | TME_SUN_SI_CSR_3E_VCC)

/* these get and put a 32-bit register: */
#define TME_SUN_SI_REG32_GET(sun_si, reg)	\
  tme_betoh_u32(*((tme_uint32_t *) &(sun_si)->tme_sun_si_regs[(reg)]))
#define TME_SUN_SI_REG32_PUT(sun_si, reg, val)	\
  (*((tme_uint32_t *) &(sun_si)->tme_sun_si_regs[(reg)]) = tme_htobe_u32(val))

/* these get and put a 16-bit register: */
#define TME_SUN_SI_REG16_GET(sun_si, reg)	\
  tme_betoh_u16(*((tme_uint16_t *) &(sun_si)->tme_sun_si_regs[(reg)]))
#define TME_SUN_SI_REG16_PUT(sun_si, reg, val)	\
  (*((tme_uint16_t *) &(sun_si)->tme_sun_si_regs[(reg)]) = tme_htobe_u16(val))

/* these get and put the CSR register: */
#define TME_SUN_SI_CSR_GET(sun_si)				\
  ((sun_si)->tme_sun_si_type == TME_SUN_SI_TYPE_COBRA		\
   ? TME_SUN_SI_REG32_GET(sun_si, TME_SUN_SI_REG_COBRA_CSR)	\
   : TME_SUN_SI_REG16_GET(sun_si, TME_SUN_SI_REG_CSR))
#define TME_SUN_SI_CSR_PUT(sun_si, csr)				\
  ((sun_si)->tme_sun_si_type == TME_SUN_SI_TYPE_COBRA		\
   ? TME_SUN_SI_REG32_PUT(sun_si, TME_SUN_SI_REG_COBRA_CSR, csr)\
   : TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_CSR, csr))

/* the callout flags: */
#define TME_SUN_SI_CALLOUT_CHECK	(0)
#define TME_SUN_SI_CALLOUT_RUNNING	TME_BIT(0)
#define TME_SUN_SI_CALLOUTS_MASK	(-2)
#define  TME_SUN_SI_CALLOUT_SIGNALS	TME_BIT(1)
#define	 TME_SUN_SI_CALLOUT_INT	TME_BIT(2)

#if 1
#define TME_SUN_SI_DEBUG
#endif

/* structures: */

/* the controller: */
struct tme_sun_si {

  /* backpointer to our element: */
  struct tme_element *tme_sun_si_element;

  /* the mutex protecting the card: */
  tme_mutex_t tme_sun_si_mutex;

  /* the rwlock protecting the card: */
  tme_rwlock_t tme_sun_si_rwlock;

  /* the bus connection for the card's registers: */
  struct tme_bus_connection *tme_sun_si_conn_regs;

  /* the bus connection for the card's memory: */
  struct tme_bus_connection *tme_sun_si_conn_memory;

  /* the bus connection for the card's ncr5380: */
  struct tme_bus_connection *tme_sun_si_conn_ncr5380;

  /* the type of the si: */
  tme_uint32_t tme_sun_si_type;

  /* the callout flags: */
  int tme_sun_si_callout_flags;

  /* if our interrupt line is currently asserted: */
  int tme_sun_si_last_int_asserted;

  /* it's easiest to just model the board registers as a chunk of memory: */
  tme_uint8_t tme_sun_si_regs[TME_SUN_SI_SIZ_REGS];

  /* any outstanding NCR 5380 TLB entry: */
  struct tme_token *tme_sun_si_ncr5380_tlb_token;

  /* the 3/E DMA buffer: */
  tme_uint8_t *tme_sun_si_3e_dma;

  /* the CSR last called out to the NCR 5380: */
  tme_uint32_t tme_sun_si_csr_ncr5380;
};

/* a sun_si internal bus connection: */
struct tme_sun_si_connection {

  /* the external bus connection: */
  struct tme_bus_connection tme_sun_si_connection;

  /* this is nonzero if a TME_CONNECTION_BUS_GENERIC chip connection
     is for the registers: */
  unsigned int tme_sun_si_connection_regs;
};

#ifdef TME_SUN_SI_DEBUG
void
_tme_sun_si_reg_put(struct tme_sun_si *sun_si,
		    int reg,
		    tme_uint32_t val_new,
		    unsigned int val_size)
{
  const char *reg_name;
  tme_uint32_t val_old;

  if (val_size == sizeof(tme_uint32_t)) {
    val_old = TME_SUN_SI_REG32_GET(sun_si, reg);
    TME_SUN_SI_REG32_PUT(sun_si, reg, val_new);
  }
  else {
    assert (val_size == sizeof(tme_uint16_t));
    val_old = TME_SUN_SI_REG16_GET(sun_si, reg);
    TME_SUN_SI_REG16_PUT(sun_si, reg, val_new);
    val_new &= 0xffff;
  }

  if (val_new == val_old) {
    return;
  }

  /* try to get the name of this register: */
  reg_name = NULL;
#define _TME_SUN_SI_REG_IS(_reg, type) (reg_name == NULL && reg == (_reg) && val_size == sizeof(type))
  if (sun_si->tme_sun_si_type != TME_SUN_SI_TYPE_ONBOARD
      && _TME_SUN_SI_REG_IS(TME_SUN_SI_REG_DMA_ADDRESS, tme_uint32_t)) {
    reg_name = "dma_address";
  }
  if (sun_si->tme_sun_si_type != TME_SUN_SI_TYPE_ONBOARD
      && _TME_SUN_SI_REG_IS(TME_SUN_SI_REG_DMA_COUNT, tme_uint32_t)) {
    reg_name = "dma_count";
  }
  if ((sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_ONBOARD
       || sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_VME)
      && _TME_SUN_SI_REG_IS(TME_SUN_SI_REG_FIFO_COUNT_L, tme_uint16_t)) {
    reg_name = "fifo_count_l";
  }
  if (sun_si->tme_sun_si_type != TME_SUN_SI_TYPE_COBRA
      && _TME_SUN_SI_REG_IS(TME_SUN_SI_REG_CSR, tme_uint16_t)) {
    reg_name = "CSR";
  }
  if (sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_VME) {
    if (_TME_SUN_SI_REG_IS(TME_SUN_SI_REG_VME_BPR_H, tme_uint16_t)) {
      reg_name = "bpr_h";
    }
    if (_TME_SUN_SI_REG_IS(TME_SUN_SI_REG_VME_BPR_L, tme_uint16_t)) {
      reg_name = "bpr_l";
    }
    if (_TME_SUN_SI_REG_IS(TME_SUN_SI_REG_VME_FIFO_COUNT_H, tme_uint16_t)) {
      reg_name = "fifo_count_h";
    }
  }
#undef _TME_SUN_SI_REG_IS
  if (reg_name == NULL) {
    reg_name = "???";
  }
  tme_log(&sun_si->tme_sun_si_element->tme_element_log_handle,
	    100, TME_OK,
	    (&sun_si->tme_sun_si_element->tme_element_log_handle,
	     "%s old 0x%04x new 0x%04x",
	     reg_name,
	     val_old,
	     val_new));
}
#undef TME_SUN_SI_REG16_PUT
#define TME_SUN_SI_REG16_PUT(sun_si, reg, val) _tme_sun_si_reg_put((sun_si), (reg), (val), sizeof(tme_uint16_t))
#undef TME_SUN_SI_REG32_PUT
#define TME_SUN_SI_REG32_PUT(sun_si, reg, val) _tme_sun_si_reg_put((sun_si), (reg), (val), sizeof(tme_uint32_t))
#endif /* TME_SUN_SI_DEBUG */

/* the sun_si callout function.  it must be called with the mutex locked: */
static void
_tme_sun_si_callout(struct tme_sun_si *sun_si, int new_callouts)
{
  struct tme_bus_connection *conn_ncr5380;
  struct tme_bus_connection *conn_bus;
  tme_uint32_t csr, csr_diff;
  unsigned int signal, level;
  int callouts, later_callouts;
  int rc;
  int int_asserted;
  
  /* add in any new callouts: */
  sun_si->tme_sun_si_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (sun_si->tme_sun_si_callout_flags & TME_SUN_SI_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  sun_si->tme_sun_si_callout_flags |= TME_SUN_SI_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; (callouts = sun_si->tme_sun_si_callout_flags) & TME_SUN_SI_CALLOUTS_MASK; ) {

    /* clear the needed callouts: */
    sun_si->tme_sun_si_callout_flags = callouts & ~TME_SUN_SI_CALLOUTS_MASK;
    callouts &= TME_SUN_SI_CALLOUTS_MASK;

    /* get the current CSR value: */
    csr = TME_SUN_SI_CSR_GET(sun_si);

    /* get the next changed CSR bit to convert to a bus signal to the
       NCR 5380: */
    csr_diff = ((csr
		 ^ sun_si->tme_sun_si_csr_ncr5380)
		& (TME_SUN_SI_CSR_RESET_CONTROLLER
		   | ((sun_si->tme_sun_si_type != TME_SUN_SI_TYPE_ONBOARD)
		      ? TME_SUN_SI_CSR_DMA_ENABLE
		      : 0)));
    csr_diff = (csr_diff ^ (csr_diff - 1)) & csr_diff;
      
    /* if there is a changed CSR bit to call out: */
    if (csr_diff != 0) {

      /* assume that if the signal's bit is set in the CSR, it will
	 be asserted: */
      level = (csr & csr_diff) != 0;

      /* dispatch on the CSR bit: */
      switch (csr_diff) {
      default:
	assert (FALSE);
	/* FALLTHROUGH */

      case TME_SUN_SI_CSR_RESET_CONTROLLER:
	signal = TME_BUS_SIGNAL_RESET;
	level = !level;
	break;

      case TME_SUN_SI_CSR_DMA_ENABLE:
	signal = TME_BUS_SIGNAL_DACK;
	break;
      }

      /* create a real signal level value: */
      level = (level
	       ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	       : TME_BUS_SIGNAL_LEVEL_NEGATED);

      /* get this card's ncr5380 connection: */
      conn_ncr5380 = sun_si->tme_sun_si_conn_ncr5380;

      /* unlock the mutex: */
      tme_mutex_unlock(&sun_si->tme_sun_si_mutex);
      
      /* do the callout: */
      rc = (conn_ncr5380 != NULL
	    ? ((*conn_ncr5380->tme_bus_signal)
	       (conn_ncr5380,
		signal | level))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&sun_si->tme_sun_si_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SUN_SI_CALLOUT_SIGNALS;
      }

      /* otherwise, the callout was successful: */
      else {

	/* update the ncr5380 image of the CSR: */
	sun_si->tme_sun_si_csr_ncr5380 = 
	  ((sun_si->tme_sun_si_csr_ncr5380 & ~csr_diff)
	   | (csr & csr_diff));

	/* there may be more signals to call out, so attempt this
	   callout again now: */
	sun_si->tme_sun_si_callout_flags |= TME_SUN_SI_CALLOUT_SIGNALS;
      }
    }

    /* get the current CSR value: */
    csr = TME_SUN_SI_CSR_GET(sun_si);

    /* see if the interrupt signal should be asserted or negated.  on
       all controllers except for onboard, DMA must be enabled for the
       interrupt signal to reach the CPU: */
    int_asserted = ((csr & TME_SUN_SI_CSR_INT_ENABLE)
		    && (sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_ONBOARD
			|| (csr & TME_SUN_SI_CSR_DMA_ENABLE))
		      && (csr & (TME_SUN_SI_CSR_DMA_CONFLICT
				 | TME_SUN_SI_CSR_DMA_BUS_ERROR
				 | TME_SUN_SI_CSR_INT_NCR5380
				 | TME_SUN_SI_CSR_INT_DMA)));

    /* if the interrupt signal doesn't already have the right state: */
    if (!int_asserted != !sun_si->tme_sun_si_last_int_asserted) {

      /* get our bus connection: */
      conn_bus = sun_si->tme_sun_si_conn_regs;
	
      /* unlock our mutex: */
      tme_mutex_unlock(&sun_si->tme_sun_si_mutex);
	
      /* call out the bus interrupt signal edge: */
      rc = (conn_bus != NULL
	    ? ((*conn_bus->tme_bus_signal)
	       (conn_bus,
		TME_BUS_SIGNAL_INT_UNSPEC
		| (int_asserted
		   ? TME_BUS_SIGNAL_LEVEL_ASSERTED
		   : TME_BUS_SIGNAL_LEVEL_NEGATED)))
	    : TME_OK);

      /* lock our mutex: */
      tme_mutex_lock(&sun_si->tme_sun_si_mutex);
	
      /* if this callout was successful, note the new state of the
	 interrupt signal: */
      if (rc == TME_OK) {
	sun_si->tme_sun_si_last_int_asserted = int_asserted;
      }

      /* otherwise, remember that at some later time this callout
	 should be attempted again: */
      else {
	later_callouts |= TME_SUN_SI_CALLOUT_INT;
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  sun_si->tme_sun_si_callout_flags = later_callouts;
}

/* the 3/E DMA bus cycle handler: */
static int
_tme_sun_si_bus_cycle_3e_dma(void *_sun_si, struct tme_bus_cycle *cycle)
{
  struct tme_sun_si *sun_si;

  /* recover our data structure: */
  sun_si = (struct tme_sun_si *) _sun_si;

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle, 
			    sun_si->tme_sun_si_3e_dma,
			    TME_SUN_SI_3E_SIZ_DMA - 1);

  /* no faults: */
  return (TME_OK);
}

/* the sun_si bus cycle handler for the board registers: */
static int
_tme_sun_si_bus_cycle_regs(void *_sun_si, 
			   struct tme_bus_cycle *cycle_init)
{
  struct tme_sun_si *sun_si;
  tme_uint32_t csr_old, csr_new, csr_diff, csr_mask;
  tme_bus_addr32_t address;
  tme_uint8_t cycle_size;
  tme_uint32_t dma_count;
  int new_callouts;

  /* get the address and cycle size: */
  address = cycle_init->tme_bus_cycle_address;
  cycle_size = cycle_init->tme_bus_cycle_size;

  /* it appears that si hardware doesn't respond to a byte access
     to its DMA count register.  at least, SunOS' sc probe routine
     seems to rely on this to distinguish an si from an sc: */
  if (address == TME_SUN_SI_REG_DMA_COUNT
      && cycle_size == sizeof(tme_uint8_t)) {
    return (EINVAL);
  }

  /* recover our data structure: */
  sun_si = (struct tme_sun_si *) _sun_si;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&sun_si->tme_sun_si_mutex);

  /* get the previous CSR value: */
  csr_old = TME_SUN_SI_CSR_GET(sun_si);

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    sun_si->tme_sun_si_regs,
			    TME_SUN_SI_SIZ_REGS - 1);

#define _TME_SUN_SI_REG_IS(reg, type) TME_RANGES_OVERLAP(address, address + cycle_size - 1, (reg), (reg) + sizeof(type) - 1)

  /* if this was a write: */
  if (cycle_init->tme_bus_cycle_type
      == TME_BUS_CYCLE_WRITE) {

    /* if this is a VME controller: */
    if (sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_VME) {

      /* if this was a write to the DMA count register: */
      if (_TME_SUN_SI_REG_IS(TME_SUN_SI_REG_DMA_COUNT, tme_uint32_t)) {

	/* get the DMA count register: */
	dma_count = TME_SUN_SI_REG32_GET(sun_si, TME_SUN_SI_REG_DMA_COUNT);

	/* copy the DMA count register to the FIFO count register: */
	TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_VME_FIFO_COUNT_H, (dma_count >> 16));
	TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_FIFO_COUNT_L, (dma_count & 0xffff));
      }
    }

    /* get the current CSR value, and put back any bits that
       software can't change: */
    csr_mask = (sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_3E
		? TME_SUN_SI_CSR_3E_READONLY
		: TME_SUN_SI_CSR_READONLY);
    csr_new = ((TME_SUN_SI_CSR_GET(sun_si)
		& ~csr_mask)
	       | (csr_old
		  & csr_mask));

    /* get the sets of CSR bits that have changed: */
    csr_diff = (csr_old ^ csr_new);

    /* if the FIFO is being reset: */
    if (sun_si->tme_sun_si_type != TME_SUN_SI_TYPE_3E
	&& (csr_diff & TME_SUN_SI_CSR_RESET_FIFO)
	&& !(csr_new & TME_SUN_SI_CSR_RESET_FIFO)) {

      /* reset the FIFOs: */
      TME_SUN_SI_REG32_PUT(sun_si, TME_SUN_SI_REG_DMA_ADDRESS, 0);
      TME_SUN_SI_REG32_PUT(sun_si, TME_SUN_SI_REG_DMA_COUNT, 0);
      switch (sun_si->tme_sun_si_type) {
      default: 
	assert(FALSE);
	/* FALLTHROUGH */
      case TME_SUN_SI_TYPE_ONBOARD:
	TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_ONBOARD_FIFO_DATA, 0);
	TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_FIFO_COUNT_L, 0);
	break;
      case TME_SUN_SI_TYPE_VME:
	TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_FIFO_COUNT_L, 0);
	TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_VME_FIFO_COUNT_H, 0);
	csr_new &= ~TME_SUN_SI_CSR_VME_LOB_MASK;
	break;
      case TME_SUN_SI_TYPE_COBRA:
	abort();
	break;
      }
    }

    /* if the CSR has changed at all: */
    if (csr_new != csr_old) {

      /* put back the new CSR: */
      tme_log(&sun_si->tme_sun_si_element->tme_element_log_handle,
	      100, TME_OK,
	      (&sun_si->tme_sun_si_element->tme_element_log_handle,
	       "CSR now 0x%04x",
	       csr_new));
      TME_SUN_SI_CSR_PUT(sun_si, csr_new);

      /* assume that we need to call out changes to bus signals and
       our interrupt: */
      new_callouts |= TME_SUN_SI_CALLOUT_INT | TME_SUN_SI_CALLOUT_SIGNALS;
    }
  }

  /* make any new callouts: */
  _tme_sun_si_callout(sun_si, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_si->tme_sun_si_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the sun_si TLB filler for the board registers (and, on the 3/E, for
   the board memory on the VME bus): */
static int
_tme_sun_si_tlb_fill_regs(struct tme_bus_connection *conn_bus,
			  struct tme_bus_tlb *tlb, 
			  tme_bus_addr_t address_bus,
			  unsigned int cycles)
{
  struct tme_sun_si *sun_si;
  tme_bus_addr32_t address;
  struct tme_bus_connection *conn_ncr5380;
  tme_bus_addr32_t address_regs;
  struct tme_bus_tlb tlb_mapping;
  int rc;

  /* recover our data structures: */
  sun_si = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the internal address: */
  address = address_bus;
  assert (address == address_bus);

  /* assume that the registers start at address zero: */
  address_regs = 0;

  /* dispatch on the controller type: */
  switch (sun_si->tme_sun_si_type) {

  case TME_SUN_SI_TYPE_3E:

    /* the board registers follow the DMA buffer: */
    address_regs = TME_SUN_SI_3E_SIZ_DMA;

    /* if this address is in the DMA buffer: */
    if (address < TME_SUN_SI_3E_SIZ_DMA) {

      /* initialize the TLB entry: */
      tme_bus_tlb_initialize(tlb);

      /* this TLB entry covers this range: */
      tlb->tme_bus_tlb_addr_first = 0;
      tlb->tme_bus_tlb_addr_last = TME_SUN_SI_3E_SIZ_DMA - 1;

      /* this TLB entry allows fast reading and writing: */
      tlb->tme_bus_tlb_emulator_off_read = sun_si->tme_sun_si_3e_dma;
      tlb->tme_bus_tlb_emulator_off_write = sun_si->tme_sun_si_3e_dma;
      
      /* allow reading and writing: */
      tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

      /* our bus cycle handler: */
      tlb->tme_bus_tlb_cycle_private = sun_si;
      tlb->tme_bus_tlb_cycle = _tme_sun_si_bus_cycle_3e_dma;

      return (TME_OK);
    }
    break;

  default:
    break;
  }

  /* offset the address: */
  address -= address_regs;

  /* the address must be within range: */
  assert(address < TME_SUN_SI_SIZ_REGS);

  /* if this address is in the NCR 5380 registers: */
  if (address < TME_SUN_SI_SIZ_NCR5380) {

    /* get the NCR 5380 connection: */
    conn_ncr5380 = sun_si->tme_sun_si_conn_ncr5380;

    /* call the NCR 5380 TLB fill function: */
    rc = (conn_ncr5380 != NULL
	  ? (*conn_ncr5380->tme_bus_tlb_fill)(conn_ncr5380, tlb,
					      address, cycles)
	  : EINVAL);

    /* if that succeeded: */
    if (rc == TME_OK) {
      
      /* create the mapping TLB entry: */
      tlb_mapping.tme_bus_tlb_addr_first = (address_regs + TME_SUN_SI_REG_NCR5380);
      tlb_mapping.tme_bus_tlb_addr_last
	= (address_regs
	   + TME_SUN_SI_REG_NCR5380
	   + TME_SUN_SI_SIZ_NCR5380
	   - 1);
      tlb_mapping.tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
      
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, 0, &tlb_mapping, address_regs + TME_SUN_SI_REG_NCR5380);
    }

    return (rc);
  }

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers this range: */
  tlb->tme_bus_tlb_addr_first = (address_regs + TME_SUN_SI_SIZ_NCR5380);
  tlb->tme_bus_tlb_addr_last = (address_regs + TME_SUN_SI_SIZ_REGS - 1);

  /* NB: even though our controller registers don't have read
     side-effects, we can't support fast reading, because we have to
     be able to fault a byte access to the DMA count register.  at
     least, SunOS' sc probe routine seems to rely on this to
     distinguish an si from an sc: */

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = sun_si;
  tlb->tme_bus_tlb_cycle = _tme_sun_si_bus_cycle_regs;

  return (TME_OK);
}

/* the interrupt acknowledge function for the VME and 3/E types: */
static int
_tme_sun_si_intack(struct tme_bus_connection *conn_bus, unsigned int signal, int *_vector)
{
  struct tme_sun_si *sun_si;
  int vector;

  /* recover our data structure: */
  sun_si = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* dispatch on the controller type: */
  switch (sun_si->tme_sun_si_type) {
  default:
    assert (FALSE);
    /* FALLTHROUGH */

  case TME_SUN_SI_TYPE_3E:
    vector = sun_si->tme_sun_si_regs[TME_SUN_SI_REG_3E_IVEC];
    break;

  case TME_SUN_SI_TYPE_VME:
    vector = TME_SUN_SI_REG16_GET(sun_si, TME_SUN_SI_REG_VME_IV_AM) & 0xff;
    break;
  }

  /* return the interrupt vector: */
  *_vector = vector;
  return (TME_OK);
}

/* the sun_si bus signal handler for the NCR 5380: */
static int
_tme_sun_si_bus_signal(struct tme_bus_connection *conn_bus, 
		       unsigned int signal)
{
  struct tme_sun_si *sun_si;
  tme_uint32_t csr;

  /* this must be the unspecified interrupt signal: */
  assert (TME_BUS_SIGNAL_WHICH(signal) == TME_BUS_SIGNAL_INT_UNSPEC);

  /* recover our data structures: */
  sun_si = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&sun_si->tme_sun_si_mutex);

  /* update the CSR value: */
  csr = TME_SUN_SI_CSR_GET(sun_si);
  csr = ((csr
	  & ~TME_SUN_SI_CSR_INT_NCR5380)
	 | (((signal & TME_BUS_SIGNAL_LEVEL_MASK)
	     == TME_BUS_SIGNAL_LEVEL_ASSERTED)
	    ? TME_SUN_SI_CSR_INT_NCR5380
	    : 0));
  TME_SUN_SI_CSR_PUT(sun_si, csr);

  /* call out an interrupt change: */
  _tme_sun_si_callout(sun_si, TME_SUN_SI_CALLOUT_INT);

  /* unlock our mutex: */
  tme_mutex_unlock(&sun_si->tme_sun_si_mutex);

  return (TME_OK);
}

/* the sun_si bus fault handler for the NCR 5380 DMA: */
static int
_tme_sun_si_bus_fault_handler(void *_sun_si, 
			      struct tme_bus_tlb *tlb,
			      struct tme_bus_cycle *cycle,
			      int rc)
{
  struct tme_sun_si *sun_si;

  /* recover our data structure: */
  sun_si = (struct tme_sun_si *) _sun_si;

  /* lock our mutex: */
  tme_mutex_lock(&sun_si->tme_sun_si_mutex);

  /* set a DMA bus error and call out an interrupt change: */
  TME_SUN_SI_CSR_PUT(sun_si, TME_SUN_SI_CSR_GET(sun_si) | TME_SUN_SI_CSR_DMA_BUS_ERROR);
  _tme_sun_si_callout(sun_si, TME_SUN_SI_CALLOUT_INT);    

  /* unlock our mutex: */
  tme_mutex_unlock(&sun_si->tme_sun_si_mutex);
  
  return (rc);
}

/* the sun_si TLB filler for the NCR 5380 DMA: */
static int
_tme_sun_si_tlb_fill(struct tme_bus_connection *conn_bus,
		     struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t ncr5380_address_wider,
		     unsigned int cycles)
{
  struct tme_sun_si *sun_si;
  tme_bus_addr32_t ncr5380_address;
  tme_uint32_t csr;
  tme_bus_addr32_t dma_address;
  tme_uint32_t dma_count;
  tme_uint32_t bpr;
  unsigned int bpr_count;
#if defined(TME_SUN_SI_STRICT_VME)
  unsigned int bpr_i;
#endif /* defined(TME_SUN_SI_STRICT_VME) */
  struct tme_bus_tlb tlb_mapping;
  int rc;

  /* recover our data structures: */
  sun_si = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the normal-width NCR 5380 DMA address: */
  ncr5380_address = ncr5380_address_wider;
  assert (ncr5380_address == ncr5380_address_wider);

  /* assume that this call will succeed: */
  rc = TME_OK;

  /* lock our mutex: */
  tme_mutex_lock(&sun_si->tme_sun_si_mutex);

  /* if the NCR 5380 already has an outstanding TLB entry, and it
     doesn't happen to be this TLB entry, invalidate it: */
  if (sun_si->tme_sun_si_ncr5380_tlb_token != NULL
      && (tlb == NULL
	  || (sun_si->tme_sun_si_ncr5380_tlb_token
	      != tlb->tme_bus_tlb_token))) {
    tme_token_invalidate(sun_si->tme_sun_si_ncr5380_tlb_token);

    /* the NCR 5380 doesn't have any outstanding TLB entry: */
    sun_si->tme_sun_si_ncr5380_tlb_token = NULL;
  }

  /* get the CSR value: */
  csr = TME_SUN_SI_CSR_GET(sun_si);

  /* assume that this is not an onboard controller, and get the DMA
     address register, plus the NCR 5380 address, and the DMA count
     register: */
  dma_address = TME_SUN_SI_REG32_GET(sun_si, TME_SUN_SI_REG_DMA_ADDRESS) + ncr5380_address;
  dma_count = TME_SUN_SI_REG32_GET(sun_si, TME_SUN_SI_REG_DMA_COUNT);
    
  /* dispatch on the controller type: */
  switch (sun_si->tme_sun_si_type) {

  default:
    assert(FALSE);
    /* FALLTHROUGH */

  case TME_SUN_SI_TYPE_3E:

    /* mask the DMA address and count registers to 16 bits: */
    dma_address &= 0xffff;
    dma_count &= 0xffff;

    /* if the NCR 5380 has stopped doing DMA, or if it has exhausted
       DMA, or if DMA is not enabled: */
    if (cycles == TME_BUS_CYCLE_UNDEF
	|| ncr5380_address >= dma_count
	|| !(csr & TME_SUN_SI_CSR_DMA_ENABLE)) {

      /* if the NCR 5380 has stopped doing DMA, cap the DMA count and
	 address to the amount of DMA that was available, thus
	 cancelling the effect of the NCR 5380 prefetch.  this
	 prevents the DMA count from going negative, confusing the Sun
	 PROM and probably SunOS.  I'm guessing that all of the SCSI
	 devices ever used with Suns change information transfer phase
	 faster than the NCR 5380 prefetches a byte from DMA, which is
	 why this DMA-count-going-negative problem is never seen.  since
	 our NCR 5380 is infinitely fast at prefetch, we have to deal: */
      if (cycles == TME_BUS_CYCLE_UNDEF
	  && ncr5380_address > dma_count) {
	assert (ncr5380_address == (dma_count + 1));
	dma_address -= (ncr5380_address - dma_count);
	ncr5380_address = dma_count;
      }

      /* update the DMA address and count registers: */
      TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_DMA_ADDRESS + sizeof(tme_uint16_t), 
			   dma_address);
      TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_DMA_COUNT + sizeof(tme_uint16_t), 
			   dma_count - ncr5380_address);

      /* return EAGAIN to the NCR 5380, unless it was the NCR 5380
         that stopped doing DMA: */
      if (cycles != TME_BUS_CYCLE_UNDEF) {
	rc = EAGAIN;
      }
      break;
    }

    /* XXX does DMA wrap around in the 3/E DMA memory? */
    dma_count = TME_MIN((TME_SUN_SI_3E_SIZ_DMA - dma_address), dma_count);

    /* initialize the TLB entry: */
    tme_bus_tlb_initialize(tlb);

    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = dma_count - 1;

    /* this TLB entry supports fast reading and writing: */
    tlb->tme_bus_tlb_emulator_off_read = sun_si->tme_sun_si_3e_dma + dma_address;
    tlb->tme_bus_tlb_emulator_off_write = sun_si->tme_sun_si_3e_dma + dma_address;

    /* allow reading and writing: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

    /* our bus cycle handler: */
    tlb->tme_bus_tlb_cycle_private = sun_si;
    tlb->tme_bus_tlb_cycle = _tme_sun_si_bus_cycle_3e_dma;

    /* unlock our mutex: */
    tme_mutex_unlock(&sun_si->tme_sun_si_mutex);

    return (TME_OK);

  case TME_SUN_SI_TYPE_VME:

    /* if the NCR 5380 has stopped doing DMA, or if it has exhausted
       DMA, or if DMA is not enabled: */
    if (cycles == TME_BUS_CYCLE_UNDEF
	|| ncr5380_address >= dma_count
	|| ((csr
	     & (TME_SUN_SI_CSR_DMA_CONFLICT
		| TME_SUN_SI_CSR_DMA_BUS_ERROR
		| TME_SUN_SI_CSR_DMA_ENABLE))
	    != TME_SUN_SI_CSR_DMA_ENABLE)) {

      /* if the NCR 5380 has stopped doing DMA, cap the DMA count and
	 address to the amount of DMA that was available, thus
	 cancelling the effect of the NCR 5380 prefetch.  this
	 prevents the DMA count from going negative, confusing the Sun
	 PROM and probably SunOS.  I'm guessing that all of the SCSI
	 devices ever used with Suns change information transfer phase
	 faster than the NCR 5380 prefetches a byte from DMA, which is
	 why this DMA-count-going-negative problem is never seen.  since
	 our NCR 5380 is infinitely fast at prefetch, we have to deal: */
      if (cycles == TME_BUS_CYCLE_UNDEF
	  && ncr5380_address > dma_count) {
	assert (ncr5380_address == (dma_count + 1));
	dma_address -= (ncr5380_address - dma_count);
	ncr5380_address = dma_count;
      }

      /* update the DMA address and count registers: */
      TME_SUN_SI_REG32_PUT(sun_si, TME_SUN_SI_REG_DMA_ADDRESS, dma_address);
      TME_SUN_SI_REG32_PUT(sun_si, TME_SUN_SI_REG_DMA_COUNT, dma_count - ncr5380_address);
      TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_VME_FIFO_COUNT_H, ((dma_count - ncr5380_address) >> 16));
      TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_FIFO_COUNT_L, ((dma_count - ncr5380_address) & 0xffff));

      /* assume that our emulated byte pack is empty: */
      bpr = 0;
      bpr_count = 0;

#ifdef TME_SUN_SI_STRICT_VME

      /* if the NCR 5380 was doing DMA writes to memory (i.e., it was
         receiving): */
      if (!(csr & TME_SUN_SI_CSR_DMA_SEND)) {

	/* calculate how many bytes must be left in the byte pack: */
	bpr_count = ncr5380_address;
	bpr_count &= ((csr & TME_SUN_SI_CSR_VME_BPCON)
		      ? (sizeof(tme_uint16_t) - 1)
		      : (sizeof(tme_uint32_t) - 1));

	/* get the byte pack contents from the memory that the NCR
	   5380 DMA'ed into.  this is why all DMA writes have to be to
	   fast memory under TME_SUN_SI_STRICT_VME - if you DMA to
	   memory with side-effects, we can't read the data back out
	   again without incurring more side-effects, and we don't
	   want to do a *full* emulation of the DMA engine: */
	for (bpr_i = bpr_count; bpr_i > 0; bpr_i--) {
	  bpr = (bpr << 8) | sun_si->tme_sun_si_dma_buffer_write[ncr5380_address - bpr_i];
	}

	/* left-justify the byte pack: */
	bpr <<= ((((csr & TME_SUN_SI_CSR_VME_BPCON)
		    ? sizeof(tme_uint16_t)
		    : sizeof(tme_uint32_t))
		   - bpr_count)
		  * 8);

      }

      /* otherwise, the NCR 5380 was doing DMA reads to memory (i.e.,
	 it was sending): */
      else {

	/* nothing to do.  we don't emulate the byte pack for sending: */
      }

#endif  /* TME_SUN_SI_STRICT_VME */

      /* update the byte pack and the CSR: */
      TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_VME_BPR_H, (bpr >> 16));
      TME_SUN_SI_REG16_PUT(sun_si, TME_SUN_SI_REG_VME_BPR_L, (bpr & 0xffff));
      TME_FIELD_MASK_DEPOSITU(csr, TME_SUN_SI_CSR_VME_LOB_MASK, bpr_count);
      TME_SUN_SI_CSR_PUT(sun_si, csr);

      /* return EAGAIN to the NCR 5380, unless it was the NCR 5380
	 that stopped doing DMA: */
      if (cycles != TME_BUS_CYCLE_UNDEF) {
	rc = EAGAIN;
      }
      break;
    }
    break;

  case TME_SUN_SI_TYPE_ONBOARD:
    /* TBD */
    abort();

  case TME_SUN_SI_TYPE_COBRA:
    /* TBD */
    abort();
  }

  /* if DMA is not ready for some reason: */
  if (rc == EAGAIN) {

    /* initialize any local TLB entry passed by the caller.  this
       will make it unusable: */
    if (tlb != NULL) {
      tme_bus_tlb_initialize(tlb);
    }

    /* if DMA is enabled: */
    if (csr & TME_SUN_SI_CSR_DMA_ENABLE) {

      /* set a DMA interrupt and call out an interrupt change: */
      TME_SUN_SI_CSR_PUT(sun_si, TME_SUN_SI_CSR_GET(sun_si) | TME_SUN_SI_CSR_INT_DMA);
      _tme_sun_si_callout(sun_si, TME_SUN_SI_CALLOUT_INT);    
    }

    /* cancel the remainder of the TLB fill: */
    cycles = TME_BUS_CYCLE_UNDEF;
  }

  /* get the bus connection for memory: */
  conn_bus = sun_si->tme_sun_si_conn_memory;

  /* if we need to fill a TLB entry on the bus: */
  if (cycles != TME_BUS_CYCLE_UNDEF) {

    /* the NCR 5380 now has this outstanding TLB entry: */
    sun_si->tme_sun_si_ncr5380_tlb_token
      = tlb->tme_bus_tlb_token;
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&sun_si->tme_sun_si_mutex);

  /* if we need to fill a TLB entry on the bus: */
  if (cycles != TME_BUS_CYCLE_UNDEF) {

    /* DMA must be OK so far: */
    assert (rc == TME_OK);
    assert (dma_count > ncr5380_address);

    /* call out to fill this TLB entry on the bus: */
    rc = (conn_bus != NULL
	  ? (*conn_bus->tme_bus_tlb_fill)(conn_bus, 
					  tlb,
					  dma_address,
					  cycles)
	  : ENXIO);

    /* if this callout succeeded: */
    if (rc == TME_OK) {

#if defined(TME_SUN_SI_STRICT_VME) || defined(TME_SUN_SI_STRICT_ONBOARD) || defined(TME_SUN_SI_STRICT_COBRA)

      /* when the NCR 5380 wants to do DMA writes, in order to
	 correctly emulate the byte pack registers on these
	 controllers without having to emulate their full DMA behavior
	 (copying 16- or 32-bit words into a controller register for
	 the NCR 5380), we just insist that all DMA be done to memory
	 that supports fast access: */
      if ((cycles & TME_BUS_CYCLE_WRITE)
	  && tlb->tme_bus_tlb_emulator_off_write == TME_EMULATOR_OFF_UNDEF) {
	abort();
      }
      
#endif /* defined(TME_SUN_SI_STRICT_VME) || defined(TME_SUN_SI_STRICT_ONBOARD) || defined(TME_SUN_SI_STRICT_COBRA) */

      /* create the mapping TLB entry: */
      tlb_mapping.tme_bus_tlb_addr_first = ncr5380_address;
      tlb_mapping.tme_bus_tlb_addr_last = dma_count - 1;
      tlb_mapping.tme_bus_tlb_cycles_ok = cycles;
      
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, dma_address, &tlb_mapping, ncr5380_address);

      /* add our bus fault handler: */
      TME_BUS_TLB_FAULT_HANDLER(tlb, _tme_sun_si_bus_fault_handler, sun_si);
    }
  }

  return (rc);
}

/* the sun_si TLB adder for the NCR 5380: */
static int
_tme_sun_si_tlb_set_add(struct tme_bus_connection *conn_bus,
			struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_sun_si *sun_si;

  /* recover our data structures: */
  sun_si = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* pass the sun_si's request through to the memory bus: */
  conn_bus = sun_si->tme_sun_si_conn_memory;
  return (conn_bus != NULL
	  ? (*conn_bus->tme_bus_tlb_set_add)(conn_bus, 
					     tlb_set_info)
	  : ENXIO);
}

/* this scores a new connection: */
static int
_tme_sun_si_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_sun_si *sun_si;
  struct tme_sun_si_connection *conn_sun_si;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  sun_si = conn->tme_connection_element->tme_element_private;
  conn_sun_si = (struct tme_sun_si_connection *)conn;

  /* this is a generic bus connection, so just score it nonzero and
     return.  note that there's no good way to differentiate a
     connection to a bus from a connection to just another chip, so we
     always return a nonzero score here: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_sun_si_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sun_si *sun_si;
  struct tme_sun_si_connection *conn_sun_si;
  struct tme_bus_connection *conn_bus;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  sun_si = conn->tme_connection_element->tme_element_private;
  conn_sun_si = (struct tme_sun_si_connection *) conn;
  conn_bus = &conn_sun_si->tme_sun_si_connection;

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&sun_si->tme_sun_si_mutex);

    /* save our connection: */
    if (conn_bus->tme_bus_tlb_fill == _tme_sun_si_tlb_fill) {
      sun_si->tme_sun_si_conn_ncr5380 = (struct tme_bus_connection *) conn->tme_connection_other;
    }
    else {
      if (conn_sun_si->tme_sun_si_connection_regs) {
	sun_si->tme_sun_si_conn_regs = (struct tme_bus_connection *) conn->tme_connection_other;
      }
      if (!(sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_ONBOARD
	    || sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_COBRA)
	  || !conn_sun_si->tme_sun_si_connection_regs) {
	sun_si->tme_sun_si_conn_memory = (struct tme_bus_connection *) conn->tme_connection_other;
      }
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&sun_si->tme_sun_si_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_sun_si_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a sun_si: */
static int
_tme_sun_si_connections_new(struct tme_element *element,
			    const char * const *args,
			    struct tme_connection **_conns,
			    char **_output)
{
  struct tme_sun_si *sun_si;
  struct tme_sun_si_connection *conn_sun_si;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  unsigned int ncr5380;
  unsigned int regs;
  int usage;
  int rc;

  /* recover our data structure: */
  sun_si = (struct tme_sun_si *) element->tme_element_private;
  
  /* we don't bother locking the mutex simply to check if connections
     already exist: */

  /* check our arguments: */
  usage = FALSE;
  rc = 0;
  regs = FALSE;
  ncr5380 = FALSE;

  /* if this connection is for the registers: */
  if (TME_ARG_IS(args[1], "csr")) {

    /* if we already have a register connection, complain: */
    if (sun_si->tme_sun_si_conn_regs != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new connection: */
    else {
      regs = TRUE;
    }
  }

  /* else, if this connection is for the memory: */
  else if ((sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_ONBOARD
	    || sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_COBRA)
	   && TME_ARG_IS(args[1], "memory")) {

    /* if we already have a memory connection, complain: */
    if (sun_si->tme_sun_si_conn_memory != NULL) {
      rc = EEXIST;
    }
  }

  /* else, the connection must be for the ncr5380: */
  else if (args[1] == NULL) {
    
    /* if we already have an ncr5380 connection, complain: */
    if (sun_si->tme_sun_si_conn_ncr5380 != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new connection: */
    else {
      ncr5380 = TRUE;
    }
  }

  /* otherwise, this is a bad argument: */
  else {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    usage = TRUE;
  }

  if (usage) {
    tme_output_append_error(_output, 
			    ((sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_ONBOARD
			      || sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_COBRA)
			     ? "%s %s [ csr | memory ]"
			     : "%s %s [ csr ]"),
			    _("usage:"),
			    args[0]);
    rc = EINVAL;
  }
  
  if (rc) {
    return (rc);
  }

  /* make a new connection: */
  conn_sun_si = tme_new0(struct tme_sun_si_connection, 1);
  conn_bus = &conn_sun_si->tme_sun_si_connection;
  conn = &conn_bus->tme_bus_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_sun_si_connection_score;
  conn->tme_connection_make = _tme_sun_si_connection_make;
  conn->tme_connection_break = _tme_sun_si_connection_break;

  /* fill in the generic bus connection: */
  conn_bus->tme_bus_subregions.tme_bus_subregion_address_first = 0;
  conn_bus->tme_bus_subregions.tme_bus_subregion_next = NULL;
  if (ncr5380) {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = ((tme_bus_addr_t) 0) - 1;
    conn_bus->tme_bus_signals_add = NULL;
    conn_bus->tme_bus_signal = _tme_sun_si_bus_signal;
    conn_bus->tme_bus_tlb_set_add = _tme_sun_si_tlb_set_add;
    conn_bus->tme_bus_tlb_fill = _tme_sun_si_tlb_fill;
  }
  else if (regs) {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_SUN_SI_SIZ_REGS - 1;
    conn_bus->tme_bus_tlb_fill = _tme_sun_si_tlb_fill_regs;
    if (sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_VME
	|| sun_si->tme_sun_si_type == TME_SUN_SI_TYPE_3E) {
      conn_bus->tme_bus_intack = _tme_sun_si_intack;
    }
  }
  else {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = 0;
  }

  /* fill in the internal information: */
  conn_sun_si->tme_sun_si_connection_regs = regs;

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new sun_si function: */
int
tme_sun_si(struct tme_element *element, const char * const *args, char **_output)
{
  struct tme_sun_si *sun_si;
  int arg_i;
  int usage;
  tme_uint32_t si_type;

  /* check our arguments: */
  usage = 0;
  si_type = TME_SUN_SI_TYPE_NULL;
  arg_i = 1;
  for (;;) {

    /* the si type: */
    if (TME_ARG_IS(args[arg_i + 0], "type")) {
      if (TME_ARG_IS(args[arg_i + 1], "vme")) {
	si_type = TME_SUN_SI_TYPE_VME;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "onboard")) {
	si_type = TME_SUN_SI_TYPE_ONBOARD;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "3/E")) {
	si_type = TME_SUN_SI_TYPE_3E;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "cobra")) {
	si_type = TME_SUN_SI_TYPE_COBRA;
      }
      else {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* if we ran out of arguments: */
    else if (args[arg_i] == NULL) {

      break;
    }

    /* otherwise this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s, ",
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  if (si_type == TME_SUN_SI_TYPE_NULL) {
    usage = TRUE;
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s type { vme | onboard | 3/E | cobra }",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the sun_si structure: */
  sun_si = tme_new0(struct tme_sun_si, 1);
  sun_si->tme_sun_si_type = si_type;
  sun_si->tme_sun_si_3e_dma = (si_type == TME_SUN_SI_TYPE_3E
			       ? tme_new(tme_uint8_t, TME_SUN_SI_3E_SIZ_DMA)
			       : NULL);
  sun_si->tme_sun_si_element = element;
  TME_SUN_SI_CSR_PUT(sun_si,
		     ((si_type == TME_SUN_SI_TYPE_VME
		       ? TME_SUN_SI_CSR_VME_MODIFIED
		       : 0)
		      | (si_type == TME_SUN_SI_TYPE_3E
			 ? TME_SUN_SI_CSR_3E_VCC
			 : TME_SUN_SI_CSR_RESET_FIFO)
		      | TME_SUN_SI_CSR_RESET_CONTROLLER));
  sun_si->tme_sun_si_csr_ncr5380 = TME_SUN_SI_CSR_GET(sun_si);
  tme_mutex_init(&sun_si->tme_sun_si_mutex);
  tme_rwlock_init(&sun_si->tme_sun_si_rwlock);

  /* fill the element: */
  element->tme_element_private = sun_si;
  element->tme_element_connections_new = _tme_sun_si_connections_new;

  return (TME_OK);
}
