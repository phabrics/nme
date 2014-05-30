/* $Id: m68k-impl.h,v 1.19 2009/08/29 19:28:08 fredette Exp $ */

/* ic/m68k/m68k-impl.h - implementation header file for Motorola 68k emulation: */

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

#ifndef _IC_M68K_IMPL_H
#define _IC_M68K_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: m68k-impl.h,v 1.19 2009/08/29 19:28:08 fredette Exp $");

/* includes: */
#include <tme/ic/m68k.h>
#include <tme/ic/ieee754.h>
#include <tme/generic/ic.h>
#include <setjmp.h>

/* macros: */

/* CPUs: */
#define TME_M68K_M68000		(0)
#define TME_M68K_M68010		(1)
#define TME_M68K_M68020		(2)
#define TME_M68K_M68030		(3)
#define TME_M68K_M68040		(4)

/* FPUs: */
#define TME_M68K_FPU_NONE	(0)
#define TME_M68K_FPU_M68881	TME_BIT(0)
#define TME_M68K_FPU_M68882	TME_BIT(1)
#define TME_M68K_FPU_M6888X	(TME_M68K_FPU_M68881 | TME_M68K_FPU_M68882)
#define TME_M68K_FPU_M68040	TME_BIT(2)
#define TME_M68K_FPU_ANY	(TME_M68K_FPU_M68881 | TME_M68K_FPU_M68882 | TME_M68K_FPU_M68040)

/* generic registers: */
#define tme_m68k_ireg_uint32(x)	tme_m68k_ic.tme_ic_ireg_uint32(x)
#define tme_m68k_ireg_int32(x)	tme_m68k_ic.tme_ic_ireg_int32(x)
#define tme_m68k_ireg_uint16(x)	tme_m68k_ic.tme_ic_ireg_uint16(x)
#define tme_m68k_ireg_int16(x)	tme_m68k_ic.tme_ic_ireg_int16(x)
#define tme_m68k_ireg_uint8(x)	tme_m68k_ic.tme_ic_ireg_uint8(x)
#define tme_m68k_ireg_int8(x)	tme_m68k_ic.tme_ic_ireg_int8(x)

/* flags: */
#define TME_M68K_FLAG_C 	TME_BIT(0)
#define TME_M68K_FLAG_V 	TME_BIT(1)
#define TME_M68K_FLAG_Z 	TME_BIT(2)
#define TME_M68K_FLAG_N 	TME_BIT(3)
#define TME_M68K_FLAG_X 	TME_BIT(4)
#define TME_M68K_FLAG_IPM(x)	TME_FIELD_EXTRACTU(x, 8, 3)
#define TME_M68K_FLAG_M		TME_BIT(12)
#define TME_M68K_FLAG_S		TME_BIT(13)
#define TME_M68K_FLAG_T0	TME_BIT(14)
#define TME_M68K_FLAG_T1	TME_BIT(15)
#define TME_M68K_FLAG_CCR	(TME_M68K_FLAG_C | TME_M68K_FLAG_V | \
				 TME_M68K_FLAG_Z | TME_M68K_FLAG_N | \
				 TME_M68K_FLAG_X)
#define TME_M68K_FLAG_SR	(TME_M68K_FLAG_CCR | (0x7 << 8) | \
				 TME_M68K_FLAG_M | TME_M68K_FLAG_S | \
				 TME_M68K_FLAG_T0 | TME_M68K_FLAG_T1)

/* conditions: */
#define TME_M68K_C_T	(0)
#define TME_M68K_C_F	(1)
#define TME_M68K_C_HI	(2)
#define TME_M68K_C_LS	(3)
#define TME_M68K_C_CC	(4)
#define TME_M68K_C_CS	(5)
#define TME_M68K_C_NE	(6)
#define TME_M68K_C_EQ	(7)
#define TME_M68K_C_VC	(8)
#define TME_M68K_C_VS	(9)
#define TME_M68K_C_PL	(10)
#define TME_M68K_C_MI	(11)
#define TME_M68K_C_GE	(12)
#define TME_M68K_C_LT	(13)
#define TME_M68K_C_GT	(14)
#define TME_M68K_C_LE	(15)
#define TME_M68K_COND_TRUE(ic, c) (_tme_m68k_conditions[(ic)->tme_m68k_ireg_ccr] & TME_BIT(c))

/* bus cycles: */
#define TME_M68K_BUS_CYCLE_NORMAL		(0)
#define TME_M68K_BUS_CYCLE_READ			TME_BIT(0)
#define TME_M68K_BUS_CYCLE_FETCH		TME_BIT(1)
#define TME_M68K_BUS_CYCLE_RMW			TME_BIT(2)
#define TME_M68K_BUS_CYCLE_RAW			TME_BIT(3)

