/* $Id: bus-el.c,v 1.18 2009/08/29 17:59:17 fredette Exp $ */

/* generic/bus-el.c - a real generic bus element: */

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
_TME_RCSID("$Id: bus-el.c,v 1.18 2009/08/29 17:59:17 fredette Exp $");

/* includes: */
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>
#include <stdlib.h>
#include <string.h>

/* macros: */

/* globals: */

/* the generic bus signals: */
static const struct tme_bus_signals _tme_bus_signals_default[] = {
  TME_BUS_SIGNALS_GENERIC
};

/* this adds a bus signal set to the bus: */
static int
_tme_bus_signals_add(struct tme_bus_connection *conn_bus_caller,
		     struct tme_bus_signals *bus_signals)
{
  struct tme_bus *bus;
  unsigned int signal_i;
  tme_uint32_t signals_count_new;
  tme_uint32_t signals_count_old;
  struct tme_bus_connection_int *conn_bus_int;
  int rc;

  /* recover our bus: */
  bus = conn_bus_caller->tme_bus_connection.tme_connection_element->tme_element_private;

  /* lock the bus for writing: */
  rc = tme_rwlock_timedwrlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);
  if (TME_THREADS_ERRNO(rc) != TME_OK) {
    return (TME_THREADS_ERRNO(rc));
  }

  /* search for an existing bus signals set that matches the caller's: */
  for (signal_i = 0;
       signal_i < bus->tme_bus_signals_count;
       signal_i++) {

    /* stop if this existing bus signals set has the right ID and the
       versions overlap: */
    if ((bus->tme_bus_signals[signal_i].tme_bus_signals_id
	 == bus_signals->tme_bus_signals_id)
	&& TME_X_VERSION_OK(bus->tme_bus_signals[signal_i].tme_bus_signals_version,
			    bus_signals->tme_bus_signals_version)) {
      break;
    }
  }

  /* assume that this call succeeds: */
  rc = TME_OK;

  /* if an existing bus signals set was not found: */
  if (signal_i == bus->tme_bus_signals_count) {

    /* get the old count of bus signals from the current last bus
       signals set in the bus signals sets array: */
    signals_count_old =
      (TME_BUS_SIGNAL_INDEX(bus->tme_bus_signals[bus->tme_bus_signals_count - 1].tme_bus_signals_first)
       + bus->tme_bus_signals[bus->tme_bus_signals_count - 1].tme_bus_signals_count);

    /* resize the bus signals sets array: */
    bus->tme_bus_signals
      = tme_renew(struct tme_bus_signals,
		  bus->tme_bus_signals,
		  bus->tme_bus_signals_count
		  + 1);

    /* add the new bus signals set: */
    signals_count_new = signals_count_old + bus_signals->tme_bus_signals_count;
    assert (signals_count_new > signals_count_old);
    bus_signals->tme_bus_signals_first = TME_BUS_SIGNAL_X(signals_count_old);
    bus->tme_bus_signals[bus->tme_bus_signals_count] = *bus_signals;
    bus->tme_bus_signals_count++;

    /* reallocate the bus' asserted-signals count array: */
    bus->tme_bus_signal_asserts
      = tme_renew(unsigned int,
		  bus->tme_bus_signal_asserts,
		  signals_count_new);
    memset ((char *) &bus->tme_bus_signal_asserts[signals_count_old],
	    0,
	    (sizeof(bus->tme_bus_signal_asserts[0])
	     * (signals_count_new
		- signals_count_old)));

    /* if needed, reallocate each connection's asserted-signals
       bitmap: */
    if (TME_BUS_SIGNAL_BIT_BYTES(signals_count_new)
	> TME_BUS_SIGNAL_BIT_BYTES(signals_count_old)) {
      for (conn_bus_int = bus->tme_bus_connections;
	   conn_bus_int != NULL;
	   conn_bus_int =
	     (struct tme_bus_connection_int *) 
	     conn_bus_int->tme_bus_connection_int
	     .tme_bus_connection
	     .tme_connection_next) {
	conn_bus_int->tme_bus_connection_int_signals
	  = tme_renew(tme_uint8_t,
		      conn_bus_int->tme_bus_connection_int_signals,
		      TME_BUS_SIGNAL_BIT_BYTES(signals_count_new));
	memset ((char *) &conn_bus_int->tme_bus_connection_int_signals[TME_BUS_SIGNAL_BIT_BYTES(signals_count_old)],
		0,
		(sizeof (conn_bus_int->tme_bus_connection_int_signals[0])
		 * (TME_BUS_SIGNAL_BIT_BYTES(signals_count_new)
		    - TME_BUS_SIGNAL_BIT_BYTES(signals_count_old))));
      }
    }
  }

  /* otherwise, we found an existing bus signals set.  however, even
     though the versions overlap, if they don't support the same least
     version, something is wrong: */
  else if ((TME_X_VERSION_CURRENT(bus->tme_bus_signals[signal_i].tme_bus_signals_version)
	    - TME_X_VERSION_AGE(bus->tme_bus_signals[signal_i].tme_bus_signals_version))
	   != (TME_X_VERSION_CURRENT(bus_signals->tme_bus_signals_version)
	       - TME_X_VERSION_AGE(bus_signals->tme_bus_signals_version))) {
    rc = EINVAL;
  }

  /* otherwise, we found an existing bus signals set that fully matches
     and is compatible with the caller's: */
  else {
    
    /* update the versioning on this bus signals set: */
    if (TME_X_VERSION_CURRENT(bus_signals->tme_bus_signals_version)
	> TME_X_VERSION_CURRENT(bus->tme_bus_signals[signal_i].tme_bus_signals_version)) {
      bus->tme_bus_signals[signal_i].tme_bus_signals_version = bus_signals->tme_bus_signals_version;
    }

    /* return the existing bus signals set: */
    *bus_signals = bus->tme_bus_signals[signal_i];
  }
    
  /* unlock the bus and return: */
  tme_rwlock_unlock(&bus->tme_bus_rwlock);
  return (rc);
}

