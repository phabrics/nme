/* $Id: bus.c,v 1.14 2009/08/29 17:41:17 fredette Exp $ */

/* generic/gen-bus.c - generic bus support: */

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
_TME_RCSID("$Id: bus.c,v 1.14 2009/08/29 17:41:17 fredette Exp $");

/* includes: */
#include <tme/generic/bus.h>
#include <tme/misc.h>
#include <stdlib.h>
#include <string.h>

/* this does a binary search of the addressable connections: */
int
tme_bus_address_search(struct tme_bus *bus, tme_bus_addr_t address)
{
  int left, right, pivot;
  struct tme_bus_connection_int *conn_int;
  const struct tme_bus_subregion *subregion;

  /* initialize for the search: */
  left = 0;
  right = bus->tme_bus_addressables_count - 1;
  
  /* do the search: */
  pivot = 0;
  for (; left <= right; ) {

    /* get the pivot: */
    pivot = (left + right) / 2;
    conn_int = bus->tme_bus_addressables[pivot].tme_bus_addressable_connection;
    subregion = bus->tme_bus_addressables[pivot].tme_bus_addressable_subregion;

    /* if we have to move left: */
    if (address
	< (conn_int->tme_bus_connection_int_address
	   + subregion->tme_bus_subregion_address_first)) {
      /* if we're done searching, pivot is already the index of the
	 first element we need to shift to the right in order to
	 insert a new element: */
      right = pivot - 1;
    }

    /* if we have to move right: */
    else if (address
	     > (conn_int->tme_bus_connection_int_address
		+ subregion->tme_bus_subregion_address_last)) {
      /* if we're done searching, pivot + 1 is the index of the
	 first element we need to shift to the right in order to
	 insert a new element: */
      left = ++pivot;
    }

    /* we found the addressable: */
    else {
      return (pivot);
    }
  }

  /* we failed to find an addressable that covers the address: */
  return (-1 - pivot);
}

/* this fills a TLB entry: */
int
tme_bus_tlb_fill(struct tme_bus *bus,
		 struct tme_bus_connection_int *conn_int_asker,
		 struct tme_bus_tlb *tlb,
		 tme_bus_addr_t address, 
		 unsigned int cycles)
{
  int pivot;
  struct tme_bus_connection_int *conn_int;
  const struct tme_bus_subregion *subregion;
  struct tme_bus_connection *conn_bus_other;
  tme_bus_addr_t sourced_address_mask, conn_address;
  tme_bus_addr_t hole_first, hole_last;
  struct tme_bus_tlb tlb_bus;
  void *cycle_fault_private;
  tme_bus_cycle_handler cycle_fault;
  int rc;

  /* get the sourced address mask: */
  sourced_address_mask = conn_int_asker->tme_bus_connection_int_sourced;

  /* get the asked address on the bus: */
  conn_address = (sourced_address_mask | address);

  /* start the mapping TLB entry: */
  tlb_bus.tme_bus_tlb_addr_first = 0;
  tlb_bus.tme_bus_tlb_addr_last = TME_MIN(((sourced_address_mask
					    | (sourced_address_mask - 1))
					   ^ sourced_address_mask),
					  bus->tme_bus_address_mask);
  tlb_bus.tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* if this bus has a controller, and this request isn't coming from
     the controller: */
  conn_int = bus->tme_bus_controller;
  if (conn_int != NULL
      && conn_int != conn_int_asker) {

    /* get the controller's connection: */
    conn_bus_other = 
      (struct tme_bus_connection *) conn_int->tme_bus_connection_int.tme_bus_connection.tme_connection_other;

    /* unlock the bus: */
    tme_rwlock_unlock(&bus->tme_bus_rwlock);

    /* call the controller's TLB fill function: */
    rc = (*conn_bus_other->tme_bus_tlb_fill)(conn_bus_other, tlb,
					     conn_address, cycles);

    /* relock the bus: */
    /* XXX FIXME - we assume that this succeeds: */
    (void) tme_rwlock_timedrdlock(&bus->tme_bus_rwlock, TME_THREAD_TIMEDLOCK);

    /* if the TLB fill succeeded: */
    if (rc == TME_OK) {

      /* map the filled TLB entry: */
      tme_bus_tlb_map(tlb, conn_address, &tlb_bus, address);
    }

    return (rc);
  }

  /* search for this address on the bus: */
  pivot = tme_bus_address_search(bus, conn_address);

  /* if this address doesn't exist: */
  if (pivot < 0) {

    /* save the bus' fault cycle handler: */
    cycle_fault_private = tlb->tme_bus_tlb_cycle_private;
    cycle_fault = tlb->tme_bus_tlb_cycle;

    /* initialize the TLB entry: */
    tme_bus_tlb_initialize(tlb);

    /* this TLB entry can cover the entire hole in the address space: */
    pivot = -1 - pivot;
    hole_first = (pivot == 0
		  ? 0
		  : ((bus->tme_bus_addressables[pivot - 1]
		      .tme_bus_addressable_connection->tme_bus_connection_int_address)
		     + (bus->tme_bus_addressables[pivot - 1]
			.tme_bus_addressable_subregion->tme_bus_subregion_address_last)
		     + 1));
    hole_last = (pivot == bus->tme_bus_addressables_count
		 ? bus->tme_bus_address_mask
		 : ((bus->tme_bus_addressables[pivot]
		     .tme_bus_addressable_connection->tme_bus_connection_int_address)
		    - 1));
    tlb->tme_bus_tlb_addr_first = hole_first;
    tlb->tme_bus_tlb_addr_last = hole_last;

    /* reads and writes are allowed: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

    /* reads and writes in this region always fault: */
    tlb->tme_bus_tlb_cycle_private = cycle_fault_private;
    tlb->tme_bus_tlb_cycle = cycle_fault;
    rc = TME_OK;
  }

  /* otherwise, this address does exist: */
  else {
    conn_int = bus->tme_bus_addressables[pivot].tme_bus_addressable_connection;
    subregion = bus->tme_bus_addressables[pivot].tme_bus_addressable_subregion;
    conn_bus_other = 
      (struct tme_bus_connection *) conn_int->tme_bus_connection_int.tme_bus_connection.tme_connection_other;

    /* call the TLB fill function for the connection: */
    conn_address -= conn_int->tme_bus_connection_int_address;
    rc = (*conn_bus_other->tme_bus_tlb_fill)(conn_bus_other, tlb,
					     conn_address, cycles);

    /* if that succeeded: */
    if (rc == TME_OK) {
      
      /* create the mapping TLB entry: */
      tlb_bus.tme_bus_tlb_addr_first =
	(TME_MAX((conn_int->tme_bus_connection_int_address
		  + subregion->tme_bus_subregion_address_first),
		 (sourced_address_mask
		  | tlb_bus.tme_bus_tlb_addr_first))
	 - sourced_address_mask);
      tlb_bus.tme_bus_tlb_addr_last = 
	(TME_MIN((conn_int->tme_bus_connection_int_address
		  + subregion->tme_bus_subregion_address_last),
		 (sourced_address_mask
		  | tlb_bus.tme_bus_tlb_addr_last))
	 - sourced_address_mask);
    }
  }

  /* if the TLB fill succeeded: */
  if (rc == TME_OK) {

    /* map the filled TLB entry: */
    tme_bus_tlb_map(tlb, conn_address, &tlb_bus, address);
  }

  /* done: */
  return (rc);
}

