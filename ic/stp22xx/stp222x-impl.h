/* $Id: stp222x-impl.h,v 1.4 2009/11/08 16:45:14 fredette Exp $ */

/* ic/stp22xx/stp222x-impl.h - implementation header file for STP2220
   and STP2222 emulation: */

/*
 * Copyright (c) 2009 Matt Fredette
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

#ifndef _STP222X_IMPL_H
#define _STP222X_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: stp222x-impl.h,v 1.4 2009/11/08 16:45:14 fredette Exp $");

/* includes: */
#include "stp22xx-impl.h"

/* macros: */

/* common interrupt diagnostic register indices: */
#define TME_STP222X_IDI0_OBIO			(0x20)
#define TME_STP222X_IDI_SCSI			(TME_STP222X_IDI0_OBIO + 0x00)
#define TME_STP222X_IDI_ETHER			(TME_STP222X_IDI0_OBIO + 0x01)
#define TME_STP222X_IDI_BPP			(TME_STP222X_IDI0_OBIO + 0x02)
#define TME_STP222X_IDI_NULL			(0x40)

/* the common register size: */
#define TME_STP222X_REG_SIZE			(sizeof(tme_uint64_t))

/* common timer register offsets: */
#define TME_STP222X_TIMER_COUNT			(0x00)
#define TME_STP222X_TIMER_LIMIT			(0x08)
#define TME_STP222X_TIMER_SIZE			(0x10)

/* the maximum count of interrupt dispatch buffers: */
#define TME_STP222X_MDU_BUFFER_COUNT		(2)

/* the number of IOMMU TLB entries: */
#define TME_STP222X_IOMMU_TLB_SIZE		(16)

/* the maximum count of streaming caches: */
#define TME_STP222X_STC_COUNT			(2)

/* stp2220-specific slots: */
#define TME_STP2220_SLOTS_CARD			(4)
#define TME_STP2220_SLOTS_OBIO			(3)
#define TME_STP2220_SLOT_CARD(n)		(n)
#define TME_STP2220_SLOT_OBIO(n)		(13 + (n))
#define TME_STP2220_SLOT_AUDIO			TME_STP2220_SLOT_OBIO(0)
#define TME_STP2220_SLOT_MACIO			TME_STP2220_SLOT_OBIO(1)
#define TME_STP2220_SLOT_SLAVIO			TME_STP2220_SLOT_OBIO(2)

/* stp2220-specific interrupt diagnostic register indices: */
#define TME_STP2220_IDI_CARD(slot, intn)	((TME_SBUS_SLOT_INTS * (slot)) + (intn))
#define TME_STP2220_IDI_AUDIO			(TME_STP222X_IDI0_OBIO + 0x03)
#define TME_STP2220_IDI_POWER			(TME_STP222X_IDI0_OBIO + 0x04)
#define TME_STP2220_IDI_ZS0_ZS1			(TME_STP222X_IDI0_OBIO + 0x05)
#define TME_STP2220_IDI_FD			(TME_STP222X_IDI0_OBIO + 0x06)
#define TME_STP2220_IDI_THERM			(TME_STP222X_IDI0_OBIO + 0x07)
#define TME_STP2220_IDI_KBD			(TME_STP222X_IDI0_OBIO + 0x08)
#define TME_STP2220_IDI_MOUSE			(TME_STP222X_IDI0_OBIO + 0x09)
#define TME_STP2220_IDI_SERIAL			(TME_STP222X_IDI0_OBIO + 0x0a)
#define TME_STP2220_IDI_TIMER(n)		(TME_STP222X_IDI0_OBIO + 0x0b + (n))
#define TME_STP2220_IDI_UE			(TME_STP222X_IDI0_OBIO + 0x0d)
#define TME_STP2220_IDI_CE			(TME_STP222X_IDI0_OBIO + 0x0e)
#define TME_STP2220_IDI_SBUS_ASYNC		(TME_STP222X_IDI0_OBIO + 0x0f)
#define TME_STP2220_IDI_POWER_MANAGE		(TME_STP222X_IDI0_OBIO + 0x10)
#define TME_STP2220_IDI_UPA			(TME_STP222X_IDI0_OBIO + 0x11)
#define TME_STP2220_IDI_RESERVED		(TME_STP222X_IDI0_OBIO + 0x12)

