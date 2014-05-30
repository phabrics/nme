/* $Id: sun-sc.c,v 1.8 2010/06/05 14:12:53 fredette Exp $ */

/* bus/multibus/sun-sc.c - implementation of Sun `sc' SCSI Multibus board emulation: */

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
_TME_RCSID("$Id: sun-sc.c,v 1.8 2010/06/05 14:12:53 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/generic/scsi.h>

/* macros: */

/* register offsets and sizes: */
#define TME_SUN_SC_REG_DATA	(0)
#define TME_SUN_SC_SIZ_DATA	(sizeof(tme_uint8_t))
#define TME_SUN_SC_REG_CMD_STAT	(2)
#define TME_SUN_SC_SIZ_CMD_STAT	(sizeof(tme_uint8_t))
#define TME_SUN_SC_REG_ICR	(4)
#define TME_SUN_SC_SIZ_ICR	(sizeof(tme_uint16_t))
#define TME_SUN_SC_REG_DMA_H	(8)
#define TME_SUN_SC_SIZ_DMA_H	(sizeof(tme_uint16_t))
#define TME_SUN_SC_REG_DMA_L	(10)
#define TME_SUN_SC_SIZ_DMA_L	(sizeof(tme_uint16_t))
#define TME_SUN_SC_REG_DMA_LEN	(12)
#define TME_SUN_SC_SIZ_DMA_LEN	(sizeof(tme_uint16_t))
#define TME_SUN_SC_REG_INTVEC	(15)
#define TME_SUN_SC_SIZ_INTVEC	(sizeof(tme_uint8_t))
#define TME_SUN_SC_SIZ_CARD	(TME_SUN_SC_REG_INTVEC + TME_SUN_SC_SIZ_INTVEC)

/* the bits in the Interface Control Register.  bits greater than
   or equal to SUNSCPAL_ICR_BUSY are read-only: */
#define _TME_SUN_SC_ICR_RO_BITS		(~(TME_SUN_SC_ICR_BUSY - 1))
#define TME_SUN_SC_ICR_PARITY_ERROR	(0x8000)  /* parity error */
#define TME_SUN_SC_ICR_BUS_ERROR	(0x4000)  /* bus error */
#define TME_SUN_SC_ICR_ODD_LENGTH	(0x2000)  /* odd length */
#define TME_SUN_SC_ICR_INT_REQUEST	(0x1000)  /* interrupt request */
#define TME_SUN_SC_ICR_REQUEST		(0x0800)  /* request */
#define TME_SUN_SC_ICR_MESSAGE		(0x0400)  /* message */
#define TME_SUN_SC_ICR_COMMAND_DATA	(0x0200)  /* 1=command, 0=data */
#define TME_SUN_SC_ICR_INPUT_OUTPUT	(0x0100)  /* 1=input (initiator should read), 0=output */
#define TME_SUN_SC_ICR_PARITY		(0x0080)  /* parity */
#define TME_SUN_SC_ICR_BUSY		(0x0040)  /* busy */
#define TME_SUN_SC_ICR_SELECT		(0x0020)  /* select */
#define TME_SUN_SC_ICR_RESET		(0x0010)  /* reset */
#define TME_SUN_SC_ICR_PARITY_ENABLE	(0x0008)  /* enable parity */
#define TME_SUN_SC_ICR_WORD_MODE	(0x0004)  /* word mode */
#define TME_SUN_SC_ICR_DMA_ENABLE	(0x0002)  /* enable DMA */
#define TME_SUN_SC_ICR_INT_ENABLE	(0x0001)  /* enable interrupts */

/* this gets the current SCSI information transfer bus phase,
   including BSY, from an ICR value: */
#define TME_SUN_SC_BUS_PHASE(icr)	\
  ((icr)				\
   & (TME_SUN_SC_ICR_BUSY		\
      | TME_SUN_SC_ICR_MESSAGE		\
      | TME_SUN_SC_ICR_COMMAND_DATA	\
      | TME_SUN_SC_ICR_INPUT_OUTPUT))

/* these get and put a 16-bit register: */
#define TME_SUN_SC_REG16_GET(sun_sc, reg)	\
  tme_betoh_u16(*((tme_uint16_t *) &(sun_sc)->tme_sun_sc_card[reg]))
#define TME_SUN_SC_REG16_PUT(sun_sc, reg, val)	\
  (*((tme_uint16_t *) &(sun_sc)->tme_sun_sc_card[reg]) = tme_htobe_u16(val))

/* these get and put the ICR: */
#define TME_SUN_SC_ICR_GET(sun_sc)	\
  TME_SUN_SC_REG16_GET(sun_sc, TME_SUN_SC_REG_ICR)
#define TME_SUN_SC_ICR_PUT(sun_sc, icr)	\
  TME_SUN_SC_REG16_PUT(sun_sc, TME_SUN_SC_REG_ICR, icr)

/* the size of the cycle callout ring buffer: */
#define TME_SUN_SC_CYCLE_RING_SIZE	(4)

/* the callout flags: */
#define TME_SUN_SC_CALLOUT_CHECK		(0)
#define TME_SUN_SC_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SUN_SC_CALLOUTS_MASK		(-2)
#define  TME_SUN_SC_CALLOUT_CYCLE		TME_BIT(1)
#define  TME_SUN_SC_CALLOUT_TLB_FILL		TME_BIT(2)
#define	 TME_SUN_SC_CALLOUT_INT			TME_BIT(3)

#if 1
#define TME_SUN_SC_DEBUG
#endif

/* structures: */

/* an entry in the cycle callout ring buffer: */
struct tme_sun_sc_cycle {

  /* the SCSI control and data signals to assert: */
  tme_scsi_control_t tme_sun_sc_cycle_control;
  tme_scsi_data_t tme_sun_sc_cycle_data;

  /* the SCSI events to wait on, and the actions to take: */
  tme_uint32_t tme_sun_sc_cycle_events;
  tme_uint32_t tme_sun_sc_cycle_actions;

  /* a DMA structure needed: */
  struct tme_scsi_dma tme_sun_sc_cycle_dma;

  /* for the cmd_stat DMA, the cmd_stat value: */
  tme_uint8_t tme_sun_sc_cycle_cmd_stat;
};

/* the card: */
struct tme_sun_sc {

  /* our simple bus device header: */
  struct tme_bus_device tme_sun_sc_device;
#define tme_sun_sc_element tme_sun_sc_device.tme_bus_device_element

  /* the mutex protecting the card: */
  tme_mutex_t tme_sun_sc_mutex;

  /* the rwlock protecting the card: */
  tme_rwlock_t tme_sun_sc_rwlock;

  /* the SCSI bus connection: */
  struct tme_scsi_connection *tme_sun_sc_scsi_connection;

  /* the callout flags: */
  int tme_sun_sc_callout_flags;

  /* if our interrupt line is currently asserted: */
  int tme_sun_sc_int_asserted;

  /* it's easiest to just model the card as a chunk of memory: */
  tme_uint8_t tme_sun_sc_card[TME_SUN_SC_SIZ_CARD];

  /* the cycle ring buffer: */
  struct tme_sun_sc_cycle tme_sun_sc_cycles[TME_SUN_SC_CYCLE_RING_SIZE];
  int tme_sun_sc_cycle_head;
  int tme_sun_sc_cycle_tail;

  /* our DMA TLB set: */
  struct tme_bus_tlb tme_sun_sc_dma_tlb;
  int tme_sun_sc_dma_tlb_added;

  /* the internal DMA buffer: */
  tme_uint8_t tme_sun_sc_int_dma[2];

#ifndef TME_NO_LOG
  tme_uint16_t tme_sun_sc_last_log_icr;
#endif /* !TME_NO_LOG */
};

/* globals: */

/* the Sun sc bus router: */
static const tme_bus_lane_t tme_sun_sc_router[TME_BUS_ROUTER_SIZE(TME_BUS16_LOG2) * 2] = {
  
  /* [sc]   initiator cycle size: 8 bits
     [gen]  responding port size: 8 bits
     [gen]  responding port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
  /* D15-D8 */	TME_BUS_LANE_UNDEF,

  /* [sc]   initiator cycle size: 8 bits
     [gen]  responding port size: 8 bits
     [gen]  responding port least lane: 1: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
  /* D15-D8 */	TME_BUS_LANE_UNDEF | TME_BUS_LANE_WARN,

  /* [sc]   initiator cycle size: 8 bits
     [gen]  responding port size: 16 bits
     [gen]  responding port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
  /* D15-D8 */	TME_BUS_LANE_UNDEF | TME_BUS_LANE_WARN,

  /* [sc]   initiator cycle size: 8 bits
     [gen]  responding port size: 16 bits
     [gen]  responding port least lane: 1 - invalid, array placeholder: */
  /* D7-D0 */	TME_BUS_LANE_ABORT,
  /* D15-D8 */	TME_BUS_LANE_ABORT,

  /* [sc]   initiator cycle size: 16 bits
     [gen]  responding port size: 8 bits
     [gen]  responding port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
  /* D15-D8 */	TME_BUS_LANE_ROUTE(1) | TME_BUS_LANE_WARN,

  /* [sc]   initiator cycle size: 16 bits
     [gen]  responding port size: 8 bits
     [gen]  responding port least lane: 1: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0) | TME_BUS_LANE_WARN,
  /* D15-D8 */	TME_BUS_LANE_ROUTE(1) | TME_BUS_LANE_WARN,

  /* [sc]   initiator cycle size: 16 bits
     [gen]  responding port size: 16 bits
     [gen]  responding port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
  /* D15-D8 */	TME_BUS_LANE_ROUTE(1),

  /* [sc]   initiator cycle size: 16 bits
     [gen]  responding port size: 16 bits
     [gen]  responding port least lane: 1 - invalid, array placeholder: */
  /* D7-D0 */	TME_BUS_LANE_ABORT,
  /* D15-D8 */	TME_BUS_LANE_ABORT,
};

#ifdef TME_SUN_SC_DEBUG
void
_tme_sun_sc_reg16_put(struct tme_sun_sc *sun_sc,
		      int reg,
		      tme_uint16_t val_new)
{
  const char *reg_name;
  tme_uint16_t val_old;

  val_old = TME_SUN_SC_REG16_GET(sun_sc, reg);
  if (val_old == val_new) {
    return;
  }
  TME_SUN_SC_REG16_PUT(sun_sc, reg, val_new);

  switch (reg) {
  case TME_SUN_SC_REG_ICR:
    reg_name = "icr";
    break;
  case TME_SUN_SC_REG_DMA_H:
  case TME_SUN_SC_REG_DMA_L:
    tme_log(&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	    100, TME_OK,
	    (&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	     "dvma now 0x%04x%04x (len 0x%04x)",
	     TME_SUN_SC_REG16_GET(sun_sc,
				  TME_SUN_SC_REG_DMA_H),
	     TME_SUN_SC_REG16_GET(sun_sc,
				  TME_SUN_SC_REG_DMA_L),
	     (TME_SUN_SC_REG16_GET(sun_sc,
				   TME_SUN_SC_REG_DMA_LEN)
	      ^ 0xffff)));
    return;
  case TME_SUN_SC_REG_DMA_LEN:
    reg_name = "len";
    val_new ^= 0xffff;
    break;
  default:
    reg_name = "???";
    break;
  }
  tme_log(&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	    100, TME_OK,
	    (&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	     "%s now 0x%04x",
	     reg_name,
	     val_new));
}
#undef TME_SUN_SC_REG16_PUT
#define TME_SUN_SC_REG16_PUT _tme_sun_sc_reg16_put
#endif /* TME_SUN_SC_DEBUG */

/* this allocates the next cycle in the ring buffer: */
struct tme_sun_sc_cycle *
_tme_sun_sc_cycle_new(struct tme_sun_sc *sun_sc,
		      tme_uint32_t events,
		      tme_uint32_t actions)
{
  int old_head;
  struct tme_sun_sc_cycle *sun_sc_cycle;
  const struct tme_sun_sc_cycle *sun_sc_cycle_old;

  /* abort if the ring buffer overflows: */
  old_head = sun_sc->tme_sun_sc_cycle_head;
  sun_sc->tme_sun_sc_cycle_head
    = ((old_head
	+ 1)
       & (TME_SUN_SC_CYCLE_RING_SIZE
	  - 1));
  if ((sun_sc->tme_sun_sc_cycle_head
       == sun_sc->tme_sun_sc_cycle_tail)
      && (sun_sc->tme_sun_sc_scsi_connection
	  != NULL)) {
    abort();
  }

  /* initialize and return the cycle.  we copy the previous cycle's
     control signals, (and data signals too, unless the caller wants
     the DMA sequence), so that callers only have to change the values
     that they know are changing: */
  sun_sc_cycle
    = &sun_sc->tme_sun_sc_cycles[old_head];
  memset(sun_sc_cycle, 0, sizeof(*sun_sc_cycle));
  sun_sc_cycle_old
    = &sun_sc->tme_sun_sc_cycles[((old_head
				   - 1)
				  & (TME_SUN_SC_CYCLE_RING_SIZE
				     - 1))];
  sun_sc_cycle->tme_sun_sc_cycle_control
    = sun_sc_cycle_old->tme_sun_sc_cycle_control;
  sun_sc_cycle->tme_sun_sc_cycle_data
    = ((actions
	== TME_SCSI_ACTION_DMA_INITIATOR)
       ? 0
       : sun_sc_cycle_old->tme_sun_sc_cycle_data);
  sun_sc_cycle->tme_sun_sc_cycle_events = events;
  sun_sc_cycle->tme_sun_sc_cycle_actions = actions;
  /* XXX parity? */
  sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_flags
    = TME_SCSI_DMA_8BIT;
  return (sun_sc_cycle);
}

/* this does a bus cycle to read or write into our internal DMA
   buffer: */
static int
_tme_sun_sc_bus_cycle_dma(struct tme_sun_sc *sun_sc,
			  struct tme_bus_tlb *tlb,
			  tme_uint8_t cycle_type,
			  tme_bus_addr32_t address,
			  int word_mode)
{
  struct tme_bus_cycle cycle_init;
  int rc;

  /* use our internal DMA buffer: */ 
  cycle_init.tme_bus_cycle_buffer
    = &sun_sc->tme_sun_sc_int_dma[0];
  
  /* if we're in word mode, use the 16-bit bus router
     and a 16-bit cycle size: */
  if (word_mode) {
    cycle_init.tme_bus_cycle_lane_routing
      = &tme_sun_sc_router[TME_BUS_ROUTER_SIZE(TME_BUS16_LOG2)];
    cycle_init.tme_bus_cycle_size 
      = sizeof(tme_uint16_t);
  }
  
  /* otherwise, use the 8-bit bus router and an 8-bit
     cycle size: */
  else {
    cycle_init.tme_bus_cycle_lane_routing
      = &tme_sun_sc_router[0];
    cycle_init.tme_bus_cycle_size 
      = sizeof(tme_uint8_t);
  }
  
  assert (tlb->tme_bus_tlb_addr_shift == 0);
  cycle_init.tme_bus_cycle_address
    = (address
       + tlb->tme_bus_tlb_addr_offset);
  
  cycle_init.tme_bus_cycle_buffer_increment
    = 1;

  cycle_init.tme_bus_cycle_type
    = cycle_type;

  cycle_init.tme_bus_cycle_port = 
    TME_BUS_CYCLE_PORT(0, TME_BUS16_LOG2);
  
  /* unlock the mutex: */
  tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);

  /* run the bus cycle: */
  rc = ((*tlb->tme_bus_tlb_cycle)
	(tlb->tme_bus_tlb_cycle_private,
	 &cycle_init));
	
  /* lock the mutex: */
  tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);

  return (rc);
}  

