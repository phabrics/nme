/* $Id: sparc-impl.h,v 1.11 2010/06/05 16:13:15 fredette Exp $ */

/* ic/sparc/sparc-impl.h - implementation header file for SPARC emulation: */

/*
 * Copyright (c) 2005, 2007, 2009 Matt Fredette
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

#ifndef _IC_SPARC_IMPL_H
#define _IC_SPARC_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: sparc-impl.h,v 1.11 2010/06/05 16:13:15 fredette Exp $");

/* includes: */
#include <tme/ic/sparc.h>
#include <tme/ic/ieee754.h>
#include <tme/generic/ic.h>
#include <tme/bus/upa.h>
#if TME_HAVE_RECODE
#include <tme/recode.h>
#endif /* TME_HAVE_RECODE */
#include <tme/runlength.h>
#include <tme/misc.h>
#include <setjmp.h>

/* macros: */

/* generic registers: */
#define tme_sparc_ireg_uint64(x)	tme_sparc_ic.tme_ic_ireg_uint64(x)
#define tme_sparc_ireg_int64(x)		tme_sparc_ic.tme_ic_ireg_int64(x)
#define tme_sparc_ireg_uint32(x)	tme_sparc_ic.tme_ic_ireg_uint32(x)
#define tme_sparc_ireg_int32(x)		tme_sparc_ic.tme_ic_ireg_int32(x)
#define tme_sparc_ireg_uint8(x)		tme_sparc_ic.tme_ic_ireg_uint8(x)

/* format 3 instruction fields: */
#define TME_SPARC_FORMAT3_MASK_RS2	(0x1f <<  0)
#define TME_SPARC_FORMAT3_MASK_RS1	(0x1f << 14)
#define TME_SPARC_FORMAT3_MASK_RD	(0x1f << 25)

/* traps: */
#define TME_SPARC_TRAP_IMPDEP_RESET			(0x80000000)
#define _TME_SPARC_TRAP_IMPDEP(x)			TME_BIT(30 - (x))
#define _TME_SPARC_TRAP(priority, tt)			(((priority) * 0x1000) + (tt))
#define TME_SPARC_TRAP_PRIORITY(trap)			\
  (((trap) / _TME_SPARC_TRAP(1, 0)) % (TME_SPARC_TRAP_IMPDEP_RESET / _TME_SPARC_TRAP(1, 0)))
#define TME_SPARC_TRAP_TT(trap)				((trap) % _TME_SPARC_TRAP(1, 0))
#define TME_SPARC_TRAP_none				_TME_SPARC_TRAP(0xfff, 0)

/* sparc32 traps: */
#define TME_SPARC32_TRAP_reset				_TME_SPARC_TRAP( 1, 0x100)
#define TME_SPARC32_TRAP_instruction_access_MMU_miss	_TME_SPARC_TRAP( 2, 0x3C)
#define TME_SPARC32_TRAP_instruction_access_error	_TME_SPARC_TRAP( 3, 0x21)
#define TME_SPARC32_TRAP_r_register_access_error	_TME_SPARC_TRAP( 4, 0x20)
#define TME_SPARC32_TRAP_instruction_access_exception	_TME_SPARC_TRAP( 5, 0x01)
#define TME_SPARC32_TRAP_privileged_instruction		_TME_SPARC_TRAP( 6, 0x03)
#define TME_SPARC32_TRAP_illegal_instruction		_TME_SPARC_TRAP( 7, 0x02)
#define TME_SPARC32_TRAP_fp_disabled			_TME_SPARC_TRAP( 8, 0x04)
#define TME_SPARC32_TRAP_cp_disabled			_TME_SPARC_TRAP( 8, 0x24)
#define TME_SPARC32_TRAP_unimplemented_FLUSH		_TME_SPARC_TRAP( 8, 0x25)
#define TME_SPARC32_TRAP_watchpoint_detected		_TME_SPARC_TRAP( 8, 0x0B)
#define TME_SPARC32_TRAP_window_overflow		_TME_SPARC_TRAP( 9, 0x05)
#define TME_SPARC32_TRAP_window_underflow		_TME_SPARC_TRAP( 9, 0x06)
#define TME_SPARC32_TRAP_mem_address_not_aligned	_TME_SPARC_TRAP(10, 0x07)
#define TME_SPARC32_TRAP_fp_exception			_TME_SPARC_TRAP(11, 0x08)
#define TME_SPARC32_TRAP_cp_exception			_TME_SPARC_TRAP(11, 0x28)
#define TME_SPARC32_TRAP_data_access_error		_TME_SPARC_TRAP(12, 0x29)
#define TME_SPARC32_TRAP_data_access_MMU_miss		_TME_SPARC_TRAP(12, 0x2C)
#define TME_SPARC32_TRAP_data_access_exception		_TME_SPARC_TRAP(13, 0x09)
#define TME_SPARC32_TRAP_tag_overflow			_TME_SPARC_TRAP(14, 0x0A)
#define TME_SPARC32_TRAP_division_by_zero		_TME_SPARC_TRAP(15, 0x2A)
#define TME_SPARC32_TRAP_trap_instruction(x)		_TME_SPARC_TRAP(16, 0x80 + (x))
#define TME_SPARC32_TRAP_interrupt_level(il)		_TME_SPARC_TRAP(32 - (il), 0x10 + (il))

/* sparc64 traps: */
#define TME_SPARC64_TRAP_power_on_reset			_TME_SPARC_TRAP( 0, 0x001)
#define TME_SPARC64_TRAP_watchdog_reset			_TME_SPARC_TRAP( 1, 0x002)
#define TME_SPARC64_TRAP_externally_initiated_reset	_TME_SPARC_TRAP( 1, 0x003)
#define TME_SPARC64_TRAP_software_initiated_reset	_TME_SPARC_TRAP( 1, 0x004)
#define TME_SPARC64_TRAP_RED_state_exception		_TME_SPARC_TRAP( 1, 0x005)
#define TME_SPARC64_TRAP_instruction_access_exception	_TME_SPARC_TRAP( 5, 0x008)
#define TME_SPARC64_TRAP_instruction_access_MMU_miss	_TME_SPARC_TRAP( 2, 0x009)
#define TME_SPARC64_TRAP_instruction_access_error	_TME_SPARC_TRAP( 3, 0x00a)
#define TME_SPARC64_TRAP_illegal_instruction		_TME_SPARC_TRAP( 7, 0x010)
#define TME_SPARC64_TRAP_privileged_opcode		_TME_SPARC_TRAP( 6, 0x011)
#define TME_SPARC64_TRAP_unimplemented_LDD		_TME_SPARC_TRAP( 6, 0x012)
#define TME_SPARC64_TRAP_unimplemented_STD		_TME_SPARC_TRAP( 6, 0x013)
#define TME_SPARC64_TRAP_fp_disabled			_TME_SPARC_TRAP( 8, 0x020)
#define TME_SPARC64_TRAP_fp_exception_ieee_754		_TME_SPARC_TRAP(11, 0x021)
#define TME_SPARC64_TRAP_fp_exception_other		_TME_SPARC_TRAP(11, 0x022)
#define TME_SPARC64_TRAP_tag_overflow			_TME_SPARC_TRAP(14, 0x023)
#define TME_SPARC64_TRAP_clean_window			_TME_SPARC_TRAP(10, 0x024)
#define TME_SPARC64_TRAP_division_by_zero		_TME_SPARC_TRAP(15, 0x028)
#define TME_SPARC64_TRAP_internal_processor_error	_TME_SPARC_TRAP( 4, 0x029)
#define TME_SPARC64_TRAP_data_access_exception		_TME_SPARC_TRAP(12, 0x030)
#define TME_SPARC64_TRAP_data_access_MMU_miss		_TME_SPARC_TRAP(12, 0x031)
#define TME_SPARC64_TRAP_data_access_error		_TME_SPARC_TRAP(12, 0x032)
#define TME_SPARC64_TRAP_data_access_protection		_TME_SPARC_TRAP(12, 0x033)
#define TME_SPARC64_TRAP_mem_address_not_aligned	_TME_SPARC_TRAP(10, 0x034)
#define TME_SPARC64_TRAP_LDDF_mem_address_not_aligned	_TME_SPARC_TRAP(10, 0x035)
#define TME_SPARC64_TRAP_STDF_mem_address_not_aligned	_TME_SPARC_TRAP(10, 0x036)
#define TME_SPARC64_TRAP_privileged_action		_TME_SPARC_TRAP(11, 0x037)
#define TME_SPARC64_TRAP_LDQF_mem_address_not_aligned	_TME_SPARC_TRAP(10, 0x038)
#define TME_SPARC64_TRAP_STQF_mem_address_not_aligned	_TME_SPARC_TRAP(10, 0x039)
#define TME_SPARC64_TRAP_async_data_error		_TME_SPARC_TRAP( 2, 0x040)
#define TME_SPARC64_TRAP_interrupt_level(n)		_TME_SPARC_TRAP(32 - (n), 0x40 + (n))
#define TME_SPARC64_TRAP_spill_normal(n)		_TME_SPARC_TRAP( 9, 0x080 + (4 * (n)))
#define TME_SPARC64_TRAP_spill_other(n)			_TME_SPARC_TRAP( 9, 0x0a0 + (4 * (n)))
#define TME_SPARC64_TRAP_fill_normal(n)			_TME_SPARC_TRAP( 9, 0x0c0 + (4 * (n)))
#define TME_SPARC64_TRAP_fill_other(n)			_TME_SPARC_TRAP( 9, 0x0e0 + (4 * (n)))
#define TME_SPARC64_TRAP_trap_instruction(x)		_TME_SPARC_TRAP(16, 0x100 + (x))