/* this handles a bus connection signal edge: */
static int
_tme_bus_signal(struct tme_bus_connection *conn_bus_edger, unsigned int signal)
{
  struct tme_bus *bus;
  struct tme_bus_connection_int *conn_bus_int_edger;
  struct tme_bus_connection_int *conn_bus_int;
  unsigned int level_edge;
  struct tme_bus_connection *conn_bus;
  struct tme_bus_connection *conn_bus_other;
  int signal_asserted, need_propagate;
  unsigned int signal_index;
  tme_uint8_t signal_mask;
  int rc;
  int deadlocked;

  /* recover our bus: */
  bus = conn_bus_edger->tme_bus_connection.tme_connection_element->tme_element_private;
  conn_bus_int_edger = (struct tme_bus_connection_int *) conn_bus_edger;

  /* take out the level and edge: */
  level_edge = signal;
  signal = TME_BUS_SIGNAL_WHICH(signal);
  level_edge ^= signal;

  /* lock the bus for writing: */
  rc = tme_rwlock_timedwrlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);
  if (TME_THREADS_ERRNO(rc) != TME_OK) {
    return (TME_THREADS_ERRNO(rc));
  }
  
  /* if this device doesn't know its interrupt signal, fix it: */
  if (signal == TME_BUS_SIGNAL_INT_UNSPEC) {
    signal = conn_bus_int_edger->tme_bus_connection_int_signal_int;
    if (signal == TME_BUS_SIGNAL_INT_UNSPEC) {
      /* this bus connection is misconfigured: */
      if (!conn_bus_int_edger->tme_bus_connection_int_logged_int) {
	conn_bus_int_edger->tme_bus_connection_int_logged_int = TRUE;
	/* XXX diagnostic */
	abort();
      }
      tme_rwlock_unlock(&bus->tme_bus_rwlock);
      return (TME_OK);
    }
  }

  /* assume we don't need to propagate this signal across the bus: */
  need_propagate = FALSE;

  /* see whether the device is asserting or negating this signal.  iff
     one or more devices on a bus are asserting a signal, the signal
     appears asserted on the bus.  this gives an ORed effect: */
  signal_asserted = TRUE;
  switch (level_edge & TME_BUS_SIGNAL_LEVEL_MASK) {
  case TME_BUS_SIGNAL_LEVEL_NEGATED:
    signal_asserted = FALSE;
  case TME_BUS_SIGNAL_LEVEL_ASSERTED:
    break;
  default:
    abort();
  }

  /* get the index and mask of this signal in signal byte arrays: */
  signal_index = TME_BUS_SIGNAL_BIT_INDEX(signal);
  signal_mask = TME_BUS_SIGNAL_BIT_MASK(signal);

  /* if this signal is being asserted: */
  if (signal_asserted) {

    /* if this device wasn't already asserting this signal: */
    if (!(conn_bus_int_edger->tme_bus_connection_int_signals[signal_index]
	  & signal_mask)) {

      /* it is now asserting this signal: */
      conn_bus_int_edger->tme_bus_connection_int_signals[signal_index]
	|= signal_mask;
      bus->tme_bus_signal_asserts[TME_BUS_SIGNAL_INDEX(signal)]++;

      /* if this is the only device asserting this signal,
	 propagate the change across the bus: */
      if (bus->tme_bus_signal_asserts[TME_BUS_SIGNAL_INDEX(signal)] == 1) {
	need_propagate = TRUE;
      }
    }

    /* otherwise, this device was already asserting this signal: */
    else {
      assert(bus->tme_bus_signal_asserts[TME_BUS_SIGNAL_INDEX(signal)] > 0);
    }
  }

  /* otherwise, this signal is being negated: */
  else {

    /* if this device was asserting this signal: */
    if (conn_bus_int_edger->tme_bus_connection_int_signals[signal_index]
	& signal_mask) {

      /* it is no longer asserting this signal: */
      conn_bus_int_edger->tme_bus_connection_int_signals[signal_index]
	&= ~signal_mask;
      assert(bus->tme_bus_signal_asserts[TME_BUS_SIGNAL_INDEX(signal)] > 0);
      bus->tme_bus_signal_asserts[TME_BUS_SIGNAL_INDEX(signal)]--;

      /* if this was the last device asserting this signal, propagate
	 the change across the bus: */
      if (bus->tme_bus_signal_asserts[TME_BUS_SIGNAL_INDEX(signal)] == 0) {
	need_propagate = TRUE;
      }
    }

    /* otherwise, this device was not asserting this signal, but it
       negated it anyways.  often, lazy code will only send negating
       edges for signals (for example, see the do_reset code in
       machine/sun2/sun2-mainbus.c, which should really assert RESET,
       sleep, then negate it), so if we get a negated edge for a
       signal that no one else (including the edger) was asserting, we
       propagate the edge anyways.

       so, TME_BUS_SIGNAL_EDGE should only be used for this purpose: */
    else if ((level_edge & TME_BUS_SIGNAL_EDGE)
	     && bus->tme_bus_signal_asserts[TME_BUS_SIGNAL_INDEX(signal)] == 0) {
      need_propagate = TRUE;
    }
  }

  /* if we're propagating this signal across the bus: */
  rc = TME_OK;
  if (need_propagate) {

    /* put the level and edge back in: */
    signal |= level_edge;

    /* assume that we won't deadlock: */
    deadlocked = FALSE;

    /* propagate the signal to each connection to the bus: */
    for (conn_bus_int = bus->tme_bus_connections;
	 conn_bus_int != NULL;
	 conn_bus_int =
	   (struct tme_bus_connection_int *) 
	   conn_bus_int->tme_bus_connection_int
	   .tme_bus_connection
	   .tme_connection_next) {
      conn_bus = &conn_bus_int->tme_bus_connection_int;
      conn_bus_other = 
	(struct tme_bus_connection *) 
	conn_bus->tme_bus_connection.tme_connection_other;
      
      /* skip this device if it edged the line to begin with: */
      if (conn_bus == conn_bus_edger) {
	continue;
      }

      /* skip this device if it doesn't care about bus signals: */
      if (conn_bus_other->tme_bus_signal == NULL) {
	continue;
      }

      /* give the edge to this connection: */
      rc =  (*conn_bus_other->tme_bus_signal)(conn_bus_other, signal);

      /* if we deadlocked, remember to tell the caller: */
      if (rc == TME_EDEADLK) {
	deadlocked = TRUE;
      }
    }
    rc = (deadlocked ? TME_EDEADLK : TME_OK);
  }

  /* unlock the bus: */
  tme_rwlock_unlock(&bus->tme_bus_rwlock);

  /* done: */
  return (rc);
}
			      
