/* $Id: i825x6.c,v 1.8 2010/06/05 14:43:27 fredette Exp $ */

/* ic/i825x6.c - implementation of the Intel 825x6 emulation: */

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
_TME_RCSID("$Id: i825x6.c,v 1.8 2010/06/05 14:43:27 fredette Exp $");

/* XXX FIXME - TLB usage here is not thread-safe: */

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/generic/ethernet.h>
#undef TME_I825X6_VERSION
#define TME_I825X6_VERSION TME_X_VERSION(0, 0)
#include <tme/ic/i825x6.h>
#include "i825x6reg.h"

/* macros: */

/* these get and put values of different sizes.  the values *must* be aligned: */
#define TME_I825X6_GET16(bytes) tme_letoh_u16(*((const tme_uint16_t *) (bytes)))
#define TME_I825X6_PUT16(bytes, val) (*((tme_uint16_t *) (bytes)) = tme_htole_u16(val))
#define TME_I82586_GET24(bytes) (tme_letoh_u32(*((const tme_uint32_t *) (bytes))) & 0xffffff)

/* these read and write regions using DMA: */
#define TME_I825X6_READ(address, v)				\
do {								\
  rc = tme_bus_device_dma_read_16(&i825x6->tme_i825x6_device,	\
				  (address),			\
				  sizeof(v),			\
				  (tme_uint8_t *) &(v),		\
				  TME_I825X6_LOCKS_DEFAULT);	\
  assert (rc == TME_OK);					\
} while (/* CONSTCOND */ 0)
#define TME_I825X6_WRITE(address, v)				\
do {								\
  rc = tme_bus_device_dma_write_16(&i825x6->tme_i825x6_device,	\
				  (address),			\
				  sizeof(v),			\
				  (const tme_uint8_t *) &(v),	\
				  TME_I825X6_LOCKS_DEFAULT);	\
  assert (rc == TME_OK);					\
} while (/* CONSTCOND */ 0)

/* these read and write values of different sizes using DMA: */
#define TME_I825X6_READ16(address, v)	\
do {					\
  TME_I825X6_READ(address, value16);	\
  v = TME_I825X6_GET16(&value16);	\
} while (/* CONSTCOND */ 0)
#define TME_I82586_READ24(address, v)	\
do {					\
  TME_I825X6_READ(address, value32);	\
  v = TME_I82586_GET24(&value32);	\
} while (/* CONSTCOND */ 0)
#define TME_I825X6_WRITE16(address, v)	\
do {					\
  TME_I825X6_PUT16(&value16, v);	\
  TME_I825X6_WRITE(address, value16);	\
} while (/* CONSTCOND */ 0)

/* the "reset" stat_cus_rus_t value: */
#define TME_I825X6_CA_RESET		(0xffff)

/* the address of the idle "Command Block": */
#define TME_I825X6_CB_ADDRESS_IDLE	(0xffffffff)

/* an undefined Receive Unit address: */
#define TME_I825X6_RU_ADDRESS_UNDEF	(0xffffffff)

/* an undefined Receive Unit offset: */
#define TME_I825X6_RU_OFFSET_UNDEF	(0xffff)

/* the default locks: */
#define TME_I825X6_LOCKS_DEFAULT	(0)

/* the size of the TLB entry hash: */
#define TME_I825X6_TLB_HASH_SIZE 	(512)

/* the callout flags: */
#define TME_I825X6_CALLOUTS_CHECK	(0)
#define TME_I825X6_CALLOUTS_RUNNING	TME_BIT(0)
#define TME_I825X6_CALLOUTS_MASK	(-2)
#define  TME_I825X6_CALLOUT_CTRL	TME_BIT(1)
#define  TME_I825X6_CALLOUT_CONFIG	TME_BIT(2)
#define  TME_I825X6_CALLOUT_READ	TME_BIT(3)
#define	 TME_I825X6_CALLOUT_INT		TME_BIT(4)
#define  TME_I825X6_CALLOUT_CA		TME_BIT(5)
#define  TME_I825X6_CALLOUT_CU		TME_BIT(6)

/* structures: */

/* an rx buffer: */
struct tme_i825x6_rx_buffer {

  /* the generic ethernet frame chunk.  this must be first, since we
     abuse its tme_ethernet_frame_chunk_next for our own next pointer: */
  union {
    struct tme_i825x6_rx_buffer *_tme_i825x6_rx_buffer_u_next;
    struct tme_ethernet_frame_chunk _tme_i825x6_rx_buffer_u_frame_chunk;
  } _tme_i825x6_rx_buffer_u;
#define TME_I825X6_RX_BUFFER_NEXT(rx_buffer) \
  ((rx_buffer)->_tme_i825x6_rx_buffer_u._tme_i825x6_rx_buffer_u_next)
#define tme_i825x6_rx_buffer_frame_chunk _tme_i825x6_rx_buffer_u._tme_i825x6_rx_buffer_u_frame_chunk

  /* when this is TME_I825X6_RU_ADDRESS_UNDEF, this rx buffer was made
     from a fast-write TLB entry, and the generic ethernet frame chunk
     points directly into the fast-write memory.  otherwise, the
     generic ethernet frame chunk points to a private intermediate
     buffer, and this is the bus address to DMA the buffer back to: */
  tme_uint32_t tme_i825x6_rx_buffer_rb_address;

  /* when this is not TME_I825X6_RU_ADDRESS_UNDEF, this rx buffer
     finishes filling the buffer attached to the Receive Buffer
     Descriptor at this address, and signals the receiver to update
     that descriptor's Size field: */
  tme_uint32_t tme_i825x6_rx_buffer_rbd_address;
};

/* the chip: */
struct tme_i825x6 {

  /* our simple bus device header: */
  struct tme_bus_device tme_i825x6_device;
#define tme_i825x6_element tme_i825x6_device.tme_bus_device_element

  /* the Ethernet connection: */
  struct tme_ethernet_connection *tme_i825x6_eth_connection;

  /* the mutex protecting the chip: */
  tme_mutex_t tme_i825x6_mutex;

  /* the callout flags: */
  int tme_i825x6_callout_flags;

  /* our DMA TLB hash: */
  struct tme_bus_tlb tme_i825x6_tlb_hash[TME_I825X6_TLB_HASH_SIZE];
  int tme_i825x6_tlb_hash_added;

  /* the i825x6 bus signals: */
  struct tme_bus_signals tme_i825x6_bus_signals;

  /* the rx buffer free list: */
  struct tme_i825x6_rx_buffer *tme_i825x6_rx_buffer_free_list;

  /* this is nonzero if the next CA follows RESET: */
  int tme_i825x6_ca_follows_reset;

  /* the Ethernet addresses.  there are always at least two addresses
     in this array - the broadcast address and the Individual Address,
     in that order: */
  unsigned int tme_i825x6_address_count;
  tme_uint8_t *tme_i825x6_addresses;

  /* the i82586 AL-LOC value: */
  int tme_i825x6_al_loc;

  /* the i82586 PRM value: */
  int tme_i825x6_prm;

  /* the i82586 and 32-bit segmented i82596 SCB base: */
  tme_uint32_t tme_i825x6_scb_base;

  /* the SCB address: */
  tme_uint32_t tme_i825x6_scb_address;

  /* the SCB status word: */
  tme_uint16_t tme_i825x6_stat_cus_rus_t;

  /* the SCB Command Unit Command: */
  tme_uint16_t tme_i825x6_cuc;

  /* the CB address: */
  tme_uint32_t tme_i825x6_cb_address;

  /* the CB status word: */
  tme_uint16_t tme_i825x6_c_b_ok_a;

  /* the CB command word: */
  tme_uint16_t tme_i825x6_el_s_i_cmd;

  /* the next CB address: */
  tme_uint32_t tme_i825x6_cb_address_next;

  /* the RFD address: */
  tme_uint32_t tme_i825x6_rfd_address;

  /* the Free Buffer List: */
  struct tme_i825x6_rx_buffer *tme_i825x6_fbl;

  /* the size of all buffers on the Free Buffer List: */
  tme_uint32_t tme_i825x6_fbl_size;
  
  /* the address of the offset of the next free RBD: */
  tme_uint32_t tme_i825x6_rbd_offset_address;
};

/* prototypes: */
static void _tme_i825x6_abort_ru _TME_P((struct tme_i825x6 *));

/* globals: */
static const struct tme_bus_signals _tme_i825x6_bus_signals = TME_BUS_SIGNALS_I825X6;

/* this resets the i825x6: */
static void
_tme_i825x6_reset(struct tme_i825x6 *i825x6)
{

  tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	  100, TME_OK,
	  (&i825x6->tme_i825x6_element->tme_element_log_handle,
	   "reset"));

  /* clear all pending callouts: */
  i825x6->tme_i825x6_callout_flags &= TME_I825X6_CALLOUTS_MASK;
  
  /* abort the Receive Unit: */
  _tme_i825x6_abort_ru(i825x6);

  /* the Receive Unit is now Idle: */
  i825x6->tme_i825x6_stat_cus_rus_t
    = ((i825x6->tme_i825x6_stat_cus_rus_t
	& ~TME_I82586_SCB_RUS_MASK)
       | TME_I825X6_SCB_RUS_IDLE);

  /* we have no packet to transmit: */
  i825x6->tme_i825x6_el_s_i_cmd = TME_I825X6_CB_CMD_NOP;

  /* if the interrupt line is currently asserted, negate it: */
  if (i825x6->tme_i825x6_stat_cus_rus_t
      & TME_I825X6_SCB_STAT_MASK) {
    i825x6->tme_i825x6_stat_cus_rus_t &= ~TME_I825X6_SCB_STAT_MASK;
    i825x6->tme_i825x6_callout_flags |= TME_I825X6_CALLOUT_INT;
  }

  /* initialize the address list to match two addresses - the
     broadcast address, and the Individual Address (which is
     initially the same as the broadcast address): */
  i825x6->tme_i825x6_address_count = 2;
  memcpy (i825x6->tme_i825x6_addresses,
	  &tme_ethernet_addr_broadcast[0],
	  TME_ETHERNET_ADDR_SIZE);
  memcpy (i825x6->tme_i825x6_addresses + TME_ETHERNET_ADDR_SIZE,
	  &tme_ethernet_addr_broadcast[0],
	  TME_ETHERNET_ADDR_SIZE);

  /* the next CA follows RESET: */
  i825x6->tme_i825x6_ca_follows_reset = TRUE;
}

