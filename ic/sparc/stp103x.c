/* $Id: stp103x.c,v 1.5 2010/06/05 18:57:04 fredette Exp $ */

/* ic/sparc/stp103x.c - implementation of UltraSPARC I (STP1030) and II (STP1031) emulation: */

/*
 * Copyright (c) 2008 Matt Fredette
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
_TME_RCSID("$Id: stp103x.c,v 1.5 2010/06/05 18:57:04 fredette Exp $");

/* includes: */
#include "sparc-impl.h"

/* macros: */

/* the PSTATE extensions: */
#define TME_STP103X_PSTATE_IG			TME_BIT(11)
#define TME_STP103X_PSTATE_MG			TME_BIT(10)

/* the load/store unit control register: */
#define TME_STP103X_LSU_IC			TME_BIT(0)
#define TME_STP103X_LSU_DC			TME_BIT(1)
#define TME_STP103X_LSU_IM			TME_BIT(2)
#define TME_STP103X_LSU_DM			TME_BIT(3)
#define TME_STP103X_LSU_FM			(TME_BIT(20) - TME_BIT(4))
#define TME_STP103X_LSU_VW			TME_BIT(21)
#define TME_STP103X_LSU_VR			TME_BIT(22)
#define TME_STP103X_LSU_PW			TME_BIT(23)
#define TME_STP103X_LSU_PR			TME_BIT(24)

/* the E-Cache (E-state) error enable register: */
#define TME_STP103X_ESTATE_ERROR_ENABLE_CEEN	TME_BIT(0)
#define TME_STP103X_ESTATE_ERROR_ENABLE_NCEEN	TME_BIT(1)
#define TME_STP103X_ESTATE_ERROR_ENABLE_ISAPEN	TME_BIT(2)

/* the start of the virtual address hole: */
#define TME_STP103X_VA_HOLE_START		(((tme_uint64_t) 1) << 43)

/* the size of the physical address space: */
#define TME_STP103X_PA_SIZE			(((tme_uint64_t) 1) << 41)

/* the maximum context number: */
#define TME_STP103X_CONTEXT_MAX			(8191)

/* the different page sizes: */
#define TME_STP103X_PAGE_SIZE_8KB		(8 * 1024)
#define TME_STP103X_PAGE_SIZE_64KB		(64 * 1024)
#define TME_STP103X_PAGE_SIZE_512KB		(512 * 1024)
#define TME_STP103X_PAGE_SIZE_4MB		(4 * 1024 * 1024)

/* because we often deal with the tme_uint64_t TLB/TTE entry tag and
   data as tme_uint32_t halves for performance, we need to easily
   generate the right bitfield masks for both types.  this macro
   converts the constant x to the type of e, and then shifts it by
   (shift mod (8 * sizeof(e))).  e is only typed, never evaluated: */
#define _TME_STP103X_TLB_X(e, x, shift)		((1 ? (x) : (e)) << ((shift) % (8 * sizeof(e))))

/* TLB/TTE data: */
#define TME_STP103X_TLB_DATA_V(e)		_TME_STP103X_TLB_X(e, 1, 63)
#define TME_STP103X_TLB_DATA_SIZE_MASK(e)	_TME_STP103X_TLB_X(e, 0x3, 61)
#define  TME_STP103X_TLB_DATA_SIZE_8KB(e)	 _TME_STP103X_TLB_X(e, 0x0, 61)
#define  TME_STP103X_TLB_DATA_SIZE_64KB(e)	 _TME_STP103X_TLB_X(e, 0x1, 61)
#define  TME_STP103X_TLB_DATA_SIZE_512KB(e)	 _TME_STP103X_TLB_X(e, 0x2, 61)
#define  TME_STP103X_TLB_DATA_SIZE_4MB(e)	 _TME_STP103X_TLB_X(e, 0x3, 61)
#define TME_STP103X_TLB_DATA_NFO(e)		_TME_STP103X_TLB_X(e, 1, 60)
#define TME_STP103X_TLB_DATA_IE(e)		_TME_STP103X_TLB_X(e, 1, 59)
#define TME_STP103X_TLB_DATA_SOFT2(e)		_TME_STP103X_TLB_X(e, 0x1ff, 50)
#define TME_STP103X_TLB_DATA_SIZE_CAM_MASK(e)	_TME_STP103X_TLB_X(e, 0x7, 45)
#define  TME_STP103X_TLB_DATA_SIZE_CAM_8KB(e)	 _TME_STP103X_TLB_X(e, 0x0, 45)
#define  TME_STP103X_TLB_DATA_SIZE_CAM_64KB(e)	 _TME_STP103X_TLB_X(e, 0x1, 45)
#define  TME_STP103X_TLB_DATA_SIZE_CAM_512KB(e)	 _TME_STP103X_TLB_X(e, 0x3, 45)
#define  TME_STP103X_TLB_DATA_SIZE_CAM_4MB(e)	 _TME_STP103X_TLB_X(e, 0x7, 45)
#define TME_STP103X_TLB_DATA_SIZE_RAM_MASK(e)	_TME_STP103X_TLB_X(e, 0x7, 42)
#define  TME_STP103X_TLB_DATA_SIZE_RAM_8KB(e)	 _TME_STP103X_TLB_X(e, 0x0, 42)
#define  TME_STP103X_TLB_DATA_SIZE_RAM_64KB(e)	 _TME_STP103X_TLB_X(e, 0x1, 42)
#define  TME_STP103X_TLB_DATA_SIZE_RAM_512KB(e)	 _TME_STP103X_TLB_X(e, 0x3, 42)
#define  TME_STP103X_TLB_DATA_SIZE_RAM_4MB(e)	 _TME_STP103X_TLB_X(e, 0x7, 42)
#define TME_STP103X_TLB_DATA_DIAG_USED(e)	_TME_STP103X_TLB_X(e, 1, 41)
#define TME_STP103X_TLB_DATA_PA			((((tme_uint64_t) 1) << 41) - (1 << 13))
#define TME_STP103X_TLB_DATA_SOFT(e)		_TME_STP103X_TLB_X(e, 0x3f, 7)
#define TME_STP103X_TLB_DATA_L(e)		_TME_STP103X_TLB_X(e, 1, 6)
#define TME_STP103X_TLB_DATA_CP(e)		_TME_STP103X_TLB_X(e, 1, 5)
#define TME_STP103X_TLB_DATA_CV(e)		_TME_STP103X_TLB_X(e, 1, 4)
#define TME_STP103X_TLB_DATA_E(e)		_TME_STP103X_TLB_X(e, 1, 3)
#define TME_STP103X_TLB_DATA_P(e)		_TME_STP103X_TLB_X(e, 1, 2)
#define TME_STP103X_TLB_DATA_W(e)		_TME_STP103X_TLB_X(e, 1, 1)
#define TME_STP103X_TLB_DATA_G(e)		_TME_STP103X_TLB_X(e, 1, 0)

/* the DMMU and IMMU SFSR: */
#define TME_STP103X_SFSR_ASI			(0xff << 16)
#define TME_STP103X_SFSR_FT_PRIVILEGE		(0x01 << 7)
#define TME_STP103X_SFSR_FT_SIDE_EFFECTS	(0x02 << 7)
#define TME_STP103X_SFSR_FT_UNCACHEABLE		(0x04 << 7)
#define TME_STP103X_SFSR_FT_ILLEGAL		(0x08 << 7)
#define TME_STP103X_SFSR_FT_NO_FAULT_FAULT	(0x10 << 7)
#define TME_STP103X_SFSR_FT_VA_RANGE		(0x20 << 7)
#define TME_STP103X_SFSR_FT_VA_RANGE_NNPC	(0x40 << 7)
#define TME_STP103X_SFSR_E			TME_BIT(6)
#define TME_STP103X_SFSR_CT_PRIMARY		(0x0 << 4)
#define TME_STP103X_SFSR_CT_SECONDARY		(0x1 << 4)
#define TME_STP103X_SFSR_CT_NUCLEUS		(0x2 << 4)
#define TME_STP103X_SFSR_CT_RESERVED		(0x3 << 4)
#define TME_STP103X_SFSR_PR			TME_BIT(3)
#define TME_STP103X_SFSR_W			TME_BIT(2)
#define TME_STP103X_SFSR_OW			TME_BIT(1)
#define TME_STP103X_SFSR_FV			TME_BIT(0)

/* the AFSR: */
#define TME_STP103X_AFSR_ME			(((tme_uint64_t) 1) << 32)
#define TME_STP103X_AFSR_PRIV			TME_BIT(31)
#define TME_STP103X_AFSR_TO			TME_BIT(27)

/* a TSB register: */
#define TME_STP103X_TSB_SIZE			(0x7)
#define TME_STP103X_TSB_SPLIT			TME_BIT(12)

/* specific traps: */
#define TME_STP103X_TRAP_MG			_TME_SPARC_TRAP_IMPDEP(0)
#define TME_STP103X_TRAP_IG			_TME_SPARC_TRAP_IMPDEP(1)
#define TME_STP103X_TRAP_interrupt_vector \
  (TME_STP103X_TRAP_IG | _TME_SPARC_TRAP(16, 0x060))
#define TME_STP103X_TRAP_fast_instruction_access_MMU_miss \
  (TME_STP103X_TRAP_MG | _TME_SPARC_TRAP(2, 0x064))
#define TME_STP103X_TRAP_fast_data_access_MMU_miss \
  (TME_STP103X_TRAP_MG | _TME_SPARC_TRAP(12, 0x068))
#define TME_STP103X_TRAP_fast_data_access_protection \
  (TME_STP103X_TRAP_MG | _TME_SPARC_TRAP(12, 0x06c))

/* specific ASIs and flags: */
#define TME_STP103X_ASI_LSU_CONTROL_REG		(0x45)
#define TME_STP103X_ASI_DCACHE_DATA		(0x46)
#define TME_STP103X_ASI_DCACHE_TAG		(0x47)
#define TME_STP103X_ASI_INTR_DISPATCH_STATUS	(0x48)
#define TME_STP103X_ASI_INTR_RECEIVE		(0x49)
#define TME_STP103X_ASI_UPA_CONFIG_REG		(0x4a)
#define TME_STP103X_ASI_ESTATE_ERROR_EN_REG	(0x4b)
#define TME_STP103X_ASI_AFSR			(0x4c)
#define TME_STP103X_ASI_AFAR			(0x4d)
#define TME_STP103X_ASI_ECACHE_TAG_DATA		(0x4e)
#define TME_STP103X_ASI_IMMU			(0x50)
#define TME_STP103X_ASI_DMMU			(0x58)
#define TME_STP103X_ASI_FLAG_TSB_8KB_PTR	(0x1)
#define TME_STP103X_ASI_FLAG_TSB_64KB_PTR	(0x2)
#define TME_STP103X_ASI_BLK_COMMIT		(0xe0)

/* the size of the IMMU and DMMU TLBs: */
#define TME_STP103X_TLB_SIZE			(64)

/* the size of the E-Cache: */
#define TME_STP103X_ECACHE_SIZE			(512 * 1024)

/* the block size of the I-Cache: */
#define TME_STP103X_ICACHE_BLOCK_SIZE		(32)

/* the TICK_compare register: */
#define TME_STP103X_TCR_INT_DIS			(((tme_uint64_t) 1) << 63)
#define TME_STP103X_TCR_TICK_CMPR		(TME_STP103X_TCR_INT_DIS - 1)

/* the SIR: */
#define TME_STP103X_SIR_SOFTINT(x)		(1 << (x))
#define TME_STP103X_SIR_TICK_INT		(1 << 0)

/* ASI_INTR_RECEIVE: */
#define TME_STP103X_INTR_RECEIVE_BUSY		(1 << 5)

/* the block load and store sizes: */
#define TME_STP103X_BLOCK_SIZE			(64)
#define TME_STP103X_BLOCK_FPREGS_DOUBLE		(TME_STP103X_BLOCK_SIZE / sizeof(tme_uint64_t))

/* other constants: */
#define TME_STP103X_MAXTL			(5)

/* the UPA configuration register: */
/* NB: this is a partial list: */
#define TME_STP1030_UPA_CONFIG_PCON		((2 << 29) - (1 << 22))
#define TME_STP1031_UPA_CONFIG_PCON		((((tme_uint64_t) 2) << 32) - (1 << 22))
#define TME_STP1031_UPA_CONFIG_ELIM		((((tme_uint64_t) 2) << 35) - (((tme_uint64_t) 1) << 33))

/* the UPA queue depths and capabilities: */
#define TME_STP103X_UPA_PINT_RDQ		(1)
#define TME_STP103X_UPA_PREQ_DQ			(0)
#define TME_STP103X_UPA_PREQ_RQ			(1)
#define TME_STP103X_UPA_UPACAP			\
  (TME_UPA_UPACAP_HANDLERSLAVE			\
   | TME_UPA_UPACAP_INTERRUPTMASTER		\
   | !TME_UPA_UPACAP_SLAVE_INT_L		\
   | TME_UPA_UPACAP_CACHEMASTER			\
   | TME_UPA_UPACAP_MASTER)

/* fixed characteristics of the stp103x: */
#undef  TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic)	(9)
#undef  TME_SPARC_NWINDOWS
#define TME_SPARC_NWINDOWS(ic)	(8)
#undef  TME_SPARC_MEMORY_FLAGS
#define TME_SPARC_MEMORY_FLAGS(ic)		\
  (TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS		\
   + TME_SPARC_MEMORY_FLAG_HAS_INVERT_ENDIAN	\
   + !TME_SPARC_MEMORY_FLAG_HAS_LDDF_STDF_32)

/* this recovers the stp103x state from the generic sparc state: */
#define TME_STP103X(ic) ((struct tme_stp103x *) (TRUE ? (ic) : (struct tme_sparc *) 0))

/* this evaluates to nonzero if an ASI mask from an ASI_DMMU* or
   ASI_IMMU* ASI is from an ASI_DMMU* ASI: */
#if (TME_STP103X_ASI_DMMU <= TME_STP103X_ASI_IMMU) || ((TME_STP103X_ASI_DMMU ^ TME_STP103X_ASI_IMMU) & ((TME_STP103X_ASI_DMMU ^ TME_STP103X_ASI_IMMU) - 1)) != 0
#error "TME_STP103X_ASI_DMMU or TME_STP103X_ASI_IMMU changed"
#endif
#define TME_STP103X_ASI_MMU_MASK_IS_DMMU(asi_mask)	\
  ((asi_mask)						\
   & TME_SPARC_ASI_MASK_RAW(TME_STP103X_ASI_DMMU	\
			    ^ TME_STP103X_ASI_IMMU))

/* specific load/store information: */
#define TME_STP103X_LSINFO_ASSERT_NO_FAULTS	_TME_SPARC_LSINFO_X(0)

/* specific load/store faults: */
#define TME_STP103X_LS_FAULT_MMU_MISS		_TME_SPARC64_LS_FAULT_X(0)
#define TME_STP103X_LS_FAULT_PRIVILEGE		_TME_SPARC64_LS_FAULT_X(1)
#define TME_STP103X_LS_FAULT_PROTECTION		_TME_SPARC64_LS_FAULT_X(2)
#define TME_STP103X_LS_FAULT_ILLEGAL		_TME_SPARC64_LS_FAULT_X(3)

/* update flags: */
#define TME_STP103X_UPDATE_NONE			(0)
#define TME_STP103X_UPDATE_DMMU			TME_BIT(0)
#define TME_STP103X_UPDATE_IMMU			(0)
#define TME_STP103X_UPDATE_MMU_TAG_ACCESS	TME_BIT(1)
#define TME_STP103X_UPDATE_MMU_SFSR		TME_BIT(2)
#define TME_STP103X_UPDATE_DMMU_SFAR		TME_BIT(3)

/* the first IMMU and DMMU TLB entry parts: */
#define TME_STP103X_TLB_PART_0_DMMU		(TME_STP103X_TLB_SIZE * 2 * 0)
#define TME_STP103X_TLB_PART_0_IMMU		(TME_STP103X_TLB_SIZE * 2 * 1)

/* this returns nonzero if this is an stp1030: */
#define TME_STP103X_IS_1030(ic)			(TME_STP103X(ic)->tme_stp103x_is_1030)

/* types: */

/* the stp103x DMMU and IMMU common state: */
struct tme_stp103x_mmu {

  /* the DMMU or IMMU synchronous fault status register: */
  tme_uint64_t tme_stp103x_mmu_sfsr;

  /* the DMMU or IMMU tag access register: */
  tme_uint64_t tme_stp103x_mmu_tag_access;

  /* the DMMU or IMMU translation storage buffer register: */
  tme_uint64_t tme_stp103x_mmu_tsb;
};

/* the stp103x state: */
struct tme_stp103x {

  /* the generic sparc state: */
  struct tme_sparc tme_stp103x_sparc;

  /* the tick comparison register: */
  tme_uint64_t tme_stp103x_tcr;

  /* the softint register: */
  tme_uint16_t tme_stp103x_sir;
  tme_memory_atomic_flag_t tme_stp103x_sir_tick_int;

  /* the dispatch control register: */
  tme_uint8_t tme_stp103x_dcr;

  /* this is nonzero if this is an stp1030: */
  tme_uint8_t tme_stp103x_is_1030;

  /* the performance control register: */
  tme_uint16_t tme_stp103x_pcr;

  /* the performance instrumentation counters: */
  union tme_value64 tme_stp103x_pic;

  /* the UPA configuration register: */
  tme_uint64_t tme_stp103x_upa_config;

  /* the load/store unit control register: */
  tme_uint64_t tme_stp103x_lsu;

  /* the estate error enable register: */
  tme_uint32_t tme_stp103x_estate_error_enable;

  /* the ecache tag data register: */
  tme_uint32_t tme_stp103x_ecache_tag_data;

  /* the ecache probe line: */
  tme_uint64_t tme_stp103x_ecache_data_probe;

  /* the transmit and receive interrupt vector data: */
  tme_uint64_t tme_stp103x_udb_intr_transmit[3];
  tme_shared tme_uint64_t tme_stp103x_udb_intr_receive[3];
  tme_shared tme_uint8_t tme_stp103x_intr_receive_mid;
  tme_memory_atomic_flag_t tme_stp103x_intr_receive_busy;

  /* the tick compare condition and time: */
  tme_cond_t tme_stp103x_tick_compare_cond;
  struct timeval tme_stp103x_tick_compare_time;

  /* the UDB low and high control registers: */
  tme_uint16_t tme_stp103x_udb_control[2];

  /* the asynchronous fault address and status registers: */
  tme_uint64_t tme_stp103x_afar;
  tme_uint64_t tme_stp103x_afsr;

  /* the DMMU and IMMU common state: */
  struct tme_stp103x_mmu tme_stp103x_immu;
  struct tme_stp103x_mmu tme_stp103x_dmmu;

  /* the DMMU synchronous fault address register: */
  tme_uint64_t tme_stp103x_dmmu_sfar;

  /* this is nonzero if the last fast_data_access_protection trap was
     for a 64KB page: */
  tme_uint8_t tme_stp103x_dmmu_direct_64KB;

  /* the DMMU and IMMU TLB: */
  /* NB: this single array is half IMMU TLB, half DMMU TLB, and each
     entry has two parts: tag and data: */
  union {
    tme_uint64_t _tme_stp103x_tlb_u_64s[2 * 2 * TME_STP103X_TLB_SIZE];
#define tme_stp103x_tlb_64s(x) _tme_stp103x_tlb_u._tme_stp103x_tlb_u_64s[x]
    tme_uint32_t _tme_stp103x_tlb_u_32s[2 * 4 * TME_STP103X_TLB_SIZE];
#if (TME_ENDIAN_NATIVE != TME_ENDIAN_BIG) && (TME_ENDIAN_NATIVE != TME_ENDIAN_LITTLE)
#error "only big- and little-endian hosts are supported"
#endif
#define tme_stp103x_tlb_32s(x, y) _tme_stp103x_tlb_u._tme_stp103x_tlb_u_32s[((x) * 2) + ((y) ^ (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG))]
  } _tme_stp103x_tlb_u;
};

/* globals: */

/* the cacheable access bus router: */
static const tme_bus_lane_t _tme_stp103x_bus_router_cacheable[1 << TME_BUS128_LOG2][1 << TME_BUS128_LOG2] = {
  /* a byte access: */
  {
    TME_BUS_LANE_ROUTE(0),
    TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF
  },
  /* a word access: */
  {
    TME_BUS_LANE_ROUTE(1),
    TME_BUS_LANE_ROUTE(0),
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF
  },
  /* a doubleword access: */
  {
    TME_BUS_LANE_ROUTE(3),
    TME_BUS_LANE_ROUTE(2),
    TME_BUS_LANE_ROUTE(1),
    TME_BUS_LANE_ROUTE(0),
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF
  },
  /* an extended word access: */
  {
    TME_BUS_LANE_ROUTE(7),
    TME_BUS_LANE_ROUTE(6),
    TME_BUS_LANE_ROUTE(5),
    TME_BUS_LANE_ROUTE(4),
    TME_BUS_LANE_ROUTE(3),
    TME_BUS_LANE_ROUTE(2),
    TME_BUS_LANE_ROUTE(1),
    TME_BUS_LANE_ROUTE(0),
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF,
    TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF, TME_BUS_LANE_UNDEF
  },
  /* a quadword access: */
  {
    TME_BUS_LANE_ROUTE(15),
    TME_BUS_LANE_ROUTE(14),
    TME_BUS_LANE_ROUTE(13),
    TME_BUS_LANE_ROUTE(12),
    TME_BUS_LANE_ROUTE(11),
    TME_BUS_LANE_ROUTE(10),
    TME_BUS_LANE_ROUTE(9),
    TME_BUS_LANE_ROUTE(8),
    TME_BUS_LANE_ROUTE(7),
    TME_BUS_LANE_ROUTE(6),
    TME_BUS_LANE_ROUTE(5),
    TME_BUS_LANE_ROUTE(4),
    TME_BUS_LANE_ROUTE(3),
    TME_BUS_LANE_ROUTE(2),
    TME_BUS_LANE_ROUTE(1),
    TME_BUS_LANE_ROUTE(0),
  }
};