/* this handles a bus interrupt acknowledge: */
static int
_tme_bus_intack(struct tme_bus_connection *conn_bus_acker, unsigned int signal, int *vector)
{
  struct tme_bus *bus;
  struct tme_bus_connection_int *conn_bus_int;
  struct tme_bus_connection *conn_bus;
  struct tme_bus_connection *conn_bus_other;
  unsigned int signal_index;
  tme_uint8_t signal_mask;
  int rc;

  /* recover our bus: */
  bus = conn_bus_acker->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get rid of any level and edge: */
  signal = TME_BUS_SIGNAL_WHICH(signal);

  /* this must be an interrupt signal: */
  assert(TME_BUS_SIGNAL_IS_INT(signal));

  /* lock the bus for writing: */
  rc = tme_rwlock_timedwrlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);
  if (TME_THREADS_ERRNO(rc) != TME_OK) {
    return (TME_THREADS_ERRNO(rc));
  }
  
  /* get the index and mask of this signal in signal byte arrays: */
  signal_index = TME_BUS_SIGNAL_BIT_INDEX(signal);
  signal_mask = TME_BUS_SIGNAL_BIT_MASK(signal);

  /* find the first connection to the bus that is asserting this
     interrupt signal.  if no connection is asserting the signal,
     return ENOENT: */
  rc = ENOENT;
  for (conn_bus_int = bus->tme_bus_connections;
       conn_bus_int != NULL;
       conn_bus_int =
	 (struct tme_bus_connection_int *) 
	 conn_bus_int->tme_bus_connection_int
	 .tme_bus_connection
	 .tme_connection_next) {
    conn_bus = &conn_bus_int->tme_bus_connection_int;
    conn_bus_other = 
      (struct tme_bus_connection *) 
      conn_bus->tme_bus_connection.tme_connection_other;
    
    /* if this device is asserting this interrupt signal: */
    if (conn_bus_int->tme_bus_connection_int_signals[signal_index]
	& signal_mask) {

      /* unlock the bus: */
      tme_rwlock_unlock(&bus->tme_bus_rwlock);

      /* if this device doesn't acknowledge interrupts, return any
	 user-specified vector or TME_BUS_INTERRUPT_VECTOR_UNDEF: */
      if (conn_bus_other->tme_bus_intack == NULL) {
	*vector = conn_bus_int->tme_bus_connection_int_vector_int;
	rc = TME_OK;
      }

      /* otherwise, run the interrupt acknowledge with this connection: */
      else {
	rc = (*conn_bus_other->tme_bus_intack)(conn_bus_other, signal, vector);
      }

      /* done: */
      return (rc);
    }
  }

  /* unlock the bus: */
  tme_rwlock_unlock(&bus->tme_bus_rwlock);

  /* done: */
  return (rc);
}

