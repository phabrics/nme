/* $Id: lsi64854.c,v 1.2 2010/06/05 14:53:16 fredette Exp $ */

/* ic/lsi64854.c - LSI 64854 emulation: */

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
_TME_RCSID("$Id: lsi64854.c,v 1.2 2010/06/05 14:53:16 fredette Exp $");

/* includes: */
#include <tme/element.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>

/* macros: */

/* channels: */
#define TME_LSI64854_CHANNEL_NULL		(0)
#define TME_LSI64854_CHANNEL_SCSI		(1)
#define TME_LSI64854_CHANNEL_ENET		(2)
#define TME_LSI64854_CHANNEL_PPRT		(3)

/* register offsets: */
#define TME_LSI64854_REG_X_CSR			(sizeof(tme_uint32_t) * 0)
#define TME_LSI64854_REG_SG_ADDRESS		(sizeof(tme_uint32_t) * 1)
#define TME_LSI64854_REG_SG_COUNT		(sizeof(tme_uint32_t) * 2)
#define TME_LSI64854_REG_SCSI_TEST		(sizeof(tme_uint32_t) * 3)
#define TME_LSI64854_SIZ_SCSI			(sizeof(tme_uint32_t) * 4)
#define TME_LSI64854_REG_ENET_TEST		(sizeof(tme_uint32_t) * 1)
#define TME_LSI64854_REG_ENET_CACHE_VALID	(sizeof(tme_uint32_t) * 2)
#define TME_LSI64854_REG_ENET_BASE		(sizeof(tme_uint32_t) * 3)
#define TME_LSI64854_REG_ENET_CSR_OTHER		(sizeof(tme_uint32_t) * 4)
#define TME_LSI64854_SIZ_ENET			(sizeof(tme_uint32_t) * 5)
#define TME_LSI64854_REG_PPRT_HCR		(0x10)
#define TME_LSI64854_REG_PPRT_OCR		(0x12)
#define TME_LSI64854_REG_PPRT_DR		(0x14)
#define TME_LSI64854_REG_PPRT_TCR		(0x15)
#define TME_LSI64854_REG_PPRT_OR		(0x16)
#define TME_LSI64854_REG_PPRT_IR		(0x17)
#define TME_LSI64854_REG_PPRT_ICR		(0x18)
#define TME_LSI64854_SIZ_PPRT			(0x1a)
#define TME_LSI64854_SIZ_X			(sizeof(tme_uint32_t) * 4)

/* master registers: */
#define TME_LSI64854_SIZ_NCR53C9X		(sizeof(tme_uint32_t) * 0x10)
#define TME_LSI64854_SIZ_AM7990			(sizeof(tme_uint16_t) * 2)

/* the common CSR: */
#define TME_LSI64854_CSR_X_INT_PENDING		(0x00000001)
#define TME_LSI64854_CSR_X_ERR_PENDING		(0x00000002)
#define TME_LSI64854_CSR_X_FIFO_PACK_COUNT	(0x0000000c)
#define TME_LSI64854_CSR_X_INT_ENABLE		(0x00000010)
#define TME_LSI64854_CSR_X_FIFO_INVALIDATE	(0x00000020)
#define TME_LSI64854_CSR_X_SLAVE_ERROR		(0x00000040)
#define TME_LSI64854_CSR_X_RESET		(0x00000080)
#define TME_LSI64854_CSR_X_MEMORY_WRITE		(0x00000100)
#define TME_LSI64854_CSR_X_DMA_ENABLE		(0x00000200)
#define TME_LSI64843_CSR_X_REQ_PENDING		(0x00000400)
#define TME_LSI64854_CSR_X_REV_MASK		(0xf0000000)
#define  TME_LSI64854_CSR_X_REV_1PLUS		 (0x90000000)
#define  TME_LSI64854_CSR_X_REV_2		 (0xa0000000)

/* the common scatter/gather CSR: */
#define TME_LSI64854_CSR_SG_COUNT_ENABLE	(0x00002000)
#define TME_LSI64854_CSR_SG_COUNT_TERMINAL	(0x00004000)
#define TME_LSI64854_CSR_SG_NO_INT_COUNT_TERM	(0x00800000)
#define TME_LSI64854_CSR_SG_CHAIN_ENABLE	(0x01000000)
#define TME_LSI64854_CSR_SG_DMA_ON		(0x02000000)
#define TME_LSI64854_CSR_SG_ADDRESS_LOADED	(0x04000000)
#define TME_LSI64854_CSR_SG_ADDRESS_NEXT_LOADED	(0x08000000)

/* connection types: */
#define TME_LSI64854_CONN_NULL			(0)
#define TME_LSI64854_CONN_REGS			(1)
#define TME_LSI64854_CONN_REGS_MASTER		(2)
#define TME_LSI64854_CONN_MASTER		(3)

/* predicates: */
#define TME_LSI64854_CHANNEL_HAS_MASTER(lsi64854)	\
  ((lsi64854)->tme_lsi64854_channel != TME_LSI64854_CHANNEL_PPRT)
#define TME_LSI64854_REV_HAS_SLAVE_ERROR(lsi64854)	\
  (TRUE)
#define TME_LSI64854_CHANNEL_HAS_SG(lsi64854)		\
  ((lsi64854)->tme_lsi64854_channel != TME_LSI64854_CHANNEL_ENET)
#define TME_LSI64854_CHANNEL_HAS_SG_CHAINING(lsi64854)	\
  (TRUE)

/* the callout flags: */
#define TME_LSI64854_CALLOUTS_RUNNING	TME_BIT(0)
#define TME_LSI64854_CALLOUTS_MASK	(-2)
#define  TME_LSI64854_CALLOUT_SIGNALS	TME_BIT(1)
#define	 TME_LSI64854_CALLOUT_INT	TME_BIT(2)

#if 1
#define TME_LSI64854_DEBUG
#endif

/* structures: */

/* one independent channel in an LSI 64854 chip: */
struct tme_lsi64854 {

  /* backpointer to our element: */
  struct tme_element *tme_lsi64854_element;

  /* the mutex protecting the channel: */
  tme_mutex_t tme_lsi64854_mutex;

