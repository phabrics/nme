/* $Id: sun4-impl.h,v 1.3 2009/08/30 14:01:55 fredette Exp $ */

/* machine/sun4/sun4-impl.h - implementation header file for Sun 4 emulation: */

/*
 * Copyright (c) 2005, 2006 Matt Fredette
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

#ifndef _MACHINE_SUN4_IMPL_H
#define _MACHINE_SUN4_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: sun4-impl.h,v 1.3 2009/08/30 14:01:55 fredette Exp $");

/* includes: */
#include <tme/generic/bus.h>
#include <tme/machine/sun.h>
#include <tme/ic/sparc.h>
#include <tme/element.h>
#include <sys/types.h>
#include <sys/time.h>

/* macros: */

/* real sun4 ASIs: */
					/* 0x00 unused */
					/* 0x01 unused */
#define TME_SUN4_32_ASI_CONTROL		(0x02)		/* all 32-bit sun4: control space */
#define TME_SUN44C_ASI_SEGMAP		(0x03)		/* sun4/4c: the segment map */
#define TME_SUN44C_ASI_PGMAP		(0x04)		/* sun4/4c: the page map */
#define TME_SUN4_ASI_COPY		(0x05)		/* sun4:  block copy */
#define TME_SUN4C_ASI_HW_FLUSH_SEG	(0x05)		/* sun4c: hardware-assisted flush segment */
#define TME_SUN4_ASI_REGMAP		(0x06)		/* sun4:  region map */
#define TME_SUN4C_ASI_HW_FLUSH_PG	(0x06)		/* sun4c: hardware-assisted flush page */
#define TME_SUN4_ASI_FLUSH_REG		(0x07)		/* sun4:  flush region */
#define TME_SUN4C_ASI_HW_FLUSH_CONTEXT	(0x07)		/* sun4c: hardware-assisted flush context */
					/* 0x08 is TME_SPARC32_ASI_UI */
					/* 0x09 is TME_SPARC32_ASI_SI */
					/* 0x0a is TME_SPARC32_ASI_UD */
					/* 0x0b is TME_SPARC32_ASI_SD */
#define TME_SUN44C_ASI_FLUSH_SEG	(0x0c)		/* sun4/4c: flush segment */
#define TME_SUN44C_ASI_FLUSH_PG		(0x0d)		/* sun4/4c: flush page */
#define TME_SUN44C_ASI_FLUSH_CONTEXT	(0x0e)		/* sun4/4c: flush context */
#define TME_SUN4_ASI_FLUSH_USER		(0x0f)		/* sun4:  flush user */
#define TME_SUN4C_ASI_HW_FLUSH_ALL	(0x0f)		/* sun4c: hardware-assisted flush all */
#define TME_SUN4_32_ASI_COUNT		(0x3a)		/* all 32-bit sun4: count of ASIs */

/* real sun4 ASI_CONTROL space addresses: */
#define TME_SUN4_CONTROL_IDPROM		(0x00000000)	/* sun4: the IDPROM */
					/* 0x10000000 was the sun3 page map */
					/* 0x20000000 was the sun3 segment map */
#define TME_SUN44C_CONTROL_CONTEXT	(0x30000000)	/* sun4/4c: the context register */
#define TME_SUN44C_CONTROL_ENABLE	(0x40000000)	/* sun4/4c: the enable register */
#define TME_SUN4_CONTROL_UDVMA		(0x50000000)	/* sun4:  the user DVMA enable register */
#define TME_SUN4_CONTROL_BUSERR		(0x60000000)	/* sun4:  the bus error register */
#define TME_SUN4C_CONTROL_SYNC_ERR	(0x60000000)	/* sun4c: the synchronous error register */
#define TME_SUN4C_CONTROL_SYNC_VADDR	(0x60000004)	/* sun4c: the synchronous error virtual address register */
#define TME_SUN4C_CONTROL_ASYNC_ERR	(0x60000008)	/* sun4c: the asynchronous error register */
#define TME_SUN4C_CONTROL_ASYNC_VADDR	(0x6000000c)	/* sun4c: the asynchronous error virtual address register */
#define TME_SUN4C_CONTROL_ASYNC_DATA_LO	(0x60000010)	/* sun4c: the asynchronous error low data register */
#define TME_SUN4C_CONTROL_ASYNC_DATA_HI	(0x60000014)	/* sun4c: the asynchronous error low data register */
#define TME_SUN4_CONTROL_DIAG		(0x70000000)	/* sun4:  the diagnostic register */
#define TME_SUN44C_CONTROL_CACHE_TAGS	(0x80000000)	/* sun4/4c: the VAC tags */
#define TME_SUN44C_CONTROL_CACHE_DATA	(0x90000000)	/* sun4/4c: the VAC data */
					/* 0xa0000000 was the sun3 VAC flush */
					/* 0xb0000000 was the sun3 block copy hardware */
					/* 0xc0000000 unused */