/* stp2222-specific IDIs: */
#define TME_STP2222_IDI_CARD(bus, slot, intn)	((TME_PCI_SLOT_INTS * (((bus) * 4) + (slot))) + (intn))
#define TME_STP2222_IDI_AUDIO_RECORD		(TME_STP222X_IDI0_OBIO + 0x03)
#define TME_STP2222_IDI_AUDIO_PLAYBACK		(TME_STP222X_IDI0_OBIO + 0x04)
#define TME_STP2222_IDI_TIMER(n)		(TME_STP222X_IDI0_OBIO + 0x0c + (n))
#define TME_STP2222_IDI_UE			(TME_STP222X_IDI0_OBIO + 0x0e)
#define TME_STP2222_IDI_CE			(TME_STP222X_IDI0_OBIO + 0x0f)
#define TME_STP2222_IDI_POWER_MANAGE		(TME_STP222X_IDI0_OBIO + 0x12)
#define TME_STP2222_IDI_FFB0			(TME_STP222X_IDI0_OBIO + 0x13)
#define TME_STP2222_IDI_FFB1			(TME_STP222X_IDI0_OBIO + 0x14)

/* the STP2222 BOOT_BUS: */
#define TME_STP2222_BOOT_BUS(stp222x)		(0 && (stp222x))

/* a register group covers 256 addresses: */
#define TME_STP222X_REGGROUP_WHICH(x)		((x) / 256)
#define TME_STP222X_REGGROUP_INDEX(x)		((((tme_uint32_t) (x)) / TME_STP222X_REG_SIZE) % (256 / TME_STP222X_REG_SIZE))

/* XXX FIXME - these should be in bus-specific headers: */
#define TME_SBUS_SLOT_INTS			(8)
#define TME_PCI_SLOT_INTS			(4)

/* connections: */
#define TME_STP222X_CONN_SLAVE0			(sizeof(((struct tme_stp222x *) 0)->tme_stp222x_io_brs) * 8)
#define TME_STP222X_CONN_NULL			(64)
#define TME_STP222X_CONN_UPA			(TME_STP222X_CONN_NULL + 1)
#define TME_STP222X_CONN_COUNT			(TME_STP222X_CONN_UPA + 1)

/* the number of TLB tokens in a list: */
#define TME_STP222X_TLB_LIST_TOKENS_COUNT	(4)

/* connection identifiers: */
/* NB: connection identifiers are complicated because we want to be
   able to quickly map identifiers for connections that can interrupt
   into an IDI, and quickly map identifiers for masters into
   connection indices, but the connection identifier often has to
   carry addressing information for the connection too.

   obio connections use one type of identifier.  known obio
   connections (that we need to get IDIs and connection indices for
   quickly) have a short form, that contains the IDI and connection
   index directly.  unknown obio connections use a long form that
   captures the slot and offset only.

   card connections use another type of identifier.  these identifiers
   always contain the addressing information for the connection, which
   can always be quickly mapped into a base IDI.  card identifiers for
   masters can usually be mapped quickly into a connection index: */
#define TME_STP222X_CONNID_TYPE			(0x1 << 0)
#define  TME_STP222X_CONNID_TYPE_OBIO		 (0x1 << 0)
#define  TME_STP222X_CONNID_TYPE_CARD		 (0x0 << 0)
#define TME_STP222X_CONNID_OBIO_TYPE		(0x1 << 1)
#define  TME_STP222X_CONNID_OBIO_TYPE_SHORT	 (0x1 << 1)
#define  TME_STP222X_CONNID_OBIO_TYPE_LONG	 (0x0 << 1)
#if (TME_STP222X_IDI_NULL & (TME_STP222X_IDI_NULL - 1))
#error "TME_STP222X_IDI_NULL must be a power of two"
#endif
#define TME_STP222X_CONNID_OBIO_SHORT_IDI	((TME_STP222X_IDI_NULL - 1) << 2)
#define TME_STP222X_CONNID_OBIO_SHORT_CONN_WHICH \
  (~ (tme_uint32_t)				\
   (TME_STP222X_CONNID_TYPE			\
    | TME_STP222X_CONNID_OBIO_TYPE		\
    | TME_STP222X_CONNID_OBIO_SHORT_IDI))