/* exceptions: */
#define TME_M68K_EXCEPTION_NONE			(0)
#define TME_M68K_EXCEPTION_RESET		TME_BIT(0)
#define TME_M68K_EXCEPTION_AERR			TME_BIT(1)
#define TME_M68K_EXCEPTION_BERR			TME_BIT(2)
#define TME_M68K_EXCEPTION_TRACE		TME_BIT(3)
#define TME_M68K_EXCEPTION_INT(ipl, vec)	(((ipl) << 4) | ((vec) << 7))
#define TME_M68K_EXCEPTION_IS_INT(x)		(((x) >> 4) & 0x7)
#define TME_M68K_EXCEPTION_INT_VEC(x)		(((x) >> 7) & 0xff)
#define TME_M68K_EXCEPTION_ILL			TME_BIT(15)
#define TME_M68K_EXCEPTION_PRIV			TME_BIT(16)
#define TME_M68K_EXCEPTION_INST(x)		((x) << 17)
#define TME_M68K_EXCEPTION_IS_INST(x)		((x) >> 17)

/* exception frame formats: */
#define TME_M68K_FORMAT_0	(0)
#define TME_M68K_FORMAT_1	(1)
#define TME_M68K_FORMAT_2	(2)
#define TME_M68K_FORMAT_8	(8)
#define TME_M68K_FORMAT_B	(0xB)

/* exception vectors: */
#define TME_M68K_VECTOR_RESET_PC	(0x00)
#define TME_M68K_VECTOR_RESET_SP	(0x01)
#define TME_M68K_VECTOR_BERR		(0x02)
#define TME_M68K_VECTOR_AERR		(0x03)
#define TME_M68K_VECTOR_ILL		(0x04)
#define TME_M68K_VECTOR_DIV0		(0x05)
#define TME_M68K_VECTOR_CHK		(0x06)
#define TME_M68K_VECTOR_TRAP		(0x07)
#define TME_M68K_VECTOR_PRIV		(0x08)
#define TME_M68K_VECTOR_TRACE		(0x09)
#define TME_M68K_VECTOR_LINE_A		(0x0a)
#define TME_M68K_VECTOR_LINE_F		(0x0b)
					/* (0x0c is unassigned, reserved) */
#define TME_M68K_VECTOR_COPROC		(0x0d)
#define TME_M68K_VECTOR_FORMAT		(0x0e)
#define TME_M68K_VECTOR_INT_UNINIT	(0x0f)
					/* (0x10 through 0x17 are unassigned, reserved) */
#define TME_M68K_VECTOR_SPURIOUS	(0x18)
					/* (0x19 through 0x1f are the interrupt autovectors) */
#define TME_M68K_VECTOR_TRAP_0		(0x20)
					/* (0x21 through 0x2f are also TRAP vectors) */

/* m6888x type specifiers: */
#define TME_M6888X_TYPE_LONG		(0)
#define TME_M6888X_TYPE_SINGLE		(1)
#define TME_M6888X_TYPE_EXTENDED80	(2)
#define TME_M6888X_TYPE_PACKEDDEC	(3)
#define TME_M6888X_TYPE_WORD		(4)
#define TME_M6888X_TYPE_DOUBLE		(5)
#define TME_M6888X_TYPE_BYTE		(6)
#define TME_M6888X_TYPE_PACKEDDEC_DK	(7)
#define TME_M6888X_TYPE_INVALID		(TME_M6888X_TYPE_PACKEDDEC_DK)

/* sizes: */
#define TME_M68K_SIZE_UNDEF	(-1)
#define TME_M68K_SIZE_UNSIZED	(0)
#define TME_M68K_SIZE_8		(1)
#define TME_M68K_SIZE_16	(2)
				/* 3 unused */
#define TME_M68K_SIZE_32	(4)
				/* 5 unused */
				/* 6 unused */
				/* 7 unused */
#define TME_M68K_SIZE_64	(8)
				/* 8 unused */
				/* 9 unused */
				/* 10 unused */
				/* 11 unused */
#define TME_M68K_SIZE_96	(12)

/* opcode parameters: */

/* bits 0 through 7, in the opcode map, are the instruction index.
   immediately after the initial decoding, these bits are replaced
   with the least significant 8 bits of the opcode word, making bits 0
   through 2 any EA reg value, and bits 3 through 5 any EA mode
   value: */
#define TME_M68K_OPCODE_INSN(x)		((x) << 0)
#define TME_M68K_OPCODE_INSN_MASK	TME_M68K_OPCODE_INSN(0xff)
#define TME_M68K_OPCODE_INSN_WHICH(params)	TME_FIELD_MASK_EXTRACTU(params, TME_M68K_OPCODE_INSN(0xff))
#define TME_M68K_OPCODE_EA_REG(x)      ((x) << 0)
#define TME_M68K_OPCODE_EA_MODE(x)     ((x) << 3)
#define TME_M68K_OPCODE_EA_REG_MASK    TME_M68K_OPCODE_EA_REG(0x7)
#define TME_M68K_OPCODE_EA_MODE_MASK   TME_M68K_OPCODE_EA_MODE(0x7)
#define TME_M68K_OPCODE_EA_MODE_WHICH(params)  TME_FIELD_MASK_EXTRACTU(params, TME_M68K_OPCODE_EA_MODE_MASK)
#define TME_M68K_OPCODE_EA_REG_WHICH(params)   TME_FIELD_MASK_EXTRACTU(params, TME_M68K_OPCODE_EA_REG_MASK)