/* this maps a virtual address: */
static void _tme_stp103x_ls_address_map _TME_P((struct tme_sparc *, struct tme_sparc_ls *));

/* this makes a never struct timeval: */
static inline void
tme_misc_timeval_never(struct timeval *tv)
{
  tv->tv_sec = 0;
  tv->tv_sec--;
  if (tv->tv_sec < 1) {
    tv->tv_sec = 1;
    tv->tv_sec <<= ((8 * sizeof(tv->tv_sec)) - 2);
    tv->tv_sec += (tv->tv_sec - 1);
  }
  tv->tv_usec = 999999;
}

/* this does an interrupt check: */
/* NB: this may do a preinstruction trap: */
static void
_tme_stp103x_interrupt_check(struct tme_sparc *ic,
			     int flags)
{
  tme_uint32_t sir;
  tme_uint32_t ipl;
  tme_uint32_t trap;

  /* if we're replaying instructions, return now: */
  if (tme_sparc_recode_verify_replay_last_pc(ic) != 0) {
    return;
  }

  /* if PSTATE.IE is clear, return now: */
  if ((ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_IE) == 0) {
    return;
  }

  /* if the incoming interrupt vector data is busy: */
  if (tme_memory_atomic_read_flag(&TME_STP103X(ic)->tme_stp103x_intr_receive_busy)) {

    /* start interrupt vector trap processing: */
    trap = TME_STP103X_TRAP_interrupt_vector;
  }

  /* otherwise, the incoming interrupt vector data is not busy: */
  else {

    /* if no SOFTINT bits greater than PIL are set, return now: */
    sir = TME_STP103X(ic)->tme_stp103x_sir;
    if (tme_memory_atomic_read_flag(&TME_STP103X(ic)->tme_stp103x_sir_tick_int)) {
      sir |= TME_STP103X_SIR_SOFTINT(14);
    }
    ipl = ic->tme_sparc64_ireg_pil + 1;
    sir >>= ipl;
    if (sir == 0) {
      return;
    }

    /* get the greatest SOFTINT bit greater than PIL that is set: */
    for (; sir != 1; sir >>= 1, ipl++);

    /* start interrupt trap processing: */
    trap = TME_SPARC64_TRAP_interrupt_level(ipl);
  }

  /* start trap processing: */
  if (flags & TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED) {
    tme_mutex_unlock(&ic->tme_sparc_external_mutex);
  }
  if (flags & TME_SPARC_EXTERNAL_CHECK_PCS_UPDATED) {
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT) = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT);
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT) = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC);
#ifdef _TME_SPARC_RECODE_VERIFY
    ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC) = TME_SPARC_RECODE_VERIFY_PC_NONE;
#endif /* _TME_SPARC_RECODE_VERIFY */
  }  
  tme_sparc64_trap_preinstruction(ic, trap);
}

/* this updates the SIR: */
/* NB: this may do a preinstruction trap: */
static void
_tme_stp103x_update_sir(struct tme_sparc *ic,
			tme_uint32_t sir_andn,
			tme_uint32_t sir_or)
{

  /* if we're clearing SIR.TICK_INT: */
  if (sir_andn & TME_STP103X_SIR_TICK_INT) {

    /* do an interrupt check: */
    _tme_stp103x_interrupt_check(ic, TME_SPARC_EXTERNAL_CHECK_PCS_UPDATED);
    
    /* clear the tick interrupt atomic flag: */
    tme_memory_atomic_write_flag(&TME_STP103X(ic)->tme_stp103x_sir_tick_int, FALSE);
  }

  /* if we're setting SIR.TICK_INT: */
  if (sir_or & TME_STP103X_SIR_TICK_INT) {

    /* set the tick interrupt atomic flag: */
    tme_memory_atomic_write_flag(&TME_STP103X(ic)->tme_stp103x_sir_tick_int, TRUE);

    /* we won't set SIR.TICK_INT in the normal SIR image: */
    sir_or ^= TME_STP103X_SIR_TICK_INT;
  }

  /* update all other bits in SIR: */
  TME_STP103X(ic)->tme_stp103x_sir
    = ((TME_STP103X(ic)->tme_stp103x_sir
	& ~sir_andn)
       | sir_or);

  /* do an interrupt check: */
  _tme_stp103x_interrupt_check(ic, TME_SPARC_EXTERNAL_CHECK_NULL);
}

/* this updates the LSU control register: */
static void
_tme_stp103x_update_lsu(struct tme_sparc *ic, tme_uint64_t lsu_new)
{
  tme_uint64_t lsu_xor;
  struct tme_sparc_tlb *tlb;

  /* get a change mask: */
  lsu_xor = lsu_new ^ TME_STP103X(ic)->tme_stp103x_lsu;

  /* if LSU.IC or LSU.DC are changing: */
  if (lsu_xor
      & (TME_STP103X_LSU_IC
	 | TME_STP103X_LSU_DC)) {

    /* nothing to do, since we don't emulate the caches: */
  }

  /* if LSU.IM or LSU.DM are changing: */
  if (lsu_xor
      & (TME_STP103X_LSU_IM
	 | TME_STP103X_LSU_DM)) {

    /* invalidate all of the sparc TLB entries: */
    tlb = &ic->tme_sparc_tlbs[0];
    do {
      tme_token_invalidate_nosync(tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
    } while (++tlb <= &ic->tme_sparc_tlbs[TME_ARRAY_ELS(ic->tme_sparc_tlbs) - 1]);
  }

  /* if LSU.FM is changing: */
  if (lsu_xor & TME_STP103X_LSU_FM) {
    /* XXX FIXME WRITEME: */
    abort();
  }

  /* if any of LSU.VR, LSU.VW, LSU.PR, LSU.PW are changing: */
  if (lsu_xor
      & (TME_STP103X_LSU_VR
	 | TME_STP103X_LSU_VW
	 | TME_STP103X_LSU_PR
	 | TME_STP103X_LSU_PW)) {
    /* XXX FIXME WRITEME: */
    abort();
  }

  /* update the register: */
  TME_STP103X(ic)->tme_stp103x_lsu = lsu_new;
}

/* this updates the UPA configuration register: */
static void
_tme_stp103x_update_upa_config(struct tme_sparc *ic, tme_uint64_t upa_config)
{

  /* if this is an STP1030: */
  if (TME_STP103X_IS_1030(ic)) {

    /* only the PCON field is writable: */
    upa_config &= TME_STP1030_UPA_CONFIG_PCON;
  }

  /* otherwise, this is an STP1031: */
  else {

    /* only the PCON and ELIM fields are writable: */
    upa_config
      &= (TME_STP1031_UPA_CONFIG_PCON
	  | TME_STP1031_UPA_CONFIG_ELIM);

    /* all read-only STP1031-specific fields are hardwired to zero: */
  }

  /* add the MID: */
  upa_config += (ic->_tme_upa_bus_connection->tme_upa_bus_connection_mid << 17);

  /* add the PCAP fields: */
  upa_config
    += ((TME_STP103X_UPA_PINT_RDQ << 15)
	+ (TME_STP103X_UPA_PREQ_DQ << 9)
	+ (TME_STP103X_UPA_PREQ_RQ << 5)
	+ (TME_STP103X_UPA_UPACAP << 0));

  /* set the UPA configuration register: */
  TME_STP103X(ic)->tme_stp103x_upa_config = upa_config;
}

/* this handles a PSTATE update: */
/* NB: this may do a preinstruction trap: */
static void
_tme_stp103x_update_pstate(struct tme_sparc *ic,
			   tme_uint32_t pstate,
			   tme_uint32_t trap)
{
  tme_uint64_t lsu_new;
  tme_uint32_t pstate_xor;
  tme_uint64_t address_mask;

  /* if we are in RED_state: */
  if (pstate & TME_SPARC64_PSTATE_RED) {

    /* traps that enter RED_state, and traps in RED_state, always
       clear the LSU_Control_Register.  a write of one to PSTATE.RED
       only clears LSU.IM: */
    lsu_new
      = (trap != TME_SPARC_TRAP_none
	 ? 0
	 : (TME_STP103X(ic)->tme_stp103x_lsu
	    & ~ (tme_uint64_t) TME_STP103X_LSU_IM));
    _tme_stp103x_update_lsu(ic, lsu_new);

    /* if this is a power-on reset: */
    if (trap == TME_SPARC64_TRAP_power_on_reset) {
    
      /* a POR clears the E-Cache (E-State) error enable register: */
      TME_STP103X(ic)->tme_stp103x_estate_error_enable = 0;

      /* a POR zeroes the writable fields in the UPA configuration
	 register: */
      _tme_stp103x_update_upa_config(ic, 0);
    }
  }

  /* if this is a trap that uses the MMU globals: */
  if (trap & TME_STP103X_TRAP_MG) {
    assert (trap != TME_SPARC_TRAP_none);
    pstate
      = ((pstate
	  & ~(TME_SPARC64_PSTATE_AG
	      + TME_STP103X_PSTATE_MG
	      + TME_STP103X_PSTATE_IG))
	 + TME_STP103X_PSTATE_MG);
  }

  /* otherwise, if this is a trap that uses the interrupt globals: */
  else if (trap & TME_STP103X_TRAP_IG) {
    assert (trap != TME_SPARC_TRAP_none);
    pstate
      = ((pstate
	  & ~(TME_SPARC64_PSTATE_AG
	      + TME_STP103X_PSTATE_MG
	      + TME_STP103X_PSTATE_IG))
	 + TME_STP103X_PSTATE_IG);
  }

  /* otherwise, if this is another trap: */
  else if (trap != TME_SPARC_TRAP_none) {
    pstate
      &= ~(TME_STP103X_PSTATE_IG
	   + TME_STP103X_PSTATE_MG);
    assert (pstate & TME_SPARC64_PSTATE_AG);
  }

  /* the global register selection can't be reserved: */
  assert ((((pstate & TME_SPARC64_PSTATE_AG) != 0)
	   + ((pstate & TME_STP103X_PSTATE_MG) != 0)
	   + ((pstate & TME_STP103X_PSTATE_IG) != 0)) < 2);

  /* update the global register offset: */
  ic->tme_sparc_reg8_offset[0]
    = ((pstate & TME_SPARC64_PSTATE_AG)
#if (TME_SPARC64_IREG_AG_G0 % 8)
#error "TME_SPARC64_IREG_AG_G0 must be a multiple of eight"
#endif
       ? (TME_SPARC64_IREG_AG_G0 / 8)
       : (pstate & TME_STP103X_PSTATE_MG)
#if (TME_SPARC64_IREG_MG_G0 % 8)
#error "TME_SPARC64_IREG_MG_G0 must be a multiple of eight"
#endif
       ? (TME_SPARC64_IREG_MG_G0 / 8)
       : (pstate & TME_STP103X_PSTATE_IG)
#if (TME_SPARC64_IREG_IG_G0 % 8)
#error "TME_SPARC64_IREG_IG_G0 must be a multiple of eight"
#endif
       ? (TME_SPARC64_IREG_IG_G0 / 8)
#if (TME_SPARC_IREG_G0 % 8)
#error "TME_SPARC_IREG_G0 must be a multiple of eight"
#endif
       : (TME_SPARC_IREG_G0 / 8));
  _TME_SPARC_RECODE_CWP_UPDATE(ic, tme_uint64_t);

  /* make sure that %g0 is zero in the normal global register set and
     the current global register set.  recode instructions thunks
     always refer to %g0 in the normal global register set: */
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_G0) = 0;
  ic->tme_sparc_ireg_uint64(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G0) = 0;

  /* get the changing bits in PSTATE: */
  pstate_xor = ic->tme_sparc64_ireg_pstate ^ pstate;

#if TME_HAVE_RECODE && TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
  
  /* if PSTATE.AM and/or PSTATE.CLE are changing: */
  if (__tme_predict_false(pstate_xor
			  & (TME_SPARC64_PSTATE_AM
			     | TME_SPARC64_PSTATE_CLE
			     ))) {

    /* clear the return address stack, since chaining doesn't check that
       PSTATE matches instructions thunks: */
    tme_recode_chain_ras_clear(ic->tme_sparc_recode_ic,
			       &ic->tme_sparc_ic);
  }

#endif /* TME_HAVE_RECODE && TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */

  /* set PSTATE: */
  ic->tme_sparc64_ireg_pstate = pstate;

  /* update the address mask: */
  address_mask
    = ((0 - (tme_uint64_t) ((~pstate / TME_SPARC64_PSTATE_AM) & 1))
       | (tme_uint32_t) (0 - (tme_uint32_t) 1));
  ic->tme_sparc_address_mask = address_mask;

  /* mask the PCs: */
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT) &= address_mask;
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT) &= address_mask;

  /* if interrupts are enabled, we can't be trapping, because
     _tme_stp103x_interrupt_check() might trap with an interrupt,
     discarding the initial trap: */
  assert ((pstate & TME_SPARC64_PSTATE_IE) == 0
	  || trap == TME_SPARC_TRAP_none);

  /* do an interrupt check: */
  _tme_stp103x_interrupt_check(ic, TME_SPARC_EXTERNAL_CHECK_NULL);
}

/* this updates the AFSR: */
static void
_tme_stp103x_update_afsr(struct tme_sparc *ic, tme_uint64_t afsr_reset)
{

  /* one bits in positions 20..32 clear those bits in the AFSR: */
  TME_STP103X(ic)->tme_stp103x_afsr
    &= ~(afsr_reset
	 & ((((tme_uint64_t) 1) << (32 + 1)) - (1 << 20)));
}

/* this updates the interrupt vector receive: */
static void
_tme_stp103x_update_intr_receive(struct tme_sparc *ic, tme_uint64_t intr_receive)
{
  tme_memory_atomic_write_flag(&TME_STP103X(ic)->tme_stp103x_intr_receive_busy,
			       (intr_receive & TME_STP103X_INTR_RECEIVE_BUSY) != 0);
}

/* the stp103x rdasr: */
static
TME_SPARC_FORMAT3(_tme_stp103x_rdasr, tme_uint64_t)
{
  unsigned int reg_rs1;
  tme_uint64_t value;

  /* if this is an implementation-specific ASR: */
  if (TME_SPARC_INSN & (0x10 << 14)) {

    /* get rs1: */
    reg_rs1 = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RS1);

    /* if this is an undefined implementation-specific ASR: */
    if (__tme_predict_false(reg_rs1 >= 0x18)) {
      TME_SPARC_INSN_ILL(ic);
    }

    /* if this is a read of GSR: */
    if (reg_rs1 == 0x13) {
      if (__tme_predict_false((ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_PEF) == 0
			      || (ic->tme_sparc64_ireg_fprs & TME_SPARC64_FPRS_FEF) == 0)) {
	tme_sparc64_trap(ic, TME_SPARC64_TRAP_fp_disabled);
      }
      value = ic->tme_sparc_vis_gsr;
    }

    /* otherwise, this is not a read of GSR: */
    else {

      /* if this is a nonprivileged read: */
      if (__tme_predict_false(!TME_SPARC_PRIV(ic))) {

	/* if this is not a read of PIC, or if PCR.PRIV is set: */
	if (reg_rs1 != 0x11
	    || (TME_STP103X(ic)->tme_stp103x_pcr & TME_BIT(0))) {
	  tme_sparc64_trap(ic, TME_SPARC64_TRAP_privileged_opcode);
	}
      }

      /* dispatch on rs1: */
      switch (reg_rs1) {
      default: TME_SPARC_INSN_ILL(ic);
      case 0x10: value = TME_STP103X(ic)->tme_stp103x_pcr; break;
      case 0x11: value = TME_STP103X(ic)->tme_stp103x_pic.tme_value64_uint; break;
      case 0x12: value = TME_STP103X(ic)->tme_stp103x_dcr; break;
      case 0x16: 
	value = TME_STP103X(ic)->tme_stp103x_sir;
	if (tme_memory_atomic_read_flag(&TME_STP103X(ic)->tme_stp103x_sir_tick_int)) {
	  value += TME_STP103X_SIR_TICK_INT;
	}
	break;
      case 0x17: value = TME_STP103X(ic)->tme_stp103x_tcr; break;
      }
    }
    TME_SPARC_FORMAT3_RD = value;
    TME_SPARC_INSN_OK;
  }
  tme_sparc64_rdasr(ic, _rs1, _rs2, _rd);
}

/* the stp103x rdpr: */
static
TME_SPARC_FORMAT3(_tme_stp103x_rdpr, tme_uint64_t)
{
  unsigned long offset_in_insns;
  tme_uint32_t delay_factor;
  tme_uint32_t insn;
  const struct tme_sparc_tlb *itlb_current;
  
  /* the PROM (at least, SUNW,501-3082-update7.bin) calculates a delay
     factor by waiting for a timer interrupt while running these two
     instructions:

     f0055720:	10 80 00 00 	b  0xf0055720
     f0055724:	a4 04 a0 01 	inc  %l2

     the PROM then does a delay by running these two instructions:

     f00557e0:	14 68 00 00 	bg  %xcc, 0xf00557e0
     f00557e4:	a2 a4 60 01 	deccc  %l1

     unfortunately, in the stp103x emulation, a deccc is more
     expensive than an inc, because it must update the condition
     codes.  the difference is significant enough to cause what
     should be a brief delay between these two PROM messages:

     Probing Memory Bank #3   0 +   0 :   0 Megabytes
     Probing /sbus@1f,0 at 0,0  Nothing there

     to be very, very long.  the cost difference appears to be even
     worse when recode is on, and the extreme delay gives the user the
     impression that the emulator is stuck.

     there's a chance that there's a cost difference between inc and
     deccc even on a real stp103x; "1000 ms" on the PROM on a real
     Ultra-1 delays noticeably longer than one second.

     this function is a gross hack that attempts to reduce the delay.
     it modifies the original loop that calculates the delay factor to
     use (for the "best" factor) an inccc instruction instead of a
     inc, to make it more symmetric with the delay loop.

     it detects the original loop by this instruction that precedes it:

     f0055708:	a7 51 00 00 	rdpr  %tick, %l3
  */

  /* if this is a "rdpr %tick, %l3" instruction: */
  if (__tme_predict_false(TME_SPARC_INSN == 0xa7510000)) {

    /* get the offset, in units of instructions, to the next PC that
       is I-Cache block aligned: */
    offset_in_insns = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC);
    offset_in_insns = ~offset_in_insns;
    offset_in_insns %= TME_STP103X_ICACHE_BLOCK_SIZE;
    offset_in_insns = (offset_in_insns + 1) / sizeof(TME_SPARC_INSN);

    /* if the CPU is in privileged mode, and the first two
       instructions at that PC are "b ." and "inc %l2": */
    if (TME_SPARC_PRIV(ic)
	&& tme_sparc_fetch_nearby(ic, offset_in_insns + 0) == 0x10800000
	&& tme_sparc_fetch_nearby(ic, offset_in_insns + 1) == 0xa404a001) {

      /* if the PROM delay factor is to be corrected: */
      delay_factor = ic->tme_sparc_prom_delay_factor;
      if (delay_factor != TME_SPARC_PROM_DELAY_FACTOR_UNCORRECTED) {

	/* if the PROM delay factor correction is "min": */
	if (delay_factor == TME_SPARC_PROM_DELAY_FACTOR_MIN) {

	  /* the delay factor is calculated by running the loop for
	     64ms and then dividing the count by 64, so a raw count of
	     64 will get the minimum delay: */
	  delay_factor = 64;
	}

	/* if the PROM delay factor correction is "best": */
	if (delay_factor == TME_SPARC_PROM_DELAY_FACTOR_BEST) {

	  /* modify the "inc %l2" to be "inccc %l2": */
	  insn = 0xa484a001;
	}

	/* otherwise, the PROM delay factor is given directly: */
	else {

	  /* modify the "inc %l2" to be "or %g0, simm13, %l2": */
	  insn = 0xa4102000 + TME_MIN(delay_factor, 0xfff);
	}

	/* get the current instruction TLB entry: */
	/* NB: we don't repeat the assert()s or the address-covering
	   checks of tme_sparc_fetch_nearby(): */
	itlb_current = tme_sparc_itlb_current(ic);

	/* modify the "inc %l2": */
	/* NB: we break const here: */
	((tme_shared tme_uint32_t *)
	 (unsigned long) 
	 (itlb_current->tme_sparc_tlb_emulator_off_read
	  + ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)))
	  [offset_in_insns + 1] = tme_htobe_u32(insn);

	/* NB: we don't invalidate the memory page that we just modified
	   (in the tme_sparc_recode_cacheable_valids sense), because we
	   assume that the loop that calculates the delay factor hasn't
	   been recoded yet, and we assume that no other CPUs or other
	   bus masters care about the modification: */
      }
    }
  }

  /* do the normal rdpr: */
  tme_sparc64_rdpr(ic, _rs1, _rs2, _rd);
}