static int
_tme_bus_fault(void *junk0, struct tme_bus_cycle *junk1)
{
  return (ENOENT);
}

/* this fills a TLB entry: */
static int
_tme_bus_tlb_fill(struct tme_bus_connection *conn_bus_asker, 
		  struct tme_bus_tlb *tlb,
		  tme_bus_addr_t address, 
		  unsigned int cycles)
{
  struct tme_bus *bus;
  struct tme_bus_connection_int *conn_int;
  int rc;

  /* recover our bus and our connection to the asker: */
  bus = conn_bus_asker->tme_bus_connection.tme_connection_element->tme_element_private;
  conn_int = (struct tme_bus_connection_int *) conn_bus_asker;

  /* put our fault handler in the TLB entry: */
  tlb->tme_bus_tlb_cycle_private = NULL;
  tlb->tme_bus_tlb_cycle = _tme_bus_fault;

  /* lock the bus for reading: */
  rc = tme_rwlock_timedrdlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);
  if (TME_THREADS_ERRNO(rc) != TME_OK) {
    return (TME_THREADS_ERRNO(rc));
  }

  /* call the generic bus support function: */
  rc = tme_bus_tlb_fill(bus,
			conn_int,
			tlb, address, cycles);

  /* unlock the bus: */
  tme_rwlock_unlock(&bus->tme_bus_rwlock);

  /* done: */
  return (rc);
}