/* bits 8 through 15 and 16 through 23 are the operands' ic offsets: */
#define _TME_M68K_OPCODE_OP(f)		((tme_uint8_t) (((tme_uint8_t *) &((struct tme_m68k *) 0)->f) - ((tme_uint8_t *) (struct tme_m68k *) 0)))
#define TME_M68K_OPCODE_OP1(f)		(_TME_M68K_OPCODE_OP(f) << 8)
#define TME_M68K_OPCODE_OP0(f)		(_TME_M68K_OPCODE_OP(f) << 16)
#define TME_M68K_OPCODE_OP1_WHICH(ic, params) (((tme_uint8_t *) (ic)) + TME_FIELD_MASK_EXTRACTU(params, 0x0000ff00))
#define TME_M68K_OPCODE_OP0_WHICH(ic, params) (((tme_uint8_t *) (ic)) + TME_FIELD_MASK_EXTRACTU(params, 0x00ff0000))

/* bits 24 and 25 give the size of any immediate operand: */
#define TME_M68K_OPCODE_IMM_16		TME_BIT(24)
#define TME_M68K_OPCODE_IMM_32		TME_BIT(25)
#define TME_M68K_OPCODE_HAS_IMM(params)	(((params) & (TME_M68K_OPCODE_IMM_16 | TME_M68K_OPCODE_IMM_32)) != 0)

/* bit 26 is set for a move instruction with a memory destination
   ((TME_M68K_OPCODE_SPECOP is additionally set for a
   move-memory-to-memory instruction).
   bit 27 is set for an opcode that needs its EA read.
   bit 28 is set for an opcode that needs its EA written.

   if the EA needs to be read or written, bits 29 and 30 give the size
   of the EA, which can be TME_M68K_SIZE_8, TME_M68K_SIZE_16, or
   TME_M68K_SIZE_32, encoded as (TME_M68K_SIZE_32 - size).  if the EA
   doesn't need to be read or written, but the EA must be calculated,
   bit 29 is set, and size of the EA is understood to be
   TME_M68K_SIZE_UNSIZED: */
#define TME_M68K_OPCODE_EA_Y		TME_BIT(26)
#define TME_M68K_OPCODE_EA_READ		TME_BIT(27)
#define TME_M68K_OPCODE_EA_WRITE	TME_BIT(28)
#define TME_M68K_OPCODE_EA_UNSIZED	TME_BIT(29)
#define TME_M68K_OPCODE_EA_SIZE(x)	((TME_M68K_SIZE_32 - (x)) * TME_M68K_OPCODE_EA_UNSIZED)
#define TME_M68K_OPCODE_EA_SIZE_MASK	TME_M68K_OPCODE_EA_SIZE(1)
#define TME_M68K_OPCODE_HAS_EA(params)	((params) & (TME_M68K_OPCODE_EA_READ | TME_M68K_OPCODE_EA_WRITE | TME_M68K_OPCODE_EA_UNSIZED))
#define TME_M68K_OPCODE_EA_SIZE_WHICH(params)					\
	(((params) & (TME_M68K_OPCODE_EA_READ | TME_M68K_OPCODE_EA_WRITE)) ?	\
	 (TME_M68K_SIZE_32							\
	  - TME_FIELD_MASK_EXTRACTU(params, TME_M68K_OPCODE_EA_SIZE_MASK))	\
	 : TME_M68K_SIZE_UNSIZED)

/* if the opcode is special, bit 31 is set: */
#define TME_M68K_OPCODE_SPECOP		TME_BIT(31)

/* major modes of the emulator: */
#define TME_M68K_MODE_EXECUTION	(0)
#define TME_M68K_MODE_EXCEPTION	(1)
#define TME_M68K_MODE_RTE	(2)
#define TME_M68K_MODE_STOP	(3)
#define TME_M68K_MODE_HALT	(4)

/* mode-specific flags: */
#define TME_M68K_EXECUTION_INST_CANFAULT	TME_BIT(0)

/* given a linear address, this hashes it into a TLB entry: */
#define _TME_M68K_TLB_HASH_SIZE (1024)
#define TME_M68K_TLB_ADDRESS_BIAS(n)	((n) << 10)
#define TME_M68K_DTLB_ENTRY(ic, context, function_code, linear_address)	\
  (((ic)->_tme_m68k_tlb_array					\
    + ((((context) * 16)			\
	+ ((linear_address)			\
	   / TME_M68K_TLB_ADDRESS_BIAS(1)))	\
       % _TME_M68K_TLB_HASH_SIZE))		\
   + (0 && (function_code)))