#define TME_SUN4_CONTROL_UDVMA_MAP	(0xd0000000)	/* sun4:  the user DVMA map */
#define TME_SUN4_CONTROL_VME_INTVEC	(0xe0000000)	/* sun4:  the VME interrupt vector */
#define TME_SUN44C_CONTROL_UART_BYPASS	(0xf0000000)	/* sun4/4c: the special UART bypass */

/* this converts a sun4/4c control space address into the register number: */
#define TME_SUN44C_CONTROL_REG(address)	((address) >> 28)

/* real sun4/4c enable register bits: */
#define TME_SUN4_ENA_DIAG		(0x01)	/* sun4:  diagnostic switch (read-only) */
#define TME_SUN4_ENA_MONITOR		(0x01)  /* sun4:  "monitor bit" (write-only) */
#define TME_SUN4_ENA_RESET_VME		(0x02)	/* sun4:  reset the VME bus */
#define TME_SUN4_ENA_RESET_CACHE	(0x04)	/* sun4:  reset the cache */
#define TME_SUN4C_ENA_RESET_SW		(0x04)  /* sun4c: software reset */
#define TME_SUN4_ENA_VIDEO		(0x08)	/* sun4:  enable video display */
#define TME_SUN44C_ENA_CACHE		(0x10)	/* sun4/4c: enable external cache */
#define TME_SUN44C_ENA_SDVMA		(0x20)	/* sun4/4c: enable system DVMA */
#define TME_SUN4_ENA_IOCACHE		(0x40)	/* sun4:  enable the I/O cache */
#define TME_SUN44C_ENA_NOTBOOT		(0x80)	/* sun4/4c: non-boot state */

/* real sun4/4c interrupt register bits: */
#define TME_SUN44C_IREG_INTS_ENAB	(0x01)  /* sun4/4c: enable interrupts */
#define TME_SUN44C_IREG_SOFT_INT_L1	(0x02)  /* sun4/4c: enable level 1 soft interrupts */
#define TME_SUN44C_IREG_SOFT_INT_L4	(0x04)  /* sun4/4c: enable level 4 soft interrupts */
#define TME_SUN44C_IREG_SOFT_INT_L6	(0x08)  /* sun4/4c: enable level 6 soft interrupts */
#define TME_SUN44C_IREG_VIDEO_INT	(0x10)	/* sun4/4c: enable level 8 video interrupts */
#define TME_SUN44C_IREG_COUNTER_L10	(0x20)	/* sun4/4c: enable counter0 level 10 interrupts */
					/* 0x40 unused */
#define TME_SUN44C_IREG_COUNTER_L14	(0x80)	/* sun4/4c: enable counter1 level 14 interrupts */

/* real sun4/4c memory error register parts: */
#define TME_SUN44C_MEMERR_REG_CSR	(0)
#define TME_SUN44C_MEMERR_SIZ_CSR	(sizeof(tme_uint32_t))
#define TME_SUN4C_MEMERR_REG_PARCTL	(4)
#define TME_SUN4C_MEMERR_SIZ_PARCTL	(sizeof(tme_uint32_t))
#define TME_SUN4_MEMERR_REG_VADDR	(4)
#define TME_SUN4_MEMERR_SIZ_VADDR	(sizeof(tme_uint32_t))
#define TME_SUN44C_MEMERR_SIZ_REG	(TME_SUN4_MEMERR_REG_VADDR + TME_SUN4_MEMERR_SIZ_VADDR)

