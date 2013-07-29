/* $Id: recode-x86.c,v 1.5 2010/02/07 17:06:28 fredette Exp $ */

/* libtme/host/x86/recode-x86.c - recode code file for x86 hosts: */

/*
 * Copyright (c) 2007 Matt Fredette
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
_TME_RCSID("$Id: recode-x86.c,v 1.5 2010/02/07 17:06:28 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* the maximum number of bytes in an x86 instruction: */
#define TME_RECODE_X86_INSN_BYTES_MAX	(15)

/* register encodings: */
#define TME_RECODE_X86_REG_A	(0)
#define TME_RECODE_X86_REG_C	(1)
#define TME_RECODE_X86_REG_D	(2)
#define TME_RECODE_X86_REG_B	(3)
#define TME_RECODE_X86_REG_SP	(4)
#define TME_RECODE_X86_REG_BP	(5)
#define TME_RECODE_X86_REG_SI	(6)
#define TME_RECODE_X86_REG_DI	(7)
#define TME_RECODE_X86_REG_N(n)	(n)
#define TME_RECODE_X86_REG_XMM(n) (n)
#define TME_RECODE_X86_REG_UNDEF (64)

/* flags: */
#define TME_RECODE_X86_FLAG_Z				(1 << 6)

/* REX prefixes and register number masking: */
#if TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
#define _TME_RECODE_X86_REX(size, reg, n)	\
  ((((size) == TME_RECODE_SIZE_8		\
     && (reg) >= TME_RECODE_X86_REG_SP		\
     && (reg) <= TME_RECODE_X86_REG_DI)		\
    ? 0x40					\
    : 0x00)					\
   | ((size) > TME_RECODE_SIZE_32		\
      ? 0x48					\
      : 0x00)					\
   | ((reg) >= TME_RECODE_X86_REG_N(8)		\
      ? (0x40 | (1 << (n)))			\
      : 0x00))
#define TME_RECODE_X86_REG(x)				((x) & 7)
#else  /* TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32 */
#define _TME_RECODE_X86_REX(size, reg, n)		(0x00)
#define TME_RECODE_X86_REG(x)				(x)
#endif /* TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32 */
#define TME_RECODE_X86_REX_B(size, reg)			_TME_RECODE_X86_REX(size, reg, 0)
#define TME_RECODE_X86_REX_X(reg)			_TME_RECODE_X86_REX(0, reg, 1)
#define TME_RECODE_X86_REX_R(size, reg)			_TME_RECODE_X86_REX(size, reg, 2)
#define TME_RECODE_X86_REX_W(size)			_TME_RECODE_X86_REX(size, TME_RECODE_X86_REG_A, 3)

/* conditions: */
#define TME_RECODE_X86_COND_NOT				(1 << 0)
#define TME_RECODE_X86_COND_O				(0x0)
#define TME_RECODE_X86_COND_C				(0x2)
#define TME_RECODE_X86_COND_Z				(0x4)
#define TME_RECODE_X86_COND_BE				(0x6)
#define TME_RECODE_X86_COND_S				(0x8)
#define TME_RECODE_X86_COND_PE				(0xa)
#define TME_RECODE_X86_COND_L				(0xc)
#define TME_RECODE_X86_COND_LE				(0xe)