/* the stp103x wrasr: */
static
TME_SPARC_FORMAT3(_tme_stp103x_wrasr, tme_uint64_t)
{
  tme_uint64_t value_xor;
  unsigned int reg_rd;
  tme_uint64_t tick;
  struct timeval tick_compare_time;
  tme_uint64_t cycles_scaled;
  tme_uint64_t usec64;
  tme_uint32_t usec32;

  /* if this is an implementation-specific ASR: */
  if (TME_SPARC_INSN & (0x10 << 25)) {

    /* get the value to write: */
    value_xor = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

    /* get rd: */
    reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);

    /* if this is an undefined implementation-specific ASR: */
    if (__tme_predict_false(reg_rd >= 0x18)) {
      TME_SPARC_INSN_ILL(ic);
    }

    /* if this is an write to GSR: */
    if (reg_rd == 0x13) {
      if (__tme_predict_false((ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_PEF) == 0
			      || (ic->tme_sparc64_ireg_fprs & TME_SPARC64_FPRS_FEF) == 0)) {
	tme_sparc64_trap(ic, TME_SPARC64_TRAP_fp_disabled);
      }
      ic->tme_sparc_vis_gsr = value_xor;
    }

    /* otherwise, this is not a write to GSR: */
    else {

      /* if this is a nonprivileged write: */
      if (__tme_predict_false(!TME_SPARC_PRIV(ic))) {

	/* if this is not a write to PIC, or if PCR.PRIV is set: */
	if (reg_rd != 0x11
	    || (TME_STP103X(ic)->tme_stp103x_pcr & TME_BIT(0))) {
	  tme_sparc64_trap(ic, TME_SPARC64_TRAP_privileged_action);
	}
      }

      /* dispatch on rd: */
      switch (reg_rd) {
      default: assert (FALSE);
      case 0x10: TME_STP103X(ic)->tme_stp103x_pcr = value_xor; break;
      case 0x11: TME_STP103X(ic)->tme_stp103x_pic.tme_value64_uint = value_xor; break;
      case 0x12: TME_STP103X(ic)->tme_stp103x_dcr = value_xor; break;
      case 0x14: _tme_stp103x_update_sir(ic, 0, value_xor); break;
      case 0x15: _tme_stp103x_update_sir(ic, value_xor, 0); break;
      case 0x16: _tme_stp103x_update_sir(ic, 0xffff, value_xor); break;

      case 0x17:
	TME_STP103X(ic)->tme_stp103x_tcr = value_xor;

	/* if we're not replaying instructions: */
	if (tme_sparc_recode_verify_replay_last_pc(ic) == 0) {

	  /* if INT_DIS is set: */
	  if (__tme_predict_false(value_xor & TME_STP103X_TCR_INT_DIS)) {

	    /* the tick compare time is never: */
	    tme_misc_timeval_never(&tick_compare_time);
	  }

	  /* otherwise, INT_DIS is clear: */
	  else {

	    /* get the current value of TICK.counter: */
	    tick = tme_misc_cycles_scaled(&ic->tme_sparc_cycles_scaling, 0).tme_value64_uint;
	    tick += ic->tme_sparc64_ireg_tick_offset;
	    tick &= TME_SPARC64_TICK_COUNTER;

	    /* get the current time: */
	    gettimeofday(&tick_compare_time, NULL);

	    /* get the number of cycles until the compare value is
	       reached: */
	    cycles_scaled = TME_STP103X(ic)->tme_stp103x_tcr - tick;
	    cycles_scaled &= TME_SPARC64_TICK_COUNTER;

	    /* if the number of cycles doesn't fit in 32 bits: */
	    if (__tme_predict_false(cycles_scaled > (tme_uint32_t) (0 - (tme_uint32_t) 1))) {

	      /* convert cycles into microseconds: */
	      usec64 = cycles_scaled / ic->tme_sparc_cycles_scaled_per_usec;

	      /* add in the whole seconds: */
	      tick_compare_time.tv_sec += (usec64 / 1000000);

	      /* get the remaining microseconds: */
	      usec32 = (usec64 % 1000000);
	    }

	    /* otherwise, the number of cycles fits in 32 bits: */
	    else {

	      /* convert cycles into microseconds: */
	      usec32 = ((tme_uint32_t) cycles_scaled) / ic->tme_sparc_cycles_scaled_per_usec;

	      /* if there is at least one whole second: */
	      if (__tme_predict_false(usec32 >= 1000000)) {

		/* add in the whole seconds: */
		tick_compare_time.tv_sec += (usec32 / 1000000);

		/* get the remaining microseconds: */
		usec32 %= 1000000;
	      }
	    }

	    /* add in the microseconds: */
	    usec32 += tick_compare_time.tv_usec;
	    if (usec32 >= 1000000) {
	      tick_compare_time.tv_sec++;
	      usec32 -= 1000000;
	    }
	    tick_compare_time.tv_usec = usec32;
	  }

	  /* lock the external mutex: */
	  tme_mutex_lock(&ic->tme_sparc_external_mutex);

	  /* set the tick compare time: */
	  TME_STP103X(ic)->tme_stp103x_tick_compare_time = tick_compare_time;

	  /* notify the tick compare thread: */
	  tme_cond_notify(&TME_STP103X(ic)->tme_stp103x_tick_compare_cond, FALSE);

	  /* unlock the external mutex: */
	  tme_mutex_unlock(&ic->tme_sparc_external_mutex);
	}
	break;
      }
    }
    TME_SPARC_INSN_OK;
  }
  tme_sparc64_wrasr(ic, _rs1, _rs2, _rd);
}

/* the stp103x impdep1: */
static
TME_SPARC_FORMAT3(_tme_stp103x_impdep1, tme_uint64_t)
{
  tme_uint32_t opf;
  tme_uint64_t rd;
  unsigned int alignaddr_off;

  /* extract the opf field: */
  opf = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0x1ff << 5));

  /* if this is a shutdown instruction: */
  if (opf == 0x80) {
    TME_SPARC_INSN_PRIV;

    /* XXX WRITEME: */
    abort();
  }

  /* if this is the VIS alignaddr instruction: */
  if ((opf | TME_BIT(1)) == 0x1a) {

    /* add the two registers into an address: */
    rd = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;

    /* store the alignaddr offset: */
    alignaddr_off
      = (((opf & TME_BIT(1))
	  ? (sizeof(tme_uint64_t) - 1)
	  : 0)
	 ^ rd);
    TME_FIELD_MASK_DEPOSITU(ic->tme_sparc_vis_gsr,
			    TME_SPARC_VIS_GSR_ALIGNADDR_OFF,
			    alignaddr_off);

    /* truncate the address: */
    TME_SPARC_FORMAT3_RD = rd & (0 - (tme_uint64_t) sizeof(tme_uint64_t));

    TME_SPARC_INSN_OK;
  }

  /* otherwise, assume that this is some other VIS instruction: */
  tme_sparc_vis(ic);
  TME_SPARC_INSN_OK;
}

/* the stp103x impdep2: */
static
TME_SPARC_FORMAT3(_tme_stp103x_impdep2, tme_uint64_t)
{
  TME_SPARC_INSN_ILL(ic);
}

/* the stp103x flush: */
static
TME_SPARC_FORMAT3(_tme_stp103x_flush, tme_uint64_t)
{
  tme_uint64_t address;
  tme_uint32_t context;
  struct tme_sparc_tlb *dtlb;
  tme_uint64_t rd;

  /* if we're replaying instructions, return now: */
  if (tme_sparc_recode_verify_replay_last_pc(ic) != 0) {
    TME_SPARC_INSN_OK;
  }

  /* get the address: */
  address = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;
  address &= ic->tme_sparc_address_mask;

  /* get the context: */
  context = ic->tme_sparc_memory_context_default;

  /* get and busy the DTLB entry: */
  dtlb = &ic->tme_sparc_tlbs[TME_SPARC_DTLB_ENTRY(ic, TME_SPARC_TLB_HASH(ic, context, address))];
  tme_sparc_tlb_busy(dtlb);

  /* a flush instruction behaves like a non-faulting load: */
  tme_sparc64_ls(ic,
		 address,
		 &rd,
		 (TME_SPARC_LSINFO_OP_LD
		  + TME_SPARC_LSINFO_ASI_FLAGS(TME_SPARC64_ASI_FLAG_NO_FAULT)
		  + sizeof(tme_uint8_t)));

  /* unbusy the DTLB entry: */
  tme_sparc_tlb_unbusy(dtlb);

  TME_SPARC_INSN_OK;
}

/* the format three opcode map: */
#define _TME_SPARC_EXECUTE_OPMAP tme_sparc_opcodes_stp103x
static const _tme_sparc64_format3 _TME_SPARC_EXECUTE_OPMAP[] = {

  /* op=2: arithmetic, logical, shift, and remaining: */

  /* 000000 */ tme_sparc64_add,
  /* 000001 */ tme_sparc64_and,
  /* 000010 */ tme_sparc64_or,
  /* 000011 */ tme_sparc64_xor,
  /* 000100 */ tme_sparc64_sub,
  /* 000101 */ tme_sparc64_andn,
  /* 000110 */ tme_sparc64_orn,
  /* 000111 */ tme_sparc64_xnor,
  /* 001000 */ tme_sparc64_addx,
  /* 001001 */ tme_sparc64_mulx,
  /* 001010 */ tme_sparc64_umul,
  /* 001011 */ tme_sparc64_smul,
  /* 001100 */ tme_sparc64_subx,
  /* 001101 */ tme_sparc64_udivx,
  /* 001110 */ tme_sparc64_udiv,
  /* 001111 */ tme_sparc64_sdiv,
  /* 010000 */ tme_sparc64_addcc,
  /* 010001 */ tme_sparc64_andcc,
  /* 010010 */ tme_sparc64_orcc,
  /* 010011 */ tme_sparc64_xorcc,
  /* 010100 */ tme_sparc64_subcc,
  /* 010101 */ tme_sparc64_andncc,
  /* 010110 */ tme_sparc64_orncc,
  /* 010111 */ tme_sparc64_xnorcc,
  /* 011000 */ tme_sparc64_addxcc,
  /* 011001 */ NULL,
  /* 011010 */ tme_sparc64_umulcc,
  /* 011011 */ tme_sparc64_smulcc,
  /* 011100 */ tme_sparc64_subxcc,
  /* 011101 */ NULL,
  /* 011110 */ tme_sparc64_udivcc,
  /* 011111 */ tme_sparc64_sdivcc,
  /* 100000 */ tme_sparc64_taddcc,
  /* 100001 */ tme_sparc64_tsubcc,
  /* 100010 */ tme_sparc64_taddcctv,
  /* 100011 */ tme_sparc64_tsubcctv,
  /* 100100 */ tme_sparc64_mulscc,
  /* 100101 */ tme_sparc64_sll,
  /* 100110 */ tme_sparc64_srl,
  /* 100111 */ tme_sparc64_sra,
  /* 101000 */ _tme_stp103x_rdasr,
  /* 101001 */ NULL,
  /* 101010 */ _tme_stp103x_rdpr,
  /* 101011 */ tme_sparc64_flushw,
  /* 101100 */ tme_sparc64_movcc,
  /* 101101 */ tme_sparc64_sdivx,
  /* 101110 */ NULL,
  /* 101111 */ tme_sparc64_movr,
  /* 110000 */ _tme_stp103x_wrasr,
  /* 110001 */ tme_sparc64_saved_restored,
  /* 110010 */ tme_sparc64_wrpr,
  /* 110011 */ NULL,
  /* 110100 */ tme_sparc64_fpop1,
  /* 110101 */ tme_sparc64_fpop2,
  /* 110110 */ _tme_stp103x_impdep1,
  /* 110111 */ _tme_stp103x_impdep2,
  /* 111000 */ tme_sparc64_jmpl,
  /* 111001 */ tme_sparc64_return,
  /* 111010 */ tme_sparc64_tcc,
  /* 111011 */ _tme_stp103x_flush,
  /* 111100 */ tme_sparc64_save_restore,
  /* 111101 */ tme_sparc64_save_restore,
  /* 111110 */ tme_sparc64_done_retry,
  /* 111111 */ NULL,

  /* op=3: memory instructions: */

  /* 000000 */ tme_sparc64_ld,
  /* 000001 */ tme_sparc64_ldb,
  /* 000010 */ tme_sparc64_ldh,
  /* 000011 */ tme_sparc64_ldd,
  /* 000100 */ tme_sparc64_st,
  /* 000101 */ tme_sparc64_stb,
  /* 000110 */ tme_sparc64_sth,
  /* 000111 */ tme_sparc64_std,
  /* 001000 */ tme_sparc64_ld,
  /* 001001 */ tme_sparc64_ldb,
  /* 001010 */ tme_sparc64_ldh,
  /* 001011 */ tme_sparc64_ldx,
  /* 001100 */ NULL,
  /* 001101 */ tme_sparc64_ldstub,
  /* 001110 */ tme_sparc64_stx,
  /* 001111 */ tme_sparc64_swap,
  /* 010000 */ tme_sparc64_lda,
  /* 010001 */ tme_sparc64_ldba,
  /* 010010 */ tme_sparc64_ldha,
  /* 010011 */ tme_sparc64_ldda,
  /* 010100 */ tme_sparc64_sta,
  /* 010101 */ tme_sparc64_stba,
  /* 010110 */ tme_sparc64_stha,
  /* 010111 */ tme_sparc64_stda,
  /* 011000 */ tme_sparc64_lda,
  /* 011001 */ tme_sparc64_ldba,
  /* 011010 */ tme_sparc64_ldha,
  /* 011011 */ tme_sparc64_ldxa,
  /* 011100 */ NULL,
  /* 011101 */ tme_sparc64_ldstuba,
  /* 011110 */ tme_sparc64_stxa,
  /* 011111 */ tme_sparc64_swapa,
  /* 100000 */ tme_sparc64_ldf,
  /* 100001 */ tme_sparc64_ldfsr,
  /* 100010 */ tme_sparc64_illegal_instruction, /* ldqf */
  /* 100011 */ tme_sparc64_lddf,
  /* 100100 */ tme_sparc64_stf,
  /* 100101 */ tme_sparc64_stfsr,
  /* 100110 */ tme_sparc64_illegal_instruction, /* stqf */
  /* 100111 */ tme_sparc64_stdf,
  /* 101000 */ NULL,
  /* 101001 */ NULL,
  /* 101010 */ NULL,
  /* 101011 */ NULL,
  /* 101100 */ NULL,
  /* 101101 */ tme_sparc64_prefetch,
  /* 101110 */ NULL,
  /* 101111 */ NULL,
  /* 110000 */ tme_sparc64_ldfa,
  /* 110001 */ NULL,
  /* 110010 */ tme_sparc64_illegal_instruction, /* ldqfa */
  /* 110011 */ tme_sparc64_lddfa,
  /* 110100 */ tme_sparc64_stfa,
  /* 110101 */ NULL,
  /* 110110 */ tme_sparc64_illegal_instruction, /* stqfa */
  /* 110111 */ tme_sparc64_stdfa,
  /* 111000 */ NULL,
  /* 111001 */ NULL,
  /* 111010 */ NULL,
  /* 111011 */ NULL,
  /* 111100 */ tme_sparc64_casa,
  /* 111101 */ tme_sparc64_prefetch,
  /* 111110 */ tme_sparc64_casxa,
  /* 111111 */ NULL,
};

/* make the executor for the STP103x: */
#define _TME_SPARC_EXECUTE_NAME _tme_sparc_execute_stp103x
#include "sparc-execute.c"