/* generic traps: */
#define TME_SPARC_TRAP(ic, trap)				\
  (TME_SPARC_VERSION(ic) < 9					\
   ? _TME_CONCAT(TME_SPARC32_TRAP_,trap)			\
   : _TME_CONCAT(TME_SPARC64_TRAP_,trap))

/* SPARC FPU FSR fields: */
#define TME_SPARC_FSR_RND		(0xc0000000)
#define  TME_SPARC_FSR_RND_RN		 (0x00000000)
#define  TME_SPARC_FSR_RND_RZ		 (0x40000000)
#define  TME_SPARC_FSR_RND_RP		 (0x80000000)
#define  TME_SPARC_FSR_RND_RM		 (0xc0000000)
#define TME_SPARC_FSR_TEM		(0x0f800000)
#define TME_SPARC_FSR_NS		TME_BIT(22)
#define TME_SPARC_FSR_VER		(0x000e0000)
#define  TME_SPARC_FSR_VER_missing	 (0x000e0000)
#define TME_SPARC_FSR_FTT		(0x0001c000)
#define  TME_SPARC_FSR_FTT_none			(0x00000000)
#define  TME_SPARC_FSR_FTT_IEEE754_exception	(0x00004000)
#define  TME_SPARC_FSR_FTT_unfinished_FPop	(0x00008000)
#define  TME_SPARC_FSR_FTT_unimplemented_FPop	(0x0000c000)
#define  TME_SPARC_FSR_FTT_sequence_error	(0x00010000)
#define  TME_SPARC_FSR_FTT_hardware_error	(0x00014000)
#define  TME_SPARC_FSR_FTT_invalid_fp_register	(0x00018000)
#define TME_SPARC_FSR_QNE		TME_BIT(13)
#define TME_SPARC_FSR_FCC		(0x00000c00)
#define  TME_SPARC_FSR_FCC_EQ		 (0x00000000)
#define  TME_SPARC_FSR_FCC_LT		 (0x00000400)
#define  TME_SPARC_FSR_FCC_GT		 (0x00000800)
#define  TME_SPARC_FSR_FCC_UN		 (0x00000c00)
#define TME_SPARC_FSR_AEXC		(0x000003e0)
#define TME_SPARC_FSR_CEXC		(0x0000001f)
#define  TME_SPARC_FSR_CEXC_NVC		TME_BIT(4)
#define  TME_SPARC_FSR_CEXC_OFC		TME_BIT(3)
#define  TME_SPARC_FSR_CEXC_UFC		TME_BIT(2)
#define  TME_SPARC_FSR_CEXC_DZC		TME_BIT(1)
#define  TME_SPARC_FSR_CEXC_NXC		TME_BIT(0)

/* sparc32 PSR fields: */
#define TME_SPARC32_PSR_IMPL	(0xf0000000)
#define TME_SPARC32_PSR_VER	(0x0f000000)
#define TME_SPARC32_PSR_ICC_N	TME_BIT(23)
#define TME_SPARC32_PSR_ICC_Z	TME_BIT(22)
#define TME_SPARC32_PSR_ICC_V	TME_BIT(21)
#define TME_SPARC32_PSR_ICC_C	TME_BIT(20)
#define TME_SPARC32_PSR_ICC	(TME_SPARC32_PSR_ICC_N | TME_SPARC32_PSR_ICC_Z | TME_SPARC32_PSR_ICC_V | TME_SPARC32_PSR_ICC_C)
#define TME_SPARC32_PSR_EC	TME_BIT(13)
#define TME_SPARC32_PSR_EF	TME_BIT(12)
#define TME_SPARC32_PSR_PIL	(0x00000f00)
#define TME_SPARC32_PSR_S	TME_BIT(7)
#define TME_SPARC32_PSR_PS	TME_BIT(6)
#define TME_SPARC32_PSR_ET	TME_BIT(5)
#define TME_SPARC32_PSR_CWP	(0x0000001f)

/* sparc64 PSTATE flags: */
#define TME_SPARC64_PSTATE_CLE	TME_BIT(9)
#define TME_SPARC64_PSTATE_TLE	TME_BIT(8)
#define TME_SPARC64_PSTATE_MM	((2 << 7) - (1 << 6))
#define TME_SPARC64_PSTATE_RED	TME_BIT(5)
#define TME_SPARC64_PSTATE_PEF	TME_BIT(4)
#define TME_SPARC64_PSTATE_AM	TME_BIT(3)
#define TME_SPARC64_PSTATE_PRIV	TME_BIT(2)
#define TME_SPARC64_PSTATE_IE	TME_BIT(1)
#define TME_SPARC64_PSTATE_AG	TME_BIT(0)

/* sparc64 CCR flags: */
#define TME_SPARC64_CCR_XCC_N	TME_BIT(7)
#define TME_SPARC64_CCR_XCC_Z	TME_BIT(6)
#define TME_SPARC64_CCR_XCC_V	TME_BIT(5)
#define TME_SPARC64_CCR_XCC_C	TME_BIT(4)
#define TME_SPARC64_CCR_ICC_N	TME_BIT(3)
#define TME_SPARC64_CCR_ICC_Z	TME_BIT(2)
#define TME_SPARC64_CCR_ICC_V	TME_BIT(1)
#define TME_SPARC64_CCR_ICC_C	TME_BIT(0)
#define TME_SPARC64_CCR_ICC	(TME_SPARC64_CCR_ICC_N | TME_SPARC64_CCR_ICC_Z | TME_SPARC64_CCR_ICC_V | TME_SPARC64_CCR_ICC_C)
#define TME_SPARC64_CCR_XCC	(TME_SPARC64_CCR_XCC_N | TME_SPARC64_CCR_XCC_Z | TME_SPARC64_CCR_XCC_V | TME_SPARC64_CCR_XCC_C)

/* sparc64 FPRS flags: */
#define TME_SPARC64_FPRS_DL	TME_BIT(0)
#define TME_SPARC64_FPRS_DU	TME_BIT(1)
#define TME_SPARC64_FPRS_FEF	TME_BIT(2)

/* sparc64 TSTATE flags: */
#define TME_SPARC64_TSTATE_MASK_CWP	(0x1f << 0)
#define TME_SPARC64_TSTATE_MASK_PSTATE	(0xfff << 8)
#define TME_SPARC64_TSTATE_MASK_ASI	(0xff << 24)
#define TME_SPARC64_TSTATE_MASK_CCR	(((tme_uint64_t) 0xff) << 32)

/* sparc64 WSTATE fields: */
#define TME_SPARC64_WSTATE_NORMAL	(0x07)
#define TME_SPARC64_WSTATE_OTHER	(0x38)

/* sparc64 VER fields: */
#define TME_SPARC64_VER_MANUF		(((tme_uint64_t) 0xffff) << 48)
#define TME_SPARC64_VER_IMPL		(((tme_uint64_t) 0xffff) << 32)
#define TME_SPARC64_VER_MASK		(((tme_uint32_t) 0xff) << 24)
#define TME_SPARC64_VER_MAXTL		(((tme_uint32_t) 0xff) << 8)
#define TME_SPARC64_VER_MAXWIN		(((tme_uint32_t) 0x1f) << 0)

/* sparc64 TICK fields: */
#define TME_SPARC64_TICK_NPT		(((tme_uint64_t) 1) << 63)
#define TME_SPARC64_TICK_COUNTER	(TME_SPARC64_TICK_NPT - 1)

/* sparc VIS ASIs: */
#define TME_SPARC_VIS_ASI_PST8		(0xc0)
#define TME_SPARC_VIS_ASI_PST16		(0xc2)
#define TME_SPARC_VIS_ASI_PST32		(0xc4)
#define TME_SPARC_VIS_ASI_FL8		(0xd0)
#define TME_SPARC_VIS_ASI_FL16		(0xd2)

/* sparc VIS GSR fields: */
#define TME_SPARC_VIS_GSR_ALIGNADDR_OFF	((2 << 2) - (1 << 0))
#define TME_SPARC_VIS_GSR_SCALE_FACTOR	((2 << 6) - (1 << 3))

/* sparc conditions: */
#define TME_SPARC_COND_N	(0)
#define TME_SPARC_COND_E	(1)
#define TME_SPARC_COND_LE	(2)
#define TME_SPARC_COND_L	(3)
#define TME_SPARC_COND_LEU	(4)
#define TME_SPARC_COND_CS	(5)
#define TME_SPARC_COND_NEG	(6)
#define TME_SPARC_COND_VS	(7)
#define TME_SPARC_COND_NOT	(8)
#define TME_SPARC_COND_IS_CONDITIONAL(cond) (((cond) % TME_SPARC_COND_NOT) != TME_SPARC_COND_N)