/* this hashes an address into a TLB entry: */
static struct tme_bus_tlb *
_tme_i825x6_tlb_hash(void *_i825x6,
		     tme_bus_addr_t linear_address,
		     unsigned int cycles)
{
  struct tme_i825x6 *i825x6;

  /* recover our data structure: */
  i825x6 = (struct tme_i825x6 *) _i825x6;

  /* return the TLB entry: */
  return (i825x6->tme_i825x6_tlb_hash
	  + ((((tme_bus_addr32_t) linear_address) >> 10) & (TME_I825X6_TLB_HASH_SIZE - 1)));
}

/* this locks the mutex: */
static void
_tme_i825x6_lock(void *_i825x6,
		 unsigned int locks)
{
  struct tme_i825x6 *i825x6;

  /* recover our data structure: */
  i825x6 = (struct tme_i825x6 *) _i825x6;

  /* lock the mutex: */
  tme_mutex_lock(&i825x6->tme_i825x6_mutex);
}

/* this unlocks the mutex: */
static void
_tme_i825x6_unlock(void *_i825x6,
		   unsigned int locks)
{
  struct tme_i825x6 *i825x6;

  /* recover our data structure: */
  i825x6 = (struct tme_i825x6 *) _i825x6;

  /* unlock the mutex: */
  tme_mutex_unlock(&i825x6->tme_i825x6_mutex);
}

/* this frees an rx buffer: */
static struct tme_i825x6_rx_buffer *
_tme_i825x6_rx_buffer_free(struct tme_i825x6 *i825x6,
			   struct tme_i825x6_rx_buffer *rx_buffer)
{
  struct tme_i825x6_rx_buffer *rx_buffer_next;

  /* get the next rx buffer: */
  rx_buffer_next = TME_I825X6_RX_BUFFER_NEXT(rx_buffer);

  /* put this rx buffer on the free list: */
  TME_I825X6_RX_BUFFER_NEXT(rx_buffer) = i825x6->tme_i825x6_rx_buffer_free_list;
  i825x6->tme_i825x6_rx_buffer_free_list = rx_buffer;

  /* return the next rx buffer: */
  return (rx_buffer_next);
}

/* this allocates a rx buffer: */
static struct tme_i825x6_rx_buffer *
_tme_i825x6_rx_buffer_new(struct tme_i825x6 *i825x6,
			  struct tme_i825x6_rx_buffer ***__prev)
{
  struct tme_i825x6_rx_buffer *rx_buffer, **_prev;

  /* if the free list is not empty: */
  rx_buffer = i825x6->tme_i825x6_rx_buffer_free_list;
  if (rx_buffer != NULL) {

    /* remove this rx buffer from the free list: */
    i825x6->tme_i825x6_rx_buffer_free_list = TME_I825X6_RX_BUFFER_NEXT(rx_buffer);
  }

  /* otherwise, the free list is empty: */
  else {

    /* allocate a new rx buffer: */
    rx_buffer = tme_new(struct tme_i825x6_rx_buffer, 1);

    /* treat this as an old fast-write TLB entry: */
    rx_buffer->tme_i825x6_rx_buffer_rb_address = TME_I825X6_RU_ADDRESS_UNDEF;
  }

  /* add this new rx buffer to the list: */
  _prev = *__prev;
  *_prev = rx_buffer;
  _prev = &TME_I825X6_RX_BUFFER_NEXT(rx_buffer);
  *__prev = _prev;

  /* return the new buffer: */
  return (rx_buffer);
}

/* given a bus address and a size, this adds rx buffers to a list: */
static struct tme_i825x6_rx_buffer *
_tme_i825x6_rx_buffers_add(struct tme_i825x6 *i825x6,
			   tme_uint32_t address_init,
			   tme_uint32_t size,
			   struct tme_i825x6_rx_buffer ***__prev)
{
  struct tme_i825x6_rx_buffer *rx_buffer, **_prev;
  struct tme_bus_tlb *tlb, tlb_local;
  struct tme_bus_connection *conn_bus;
  tme_bus_addr32_t count_minus_one, count;
  tme_bus_addr32_t tlb_addr_last;
  int err;

  /* recover the rx buffers list: */
  _prev = *__prev;

  /* there isn't a last rx buffer yet: */
  rx_buffer = NULL;

  /* loop while we have more addresses to cover: */
  for (; size > 0; ) {

    /* hash this address into a TLB entry: */
    tlb = _tme_i825x6_tlb_hash(i825x6,
			       address_init,
			       TME_BUS_CYCLE_WRITE);

    /* busy this TLB entry: */
    tme_bus_tlb_busy(tlb);
    
    /* if this TLB entry is invalid, or doesn't cover this address, or
       if it doesn't allow writing, reload it: */
    tlb_addr_last = tlb->tme_bus_tlb_addr_last;
    if (tme_bus_tlb_is_invalid(tlb)
	|| address_init < (tme_bus_addr32_t) tlb->tme_bus_tlb_addr_first
	|| address_init > tlb_addr_last
	|| (tlb->tme_bus_tlb_emulator_off_write == TME_EMULATOR_OFF_UNDEF
	    && !(tlb->tme_bus_tlb_cycles_ok & TME_BUS_CYCLE_WRITE))) {

      /* unbusy this TLB entry for filling: */
      tme_bus_tlb_unbusy_fill(tlb);
      
      /* pass this TLB's token: */
      tlb_local.tme_bus_tlb_token = tlb->tme_bus_tlb_token;
      
      /* get our bus connection: */
      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
				                i825x6->tme_i825x6_device.tme_bus_device_connection,
				                &i825x6->tme_i825x6_device.tme_bus_device_connection_rwlock);
      
      /* unlock the mutex: */
      tme_mutex_unlock(&i825x6->tme_i825x6_mutex);
      
      /* reload the TLB entry: */
      err = (*conn_bus->tme_bus_tlb_fill)
	(conn_bus,
	 &tlb_local,
	 address_init,
	 TME_BUS_CYCLE_WRITE);
      
      /* lock the mutex: */
      tme_mutex_lock(&i825x6->tme_i825x6_mutex);
      
      /* XXX we could create a poison frame chunk instead of
	 aborting: */
      if (err != TME_OK) {
	abort();
      }
      
      /* store the TLB entry: */
      *tlb = tlb_local;
      
      /* loop to check the newly filled TLB entry: */
      continue;
    }
    
    /* see how many addresses we can cover with this TLB entry,
       starting at this address: */
    count_minus_one = (tlb_addr_last - address_init);
    count_minus_one = TME_MIN(count_minus_one,
				(size - 1));
    count = count_minus_one + 1;
    assert (count != 0);
    
    /* allocate another rx buffer: */
    rx_buffer = _tme_i825x6_rx_buffer_new(i825x6, &_prev);

    /* if this TLB entry allows fast writing: */
    if (tlb->tme_bus_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF) {

      /* if this rx buffer was previously a slow rx buffer, free its
	 chunk buffer: */
      if (rx_buffer->tme_i825x6_rx_buffer_rb_address != TME_I825X6_RU_ADDRESS_UNDEF) {
	tme_free(rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes);
      }

      /* this is a fast rx buffer: */
      rx_buffer->tme_i825x6_rx_buffer_rb_address = TME_I825X6_RU_ADDRESS_UNDEF;
      rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes
	/* XXX FIXME - this breaks volatile: */
	= (tme_uint8_t *) tlb->tme_bus_tlb_emulator_off_write + address_init;

      /* unbusy the TLB: */
      /* XXX FIXME - this is not thread-safe: */
      tme_bus_tlb_unbusy(tlb);
    }

    /* otherwise, this TLB entry does not allow fast writing: */
    else {

      /* if this rx buffer was previously a fast rx buffer,
	 allocate a new chunk buffer: */
      if (rx_buffer->tme_i825x6_rx_buffer_rb_address == TME_I825X6_RU_ADDRESS_UNDEF) {
	rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes
	  = tme_new(tme_uint8_t, 
		    count);
      }

      /* otherwise, if this rx buffer was previously a slow receive
	 buffer, with a chunk buffer smaller than what we need,
	 reallocate the chunk buffer: */
      else if (rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes_count 
	       < count) {
	rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes
	  = tme_renew(tme_uint8_t, 
		      rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes, 
		      count);
      }
      
      /* this is a slow frame chunk: */
      rx_buffer->tme_i825x6_rx_buffer_rb_address = address_init;
    }

    /* finish this rx buffer: */
    rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes_count = count;
    rx_buffer->tme_i825x6_rx_buffer_rbd_address = TME_I825X6_RU_ADDRESS_UNDEF;    
    
    /* update the address and size: */
    address_init += count;
    size -= count;
  }

  /* update the rx buffers list end: */
  *__prev = _prev;

  /* return the last rx buffer: */
  return (rx_buffer);
}