/* this invalidates a specific valid stp103x TLB entry: */
static void
_tme_stp103x_tlb_invalidate(struct tme_sparc *ic,
			    signed long tlb_part_i)
{
  tme_uint32_t tlb_data_32_63;
  struct tme_sparc_tlb *tlb;
  tme_uint32_t tlb_count;
  tme_uint32_t size;
  tme_uint64_t address;
  tme_uint32_t context;

  /* load bits 32..63 of the TLB entry's data: */
  tlb_data_32_63 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1);

  /* this TLB entry must be valid: */
  assert (tlb_data_32_63 & TME_STP103X_TLB_DATA_V(tlb_data_32_63));

  /* this TLB entry is now invalid: */
  tlb_data_32_63 &= ~TME_STP103X_TLB_DATA_V(tlb_data_32_63);
  TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1) = tlb_data_32_63;

  /* if this TLB entry is global: */
  if (TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 0)
      & TME_STP103X_TLB_DATA_G((tme_uint32_t) 0)) {

    /* assume that this is an stp103x DTLB entry, and we need to
       invalidate sparc DTLB entries: */
    tlb = &ic->tme_sparc_tlbs[0];
    tlb_count = _TME_SPARC_DTLB_HASH_SIZE;

    /* if this is an stp103x ITLB entry: */
#if TME_STP103X_TLB_PART_0_DMMU >= TME_STP103X_TLB_PART_0_IMMU
#error "TME_STP103X_TLB_PART_0_DMMU or TME_STP103X_TLB_PART_0_IMMU changed"
#endif
    if (tlb_part_i >= TME_STP103X_TLB_PART_0_IMMU) {

      /* we need to invalidate sparc ITLB entries: */
      tlb += _TME_SPARC_DTLB_HASH_SIZE;
      tlb_count = _TME_SPARC_ITLB_HASH_SIZE;
    }

    /* invalidate all of the sparc TLB entries linked to this stp103x
       TLB entry: */
    do {
      if (tlb->tme_sparc_tlb_link == tlb_part_i) {
	tme_token_invalidate_nosync(tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
      }
      tlb++;
    } while (--tlb_count);
  }

  /* otherwise, this TLB entry isn't global: */
  else {

    /* get the size of this mapping: */
    size
      = (TME_STP103X_PAGE_SIZE_8KB
	 << (3 * TME_FIELD_MASK_EXTRACTU(tlb_data_32_63,
					 TME_STP103X_TLB_DATA_SIZE_MASK(tlb_data_32_63))));

    /* get the tag, and divide it into context and address: */
    address = TME_STP103X(ic)->tme_stp103x_tlb_64s(tlb_part_i + 0);
    context = address;
    context &= TME_STP103X_CONTEXT_MAX;
    address &= (0 - (tme_uint64_t) size);

    /* we assume that a stride of 8KB will find all sparc TLB entries
       for this stp103x TLB entry: */
    assert ((1 << ic->tme_sparc_tlb_page_size_log2) == TME_STP103X_PAGE_SIZE_8KB);

    /* if this is an stp103x DTLB entry: */
#if TME_STP103X_TLB_PART_0_DMMU >= TME_STP103X_TLB_PART_0_IMMU
#error "TME_STP103X_TLB_PART_0_DMMU or TME_STP103X_TLB_PART_0_IMMU changed"
#endif
    if (tlb_part_i < TME_STP103X_TLB_PART_0_IMMU) {

      /* invalidate all of the sparc DTLB entries linked to this
	 stp103x DTLB entry: */
      do {
	tlb = &ic->tme_sparc_tlbs[TME_SPARC_DTLB_ENTRY(ic, TME_SPARC_TLB_HASH(ic, context, address))];
	if (tlb->tme_sparc_tlb_link == tlb_part_i) {
	  tme_token_invalidate_nosync(tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
	}
	address += TME_STP103X_PAGE_SIZE_8KB;
      } while ((size -= TME_STP103X_PAGE_SIZE_8KB) > 0);
    }

    /* otherwise, this is an stp103x ITLB entry: */
    else {

      /* invalidate all of the sparc ITLB entries linked to this
	 stp103x ITLB entry: */
      do {
	tlb = &ic->tme_sparc_tlbs[TME_SPARC_ITLB_ENTRY(ic, TME_SPARC_TLB_HASH(ic, context, address))];
	if (tlb->tme_sparc_tlb_link == tlb_part_i) {
	  tme_token_invalidate_nosync(tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token);
	}
	address += TME_STP103X_PAGE_SIZE_8KB;
      } while ((size -= TME_STP103X_PAGE_SIZE_8KB) > 0);
    }
  }
}

/* this handles a load/store trap: */
static void
_tme_stp103x_ls_trap(struct tme_sparc *ic,
		     struct tme_sparc_ls *ls)
{
  tme_uint32_t lsinfo;
  tme_uint32_t sfsr;
  tme_uint32_t ls_faults;
  tme_uint32_t trap;
  tme_uint32_t updates;
  tme_uint32_t insn;
  tme_uint64_t afsr;
  tme_uint64_t address;
  tme_uint32_t asi_mask;
  struct tme_stp103x_mmu *mmu;

  /* get the information about this load/store: */
  lsinfo = ls->tme_sparc_ls_lsinfo;

  /* check that we weren't supposed to fault: */
  assert ((lsinfo & TME_STP103X_LSINFO_ASSERT_NO_FAULTS) == 0);

  /* if we're supposed to ignore faults: */
  if (__tme_predict_false(lsinfo & TME_SPARC_LSINFO_NO_FAULT)) {

    /* clear all faults and complete the load/store: */
    ls->tme_sparc_ls_faults = TME_SPARC_LS_FAULT_NONE;
    ls->tme_sparc_ls_size = 0;
    return;
  }

  /* start an SFSR value: */
  sfsr = 0;

  /* get the list of faults from this load/store: */
  ls_faults = ls->tme_sparc_ls_faults;

  /* there must be at least one fault: */
  assert (ls_faults != TME_SPARC_LS_FAULT_NONE);

  /* we don't support generic bus faults: */
  assert ((ls_faults
	   & (TME_SPARC_LS_FAULT_BUS_FAULT)) == 0);

  /* if this is an IMMU miss, this must be the only fault.  we rely on
     this to honor trap priorities, since we handle IMMU misses and
     DMMU misses as if they have the same priority, even though they
     don't: */
  assert ((ls_faults & TME_STP103X_LS_FAULT_MMU_MISS) == 0
	  || (lsinfo & TME_SPARC_LSINFO_OP_FETCH) == 0
	  || ls_faults == TME_STP103X_LS_FAULT_MMU_MISS);

  /* if this is a instruction access error, it must happen alone.  we
     rely on this to honor trap priorities, since we handle
     instruction access errors and data access errors as if they have
     the same priority, even though they don't: */
  assert ((ls_faults & TME_SPARC_LS_FAULT_BUS_ERROR) == 0
	  || (lsinfo & TME_SPARC_LSINFO_OP_FETCH) == 0
	  || ls_faults == TME_SPARC_LS_FAULT_BUS_ERROR);

  /* if this is an instruction access exception, it either happens
     alone (because of a privilege violation), or it's because of a
     jmpl/return target address out-of-range.  we rely on this to
     honor trap priorities, since we handle privilege violation
     instruction access exceptions and data access exceptions as if
     they have the same priority, even though they don't.  (a
     jmpl/return target instruction access exception explicitly has
     the same priority as a data access exception): */
  assert ((ls_faults
	   & (TME_STP103X_LS_FAULT_PRIVILEGE
	      | TME_SPARC64_LS_FAULT_SIDE_EFFECTS
	      | TME_SPARC64_LS_FAULT_UNCACHEABLE
	      | TME_SPARC64_LS_FAULT_NO_FAULT_NON_LOAD
	      | TME_STP103X_LS_FAULT_ILLEGAL
	      | TME_SPARC64_LS_FAULT_NO_FAULT_FAULT
	      | TME_SPARC64_LS_FAULT_VA_RANGE
	      | TME_SPARC64_LS_FAULT_VA_RANGE_NNPC)) == 0
	  || (lsinfo & TME_SPARC_LSINFO_OP_FETCH) == 0
	  || ls_faults == TME_STP103X_LS_FAULT_PRIVILEGE
	  || ls_faults == TME_SPARC64_LS_FAULT_VA_RANGE_NNPC);

  /* these traps are sorted by priority, most important first: */

  /* if this was an illegal instruction: */
  if (ls_faults
      & (TME_SPARC_LS_FAULT_LDD_STD_RD_ODD)) {

    /* make an illegal_instruction trap, which doesn't update any MMU
       registers: */
    trap = TME_SPARC64_TRAP_illegal_instruction;
    updates = TME_STP103X_UPDATE_NONE;
  }

  /* otherwise, if the address isn't aligned: */
  else if (ls_faults & TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED) {

    /* assume that this is not an lddf or stdf instruction, or that
       the address is not even 32-bit aligned, and make the normal
       mem_address_not_aligned trap: */
    trap = TME_SPARC64_TRAP_mem_address_not_aligned;

    /* if this is an lddf or stdf instruction: */
    insn = ic->_tme_sparc_insn;
    if ((insn
	 & (0x3b << 19))
	== (0x23 << 19)) {

      /* if the address is 32-bit aligned, but not 64-bit aligned: */
      if ((ls->tme_sparc_ls_address64 % sizeof(tme_uint64_t))
	  == sizeof(tme_uint32_t)) {

	/* make the LDDF_mem_address_not_aligned or
	   STDF_mem_address_not_aligned trap: */
	trap
	  = ((insn & (4 << 19))
	     ? TME_SPARC64_TRAP_STDF_mem_address_not_aligned
	     : TME_SPARC64_TRAP_LDDF_mem_address_not_aligned);
      }
    }

    /* these traps update the DMMU SFSR and SFAR: */
    updates
      = (TME_STP103X_UPDATE_DMMU
	 + TME_STP103X_UPDATE_MMU_SFSR
	 + TME_STP103X_UPDATE_DMMU_SFAR);
  }

  /* otherwise, if the ASI is privileged: */
  else if (ls_faults & TME_SPARC64_LS_FAULT_PRIVILEGED_ASI) {

    /* make a privileged_action trap, which updates the DMMU SFSR and
       SFAR: */
    trap = TME_SPARC64_TRAP_privileged_action;
    updates
      = (TME_STP103X_UPDATE_DMMU
	 + TME_STP103X_UPDATE_MMU_SFSR
	 + TME_STP103X_UPDATE_DMMU_SFAR);
  }

  /* otherwise, if this is an access exception: */
  else if (ls_faults
	   & (TME_STP103X_LS_FAULT_PRIVILEGE
	      | TME_SPARC64_LS_FAULT_SIDE_EFFECTS
	      | TME_SPARC64_LS_FAULT_UNCACHEABLE
	      | TME_SPARC64_LS_FAULT_NO_FAULT_NON_LOAD
	      | TME_STP103X_LS_FAULT_ILLEGAL
	      | TME_SPARC64_LS_FAULT_NO_FAULT_FAULT
	      | TME_SPARC64_LS_FAULT_VA_RANGE
	      | TME_SPARC64_LS_FAULT_VA_RANGE_NNPC)) {

    /* make the SFSR FT field: */
    if (ls_faults & TME_STP103X_LS_FAULT_PRIVILEGE) {
      sfsr += TME_STP103X_SFSR_FT_PRIVILEGE;
    }
    if (ls_faults & TME_SPARC64_LS_FAULT_SIDE_EFFECTS) {
      sfsr += TME_STP103X_SFSR_FT_SIDE_EFFECTS;
    }
    if (ls_faults & TME_SPARC64_LS_FAULT_UNCACHEABLE) {
      /* XXX FIXME WRITEME table 6-11 hints that it's possible to do a
	 casxa to DTLB_DATA_ACCESS_REG, and that a fault while doing
	 so will set this bit: */
      sfsr += TME_STP103X_SFSR_FT_UNCACHEABLE;
    }
    if (ls_faults & 
	(TME_SPARC64_LS_FAULT_NO_FAULT_NON_LOAD
	 | TME_STP103X_LS_FAULT_ILLEGAL)) {
      sfsr += TME_STP103X_SFSR_FT_ILLEGAL;
    }
    if (ls_faults & TME_SPARC64_LS_FAULT_NO_FAULT_FAULT) {
      sfsr += TME_STP103X_SFSR_FT_NO_FAULT_FAULT;
    }
    if (ls_faults & TME_SPARC64_LS_FAULT_VA_RANGE) {
      sfsr += TME_STP103X_SFSR_FT_VA_RANGE;
    }
    if (ls_faults & TME_SPARC64_LS_FAULT_VA_RANGE_NNPC) {
      sfsr += TME_STP103X_SFSR_FT_VA_RANGE_NNPC;
    }

    /* if this is an instruction fetch: */
    if (lsinfo & TME_SPARC_LSINFO_OP_FETCH) {

      /* the IMMU SFSR FT field must have exactly one of the privilege
	 and VA range bits set: */
      assert (sfsr != 0
	      && (sfsr
		  & ~(TME_STP103X_SFSR_FT_PRIVILEGE
		      | TME_STP103X_SFSR_FT_VA_RANGE
		      | TME_STP103X_SFSR_FT_VA_RANGE_NNPC)) == 0
	      && (sfsr & (sfsr - 1)) == 0);

      /* make an instruction_access_exception trap, which updates the
	 IMMU SFSR and tag access register: */
      trap = (TME_STP103X_TRAP_MG | TME_SPARC64_TRAP_instruction_access_exception);
      updates
	= (TME_STP103X_UPDATE_IMMU
	   + TME_STP103X_UPDATE_MMU_SFSR
	   + TME_STP103X_UPDATE_MMU_TAG_ACCESS);
    }

    /* otherwise, this is not an instruction fetch: */
    else {

      /* make a data_access_exception trap, which updates the DMMU
	 SFSR, SFAR, and tag access register: */
      trap = (TME_STP103X_TRAP_MG | TME_SPARC64_TRAP_data_access_exception);
      updates
	= (TME_STP103X_UPDATE_DMMU
	   + TME_STP103X_UPDATE_MMU_SFSR
	   + TME_STP103X_UPDATE_DMMU_SFAR
	   + TME_STP103X_UPDATE_MMU_TAG_ACCESS);
    }
  }

  /* otherwise, if this is an MMU miss: */
  else if (ls_faults & TME_STP103X_LS_FAULT_MMU_MISS) {

    /* if this is an instruction fetch: */
    if (lsinfo & TME_SPARC_LSINFO_OP_FETCH) {

      /* make an instruction_access_MMU_miss trap, which updates the
	 IMMU tag access register: */
      trap = TME_STP103X_TRAP_fast_instruction_access_MMU_miss;
      updates
	= (TME_STP103X_UPDATE_IMMU
	   + TME_STP103X_UPDATE_MMU_TAG_ACCESS);
    }

    /* otherwise, this is not an instruction fetch: */
    else {

      /* make a data_access_MMU_miss trap, which updates the DMMU tag
	 access: */
      trap = TME_STP103X_TRAP_fast_data_access_MMU_miss;
      updates
	= (TME_STP103X_UPDATE_DMMU
	   + TME_STP103X_UPDATE_MMU_TAG_ACCESS);
    }
  }

  /* otherwise, if this is a data access protection fault: */
  else if (ls_faults & TME_STP103X_LS_FAULT_PROTECTION) {

      /* make a fast_data_access_protection trap, which updates the
	 DMMU SFSR, SFAR, and tag access register: */
      trap = TME_STP103X_TRAP_fast_data_access_protection;
      updates
	= (TME_STP103X_UPDATE_DMMU
	   + TME_STP103X_UPDATE_MMU_SFSR
	   + TME_STP103X_UPDATE_DMMU_SFAR
	   + TME_STP103X_UPDATE_MMU_TAG_ACCESS);
  }

  /* otherwise, this must be an access error fault: */
  else {
    assert (ls_faults & TME_SPARC_LS_FAULT_BUS_ERROR);

    /* get the asynchronous fault status register bit for this
       fault: */
    /* NB: we assume that all bus errors are timeouts: */
    afsr = TME_STP103X_AFSR_TO;

    /* if no same or higher-priority asynchronous fault has already
       happened: */
    if ((TME_STP103X(ic)->tme_stp103x_afsr
	 & (TME_STP103X_AFSR_ME - afsr)) == 0) {
      
      /* update the asynchronous fault address register: */
      assert (ls->tme_sparc_ls_tlb->tme_sparc_tlb_addr_shift == 0);
      TME_STP103X(ic)->tme_stp103x_afar
	= ((ls->tme_sparc_ls_address64
	    + (tme_uint64_t) ls->tme_sparc_ls_tlb->tme_sparc_tlb_addr_offset)
	   & (0 - (tme_uint64_t) (1 << 4)));
    }

    /* update the asynchronous fault status register: */
    TME_STP103X(ic)->tme_stp103x_afsr
      |= (((TME_STP103X(ic)->tme_stp103x_afsr
	    & afsr)
	   ? TME_STP103X_AFSR_ME
	   : !TME_STP103X_AFSR_ME)
	  + (TME_SPARC_PRIV(ic)
	     ? TME_STP103X_AFSR_PRIV
	     : !TME_STP103X_AFSR_PRIV));

    /* if noncacheable errors are not enabled: */
    if ((TME_STP103X(ic)->tme_stp103x_estate_error_enable & TME_STP103X_ESTATE_ERROR_ENABLE_NCEEN) == 0) {

      /* clear the fault: */
      ls->tme_sparc_ls_faults = TME_SPARC_LS_FAULT_NONE;

      /* if this is a load: */
      if (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_LD) {

	/* force all bytes read to be all-bits-one by filling the
	   entire memory buffer with all-bits-one: */
	/* NB: we do this even though earlier parts of the load may
	   have succeeded, to keep things simple.  we assume that
	   nothing depends on partially successful loads: */
	memset (ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s,
		0xff, 
		sizeof(ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s));
      }

      /* stop this load or store: */
      /* NB: we do this even though later parts of the load or store
	 may succeed, to keep things simple.  we assume that nothing
	 depends on partially successful loads or stores: */
      /* NB: we don't set TME_SPARC_LSINFO_LD_COMPLETED because we are
	 only stopping the load or store here, not completing it.  we
	 still let the original load instruction function complete the
	 load from the memory buffer: */
      ls->tme_sparc_ls_size = 0;

      /* make no trap: */
      trap = TME_SPARC_TRAP_none;
    }

    /* otherwise, if this is an instruction fetch: */
    else if (lsinfo & TME_SPARC_LSINFO_OP_FETCH) {

      /* make an instruction_access_error trap: */
      trap = TME_SPARC64_TRAP_instruction_access_error;
    }

    /* otherwise, this is not an instruction fetch: */
    else {

      /* make a data_access_error trap: */
      trap = TME_SPARC64_TRAP_data_access_error;
    }

    /* this trap doesn't update any other state: */
    updates = TME_STP103X_UPDATE_NONE;
  }

  /* get the virtual address, forced to be in range: */
  address = ls->tme_sparc_ls_address64;
  address |= (0 - (TME_STP103X_VA_HOLE_START * 2));
  address = (address ^ TME_STP103X_VA_HOLE_START) + TME_STP103X_VA_HOLE_START;

  /* if this trap updates the DMMU SFAR: */
  if (updates & TME_STP103X_UPDATE_DMMU_SFAR) {
    TME_STP103X(ic)->tme_stp103x_dmmu_sfar = address;
  }

  /* assume that this trap updates the IMMU: */
  mmu = &TME_STP103X(ic)->tme_stp103x_immu;

  /* if this trap updates the DMMU: */
  if ((updates
       & (TME_STP103X_UPDATE_DMMU
	  | TME_STP103X_UPDATE_IMMU))
      != TME_STP103X_UPDATE_IMMU) {

    /* assume that this trap updates the DMMU SFSR and define
       SFSR.E: */
    if (ls->tme_sparc_ls_tlb->tme_sparc_tlb_asi_mask
	& TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS) {
      sfsr += TME_STP103X_SFSR_E;
    }

    /* update the common DMMU state: */
    mmu = &TME_STP103X(ic)->tme_stp103x_dmmu;
  }

  /* get the ASI mask from the instruction: */
  asi_mask = ls->tme_sparc_ls_asi_mask;

  /* if this trap updates the DMMU or IMMU tag access register: */
  if (updates & TME_STP103X_UPDATE_MMU_TAG_ACCESS) {

    /* update the tag access: */
    mmu->tme_stp103x_mmu_tag_access
      = ((address
	  & (0 - (tme_uint64_t) (TME_STP103X_CONTEXT_MAX + 1)))
	 + ((asi_mask & TME_SPARC64_ASI_MASK_FLAG_SPECIAL)
	    ? 0
	    : ls->tme_sparc_ls_context));
  }

  /* if this trap updates the DMMU or IMMU SFSR: */
  if (updates & TME_STP103X_UPDATE_MMU_SFSR) {

    /* define SFSR.ASI: */
    TME_FIELD_MASK_DEPOSITU(sfsr,
			    TME_STP103X_SFSR_ASI,
			    TME_SPARC_ASI_MASK_WHICH(asi_mask));

    /* define SFSR.CT: */
    sfsr
      += ((asi_mask & TME_SPARC64_ASI_MASK_FLAG_SPECIAL)
	  ? TME_STP103X_SFSR_CT_RESERVED
	  : (asi_mask & TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS)
	  ? TME_STP103X_SFSR_CT_NUCLEUS
	  : (asi_mask & TME_SPARC64_ASI_FLAG_SECONDARY)
	  ? TME_STP103X_SFSR_CT_SECONDARY
	  : TME_STP103X_SFSR_CT_PRIMARY);

    /* define SFSR.PR: */
    if (TME_SPARC_PRIV(ic)) {
      sfsr += TME_STP103X_SFSR_PR;
    }

    /* define SFSR.W: */
    if (lsinfo
	& (TME_SPARC_LSINFO_OP_ST
	   | TME_SPARC_LSINFO_OP_ATOMIC)) {
      sfsr += TME_STP103X_SFSR_W;
    }

    /* define SFSR.FV: */
    sfsr += TME_STP103X_SFSR_FV;

    /* define SFSR.OW: */
    if (mmu->tme_stp103x_mmu_sfsr & TME_STP103X_SFSR_FV) {
      sfsr += TME_STP103X_SFSR_OW;
    }

    /* update the DMMU or IMMU SFSR: */
    mmu->tme_stp103x_mmu_sfsr = sfsr;
  }

  /* if there is a trap: */
  if (__tme_predict_true(trap != TME_SPARC_TRAP_none)) {

    /* trap: */
    tme_sparc64_trap(ic, trap);
  }
}

/* this checks that the virtual address of a load/store is in range.
   if it is out of range and ic is non-NULL, it traps immediately: */
static tme_uint64_t
_tme_stp103x_ls_address_check(struct tme_sparc *ic,
			      struct tme_sparc_ls *ls)
{
  tme_uint64_t address;
  tme_uint32_t address_32_63;

  /* get bits 32..63 of the address: */
  address = ls->tme_sparc_ls_address64;
  address_32_63 = address >> 32;

  /* if this address is in the address space hole: */
  if (__tme_predict_false((address_32_63 + (tme_uint32_t) (TME_STP103X_VA_HOLE_START >> 32))
			  >= (tme_uint32_t) ((TME_STP103X_VA_HOLE_START * 2) >> 32))) {

    /* note the fault: */
    ls->tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_VA_RANGE;

    /* if we can, trap now: */
    if (ic != NULL) {
      _tme_stp103x_ls_trap(ic, ls);
      abort();
      /* NOTREACHED */
    }
  }

  /* return the address: */
  return (address);
}

/* this maps a virtual address directly into a physical address: */
static void
_tme_stp103x_ls_address_map_phys(struct tme_sparc *ic,
				 struct tme_sparc_ls *ls)
{
  tme_uint64_t address;
  tme_uint32_t asi;
  tme_uint32_t asi_mask;

  /* check the address: */
  address = _tme_stp103x_ls_address_check(ic, ls);

  /* get the original ASI: */
  asi = TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask);

  /* all of the bypass ASIs behave as if the P bit were clear: */
  asi_mask
    = (TME_SPARC64_ASI_MASK_PRIV
       + TME_SPARC64_ASI_MASK_USER);

  /* ASIs 0x15 and 0x1d behave as if the E bit were set and the CP bit
     were clear: */
  if (asi & 1) {
    asi_mask
      += (TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS
	  + TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE);
  }

  /* update the flags on the TLB entry: */
  ls->tme_sparc_ls_tlb->tme_sparc_tlb_asi_mask
    |= asi_mask;

  /* do the truncating mapping: */
  address &= (0 - (tme_uint64_t) TME_STP103X_PAGE_SIZE_8KB);
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first = address;
  address |= (TME_STP103X_PAGE_SIZE_8KB - 1);
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last = address;
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE);
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset
    = ((address % TME_STP103X_PA_SIZE)
       - address);
}

/* this maps a virtual address: */
static void
_tme_stp103x_ls_address_map(struct tme_sparc *ic,
			    struct tme_sparc_ls *ls)
{
  tme_uint64_t address;
  tme_uint32_t lsu_0_31;
  signed long tlb_part_i;
  tme_uint32_t tlb_tag_match_0_31;
  tme_uint32_t tlb_tag_match_32_63;
  tme_uint32_t tlb_tag_xor_0_31;
  tme_uint32_t tlb_data_32_63;
  tme_uint32_t size;
  tme_uint32_t tlb_data_0_31;
  struct tme_sparc_tlb *tlb;
  tme_uint32_t asi_mask;
  tme_uint32_t cycles_ok;

  /* check the address: */
  address = _tme_stp103x_ls_address_check(ic, ls);

  /* get bits 0..31 of the load/store unit control register: */
  lsu_0_31 = TME_STP103X(ic)->tme_stp103x_lsu;

  /* assume that this is not an instruction fetch, and start at the
     beginning of the DMMU TLB entries: */
  tlb_part_i = TME_STP103X_TLB_PART_0_DMMU;

  /* if this is an instruction fetch: */
  if (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_FETCH) {

    /* assume that LSU.IM is set, and set LSU.DM to indicate an
       enabled IMMU (!): */
    lsu_0_31 |= TME_STP103X_LSU_DM;

    /* if the IMMU is disabled, or if the CPU is in RED_state: */
    if (__tme_predict_false((lsu_0_31 & TME_STP103X_LSU_IM) == 0
			    || (ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_RED))) {

      /* clear LSU.DM, to indicate a disabled IMMU (!): */
      lsu_0_31 &= ~TME_STP103X_LSU_DM;
    }

    /* start at the beginning of the IMMU TLB entries: */
    tlb_part_i = TME_STP103X_TLB_PART_0_IMMU;
  }