/* idle types and idle type state: */
#define TME_SPARC_IDLE_TYPE_NULL		(0)
#define TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_0	TME_BIT(0)
#define TME_SPARC_IDLE_TYPE_SUNOS32_TYPE_0	TME_BIT(1)
#define TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_1	TME_BIT(2)
#define TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_0	TME_BIT(3)
#define TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_1	TME_BIT(4)
#define TME_SPARC_IDLE_TYPE_SUNOS64_TYPE_0	TME_BIT(5)
#define TME_SPARC_IDLE_TYPES_32			\
  (TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_0		\
   + TME_SPARC_IDLE_TYPE_SUNOS32_TYPE_0		\
   + TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_1	\
   )
#define TME_SPARC_IDLE_TYPES_64			\
  (TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_0		\
   + TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_1	\
   + TME_SPARC_IDLE_TYPE_SUNOS64_TYPE_0		\
   )
#define TME_SPARC_IDLE_TYPES_TARGET_CALL	\
  (TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_1		\
   + TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_1	\
   )
#define TME_SPARC_IDLE_TYPES_TARGET_BRANCH	\
  (TME_SPARC_IDLE_TYPE_SUNOS32_TYPE_0		\
   )
#define TME_SPARC_IDLE_TYPES_PC_RANGE		\
  (TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_0		\
   + TME_SPARC_IDLE_TYPE_SUNOS64_TYPE_0		\
   )
#define TME_SPARC_IDLE_TYPE_IS_SUPPORTED(ic, x)	\
  (((x)						\
    & (0 - (unsigned int) 1))			\
   & (TME_SPARC_VERSION(ic) < 9			\
      ? TME_SPARC_IDLE_TYPES_32			\
      : TME_SPARC_IDLE_TYPES_64))
#define TME_SPARC_IDLE_TYPE_IS(ic, x)		\
  (TME_SPARC_IDLE_TYPE_IS_SUPPORTED(ic, x)	\
   & (ic)->tme_sparc_idle_type)
#define TME_SPARC_IDLE_TYPE_PC_STATE(x)		(((tme_uint32_t) (x)) % sizeof(tme_uint32_t))

/* this makes an idle mark: */
#define TME_SPARC_IDLE_MARK(ic)					\
  do {								\
								\
    /* increment the idle marks, up to two: */			\
    ic->tme_sparc_idle_marks += (ic->tme_sparc_idle_marks < 2);	\
								\
    /* limit the remaining instruction burst to no more than	\
       an idle instruction burst: */				\
    ic->_tme_sparc_instruction_burst_remaining			\
      = TME_MIN(ic->_tme_sparc_instruction_burst_remaining,	\
		ic->_tme_sparc_instruction_burst_idle);		\
    ic->_tme_sparc_instruction_burst_other = TRUE;		\
  } while (/* CONSTCOND */ 0)

/* this stops idling: */
#define TME_SPARC_IDLE_STOP(ic)					\
  do {								\
								\
    /* clear the idle marks: */					\
    ic->tme_sparc_idle_marks = 0;				\
  } while (/* CONSTCOND */ 0)

/* major modes of the emulator: */
#define TME_SPARC_MODE_EXECUTION	(0)
#define TME_SPARC_MODE_STOP		(1)
#define TME_SPARC_MODE_HALT		(2)
#define TME_SPARC_MODE_OFF		(3)
#define TME_SPARC_MODE_TIMING_LOOP	(4)

/* the maximum number of windows: */
#define TME_SPARC_WINDOWS_MAX		(16)

/* the maximum number of trap levels: */
#define TME_SPARC_TL_MAX		(8)

/* this updates the recode CWP register offsets: */
#if TME_HAVE_RECODE
#define _TME_SPARC_RECODE_CWP_UPDATE(ic, reg_type)		\
  do {								\
    (ic)->tme_sparc_recode_window_base_offsets[0]		\
      = (ic)->tme_sparc_reg8_offset[1] * 8 * sizeof(reg_type);	\
    (ic)->tme_sparc_recode_window_base_offsets[1]		\
      = (ic)->tme_sparc_reg8_offset[3] * 8 * sizeof(reg_type);	\
    if (sizeof(reg_type) > sizeof(tme_uint32_t)) {		\
      (ic)->tme_sparc_recode_window_base_offsets[2]		\
	= (ic)->tme_sparc_reg8_offset[0] * 8 * sizeof(reg_type);\
    }								\
  } while (/* CONSTCOND */ 0)
#else  /* !TME_HAVE_RECODE */
#define _TME_SPARC_RECODE_CWP_UPDATE(ic, reg_type)		\
  do {								\
  } while (/* CONSTCOND */ 0 && (ic) && (reg_type) 0)
#endif /* !TME_HAVE_RECODE */

/* this updates the CWP register offset: */
#define _TME_SPARC_CWP_UPDATE(ic, cwp, reg8_offset_r8_r23, cwp_wraps, reg_type) \
  do {								\
    (ic)->tme_sparc_reg8_offset[1] = (reg8_offset_r8_r23);	\
    (ic)->tme_sparc_reg8_offset[2] = (reg8_offset_r8_r23);	\
    (ic)->tme_sparc_reg8_offset[3]				\
      = ((cwp) == (cwp_wraps)					\
	 ? ((8 - 24) / 8)					\
	 : (reg8_offset_r8_r23));				\
    _TME_SPARC_RECODE_CWP_UPDATE(ic, reg_type); 		\
  } while (/* CONSTCOND */ 0)

/* this updates the sparc32 CWP register offsets: */
#define TME_SPARC32_CWP_UPDATE(ic, cwp, reg8_offset_r8_r23)	\
  do {								\
    (reg8_offset_r8_r23) = (cwp) * 2;				\
    _TME_SPARC_CWP_UPDATE(ic, cwp, reg8_offset_r8_r23, TME_SPARC_NWINDOWS(ic) - 1, tme_uint32_t); \
  } while (/* CONSTCOND */ 0)

/* this updates the sparc64 CWP register offsets: */
#define TME_SPARC64_CWP_UPDATE(ic, cwp, reg8_offset_r8_r23)	\
  do {								\
    assert (cwp < TME_SPARC_NWINDOWS(ic));			\
    (reg8_offset_r8_r23)					\
      = (((TME_SPARC_NWINDOWS(ic) - 1) - cwp)			\
	 * 2);							\
    _TME_SPARC_CWP_UPDATE(ic, cwp, reg8_offset_r8_r23, 0, tme_uint64_t); \
  } while (/* CONSTCOND */ 0)

/* this gives the current %g0 register set index: */
#define TME_SPARC_G0_OFFSET(ic)			\
  (TME_SPARC_VERSION(ic) < 9 ? 0 : ((ic)->tme_sparc_reg8_offset[0] * 8))

/* this converts the given lvalue from a register number into a
   register set index: */
#define TME_SPARC_REG_INDEX(ic, reg)				\
  do {								\
    (reg) += ((ic)->tme_sparc_reg8_offset[(reg) / 8] * 8);	\
  } while (/* CONSTCOND */ 0)

/* this gives the hash for an address: */
#define TME_SPARC_TLB_HASH(ic, context, address)		\
  ((((tme_uint32_t) (address))					\
    >> (ic)->tme_sparc_tlb_page_size_log2)			\
   + (0 && (context)))

/* the size of the DTLB hash: */
#define _TME_SPARC_DTLB_HASH_SIZE 	(1024)

/* this gives the DTLB entry for a hash key: */
#define TME_SPARC_DTLB_ENTRY(ic, tlb_hash)			\
  (((tlb_hash) % _TME_SPARC_DTLB_HASH_SIZE)			\
   + (0 && (ic)))

/* the size of the ITLB hash: */
#define _TME_SPARC_ITLB_HASH_SIZE	(32)

/* this gives the ITLB entry for a hash key: */
#define TME_SPARC_ITLB_ENTRY(ic, tlb_hash)			\
  (_TME_SPARC_DTLB_HASH_SIZE					\
   + ((tlb_hash) % _TME_SPARC_ITLB_HASH_SIZE)			\
   + (0 && (ic)))

/* the count of all TLB entries: */
#define _TME_SPARC_TLB_COUNT					\
  (_TME_SPARC_DTLB_HASH_SIZE					\
   + _TME_SPARC_ITLB_HASH_SIZE)