/* this adds a new TLB set: */
int
tme_bus_tlb_set_add(struct tme_bus *bus,
		    struct tme_bus_connection_int *conn_int_asker,
		    struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_bus_connection *conn_bus_other, *conn_bus_dma;
  int conn_int_i;
  int rc;

  /* at most one of our addressable connections may provide a TLB set
     add function.  generally, this means that connection is a
     DMA-controller-like connection to the bus, where it may need to
     invalidate at any later time the TLBs it fills out, due to sudden
     changes in how the DMA region on the bus is mapped: */
  conn_bus_dma = NULL;
  for (conn_int_i = 0;
       conn_int_i < bus->tme_bus_addressables_count;
       conn_int_i++) {
    conn_bus_other = 
      ((struct tme_bus_connection *)
       bus->tme_bus_addressables[conn_int_i].tme_bus_addressable_connection
       ->tme_bus_connection_int.tme_bus_connection.tme_connection_other);

    /* if this bus connection offers a TLB set add function, it is
       a DMA-controller-like connection to the bus: */
    if (conn_bus_other->tme_bus_tlb_set_add != NULL) {

      /* if there is more than one of these, it is likely a
	 configuration error.  if we had some way of specifying which
	 of several DMA regions a given connection will always use, we
	 could avoid this: */
      if (conn_bus_dma != NULL) {
	abort();
      }

      conn_bus_dma = conn_bus_other;
    }
  }

  /* if there is a DMA-controller-like connection to the bus, 
     let it add the TLB set: */
  if (conn_bus_dma != NULL) {
    rc = (*conn_bus_dma->tme_bus_tlb_set_add)
      (conn_bus_dma, tlb_set_info);
  }

  /* otherwise, handle the add ourselves: */
  else {
    
    /* if this TLB set provides a bus context register: */
    if (tlb_set_info->tme_bus_tlb_set_info_bus_context != NULL) {

      /* this bus only has one context: */
      (*tlb_set_info->tme_bus_tlb_set_info_bus_context) = 0;
      tlb_set_info->tme_bus_tlb_set_info_bus_context_max = 0;
    }
    rc = TME_OK;
  }
      
  /* done: */
  return (rc);
}