  /* if the MMU is disabled: */
  if (__tme_predict_false((lsu_0_31 & TME_STP103X_LSU_DM) == 0)) {

    /* a disabled MMU behaves as if the E bit were set, and the P and
       CP bits were clear: */
    ls->tme_sparc_ls_tlb->tme_sparc_tlb_asi_mask
      |= (TME_SPARC64_ASI_MASK_PRIV
	  + TME_SPARC64_ASI_MASK_USER
	  + TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS
	  + TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE);

    /* do the truncating mapping: */
    address &= (0 - (tme_uint64_t) TME_STP103X_PAGE_SIZE_8KB);
    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first = address;
    address |= (TME_STP103X_PAGE_SIZE_8KB - 1);
    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last = address;
    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE);
    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset
      = ((address % TME_STP103X_PA_SIZE)
	 - address);
    return;
  }

  /* make the tag to match: */
  tlb_tag_match_32_63 = (address >> 32);
  tlb_tag_match_0_31 = address;
  tlb_tag_match_0_31 &= ~TME_STP103X_CONTEXT_MAX;
  tlb_tag_match_0_31 += ls->tme_sparc_ls_context;

  /* loop over TLB entries: */
  do {

    /* if bits 32..63 of the tag match: */
    if (TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 0), 1) == tlb_tag_match_32_63) {

      /* if bits 22..31 of the tag match: */
      tlb_tag_xor_0_31 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 0), 0) ^ tlb_tag_match_0_31;
      if (tlb_tag_xor_0_31 < TME_STP103X_PAGE_SIZE_4MB) {

	/* load bits 32..63 of the data: */
	tlb_data_32_63 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1);

	/* if the data is valid: */
	if (tlb_data_32_63 & TME_STP103X_TLB_DATA_V(tlb_data_32_63)) {

	  /* get the size of this mapping: */
	  size
	    = (TME_STP103X_PAGE_SIZE_8KB
	       << (3 * TME_FIELD_MASK_EXTRACTU(tlb_data_32_63,
					       TME_STP103X_TLB_DATA_SIZE_MASK(tlb_data_32_63))));

	  /* load bits 0..31 of the data: */
	  tlb_data_0_31 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 0);

	  /* if bits 0..31 of the tag match: */
	  if ((tlb_tag_xor_0_31
	       & ((0 - size)
		  + ((tlb_data_0_31
		      & TME_STP103X_TLB_DATA_G(tlb_data_0_31))
		     ? 0
		     : TME_STP103X_CONTEXT_MAX))) == 0) {

	    /* set the used bit: */
	    tlb_data_32_63 |= TME_STP103X_TLB_DATA_DIAG_USED(tlb_data_32_63);
	    TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1) = tlb_data_32_63;

	    /* get the sparc TLB entry: */
	    tlb = ls->tme_sparc_ls_tlb;

	    /* if this is a global mapping, update the TLB context: */
	    if (tlb_data_0_31 & TME_STP103X_TLB_DATA_G(tlb_data_0_31)) {
	      tlb->tme_sparc_tlb_context = TME_STP103X_CONTEXT_MAX + 1;
	    }

	    /* link the TLB entries: */
	    tlb->tme_sparc_tlb_link = tlb_part_i;

	    /* start the address offset for this mapping: */
	    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset
	      = ((tlb_data_0_31
		  & (tme_uint32_t) TME_STP103X_TLB_DATA_PA)
		 + (((tme_uint64_t)
		     (tlb_data_32_63
		      & (tme_uint32_t) (TME_STP103X_TLB_DATA_PA >> 32)))
		    << 32));

	    /* copy the E, !CP, IE, and NFO bits into the TLB entry's
	       ASI mask.  we predict for the common case, which has CP
	       set and all of the other bits clear: */
	    /* NB: we ignore the CV bit, because it's always zero in
	       the IMMU, and in the DMMU it doesn't count as
	       uncacheable as far as atomic instructions are
	       concerned: */
	    if (__tme_predict_false((tlb_data_0_31
				     & (TME_STP103X_TLB_DATA_E(tlb_data_0_31)
					| TME_STP103X_TLB_DATA_CP(tlb_data_0_31)))
				    != TME_STP103X_TLB_DATA_CP(tlb_data_0_31))) {
	      asi_mask = 0;
	      if (tlb_data_0_31 & TME_STP103X_TLB_DATA_E(tlb_data_0_31)) {
		asi_mask += TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS;
	      }
	      if ((tlb_data_0_31 & TME_STP103X_TLB_DATA_CP(tlb_data_0_31)) == 0) {
		asi_mask += TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE;
	      }
	    }
	    else {
	      asi_mask = 0;
	    }
	    if (__tme_predict_false(tlb_data_32_63
				    & (TME_STP103X_TLB_DATA_NFO(tlb_data_32_63)
				       | TME_STP103X_TLB_DATA_IE(tlb_data_32_63)))) {
	      if (tlb_data_32_63 & TME_STP103X_TLB_DATA_NFO(tlb_data_32_63)) {
		asi_mask += TME_SPARC64_ASI_FLAG_NO_FAULT;
	      }
	      if (tlb_data_32_63 & TME_STP103X_TLB_DATA_IE(tlb_data_32_63)) {
		asi_mask += TME_SPARC64_ASI_FLAG_LITTLE;
	      }
	    }

	    /* if this mapping is privileged: */
	    if (tlb_data_0_31 & TME_STP103X_TLB_DATA_P(tlb_data_0_31)) {

	      /* if this access is not privileged: */
	      if (__tme_predict_false(!TME_SPARC_PRIV(ic))) {

		/* trap: */
		ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_PRIVILEGE;
		_tme_stp103x_ls_trap(ic, ls);
		assert (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_NO_FAULT);
		return;
	      }
	    }

	    /* otherwise, this mapping is not privileged: */
	    else {

	      /* this TLB entry can be used for privileged and nonprivileged
		 accesses: */
	      asi_mask
		+= (TME_SPARC64_ASI_MASK_PRIV
		    + TME_SPARC64_ASI_MASK_USER);
	    }

	    /* update the TLB entry's ASI mask: */
	    ls->tme_sparc_ls_tlb->tme_sparc_tlb_asi_mask |= asi_mask;

	    /* if this page is writable: */
	    if (tlb_data_0_31 & TME_STP103X_TLB_DATA_W(tlb_data_0_31)) {

	      /* this mapping can be read and written: */
	      cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
	    }

	    /* otherwise, this page is not writable: */
	    else {

	      /* if this is a store or an atomic: */
	      if (__tme_predict_false(ls->tme_sparc_ls_lsinfo
				      & (TME_SPARC_LSINFO_OP_ST
					 | TME_SPARC_LSINFO_OP_ATOMIC))) {

		/* remember if this is a 64KB page: */
		TME_STP103X(ic)->tme_stp103x_dmmu_direct_64KB = (size == TME_STP103X_PAGE_SIZE_64KB);

		/* trap: */
		ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_PROTECTION;
		_tme_stp103x_ls_trap(ic, ls);
		abort();
		/* NOTREACHED */
	      }

	      /* this mapping can only be read: */
	      cycles_ok = TME_BUS_CYCLE_READ;
	    }

	    /* set the cycles for this mapping: */
	    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok = cycles_ok;

	    /* set the first and last addresses for this mapping: */
	    address = ls->tme_sparc_ls_address64;
	    address |= (size - 1);
	    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last = address;
	    address &= (0 - (tme_uint64_t) size);
	    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first = address;

	    /* finish the address offset for this mapping: */
	    ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset -= address;

	    return;
	  }
	}
      }
    }

    tlb_part_i += 2;
  } while (tlb_part_i % (2 * TME_STP103X_TLB_SIZE));

  /* this is a miss: */
  ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_MMU_MISS;
  _tme_stp103x_ls_trap(ic, ls);
  assert (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_NO_FAULT);
}