/* load/store information: */
#define TME_SPARC_LSINFO_SIZE(x)		(x)
#define TME_SPARC_LSINFO_WHICH_SIZE(x)		(((x) / TME_SPARC_LSINFO_SIZE(1)) & 0xff)
#define TME_SPARC_LSINFO_ASI(x)			((x) << 8)
#define TME_SPARC_LSINFO_WHICH_ASI(x)		(((x) / TME_SPARC_LSINFO_ASI(1)) & 0xff)
#define TME_SPARC_LSINFO_ASI_FLAGS(x)		TME_SPARC_LSINFO_ASI(x)
#define TME_SPARC_LSINFO_WHICH_ASI_FLAGS(x)	TME_SPARC_LSINFO_WHICH_ASI(x)
#define TME_SPARC_LSINFO_A			(1 << 16)
#define TME_SPARC_LSINFO_OP_LD			(1 << 17)
#define TME_SPARC_LSINFO_OP_ST			(1 << 18)
#define TME_SPARC_LSINFO_OP_ATOMIC		(1 << 19)
#define TME_SPARC_LSINFO_OP_FETCH		(1 << 20)
#define TME_SPARC_LSINFO_LDD_STD		(1 << 21)
#define TME_SPARC_LSINFO_NO_FAULT		(1 << 22)
#define TME_SPARC_LSINFO_NO_CHECK_TLB		(1 << 23)
#define TME_SPARC_LSINFO_SLOW_CYCLES		(1 << 24)
#define TME_SPARC_LSINFO_LD_COMPLETED		(1 << 25)
#define TME_SPARC_LSINFO_ENDIAN_LITTLE		(1 << 26)
#define _TME_SPARC_LSINFO_X(x)			(1 << (27 + (x)))

/* load/store faults: */
#define TME_SPARC_LS_FAULT_NONE			(0)
#define TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED	TME_BIT(0)
#define TME_SPARC_LS_FAULT_LDD_STD_RD_ODD	TME_BIT(1)
#define TME_SPARC_LS_FAULT_BUS_FAULT		TME_BIT(2)
#define TME_SPARC_LS_FAULT_BUS_ERROR		TME_BIT(3)
#define _TME_SPARC_LS_FAULT_X(x)		TME_BIT(4 + (x))
#define TME_SPARC64_LS_FAULT_PRIVILEGED_ASI	_TME_SPARC_LS_FAULT_X(0)
#define TME_SPARC64_LS_FAULT_NO_FAULT_NON_LOAD	_TME_SPARC_LS_FAULT_X(1)
#define TME_SPARC64_LS_FAULT_NO_FAULT_FAULT	_TME_SPARC_LS_FAULT_X(2)
#define TME_SPARC64_LS_FAULT_SIDE_EFFECTS	_TME_SPARC_LS_FAULT_X(3)
#define TME_SPARC64_LS_FAULT_VA_RANGE		_TME_SPARC_LS_FAULT_X(4)
#define TME_SPARC64_LS_FAULT_VA_RANGE_NNPC	_TME_SPARC_LS_FAULT_X(5)
#define TME_SPARC64_LS_FAULT_UNCACHEABLE	_TME_SPARC_LS_FAULT_X(6)
#define _TME_SPARC64_LS_FAULT_X(x)		_TME_SPARC_LS_FAULT_X(7 + (x))

/* flags for memory features: */
#define TME_SPARC_MEMORY_FLAG_HAS_NUCLEUS	(1 << 0)
#define TME_SPARC_MEMORY_FLAG_HAS_INVERT_ENDIAN	(1 << 1)
#define TME_SPARC_MEMORY_FLAG_HAS_LDDF_STDF_32	(1 << 2)
#define TME_SPARC_MEMORY_FLAG_HAS_LDQF_STQF_32	(1 << 3)

/* the undefined FPU register number: */
#define TME_SPARC_FPU_FPREG_NUMBER_UNDEF	(64)

/* flags for FPU features: */
#define TME_SPARC_FPU_FLAG_NO_QUAD		TME_BIT(0)
#define TME_SPARC_FPU_FLAG_NO_FSQRT		TME_BIT(1)
#define TME_SPARC_FPU_FLAG_NO_FMUL_WIDER	TME_BIT(2)
#define TME_SPARC_FPU_FLAG_OK_REG_MISALIGNED	TME_BIT(3)

/* FPU modes: */
#define TME_SPARC_FPU_MODE_EXECUTE		(0)
#define TME_SPARC_FPU_MODE_EXCEPTION_PENDING	(1)
#define TME_SPARC_FPU_MODE_EXCEPTION		(2)

/* this marks an FPU register as dirty: */
#define TME_SPARC_FPU_DIRTY(ic, fpreg_number)	\
  do {						\
    if (TME_SPARC_VERSION(ic) >= 9) {		\
      assert ((TME_SPARC64_FPRS_DU - 1)		\
	      == TME_SPARC64_FPRS_DL);		\
      (ic)->tme_sparc64_ireg_fprs		\
	|= (TME_SPARC64_FPRS_DU			\
	    - ((fpreg_number) < 32));		\
    }						\
  } while (/* CONSTCOND */ 0)

/* this returns nonzero if the FPU is disabled: */
#define TME_SPARC_FPU_IS_DISABLED(ic)					\
  ((TME_SPARC_VERSION(ic) < 9)						\
   ? ((ic)->tme_sparc32_ireg_psr & TME_SPARC32_PSR_EF) == 0		\
   : (((ic)->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_PEF) == 0	\
      || ((ic)->tme_sparc64_ireg_fprs & TME_SPARC64_FPRS_FEF) == 0))

/* this returns nonzero if recode supports this sparc: */
#if TME_HAVE_RECODE
#ifndef TME_HAVE_INT64_T
#define TME_SPARC_HAVE_RECODE(ic)		(1)
#else  /* TME_HAVE_INT64_T */
#define TME_SPARC_HAVE_RECODE(ic)			\
  (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32	\
   || TME_SPARC_VERSION(ic) < 9)
#endif /* TME_HAVE_INT64_T */
#else  /* !TME_HAVE_RECODE */
#define TME_SPARC_HAVE_RECODE(ic)		(0)
#endif /* !TME_HAVE_RECODE */

#if TME_HAVE_RECODE

/* this is the maximum number of cacheable address regions that we
   will recode from: */
#define TME_SPARC_RECODE_CACHEABLES_MAX		(4)

/* this is the modulus used in the recode source address hash: */
#define TME_SPARC_RECODE_SRC_HASH_MODULUS	(16381)

/* the recode source address hash is set associative.  this is the
   number of ways in each set: */
#define TME_SPARC_RECODE_SRC_HASH_SIZE_SET	(4)

/* the sparc recode TLB flags for loads and stores: */
#define TME_SPARC_RECODE_TLB_FLAG_LD_USER(ic)	TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 0)
#define TME_SPARC_RECODE_TLB_FLAG_LD_PRIV(ic)	TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 1)
#define TME_SPARC_RECODE_TLB_FLAG_LD_NF(ic)	TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 2)
#define TME_SPARC_RECODE_TLB_FLAG_LD_F(ic)	TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 3)
#define TME_SPARC_RECODE_TLB_FLAG_ST_USER(ic)	TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 4)
#define TME_SPARC_RECODE_TLB_FLAG_ST_PRIV(ic)	TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 5)
#define TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_LITTLE(ic) TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 6)
#define TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_BIG(ic) TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 7)
#define TME_SPARC_RECODE_TLB_FLAG_LS_ENDIAN_INVERT(ic) TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 8)

/* the sparc recode TLB flags for chaining: */
#define TME_SPARC_RECODE_TLB_FLAG_CHAIN_USER(ic) TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 0)
#define TME_SPARC_RECODE_TLB_FLAG_CHAIN_PRIV(ic) TME_RECODE_TLB_FLAG(ic->tme_sparc_recode_ic, 1)

#endif /* TME_HAVE_RECODE */

/* instruction handler macros: */
#define TME_SPARC_FORMAT3_DECL(name, type) void name _TME_P((struct tme_sparc *, const type *, const type *, type *))
#ifdef __STDC__
#define TME_SPARC_FORMAT3(name, type) void name(struct tme_sparc *ic, const type *_rs1, const type *_rs2, type *_rd)
#else  /* !__STDC__ */
#define TME_SPARC_FORMAT3(name, type) void name(ic, _rs1, _rs2, _rd) struct tme_sparc *ic; const type *_rs1, *_rs2; type *_rd;
#endif /* !__STDC__ */
#define TME_SPARC_FORMAT3_RS1		(*_rs1)
#define TME_SPARC_FORMAT3_RS2		(*_rs2)
#define TME_SPARC_FORMAT3_RD		(*_rd)
#define TME_SPARC_FORMAT3_RD_ODD(iregs)	(*(_rd + (&(((struct tme_ic *) NULL)->iregs(1)) - &(((struct tme_ic *) NULL)->iregs(0)))))
#define TME_SPARC_INSN			ic->_tme_sparc_insn
#define TME_SPARC_INSN_OK		return
#define TME_SPARC_INSN_TRAP(trap)			\
  do {							\
    if (TME_SPARC_VERSION(ic) < 9) {			\
      tme_sparc32_trap(ic, trap);			\
    }							\
    else {						\
      tme_sparc64_trap(ic, trap);			\
    }							\
  } while (/* CONSTCOND */ 0)
#define TME_SPARC_INSN_PRIV				\
  do {							\
    if (__tme_predict_false(!TME_SPARC_PRIV(ic))) {	\
      TME_SPARC_INSN_TRAP(TME_SPARC_VERSION(ic) < 9	\
			  ? TME_SPARC32_TRAP_privileged_instruction \
			  : TME_SPARC64_TRAP_privileged_opcode); \
    }							\
  } while (/* CONSTCOND */ 0)
