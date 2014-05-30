/* $Id: sun-mie.c,v 1.4 2010/06/05 13:57:27 fredette Exp $ */

/* bus/multibus/sun_mie.c - implementation of the Sun Intel Ethernet Multibus emulation: */

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
_TME_RCSID("$Id: sun-mie.c,v 1.4 2010/06/05 13:57:27 fredette Exp $");

/* includes: */
#include <tme/element.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>
#undef TME_I825X6_VERSION
#define TME_I825X6_VERSION TME_X_VERSION(0, 0)
#include <tme/ic/i825x6.h>

/* macros: */

/* the amount of memory on the board: */
#define TME_SUN_MIE_MEMSIZE	(256 * 1024)

/* the board page size: */
#define TME_SUN_MIE_PAGESIZE	(1024)

/* the number of page map entries: */
#define TME_SUN_MIE_PGMAP_COUNT	(1024)

/* the number of active TLB entries we can have per page map entry: */
#define TME_SUN_MIE_PGMAP_TLBS	(4)

/* register offsets and sizes: */
#define TME_SUN_MIE_REG_PGMAP	(0)
#define TME_SUN_MIE_SIZ_PGMAP	(TME_SUN_MIE_PGMAP_COUNT * sizeof(tme_uint16_t))
#define TME_SUN_MIE_REG_PROM	(TME_SUN_MIE_REG_PGMAP + TME_SUN_MIE_SIZ_PGMAP)
#define TME_SUN_MIE_SIZ_PROM	(32 * sizeof(tme_uint16_t))
#define TME_SUN_MIE_REG_CSR	(TME_SUN_MIE_REG_PROM + TME_SUN_MIE_SIZ_PROM)
#define TME_SUN_MIE_SIZ_CSR	(sizeof(tme_uint16_t))
#define TME_SUN_MIE_REG_PCR	(TME_SUN_MIE_REG_CSR + TME_SUN_MIE_SIZ_CSR + sizeof(tme_uint16_t))
#define TME_SUN_MIE_SIZ_PCR	(sizeof(tme_uint16_t))
#define TME_SUN_MIE_REG_PE_ALO	(TME_SUN_MIE_REG_PCR + TME_SUN_MIE_SIZ_PCR)
#define TME_SUN_MIE_SIZ_PE_ALO	(sizeof(tme_uint16_t))
#define TME_SUN_MIE_SIZ_REGS	(TME_SUN_MIE_REG_PE_ALO + TME_SUN_MIE_SIZ_PE_ALO)

/* the bits in a pagemap entry: */
#define TME_SUN_MIE_PGMAP_SWAB	(0x8000)
				/* 0x4000 unused */
#define TME_SUN_MIE_PGMAP_P2MEM	(0x2000)
				/* 0x1000 unused */
#define TME_SUN_MIE_PGMAP_PFNUM	(0x0fff)

/* the bits in the Control/Status Register: */
#define TME_SUN_MIE_CSR_RESET	(0x8000)
#define TME_SUN_MIE_CSR_NOLOOP	(0x4000)
#define TME_SUN_MIE_CSR_CA	(0x2000)
#define TME_SUN_MIE_CSR_IE	(0x1000)
#define TME_SUN_MIE_CSR_PIE	(0x0800)
				/* 0x0400 unused */
#define TME_SUN_MIE_CSR_PE	(0x0200)
#define TME_SUN_MIE_CSR_INTR	(0x0100)
				/* 0x0080 unused */
				/* 0x0040 unused */
#define TME_SUN_MIE_CSR_P2MEM	(0x0020)
#define TME_SUN_MIE_CSR_BIGRAM	(0x0010)
#define TME_SUN_MIE_CSR_MPMHI	(0x000f)
#define TME_SUN_MIE_CSR_READONLY	(0x0400			\
					 | TME_SUN_MIE_CSR_PE	\
					 | TME_SUN_MIE_CSR_INTR	\
					 | 0x0080		\
					 | 0x0040		\
					 | TME_SUN_MIE_CSR_BIGRAM\
					 | TME_SUN_MIE_CSR_MPMHI)

/* the bits in the Parity Control Register: */
#define TME_SUN_MIE_PCR_PEACK	(0x0100)
#define TME_SUN_MIE_PCR_PESRC	(0x0080)
#define TME_SUN_MIE_PCR_PEBYTE	(0x0040)
				/* 0x0020 unused */
				/* 0x0010 unused */