/* the ASI handler for ASI_PHYS_USE_EC*, ASI_PHYS_BYPASS_EC_WITH_EBIT*: */
static void
_tme_stp103x_ls_asi_phys(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{

  /* override the address map function: */
  ls->tme_sparc_ls_address_map = _tme_stp103x_ls_address_map_phys;
}

/* the cycle handler for ASI_NUCLEUS_QUAD_LDD*: */
static void
_tme_stp103x_ls_cycle_quad(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  struct tme_sparc_tlb *tlb;
  tme_uint32_t asi_mask;
  _tme_const tme_shared tme_uint8_t *memory;
#if !TME_THREADS_COOPERATIVE
#ifdef tme_memory_atomic_read128
  tme_uint128_t quad;
#endif /* tme_memory_atomic_read128 */
#endif /* !TME_THREADS_COOPERATIVE */
  tme_uint64_t quad_64lo;
  tme_uint64_t quad_64hi;
  tme_uint64_t *_rd;

  /* get the TLB entry: */
  tlb = ls->tme_sparc_ls_tlb;

  /* get the ASI mask: */
  asi_mask = tlb->tme_sparc_tlb_asi_mask;
  
  /* this TLB entry must be for cacheable memory: */
  if (__tme_predict_false(asi_mask
			  & TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE)) {

    /* we must have caught this on the first cycle: */
    assert (ls->tme_sparc_ls_buffer_offset == 0);

    /* fault: */
    ls->tme_sparc_ls_faults |= TME_SPARC64_LS_FAULT_UNCACHEABLE;
    return;
  }

  /* assume that we can't do a fast transfer: */
  memory = TME_EMULATOR_OFF_UNDEF;

  /* if this is the first cycle: */
  if (ls->tme_sparc_ls_buffer_offset == 0) {

    /* if this TLB entry allows fast transfer of all of the addresses: */
    if (__tme_predict_true((((tme_bus_addr64_t) tlb->tme_sparc_tlb_addr_last)
			    - ls->tme_sparc_ls_address64)
			   >= ((sizeof(tme_uint64_t) * 2) - 1))) {

      /* we may be able do a fast transfer: */
      memory = tlb->tme_sparc_tlb_emulator_off_read;
    }
  }

  /* if we can't do a fast transfer: */
  if (__tme_predict_false(memory == TME_EMULATOR_OFF_UNDEF)) {

    /* do a slow cycle: */
    tme_sparc64_load(ic, ls);

    /* if this was not the last cycle, return now: */
    if (ls->tme_sparc_ls_size != 0) {
      return;
    }

    /* fake a fast transfer from the memory buffer: */
    memory = &ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer8s[0];
    memory -= ls->tme_sparc_ls_address64;
  }

  /* finish the memory address: */
  memory += ls->tme_sparc_ls_address64;

  /* if threads are cooperative: */
#if TME_THREADS_COOPERATIVE

  /* do two 64-bit loads: */
  quad_64lo
    = tme_memory_bus_read64(((_tme_const tme_shared tme_uint64_t *) memory) + 0,
			    tlb->tme_sparc_tlb_bus_rwlock,
			    (sizeof(tme_uint64_t) * 2),
			    sizeof(tme_uint64_t));
  quad_64hi
    = tme_memory_bus_read64(((_tme_const tme_shared tme_uint64_t *) memory) + 1,
			    tlb->tme_sparc_tlb_bus_rwlock,
			    (sizeof(tme_uint64_t) * 1),
			    sizeof(tme_uint64_t));

  /* otherwise, threads are not cooperative: */
#else  /* !TME_THREADS_COOPERATIVE */

  /* if host supports an atomic 128-bit read: */
#ifdef tme_memory_atomic_read128

  /* do the atomic 128-bit read: */
  quad
    = tme_memory_atomic_read128((_tme_const tme_shared tme_uint128_t *) memory,
				tlb->tme_sparc_tlb_bus_rwlock,
				sizeof(tme_uint128_t));

  /* get the two parts of the load: */
#if TME_ENDIAN_NATIVE == TME_ENDIAN_BIG
  quad_64lo = quad >> 64;
  quad_64hi = quad;
#elif TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE
  quad_64lo = quad;
  quad_64hi = quad >> 64;
#endif
#else  /* !tme_memory_atomic_read128 */
#error "non-cooperative threads requires an atomic 128-bit read"
#endif /* !tme_memory_atomic_read128 */
#endif /* !TME_THREADS_COOPERATIVE */

  /* swap the two 64-bit values as needed: */
  if (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_ENDIAN_LITTLE) {
    quad_64lo = tme_letoh_u64(quad_64lo);
    quad_64hi = tme_letoh_u64(quad_64hi);
  }
  else {
    quad_64lo = tme_betoh_u64(quad_64lo);
    quad_64hi = tme_betoh_u64(quad_64hi);
  }

  /* complete the load: */
  ls->tme_sparc_ls_size = 0;
  _rd = ls->tme_sparc_ls_rd64;
  TME_SPARC_FORMAT3_RD = quad_64lo;
  TME_SPARC_FORMAT3_RD_ODD(tme_ic_ireg_uint64) = quad_64hi;
}

/* the ASI handler for ASI_NUCLEUS_QUAD_LDD*: */
static void
_tme_stp103x_ls_asi_quad(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{

  /* we need to do the complete load: */
  ls->tme_sparc_ls_size = sizeof(tme_uint64_t) * 2;
  ls->tme_sparc_ls_buffer_offset = 0;
  ls->tme_sparc_ls_lsinfo
    |= (TME_SPARC_LSINFO_SLOW_CYCLES
	+ TME_SPARC_LSINFO_LD_COMPLETED);
  ls->tme_sparc_ls_cycle = _tme_stp103x_ls_cycle_quad;

  /* an instruction other than ldda is illegal: */
  if (__tme_predict_false((ic->_tme_sparc_insn
			   & (0x3f << 19))
			  != (0x13 << 19))) {
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* the address must be 128-bit aligned: */
  if (__tme_predict_false(((tme_uint32_t) ls->tme_sparc_ls_address64)
			  % (sizeof(tme_uint64_t) * 2))) {
    ls->tme_sparc_ls_faults |= TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;
  }
}

/* the ASI handler for various infrequently-used ASIs: */
static void
_tme_stp103x_ls_asi_slow(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint32_t lsinfo_ops;
  int size_ok;
  int address_ok;
  tme_uint64_t value_mask;
  void (*update_write) _TME_P((struct tme_sparc *, tme_uint64_t));
  int redispatch;
  tme_uint64_t *_value64;
  tme_uint32_t *_value32;
  tme_uint64_t value;
  tme_uint64_t value_now;

  /* assume that this ASI allows reads and writes: */
  lsinfo_ops
    = (TME_SPARC_LSINFO_OP_LD
       | TME_SPARC_LSINFO_OP_ST);

  /* assume that this ASI allows only 64-bit accesses: */
  size_ok = (ls->tme_sparc_ls_size == sizeof(tme_uint64_t));

  /* assume that this ASI allows only accesses to address zero: */
  address_ok = (ls->tme_sparc_ls_address64 == 0);

  /* assume that this ASI doesn't mask values written: */
  value_mask = 0 - (tme_uint64_t) 1;

  /* assume that this ASI has no write side-effects: */
  update_write = NULL;

  /* assume that a write won't need a redispatch: */
  redispatch = FALSE;

  /* assume that the access is invalid: */
  _value64 = NULL;
  _value32 = NULL;

  /* dispatch on the ASI: */
  switch (TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask)) {

    /* all unknown ASIs: */
  default:
    address_ok = FALSE;
    break;

  case TME_STP103X_ASI_LSU_CONTROL_REG:
    _value64 = &TME_STP103X(ic)->tme_stp103x_lsu;
    update_write = _tme_stp103x_update_lsu;
    break;

  case TME_STP103X_ASI_INTR_DISPATCH_STATUS:
    lsinfo_ops = TME_SPARC_LSINFO_OP_LD;
    /* XXX FIXME WRITEME: */
    abort();
    break;

  case TME_STP103X_ASI_INTR_RECEIVE:
    value_now
      = ((tme_memory_atomic_read_flag(&TME_STP103X(ic)->tme_stp103x_intr_receive_busy)
	  ? TME_STP103X_INTR_RECEIVE_BUSY
	  : 0)
	 + TME_STP103X(ic)->tme_stp103x_intr_receive_mid);
    _value64 = &value_now;
    update_write = _tme_stp103x_update_intr_receive;
    break;

  case TME_STP103X_ASI_UPA_CONFIG_REG:
    _value64 = &TME_STP103X(ic)->tme_stp103x_upa_config;
    update_write = _tme_stp103x_update_upa_config;
    break;
    
  case TME_STP103X_ASI_ESTATE_ERROR_EN_REG:
    _value32 = &TME_STP103X(ic)->tme_stp103x_estate_error_enable;
    value_mask
      = (TME_STP103X_ESTATE_ERROR_ENABLE_CEEN
	 | TME_STP103X_ESTATE_ERROR_ENABLE_NCEEN
	 | TME_STP103X_ESTATE_ERROR_ENABLE_ISAPEN);
    break;

  case TME_STP103X_ASI_AFSR:
    _value64 = &TME_STP103X(ic)->tme_stp103x_afsr;
    update_write = _tme_stp103x_update_afsr;
    break;

  case TME_STP103X_ASI_AFAR:
    _value64 = &TME_STP103X(ic)->tme_stp103x_afar;
    value_mask = TME_STP103X_PA_SIZE - (1 << 4);
    break;

  case TME_STP103X_ASI_ECACHE_TAG_DATA:
    _value32 = &TME_STP103X(ic)->tme_stp103x_ecache_tag_data;
    break;
  }

  /* check the access: */
  if (__tme_predict_false((ls->tme_sparc_ls_lsinfo & lsinfo_ops) == 0
			  || !size_ok
			  || !address_ok)) {
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if there are any faults: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults |= TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* get the raw value to read or write: */
  value
    = ((ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ST)
       ? *ls->tme_sparc_ls_rd64
       : _value64 != NULL
       ? *_value64
       : *_value32);

  /* mask the value: */
  value &= value_mask;

  /* complete the load or store: */
  ls->tme_sparc_ls_size = 0;

  /* if this is a load: */
  if (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_LD) {

    /* complete the load: */
    *ls->tme_sparc_ls_rd64 = value;
    ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
  }

  /* otherwise, this is a store: */
  else {

    /* if this value has write side-effects: */
    if (update_write != NULL) {

      /* do the write side-effects: */
      (*update_write)(ic, value);
    }

    /* otherwise, this value has no write side-effects: */
    else {

      /* complete the store: */
      if (_value64 != NULL) {
	*_value64 = value;
      }
      else {
	*_value32 = value;
      }
    }

    /* if this store needs a redispatch: */
    if (redispatch) {
      tme_bus_tlb_unbusy(&ic->tme_sparc_tlbs[ls->tme_sparc_ls_tlb_i].tme_sparc_tlb_bus_tlb);
      tme_sparc_redispatch(ic);
    }
  }
}

/* the ASI handler for ASI_DMMU and ASI_IMMU: */
static void
_tme_stp103x_ls_asi_mmu(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint64_t address;
  tme_uint32_t address_0_31;
  struct tme_stp103x_mmu *mmu;
  tme_uint32_t lsinfo;
  tme_uint64_t tag_target;
  tme_uint32_t context;
  tme_uint64_t *_value64;
  tme_uint32_t *_value32;
  int value_has_va;
  tme_uint64_t value_mask;
  tme_uint32_t lsinfo_ops;
  int redispatch;
  tme_uint64_t value;

  /* if this is a 64-bit access that hasn't faulted yet: */
  if (__tme_predict_true(ls->tme_sparc_ls_size == sizeof(tme_uint64_t)
			 && ls->tme_sparc_ls_faults == TME_SPARC_LS_FAULT_NONE)) {

    /* get the address: */
    address = ls->tme_sparc_ls_address64;

    /* if the address fits into 32 bits: */
    if (__tme_predict_true((address & (0 - (((tme_uint64_t) 1) << 32))) == 0)) {

      /* truncate the address to 32 bits: */
      address_0_31 = address;

      /* get the MMU state: */
      mmu
	= (TME_STP103X_ASI_MMU_MASK_IS_DMMU(ls->tme_sparc_ls_asi_mask)
	   ? &TME_STP103X(ic)->tme_stp103x_dmmu
	   : &TME_STP103X(ic)->tme_stp103x_immu);

      /* get the load/store information: */
      lsinfo = ls->tme_sparc_ls_lsinfo;

      /* address 0x0 is the tag target register: */
      if (address_0_31 == 0) {

	/* if this is a load: */
	if (lsinfo & TME_SPARC_LSINFO_OP_LD) {

	  /* make the value for the tag target register: */
	  tag_target = mmu->tme_stp103x_mmu_tag_access;
	  context = ((tme_uint32_t) tag_target) & TME_STP103X_CONTEXT_MAX;
	  tag_target >>= 22;
	  tag_target |= ((tme_uint64_t) (context << (48 - 32))) << 32;

	  /* complete the load: */
	  *ls->tme_sparc_ls_rd64 = tag_target;
	  ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
	  ls->tme_sparc_ls_size = 0;
	  return;
	}
      }

      else {

	/* assume that the register is invalid: */
	_value64 = NULL;
	_value32 = NULL;

	/* assume that values for this register aren't virtual
	   addresses, and don't have any masked bits: */
	value_has_va = FALSE;
	value_mask = 0 - (tme_uint64_t) 1;

	/* assume that this register allows reads and writes: */
	lsinfo_ops
	  = (TME_SPARC_LSINFO_OP_LD
	     | TME_SPARC_LSINFO_OP_ST);

	/* assume that a write to this register won't need a redispatch: */
	redispatch = FALSE;

	/* address 0x18 is the synchronous fault status register: */
	if (address_0_31 == 0x18) {
	  _value64 = &mmu->tme_stp103x_mmu_sfsr;
	}

	/* address 0x28 is the TSB register: */
	else if (address_0_31 == 0x28) {
	  _value64 = &mmu->tme_stp103x_mmu_tsb;
	  value_has_va = TRUE;
	}

	/* address 0x30 is the tag access register: */
	else if (address_0_31 == 0x30) {
	  _value64 = &mmu->tme_stp103x_mmu_tag_access;
	  value_has_va = TRUE;
	}

	/* if this is ASI_DMMU: */
	else if (mmu == &TME_STP103X(ic)->tme_stp103x_dmmu) {

	  /* address 0x8 is the primary context register: */
	  if (address_0_31 == 0x8) {
	    _value32 = &ic->tme_sparc_memory_context_primary;
	    value_mask = TME_STP103X_CONTEXT_MAX;
	    redispatch = TRUE;
	  }

	  /* address 0x10 is the secondary context register: */
	  else if (address_0_31 == 0x10) {
	    _value32 = &ic->tme_sparc_memory_context_secondary;
	    value_mask = TME_STP103X_CONTEXT_MAX;
	  }

	  /* address 0x20 is the synchronous fault address register: */
	  else if (address_0_31 == 0x20) {
	    _value64 = &TME_STP103X(ic)->tme_stp103x_dmmu_sfar;
	    lsinfo_ops = TME_SPARC_LSINFO_OP_LD;
	  }

	  /* address 0x38 is the VA Data Watchpoint register: */
	  else if (address_0_31 == 0x38) {
	    abort();
	  }

	  /* address 0x40 is the PA Data Watchpoint register: */
	  else if (address_0_31 == 0x38) {
	    abort();
	  }
	}

	/* if the register valid and supports this access: */
	if (__tme_predict_true((_value64 != NULL
				|| _value32 != NULL)
			       && (lsinfo & lsinfo_ops) != 0)) {

	  /* get the raw value to read or write: */
	  value
	    = (lsinfo & TME_SPARC_LSINFO_OP_ST
	       ? *ls->tme_sparc_ls_rd64
	       : _value64 != NULL
	       ? *_value64
	       : *_value32);

	  /* if this value has a virtual address: */
	  if (value_has_va) {

	    /* force the virtual address to be in range: */
	    value |= (0 - (TME_STP103X_VA_HOLE_START * 2));
	    value = (value ^ TME_STP103X_VA_HOLE_START) + TME_STP103X_VA_HOLE_START;
	  }

	  /* mask the value: */
	  value &= value_mask;

	  /* if this is a load: */
	  if (lsinfo & TME_SPARC_LSINFO_OP_LD) {

	    /* complete the load: */
	    *ls->tme_sparc_ls_rd64 = value;
	    ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
	  }

	  /* otherwise, this is a store: */
	  else {

	    /* complete the store: */
	    if (_value64 != NULL) {
	      *_value64 = value;
	    }
	    else {
	      *_value32 = value;
	    }

	    /* if this store needs a redispatch: */
	    if (redispatch) {
	      tme_bus_tlb_unbusy(&ic->tme_sparc_tlbs[ls->tme_sparc_ls_tlb_i].tme_sparc_tlb_bus_tlb);
	      tme_sparc_redispatch(ic);
	    }
	  }

	  /* complete the load or store: */
	  ls->tme_sparc_ls_size = 0;
	  return;
	}
      }
    }
  }

  /* this is an illegal access: */
  ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
}  

/* the ASI handler for:
   ASI_DMMU_TSB_8KB_PTR_REG, ASI_DMMU_TSB_64KB_PTR_REG,
   ASI_IMMU_TSB_8KB_PTR_REG, ASI_IMMU_TSB_64KB_PTR_REG,
   ASI_DMMU_TSB_DIRECT_PTR_REG: */
static void
_tme_stp103x_ls_asi_tsb_ptr(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint32_t asi_mask;
  struct tme_stp103x_mmu *mmu;
  tme_uint32_t pointer_0_31;
  tme_uint32_t size_64KB;
  tme_uint32_t tsb_0_31;
  tme_uint32_t tsb_size;

  /* if this is not a 64-bit load of address zero: */
  if (__tme_predict_false(ls->tme_sparc_ls_size != sizeof(tme_uint64_t)
			  || (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_LD) == 0
			  || ls->tme_sparc_ls_address64 != 0)) {

    /* this is an illegal access: */
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if this access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* get the ASI mask from the instruction: */
  asi_mask = ls->tme_sparc_ls_asi_mask;

  /* get the common MMU state: */
  mmu
    = (TME_STP103X_ASI_MMU_MASK_IS_DMMU(asi_mask)
       ? &TME_STP103X(ic)->tme_stp103x_dmmu
       : &TME_STP103X(ic)->tme_stp103x_immu);

  /* start the TSB pointer with the tag access register: */
  pointer_0_31 = mmu->tme_stp103x_mmu_tag_access;

  /* if this might be a 64KB page: */
  size_64KB = asi_mask & TME_SPARC_ASI_MASK_RAW(TME_STP103X_ASI_FLAG_TSB_64KB_PTR);
  if (size_64KB) {

    /* if this is ASI_DMMU_TSB_DIRECT_PTR_REG: */
    if (asi_mask & TME_SPARC_ASI_MASK_RAW(TME_STP103X_ASI_FLAG_TSB_8KB_PTR)) {

      /* this is a 64KB page if the last fast_data_access_protection
	 trap was for a 64KB page: */
      size_64KB = TME_STP103X(ic)->tme_stp103x_dmmu_direct_64KB;
    }

    /* if this is a 64KB page, shift the TSB pointer: */
    if (size_64KB) {
      pointer_0_31 /= (TME_STP103X_PAGE_SIZE_64KB / TME_STP103X_PAGE_SIZE_8KB);
    }
  }

  /* shift the tag access register in the TSB pointer down to index a
     16-byte TSB entry: */
  pointer_0_31 = (pointer_0_31 / (TME_STP103X_PAGE_SIZE_8KB / 16)) & (0 - (tme_uint32_t) 16);

  /* get bits 0..31 of the TSB register: */
  tsb_0_31 = mmu->tme_stp103x_mmu_tsb;

  /* get the size of (one half of) the TSB: */
  tsb_size = TME_STP103X_PAGE_SIZE_8KB;
  tsb_size <<= (tsb_0_31 & TME_STP103X_TSB_SIZE);

  /* finish the offset of the entry in (one half of) the TSB: */
  pointer_0_31 &= (tsb_size - 1);

  /* if this is a split TSB: */
  if (tsb_0_31 & TME_STP103X_TSB_SPLIT) {

    /* if this is a 64KB page, select the other half of the TSB: */
    if (size_64KB) {
      pointer_0_31 += tsb_size;
    }

    /* the TSB is actually two halves: */
    tsb_size *= 2;
  }

  /* finish bits 0..31 of the TSB pointer: */
  pointer_0_31 += (tsb_0_31 & (0 - tsb_size));

  /* complete the load: */
  *ls->tme_sparc_ls_rd64
    = ((mmu->tme_stp103x_mmu_tsb
	& (0 - (((tme_uint64_t) 1) << 32)))
       | pointer_0_31);
  ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
  ls->tme_sparc_ls_size = 0;
}

/* the ASI handler for ASI_ITLB_DATA_IN_REG, ASI_DTLB_DATA_IN_REG: */
static void
_tme_stp103x_ls_asi_tlb_data_in(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  signed long tlb_part_i;
  signed long tlb_part_i_invalid;
  signed long tlb_part_i_unlocked;
  signed long tlb_part_i_unlocked_unused;
  tme_uint32_t tlb_data_32_63;

  /* if this is not a 64-bit store of address zero: */
  if (__tme_predict_false(ls->tme_sparc_ls_size != sizeof(tme_uint64_t)
			  || (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ST) == 0
			  || ls->tme_sparc_ls_address64 != 0)) {

    /* this is an illegal access: */
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if this access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* if this is ASI_DTLB_DATA_IN_REG, start at the last entry of the
     DTLB, otherwise start at the last entry of the ITLB: */
  tlb_part_i
    = (TME_STP103X_ASI_MMU_MASK_IS_DMMU(ls->tme_sparc_ls_asi_mask)
       ? TME_STP103X_TLB_PART_0_DMMU + (TME_STP103X_TLB_SIZE * 2) - 2
       : TME_STP103X_TLB_PART_0_IMMU + (TME_STP103X_TLB_SIZE * 2) - 2);

  /* search for invalid, unlocked, and unlocked+unused TLB entries: */
  tlb_part_i_invalid = -1;
  tlb_part_i_unlocked = -1;
  tlb_part_i_unlocked_unused = -1;
  for (;;) {

    /* load bits 32..63 of the TLB entry's data: */
    tlb_data_32_63 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1);

    /* if this TLB entry is invalid: */
    if ((tlb_data_32_63 & TME_STP103X_TLB_DATA_V(tlb_data_32_63)) == 0) {

      /* track the lowest-numbered invalid TLB entry: */
      tlb_part_i_invalid = tlb_part_i;
    }

    /* if this TLB entry is not locked: */
    if ((TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 0)
	 & TME_STP103X_TLB_DATA_L((tme_uint32_t) 0)) == 0) {

      /* track the lowest-numbered unlocked TLB entry: */
      tlb_part_i_unlocked = tlb_part_i;

      /* if the TLB entry's used bit is clear: */
      if ((tlb_data_32_63 & TME_STP103X_TLB_DATA_DIAG_USED(tlb_data_32_63)) == 0) {

	/* track the lowest-numbered unlocked and unused TLB entry: */
	tlb_part_i_unlocked_unused = tlb_part_i;
      }
    }

    /* if we have not exhausted the TLB: */
    if (tlb_part_i % (TME_STP103X_TLB_SIZE * 2)) {
      tlb_part_i -= 2;
      continue;
    }

    /* if there is an invalid TLB entry: */
    if (tlb_part_i_invalid >= 0) {
      tlb_part_i = tlb_part_i_invalid;
      break;
    }

    /* otherwise, if there is an unlocked and unused TLB entry: */
    if (tlb_part_i_unlocked_unused >= 0) {
      tlb_part_i = tlb_part_i_unlocked_unused;
    }

    /* otherwise, there is no invalid TLB entry and no unlocked and
       unused TLB entry: */
    else {

      /* clear the used bits on all TLB entries: */
      do {
	TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1)
	  &= ~TME_STP103X_TLB_DATA_DIAG_USED(tlb_data_32_63);
	tlb_part_i += 2;
      } while (tlb_part_i % (TME_STP103X_TLB_SIZE * 2));

      /* there must be an unlocked TLB entry: */
      assert (tlb_part_i_unlocked >= 0);
      tlb_part_i = tlb_part_i_unlocked;
    }

    /* invalidate this TLB entry: */
    _tme_stp103x_tlb_invalidate(ic, tlb_part_i);
    break;
  }

  /* complete the store: */
#if TME_STP103X_TLB_PART_0_DMMU >= TME_STP103X_TLB_PART_0_IMMU
#error "TME_STP103X_TLB_PART_0_DMMU or TME_STP103X_TLB_PART_0_IMMU changed"
#endif
  TME_STP103X(ic)->tme_stp103x_tlb_64s(tlb_part_i + 0)
    = (tlb_part_i < TME_STP103X_TLB_PART_0_IMMU
       ? &TME_STP103X(ic)->tme_stp103x_dmmu
       : &TME_STP103X(ic)->tme_stp103x_immu)->tme_stp103x_mmu_tag_access;
  TME_STP103X(ic)->tme_stp103x_tlb_64s(tlb_part_i + 1) = *ls->tme_sparc_ls_rd64;
  ls->tme_sparc_ls_size = 0;
}

/* ASI_ITLB_DATA_ACCESS_REG, ASI_DTLB_DATA_ACCESS_REG: */
static void
_tme_stp103x_ls_asi_tlb_data_access(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  unsigned long tlb_part_i;
  tme_uint64_t *_tag_access;

  /* XXX FIXME WRITEME table 6-11 hints that it's possible to do a
     casxa to DTLB_DATA_ACCESS_REG.  also see the WRITEME in
     _tme_stp103x_ls_trap(): */
  assert ((ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ATOMIC) == 0);

  /* if this is not a 64-bit load or store: */
  if (__tme_predict_false(ls->tme_sparc_ls_size != sizeof(tme_uint64_t)
			  || (ls->tme_sparc_ls_lsinfo
			      & (TME_SPARC_LSINFO_OP_LD
				 | TME_SPARC_LSINFO_OP_ST)) == 0)) {

    /* this is an illegal access: */
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if this access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* get the addressed TLB entry and common MMU state: */
  tlb_part_i = ls->tme_sparc_ls_address64;
  tlb_part_i %= TME_STP103X_TLB_SIZE * sizeof(tme_uint64_t);
  tlb_part_i /= (sizeof(tme_uint64_t) / 2);
  tlb_part_i += TME_STP103X_TLB_PART_0_DMMU;
  _tag_access = &TME_STP103X(ic)->tme_stp103x_dmmu.tme_stp103x_mmu_tag_access;
  if (!TME_STP103X_ASI_MMU_MASK_IS_DMMU(ls->tme_sparc_ls_asi_mask)) {
    tlb_part_i = (tlb_part_i - TME_STP103X_TLB_PART_0_DMMU) + TME_STP103X_TLB_PART_0_IMMU;
    _tag_access = &TME_STP103X(ic)->tme_stp103x_immu.tme_stp103x_mmu_tag_access;
  }

  /* if this is a load: */
  if (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_LD) {

    /* complete the load: */
    *ls->tme_sparc_ls_rd64 = TME_STP103X(ic)->tme_stp103x_tlb_64s(tlb_part_i + 1);
    ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
  }

  /* otherwise, this is a store: */
  else {

    /* if the TLB entry is valid: */
    if (TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1)
	& TME_STP103X_TLB_DATA_V((tme_uint32_t) 0)) {

      /* invalidate this TLB entry: */
      _tme_stp103x_tlb_invalidate(ic,
				  tlb_part_i);
    }

    /* complete the store: */
    TME_STP103X(ic)->tme_stp103x_tlb_64s(tlb_part_i + 0) = *_tag_access;
    TME_STP103X(ic)->tme_stp103x_tlb_64s(tlb_part_i + 1) = *ls->tme_sparc_ls_rd64;
  }

  /* we completed the load or store: */
  ls->tme_sparc_ls_size = 0;
}

/* the ASI handler for ASI_ITLB_TAG_READ_REG, ASI_DTLB_TAG_READ_REG: */
static void
_tme_stp103x_ls_asi_tlb_tag_read(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  unsigned long tlb_part_i;

  /* if this is not a 64-bit load: */
  if (__tme_predict_false(ls->tme_sparc_ls_size != sizeof(tme_uint64_t)
			  || (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_LD) == 0)) {

    /* this is an illegal access: */
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if this access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* get the addressed TLB entry and common MMU state: */
  tlb_part_i = ls->tme_sparc_ls_address64;
  tlb_part_i %= TME_STP103X_TLB_SIZE * sizeof(tme_uint64_t);
  tlb_part_i /= (sizeof(tme_uint64_t) / 2);
  tlb_part_i += TME_STP103X_TLB_PART_0_DMMU;
  if (!TME_STP103X_ASI_MMU_MASK_IS_DMMU(ls->tme_sparc_ls_asi_mask)) {
    tlb_part_i = (tlb_part_i - TME_STP103X_TLB_PART_0_DMMU) + TME_STP103X_TLB_PART_0_IMMU;
  }

  /* complete the load: */
  *ls->tme_sparc_ls_rd64 = TME_STP103X(ic)->tme_stp103x_tlb_64s(tlb_part_i + 0);
  ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
  ls->tme_sparc_ls_size = 0;
}

/* the ASI handler for ASI_IMMU_DEMAP, ASI_DMMU_DEMAP: */
static void
_tme_stp103x_ls_asi_mmu_demap(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint64_t address;
  tme_uint32_t tlb_tag_match_32_63;
  tme_uint32_t tlb_tag_match_0_31;
  tme_uint32_t context;
  tme_uint32_t tlb_tag_mask32;
  unsigned long tlb_part_i;
  tme_uint32_t tlb_tag_xor_32_63;
  tme_uint32_t tlb_data_0_31;
  tme_uint32_t tlb_tag_xor_0_31;
  tme_uint32_t tlb_data_32_63;
  tme_uint32_t size;

  /* if this is not a 64-bit store: */
  if (__tme_predict_false(ls->tme_sparc_ls_size != sizeof(tme_uint64_t)
			  || (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ST) == 0)) {

    /* this is an illegal access: */
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* check the address: */
  address = _tme_stp103x_ls_address_check(NULL, ls);
  tlb_tag_match_32_63 = address >> 32;
  tlb_tag_match_0_31 = address;

  /* if this access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* we will complete this store: */
  ls->tme_sparc_ls_size = 0;

  /* assume that this demap uses the primary context: */
  context = ic->tme_sparc_memory_context_primary;

  /* if this demap might use the secondary context: */
  if (tlb_tag_match_0_31 & TME_BIT(4)) {
    context = ic->tme_sparc_memory_context_secondary;
  }

  /* if this demap uses the nucleus context: */
  if (tlb_tag_match_0_31 & TME_BIT(5)) {
    context = 0;

    /* "Use of the reserved value causes the demap to be ignored" */
    if (tlb_tag_match_0_31 & TME_BIT(4)) {
      return;
    }
  }

  /* if this is a demap page, we must match the VA part of a tag.  if
     this is a demap context, we must ignore the VA part of a tag: */
  tlb_tag_mask32 = 0 - (tme_uint32_t) ((tlb_tag_match_0_31 & TME_BIT(6)) == 0);

  /* finish the tag to match: */
  tlb_tag_match_0_31 &= ~TME_STP103X_CONTEXT_MAX;
  tlb_tag_match_0_31 += context;

  /* if this is ASI_DMMU_DEMAP, start at the first entry of the DTLB,
     otherwise start at the first entry of the ITLB: */
  tlb_part_i
    = (TME_STP103X_ASI_MMU_MASK_IS_DMMU(ls->tme_sparc_ls_asi_mask)
       ? TME_STP103X_TLB_PART_0_DMMU
       : TME_STP103X_TLB_PART_0_IMMU);

  /* loop over the TLB entries: */
  do {

    /* if bits 32..63 of the tag match: */
    tlb_tag_xor_32_63 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 0), 1) ^ tlb_tag_match_32_63;
    if ((tlb_tag_xor_32_63 & tlb_tag_mask32) == 0) {

      /* load bits 0..31 of the data: */
      tlb_data_0_31 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 0);

      /* exclusive-OR bits 0..31 of the tag with bits 0..31 of the tag
	 to match: */
      tlb_tag_xor_0_31 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 0), 0) ^ tlb_tag_match_0_31;

      /* if this is a global entry: */
      if (tlb_data_0_31 & TME_STP103X_TLB_DATA_G(tlb_data_0_31)) {

	/* assume that this is a demap page, which can match a global
	   page, and force a match of the context field of the tag: */
	tlb_tag_xor_0_31 &= ~TME_STP103X_CONTEXT_MAX;

	/* if this a demap context: */
	if (tlb_tag_mask32 == 0) {

	  /* a demap context never matches a global page.  force a
	     mismatch of the context field of the tag: */
	  tlb_tag_xor_0_31 += 1;
	}
      }

      /* load bits 32..63 of the data: */
      tlb_data_32_63 = TME_STP103X(ic)->tme_stp103x_tlb_32s((tlb_part_i + 1), 1);

      /* if this TLB entry is valid: */
      if (tlb_data_32_63 & TME_STP103X_TLB_DATA_V(tlb_data_32_63)) {

	/* get the size of this mapping: */
	size
	  = (TME_STP103X_PAGE_SIZE_8KB
	     << (3 * TME_FIELD_MASK_EXTRACTU(tlb_data_32_63,
					     TME_STP103X_TLB_DATA_SIZE_MASK(tlb_data_32_63))));

	/* if bits 0..31 of the tag match: */
	if ((tlb_tag_xor_0_31
	     & (((0 - size)
		 & tlb_tag_mask32)
		+ TME_STP103X_CONTEXT_MAX)) == 0) {

	  /* invalidate this TLB entry: */
	  _tme_stp103x_tlb_invalidate(ic, tlb_part_i);
	}
      }
    }

    tlb_part_i += 2;
  } while (tlb_part_i % (2 * TME_STP103X_TLB_SIZE));
}

/* the ASI handler for ASI_ECACHE_W and ASI_ECACHE_R: */
static void
_tme_stp103x_ls_asi_ecache(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint64_t address;
  tme_uint32_t address_0_31;
  tme_uint32_t address_32_63;
  unsigned int ecache_what;
  int is_write;

  /* get the address: */
  address = ls->tme_sparc_ls_address64;
  address_32_63 = (address >> 32);
  address_0_31 = address;
  
  /* see if this is an E-Cache data access, or a tag/state/parity
     access: */
  ecache_what = (address_32_63 >> (39 - 32)) & 0x3;

  /* truncate the E-Cache address to the cache size, and 64-bit align
     it: */
  address_0_31 &= (TME_STP103X_ECACHE_SIZE - sizeof(tme_uint64_t));

  /* see if this is ASI_ECACHE_W or ASI_ECACHE_R: */
  is_write = (TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask) == 0x76);
  assert (is_write || TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask) == 0x7e);

  /* check the access: */
  if (__tme_predict_false((ls->tme_sparc_ls_lsinfo
			   & (is_write
			      ? TME_SPARC_LSINFO_OP_ST
			      : TME_SPARC_LSINFO_OP_LD)) == 0
			  || ls->tme_sparc_ls_size != sizeof(tme_uint64_t)
			  || (ecache_what != 0x1
			      && ecache_what != 0x2))) {
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if the access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* if this is a data access: */
  if (ecache_what == 0x1) {

    /* the PROM probes the size of the E-Cache by writing power-of-two
       sizes to those same addresses in the E-Cache, from high sizes
       to low sizes.  then it reads address zero to see the last size
       that got truncated to address zero.  we don't emulate the
       E-Cache, but we do emulate a single line at address zero, so we
       appear to the probe as having the smallest E-Cache size
       (512KB): */
#if TME_STP103X_ECACHE_SIZE != (512 * 1024)
#error "TME_STP103X_ECACHE_SIZE changed"
#endif
    if (address_0_31 == 0) {
      if (is_write) {
	TME_STP103X(ic)->tme_stp103x_ecache_data_probe = *ls->tme_sparc_ls_rd64;
      }
      else {
	*ls->tme_sparc_ls_rd64 = TME_STP103X(ic)->tme_stp103x_ecache_data_probe;
      }
    }
    else {
      abort();
    }
  }

  /* otherwise, this must be a tag access: */
  else {
    assert (ecache_what == 0x2);

    /* the PROM initializes all tags in the E-Cache.  we don't emulate
       the E-Cache, but we do support initializing any tag: */
    if (is_write
	&& ((TME_STP103X(ic)->tme_stp103x_ecache_tag_data % (2 << 28))
	    == (0x00000 /* a EC_tag of zero */
		+ (0x0 << 22) /* an EC_state of Invalid */
		+ (0xf << 25)))) { /* correct odd EC_parity */
      /* nothing to do */
    }
    else {
      abort();
    }
  }

  /* complete the load or store: */
  if (!is_write) {
    ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
  }
  ls->tme_sparc_ls_size = 0;
}

