/* $Id: bus.h,v 1.15 2009/08/29 17:35:08 fredette Exp $ */

/* tme/generic/bus.h - header file for generic bus support: */

/*
 * Copyright (c) 2002, 2003 Matt Fredette
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

#ifndef _TME_GENERIC_BUS_H
#define _TME_GENERIC_BUS_H

#include <tme/common.h>
_TME_RCSID("$Id: bus.h,v 1.15 2009/08/29 17:35:08 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/threads.h>
#include <tme/memory.h>
#include <tme/token.h>

/* macros: */

/* the log2 of various bus sizes, named by number of bits but really
   in terms of bytes: */
#define TME_BUS8_LOG2		(0)
#define TME_BUS16_LOG2		(1)
#define TME_BUS32_LOG2		(2)
#define TME_BUS64_LOG2		(3)
#define TME_BUS128_LOG2		(4)

/* bus signal flags and form: */
#define TME_BUS_SIGNAL_LEVEL_MASK	(0x03)
#define  TME_BUS_SIGNAL_LEVEL_LOW	(0x00)
#define  TME_BUS_SIGNAL_LEVEL_HIGH	(0x01)
#define  TME_BUS_SIGNAL_LEVEL_NEGATED	(0x02)
#define  TME_BUS_SIGNAL_LEVEL_ASSERTED	(0x03)
#define TME_BUS_SIGNAL_EDGE		(0x04)
#define _TME_BUS_SIGNAL_BITS		(5)
#define TME_BUS_SIGNAL_WHICH(x)		((x) & ~((1 << _TME_BUS_SIGNAL_BITS) - 1))
#define TME_BUS_SIGNAL_INDEX(x)		((x) >> _TME_BUS_SIGNAL_BITS)
#define TME_BUS_SIGNAL_X(x)		((x) << _TME_BUS_SIGNAL_BITS)

/* all bus signal set identifiers: */
#define TME_BUS_SIGNALS_ID_GENERIC	(0)
#define TME_BUS_SIGNALS_ID_I825X6	(1)

/* the generic bus signal set: */
#define TME_BUS_SIGNAL_INT(x)		TME_BUS_SIGNAL_X(x)
#define TME_BUS_SIGNAL_IS_INT(x)	(TME_BUS_SIGNAL_INDEX(x) < 256)
#define TME_BUS_SIGNAL_INDEX_INT(x)	TME_BUS_SIGNAL_INDEX(x)
#define TME_BUS_SIGNAL_INT_UNSPEC	TME_BUS_SIGNAL_X(256)
#define TME_BUS_SIGNAL_HALT		TME_BUS_SIGNAL_X(257)
#define TME_BUS_SIGNAL_RESET		TME_BUS_SIGNAL_X(258)
#define TME_BUS_SIGNAL_IGNORE		TME_BUS_SIGNAL_X(259)
#define TME_BUS_SIGNAL_ABORT		TME_BUS_SIGNAL_X(260)
#define TME_BUS_SIGNAL_DRQ		TME_BUS_SIGNAL_X(261)
#define TME_BUS_SIGNAL_DACK		TME_BUS_SIGNAL_X(262)
#define TME_BUS_SIGNAL_BR		TME_BUS_SIGNAL_X(263)
#define TME_BUS_SIGNAL_BG		TME_BUS_SIGNAL_X(264)
#define TME_BUS_SIGNALS_GENERIC		{ TME_BUS_SIGNALS_ID_GENERIC, TME_BUS_VERSION, 384, 0 }

/* this gets the index and mask of a bus signal bit in a byte array: */
#define TME_BUS_SIGNAL_BIT_INDEX(x)	(TME_BUS_SIGNAL_INDEX(x) >> 3)
#define TME_BUS_SIGNAL_BIT_MASK(x)	TME_BIT(TME_BUS_SIGNAL_INDEX(x) & 7)

/* this gets the number of bytes in a byte array of bus signal bits: */
#define TME_BUS_SIGNAL_BIT_BYTES(count)	(((count) + 7) >> 3)

/* the undefined interrupt vector: */
#define TME_BUS_INTERRUPT_VECTOR_UNDEF	(-1)

/* bus cycles: */
#define TME_BUS_CYCLE_UNDEF	(0)
#define TME_BUS_CYCLE_READ	TME_BIT(0)
#define TME_BUS_CYCLE_WRITE	TME_BIT(1)
#define TME_BUS_CYCLE_LOCK	TME_BIT(2)
#define TME_BUS_CYCLE_UNLOCK	TME_BIT(3)