/* this refills the Free Buffer List: */
static tme_uint16_t
_tme_i825x6_fbl_refill(struct tme_i825x6 *i825x6, int from_rfd)
{
  struct tme_i825x6_rx_buffer *rx_buffer, **_prev;
  tme_uint32_t fbl_size;
  tme_uint32_t rbd_offset_address, rbd_address, rb_address;
  tme_uint16_t rbd_offset, rbd_offset_first;
  tme_uint16_t el_p_size;
  tme_uint16_t size;
  tme_uint32_t value32;
  tme_uint16_t value16;
  int rc;

  /* assume that there are no free Receive Buffer Descriptors: */
  rbd_offset_first = TME_I825X6_RU_OFFSET_UNDEF;

  /* find the end of the Free Buffer List: */
  for (_prev = &i825x6->tme_i825x6_fbl;
       (rx_buffer = *_prev) != NULL;
       _prev = &TME_I825X6_RX_BUFFER_NEXT(rx_buffer)) {

    /* if we don't have the first free Receive Buffer Descriptor yet,
       and this is the last rx buffer for a Receive Buffer, its
       Receive Buffer Descriptor is the first free one: */
    rbd_address = rx_buffer->tme_i825x6_rx_buffer_rbd_address;
    if (rbd_offset_first == TME_I825X6_RU_OFFSET_UNDEF
	&& rbd_address != TME_I825X6_RU_ADDRESS_UNDEF) {
      rbd_offset_first = rbd_address - i825x6->tme_i825x6_scb_base;
    }
  }

  /* get the address of the next RBD offset: */
  rbd_offset_address = i825x6->tme_i825x6_rbd_offset_address;

  /* stop now if the address of the next RBD offset is undefined: */
  if (rbd_offset_address == TME_I825X6_RU_ADDRESS_UNDEF) {
    return (rbd_offset_first);
  }

  /* get the current size of the Free Buffer List: */
  fbl_size = i825x6->tme_i825x6_fbl_size;

  /* turn Receive Buffers into rx buffers on the Free Buffer List,
     until the Free Buffer List has a maximum-sized Ethernet frame's
     worth of buffers: */
  for (; fbl_size < TME_ETHERNET_FRAME_MAX; ) {

    /* read in the Receive Buffer Descriptor offset: */
    TME_I825X6_READ16(rbd_offset_address,
		      rbd_offset);

    /* if this Receive Buffer Descriptor offset is in an RFD, stop if
       the offset is all-bits-one, indicating no RBDs: */
    if (from_rfd
	&& rbd_offset == TME_I825X6_RU_OFFSET_UNDEF) {
      break;
    }
    from_rfd = FALSE;

    /* if we don't have the first free Receive Buffer Descriptor yet,
       this is it: */
    if (rbd_offset_first == TME_I825X6_RU_OFFSET_UNDEF) {
      rbd_offset_first = rbd_offset;
    }

    /* make the Receive Buffer Descriptor address: */
    rbd_address = i825x6->tme_i825x6_scb_base + rbd_offset;

    /* read in the Receive Buffer address: */
    TME_I82586_READ24((rbd_address
		       + TME_I82586_RBD_RB_ADDRESS),
		      rb_address);

    /* read in the EL_P_SIZE field: */
    TME_I825X6_READ16((rbd_address
		       + TME_I82586_RBD_EL_P_SIZE),
		      el_p_size);

    /* get the size of this Receive Buffer: */
    size = el_p_size & TME_I825X6_RBD_SIZE_MASK;

    /* if this Receive Buffer has zero size, stop now.  this can
       happen with NetBSD 1.6.x, which zeroes and reinitializes the
       memory for the i825x6 without stopping the Receive Unit: */
    if (size == 0) {

      /* log a complaint: */
      tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	      0, EBADF,
	      (&i825x6->tme_i825x6_element->tme_element_log_handle,
	       _("caught an empty Receive Buffer")));

      break;
    }

    /* add this Receive Buffer to the rx buffers: */
    rx_buffer = _tme_i825x6_rx_buffers_add(i825x6,
					   rb_address,
					   size,
					   &_prev);

    /* on the last rx buffer for a Receive Buffer, set the address of
       the Receive Buffer Descriptor, so we can update its size field: */
    rx_buffer->tme_i825x6_rx_buffer_rbd_address = rbd_address;

    /* update the amount of space on the Free Buffer list: */
    fbl_size += size;

    /* if this is the last Receive Buffer Descriptor: */
    if (el_p_size & TME_I825X6_RBD_EL) {

      /* the address of the next RBD offset is undefined: */
      rbd_offset_address = TME_I825X6_RU_ADDRESS_UNDEF;
      
      /* stop now: */
      break;
    }

    /* get the address of the next RBD offset: */
    rbd_offset_address
      = (rbd_address
	 + TME_I82586_RBD_RBD_OFFSET);
  }

  /* terminate the Free Buffer List: */
  *_prev = NULL;

  /* save the current size of the Free Buffer List: */
  i825x6->tme_i825x6_fbl_size = fbl_size;

  /* save the address of the next RBD offset: */
  i825x6->tme_i825x6_rbd_offset_address = rbd_offset_address;

  /* return the address of the first free Receive Buffer Descriptor: */
  return (rbd_offset_first);
}

/* this aborts the Receive Unit: */
static void
_tme_i825x6_abort_ru(struct tme_i825x6 *i825x6)
{
  struct tme_i825x6_rx_buffer *rx_buffer;

  tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	  100, TME_OK,
	  (&i825x6->tme_i825x6_element->tme_element_log_handle,
	   "RU abort"));

  /* free the Free Buffer List: */
  for (rx_buffer = i825x6->tme_i825x6_fbl;
       rx_buffer != NULL;
       rx_buffer = _tme_i825x6_rx_buffer_free(i825x6, rx_buffer));
  
  /* the Receive Unit now has no resources: */
  i825x6->tme_i825x6_rfd_address = TME_I825X6_RU_ADDRESS_UNDEF;
  i825x6->tme_i825x6_rbd_offset_address = TME_I825X6_RU_ADDRESS_UNDEF;	
  i825x6->tme_i825x6_fbl = NULL;
  i825x6->tme_i825x6_fbl_size = 0;
}

/* this does a DMA directly into transmit frame chunks: */
static int
_tme_i825x6_chunks_dma_tx(struct tme_i825x6 *i825x6,
			  struct tme_ethernet_frame_chunk *frame_chunks,
			  tme_uint32_t address,
			  tme_uint32_t size)
{
  tme_uint32_t count;
  int rc;

  /* while we have bytes left to DMA: */
  for (; size > 0; ) {
    
    /* get the count of bytes to copy in this iteration: */
    count = frame_chunks->tme_ethernet_frame_chunk_bytes_count;
    if (count == 0) {
      break;
    }
    count = TME_MIN(count, size);

    /* do the copy: */
    /* XXX FIXME this assumes an i82586: */
    rc = tme_bus_device_dma_read_16(&i825x6->tme_i825x6_device,
				    address,
				    count,
				    frame_chunks->tme_ethernet_frame_chunk_bytes,
				    TME_I825X6_LOCKS_DEFAULT);
    if (rc != TME_OK) {
      return (rc);
    }
    
    /* update: */
    size -= count;
    frame_chunks->tme_ethernet_frame_chunk_bytes += count;
    if ((frame_chunks->tme_ethernet_frame_chunk_bytes_count -= count) == 0
	&& frame_chunks->tme_ethernet_frame_chunk_next != NULL) {
      *frame_chunks = *frame_chunks->tme_ethernet_frame_chunk_next;
    }
  }

  /* success: */
  return (TME_OK);
}

/* this does a memcpy directly into transmit frame chunks: */
static void
_tme_i825x6_chunks_mem_tx(struct tme_ethernet_frame_chunk *frame_chunks,
			  const tme_uint8_t *data,
			  unsigned int size)
{
  unsigned int count;

  /* while we have bytes left to copy: */
  for (; size > 0; ) {
    
    /* get the count of bytes to copy in this iteration: */
    count = frame_chunks->tme_ethernet_frame_chunk_bytes_count;
    if (count == 0) {
      break;
    }
    count = TME_MIN(count, size);

    /* do the copy: */
    memcpy(frame_chunks->tme_ethernet_frame_chunk_bytes,
	   data,
	   count);
    
    /* update: */
    size -= count;
    frame_chunks->tme_ethernet_frame_chunk_bytes += count;
    if ((frame_chunks->tme_ethernet_frame_chunk_bytes_count -= count) == 0
	&& frame_chunks->tme_ethernet_frame_chunk_next != NULL) {
      *frame_chunks = *frame_chunks->tme_ethernet_frame_chunk_next;
    }
  }
}