/* the ASI handler for ASI_DCACHE_DATA and ASI_DCACHE_TAG: */
static void
_tme_stp103x_ls_asi_dcache(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint32_t address_0_31;
  tme_uint64_t value;

  /* get the address: */
  address_0_31 = ls->tme_sparc_ls_address64 % (16 * 1024);

  /* check the access: */
  if (__tme_predict_false((ls->tme_sparc_ls_lsinfo
			   & (TME_SPARC_LSINFO_OP_ST
			      | TME_SPARC_LSINFO_OP_LD)) == 0
			  || ls->tme_sparc_ls_size != sizeof(tme_uint64_t))) {
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if the access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* if this is a store: */
  if (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ST) {
    
    /* get the value being stored: */
    value = *ls->tme_sparc_ls_rd64;

    /* we support writing zeros to tags - the PROM does this to
       initialize all tags in the D-cache, and kernels do this to
       flush the D-cache: */
    if (__tme_predict_true(TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask) == TME_STP103X_ASI_DCACHE_TAG
			   && value == 0)) {

      /* complete this store: */
      ls->tme_sparc_ls_size = 0;
      return;
    }

    /* soon after POR, the PROM writes 0xdeadbeef to address zero in
       both ASI_DCACHE_DATA and ASI_DCACHE_TAG.  we support these
       writes: */
    if (address_0_31 == 0
	&& value == 0xdeadbeef) {

      /* complete this store: */
      ls->tme_sparc_ls_size = 0;
      return;
    }
  }

  /* otherwise, this is a load: */
  else {

    /* we support reading tags, which always read as zeroes.  kernels
       may read tags when flushing the D-cache: */
    if (__tme_predict_true(TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask) == TME_STP103X_ASI_DCACHE_TAG)) {

      /* complete this load: */
      *ls->tme_sparc_ls_rd64 = 0;
      ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
      ls->tme_sparc_ls_size = 0;
      return;
    }
  }

  /* XXX FIXME WRITEME: */
  abort();
}

/* the ASI handler for ASI_ICACHE_INSTR, ASI_ICACHE_TAG,
   ASI_ICACHE_PRE_DECODE, ASI_ICACHE_NEXT_FIELD: */
static void
_tme_stp103x_ls_asi_icache(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint32_t address_0_31;
  tme_uint64_t value;

  /* get the address: */
  address_0_31 = ls->tme_sparc_ls_address64 % (16 * 1024);

  /* check the access: */
  if (__tme_predict_false((ls->tme_sparc_ls_lsinfo
			   & (TME_SPARC_LSINFO_OP_ST
			      | TME_SPARC_LSINFO_OP_LD)) == 0
			  || ls->tme_sparc_ls_size != sizeof(tme_uint64_t))) {
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if the access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* if this is a store: */
  if (ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_OP_ST) {
    
    /* get the value being stored: */
    value = *ls->tme_sparc_ls_rd64;

    /* soon after POR, the PROM writes 0xdeadbeef to address zero in
       both ASI_ICACHE_DATA and ASI_ICACHE_TAG.  later, the PROM
       initializes all tags in the I-cache.  we don't emulate the
       I-cache, but we do support these writes: */
    if ((address_0_31 == 0
	 && value == 0xdeadbeef)
	|| (TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask) == 0x67
	    && value == 0)) {

      /* complete this store: */
      ls->tme_sparc_ls_size = 0;
      return;
    }
  }

  /* XXX FIXME WRITEME: */
  abort();
}

/* this swaps double-precision floating-point register values in the
   memory buffer for the block transfer ASIs: */
static void
_tme_stp103x_block_buffer_bswap(struct tme_sparc *ic,
				const struct tme_sparc_ls *ls)
{
  const struct tme_sparc_tlb *tlb;
  tme_uint32_t endian_little;
  signed int value_i;

  /* get the TLB entry: */
  tlb = ls->tme_sparc_ls_tlb;

  /* get the byte order of the memory: */
  endian_little = ls->tme_sparc_ls_lsinfo & TME_SPARC_LSINFO_ENDIAN_LITTLE;

  /* if the host and memory byte orders don't match: */
  if (TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE
      ? !endian_little
      : TME_ENDIAN_NATIVE == TME_ENDIAN_BIG
      ? endian_little
      : TRUE) {

    /* if the host is big- or little-endian: */
    if (TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE
	|| TME_ENDIAN_NATIVE == TME_ENDIAN_BIG) {

      /* swap the values in the memory buffer: */
      value_i = TME_STP103X_BLOCK_FPREGS_DOUBLE - 1;
      do {
	ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer64s[value_i]
	  = tme_bswap_u64(ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer64s[value_i]);
      } while (--value_i >= 0);
    }

    /* otherwise, the host has an unusual byte order: */
    else {
      abort();
    }
  }
}

/* the cycle handler for loads with ASI_BLOCK_AS_IF_USER*,
   ASI_BLK_COMMIT*, and ASI_BLOCK*: */
static void
_tme_stp103x_ls_cycle_block_ld(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  unsigned int fpreg_number;

  /* do a load cycle: */
  tme_sparc64_load(ic, ls);

  /* if this was not the last cycle, return now: */
  if (ls->tme_sparc_ls_size != 0) {
    return;
  }

  /* swap the memory buffer: */
  _tme_stp103x_block_buffer_bswap(ic, ls);

  /* save the block load for verification: */
  tme_sparc_recode_verify_mem_block(ic, TME_SPARC_RECODE_VERIFY_MEM_LOAD);

  /* decode rd: */
  fpreg_number
    = tme_sparc_fpu_fpreg_decode(ic,
				 TME_FIELD_MASK_EXTRACTU(ic->_tme_sparc_insn,
							 TME_SPARC_FORMAT3_MASK_RD),
				 TME_IEEE754_FPREG_FORMAT_DOUBLE);

  /* loop over a block's worth of double-precision floating-point
     registers: */
  do {

    /* make sure the floating-point register is double-precision: */
    tme_sparc_fpu_fpreg_format(ic,
			       fpreg_number,
			       (TME_IEEE754_FPREG_FORMAT_DOUBLE
				| TME_IEEE754_FPREG_FORMAT_BUILTIN));

    /* copy the double-precision value from the memory buffer: */
    ic->tme_sparc_fpu_fpregs[fpreg_number].tme_float_format = TME_FLOAT_FORMAT_IEEE754_DOUBLE;
    ic->tme_sparc_fpu_fpregs[fpreg_number].tme_float_value_ieee754_double.tme_value64_uint
      = (ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer64s
	 [(fpreg_number / 2)
	  % TME_STP103X_BLOCK_FPREGS_DOUBLE]);

    /* NB: tme_sparc64_lddfa() will eventually use
       TME_SPARC_FPU_DIRTY() to mark the right half of the FPU
       dirty: */

    /* log the value loaded, except for the first, which will be
       logged eventually by tme_sparc64_ldxa(): */
    if (((fpreg_number / 2) % TME_STP103X_BLOCK_FPREGS_DOUBLE) != 0) {
      tme_sparc_log(ic, 1000, TME_OK,
		    (TME_SPARC_LOG_HANDLE(ic),
		     _("ldxa 0x%02x:0x%016" TME_PRIx64 ":    0x%016" TME_PRIx64),
		     TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask),
		     ((ls->tme_sparc_ls_address64
		       - TME_STP103X_BLOCK_SIZE)
		      + (sizeof(tme_uint64_t)
			 * ((fpreg_number / 2)
			    % TME_STP103X_BLOCK_FPREGS_DOUBLE))),
		     ic->tme_sparc_fpu_fpregs[fpreg_number].tme_float_value_ieee754_double.tme_value64_uint));
    }

  } while ((fpreg_number += 2)
	   % (TME_STP103X_BLOCK_FPREGS_DOUBLE * 2));

  /* complete this load for the lddfa function: */
  assert (ls->tme_sparc_ls_rd64
	  == &ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX));
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_FPX)
    = ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer64s[0];
}

/* the cycle handler for stores with ASI_BLOCK_AS_IF_USER*,
   ASI_BLK_COMMIT*, and ASI_BLOCK*: */
static void
_tme_stp103x_ls_cycle_block_st(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  unsigned int fpreg_number;
  union tme_value64 value_double_buffer;

  /* this must be the first cycle: */
  assert (ls->tme_sparc_ls_buffer_offset == 0);

  /* decode rd: */
  fpreg_number
    = tme_sparc_fpu_fpreg_decode(ic,
				 TME_FIELD_MASK_EXTRACTU(ic->_tme_sparc_insn,
							 TME_SPARC_FORMAT3_MASK_RD),
				 TME_IEEE754_FPREG_FORMAT_DOUBLE);

  /* loop over a block's worth of double-precision floating-point
     registers: */
  do {

    /* make sure the floating-point register is double-precision: */
    tme_sparc_fpu_fpreg_format(ic,
			       fpreg_number,
			       (TME_IEEE754_FPREG_FORMAT_DOUBLE
				| TME_IEEE754_FPREG_FORMAT_BUILTIN));

    /* copy the double-precision value into the memory buffer: */
    ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer64s
      [(fpreg_number / 2)
       % TME_STP103X_BLOCK_FPREGS_DOUBLE]
      = (tme_ieee754_double_value_get(&ic->tme_sparc_fpu_fpregs[fpreg_number],
				      &value_double_buffer)
	 ->tme_value64_uint);

    /* log the value stored, except for the first, which was already
       logged by tme_sparc64_stxa(): */
    if (((fpreg_number / 2) % TME_STP103X_BLOCK_FPREGS_DOUBLE) != 0) {
      tme_sparc_log(ic, 1000, TME_OK,
		    (TME_SPARC_LOG_HANDLE(ic),
		     _("stxa 0x%02x:0x%016" TME_PRIx64 ":    0x%016" TME_PRIx64),
		     TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask),
		     (ls->tme_sparc_ls_address64
		      + (sizeof(tme_uint64_t)
			 * ((fpreg_number / 2)
			    % TME_STP103X_BLOCK_FPREGS_DOUBLE))),
		     ic->tme_sparc_memory_buffer.tme_sparc_memory_buffer64s
		     [(fpreg_number / 2)
		      % TME_STP103X_BLOCK_FPREGS_DOUBLE]));
    }

  } while ((fpreg_number += 2)
	   % (TME_STP103X_BLOCK_FPREGS_DOUBLE * 2));

  /* save the block store for verification: */
  tme_sparc_recode_verify_mem_block(ic, TME_SPARC_RECODE_VERIFY_MEM_STORE);

  /* swap the memory buffer: */
  _tme_stp103x_block_buffer_bswap(ic, ls);

  /* do any leftover store cycles directly: */
  ls->tme_sparc_ls_cycle = tme_sparc64_store;

  /* do a store cycle: */
  tme_sparc64_store(ic, ls);
}

/* the ASI handler for ASI_BLOCK_AS_IF_USER*, ASI_BLK_COMMIT*, and
   ASI_BLOCK*: */
static void
_tme_stp103x_ls_asi_block(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{  
  tme_uint32_t insn;

  /* NB: tme_sparc64_lddfa() or tme_sparc64_stdfa() has already done
     an TME_SPARC_INSN_FPU: */

  /* we need to do the complete transfer: */
  /* NB: even if this is an stdfa, the TME_SPARC_LSINFO_LD_COMPLETED
     won't cause any problems: */
  assert (sizeof(ic->tme_sparc_memory_buffer) >= TME_STP103X_BLOCK_SIZE);
  ls->tme_sparc_ls_size = TME_STP103X_BLOCK_SIZE;
  ls->tme_sparc_ls_buffer_offset = 0;
  ls->tme_sparc_ls_lsinfo
    |= (TME_SPARC_LSINFO_SLOW_CYCLES
	| TME_SPARC_LSINFO_LD_COMPLETED);

  /* an instruction other than lddfa or stdfa, or an lddfa with an
     ASI_BLK_COMMIT*, or with an rd that isn't a multiple of 16 is
     illegal: */
  insn = ic->_tme_sparc_insn;
  /* NB: we flip the stdfa op3 bits, and check that the whole op3
     field becomes zero (for ASI_BLK_COMMIT*) or that all op3 bits
     except the one that differentiates stdfa from lddfa become zero
     (for all other ASIs): */
  /* NB: we only need to test bits 1, 2, and 3 in the double-precision
     encoded rd, since bit 0 is the encoded bit 5: */
  insn ^= (0x37 << 19);
  if (__tme_predict_false((insn
			   & ((((TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask)
				 ^ TME_STP103X_ASI_BLK_COMMIT)
				& ~TME_SPARC64_ASI_FLAG_SECONDARY)
			       ? (0x3b << 19)
			       : (0x3f << 19))
			      | TME_BIT(3 + 25)
			      | TME_BIT(2 + 25)
			      | TME_BIT(1 + 25))))) {
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* the address must be block-aligned: */
  else if (__tme_predict_false(((tme_uint32_t) ls->tme_sparc_ls_address64)
			       % TME_STP103X_BLOCK_SIZE)) {
    ls->tme_sparc_ls_faults |= TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;
  }

  /* set the cycle function: */
  /* NB: we flipped the stdfa op3 bits in insn above, so bit two of
     op3 is now set for an lddfa, and clear for an stdfa: */
  ls->tme_sparc_ls_cycle
    = ((insn & (4 << 19))
       ? _tme_stp103x_ls_cycle_block_ld
       : _tme_stp103x_ls_cycle_block_st);
}

/* the ASI handler for the UDB registers: */
static void
_tme_stp103x_ls_asi_udb(struct tme_sparc *ic, struct tme_sparc_ls *ls)
{
  tme_uint32_t address_0_31;
  int is_write;
  tme_uint16_t value16;
  unsigned int intr_reg;

  /* get the address: */
  address_0_31 = ls->tme_sparc_ls_address64;

  /* see if this is should be a write or a read: */
  is_write = (TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask) == 0x77);
  assert (is_write || TME_SPARC_ASI_MASK_WHICH(ls->tme_sparc_ls_asi_mask) == 0x7f);

  /* check the access type and size, and that the address fits in 32
     bits: */
  if (__tme_predict_false((ls->tme_sparc_ls_lsinfo
			   & (is_write
			      ? TME_SPARC_LSINFO_OP_ST
			      : TME_SPARC_LSINFO_OP_LD)) == 0
			  || ls->tme_sparc_ls_size != sizeof(tme_uint64_t)
			  || ls->tme_sparc_ls_address64 > (tme_uint32_t) (0 - (tme_uint32_t) 1))) {
    ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
  }

  /* if the access hasn't faulted yet: */
  if (__tme_predict_true(ls->tme_sparc_ls_faults == TME_SPARC_LS_FAULT_NONE)) {

    /* dispatch on the address: */
    switch (address_0_31) {

      /* the low and high UDB error registers: */
    case 0x00:
    case 0x18:
      /* we never generate ECC errors, so these registers
	 always read as zero, and writes are ignored: */
      if (!is_write) {
	*ls->tme_sparc_ls_rd64 = 0;
      }
      break;

      /* the low and high UDB control registers: */
    case 0x20:
    case 0x38:
      if (is_write) {
	value16 = *ls->tme_sparc_ls_rd64;
	if (value16 & TME_BIT(8)) { /* F_MODE */
	  abort();
	}
	TME_STP103X(ic)->tme_stp103x_udb_control[(address_0_31 & 8) == 0] = value16;
      }
      else {
	*ls->tme_sparc_ls_rd64
	  = ((0x00 << 9) /* UDB version number */
	     + (TME_STP103X(ic)->tme_stp103x_udb_control[(address_0_31 & 8) == 0]
		% (2 << 8))); /* F_MODE, FCBV */
      }
      break;

      /* the UDB transmit and receive interrupt vector data: */
    case 0x40:
    case 0x50:
    case 0x60:
      intr_reg = (address_0_31 - 0x40) / sizeof(tme_uint64_t);
      if (is_write) {
	TME_STP103X(ic)->tme_stp103x_udb_intr_transmit[intr_reg]
	  = *ls->tme_sparc_ls_rd64;
      }
      else {
	*ls->tme_sparc_ls_rd64
	  = TME_STP103X(ic)->tme_stp103x_udb_intr_receive[intr_reg];
      }
    break;

    default:

      /* if this isn't a write of the UDB interrupt vector dispatch: */
      if (!is_write
	  || (address_0_31 & ~((2 << 18) - (1 << 14))) != 0x70) {

	/* this is an illegal access: */
	ls->tme_sparc_ls_faults |= TME_STP103X_LS_FAULT_ILLEGAL;
	break;
      }

      abort();
      break;
    }
  }

  /* if the access has faulted: */
  if (__tme_predict_false(ls->tme_sparc_ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    return;
  }

  /* complete the load or store: */
  if (!is_write) {
    ls->tme_sparc_ls_lsinfo |= TME_SPARC_LSINFO_LD_COMPLETED;
  }
  ls->tme_sparc_ls_size = 0;
}

/* the ASI handlers: */
static const _tme_sparc_ls_asi_handler _tme_stp103x_ls_asi_handlers[] = {
  NULL,
  _tme_stp103x_ls_asi_mmu,
  _tme_stp103x_ls_asi_tsb_ptr,
  _tme_stp103x_ls_asi_quad,
  _tme_stp103x_ls_asi_tlb_data_in,
  _tme_stp103x_ls_asi_mmu_demap,
  _tme_stp103x_ls_asi_slow,
  _tme_stp103x_ls_asi_phys,
  _tme_stp103x_ls_asi_dcache,
  _tme_stp103x_ls_asi_ecache,
  _tme_stp103x_ls_asi_tlb_data_access,
  _tme_stp103x_ls_asi_tlb_tag_read,
  _tme_stp103x_ls_asi_icache,
  _tme_stp103x_ls_asi_block,
  _tme_stp103x_ls_asi_udb,
  tme_sparc64_vis_ls_asi_pst,
  tme_sparc64_vis_ls_asi_fl,
};

/* the tick compare register thread: */
static void
_tme_stp103x_tick_compare_th(void *_ic)
{
  struct tme_sparc *ic;
  struct timeval now;
  unsigned long now_tv_sec;
  unsigned long now_tv_usec;
  unsigned long tick_compare_time_tv_sec;
  unsigned long tick_compare_time_tv_usec;
  struct timeval sleep;

  /* recover our data structure: */
  ic = (struct tme_sparc *) _ic;

  /* lock the external mutex: */
  tme_mutex_lock(&ic->tme_sparc_external_mutex);

  /* loop forever: */
  for (;;) {

    /* get the current time: */
    tme_gettimeofday(&now);

    /* if the current time is greater than or equal to the tick compare time: */
    now_tv_sec = now.tv_sec;
    now_tv_usec = now.tv_usec;
    tick_compare_time_tv_sec = TME_STP103X(ic)->tme_stp103x_tick_compare_time.tv_sec;
    tick_compare_time_tv_usec = TME_STP103X(ic)->tme_stp103x_tick_compare_time.tv_usec;
    if (now_tv_sec > tick_compare_time_tv_sec
	|| (now_tv_sec == tick_compare_time_tv_sec
	    && now_tv_usec >= tick_compare_time_tv_usec)) {

      /* set the tick interrupt atomic flag: */
      tme_memory_atomic_write_flag(&TME_STP103X(ic)->tme_stp103x_sir_tick_int, TRUE);

      /* set the external flag: */
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_flag, TRUE);

      /* notify any thread waiting on the external condition: */
      tme_cond_notify(&ic->tme_sparc_external_cond, FALSE);

      /* wait on the tick compare condition: */
      tme_cond_wait_yield(&TME_STP103X(ic)->tme_stp103x_tick_compare_cond,
			  &ic->tme_sparc_external_mutex);
    }

    /* otherwise, the current time is less than the tick compare
       time: */
    else {
      
      /* make the sleep time, but don't sleep more than a minute at a time: */
      if (tick_compare_time_tv_usec < now_tv_usec) {
	tick_compare_time_tv_sec--;
	tick_compare_time_tv_usec += 1000000;
      }
      sleep.tv_sec = TME_MIN(tick_compare_time_tv_sec - now_tv_sec, 60);
      sleep.tv_usec = tick_compare_time_tv_usec - now_tv_usec;

      /* sleep on the tick compare condition: */
      tme_cond_sleep_yield(&TME_STP103X(ic)->tme_stp103x_tick_compare_cond,
			   &ic->tme_sparc_external_mutex,
			   &sleep);
    }
  }
  /* NOTREACHED */
}

/* this checks for external signals: */
/* NB: this may do a preinstruction trap: */
static void
_tme_stp103x_external_check(struct tme_sparc *ic,
			    int flags)
{

  /* if RESET_L has been negated since the last check: */
  if (__tme_predict_false(tme_memory_atomic_read_flag(&ic->tme_sparc_external_reset_negated))) {

    /* clear the XIR and RESET_L asserted flags, then clear the
       RESET_L negated flag: */
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_halt_asserted, FALSE);
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_reset_asserted, FALSE);
    tme_memory_barrier(ic, sizeof(*ic), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_reset_negated, FALSE);

    /* start POR trap processing: */
    if (flags & TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED) {
      tme_mutex_unlock(&ic->tme_sparc_external_mutex);
    }
    tme_sparc64_trap_preinstruction(ic, TME_SPARC64_TRAP_power_on_reset);
  }

  /* if RESET_L is asserted: */
  if (__tme_predict_false(tme_memory_atomic_read_flag(&ic->tme_sparc_external_reset_asserted))) {

    /* halt: */
    if (flags & TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED) {
      tme_mutex_unlock(&ic->tme_sparc_external_mutex);
    }
    ic->_tme_sparc_mode = TME_SPARC_MODE_HALT;
    tme_sparc_redispatch(ic);
  }

  /* if XIR has been asserted since the last check: */
  if (__tme_predict_false(tme_memory_atomic_read_flag(&ic->tme_sparc_external_halt_asserted))) {

    /* clear the XIR asserted flag: */
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_halt_asserted, FALSE);

    /* start XIR trap processing: */
    if (flags & TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED) {
      tme_mutex_unlock(&ic->tme_sparc_external_mutex);
    }
    tme_sparc64_trap_preinstruction(ic, TME_SPARC64_TRAP_externally_initiated_reset);
  }

  /* do an interrupt check: */
  _tme_stp103x_interrupt_check(ic, flags);
}