/* macros for sequence control: */
#define TME_M68K_SEQUENCE_START						\
do {									\
  ic->_tme_m68k_sequence._tme_m68k_sequence_mode_flags = 0;		\
  ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted = 0;	\
  ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next = 1;		\
} while (/* CONSTCOND */ 0)
#define TME_M68K_SEQUENCE_RESTARTING					\
  (ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted		\
   >= ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next)
#define TME_M68K_SEQUENCE_RESTART					\
do {									\
  if (!TME_M68K_SEQUENCE_RESTARTING)					\
    ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted =	\
      ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next;		\
  ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next = 1;		\
} while (/* CONSTCOND */ 0)
#define TME_M68K_SEQUENCE_TRANSFER_STEP					\
do {									\
  ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next++;		\
} while (/* CONSTCOND */ 0)

/* instruction handler macros: */
#define TME_M68K_INSN_DECL(name) void name _TME_P((struct tme_m68k *, void *, void *))
#ifdef __STDC__
#define TME_M68K_INSN(name) void name(struct tme_m68k *ic, void *_op0, void *_op1)
#else  /* !__STDC__ */
#define TME_M68K_INSN(name) void name(ic, _op0, _op1) struct tme_m68k *ic; void *_op0, *_op1;
#endif /* !__STDC__ */
#define TME_M68K_INSN_OP0(t) 		(*((t *) _op0))
#define TME_M68K_INSN_OP1(t) 		(*((t *) _op1))
#define TME_M68K_INSN_OPCODE		ic->_tme_m68k_insn_opcode
#define TME_M68K_INSN_SPECOP		ic->_tme_m68k_insn_specop
#define TME_M68K_INSN_OK		return
#define TME_M68K_INSN_EXCEPTION(e)	tme_m68k_exception(ic, e)
#define TME_M68K_INSN_PRIV			\
  if (!TME_M68K_PRIV(ic))	\
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_PRIV)
#define TME_M68K_INSN_ILL			\
  TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL)
#define TME_M68K_INSN_CANFAULT			\
do {						\
  ic->_tme_m68k_mode_flags			\
    |= TME_M68K_EXECUTION_INST_CANFAULT;	\
} while (/* CONSTCOND */ 0)
#define TME_M68K_INSN_BRANCH(pc)				\
do {								\
  tme_m68k_verify_end_branch(ic, pc);				\
  ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next = (pc);	\
  if (__tme_predict_false((ic->tme_m68k_ireg_sr			\
			   & ic->_tme_m68k_sr_mask_t) != 0)) {	\
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_TRACE);		\
  }								\
  if (tme_m68k_go_slow(ic)) {					\
    TME_M68K_SEQUENCE_START;					\
    tme_m68k_redispatch(ic);					\
  }								\
} while (/* CONSTCOND */ 0)
#define TME_M68K_INSN_CHANGE_SR(reg)				\
do {								\
  TME_M68K_INSN_PRIV;						\
  tme_m68k_change_sr(ic, reg);					\
  tme_m68k_verify_end_branch(ic, ic->tme_m68k_ireg_pc_next);	\
  if (__tme_predict_false((ic->tme_m68k_ireg_sr			\
			   & ic->_tme_m68k_sr_mask_t) != 0)) {	\
    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;		\
    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;		\
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_TRACE);		\
  }								\
  if (tme_m68k_go_slow(ic)) {					\
    TME_M68K_SEQUENCE_START;					\
    tme_m68k_redispatch(ic);					\
  }								\
} while (/* CONSTCOND */ 0)

/* logging: */
#define TME_M68K_LOG_HANDLE(ic)					\
  (&(ic)->tme_m68k_element->tme_element_log_handle)
#define tme_m68k_log_start(ic, level, rc)			\
do {								\
  tme_log_start(TME_M68K_LOG_HANDLE(ic), level, rc) {		\
    if ((ic)->_tme_m68k_mode != TME_M68K_MODE_EXECUTION) {	\
      tme_log_part(TME_M68K_LOG_HANDLE(ic),			\
                   "mode=%d ",					\
                   (ic)->_tme_m68k_mode);			\
    }								\
    else {							\
      tme_log_part(TME_M68K_LOG_HANDLE(ic),			\
	           "pc=%c/0x%08x ",				\
	           (((ic)->tme_m68k_ireg_sr 			\
		     & (TME_M68K_FLAG_M				\
		        | TME_M68K_FLAG_S))			\
		    ? 'S'					\
		    : 'U'),					\
		   ic->tme_m68k_ireg_pc);			\
    }								\
    do
#define tme_m68k_log_finish(ic)					\
    while (/* CONSTCOND */ 0);					\
  } tme_log_finish(TME_M68K_LOG_HANDLE(ic));			\
} while (/* CONSTCOND */ 0)
#define tme_m68k_log(ic, level, rc, x)		\
do {						\
  tme_m68k_log_start(ic, level, rc) {		\
    tme_log_part x;				\
  } tme_m68k_log_finish(ic);			\
} while (/* CONSTCOND */ 0)