  /* which channel: */
  tme_uint32_t tme_lsi64854_channel;

  /* the CSR: */
  tme_uint32_t tme_lsi64854_csr;

  /* the address register: */
  tme_uint32_t tme_lsi64854_address;

  /* the count register: */
  tme_uint32_t tme_lsi64854_count;

  /* the bus connection for the channel's registers: */
  struct tme_bus_connection *tme_lsi64854_conn_regs;

  /* if this is the SCSI or Ethernet channel, the bus connection for
     the master's registers: */
  struct tme_bus_connection *tme_lsi64854_conn_regs_master;

  /* if this is the SCSI or Ethernet channel, the bus connection for
     the channel's master: */
  struct tme_bus_connection *tme_lsi64854_conn_master;

  /* the callout flags: */
  int tme_lsi64854_callout_flags;

  /* this is nonzero if the interrupt is asserted: */
  int tme_lsi64854_int_asserted;

  /* any outstanding master TLB entry: */
  struct tme_token *tme_lsi64854_master_tlb_token;

  /* a CSR subset between the channel and master: */
  tme_uint32_t tme_lsi64854_csr_master;

  /* parallel-port specific registers: */
  tme_uint16_t tme_lsi64854_pprt_hcr;
  tme_uint16_t tme_lsi64854_pprt_icr;
};

/* a lsi64854 internal bus connection: */
struct tme_lsi64854_connection {

  /* the external bus connection: */
  struct tme_bus_connection tme_lsi64854_connection;

  /* this is nonzero if a TME_CONNECTION_BUS_GENERIC chip connection
     is for the registers: */
  unsigned int tme_lsi64854_connection_which;
};

#define TME_LSI64854_DEBUG_REG_READ	(0)
#define TME_LSI64854_DEBUG_REG_WRITE	(1)
#define TME_LSI64854_DEBUG_REG_PUT	(2)
#ifdef TME_LSI64854_DEBUG
void
_tme_lsi64854_debug_reg(struct tme_lsi64854 *lsi64854,
			const tme_uint32_t *_reg,
			unsigned int why,
			tme_uint32_t value_new)
{
  const char *why_name;
  const char *reg_name;

  switch (why) {
  case TME_LSI64854_DEBUG_REG_READ:
    why_name = "rd"; 
    break;
  case TME_LSI64854_DEBUG_REG_WRITE:
    why_name = "wr"; 
    break;
  default: assert (FALSE);
  case TME_LSI64854_DEBUG_REG_PUT:
    if (*_reg == value_new) {
      return;
    }
    why_name = "<-";
    break;
  }

  /* try to get the name of this register: */
  reg_name = NULL;
  if (_reg == &lsi64854->tme_lsi64854_csr) {
    reg_name = "CSR";
  }
  else if (_reg == &lsi64854->tme_lsi64854_address) {
    reg_name = "address";
  }
  else if (_reg == &lsi64854->tme_lsi64854_count) {
    reg_name = "count";
  }
  if (reg_name == NULL) {
    reg_name = "???";
  }
  tme_log(&lsi64854->tme_lsi64854_element->tme_element_log_handle,
	  100, TME_OK,
	  (&lsi64854->tme_lsi64854_element->tme_element_log_handle,
	   "%s %s 0x%04x",
	   reg_name,
	   why_name,
	   value_new));
}
#else  /* !TME_LSI64854_DEBUG */
#define _tme_lsi64854_debug_reg(l, r, w, v) do { } while (/* CONSTCOND */ 0 && (l) && (r) && (w) && (v))
#endif /* !TME_LSI64854_DEBUG */
#define TME_LSI64854_REG_PUT(lsi64854, field, value)	\
  do {							\
    _tme_lsi64854_debug_reg((lsi64854), &(lsi64854)->field, TME_LSI64854_DEBUG_REG_PUT, (value)); \
    (lsi64854)->field = (value);			\
  } while (/* CONSTCOND */ 0)

/* this resets the lsi64854: */
static void
_tme_lsi64854_reset(struct tme_lsi64854 *lsi64854)
{

  tme_log(&lsi64854->tme_lsi64854_element->tme_element_log_handle,
	  100, TME_OK,
	  (&lsi64854->tme_lsi64854_element->tme_element_log_handle,
	   "reset"));

  /* clear all pending callouts: */
  lsi64854->tme_lsi64854_callout_flags &= TME_LSI64854_CALLOUTS_MASK;

  /* an Ethernet channel apparently comes out of reset with its
     address register set to this: */
  if (lsi64854->tme_lsi64854_channel == TME_LSI64854_CHANNEL_ENET) {
    lsi64854->tme_lsi64854_address = 0xff000000;
  }
}

/* this returns the count of bytes that the master can DMA: */
static tme_bus_addr_t
_tme_lsi64854_dma_count(struct tme_lsi64854 *lsi64854)
{
  tme_uint32_t csr;

  /* get the CSR: */
  csr = lsi64854->tme_lsi64854_csr;

  /* if this is not a scatter/gather channel, return zero: */
  if (!TME_LSI64854_CHANNEL_HAS_SG(lsi64854)) {
    return (0);
  }

  /* if DMA is disabled, return zero: */
  if (!(csr & TME_LSI64854_CSR_X_DMA_ENABLE)) {
    return (0);
  }

  /* if this is a scatter/gather channel that supports chaining,
     and the count register is enabled: */
  if (TME_LSI64854_CHANNEL_HAS_SG_CHAINING(lsi64854)
      && (csr & TME_LSI64854_CSR_SG_COUNT_ENABLE)) {

    abort();
  }

  /* otherwise, this scatter/gather channel either doesn't support
     chaining, or the count register is disabled: */
  else {

    /* return an very large byte count: */
    return (((tme_bus_addr_t) 0) - 1);
  }
}