/* the maximum number of fault handlers on a TLB entry: */
#define TME_BUS_TLB_FAULT_HANDLERS	(4)

/* this returns nonzero if a TLB entry is valid: */
#define tme_bus_tlb_is_valid(tlb)					\
  (tme_token_is_valid((tlb)->tme_bus_tlb_token))

/* this returns nonzero if a TLB entry is invalid: */
#define tme_bus_tlb_is_invalid(tlb)					\
  (tme_token_is_invalid((tlb)->tme_bus_tlb_token))

/* this busies a TLB entry: */
#define tme_bus_tlb_busy(tlb)		tme_token_busy((tlb)->tme_bus_tlb_token)

/* this unbusies a TLB entry: */
#define tme_bus_tlb_unbusy(tlb)		tme_token_unbusy((tlb)->tme_bus_tlb_token)

/* this unbusies a TLB entry for filling: */
/* NB: we must clear the invalid on the TLB's token before we fill it.
   we can't do that after we fill it, because we will race with an
   immediate invalidation.  however, by marking it valid before we fill
   it, we race with ourselves - before the TLB entry is filled, we may
   run again and see a TLB entry that was actually invalidated, as
   valid.  to avoid this case, we invalidate the TLB entry in another
   way - we poison its first and last addresses to impossible values: */
/* NB: once all element callins have been rearchitected to block until
   a callout has completed, this poisoning can go away: */
#define tme_bus_tlb_unbusy_fill(tlb)					\
  do {									\
    tme_bus_tlb_unbusy(tlb);						\
    if (tme_bus_tlb_is_invalid(tlb)) {					\
      (tlb)->tme_bus_tlb_addr_first = 1;				\
      (tlb)->tme_bus_tlb_addr_last = 0;					\
      tme_token_invalid_clear((tlb)->tme_bus_tlb_token);		\
    }									\
  } while (/* CONSTCOND */ 0)

/* this adds a fault handler to a TLB entry: */
#define TME_BUS_TLB_FAULT_HANDLER(tlb, func, private)	\
do {							\
  assert(tlb->tme_bus_tlb_fault_handler_count		\
	 < TME_BUS_TLB_FAULT_HANDLERS);			\
  tlb->tme_bus_tlb_fault_handlers			\
    [tlb->tme_bus_tlb_fault_handler_count]		\
    .tme_bus_tlb_fault_handler_private = private;	\
  tlb->tme_bus_tlb_fault_handlers			\
    [tlb->tme_bus_tlb_fault_handler_count]		\
    .tme_bus_tlb_fault_handler = func;			\
  tlb->tme_bus_tlb_fault_handler_count++;		\
} while (/* CONSTCOND */ 0)

/* this indexes a generic bus router array for a device with a port
   size of 8 * (2 ^ siz_lg2) bits: */
#define TME_BUS_ROUTER_INDEX(siz_lg2, other_port_siz_lg2, other_port_lane_least) \
(((							\
   /* by the (overlapping) other port size: */		\
   (other_port_siz_lg2)					\
							\
   /* by the (overlapping) other port least lane: */	\
    << (siz_lg2))					\
   + other_port_lane_least)				\
							\
  /* by lane number, which we add later: */		\
  << (siz_lg2))

/* this gives the number of entries that must be in a generic bus
   router array for a device with a bus size of 8 * (2 ^ siz_lg2)
   bits: */
#define TME_BUS_ROUTER_SIZE(siz_lg2)				\
  TME_BUS_ROUTER_INDEX(siz_lg2, (siz_lg2) + 1, 0)

/* bus lane routing entries: */
#define TME_BUS_LANE_WARN			TME_BIT(7)
#define TME_BUS_LANE_ABORT			(0x7f)
#define TME_BUS_LANE_UNDEF			(0x7e)
#define TME_BUS_LANE_ROUTE_WRITE_IGNORE		TME_BIT(6)
#define TME_BUS_LANE_ROUTE(x)			(x)

/* this is a special bus cycle return value.  it doesn't indicate a
   fault, but instead tells the initiator that some other event has
   happened on the bus, synchronous with the bus cycle, that the
   initiator should handle: */
#define TME_BUS_CYCLE_SYNCHRONOUS_EVENT		(EINTR)

/* internal bus connection types: */
#define TME_BUS_CONNECTION_INT_FLAG_ADDRESSABLE		TME_BIT(0)
#define TME_BUS_CONNECTION_INT_FLAG_CONTROLLER		TME_BIT(1)