/* miscellaneous: */
#define TME_M68K_PRIV(ic)	((ic)->tme_m68k_ireg_sr & TME_M68K_FLAG_S)
#define TME_M68K_FUNCTION_CODE_DATA(ic)	\
  (TME_M68K_PRIV(ic) ? TME_M68K_FC_SD : TME_M68K_FC_UD)
#define TME_M68K_FUNCTION_CODE_PROGRAM(ic)	\
  (TME_M68K_PRIV(ic) ? TME_M68K_FC_SP : TME_M68K_FC_UP)
#define TME_M68K_INSN_WORDS_MAX	(11)

/* structures: */
struct tme_m68k;

/* an memory read or write function: */
typedef void (*_tme_m68k_xfer_memx) _TME_P((struct tme_m68k *));
typedef void (*_tme_m68k_xfer_mem) _TME_P((struct tme_m68k *, int));

/* an insn function: */
typedef TME_M68K_INSN_DECL((*_tme_m68k_insn));

/* an m68k sequence: */
struct _tme_m68k_sequence {
  
  /* the mode of the emulator: */
  unsigned int _tme_m68k_sequence_mode;

  /* any mode-specific flags for the sequence: */
  unsigned int _tme_m68k_sequence_mode_flags;
  
  /* the ordinal of the next memory transfer.  always starts from one: */
  unsigned short _tme_m68k_sequence_transfer_next;

  /* the ordinal of the memory transfer that faulted.  if this is
     greater than or equal to _tme_m68k_sequence_transfer_next, we are
     restarting, and bus cycles and changes to the m68k state are
     forbidden: */
  unsigned short _tme_m68k_sequence_transfer_faulted;

  /* the fault happened after this number of bytes were successfully
     transferred: */
  unsigned short _tme_m68k_sequence_transfer_faulted_after;

#ifdef _TME_M68K_VERIFY
  /* the sequence unique identifier: */
  unsigned long _tme_m68k_sequence_uid;
#endif /* _TME_M68K_VERIFY */
};

/* the m68k state: */
struct tme_m68k {

  /* the IC data structure.  it is beneficial to have this structure
     first, since register numbers can often simply be scaled and 
     added without an offset to the struct tme_m68k pointer to get
     to their contents: */
  struct tme_ic tme_m68k_ic;

  /* the m68k type: */
  int tme_m68k_type;

  /* the backpointer to our element: */
  struct tme_element *tme_m68k_element;

  /* our bus connection.  if both are defined, the m68k bus connection
     is an adaptation layer for the generic bus connection: */
  struct tme_m68k_bus_connection *_tme_m68k_bus_connection;
  struct tme_bus_connection *_tme_m68k_bus_generic;

  /* a jmp_buf back to the dispatcher: */
  jmp_buf _tme_m68k_dispatcher;

  /* the current sequence: */
  struct _tme_m68k_sequence _tme_m68k_sequence;
#define _tme_m68k_mode _tme_m68k_sequence._tme_m68k_sequence_mode
#define _tme_m68k_mode_flags _tme_m68k_sequence._tme_m68k_sequence_mode_flags
#define _tme_m68k_insn_uid _tme_m68k_sequence._tme_m68k_sequence_uid

  /* the CPU-dependent functions for the different modes: */
  void (*_tme_m68k_mode_execute) _TME_P((struct tme_m68k *));
  void (*_tme_m68k_mode_exception) _TME_P((struct tme_m68k *));
  void (*_tme_m68k_mode_rte) _TME_P((struct tme_m68k *));

  /* the CPU-dependent status register T bits mask: */
  tme_uint16_t _tme_m68k_sr_mask_t;

  /* the instruction burst count, and the remaining burst: */
  unsigned int _tme_m68k_instruction_burst;
  unsigned int _tme_m68k_instruction_burst_remaining;

  /* the effective address: */
#define _tme_m68k_ea_address tme_m68k_ireg_ea
  unsigned int _tme_m68k_ea_function_code;

  /* instruction fetch information: */
  tme_uint16_t _tme_m68k_insn_fetch_buffer[TME_M68K_INSN_WORDS_MAX];
#define _tme_m68k_insn_opcode _tme_m68k_insn_fetch_buffer[0]
#define _tme_m68k_insn_specop _tme_m68k_insn_fetch_buffer[1]
  struct tme_m68k_tlb *_tme_m68k_insn_fetch_fast_itlb;
  const tme_shared tme_uint8_t *_tme_m68k_insn_fetch_fast_start;
  const tme_shared tme_uint8_t *_tme_m68k_insn_fetch_fast_next;
  const tme_shared tme_uint8_t *_tme_m68k_insn_fetch_fast_last;
  unsigned int _tme_m68k_insn_fetch_slow_next;
  unsigned int _tme_m68k_insn_fetch_slow_count_fast;
  unsigned int _tme_m68k_insn_fetch_slow_count_total;

