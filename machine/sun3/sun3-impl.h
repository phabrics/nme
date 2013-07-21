/* $Id: sun3-impl.h,v 1.4 2009/08/30 14:19:55 fredette Exp $ */

/* machine/sun3/sun3-impl.h - implementation header file for Sun 3 emulation: */

/*
 * Copyright (c) 2003, 2004 Matt Fredette
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

#ifndef _MACHINE_SUN3_IMPL_H
#define _MACHINE_SUN3_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: sun3-impl.h,v 1.4 2009/08/30 14:19:55 fredette Exp $");

/* includes: */
#include <tme/generic/bus.h>
#include <tme/machine/sun.h>
#include <tme/ic/m68k.h>
#include <tme/element.h>

/* macros: */

/* real sun3 control space addresses: */
#define TME_SUN3_CONTROL_IDPROM		(0x00000000)	/* the IDPROM */
#define TME_SUN3_CONTROL_PGMAP		(0x10000000)	/* the page map */
#define TME_SUN3_CONTROL_SEGMAP		(0x20000000)	/* the segment map */
#define TME_SUN3_CONTROL_CONTEXT	(0x30000000)	/* the context register */
#define TME_SUN3_CONTROL_ENABLE		(0x40000000)	/* the enable register */
#define TME_SUN3_CONTROL_UDVMA	 	(0x50000000)	/* the user DVMA enable register */
#define TME_SUN3_CONTROL_BUSERR		(0x60000000)	/* the bus error register */
#define TME_SUN3_CONTROL_DIAG		(0x70000000)	/* the diagnostic register */
#define TME_SUN3_CONTROL_VAC_TAGS	(0x80000000)	/* the VAC tags */
#define TME_SUN3_CONTROL_VAC_DATA	(0x90000000)	/* the VAC data */
#define TME_SUN3_CONTROL_VAC_FLUSH	(0xa0000000)	/* the VAC flush address */
#define TME_SUN3_CONTROL_COPY		(0xb0000000)	/* the block copy hardware */
					/* 0xc0000000 unused */
					/* 0xd0000000 unused */
					/* 0xe0000000 unused */
#define TME_SUN3_CONTROL_UART_BYPASS	(0xf0000000)	/* the special UART bypass */

/* this converts a sun3 control space address into the register number: */
#define TME_SUN3_CONTROL_REG(address)	((address) >> 28)

/* this masks a sun3 control space address into a virtual address: */
#define TME_SUN3_CONTROL_MASK_ADDRESS	(0x0ffffffc)

/* real sun3 enable register bits: */
#define TME_SUN3_ENA_DIAG		(0x01)	/* diagnostic switch (read-only) */
#define TME_SUN3_ENA_FPA		(0x02)	/* enable FPA */
#define TME_SUN3_ENA_COPY		(0x04)	/* enable copy update mode */
#define TME_SUN3_ENA_VIDEO		(0x08)	/* enable video display */
#define TME_SUN3_ENA_CACHE		(0x10)	/* enable external cache */
#define TME_SUN3_ENA_SDVMA		(0x20)	/* enable system DVMA */
#define TME_SUN3_ENA_FPP		(0x40)	/* enable 6888x */
#define TME_SUN3_ENA_NOTBOOT		(0x80)	/* non-boot state */

/* real sun3 interrupt register bits: */
#define TME_SUN3_IREG_INTS_ENAB		(0x01)  /* enable interrupts */
#define TME_SUN3_IREG_SOFT_INT_1	(0x02)  /* enable level 1 soft interrupts */
#define TME_SUN3_IREG_SOFT_INT_2	(0x04)  /* enable level 2 soft interrupts */
#define TME_SUN3_IREG_SOFT_INT_3	(0x08)  /* enable level 3 soft interrupts */
#define TME_SUN3_IREG_VIDEO_ENAB	(0x10)	/* enable video */
#define TME_SUN3_IREG_CLOCK_ENAB_5	(0x20)	/* enable clock interrupts */
					/* 0x40 unused */