#define TME_STP222X_CONNID_OBIO_LONG_WHICH	(0x3 << 2)
#define TME_STP222X_CONNID_OBIO_LONG_OFFSET	((TME_STP2220_SLOT_SIZE - 1) << 4)
#define TME_STP2220_CONNID_CARD_WHICH		(0x3 << 1)
#define TME_STP2220_CONNID_CARD_ALTERNATE	(0x1 << 3)
#define TME_STP2220_CONNID_CARD_OFFSET		((TME_STP2220_SLOT_SIZE - 1) << 4)
#define TME_STP2222_CONNID_DEVICE_WHICH		(0x3 << 1)
#define TME_STP2222_CONNID_BUS_WHICH		(0x4 << 1)

/* address spaces: */
#define TME_STP2220_ASPACE_SBUS			(0)
#define TME_STP2222_ASPACE_PCI_MEMORY(bus)	(1 + (bus))
#define TME_STP2222_ASPACE_PCI_IO(bus)		(3 + (bus))
#define TME_STP2222_ASPACE_PCI_CONFIGURATION	(5)
#define TME_STP222X_ASPACE_NULL			(6)

/* address searches: */
#define TME_STP222X_ASEARCH_MISS		(((tme_uint32_t) 1) << 31)

/* predicates: */
#define TME_STP222X_IS_2220(stp222x)		((stp222x)->tme_stp222x_is_2220)

/* miscellaneous: */
#define TME_STP222X_LOG_HANDLE(stp222x)		(&(stp222x)->tme_stp222x.tme_stp22xx_element->tme_element_log_handle)

/* these busy a bus connection: */
#define tme_stp222x_busy_bus(stp222x, conn_index) \
  tme_stp22xx_busy_bus(&(stp222x)->tme_stp222x, conn_index)
#define tme_stp222x_busy_upa(stp222x) \
  ((struct tme_upa_bus_connection *) tme_stp222x_busy_bus(stp222x, TME_STP222X_CONN_UPA))

/* this unbusies a generic bus connection: */
#define tme_stp222x_unbusy_bus(stp222x, conn_bus) \
  tme_stp22xx_unbusy_bus(&(stp222x)->tme_stp222x, conn_bus)

/* this busies a slave generic bus connection: */
#define tme_stp222x_slave_busy_bus(stp222x, conn_index) \
  tme_stp22xx_slave_busy_bus(&(stp222x)->tme_stp222x, conn_index)

/* this unbusies a slave generic bus connection: */
#define tme_stp222x_slave_unbusy(stp222x) \
  tme_stp22xx_slave_unbusy(&(stp222x)->tme_stp222x)

/* this enters an stp222x: */
#define tme_stp222x_enter_bus(conn_bus) \
  ((struct tme_stp222x *) tme_stp22xx_enter((struct tme_stp22xx *) (conn_bus)->tme_bus_connection.tme_connection_element->tme_element_private))

/* this enters an stp222x as the bus master: */
#define tme_stp222x_enter_master_bus(master_conn_bus) \
  ((struct tme_stp222x *) tme_stp22xx_enter_master(master_conn_bus))

/* this leaves an stp222x: */
#define tme_stp222x_leave(stp222x) \
  tme_stp22xx_leave(&(stp222x)->tme_stp222x)

/* this validates a completion: */
/* NB: completion may be NULL: */
#define tme_stp222x_completion_validate(stp222x, completion) \
  tme_stp22xx_completion_validate(&(stp222x)->tme_stp222x, completion)

/* this allocates a completion: */
#define tme_stp222x_completion_alloc(stp222x, handler, arg) \
  tme_stp22xx_completion_alloc(&(stp222x)->tme_stp222x, handler, arg)

/* types: */

/* a timer: */
struct tme_stp222x_timer {

  /* a backpointer to the stp222x: */
  struct tme_stp222x *tme_stp222x_timer_stp222x;

  /* the real limit register value: */
  tme_uint32_t tme_stp222x_timer_limit;

  /* the current period of this timer: */
  struct timeval tme_stp222x_timer_period;

  /* when the timer reaches its next limit: */
  struct timeval tme_stp222x_timer_limit_next;