#define TME_SPARC_INSN_FPU_ENABLED			\
  do {							\
    if (__tme_predict_false(TME_SPARC_FPU_IS_DISABLED(ic))) { \
      TME_SPARC_INSN_TRAP(TME_SPARC_TRAP(ic,fp_disabled));\
    }							 \
  } while (/* CONSTCOND */ 0)
#define TME_SPARC_INSN_FPU				\
  do {							\
    TME_SPARC_INSN_FPU_ENABLED;				\
    if (__tme_predict_false((ic)->tme_sparc_fpu_mode	\
			    != TME_SPARC_FPU_MODE_EXECUTE)) { \
      tme_sparc_fpu_exception_check(ic);		\
    }							\
  } while (/* CONSTCOND */ 0)

#define TME_SPARC_INSN_ILL(ic)				\
  TME_SPARC_INSN_TRAP(TME_SPARC_TRAP(ic,illegal_instruction))

/* logging: */
#define TME_SPARC_LOG_HANDLE(ic)				\
  (&(ic)->tme_sparc_element->tme_element_log_handle)
#define tme_sparc_log_start(ic, level, rc)			\
  do {								\
    tme_log_start(TME_SPARC_LOG_HANDLE(ic), level, rc) {	\
      if ((ic)->_tme_sparc_mode != TME_SPARC_MODE_EXECUTION) {	\
        tme_log_part(TME_SPARC_LOG_HANDLE(ic),			\
                     "mode=%d ",				\
                     (ic)->_tme_sparc_mode);			\
      }								\
      else if (TME_SPARC_VERSION(ic) < 9) {			\
        tme_log_part(TME_SPARC_LOG_HANDLE(ic),			\
	             "pc=%c/0x%08" TME_PRIx32 " ",		\
	             (TME_SPARC_PRIV(ic)			\
		      ? 'S'					\
		      : 'U'),					\
		     ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)); \
      }								\
      else {							\
        tme_log_part(TME_SPARC_LOG_HANDLE(ic),			\
	             "pc=%c/0x%08" TME_PRIx64 " ",		\
	             (TME_SPARC_PRIV(ic)			\
		      ? 'S'					\
		      : 'U'),					\
		     ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)); \
      }								\
      do
#define tme_sparc_log_finish(ic)				\
      while (/* CONSTCOND */ 0);				\
    } tme_log_finish(TME_SPARC_LOG_HANDLE(ic));			\
  } while (/* CONSTCOND */ 0)
#define tme_sparc_log(ic, level, rc, x)		\
  do {						\
    tme_sparc_log_start(ic, level, rc) {	\
      tme_log_part x;				\
    } tme_sparc_log_finish(ic);			\
  } while (/* CONSTCOND */ 0)

/* PROM delay factors: */
#define TME_SPARC_PROM_DELAY_FACTOR_UNCORRECTED	((tme_uint32_t) (0 - (tme_uint32_t) 1))
#define TME_SPARC_PROM_DELAY_FACTOR_BEST	((tme_uint32_t) (0 - (tme_uint32_t) 2))
#define TME_SPARC_PROM_DELAY_FACTOR_MIN		((tme_uint32_t) (0 - (tme_uint32_t) 3))

/* flags for _tme_sparc_external_check(): */
#define TME_SPARC_EXTERNAL_CHECK_NULL		(0)
#define TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED	(1 << 0)
#define TME_SPARC_EXTERNAL_CHECK_PCS_UPDATED	(1 << 1)

/* miscellaneous: */
#define _TME_SPARC_VERSION(ic)	((ic)->tme_sparc_version)
#define _TME_SPARC_NWINDOWS(ic) ((ic)->tme_sparc_nwindows)
#define _TME_SPARC_MEMORY_FLAGS(ic) ((ic)->tme_sparc_memory_flags)
#define _TME_SPARC32_PRIV(ic)   (((ic)->tme_sparc32_ireg_psr & TME_SPARC32_PSR_S) != 0)
#define _TME_SPARC64_PRIV(ic)   (((ic)->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_PRIV) != 0)
#define TME_SPARC_VERSION(ic)	_TME_SPARC_VERSION(ic)
#define TME_SPARC_NWINDOWS(ic)	_TME_SPARC_NWINDOWS(ic)
#define TME_SPARC_MEMORY_FLAGS(ic) _TME_SPARC_MEMORY_FLAGS(ic)
#define TME_SPARC_PRIV(ic)		\
  ((TME_SPARC_VERSION(ic) < 9)		\
   ? _TME_SPARC32_PRIV(ic)		\
   : _TME_SPARC64_PRIV(ic))

/* structures: */
struct tme_sparc;

/* the widest supported sparc register: */
#ifdef TME_HAVE_INT64_T
typedef tme_uint64_t tme_sparc_ireg_umax_t;
#else  /* !TME_HAVE_INT64_T */
typedef tme_uint32_t tme_sparc_ireg_umax_t;
#endif /* !TME_HAVE_INT64_T */

/* format 3 instruction functions: */
typedef TME_SPARC_FORMAT3_DECL((*_tme_sparc32_format3), tme_uint32_t);
#ifdef TME_HAVE_INT64_T
typedef TME_SPARC_FORMAT3_DECL((*_tme_sparc64_format3), tme_uint64_t);
#endif /* TME_HAVE_INT64_T */

/* a sparc deferred-trap queue: */
struct tme_sparc_trapqueue {
#ifdef TME_HAVE_INT64_T
  tme_uint64_t tme_sparc_trapqueue_address;
#else  /* !TME_HAVE_INT64_T */
  tme_uint32_t tme_sparc_trapqueue_address;
#endif /* !TME_HAVE_INT64_T */
  tme_uint32_t tme_sparc_trapqueue_insn;
};

/* a sparc load/store: */
struct tme_sparc_ls {

  /* this maps an address for the bus: */
  void (*tme_sparc_ls_address_map) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));

  /* the current slow cycle function: */
  void (*tme_sparc_ls_cycle) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));

  /* a pointer to the rd register: */
  union {
    tme_uint32_t *_tme_sparc_ls_rd_u_32;
#define tme_sparc_ls_rd32 _tme_sparc_ls_rd_u._tme_sparc_ls_rd_u_32
#ifdef TME_HAVE_INT64_T
    tme_uint64_t *_tme_sparc_ls_rd_u_64;
#define tme_sparc_ls_rd64 _tme_sparc_ls_rd_u._tme_sparc_ls_rd_u_64
#endif /* TME_HAVE_INT64_T */
  } _tme_sparc_ls_rd_u;

  /* a pointer to the TLB entry: */
  struct tme_sparc_tlb *tme_sparc_ls_tlb;

  /* the current address: */
  union {
    tme_uint32_t _tme_sparc_ls_address_u_32;
#define tme_sparc_ls_address32 _tme_sparc_ls_address_u._tme_sparc_ls_address_u_32
#ifdef TME_HAVE_INT64_T
    tme_uint64_t _tme_sparc_ls_address_u_64;
#define tme_sparc_ls_address64 _tme_sparc_ls_address_u._tme_sparc_ls_address_u_64
#endif /* TME_HAVE_INT64_T */
  } _tme_sparc_ls_address_u;

  /* the context and ASI mask: */
  tme_bus_context_t tme_sparc_ls_context;
  tme_uint32_t tme_sparc_ls_asi_mask;

  /* the index of the TLB entry: */
  tme_uint32_t tme_sparc_ls_tlb_i;

  /* the lsinfo: */
  tme_uint32_t tme_sparc_ls_lsinfo;

  /* any fault information: */
  tme_uint32_t tme_sparc_ls_faults;

  /* the current size: */
  tme_uint8_t tme_sparc_ls_size;

  /* the current offset in the memory buffer: */
  tme_uint8_t tme_sparc_ls_buffer_offset;

  /* some current state of the operation: */
  tme_uint8_t tme_sparc_ls_state;

  /* a mapping TLB entry: */
  struct tme_bus_tlb tme_sparc_ls_tlb_map;

  /* a bus cycle structure: */
  struct tme_bus_cycle tme_sparc_ls_bus_cycle;
};

/* ASI handlers: */
typedef void (*_tme_sparc_ls_asi_handler) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));

#if TME_HAVE_RECODE

/* this is the type used for keys in the recode source address hash.
   if the widest recode guest is only 32 bits, we only need a
   tme_uint32_t, because a sparc32 recode guest can only make 32-bit
   guest addresses.

   otherwise, we use an unsigned long, which we assume is at least as
   wide as the address size of the *host*.  NB that this may be
   narrower than the address size of a sparc guest, but since source
   address hash keys are for guest addresses that are cacheable, which
   correspond to host addresses, this is fine (the source address hash
   key is essentially the guest address mapped into the host address
   space): */
#if TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_32
typedef tme_uint32_t tme_sparc_recode_src_key_t;
#else  /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
typedef unsigned long tme_sparc_recode_src_key_t;
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */

/* a recode cacheable source memory: */
struct tme_sparc_recode_cacheable {
    
  /* the cacheable contents and size: */
  const tme_shared tme_uint8_t *tme_sparc_recode_cacheable_contents;
  unsigned long tme_sparc_recode_cacheable_size;

