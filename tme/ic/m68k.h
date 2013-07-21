/* $Id: m68k.h,v 1.8 2009/08/29 18:02:48 fredette Exp $ */

/* tme/ic/m68k.h - public header file for Motorola 68k emulation */

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

#ifndef _TME_IC_M68K_H
#define _TME_IC_M68K_H

#include <tme/common.h>
_TME_RCSID("$Id: m68k.h,v 1.8 2009/08/29 18:02:48 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/generic/bus.h>

/* macros: */

/* function codes: */
#define TME_M68K_FC_0	(0)
#define TME_M68K_FC_UD	(1)
#define TME_M68K_FC_UP	(2)
#define TME_M68K_FC_3	(3)
#define TME_M68K_FC_4	(4)
#define TME_M68K_FC_SD	(5)
#define TME_M68K_FC_SP	(6)
#define TME_M68K_FC_7	(7)

/* the minimum and maximum IPL levels: */
#define TME_M68K_IPL_NONE	(0)
#define TME_M68K_IPL_MIN	(1)
#define TME_M68K_IPL_MAX	(7)
#define TME_M68K_IPL_NMI	(7)

/* these busy and unbusy a TLB entry: */
#define tme_m68k_tlb_busy(tlb)					\
  tme_token_busy(&(tlb)->tme_m68k_tlb_token)
#define tme_m68k_tlb_unbusy(tlb)				\
  tme_token_unbusy(&(tlb)->tme_m68k_tlb_token)

/* these test the validity of a TLB entry: */
#define tme_m68k_tlb_is_valid(tlb)				\
  tme_token_is_valid(&(tlb)->tme_m68k_tlb_token)
#define tme_m68k_tlb_is_invalid(tlb)				\
  tme_token_is_invalid(&(tlb)->tme_m68k_tlb_token)

/* this indexes an m68k bus router array for an m68k with a port size
   of 8 * (2 ^ siz_lg2) bits: */
#define TME_M68K_BUS_ROUTER_INDEX(siz_lg2, cycle_size, address)	\
(((								\
   /* by the maximum cycle size: */				\
   ((cycle_size) - 1)						\
								\
   /* by the address alignment: */				\
   << siz_lg2)							\
  + ((address) & ((1 << (siz_lg2)) - 1)))			\
								\
 /* factor in the size of the generic bus router array: */	\
 * TME_BUS_ROUTER_SIZE(siz_lg2))

/* this gives the number of entries that must be in a generic bus
   router array for a device with a bus size of 8 * (2 ^ siz_lg2)
   bits: */
#define TME_M68K_BUS_ROUTER_SIZE(siz_lg2)			\
  TME_M68K_BUS_ROUTER_INDEX(siz_lg2, (1 << (siz_lg2)) + 1, 0)

/* structures: */

/* an m68k TLB entry: */
struct tme_m68k_tlb {

  /* the generic bus TLB associated with this TLB entry: */
  struct tme_bus_tlb tme_m68k_tlb_bus_tlb;
#define tme_m68k_tlb_linear_first tme_m68k_tlb_bus_tlb.tme_bus_tlb_addr_first
#define tme_m68k_tlb_linear_last tme_m68k_tlb_bus_tlb.tme_bus_tlb_addr_last
#define tme_m68k_tlb_bus_rwlock tme_m68k_tlb_bus_tlb.tme_bus_tlb_rwlock
#define tme_m68k_tlb_cycles_ok tme_m68k_tlb_bus_tlb.tme_bus_tlb_cycles_ok
#define tme_m68k_tlb_addr_offset tme_m68k_tlb_bus_tlb.tme_bus_tlb_addr_offset
#define tme_m68k_tlb_addr_shift tme_m68k_tlb_bus_tlb.tme_bus_tlb_addr_shift
#define tme_m68k_tlb_emulator_off_read tme_m68k_tlb_bus_tlb.tme_bus_tlb_emulator_off_read
#define tme_m68k_tlb_emulator_off_write tme_m68k_tlb_bus_tlb.tme_bus_tlb_emulator_off_write

  /* the token for this TLB entry: */
  struct tme_token tme_m68k_tlb_token;

  /* the bus context for this TLB entry: */
  tme_bus_context_t tme_m68k_tlb_bus_context;

  /* the function codes handled by this entry: */
  unsigned int tme_m68k_tlb_function_codes_mask;
};

/* an m68k bus connection: */
struct tme_m68k_bus_connection {

  /* a generic bus connection: */
  struct tme_bus_connection tme_m68k_bus_connection;

  /* the m68k interrupt function: */
  int (*tme_m68k_bus_interrupt) _TME_P((struct tme_m68k_bus_connection *, unsigned int));

  /* the m68k TLB entry filler: */
  int (*tme_m68k_bus_tlb_fill) _TME_P((struct tme_m68k_bus_connection *, struct tme_m68k_tlb *,
				       unsigned int, tme_uint32_t, unsigned int));

  /* the m68k m6888x enabler: */
  int (*tme_m68k_bus_m6888x_enable) _TME_P((struct tme_m68k_bus_connection *, int));
};

/* globals: */
extern _tme_const tme_bus_lane_t tme_m68k_router_16[];
extern _tme_const tme_bus_lane_t tme_m68k_router_32[];

#endif /* !_TME_IC_M68K_H */