  /* a condition for waking up the thread for this timer: */
  struct tme_stp22xx_cond tme_stp222x_timer_cond;

  /* these are used to track the interrupt rate for this timer: */
  tme_uint32_t tme_stp222x_timer_track_ints;
  struct timeval tme_stp222x_timer_track_sample;

  /* the IDI for this timer: */
  tme_uint8_t tme_stp222x_timer_idi;
};

/* a TLB list: */
struct tme_stp222x_tlb_list {

  /* the head of the list: */
  unsigned int tme_stp222x_tlb_list_head;

  /* the tokens for the TLBs in the list: */
  struct tme_token *tme_stp222x_tlb_list_tokens[TME_STP222X_TLB_LIST_TOKENS_COUNT];
};

/* a streaming cache: */
struct tme_stp222x_stc {

  /* the STC control register: */
  tme_uint32_t tme_stp222x_stc_cr;

  /* if this is nonzero, a flush is active: */
  int tme_stp222x_stc_pgflush;

  /* the STC FLUSHSYNC register: */
  tme_uint64_t tme_stp222x_stc_flushsync;

  /* the TLB entry for the flushsync range: */
  struct tme_bus_tlb tme_stp222x_stc_flushsync_tlb;
  struct tme_token tme_stp222x_stc_flushsync_tlb_token;
};

/* an address range: */
struct tme_stp222x_arange {

  /* the first address in the address range: */
  tme_bus_addr64_t tme_stp222x_arange_first;

  /* the size minus one of the address range: */
  tme_bus_addr64_t tme_stp222x_arange_size_m1;

  /* the key for this address range: */
  tme_uint32_t tme_stp222x_arange_key;
};

/* an address space: */
struct tme_stp222x_aspace {

  /* all address ranges claimed in this space: */
  struct tme_stp222x_arange *tme_stp222x_aspace_aranges;
  tme_uint32_t tme_stp222x_aspace_arange_count;

  /* the count of 32-bit address ranges claimed in this space: */
  tme_uint32_t tme_stp222x_aspace_arange32_count;

  /* the offset in this address space for each connection index: */
  tme_bus_addr32_t tme_stp222x_aspace_conn_offset[TME_STP222X_CONN_NULL];
};

/* a set of IDIs: */
typedef tme_uint32_t tme_stp222x_idis_t;

/* a register access: */
struct tme_stp222x_reg {

  /* the register value: */
  tme_uint64_t tme_stp222x_reg_value;

  /* the register address: */
  tme_uint16_t tme_stp222x_reg_address;

  /* if nonzero, this is a write: */
  tme_uint8_t tme_stp222x_reg_write;

  /* if nonzero, the access was completed: */
  tme_uint8_t tme_stp222x_reg_completed;
};

/* the device: */
struct tme_stp222x {

  /* the common structure, and space for the connections: */
  struct tme_stp22xx tme_stp222x;
#define tme_stp222x_master_conn_index_pending tme_stp222x.tme_stp22xx_master_conn_index_pending
#define tme_stp222x_master_conn_index tme_stp222x.tme_stp22xx_master_conn_index
#define tme_stp222x_master_completion tme_stp222x.tme_stp22xx_master_completion
  union tme_stp22xx_conn __tme_stp222x_conns[TME_STP222X_CONN_COUNT];

  /* this is nonzero if this is an STP2220: */
  int tme_stp222x_is_2220;

  /* the next slave connection index: */
  tme_uint32_t tme_stp222x_slave_conn_index_next;

  /* this is nonzero if we are asserting bus request on the UPA bus: */
  tme_uint8_t tme_stp222x_br;

  /* this is nonzero if we have been granted the UPA bus: */
  tme_uint8_t tme_stp222x_bg;

  /* the UPA port configuration register: */
  tme_uint8_t tme_stp222x_upa_port_config;

  /* the ECC control register: */
  tme_uint8_t tme_stp222x_ecc_control;

  /* the CSR: */
  tme_uint64_t tme_stp222x_csr;

  /* the wired-together state of the zs0 and zs1 IDIs: */
  tme_uint8_t tme_stp2220_mdu_idi_zs0_zs1_active;

  /* the active IDIs: */
  tme_stp222x_idis_t tme_stp222x_mdu_idis_active[TME_STP222X_IDI_NULL / (sizeof(tme_stp222x_idis_t) * 8)];