/* this invalidates a TLB set: */
void
tme_bus_tlb_set_invalidate(const struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_token *token;
  unsigned long token_count;

  /* invalidate the tokens in the TLB set: */
  token = tlb_set_info->tme_bus_tlb_set_info_token0;
  token_count = tlb_set_info->tme_bus_tlb_set_info_token_count;
  do {
    tme_token_invalidate(token);
    token = (struct tme_token *) (((tme_uint8_t *) token) + tlb_set_info->tme_bus_tlb_set_info_token_stride);
  } while (--token_count > 0);
}

/* this returns nonzero if the connection's address space is available: */
int
tme_bus_connection_ok(struct tme_bus *bus,
		      struct tme_bus_connection_int *conn_int)
{
  const struct tme_bus_subregion *subregion;
  const struct tme_bus_connection *conn_bus_other;
  int pivot_start, pivot_end;

  /* if this connection isn't addressable, it's always OK: */
  if (!(conn_int->tme_bus_connection_int_flags
	& TME_BUS_CONNECTION_INT_FLAG_ADDRESSABLE)) {
    return (TRUE);
  }

  /* all subregions of this connection must fit on the bus,
     and they must not overlap with any other subregion on 
     any other existing connection: */
  /* XXX we should also check that the connection's subregions don't
     overlap with each other: */
  conn_bus_other
    = ((struct tme_bus_connection *)
       conn_int->tme_bus_connection_int.tme_bus_connection.tme_connection_other);
  for (subregion = &conn_bus_other->tme_bus_subregions;
       subregion != NULL;
       subregion = subregion->tme_bus_subregion_next) {

    /* the subregion's last address cannot be less than
       the first address: */
    if (subregion->tme_bus_subregion_address_last
	< subregion->tme_bus_subregion_address_first) {
      return (FALSE);
    }
    
    /* this subregion must fit on the bus: */
    if (subregion->tme_bus_subregion_address_last >
	(bus->tme_bus_address_mask
	 - conn_int->tme_bus_connection_int_address)) {
      return (FALSE);
    }

    /* search for anything covering the start or end of the new
       addressable subregion: */
    pivot_start = 
      tme_bus_address_search(bus, 
			     (conn_int->tme_bus_connection_int_address
			      + subregion->tme_bus_subregion_address_first));
    pivot_end =
      tme_bus_address_search(bus,
			     (conn_int->tme_bus_connection_int_address
			      + subregion->tme_bus_subregion_address_last));

    /* both searches must have failed, and they must have stopped at the
       same point in the sorted addressables, further indicating that no
       addressable exists anywhere *between* the start and end of the
       new addressable, either.  otherwise, this connection fails: */
    if (pivot_start >= 0
	|| pivot_end >= 0
	|| pivot_start != pivot_end) {
      return (FALSE);
    }
  }

  /* this connection's address space is available: */
  return (TRUE);
}

/* this makes a new connection: */
int
tme_bus_connection_make(struct tme_bus *bus,
			struct tme_bus_connection_int *conn_int,
			unsigned int state)
{
  const struct tme_bus_connection *conn_bus_other;
  const struct tme_bus_subregion *subregion;
  int pivot;

  /* if this connection is not full, return now: */
  if (state == TME_CONNECTION_HALF) {
    return (TME_OK);
  }

  /* if this connection is to a bus controller: */
  if (conn_int->tme_bus_connection_int_flags
      & TME_BUS_CONNECTION_INT_FLAG_CONTROLLER) {

    /* if this bus already has a controller: */
    if (bus->tme_bus_controller != NULL) {

      /* we can't make this connection: */
      return (EEXIST);
    }

    /* this connection is to the bus controller: */
    bus->tme_bus_controller = conn_int;
  }

  /* add this connection to our list: */
  conn_int->tme_bus_connection_int.tme_bus_connection.tme_connection_next
    = (struct tme_connection *) bus->tme_bus_connections;
  bus->tme_bus_connections = conn_int;

  /* if this connection is addressable, and this is connection is now
     fully made, add it to our list of addressables: */
  if ((conn_int->tme_bus_connection_int_flags
       & TME_BUS_CONNECTION_INT_FLAG_ADDRESSABLE)
      && state == TME_CONNECTION_FULL) {
    
    /* add all subregions of this connection as addressables: */
    conn_int->tme_bus_connection_int_address_last = 0;
    conn_bus_other
      = ((struct tme_bus_connection *)
	 conn_int->tme_bus_connection_int.tme_bus_connection.tme_connection_other);
    for (subregion = &conn_bus_other->tme_bus_subregions;
	 subregion != NULL;
	 subregion = subregion->tme_bus_subregion_next) {

      /* search for the place to insert this new addressable: */
      pivot = tme_bus_address_search(bus, 
				     (conn_int->tme_bus_connection_int_address
				      + subregion->tme_bus_subregion_address_first));
      assert(pivot < 0);
      pivot = -1 - pivot;
    
      /* if we have to, grow the addressable array: */
      if (bus->tme_bus_addressables_count
	  == bus->tme_bus_addressables_size) {
	bus->tme_bus_addressables_size += (bus->tme_bus_addressables_size >> 1) + 1;
	bus->tme_bus_addressables = tme_renew(struct tme_bus_addressable,
					      bus->tme_bus_addressables,
					      bus->tme_bus_addressables_size);
      }

      /* move all of the later addressables down: */
      memmove(&bus->tme_bus_addressables[pivot + 1],
	      &bus->tme_bus_addressables[pivot],
	      sizeof(bus->tme_bus_addressables[pivot])
	      * (bus->tme_bus_addressables_count
		 - pivot));

      /* insert this new addressable: */
      bus->tme_bus_addressables[pivot].tme_bus_addressable_connection = conn_int;
      bus->tme_bus_addressables[pivot].tme_bus_addressable_subregion = subregion;
      bus->tme_bus_addressables_count++;

      /* update the last address on this connection.  NB that the
	 subregion information should be used almost always.
	 currently this value is only used as the width of the
	 connection for the purposes of determining TLB entry limits
	 when the connection itself asks to fill a TLB entry: */
      conn_int->tme_bus_connection_int_address_last
	= TME_MAX(conn_int->tme_bus_connection_int_address_last,
		  subregion->tme_bus_subregion_address_last);
    }
  }

  return (TME_OK);
}