#define TME_SUN3_IREG_CLOCK_ENAB_7	(0x80)	/* enable clock NMI interrupts */

/* real sun3 memory error register parts: */
#define TME_SUN3_MEMERR_REG_CSR		(0)
#define TME_SUN3_MEMERR_SIZ_CSR		(sizeof(tme_uint8_t))
#define TME_SUN3_MEMERR_REG_VADDR	(4)
#define TME_SUN3_MEMERR_SIZ_VADDR	(sizeof(tme_uint32_t))
#define TME_SUN3_MEMERR_SIZ_REG		(TME_SUN3_MEMERR_REG_VADDR + TME_SUN3_MEMERR_SIZ_VADDR)

/* real sun3 memory error control register bits: */
#define TME_SUN3_MEMERR_X_INT_ACTIVE	(0x80)	/* interrupt is active */
#define TME_SUN3_MEMERR_X_ENABLE_INT 	(0x40)	/* enable memory error interrupts */
#define TME_SUN3_MEMERR_PAR_TEST 	(0x20)	/* write inverse parity */
#define TME_SUN3_MEMERR_PAR_ENABLE	(0x10)	/* enable parity checking */
#define TME_SUN3_MEMERR_PAR_ERR_BL3 	(0x08)	/* parity error in D24..D31 */
#define TME_SUN3_MEMERR_PAR_ERR_BL2 	(0x04)	/* parity error in D16..D23 */
#define TME_SUN3_MEMERR_PAR_ERR_BL1 	(0x02)	/* parity error in D8..D15 */
#define TME_SUN3_MEMERR_PAR_ERR_BL0 	(0x01)	/* parity error in D0..D7 */

/* the page size: */
#define TME_SUN3_PAGE_SIZE_LOG2		(13)
#define TME_SUN3_PAGE_SIZE		(1 << TME_SUN3_PAGE_SIZE_LOG2)

/* the number of PMEGs: */
#define TME_SUN3_PMEGS			(256)

/* the PROM location: */
#define TME_SUN3_PROM_BASE		(0x0FEF0000)
#define TME_SUN3_PROM_SIZE		(0x00010000)

/* the obio addresses of zs0 and the PROM: */
#define TME_SUN3_OBIO_ZS0		(0x00020000)
#define TME_SUN3_OBIO_PROM		(0x00100000)

/* identifiers for the different mainbus connections.  the buses are
   together at the beginning of the value space: */
#define TME_SUN3_CONN_BUS_OBIO		(0)
#define TME_SUN3_CONN_BUS_OBMEM		(1)
#define TME_SUN3_CONN_BUS_VME		(2)
#define  TME_SUN3_CONN_BUS_COUNT	(3)
#define TME_SUN3_CONN_OBIO_MASTER	(3)
#define TME_SUN3_CONN_REG_MEMERR	(4)
#define TME_SUN3_CONN_REG_INTREG	(5)

/* the DVMA sizes: */
#define TME_SUN3_DVMA_SIZE_OBIO		(0x01000000)
#define TME_SUN3_DVMA_SIZE_VME		(0x00100000)

#define TME_SUN3_LOG_HANDLE(sun3) (&(sun3)->tme_sun3_element->tme_element_log_handle)

/* types: */

/* a sun3 mainbus connection: */
struct tme_sun3_bus_connection {

  /* the generic bus connection: */
  struct tme_bus_connection tme_sun3_bus_connection;

  /* what kind of connection this is: */
  unsigned int tme_sun3_bus_connection_which;
};

/* a sun3: */
struct tme_sun3 {

  /* backpointer to our element: */
  struct tme_element *tme_sun3_element;

  /* the IDPROM: */
  tme_uint8_t tme_sun3_idprom_contents[TME_SUN_IDPROM_SIZE];