/* real sun4/4c parity and ECC memory error control register bits: */
#define TME_SUN4_MEMERR_X_CONTEXT_MASK	(0x1fe00) /* sun4: context mask */
#define TME_SUN4_MEMERR_X_DVMA		(0x100)	/* sun4: access was DVMA */
#define TME_SUN4_MEMERR_X_INT_ACTIVE	(0x80)	/* sun4: interrupt is active */
#define TME_SUN4C_MEMERR_PAR_ERROR	(0x80)  /* sun4c: parity error detected */
#define TME_SUN4_MEMERR_X_ENABLE_INT 	(0x40)	/* sun4: enable memory error interrupts */
#define TME_SUN4C_MEMERR_PAR_MULTI	(0x40)	/* sun4c: multiple parity errors detected */
#define TME_SUN44C_MEMERR_PAR_TEST 	(0x20)	/* sun4/4c: write inverse parity */
#define TME_SUN44C_MEMERR_PAR_ENABLE	(0x10)	/* sun4/4c: enable parity checking */
#define TME_SUN44C_MEMERR_PAR_ERR_BL3 	(0x08)	/* sun4/4c: parity error in (sun4) D24..D31 (sun4c) D0..D7 */
#define TME_SUN44C_MEMERR_PAR_ERR_BL2 	(0x04)	/* sun4/4c: parity error in (sun4) D16..D23 (sun4c) D8..D15 */
#define TME_SUN44C_MEMERR_PAR_ERR_BL1 	(0x02)	/* sun4/4c: parity error in (sun4) D8..D15  (sun4c) D16..D23 */
#define TME_SUN44C_MEMERR_PAR_ERR_BL0 	(0x01)	/* sun4/4c: parity error in (sun4) D0..D7   (sun4c) D24..D31 */

/* real sun4/4c/4m timer register parts: */
#define TME_SUN4_32_TIMER_REG_COUNTER	(0)
#define TME_SUN4_32_TIMER_SIZ_COUNTER	(sizeof(tme_uint32_t))
#define TME_SUN4_32_TIMER_REG_LIMIT	(TME_SUN4_32_TIMER_REG_COUNTER + TME_SUN4_32_TIMER_SIZ_COUNTER)
#define TME_SUN4_32_TIMER_SIZ_LIMIT	(sizeof(tme_uint32_t))
#define TME_SUN44C_TIMER_SIZ_REG	(TME_SUN4_32_TIMER_REG_LIMIT + TME_SUN4_32_TIMER_SIZ_LIMIT)

/* the page sizes: */
#define TME_SUN4_PAGE_SIZE_LOG2		(13)
#define TME_SUN4_PAGE_SIZE		(1 << TME_SUN4_PAGE_SIZE_LOG2)
#define TME_SUN4C_PAGE_SIZE_LOG2	(12)
#define TME_SUN4C_PAGE_SIZE		(1 << TME_SUN4C_PAGE_SIZE_LOG2)

/* all 32-bit sun4s have the same segment size: */
#define TME_SUN4_32_SEGMENT_SIZE_LOG2	(18)
#define TME_SUN4_32_SEGMENT_SIZE	(1 << TME_SUN4_32_SEGMENT_SIZE_LOG2)

/* real sun4/4c PTE entry bits: */
#define TME_SUN44C_PTE_VALID		(0x80000000)
#define TME_SUN44C_PTE_WRITE		(0x40000000)
#define TME_SUN44C_PTE_SYSTEM		(0x20000000)
#define TME_SUN44C_PTE_NC		(0x10000000)
#define TME_SUN44C_PTE_PGTYPE		(0x0C000000)
#define TME_SUN44C_PTE_REF		(0x02000000)
#define TME_SUN44C_PTE_MOD		(0x01000000)
#define TME_SUN4_PTE_PGFRAME		(0x0007FFFF)
#define TME_SUN4C_PTE_PGFRAME		(0x0000FFFF)

/* the PROM location: */
#define TME_SUN44C_PROM_BASE		(0xF6000000)
#define TME_SUN44C_PROM_SIZE		(0x00040000)

/* the obio addresses of zs0 and zs1: */
#define TME_SUN44C_OBIO_ZS0		(0xf1000000)

/* the obio address of the start of the SBus slots: */
#define TME_SUN4C_OBIO_SBUS		(0xf8000000)

/* identifiers for the different board bus connections.  the buses are
   together at the beginning of the value space: */