/* this breaks a connection: */
int
tme_bus_connection_break(struct tme_bus *bus,
			 struct tme_bus_connection_int *conn_int,
			 unsigned int state)
{
  abort();
}

/* this map the first bus TLB entry to be valid on another bus, according to
   the information in the second bus TLB entry: */
void
tme_bus_tlb_map(struct tme_bus_tlb *tlb0, tme_bus_addr_t addr0, 
		const struct tme_bus_tlb *tlb1, tme_bus_addr_t addr1)
{
  tme_bus_addr_t extra_before0, extra_after0;
  tme_bus_addr_t extra_before1, extra_after1;
  tme_bus_addr_t addr_offset;
  unsigned int cycles_ok;

  /* get the address offset: */
  addr_offset = addr1;
  addr_offset -= addr0;

  /* intersect the amount of bus address space covered: */
  extra_before0 = addr0 - tlb0->tme_bus_tlb_addr_first;
  extra_after0 = tlb0->tme_bus_tlb_addr_last - addr0;
  extra_before1 = addr1 - tlb1->tme_bus_tlb_addr_first;
  extra_after1 = tlb1->tme_bus_tlb_addr_last - addr1;
  tlb0->tme_bus_tlb_addr_first = addr1 - TME_MIN(extra_before0, extra_before1);
  tlb0->tme_bus_tlb_addr_last = addr1 + TME_MIN(extra_after0, extra_after1);

  /* intersect the kinds of bus cycles allowed: */
  cycles_ok = (tlb0->tme_bus_tlb_cycles_ok &= tlb1->tme_bus_tlb_cycles_ok);
  if (!(cycles_ok & TME_BUS_CYCLE_READ)) {
    tlb0->tme_bus_tlb_emulator_off_read = TME_EMULATOR_OFF_UNDEF;
  }
  else if (tlb0->tme_bus_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF) {
    tlb0->tme_bus_tlb_emulator_off_read -= addr_offset;
  }
  if (!(cycles_ok & TME_BUS_CYCLE_WRITE)) {
    tlb0->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
  }
  else if (tlb0->tme_bus_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF) {
    tlb0->tme_bus_tlb_emulator_off_write -= addr_offset;
  }

  /* update the address shift for the cycle handler: */
  tlb0->tme_bus_tlb_addr_offset -= addr_offset;
}

/* this initializes a bus TLB entry: */
void
tme_bus_tlb_initialize(struct tme_bus_tlb *tlb)
{
  
  /* make the first address covered all-bits-one: */
  tlb->tme_bus_tlb_addr_first = (((tme_bus_addr_t) 0) - 1);
  
  /* make the last address covered all-bits-zero: */
  tlb->tme_bus_tlb_addr_last = 0;

  /* no fast (memory) transfers allowed: */
  tlb->tme_bus_tlb_emulator_off_read = TME_EMULATOR_OFF_UNDEF;
  tlb->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
  tlb->tme_bus_tlb_rwlock = NULL;

  /* no bus cycles allowed: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_UNDEF;

  /* no address offset or shift: */
  tlb->tme_bus_tlb_addr_offset = 0;
  tlb->tme_bus_tlb_addr_shift = 0;

  /* not cacheable: */
  tlb->tme_bus_tlb_cacheable = NULL;

  /* no bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = NULL;
  tlb->tme_bus_tlb_cycle = NULL;

  /* no bus fault handlers: */
  tlb->tme_bus_tlb_fault_handler_count = 0;
}