/* one-byte opcode table instructions: */
#define TME_RECODE_X86_OPCODE_BINOP_ADD			(0x00)
#define TME_RECODE_X86_OPCODE_BINOP_OR			(0x08)
#define TME_RECODE_X86_OPCODE_BINOP_ADC			(0x10)
#define TME_RECODE_X86_OPCODE_BINOP_SBB			(0x18)
#define TME_RECODE_X86_OPCODE_BINOP_AND			(0x20)
#define TME_RECODE_X86_OPCODE_BINOP_SUB			(0x28)
#define TME_RECODE_X86_OPCODE_BINOP_XOR			(0x30)
#define TME_RECODE_X86_OPCODE_BINOP_CMP			(0x38)
#define TME_RECODE_X86_OPCODE_PUSH_Gv(reg)		(0x50 + TME_RECODE_X86_REG(reg))
#define TME_RECODE_X86_OPCODE_POP_Gv(reg)		(0x58 + TME_RECODE_X86_REG(reg))
#define TME_RECODE_X86_OPCODE_MOVS_El_Gv		(0x63)
#define TME_RECODE_X86_PREFIX_OPSIZ			(0x66)
#define TME_RECODE_X86_OPCODE_PUSH_Ib			(0x6a)
#define TME_RECODE_X86_OPCODE_JCC(cond)			(0x70 + (cond))
#define TME_RECODE_X86_OPCODE_GRP1_Ib_Eb		(0x80)
#define TME_RECODE_X86_OPCODE_GRP1_Iz_Ev		(0x81)
#define TME_RECODE_X86_OPCODE_GRP1_Ib_Ev		(0x83)
#define TME_RECODE_X86_OPCODE_BINOP_TEST		(0x84)
#define TME_RECODE_X86_OPCODE_BINOP_XCHG		(0x86)
#define TME_RECODE_X86_OPCODE_BINOP_MOV			(0x88)
#define  TME_RECODE_X86_OPCODE_BINOP_Gb_Eb		 (0x0)
#define  TME_RECODE_X86_OPCODE_BINOP_Gv_Ev		 (0x1)
#define  TME_RECODE_X86_OPCODE_BINOP_Eb_Gb		 (0x2)
#define  TME_RECODE_X86_OPCODE_BINOP_Ev_Gv		 (0x3)
#define  TME_RECODE_X86_OPCODE_BINOP_Iz_A		 (0x5)
#define  TME_RECODE_X86_OPCODE_GRP1_BINOP(binop)	 ((binop) / 0x08)
#define TME_RECODE_X86_OPCODE_LEA			(0x8d)
#define TME_RECODE_X86_OPCODE_PUSHF			(0x9c)
#define TME_RECODE_X86_OPCODE_POPF			(0x9d)
#define TME_RECODE_X86_OPCODE_MOV_Iv_Gv(reg)		(0xb8 + TME_RECODE_X86_REG(reg))
#define TME_RECODE_X86_OPCODE_GRP2_Ib_Ev		(0xc1)
#define  TME_RECODE_X86_OPCODE_GRP2_ROR			 (0x1)
#define  TME_RECODE_X86_OPCODE_GRP2_SHL			 (0x4)
#define  TME_RECODE_X86_OPCODE_GRP2_SHR			 (0x5)
#define  TME_RECODE_X86_OPCODE_GRP2_SAR			 (0x7)
#define TME_RECODE_X86_OPCODE_RET			(0xc3)
#define TME_RECODE_X86_OPCODE_MOV_Ib_Eb			(0xc6)
#define TME_RECODE_X86_OPCODE_MOV_Iz_Ev			(0xc7)
#define TME_RECODE_X86_OPCODE_GRP2_1_Ev			(0xd1)
#define TME_RECODE_X86_OPCODE_CALL_RELz			(0xe8)
#define TME_RECODE_X86_OPCODE_JMP_RELz			(0xe9)
#define TME_RECODE_X86_OPCODE_JMP_RELb			(0xeb)
#define TME_RECODE_X86_PREFIX_REP			(0xf3)
#define TME_RECODE_X86_OPCODE_GRP3_Eb			(0xf6)
#define TME_RECODE_X86_OPCODE_GRP3_Ev			(0xf7)
#define  TME_RECODE_X86_OPCODE_GRP3_TEST		 (0x0)
#define  TME_RECODE_X86_OPCODE_GRP3_NOT			 (0x2)
#define  TME_RECODE_X86_OPCODE_GRP3_NEG			 (0x3)
#define TME_RECODE_X86_OPCODE_GRP5			(0xff)
#define  TME_RECODE_X86_OPCODE_GRP5_CALL		 (0x2)
#define  TME_RECODE_X86_OPCODE_GRP5_JMP			 (0x4)
#define  TME_RECODE_X86_OPCODE_GRP5_PUSH		 (0x6)