/* this is called to handle a Channel Attention (CA) callout: */
static tme_uint16_t
_tme_i825x6_callout_ca(struct tme_i825x6 *i825x6, tme_uint16_t stat_cus_rus_t)
{
  tme_uint8_t scp[TME_I825X6_SCP_SIZE];
  tme_uint32_t iscp_address, rfd_offset;
  tme_uint8_t iscp[TME_I825X6_ISCP_SIZE];
  tme_uint16_t ack_cuc_r_ruc;
  tme_uint16_t cuc;
  tme_uint16_t value16;
  tme_uint16_t rbd_offset_first;
  int rc;

  /* if this CA follows RESET: */
  if (i825x6->tme_i825x6_ca_follows_reset) {
    
    /* the next CA will not follow RESET: */
    i825x6->tme_i825x6_ca_follows_reset = FALSE;
    
    /* read in the SCP: */
    TME_I825X6_READ(TME_I825X6_SCP_ADDRESS, scp);
    
    /* check the SYSBUS byte: */
    assert ((scp[TME_I825X6_SCP_SYSBUS]
	     & TME_I825X6_SCP_SYSBUS_MODE_MASK)
	    == TME_I825X6_SCP_SYSBUS_MODE_82586);
    
    /* get the ISCP address: */
    iscp_address = TME_I82586_GET24(&scp[TME_I825X6_SCP_ISCP_ADDRESS]);
    
    /* read in the ISCP: */
    TME_I825X6_READ(iscp_address, iscp);
    
    /* get the SCB base and SCB address: */
    i825x6->tme_i825x6_scb_base = TME_I82586_GET24(&iscp[TME_I82586_ISCP_SCB_BASE]);
    i825x6->tme_i825x6_scb_address
      = (i825x6->tme_i825x6_scb_base
	 + TME_I825X6_GET16(&iscp[TME_I82586_ISCP_SCB_OFFSET]));

    /* "The 82596 clears BUSY": */
    iscp[TME_I825X6_ISCP_BUSY] = 0;
    TME_I825X6_WRITE((iscp_address
		      + TME_I825X6_ISCP_BUSY),
		     iscp[TME_I825X6_ISCP_BUSY]);
    
    /* "The 82596 ... sets CX and CNR to equal 1 in the SCB": */
    stat_cus_rus_t
      = (TME_I825X6_SCB_STAT_CX
	 | TME_I825X6_SCB_STAT_CNA
	 | TME_I825X6_SCB_CUS_IDLE
	 | TME_I825X6_SCB_RUS_IDLE);
    
    /* "The 82596 ... sends an interrupt to the CPU": */
    i825x6->tme_i825x6_callout_flags = TME_I825X6_CALLOUTS_RUNNING | TME_I825X6_CALLOUT_INT;
  }

  /* otherwise, this CA does not follow RESET: */
  else {
    
    /* read in the SCB command word: */
    TME_I825X6_READ16((i825x6->tme_i825x6_scb_address
		       + TME_I825X6_SCB_ACK_CUC_R_RUC),
		      ack_cuc_r_ruc);
    
    tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	    100, TME_OK,
	    (&i825x6->tme_i825x6_element->tme_element_log_handle,
	     "SCB command 0x%04x",
	     ack_cuc_r_ruc));

    /* handle a Reset: */
    if (ack_cuc_r_ruc & TME_I825X6_SCB_RESET) {
      _tme_i825x6_reset(i825x6);
      return (TME_I825X6_CA_RESET);
    }
      
    /* clear any acknowledged Status bits: */
    stat_cus_rus_t
      &= ~(ack_cuc_r_ruc
	   & TME_I825X6_SCB_STAT_MASK);

    /* if the Command Unit Command isn't a NOP: */
    cuc = (ack_cuc_r_ruc
	   & TME_I825X6_SCB_CUC_MASK);
    if (cuc != TME_I825X6_SCB_CUC_NOP) {

      /* set this Command Unit Command: */
      i825x6->tme_i825x6_cuc = cuc;
      
      /* if the Command Unit Command is an Abort, or if the Command
	 Unit isn't already Active, run the Command Unit: */
      if ((cuc == TME_I825X6_SCB_CUC_ABORT)
	  || ((stat_cus_rus_t
	       & TME_I825X6_SCB_CUS_MASK)
	      != TME_I825X6_SCB_CUS_ACTIVE)) {
	i825x6->tme_i825x6_callout_flags |= TME_I825X6_CALLOUT_CU;
      }
    }
    
    /* dispatch on the Receive Unit command: */
    switch (ack_cuc_r_ruc & TME_I825X6_SCB_RUC_MASK) {
      
    case TME_I825X6_SCB_RUC_NOP:
      break;
      
    case TME_I825X6_SCB_RUC_START:
      
      /* if the Receive Unit is not Idle: */
      switch (stat_cus_rus_t & TME_I82586_SCB_RUS_MASK) {
      case TME_I825X6_SCB_RUS_READY:
      case TME_I825X6_SCB_RUS_SUSPENDED:
      case TME_I825X6_SCB_RUS_ERESOURCE:

	/* abort the Receive Unit: */
	_tme_i825x6_abort_ru(i825x6);
	break;

      case TME_I825X6_SCB_RUS_IDLE:
	break;

      default:
	abort();
      }

      /* the Receive Unit must have no resources: */
      assert (i825x6->tme_i825x6_rfd_address == TME_I825X6_RU_ADDRESS_UNDEF);
      assert (i825x6->tme_i825x6_rbd_offset_address == TME_I825X6_RU_ADDRESS_UNDEF);
      assert (i825x6->tme_i825x6_fbl == NULL && i825x6->tme_i825x6_fbl_size == 0);

      /* get the RFD offset from the SCB: */
      TME_I825X6_READ16((i825x6->tme_i825x6_scb_address
			 + TME_I82586_SCB_RFA_OFFSET),
			rfd_offset);

      /* if the RFD offset is defined: */
      if (rfd_offset != TME_I825X6_RU_OFFSET_UNDEF) {
	
	/* set the RFD address: */
	i825x6->tme_i825x6_rfd_address
	  = (i825x6->tme_i825x6_scb_base
	     + rfd_offset);
	
	/* set the RBD offset address: */
	i825x6->tme_i825x6_rbd_offset_address
	  = (i825x6->tme_i825x6_rfd_address
	     + TME_I82586_RFD_RBD_OFFSET);
	
	/* refill the Free Buffer List: */
	rbd_offset_first = _tme_i825x6_fbl_refill(i825x6, TRUE);

	tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
		100, TME_OK,
		(&i825x6->tme_i825x6_element->tme_element_log_handle,
		 "RU start RFD 0x%06x RBD 0x%04x",
		 i825x6->tme_i825x6_rfd_address,
		 rbd_offset_first));
      }

      /* FALLTHROUGH: */
    case TME_I825X6_SCB_RUC_RESUME:

      /* the Receive Unit is now Ready: */
      stat_cus_rus_t
	= ((stat_cus_rus_t
	    & ~TME_I82586_SCB_RUS_MASK)
	   | TME_I825X6_SCB_RUS_READY);
      break;
      
    case TME_I825X6_SCB_RUC_SUSPEND:	  
      
      /* the Receive Unit is now Suspended: */
      stat_cus_rus_t
	= ((stat_cus_rus_t
	    & ~TME_I82586_SCB_RUS_MASK)
	   | TME_I825X6_SCB_RUS_SUSPENDED);
      break;
      
    case TME_I825X6_SCB_RUC_ABORT:

      /* abort the Receive Unit: */
      _tme_i825x6_abort_ru(i825x6);

      /* the Receive Unit is now Idle: */
      stat_cus_rus_t
	= ((stat_cus_rus_t
	    & ~TME_I82586_SCB_RUS_MASK)
	   | TME_I825X6_SCB_RUS_IDLE);
      break;
      
    default:
      abort();
    }
  }
  
  /* always clear the SCB command word after a CA: */
  TME_I825X6_WRITE16((i825x6->tme_i825x6_scb_address
		      + TME_I825X6_SCB_ACK_CUC_R_RUC),
		     0);

  /* return the current status word: */
  return (stat_cus_rus_t);
}