/* the Sun sc callout function.  it must be called with the mutex locked: */
static void
_tme_sun_sc_callout(struct tme_sun_sc *sun_sc, int new_callouts)
{
  struct tme_scsi_connection *conn_scsi;
  struct tme_bus_connection *conn_bus;
  struct tme_bus_tlb *tlb;
  struct tme_bus_tlb tlb_local;
  int old_tail;
  struct tme_sun_sc_cycle *sun_sc_cycle;
  int callouts, later_callouts;
  tme_bus_addr32_t address;
  tme_uint16_t resid;
  tme_uint8_t cycle_type;
  tme_bus_addr32_t avail;
  tme_uint8_t *emulator_off;
  tme_uint16_t icr;
  int rc;
  int int_asserted;
  
  /* add in any new callouts: */
  sun_sc->tme_sun_sc_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (sun_sc->tme_sun_sc_callout_flags
      & TME_SUN_SC_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  sun_sc->tme_sun_sc_callout_flags
    |= TME_SUN_SC_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = sun_sc->tme_sun_sc_callout_flags)
	  & TME_SUN_SC_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    sun_sc->tme_sun_sc_callout_flags
      = (callouts
	 & ~TME_SUN_SC_CALLOUTS_MASK);
    callouts
      &= TME_SUN_SC_CALLOUTS_MASK;

    /* get this card's bus and SCSI connections: */
    conn_scsi = sun_sc->tme_sun_sc_scsi_connection;
    conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
					      sun_sc->tme_sun_sc_device.tme_bus_device_connection,
					      &sun_sc->tme_sun_sc_device.tme_bus_device_connection_rwlock);

    /* if we need to call out a SCSI bus cycle: */
    if (callouts & TME_SUN_SC_CALLOUT_CYCLE) {

      /* there must be a cycle to call out: */
      old_tail = sun_sc->tme_sun_sc_cycle_tail;
      assert (old_tail
	      != sun_sc->tme_sun_sc_cycle_head);
      sun_sc_cycle
	= &sun_sc->tme_sun_sc_cycles[old_tail];

      /* unlock the mutex: */
      tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);
      
      /* do the callout: */
      /* XXX FIXME THREADS - this is not thread-safe, since we're
	 passing values from (and pointers to) the non-local
	 sun_sc_cycle: */
      rc = (conn_scsi != NULL
	    ? ((*conn_scsi->tme_scsi_connection_cycle)
	       (conn_scsi,
		sun_sc_cycle->tme_sun_sc_cycle_control,
		sun_sc_cycle->tme_sun_sc_cycle_data,
		sun_sc_cycle->tme_sun_sc_cycle_events,
		sun_sc_cycle->tme_sun_sc_cycle_actions,
		((sun_sc_cycle->tme_sun_sc_cycle_actions
		  == TME_SCSI_ACTION_DMA_INITIATOR)
		 ? &sun_sc_cycle->tme_sun_sc_cycle_dma
		 : NULL)))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SUN_SC_CALLOUT_CYCLE;
      }

      /* otherwise, this callout was successful.  if this cycle did
	 not use the DMA initiator sequence, it either used no
	 sequence or it used the wait_change sequence: */
      else if (sun_sc_cycle->tme_sun_sc_cycle_actions
	       != TME_SCSI_ACTION_DMA_INITIATOR) {

	/* advance the tail pointer if it hasn't been advanced
	   already: */
	if (sun_sc->tme_sun_sc_cycle_tail
	    == old_tail) {
	  sun_sc->tme_sun_sc_cycle_tail
	    = ((sun_sc->tme_sun_sc_cycle_tail
		+ 1)
	       & (TME_SUN_SC_CYCLE_RING_SIZE
		  - 1));
	}

	/* if there are more cycles to run, run them now: */
	if (sun_sc->tme_sun_sc_cycle_tail
	    != sun_sc->tme_sun_sc_cycle_head) {
	  sun_sc->tme_sun_sc_callout_flags
	    |= TME_SUN_SC_CALLOUT_CYCLE;
	}
      }
    }

    /* if we need to call out a TLB fill: */
    if (callouts & TME_SUN_SC_CALLOUT_TLB_FILL) {

      /* get the current ICR value: */
      icr = TME_SUN_SC_ICR_GET(sun_sc);

      /* get the DMA address: */
      address
	= TME_SUN_SC_REG16_GET(sun_sc,
			       TME_SUN_SC_REG_DMA_H);
      address
	= ((address
	    << 16)
	   | TME_SUN_SC_REG16_GET(sun_sc,
				  TME_SUN_SC_REG_DMA_L));

      /* get the cycle type: */
      cycle_type
	= ((icr
	    & TME_SUN_SC_ICR_INPUT_OUTPUT)
	   ? TME_BUS_CYCLE_WRITE
	   : TME_BUS_CYCLE_READ);

      /* get the resid: */
      resid
	= (TME_SUN_SC_REG16_GET(sun_sc,
				TME_SUN_SC_REG_DMA_LEN)
	   ^ 0xffff);
      assert (resid
	      >= ((icr
		   & TME_SUN_SC_ICR_WORD_MODE)
		  ? sizeof(tme_uint16_t)
		  : sizeof(tme_uint8_t)));

      /* get the TLB entry: */
      tlb = &sun_sc->tme_sun_sc_dma_tlb;
      
      /* pass this TLB's token: */
      tlb_local.tme_bus_tlb_token = tlb->tme_bus_tlb_token;

      /* unlock the mutex: */
      tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);
      
      /* do the callout: */
      rc = (conn_bus != NULL
	    ? ((*conn_bus->tme_bus_tlb_fill)
	       (conn_bus,
		&tlb_local,
		address,
		cycle_type))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SUN_SC_CALLOUT_TLB_FILL;
      }

      /* otherwise, use the filled TLB entry to start a SCSI
	 DMA sequence: */
      else {

	/* store the TLB entry: */
	*tlb = tlb_local;

	/* get the number of bytes available in this TLB entry: */
	avail
	  = ((tlb->tme_bus_tlb_addr_last
	      - address)
	     + 1);
	if (avail == 0
	    || avail > resid) {
	  avail = resid;
	}
	assert (avail
		>= ((icr
		     & TME_SUN_SC_ICR_WORD_MODE)
		    ? sizeof(tme_uint16_t)
		    : sizeof(tme_uint8_t)));

	/* allocate the new SCSI bus cycle: */
	sun_sc_cycle
	  = _tme_sun_sc_cycle_new(sun_sc,
				  TME_SCSI_EVENT_NONE,
				  TME_SCSI_ACTION_DMA_INITIATOR);

	/* if this TLB entry allows fast transfers: */
	emulator_off
	  = ((cycle_type
	      == TME_BUS_CYCLE_READ)
	     /* XXX FIXME - this breaks volatile: */
	     ? (tme_uint8_t *) tlb->tme_bus_tlb_emulator_off_read
	     : (tme_uint8_t *) tlb->tme_bus_tlb_emulator_off_write);
	if (emulator_off
	    != TME_EMULATOR_OFF_UNDEF) {

	  /* do the SCSI DMA sequence with the TLB fast transfer
             buffer: */
	  sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in
	    = emulator_off + address;
	}

	/* otherwise, this TLB entry does not allow fast transfers: */
	else {

	  /* if this we need to read from this TLB, do a memory
	     read cycle into our internal DMA buffer: */
	  if (cycle_type == TME_BUS_CYCLE_READ) {
	    rc = _tme_sun_sc_bus_cycle_dma(sun_sc,
					   /* XXX FIXME this is not thread-safe: */
					   tlb,
					   TME_BUS_CYCLE_READ,
					   address,
					   (icr
					    & TME_SUN_SC_ICR_WORD_MODE));
	    assert (rc == TME_OK);
	  }

	  /* do the SCSI DMA sequence with our internal DMA buffer: */
	  sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in
	    = emulator_off + address;
	  avail
	    = ((icr
		& TME_SUN_SC_ICR_WORD_MODE)
	       ? sizeof(tme_uint16_t)
	       : sizeof(tme_uint8_t));
	}

	/* finish the SCSI cycle: */
	sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_out
	  = sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in;
	sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_resid
	  = avail;

	/* we now need to call out a SCSI cycle: */
	sun_sc->tme_sun_sc_callout_flags
	  |= TME_SUN_SC_CALLOUT_CYCLE;
      }
    }

    /* if we need to call out a possible change to our interrupt
       signal: */
    if (callouts & TME_SUN_SC_CALLOUT_INT) {

      /* get the current ICR value: */
      icr = TME_SUN_SC_ICR_GET(sun_sc);

      /* see if the interrupt signal should be asserted or negated: */
      int_asserted = ((icr
		       & TME_SUN_SC_ICR_INT_REQUEST)
		      && (icr
			  & TME_SUN_SC_ICR_INT_ENABLE));

      /* if the interrupt signal doesn't already have the right state: */
      if (!int_asserted
	  != !sun_sc->tme_sun_sc_int_asserted) {

	/* unlock our mutex: */
	tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);
	
	/* call out the bus interrupt signal edge: */
	rc = (*conn_bus->tme_bus_signal)
	  (conn_bus,
	   TME_BUS_SIGNAL_INT_UNSPEC
	   | (int_asserted
	      ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	      : TME_BUS_SIGNAL_LEVEL_NEGATED));

	/* lock our mutex: */
	tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);
	
	/* if this callout was successful, note the new state of the
	   interrupt signal: */
	if (rc == TME_OK) {
	  sun_sc->tme_sun_sc_int_asserted = int_asserted;
	}

	/* otherwise, remember that at some later time this callout
	   should be attempted again: */
	else {
	  later_callouts |= TME_SUN_SC_CALLOUT_INT;
	}
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  sun_sc->tme_sun_sc_callout_flags = later_callouts;
}