/* two-byte opcode table instructions: */
#define TME_RECODE_X86_OPCODE_ESC_0F			(0x0f)
#define TME_RECODE_X86_OPCODE0F_UD2			(0x0b)
#define TME_RECODE_X86_OPCODE0F_JCC(cond)		(0x80 + (cond))
#define TME_RECODE_X86_OPCODE0F_SETCC(cond)		(0x90 + (cond))
#define TME_RECODE_X86_OPCODE0F_SHRD_Ib_Gv_Ev		(0xac)
#define TME_RECODE_X86_OPCODE0F_GRP15			(0xae)
#define  TME_RECODE_X86_OPCODE0F_GRP15_MFENCE		TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(0), 6)
#define TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv		(0xb6)
#define TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv		(0xb7)
#define TME_RECODE_X86_OPCODE0F_MOVS_Eb_Gv		(0xbe)
#define TME_RECODE_X86_OPCODE0F_MOVS_Ew_Gv		(0xbf)
#define TME_RECODE_X86_OPCODE0F_BSWAP(reg)		(0xc8 + TME_RECODE_X86_REG(reg))

/* OPSIZ and REP two-byte opcode table instructions: */
#define TME_RECODE_X86_OPCODE660F_MOVDQA_Wdq_Vdq	(0x6f)
#define TME_RECODE_X86_OPCODEF30F_MOVDQU_Wdq_Vdq	(0x6f)
#define TME_RECODE_X86_OPCODE660F_MOVQ_Vq_Wq		(0xd6)
#define TME_RECODE_X86_OPCODEF30F_MOVQ_Wq_Vq		(0x7e)
#define TME_RECODE_X86_OPCODE660F_MOVDQA_Vdq_Wdq	(0x7f)
#define TME_RECODE_X86_OPCODEF30F_MOVDQU_Vdq_Wdq	(0x7f)

/* modR/M bytes: */
#define TME_RECODE_X86_MOD_OPREG_RM(mod_rm, opreg)	((mod_rm) + ((opreg) << 3))
#define TME_RECODE_X86_MOD_RM_EA(reg)			((0x0 << 6) + TME_RECODE_X86_REG(reg))
#define TME_RECODE_X86_MOD_RM_EA_DISP8(reg)		((0x1 << 6) + TME_RECODE_X86_REG(reg))
#define TME_RECODE_X86_MOD_RM_EA_DISP32(reg)		((0x2 << 6) + TME_RECODE_X86_REG(reg))
#define TME_RECODE_X86_MOD_RM_REG(reg)			((0x3 << 6) + TME_RECODE_X86_REG(reg))
#define TME_RECODE_X86_EA_BASE_SIB			TME_RECODE_X86_REG_SP
#define TME_RECODE_X86_EA_BASE_NONE			TME_RECODE_X86_REG_BP
#define TME_RECODE_X86_EA_BASE_IP			TME_RECODE_X86_EA_BASE_NONE

/* scale-index-base bytes: */
#define TME_RECODE_X86_SIB(base, index, scale)		\
  ((((((scale) - 1) - ((scale) == 4)) & 3) << 6)	\
   | (TME_RECODE_X86_REG(index) << 3)			\
   | TME_RECODE_X86_REG(base))
#define TME_RECODE_X86_SIB_INDEX_NONE			TME_RECODE_X86_REG_SP
#define TME_RECODE_X86_SIB_BASE_NONE			TME_RECODE_X86_REG_BP

/* multibyte NOP instructions: */
#define _TME_RECODE_X86_NOP2				\
  ((TME_RECODE_X86_OPCODE_BINOP_MOV			\
    + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev)		\
   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C), \
				  TME_RECODE_X86_REG_C)	\
      << 8))
#define _TME_RECODE_X86_NOP3				\
  (TME_RECODE_X86_OPCODE_LEA				\
   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_REG_C), \
				  TME_RECODE_X86_REG_C)	\
	 << 8)						\
   + (0x00 << 16))
#define _TME_RECODE_X86_NOP4				\
   (TME_RECODE_X86_OPCODE_LEA				\
    + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_EA_BASE_SIB), \
				   TME_RECODE_X86_REG_C)\
       << 8)						\
    + (TME_RECODE_X86_SIB(TME_RECODE_X86_REG_C, TME_RECODE_X86_SIB_INDEX_NONE, 1) \
       << 16)						\
    + (0x00 << 24))
#define TME_RECODE_X86_NOP3				\
  (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32		\
   ? (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST,	\
			   TME_RECODE_X86_REG_C)	\
      + (_TME_RECODE_X86_NOP2				\
	 << 8))						\
   : _TME_RECODE_X86_NOP3)
#define TME_RECODE_X86_NOP4				\
  (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32		\
   ? (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST,	\
			   TME_RECODE_X86_REG_C)	\
      + (_TME_RECODE_X86_NOP3				\
	 << 8))						\
   : _TME_RECODE_X86_NOP4)