/* this adds a new TLB set: */
static int
_tme_bus_tlb_set_add(struct tme_bus_connection *conn_bus_asker,
		     struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_bus *bus;
  struct tme_bus_connection_int *conn_int;
  int rc;

  /* recover our bus and our connection to the asker: */
  bus = conn_bus_asker->tme_bus_connection.tme_connection_element->tme_element_private;
  conn_int = (struct tme_bus_connection_int *) conn_bus_asker;

  /* lock the bus for reading: */
  rc = tme_rwlock_timedrdlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);
  if (TME_THREADS_ERRNO(rc) != TME_OK) {
    return (TME_THREADS_ERRNO(rc));
  }

  /* call the generic bus support function: */
  rc = tme_bus_tlb_set_add(bus,
			   conn_int,
			   tlb_set_info);

  /* unlock the bus: */
  tme_rwlock_unlock(&bus->tme_bus_rwlock);

  /* done: */
  return (rc);
}

/* this scores a new connection: */
static int
_tme_bus_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_bus *bus;
  struct tme_bus_connection_int *conn_int;
  int rc, ok;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_BUS_GENERIC);

  /* recover our bus and our internal connection side: */
  bus = conn->tme_connection_element->tme_element_private;
  conn_int = (struct tme_bus_connection_int *) conn;

  /* lock the bus for reading: */
  rc = tme_rwlock_timedrdlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);
  if (TME_THREADS_ERRNO(rc) != TME_OK) {
    return (TME_THREADS_ERRNO(rc));
  }
  
  /* call the generic bus support function: */
  ok = tme_bus_connection_ok(bus,
			     conn_int);

  /* unlock the bus: */
  tme_rwlock_unlock(&bus->tme_bus_rwlock);

  /* return the score: */
  *_score = (ok ? 1 : 0);
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_bus_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_bus *bus;
  struct tme_bus_connection_int *conn_int;
  const struct tme_bus_signals *bus_signals;
  int rc;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_BUS_GENERIC);

  /* recover our bus and our internal connection side: */
  bus = conn->tme_connection_element->tme_element_private;
  conn_int = (struct tme_bus_connection_int *) conn;
  
  /* lock the bus for writing: */
  rc = tme_rwlock_timedwrlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);
  if (TME_THREADS_ERRNO(rc) != TME_OK) {
    return (TME_THREADS_ERRNO(rc));
  }

  /* call the generic bus support function: */
  rc = tme_bus_connection_make(bus,
			       conn_int,
			       state);

  /* XXX this is a perfect example of the poor division between
     bus-el.c and bus.c.  should the signal handling code be in bus.c?  */
  if (rc == TME_OK) {
    bus_signals = &bus->tme_bus_signals[bus->tme_bus_signals_count - 1];
    conn_int->tme_bus_connection_int_signals
      = tme_new0(tme_uint8_t,
		 TME_BUS_SIGNAL_BIT_BYTES(TME_BUS_SIGNAL_INDEX(bus_signals->tme_bus_signals_first)
					  + bus_signals->tme_bus_signals_count));
  }

  /* unlock the bus: */
  tme_rwlock_unlock(&bus->tme_bus_rwlock);

  return (rc);
}