/* this calls a TLB entry's fault handlers: */
int
tme_bus_tlb_fault(struct tme_bus_tlb *tlb, struct tme_bus_cycle *cycle, int rc)
{
  unsigned int i;

  /* call all of the fault handlers: */
  for (i = 0; i < tlb->tme_bus_tlb_fault_handler_count; i++) {
    rc = ((*tlb->tme_bus_tlb_fault_handlers[i].tme_bus_tlb_fault_handler)
	  (tlb->tme_bus_tlb_fault_handlers[i].tme_bus_tlb_fault_handler_private,
	   tlb, cycle, rc));
  }

  return (rc);
}

/* this parses any bus address: */
tme_bus_addr_t
tme_bus_addr_parse_any(const char *address_string, int *_failed)
{
  return (tme_misc_unumber_parse_any(address_string, _failed));
}

/* this parses a bus address that has a restricted range: */
tme_bus_addr_t
tme_bus_addr_parse(const char *address_string, tme_bus_addr_t failure_value)
{
  int failed;
  tme_bus_addr_t address;
  address = tme_bus_addr_parse_any(address_string, &failed);
  return (failed ? failure_value : address);
}

/* this transfers bytes between the two participants in a bus cycle: */
void
tme_bus_cycle_xfer(struct tme_bus_cycle *cycle_init, struct tme_bus_cycle *cycle_resp)
{
  struct tme_bus_cycle *cycle_reader, *cycle_writer;
  int buffer_increment_mask_reader, buffer_increment_mask_writer;
  int port_size_reader, port_size_writer;
  int port_overlap_lane_least, port_overlap_size, port_overlap_size_lg2;
  int lane, lane_end;
  int lane_reader, lane_writer;
  int lane_in_reader, lane_in_writer;
  int lane_routing_offset_reader, lane_routing_offset_writer;
  tme_bus_lane_t lane_routing_reader, lane_routing_writer;
  tme_uint8_t lane_value;
  int warn_on_lane;
  unsigned int cycle_size_reader, cycle_size_writer;

  /* sort the initiator and responder into bus reader and bus writer: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {
    assert(cycle_resp->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE);
    cycle_reader = cycle_init;
    cycle_writer = cycle_resp;
  }
  else {
    assert(cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE);
    assert(cycle_resp->tme_bus_cycle_type == TME_BUS_CYCLE_READ);
    cycle_reader = cycle_resp;
    cycle_writer = cycle_init;
  }

  /* get the increment masks for the reader and writer.  since
     tme_bus_cycle_buffer_increment is always 1 or -1, this mask is
     used to negate values without multiplication: */
  if (cycle_reader->tme_bus_cycle_buffer_increment == -1) {
    buffer_increment_mask_reader = -1;
  }
  else {
    assert(cycle_reader->tme_bus_cycle_buffer_increment == 1);
    buffer_increment_mask_reader = 0;
  }
  if (cycle_writer->tme_bus_cycle_buffer_increment == -1) {
    buffer_increment_mask_writer = -1;
  }
  else {
    assert(cycle_writer->tme_bus_cycle_buffer_increment == 1);
    buffer_increment_mask_writer = 0;
  }
#define _TME_BUS_CYCLE_BUFFER_MULTIPLY(value, mask) \
  (((value) ^ (mask)) + ((mask) & 1))

  /* get the sizes, in bytes, of the reader and writer ports: */
  port_size_reader = (1 << TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_reader->tme_bus_cycle_port));
  port_size_writer = (1 << TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_writer->tme_bus_cycle_port));

  /* determine how the writer's port and the reader's port overlap: */
  port_overlap_size = port_size_writer;
  port_overlap_lane_least = TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_writer->tme_bus_cycle_port);
  lane = TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_reader->tme_bus_cycle_port);
  if (port_overlap_lane_least < lane) {
    port_overlap_size -= (lane - port_overlap_lane_least);
    port_overlap_lane_least = lane;
  }
  lane += port_size_reader;
  if ((port_overlap_lane_least + port_overlap_size) > lane) {
    port_overlap_size -= (lane - (port_overlap_lane_least + port_overlap_size));
  }
  assert(port_overlap_size > 0);
  for (port_overlap_size_lg2 = 0;
       (port_overlap_size >>= 1) != 0;
       port_overlap_size_lg2++);

  /* select the reader's lane routing: */
  lane_routing_offset_reader =
    TME_BUS_ROUTER_INDEX(TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_reader->tme_bus_cycle_port),
			 port_overlap_size_lg2,
			 port_overlap_lane_least
			 - TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_reader->tme_bus_cycle_port));

  /* select the writer's lane routing: */
  lane_routing_offset_writer =
    TME_BUS_ROUTER_INDEX(TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_writer->tme_bus_cycle_port),
			 port_overlap_size_lg2,
			 port_overlap_lane_least
			 - TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_writer->tme_bus_cycle_port));

  /* loop over all byte lanes in one or both ports: */
  lane = TME_MIN(TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_reader->tme_bus_cycle_port),
		 TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_writer->tme_bus_cycle_port));
  lane_end = TME_MAX(TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_reader->tme_bus_cycle_port) + port_size_reader,
		     TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_writer->tme_bus_cycle_port) + port_size_writer);
  cycle_size_reader = cycle_size_writer = 0;
  for (; lane < lane_end; lane++) {

    /* assume that we won't have to warn on this lane: */
    warn_on_lane = FALSE;

    /* see if this lane falls in the reader or writer's port: */
    lane_reader = lane - TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_reader->tme_bus_cycle_port);
    lane_writer = lane - TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_writer->tme_bus_cycle_port);
    lane_in_reader = (lane_reader >= 0 && lane_reader < port_size_reader);
    lane_in_writer = (lane_writer >= 0 && lane_writer < port_size_writer);

    /* get the value being written to this byte lane.  assume a
       garbage value: */
    lane_value = 0xd2;

    /* if this lane is in the writer's port, it may supply a real
       lane value: */
    if (lane_in_writer) {

      /* get the routing for the writer: */
      lane_routing_writer = 
	cycle_writer->tme_bus_cycle_lane_routing[lane_routing_offset_writer + lane_writer];

      /* if the writer doesn't expect this lane to be connected to the
	 reader, we will issue a warning on this lane: */
      if ((lane_routing_writer & TME_BUS_LANE_WARN)
	  && lane_in_reader) {
	warn_on_lane = TRUE;
      }
      lane_routing_writer &= ~TME_BUS_LANE_WARN;

      /* dispatch on the routing to get the lane value: */
      if (lane_routing_writer == TME_BUS_LANE_ABORT) {
	abort();
      }
      else if (lane_routing_writer != TME_BUS_LANE_UNDEF) {
	if (!(lane_routing_writer & TME_BUS_LANE_ROUTE_WRITE_IGNORE)
	    && lane_routing_writer >= cycle_size_writer) {
	  cycle_size_writer = lane_routing_writer + 1;
	}
	lane_routing_writer &= ~TME_BUS_LANE_ROUTE_WRITE_IGNORE;

	/* if the writer is the responder, make sure that only bytes
	   in the given register are ever referenced.  given the
	   writer's port size, we could warp the reference index as
	   needed, but hopefully we'll never have to: */
	assert(!(cycle_writer == cycle_resp
		 && (((cycle_writer->tme_bus_cycle_address + lane_routing_writer)
		      ^  cycle_writer->tme_bus_cycle_address)
		     & ~(port_size_writer - 1)) != 0));

	lane_value =
	  *(cycle_writer->tme_bus_cycle_buffer
	    + _TME_BUS_CYCLE_BUFFER_MULTIPLY(lane_routing_writer,
					     buffer_increment_mask_writer));
      }
    }

    /* if this lane is in the reader's port, it may take the lane
       value: */
    if (lane_in_reader) {

      /* get the routing for the reader: */
      lane_routing_reader =
	cycle_reader->tme_bus_cycle_lane_routing[lane_routing_offset_reader + lane_reader];

      /* if the reader doesn't expect this lane to be connected to the
	 writer, we will issue a warning on this lane: */
      if ((lane_routing_reader & TME_BUS_LANE_WARN)
	  && lane_in_writer) {
	warn_on_lane = TRUE;
      }
      lane_routing_reader &= ~TME_BUS_LANE_WARN;

      /* dispatch on the routing to take the lane value: */
      if (lane_routing_reader == TME_BUS_LANE_ABORT) {
	abort();
      }
      else if (lane_routing_reader != TME_BUS_LANE_UNDEF
	       && !(lane_routing_reader & TME_BUS_LANE_ROUTE_WRITE_IGNORE)) {
	if (lane_routing_reader >= cycle_size_reader) {
	  cycle_size_reader = lane_routing_reader + 1;
	}

	/* if the reader is the responder, make sure that only bytes
	   in the given register are ever referenced.  given the
	   reader's port size, we could warp the reference index as
	   needed, but hopefully we'll never have to: */
	assert(!(cycle_reader == cycle_resp
		 && (((cycle_reader->tme_bus_cycle_address + lane_routing_reader)
		      ^  cycle_reader->tme_bus_cycle_address)
		     & ~(port_size_reader - 1)) != 0));

	*(cycle_reader->tme_bus_cycle_buffer
	  + _TME_BUS_CYCLE_BUFFER_MULTIPLY(lane_routing_reader,
					   buffer_increment_mask_reader)) =
	  lane_value;
      }
    }

    /* if we need to issue a warning on this lane: */
    if (warn_on_lane) {
      /* XXX TBD: */
      abort();
    }
  }

  /* give the reader feedback: */
  cycle_reader->tme_bus_cycle_size = cycle_size_reader;
  cycle_reader->tme_bus_cycle_address += cycle_size_reader;
  cycle_reader->tme_bus_cycle_buffer += 
    _TME_BUS_CYCLE_BUFFER_MULTIPLY(cycle_size_reader,
				   buffer_increment_mask_reader);
  cycle_reader->tme_bus_cycle_lane_routing += lane_routing_offset_reader;
  cycle_reader->tme_bus_cycle_port = 
    TME_BUS_CYCLE_PORT(port_overlap_lane_least, port_overlap_size_lg2);
  
  /* give the writer feedback: */
  cycle_writer->tme_bus_cycle_size = cycle_size_writer;
  cycle_writer->tme_bus_cycle_address += cycle_size_writer;
  cycle_writer->tme_bus_cycle_buffer += 
    _TME_BUS_CYCLE_BUFFER_MULTIPLY(cycle_size_writer,
				   buffer_increment_mask_writer);
  cycle_writer->tme_bus_cycle_lane_routing += lane_routing_offset_writer;
  cycle_writer->tme_bus_cycle_port = 
    TME_BUS_CYCLE_PORT(port_overlap_lane_least, port_overlap_size_lg2);
}