/* the interrupt acknowledge function for the VME version: */
static int
_tme_sun_sc_intack(void *_sun_sc, unsigned int signal, int *_vector)
{
  struct tme_sun_sc *sun_sc;

  /* recover our data structure: */
  sun_sc = (struct tme_sun_sc *) _sun_sc;

  /* return the interrupt vector: */
  *_vector = sun_sc->tme_sun_sc_card[TME_SUN_SC_REG_INTVEC];

  return (TME_OK);
}

/* the Sun sc bus cycle handler for the data and cmd_stat registers.
   these registers are connected directly to the SCSI data signals.

   a read of one of these registers samples the SCSI data bus, and a
   write to one of these registers changes the SCSI data bus signals
   asserted by the card.

   additionally, depending on context, a read or write of one of these
   registers will cause the card to execute the initiator's side of a
   REQ/ACK handshake to transfer a byte.

   a read or write of the cmd_stat register always causes the
   handshake.  a read or write of the data register when the SCSI bus
   is in the DATA IN or DATA OUT phase with REQ asserted also causes
   the handshake: */
static int
_tme_sun_sc_bus_cycle_data_reg(struct tme_sun_sc *sun_sc,
			       struct tme_bus_cycle *cycle_init,
			       int is_cmd_stat)
{
  tme_uint16_t icr;
  tme_scsi_data_t data_old, data_new;
  struct tme_sun_sc_cycle *sun_sc_cycle;
  int new_callouts;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);

  /* get the current ICR value: */
  icr = TME_SUN_SC_ICR_GET(sun_sc);

  /* save the old data register (really, the current
     signals asserted on the data bus), and, in case
     this is a read of the cmd_stat register, save
     it in that register: */
  data_old = sun_sc->tme_sun_sc_card[TME_SUN_SC_REG_DATA];
  sun_sc->tme_sun_sc_card[TME_SUN_SC_REG_CMD_STAT] = data_old;

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    sun_sc->tme_sun_sc_card,
			    sun_sc->tme_sun_sc_device.tme_bus_device_address_last);

  /* put back the old data register, but get any value that
     may have been written: */
  data_new
    = (is_cmd_stat
       ? sun_sc->tme_sun_sc_card[TME_SUN_SC_REG_CMD_STAT]
       : sun_sc->tme_sun_sc_card[TME_SUN_SC_REG_DATA]);
  sun_sc->tme_sun_sc_card[TME_SUN_SC_REG_DATA] = data_old;

  /* if this was a read or write of the cmd_stat register, or a read
     or write of the data register while the bus is in the DATA IN or
     DATA OUT phase with REQ asserted, do the initiator's side of the
     REQ/ACK handshake: */
  if (is_cmd_stat
      || ((icr
	   & (TME_SUN_SC_ICR_BUSY
	      | TME_SUN_SC_ICR_MESSAGE
	      | TME_SUN_SC_ICR_COMMAND_DATA
	      | TME_SUN_SC_ICR_REQUEST))
	  == (TME_SUN_SC_ICR_BUSY
	      | TME_SUN_SC_ICR_REQUEST))) {
    
    /* make a new SCSI bus cycle: */
    /* XXX parity? */
    sun_sc_cycle
      = _tme_sun_sc_cycle_new(sun_sc,
			      TME_SCSI_EVENT_NONE,
			      TME_SCSI_ACTION_DMA_INITIATOR);
    sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_resid
      = sizeof(sun_sc_cycle->tme_sun_sc_cycle_cmd_stat);
    sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in
      = &sun_sc_cycle->tme_sun_sc_cycle_cmd_stat;
    sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_out
      = &sun_sc_cycle->tme_sun_sc_cycle_cmd_stat;
    sun_sc_cycle->tme_sun_sc_cycle_cmd_stat
      = data_new;

    tme_log(&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	    100, TME_OK,
	    (&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	     ((cycle_init->tme_bus_cycle_type
	       == TME_BUS_CYCLE_WRITE)
	      ? (is_cmd_stat
		 ? "0x%02x -> cmd_stat"
		 : "0x%02x -> data (handshake)")
	      : (is_cmd_stat
		 ? "cmd_stat-> 0x%02x"
		 : "data -> 0x%02x (handshake)")),
	     sun_sc_cycle->tme_sun_sc_cycle_cmd_stat));
    
    new_callouts |= TME_SUN_SC_CALLOUT_CYCLE;

    /* since this SCSI DMA sequence won't be run right away, it's
       important that we clear REQ now - otherwise if the sc device
       driver happens to see REQ still set, it will think that the SCSI
       handshake *did* happen, and that this REQ is now requesting the
       next byte.  when the DMA sequence ends we'll get a cycle call
       that will bring us up-to-date: */
    TME_SUN_SC_ICR_PUT(sun_sc,
		       (icr
			& ~TME_SUN_SC_ICR_REQUEST));
  }

  /* otherwise, if this was a write of the data register in a 
     non-handshake bus phase: */
  else if (cycle_init->tme_bus_cycle_type
	   == TME_BUS_CYCLE_WRITE) {

    /* if the data signals that we are asserting have changed, call
       out a cycle, then wait: */
    sun_sc_cycle
      = &sun_sc->tme_sun_sc_cycles[((sun_sc->tme_sun_sc_cycle_head
				     - 1)
				    & (TME_SUN_SC_CYCLE_RING_SIZE
				       - 1))];
    if (data_new != sun_sc_cycle->tme_sun_sc_cycle_data) {
      tme_log(&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	      100, TME_OK,
	      (&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	       "0x%02x -> data (no handshake)",
	       data_new));
      sun_sc_cycle
	= _tme_sun_sc_cycle_new(sun_sc,
				TME_SCSI_EVENT_BUS_CHANGE,
				TME_SCSI_ACTION_NONE);
      sun_sc_cycle->tme_sun_sc_cycle_data
	= data_new;
      new_callouts
	|= TME_SUN_SC_CALLOUT_CYCLE;
    }
  }

  /* otherwise, this was a read of the data register in a
     non-handshake bus phase: */
  else {
    tme_log(&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	    100, TME_OK,
	    (&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	     "data -> 0x%02x (no handshake)",
	     data_old));
  }
    
  /* make any new callouts: */
  _tme_sun_sc_callout(sun_sc, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the Sun sc bus cycle handler for the data register.  this register
   is connected directly to the SCSI data signals: */
static int
_tme_sun_sc_bus_cycle_data(void *_sun_sc,
			   struct tme_bus_cycle *cycle_init)
{
  return (_tme_sun_sc_bus_cycle_data_reg((struct tme_sun_sc *) _sun_sc,
					 cycle_init,
					 FALSE));
}

/* the Sun sc bus cycle handler for the command/status register.  this
   register is also connected directly to the SCSI data signals, but
   reading or writing it additionally causes the card to execute the
   initiator's side of a REQ/ACK handshake to transfer a byte: */
static int
_tme_sun_sc_bus_cycle_cmd_stat(void *_sun_sc,
			       struct tme_bus_cycle *cycle_init)
{
  return (_tme_sun_sc_bus_cycle_data_reg((struct tme_sun_sc *) _sun_sc,
					 cycle_init,
					 TRUE));
}

/* this tries to start or continue a DMA transfer: */
static int
_tme_sun_sc_dma_start(struct tme_sun_sc *sun_sc,
		      tme_uint16_t *_icr)
{
  tme_uint16_t icr;
  tme_uint16_t resid;

  /* get the current ICR value: */
  icr = *_icr;

  /* get the resid: */
  resid
    = (TME_SUN_SC_REG16_GET(sun_sc,
			    TME_SUN_SC_REG_DMA_LEN)
       ^ 0xffff);
  
  tme_log(&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	  100, TME_OK,
	  (&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	   "dma_start: icr=0x%04x dvma=0x%04x%04x len=0x%04x",
	   icr,
	   TME_SUN_SC_REG16_GET(sun_sc,
				TME_SUN_SC_REG_DMA_H),
	   TME_SUN_SC_REG16_GET(sun_sc,
				TME_SUN_SC_REG_DMA_L),
	   resid));

  /* if we're not in the DATA IN or DATA OUT phases, do nothing.  the
     sun2 PROMs seem to set DMA_ENABLE when DMA is impossible: */
  if ((icr
       & (TME_SUN_SC_ICR_DMA_ENABLE
	  | TME_SUN_SC_ICR_BUSY
	  | TME_SUN_SC_ICR_MESSAGE
	  | TME_SUN_SC_ICR_COMMAND_DATA
	  | TME_SUN_SC_ICR_REQUEST))
      != (TME_SUN_SC_ICR_DMA_ENABLE
	  | TME_SUN_SC_ICR_BUSY
	  | TME_SUN_SC_ICR_REQUEST)) {
    return (TME_SUN_SC_CALLOUT_CHECK);
  }
  
  /* if there are no more bytes left to transfer, we need
     an interrupt: */
  if (resid == 0) {
    *_icr
      = ((icr
	  & ~TME_SUN_SC_ICR_ODD_LENGTH) 
	 | TME_SUN_SC_ICR_INT_REQUEST);
    return (TME_SUN_SC_CALLOUT_INT);
  }

  /* if there is only one byte left to transfer, and we're in word
     mode, note the odd byte, and we need an interrupt: */
  else if (resid == 1
	   && (icr
	       & TME_SUN_SC_ICR_WORD_MODE)) {
    *_icr
      = (icr
	 | TME_SUN_SC_ICR_ODD_LENGTH
	 | TME_SUN_SC_ICR_INT_REQUEST);
    return (TME_SUN_SC_CALLOUT_INT);
  }

  /* otherwise, start the DMA transfer with a TLB fill callout: */
  return (TME_SUN_SC_CALLOUT_TLB_FILL);
}

/* the Sun sc bus cycle handler for the ICR.  parts of this register
   are connected directly to the SCSI control signals: */
static int
_tme_sun_sc_bus_cycle_icr(void *_sun_sc,
			  struct tme_bus_cycle *cycle_init)
{
  struct tme_sun_sc *sun_sc;
  struct tme_sun_sc_cycle *sun_sc_cycle;
  tme_uint16_t icr_old, icr_new, icr_diff;
  int new_callouts;

  /* recover our data structure: */
  sun_sc = (struct tme_sun_sc *) _sun_sc;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);

  /* save the old ICR value: */
  icr_old = TME_SUN_SC_ICR_GET(sun_sc);
  icr_new = icr_old;

  /* if we were requesting an interrupt, clear it now and call out an
     interrupt change: */
  if (icr_new & TME_SUN_SC_ICR_INT_REQUEST) {
    icr_new &= ~TME_SUN_SC_ICR_INT_REQUEST;
    new_callouts |= TME_SUN_SC_CALLOUT_INT;
  }

  /* run the bus cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    sun_sc->tme_sun_sc_card,
			    sun_sc->tme_sun_sc_device.tme_bus_device_address_last);


  /* if this was a write: */
  if (cycle_init->tme_bus_cycle_type
      == TME_BUS_CYCLE_WRITE) {

    /* put back the read-only bits: */
    icr_new = TME_SUN_SC_ICR_GET(sun_sc);
    icr_new = ((icr_old
		& _TME_SUN_SC_ICR_RO_BITS)
	       | (icr_new
		  & ~_TME_SUN_SC_ICR_RO_BITS));
    
    /* get the set of changed bits: */
    icr_diff = icr_old ^ icr_new;
    
    /* a change in RESET: */
    if (icr_diff & TME_SUN_SC_ICR_RESET) {
      
      /* make a new cycle that asserts no data or control signals,
	 except possibly for RST: */
      sun_sc_cycle
	= _tme_sun_sc_cycle_new(sun_sc,
				TME_SCSI_EVENT_BUS_CHANGE,
				TME_SCSI_ACTION_NONE);
      sun_sc_cycle->tme_sun_sc_cycle_control
	= ((icr_new
	    & TME_SUN_SC_ICR_RESET)
	   ? TME_SCSI_SIGNAL_RST
	   : 0);
      sun_sc_cycle->tme_sun_sc_cycle_data
	= 0;
      new_callouts
	|= TME_SUN_SC_CALLOUT_CYCLE;
    }
    
    /* a change in SELECT: */
    else if (icr_diff & TME_SUN_SC_ICR_SELECT) {
      
      /* make a new cycle that sets the new state of SEL: */
      sun_sc_cycle
	= _tme_sun_sc_cycle_new(sun_sc,
				TME_SCSI_EVENT_BUS_CHANGE,
				TME_SCSI_ACTION_NONE);
      sun_sc_cycle->tme_sun_sc_cycle_control
	= ((sun_sc_cycle->tme_sun_sc_cycle_control
	    & ~TME_SCSI_SIGNAL_SEL)
	   | ((icr_new
	       & TME_SUN_SC_ICR_SELECT)
	      ? TME_SCSI_SIGNAL_SEL
	      : 0));
      new_callouts
	|= TME_SUN_SC_CALLOUT_CYCLE;
    }
    
    /* if DMA_ENABLE is now set: */
    if (icr_diff
	& icr_new
	& TME_SUN_SC_ICR_DMA_ENABLE) {
      
      /* try to start DMA: */
      new_callouts
	|= _tme_sun_sc_dma_start(sun_sc,
				 &icr_new);
    }
    
    /* a change in INT_ENABLE: */
    if (icr_diff
	& icr_new
	& TME_SUN_SC_ICR_INT_ENABLE) {
      
      /* our interrupt signal may be changing: */
      new_callouts |= TME_SUN_SC_CALLOUT_INT;
    }
  }

  /* if the ICR changed, save and log the new value: */
  if (icr_new != icr_old) {
    TME_SUN_SC_ICR_PUT(sun_sc, icr_new);
  }

  /* make any new callouts: */
  _tme_sun_sc_callout(sun_sc, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the Sun sc bus cycle handler for other card registers: */
static int
_tme_sun_sc_bus_cycle_other(void *_sun_sc,
			    struct tme_bus_cycle *cycle_init)
{
  struct tme_sun_sc *sun_sc;

  /* recover our data structure: */
  sun_sc = (struct tme_sun_sc *) _sun_sc;

  /* lock the mutex: */
  tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);

  /* run the bus cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    sun_sc->tme_sun_sc_card,
			    sun_sc->tme_sun_sc_device.tme_bus_device_address_last);

  /* if this was a write, dump out the other registers: */
  if (cycle_init->tme_bus_cycle_type
      == TME_BUS_CYCLE_WRITE) {
    tme_log(&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	    100, TME_OK,
	    (&sun_sc->tme_sun_sc_element->tme_element_log_handle,
	     "dvma=0x%04x%04x len=0x%04x",
	     TME_SUN_SC_REG16_GET(sun_sc,
				  TME_SUN_SC_REG_DMA_H),
	     TME_SUN_SC_REG16_GET(sun_sc,
				  TME_SUN_SC_REG_DMA_L),
	     (TME_SUN_SC_REG16_GET(sun_sc,
				   TME_SUN_SC_REG_DMA_LEN)
	      ^ 0xffff)));
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the Sun sc TLB filler: */
static int
_tme_sun_sc_tlb_fill(void *_sun_sc, 
		     struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address_wider,
		     unsigned int cycles)
{
  struct tme_sun_sc *sun_sc;
  tme_bus_addr32_t address;

  /* recover our data structure: */
  sun_sc = (struct tme_sun_sc *) _sun_sc;

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* the address must be within range: */
  assert(address < TME_SUN_SC_SIZ_CARD);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

#define TME_SUN_SC_TLB_REG(reg, siz, handler, rd)\
do {						\
						\
  /* if this address falls in this register: */	\
  if (((reg)					\
       <= address)				\
      && (address				\
	  < ((reg)				\
	     + (siz)))) {			\
						\
    /* this TLB entry covers this register: */	\
    tlb->tme_bus_tlb_addr_first = (reg);	\
    tlb->tme_bus_tlb_addr_last = (reg) + (siz) - 1;\
						\
    /* our bus cycle handler: */		\
    tlb->tme_bus_tlb_cycle = (handler);		\
						\
    /* if this TLB entry allows fast reading: */\
    if (rd) {					\
      tlb->tme_bus_tlb_emulator_off_read	\
         = &sun_sc->tme_sun_sc_card[0];		\
    }						\
  }						\
} while (/* CONSTCOND */ 0)

  TME_SUN_SC_TLB_REG(TME_SUN_SC_REG_DATA,
		     TME_SUN_SC_SIZ_DATA,
		     _tme_sun_sc_bus_cycle_data,
		     FALSE);
  TME_SUN_SC_TLB_REG(TME_SUN_SC_REG_CMD_STAT,
		     TME_SUN_SC_SIZ_CMD_STAT,
		     _tme_sun_sc_bus_cycle_cmd_stat,
		     FALSE);
  TME_SUN_SC_TLB_REG(TME_SUN_SC_REG_ICR,
		     TME_SUN_SC_SIZ_ICR,
		     _tme_sun_sc_bus_cycle_icr,
		     TRUE);
#undef TME_SUN_SC_TLB_REG

  /* anything else is some other register: */
  if (tlb->tme_bus_tlb_cycle == NULL) {

    /* if this address is past the ICR, this TLB entry covers from
       past the ICR to the end of the card, else this TLB entry covers
       the byte at this address only: */
    if (address
	>= (TME_SUN_SC_REG_ICR
	    + TME_SUN_SC_SIZ_ICR)) {
      tlb->tme_bus_tlb_addr_first = TME_SUN_SC_REG_ICR + TME_SUN_SC_SIZ_ICR;
      tlb->tme_bus_tlb_addr_last = TME_SUN_SC_SIZ_CARD - 1;
    }
    else {
      tlb->tme_bus_tlb_addr_first = address;
      tlb->tme_bus_tlb_addr_last = address;
    }

    /* this TLB entry allows fast reading and writing: */
    tlb->tme_bus_tlb_emulator_off_read
      = &sun_sc->tme_sun_sc_card[0];
    tlb->tme_bus_tlb_emulator_off_write
      = &sun_sc->tme_sun_sc_card[0];

    /* bus cycles are handled by the other handler: */
    tlb->tme_bus_tlb_cycle = _tme_sun_sc_bus_cycle_other;
  }

#ifdef TME_SUN_SC_DEBUG
  /* XXX when debugging, nothing is fast readable or writable: */
  tlb->tme_bus_tlb_emulator_off_read
    = TME_EMULATOR_OFF_UNDEF;
  tlb->tme_bus_tlb_emulator_off_write
    = TME_EMULATOR_OFF_UNDEF;
#endif /* TME_SUN_SC_DEBUG */

  /* in case this TLB entry allows fast access: */
  tlb->tme_bus_tlb_rwlock = &sun_sc->tme_sun_sc_rwlock;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler private data: */
  tlb->tme_bus_tlb_cycle_private = sun_sc;

  return (TME_OK);
}

/* this is called for an event on the SCSI bus: */
static int
_tme_sun_sc_scsi_cycle(struct tme_scsi_connection *conn_scsi,
		       tme_scsi_control_t control,
		       tme_scsi_data_t data,
		       tme_uint32_t events_triggered,
		       tme_uint32_t actions_taken,
		       const struct tme_scsi_dma *dma)
{
  struct tme_sun_sc *sun_sc;
  struct tme_sun_sc_cycle *sun_sc_cycle;
  unsigned long count;
  struct tme_bus_tlb *tlb;
  tme_uint32_t address;
  tme_uint16_t resid;
  tme_uint16_t icr_old, icr_new;
  int new_callouts;
  int new_callouts_dma;
  int rc;
  
  /* recover our data structure: */
  sun_sc = conn_scsi->tme_scsi_connection.tme_connection_element->tme_element_private;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);

  /* get the old ICR value: */
  icr_old = TME_SUN_SC_ICR_GET(sun_sc);
  
  /* update the ICR to reflect the current SCSI control signals: */
  icr_new = icr_old;
#define _TME_SUN_SC_ICR_CONTROL(_icr, _control) \
do {						\
  if (control & _control) {			\
    icr_new |= _icr;				\
  }						\
  else {					\
    icr_new &= ~_icr;				\
  }						\
} while (/* CONSTCOND */ 0)
  _TME_SUN_SC_ICR_CONTROL(TME_SUN_SC_ICR_REQUEST,
			  TME_SCSI_SIGNAL_REQ);
  _TME_SUN_SC_ICR_CONTROL(TME_SUN_SC_ICR_MESSAGE,
			  TME_SCSI_SIGNAL_MSG);
  _TME_SUN_SC_ICR_CONTROL(TME_SUN_SC_ICR_COMMAND_DATA,
			  TME_SCSI_SIGNAL_C_D);
  _TME_SUN_SC_ICR_CONTROL(TME_SUN_SC_ICR_INPUT_OUTPUT,
			  TME_SCSI_SIGNAL_I_O);
  _TME_SUN_SC_ICR_CONTROL(TME_SUN_SC_ICR_PARITY,
			  TME_SCSI_SIGNAL_DBP);
  _TME_SUN_SC_ICR_CONTROL(TME_SUN_SC_ICR_BUSY,
			  TME_SCSI_SIGNAL_BSY);
#undef _TME_SUN_SC_ICR_CONTROL

  /* if the bus phase has changed to the STATUS phase, call out an
     interrupt: */
  /* XXX is this really how the board behaves? the sun2 PROM seems to
     rely on this behavior: */
  if ((TME_SUN_SC_BUS_PHASE(icr_new)
       != TME_SUN_SC_BUS_PHASE(icr_old))
      && (TME_SUN_SC_BUS_PHASE(icr_new)
	  == (TME_SUN_SC_ICR_BUSY
	      | TME_SUN_SC_ICR_COMMAND_DATA
	      | TME_SUN_SC_ICR_INPUT_OUTPUT))) {
    icr_new
      |= TME_SUN_SC_ICR_INT_REQUEST;
    new_callouts
      |= TME_SUN_SC_CALLOUT_INT;
  }

  /* get the last SCSI cycle that we called out: */
  sun_sc_cycle
    = &sun_sc->tme_sun_sc_cycles[sun_sc->tme_sun_sc_cycle_tail];

  /* if the last SCSI cycle was for a DMA sequence: */
  if (sun_sc_cycle->tme_sun_sc_cycle_actions
      == TME_SCSI_ACTION_DMA_INITIATOR) {

    /* copy in the finished DMA structure: */
    sun_sc_cycle->tme_sun_sc_cycle_dma = *dma;

    /* if the last SCSI cycle was for a genuine DMA sequence (as opposed
       to a DMA sequence to transfer the cmd_stat register), update the
       card's DMA engine registers: */
    if ((sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_out
	 != &sun_sc_cycle->tme_sun_sc_cycle_cmd_stat)
	&& (sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in
	    != &sun_sc_cycle->tme_sun_sc_cycle_cmd_stat)) {

      /* get the number of bytes that were transferred: */
      count
	= ((sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_out
	    > sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in)
	   ? (sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_out
	      - sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in)
	   : (sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_in
	      - sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_out));
      
      /* get the TLB entry: */
      tlb = &sun_sc->tme_sun_sc_dma_tlb;
      
      /* get the DMA address: */
      address
	= TME_SUN_SC_REG16_GET(sun_sc,
			       TME_SUN_SC_REG_DMA_H);
      address
	= ((address
	    << 16)
	   | TME_SUN_SC_REG16_GET(sun_sc,
				  TME_SUN_SC_REG_DMA_L));

      /* if we were doing a DMA write, but the TLB entry we filled
	 doesn't allow fast writing, we need to do a memory write
	 cycle to transfer the bytes from our internal DMA buffer into
	 memory: */
      if ((icr_old
	   & TME_SUN_SC_ICR_INPUT_OUTPUT)
	  && (tlb->tme_bus_tlb_emulator_off_write
	      == TME_EMULATOR_OFF_UNDEF)) {
	rc = _tme_sun_sc_bus_cycle_dma(sun_sc,
				       /* XXX FIXME this is not thread-safe: */
				       tlb,
				       TME_BUS_CYCLE_WRITE,
				       address,
				       (icr_old
					& TME_SUN_SC_ICR_WORD_MODE));
	assert (rc == TME_OK);
      }
      
      /* update the DMA address: */
      address += count;
      TME_SUN_SC_REG16_PUT(sun_sc,
			   TME_SUN_SC_REG_DMA_H,
			   address >> 16);
      TME_SUN_SC_REG16_PUT(sun_sc,
			   TME_SUN_SC_REG_DMA_L,
			   address & 0xffff);
      
      /* update the DMA length: */
      resid
	= (TME_SUN_SC_REG16_GET(sun_sc,
				TME_SUN_SC_REG_DMA_LEN)
	   ^ 0xffff);
      assert (resid >= count);
      TME_SUN_SC_REG16_PUT(sun_sc,
			   TME_SUN_SC_REG_DMA_LEN,
			   ((resid
			     - count)
			    ^ 0xffff));
      
      /* XXX this idea doesn't work because the initiator can't know
	 the amount of data returned by some commands, like REQUEST
	 SENSE.  when does this card signal BUS_ERROR, then? */
#if 0
      /* if the DMA sequence didn't transfer all of the DMA bytes that
	 we had made available, that usually means that the SCSI bus
	 phase changed in the middle of the transfer, which is a bus
	 error: */
      if (sun_sc_cycle->tme_sun_sc_cycle_dma.tme_scsi_dma_resid
	  > 0) {
	
	/* note the bus error and request an interrupt: */
	icr_new
	  |= (TME_SUN_SC_ICR_BUS_ERROR
	      | TME_SUN_SC_ICR_INT_REQUEST);
	new_callouts |= TME_SUN_SC_CALLOUT_INT;
      }
#endif
    }

    /* advance the tail pointer: */
    sun_sc->tme_sun_sc_cycle_tail
      = ((sun_sc->tme_sun_sc_cycle_tail
	  + 1)
	 & (TME_SUN_SC_CYCLE_RING_SIZE
	    - 1));
  }

  /* always try to start or continue a DMA transfer.  if one
     can't be started, just wait for another SCSI bus change: */
  new_callouts_dma
    = _tme_sun_sc_dma_start(sun_sc,
			    &icr_new);
  if (new_callouts_dma != 0) {
    new_callouts |= new_callouts_dma;
  }
  else {
    sun_sc_cycle
      = _tme_sun_sc_cycle_new(sun_sc,
			      TME_SCSI_EVENT_BUS_CHANGE,
			      TME_SCSI_ACTION_NONE);
    new_callouts
      |= TME_SUN_SC_CALLOUT_CYCLE;
  }
  
  /* update the ICR and data registers: */
  TME_SUN_SC_ICR_PUT(sun_sc, icr_new);
  sun_sc->tme_sun_sc_card[TME_SUN_SC_REG_DATA] = data;

  /* make any new callouts: */
  _tme_sun_sc_callout(sun_sc, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);

  return (TME_OK);
}

/* this makes a new bus connection: */
static int
_tme_sun_sc_connection_make_bus(struct tme_connection *conn,
				unsigned int state)
{
  struct tme_sun_sc *sun_sc;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* recover our data structure: */
  sun_sc = conn->tme_connection_element->tme_element_private;

  /* call the bus device connection maker: */
  rc = tme_bus_device_connection_make(conn, state);

  /* if the full connection was successful, and we don't have a TLB
     set yet, allocate it: */
  if (rc == TME_OK
      && state == TME_CONNECTION_FULL
      && !sun_sc->tme_sun_sc_dma_tlb_added) {

    /* get our bus connection: */
    conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
					      sun_sc->tme_sun_sc_device.tme_bus_device_connection,
					      &sun_sc->tme_sun_sc_device.tme_bus_device_connection_rwlock);

    /* allocate the TLB set: */
    rc = tme_bus_device_tlb_set_add(&sun_sc->tme_sun_sc_device,
				    1, 
				    &sun_sc->tme_sun_sc_dma_tlb);
    assert (rc == TME_OK);
    sun_sc->tme_sun_sc_dma_tlb_added = TRUE;
  }

  return (rc);
}

/* this makes a new SCSI connection: */
static int
_tme_sun_sc_connection_make_scsi(struct tme_connection *conn,
				 unsigned int state)
{
  struct tme_sun_sc *sun_sc;
  struct tme_sun_sc_cycle *sun_sc_cycle;
  struct tme_scsi_connection *conn_scsi;
  struct tme_scsi_connection *conn_scsi_other;

  /* recover our data structures: */
  sun_sc = conn->tme_connection_element->tme_element_private;
  conn_scsi = (struct tme_scsi_connection *) conn;
  conn_scsi_other = (struct tme_scsi_connection *) conn->tme_connection_other;

  /* both sides must be SCSI connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_SCSI);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SCSI);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&sun_sc->tme_sun_sc_mutex);

    /* save our connection: */
    sun_sc->tme_sun_sc_scsi_connection = conn_scsi_other;

    /* call out a cycle that asserts no signals and runs the
       wait-change sequence.  this also fully-initializes
       this cycle - _tme_sun_sc_cycle_new copies the previous
       cycle into a newly allocated cycle, so this hopefully
       starts the chain of well-initialized cycles: */       
    sun_sc_cycle
      = _tme_sun_sc_cycle_new(sun_sc,
			      TME_SCSI_EVENT_BUS_CHANGE,
			      TME_SCSI_ACTION_NONE);
    sun_sc_cycle->tme_sun_sc_cycle_control
      = 0;
    sun_sc_cycle->tme_sun_sc_cycle_data
      = 0;
    _tme_sun_sc_callout(sun_sc, TME_SUN_SC_CALLOUT_CYCLE);

    /* unlock our mutex: */
    tme_mutex_unlock(&sun_sc->tme_sun_sc_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_sun_sc_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a Sun sc: */
static int
_tme_sun_sc_connections_new(struct tme_element *element,
			    const char * const *args,
			    struct tme_connection **_conns,
			    char **_output)
{
  struct tme_sun_sc *sun_sc;
  struct tme_scsi_connection *conn_scsi;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  sun_sc = (struct tme_sun_sc *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* since we need to allocate a TLB set when we make our bus
     connection, make sure any generic bus device connection sides
     use our connection maker: */
  for (conn = *_conns;
       conn != NULL;
       conn = conn->tme_connection_next) {
    if ((conn->tme_connection_type
	 == TME_CONNECTION_BUS_GENERIC)
	&& (conn->tme_connection_make
	    == tme_bus_device_connection_make)) {
      conn->tme_connection_make
	= _tme_sun_sc_connection_make_bus;
    }
  }

  /* if we don't have a SCSI connection, make one: */
  if (sun_sc->tme_sun_sc_scsi_connection == NULL) {

    /* allocate the new SCSI connection: */
    conn_scsi = tme_new0(struct tme_scsi_connection, 1);
    conn = &conn_scsi->tme_scsi_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SCSI;
    conn->tme_connection_score = tme_scsi_connection_score;
    conn->tme_connection_make = _tme_sun_sc_connection_make_scsi;
    conn->tme_connection_break = _tme_sun_sc_connection_break;

    /* fill in the SCSI connection: */
    conn_scsi->tme_scsi_connection_cycle = _tme_sun_sc_scsi_cycle;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new Sun sc function: */
TME_ELEMENT_SUB_NEW_DECL(tme_bus_multibus,sun_sc) {
  struct tme_sun_sc *sun_sc;
  int arg_i;
  int usage;
  int vme;

  /* check our arguments: */
  usage = 0;
  arg_i = 1;
  vme = FALSE;
  for (;;) {

    if (TME_ARG_IS(args[arg_i], "vme")) {
      vme = TRUE;
      arg_i++;
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

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s [ vme ]",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the Sun sc structure: */
  sun_sc = tme_new0(struct tme_sun_sc, 1);
  sun_sc->tme_sun_sc_element = element;
  tme_mutex_init(&sun_sc->tme_sun_sc_mutex);
  tme_rwlock_init(&sun_sc->tme_sun_sc_rwlock);

  /* initialize our simple bus device descriptor: */
  sun_sc->tme_sun_sc_device.tme_bus_device_element = element;
  sun_sc->tme_sun_sc_device.tme_bus_device_tlb_fill = _tme_sun_sc_tlb_fill;
  sun_sc->tme_sun_sc_device.tme_bus_device_address_last = TME_SUN_SC_SIZ_CARD - 1;
  if (vme) {
    sun_sc->tme_sun_sc_device.tme_bus_device_intack = _tme_sun_sc_intack;
  }

  /* fill the element: */
  element->tme_element_private = sun_sc;
  element->tme_element_connections_new = _tme_sun_sc_connections_new;

  return (TME_OK);
}