/* fixed registers: */

/* we always use the b register to hold the struct tme_ic *: */
#define TME_RECODE_X86_REG_IC				TME_RECODE_X86_REG_B

/* we use the insn generic thunk offset to hold a subs thunk
   offset: */
#define tme_recode_x86_insn_subs_thunk_off tme_recode_insn_thunk_off

/* this returns the bit number of the first set bit in the value.  it
   returns zero if the value is zero.  this is slow, but it's only
   meant to be used at initialization time: */
static unsigned int
_tme_recode_x86_ffs(tme_recode_uguest_t value)
{
  unsigned int shift;

  shift = 0;
  if (value != 0) {
    for (; (value & 1) == 0; value >>= 1, shift++);
  }
  return (shift);
}

/* this returns the value with any first set bit in the value shifted
   down into the first byte.  only whole bytes of zero bits are
   shifted off.  this is slow, but it's only meant to be used at
   initialization time: */
static tme_recode_uguest_t
_tme_recode_x86_ffs_byte_shift(tme_recode_uguest_t value)
{
  return (value
	  >> (_tme_recode_x86_ffs(value)
	      & (0 - (unsigned int) 8)));
}

/* this starts more instructions: */
#define tme_recode_x86_insns_start(ic, thunk_bytes)			\
  do {									\
    thunk_bytes = (ic)->tme_recode_ic_thunk_build_next;			\
  } while (/* CONSTCOND */ 0)

/* this finishes instructions: */
#define tme_recode_x86_insns_finish(ic, thunk_bytes)			\
  do {									\
    assert (thunk_bytes <= (ic)->tme_recode_ic_thunk_build_end);	\
    (ic)->tme_recode_ic_thunk_build_next = thunk_bytes;			\
  } while (/* CONSTCOND */ 0)

/* this emits an instruction to adjust the stack pointer: */
static inline tme_uint8_t *
_tme_recode_x86_emit_adjust_sp(tme_uint8_t *thunk_bytes, int adjust)
{

  /* emit the add $imm, %sp: */
  assert (adjust <= 127 && adjust >= -128);
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, TME_RECODE_X86_REG_SP);
  }
  *((tme_uint16_t *) (thunk_bytes + (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32)))
    = (TME_RECODE_X86_OPCODE_GRP1_Ib_Ev
       + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_SP),
				      TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADD))
	  << 8));
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2] = (tme_int8_t) adjust;
  thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 3;
  return (thunk_bytes);
}

/* this gives the raw bytes for a binop instruction from one register into another: */
#define _tme_recode_x86_raw_reg_binop(binop, reg_x86_src, reg_x86_dst)		\
  ((TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_dst)			\
    | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86_src))			\
   + (((binop)									\
       + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev)					\
      << (8 * (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32)))			\
   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_dst),	\
				  TME_RECODE_X86_REG(reg_x86_src))		\
      << (8 + 8 * (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32))))

/* this emits an instruction that copies one register into another: */
#define _tme_recode_x86_emit_reg_copy(thunk_bytes, reg_x86_src, reg_x86_dst)		\
  _tme_recode_x86_emit_reg_binop(thunk_bytes, TME_RECODE_X86_OPCODE_BINOP_MOV, reg_x86_src, reg_x86_dst)

/* this emits a binop instruction from one register into another: */
#define _tme_recode_x86_emit_reg_binop(thunk_bytes, binop, reg_x86_src, reg_x86_dst)	\
  do {											\
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {					\
      *((tme_uint32_t *) (thunk_bytes))							\
        = _tme_recode_x86_raw_reg_binop(binop, reg_x86_src, reg_x86_dst); 		\
      (thunk_bytes) += 3;								\
    }											\
    else {										\
      *((tme_uint16_t *) (thunk_bytes))							\
        = (tme_uint16_t) _tme_recode_x86_raw_reg_binop(binop, reg_x86_src, reg_x86_dst);\
      (thunk_bytes) += 2;								\
    }											\
  } while (/* CONSTCOND */ 0)