  /* the received IDIs: */
  tme_stp222x_idis_t tme_stp222x_mdu_idis_received[TME_STP222X_IDI_NULL / (sizeof(tme_stp222x_idis_t) * 8)];

  /* the IMRs: */
  tme_uint32_t tme_stp222x_mdu_imrs[TME_STP222X_IDI_NULL];

  /* the pending IDIs: */
  tme_stp222x_idis_t tme_stp222x_mdu_idis_pending[TME_STP222X_IDI_NULL / (sizeof(tme_stp222x_idis_t) * 8)];

  /* the interrupt dispatch buffers: */
  tme_uint32_t tme_stp222x_mdu_dispatch_imr[TME_STP222X_MDU_BUFFER_COUNT];
  tme_uint8_t tme_stp222x_mdu_dispatch_idi[TME_STP222X_MDU_BUFFER_COUNT];
  tme_uint8_t tme_stp222x_mdu_dispatch_state[TME_STP222X_MDU_BUFFER_COUNT];

  /* the next interrupt dispatch buffer: */
  unsigned int tme_stp222x_mdu_dispatch_buffer;

  /* the interrupt retry period and condition: */
  tme_uint32_t tme_stp222x_mdu_retry;
  struct timeval tme_stp222x_mdu_retry_sleep;
  struct tme_stp22xx_cond tme_stp222x_mdu_retry_cond;

  /* the UPA bus reset level: */
  unsigned int tme_stp222x_reset_level;

  /* the reset state: */
  tme_uint32_t tme_stp222x_reset_state;

  /* the mask of I/O connection indices requesting the bus: */
  tme_uint32_t tme_stp222x_io_brs;

  /* the address spaces: */
  struct tme_stp222x_aspace tme_stp222x_aspaces[TME_STP222X_ASPACE_NULL];

  /* the STP2220 SBus registers: */
  tme_uint64_t tme_stp2220_sbus_csr;
  tme_uint64_t tme_stp2220_sbus_config_card[TME_STP2220_SLOTS_CARD + TME_STP2220_SLOTS_OBIO];

  /* the SBus offset for each connection index: */
  tme_bus_addr32_t tme_stp2220_conn_offset[TME_STP222X_CONN_NULL];

  /* the timers: */
  struct tme_stp222x_timer tme_stp222x_timers[2];

  /* the IOMMU control register: */
  tme_uint32_t tme_stp222x_iommu_cr;

  /* the most recently used IOMMU TLB entry: */
  unsigned int tme_stp222x_iommu_tlb_i_mru;

  /* the IOMMU TLB LRU queue: */
  tme_uint8_t tme_stp222x_iommu_lru[TME_STP222X_IOMMU_TLB_SIZE][2];
#define tme_stp222x_iommu_lru_prev(x) tme_stp222x_iommu_lru[x][0]
#define tme_stp222x_iommu_lru_next(x) tme_stp222x_iommu_lru[x][1]

  /* the IOMMU TLB entry tags: */
  tme_uint32_t tme_stp222x_iommu_tlb_tags[TME_STP222X_IOMMU_TLB_SIZE];

  /* the IOMMU TLB entry data: */
  tme_uint32_t tme_stp222x_iommu_tlb_data[TME_STP222X_IOMMU_TLB_SIZE];

  /* the TSB address: */
  tme_uint64_t tme_stp222x_iommu_tsb;

  /* the TSB TLB: */
  struct tme_bus_tlb tme_stp222x_iommu_tsb_tlb;
  struct tme_token tme_stp222x_iommu_tsb_tlb_token;

  /* the TLB lists for the IOMMU: */
  struct tme_stp222x_tlb_list tme_stp222x_iommu_tlb_list_fixed;
  struct tme_stp222x_tlb_list tme_stp222x_iommu_tlb_list[TME_STP222X_IOMMU_TLB_SIZE];

  /* the IOMMU VA and compare registers: */
  tme_uint32_t tme_stp222x_iommu_va;
  tme_uint32_t tme_stp222x_iommu_compare;

#if TME_STP22XX_BUS_TRANSITION
  /* the token for a TLB entry that missed the IOMMU TLB: */
  struct tme_token *tme_stp222x_iommu_tlb_missed_token;
#endif /* TME_STP22XX_BUS_TRANSITION */