/* the lsi64854 callout function.  it must be called with the mutex locked: */
static void
_tme_lsi64854_callout(struct tme_lsi64854 *lsi64854)
{
  struct tme_bus_connection *conn_master;
  struct tme_bus_connection *conn_bus;
  tme_uint32_t csr;
  unsigned int signal;
  int again;
  int rc;
  int int_asserted;
  
  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (lsi64854->tme_lsi64854_callout_flags & TME_LSI64854_CALLOUTS_RUNNING) {
    return;
  }

  /* callouts are now running: */
  lsi64854->tme_lsi64854_callout_flags |= TME_LSI64854_CALLOUTS_RUNNING;

  /* loop while we have work to do: */
  do {
    again = FALSE;

    /* assume that we have no bus signal for the master: */
    signal = TME_BUS_SIGNAL_IGNORE;

    /* if DMA is possible but the master doesn't know that: */
    if (_tme_lsi64854_dma_count(lsi64854) > 0
	&& !(lsi64854->tme_lsi64854_csr_master & TME_LSI64854_CSR_X_DMA_ENABLE)) {

      /* tell the master that DMA is possible: */
      signal = TME_BUS_SIGNAL_DACK | TME_BUS_SIGNAL_LEVEL_ASSERTED;
      lsi64854->tme_lsi64854_csr_master |= TME_LSI64854_CSR_X_DMA_ENABLE;
    }

    /* if we have a bus signal for the master: */
    if (signal != TME_BUS_SIGNAL_IGNORE) {
      again = TRUE;

      /* get this card's master connection: */
      conn_master = lsi64854->tme_lsi64854_conn_master;

      /* unlock the mutex: */
      tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);
      
      /* do the callout: */
      rc = (conn_master != NULL
	    ? ((*conn_master->tme_bus_signal)
	       (conn_master,
		signal))
	    : TME_OK);
      assert (rc == TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&lsi64854->tme_lsi64854_mutex);
    }

    /* see if the interrupt signal should be asserted or negated: */
    int_asserted = FALSE;
    csr = lsi64854->tme_lsi64854_csr;
    if (csr & TME_LSI64854_CSR_X_INT_ENABLE) {
      if (csr & (TME_LSI64854_CSR_X_INT_PENDING
		 | TME_LSI64854_CSR_X_ERR_PENDING)) {
	int_asserted = TRUE;
      }
    }

    /* if the interrupt signal doesn't already have the right state: */
    if (!int_asserted != !lsi64854->tme_lsi64854_int_asserted) {
      again = TRUE;

      /* note the new state of the interrupt signal: */
      lsi64854->tme_lsi64854_int_asserted = int_asserted;

      /* get the bus connection for the interrupt signal: */
      conn_bus
	= (TME_LSI64854_CHANNEL_HAS_MASTER(lsi64854)
	   ? lsi64854->tme_lsi64854_conn_regs_master
	   : lsi64854->tme_lsi64854_conn_regs);
	
      /* unlock our mutex: */
      tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);
	
      /* call out the bus interrupt signal edge: */
      rc = (conn_bus != NULL
	    ? ((*conn_bus->tme_bus_signal)
	       (conn_bus,
		TME_BUS_SIGNAL_INT_UNSPEC
		| TME_BUS_SIGNAL_EDGE
		| (int_asserted
		   ? TME_BUS_SIGNAL_LEVEL_ASSERTED
		   : TME_BUS_SIGNAL_LEVEL_NEGATED)))
	    : TME_OK);
      assert (rc == TME_OK);

      /* lock our mutex: */
      tme_mutex_lock(&lsi64854->tme_lsi64854_mutex);
    }
  } while (again);

  /* clear that callouts are running: */
  lsi64854->tme_lsi64854_callout_flags &= ~TME_LSI64854_CALLOUTS_RUNNING;
}

/* the lsi64854 bus cycle handler for the board registers: */
static int
_tme_lsi64854_bus_cycle_regs(void *_lsi64854, 
			     struct tme_bus_cycle *cycle_init)
{
  struct tme_lsi64854 *lsi64854;
  tme_bus_addr32_t reg;
  tme_uint8_t value8;
  tme_uint16_t value16;
  tme_uint32_t value;
  tme_uint32_t csr_old;
  tme_uint32_t csr_mask;
  tme_uint32_t csr_new;

  /* recover our data structure: */
  lsi64854 = (struct tme_lsi64854 *) _lsi64854;

  /* we only emulate aligned 8-, 16, and 32-bit accesses: */
  reg = cycle_init->tme_bus_cycle_address;
  if (cycle_init->tme_bus_cycle_size > sizeof(tme_uint32_t)
      || cycle_init->tme_bus_cycle_size == (sizeof(tme_uint16_t) + sizeof(tme_uint8_t))
      || (reg & (cycle_init->tme_bus_cycle_size - 1)) != 0) {
    abort();
  }

  /* lock the mutex: */
  tme_mutex_lock(&lsi64854->tme_lsi64854_mutex);

  /* if this is an 8- or 16-bit access: */
  if (cycle_init->tme_bus_cycle_size < sizeof(tme_uint32_t)) {

    /* this must be a parallel channel: */
    if (lsi64854->tme_lsi64854_channel != TME_LSI64854_CHANNEL_PPRT) {
      abort();
    }

    /* if this is a write: */
    if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

      /* run the bus cycle: */
      if (cycle_init->tme_bus_cycle_size != sizeof(tme_uint16_t)) {
	abort();
      }
      tme_bus_cycle_xfer_reg(cycle_init, 
			     &value16,
			     TME_BUS16_LOG2);

      /* dispatch on the register: */
      switch (reg) {
      default: abort();
      case TME_LSI64854_REG_PPRT_HCR:
	lsi64854->tme_lsi64854_pprt_hcr = value16;
	break;
      case TME_LSI64854_REG_PPRT_ICR:
	lsi64854->tme_lsi64854_pprt_icr = value16;
	break;
      }
    }

