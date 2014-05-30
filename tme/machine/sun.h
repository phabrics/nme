/* $Id: sun.h,v 1.13 2009/08/30 13:54:40 fredette Exp $ */

/* tme/machine/sun.h - public header file for Sun emulation: */

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

#ifndef _TME_MACHINE_SUN_H
#define _TME_MACHINE_SUN_H

#include <tme/common.h>
_TME_RCSID("$Id: sun.h,v 1.13 2009/08/30 13:54:40 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/generic/bus.h>

/* macros: */

/* the size of the classic Sun IDPROM: */
#define TME_SUN_IDPROM_SIZE	(32)

/* the size of a classic Sun z8530: */
#define TME_SUN_Z8530_SIZE	(8)

/* classic Sun z8530 socket: */
#define TME_SUN_Z8530_SOCKET_INIT { \
  TME_Z8530_SOCKET_0, /* the socket version */ \
  0, /* socket flags */ \
  4, /* the bus address of channel A */ \
  0, /* the bus address of channel B */ \
  0, /* within each channel, the offset of the csr register */ \
  2, /* within each channel, the offset of the data register */ \
  3, /* the system bus byte lane the chip is wired to (D31-D24, may be overridden) */ \
  (9600 * 512) /* the clock rate provided to the chip: */ \
}

/* classic two-level Sun MMU protections: */
#define TME_SUN_MMU_PTE_PROT_MASK	(0x3)
#define  TME_SUN_MMU_PTE_PROT_ABORT	(0x0)	
#define  TME_SUN_MMU_PTE_PROT_ERROR	(0x1)
#define  TME_SUN_MMU_PTE_PROT_RO	(0x2)
#define  TME_SUN_MMU_PTE_PROT_RW	(0x3)

/* classic two-level Sun MMU page table entry bits: */
#define TME_SUN_MMU_PTE_PROT_USER(x)	((x) << 0)
#define TME_SUN_MMU_PTE_PROT_SYSTEM(x)	((x) << 2)
#define TME_SUN_MMU_PTE_VALID		TME_BIT(4)
#define TME_SUN_MMU_PTE_REF		TME_BIT(5)
#define TME_SUN_MMU_PTE_MOD		TME_BIT(6)
#define _TME_SUN_MMU_PTE_X(x)		((x) << 7)

/* a filled TLB entry can be valid for the system, or the user, or
   both.  because TME_BIT is a little too paranoid and uses a type
   cast, we can't use it and then use these macros in preprocessor
   expressions: */
#define TME_SUN_MMU_TLB_SYSTEM	(1 << 0)
#define TME_SUN_MMU_TLB_USER	(1 << 1)

/* real sun IDPROM offsets: */
#define TME_SUN_IDPROM_OFF_MACHTYPE		(0x1)

/* real sun IDPROM machine type architecture and model identifiers: */
#define TME_SUN_IDPROM_TYPE_MASK_ARCH		(0xf0)
#define TME_SUN_IDPROM_TYPE_ARCH_SUN4		(0x20)
#define TME_SUN_IDPROM_TYPE_ARCH_SUN4C		(0x50)
#define TME_SUN_IDPROM_TYPE_CODE_CALVIN		(0x55)

/* structures: */

/* one page table entry in a classic two-level Sun MMU: */
struct tme_sun_mmu_pte {

  /* the original raw page table entry: */
  tme_uint32_t tme_sun_mmu_pte_raw;

  /* a collection of flags: */
  unsigned short tme_sun_mmu_pte_flags;
};

/* the parameters for a classic two-level Sun MMU: */
struct tme_sun_mmu_info {

  /* the mainbus element: */
  struct tme_element *tme_sun_mmu_info_element;

  /* the number of bits in an address: */
  tme_uint8_t tme_sun_mmu_info_address_bits;

  /* the number of bits in a page offset: */
  tme_uint8_t tme_sun_mmu_info_pgoffset_bits;

  /* the number of bits in a page table entry index: */
  tme_uint8_t tme_sun_mmu_info_pteindex_bits;
 
  /* if greater than zero, the number of top address bits in a region
     map entry index.  if less than zero, the number of top address
     bits that must be the same, and define the address hole: */
  tme_int8_t tme_sun_mmu_info_topindex_bits;

  /* the number of contexts: */
  tme_uint8_t tme_sun_mmu_info_contexts;

  /* the number of PMEGs: */
  unsigned short tme_sun_mmu_info_pmegs;

  /* a TLB filler: */
  void *tme_sun_mmu_info_tlb_fill_private;
  int (*tme_sun_mmu_info_tlb_fill) _TME_P((void *, struct tme_bus_tlb *, struct tme_sun_mmu_pte *, tme_uint32_t *, unsigned int));

  /* the protection error cycle handler: */
  void *tme_sun_mmu_info_proterr_private;
  tme_bus_cycle_handler tme_sun_mmu_info_proterr;

  /* the page-invalid cycle handler: */
  void *tme_sun_mmu_info_invalid_private;
  tme_bus_cycle_handler tme_sun_mmu_info_invalid;
};

/* prototypes: */

/* MMU support: */
void *tme_sun_mmu_new _TME_P((struct tme_sun_mmu_info *));
int tme_sun_mmu_pte_set _TME_P((void *, tme_uint8_t, tme_uint32_t, struct tme_sun_mmu_pte *));
int tme_sun_mmu_pte_get _TME_P((void *, tme_uint8_t, tme_uint32_t, struct tme_sun_mmu_pte *));
void tme_sun_mmu_segmap_set _TME_P((void *, tme_uint8_t, tme_uint32_t, unsigned short));
unsigned short tme_sun_mmu_segmap_get _TME_P((void *, tme_uint8_t, tme_uint32_t));
unsigned short tme_sun_mmu_tlb_fill _TME_P((void *, struct tme_bus_tlb *, tme_uint8_t, tme_uint32_t, unsigned short));
void tme_sun_mmu_tlbs_invalidate _TME_P((void *));
void tme_sun_mmu_context_add _TME_P((void *, _tme_const struct tme_bus_tlb *));
void tme_sun_mmu_context_switched _TME_P((void *));
int tme_sun_mmu_tlb_set_add _TME_P((void *, struct tme_bus_tlb_set_info *));

/* onboard Intel Ethernet support: */
int tme_sun_obie _TME_P((struct tme_element *, _tme_const char * _tme_const *, char **));

/* bwtwo support: */
int tme_sun_bwtwo _TME_P((struct tme_element *, _tme_const char * _tme_const *, char **));

/* si support: */
int tme_sun_si _TME_P((struct tme_element *, _tme_const char * _tme_const *, char **));

/* cgtwo support: */
int tme_sun_cgtwo _TME_P((struct tme_element *, _tme_const char * _tme_const *, char **));

/* cgthree support: */
int tme_sun_cgthree _TME_P((struct tme_element *, _tme_const char * _tme_const *, char **));

/* cgsix support: */
int tme_sun_cgsix _TME_P((struct tme_element *, _tme_const char * _tme_const *, char **));

#endif /* !_TME_MACHINE_SUN_H */