  /* the streaming caches: */
  struct tme_stp222x_stc tme_stp222x_stcs[TME_STP222X_STC_COUNT];
};

/* prototypes: */

/* the timer register handler: */
void tme_stp222x_timer_regs _TME_P((struct tme_stp222x *, struct tme_stp222x_reg *));

/* this initializes an stp222x timer: */
void tme_stp222x_timer_init _TME_P((struct tme_stp222x *, struct tme_stp222x_timer *));

/* this updates the interrupt concentrator: */
void tme_stp222x_mdu_intcon _TME_P((struct tme_stp222x *, tme_uint32_t, tme_uint32_t));

/* this receives an interrupt: */
void tme_stp222x_mdu_receive _TME_P((struct tme_stp222x *, tme_uint32_t));

/* this dispatches an interrupt: */
int tme_stp222x_mdu_dispatch _TME_P((struct tme_stp222x *));

/* the MDU IMR and retry register handler: */
void tme_stp222x_mdu_regs_imr_retry _TME_P((struct tme_stp222x *, struct tme_stp222x_reg *));

/* the MDU clear register handler: */
void tme_stp222x_mdu_regs_clear _TME_P((struct tme_stp222x *, struct tme_stp222x_reg *));

/* the MDU diagnostic register handler: */
void tme_stp222x_mdu_regs_diag _TME_P((struct tme_stp222x *, struct tme_stp222x_reg *));

/* this updates the IGN for the partial IMRs: */
void tme_stp222x_mdu_ign_update _TME_P((struct tme_stp222x *, tme_uint32_t));

/* this initializes the MDU: */
void tme_stp222x_mdu_init _TME_P((struct tme_stp222x *));

/* this handles an IOMMU bus cycle: */
void tme_stp222x_iommu_cycle _TME_P((struct tme_bus_connection *, struct tme_bus_cycle *, tme_uint32_t *, struct tme_completion *));

/* this fills a TLB entry from the IOMMU: */
void tme_stp222x_iommu_tlb_fill _TME_P((struct tme_bus_connection *, struct tme_bus_tlb *, tme_bus_addr_t, unsigned int));

/* the IOMMU register handler: */
void tme_stp222x_iommu_regs _TME_P((struct tme_stp222x *, struct tme_stp222x_reg *));

/* the IOMMU diagnostic register handler: */
void tme_stp222x_iommu_regs_diag _TME_P((struct tme_stp222x *, struct tme_stp222x_reg *));

/* this initializes the IOMMU: */
void tme_stp222x_iommu_init _TME_P((struct tme_stp222x *));

/* this flushes a streaming cache: */
int tme_stp222x_stc_flush _TME_P((struct tme_stp222x *));

/* the STC register handler: */
void tme_stp222x_stc_regs _TME_P((struct tme_stp222x *, unsigned long, struct tme_stp222x_reg *));

/* the STC diagnostic register handler: */
void tme_stp222x_stc_regs_diag _TME_P((struct tme_stp222x *, unsigned long, struct tme_stp222x_reg *));

/* this initializes a streaming cache: */
void tme_stp222x_stc_init _TME_P((struct tme_stp222x_stc *));

/* these search address ranges for an address: */
tme_uint32_t tme_stp222x_asearch32 _TME_P((const struct tme_stp222x_arange *, tme_uint32_t, tme_bus_addr32_t));
tme_uint32_t tme_stp222x_asearch64 _TME_P((const struct tme_stp222x_arange *, tme_uint32_t, tme_bus_addr64_t));

/* this searches an address space: */
tme_uint32_t tme_stp222x_aspace_search _TME_P((const struct tme_stp222x_aspace *, tme_bus_addr64_t));

/* this rebuilds the address spaces: */
int tme_stp222x_aspaces_rebuild _TME_P((struct tme_stp222x *));

/* this looks up the slave connection for an address in a space, and
   adjusts the address to be slave-relative: */
tme_uint32_t tme_stp222x_aspace_lookup _TME_P((const struct tme_stp222x *, unsigned int, tme_bus_addr64_t *));

#endif /* !_STP222X_IMPL_H */