  /* the source address hash key for the first instruction word in the
     cacheable contents: */
  tme_sparc_recode_src_key_t tme_sparc_recode_cacheable_src_key_first;

  /* the valids bitmap for this cacheable: */
  tme_shared tme_uint8_t *tme_sparc_recode_cacheable_valids;
};

#endif /* TME_HAVE_RECODE */

/* the sparc state: */
struct tme_sparc {

  /* the IC data structure.  it is beneficial to have this structure
     first, since register numbers can often simply be scaled and 
     added without an offset to the struct tme_sparc pointer to get
     to their contents: */
  struct tme_ic tme_sparc_ic;

  /* the cycles scaling: */
  tme_misc_cycles_scaling_t tme_sparc_cycles_scaling;
  tme_misc_cycles_scaling_t tme_sparc_cycles_unscaling;

  /* the number of scaled cycles per microsecond: */
  tme_uint32_t tme_sparc_cycles_scaled_per_usec;

  /* the register offsets for the four groups of eight registers: */
  tme_int8_t tme_sparc_reg8_offset[4];

  /* the architecture version, and number of windows: */
  unsigned int tme_sparc_version;
  unsigned int tme_sparc_nwindows;

  /* v9 constants: */
  unsigned int tme_sparc64_maxtl;

  /* the backpointer to our element: */
  struct tme_element *tme_sparc_element;

  /* our bus connection.  the sparc bus connection may be an
     adaptation layer for another bus connection type: */
  struct tme_upa_bus_connection *_tme_upa_bus_connection;
  struct tme_sparc_bus_connection *_tme_sparc_bus_connection;
  struct tme_bus_connection *_tme_sparc_bus_generic;

  /* a jmp_buf back to the dispatcher: */
  jmp_buf _tme_sparc_dispatcher;

  /* the current mode of the CPU: */
  int _tme_sparc_mode;

  /* address space identifiers and masks: */
  tme_uint32_t tme_sparc_asi_mask_insn;
  tme_uint32_t tme_sparc_asi_mask_data;

  /* the implementation-dependent data: */
  union {
    const _tme_sparc32_format3 *_tme_sparc_execute_opmap_u_32;
#define _tme_sparc32_execute_opmap _tme_sparc_execute_opmap_u._tme_sparc_execute_opmap_u_32
#ifdef TME_HAVE_INT64_T
    const _tme_sparc64_format3 *_tme_sparc_execute_opmap_u_64;
#define _tme_sparc64_execute_opmap _tme_sparc_execute_opmap_u._tme_sparc_execute_opmap_u_64
#endif /* TME_HAVE_INT64_T */
  } _tme_sparc_execute_opmap_u;
  tme_uint32_t (*_tme_sparc_ls_asi_misaligned) _TME_P((struct tme_sparc *, tme_uint32_t));
  const _tme_sparc_ls_asi_handler *_tme_sparc_ls_asi_handlers;
#ifdef TME_HAVE_INT64_T
  tme_uint64_t tme_sparc64_rstvaddr;
#endif /* TME_HAVE_INT64_T */

  /* the implementation-dependent functions: */
  void (*_tme_sparc_execute) _TME_P((struct tme_sparc *));
  tme_uint32_t (*_tme_sparc_fpu_ver) _TME_P((struct tme_sparc *, const char *, char **));
  void (*_tme_sparc_external_check) _TME_P((struct tme_sparc *, int));
  void (*_tme_sparc_ls_address_map) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));
  void (*_tme_sparc_ls_bus_cycle) _TME_P((const struct tme_sparc *, struct tme_sparc_ls *));
  void (*_tme_sparc_ls_bus_fault) _TME_P((struct tme_sparc *, struct tme_sparc_ls *, int));
  void (*_tme_sparc_ls_trap) _TME_P((struct tme_sparc *, struct tme_sparc_ls *));
  int (*_tme_sparc_tlb_fill) _TME_P((struct tme_bus_connection *,
				     struct tme_bus_tlb *,
				     tme_bus_addr_t,
				     unsigned int));
#ifdef TME_HAVE_INT64_T
  void (*_tme_sparc_upa_interrupt) _TME_P((struct tme_upa_bus_connection *,
					   tme_uint32_t,
					   const tme_uint64_t *,
					   struct tme_completion *));
#endif /* TME_HAVE_INT64_T */
  void (*_tme_sparc64_update_pstate) _TME_P((struct tme_sparc *, tme_uint32_t, tme_uint32_t));

  /* the runlength state: */
  struct tme_runlength tme_sparc_runlength;
  tme_uint32_t tme_sparc_runlength_update_period;
  tme_uint32_t tme_sparc_runlength_update_next;

  /* the instruction burst counts, and the remaining burst: */
#define _tme_sparc_instruction_burst tme_sparc_runlength.tme_runlength_value
  tme_uint32_t _tme_sparc_instruction_burst_idle;
  tme_uint32_t _tme_sparc_instruction_burst_remaining;
  unsigned int _tme_sparc_instruction_burst_other;

  /* the token for any currently busy instruction TLB entry: */
  struct tme_token *_tme_sparc_itlb_current_token;

  /* instruction information: */
  tme_uint32_t _tme_sparc_insn;
  
  /* memory flags: */
  tme_uint32_t tme_sparc_memory_flags;

  /* ASIs: */
  struct {
    tme_uint8_t tme_sparc_asi_mask_flags;
    tme_uint8_t tme_sparc_asi_handler;
  } tme_sparc_asis[0x100];

  /* contexts: */
  tme_bus_context_t tme_sparc_memory_context_max;
  tme_bus_context_t tme_sparc_memory_context_default;
  tme_uint32_t tme_sparc_memory_context_primary;
  tme_uint32_t tme_sparc_memory_context_secondary;

  /* the external interface: */
  tme_mutex_t tme_sparc_external_mutex;
  tme_cond_t tme_sparc_external_cond;
  tme_memory_atomic_flag_t tme_sparc_external_flag;
  tme_memory_atomic_flag_t tme_sparc_external_reset_asserted;
  tme_memory_atomic_flag_t tme_sparc_external_reset_negated;
  tme_memory_atomic_flag_t tme_sparc_external_halt_asserted;
  tme_memory_atomic_flag_t tme_sparc_external_halt_negated;
  tme_memory_atomic_flag_t tme_sparc_external_bg_asserted;
  tme_shared tme_uint8_t tme_sparc_external_ipl;
  tme_rwlock_t tme_sparc_external_ipl_rwlock;

  /* the slow load/store buffer: */
  union {
    tme_uint8_t tme_sparc_memory_buffer8s[64];
    tme_uint16_t tme_sparc_memory_buffer16s[32];
    tme_uint32_t tme_sparc_memory_buffer32s[16];
#ifdef TME_HAVE_INT64_T
    tme_uint64_t tme_sparc_memory_buffer64s[8];
#endif /* TME_HAVE_INT64_T */
  } tme_sparc_memory_buffer;

  /* any FPU state: */
  struct tme_ieee754_ctl tme_sparc_fpu_ieee754_ctl;
  _tme_const struct tme_ieee754_ops *tme_sparc_fpu_ieee754_ops;
  _tme_const struct tme_ieee754_ops *tme_sparc_fpu_ieee754_ops_user;
  _tme_const struct tme_ieee754_ops *tme_sparc_fpu_ieee754_ops_strict;
  struct tme_float tme_sparc_fpu_fpregs[TME_SPARC_FPU_FPREG_NUMBER_UNDEF];
  unsigned int tme_sparc_fpu_fpreg_sizes[TME_SPARC_FPU_FPREG_NUMBER_UNDEF];
  tme_uint32_t tme_sparc_fpu_fsr;
  tme_uint32_t tme_sparc_fpu_xfsr;
  struct tme_sparc_trapqueue tme_sparc_fpu_fq[1];
  unsigned int tme_sparc_fpu_mode;
  unsigned int tme_sparc_fpu_flags;
  int tme_sparc_fpu_incomplete_abort;

  /* any VIS state: */
  tme_uint32_t tme_sparc_vis_ls_fault_illegal;
  tme_uint8_t tme_sparc_vis_gsr;

  /* the idle state: */
  tme_uint8_t tme_sparc_idle_marks;
  unsigned int tme_sparc_idle_type;
  union {
    tme_uint32_t tme_sparc_idle_pcs_32[4];
#ifdef TME_HAVE_INT64_T
    tme_uint64_t tme_sparc_idle_pcs_64[4];
#endif /* TME_HAVE_INT64_T */
  } tme_sparc_idle_pcs;