/* types: */
struct tme_bus_tlb;

/* a bus address: */
typedef tme_uint32_t		tme_bus_addr32_t;
#ifdef TME_HAVE_INT64_T
typedef tme_uint64_t		tme_bus_addr64_t;
#endif /* TME_HAVE_INT64_T */
#if TME_BUSMAX_LOG2 <= TME_BUS32_LOG2
typedef tme_bus_addr32_t	tme_bus_addr_t;
#elif TME_BUSMAX_LOG2 == TME_BUS64_LOG2
#ifndef TME_HAVE_INT64_T
#error "64-bit guests require 64-bit integer types"
#else  /* TME_HAVE_INT64_T */
typedef tme_bus_addr64_t	tme_bus_addr_t;
#endif /* TME_HAVE_INT64_T */
#else  /* TME_BUSMAX_LOG2 */
#error "unsupported maximum guest bus size"
#endif /* TME_BUSMAX_LOG2 */

/* a bus byte lane routing entry: */
typedef tme_uint8_t		tme_bus_lane_t;

/* a bus context: */
typedef tme_uint32_t		tme_bus_context_t;

/* a bus cycle: */
struct tme_bus_cycle {

  /* the bus cycle data buffer pointer.  this points to the byte
     associated with the bus address given below: */
  tme_uint8_t *tme_bus_cycle_buffer;

  /* how bus byte lanes are connected to the bus cycle data buffer: */
  _tme_const tme_bus_lane_t *tme_bus_cycle_lane_routing;

  /* the bus address: */
  tme_bus_addr_t tme_bus_cycle_address;

  /* when adding one to the bus address, add this to the bus cycle
     data buffer pointer to get a pointer to the byte associated with
     the new bus address: */
  tme_int8_t tme_bus_cycle_buffer_increment;

  /* the type of bus cycle: */
  tme_uint8_t tme_bus_cycle_type;

  /* the maximum number of addresses that could be covered by this
     cycle.  depending on where and how wide the initiator and
     responder ports overlap, the number of addresses actually covered
     may be less: */
  tme_uint8_t tme_bus_cycle_size;

  /* the starting lane and size of this device's port.  bits 0-2 are
     the log2 of the lane size of the port.  zero corresponds to a
     one-lane (8-bit) port, one to a two-lane (16-bit) port, etc.
     bits 3-7 are the least byte lane in this device's port.  zero
     corresponds to D7-D0, one to D15-D8, etc.: */
  tme_uint8_t tme_bus_cycle_port;
#define TME_BUS_CYCLE_PORT(lane_least, lane_size_lg2) \
  (((lane_least) << 3) | (lane_size_lg2))
#define TME_BUS_CYCLE_PORT_SIZE_LG2(port) \
  TME_FIELD_EXTRACTU(port, 0, 3)
#define TME_BUS_CYCLE_PORT_LANE_LEAST(port) \
  TME_FIELD_EXTRACTU(port, 3, 5)
};

/* a bus cycle handler: */
typedef int (*tme_bus_cycle_handler) _TME_P((void *, struct tme_bus_cycle *));

/* a bus fault handler: */
typedef int (*tme_bus_fault_handler) _TME_P((void *, struct tme_bus_tlb *, struct tme_bus_cycle *, int));

/* a bus cacheable: */
struct tme_bus_cacheable {

  /* the cacheable contents and size: */
  tme_shared tme_uint8_t *tme_bus_cacheable_contents;
  unsigned long tme_bus_cacheable_size;

  /* the cacheable private state: */
  void *tme_bus_cacheable_private;

  /* this function allocates a new valids bitmask: */
  tme_shared tme_uint8_t *(*tme_bus_cacheable_valids_new) _TME_P((void *, tme_uint32_t));

  /* this function sets a bit in the valids bitmask: */
  void (*tme_bus_cacheable_valids_set) _TME_P((void *, tme_shared tme_uint8_t *, unsigned long));
};

/* a bus TLB entry: */
struct tme_bus_tlb {

  /* the bus address region covered by this TLB entry: */
  tme_bus_addr_t tme_bus_tlb_addr_first;
  tme_bus_addr_t tme_bus_tlb_addr_last;

  /* the token associated with this TLB entry: */
  struct tme_token *tme_bus_tlb_token;

  /* when one or both of these pointers are not TME_EMULATOR_OFF_UNDEF, 
     this TLB entry allows fast (memory) reads of and/or writes to the
     bus region.  adding an address in the bus region to one of these
     pointers yields the desired host memory address: */
  _tme_const tme_shared tme_uint8_t *tme_bus_tlb_emulator_off_read;
  tme_shared tme_uint8_t *tme_bus_tlb_emulator_off_write;

