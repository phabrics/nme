/* $Id: sun2-impl.h,v 1.10 2009/08/30 14:30:16 fredette Exp $ */

/* machine/sun2/sun2-impl.h - implementation header file for Sun 2 emulation: */

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

#ifndef _MACHINE_SUN2_IMPL_H
#define _MACHINE_SUN2_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: sun2-impl.h,v 1.10 2009/08/30 14:30:16 fredette Exp $");

/* includes: */
#include <tme/generic/bus.h>
#include <tme/generic/ic.h>
#include <tme/machine/sun.h>
#include <tme/ic/m68k.h>
#include <tme/element.h>

/* macros: */

/* the sun2 control register file can be thought of as 16-bit
   big-endian memory, so we use a struct tme_ic to hold the register
   file, and use the TME_IC_BUS_IREGN macros to map it using real sun2
   control addresses: */
#define _TME_SUN2_CONTROL_IREG8(addr)	tme_sun2_ic.tme_ic_ireg_uint8(TME_IC_BUS_IREG8(1, TME_ENDIAN_BIG, addr))
#define _TME_SUN2_CONTROL_IREG16(addr)	tme_sun2_ic.tme_ic_ireg_uint16(TME_IC_BUS_IREG16(1, TME_ENDIAN_BIG, addr))
#define tme_sun2_pgmap_hi	_TME_SUN2_CONTROL_IREG16(0x00000000)
#define tme_sun2_pgmap_lo	_TME_SUN2_CONTROL_IREG16(0x00000002)
#define tme_sun2_segmap		_TME_SUN2_CONTROL_IREG8(0x00000005)
#define tme_sun2_context_system	_TME_SUN2_CONTROL_IREG8(0x00000006)
#define tme_sun2_context_user	_TME_SUN2_CONTROL_IREG8(0x00000007)
#define tme_sun2_idprom		_TME_SUN2_CONTROL_IREG8(0x00000008)
#define tme_sun2_diag		_TME_SUN2_CONTROL_IREG8(0x0000000B)
#define tme_sun2_buserr		_TME_SUN2_CONTROL_IREG16(0x0000000C)
#define tme_sun2_enable		_TME_SUN2_CONTROL_IREG16(0x0000000E)
#define TME_SUN2_CONTROL_JUNK	0x00000010

/* real enable register bits: */
#define TME_SUN2_ENA_PAR_GEN		(0x01)	/* enable parity generation */
#define TME_SUN2_ENA_SOFT_INT_1		(0x02)	/* software interrupt on level 1 */
#define TME_SUN2_ENA_SOFT_INT_2		(0x04)	/* software interrupt on level 2 */
#define TME_SUN2_ENA_SOFT_INT_3		(0x08)	/* software interrupt on level 3 */
#define TME_SUN2_ENA_PAR_CHECK		(0x10)	/* enable parity checking and errors */
#define TME_SUN2_ENA_SDVMA		(0x20)	/* enable DVMA */
#define TME_SUN2_ENA_INTS		(0x40)	/* enable interrupts */
#define TME_SUN2_ENA_NOTBOOT		(0x80)	/* non-boot state */

/* this recovers the control address of a register.  it should
   optimize right down to a constant: */
#define TME_SUN2_CONTROL_ADDRESS(reg)	\
  TME_IC_IREG_BUS(1, TME_ENDIAN_BIG, reg, tme_sun2, tme_sun2_ic.)

/* this starts a bus cycle structure: */
#define TME_SUN2_CONTROL_BUS_CYCLE(sun2, addr, cycle)	\
  TME_IC_IREG_BUS_CYCLE(1, TME_ENDIAN_BIG, &sun2->tme_sun2_ic, addr, cycle)

/* the page size: */
#define TME_SUN2_PAGE_SIZE_LOG2	(11)
#define TME_SUN2_PAGE_SIZE	(1 << TME_SUN2_PAGE_SIZE_LOG2)

/* the PROM location: */
#define TME_SUN2_PROM_BASE	(0x00EF0000)
#define TME_SUN2_PROM_SIZE	(0x00010000)