  /* the TLB set: */
  struct tme_m68k_tlb _tme_m68k_tlb_array[_TME_M68K_TLB_HASH_SIZE + 1];
#define _tme_m68k_itlb _tme_m68k_tlb_array[_TME_M68K_TLB_HASH_SIZE]

  /* the bus context: */
  tme_bus_context_t _tme_m68k_bus_context;

  /* exception handling information: */
  tme_uint32_t _tme_m68k_exceptions;

  /* this must be one iff this CPU has a 16-bit bus, else zero: */
  tme_uint32_t _tme_m68k_bus_16bit;

  /* group 0 exception information: */
  void (*_tme_m68k_group0_hook) _TME_P((struct tme_m68k *));
  unsigned int _tme_m68k_group0_flags;
  unsigned int _tme_m68k_group0_function_code;
  tme_uint32_t _tme_m68k_group0_address;
  struct _tme_m68k_sequence _tme_m68k_group0_sequence;
  tme_uint8_t _tme_m68k_group0_buffer_read[sizeof(tme_uint32_t)];
  unsigned int _tme_m68k_group0_buffer_read_size;
  unsigned int _tme_m68k_group0_buffer_read_softrr;
  tme_uint8_t _tme_m68k_group0_buffer_write[sizeof(tme_uint32_t)];
  unsigned int _tme_m68k_group0_buffer_write_size;
  unsigned int _tme_m68k_group0_buffer_write_softrr;

  /* the external request lines: */
  tme_mutex_t tme_m68k_external_mutex;
  tme_cond_t tme_m68k_external_cond;
  unsigned int tme_m68k_external_reset;
  unsigned int tme_m68k_external_halt;
  unsigned int tme_m68k_external_ipl;
  unsigned int tme_m68k_external_ipl_previous_nmi;

  /* any FPU state: */
  int tme_m68k_fpu_enabled;
  int tme_m68k_fpu_type;
  struct tme_ieee754_ctl tme_m68k_fpu_ieee754_ctl;
  _tme_const struct tme_ieee754_ops *tme_m68k_fpu_ieee754_ops;
  struct tme_float tme_m68k_fpu_fpreg[8];
  tme_uint32_t tme_m68k_fpu_fpcr;
  tme_uint32_t tme_m68k_fpu_fpsr;
  tme_uint32_t tme_m68k_fpu_fpiar;
  int tme_m68k_fpu_incomplete_abort;

#ifdef _TME_M68K_STATS
  /* statistics: */
  struct {

    /* the total number of instructions executed: */
    tme_uint64_t tme_m68k_stats_insns_total;

    /* the total number of instructions executed by the slow executor: */
    tme_uint64_t tme_m68k_stats_insns_slow;

    /* the total number of redispatches: */
    tme_uint64_t tme_m68k_stats_redispatches;

    /* the total number of data memory operations: */
    tme_uint64_t tme_m68k_stats_memory_total;

    /* the total number of ITLB fills: */
    tme_uint64_t tme_m68k_stats_itlb_fill;

    /* the total number of DTLB fills: */
    tme_uint64_t tme_m68k_stats_dtlb_fill;

  } tme_m68k_stats;
#endif /* _TME_M68K_STATS */
};

/* the read-modify-write cycle state: */
struct tme_m68k_rmw {

  /* the operand size: */
  unsigned int tme_m68k_rmw_size;     

  /* the address count, and up to two addresses: */
  unsigned int tme_m68k_rmw_address_count;
  tme_uint32_t tme_m68k_rmw_addresses[2];

  /* if nonzero, the operand at the corresponding address has been
     read with a slow bus cycle.  address zero is read into the memx
     register, and address one is read into the memy register: */
  unsigned int tme_m68k_rmw_slow_reads[2];

  /* the TLB entries used by the addresses.  if two addresses are
     sharing one TLB entry, that TLB entry is listed twice: */
  struct tme_m68k_tlb *tme_m68k_rmw_tlbs[2];
};

/* globals: */
extern const tme_uint16_t _tme_m68k_conditions[32];
extern const _tme_m68k_xfer_memx _tme_m68k_read_memx[5];
extern const _tme_m68k_xfer_memx _tme_m68k_write_memx[5];
extern const _tme_m68k_xfer_mem _tme_m68k_read_mem[5];
extern const _tme_m68k_xfer_mem _tme_m68k_write_mem[5];
extern tme_uint32_t tme_m68k_opcodes_m68000[65536];
extern tme_uint32_t tme_m68k_opcodes_m68010[65536];
extern tme_uint32_t tme_m68k_opcodes_m68020[65536];
extern const _tme_m68k_insn tme_m68k_opcode_insns[];
extern const tme_uint8_t _tme_m6888x_fpgen_opmode_bitmap[128 / 8];