  /* fast (memory) reads and writes are protected by this rwlock: */
  tme_rwlock_t *tme_bus_tlb_rwlock;

  /* if non-NULL, this bus region is cacheable memory: */
  _tme_const struct tme_bus_cacheable *tme_bus_tlb_cacheable;

  /* when one or both of TLB_BUS_CYCLE_READ and TLB_BUS_CYCLE_WRITE
     are set in this value, this TLB entry allows slow (function call)
     reads of and/or writes to the bus region: */
  unsigned int tme_bus_tlb_cycles_ok;

  /* adding an address in the bus region to this offset, and then
     shifting that result to the right (shift > 0) or to the left
     (shift < 0) yields an address for the bus cycle handler: */
  tme_bus_addr_t tme_bus_tlb_addr_offset;
  int tme_bus_tlb_addr_shift;

  /* the bus cycle handler: */
  void *tme_bus_tlb_cycle_private;
  tme_bus_cycle_handler tme_bus_tlb_cycle;

  /* the bus fault handlers: */
  unsigned int tme_bus_tlb_fault_handler_count;
  struct {
    void *tme_bus_tlb_fault_handler_private;
    tme_bus_fault_handler tme_bus_tlb_fault_handler;
  } tme_bus_tlb_fault_handlers[TME_BUS_TLB_FAULT_HANDLERS];
};

/* a bus signals set: */
struct tme_bus_signals {

  /* the bus signals set identifier: */
  tme_uint32_t tme_bus_signals_id;

  /* the version of the bus signals: */
  tme_uint32_t tme_bus_signals_version;

  /* the maximum number of bus signals in the set: */
  tme_uint32_t tme_bus_signals_count;

  /* the first signal in the bus signals set: */
  tme_uint32_t tme_bus_signals_first;
};

/* a bus master's TLB set information: */
struct tme_bus_tlb_set_info {

  /* the first token in the set: */
  struct tme_token *tme_bus_tlb_set_info_token0;

  /* the stride between tokens in the set, in bytes: */
  unsigned long tme_bus_tlb_set_info_token_stride;

  /* the count of tokens in the set: */
  unsigned long tme_bus_tlb_set_info_token_count;

  /* the set's optional bus context register: */
  /* NB: this isn't tme_shared because the bus context, when it does
     exist, is only exposed to the one bus master that can actually
     change it - and we assume that changing the bus context
     synchronizes the master: */
  tme_bus_context_t *tme_bus_tlb_set_info_bus_context;

  /* the maximum value of the bus context register: */
  tme_bus_context_t tme_bus_tlb_set_info_bus_context_max;
};

/* a bus connection: */
struct tme_bus_connection {

  /* the generic connection side: */
  struct tme_connection tme_bus_connection;

  /* the subregions on the bus for this connection.  most connections
     will only have one subregion, with a first address of zero and a
     last address of their size on the bus: */
  struct tme_bus_subregion {

    /* the first and last addresses, starting from zero, of this
       subregion: */
    tme_bus_addr_t tme_bus_subregion_address_first;
    tme_bus_addr_t tme_bus_subregion_address_last;

    /* any other subregions for this bus connection: */
    _tme_const struct tme_bus_subregion *tme_bus_subregion_next;
  } tme_bus_subregions;

  /* the bus signal set adder: */
  int (*tme_bus_signals_add) _TME_P((struct tme_bus_connection *,
				     struct tme_bus_signals *));

  /* the bus signal handler: */
  int (*tme_bus_signal) _TME_P((struct tme_bus_connection *, unsigned int));

  /* the bus interrupt acknowledge handler: */
  int (*tme_bus_intack) _TME_P((struct tme_bus_connection *, unsigned int, int *));

  /* the bus TLB set add handler: */
  int (*tme_bus_tlb_set_add) _TME_P((struct tme_bus_connection *, struct tme_bus_tlb_set_info *));

  /* the bus TLB entry filler: */
  int (*tme_bus_tlb_fill) _TME_P((struct tme_bus_connection *, struct tme_bus_tlb *,
				  tme_bus_addr_t, unsigned int));
};

/* internal information about a bus connection: */
struct tme_bus_connection_int {

  /* the external bus connection: */
  struct tme_bus_connection tme_bus_connection_int;

  /* flags on the bus connection: */
  int tme_bus_connection_int_flags;