    /* otherwise, this must be a read: */
    else {
      assert (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

      /* dispatch on the register: */
      switch (reg) {
      default: abort();
      case TME_LSI64854_REG_PPRT_HCR:
	value16 = lsi64854->tme_lsi64854_pprt_hcr;
	break;
      case TME_LSI64854_REG_PPRT_OCR:
	value16 = 0;
	break;
      case TME_LSI64854_REG_PPRT_DR:
	value16 = 0;
	break;
      case TME_LSI64854_REG_PPRT_TCR:
	value16 = 0;
	break;
      case TME_LSI64854_REG_PPRT_OR:
	value16 = 0;
	break;
      case TME_LSI64854_REG_PPRT_IR:
	value16 = 0;
	break;
      case TME_LSI64854_REG_PPRT_ICR:
	value16 = lsi64854->tme_lsi64854_pprt_icr;
	break;
      }

      /* run the bus cycle: */
      if (cycle_init->tme_bus_cycle_size == sizeof(tme_uint8_t)) {
	value8 = value16;
	tme_bus_cycle_xfer_reg(cycle_init, 
			       &value8,
			       TME_BUS8_LOG2);
      }
      else {
	tme_bus_cycle_xfer_reg(cycle_init, 
			       &value16,
			       TME_BUS16_LOG2);
      }
    }

    /* unlock the mutex: */
    tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);

    /* no faults: */
    return (TME_OK);
  }

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init, 
			   &value,
			   TME_BUS32_LOG2);

    /* if this is a write to the CSR register: */
    if (reg == TME_LSI64854_REG_X_CSR) {

      /* get the old CSR value: */
      csr_old = lsi64854->tme_lsi64854_csr;

      /* make the new CSR value, preserving the read-only bits: */
      csr_mask
	= (TME_LSI64854_CSR_X_INT_PENDING
	   | TME_LSI64854_CSR_X_ERR_PENDING
	   | TME_LSI64854_CSR_X_FIFO_PACK_COUNT
	   | TME_LSI64854_CSR_X_FIFO_INVALIDATE
	   | (TME_LSI64854_REV_HAS_SLAVE_ERROR(lsi64854)
	      ? TME_LSI64854_CSR_X_SLAVE_ERROR
	      : 0)
	   | TME_LSI64843_CSR_X_REQ_PENDING
	   | TME_LSI64854_CSR_X_REV_MASK
	   | (TME_LSI64854_CHANNEL_HAS_SG(lsi64854)
	      ? (TME_LSI64854_CSR_SG_COUNT_TERMINAL
		 | TME_LSI64854_CSR_SG_DMA_ON
		 | TME_LSI64854_CSR_SG_ADDRESS_LOADED
		 | TME_LSI64854_CSR_SG_ADDRESS_NEXT_LOADED)
	      : 0));
      csr_new = (value & ~csr_mask) | (csr_old & csr_mask);

      /* write the CSR register: */
      _tme_lsi64854_debug_reg(lsi64854, &lsi64854->tme_lsi64854_csr, TME_LSI64854_DEBUG_REG_WRITE, csr_new);
      lsi64854->tme_lsi64854_csr = csr_new;

      /* if we're being reset: */
      if (csr_new & TME_LSI64854_CSR_X_RESET) {
	
	/* reset the channel: */
	_tme_lsi64854_reset(lsi64854);
      }
    }

    /* otherwise, if this is a scatter/gather channel and this is a
       write to the address register: */
    else if (TME_LSI64854_CHANNEL_HAS_SG(lsi64854)
	     && reg == TME_LSI64854_REG_SG_ADDRESS) {

      /* if this scatter/gather channel supports chaining, and
	 chaining is enabled: */
      if (TME_LSI64854_CHANNEL_HAS_SG_CHAINING(lsi64854)
	  && (lsi64854->tme_lsi64854_csr & TME_LSI64854_CSR_SG_CHAIN_ENABLE)) {

	abort();
      }

      /* otherwise, chaining is not supported or is disabled: */
      else {
	  
	/* write the address register: */
	_tme_lsi64854_debug_reg(lsi64854, &lsi64854->tme_lsi64854_address, TME_LSI64854_DEBUG_REG_WRITE, value);
	lsi64854->tme_lsi64854_address = value;
      }
    }

    /* otherwise, if this is a scatter/gather channel and this is a
       write to the count register: */
    else if (TME_LSI64854_CHANNEL_HAS_SG(lsi64854)
	     && reg == TME_LSI64854_REG_SG_COUNT) {

      /* if this scatter/gather channel supports chaining, and
	 chaining is enabled: */
      if (TME_LSI64854_CHANNEL_HAS_SG_CHAINING(lsi64854)
	  && (lsi64854->tme_lsi64854_csr & TME_LSI64854_CSR_SG_CHAIN_ENABLE)) {

	abort();
      }

      /* otherwise, chaining is not supported or is disabled: */
      else {

	/* write the count register: */
	_tme_lsi64854_debug_reg(lsi64854, &lsi64854->tme_lsi64854_count, TME_LSI64854_DEBUG_REG_WRITE, value);
	lsi64854->tme_lsi64854_count = value;
      }
    }

    /* otherwise, if this is an Ethernet channel and this is a write
       to the base register: */
    else if (lsi64854->tme_lsi64854_channel == TME_LSI64854_CHANNEL_ENET
	     && reg == TME_LSI64854_REG_ENET_BASE) {

      /* write the address register: */
      value &= 0xff000000;
      _tme_lsi64854_debug_reg(lsi64854, &lsi64854->tme_lsi64854_address, TME_LSI64854_DEBUG_REG_WRITE, value);
      lsi64854->tme_lsi64854_address = value;
    }

    /* any other register: */
    else {
      abort();
    }
  }

  /* otherwise, if this isn't a read: */
  else if (cycle_init->tme_bus_cycle_type != TME_BUS_CYCLE_READ) {
    abort();
  }

  /* otherwise, this is a read: */
  else {

    /* if this is a read of the CSR register: */
    if (reg == TME_LSI64854_REG_X_CSR) {

      /* read the CSR register: */
      value = lsi64854->tme_lsi64854_csr;
      _tme_lsi64854_debug_reg(lsi64854, &lsi64854->tme_lsi64854_csr, TME_LSI64854_DEBUG_REG_READ, value);

      /* some bits in the CSR reset when read: */
      csr_new = value;
      if (TME_LSI64854_REV_HAS_SLAVE_ERROR(lsi64854)) {
	csr_new &= ~TME_LSI64854_CSR_X_SLAVE_ERROR;
      }
      TME_LSI64854_REG_PUT(lsi64854, tme_lsi64854_csr, csr_new);
    }

    /* otherwise, if this is a scatter/gather channel and this is a
       read of the address register: */
    else if (TME_LSI64854_CHANNEL_HAS_SG(lsi64854)
	     && reg == TME_LSI64854_REG_SG_ADDRESS) {

      /* if this scatter/gather channel supports chaining, and
	 chaining is enabled: */
      if (TME_LSI64854_CHANNEL_HAS_SG_CHAINING(lsi64854)
	  && (lsi64854->tme_lsi64854_csr & TME_LSI64854_CSR_SG_CHAIN_ENABLE)) {

	abort();
      }

      /* otherwise, chaining is not supported or is disabled: */
      else {
	  
	/* read the address register: */
	value = lsi64854->tme_lsi64854_address;
	_tme_lsi64854_debug_reg(lsi64854, &lsi64854->tme_lsi64854_address, TME_LSI64854_DEBUG_REG_READ, value);
      }
    }

    /* otherwise, if this is a scatter/gather channel and this is a
       read of the count register: */
    else if (TME_LSI64854_CHANNEL_HAS_SG(lsi64854)
	     && reg == TME_LSI64854_REG_SG_COUNT) {

      /* if this scatter/gather channel supports chaining, and
	 chaining is enabled: */
      if (TME_LSI64854_CHANNEL_HAS_SG_CHAINING(lsi64854)
	  && (lsi64854->tme_lsi64854_csr & TME_LSI64854_CSR_SG_CHAIN_ENABLE)) {

	abort();
      }

      /* otherwise, chaining is not supported or is disabled: */
      else {

	/* read the count register: */
	value = lsi64854->tme_lsi64854_count;
	_tme_lsi64854_debug_reg(lsi64854, &lsi64854->tme_lsi64854_count, TME_LSI64854_DEBUG_REG_READ, value);
      }
    }

    /* otherwise, if this is an Ethernet channel: */
    else if (lsi64854->tme_lsi64854_channel == TME_LSI64854_CHANNEL_ENET) {

      /* if this is a read of the undocumented other CSR: */
      if (reg == TME_LSI64854_REG_ENET_CSR_OTHER) {

	/* return a good value: */
	value = (lsi64854->tme_lsi64854_csr & TME_LSI64854_CSR_X_REV_MASK);
      }

      /* any other register: */
      else {
	abort();
      }
    }

    /* otherwise, if this is a parallel channel: */
    else if (lsi64854->tme_lsi64854_channel == TME_LSI64854_CHANNEL_PPRT) {

      /* if this is a read of HCR and OCR: */
      if (reg == TME_LSI64854_REG_PPRT_HCR) {

	/* read HCR and OCR: */
	value = lsi64854->tme_lsi64854_pprt_hcr;
	value <<= 16;
      }

      /* any other register: */
      else {
	abort();
      }
    }

    /* any other register: */
    else {
      abort();
    }

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init, 
			   &value,
			   TME_BUS32_LOG2);
  }

  /* make any new callouts: */
  _tme_lsi64854_callout(lsi64854);

  /* unlock the mutex: */
  tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the lsi64854 TLB filler for the channel registers: */