/* this emits an instruction that pushes or pops a register: */
#define __tme_recode_x86_emit_reg_push_pop(thunk_bytes, reg_x86, opcode)		\
  do {											\
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32					\
	&& (reg_x86) >= TME_RECODE_X86_REG_N(8)) {					\
      *((tme_uint16_t *) (thunk_bytes))							\
	= (TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_N(8))				\
	   + ((opcode)									\
	      << 8));									\
      (thunk_bytes) += 2;								\
    }											\
    else {										\
      *(thunk_bytes) = (opcode);							\
      (thunk_bytes) += 1;								\
    }											\
  } while (/* CONSTCOND */ 0)

/* this emits an instruction that pushes a register: */
#define _tme_recode_x86_emit_reg_push(thunk_bytes, reg_x86)				\
  __tme_recode_x86_emit_reg_push_pop(thunk_bytes, reg_x86, TME_RECODE_X86_OPCODE_PUSH_Gv(reg_x86))

/* this emits an instruction that pops a register: */
#define _tme_recode_x86_emit_reg_pop(thunk_bytes, reg_x86)				\
  __tme_recode_x86_emit_reg_push_pop(thunk_bytes, reg_x86, TME_RECODE_X86_OPCODE_POP_Gv(reg_x86))

/* this emits a ModR/M byte with a constant displacement to reference
   a struct tme_ic *: */
static inline tme_uint8_t *
_tme_recode_x86_emit_ic_modrm(tme_uint8_t *thunk_bytes,
			      unsigned long disp,
			      tme_uint8_t opreg)
{
  unsigned int disp_size;
  tme_uint8_t mod_rm;

  /* on x86-64 the displacement must fit into 31 bits, because it's
     sign-extended: */
  assert (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	  || disp <= 0x7fffffff);

  /* assume that this must be a 32-bit displacement: */
  *((tme_uint32_t *) (thunk_bytes + 1)) = disp;
  disp_size = sizeof(tme_uint32_t);
  mod_rm = TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_REG_IC);

  /* if this can be an eight-bit displacement, adjust the displacement
     size and ModR/M byte: */
  if (disp < 0x80) {
    disp_size = sizeof(tme_uint8_t);
  }
  if (disp < 0x80) {
    mod_rm = TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_REG_IC);
  }

  /* write the ModR/M byte and return the end of the instruction: */
  thunk_bytes[0] = TME_RECODE_X86_MOD_OPREG_RM(mod_rm, TME_RECODE_X86_REG(opreg));
  return (thunk_bytes + 1 + disp_size);
}

/* this emits a call or a jmp to a C function: */
static void
_tme_recode_x86_emit_transfer_func(struct tme_recode_ic *ic,
				   unsigned int opcode_relz,
				   void (*func) _TME_P((void)))
{
  tme_uint8_t *thunk_bytes;
  tme_recode_thunk_off_t thunk_off;
  tme_int32_t relv;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* assume that we can emit a relative call or jmp, and get the
     relative offset to the C function: */
  thunk_bytes += 1 + sizeof(tme_int32_t);
  thunk_off = tme_recode_build_to_thunk_off(ic, thunk_bytes);
  relv = tme_recode_function_to_thunk_off(ic, func) - thunk_off;

  /* if this relative offset will reach the C function: */
  if (tme_recode_thunk_off_to_pointer(ic,
				      (thunk_off + relv),
				      void (*) _TME_P((void)))
      == func) {

    /* emit the relative call or jmp: */
    thunk_bytes[-(1 + sizeof(tme_int32_t))] = opcode_relz;
    ((tme_int32_t *) thunk_bytes)[-1] = relv;
  }

  /* otherwise, we have to do an indirect call or jmp: */
  else {

    /* we must be on an x86-64 host: */
    assert (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32);

    /* abort the relative call or jmp: */
    thunk_bytes -= 1 + sizeof(tme_int32_t);

    /* load the a register with the address of the C function: */
    thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, TME_RECODE_X86_REG_A);
    thunk_bytes[1] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(TME_RECODE_X86_REG_A);
    memset(&thunk_bytes[2], 0, TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));
    memcpy(&thunk_bytes[2], &func, sizeof(func));
    thunk_bytes += 2 + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);

    /* emit the indirect call or jmp: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP5;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_A),
				    (opcode_relz == TME_RECODE_X86_OPCODE_CALL_RELz
				     ? TME_RECODE_X86_OPCODE_GRP5_CALL
				     : TME_RECODE_X86_OPCODE_GRP5_JMP));
    thunk_bytes += 2;
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits a multiplication by a constant: */
/* NB: since this only does shifts and adds, this shouldn't be used
   for large factors: */
