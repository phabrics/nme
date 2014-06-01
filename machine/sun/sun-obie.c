/* $Id: sun-obie.c,v 1.5 2010/06/05 19:20:30 fredette Exp $ */

/* machine/sun/sun-obie.c - classic Sun onboard Intel Ethernet implementation: */

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
_TME_RCSID("$Id: sun-obie.c,v 1.5 2010/06/05 19:20:30 fredette Exp $");

/* includes: */
#include <tme/element.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>
#undef TME_I825X6_VERSION
#define TME_I825X6_VERSION TME_X_VERSION(0, 0)
#include <tme/ic/i825x6.h>
#include <tme/machine/sun.h>

/* macros: */

/* register offsets and sizes: */
#define TME_SUN_OBIE_REG_CSR	(0)
#define TME_SUN_OBIE_SIZ_CSR	(sizeof(tme_uint16_t))
#define TME_SUN_OBIE_SIZ_REGS	(TME_SUN_OBIE_REG_CSR + TME_SUN_OBIE_SIZ_CSR)

/* the bits in the Control/Status Register: */
#define TME_SUN_OBIE_CSR_NORESET	(0x8000)
#define TME_SUN_OBIE_CSR_NOLOOP		(0x4000)
#define TME_SUN_OBIE_CSR_CA		(0x2000)
#define TME_SUN_OBIE_CSR_IE		(0x1000)
					/* 0x0800 unused */
#define TME_SUN_OBIE_CSR_LEVEL2		(0x0400)
#define TME_SUN_OBIE_CSR_BUSERR		(0x0200)
#define TME_SUN_OBIE_CSR_INTR		(0x0100)
#define TME_SUN_OBIE_CSR_READONLY	(0x0800				\
					 | TME_SUN_OBIE_CSR_LEVEL2	\
					 | TME_SUN_OBIE_CSR_BUSERR	\
					 | TME_SUN_OBIE_CSR_INTR)

/* these get and put the CSR: */
#define TME_SUN_OBIE_CSR_GET(sun_obie)	\
  ((((tme_uint16_t) (sun_obie)->tme_sun_obie_regs[TME_SUN_OBIE_REG_CSR + 0]) << 8)	\
   + (sun_obie)->tme_sun_obie_regs[TME_SUN_OBIE_REG_CSR + 1])
#define TME_SUN_OBIE_CSR_PUT(sun_obie, csr)	\
  do {											\
    (sun_obie)->tme_sun_obie_regs[TME_SUN_OBIE_REG_CSR + 0] = (csr) >> 8;		\
    (sun_obie)->tme_sun_obie_regs[TME_SUN_OBIE_REG_CSR + 1] = (tme_uint8_t) (csr);	\
  } while (/* CONSTCOND */ 0)

/* the callout flags: */
#define TME_SUN_OBIE_CALLOUT_CHECK	(0)
#define TME_SUN_OBIE_CALLOUT_RUNNING	TME_BIT(0)
#define TME_SUN_OBIE_CALLOUTS_MASK	(-2)
#define  TME_SUN_OBIE_CALLOUT_SIGNALS	TME_BIT(1)
#define	 TME_SUN_OBIE_CALLOUT_INT	TME_BIT(2)

/* structures: */

/* the card: */
struct tme_sun_obie {

  /* backpointer to our element: */
  struct tme_element *tme_sun_obie_element;

  /* the mutex protecting the card: */
  tme_mutex_t tme_sun_obie_mutex;

  /* the rwlock protecting the card: */
  tme_rwlock_t tme_sun_obie_rwlock;

  /* the bus connection for the card's registers: */
  struct tme_bus_connection *tme_sun_obie_conn_regs;

  /* the bus connection for the card's memory: */
  struct tme_bus_connection *tme_sun_obie_conn_memory;

  /* the bus connection for the card's i825x6 chip: */
  struct tme_bus_connection *tme_sun_obie_conn_i825x6;

  /* the callout flags: */
  int tme_sun_obie_callout_flags;

  /* if our interrupt line is currently asserted: */
  int tme_sun_obie_int_asserted;

  /* it's easiest to just model the board registers as a chunk of memory: */
  tme_uint8_t tme_sun_obie_regs[TME_SUN_OBIE_SIZ_REGS];