static int
_tme_lsi64854_tlb_fill_regs(struct tme_bus_connection *conn_bus,
			    struct tme_bus_tlb *tlb, 
			    tme_bus_addr_t address,
			    unsigned int cycles)
{
  struct tme_lsi64854 *lsi64854;

  /* recover our data structures: */
  lsi64854 = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* dispatch on the channel: */
  switch (lsi64854->tme_lsi64854_channel) {
  default: assert(FALSE);
  case TME_LSI64854_CHANNEL_SCSI:
    tlb->tme_bus_tlb_addr_last = TME_LSI64854_SIZ_SCSI - 1;
    break;
  case TME_LSI64854_CHANNEL_ENET:
    tlb->tme_bus_tlb_addr_last = TME_LSI64854_SIZ_ENET - 1;
    break;
  case TME_LSI64854_CHANNEL_PPRT:
    tlb->tme_bus_tlb_addr_last = TME_LSI64854_SIZ_PPRT - 1;
    break;
  }

  /* the address must be within range: */
  tlb->tme_bus_tlb_addr_first = 0;
  assert(address <= tlb->tme_bus_tlb_addr_last);

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = lsi64854;
  tlb->tme_bus_tlb_cycle = _tme_lsi64854_bus_cycle_regs;

  return (TME_OK);
}

/* the lsi64854 TLB filler for the master registers: */
static int
_tme_lsi64854_tlb_fill_regs_master(struct tme_bus_connection *conn_bus,
				   struct tme_bus_tlb *tlb, 
				   tme_bus_addr_t address,
				   unsigned int cycles)
{
  struct tme_lsi64854 *lsi64854;
  struct tme_bus_connection *conn_master;
  tme_bus_addr32_t address_mask;
  int address_shift;
  int rc;

  /* recover our data structures: */
  lsi64854 = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the master connection: */
  conn_master = lsi64854->tme_lsi64854_conn_master;

  /* dispatch on the channel: */
  switch (lsi64854->tme_lsi64854_channel) {
  default: assert(FALSE);
  case TME_LSI64854_CHANNEL_SCSI:

    /* each 8-bit NCR 53c9x register is at a 32-bit aligned address: */
    address_mask = TME_LSI64854_SIZ_NCR53C9X - 1;
    address_shift = 2; /* log2(sizeof(tme_uint32_t)) */
    break;

  case TME_LSI64854_CHANNEL_ENET:

    /* each 16-bit am7990 register is at a 16-bit aligned address: */
    address_mask = TME_LSI64854_SIZ_AM7990 - 1;
    address_shift = 0;
    break;
  }

  assert (address <= address_mask);

  /* call the master TLB fill function: */
  rc = (conn_master != NULL
	? (*conn_master->tme_bus_tlb_fill)(conn_master,
					   tlb,
					   (address >> address_shift),
					   cycles)
	: EINVAL);

  /* if that succeeded: */
  if (rc == TME_OK) {

    /* add the shift to the TLB entry: */
    tlb->tme_bus_tlb_addr_first <<= address_shift;
    tlb->tme_bus_tlb_addr_last <<= address_shift;
    tlb->tme_bus_tlb_addr_shift += address_shift;
  }

  return (rc);
}