static tme_uint8_t *
_tme_recode_x86_emit_mul_constant(tme_uint8_t *thunk_bytes,
				  unsigned int reg_size,
				  unsigned int reg_x86_factor,
				  tme_recode_uguest_t constant_factor,
				  unsigned int reg_x86_scratch)
{
  unsigned int shift_count;
  unsigned int rex;
  unsigned int scale_factor;
  int need_pop;

  assert (constant_factor > 0);

  /* if the constant has any two factors in it: */
  shift_count = _tme_recode_x86_ffs(constant_factor);
  if (shift_count > 0) {

    /* multiply by the constant with the two factors removed: */
    thunk_bytes = _tme_recode_x86_emit_mul_constant(thunk_bytes,
						    reg_size,
						    reg_x86_factor,
						    constant_factor >> shift_count,
						    reg_x86_scratch);

    /* emit a shift for the two factors: */
    rex = TME_RECODE_X86_REX_B(reg_size, reg_x86_factor);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_factor),
				    TME_RECODE_X86_OPCODE_GRP2_SHL);
    thunk_bytes[2] = shift_count;
    thunk_bytes += 3;

    /* done: */
    return (thunk_bytes);
  }

  /* handle all of the nine, five, and three factors: */
  scale_factor = 8;
  for (; scale_factor > 1; ) {

    /* if this constant doesn't have any more of this factor: */
    if ((constant_factor % (1 + scale_factor)) != 0) {

      /* advance to the next factor: */
      scale_factor /= 2;
      continue;
    }

    /* emit an lea to multiply by this factor: */
    rex
      = (TME_RECODE_X86_REX_W(reg_size)
	 | TME_RECODE_X86_REX_R(reg_size, reg_x86_factor)
	 | TME_RECODE_X86_REX_X(reg_x86_factor)
	 | TME_RECODE_X86_REX_B(reg_size, reg_x86_factor));
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_LEA;
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB),
						 TME_RECODE_X86_REG(reg_x86_factor));
    thunk_bytes[2] = TME_RECODE_X86_SIB(reg_x86_factor, reg_x86_factor, scale_factor);
    thunk_bytes += 3;
    constant_factor /= (1 + scale_factor);
  }

  /* if we still have a constant: */
  if (constant_factor > 1) {

    /* if we need to, push the scratch register: */
    need_pop = (reg_x86_scratch > TME_RECODE_X86_REG_UNDEF);
    if (need_pop) {
      reg_x86_scratch -= TME_RECODE_X86_REG_UNDEF;
      _tme_recode_x86_emit_reg_push(thunk_bytes, reg_x86_scratch);
    }

    /* copy the register into the scratch register: */
    rex
      = (TME_RECODE_X86_REX_B(reg_size, reg_x86_factor)
	 | TME_RECODE_X86_REX_R(reg_size, reg_x86_scratch));
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_factor),
				    reg_x86_scratch);
    thunk_bytes += 2;

    /* if the constant ends in 11 binary, we will multiply by the
       constant plus one and then subtract one, otherwise we will
       multiply by the constant minus one and add one: */
    thunk_bytes
      = _tme_recode_x86_emit_mul_constant(thunk_bytes,
					  reg_size,
					  reg_x86_factor,
					  (constant_factor
					   + (constant_factor & 2)
					   - 1),
					  (TME_RECODE_X86_REG_UNDEF
					   + reg_x86_scratch));
    rex
      = (TME_RECODE_X86_REX_B(reg_size, reg_x86_scratch)
	 | TME_RECODE_X86_REX_R(reg_size, reg_x86_factor));
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0]
      = (TME_RECODE_X86_OPCODE_BINOP_Ev_Gv
	 + ((constant_factor & 2)
	    ? TME_RECODE_X86_OPCODE_BINOP_SUB
	    : TME_RECODE_X86_OPCODE_BINOP_ADD));
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_scratch),
				    reg_x86_factor);
    thunk_bytes += 2;

    /* if we need to, pop the scratch register: */
    if (need_pop) {
      _tme_recode_x86_emit_reg_pop(thunk_bytes, reg_x86_scratch);
    }
  }

  /* done: */
  return (thunk_bytes);
}