  /* the first and last addresses of this connection.  most code
     should never use the last-address value, and instead should honor
     the connection's subregions: */
  tme_bus_addr_t tme_bus_connection_int_address;
  tme_bus_addr_t tme_bus_connection_int_address_last;

  /* the mask added to addresses sourced by this connection: */
  tme_bus_addr_t tme_bus_connection_int_sourced;

  /* the single interrupt signal used by this connection, when
     the connection doesn't know already: */
  int tme_bus_connection_int_signal_int;

  /* the single interrupt vector used by this connection, when the
     connection doesn't know already: */
  int tme_bus_connection_int_vector_int;

  /* nonzero iff we've already logged an unconfigured interrupt
     signal: */
  int tme_bus_connection_int_logged_int;

  /* the current status of the bus signals for this connection: */
  tme_uint8_t *tme_bus_connection_int_signals;
};

/* a generic bus slot: */
struct tme_bus_slot {

  /* generic bus slots are kept on a list: */
  struct tme_bus_slot *tme_bus_slot_next;

  /* the name of this bus slot: */
  char *tme_bus_slot_name;

  /* the address and size of this bus slot: */
  tme_bus_addr_t tme_bus_slot_address;
  tme_bus_addr_t tme_bus_slot_size;
};  

/* a generic bus: */
struct tme_bus {

  /* the optional rwlock protecting this bus: */
  tme_rwlock_t tme_bus_rwlock;

  /* the address mask used on this bus: */
  tme_bus_addr_t tme_bus_address_mask;

  /* all connections to this bus: */
  struct tme_bus_connection_int *tme_bus_connections;

  /* the number of addressable connections to this bus: */
  int tme_bus_addressables_count;

  /* the size of the addressable connections array: */
  int tme_bus_addressables_size;

  /* the addressable connections array: */
  struct tme_bus_addressable {
    struct tme_bus_connection_int *tme_bus_addressable_connection;
    _tme_const struct tme_bus_subregion *tme_bus_addressable_subregion;
  } *tme_bus_addressables;

  /* the bus signal sets on this bus: */
  unsigned int tme_bus_signals_count;
  struct tme_bus_signals *tme_bus_signals;

  /* the number of devices asserting the various bus signals: */
  unsigned int *tme_bus_signal_asserts;

  /* any bus slots: */
  struct tme_bus_slot *tme_bus_slots;

  /* any bus controller connection: */
  struct tme_bus_connection_int *tme_bus_controller;
};

/* prototypes: */
int tme_bus_address_search _TME_P((struct tme_bus *, tme_bus_addr_t));
int tme_bus_connection_ok _TME_P((struct tme_bus *,
				  struct tme_bus_connection_int *));
int tme_bus_connection_make _TME_P((struct tme_bus *,
				    struct tme_bus_connection_int *,
				    unsigned int));
int tme_bus_connection_break _TME_P((struct tme_bus *,
				     struct tme_bus_connection_int *,
				     unsigned int));
int tme_bus_tlb_fill _TME_P((struct tme_bus *, 
			     struct tme_bus_connection_int *, 
			     struct tme_bus_tlb *, tme_bus_addr_t, unsigned int));
int tme_bus_tlb_set_add _TME_P((struct tme_bus *,
				struct tme_bus_connection_int *,
				struct tme_bus_tlb_set_info *));
void tme_bus_tlb_set_invalidate _TME_P((_tme_const struct tme_bus_tlb_set_info *));
void tme_bus_tlb_map _TME_P((struct tme_bus_tlb *, tme_bus_addr_t, _tme_const struct tme_bus_tlb *, tme_bus_addr_t));
void tme_bus_tlb_initialize _TME_P((struct tme_bus_tlb *));
int tme_bus_tlb_fault _TME_P((struct tme_bus_tlb *, struct tme_bus_cycle *, int));
tme_bus_addr_t tme_bus_addr_parse _TME_P((_tme_const char *, tme_bus_addr_t));
tme_bus_addr_t tme_bus_addr_parse_any _TME_P((_tme_const char *, int *));
void tme_bus_cycle_xfer _TME_P((struct tme_bus_cycle *, struct tme_bus_cycle *));
void tme_bus_cycle_xfer_memory _TME_P((struct tme_bus_cycle *, tme_uint8_t *, tme_bus_addr_t));
void tme_bus_cycle_xfer_reg _TME_P((struct tme_bus_cycle *, void *, unsigned int));

#endif /* !_TME_GENERIC_BUS_H */