  /* the MMU: */
  void *tme_sun3_mmu;

  /* the CPU: */
  struct tme_m68k_bus_connection *tme_sun3_m68k;

  /* the different buses: */
  struct tme_bus_connection *tme_sun3_buses[TME_SUN3_CONN_BUS_COUNT];
#define tme_sun3_obio tme_sun3_buses[TME_SUN3_CONN_BUS_OBIO]
#define tme_sun3_obmem tme_sun3_buses[TME_SUN3_CONN_BUS_OBMEM]
#define tme_sun3_vmebus tme_sun3_buses[TME_SUN3_CONN_BUS_VME]

  /* the context register: */
  tme_uint8_t tme_sun3_context;

  /* the enable register: */
  tme_uint8_t tme_sun3_enable;

  /* the UDVMA register: */
  tme_uint8_t tme_sun3_udvma;

  /* the bus error register: */
  tme_uint8_t tme_sun3_buserr;

  /* the diagnostic register: */
  tme_uint8_t tme_sun3_diag;

  /* the interrupt register: */
  tme_uint8_t tme_sun3_ints;

  /* the memory error register: */
  tme_uint8_t tme_sun3_memerr_csr;
  tme_uint32_t tme_sun3_memerr_vaddr;
  unsigned int tme_sun3_memerr_int_asserted;
  struct tme_bus_connection *tme_sun3_memerr_bus;

  /* memory error register test state: */
  struct tme_bus_tlb *tme_sun3_memerr_tlb;
  void *tme_sun3_memerr_cycle_private;
  tme_bus_cycle_handler tme_sun3_memerr_cycle;
  tme_uint8_t tme_sun3_memerr_pending_csr;
  tme_uint32_t tme_sun3_memerr_pending_vaddr;

  /* the interrupt lines that are being asserted: */
  tme_uint8_t tme_sun3_int_signals[(TME_M68K_IPL_MAX + 1 + 7) >> 3];

  /* the last ipl we gave to the CPU: */
  unsigned int tme_sun3_int_ipl_last;

  /* the last clock interrupt bus signal: */
  unsigned int tme_sun3_int_signal_clock_last;

  /* the set of active SDVMA TLB entries: */
  unsigned int tme_sun3_sdvma_tlb_next;
#define TME_SUN3_SDVMA_TLBS		(16)
  struct tme_token *tme_sun3_sdvma_tlb_tokens[TME_SUN3_SDVMA_TLBS];

  /* the m68k bus context register: */
  tme_bus_context_t *tme_sun3_m68k_bus_context;
};

/* prototypes: */
void _tme_sun3_mmu_new _TME_P((struct tme_sun3 *));
int _tme_sun3_m68k_tlb_fill _TME_P((struct tme_m68k_bus_connection *, struct tme_m68k_tlb *,
				    unsigned int, tme_uint32_t, unsigned int));
int _tme_sun3_bus_tlb_fill _TME_P((struct tme_bus_connection *, struct tme_bus_tlb *,
				   tme_bus_addr_t, unsigned int));
int _tme_sun3_mmu_tlb_set_add _TME_P((struct tme_bus_connection *,
				      struct tme_bus_tlb_set_info *));
int _tme_sun3_mmu_pte_get _TME_P((struct tme_sun3 *, tme_uint32_t, tme_uint32_t *));
int _tme_sun3_mmu_pte_set _TME_P((struct tme_sun3 *, tme_uint32_t, tme_uint32_t));
void _tme_sun3_mmu_sdvma_change _TME_P((struct tme_sun3 *));
void _tme_sun3_mmu_context_set _TME_P((struct tme_sun3 *));

int _tme_sun3_control_cycle_handler _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun3_intreg_cycle_handler _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun3_memerr_cycle_handler _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun3_memerr_test_cycle_handler _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun3_ipl_check _TME_P((struct tme_sun3 *));

#endif /* !_MACHINE_SUN3_IMPL_H */