/* this emits a jmp or jcc: */
static tme_uint8_t *
_tme_recode_x86_emit_jmp(tme_uint8_t *thunk_bytes,
			 tme_uint32_t opcode,
			 const tme_uint8_t *thunk_bytes_target)
{
  int one_if_opcode0f_jcc;
  signed long disp;

  /* write the opcode: */
  *((tme_uint16_t *) thunk_bytes) = opcode;
  
  /* see if this is a six-byte jcc instruction: */
  one_if_opcode0f_jcc = ((opcode & 0xff) == TME_RECODE_X86_OPCODE_ESC_0F);

  /* the opcode must be for a jmp or jcc instruction: */
  assert (opcode == TME_RECODE_X86_OPCODE_JMP_RELb
	  || opcode == TME_RECODE_X86_OPCODE_JMP_RELz
	  || (opcode & 0xf0) == TME_RECODE_X86_OPCODE_JCC(0)
	  || (one_if_opcode0f_jcc
	      && (opcode >> 12) == (TME_RECODE_X86_OPCODE0F_JCC(0) >> 4)));

  /* if we know the jump target now: */
  if (thunk_bytes_target != NULL) {

    /* if the displacement can be a sign-extended eight bits: */
    disp = thunk_bytes_target - (thunk_bytes + 2);
    if (disp == (tme_int8_t) disp) {

      /* write the displacement: */
      thunk_bytes[1] = disp;

      /* if the given opcode uses a 32-bit displacement, convert it to
	 one the uses an 8-bit displacement: */
      if (opcode == TME_RECODE_X86_OPCODE_JMP_RELz) {
	opcode = TME_RECODE_X86_OPCODE_JMP_RELb;
      }
      else if (one_if_opcode0f_jcc) {
	opcode
	  = TME_RECODE_X86_OPCODE_JCC((opcode >> 8)
				      - TME_RECODE_X86_OPCODE0F_JCC(0));
      }
      one_if_opcode0f_jcc = 0;

      /* rewrite the possibly changed opcode: */
      thunk_bytes[0] = opcode;
    }

    /* otherwise, the displacement can't be a sign-extended eight bits: */
    else {

      /* the opcode must use a 32-bit displacement: */
      assert (opcode == TME_RECODE_X86_OPCODE_JMP_RELz
	      || one_if_opcode0f_jcc);

      /* the displacement must fit in a sign-extended 32 bits: */
      disp = (thunk_bytes_target
	      - (thunk_bytes
		 + 1
		 + one_if_opcode0f_jcc
		 + sizeof(tme_int32_t)));
      assert (disp == (tme_int32_t) disp);

      /* write the displacement: */
      *((tme_int32_t *) &thunk_bytes[1 + one_if_opcode0f_jcc]) = disp;
    }
  }

  /* advance: */
  thunk_bytes += 1 + one_if_opcode0f_jcc + 1;
  if (opcode == TME_RECODE_X86_OPCODE_JMP_RELz
      || one_if_opcode0f_jcc) {
    thunk_bytes += sizeof(tme_uint32_t) - 1;
  }

  return (thunk_bytes);
}

/* this fixes up a jmp or jcc: */
static void
_tme_recode_x86_fixup_jmp(tme_uint8_t *thunk_bytes,
			  const tme_uint8_t *thunk_bytes_target)
{
  tme_uint8_t opcode;
  int one_if_opcode0f_jcc;
  signed long disp;

  /* get the first byte of the opcode: */
  opcode = thunk_bytes[0];

  /* see if this is a six-byte jcc instruction: */
  one_if_opcode0f_jcc = (opcode == TME_RECODE_X86_OPCODE_ESC_0F);

  /* if the opcode uses a 32-bit displacement: */
  if (opcode == TME_RECODE_X86_OPCODE_JMP_RELz
      || one_if_opcode0f_jcc) {

    /* the displacement must fit in a sign-extended 32 bits: */
    disp = (thunk_bytes_target
	    - (thunk_bytes
	       + 1
	       + one_if_opcode0f_jcc
	       + sizeof(tme_int32_t)));
    assert (disp == (tme_int32_t) disp);

    /* write the displacement: */
    *((tme_int32_t *) &thunk_bytes[1 + one_if_opcode0f_jcc]) = disp;
  }

  /* otherwise, the opcode uses an 8-bit displacement: */
  else {

    /* the displacement must fit in a sign-extended eight bits: */
    disp = thunk_bytes_target - (thunk_bytes + 2);
    assert (disp == (tme_int8_t) disp);

    /* write the displacement: */
    thunk_bytes[1] = disp;
  }
}