/* the lsi64854 bus signal handler for the master: */
static int
_tme_lsi64854_bus_signal(struct tme_bus_connection *conn_bus, 
			 unsigned int signal)
{
  struct tme_lsi64854 *lsi64854;
  tme_uint32_t csr;

  /* this must be the unspecified interrupt signal: */
  assert (TME_BUS_SIGNAL_WHICH(signal) == TME_BUS_SIGNAL_INT_UNSPEC);

  /* recover our data structures: */
  lsi64854 = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&lsi64854->tme_lsi64854_mutex);

  /* update the CSR value: */
  csr = lsi64854->tme_lsi64854_csr;
  csr = ((csr
	  & ~TME_LSI64854_CSR_X_INT_PENDING)
	 | (((signal & TME_BUS_SIGNAL_LEVEL_MASK)
	     == TME_BUS_SIGNAL_LEVEL_ASSERTED)
	    ? TME_LSI64854_CSR_X_INT_PENDING
	    : 0));
  TME_LSI64854_REG_PUT(lsi64854, tme_lsi64854_csr, csr);

  /* make any new callouts: */
  _tme_lsi64854_callout(lsi64854);

  /* unlock our mutex: */
  tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);

  return (TME_OK);
}

/* the lsi64854 bus fault handler for the master DMA: */
static int
_tme_lsi64854_bus_fault_handler(void *_lsi64854, 
				struct tme_bus_tlb *tlb,
				struct tme_bus_cycle *cycle,
				int rc)
{
  struct tme_lsi64854 *lsi64854;

  /* recover our data structure: */
  lsi64854 = (struct tme_lsi64854 *) _lsi64854;

  /* lock our mutex: */
  tme_mutex_lock(&lsi64854->tme_lsi64854_mutex);

  /* set a DMA bus error and call out an interrupt change: */
  TME_LSI64854_REG_PUT(lsi64854, tme_lsi64854_csr,
		       (lsi64854->tme_lsi64854_csr
			| TME_LSI64854_CSR_X_ERR_PENDING));
  _tme_lsi64854_callout(lsi64854);    

  /* unlock our mutex: */
  tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);
  
  return (rc);
}

/* the lsi64854 TLB filler for the master DMA: */
static int
_tme_lsi64854_tlb_fill(struct tme_bus_connection *conn_bus,
		       struct tme_bus_tlb *tlb, 
		       tme_bus_addr_t master_address_wider,
		       unsigned int cycles)
{
  struct tme_lsi64854 *lsi64854;
  tme_bus_addr_t master_address;
  tme_uint32_t csr;
  tme_bus_addr32_t dma_address;
  tme_bus_addr32_t dma_count;
  tme_uint32_t bpr;
  unsigned int bpr_count;
#ifdef TME_LSI64854_STRICT_PACK
  unsigned int bpr_i;
#endif /* TME_LSI64854_STRICT_PACK */
  struct tme_bus_tlb tlb_mapping;
  int rc;

  /* recover our data structures: */
  lsi64854 = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* assume that this call will succeed: */
  rc = TME_OK;

  /* lock our mutex: */
  tme_mutex_lock(&lsi64854->tme_lsi64854_mutex);

  /* get the normal-width address: */
  master_address = master_address_wider;
  assert (master_address == master_address_wider);

  /* if the master already has an outstanding TLB entry, and it
     doesn't happen to be this TLB entry, invalidate it: */
  if (lsi64854->tme_lsi64854_master_tlb_token != NULL
      && (tlb == NULL
	  || (lsi64854->tme_lsi64854_master_tlb_token 
	      != tlb->tme_bus_tlb_token))) {
    tme_token_invalidate(lsi64854->tme_lsi64854_master_tlb_token);

    /* the master doesn't have any outstanding TLB entry: */
    lsi64854->tme_lsi64854_master_tlb_token = NULL;
  }

  /* get the CSR value: */
  csr = lsi64854->tme_lsi64854_csr;

  /* dispatch on the channel: */
  switch (lsi64854->tme_lsi64854_channel) {
  default: assert(FALSE);
  case TME_LSI64854_CHANNEL_SCSI:

    /* get the DMA address register, plus the NCR 53c9x address, and
       the DMA count register: */
    dma_address = lsi64854->tme_lsi64854_address + master_address;
    dma_count = _tme_lsi64854_dma_count(lsi64854);

    /* if the NCR 53c9x has stopped doing DMA, or if it has exhausted
       DMA, or if DMA is not enabled: */
    if (cycles == TME_BUS_CYCLE_UNDEF
	|| master_address >= dma_count
	|| ((csr
	     & (TME_LSI64854_REV_HAS_SLAVE_ERROR(lsi64854)
		? (TME_LSI64854_CSR_X_ERR_PENDING
		   | TME_LSI64854_CSR_X_SLAVE_ERROR
		   | TME_LSI64854_CSR_X_DMA_ENABLE)
		: (TME_LSI64854_CSR_X_ERR_PENDING
		   | TME_LSI64854_CSR_X_DMA_ENABLE)))
	    != TME_LSI64854_CSR_X_DMA_ENABLE)) {

      /* we assume that the NCR 53c9x doesn't have the NCR 5380
	 prefetch problem: */
      if (master_address > dma_count) {
	abort();
      }

      /* update the DMA address and count registers: */
      TME_LSI64854_REG_PUT(lsi64854, tme_lsi64854_address, dma_address);

      /* assume that our emulated byte pack is empty: */
      bpr = 0;
      bpr_count = 0;

#ifdef TME_LSI64854_STRICT_PACK

      /* if the NCR 53c9x was doing DMA writes to memory (i.e., it was
         receiving): */
      if (csr & TME_LSI64854_CSR_X_MEMORY_WRITE) {

	/* calculate how many bytes must be left in the byte pack: */
	bpr_count = master_address % sizeof(tme_uint32_t);
      }

      /* otherwise, the NCR 53c9x was doing DMA reads to memory (i.e.,
	 it was sending): */
      else {

	/* nothing to do.  we don't emulate the byte pack for sending: */
      }

#endif  /* TME_LSI64854_STRICT_PACK */

      /* update the byte pack and the CSR: */
      TME_FIELD_MASK_DEPOSITU(csr, TME_LSI64854_CSR_X_FIFO_PACK_COUNT, bpr_count);
      TME_LSI64854_REG_PUT(lsi64854, tme_lsi64854_csr, csr);

      /* return EAGAIN to the NCR 53c9x, unless it was the NCR 53c9x
	 that stopped doing DMA: */
      if (cycles != TME_BUS_CYCLE_UNDEF) {
	rc = EAGAIN;
      }
      break;
    }
    break;

  case TME_LSI64854_CHANNEL_ENET:

    /* get the DMA address register, plus the am7990 address: */
    dma_address = lsi64854->tme_lsi64854_address + master_address;

    /* the am7990 has a 24-bit address space: */
    dma_count = (1 << 24);
    break;
  }

  /* if DMA is not ready for some reason: */
  if (rc == EAGAIN) {

    /* initialize any local TLB entry passed by the caller.  this
       will make it unusable: */
    if (tlb != NULL) {
      tme_bus_tlb_initialize(tlb);
    }

    /* cancel the remainder of the TLB fill: */
    cycles = TME_BUS_CYCLE_UNDEF;

    /* remember that we have told the master that DMA is not possible: */
    lsi64854->tme_lsi64854_csr_master &= ~TME_LSI64854_CSR_X_DMA_ENABLE;
  }

  /* get the bus connection for memory: */
  conn_bus = lsi64854->tme_lsi64854_conn_regs;

  /* if we need to fill a TLB entry on the bus: */
  if (cycles != TME_BUS_CYCLE_UNDEF) {

    /* the master now has this outstanding TLB entry: */
    lsi64854->tme_lsi64854_master_tlb_token
      = tlb->tme_bus_tlb_token;
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);

  /* if we need to fill a TLB entry on the bus: */
  if (cycles != TME_BUS_CYCLE_UNDEF) {

    /* DMA must be OK so far: */
    assert (rc == TME_OK);
    assert (dma_count > master_address);

    /* call out to fill this TLB entry on the bus: */
    rc = (conn_bus != NULL
	  ? (*conn_bus->tme_bus_tlb_fill)(conn_bus, 
					  tlb,
					  dma_address,
					  cycles)
	  : ENXIO);

    /* if this callout succeeded: */
    if (rc == TME_OK) {

#ifdef TME_LSI64854_STRICT_PACK

      /* when the master wants to do DMA writes, in order to correctly
	 emulate the byte pack FIFO without having to implement it
	 literally, we just insist that all DMA be done to memory that
	 supports fast access: */
      if ((cycles & TME_BUS_CYCLE_WRITE)
	  && tlb->tme_bus_tlb_emulator_off_write == TME_EMULATOR_OFF_UNDEF) {
	abort();
      }
      
#endif /* TME_LSI64854_STRICT_PACK */

      /* create the mapping TLB entry: */
      tlb_mapping.tme_bus_tlb_addr_first = master_address;
      tlb_mapping.tme_bus_tlb_addr_last = dma_count - 1;
      tlb_mapping.tme_bus_tlb_cycles_ok = cycles;
      
      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, dma_address, &tlb_mapping, master_address);

      /* add our bus fault handler: */
      TME_BUS_TLB_FAULT_HANDLER(tlb, _tme_lsi64854_bus_fault_handler, lsi64854);
    }
  }

  return (rc);
}