/* prototypes: */
int tme_m68k_new _TME_P((struct tme_m68k *, const char * const *, const void *, char **));
void tme_m68k_redispatch _TME_P((struct tme_m68k *));
int tme_m68k_go_slow _TME_P((const struct tme_m68k *));
void tme_m68k_change_sr _TME_P((struct tme_m68k *, tme_uint16_t));
void tme_m68k_external_check _TME_P((struct tme_m68k *, tme_uint32_t));
void tme_m68k_tlb_fill _TME_P((struct tme_m68k *, struct tme_m68k_tlb *, unsigned int, tme_uint32_t, unsigned int));
void tme_m68k_do_reset _TME_P((struct tme_m68k *));
void tme_m68k_callout_unlock _TME_P((struct tme_m68k *ic));
void tme_m68k_callout_relock _TME_P((struct tme_m68k *ic));

/* exception support: */
void tme_m68k_exception _TME_P((struct tme_m68k *, tme_uint32_t));
void tme_m68k_exception_process_start _TME_P((struct tme_m68k *, unsigned int));
void tme_m68k_exception_process_finish _TME_P((struct tme_m68k *, tme_uint8_t, tme_uint8_t));
void tme_m68000_exception_process _TME_P((struct tme_m68k *));
void tme_m68020_exception_process _TME_P((struct tme_m68k *));

/* rte support: */
tme_uint16_t tme_m68k_rte_start _TME_P((struct tme_m68k *));
void tme_m68k_rte_finish _TME_P((struct tme_m68k *, tme_uint32_t));

/* decoder map support: */
void tme_m68k_opcodes_init_m68000 _TME_P((tme_uint32_t *));
void tme_m68k_opcodes_init_m68010 _TME_P((tme_uint32_t *));
void tme_m68k_opcodes_init_m68020 _TME_P((tme_uint32_t *));

/* read/modify/write cycle support: */
int tme_m68k_rmw_start _TME_P((struct tme_m68k *, struct tme_m68k_rmw *));
void tme_m68k_rmw_finish _TME_P((struct tme_m68k *, struct tme_m68k_rmw *, int));

/* group 0 fault support: */
void tme_m68k_group0_hook_fast _TME_P((struct tme_m68k *));
unsigned int tme_m68k_insn_buffer_empty _TME_P((const struct tme_m68k *, tme_uint8_t *, unsigned int));
unsigned int tme_m68k_insn_buffer_fill _TME_P((struct tme_m68k *, const tme_uint8_t *, unsigned int));
unsigned int tme_m68k_sequence_empty _TME_P((const struct tme_m68k *, tme_uint8_t *, unsigned int));
unsigned int tme_m68k_sequence_fill _TME_P((struct tme_m68k *, const tme_uint8_t *, unsigned int));

/* bitfield support: */
unsigned int tme_m68k_bitfield_width _TME_P((struct tme_m68k *));
tme_uint32_t _tme_m68k_bitfield_read _TME_P((struct tme_m68k *, int));
#define tme_m68k_bitfield_read_signed(ic) ((tme_int32_t) _tme_m68k_bitfield_read(ic, TRUE))
#define tme_m68k_bitfield_read_unsigned(ic) _tme_m68k_bitfield_read(ic, FALSE)
void tme_m68k_bitfield_write_unsigned _TME_P((struct tme_m68k *, tme_uint32_t, int));
#define tme_m68k_bitfield_write_signed(ic, v, sf) tme_m68k_bitfield_write_unsigned(ic, (tme_uint32_t) (v), sf)

/* FPU support: */
int tme_m68k_fpu_new _TME_P((struct tme_m68k *, const char * const *, int *, int *, char **));
void tme_m68k_fpu_reset _TME_P((struct tme_m68k *));
void tme_m68k_fpu_usage _TME_P((char **));

/* verification: */
void tme_m68k_verify_hook _TME_P((void));
#ifdef _TME_M68K_VERIFY
void tme_m68k_verify_init _TME_P((void));
void tme_m68k_verify_begin _TME_P((const struct tme_m68k *, const tme_uint8_t *));
void tme_m68k_verify_mem_any _TME_P((const struct tme_m68k *,
				     unsigned int, tme_uint32_t,
				     tme_uint8_t *, int, int));
void tme_m68k_verify_end_branch _TME_P((const struct tme_m68k *, tme_uint32_t));
void tme_m68k_verify_end _TME_P((const struct tme_m68k *,
				 void (*)(struct tme_m68k *, void *, void *)));
#else  /* _TME_M68K_VERIFY */
#define tme_m68k_verify_init() do { } while (/* CONSTCOND */ 0)
#define tme_m68k_verify_begin(ic, s) do { } while (/* CONSTCOND */ 0)
#define tme_m68k_verify_mem_any(ic, fc, a, v, c, rw) do { } while (/* CONSTCOND */ 0)
#define tme_m68k_verify_end_branch(ic, pc) do { } while (/* CONSTCOND */ 0)
#define tme_m68k_verify_end(ic, f) do { } while (/* CONSTCOND */ 0)
#define tme_m68k_verify_hook() do { } while (/* CONSTCOND */ 0)
#endif /* _TME_M68K_VERIFY */
#define tme_m68k_verify_mem8(ic, fc, a, v, rw) tme_m68k_verify_mem_any(ic, fc, a, (tme_uint8_t *) &(v), -sizeof(tme_uint8_t), rw)
#define tme_m68k_verify_mem16(ic, fc, a, v, rw) tme_m68k_verify_mem_any(ic, fc, a, (tme_uint8_t *) &(v), -sizeof(tme_uint16_t), rw)
#define tme_m68k_verify_mem32(ic, fc, a, v, rw) tme_m68k_verify_mem_any(ic, fc, a, (tme_uint8_t *) &(v), -sizeof(tme_uint32_t), rw)