/* this is called to handle a Command Unit (CU) callout: */
static tme_uint16_t
_tme_i825x6_callout_cu(struct tme_i825x6 *i825x6, tme_uint16_t stat_cus_rus_t)
{
  tme_uint16_t cuc;
  tme_uint16_t el_s_i_cmd;
  tme_uint16_t mc_count;
  tme_uint8_t config_bytes[12];
  unsigned int config_byte_count;
  unsigned int callouts;
  tme_uint16_t value16;
  int rc;

  /* get the SCB Command Unit Command: */
  cuc = i825x6->tme_i825x6_cuc;

  /* if the Command Unit was active, complete the command that it
     was working on: */
  if ((stat_cus_rus_t
       & TME_I825X6_SCB_CUS_MASK)
      == TME_I825X6_SCB_CUS_ACTIVE) {
    
    /* finish the CB status word.  clear B, and set C.  if the
       command was aborted, clear OK and set A, else set OK and
       clear A: */
    i825x6->tme_i825x6_c_b_ok_a
      = ((i825x6->tme_i825x6_c_b_ok_a
	  & ~TME_I825X6_FLAG_B)
	 | TME_I825X6_FLAG_C
	 | TME_I825X6_FLAG_OK
	 | TME_I825X6_CB_A);
    i825x6->tme_i825x6_c_b_ok_a
      ^= ((cuc
	   == TME_I825X6_SCB_CUC_ABORT)
	  ? TME_I825X6_FLAG_OK
	  : TME_I825X6_CB_A);
    
    /* write the CB status word: */
    TME_I825X6_WRITE16((i825x6->tme_i825x6_cb_address
			+ TME_I825X6_CB_C_B_OK_A),
		       i825x6->tme_i825x6_c_b_ok_a);
    
    /* if this command had the I bit set, set CX: */
    if (i825x6->tme_i825x6_el_s_i_cmd & TME_I825X6_CB_I) {
      stat_cus_rus_t |= TME_I825X6_SCB_STAT_CX;
    }

    tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	    100, TME_OK,
	    (&i825x6->tme_i825x6_element->tme_element_log_handle,
	     "CU finish 0x%06x status 0x%04x",
	     i825x6->tme_i825x6_cb_address,
	     i825x6->tme_i825x6_c_b_ok_a));
  }

  /* dispatch on the Command Unit command: */
  switch (cuc) {
    
  case TME_I825X6_SCB_CUC_NOP:
    break;
    
  case TME_I825X6_SCB_CUC_START:
    
    /* get the CBL address: */
    TME_I825X6_READ16((i825x6->tme_i825x6_scb_address
		       + TME_I82586_SCB_CBL_OFFSET),
		      i825x6->tme_i825x6_cb_address_next);
    i825x6->tme_i825x6_cb_address_next += i825x6->tme_i825x6_scb_base;
    
    /* FALLTHROUGH */
  case TME_I825X6_SCB_CUC_RESUME:
    
    /* the Command Unit is now active: */
    stat_cus_rus_t
      = ((stat_cus_rus_t
	  & ~TME_I825X6_SCB_CUS_MASK)
	 | TME_I825X6_SCB_CUS_ACTIVE);
    break;
    
  case TME_I825X6_SCB_CUC_ABORT:
    
    /* the Command Unit is now Idle: */
    stat_cus_rus_t
      = ((stat_cus_rus_t
	  & ~TME_I825X6_SCB_CUS_MASK)
	 | TME_I825X6_SCB_CUS_IDLE);
    break;
    
  case TME_I825X6_SCB_CUC_SUSPEND:
    
    /* the Command Unit is now Suspended: */
    stat_cus_rus_t
      = ((stat_cus_rus_t
	  & ~TME_I825X6_SCB_CUS_MASK)
	 | TME_I825X6_SCB_CUS_SUSPENDED);
    break;
    
  default: 
    abort();
  }
  
  /* if the Command Unit is not active, return now: */
  if ((stat_cus_rus_t
       & TME_I825X6_SCB_CUS_MASK)
      != TME_I825X6_SCB_CUS_ACTIVE) {
    return (stat_cus_rus_t);
  }
    
  /* advance to the next command: */
  i825x6->tme_i825x6_cb_address = i825x6->tme_i825x6_cb_address_next;
  
  /* if there is no next command, the Command Unit is now Idle: */
  if (i825x6->tme_i825x6_cb_address == TME_I825X6_CB_ADDRESS_IDLE) {
    tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	    100, TME_OK,
	    (&i825x6->tme_i825x6_element->tme_element_log_handle,
	     "CU idle"));
    stat_cus_rus_t
      = ((stat_cus_rus_t
	  & ~TME_I825X6_SCB_CUS_MASK)
	 | TME_I825X6_SCB_CUS_IDLE);
    return (stat_cus_rus_t);
  }

  /* start the CB status word: */
  i825x6->tme_i825x6_c_b_ok_a = TME_I825X6_FLAG_B;
  TME_I825X6_WRITE16((i825x6->tme_i825x6_cb_address
		      + TME_I825X6_CB_C_B_OK_A),
		     i825x6->tme_i825x6_c_b_ok_a);
  
  /* get the CB command word: */
  TME_I825X6_READ16((i825x6->tme_i825x6_cb_address
		     + TME_I825X6_CB_EL_S_I_CMD),
		    i825x6->tme_i825x6_el_s_i_cmd);
  el_s_i_cmd = i825x6->tme_i825x6_el_s_i_cmd;
  
  tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	  100, TME_OK,
	  (&i825x6->tme_i825x6_element->tme_element_log_handle,
	   "CU start 0x%06x el_s_i_cmd 0x%04x",
	   i825x6->tme_i825x6_cb_address,
	   el_s_i_cmd));

  /* if EL is set, there is no next Command Block, and when
     the Command Unit tries to execute the next command it
     will go Idle: */
  if (el_s_i_cmd & TME_I825X6_FLAG_EL) {
    i825x6->tme_i825x6_cb_address_next = TME_I825X6_CB_ADDRESS_IDLE;
  }
  
  /* otherwise, get the address of the next Command Block: */
  else {
    TME_I825X6_READ16((i825x6->tme_i825x6_cb_address
		       + TME_I82586_CB_LINK_OFFSET),
		      i825x6->tme_i825x6_cb_address_next);
    i825x6->tme_i825x6_cb_address_next += i825x6->tme_i825x6_scb_base;
  }
  
  /* if S is set, after the Command Unit executes this command
     it will Suspend, else it will stay Active: */
  i825x6->tme_i825x6_cuc
    = ((el_s_i_cmd & TME_I825X6_FLAG_S)
       ? TME_I825X6_SCB_CUC_SUSPEND
       : TME_I825X6_SCB_CUC_NOP);
  
  /* assume that this command will complete now: */
  callouts = TME_I825X6_CALLOUT_CU;
  
  /* dispatch on this command: */
  switch (el_s_i_cmd & TME_I825X6_CB_CMD_MASK) {
    
  case TME_I825X6_CB_CMD_NOP:
    break;
    
  case TME_I825X6_CB_CMD_SETUP_IA:
    
    /* read in the new Individual Address, into the second element of
       the addresses array: */
    rc = tme_bus_device_dma_read_16(&i825x6->tme_i825x6_device,
				    (i825x6->tme_i825x6_cb_address
				     + TME_I82586_CB_X),
				    TME_ETHERNET_ADDR_SIZE,
				    (i825x6->tme_i825x6_addresses
				     + TME_ETHERNET_ADDR_SIZE),
				    TME_I825X6_LOCKS_DEFAULT);
    assert (rc == TME_OK);
    
    /* call out an Ethernet configuration change: */
    callouts |= TME_I825X6_CALLOUT_CONFIG;
    break;
    
  case TME_I825X6_CB_CMD_CONFIGURE:
    
    /* read in bytes 0 and 1 of the configuration: */
    rc = tme_bus_device_dma_read_16(&i825x6->tme_i825x6_device,
				    (i825x6->tme_i825x6_cb_address
				     + TME_I82586_CB_X),
				    sizeof(tme_uint16_t),
				    config_bytes,
				    TME_I825X6_LOCKS_DEFAULT);
    assert (rc == TME_OK);
    
    /* "In the 82586 mode the maximum number of configuration bytes
       is 12.  Any number larger than 12 will be reduced to 12 and
       any number less than 4 will be increased to 4." */
    config_byte_count = (config_bytes[0] & 0x0f);
    if (config_byte_count < 4) {
      config_byte_count = 4;
    }
    else if (config_byte_count > 12) {
      config_byte_count = 12;
    }
    
    /* read in the remaining bytes of the configuration: */
    rc = tme_bus_device_dma_read_16(&i825x6->tme_i825x6_device,
				    (i825x6->tme_i825x6_cb_address
				     + TME_I82586_CB_X
				     + sizeof(tme_uint16_t)),
				    (config_byte_count
				     - sizeof(tme_uint16_t)),
				    (config_bytes
				     + sizeof(tme_uint16_t)),
				    TME_I825X6_LOCKS_DEFAULT);
    assert (rc == TME_OK);
    
    /* byte 3: */
    if (config_byte_count > 3) {
      
      /* AL-LOC: */
      i825x6->tme_i825x6_al_loc = config_bytes[3] & 0x08;
      
      /* Address Length: */
      if ((config_bytes[3] & 0x07) != TME_ETHERNET_ADDR_SIZE) {
	abort();
      }
    }
    
    /* byte 8: */
    if (config_byte_count > 8) {
      
      /* Promiscuous Mode: */
      i825x6->tme_i825x6_prm = config_bytes[8] & 0x01;
      
      /* call out an Ethernet configuration change: */
      callouts |= TME_I825X6_CALLOUT_CONFIG;
    }
    break;
    
  case TME_I825X6_CB_CMD_SETUP_MC:
    
    /* read in the MC COUNT byte count: */
    TME_I825X6_READ16((i825x6->tme_i825x6_cb_address
		       + TME_I82586_CB_X),
		      mc_count);
    mc_count -= (mc_count % TME_ETHERNET_ADDR_SIZE);
    
    /* reallocate the addresses list, which is always at least two
       elements long - the broadcast address and the individual
       address: */
    i825x6->tme_i825x6_address_count = (2 + (mc_count / TME_ETHERNET_ADDR_SIZE));
    i825x6->tme_i825x6_addresses
      = tme_renew(tme_uint8_t, 
		  i825x6->tme_i825x6_addresses,
		  (i825x6->tme_i825x6_address_count
		   * TME_ETHERNET_ADDR_SIZE));
    
    /* read in the new Multicast addresses: */
    rc = tme_bus_device_dma_read_16(&i825x6->tme_i825x6_device,
				    (i825x6->tme_i825x6_cb_address
				     + TME_I82586_CB_X
				     + sizeof(mc_count)),
				    mc_count,
				    (i825x6->tme_i825x6_addresses
				     + (2
					* TME_ETHERNET_ADDR_SIZE)),
				    TME_I825X6_LOCKS_DEFAULT);
    assert (rc == TME_OK);
    
    /* call out an Ethernet configuration change: */
    callouts |= TME_I825X6_CALLOUT_CONFIG;
    break;
    
  case TME_I825X6_CB_CMD_TRANSMIT:
    
    /* call out only a control change; this command is not
       completing now: */
    callouts = TME_I825X6_CALLOUT_CTRL;
    break;
    
  case TME_I825X6_CB_CMD_TDR:

    /* write a successful TDR status: */
    TME_I825X6_WRITE16((i825x6->tme_i825x6_cb_address
			+ TME_I82586_CB_X),
		       TME_I82586_TDR_STATUS_OK);
    break;

  case TME_I825X6_CB_CMD_DUMP:
    abort();

  case TME_I825X6_CB_CMD_DIAGNOSE:
    break;
  }
  
  /* add to the callouts and return the current status: */
  i825x6->tme_i825x6_callout_flags |= callouts;
  return (stat_cus_rus_t);
}