#define tme_sparc_idle_pcs_32 tme_sparc_idle_pcs.tme_sparc_idle_pcs_32
#define tme_sparc_idle_pcs_64 tme_sparc_idle_pcs.tme_sparc_idle_pcs_64

  /* the address mask: */
  tme_sparc_ireg_umax_t tme_sparc_address_mask;

  /* the PROM delay factor: */
  tme_uint32_t tme_sparc_prom_delay_factor;

  /* the log2 of the TLB page size: */
  unsigned int tme_sparc_tlb_page_size_log2;

  /* the data and instruction TLB entry sets: */
  struct tme_sparc_tlb tme_sparc_tlbs[_TME_SPARC_TLB_COUNT];

  /* either tokens or recode TLB entries for the sparc TLB entries: */
  union {
    struct tme_token _tme_sparc_tlb_tokens_u_tokens[_TME_SPARC_TLB_COUNT];
#define tme_sparc_tlb_tokens _tme_sparc_tlb_tokens_u._tme_sparc_tlb_tokens_u_tokens
#if TME_HAVE_RECODE
    struct tme_recode_tlb_c16_a32 _tme_sparc_tlb_tokens_u_tlb32s[_TME_SPARC_TLB_COUNT];
#define tme_sparc_recode_tlb32s _tme_sparc_tlb_tokens_u._tme_sparc_tlb_tokens_u_tlb32s
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
    struct tme_recode_tlb_c16_a64 _tme_sparc_tlb_tokens_u_tlb64s[_TME_SPARC_TLB_COUNT];
#define tme_sparc_recode_tlb64s _tme_sparc_tlb_tokens_u._tme_sparc_tlb_tokens_u_tlb64s
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
#endif /* TME_HAVE_RECODE */
  } _tme_sparc_tlb_tokens_u;

  /* timing information: */
  tme_uint8_t tme_sparc_timing_loop_cycles_each;
  tme_int8_t tme_sparc_timing_loop_addend;
  tme_uint8_t tme_sparc_timing_loop_branch_taken_max;
  tme_sparc_ireg_umax_t tme_sparc_timing_loop_branch_taken_count_max_m1;
  union tme_value64 tme_sparc_timing_loop_start;
  union tme_value64 tme_sparc_timing_loop_finish;

#if TME_HAVE_RECODE

  /* the recode IC: */
  struct tme_recode_ic *tme_sparc_recode_ic;

  /* the register window base offsets.  %r8 through %r23 use window
     zero, %r24 through %r31 use window one, and %r0 through %r7 use
     window two: */
  tme_int32_t tme_sparc_recode_window_base_offsets[3];

  /* the flags thunks: */
  const struct tme_recode_flags_thunk *tme_sparc_recode_flags_thunk_add;
  const struct tme_recode_flags_thunk *tme_sparc_recode_flags_thunk_sub;
  const struct tme_recode_flags_thunk *tme_sparc_recode_flags_thunk_logical;
  const struct tme_recode_flags_thunk *tme_sparc_recode_flags_thunk_rcc;

  /* the conditions thunks: */
  const struct tme_recode_conds_thunk *tme_sparc_recode_conds_thunk_icc;
  const struct tme_recode_conds_thunk *tme_sparc_recode_conds_thunk_xcc;
  const struct tme_recode_conds_thunk *tme_sparc_recode_conds_thunk_rcc;

  /* the read/write thunks: */
  const struct tme_recode_rw_thunk *tme_sparc_recode_rw_thunks[128];

  /* the current read/write TLB flags: */
  tme_uint32_t tme_sparc_recode_rw_tlb_flags;

  /* the current chain TLB flags: */
  tme_uint32_t tme_sparc_recode_chain_tlb_flags;

  /* the chain return address stack: */
  tme_recode_ras_entry_t _tme_sparc_recode_chain_ras[16];
  tme_uint32_t _tme_sparc_recode_chain_ras_pointer;

  /* the recode cacheable source memories: */
  struct tme_sparc_recode_cacheable tme_sparc_recode_cacheables[TME_SPARC_RECODE_CACHEABLES_MAX];

  /* the most recently added cacheable: */
  struct tme_sparc_recode_cacheable *tme_sparc_recode_cacheable_first;

  /* the bitmap of active cacheable recode pages: */
  tme_uint8_t *tme_sparc_recode_cacheable_actives;

  /* the recode source hash: */
  /* NB: conceptually, each entry in the recode source address hash is
     one tme_sparc_recode_src_key_t and one tme_recode_thunk_off_t.
     since these types may be different sizes, to guarantee that the
     recode source address hash is packed, each element of this recode
     source address hash array covers this many entries: */
#define TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT	\
  ((sizeof(tme_sparc_recode_src_key_t)		\
    + (sizeof(tme_recode_thunk_off_t) - 1))	\
   / sizeof(tme_recode_thunk_off_t))
  struct {

    /* the source address hash keys: */
    tme_sparc_recode_src_key_t tme_sparc_recode_src_hash_keys[TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT];

    /* if bit zero is set, the remaining bits of the hash value are
       the hit count for the program counter, otherwise, the entire
       hash value is the offset of the instructions thunk for the
       program counter: */
    tme_recode_thunk_off_t tme_sparc_recode_src_hash_values[TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT];

  } tme_sparc_recode_src_hash[(TME_SPARC_RECODE_SRC_HASH_MODULUS
			       * TME_SPARC_RECODE_SRC_HASH_SIZE_SET)
			      / TME_SPARC_RECODE_SRC_HASH_SIZE_ELEMENT];

  /* an instructions group: */
  struct tme_recode_insns_group tme_sparc_recode_insns_group;

  /* the recode insns buffer: */
  struct tme_recode_insn tme_sparc_recode_insns[TME_RECODE_INSNS_THUNK_INSNS_MAX];

#ifdef _TME_SPARC_RECODE_VERIFY

  /* the last PC to replay, or zero if instructions are not being
     replayed: */
  tme_recode_uguest_t tme_sparc_recode_verify_replay_last_pc;

  /* the size of the sparc state to verify, and the sparc state total
     size: */
  unsigned long tme_sparc_recode_verify_ic_size;
  unsigned long tme_sparc_recode_verify_ic_size_total;

#endif /* _TME_SPARC_RECODE_VERIFY */

#endif /* TME_HAVE_RECODE */

#ifdef _TME_SPARC_STATS
  /* statistics: */
  struct {

    /* the total number of instructions executed: */
    tme_uint64_t tme_sparc_stats_insns_total;

    /* the total number of instructions fetched slowly: */
    tme_uint64_t tme_sparc_stats_insns_slow;

    /* the total number of redispatches: */
    tme_uint64_t tme_sparc_stats_redispatches;

    /* the total number of data memory operations: */
    tme_uint64_t tme_sparc_stats_memory_total;

    /* the total number of ITLB maps: */
    tme_uint64_t tme_sparc_stats_itlb_map;

    /* the total number of DTLB map: */
    tme_uint64_t tme_sparc_stats_dtlb_map;

    /* the total number of ITLB fills: */
    tme_uint64_t tme_sparc_stats_itlb_fill;

    /* the total number of DTLB fills: */
    tme_uint64_t tme_sparc_stats_dtlb_fill;

#if TME_HAVE_RECODE

    /* the total number of calls to tme_sparc_recode(): */
    tme_uint64_t tme_sparc_stats_recode_calls;

    /* the total number of active recode page invalidations: */
    tme_uint64_t tme_sparc_stats_recode_page_invalids;

    /* the total number of recode source hash probes: */
    tme_uint64_t tme_sparc_stats_recode_source_hash_probes;

    /* the total number of recode source hash misses: */
    tme_uint64_t tme_sparc_stats_recode_source_hash_misses;

    /* the total number of instructions executed in thunks: */
    tme_uint64_t tme_sparc_stats_recode_insns_total;

    /* the total number of various assists: */
    tme_uint64_t tme_sparc_stats_recode_assist;
    tme_uint64_t tme_sparc_stats_recode_assist_full;
    tme_uint64_t tme_sparc_stats_recode_assist_ld;
    tme_uint64_t tme_sparc_stats_recode_assist_st;
    
    /* the total number of assists by opcode: */
    tme_uint64_t tme_sparc_stats_recode_assist_opcode[128];

#endif /* TME_HAVE_RECODE */

  } tme_sparc_stats;
#define TME_SPARC_STAT_N(ic, x, n) do { (ic)->tme_sparc_stats.x += (n); } while (/* CONSTCOND */ 0)
#else  /* !_TME_SPARC_STATS */
#define TME_SPARC_STAT_N(ic, x, n) do { } while (/* CONSTCOND */ 0 && (ic) && (n))
#endif /* !_TME_SPARC_STATS */
#define TME_SPARC_STAT(ic, x) TME_SPARC_STAT_N(ic, x, 1)
};

/* globals: */
extern const tme_uint8_t _tme_sparc_conds_icc[16];
extern const tme_uint8_t _tme_sparc_conds_fcc[4];

/* prototypes: */
void tme_sparc_sync_init _TME_P((struct tme_sparc *));
int tme_sparc_new _TME_P((struct tme_sparc *, const char * const *, const void *, char **));
void tme_sparc_redispatch _TME_P((struct tme_sparc *));
void tme_sparc_do_reset _TME_P((struct tme_sparc *));
void tme_sparc_do_idle _TME_P((struct tme_sparc *));
void tme_sparc32_external_check _TME_P((struct tme_sparc *, int));
struct tme_sparc_tlb *tme_sparc_itlb_current _TME_P((struct tme_sparc *));
tme_uint32_t tme_sparc_insn_peek _TME_P((struct tme_sparc *, tme_sparc_ireg_umax_t));
tme_uint32_t tme_sparc_fetch_nearby _TME_P((struct tme_sparc *, long));
void tme_sparc_callout_unlock _TME_P((struct tme_sparc *));
void tme_sparc_callout_relock _TME_P((struct tme_sparc *));