/* instruction functions: */
TME_M68K_INSN_DECL(tme_m68k_illegal);
TME_M68K_INSN_DECL(tme_m68k_exg);
TME_M68K_INSN_DECL(tme_m68k_extw);
TME_M68K_INSN_DECL(tme_m68k_extl);
TME_M68K_INSN_DECL(tme_m68k_extbl);
TME_M68K_INSN_DECL(tme_m68k_lea);
TME_M68K_INSN_DECL(tme_m68k_move_from_ccr);
TME_M68K_INSN_DECL(tme_m68k_move_from_sr);
TME_M68K_INSN_DECL(tme_m68k_move_from_sr0);
TME_M68K_INSN_DECL(tme_m68k_swap);
TME_M68K_INSN_DECL(tme_m68k_nop);
TME_M68K_INSN_DECL(tme_m68k_scc);
TME_M68K_INSN_DECL(tme_m68k_dbcc);
TME_M68K_INSN_DECL(tme_m68k_bcc);
TME_M68K_INSN_DECL(tme_m68k_bccl);
TME_M68K_INSN_DECL(tme_m68k_bsr);
TME_M68K_INSN_DECL(tme_m68k_bsrl);
TME_M68K_INSN_DECL(tme_m68k_pea);
TME_M68K_INSN_DECL(tme_m68k_bkpt);
TME_M68K_INSN_DECL(tme_m68k_tas);
TME_M68K_INSN_DECL(tme_m68k_tas_r);
TME_M68K_INSN_DECL(tme_m68k_move_usp);
TME_M68K_INSN_DECL(tme_m68k_trap);
TME_M68K_INSN_DECL(tme_m68k_trapv);
TME_M68K_INSN_DECL(tme_m68k_link);
TME_M68K_INSN_DECL(tme_m68k_unlk);
TME_M68K_INSN_DECL(tme_m68k_movec);
TME_M68K_INSN_DECL(tme_m68k_reset);
TME_M68K_INSN_DECL(tme_m68k_rtd);
TME_M68K_INSN_DECL(tme_m68k_rtr);
TME_M68K_INSN_DECL(tme_m68k_rts);
TME_M68K_INSN_DECL(tme_m68k_jsr);
TME_M68K_INSN_DECL(tme_m68k_jmp);
TME_M68K_INSN_DECL(tme_m68k_rte);
TME_M68K_INSN_DECL(tme_m68k_stop);
TME_M68K_INSN_DECL(tme_m68k_priv);
TME_M68K_INSN_DECL(tme_m68k_cmp2_chk2);
TME_M68K_INSN_DECL(tme_m68k_callm);
TME_M68K_INSN_DECL(tme_m68k_rtm);
TME_M68K_INSN_DECL(tme_m68k_trapcc);
TME_M68K_INSN_DECL(tme_m68k_bfchg);
TME_M68K_INSN_DECL(tme_m68k_bfclr);
TME_M68K_INSN_DECL(tme_m68k_bfexts);
TME_M68K_INSN_DECL(tme_m68k_bfextu);
TME_M68K_INSN_DECL(tme_m68k_bfffo);
TME_M68K_INSN_DECL(tme_m68k_bfins);
TME_M68K_INSN_DECL(tme_m68k_bfset);
TME_M68K_INSN_DECL(tme_m68k_bftst);
TME_M68K_INSN_DECL(tme_m68k_pack);
TME_M68K_INSN_DECL(tme_m68k_unpk);
TME_M68K_INSN_DECL(tme_m68k_fpgen);
TME_M68K_INSN_DECL(tme_m68k_fmovemctl);
TME_M68K_INSN_DECL(tme_m68k_fmovem);
TME_M68K_INSN_DECL(tme_m68k_fmove_rm);
TME_M68K_INSN_DECL(tme_m68k_fdbcc);
TME_M68K_INSN_DECL(tme_m68k_ftrapcc);
TME_M68K_INSN_DECL(tme_m68k_fscc);
TME_M68K_INSN_DECL(tme_m68k_fbcc);
TME_M68K_INSN_DECL(tme_m68k_fsave);
TME_M68K_INSN_DECL(tme_m68k_frestore);
TME_M68K_INSN_DECL(tme_m68k_divl);
TME_M68K_INSN_DECL(tme_m68k_mull);

#endif /* !_IC_M68K_IMPL_H */

/* the automatically-generated header information: */
#include <m68k-auto.h>
