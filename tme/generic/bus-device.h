/* $Id: bus-device.h,v 1.9 2009/08/29 17:48:27 fredette Exp $ */

/* tme/generic/bus-device.h - header file for generic bus device support: */

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

#ifndef _TME_GENERIC_BUS_DEVICE_H
#define _TME_GENERIC_BUS_DEVICE_H

#include <tme/common.h>
_TME_RCSID("$Id: bus-device.h,v 1.9 2009/08/29 17:48:27 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/generic/bus.h>

/* macros: */

/* this indexes an initiator bus router array for a device with a port size
   of 8 * (2 ^ siz_lg2) bits: */
#define TME_BUS_ROUTER_INIT_INDEX(siz_lg2, cycle_size, address) \
(((                                                             \
   /* by the maximum cycle size: */                             \
   ((cycle_size) - 1)                                           \
                                                                \
   /* by the address alignment: */                              \
   << siz_lg2)                                                  \
  + ((address) & ((1 << (siz_lg2)) - 1)))                       \
                                                                \
 /* factor in the size of the generic bus router array: */      \
 * TME_BUS_ROUTER_SIZE(siz_lg2))

/* structures: */

/* a bus device: */
struct tme_bus_device {
  
  /* backpointer to the device's element: */
  struct tme_element *tme_bus_device_element;

  /* this device's bus connection: */
  struct tme_bus_connection * tme_shared tme_bus_device_connection;
  tme_rwlock_t tme_bus_device_connection_rwlock;

  /* the subregions for this device.  for some backwards compatibility
     for older sources, we define tme_bus_device_address_last: */
  struct tme_bus_subregion tme_bus_device_subregions;
#define tme_bus_device_address_last tme_bus_device_subregions.tme_bus_subregion_address_last

  /* the bus signal handler: */
  int (*tme_bus_device_signal) _TME_P((void *, unsigned int));

  /* the bus interrupt acknowledge handler: */
  int (*tme_bus_device_intack) _TME_P((void *, unsigned int, int *));

  /* the bus TLB entry filler: */
  int (*tme_bus_device_tlb_fill) _TME_P((void *, struct tme_bus_tlb *, tme_bus_addr_t, unsigned int));

  /* the device lock and unlock functions: */
  void (*tme_bus_device_lock) _TME_P((void *, unsigned int));
  void (*tme_bus_device_unlock) _TME_P((void *, unsigned int));

  /* the bus master TLB entry hasher: */
  struct tme_bus_tlb *(*tme_bus_device_tlb_hash) _TME_P((void *, tme_bus_addr_t, unsigned int));

  /* the bus master bus router: */
  _tme_const tme_bus_lane_t *tme_bus_device_router;
};

/* globals: */

/* the 16-bit big-endian bus master bus router: */
extern _tme_const tme_bus_lane_t tme_bus_device_router_16eb[];

/* the 16-bit little-endian bus master bus router: */
extern _tme_const tme_bus_lane_t tme_bus_device_router_16el[];

/* the 32-bit big-endian bus master bus router: */
extern _tme_const tme_bus_lane_t tme_bus_device_router_32eb[];

/* the 32-bit little-endian bus master bus router: */
extern _tme_const tme_bus_lane_t tme_bus_device_router_32el[];

/* prototypes: */
int tme_bus_device_connection_score _TME_P((struct tme_connection *, unsigned int *));
int tme_bus_device_connection_make _TME_P((struct tme_connection *, unsigned int));
int tme_bus_device_connection_break _TME_P((struct tme_connection *, unsigned int));
int tme_bus_device_connections_new _TME_P((struct tme_element *, _tme_const char * _tme_const *, struct tme_connection **, char **));

/* this adds a bus device generic TLB set: */
int tme_bus_device_tlb_set_add _TME_P((struct tme_bus_device *,
				       unsigned long,
				       struct tme_bus_tlb *));

/* the 16-bit bus master DMA read function: */
int tme_bus_device_dma_read_16 _TME_P((struct tme_bus_device *,
                                       tme_bus_addr_t,
                                       tme_bus_addr_t,
                                       tme_uint8_t *,
				       unsigned int));

/* the 16-bit bus master DMA write function: */
int tme_bus_device_dma_write_16 _TME_P((struct tme_bus_device *,
                                       tme_bus_addr_t,
                                       tme_bus_addr_t,
                                       _tme_const tme_uint8_t *,
				       unsigned int));

/* the 32-bit bus master DMA read function: */
int tme_bus_device_dma_read_32 _TME_P((struct tme_bus_device *,
                                       tme_bus_addr_t,
                                       tme_bus_addr_t,
                                       tme_uint8_t *,
				       unsigned int));

/* the 32-bit bus master DMA write function: */
int tme_bus_device_dma_write_32 _TME_P((struct tme_bus_device *,
                                       tme_bus_addr_t,
                                       tme_bus_addr_t,
                                       _tme_const tme_uint8_t *,
				       unsigned int));

#endif /* !_TME_GENERIC_BUS_DEVICE_H */