/* load/store support: */
void tme_sparc_ls_bus_fault _TME_P((struct tme_sparc *, struct tme_sparc_ls *, int));
void tme_sparc32_ls_address_map _TME_P((struct tme_sparc *, struct tme_sparc_ls *));
void tme_sparc32_ls_bus_cycle _TME_P((const struct tme_sparc *, struct tme_sparc_ls *));
void tme_sparc32_ls_trap _TME_P((struct tme_sparc *, struct tme_sparc_ls *));

/* trap support: */
void tme_sparc32_trap_preinstruction _TME_P((struct tme_sparc *, tme_uint32_t));
void tme_sparc32_trap _TME_P((struct tme_sparc *, tme_uint32_t));
void tme_sparc64_trap_preinstruction _TME_P((struct tme_sparc *, tme_uint32_t));
void tme_sparc64_trap _TME_P((struct tme_sparc *, tme_uint32_t));
void tme_sparc64_trap_error_state _TME_P((struct tme_sparc *));
void tme_sparc_nnpc_trap _TME_P((struct tme_sparc *, tme_uint32_t));

/* FPU support: */
int tme_sparc_fpu_new _TME_P((struct tme_sparc *, const char * const *, int *, int *, char **));
void tme_sparc_fpu_reset _TME_P((struct tme_sparc *));
void tme_sparc_fpu_usage _TME_P((struct tme_sparc *, char **));
void tme_sparc_fpu_strict _TME_P((struct tme_sparc_bus_connection *, unsigned int));
void tme_sparc_fpu_exception_check _TME_P((struct tme_sparc *));
unsigned int tme_sparc_fpu_fpreg_decode _TME_P((struct tme_sparc *, unsigned int, unsigned int));
void tme_sparc_fpu_fpreg_format _TME_P((struct tme_sparc *, unsigned int, unsigned int));
void tme_sparc_fpu_fpop1 _TME_P((struct tme_sparc *));
void tme_sparc_fpu_fpop2 _TME_P((struct tme_sparc *));
#ifdef TME_HAVE_INT64_T
void tme_sparc_vis _TME_P((struct tme_sparc *));
void tme_sparc64_vis_ls_asi_pst _TME_P((struct tme_sparc *, struct tme_sparc_ls *));
void tme_sparc64_vis_ls_asi_fl _TME_P((struct tme_sparc *, struct tme_sparc_ls *));
tme_uint32_t tme_sparc64_vis_ls_asi_misaligned _TME_P((struct tme_sparc *, tme_uint32_t));
#endif /* TME_HAVE_INT64_T */

/* timing support: */
int tme_sparc_timing_loop_ok _TME_P((tme_uint32_t, tme_uint32_t));
void tme_sparc_timing_loop_start _TME_P((struct tme_sparc *));
void tme_sparc_timing_loop_finish _TME_P((struct tme_sparc *));
#if TME_HAVE_RECODE
tme_recode_uguest_t tme_sparc_timing_loop_assist _TME_P((struct tme_ic *, tme_recode_uguest_t, tme_recode_uguest_t));
#endif /* TME_HAVE_RECODE */

/* recode support: */
void tme_sparc_recode_init _TME_P((struct tme_sparc *));
#if TME_HAVE_RECODE
tme_recode_thunk_off_t tme_sparc_recode _TME_P((struct tme_sparc *, const struct tme_sparc_tlb *, const tme_shared tme_uint32_t *));
void tme_sparc_recode_invalidate_all _TME_P((struct tme_sparc *));
tme_recode_uguest_t tme_sparc32_recode_insn_assist_redispatch _TME_P((struct tme_sparc *));
tme_uint32_t tme_sparc32_recode_insn_current _TME_P((const struct tme_sparc *));
void tme_sparc32_recode_chain_tlb_update _TME_P((struct tme_sparc *, const struct tme_sparc_ls *));
void tme_sparc32_recode_ls_tlb_update _TME_P((struct tme_sparc *, const struct tme_sparc_ls *));
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
tme_recode_uguest_t tme_sparc64_recode_insn_assist_redispatch _TME_P((struct tme_sparc *));
tme_uint32_t tme_sparc64_recode_insn_current _TME_P((const struct tme_sparc *));
void tme_sparc64_recode_chain_tlb_update _TME_P((struct tme_sparc *, const struct tme_sparc_ls *));
void tme_sparc64_recode_ls_tlb_update _TME_P((struct tme_sparc *, const struct tme_sparc_ls *));
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
#define TME_SPARC_RECODE_VERIFY_MEM_SIZE_MASK	(0x7)
#define TME_SPARC_RECODE_VERIFY_MEM_LOAD	TME_BIT(3)
#define TME_SPARC_RECODE_VERIFY_MEM_STORE	TME_BIT(4)
#define TME_SPARC_RECODE_VERIFY_MEM_CAS		TME_BIT(5)
#ifdef _TME_SPARC_RECODE_VERIFY
void tme_sparc_recode_verify_begin _TME_P((struct tme_sparc *));
void tme_sparc_recode_verify_mem _TME_P((struct tme_sparc *, void *, unsigned int, tme_recode_uguest_t, unsigned int));
void tme_sparc_recode_verify_mem_load _TME_P((struct tme_sparc *, const void *));
void tme_sparc_recode_verify_mem_block _TME_P((struct tme_sparc *, unsigned int));
void tme_sparc_recode_verify_reg_tick _TME_P((struct tme_sparc *, void *));
void tme_sparc_recode_verify_reg_tick_now _TME_P((struct tme_sparc *, const void *));
void tme_sparc_recode_verify_end _TME_P((struct tme_sparc *, tme_uint32_t));
void tme_sparc_recode_verify_end_preinstruction _TME_P((struct tme_sparc *));
#define tme_sparc_recode_verify_replay_last_pc(ic) ((ic)->tme_sparc_recode_verify_replay_last_pc)
#define TME_SPARC_RECODE_VERIFY_PC_NONE		(1)
#endif /* _TME_SPARC_RECODE_VERIFY */
#else  /* !TME_HAVE_RECODE */
#define tme_sparc_recode_invalidate_all(ic) do { } while (/* CONSTCOND */ 0 && (ic))
#endif /* !TME_HAVE_RECODE */
#ifndef _TME_SPARC_RECODE_VERIFY
#define tme_sparc_recode_verify_begin(ic) \
  do { } while (/* CONSTCOND */ 0 && (ic))
#define tme_sparc_recode_verify_mem(ic, rd, asi, addr, flags) \
  do { } while (/* CONSTCOND */ 0 && (ic) && (rd) && (asi) && (addr))
#define tme_sparc_recode_verify_mem_load(ic, rd) \
  do { } while (/* CONSTCOND */ 0 && (ic) && (rd))
#define tme_sparc_recode_verify_mem_block(ic, flags) \
  do { } while (/* CONSTCOND */ 0 && (ic))
#define tme_sparc_recode_verify_reg_tick(ic, rd) \
  do { } while (/* CONSTCOND */ 0 && (ic) && (rd))
#define tme_sparc_recode_verify_reg_tick_now(ic, rd) \
  do { } while (/* CONSTCOND */ 0 && (ic) && *(rd))
#define tme_sparc_recode_verify_end(ic, trap) \
  do { } while (/* CONSTCOND */ 0 && (ic) && (trap))
#define tme_sparc_recode_verify_end_preinstruction(ic) \
  do { } while (/* CONSTCOND */ 0 && (ic))
#define tme_sparc_recode_verify_replay_last_pc(ic) (0)
#endif /* !_TME_SPARC_RECODE_VERIFY */

/* instruction functions: */
TME_SPARC_FORMAT3_DECL(tme_sparc32_illegal, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_cpop1, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_cpop2, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_ldc, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_ldcsr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_lddc, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_stc, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_stcsr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_stdc, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_stdcq, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_rdasr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_rdpsr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_rdwim, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_rdtbr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_wrasr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_wrpsr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_wrwim, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_wrtbr, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_flush, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_rett, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_save_restore, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_ticc, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_stdfq, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_fpop1, tme_uint32_t);
TME_SPARC_FORMAT3_DECL(tme_sparc32_fpop2, tme_uint32_t);
#ifdef TME_HAVE_INT64_T
TME_SPARC_FORMAT3_DECL(tme_sparc64_movcc, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_movr, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_tcc, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_save_restore, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_return, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_saved_restored, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_flushw, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_prefetch, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_rdpr, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_wrpr, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_rdasr, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_wrasr, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_done_retry, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_fpop1, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_fpop2, tme_uint64_t);
TME_SPARC_FORMAT3_DECL(tme_sparc64_illegal_instruction, tme_uint64_t);
#endif /* TME_HAVE_INT64_T */

/* the automatically-generated header information: */
#include <sparc-auto.h>

#endif /* !_IC_SPARC_IMPL_H */