#define TME_SUN4_32_CONN_BUS_OBIO	(0)
#define TME_SUN4_32_CONN_BUS_OBMEM	(1)
#define TME_SUN4_CONN_BUS_VME		(2)
#define TME_SUN4_32_CONN_BUS_COUNT	(3)
#define TME_SUN4_32_CONN_REG_TIMER	(3)
#define TME_SUN4_32_CONN_REG_MEMERR	(4)
#define TME_SUN4_32_CONN_REG_INTREG	(5)
#define TME_SUN4C4M_CONN_REG_AUXREG	(6)
#define TME_SUN4_32_CONN_REG_COUNT	(7)

/* the DVMA sizes: */
#define TME_SUN4_DVMA_SIZE_VME		(0x00100000)

/* these return nonzero on a match of the IDPROM machine type byte: */
#define TME_SUN4_IS_ARCH(sun4, arch)	\
  (((sun4)->tme_sun4_idprom_contents[TME_SUN_IDPROM_OFF_MACHTYPE] & TME_SUN_IDPROM_TYPE_MASK_ARCH) == (arch))
#define TME_SUN4_IS_MODEL(sun4, model)	\
  ((sun4)->tme_sun4_idprom_contents[TME_SUN_IDPROM_OFF_MACHTYPE] == (model))
#define TME_SUN4_IS_SUN4(sun4)		TME_SUN4_IS_ARCH(sun4, TME_SUN_IDPROM_TYPE_ARCH_SUN4)
#define TME_SUN4_IS_SUN4C(sun4)		TME_SUN4_IS_ARCH(sun4, TME_SUN_IDPROM_TYPE_ARCH_SUN4C)
#define TME_SUN4_IS_SUN44C(sun4)	TRUE
#define TME_SUN4_IS_SUN4M(sun4)		FALSE
#define TME_SUN4_IS_SUN4C4M(sun4)	(TME_SUN4_IS_SUN4C(sun4) || TME_SUN4_IS_SUN4M(sun4))

/* this returns the MMU context used by a particular bus connection: */
#define TME_SUN44C_BUS_MMU_CONTEXT(sun4, conn_bus)	((sun4)->tme_sun44c_context)

/* these returns nonzero if memory error testing is visible: */
/* on the sun4/4c, memory error testing is visible if there are any
   bad addresses and parity checking is enabled, or if bad parity
   writing is enabled: */
#define TME_SUN44C_MEMERR_VISIBLE(sun4)			\
  ((sun4)->tme_sun4_memerr_bad_memory_count > 0		\
   || (((sun4)->tme_sun44c_memerr_csr[0]		\
	| (sun4)->tme_sun44c_memerr_csr[1])		\
       & TME_SUN44C_MEMERR_PAR_TEST) != 0)

#define TME_SUN4_LOG_HANDLE(sun4)	(&(sun4)->tme_sun4_element->tme_element_log_handle)

/* types: */

/* a sun4 bus connection: */
struct tme_sun4_bus_connection {

  /* the generic bus connection: */
  struct tme_bus_connection tme_sun4_bus_connection;

  /* what kind of connection this is: */
  unsigned int tme_sun4_bus_connection_which;
};

/* a sun4 timer: */
struct tme_sun4_timer {

  /* a backpointer to the sun4: */
  struct tme_sun4 *tme_sun4_timer_sun4;

  /* the real counter and limit register values: */
  tme_uint32_t tme_sun4_timer_counter;
  tme_uint32_t tme_sun4_timer_limit;

  /* the period of this timer: */
  struct timeval tme_sun4_timer_period;

  /* when the timer reaches its next limit: */
  struct timeval tme_sun4_timer_limit_next;

  /* a condition for waking up the thread for this timer: */
  tme_cond_t tme_sun4_timer_cond;

  /* this is nonzero if the interrupt for this timer is asserted: */
  unsigned int tme_sun4_timer_int_asserted;

  /* these are used to track the interrupt rate for this timer: */
  tme_uint32_t tme_sun4_timer_track_ints;
  struct timeval tme_sun4_timer_track_sample;
};

/* a sun4: */
struct tme_sun4 {

  /* our mutex: */
  tme_mutex_t tme_sun4_mutex;

  /* backpointer to our element: */
  struct tme_element *tme_sun4_element;

  /* the IDPROM: */
  tme_uint8_t tme_sun4_idprom_contents[TME_SUN_IDPROM_SIZE];