/* this is called to handle a Receive Unit (RU) callout: */
static tme_uint16_t
_tme_i825x6_callout_ru(struct tme_i825x6 *i825x6, tme_uint16_t stat_cus_rus_t)
{
  struct tme_ethernet_connection *conn_eth;
  tme_ethernet_fid_t frame_id;
  tme_uint16_t el_s_sf;
  tme_uint16_t eof_f_act_count;
  tme_uint16_t c_b_ok_status;
  struct tme_i825x6_rx_buffer *rx_buffers, *rx_buffer, **_prev;
  unsigned int rbd_size, rx_buffer_size;
  tme_uint32_t rfd_address;
  tme_uint16_t rbd_offset_next;
  tme_uint16_t discards;
  int resid;
  int rc;
  tme_uint16_t value16;

  /* start a list of rx buffers: */
  rx_buffers = NULL;
  _prev = &rx_buffers;

  /* start counting the number of bytes in the first Receive Buffer: */
  rbd_size = 0;

  /* assume that we have no Receive Frame Descriptor: */
  rfd_address = TME_I825X6_RU_ADDRESS_UNDEF;
  el_s_sf = 0;

  /* if the Receive Unit is Active and we have a Receive Frame Descriptor: */
  if (((stat_cus_rus_t & TME_I82586_SCB_RUS_MASK)
       == TME_I825X6_SCB_RUS_READY)
      && ((rfd_address = i825x6->tme_i825x6_rfd_address)
	  != TME_I825X6_RU_ADDRESS_UNDEF)) {

    /* get the flags word: */
    TME_I825X6_READ16((rfd_address
		       + TME_I825X6_RFD_EL_S_SF),
		      el_s_sf);
    
    /* if AL-LOC is set to zero, the Ethernet/802.3 MAC header will be
       received into the Receive Frame Descriptor: */
    if (i825x6->tme_i825x6_al_loc == 0) {

      /* make a set of rx buffers out of the Ethernet header part of
	 the Receive Frame Descriptor: */
      _tme_i825x6_rx_buffers_add(i825x6,
				 (rfd_address
				  + TME_I82586_RFD_RBD_ETH_HEADER),
				 TME_ETHERNET_HEADER_SIZE,
				 &_prev);

      /* account for the Receive Frame Descriptor header bytes in the
	 Free Buffer List space and Receive Buffer size calculations: */
      i825x6->tme_i825x6_fbl_size += TME_ETHERNET_HEADER_SIZE;
      rbd_size = 0 - TME_ETHERNET_HEADER_SIZE;
    }

    /* take the Free Buffer List: */
    *_prev = i825x6->tme_i825x6_fbl;
  }

  /* get the Ethernet connection: */
  conn_eth = i825x6->tme_i825x6_eth_connection;

  /* unlock the mutex: */
  tme_mutex_unlock(&i825x6->tme_i825x6_mutex);
  
  /* do the callout: */
  resid = (conn_eth == NULL
	   ? 0
	   : ((*conn_eth->tme_ethernet_connection_read)
	      (conn_eth,
	       &frame_id,
	       &rx_buffers->tme_i825x6_rx_buffer_frame_chunk,
	       TME_ETHERNET_READ_NEXT)));
    
  /* lock the mutex: */
  tme_mutex_lock(&i825x6->tme_i825x6_mutex);

  /* if the read failed: */
  if (resid <= 0) {

    /* convention dictates that we forget that the connection was
       readable, which we already have done by clearing the
       CALLOUT_READ flag: */
    return (stat_cus_rus_t);
  }

  /* mark that we need to loop to callout to read more frames: */
  i825x6->tme_i825x6_callout_flags |= TME_I825X6_CALLOUT_READ;

  /* return now if the Receive Unit is Idle: */
  if ((stat_cus_rus_t & TME_I82586_SCB_RUS_MASK)
      == TME_I825X6_SCB_RUS_IDLE) {
    return (stat_cus_rus_t);
  }
  
  tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	  100, TME_OK,
	  (&i825x6->tme_i825x6_element->tme_element_log_handle,
	   "RU RFD 0x%06x el_s_sf 0x%04x",
	   rfd_address,
	   el_s_sf));

  /* we must have received more than just headers: */
  assert (resid > TME_ETHERNET_HEADER_SIZE);

  /* if we have a Receive Frame Descriptor: */
  if (rfd_address != TME_I825X6_RU_ADDRESS_UNDEF) {

    /* walk the rx buffers, stopping early only if we have exhausted
       the Ethernet frame *and* we have updated the last Receive
       Buffer used to receive it: */
    for (rx_buffer = rx_buffers; 
	 (rx_buffer != NULL
	  && (resid > 0
	      || rbd_size > 0)); ) {

      /* calculate the number of bytes used in this rx buffer.  since
	 a single Receive Buffer can be split into many rx buffers,
	 the Ethernet frame may not fill all of them, so this can be
	 zero: */
      rx_buffer_size
	= TME_MIN((unsigned int) resid, 
		  rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes_count);

      /* this many more bytes have been received into this Receive Buffer: */
      rbd_size += rx_buffer_size;

      /* this many more bytes have been accounted for in the Ethernet frame: */
      resid -= rx_buffer_size;

      /* if this was a slow frame chunk: */
      if (rx_buffer->tme_i825x6_rx_buffer_rb_address
	  != TME_I825X6_RU_ADDRESS_UNDEF) {

	/* DMA the contents out: */
	rc = tme_bus_device_dma_write_16(&i825x6->tme_i825x6_device,
					 rx_buffer->tme_i825x6_rx_buffer_rb_address,
					 rx_buffer_size,
					 rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes,
					 TME_I825X6_LOCKS_DEFAULT);
	assert (rc == TME_OK);
      }

      /* if this was the last rx buffer for a Receive Buffer: */
      if (rx_buffer->tme_i825x6_rx_buffer_rbd_address
	  != TME_I825X6_RU_ADDRESS_UNDEF) {

	/* make the EOF_F_ACT_COUNT field: */
	assert (rbd_size > 0 && rbd_size <= TME_I825X6_RBD_ACT_COUNT_MASK);
	eof_f_act_count = ((resid == 0
			    ? TME_I825X6_RBD_EOF
			    : 0)
			   | TME_I825X6_RBD_F
			   | rbd_size);

	/* write the EOF_F_ACT_COUNT field out to the Receive Buffer
           Descriptor: */
	TME_I825X6_WRITE16((rx_buffer->tme_i825x6_rx_buffer_rbd_address
			    + TME_I825X6_RBD_EOF_F_ACT_COUNT),
			   eof_f_act_count);

	tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
		100, TME_OK,
		(&i825x6->tme_i825x6_element->tme_element_log_handle,
		 "RU RBD 0x%06x last-size 0x%04x eof_f_act_count 0x%04x",
		 rx_buffer->tme_i825x6_rx_buffer_rbd_address,
		 rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes_count,
		 eof_f_act_count));

	/* we're starting on the next Receive Buffer: */
	rbd_size = 0;
      }

      /* free this rx buffer and move to the next rx buffer: */
      i825x6->tme_i825x6_fbl_size -= rx_buffer->tme_i825x6_rx_buffer_frame_chunk.tme_ethernet_frame_chunk_bytes_count;
      rx_buffer = _tme_i825x6_rx_buffer_free(i825x6, rx_buffer);
    }

    /* update the Free Buffer List: */
    assert ((rx_buffer != NULL) == (i825x6->tme_i825x6_fbl_size != 0));
    i825x6->tme_i825x6_fbl = rx_buffer;

    /* make the C_B_OK_STATUS field: */
    c_b_ok_status = (TME_I825X6_FLAG_C
		     | (resid == 0
			? TME_I825X6_FLAG_OK
			: TME_I825X6_RFD_STATUS_RNR));
    
    /* write the C_B_OK_STATUS field in the RFD: */
    TME_I825X6_WRITE16((rfd_address
			+ TME_I825X6_RFD_C_B_OK_STATUS),
		       c_b_ok_status);
    
    /* signal an interrupt: */
    stat_cus_rus_t |= TME_I825X6_SCB_STAT_FR;

    /* if EL is set, there is no next Receive Frame Descriptor,
       and when the Receive Unit tries to receive another frame
       it will go No Resources: */
    if (el_s_sf & TME_I825X6_FLAG_EL) {
      rfd_address = TME_I825X6_RU_ADDRESS_UNDEF;
    }

    /* otherwise, get the address of the next Receive Frame Descriptor: */
    else {
      TME_I825X6_READ16((rfd_address
			 + TME_I82586_RFD_LINK_OFFSET),
			rfd_address);
      rfd_address += i825x6->tme_i825x6_scb_base;
    }

    /* set the next RFD address: */
    i825x6->tme_i825x6_rfd_address = rfd_address;

    /* if there is a next RFD address: */
    if (rfd_address != TME_I825X6_RU_ADDRESS_UNDEF) {
      
      /* refill the Free Buffer List: */
      rbd_offset_next = _tme_i825x6_fbl_refill(i825x6, FALSE);
	
      /* write the offset of the first free Receive Buffer Descriptor
	 in the RBD Offset field in the RFD: */
      TME_I825X6_WRITE16((rfd_address
			  + TME_I82586_RFD_RBD_OFFSET),
			 rbd_offset_next);
    }
    
    /* if S is set, the Receive Unit becomes Suspended: */
    if (el_s_sf & TME_I825X6_FLAG_S) {
      
      /* the Receive Unit is now Suspended: */
      stat_cus_rus_t
	= ((stat_cus_rus_t
	    & ~TME_I82586_SCB_RUS_MASK)
	   | TME_I825X6_SCB_RUS_SUSPENDED);
    }
  }

  /* otherwise, we had no Receive Frame Descriptor, so we had
     to discard this packet entirely: */
  else {
    
    /* account for the discarded packet: */
    TME_I825X6_READ16((i825x6->tme_i825x6_scb_address
		       + TME_I82586_SCB_ERRORS_RESOURCE),
		      discards);
    discards++;
    TME_I825X6_WRITE16((i825x6->tme_i825x6_scb_address
			+ TME_I82586_SCB_ERRORS_RESOURCE),
		       discards);
  }

  /* if we ran out of resources: */
  if (resid > 0) {

    /* the receiver is now out of resources: */
    stat_cus_rus_t = ((stat_cus_rus_t
		       & ~TME_I82586_SCB_RUS_MASK)
		      | TME_I825X6_SCB_RUS_ERESOURCE);
  }

  /* done: */
  return (stat_cus_rus_t);
}