/* identifiers for the different buses: */
#define TME_SUN2_BUS_OBIO	(0)
#define TME_SUN2_BUS_OBMEM	(1)
#define TME_SUN2_BUS_MBIO	(2)
#define TME_SUN2_BUS_MBMEM	(3)
#define TME_SUN2_BUS_VME	(4)
#define TME_SUN2_BUS_COUNT	(5)

/* the DVMA sizes: */
#define TME_SUN2_DVMA_SIZE_MBMEM	(0x00040000)
#define TME_SUN2_DVMA_SIZE_VME		(0x000F8000)

#define TME_SUN2_LOG_HANDLE(sun2) (&(sun2)->tme_sun2_element->tme_element_log_handle)

/* types: */

/* a sun2 mainbus connection: */
struct tme_sun2_bus_connection {

  /* the generic bus connection: */
  struct tme_bus_connection tme_sun2_bus_connection;

  /* which bus this is: */
  unsigned int tme_sun2_bus_connection_which;
};

/* a sun2: */
struct tme_sun2 {

  /* our IC data structure, containing our various registers: */
  struct tme_ic tme_sun2_ic;

  /* backpointer to our element: */
  struct tme_element *tme_sun2_element;

  /* nonzero if this is a VME sun2: */
  int tme_sun2_has_vme;

  /* the IDPROM: */
  tme_uint8_t tme_sun2_idprom_contents[TME_SUN_IDPROM_SIZE];

  /* the MMU: */
  void *tme_sun2_mmu;

  /* the CPU: */
  struct tme_m68k_bus_connection *tme_sun2_m68k;

  /* the different buses: */
  struct tme_bus_connection *tme_sun2_buses[TME_SUN2_BUS_COUNT];
#define tme_sun2_obio tme_sun2_buses[TME_SUN2_BUS_OBIO]
#define tme_sun2_obmem tme_sun2_buses[TME_SUN2_BUS_OBMEM]
#define tme_sun2_mbio tme_sun2_buses[TME_SUN2_BUS_MBIO]
#define tme_sun2_mbmem tme_sun2_buses[TME_SUN2_BUS_MBMEM]
#define tme_sun2_vmebus tme_sun2_buses[TME_SUN2_BUS_VME]

  /* the interrupt lines that are being asserted: */
  tme_uint8_t tme_sun2_int_signals[(TME_M68K_IPL_MAX + 1 + 7) >> 3];

  /* the last ipl we gave to the CPU: */
  unsigned int tme_sun2_int_ipl_last;

  /* the m68k bus context register: */
  tme_bus_context_t *tme_sun2_m68k_bus_context;
};

/* prototypes: */
void _tme_sun2_mmu_new _TME_P((struct tme_sun2 *));
int _tme_sun2_m68k_tlb_fill _TME_P((struct tme_m68k_bus_connection *, struct tme_m68k_tlb *,
				    unsigned int, tme_uint32_t, unsigned int));
int _tme_sun2_bus_tlb_fill _TME_P((struct tme_bus_connection *, struct tme_bus_tlb *,
				   tme_bus_addr_t, unsigned int));
int _tme_sun2_mmu_tlb_set_add _TME_P((struct tme_bus_connection *,
				      struct tme_bus_tlb_set_info *));
int _tme_sun2_mmu_pte_get _TME_P((struct tme_sun2 *, tme_uint32_t, tme_uint32_t *));
int _tme_sun2_mmu_pte_set _TME_P((struct tme_sun2 *, tme_uint32_t, tme_uint32_t));
void _tme_sun2_mmu_context_system_set _TME_P((struct tme_sun2 *));
void _tme_sun2_mmu_context_user_set _TME_P((struct tme_sun2 *));

int _tme_sun2_control_cycle_handler _TME_P((void *, struct tme_bus_cycle *));
int _tme_sun2_ipl_check _TME_P((struct tme_sun2 *));

#endif /* !_MACHINE_SUN2_IMPL_H */