/* this handles a bus cycle for a memory-like device: */
void
tme_bus_cycle_xfer_memory(struct tme_bus_cycle *cycle_init, tme_uint8_t *memory, tme_bus_addr_t address_last)
{
  tme_uint8_t memory_junk[sizeof(tme_bus_addr_t)];
  struct tme_bus_cycle cycle_resp;

  /* check the starting address: */
  assert(cycle_init->tme_bus_cycle_address <= address_last);

  /* get the start of the buffer for this starting address: */
  if (memory != NULL) {
    memory += cycle_init->tme_bus_cycle_address;
  }
  else {
    assert(sizeof(memory_junk)
	   >= ((unsigned int) 1 << TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_init->tme_bus_cycle_port)));
    memory = memory_junk;
  }

  /* create the responder cycle: */
  cycle_resp.tme_bus_cycle_buffer = memory;
  cycle_resp.tme_bus_cycle_buffer_increment = 1;
  cycle_resp.tme_bus_cycle_lane_routing = cycle_init->tme_bus_cycle_lane_routing;
  cycle_resp.tme_bus_cycle_address = cycle_init->tme_bus_cycle_address;
  cycle_resp.tme_bus_cycle_type = (cycle_init->tme_bus_cycle_type
				   ^ (TME_BUS_CYCLE_WRITE
				      | TME_BUS_CYCLE_READ));
  cycle_resp.tme_bus_cycle_port = cycle_init->tme_bus_cycle_port;

  /* run the cycle: */
  tme_bus_cycle_xfer(cycle_init, &cycle_resp);

  /* check the finishing address: */
  assert((cycle_init->tme_bus_cycle_address - 1) <= address_last);
}
  