/* the i825x6 callout function.  it must be called with the mutex locked: */
static void
_tme_i825x6_callout(struct tme_i825x6 *i825x6, int new_callouts)
{
  struct tme_ethernet_connection *conn_eth;
  struct tme_bus_connection *conn_bus;
  int callouts, later_callouts;
  unsigned int ctrl;
  struct tme_ethernet_config config;
  int rc;
  tme_uint16_t stat_cus_rus_t;
  tme_uint16_t value16;
  int int_asserted;
  unsigned int address_i;

  /* add in any new callouts: */
  i825x6->tme_i825x6_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (i825x6->tme_i825x6_callout_flags & TME_I825X6_CALLOUTS_RUNNING) {
    return;
  }

  /* callouts are now running: */
  i825x6->tme_i825x6_callout_flags |= TME_I825X6_CALLOUTS_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; (callouts = i825x6->tme_i825x6_callout_flags) & TME_I825X6_CALLOUTS_MASK; ) {

    /* clear the needed callouts: */
    i825x6->tme_i825x6_callout_flags = callouts & ~TME_I825X6_CALLOUTS_MASK;
    callouts &= TME_I825X6_CALLOUTS_MASK;

    /* get this card's connection: */
    conn_eth = i825x6->tme_i825x6_eth_connection;

    /* get the current SCB status word.  this word is referenced and
       updated throughout the body of this for loop; any changes made
       are written out at the bottom of the loop: */
    stat_cus_rus_t = i825x6->tme_i825x6_stat_cus_rus_t;

    /* if we need to handle a Channel Attention: */
    if (callouts & TME_I825X6_CALLOUT_CA) {
      stat_cus_rus_t = _tme_i825x6_callout_ca(i825x6, stat_cus_rus_t);
      if (stat_cus_rus_t == TME_I825X6_CA_RESET) {
	continue;
      }
    }

    /* if we need to run the Command Unit: */
    if (callouts & TME_I825X6_CALLOUT_CU) {
      stat_cus_rus_t = _tme_i825x6_callout_cu(i825x6, stat_cus_rus_t);
    }

    /* if we need to call out new control information: */
    if (callouts & TME_I825X6_CALLOUT_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (((stat_cus_rus_t
	    & TME_I825X6_SCB_CUS_MASK)
	   == TME_I825X6_SCB_CUS_ACTIVE)
	  && ((i825x6->tme_i825x6_el_s_i_cmd
	       & TME_I825X6_CB_CMD_MASK)
	      == TME_I825X6_CB_CMD_TRANSMIT)) {
	ctrl |= TME_ETHERNET_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&i825x6->tme_i825x6_mutex);
      
      /* do the callout: */
      rc = (conn_eth != NULL
	    ? ((*conn_eth->tme_ethernet_connection_ctrl)
	       (conn_eth,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&i825x6->tme_i825x6_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_I825X6_CALLOUT_CTRL;
      }
    }

    /* if we need to call out new config information: */
    if (callouts & TME_I825X6_CALLOUT_CONFIG) {
      
      /* form the new config: */
      memset(&config, 0, sizeof(config));
      
      /* our Ethernet addresses: */
      config.tme_ethernet_config_addr_count = i825x6->tme_i825x6_address_count;
      config.tme_ethernet_config_addrs
	= tme_new(const tme_uint8_t *,
		  i825x6->tme_i825x6_address_count);
      for (address_i = 0;
	   address_i < i825x6->tme_i825x6_address_count;
	   address_i++) {
	config.tme_ethernet_config_addrs[address_i]
	  = &i825x6->tme_i825x6_addresses[(address_i
					   * TME_ETHERNET_ADDR_SIZE)];
      }
      
      /* our config flags: */
      config.tme_ethernet_config_flags
	= (TME_ETHERNET_CONFIG_NORMAL
	   | (i825x6->tme_i825x6_prm
	      ? TME_ETHERNET_CONFIG_PROMISC
	      : 0));
      
      /* unlock the mutex: */
      tme_mutex_unlock(&i825x6->tme_i825x6_mutex);
      
      /* do the callout: */
      rc = (conn_eth == NULL
	    ? TME_OK
	    : ((*conn_eth->tme_ethernet_connection_config)
	       (conn_eth,
		&config)));
      
      /* lock the mutex: */
      tme_mutex_lock(&i825x6->tme_i825x6_mutex);

      /* free the Ethernet address pointer array: */
      tme_free(config.tme_ethernet_config_addrs);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_I825X6_CALLOUT_CONFIG;
      }
    }

    /* if the Ethernet is readable: */
    if (callouts & TME_I825X6_CALLOUT_READ) {

      /* if the Receive Unit is Suspended, make this callout later: */
      if ((stat_cus_rus_t
	   & TME_I82586_SCB_RUS_MASK)
	  == TME_I825X6_SCB_RUS_SUSPENDED) {
	later_callouts |= TME_I825X6_CALLOUT_READ;
      }

      /* otherwise, the Receive Unit is not Suspended, so read this
	 frame.  the frame will not be stored if the Receive Unit is
	 in the Idle or No Resources state: */
      else {
	stat_cus_rus_t = _tme_i825x6_callout_ru(i825x6, stat_cus_rus_t);
      }
    }

    /* note if the Command Unit left the Active state: */
    if (((i825x6->tme_i825x6_stat_cus_rus_t
	  & TME_I825X6_SCB_CUS_MASK)
	 == TME_I825X6_SCB_CUS_ACTIVE)
	&& ((stat_cus_rus_t
	     & TME_I825X6_SCB_CUS_MASK)
	    != TME_I825X6_SCB_CUS_ACTIVE)) {
      stat_cus_rus_t |= TME_I825X6_SCB_STAT_CNA;
    }

    /* note if the Receive Unit left the Ready state: */
    if (((i825x6->tme_i825x6_stat_cus_rus_t
	  & TME_I82586_SCB_RUS_MASK)
	 == TME_I825X6_SCB_RUS_READY)
	&& ((stat_cus_rus_t
	     & TME_I82586_SCB_RUS_MASK)
	    != TME_I825X6_SCB_RUS_READY)) {
      stat_cus_rus_t |= TME_I825X6_SCB_STAT_RNR;
    }

    /* if our Status bits have changed such that our interrupt signal
       changes, we need to call out an interrupt: */
    if (!(i825x6->tme_i825x6_stat_cus_rus_t
	  & TME_I825X6_SCB_STAT_MASK)
	!= !(stat_cus_rus_t
	     & TME_I825X6_SCB_STAT_MASK)) {
      i825x6->tme_i825x6_callout_flags |= TME_I825X6_CALLOUT_INT;
    }

    /* if our SCB status word has changed, write it out: */
    if (stat_cus_rus_t != i825x6->tme_i825x6_stat_cus_rus_t) {

      /* log: */
      tme_log(&i825x6->tme_i825x6_element->tme_element_log_handle,
	      100, TME_OK,
	      (&i825x6->tme_i825x6_element->tme_element_log_handle,
	       "SCB status 0x%04x -> 0x%04x",
	       i825x6->tme_i825x6_stat_cus_rus_t,
	       stat_cus_rus_t));

      TME_I825X6_WRITE16((i825x6->tme_i825x6_scb_address
			  + TME_I825X6_SCB_STAT_CUS_RUS_T),
			 stat_cus_rus_t);
      i825x6->tme_i825x6_stat_cus_rus_t = stat_cus_rus_t;
    }

    /* if we need to call out a change to our interrupt signal: */
    if (callouts & TME_I825X6_CALLOUT_INT) {

      /* see if the interrupt signal should be asserted or negated: */
      int_asserted = (stat_cus_rus_t & TME_I825X6_SCB_STAT_MASK);

      /* unlock our mutex: */
      tme_mutex_unlock(&i825x6->tme_i825x6_mutex);
      
      /* get our bus connection: */
      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						i825x6->tme_i825x6_device.tme_bus_device_connection,
						&i825x6->tme_i825x6_device.tme_bus_device_connection_rwlock);
      
      /* call out the bus interrupt signal edge: */
      rc = (*conn_bus->tme_bus_signal)
	(conn_bus,
	 TME_BUS_SIGNAL_INT_UNSPEC
	 | (int_asserted
	    ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	    : TME_BUS_SIGNAL_LEVEL_NEGATED));
      
      /* lock our mutex: */
      tme_mutex_lock(&i825x6->tme_i825x6_mutex);
      
      /* if this callout failed, remember that at some later time this
	 callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_I825X6_CALLOUT_INT;
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  i825x6->tme_i825x6_callout_flags = later_callouts;
}

/* the i825x6 bus signal handler: */
static int
_tme_i825x6_signal(void *_i825x6, 
		   unsigned int signal)
{
  struct tme_i825x6 *i825x6;
  int new_callouts;
  unsigned int level;

  /* recover our data structure: */
  i825x6 = (struct tme_i825x6 *) _i825x6;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&i825x6->tme_i825x6_mutex);

  /* take out the signal level: */
  level = signal & TME_BUS_SIGNAL_LEVEL_MASK;
  signal = TME_BUS_SIGNAL_WHICH(signal);

  /* dispatch on the generic bus signals: */
  switch (signal) {
  case TME_BUS_SIGNAL_RESET:
    if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
      _tme_i825x6_reset(i825x6);
    }
    break;
  default:
    break;
  }

  /* dispatch on the i825x6 bus signals: */
  if (signal >= i825x6->tme_i825x6_bus_signals.tme_bus_signals_first) {
    switch (signal - i825x6->tme_i825x6_bus_signals.tme_bus_signals_first) {

    case TME_I825X6_SIGNAL_CA:
      if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
	new_callouts |= TME_I825X6_CALLOUT_CA;
      }
      break;

    case TME_I825X6_SIGNAL_LOOP:
#if 0
      if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
	abort();
      }
#endif
      break;

    default:
      break;
    }
  }

  /* make any new callouts: */
  _tme_i825x6_callout(i825x6, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&i825x6->tme_i825x6_mutex);

  /* no faults: */
  return (TME_OK);
}

/* this is called when a device changes its configuration: */
static int
_tme_i825x6_config(struct tme_ethernet_connection *conn_eth, 
		  struct tme_ethernet_config *config)
{
  /* we don't care when other devices on the Ethernet
     reconfigure themselves: */
  return (TME_OK);
}

/* this is called when control lines change: */
static int
_tme_i825x6_ctrl(struct tme_ethernet_connection *conn_eth, 
		 unsigned int ctrl)
{
  struct tme_i825x6 *i825x6;
  int new_callouts;

  /* recover our data structures: */
  i825x6 = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&i825x6->tme_i825x6_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_ETHERNET_CTRL_OK_READ) {
    new_callouts |= TME_I825X6_CALLOUT_READ;
  }

  /* make any new callouts: */
  _tme_i825x6_callout(i825x6, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&i825x6->tme_i825x6_mutex);

  return (TME_OK);
}