/* the lsi64854 TLB adder for the master DMA: */
static int
_tme_lsi64854_tlb_set_add(struct tme_bus_connection *conn_bus,
			  struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_lsi64854 *lsi64854;

  /* recover our data structures: */
  lsi64854 = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* pass the lsi64854's request through: */
  conn_bus = lsi64854->tme_lsi64854_conn_regs;
  return (conn_bus != NULL
	  ? (*conn_bus->tme_bus_tlb_set_add)(conn_bus, 
					     tlb_set_info)
	  : ENXIO);
}

/* this scores a new connection: */
static int
_tme_lsi64854_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_lsi64854 *lsi64854;
  struct tme_lsi64854_connection *conn_lsi64854;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  lsi64854 = conn->tme_connection_element->tme_element_private;
  conn_lsi64854 = (struct tme_lsi64854_connection *)conn;

  /* this is a generic bus connection, so just score it nonzero and
     return.  note that there's no good way to differentiate a
     connection to a bus from a connection to just another chip, so we
     always return a nonzero score here: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_lsi64854_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_lsi64854 *lsi64854;
  struct tme_lsi64854_connection *conn_lsi64854;
  struct tme_bus_connection *conn_bus;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  lsi64854 = conn->tme_connection_element->tme_element_private;
  conn_lsi64854 = (struct tme_lsi64854_connection *) conn;
  conn_bus = &conn_lsi64854->tme_lsi64854_connection;

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&lsi64854->tme_lsi64854_mutex);

    /* save our connection: */
    switch (conn_lsi64854->tme_lsi64854_connection_which) {
    default: assert(FALSE);
    case TME_LSI64854_CONN_REGS:
      lsi64854->tme_lsi64854_conn_regs = (struct tme_bus_connection *) conn->tme_connection_other;
      break;
    case TME_LSI64854_CONN_REGS_MASTER:
      lsi64854->tme_lsi64854_conn_regs_master = (struct tme_bus_connection *) conn->tme_connection_other;
      break;
    case TME_LSI64854_CONN_MASTER:
      lsi64854->tme_lsi64854_conn_master = (struct tme_bus_connection *) conn->tme_connection_other;
      break;
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&lsi64854->tme_lsi64854_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_lsi64854_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a lsi64854: */
static int
_tme_lsi64854_connections_new(struct tme_element *element,
			    const char * const *args,
			    struct tme_connection **_conns,
			    char **_output)
{
  struct tme_lsi64854 *lsi64854;
  struct tme_lsi64854_connection *conn_lsi64854;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  unsigned int conn_which;
  int usage;
  int rc;

  /* recover our data structure: */
  lsi64854 = (struct tme_lsi64854 *) element->tme_element_private;
  
  /* we don't bother locking the mutex simply to check if connections
     already exist: */

  /* check our arguments: */
  usage = FALSE;
  rc = 0;
  conn_which = TME_LSI64854_CONN_NULL;

  /* if this connection is for the channel's registers: */
  if (TME_LSI64854_CHANNEL_HAS_MASTER(lsi64854)
	   ? TME_ARG_IS(args[1], "dma")
	   : args[1] == NULL) {

    /* if we already have a connection for the channel's registers,
       complain: */
    if (lsi64854->tme_lsi64854_conn_regs != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new connection: */
    else {
      conn_which = TME_LSI64854_CONN_REGS;
    }
  }

  /* otherwise, if this channel has a master, and this connection is
     for the master's registers: */
  else if (TME_LSI64854_CHANNEL_HAS_MASTER(lsi64854)
	   && args[1] == NULL) {

    /* if we already have a connection for the master's registers, complain: */
    if (lsi64854->tme_lsi64854_conn_regs_master != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new connection: */
    else {
      conn_which = TME_LSI64854_CONN_REGS_MASTER;
    }
  }

  /* otherwise, if this channel has a master, and this connection is
     for the master: */
  else if (TME_LSI64854_CHANNEL_HAS_MASTER(lsi64854)
	   && TME_ARG_IS(args[1], "master")) {
    
    /* if we already have an connection for the master, complain: */
    if (lsi64854->tme_lsi64854_conn_master != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new connection: */
    else {
      conn_which = TME_LSI64854_CONN_MASTER;
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
			    (TME_LSI64854_CHANNEL_HAS_MASTER(lsi64854)
			     ? "%s %s [ dma | master ]"
			     : "%s %s"),
			    _("usage:"),
			    args[0]);
    rc = EINVAL;
  }
  
  if (rc) {
    return (rc);
  }

  /* make a new connection: */
  conn_lsi64854 = tme_new0(struct tme_lsi64854_connection, 1);
  conn_bus = &conn_lsi64854->tme_lsi64854_connection;
  conn = &conn_bus->tme_bus_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_lsi64854_connection_score;
  conn->tme_connection_make = _tme_lsi64854_connection_make;
  conn->tme_connection_break = _tme_lsi64854_connection_break;

  /* fill in the generic bus connection: */
  conn_bus->tme_bus_subregions.tme_bus_subregion_address_first = 0;
  conn_bus->tme_bus_subregions.tme_bus_subregion_next = NULL;
  switch (conn_which) {
  default: assert(FALSE);
  case TME_LSI64854_CONN_MASTER:
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = ((tme_bus_addr_t) 0) - 1;
    conn_bus->tme_bus_signals_add = NULL;
    conn_bus->tme_bus_signal = _tme_lsi64854_bus_signal;
    conn_bus->tme_bus_tlb_set_add = _tme_lsi64854_tlb_set_add;
    conn_bus->tme_bus_tlb_fill = _tme_lsi64854_tlb_fill;
    break;
  case TME_LSI64854_CONN_REGS_MASTER:
    conn_bus->tme_bus_tlb_fill = _tme_lsi64854_tlb_fill_regs_master;
    switch (lsi64854->tme_lsi64854_channel) {
    default: assert(FALSE);
    case TME_LSI64854_CHANNEL_SCSI:
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_LSI64854_SIZ_NCR53C9X - 1;
      break;
    case TME_LSI64854_CHANNEL_ENET:
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_LSI64854_SIZ_AM7990 - 1;
      break;
    }
    break;
  case TME_LSI64854_CONN_REGS:
    conn_bus->tme_bus_tlb_fill = _tme_lsi64854_tlb_fill_regs;
    switch (lsi64854->tme_lsi64854_channel) {
    default: assert(FALSE);
    case TME_LSI64854_CHANNEL_SCSI:
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_LSI64854_SIZ_SCSI - 1;
      break;
    case TME_LSI64854_CHANNEL_ENET:
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_LSI64854_SIZ_ENET - 1;
      break;
    case TME_LSI64854_CHANNEL_PPRT:
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_LSI64854_SIZ_PPRT - 1;
      break;
    }
    break;
  }

  /* fill in the internal information: */
  conn_lsi64854->tme_lsi64854_connection_which = conn_which;

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* this creates one independent channel of an LSI 64854: */
TME_ELEMENT_NEW_DECL(tme_ic_lsi64854) {
  struct tme_lsi64854 *lsi64854;
  int arg_i;
  int usage;
  unsigned int channel;
  tme_uint32_t rev;

  /* check our arguments: */
  usage = 0;
  channel = TME_LSI64854_CHANNEL_NULL;
  rev = ~TME_LSI64854_CSR_X_REV_MASK;
  arg_i = 1;
  for (;;) {

    /* the channel: */
    if (TME_ARG_IS(args[arg_i + 0], "channel")) {
      if (TME_ARG_IS(args[arg_i + 1], "scsi")) {
	channel = TME_LSI64854_CHANNEL_SCSI;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "ethernet")) {
	channel = TME_LSI64854_CHANNEL_ENET;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "parallel")) {
	channel = TME_LSI64854_CHANNEL_PPRT;
      }
      else {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* the revision: */
    else if (TME_ARG_IS(args[arg_i + 0], "revision")) {
      if (TME_ARG_IS(args[arg_i + 1], "1+")) {
	rev = TME_LSI64854_CSR_X_REV_1PLUS;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "2")) {
	rev = TME_LSI64854_CSR_X_REV_2;
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

  if (channel == TME_LSI64854_CHANNEL_NULL) {
    usage = TRUE;
  }
  if (rev == ~TME_LSI64854_CSR_X_REV_MASK) {
    usage = TRUE;
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s channel { scsi | ethernet | parallel } revision { 1+ | 2 }",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the lsi64854 structure: */
  lsi64854 = tme_new0(struct tme_lsi64854, 1);
  lsi64854->tme_lsi64854_channel = channel;
  lsi64854->tme_lsi64854_csr = rev;
  lsi64854->tme_lsi64854_element = element;
  tme_mutex_init(&lsi64854->tme_lsi64854_mutex);

  /* fill the element: */
  element->tme_element_private = lsi64854;
  element->tme_element_connections_new = _tme_lsi64854_connections_new;

  /* reset the lsi64854: */
  _tme_lsi64854_reset(lsi64854);

  return (TME_OK);
}