/* given an initiator's cycle and a responder's port size, assuming
   that the responder's port fits completely within the initiator's
   port, this internal function returns the "correct" least lane for
   the responder's port, relative to the least lane of the initiator's
   port.  this requires that the initiator's lane routing use either
   TME_BUS_LANE_ABORT or TME_BUS_LANE_WARN on routings for incorrect
   lanes: */
static unsigned int
_tme_bus_cycle_xfer_resp_least_lane(const struct tme_bus_cycle *cycle_init, 
				    unsigned int port_size_log2_resp)
{
  unsigned int port_size_resp;
  unsigned int port_lane_least_max_resp;
  unsigned int port_lane_least_resp;
  const tme_bus_lane_t *lane_routing_init;
  unsigned int lane;
  int lane_routing;

  /* get the responder's port size: */
  port_size_resp = (1 << port_size_log2_resp);

  /* calculate the maximum possible least lane for the responder.  we
     require that the responder's port fit completely within the
     initiator's port: */
  port_lane_least_max_resp = (1 << TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_init->tme_bus_cycle_port));
  if (__tme_predict_false(port_size_resp > port_lane_least_max_resp)) {
    abort();
  }
  port_lane_least_max_resp -= port_size_resp;

  /* assume that the responder will have the same least lane as the
     initiator, and get the initiator's lane routing for that
     responder least lane: */
  port_lane_least_resp = 0;
  lane_routing_init
    = (cycle_init->tme_bus_cycle_lane_routing
       + TME_BUS_ROUTER_INDEX(TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_init->tme_bus_cycle_port),
			      port_size_log2_resp,
			      port_lane_least_resp));

  /* loop over all of the possible least lanes for the responder: */
  for (;
       port_lane_least_resp <= port_lane_least_max_resp;
       port_lane_least_resp++) {

    /* check the routing for all of the lanes in the responder's port, starting
       from the highest numbered lane and working down: */
    lane = port_lane_least_resp + port_size_resp;
    for (;;) {
      lane = lane - 1;
      lane_routing = lane_routing_init[lane];

      /* if this lane gets a warning or an abort, this is not the
	 correct least lane for the responder: */
      if ((lane_routing & TME_BUS_LANE_WARN) 
	  || lane_routing == TME_BUS_LANE_ABORT) {
	break;
      }

      /* if we have now checked all of the lanes in the overlap between
	 initiator and responder, we've found the correct least lane for
	 the responder: */
      if (lane == port_lane_least_resp) {
	return (port_lane_least_resp);
      }
    }

    /* advance the offset into the initiator's lane routing: */
    lane_routing_init += (1 << TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_init->tme_bus_cycle_port));
  }

  /* if we get here, the initiator doesn't allow responders of the
     given port size: */
  abort();
}
	  