  /* the CPU: */
  struct tme_sparc_bus_connection *tme_sun4_sparc;

  /* a set of bus connections: */
  struct tme_bus_connection *tme_sun4_buses[TME_SUN4_32_CONN_REG_COUNT];
#define tme_sun4_32_obio tme_sun4_buses[TME_SUN4_32_CONN_BUS_OBIO]
#define tme_sun4_32_obmem tme_sun4_buses[TME_SUN4_32_CONN_BUS_OBMEM]
#define tme_sun4_vmebus tme_sun4_buses[TME_SUN4_CONN_BUS_VME]

  /* these dummy bus connection structures are used to thread bus
     information to bus error handlers: */
  struct tme_connection tme_sun4_dummy_connection_sparc;

  /* these structures are used to thread ASI information to a control
     cycle handler.  if the backpointer for an ASI is NULL, the ASI is
     undefined: */
  struct tme_sun4_asi {
    struct tme_sun4 *tme_sun4_asi_sun4;
  } tme_sun4_asis[TME_SUN4_32_ASI_COUNT];

  /* the current TLB fill function: */
  int (*tme_sun4_tlb_fill) _TME_P((const struct tme_bus_connection *,
				   struct tme_bus_tlb *,
				   tme_uint32_t *,
				   tme_uint32_t,
				   unsigned int));

  /* visible memory test support: */
  struct tme_bus_tlb *tme_sun4_memtest_tlb;
  tme_uint32_t tme_sun4_memtest_tlb_asi_mask;

  /* cache support: */
  unsigned int tme_sun4_cache_size_log2;
  unsigned int tme_sun4_cache_size_line_log2;
  unsigned int tme_sun4_cache_writeback;
  tme_shared tme_uint8_t *tme_sun4_cache_data;
  tme_rwlock_t tme_sun4_cache_rwlock;
  tme_uint32_t tme_sun4_cache_visible;
  struct tme_bus_tlb tme_sun4_cache_tlb_internal;
  struct tme_token tme_sun4_cache_tlb_internal_token;

  /* memory error support: */
  unsigned int tme_sun4_memerr_int_asserted;
  const tme_shared tme_uint8_t *tme_sun4_memerr_bad_memory[128];
  unsigned int tme_sun4_memerr_bad_memory_count;
  const tme_shared tme_uint8_t *tme_sun4_memerr_tlb_emulator_off_read;
  tme_shared tme_uint8_t *tme_sun4_memerr_tlb_emulator_off_write;

  /* timer support: */
  unsigned int tme_sun4_timer_callouts_running;
  struct tme_sun4_timer tme_sun4_timers[2];
#define tme_sun4_timer_l10 tme_sun4_timers[0]
#define tme_sun4_timer_l14 tme_sun4_timers[1]

  /* the sun4/4c MMU and context register: */
  void *tme_sun44c_mmu;
  tme_uint32_t tme_sun44c_mmu_pmegs;
  tme_uint8_t tme_sun44c_context;

  /* the sun4/4c enable register: */
  tme_uint8_t tme_sun44c_enable;

  /* the sun4/4c UDVMA register: */
  tme_uint8_t tme_sun4_udvma;

  /* the sun4 bus error register: */
  tme_uint8_t tme_sun4_buserr;

  /* the sun4 diagnostic register: */
  tme_uint8_t tme_sun4_diag;

  /* the sun4/4c interrupt register: */
  tme_uint8_t tme_sun44c_ints;

  /* the sun4c/4m auxiliary register: */
  tme_uint8_t tme_sun4c4m_aux;

  /* the sun4/4c cache: */
  tme_uint32_t *tme_sun44c_cache_tags;

  /* the sun4c synchronous and asynchronous error registers: */
  tme_uint32_t tme_sun4c_sync_err;
  tme_uint32_t tme_sun4c_sync_vaddr;
  tme_uint32_t tme_sun4c_async_err;
  tme_uint32_t tme_sun4c_async_vaddr;
  tme_uint32_t tme_sun4c_async_data_lo;
  tme_uint32_t tme_sun4c_async_data_hi;

  /* the sun4/4c memory error registers: */
  tme_uint32_t tme_sun44c_memerr_csr[2];
  tme_uint32_t tme_sun4c_memerr_parctl[2];
  tme_uint32_t tme_sun4_memerr_vaddr;