/* this is called to read frames (from the i825x6 perspective, to transmit them): */
static int
_tme_i825x6_read(struct tme_ethernet_connection *conn_eth, 
		 tme_ethernet_fid_t *_frame_id,
		 struct tme_ethernet_frame_chunk *frame_chunks,
		 unsigned int flags)
{
  struct tme_i825x6 *i825x6;
  int new_callouts;
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  tme_uint32_t tbd_address, tb_address;
  tme_uint16_t eof_size;
  tme_uint16_t c_b_ok_a;
  tme_uint16_t value16;
  tme_uint32_t value32;
  int rc, err;
  int length;

  /* recover our data structures: */
  i825x6 = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* start our local copy of the caller's frame chunks: */
  if (frame_chunks != NULL) {
    frame_chunk_buffer = *frame_chunks;
  }
  else {
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count = 0;
  }
  frame_chunks = &frame_chunk_buffer;

  /* lock our mutex: */
  tme_mutex_lock(&i825x6->tme_i825x6_mutex);

  /* assume that we will have no packet to transmit: */
  length = 0;

  /* if we have a packet to transmit: */
  if ((i825x6->tme_i825x6_el_s_i_cmd
       & TME_I825X6_CB_CMD_MASK)
      == TME_I825X6_CB_CMD_TRANSMIT) {

    /* assume that we will succeed: */
    err = TME_OK;
    do {

      /* a helper macro for DMAing and copying data into chunks: */
#define CHUNKS_DMA_TX(addr, size)						\
      err = _tme_i825x6_chunks_dma_tx(i825x6, frame_chunks, (addr), (size));	\
      if (err != TME_OK) break;							\
      length += size
#define CHUNKS_MEM_TX(data, size)						\
      _tme_i825x6_chunks_mem_tx(frame_chunks, (data), (size));			\
      length += size

      /* if AL-LOC is set to zero, add the Ethernet/802.3 MAC header: */
      if (i825x6->tme_i825x6_al_loc == 0) {

	/* the destination address: */
	CHUNKS_DMA_TX(i825x6->tme_i825x6_cb_address + TME_I82586_TCB_ADDR_DEST,
		      TME_ETHERNET_ADDR_SIZE);

	/* our source address (our Individual Address): */
	CHUNKS_MEM_TX(&i825x6->tme_i825x6_addresses[TME_ETHERNET_ADDR_SIZE],
		      TME_ETHERNET_ADDR_SIZE);

	/* the length field: */
	CHUNKS_DMA_TX(i825x6->tme_i825x6_cb_address + TME_I82586_TCB_LENGTH,
		      TME_ETHERNET_LENGTH_SIZE);
      }

      /* the transmit buffers: */
      TME_I825X6_READ16((i825x6->tme_i825x6_cb_address
			 + TME_I82586_TCB_TBD_OFFSET),
			tbd_address);
      for (; tbd_address != 0xffff; ) {
	tbd_address += i825x6->tme_i825x6_scb_base;

	TME_I825X6_READ16((tbd_address
			   + TME_I82586_TBD_EOF_SIZE),
			  eof_size);
	TME_I82586_READ24((tbd_address
			   + TME_I82586_TBD_TB_ADDRESS),
			  tb_address);

	/* the transmit buffer contents: */
	CHUNKS_DMA_TX(tb_address,
		      (eof_size & TME_I82586_TBD_SIZE_MASK));

	/* the next transmit buffer: */
	if (eof_size & TME_I82586_TBD_EOF) {
	  break;
	}
	TME_I825X6_READ16((tbd_address
			   + TME_I82586_TBD_TBD_OFFSET),
			  tbd_address);
      }

#undef CHUNKS_DMA_TX
#undef CHUNKS_MEM_TX
  
    } while (/* CONSTCOND */ 0);

    /* get the CB status word and clear all of the transmit status bits: */
    c_b_ok_a
      = (i825x6->tme_i825x6_c_b_ok_a
	 & ~TME_I825X6_TCB_STATUS_MASK);
  
    /* if we got a bus error: */
    if (err != TME_OK) {
      
      /* set the DMA underrun transmit status bit: */
      c_b_ok_a |= TME_I825X6_TCB_STATUS_UNDERRUN;

      /* return an error to our caller: */
      length = -ENOENT;
    }

    /* update the CB status word: */
    i825x6->tme_i825x6_c_b_ok_a = c_b_ok_a;

    /* run the Command Unit: */
    new_callouts |= TME_I825X6_CALLOUT_CU;

    /* we no longer have a packet to transmit: */
    i825x6->tme_i825x6_el_s_i_cmd
      = ((i825x6->tme_i825x6_el_s_i_cmd
	  & ~TME_I825X6_CB_CMD_MASK)
	 | TME_I825X6_CB_CMD_NOP);
  }

  /* make any new callouts: */
  _tme_i825x6_callout(i825x6, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&i825x6->tme_i825x6_mutex);

  /* done: */
  return (length);
}

/* this makes a new Ethernet connection: */
static int
_tme_i825x6_connection_make_eth(struct tme_connection *conn, unsigned int state)
{
  struct tme_i825x6 *i825x6;
  struct tme_ethernet_connection *conn_eth;
  struct tme_ethernet_connection *conn_eth_other;

  /* recover our data structures: */
  i825x6 = conn->tme_connection_element->tme_element_private;
  conn_eth = (struct tme_ethernet_connection *) conn;
  conn_eth_other = (struct tme_ethernet_connection *) conn->tme_connection_other;

  /* both sides must be Ethernet connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_ETHERNET);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_ETHERNET);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&i825x6->tme_i825x6_mutex);

    /* save our connection: */
    i825x6->tme_i825x6_eth_connection = conn_eth_other;

    /* unlock our mutex: */
    tme_mutex_unlock(&i825x6->tme_i825x6_mutex);
  }

  return (TME_OK);
}

/* this makes a new bus connection: */
static int
_tme_i825x6_connection_make_bus(struct tme_connection *conn,
				unsigned int state)
{
  struct tme_i825x6 *i825x6;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* recover our data structure: */
  i825x6 = conn->tme_connection_element->tme_element_private;

  /* call the bus device connection maker: */
  rc = tme_bus_device_connection_make(conn, state);

  /* if the full connection was successful, and we don't have a TLB
     hash yet, allocate it and add our bus signals: */
  if (rc == TME_OK
      && state == TME_CONNECTION_FULL
      && !i825x6->tme_i825x6_tlb_hash_added) {

    /* get our bus connection: */
    conn_bus
      = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
				       i825x6->tme_i825x6_device.tme_bus_device_connection,
				       &i825x6->tme_i825x6_device.tme_bus_device_connection_rwlock);

    /* add the TLB set: */
    rc = tme_bus_device_tlb_set_add(&i825x6->tme_i825x6_device,
				    TME_I825X6_TLB_HASH_SIZE, 
				    i825x6->tme_i825x6_tlb_hash);
    assert (rc == TME_OK);
    i825x6->tme_i825x6_tlb_hash_added = TRUE;

    /* add our bus signals: */
    i825x6->tme_i825x6_bus_signals = _tme_i825x6_bus_signals;
    rc = ((*conn_bus->tme_bus_signals_add)
	  (conn_bus,
	   &i825x6->tme_i825x6_bus_signals));
    assert (rc == TME_OK);    
  }

  return (rc);
}

/* this breaks a connection: */
static int
_tme_i825x6_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a i825x6: */
static int
_tme_i825x6_connections_new(struct tme_element *element,
			   const char * const *args,
			   struct tme_connection **_conns,
			   char **_output)
{
  struct tme_i825x6 *i825x6;
  struct tme_ethernet_connection *conn_eth;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  i825x6 = (struct tme_i825x6 *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* since we need to allocate our TLB hash and add our signals sets
     when we make our bus connection, make sure any generic bus device
     connection sides use our connection maker: */
  for (conn = *_conns;
       conn != NULL;
       conn = conn->tme_connection_next) {
    if ((conn->tme_connection_type
	 == TME_CONNECTION_BUS_GENERIC)
	&& (conn->tme_connection_make
	    == tme_bus_device_connection_make)) {
      conn->tme_connection_make
	= _tme_i825x6_connection_make_bus;
    }
  }

  /* if we don't have an Ethernet connection, make one: */
  if (i825x6->tme_i825x6_eth_connection == NULL) {

    /* allocate the new Ethernet connection: */
    conn_eth = tme_new0(struct tme_ethernet_connection, 1);
    conn = &conn_eth->tme_ethernet_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_ETHERNET;
    conn->tme_connection_score = tme_ethernet_connection_score;
    conn->tme_connection_make = _tme_i825x6_connection_make_eth;
    conn->tme_connection_break = _tme_i825x6_connection_break;

    /* fill in the Ethernet connection: */
    conn_eth->tme_ethernet_connection_config = _tme_i825x6_config;
    conn_eth->tme_ethernet_connection_ctrl = _tme_i825x6_ctrl;
    conn_eth->tme_ethernet_connection_read = _tme_i825x6_read;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new i82586 function: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,i825x6,i82586) {
  struct tme_i825x6 *i825x6;
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

  /* start the i825x6 structure: */
  i825x6 = tme_new0(struct tme_i825x6, 1);
  i825x6->tme_i825x6_element = element;
  tme_mutex_init(&i825x6->tme_i825x6_mutex);
  i825x6->tme_i825x6_address_count = 2;
  i825x6->tme_i825x6_addresses
    = tme_new(tme_uint8_t, 
	      i825x6->tme_i825x6_address_count
	      * TME_ETHERNET_ADDR_SIZE);

  /* initialize our simple bus device descriptor: */
  i825x6->tme_i825x6_device.tme_bus_device_element = element;
  i825x6->tme_i825x6_device.tme_bus_device_signal = _tme_i825x6_signal;
  i825x6->tme_i825x6_device.tme_bus_device_lock = _tme_i825x6_lock;
  i825x6->tme_i825x6_device.tme_bus_device_unlock = _tme_i825x6_unlock;
  i825x6->tme_i825x6_device.tme_bus_device_tlb_hash = _tme_i825x6_tlb_hash;
  i825x6->tme_i825x6_device.tme_bus_device_router = tme_bus_device_router_16el;

  /* fill the element: */
  element->tme_element_private = i825x6;
  element->tme_element_connections_new = _tme_i825x6_connections_new;

  /* reset the i825x6: */
  _tme_i825x6_reset(i825x6);

  return (TME_OK);
}