#define TME_SUN_MIE_PCR_PE_AHI	(0x000f)
#define TME_SUN_MIE_PCR_READONLY	(~TME_SUN_MIE_PCR_PEACK)

/* the size of the memory port: */
#define TME_SUN_MIE_SIZ_MBM	(0x10000)

/* these get and put the CSR: */
#define TME_SUN_MIE_CSR_GET(sun_mie)	\
  tme_betoh_u16(*((tme_uint16_t *) &(sun_mie)->tme_sun_mie_regs[TME_SUN_MIE_REG_CSR]))
#define TME_SUN_MIE_CSR_PUT(sun_mie, csr)	\
  (*((tme_uint16_t *) &(sun_mie)->tme_sun_mie_regs[TME_SUN_MIE_REG_CSR]) = tme_htobe_u16(csr))

/* these get and put the PCR: */
#define TME_SUN_MIE_PCR_GET(sun_mie)	\
  tme_betoh_u16(*((tme_uint16_t *) &(sun_mie)->tme_sun_mie_regs[TME_SUN_MIE_REG_PCR]))
#define TME_SUN_MIE_PCR_PUT(sun_mie, pcr)	\
  (*((tme_uint16_t *) &(sun_mie)->tme_sun_mie_regs[TME_SUN_MIE_REG_PCR]) = tme_htobe_u16(pcr))

/* the callout flags: */
#define TME_SUN_MIE_CALLOUT_CHECK	(0)
#define TME_SUN_MIE_CALLOUT_RUNNING	TME_BIT(0)
#define TME_SUN_MIE_CALLOUTS_MASK	(-2)
#define  TME_SUN_MIE_CALLOUT_SIGNALS	TME_BIT(1)
#define	 TME_SUN_MIE_CALLOUT_INT	TME_BIT(2)

/* structures: */

/* the card: */
struct tme_sun_mie {

  /* backpointer to our element: */
  struct tme_element *tme_sun_mie_element;

  /* the mutex protecting the card: */
  tme_mutex_t tme_sun_mie_mutex;

  /* the rwlock protecting the card: */
  tme_rwlock_t tme_sun_mie_rwlock;

  /* the bus connection for the card's registers: */
  struct tme_bus_connection *tme_sun_mie_conn_regs;

  /* the bus connection for the card's memory: */
  struct tme_bus_connection *tme_sun_mie_conn_memory;

  /* the bus connection for the card's i825x6 chip: */
  struct tme_bus_connection *tme_sun_mie_conn_i825x6;

  /* the callout flags: */
  int tme_sun_mie_callout_flags;

  /* if our interrupt line is currently asserted: */
  int tme_sun_mie_int_asserted;

  /* it's easiest to just model the board registers as a chunk of memory: */
  tme_uint8_t tme_sun_mie_regs[TME_SUN_MIE_SIZ_REGS];

  /* the board memory really is a chunk of memory: */
  tme_uint8_t tme_sun_mie_memory[TME_SUN_MIE_MEMSIZE];

  /* the active TLB entries for each page map entry on the board: */
  struct tme_token *tme_sun_mie_tlb_tokens[TME_SUN_MIE_PGMAP_COUNT * TME_SUN_MIE_PGMAP_TLBS];
  unsigned int tme_sun_mie_tlb_head[TME_SUN_MIE_PGMAP_COUNT];

  /* the i825x6 image of the CSR: */
  tme_uint16_t tme_sun_mie_csr_i825x6;

#ifndef TME_NO_LOG
  tme_uint16_t tme_sun_mie_last_log_csr;
#endif /* !TME_NO_LOG */
};

/* a sun_mie internal bus connection: */
struct tme_sun_mie_connection {

  /* the external bus connection: */
  struct tme_bus_connection tme_sun_mie_connection;

  /* this is nonzero if a TME_CONNECTION_BUS_GENERIC chip connection
     is for the registers: */
  tme_uint8_t tme_sun_mie_connection_regs;

  /* if this connection is for the memory port, this is the high
     nibble (A19..A16) of that port's connection: */
  tme_uint8_t tme_sun_mie_connection_mpmhi;
};

/* globals: */

/* our bus signals sets: */
static const struct tme_bus_signals _tme_sun_mie_bus_signals_generic = TME_BUS_SIGNALS_GENERIC;
static const struct tme_bus_signals _tme_sun_mie_bus_signals_i825x6 = TME_BUS_SIGNALS_I825X6;