/* this handles a bus cycle for a simple register device: */
void
tme_bus_cycle_xfer_reg(struct tme_bus_cycle *cycle_init, 
		       void *resp_reg,
		       unsigned int port_size_log2_resp)
{
  unsigned int port_lane_least_resp;
  const tme_bus_lane_t *lane_routing_init;
  unsigned int lane_count;
  unsigned int lane;
  tme_uint8_t *buffer_init;
  tme_uint8_t *buffer_resp;
  int lane_routing;
  int cycle_size_init;
  int buffer_increment_mask_init;
  int writer_init;

  /* see if the initiator is writing: */
  writer_init = (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE);
  assert (writer_init || cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

  /* get the increment mask for the initiator.  since
     tme_bus_cycle_buffer_increment is always 1 or -1, this mask is
     used to negate values without multiplication: */
  if (cycle_init->tme_bus_cycle_buffer_increment == -1) {
    buffer_increment_mask_init = -1;
  }
  else {
    assert(cycle_init->tme_bus_cycle_buffer_increment == 1);
    buffer_increment_mask_init = 0;
  }

  /* get the least lane for the responder's port relative to the
     initiator port's least lane, assuming that the responder's port
     fits completely within the initiator's port and starts at the
     "correct" least lane: */
  port_lane_least_resp = _tme_bus_cycle_xfer_resp_least_lane(cycle_init,
							     port_size_log2_resp);

  /* get the initiator's lane routing: */
  lane_routing_init
    = (cycle_init->tme_bus_cycle_lane_routing
       + TME_BUS_ROUTER_INDEX(TME_BUS_CYCLE_PORT_SIZE_LG2(cycle_init->tme_bus_cycle_port),
			      port_size_log2_resp,
			      port_lane_least_resp));

  /* return some things to the initiator: */
  cycle_init->tme_bus_cycle_lane_routing = lane_routing_init;
  cycle_init->tme_bus_cycle_port = 
    TME_BUS_CYCLE_PORT((TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_init->tme_bus_cycle_port)
			+ port_lane_least_resp),
		       port_size_log2_resp);

  /* advance the initiator's lane routing to the first lane in the
     responder's port: */
  lane_routing_init += port_lane_least_resp;

  /* get the lane count: */
  lane_count = (1 << port_size_log2_resp);

  /* get the initial pointer into the responder's register buffer.  we
     move from lower-numbered lanes (with data of lesser significance)
     to higher-numbered lanes (with data of more significance).  we
     are always called with pointers to registers in host native byte
     order, so if the host is little-endian, we start from the given
     pointer, else we start from the other end of the register
     buffer: */
  buffer_resp = (((tme_uint8_t *) resp_reg)
		 + (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG
		    ? (lane_count - 1)
		    : 0));

  /* loop over the lanes: */
  lane = 0;
  cycle_size_init = 0;
  do {

    /* get the routing for this lane: */
    lane_routing = *(lane_routing_init++);

    /* this cannot be a TME_BUS_LANE_WARN, or a TME_BUS_LANE_ABORT, or
       an enabled byte lane with an undefined value: */
    assert (!(lane_routing & TME_BUS_LANE_WARN)
	    && lane_routing != TME_BUS_LANE_ABORT
	    && lane_routing != TME_BUS_LANE_UNDEF);

    /* if this is an enabled byte lane: */
    if (__tme_predict_true(!(lane_routing & TME_BUS_LANE_ROUTE_WRITE_IGNORE))) {

      /* get a pointer into the initiator's buffer for this lane: */
      buffer_init
	= (cycle_init->tme_bus_cycle_buffer
	   + _TME_BUS_CYCLE_BUFFER_MULTIPLY(lane_routing,
					    buffer_increment_mask_init));

      /* transfer the byte: */
      if (writer_init) {
	*buffer_resp = *buffer_init;
      }
      else {
	*buffer_init = *buffer_resp;
      }

      /* update the cycle size for the initiator: */
      if (lane_routing >= cycle_size_init) {
	cycle_size_init = lane_routing + 1;
      }
    }

    /* update the pointer into the responder's buffer for the next lane: */
    buffer_resp += (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG ? -1 : 1);

  } while (--lane_count > 0);

  /* give the initiator feedback: */
  cycle_init->tme_bus_cycle_size = cycle_size_init;
  cycle_init->tme_bus_cycle_address += cycle_size_init;
  cycle_init->tme_bus_cycle_buffer += 
    _TME_BUS_CYCLE_BUFFER_MULTIPLY(cycle_size_init,
				   buffer_increment_mask_init);
}