/* this breaks a connection: */
static int 
_tme_bus_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this returns the new connections possible: */
static int
_tme_bus_connections_new(struct tme_element *element,
			 const char * const *args,
			 struct tme_connection **_conns,
			 char **_output)
{
  const struct tme_bus *bus;
  struct tme_bus_connection_int *conn_int;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  int ipl;
  int vector;
  const struct tme_bus_slot *bus_slot;
  int arg_i;
  int usage;

  /* recover our bus.  we only read the address mask, so we don't lock
     the rwlock: */
  bus = element->tme_element_private;

  /* allocate the new connection side: */
  conn_int = tme_new0(struct tme_bus_connection_int, 1);
  conn_bus = &conn_int->tme_bus_connection_int;
  conn = &conn_bus->tme_bus_connection;

  /* loop reading our arguments: */
  usage = FALSE;
  arg_i = 1;
  conn_int->tme_bus_connection_int_vector_int = TME_BUS_INTERRUPT_VECTOR_UNDEF;
  bus_slot = NULL;
  for (;;) {

    /* the address of this connection: */
    if (TME_ARG_IS(args[arg_i + 0], "addr")) {
      conn_int->tme_bus_connection_int_flags |= TME_BUS_CONNECTION_INT_FLAG_ADDRESSABLE;
      conn_int->tme_bus_connection_int_address = tme_bus_addr_parse_any(args[arg_i + 1], &usage);
      if (usage
	  || (conn_int->tme_bus_connection_int_address
	      > bus->tme_bus_address_mask)) {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* the interrupt signal for this connection: */
    else if (TME_ARG_IS(args[arg_i + 0], "ipl")
	     && args[arg_i + 1] != NULL
	     && (ipl = atoi(args[arg_i + 1])) > 0) {
      conn_int->tme_bus_connection_int_signal_int = TME_BUS_SIGNAL_INT(ipl);
      arg_i += 2;
    }

    /* the interrupt vector for this connection: */
    else if (TME_ARG_IS(args[arg_i + 0], "vector")
	     && args[arg_i + 1] != NULL
	     && (vector = atoi(args[arg_i + 1])) > 0) {
      conn_int->tme_bus_connection_int_vector_int = vector;
      arg_i += 2;
    }      

    /* the slot for this connection: */
    else if (TME_ARG_IS(args[arg_i + 0], "slot")
	     && args[arg_i + 1] != NULL) {

      /* you can't give more than one slot for a connection: */
      if (bus_slot != NULL) {
	tme_output_append_error(_output,
				"slot %s %s, ",
				args[arg_i + 1],
				_("redefined"));
	usage = TRUE;
	break;
      }

      /* make sure this slot has been defined: */
      for (bus_slot = bus->tme_bus_slots;
	   bus_slot != NULL;
	   bus_slot = bus_slot->tme_bus_slot_next) {
	if (strcmp(bus_slot->tme_bus_slot_name,
		   args[arg_i + 1]) == 0) {
	  break;
	}
      }
      if (bus_slot == NULL) {
	tme_output_append_error(_output,
				"slot %s %s, ",
				args[arg_i + 1],
				_("unknown"));
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* the slot offset for this connection: */
    else if (TME_ARG_IS(args[arg_i + 0], "offset")) {
      if (bus_slot == NULL) {
	tme_output_append_error(_output,
				"slot %s, ",
				_("unknown"));
	usage = TRUE;
	break;
      }
      conn_int->tme_bus_connection_int_flags |= TME_BUS_CONNECTION_INT_FLAG_ADDRESSABLE;
      conn_int->tme_bus_connection_int_address
	= (bus_slot->tme_bus_slot_address
	   + tme_bus_addr_parse_any(args[arg_i + 1], &usage));
      if (usage
	  || (conn_int->tme_bus_connection_int_address
	      > bus->tme_bus_address_mask)) {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* if this connection is for a slot controller: */
    else if (TME_ARG_IS(args[arg_i + 0], "controller")) {
      if (bus->tme_bus_controller != NULL) {
	tme_free(conn_int);
	return (EEXIST);
      }
      conn_int->tme_bus_connection_int_flags |= TME_BUS_CONNECTION_INT_FLAG_CONTROLLER;
      arg_i++;
    }

    /* if this connection has an automatic DMA offset: */
    else if (TME_ARG_IS(args[arg_i + 0], "dma-offset")) {
      conn_int->tme_bus_connection_int_sourced = tme_bus_addr_parse_any(args[arg_i + 1], &usage);
      if (usage
	  || (conn_int->tme_bus_connection_int_sourced
	      > bus->tme_bus_address_mask)) {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {
      break;
    }

    /* this is a bad argument: */
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
			    "%s %s [ controller ] [ addr %s ] [ slot %s offset %s ] [ dma-offset %s ] [ ipl %s ] [ vector %s ]",
			    _("usage:"),
			    args[0],
			    _("BUS-ADDRESS"),
			    _("SLOT"),
			    _("OFFSET"),
			    _("OFFSET"),
			    _("INTERRUPT-LEVEL"),
			    _("INTERRUPT-VECTOR"));
    tme_free(conn_int);
    return (EINVAL);
  }

  /* fill in the bus connection: */
  conn_bus->tme_bus_subregions.tme_bus_subregion_address_first
    = 0;
  conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
    = bus->tme_bus_address_mask;
  conn_bus->tme_bus_subregions.tme_bus_subregion_next
    = NULL;
  conn_bus->tme_bus_signals_add = _tme_bus_signals_add;
  conn_bus->tme_bus_signal = _tme_bus_signal;
  conn_bus->tme_bus_intack = _tme_bus_intack;
  conn_bus->tme_bus_tlb_set_add = _tme_bus_tlb_set_add;
  conn_bus->tme_bus_tlb_fill = _tme_bus_tlb_fill;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_bus_connection_score;
  conn->tme_connection_make = _tme_bus_connection_make;
  conn->tme_connection_break = _tme_bus_connection_break;
  
  /* return the new connection side: */
  *_conns = conn;
  return (TME_OK);
}

/* this creates a new bus element: */
TME_ELEMENT_SUB_NEW_DECL(tme_generic,bus) {
  struct tme_bus *bus;
  tme_bus_addr_t bus_size_mask;
  tme_bus_addr_t bus_slot_size;
  tme_bus_addr_t bus_slot_addr;
  int bus_slot_addr_defined;
  struct tme_bus_slot *bus_slot;
  struct tme_bus_slot *bus_slots;
  int arg_i;
  int failed;

  /* our arguments must include the bus size, and the
     bus size must be a power of two: */
  failed = FALSE;
  arg_i = 1;
  bus_size_mask = 0;
  bus_slot_size = 0;
  bus_slot_addr_defined = FALSE;
  bus_slot_addr = 0;
  bus_slots = NULL;
  for (; !failed; ) {

    /* the bus size: */
    if (TME_ARG_IS(args[arg_i + 0], "size")) {
      /* XXX FIXME - this is a hack: */
      if (sizeof(bus_size_mask) == sizeof(tme_uint32_t) &&
	  TME_ARG_IS(args[arg_i + 1], "4GB")) {
	bus_size_mask = ((tme_bus_addr_t) 0) - 1;
      }
      else {
	bus_size_mask = tme_bus_addr_parse_any(args[arg_i + 1], &failed);
	if (!failed
	    && bus_size_mask < 2) {
	  failed = TRUE;
	}
	else {
	  bus_size_mask -= 1;
	}
      }
      if (bus_size_mask & (bus_size_mask + 1)) {
	failed = TRUE;
      }
      arg_i += 2;
    }

    /* the address for the next slots: */
    else if (TME_ARG_IS(args[arg_i + 0], "slot-addr")) {
      bus_slot_addr = tme_bus_addr_parse_any(args[arg_i + 1], &failed);
      bus_slot_addr_defined = TRUE;
      arg_i += 2;
    }

    /* the size for the next slots: */
    else if (TME_ARG_IS(args[arg_i + 0], "slot-size")) {
      bus_slot_size = tme_bus_addr_parse_any(args[arg_i + 1], &failed);
      if (bus_slot_size < 1) {
	failed = TRUE;
      }
      arg_i += 2;
    }

    /* a slot definition: */
    else if (TME_ARG_IS(args[arg_i + 0], "slot")) {
      if (args[arg_i + 1] == NULL) {
	failed = TRUE;
	break;
      }
      if (!bus_slot_addr_defined) {
	failed = TRUE;
	break;
      }
      if (bus_slot_size == 0) {
	failed = TRUE;
	break;
      }

      /* make sure this slot hasn't already been defined: */
      for (bus_slot = bus_slots;
	   bus_slot != NULL;
	   bus_slot = bus_slot->tme_bus_slot_next) {
	if (strcmp(bus_slot->tme_bus_slot_name,
		   args[arg_i + 1]) == 0) {
	  tme_output_append_error(_output,
				  "slot %s %s",
				  args[arg_i + 1],
				  _("redefined"));
	  failed = TRUE;
	  break;
	}
      }
      if (failed) {
	break;
      }

      /* add this slot: */
      bus_slot = tme_new0(struct tme_bus_slot, 1);
      bus_slot->tme_bus_slot_next = bus_slots;
      bus_slots = bus_slot;
      bus_slot->tme_bus_slot_name = tme_strdup(args[arg_i + 1]);
      bus_slot->tme_bus_slot_address = bus_slot_addr;
      bus_slot->tme_bus_slot_size = bus_slot_size;

      /* advance for the next slot: */
      bus_slot_addr += bus_slot_size;
      arg_i += 2;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {
      break;
    }

    /* an unknown argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s, ",
			      args[arg_i],
			      _("unexpected"));
      failed = TRUE;
    }
  }
  if (failed) {
    tme_output_append_error(_output,
			    "%s %s size %s [ slot-addr %s slot-size %s slot %s0 .. slot %sN ]",
			    _("usage:"),
			    args[0],
			    _("SIZE"),
			    _("ADDRESS"),
			    _("SIZE"),
			    _("SLOT-NAME"),
			    _("SLOT-NAME"));
    for (; (bus_slot = bus_slots) != NULL; ) {
      bus_slots = bus_slots->tme_bus_slot_next;
      tme_free(bus_slot->tme_bus_slot_name);
      tme_free(bus_slot);
    }
    return (EINVAL);
  }

  /* allocate and initialize the new bus: */
  bus = tme_new0(struct tme_bus, 1);
  tme_rwlock_init(&bus->tme_bus_rwlock);
  bus->tme_bus_address_mask = bus_size_mask;
  bus->tme_bus_addressables_count = 0;
  bus->tme_bus_addressables_size = 1;
  bus->tme_bus_addressables = tme_new(struct tme_bus_addressable,
				      bus->tme_bus_addressables_size);
  bus->tme_bus_signals_count = TME_ARRAY_ELS(_tme_bus_signals_default);
  bus->tme_bus_signals = tme_dup(struct tme_bus_signals,
				 _tme_bus_signals_default,
				 TME_ARRAY_ELS(_tme_bus_signals_default));
  bus->tme_bus_signal_asserts = tme_new0(unsigned int,
					 _tme_bus_signals_default[0].tme_bus_signals_count);
  bus->tme_bus_slots = bus_slots;

  /* fill the element: */
  element->tme_element_private = bus;
  element->tme_element_connections_new = _tme_bus_connections_new;

  return (TME_OK);
}