/* the sun_mie callout function.  it must be called with the mutex locked: */
static void
_tme_sun_mie_callout(struct tme_sun_mie *sun_mie, int new_callouts)
{
  struct tme_bus_connection *conn_i825x6;
  struct tme_bus_connection *conn_bus;
  tme_uint16_t csr, csr_diff;
  unsigned int signal, level;
  int callouts, later_callouts;
  int rc;
  int int_asserted;
  
  /* add in any new callouts: */
  sun_mie->tme_sun_mie_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (sun_mie->tme_sun_mie_callout_flags & TME_SUN_MIE_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  sun_mie->tme_sun_mie_callout_flags |= TME_SUN_MIE_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; (callouts = sun_mie->tme_sun_mie_callout_flags) & TME_SUN_MIE_CALLOUTS_MASK; ) {

    /* clear the needed callouts: */
    sun_mie->tme_sun_mie_callout_flags = callouts & ~TME_SUN_MIE_CALLOUTS_MASK;
    callouts &= TME_SUN_MIE_CALLOUTS_MASK;

    /* if we need to call out one or more signals to the i825x6: */
    if (callouts & TME_SUN_MIE_CALLOUT_SIGNALS) {

      /* get the current CSR value: */
      csr = TME_SUN_MIE_CSR_GET(sun_mie);

      /* get the next signal to call out to the i825x6: */
      csr_diff = ((csr
		   ^ sun_mie->tme_sun_mie_csr_i825x6)
		  & (TME_SUN_MIE_CSR_RESET
		     | TME_SUN_MIE_CSR_NOLOOP
		     | TME_SUN_MIE_CSR_CA));
      csr_diff = (csr_diff ^ (csr_diff - 1)) & csr_diff;
      
      /* if there is a signal to call out: */
      if (csr_diff != 0) {

	/* assume that if the signal's bit is set in the CSR, it will
	   be asserted: */
	level = csr & csr_diff;

	/* assume that we're calling out an i825x6 signal: */
	signal = (_tme_sun_mie_bus_signals_generic.tme_bus_signals_first
		  + TME_BUS_SIGNAL_X(_tme_sun_mie_bus_signals_generic.tme_bus_signals_count));

	/* dispatch on the CSR bit: */
	switch (csr_diff) {
	default:
	  assert (FALSE);
	case TME_SUN_MIE_CSR_RESET:
	  signal = TME_BUS_SIGNAL_RESET;
	  break;
	case TME_SUN_MIE_CSR_NOLOOP:
	  signal += TME_I825X6_SIGNAL_LOOP;
	  level = !level;
	  break;
	case TME_SUN_MIE_CSR_CA:
	  signal += TME_I825X6_SIGNAL_CA;
	  break;
	}

	/* create a real signal level value: */
	level = (level
		 ? TME_BUS_SIGNAL_LEVEL_ASSERTED
		 : TME_BUS_SIGNAL_LEVEL_NEGATED);

	/* get this card's i825x6 connection: */
	conn_i825x6 = sun_mie->tme_sun_mie_conn_i825x6;

	/* unlock the mutex: */
	tme_mutex_unlock(&sun_mie->tme_sun_mie_mutex);
      
	/* do the callout: */
	rc = (conn_i825x6 != NULL
	      ? ((*conn_i825x6->tme_bus_signal)
		 (conn_i825x6,
		  signal | level))
	      : TME_OK);
	
	/* lock the mutex: */
	tme_mutex_lock(&sun_mie->tme_sun_mie_mutex);
      
	/* if the callout was unsuccessful, remember that at some later
	   time this callout should be attempted again: */
	if (rc != TME_OK) {
	  later_callouts |= TME_SUN_MIE_CALLOUT_SIGNALS;
	}

	/* otherwise, the callout was successful: */
	else {

	  /* update the i825x6 image of the CSR: */
	  sun_mie->tme_sun_mie_csr_i825x6 = 
	    ((sun_mie->tme_sun_mie_csr_i825x6 & ~csr_diff)
	     | (csr & csr_diff));

	  /* there may be more signals to call out, so attempt this
             callout again now: */
	  sun_mie->tme_sun_mie_callout_flags |= TME_SUN_MIE_CALLOUT_SIGNALS;
	}
      }
    }

    /* if we need to call out a possible change to our interrupt
       signal: */
    if (callouts & TME_SUN_MIE_CALLOUT_INT) {

      /* get the current CSR value: */
      csr = TME_SUN_MIE_CSR_GET(sun_mie);

      /* see if the interrupt signal should be asserted or negated: */
      int_asserted = ((csr & (TME_SUN_MIE_CSR_IE
			      | TME_SUN_MIE_CSR_INTR))
		      == (TME_SUN_MIE_CSR_IE
			  | TME_SUN_MIE_CSR_INTR));

      /* if the interrupt signal doesn't already have the right state: */
      if (!int_asserted != !sun_mie->tme_sun_mie_int_asserted) {

	/* get our bus connection: */
	conn_bus = sun_mie->tme_sun_mie_conn_regs;
	
	/* unlock our mutex: */
	tme_mutex_unlock(&sun_mie->tme_sun_mie_mutex);
	
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
	tme_mutex_lock(&sun_mie->tme_sun_mie_mutex);
	
	/* if this callout was successful, note the new state of the
	   interrupt signal: */
	if (rc == TME_OK) {
	  sun_mie->tme_sun_mie_int_asserted = int_asserted;
	}

	/* otherwise, remember that at some later time this callout
	   should be attempted again: */
	else {
	  later_callouts |= TME_SUN_MIE_CALLOUT_INT;
	}
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  sun_mie->tme_sun_mie_callout_flags = later_callouts;
}

/* the sun_mie bus cycle handler for the board memory: */
static int
_tme_sun_mie_bus_cycle(void *_sun_mie, 
		       struct tme_bus_cycle *cycle_init)
{
  struct tme_sun_mie *sun_mie;

  /* recover our data structure: */
  sun_mie = (struct tme_sun_mie *) _sun_mie;

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    sun_mie->tme_sun_mie_memory,
			    TME_SUN_MIE_MEMSIZE - 1);

  /* no faults: */
  return (TME_OK);
}

/* the sun_mie bus cycle handler for the board registers: */
static int
_tme_sun_mie_bus_cycle_regs(void *_sun_mie, 
			    struct tme_bus_cycle *cycle_init)
{
  struct tme_sun_mie *sun_mie;
  unsigned int pgmap_i;
  unsigned int pgmap_j;
  unsigned int tlb_i;
  unsigned int tlb_j;
  tme_uint16_t csr_old, csr_new, csr_diff;
  tme_uint16_t pcr_old, pcr_new, pcr_diff;
  int new_callouts;

  /* recover our data structure: */
  sun_mie = (struct tme_sun_mie *) _sun_mie;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&sun_mie->tme_sun_mie_mutex);

  /* if this is a write cycle and the address falls within one or more
     page map entries, invalidate the TLBs associated with those
     entries: */
  if ((cycle_init->tme_bus_cycle_type
       & TME_BUS_CYCLE_WRITE)
      && (TME_SUN_MIE_REG_PGMAP
	  <= cycle_init->tme_bus_cycle_address)
      && (cycle_init->tme_bus_cycle_address
	  < (TME_SUN_MIE_REG_PGMAP
	     + TME_SUN_MIE_SIZ_PGMAP))) {

    /* get the range of page map entries: */
    pgmap_i
      = (cycle_init->tme_bus_cycle_address
	 / sizeof(tme_uint16_t));
    pgmap_j
      = ((cycle_init->tme_bus_cycle_address
	  + cycle_init->tme_bus_cycle_size
	  + sizeof(tme_uint16_t)
	  - 1)
	 / sizeof(tme_uint16_t));
    pgmap_j = TME_MIN(pgmap_j, TME_SUN_MIE_PGMAP_COUNT);

    /* get the range of TLB handles: */
    tlb_i = pgmap_i * TME_SUN_MIE_PGMAP_TLBS;
    tlb_j = pgmap_j * TME_SUN_MIE_PGMAP_TLBS;

    /* invalidate the TLB entries: */
    for (; tlb_i < tlb_j; tlb_i++) {
      if (sun_mie->tme_sun_mie_tlb_tokens[tlb_i] != NULL) {
	tme_token_invalidate(sun_mie->tme_sun_mie_tlb_tokens[tlb_i]);
	sun_mie->tme_sun_mie_tlb_tokens[tlb_i] = NULL;
      }
    }
  }

  /* get the previous CSR and PCR values: */
  csr_old = TME_SUN_MIE_CSR_GET(sun_mie);
  pcr_old = TME_SUN_MIE_PCR_GET(sun_mie);

  /* unless this address falls within the PROM, run the cycle: */
  if ((cycle_init->tme_bus_cycle_address 
       < TME_SUN_MIE_REG_PROM)
      || (cycle_init->tme_bus_cycle_address
	  >= (TME_SUN_MIE_REG_PROM
	      + TME_SUN_MIE_SIZ_PROM))) {
    tme_bus_cycle_xfer_memory(cycle_init, 
			      sun_mie->tme_sun_mie_regs,
			      TME_SUN_MIE_SIZ_REGS - 1);
  }

  /* get the current CSR and PCR values, and put back any bits that
     software can't change: */
  csr_new = ((TME_SUN_MIE_CSR_GET(sun_mie)
	      & ~TME_SUN_MIE_CSR_READONLY)
	     | (csr_old
		& TME_SUN_MIE_CSR_READONLY));
  TME_SUN_MIE_CSR_PUT(sun_mie, csr_new);
  pcr_new = ((TME_SUN_MIE_PCR_GET(sun_mie)
	      & ~TME_SUN_MIE_PCR_READONLY)
	     | (pcr_old
		& TME_SUN_MIE_PCR_READONLY));
  TME_SUN_MIE_PCR_PUT(sun_mie, pcr_new);

  /* get the sets of CSR and PCR bits that have changed: */
  csr_diff = (csr_old ^ csr_new);
  pcr_diff = (pcr_old ^ pcr_new);

  /* if this is a RESET, NOLOOP or CA change, possibly call out the
     appropriate signal change to the i825x6: */
  if (csr_diff & (TME_SUN_MIE_CSR_RESET
		  | TME_SUN_MIE_CSR_NOLOOP
		  | TME_SUN_MIE_CSR_CA)) {
    new_callouts |= TME_SUN_MIE_CALLOUT_SIGNALS;
  }
  
  /* if this is an interrupt mask change, possibly call out an
     interrupt signal change to the bus: */
  if (csr_diff & TME_SUN_MIE_CSR_IE) {
    new_callouts |= TME_SUN_MIE_CALLOUT_INT;
  }
  
  /* abort on any attempt to enable the P2 bus: */
  if (csr_new & TME_SUN_MIE_CSR_P2MEM) {
    abort ();
  }
  
  /* if this is acknowledging a parity error: */
  if (pcr_diff & TME_SUN_MIE_PCR_PEACK) {
    /* nothing to do */
  }

#ifndef TME_NO_LOG
  if (csr_new != sun_mie->tme_sun_mie_last_log_csr) {
    sun_mie->tme_sun_mie_last_log_csr = csr_new;
    tme_log(&sun_mie->tme_sun_mie_element->tme_element_log_handle,
	    1000, TME_OK,
	    (&sun_mie->tme_sun_mie_element->tme_element_log_handle,
	     "csr now 0x%04x",
	     csr_new));
  }
#endif /* !TME_NO_LOG */

  /* make any new callouts: */
  _tme_sun_mie_callout(sun_mie, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&sun_mie->tme_sun_mie_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the sun_mie bus signal handler: */
static int
_tme_sun_mie_bus_signal(struct tme_bus_connection *conn_bus, 
			unsigned int signal)
{
  struct tme_sun_mie *sun_mie;

  /* return now if this is not a generic bus signal: */
  if (TME_BUS_SIGNAL_INDEX(signal)
      > _tme_sun_mie_bus_signals_generic.tme_bus_signals_count) {
    return (TME_OK);
  }

  /* recover our data structures: */
  sun_mie = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* since this function is currently only given to the i825x6,
     just copy its signal through to the Multibus: */
  conn_bus = sun_mie->tme_sun_mie_conn_regs;
  return ((*conn_bus->tme_bus_signal)(conn_bus, signal));
}

/* the sun_mie bus signals adder for the i825x6: */
static int
_tme_sun_mie_bus_signals_add(struct tme_bus_connection *conn_bus,
			     struct tme_bus_signals *bus_signals_caller)
{
  const struct tme_bus_signals *bus_signals;
  tme_uint32_t signal_first;

  /* we only support the generic and i825x6 bus signals: */
  switch (bus_signals_caller->tme_bus_signals_id) {
  case TME_BUS_SIGNALS_ID_GENERIC:
    bus_signals = &_tme_sun_mie_bus_signals_generic;
    signal_first = _tme_sun_mie_bus_signals_generic.tme_bus_signals_first;
    break;
  case TME_BUS_SIGNALS_ID_I825X6:
    bus_signals = &_tme_sun_mie_bus_signals_i825x6;
    signal_first = (_tme_sun_mie_bus_signals_generic.tme_bus_signals_first
		    + TME_BUS_SIGNAL_X(_tme_sun_mie_bus_signals_generic.tme_bus_signals_count));
    break;
  default:
    return (ENOENT);
  }
  
  /* XXX we should check versions here: */
  *bus_signals_caller = *bus_signals;
  bus_signals_caller->tme_bus_signals_first = signal_first;
  return (TME_OK);
}

/* the sun_mie TLB adder for the i825x6: */
static int
_tme_sun_mie_tlb_set_add(struct tme_bus_connection *conn_bus,
			 struct tme_bus_tlb_set_info *tlb_set_info)
{

  /* if this TLB set provides a bus context register: */
  if (tlb_set_info->tme_bus_tlb_set_info_bus_context != NULL) {

    /* this bus only has one context: */
    (*tlb_set_info->tme_bus_tlb_set_info_bus_context) = 0;
    tlb_set_info->tme_bus_tlb_set_info_bus_context_max = 0;
  }

  return (TME_OK);
}

/* the sun_mie TLB filler for the board memory: */
static int
_tme_sun_mie_tlb_fill(struct tme_bus_connection *conn_bus,
		      struct tme_bus_tlb *tlb, 
		      tme_bus_addr_t address_wider,
		      unsigned int cycles)
{
  struct tme_sun_mie *sun_mie;
  tme_bus_addr32_t address;
  unsigned int pgmap_i;
  unsigned int tlb_i;
  tme_uint16_t pgmap;

  /* recover our data structures: */
  sun_mie = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* the address must be within range: */
  assert(address <= 0xffffff);

  /* mask the address with the board page size: */
  address &= -TME_SUN_MIE_PAGESIZE;

  /* lock our mutex: */
  tme_mutex_lock(&sun_mie->tme_sun_mie_mutex);

  /* get the pagemap entry: */
  pgmap_i = (address / TME_SUN_MIE_PAGESIZE) & (TME_SUN_MIE_PGMAP_COUNT - 1);
  pgmap = tme_betoh_u16(((tme_uint16_t *) &sun_mie->tme_sun_mie_regs[TME_SUN_MIE_REG_PGMAP])[pgmap_i]);

  /* update the head pointer for this page map entry's active TLB
     entry list: */
  tlb_i = sun_mie->tme_sun_mie_tlb_head[pgmap_i];
  if (++tlb_i == TME_SUN_MIE_PGMAP_TLBS) {
    tlb_i = 0;
  }
  sun_mie->tme_sun_mie_tlb_head[pgmap_i] = tlb_i;
  tlb_i += pgmap_i * TME_SUN_MIE_PGMAP_TLBS;

  /* if the new head pointer already has a TLB entry, and it doesn't
     happen to be the same one that we're filling now, invalidate it: */
  if (sun_mie->tme_sun_mie_tlb_tokens[tlb_i] != NULL
      && sun_mie->tme_sun_mie_tlb_tokens[tlb_i] != tlb->tme_bus_tlb_token) {
    tme_token_invalidate(sun_mie->tme_sun_mie_tlb_tokens[tlb_i]);
  }

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers this range: */
  tlb->tme_bus_tlb_addr_first = address;
  tlb->tme_bus_tlb_addr_last = address | (TME_SUN_MIE_PAGESIZE - 1);

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = sun_mie;
  tlb->tme_bus_tlb_cycle = _tme_sun_mie_bus_cycle;

  /* this TLB entry allows fast reading and writing: */
  tlb->tme_bus_tlb_emulator_off_write = 
    (&sun_mie->tme_sun_mie_memory[((pgmap & TME_SUN_MIE_PGMAP_PFNUM)
				   * TME_SUN_MIE_PAGESIZE)]
     - address);
  tlb->tme_bus_tlb_emulator_off_read = 
    tlb->tme_bus_tlb_emulator_off_write;

  /* add this TLB entry to the active list: */
  sun_mie->tme_sun_mie_tlb_tokens[tlb_i] =
    tlb->tme_bus_tlb_token;

  /* unlock our mutex: */
  tme_mutex_unlock(&sun_mie->tme_sun_mie_mutex);

  return (TME_OK);
}

/* the sun_mie TLB filler for the board registers: */
static int
_tme_sun_mie_tlb_fill_regs(struct tme_bus_connection *conn_bus,
			   struct tme_bus_tlb *tlb, 
			   tme_bus_addr_t address_wider,
			   unsigned int cycles)
{
  struct tme_sun_mie *sun_mie;
  tme_bus_addr32_t address;

  /* recover our data structures: */
  sun_mie = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* the address must be within range: */
  assert(address < TME_SUN_MIE_SIZ_REGS);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* if the address falls in the page map: */
  if (TME_SUN_MIE_REG_PGMAP <= address
      && address < (TME_SUN_MIE_REG_PGMAP
		    + TME_SUN_MIE_SIZ_PGMAP)) {
    
    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_SUN_MIE_REG_PGMAP;
    tlb->tme_bus_tlb_addr_last = (TME_SUN_MIE_REG_PGMAP + TME_SUN_MIE_SIZ_PGMAP - 1);
  }

  /* if this address falls in the PROM: */
  else if (TME_SUN_MIE_REG_PROM <= address
	   && address < (TME_SUN_MIE_REG_PROM
			 + TME_SUN_MIE_SIZ_PROM)) {

    
    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_SUN_MIE_REG_PROM;
    tlb->tme_bus_tlb_addr_last = TME_SUN_MIE_REG_PROM + TME_SUN_MIE_SIZ_PROM - 1;
  }

  /* if this address falls in the CSR: */
  else if (TME_SUN_MIE_REG_CSR <= address
	   && address < (TME_SUN_MIE_REG_CSR
			 + TME_SUN_MIE_SIZ_CSR)) {

    
    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_SUN_MIE_REG_CSR;
    tlb->tme_bus_tlb_addr_last = TME_SUN_MIE_REG_CSR + TME_SUN_MIE_SIZ_CSR - 1;
  }

  /* otherwise, this address must fall in the unused hole or in the
     parity registers: */
  else {
    assert (address >= (TME_SUN_MIE_REG_CSR
			+ TME_SUN_MIE_SIZ_CSR));

    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = (TME_SUN_MIE_REG_CSR + TME_SUN_MIE_SIZ_CSR);
    tlb->tme_bus_tlb_addr_last = (TME_SUN_MIE_REG_PE_ALO + TME_SUN_MIE_SIZ_PE_ALO - 1);
  }

  /* all address ranges allow fast reading: */
  tlb->tme_bus_tlb_emulator_off_read = &sun_mie->tme_sun_mie_regs[0];
  tlb->tme_bus_tlb_rwlock = &sun_mie->tme_sun_mie_rwlock;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = sun_mie;
  tlb->tme_bus_tlb_cycle = _tme_sun_mie_bus_cycle_regs;

  return (TME_OK);
}

/* this scores a new connection: */
static int
_tme_sun_mie_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_sun_mie *sun_mie;
  struct tme_sun_mie_connection *conn_sun_mie;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  sun_mie = conn->tme_connection_element->tme_element_private;
  conn_sun_mie = (struct tme_sun_mie_connection *)conn;

  /* this is a generic bus connection, so just score it nonzero and
     return.  note that there's no good way to differentiate a
     connection to a bus from a connection to just another chip, so we
     always return a nonzero score here: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_sun_mie_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sun_mie *sun_mie;
  struct tme_sun_mie_connection *conn_sun_mie;
  struct tme_bus_connection *conn_bus;
  tme_uint16_t csr;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  sun_mie = conn->tme_connection_element->tme_element_private;
  conn_sun_mie = (struct tme_sun_mie_connection *)conn;
  conn_bus = &conn_sun_mie->tme_sun_mie_connection;

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&sun_mie->tme_sun_mie_mutex);

    /* save our connection: */
    if (conn_bus->tme_bus_signals_add != NULL) {
      sun_mie->tme_sun_mie_conn_i825x6 = (struct tme_bus_connection *) conn->tme_connection_other;
    }
    else if (conn_sun_mie->tme_sun_mie_connection_regs) {
      sun_mie->tme_sun_mie_conn_regs = (struct tme_bus_connection *) conn->tme_connection_other;
    }
    else {
      sun_mie->tme_sun_mie_conn_memory = (struct tme_bus_connection *) conn->tme_connection_other;
      csr = TME_SUN_MIE_CSR_GET(sun_mie);
      csr &= ~TME_SUN_MIE_CSR_MPMHI;
      csr |= conn_sun_mie->tme_sun_mie_connection_mpmhi;
      TME_SUN_MIE_CSR_PUT(sun_mie, csr);
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&sun_mie->tme_sun_mie_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_sun_mie_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a sun_mie: */
static int
_tme_sun_mie_connections_new(struct tme_element *element,
			     const char * const *args,
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_sun_mie *sun_mie;
  struct tme_sun_mie_connection *conn_sun_mie;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  unsigned int i825x6;
  tme_uint8_t regs;
  tme_bus_addr_t mpmhi;
  int usage;
  int rc;

  /* recover our data structure: */
  sun_mie = (struct tme_sun_mie *) element->tme_element_private;
  
  /* we don't bother locking the mutex simply to check if connections
     already exist: */

  /* check our arguments: */
  usage = FALSE;
  rc = 0;
  i825x6 = FALSE;
  regs = FALSE;
  mpmhi = 0;

  /* if this connection is for the registers: */
  if (TME_ARG_IS(args[1], "csr")) {

    /* if we already have a register connection, complain: */
    if (sun_mie->tme_sun_mie_conn_regs != NULL) {
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
    if (sun_mie->tme_sun_mie_conn_memory != NULL) {
      rc = EEXIST;
    }

    /* otherwise, check the value after "memory".  it must be the low
       20 bits of the bus address of the board's memory; in our csr we
       have to report the most significant nibble (A19-A16, as A15..A0
       are expected to be zero) to software so it can find that
       memory: */
    else {
      mpmhi = tme_bus_addr_parse_any(args[2], &usage);
      if (!usage
	  && ((mpmhi > 0xfffff)
	      || (mpmhi & (TME_SUN_MIE_SIZ_MBM - 1)))) {
	tme_output_append_error(_output,
				"%s %s, ",
				args[2],
				_(" is not a 20-bit address with A15..A0 zero"));
	usage = TRUE;
      }
    }
  }

  /* else, the connection must be for the i825x6: */
  else if (args[1] == NULL) {
    
    /* if we already have an i825x6 connection, complain: */
    if (sun_mie->tme_sun_mie_conn_i825x6 != NULL) {
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
			    "%s %s [ csr | memory %s ]",
			    _("usage:"),
			    args[0],
			    _("BUS-ADDRESS"));
    rc = EINVAL;
  }
  
  if (rc) {
    return (rc);
  }

  /* make a new connection: */
  conn_sun_mie = tme_new0(struct tme_sun_mie_connection, 1);
  conn_bus = &conn_sun_mie->tme_sun_mie_connection;
  conn = &conn_bus->tme_bus_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_sun_mie_connection_score;
  conn->tme_connection_make = _tme_sun_mie_connection_make;
  conn->tme_connection_break = _tme_sun_mie_connection_break;

  /* fill in the generic bus connection: */
  conn_bus->tme_bus_subregions.tme_bus_subregion_address_first = 0;
  conn_bus->tme_bus_subregions.tme_bus_subregion_next = NULL;
  if (i825x6) {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = 0xffffff;
    conn_bus->tme_bus_signals_add = _tme_sun_mie_bus_signals_add;
    conn_bus->tme_bus_signal = _tme_sun_mie_bus_signal;
    conn_bus->tme_bus_tlb_set_add = _tme_sun_mie_tlb_set_add;
    conn_bus->tme_bus_tlb_fill = _tme_sun_mie_tlb_fill;
  }
  else if (regs) {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_SUN_MIE_SIZ_REGS - 1;
    conn_bus->tme_bus_tlb_fill = _tme_sun_mie_tlb_fill_regs;
  }
  else {
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = TME_SUN_MIE_SIZ_MBM - 1;
    conn_bus->tme_bus_tlb_fill = _tme_sun_mie_tlb_fill;
  }

  /* fill in the internal information: */
  conn_sun_mie->tme_sun_mie_connection_regs = regs;
  conn_sun_mie->tme_sun_mie_connection_mpmhi = (mpmhi >> 16);

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new sun_mie function: */
TME_ELEMENT_SUB_NEW_DECL(tme_bus_multibus,sun_mie) {
  struct tme_sun_mie *sun_mie;
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

  /* start the sun_mie structure: */
  sun_mie = tme_new0(struct tme_sun_mie, 1);
  sun_mie->tme_sun_mie_element = element;
  TME_SUN_MIE_CSR_PUT(sun_mie, 
		      (TME_SUN_MIE_CSR_NOLOOP
		       | TME_SUN_MIE_CSR_BIGRAM));
  tme_mutex_init(&sun_mie->tme_sun_mie_mutex);
  tme_rwlock_init(&sun_mie->tme_sun_mie_rwlock);

  /* fill the element: */
  element->tme_element_private = sun_mie;
  element->tme_element_connections_new = _tme_sun_mie_connections_new;

  return (TME_OK);
}