/* prototypes: */

/* this emits instructions for a chain in: */
static void _tme_recode_x86_chain_in _TME_P((struct tme_recode_ic *, const struct tme_recode_insns_group *));

/* this emits instructions for a chain out: */
static void _tme_recode_x86_chain_out _TME_P((struct tme_recode_ic *, const struct tme_recode_insns_group *));

/* include the other code files: */
#include "host/x86/rc-x86-subs.c"
#include "host/x86/rc-x86-regs.c"
#include "host/x86/rc-x86-conds.c"
#include "host/x86/rc-x86-insns.c"
#include "host/x86/rc-x86-flags.c"
#include "host/x86/rc-x86-tlb.c"
#include "host/x86/rc-x86-rws.c"
#include "host/x86/rc-x86-chain.c"

/* this host function starts a new IC: */
void
tme_recode_host_ic_new(struct tme_recode_ic *ic)
{
  tme_uint8_t *thunk_bytes;
  tme_recode_thunk_off_t thunk_off;

  /* make the chain epilogue: */
  _tme_recode_x86_chain_epilogue(ic);

  /* copy the hand-coded subs: */
  tme_recode_x86_insns_start(ic, thunk_bytes);
  thunk_off = tme_recode_build_to_thunk_off(ic, thunk_bytes);
  memcpy(thunk_bytes, _tme_recode_x86_subs, sizeof(_tme_recode_x86_subs));
  thunk_bytes += sizeof(_tme_recode_x86_subs);
  tme_recode_x86_insns_finish(ic, thunk_bytes);
  tme_recode_host_thunk_finish(ic);

  /* set the thunk offsets of the shift insn subs: */
#if (TME_RECODE_OPCODE_SHLL + 1) != TME_RECODE_OPCODE_SHRL || (TME_RECODE_OPCODE_SHRL + 1) != TME_RECODE_OPCODE_SHRA
#error "TME_RECODE_OPCODE_ values changed"
#endif
#define _tme_recode_x86_set_subs_shift(size, opcode, subs)		\
  ic->tme_recode_x86_ic_subs_shift					\
    [_TME_CONCAT(TME_RECODE_SIZE_,size) - TME_RECODE_SIZE_8]		\
    [_TME_CONCAT(TME_RECODE_OPCODE_,opcode) - TME_RECODE_OPCODE_SHLL]	\
    = (thunk_off + _TME_CONCAT3(tme_recode_x86_,subs,size))
  _tme_recode_x86_set_subs_shift(8, SHLL, shll);
  _tme_recode_x86_set_subs_shift(8, SHRL, shrl);
  _tme_recode_x86_set_subs_shift(8, SHRA, shra);
  _tme_recode_x86_set_subs_shift(16, SHLL, shll);
  _tme_recode_x86_set_subs_shift(16, SHRL, shrl);
  _tme_recode_x86_set_subs_shift(16, SHRA, shra);
  _tme_recode_x86_set_subs_shift(32, SHLL, shll);
  _tme_recode_x86_set_subs_shift(32, SHRL, shrl);
  _tme_recode_x86_set_subs_shift(32, SHRA, shra);
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
  _tme_recode_x86_set_subs_shift(64, SHLL, shll);
  _tme_recode_x86_set_subs_shift(64, SHRL, shrl);
  _tme_recode_x86_set_subs_shift(64, SHRA, shra);
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
#if TME_RECODE_SIZE_GUEST_MAX > (TME_RECODE_SIZE_32 + 1)
  _tme_recode_x86_set_subs_shift(128, SHLL, shll);
  _tme_recode_x86_set_subs_shift(128, SHRL, shrl);
  _tme_recode_x86_set_subs_shift(128, SHRA, shra);
#endif /* TME_RECODE_SIZE_GUEST_MAX > (TME_RECODE_SIZE_32 + 1) */
#undef _tme_recode_x86_set_subs_shift

  /* check the value of TME_RECODE_HOST_INSN_SIZE_MAX: */
  _tme_recode_x86_insn_size_max_check();
}

#endif /* TME_HAVE_RECODE */