  /* the i825x6 image of the CSR: */
  tme_uint16_t tme_sun_obie_csr_i825x6;

#ifndef TME_NO_LOG
  tme_uint16_t tme_sun_obie_last_log_csr;
#endif /* !TME_NO_LOG */
};

/* a sun_obie internal bus connection: */
struct tme_sun_obie_connection {

  /* the external bus connection: */
  struct tme_bus_connection tme_sun_obie_connection;

  /* this is nonzero if a TME_CONNECTION_BUS_GENERIC chip connection
     is for the registers: */
  tme_uint8_t tme_sun_obie_connection_regs;
};

/* globals: */

/* our bus signals sets: */
static const struct tme_bus_signals _tme_sun_obie_bus_signals_generic = TME_BUS_SIGNALS_GENERIC;
static const struct tme_bus_signals _tme_sun_obie_bus_signals_i825x6 = TME_BUS_SIGNALS_I825X6;

/* the sun_obie callout function.  it must be called with the mutex locked: */
static void
_tme_sun_obie_callout(struct tme_sun_obie *sun_obie, int new_callouts)
{
  struct tme_bus_connection *conn_i825x6;
  struct tme_bus_connection *conn_bus;
  tme_uint16_t csr, csr_diff;
  unsigned int signal, level;
  int callouts, later_callouts;
  int rc;
  int int_asserted;
  
  /* add in any new callouts: */
  sun_obie->tme_sun_obie_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (sun_obie->tme_sun_obie_callout_flags & TME_SUN_OBIE_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  sun_obie->tme_sun_obie_callout_flags |= TME_SUN_OBIE_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; (callouts = sun_obie->tme_sun_obie_callout_flags) & TME_SUN_OBIE_CALLOUTS_MASK; ) {

    /* clear the needed callouts: */
    sun_obie->tme_sun_obie_callout_flags = callouts & ~TME_SUN_OBIE_CALLOUTS_MASK;
    callouts &= TME_SUN_OBIE_CALLOUTS_MASK;

    /* if we need to call out one or more signals to the i825x6: */
    if (callouts & TME_SUN_OBIE_CALLOUT_SIGNALS) {

      /* get the current CSR value: */
      csr = TME_SUN_OBIE_CSR_GET(sun_obie);

      /* get the next signal to call out to the i825x6: */
      csr_diff = ((csr
		   ^ sun_obie->tme_sun_obie_csr_i825x6)
		  & (TME_SUN_OBIE_CSR_NORESET
		     | TME_SUN_OBIE_CSR_NOLOOP
		     | TME_SUN_OBIE_CSR_CA));
      csr_diff = (csr_diff ^ (csr_diff - 1)) & csr_diff;
      
      /* if there is a signal to call out: */
      if (csr_diff != 0) {

	/* assume that if the signal's bit is set in the CSR, it will
	   be asserted: */
	level = csr & csr_diff;

	/* assume that we're calling out an i825x6 signal: */
	signal = (_tme_sun_obie_bus_signals_generic.tme_bus_signals_first
		  + TME_BUS_SIGNAL_X(_tme_sun_obie_bus_signals_generic.tme_bus_signals_count));

	/* dispatch on the CSR bit: */
	switch (csr_diff) {
	default:
	  assert (FALSE);
	case TME_SUN_OBIE_CSR_NORESET:
	  signal = TME_BUS_SIGNAL_RESET;
	  level = !level;
	  break;
	case TME_SUN_OBIE_CSR_NOLOOP:
	  signal += TME_I825X6_SIGNAL_LOOP;
	  level = !level;
	  break;
	case TME_SUN_OBIE_CSR_CA:
	  signal += TME_I825X6_SIGNAL_CA;
	  break;
	}

	/* create a real signal level value: */
	level = (level
		 ? TME_BUS_SIGNAL_LEVEL_ASSERTED
		 : TME_BUS_SIGNAL_LEVEL_NEGATED);

	/* get this card's i825x6 connection: */
	conn_i825x6 = sun_obie->tme_sun_obie_conn_i825x6;

	/* unlock the mutex: */
	tme_mutex_unlock(&sun_obie->tme_sun_obie_mutex);
      
	/* do the callout: */
	rc = (conn_i825x6 != NULL
	      ? ((*conn_i825x6->tme_bus_signal)
		 (conn_i825x6,
		  signal | level))
	      : TME_OK);
	
	/* lock the mutex: */
	tme_mutex_lock(&sun_obie->tme_sun_obie_mutex);
      
	/* if the callout was unsuccessful, remember that at some later
	   time this callout should be attempted again: */
	if (rc != TME_OK) {
	  later_callouts |= TME_SUN_OBIE_CALLOUT_SIGNALS;
	}

	/* otherwise, the callout was successful: */
	else {

	  /* update the i825x6 image of the CSR: */
	  sun_obie->tme_sun_obie_csr_i825x6 = 
	    ((sun_obie->tme_sun_obie_csr_i825x6 & ~csr_diff)
	     | (csr & csr_diff));

	  /* there may be more signals to call out, so attempt this
             callout again now: */
	  sun_obie->tme_sun_obie_callout_flags |= TME_SUN_OBIE_CALLOUT_SIGNALS;
	}
      }
    }

    /* if we need to call out a possible change to our interrupt
       signal: */
    if (callouts & TME_SUN_OBIE_CALLOUT_INT) {

      /* get the current CSR value: */
      csr = TME_SUN_OBIE_CSR_GET(sun_obie);

      /* see if the interrupt signal should be asserted or negated: */
      int_asserted = ((csr & (TME_SUN_OBIE_CSR_IE
			      | TME_SUN_OBIE_CSR_INTR))
		      == (TME_SUN_OBIE_CSR_IE
			  | TME_SUN_OBIE_CSR_INTR));

      /* if the interrupt signal doesn't already have the right state: */
      if (!int_asserted != !sun_obie->tme_sun_obie_int_asserted) {

	/* get our bus connection: */
	conn_bus = sun_obie->tme_sun_obie_conn_regs;
	
	/* unlock our mutex: */
	tme_mutex_unlock(&sun_obie->tme_sun_obie_mutex);
	
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
	tme_mutex_lock(&sun_obie->tme_sun_obie_mutex);
	
	/* if this callout was successful, note the new state of the
	   interrupt signal: */
	if (rc == TME_OK) {
	  sun_obie->tme_sun_obie_int_asserted = int_asserted;
	}

	/* otherwise, remember that at some later time this callout
	   should be attempted again: */
	else {
	  later_callouts |= TME_SUN_OBIE_CALLOUT_INT;
	}
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  sun_obie->tme_sun_obie_callout_flags = later_callouts;
}

/* the sun_obie bus cycle handler for the board registers: */
static int
_tme_sun_obie_bus_cycle_regs(void *_sun_obie, 
			     struct tme_bus_cycle *cycle_init)
{
  struct tme_sun_obie *sun_obie;
  tme_uint16_t csr_old, csr_new, csr_diff;
  int new_callouts;

  /* recover our data structure: */
  sun_obie = (struct tme_sun_obie *) _sun_obie;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&sun_obie->tme_sun_obie_mutex);

  /* get the previous CSR value: */
  csr_old = TME_SUN_OBIE_CSR_GET(sun_obie);

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    sun_obie->tme_sun_obie_regs,
			    TME_SUN_OBIE_SIZ_REGS - 1);

  /* get the current CSR value, and put back any bits that
     software can't change: */
  csr_new = ((TME_SUN_OBIE_CSR_GET(sun_obie)
	      & ~TME_SUN_OBIE_CSR_READONLY)
	     | (csr_old
		& TME_SUN_OBIE_CSR_READONLY));
  TME_SUN_OBIE_CSR_PUT(sun_obie, csr_new);

  /* get the sets of CSR bits that have changed: */
  csr_diff = (csr_old ^ csr_new);

  /* if this is a NORESET, NOLOOP or CA change, possibly call out the
     appropriate signal change to the i825x6: */
  if (csr_diff & (TME_SUN_OBIE_CSR_NORESET
		  | TME_SUN_OBIE_CSR_NOLOOP
		  | TME_SUN_OBIE_CSR_CA)) {
    new_callouts |= TME_SUN_OBIE_CALLOUT_SIGNALS;
  }
  
  /* if this is an interrupt mask change, possibly call out an
     interrupt signal change to the bus: */
  if (csr_diff & TME_SUN_OBIE_CSR_IE) {
    new_callouts |= TME_SUN_OBIE_CALLOUT_INT;
  }
  
#ifndef TME_NO_LOG
  if (csr_new != sun_obie->tme_sun_obie_last_log_csr) {
    sun_obie->tme_sun_obie_last_log_csr = csr_new;
    tme_log(&sun_obie->tme_sun_obie_element->tme_element_log_handle,
	    1000, TME_OK,
	    (&sun_obie->tme_sun_obie_element->tme_element_log_handle,
	     "csr now 0x%04x",
	     csr_new));
  }
#endif /* !TME_NO_LOG */

  /* make any new callouts: */
  _tme_sun_obie_callout(sun_obie, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_obie->tme_sun_obie_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the sun_obie bus signal handler: */
static int
_tme_sun_obie_bus_signal(struct tme_bus_connection *conn_bus, 
			 unsigned int signal)
{
  struct tme_sun_obie *sun_obie;
  tme_uint16_t csr;
  int new_callouts;

  /* return now if this is not a generic bus signal: */
  if (TME_BUS_SIGNAL_INDEX(signal)
      > _tme_sun_obie_bus_signals_generic.tme_bus_signals_count) {
    return (TME_OK);
  }

  /* recover our data structures: */
  sun_obie = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = TME_SUN_OBIE_CALLOUT_CHECK;

  /* lock the mutex: */
  tme_mutex_lock(&sun_obie->tme_sun_obie_mutex);

  /* get the current CSR value: */
  csr = TME_SUN_OBIE_CSR_GET(sun_obie);

  /* if this bus signal is from the i825x6: */
  if (conn_bus->tme_bus_connection.tme_connection_other
      == &sun_obie->tme_sun_obie_conn_i825x6->tme_bus_connection) {

    /* this must be the unspecified interrupt signal: */
    assert (TME_BUS_SIGNAL_WHICH(signal) == TME_BUS_SIGNAL_INT_UNSPEC);

    /* update the CSR value: */
    csr
      = ((csr
	  & ~TME_SUN_OBIE_CSR_INTR)
	 | (((signal & TME_BUS_SIGNAL_LEVEL_MASK)
	     == TME_BUS_SIGNAL_LEVEL_ASSERTED)
	    ? TME_SUN_OBIE_CSR_INTR
	    : 0));

    /* possibly call out an interrupt change to obio: */
    new_callouts = TME_SUN_OBIE_CALLOUT_INT;
  }

  /* otherwise, this bus signal must be from obio: */
  else {
    assert (conn_bus->tme_bus_connection.tme_connection_other
	    == &sun_obie->tme_sun_obie_conn_regs->tme_bus_connection);

    /* if this is the negating edge of the reset signal: */
    if (TME_BUS_SIGNAL_WHICH(signal) == TME_BUS_SIGNAL_RESET
	&& (signal & TME_BUS_SIGNAL_LEVEL_MASK) == TME_BUS_SIGNAL_LEVEL_NEGATED) {

      /* update the CSR value: */
      csr &= TME_SUN_OBIE_CSR_NOLOOP;

      /* possibly call out bus signal changes to the i825x6: */
      new_callouts = TME_SUN_OBIE_CALLOUT_SIGNALS;
    }
  }

  /* put the new CSR value: */
  TME_SUN_OBIE_CSR_PUT(sun_obie, csr);

  /* make any new callouts: */
  if (new_callouts != TME_SUN_OBIE_CALLOUT_CHECK) {
    _tme_sun_obie_callout(sun_obie, new_callouts);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_obie->tme_sun_obie_mutex);

  return (TME_OK);
}

/* the sun_obie bus signals adder for the i825x6: */
static int
_tme_sun_obie_bus_signals_add(struct tme_bus_connection *conn_bus,
			     struct tme_bus_signals *bus_signals_caller)
{
  const struct tme_bus_signals *bus_signals;
  tme_uint32_t signal_first;

  /* we only support the generic and i825x6 bus signals: */
  switch (bus_signals_caller->tme_bus_signals_id) {
  case TME_BUS_SIGNALS_ID_GENERIC:
    bus_signals = &_tme_sun_obie_bus_signals_generic;
    signal_first = _tme_sun_obie_bus_signals_generic.tme_bus_signals_first;
    break;
  case TME_BUS_SIGNALS_ID_I825X6:
    bus_signals = &_tme_sun_obie_bus_signals_i825x6;
    signal_first = (_tme_sun_obie_bus_signals_generic.tme_bus_signals_first
		    + TME_BUS_SIGNAL_X(_tme_sun_obie_bus_signals_generic.tme_bus_signals_count));
    break;
  default:
    return (ENOENT);
  }
  
  /* XXX we should check versions here: */
  *bus_signals_caller = *bus_signals;
  bus_signals_caller->tme_bus_signals_first = signal_first;
  return (TME_OK);
}

/* the sun_obie TLB adder for the i825x6: */
static int
_tme_sun_obie_tlb_set_add(struct tme_bus_connection *conn_bus,
			  struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_sun_obie *sun_obie;

  /* recover our data structures: */
  sun_obie = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* pass the i825x6's request through to the mainbus: */
  conn_bus = sun_obie->tme_sun_obie_conn_memory;
  return (conn_bus != NULL
	  ? (*conn_bus->tme_bus_tlb_set_add)(conn_bus, 
					     tlb_set_info)
	  : ENXIO);
}

/* the sun_obie TLB filler for the memory: */
static int
_tme_sun_obie_tlb_fill(struct tme_bus_connection *conn_bus,
		       struct tme_bus_tlb *tlb, 
		       tme_bus_addr_t address, 
		       unsigned int cycles)
{
  struct tme_sun_obie *sun_obie;

  /* the address must be within range: */
  assert(address <= 0xffffff);

  /* recover our data structures: */
  sun_obie = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* pass the i825x6's request through to the mainbus: */
  conn_bus = sun_obie->tme_sun_obie_conn_memory;
  return (conn_bus != NULL
	  ? (*conn_bus->tme_bus_tlb_fill)(conn_bus, 
					  tlb,
					  address,
					  cycles)
	  : ENXIO);
}

/* the sun_obie TLB filler for the board registers: */
static int
_tme_sun_obie_tlb_fill_regs(struct tme_bus_connection *conn_bus,
			   struct tme_bus_tlb *tlb, 
			   tme_bus_addr_t address, unsigned int cycles)
{
  struct tme_sun_obie *sun_obie;

  /* recover our data structures: */
  sun_obie = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* the address must be within range: */
  assert(address < TME_SUN_OBIE_SIZ_CSR);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this address falls in the CSR: */
    
  /* this TLB entry covers this range: */
  tlb->tme_bus_tlb_addr_first = TME_SUN_OBIE_REG_CSR;
  tlb->tme_bus_tlb_addr_last = TME_SUN_OBIE_REG_CSR + TME_SUN_OBIE_SIZ_CSR - 1;

  /* this TLB entry allows fast reading: */
  tlb->tme_bus_tlb_emulator_off_read = sun_obie->tme_sun_obie_regs;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = sun_obie;
  tlb->tme_bus_tlb_cycle = _tme_sun_obie_bus_cycle_regs;

  return (TME_OK);
}

/* this scores a new connection: */
static int
_tme_sun_obie_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_sun_obie *sun_obie;
  struct tme_sun_obie_connection *conn_sun_obie;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  sun_obie = conn->tme_connection_element->tme_element_private;
  conn_sun_obie = (struct tme_sun_obie_connection *)conn;

  /* this is a generic bus connection, so just score it nonzero and
     return.  note that there's no good way to differentiate a
     connection to a bus from a connection to just another chip, so we
     always return a nonzero score here: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_sun_obie_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sun_obie *sun_obie;
  struct tme_sun_obie_connection *conn_sun_obie;
  struct tme_bus_connection *conn_bus;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  sun_obie = conn->tme_connection_element->tme_element_private;
  conn_sun_obie = (struct tme_sun_obie_connection *)conn;
  conn_bus = &conn_sun_obie->tme_sun_obie_connection;

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&sun_obie->tme_sun_obie_mutex);

    /* save our connection: */
    if (conn_bus->tme_bus_signals_add != NULL) {
      sun_obie->tme_sun_obie_conn_i825x6 = (struct tme_bus_connection *) conn->tme_connection_other;
    }
    else if (conn_sun_obie->tme_sun_obie_connection_regs) {
      sun_obie->tme_sun_obie_conn_regs = (struct tme_bus_connection *) conn->tme_connection_other;
    }
    else {
      sun_obie->tme_sun_obie_conn_memory = (struct tme_bus_connection *) conn->tme_connection_other;
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&sun_obie->tme_sun_obie_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_sun_obie_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a sun_obie: */
static int
_tme_sun_obie_connections_new(struct tme_element *element,
			     const char * const *args,
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_sun_obie *sun_obie;
  struct tme_sun_obie_connection *conn_sun_obie;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  unsigned int i825x6;
  tme_uint8_t regs;
  int usage;
  int rc;

  /* recover our data structure: */
  sun_obie = (struct tme_sun_obie *) element->tme_element_private;
  
  /* we don't bother locking the mutex simply to check if connections
     already exist: */

  /* check our arguments: */
  usage = FALSE;
  rc = 0;
  i825x6 = FALSE;
  regs = FALSE;

  /* if this connection is for the registers: */
  if (TME_ARG_IS(args[1], "csr")) {

    /* if we already have a register connection, complain: */
    if (sun_obie->tme_sun_obie_conn_regs != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new connection: */
    else {
      regs = TRUE;
    }
  }

  /* else, if this connection is for the memory: */
  else if (TME_ARG_IS(args[1], "memory")) {

    /* if we already have a memory connection, complain: */
    if (sun_obie->tme_sun_obie_conn_memory != NULL) {
      rc = EEXIST;
    }
  }

  /* else, the connection must be for the i825x6: */
  else if (args[1] == NULL) {
    
    /* if we already have an i825x6 connection, complain: */
    if (sun_obie->tme_sun_obie_conn_i825x6 != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new conection: */
    else {
      i825x6 = TRUE;
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
			    "%s %s [ csr | memory ]",
			    _("usage:"),
			    args[0]);
    rc = EINVAL;
  }
  
  if (rc) {
    return (rc);
  }

  /* make a new connection: */
  conn_sun_obie = tme_new0(struct tme_sun_obie_connection, 1);
  conn_bus = &conn_sun_obie->tme_sun_obie_connection;
  conn = &conn_bus->tme_bus_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_sun_obie_connection_score;
  conn->tme_connection_make = _tme_sun_obie_connection_make;
  conn->tme_connection_break = _tme_sun_obie_connection_break;

  /* fill in the generic bus connection: */
  conn_bus->tme_bus_subregions.tme_bus_subregion_address_first = 0;
  conn_bus->tme_bus_subregions.tme_bus_subregion_next = NULL;
  if (i825x6) {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = 0xffffff;
    conn_bus->tme_bus_signals_add = _tme_sun_obie_bus_signals_add;
    conn_bus->tme_bus_signal = _tme_sun_obie_bus_signal;
    conn_bus->tme_bus_tlb_set_add = _tme_sun_obie_tlb_set_add;
    conn_bus->tme_bus_tlb_fill = _tme_sun_obie_tlb_fill;
  }
  else if (regs) {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_SUN_OBIE_SIZ_REGS - 1;
    conn_bus->tme_bus_signal = _tme_sun_obie_bus_signal;
    conn_bus->tme_bus_tlb_fill = _tme_sun_obie_tlb_fill_regs;
  }
  else {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = 0;
  }

  /* fill in the internal information: */
  conn_sun_obie->tme_sun_obie_connection_regs = regs;

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new sun_obie function: */
int
tme_sun_obie(struct tme_element *element, const char * const *args, char **_output)
{
  struct tme_sun_obie *sun_obie;
  int arg_i;
  int usage;

  /* check our arguments: */
  usage = 0;
  arg_i = 1;
  for (;;) {

    if (0) {
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
			    "%s %s",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the sun_obie structure: */
  sun_obie = tme_new0(struct tme_sun_obie, 1);
  sun_obie->tme_sun_obie_element = element;
  TME_SUN_OBIE_CSR_PUT(sun_obie,
		       (TME_SUN_OBIE_CSR_NORESET
			| TME_SUN_OBIE_CSR_NOLOOP));
  tme_mutex_init(&sun_obie->tme_sun_obie_mutex);
  tme_rwlock_init(&sun_obie->tme_sun_obie_rwlock);

  /* fill the element: */
  element->tme_element_private = sun_obie;
  element->tme_element_connections_new = _tme_sun_obie_connections_new;

  return (TME_OK);
}