  /* the interrupt lines that are being asserted: */
  tme_uint8_t tme_sun4_int_signals[(TME_SPARC_IPL_MAX + 1 + 7) >> 3];

  /* the last ipl we gave to the CPU: */
  unsigned int tme_sun4_int_ipl_last;

  /* the set of active sun4/4c SDVMA TLB entries: */
  unsigned int tme_sun44c_sdvma_tlb_next;
#define TME_SUN44C_SDVMA_TLBS		(16)
  struct tme_token *tme_sun44c_sdvma_tlb_tokens[TME_SUN44C_SDVMA_TLBS];

  /* the sun4/4c sparc v7 bus context register: */
  tme_bus_context_t *tme_sun44c_sparc_bus_context;
};

/* sun4/4c cache prototypes: */
void _tme_sun44c_cache_new _TME_P((struct tme_sun4 *));
void _tme_sun44c_cache_enable_change _TME_P((struct tme_sun4 *));
int _tme_sun44c_cache_cycle_control _TME_P((struct tme_sun4 *, struct tme_bus_cycle *));
void _tme_sun44c_cache_cycle_flush _TME_P((struct tme_sun4 *sun4, tme_uint32_t, tme_uint32_t));

/* sun4/4c memory error prototypes: */
int _tme_sun44c_memerr_cycle_control _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun44c_memerr_cycle_bus _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun44c_memerr_check _TME_P((const struct tme_bus_connection *, tme_uint32_t, tme_uint32_t, const tme_shared tme_uint8_t *, unsigned int));
void _tme_sun44c_memerr_update _TME_P((struct tme_sun4 *, tme_uint32_t, const tme_shared tme_uint8_t *, unsigned int));
int _tme_sun44c_tlb_fill_memerr _TME_P((const struct tme_bus_connection *,
					struct tme_bus_tlb *,
					tme_uint32_t *,
					tme_uint32_t,
					unsigned int));

/* sun4/4c MMU prototypes: */
void _tme_sun44c_mmu_new _TME_P((struct tme_sun4 *));
int _tme_sun44c_mmu_tlb_set_add _TME_P((struct tme_bus_connection *,
					struct tme_bus_tlb_set_info *));
void _tme_sun44c_mmu_sdvma_change _TME_P((struct tme_sun4 *));
void _tme_sun44c_mmu_context_set _TME_P((struct tme_sun4 *));
int _tme_sun44c_mmu_pte_get _TME_P((struct tme_sun4 *, tme_uint32_t, tme_uint32_t *));
int _tme_sun44c_mmu_pte_set _TME_P((struct tme_sun4 *, tme_uint32_t, tme_uint32_t));
int _tme_sun44c_mmu_proterr _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun44c_tlb_fill_sparc _TME_P((struct tme_sparc_bus_connection *,
				       struct tme_sparc_tlb *,
				       tme_uint32_t asi_mask,
				       tme_bus_addr_t address,
				       unsigned int cycles));
int _tme_sun44c_tlb_fill_bus _TME_P((struct tme_bus_connection *,
				     struct tme_bus_tlb *,
				     tme_bus_addr_t,
				     unsigned int));
int _tme_sun44c_tlb_fill_mmu _TME_P((const struct tme_bus_connection *,
				     struct tme_bus_tlb *,
				     tme_uint32_t *,
				     tme_uint32_t,
				     unsigned int));
int _tme_sun44c_ob_fault_handler _TME_P((void *, struct tme_bus_tlb *, struct tme_bus_cycle *, int));

/* timer prototypes: */
void _tme_sun4_timer_new _TME_P((struct tme_sun4 *));
int _tme_sun4_timer_cycle_control _TME_P((void *, struct tme_bus_cycle *));
void _tme_sun4_timer_int_force _TME_P((struct tme_sun4 *, struct tme_sun4_timer *));

/* other prototypes: */
int _tme_sun44c_control_cycle_handler _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun44c_intreg_cycle_control _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun4c_auxreg_cycle_control _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun4_ipl_check _TME_P((struct tme_sun4 *));
int _tme_sun4_reset _TME_P((struct tme_sun4 *, int));

#endif /* !_MACHINE_SUN4_IMPL_H */