/* the bus cycle function: */
static void
_tme_stp103x_ls_bus_cycle(const struct tme_sparc *ic,
			  struct tme_sparc_ls *ls)
{
  tme_uint32_t asi_mask;
  unsigned int cycle_size_log2;

  /* NB: we provide the old sparc32 bus routing information when
     emulating stp103x noncached read and write transactions on the
     UPA bus.

     this is a bad hack that we do only to save duplicating that same
     information in the stp2220 emulation, which needs it for cycles
     that pass through it onto its SBus.

     since the old sparc32 bus routing information doesn't have
     anything to do with how the UPA bus works, we assume that all TLB
     fills for noncacheable accesses that don't allow fast transfers
     will either end up at another CPU (which will know about the
     disagreement and ignore the bus routing information), or end up
     at an I/O bridge, which will either rely on the sparc32 bus
     routing information (stp2220), or replace it with its own bus
     routing information (stp2222), before running the cycle on its
     I/O bus.  if the cycle is for an I/O bridge's internal registers,
     like a CPU it will know about the disagreement and ignore the bus
     routing information.

     we assume that all TLB fills for cacheable accesses target main
     memory, which we assume will tolerate any bus routing: */

  /* get the ASI mask for this TLB entry: */
  asi_mask = ls->tme_sparc_ls_tlb->tme_sparc_tlb_asi_mask;

  /* if the TLB entry is for noncacheable accesses: */
  if (asi_mask & TME_SPARC64_ASI_MASK_FLAG_TLB_UNCACHEABLE) {

    /* call the default sparc32 bus cycle function: */
    tme_sparc32_ls_bus_cycle(ic, ls);
    return;
  }

  /* provide a simple bus routing for cacheable accesses: */
  cycle_size_log2 = TME_BUS8_LOG2;
  for (; (1 << cycle_size_log2) != ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size; ) {
    assert (cycle_size_log2 < TME_BUS128_LOG2);
    cycle_size_log2++;
  }
  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS128_LOG2);
  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_lane_routing
    = (&(_tme_stp103x_bus_router_cacheable
	 [cycle_size_log2]
	 [0])
       - TME_BUS_ROUTER_INDEX(TME_BUS128_LOG2, TME_BUS128_LOG2, 0));
}

/* this fills a TLB for the CPU: */
static int
_tme_stp103x_tlb_fill(struct tme_bus_connection *conn_bus,
		      struct tme_bus_tlb *tlb,
		      tme_bus_addr_t address,
		      unsigned int cycle_type)
{
  abort();
}

/* this handles an interrupt: */
static void
_tme_stp103x_interrupt(struct tme_upa_bus_connection *conn_upa,
		       tme_uint32_t master_mid,
		       const tme_uint64_t *data,
		       struct tme_completion *completion)
{
  struct tme_sparc *ic;

  /* recover our data structure: */
  ic = conn_upa->tme_upa_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* if the receive interrupt vector data is already busy: */
  if (tme_memory_atomic_read_flag(&TME_STP103X(ic)->tme_stp103x_intr_receive_busy)) {

    /* NACK this interrupt: */
    completion->tme_completion_error = EAGAIN;
    tme_memory_barrier(completion, sizeof(*completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  }

  /* otherwise, the incoming data isn't busy: */
  else {

    /* save the interrupt data and ACK this interrupt: */
    /* NB: the interrupt data is an array of eight big-endian
       tme_uint64_t, since an interrupt packet is four 128-bit values.
       the even indices are the least-significant halves of the
       128-bit values: */
    TME_STP103X(ic)->tme_stp103x_intr_receive_mid = master_mid;
    TME_STP103X(ic)->tme_stp103x_udb_intr_receive[0] = tme_betoh_u64(data[2 * 0]);
    TME_STP103X(ic)->tme_stp103x_udb_intr_receive[1] = tme_betoh_u64(data[2 * 1]);
    TME_STP103X(ic)->tme_stp103x_udb_intr_receive[2] = tme_betoh_u64(data[2 * 2]);
    completion->tme_completion_error = TME_OK;
    tme_memory_barrier(0, 0, TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
    tme_memory_atomic_write_flag(&TME_STP103X(ic)->tme_stp103x_intr_receive_busy, 1);
    tme_memory_barrier(ic, sizeof(*ic), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_flag, 1);
    tme_cond_notify(&ic->tme_sparc_external_cond, FALSE);
  }

  /* validate this completion: */
  tme_completion_validate(completion);
}

/* this returns the version of an FPU for an stp103x: */
static tme_uint32_t
_tme_sparc_fpu_ver_stp103x(struct tme_sparc *ic, const char *fpu_name, char **_output)
{
  tme_uint32_t ver;

  /* if we're returning a usage: */
  if (_output != NULL) {
    tme_output_append_error(_output, 
			    "builtin");
    return (TME_SPARC_FSR_VER_missing);
  }

  if (TME_ARG_IS(fpu_name, "builtin")) {
    /* XXX FIXME - the stp1030 has an FSR_version of zero.  we assume
       that the stp1031 does too: */
    ver = 0;
  }
  else {
    return (TME_SPARC_FSR_VER_missing);
  }

  ic->tme_sparc_fpu_flags
    = (!TME_SPARC_FPU_FLAG_OK_REG_MISALIGNED);
  return (ver * _TME_FIELD_MASK_FACTOR(TME_SPARC_FSR_VER));
}

/* this creates and returns a new stp103x: */
static int
_tme_stp103x_new(struct tme_element *element,
		 const char * const *args,
		 const void *extra,
		 char **_output,
		 int is_1030)
{
  struct tme_sparc *ic;
  tme_uint32_t psr;
  tme_uint32_t asi;
  tme_uint32_t asi_mask_flags;
  void (*handler) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));
  tme_uint32_t handler_i;

  /* allocate the stp103x structure: */
  ic = &tme_new0(struct tme_stp103x, 1)->tme_stp103x_sparc;
  ic->tme_sparc_element = element;

  /* set the type: */
  is_1030 = !!is_1030;
  TME_STP103X(ic)->tme_stp103x_is_1030 = is_1030;
  if (TME_STP103X_IS_1030(ic) != is_1030) {
    tme_free(ic);
    return (ENXIO);
  }

  /* initialize the synchronization parts of the structure: */
  tme_sparc_sync_init(ic);

  /* initialize the stp103x private structure: */
  TME_STP103X(ic)->tme_stp103x_tcr = TME_STP103X_TCR_INT_DIS;
  tme_cond_init(&TME_STP103X(ic)->tme_stp103x_tick_compare_cond);
  tme_misc_timeval_never(&TME_STP103X(ic)->tme_stp103x_tick_compare_time);
  tme_memory_atomic_init_flag(&TME_STP103X(ic)->tme_stp103x_sir_tick_int, FALSE);
  tme_memory_atomic_init_flag(&TME_STP103X(ic)->tme_stp103x_intr_receive_busy, FALSE);

  /* start the tick compare thread: */
  tme_thread_create((tme_thread_t) _tme_stp103x_tick_compare_th, ic);

  /* fill in the stp103x-specific parts of the structure: */
  psr = 0;
  TME_FIELD_MASK_DEPOSITU(psr, TME_SPARC32_PSR_IMPL, 1);
  TME_FIELD_MASK_DEPOSITU(psr, TME_SPARC32_PSR_VER, 1);
  ic->tme_sparc32_ireg_psr = psr;
  ic->tme_sparc_version = TME_SPARC_VERSION(ic);
  ic->tme_sparc_nwindows = TME_SPARC_NWINDOWS(ic);
  ic->tme_sparc_memory_flags = TME_SPARC_MEMORY_FLAGS(ic);
  ic->tme_sparc64_maxtl = TME_STP103X_MAXTL;
  ic->tme_sparc_tlb_page_size_log2 = 13; /* log2(TME_STP103X_PAGE_SIZE_8KB) */
  ic->tme_sparc_memory_context_max = TME_STP103X_CONTEXT_MAX;
  ic->_tme_sparc64_execute_opmap = _TME_SPARC_EXECUTE_OPMAP;
  ic->tme_sparc64_rstvaddr = 0 - (tme_uint64_t) (1 << 28);
  ic->tme_sparc64_ireg_va_hole_start = TME_STP103X_VA_HOLE_START;
  ic->tme_sparc64_ireg_ver
    = ((0x0017 * _TME_FIELD_MASK_FACTOR(TME_SPARC64_VER_MANUF))
       + (0x0010 * _TME_FIELD_MASK_FACTOR(TME_SPARC64_VER_IMPL))
       + (0x40 * _TME_FIELD_MASK_FACTOR(TME_SPARC64_VER_MASK))
       + (ic->tme_sparc64_maxtl * _TME_FIELD_MASK_FACTOR(TME_SPARC64_VER_MAXTL))
       + ((TME_SPARC_NWINDOWS(ic) - 1) * _TME_FIELD_MASK_FACTOR(TME_SPARC64_VER_MAXWIN)));
  ic->tme_sparc64_ireg_winstates_mask
    = (TME_SPARC64_WINSTATES_CWP(TME_SPARC_NWINDOWS(ic) - 1)
       + TME_SPARC64_WINSTATES_CANRESTORE(TME_SPARC_NWINDOWS(ic) - 1)
       + TME_SPARC64_WINSTATES_CANSAVE(TME_SPARC_NWINDOWS(ic) - 1)
       + TME_SPARC64_WINSTATES_OTHERWIN(TME_SPARC_NWINDOWS(ic) - 1));
  ic->_tme_sparc_execute = _tme_sparc_execute_stp103x;
  ic->_tme_sparc_fpu_ver = _tme_sparc_fpu_ver_stp103x;
  ic->_tme_sparc_external_check = _tme_stp103x_external_check;
  ic->_tme_sparc_tlb_fill = _tme_stp103x_tlb_fill;
  ic->_tme_sparc_upa_interrupt = _tme_stp103x_interrupt;
  ic->_tme_sparc_ls_asi_misaligned = tme_sparc64_vis_ls_asi_misaligned;
  ic->_tme_sparc_ls_asi_handlers = _tme_stp103x_ls_asi_handlers;
  ic->_tme_sparc_ls_address_map = _tme_stp103x_ls_address_map;
  ic->_tme_sparc_ls_bus_cycle = _tme_stp103x_ls_bus_cycle;
  ic->_tme_sparc_ls_bus_fault = tme_sparc_ls_bus_fault;
  ic->_tme_sparc_ls_trap = _tme_stp103x_ls_trap;
  ic->_tme_sparc64_update_pstate = _tme_stp103x_update_pstate;
  ic->tme_sparc_vis_ls_fault_illegal = TME_STP103X_LS_FAULT_ILLEGAL;
  ic->tme_sparc_timing_loop_cycles_each = 1;
#ifdef _TME_SPARC_RECODE_VERIFY
  /* NB: struct tme_stp103x has been deliberately laid out to have all
     of the verifiable contents first, followed by everything that
     isn't verifiable (basically, anything that is accessed using
     loads and stores, since replay only simulates them): */
  ic->tme_sparc_recode_verify_ic_size
    = (((char *) &((struct tme_stp103x *) 0)->tme_stp103x_upa_config)
       - (char *) ((struct tme_stp103x *) 0));
  ic->tme_sparc_recode_verify_ic_size_total = sizeof(struct tme_stp103x);
#endif /* _TME_SPARC_RECODE_VERIFY */

  /* initialize the ASIs: */
  for (asi = 0; asi < TME_ARRAY_ELS(ic->tme_sparc_asis); asi++) {

    /* dispatch on this ASI: */
    switch (asi) {

      /* ASI_PHYS_USE_EC*, ASI_PHYS_BYPASS_EC_WITH_EBIT*: */
    case (0x14):
    case (0x14
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (0x15):
    case (0x15
	  + TME_SPARC64_ASI_FLAG_LITTLE):
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_phys;
      break;

      /* ASI_NUCLEUS_QUAD_LDD*: */
    case (0x24):
    case (0x24
	  + TME_SPARC64_ASI_FLAG_LITTLE):
      asi_mask_flags
	= (TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS
	   + (asi
	      & TME_SPARC64_ASI_FLAG_LITTLE));
      handler = _tme_stp103x_ls_asi_quad;
      break;

      /* ASI_DCACHE_DATA, ASI_DCACHE_TAG: */
    case TME_STP103X_ASI_DCACHE_DATA:
    case TME_STP103X_ASI_DCACHE_TAG:
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_dcache;
      break;

      /* ASI_ECACHE_W, ASI_ECACHE_R: */
    case 0x76:
    case 0x7e:
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_ecache;
      break;

      /* ASI_IMMU, ASI_DMMU: */
    case TME_STP103X_ASI_IMMU:
    case TME_STP103X_ASI_DMMU:
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_mmu;
      break;

      /* ASI_IMMU_TSB_8KB_PTR_REG, ASI_IMMU_TSB_64KB_PTR_REG,
	 ASI_DMMU_TSB_8KB_PTR_REG, ASI_DMMU_TSB_64KB_PTR_REG,
	 ASI_DMMU_TSB_DIRECT_PTR_REG: */
    case (TME_STP103X_ASI_IMMU
	  + TME_STP103X_ASI_FLAG_TSB_8KB_PTR):
    case (TME_STP103X_ASI_IMMU
	  + TME_STP103X_ASI_FLAG_TSB_64KB_PTR):
    case (TME_STP103X_ASI_DMMU
	  + TME_STP103X_ASI_FLAG_TSB_8KB_PTR):
    case (TME_STP103X_ASI_DMMU
	  + TME_STP103X_ASI_FLAG_TSB_64KB_PTR):
    case (TME_STP103X_ASI_DMMU
	  + (TME_STP103X_ASI_FLAG_TSB_8KB_PTR
	     | TME_STP103X_ASI_FLAG_TSB_64KB_PTR)):
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_tsb_ptr;
      break;

      /* ASI_ITLB_DATA_IN_REG, ASI_DTLB_DATA_IN_REG: */
    case (TME_STP103X_ASI_IMMU + 0x4):
    case (TME_STP103X_ASI_DMMU + 0x4):
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_tlb_data_in;
      break;

      /* ASI_ITLB_DATA_ACCESS_REG, ASI_DTLB_DATA_ACCESS_REG: */
    case (TME_STP103X_ASI_IMMU + 0x5):
    case (TME_STP103X_ASI_DMMU + 0x5):
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_tlb_data_access;
      break;

      /* ASI_ITLB_TAG_READ_REG, ASI_DTLB_TAG_READ_REG: */
    case (TME_STP103X_ASI_IMMU + 0x6):
    case (TME_STP103X_ASI_DMMU + 0x6):
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_tlb_tag_read;
      break;

      /* ASI_IMMU_DEMAP, ASI_DMMU_DEMAP: */
    case (TME_STP103X_ASI_IMMU + 0x7):
    case (TME_STP103X_ASI_DMMU + 0x7):
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_mmu_demap;
      break;

      /* ASI_ICACHE_INSTR: */
      /* ASI_ICACHE_TAG: */
      /* ASI_ICACHE_PRE_DECODE: */
      /* ASI_ICACHE_NEXT_FIELD: */
    case 0x66:
    case 0x67:
    case 0x6e:
    case 0x6f:
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_icache;
      break;

      /* ASI_BLOCK_AS_IF_USER*: */
    case (0x70):
    case (0x70
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (0x70
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (0x70
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
      asi_mask_flags
	= (TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER
	   + (asi
	      & (TME_SPARC64_ASI_FLAG_SECONDARY
		 | TME_SPARC64_ASI_FLAG_LITTLE)));
      handler = _tme_stp103x_ls_asi_block;
      break;

      /* ASI_UDB*: */
    case 0x77:
    case 0x7f:
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_udb;
      break;

      /* the mandatory v9 ASIs: */
#if TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS != 4
#error "TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS changed"
#endif
    case (TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS):
    case (TME_SPARC64_ASI_MASK_FLAG_INSN_NUCLEUS
	  + TME_SPARC64_ASI_FLAG_LITTLE):
#if TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER != 0x10
#error "TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER changed"
#endif
    case (TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER):
    case (TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC64_ASI_MASK_FLAG_INSN_AS_IF_USER
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED
	  + TME_SPARC64_ASI_FLAG_NO_FAULT):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_NO_FAULT):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED
	  + TME_SPARC64_ASI_FLAG_NO_FAULT
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC64_ASI_FLAG_UNRESTRICTED
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_NO_FAULT
	  + TME_SPARC64_ASI_FLAG_LITTLE):

      /* the mandatory v9 ASIs are all normal and have no handlers: */
      asi_mask_flags = asi;
      handler = NULL;
      assert ((asi_mask_flags & TME_SPARC64_ASI_MASK_FLAG_SPECIAL) == 0);
      assert (_tme_stp103x_ls_asi_handlers[0] == handler);
      break;

      /* ASI_PST*: */
    case (TME_SPARC_VIS_ASI_PST8):
    case (TME_SPARC_VIS_ASI_PST8
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (TME_SPARC_VIS_ASI_PST8
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_PST8
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_PST16):
    case (TME_SPARC_VIS_ASI_PST16
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (TME_SPARC_VIS_ASI_PST16
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_PST16
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_PST32):
    case (TME_SPARC_VIS_ASI_PST32
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (TME_SPARC_VIS_ASI_PST32
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_PST32
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
      asi_mask_flags
	= (asi
	   & (TME_SPARC64_ASI_FLAG_SECONDARY
	      | TME_SPARC64_ASI_FLAG_LITTLE));
      handler = tme_sparc64_vis_ls_asi_pst;
      break;

      /* ASI_FL*: */
    case (TME_SPARC_VIS_ASI_FL8):
    case (TME_SPARC_VIS_ASI_FL8
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (TME_SPARC_VIS_ASI_FL8
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_FL8
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_FL16):
    case (TME_SPARC_VIS_ASI_FL16
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (TME_SPARC_VIS_ASI_FL16
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (TME_SPARC_VIS_ASI_FL16
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
      asi_mask_flags
	= (asi
	   & (TME_SPARC64_ASI_FLAG_SECONDARY
	      | TME_SPARC64_ASI_FLAG_LITTLE));
      handler = tme_sparc64_vis_ls_asi_fl;
      break;

      /* ASI_BLK_COMMIT*: */
    case (TME_STP103X_ASI_BLK_COMMIT):
    case (TME_STP103X_ASI_BLK_COMMIT
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
      asi_mask_flags
	= (asi
	   & (TME_SPARC64_ASI_FLAG_SECONDARY));
      handler = _tme_stp103x_ls_asi_block;
      break;

      /* ASI_BLOCK*: */
    case (0xf0):
    case (0xf0
	  + TME_SPARC64_ASI_FLAG_SECONDARY):
    case (0xf0
	  + TME_SPARC64_ASI_FLAG_LITTLE):
    case (0xf0
	  + TME_SPARC64_ASI_FLAG_SECONDARY
	  + TME_SPARC64_ASI_FLAG_LITTLE):
      asi_mask_flags
	= (asi
	   & (TME_SPARC64_ASI_FLAG_SECONDARY
	      | TME_SPARC64_ASI_FLAG_LITTLE));
      handler = _tme_stp103x_ls_asi_block;
      break;

    case TME_STP103X_ASI_LSU_CONTROL_REG:
    case TME_STP103X_ASI_INTR_DISPATCH_STATUS:
    case TME_STP103X_ASI_INTR_RECEIVE:
    case TME_STP103X_ASI_UPA_CONFIG_REG:
    case TME_STP103X_ASI_ESTATE_ERROR_EN_REG:
    case TME_STP103X_ASI_AFSR:
    case TME_STP103X_ASI_AFAR:
    case TME_STP103X_ASI_ECACHE_TAG_DATA:
    default:
      asi_mask_flags = TME_SPARC64_ASI_MASK_FLAG_SPECIAL;
      handler = _tme_stp103x_ls_asi_slow;
      break;
    }

    /* get any ASI handler index: */
    for (handler_i = 0; _tme_stp103x_ls_asi_handlers[handler_i] != handler; handler_i++) {
      assert (handler_i < (TME_ARRAY_ELS(_tme_stp103x_ls_asi_handlers) - 1));
    }

    /* initialize this ASI: */
    ic->tme_sparc_asis[asi].tme_sparc_asi_mask_flags = asi_mask_flags;
    ic->tme_sparc_asis[asi].tme_sparc_asi_handler = handler_i;
  }

  /* call the common sparc new function: */
  return (tme_sparc_new(ic, args, extra, _output));
}

/* this creates and returns a new stp1030: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,sparc,stp1030) {
  return (_tme_stp103x_new(element, args, extra, _output, TRUE));
}

#undef  TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) _TME_SPARC_VERSION(ic)
#undef  TME_SPARC_NWINDOWS
#define TME_SPARC_NWINDOWS(ic) _TME_SPARC_NWINDOWS(ic)
#undef _TME_SPARC_EXECUTE_NAME
#undef _TME_SPARC_EXECUTE_OPMAP
