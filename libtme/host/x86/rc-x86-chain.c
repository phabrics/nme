/* $Id: rc-x86-chain.c,v 1.3 2010/02/15 22:15:59 fredette Exp $ */

/* libtme/host/x86/rc-x86-chain.c - x86 host recode chain support: */

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

_TME_RCSID("$Id: rc-x86-chain.c,v 1.3 2010/02/15 22:15:59 fredette Exp $");

/* macros: */

/* the x86 register for the pointer to the token for the current
   instruction TLB entry: */
#define TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN	(TME_RECODE_X86_REG_TLB)

/* the x86 register for the chain return address: */
#define TME_RECODE_X86_REG_CHAIN_RETURN_ADDRESS		(TME_RECODE_X86_REG_TLB)

/* the x86 register for the chain return address stack pointer: */
#define TME_RECODE_X86_REG_CHAIN_RAS_POINTER		(TME_RECODE_X86_REG_TLB_SCRATCH)

/* the x86 register for the fixup address: */
#define TME_RECODE_X86_REG_CHAIN_FIXUP_ADDRESS		(TME_RECODE_X86_REG_TLB_SCRATCH)

/* the x86 register for the guest instructions source address: */
/* NB: we use the a register to take advantage of the fact that there
   is a special cmp $imm, %a form that will save a byte in each
   instruction thunk: */
#define TME_RECODE_X86_REG_CHAIN_GUEST_SRC		(TME_RECODE_X86_REG_A)

/* this gives the one or two opcode bytes for a cmp $imm32,
   %TME_RECODE_X86_REG_CHAIN_GUEST_SRC: */
#if TME_RECODE_X86_REG_CHAIN_GUEST_SRC != TME_RECODE_X86_REG_A
#error "TME_RECODE_X86_REG_CHAIN_GUEST_SRC changed"
#endif
#define TME_RECODE_X86_INSN_CMP_Iz_REG_CHAIN_GUEST_SRC		\
  (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST,			\
			TME_RECODE_X86_REG_CHAIN_GUEST_SRC)	\
   + ((TME_RECODE_X86_OPCODE_BINOP_CMP				\
       + TME_RECODE_X86_OPCODE_BINOP_Iz_A)			\
      << (8 * (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32))))

/* this gives the one opcode byte for a movl $imm32, %TME_RECODE_X86_REG_TLB_SCRATCH: */
#if TME_RECODE_X86_REG(TME_RECODE_X86_REG_TLB_SCRATCH) != TME_RECODE_X86_REG_TLB_SCRATCH
#error "TME_RECODE_X86_REG_TLB_SCRATCH changed"
#endif
#define TME_RECODE_X86_INSN_MOVL_IMM32_REG_TLB_SCRATCH		\
  TME_RECODE_X86_OPCODE_MOV_Iv_Gv(TME_RECODE_X86_REG_TLB_SCRATCH)

/* this gives the three opcode bytes for a lea disp32(%ip),
   %TME_RECODE_X86_REG_TLB_SCRATCH: */
#define TME_RECODE_X86_INSN_LEA_DISP32_IP_REG_TLB_SCRATCH	\
  (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST,			\
			TME_RECODE_X86_REG_TLB_SCRATCH)		\
   + (TME_RECODE_X86_OPCODE_LEA					\
      << 8)							\
   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_IP),\
				  TME_RECODE_X86_REG(TME_RECODE_X86_REG_TLB_SCRATCH))\
      << 16))

/* this gives the two opcode bytes for a movq $imm64,
   %TME_RECODE_X86_REG_TLB_SCRATCH: */
#define TME_RECODE_X86_INSN_MOVQ_IMM64_REG_TLB_SCRATCH		\
  (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST,			\
			TME_RECODE_X86_REG_TLB_SCRATCH)		\
   + (TME_RECODE_X86_OPCODE_MOV_Iv_Gv(TME_RECODE_X86_REG_TLB_SCRATCH)\
      << 8))

/* this gives the three opcode bytes for a testb $imm8, addr32: */
#define TME_RECODE_X86_INSN_TESTB_IMM8_ADDR32			\
  (TME_RECODE_X86_OPCODE_GRP3_Eb				\
   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB),\
				  TME_RECODE_X86_OPCODE_GRP3_TEST)\
      << 8)							\
   + (TME_RECODE_X86_SIB(TME_RECODE_X86_SIB_BASE_NONE,		\
			 TME_RECODE_X86_SIB_INDEX_NONE,		\
			 1)					\
      << 16))

/* this gives the two opcode bytes for a testb $imm8, disp32(%ip): */
#define TME_RECODE_X86_INSN_TESTB_IMM8_DISP32_IP		\
  (TME_RECODE_X86_OPCODE_GRP3_Eb				\
   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_IP),\
				  TME_RECODE_X86_OPCODE_GRP3_TEST)\
      << 8))

/* this gives the two opcode bytes for a testb $imm8,
   disp32(%TME_RECODE_X86_REG_CHAIN_GUEST_SRC): */
#if TME_RECODE_X86_REG(TME_RECODE_X86_REG_CHAIN_GUEST_SRC) != TME_RECODE_X86_REG_CHAIN_GUEST_SRC
#error "TME_RECODE_X86_REG_CHAIN_GUEST_SRC changed"
#endif
#define TME_RECODE_X86_INSN_TESTB_IMM8_DISP32_REG_CHAIN_GUEST_SRC \
  (TME_RECODE_X86_OPCODE_GRP3_Eb				\
   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_REG_CHAIN_GUEST_SRC),\
				  TME_RECODE_X86_OPCODE_GRP3_TEST)\
      << 8))

/* this is the size of a jcc epilogue instruction: */
#define TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE	\
  (6	/* jcc epilogue */)

/* this is the size of the chain in far instructions on an ia32 host: */
#define TME_RECODE_IA32_CHAIN_IN_SIZE_FAR		\
  (5	/* cmpl $imm32, %eax */				\
   + TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE		\
   + 7  /* testb $imm8, addr32 */			\
   + TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE)

/* the size of cmpq $imm32, %TME_RECODE_X86_REG_CHAIN_GUEST_SRC ; jnz
   epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_CMPQ_IMM32_JNZ	\
  (6	/* cmpq $imm32, %rax */				\
   + TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE)

/* the size of cmpq %reg, %reg ; jnz epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_CMP_JNZ		\
  (3	/* cmpq %reg, %reg */				\
   + TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE)

/* the size of movl $imm32, %reg ; cmpq %reg, %reg ; jnz epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVL_CMP_JNZ	\
  (5	/* movl $imm32, %reg */				\
   + TME_RECODE_X86_64_CHAIN_IN_SIZE_CMP_JNZ)

/* the size of lea disp32(%ip), %reg ; cmpq %reg, %reg ; jnz epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_LEA_IP_CMP_JNZ	\
  (7	/* leaq disp32(%ip), %reg */			\
   + TME_RECODE_X86_64_CHAIN_IN_SIZE_CMP_JNZ)

/* the size of movq $imm64, %reg ; cmpq %reg, %reg ; jnz epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVQ_CMP_JNZ	\
  (10	/* movq $imm64, %reg */				\
   + TME_RECODE_X86_64_CHAIN_IN_SIZE_CMP_JNZ)

/* the size of testb $imm8, addr32 ; jz epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_ADDR32_JZ	\
  (8	/* testb $imm8, addr32 */			\
   + TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE)

/* the size of testb $imm8, base32(%reg) ; jz epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_BASE_JZ	\
  (7	/* testb $imm8, addr32 */			\
   + TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE)

/* the size of movq $imm64, %reg ; testb $imm32, (%reg) ; jz epilogue */
#define TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVQ_TESTB_JZ	\
  (10	/* movq $imm64, %reg */				\
   + 3	/* testb $imm32, (%reg) */			\
   + TME_RECODE_X86_CHAIN_IN_SIZE_JCC_EPILOGUE)

/* this gives a chain subs: */
#if (TME_RECODE_CHAIN_INFO_CONDITIONAL | TME_RECODE_CHAIN_INFO_FAR) != 3
#error "TME_RECODE_CHAIN_INFO_ values changed"
#endif
#if (TME_RECODE_CHAIN_INFO_JUMP != 4 || TME_RECODE_CHAIN_INFO_RETURN != 8)
#error "TME_RECODE_CHAIN_INFO_ values changed"
#endif
#define TME_RECODE_X86_CHAIN_SUBS(chain_thunk, chain_info)\
  ((chain_thunk)->tme_recode_x86_chain_thunk_subs	\
   [(chain_info)					\
    & (TME_RECODE_CHAIN_INFO_CONDITIONAL		\
       | TME_RECODE_CHAIN_INFO_FAR			\
       | TME_RECODE_CHAIN_INFO_JUMP			\
       | TME_RECODE_CHAIN_INFO_RETURN)])

/* these give a chain fixup target for the regular target or alternate
   target: */
#if TME_RECODE_CHAIN_INFO_FAR != 2 || TME_RECODE_CHAIN_INFO_ALTERNATE_FAR == 0
#error "TME_RECODE_CHAIN_INFO_ values changed"
#endif
#define TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic, chain_info) \
  ((ic)->tme_recode_x86_ic_chain_fixup_target		\
   [((chain_info) & TME_RECODE_CHAIN_INFO_FAR)		\
    + (((chain_info) / TME_RECODE_CHAIN_INFO_CALL) & 1)])
#define TME_RECODE_X86_CHAIN_FIXUP_TARGET_ALTERNATE(ic, chain_info) \
  ((ic)->tme_recode_x86_ic_chain_fixup_target		\
   [4							\
    + (((chain_info) / TME_RECODE_CHAIN_INFO_ALTERNATE_FAR) & 1)])

/* on an ia32 host, an entry on the return address stack is the
   absolute address of either an instructions thunk or the chain
   epilogue.  on an x86-64 host, an entry on the return address stack
   is the instructions thunk offset of either an instructions thunk or
   the chain epilogue: */
#if TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
#define TME_RECODE_X86_CHAIN_RETURN_ADDRESS(ic, insns_thunk)\
  ((tme_recode_ras_entry_t) (insns_thunk))
#else  /* TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32 */
#define TME_RECODE_X86_CHAIN_RETURN_ADDRESS(ic, insns_thunk)\
  tme_recode_thunk_off_to_pointer(ic,		    	\
				  insns_thunk,		\
				  tme_recode_ras_entry_t)
#endif /* TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32 */

/* this emits instructions for a chain epilogue: */
static void
_tme_recode_x86_chain_epilogue(struct tme_recode_ic *ic)
{
  tme_uint8_t *thunk_bytes;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* set the instructions thunk offset of the chain epilogue: */
  ic->tme_recode_x86_ic_chain_epilogue = tme_recode_build_to_thunk_off(ic, thunk_bytes);

  /* remove the recode carry flag stack word: */
  thunk_bytes
    = _tme_recode_x86_emit_adjust_sp(thunk_bytes,
				     TME_BIT(TME_RECODE_SIZE_HOST
					     - TME_RECODE_SIZE_8));

  /* pop all callee-saved registers: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(15));
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(14));
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(13));
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(12));
  }
  else {
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_DI);
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_SI);
  }
  _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_B);
  _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_BP);

  /* emit the return: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_RET;
  thunk_bytes++;

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* finish the chain epilogue: */
  tme_recode_host_thunk_finish(ic);
}

/* this emits instructions to advance the fixup address register
   past a chain conditional six-byte jcc instruction: */
static tme_uint8_t *
_tme_recode_x86_chain_fixup_alternate(tme_uint8_t *thunk_bytes)
{

  /* NB: this is always a 32-bit instruction, even on an x86-64 host,
     because we end up converting the fixup address into an
     instructions thunk offset, which is only 32 bits: */
  assert (sizeof(tme_recode_thunk_off_t) == sizeof(tme_int32_t));
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
  thunk_bytes[1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_CHAIN_FIXUP_ADDRESS),
				  TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADD));
  thunk_bytes[2] = 2 + sizeof(tme_int32_t);
  thunk_bytes += 3;
  return (thunk_bytes);
}

/* this emits instructions to make a constant for the third argument
   to the chain fixup function: */
static tme_uint8_t *
_tme_recode_x86_chain_fixup_arg2(tme_uint8_t *thunk_bytes,
				 tme_uint32_t chain_info)
{

  /* if this is an ia32 host: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

    /* push the argument: */
    assert (chain_info < 0x80);
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_PUSH_Ib;
    thunk_bytes[1] = chain_info;
    thunk_bytes += 2;
  }

  /* otherwise, this is an x86-64 host: */
  else {

    /* load the third argument register: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(TME_RECODE_X86_REG_D);
    *((tme_uint32_t *) &thunk_bytes[1]) = chain_info;
    thunk_bytes += 1 + sizeof(tme_uint32_t);
  }

  return (thunk_bytes);
}   

/* this emits instructions for the chain fixup targets for chain jump
   far, chain call far, and chain call near: */
static void
_tme_recode_x86_chain_fixup_target(struct tme_recode_ic *ic,
				   tme_uint32_t chain_info,
				   const tme_uint8_t *thunk_bytes_fixup_call)
{
  tme_uint8_t *thunk_bytes;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* set next fixup target: */
  TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic,
				    chain_info)
    = tme_recode_build_to_thunk_off(ic, thunk_bytes);

  /* make the third argument for the chain fixup function: */
  thunk_bytes = _tme_recode_x86_chain_fixup_arg2(thunk_bytes, chain_info);

  /* jmp to the common chain fixup call point: */
  thunk_bytes
    = _tme_recode_x86_emit_jmp(thunk_bytes,
			       TME_RECODE_X86_OPCODE_JMP_RELz,
			       thunk_bytes_fixup_call);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this makes the chain fixup targets for a new IC: */
static void
_tme_recode_x86_chain_fixup_targets(struct tme_recode_ic *ic)
{
  tme_uint8_t *thunk_bytes;
  const tme_uint8_t *thunk_bytes_fixup_call;
  unsigned long thunk_address0;
  unsigned int stack_adjust;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* the first fixup target is for chain jump alternate near: */
  TME_RECODE_X86_CHAIN_FIXUP_TARGET_ALTERNATE(ic,
					      (TME_RECODE_CHAIN_INFO_JUMP
					       + TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR))
    = tme_recode_build_to_thunk_off(ic, thunk_bytes);

  /* advance the fixup address register past the chain jump
     conditional six-byte jcc instruction: */
  thunk_bytes = _tme_recode_x86_chain_fixup_alternate(thunk_bytes);

  /* the next fixup target is for chain jump near: */
  /* NB: this fixup target is also for chain return alternate near,
     which is treated like a chain jump near: */
  TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic,
				    (TME_RECODE_CHAIN_INFO_JUMP
				     + TME_RECODE_CHAIN_INFO_NEAR))
    = tme_recode_build_to_thunk_off(ic, thunk_bytes);

  /* make the third argument for the chain fixup function: */
  thunk_bytes
    = _tme_recode_x86_chain_fixup_arg2(thunk_bytes,
				       TME_RECODE_CHAIN_INFO_NEAR);

  /* this is the common chain fixup call point: */
  thunk_bytes_fixup_call = thunk_bytes;

  /* get the near address of the thunk at offset zero: */
  thunk_address0 = tme_recode_thunk_off_to_pointer(ic, 0, char *) - (char *) 0;

  /* convert the fixup address (which has been already truncated to 32
     bits) into the instructions thunk offset: */
  /* NB: this is always a 32-bit instruction, because a
     tme_recode_thunk_off_t is a tme_int32_t: */
  assert (sizeof(tme_recode_thunk_off_t) == sizeof(tme_int32_t));
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Iz_Ev;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_CHAIN_FIXUP_ADDRESS),
					       TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_SUB));
  *((tme_uint32_t *) &thunk_bytes[2]) = thunk_address0;
  thunk_bytes += 2 + sizeof(tme_uint32_t);

  /* if this is an ia32 host: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

    /* push the arguments for the chain fixup function: */
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_CHAIN_FIXUP_ADDRESS);
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_IC);

    /* we will need to remove the arguments from the stack: */
    stack_adjust = sizeof(struct tme_ic *) + sizeof(tme_recode_thunk_off_t) + sizeof(tme_uint32_t);
  }

  /* otherwise, this is an x86-64 host: */
  else {

    /* copy the arguments for the guest jump chain function: */
    _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_IC, TME_RECODE_X86_REG_DI);
    _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_CHAIN_FIXUP_ADDRESS, TME_RECODE_X86_REG_SI);

    /* we don't need to adjust the stack.  NB that the instructions
       thunk, where the stack pointer is 16-byte aligned, jumped to
       us, so we don't have to align it for the x86-64 ABI: */
    stack_adjust = 0;
  }

  /* call the chain fixup function: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
  _tme_recode_x86_emit_transfer_func(ic,
				     TME_RECODE_X86_OPCODE_CALL_RELz,
				     ((void (*) _TME_P((void)))
				      ic->tme_recode_ic_chain_fixup));
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* do any stack adjust: */
  if (stack_adjust) {
    thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, stack_adjust);
  }

  /* jmp to the chain epilogue: */
  thunk_bytes
    = _tme_recode_x86_emit_jmp(thunk_bytes,
			       TME_RECODE_X86_OPCODE_JMP_RELz,
			       tme_recode_thunk_off_to_pointer(ic, 
							       ic->tme_recode_x86_ic_chain_epilogue,
							       tme_uint8_t *));

  /* the next fixup target is for chain jump alternate far: */
  TME_RECODE_X86_CHAIN_FIXUP_TARGET_ALTERNATE(ic,
					      (TME_RECODE_CHAIN_INFO_JUMP
					       + TME_RECODE_CHAIN_INFO_ALTERNATE_FAR))
    = tme_recode_build_to_thunk_off(ic, thunk_bytes);

  /* advance the fixup address register past the chain jump
     conditional six-byte jcc instruction: */
  thunk_bytes = _tme_recode_x86_chain_fixup_alternate(thunk_bytes);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* the next fixup target is for chain jump far: */
  _tme_recode_x86_chain_fixup_target(ic,
				     TME_RECODE_CHAIN_INFO_FAR,
				     thunk_bytes_fixup_call);

  /* the next fixup target is for chain call far: */
  _tme_recode_x86_chain_fixup_target(ic,
				     (TME_RECODE_CHAIN_INFO_CALL
				      + TME_RECODE_CHAIN_INFO_FAR),
				     thunk_bytes_fixup_call);

  /* the next fixup target is for chain call near: */
  _tme_recode_x86_chain_fixup_target(ic,
				     (TME_RECODE_CHAIN_INFO_CALL
				      + TME_RECODE_CHAIN_INFO_NEAR),
				     thunk_bytes_fixup_call);

  /* finish the fixup targets thunk: */
  tme_recode_host_thunk_finish(ic);
}

/* this emits a jmp to a chain subs: */
static tme_uint8_t *
_tme_recode_x86_chain_subs_jmp(struct tme_recode_ic *ic,
			       tme_uint32_t opcode,
			       struct tme_recode_chain_thunk *chain_thunk,
			       tme_uint32_t chain_info)
{
  tme_uint8_t *thunk_bytes;
  tme_uint8_t *thunk_bytes_jc;
  tme_recode_thunk_off_t chain_subs;
  tme_uint8_t *thunk_bytes_target;

  /* if this is a conditional jmp: */
  if (opcode != TME_RECODE_X86_OPCODE_JMP_RELb
      && opcode != TME_RECODE_X86_OPCODE_JMP_RELz) {

    /* define CF with the recode jump flag.  NB that the recode flags
       are now above the return address to the instructions thunk: */
    _tme_recode_x86_conds_testc(ic, TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8) + TME_RECODE_FLAG_JUMP);
  }

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* emit the jmp: */
  chain_subs = TME_RECODE_X86_CHAIN_SUBS(chain_thunk, chain_info);
  if (chain_subs == 0) {
    thunk_bytes_jc = thunk_bytes;
    thunk_bytes_target = (tme_uint8_t *) NULL;
  }
  else {
    thunk_bytes_jc = (tme_uint8_t *) NULL;
    thunk_bytes_target = tme_recode_thunk_off_to_pointer(ic, chain_subs, tme_uint8_t *);
  }
  thunk_bytes = _tme_recode_x86_emit_jmp(thunk_bytes, opcode, thunk_bytes_target);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  return (thunk_bytes_jc);
}

/* this emits instructions to add a sign-extended byte to the chain
   return address stack pointer, mask it, and then store it: */
static void
_tme_recode_x86_chain_ras_pointer_update(struct tme_recode_ic *ic,
					 tme_int8_t addend)
{
  tme_uint8_t *thunk_bytes;
  tme_uint8_t ras_size;

  /* start these instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* emit the add $imm8: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_CHAIN_RAS_POINTER),
					       TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADD));
  thunk_bytes[2] = addend;
  thunk_bytes += 3;

  /* emit the and $imm8: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_CHAIN_RAS_POINTER),
					       TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_AND));
  ras_size = ic->tme_recode_ic_chain_ras_size;
  assert (ras_size > 0 && (ras_size & (ras_size - 1)) == 0);
  thunk_bytes[2] = ras_size - 1;
  thunk_bytes += 3;

  /* store the chain return address stack pointer: */
  thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev);
  thunk_bytes = _tme_recode_x86_emit_ic_modrm(thunk_bytes + 1,
					      ic->tme_recode_ic_chain_ras_pointer_offset,
					      TME_RECODE_X86_REG_CHAIN_RAS_POINTER);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits an instruction to load or store an entry on the chain
   return address stack: */
static void
_tme_recode_x86_chain_ras_ls(struct tme_recode_ic *ic,
			     int store)
{
  tme_uint8_t *thunk_bytes;
  tme_int32_t ras_offset;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* emit the movl: */
  thunk_bytes[0]
    = (TME_RECODE_X86_OPCODE_BINOP_MOV
       + (store
	  ? TME_RECODE_X86_OPCODE_BINOP_Gv_Ev
	  : TME_RECODE_X86_OPCODE_BINOP_Ev_Gv));
  ras_offset = ic->tme_recode_ic_chain_ras_offset;
  thunk_bytes[1]
    = TME_RECODE_X86_MOD_OPREG_RM((ras_offset < 0x80
				   ? TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_EA_BASE_SIB)
				   : TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_EA_BASE_SIB)),
				  TME_RECODE_X86_REG(TME_RECODE_X86_REG_CHAIN_RETURN_ADDRESS));
  thunk_bytes[2]
    = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_IC,
			 TME_RECODE_X86_REG_CHAIN_RAS_POINTER,
			 sizeof(tme_recode_ras_entry_t));
  *((tme_uint32_t *) (thunk_bytes + 3)) = ras_offset;
  thunk_bytes += 3 + (ras_offset < 0x80 ? sizeof(tme_uint8_t) : sizeof(tme_uint32_t));

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits instructions to push the chain return address stack: */
static void
_tme_recode_x86_chain_ras_push(struct tme_recode_ic *ic)
{
  tme_uint8_t *thunk_bytes;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* load the return address stack pointer: */
  thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
  thunk_bytes = _tme_recode_x86_emit_ic_modrm(thunk_bytes + 1,
					      ic->tme_recode_ic_chain_ras_pointer_offset,
					      TME_RECODE_X86_REG_CHAIN_RAS_POINTER);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
  
  /* subtract one from the return address stack pointer, and store it: */
  _tme_recode_x86_chain_ras_pointer_update(ic, -1);

  /* store the return address: */
  _tme_recode_x86_chain_ras_ls(ic, TRUE);
}

/* this emits instructions to load or store the pointer to the token
   for the current recode instruction TLB entry: */
static void
_tme_recode_x86_chain_itlb_current_token_ls(struct tme_recode_ic *ic,
					    int store)
{
  tme_uint8_t *thunk_bytes;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* load or store the pointer to the token for the current recode
     instruction TLB entry: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    *(thunk_bytes++)
      = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST,
			     TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN);
  }
  thunk_bytes[0]
    = (TME_RECODE_X86_OPCODE_BINOP_MOV
       + (store
	  ? TME_RECODE_X86_OPCODE_BINOP_Gv_Ev
	  : TME_RECODE_X86_OPCODE_BINOP_Ev_Gv));
  thunk_bytes = _tme_recode_x86_emit_ic_modrm(thunk_bytes + 1,
					      ic->tme_recode_ic_itlb_current_token_offset,
					      TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}
#define _tme_recode_x86_chain_itlb_current_token_load(ic) _tme_recode_x86_chain_itlb_current_token_ls(ic, FALSE)

/* this stores the pointer to the current recode instruction TLB entry: */
static void
_tme_recode_x86_chain_itlb_current_store(struct tme_recode_ic *ic,
					 const struct tme_recode_x86_tlb_type *x86_tlb_type)
{
  tme_uint8_t *thunk_bytes;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* convert the pointer to the recode instruction TLB entry in
     TME_RECODE_X86_REG_TLB into a pointer to the token for that entry
     in TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN: */
#if TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN != TME_RECODE_X86_REG_TLB
#error "TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN changed"
#endif
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_TLB),
					       TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADD));
  assert (x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_token < 0x80);
  thunk_bytes[2] = x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_token;
  thunk_bytes += 3;

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* store the pointer to the token for the current recode instruction
     TLB entry: */
  _tme_recode_x86_chain_itlb_current_token_ls(ic, TRUE);
}

/* this emits instructions for the call and jump chain subs, or for
   the return chain subs: */
static void
_tme_recode_x86_chain_subs(struct tme_recode_ic *ic,
			   const struct tme_recode_chain *chain,
			   struct tme_recode_chain_thunk *chain_thunk,
			   tme_uint32_t chain_info_call_or_return)
{
  tme_uint8_t *thunk_bytes_jmp_to_jump_far_unconditional;
  tme_uint8_t *thunk_bytes;
  struct tme_recode_x86_tlb_type x86_tlb_type;
  union tme_recode_reginfo reginfo;
  unsigned int rex;
  unsigned int reg_x86_address;
  tme_uint8_t *thunk_bytes_jcc_chain_counter;
  const tme_uint8_t *thunk_bytes_jmp_to_epilogue;

  /* the first chain subs is for a chain call far conditional or chain
     return far conditional: */
  TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			    (chain_info_call_or_return
			     + TME_RECODE_CHAIN_INFO_FAR
			     + TME_RECODE_CHAIN_INFO_CONDITIONAL))
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

  /* if the recode jump flag is false, jmp to the chain subs for a
     chain jump far unconditional: */
  thunk_bytes_jmp_to_jump_far_unconditional
    = _tme_recode_x86_chain_subs_jmp(ic,
				     (TME_RECODE_X86_OPCODE_ESC_0F
				      + (TME_RECODE_X86_OPCODE0F_JCC(TME_RECODE_X86_COND_NOT
								     | TME_RECODE_X86_COND_C)
					 << 8)),
				     chain_thunk,
				     (TME_RECODE_CHAIN_INFO_JUMP
				      + TME_RECODE_CHAIN_INFO_FAR
				      + TME_RECODE_CHAIN_INFO_UNCONDITIONAL));

  /* if this the chain subs for a chain return far conditional: */
  if (chain_info_call_or_return == TME_RECODE_CHAIN_INFO_RETURN) {

    /* throw away the return address to the instructions thunk: */
    tme_recode_x86_insns_start(ic, thunk_bytes);
    thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes,
						 TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));
    tme_recode_x86_insns_finish(ic, thunk_bytes);
  }

  /* the next chain subs is for a chain call far unconditional or a
     chain return far unconditional: */
  TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			    (chain_info_call_or_return
			     + TME_RECODE_CHAIN_INFO_FAR
			     + TME_RECODE_CHAIN_INFO_UNCONDITIONAL))
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

  /* if this is the chain subs for a chain call far unconditional: */
  if (chain_info_call_or_return == TME_RECODE_CHAIN_INFO_CALL) {

    /* push the chain return address stack: */
    _tme_recode_x86_chain_ras_push(ic);

    /* the next chain subs is for both a chain jump far conditional
       and a chain jump far unconditional: */
    TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			      (TME_RECODE_CHAIN_INFO_JUMP
			       + TME_RECODE_CHAIN_INFO_FAR
			       + TME_RECODE_CHAIN_INFO_CONDITIONAL))
      = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);
    TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			      (TME_RECODE_CHAIN_INFO_JUMP
			       + TME_RECODE_CHAIN_INFO_FAR
			       + TME_RECODE_CHAIN_INFO_UNCONDITIONAL))
      = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

    /* fix up the jmp to the chain subs for a chain jump far
       unconditional: */
    _tme_recode_x86_fixup_jmp(thunk_bytes_jmp_to_jump_far_unconditional,
			      ic->tme_recode_ic_thunk_build_next);
  }

  /* load the pointer to the token for the current instruction TLB
     entry: */
  _tme_recode_x86_chain_itlb_current_token_load(ic);

  /* get the TLB type for the address type: */
  tme_recode_address_type_tlb_type(ic,
				   &chain->tme_recode_chain_address_type,
				   &x86_tlb_type.tme_recode_tlb_type);

  /* unbusy the token for the current instruction TLB entry: */
#if TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN	!= TME_RECODE_X86_REG_TLB
#error "TME_RECODE_X86_REG_CHAIN_ITLB_CURRENT_TOKEN changed"
#endif
  _tme_recode_x86_tlb_unbusy(ic, 0);

  /* load the PC to chain to: */
  reginfo = ic->tme_recode_ic_reginfo[chain->tme_recode_chain_reg_guest];
  reginfo.tme_recode_reginfo_tags_ruses
    = (TME_RECODE_REGINFO_TAGS_REG_HOST(_tme_recode_x86_tlb_reg_host_address(ic))
       + TME_RECODE_REGINFO_TAGS_VALID_SIZE(ic->tme_recode_ic_reg_size)
       + TME_RECODE_REGINFO_TAGS_CLEAN);
  ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;
  tme_recode_host_reg_move(ic,
			   chain->tme_recode_chain_reg_guest,
			   reginfo.tme_recode_reginfo_all);
  ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

  /* find, busy, and check an instruction TLB entry: */
  _tme_recode_x86_tlb_busy(ic,
			   &chain->tme_recode_chain_address_type,
			   &x86_tlb_type);

  /* store the pointer to the current recode instruction TLB entry: */
  _tme_recode_x86_chain_itlb_current_store(ic, &x86_tlb_type);

  /* if this is the chain subs for a chain jump far conditional and
     a chain jump far unconditional: */
  if (chain_info_call_or_return == TME_RECODE_CHAIN_INFO_CALL) {

    /* the next chain subs is for both a chain jump near conditional
       and a chain jump near unconditional: */
    TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			      (TME_RECODE_CHAIN_INFO_JUMP
			       + TME_RECODE_CHAIN_INFO_NEAR
			       + TME_RECODE_CHAIN_INFO_CONDITIONAL))
      = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);
    TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			      (TME_RECODE_CHAIN_INFO_JUMP
			       + TME_RECODE_CHAIN_INFO_NEAR
			       + TME_RECODE_CHAIN_INFO_UNCONDITIONAL))
      = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);
  }

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* subtract one from the guest chain counter: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_REG_IC),
					       TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_SUB));
  *((tme_int32_t *) &thunk_bytes[2]) = ic->tme_recode_ic_chain_counter_offset;
  thunk_bytes[2 + sizeof(tme_int32_t)] = 1;
  thunk_bytes += 2 + sizeof(tme_int32_t) + 1;

  /* if this is the chain subs for a chain jump near conditional and a
     chain jump near unconditional: */
  if (chain_info_call_or_return == TME_RECODE_CHAIN_INFO_CALL) {

    /* load the return address to the instructions thunk into the
       fixup address register: */
    /* NB: this is always a 32-bit instruction, even on an x86-64
       host, because we end up converting the return address into an
       instructions thunk offset, which is only 32 bits: */
    assert (sizeof(tme_recode_thunk_off_t) == sizeof(tme_int32_t));
    rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, TME_RECODE_X86_REG_CHAIN_FIXUP_ADDRESS);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv;
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB),
						 TME_RECODE_X86_REG_CHAIN_FIXUP_ADDRESS);
    thunk_bytes[2] = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1);
    thunk_bytes += 3;
  }

  /* otherwise, this is the chain subs for a return far unconditional: */
  else {

    /* load the chain return address stack pointer: */
    thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    thunk_bytes = _tme_recode_x86_emit_ic_modrm(thunk_bytes + 1,
						ic->tme_recode_ic_chain_ras_pointer_offset,
						TME_RECODE_X86_REG_CHAIN_RAS_POINTER);
  }

  /* if needed, copy the guest instructions source address into the
     correct register: */
  reg_x86_address
    = tme_recode_x86_reg_from_host[_tme_recode_x86_tlb_reg_host_address(ic)];
  if (reg_x86_address != TME_RECODE_X86_REG_CHAIN_GUEST_SRC) {
    _tme_recode_x86_emit_reg_copy(thunk_bytes, reg_x86_address, TME_RECODE_X86_REG_CHAIN_GUEST_SRC);
  }

  /* if the guest chain counter was zero or one, jmp to the code that
     handles a guest chain counter underflow: */
  thunk_bytes_jcc_chain_counter = thunk_bytes;
  thunk_bytes
    = _tme_recode_x86_emit_jmp(thunk_bytes,
			       TME_RECODE_X86_OPCODE_JCC(TME_RECODE_X86_COND_BE),
			       (tme_uint8_t *) NULL);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* if this is the chain subs for a chain jump near conditional and a
     chain jump near unconditional: */
  if (chain_info_call_or_return == TME_RECODE_CHAIN_INFO_CALL) {

    /* define CF with the recode jump flag.  NB that the recode flags
       are now above the return address to the instructions thunk: */
    _tme_recode_x86_conds_testc(ic, TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8) + TME_RECODE_FLAG_JUMP);

    /* emit a return back to the instructions thunk: */
    tme_recode_x86_insns_start(ic, thunk_bytes);
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_RET;
    thunk_bytes++;
    tme_recode_x86_insns_finish(ic, thunk_bytes);

    /* the next chain subs is for a chain call near conditional: */
    TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			      (TME_RECODE_CHAIN_INFO_CALL
			       + TME_RECODE_CHAIN_INFO_NEAR
			       + TME_RECODE_CHAIN_INFO_CONDITIONAL))
      = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

    /* if the recode jump flag is false, jmp to the chain subs for a
       chain jump near conditional: */
    _tme_recode_x86_chain_subs_jmp(ic,
				   TME_RECODE_X86_OPCODE_JCC(TME_RECODE_X86_COND_NOT
							     | TME_RECODE_X86_COND_C),
				   chain_thunk,
				   (TME_RECODE_CHAIN_INFO_JUMP
				    + TME_RECODE_CHAIN_INFO_NEAR
				    + TME_RECODE_CHAIN_INFO_CONDITIONAL));

    /* the next chain subs is for a chain call near unconditional: */
    TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
			      (TME_RECODE_CHAIN_INFO_CALL
			       + TME_RECODE_CHAIN_INFO_NEAR
			       + TME_RECODE_CHAIN_INFO_UNCONDITIONAL))
      = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

    /* push the chain return address stack: */
    _tme_recode_x86_chain_ras_push(ic);

    /* jmp to the chain subs for a chain jump near unconditional: */
    _tme_recode_x86_chain_subs_jmp(ic,
				   TME_RECODE_X86_OPCODE_JMP_RELb,
				   chain_thunk,
				   (TME_RECODE_CHAIN_INFO_JUMP
				    + TME_RECODE_CHAIN_INFO_NEAR
				    + TME_RECODE_CHAIN_INFO_UNCONDITIONAL));
  }

  /* otherwise, this is the chain subs for a chain return far
     unconditional: */
  else {

    /* load the return address: */
    _tme_recode_x86_chain_ras_ls(ic, FALSE);

    /* add one to the return address stack pointer, and store it: */
    _tme_recode_x86_chain_ras_pointer_update(ic, 1);

    /* start more instructions: */
    tme_recode_x86_insns_start(ic, thunk_bytes);

    /* if this is an x86-64 host: */
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

      /* emit an %ip-relative lea instruction, to load the address of
	 thunk offset zero into the TLB scratch register: */
      thunk_bytes[0] = TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, TME_RECODE_X86_REG_TLB_SCRATCH);
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_LEA;
      thunk_bytes[2] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_IP),
						   TME_RECODE_X86_REG(TME_RECODE_X86_REG_TLB_SCRATCH));
      thunk_bytes += 3 + sizeof(tme_int32_t);
      ((tme_int32_t *) thunk_bytes)[-1]
	= (0 - (tme_int32_t) tme_recode_build_to_thunk_off(ic, thunk_bytes));

      /* add the address of thunk offset zero into the return address register: */
      _tme_recode_x86_emit_reg_binop(thunk_bytes,
				     TME_RECODE_X86_OPCODE_BINOP_ADD,
				     TME_RECODE_X86_REG_TLB_SCRATCH,
				     TME_RECODE_X86_REG_CHAIN_RETURN_ADDRESS);
    }

    /* emit the indirect jmp instruction with the return address
       register: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP5;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_CHAIN_RETURN_ADDRESS),
				    TME_RECODE_X86_OPCODE_GRP5_JMP);
    thunk_bytes += 2;

    /* unknown indirect jmps are predicted to fallthrough; placing a UD2
       instruction after an indirect jmp can stop a processor from
       speculatively executing garbage fallthrough instructions: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;
    thunk_bytes[1] = TME_RECODE_X86_OPCODE0F_UD2;
    thunk_bytes += 2;

    /* finish these instructions: */
    tme_recode_x86_insns_finish(ic, thunk_bytes);
  }

  /* this is the code that handles an instruction TLB miss: */

  /* fix up the assist jcc(s) in the instruction TLB busy: */
  if (x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp_address_ok != NULL) {
    _tme_recode_x86_fixup_jmp(x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp_address_ok,
			      ic->tme_recode_ic_thunk_build_next);
  }
  _tme_recode_x86_fixup_jmp(x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp,
			    ic->tme_recode_ic_thunk_build_next);

  /* store the pointer to the current recode instruction TLB entry: */
  /* NB: this is necessary because the instruction TLB that missed is
     still busy: */
  _tme_recode_x86_chain_itlb_current_store(ic, &x86_tlb_type);

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* this is the code that jmps to the chain epilogue: */
  thunk_bytes_jmp_to_epilogue = thunk_bytes;

  /* if this is not for the chain return far subs: */
  if (chain_info_call_or_return != TME_RECODE_CHAIN_INFO_RETURN) {

    /* throw away the return address to the instructions thunk: */
    thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes,
						 TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));
  }

  /* jmp to the chain epilogue: */
  thunk_bytes
    = _tme_recode_x86_emit_jmp(thunk_bytes,
			       TME_RECODE_X86_OPCODE_JMP_RELz,
			       tme_recode_thunk_off_to_pointer(ic, 
							       ic->tme_recode_x86_ic_chain_epilogue,
							       tme_uint8_t *));

  /* this is the code that handles a guest chain counter underflow: */
  _tme_recode_x86_fixup_jmp(thunk_bytes_jcc_chain_counter, thunk_bytes);

  /* clear the guest chain counter, since we may have wrapped it past
     zero: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iz_Ev;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_REG_IC),
					       0 /* undefined */);
  *((tme_int32_t *) &thunk_bytes[2]) = ic->tme_recode_ic_chain_counter_offset;
  *((tme_uint32_t *) &thunk_bytes[6]) = 0;
  thunk_bytes += 2 + sizeof(tme_int32_t) + sizeof(tme_uint32_t);

  /* jmp to the code that jmps to the chain epilogue: */
  thunk_bytes
    = _tme_recode_x86_emit_jmp(thunk_bytes,
			       TME_RECODE_X86_OPCODE_JMP_RELb,
			       thunk_bytes_jmp_to_epilogue);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits instructions for a chain out: */
static void
_tme_recode_x86_chain_out(struct tme_recode_ic *ic,
			  const struct tme_recode_insns_group *insns_group)
{
  tme_uint8_t *thunk_bytes;
  tme_uint8_t *thunk_bytes_start;
  tme_uint32_t chain_info;

  /* the chain subs, and a chain call instruction itself, destroy the
     c register, so it won't hold the base offset of a guest register
     window any more: */
  ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* remember where these instructions started: */
  thunk_bytes_start = thunk_bytes;

  /* get the chain information: */
  chain_info = insns_group->tme_recode_insns_group_chain_info;

  /* exactly one of jump, call, and return may be set: */
  assert ((chain_info
	   & (TME_RECODE_CHAIN_INFO_JUMP
	      | TME_RECODE_CHAIN_INFO_CALL
	      | TME_RECODE_CHAIN_INFO_RETURN)) != 0
	  && ((chain_info
	       & (TME_RECODE_CHAIN_INFO_JUMP
		  | TME_RECODE_CHAIN_INFO_CALL
		  | TME_RECODE_CHAIN_INFO_RETURN))
	      & ((chain_info
		  & (TME_RECODE_CHAIN_INFO_JUMP
		     | TME_RECODE_CHAIN_INFO_CALL
		     | TME_RECODE_CHAIN_INFO_RETURN))
		 - 1)) == 0);

  /* there is no such thing as a chain return near: */
  assert ((chain_info & TME_RECODE_CHAIN_INFO_RETURN) == 0
	  || (chain_info
	      & (TME_RECODE_CHAIN_INFO_NEAR
		 | TME_RECODE_CHAIN_INFO_FAR)) != TME_RECODE_CHAIN_INFO_NEAR);

  /* if this is a chain conditional, and the alternate target is far,
     force the main target to be far, since the alternate target needs
     the information generated by a chain far subs: */
  assert (((chain_info
	    & (TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR
	       | TME_RECODE_CHAIN_INFO_ALTERNATE_FAR))
	   == TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR)
	  || ((chain_info
	       & (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
		  | TME_RECODE_CHAIN_INFO_CONDITIONAL))
	      != TME_RECODE_CHAIN_INFO_UNCONDITIONAL));
#if TME_RECODE_CHAIN_INFO_FAR == 0 || TME_RECODE_CHAIN_INFO_ALTERNATE_FAR <= TME_RECODE_CHAIN_INFO_FAR
#error "TME_RECODE_CHAIN_INFO_ values changed"
#endif
  chain_info
    |= ((chain_info
	 / (TME_RECODE_CHAIN_INFO_ALTERNATE_FAR
	    / TME_RECODE_CHAIN_INFO_FAR))
	& TME_RECODE_CHAIN_INFO_FAR);

  /* if this is a chain call: */
  if (chain_info & TME_RECODE_CHAIN_INFO_CALL) {

    /* load the return address register with a value that indicates
       the chain epilogue.  this will later get fixed up to indicate
       the instructions thunk to return to: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(TME_RECODE_X86_REG_CHAIN_RETURN_ADDRESS);
    *((tme_uint32_t *) (thunk_bytes + 1))
      = TME_RECODE_X86_CHAIN_RETURN_ADDRESS(ic, ic->tme_recode_x86_ic_chain_epilogue);
    thunk_bytes += 1 + sizeof(tme_uint32_t);
  }

  /* if this is a chain return unconditional, jump to the chain subs,
     otherwise call the chain subs: */
  thunk_bytes[0]
    = (((chain_info
	 & (TME_RECODE_CHAIN_INFO_RETURN
	    + (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
	       | TME_RECODE_CHAIN_INFO_CONDITIONAL)))
	== (TME_RECODE_CHAIN_INFO_RETURN
	    + TME_RECODE_CHAIN_INFO_UNCONDITIONAL))
       ? TME_RECODE_X86_OPCODE_JMP_RELz
       : TME_RECODE_X86_OPCODE_CALL_RELz);
  thunk_bytes += 1 + sizeof(tme_int32_t);
  ((tme_int32_t *) thunk_bytes)[-1]
    = (TME_RECODE_X86_CHAIN_SUBS(insns_group->tme_recode_insns_group_chain_thunk,
				 chain_info)
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* for the regular target, emit the unconditional jmp instruction,
     or the conditional jc instruction, to the chain fixup target: */
  if ((chain_info
       & (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
	  | TME_RECODE_CHAIN_INFO_CONDITIONAL))
      == TME_RECODE_CHAIN_INFO_UNCONDITIONAL) {
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_JMP_RELz;
  }
  else {
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_OPCODE_ESC_0F
	 + (TME_RECODE_X86_OPCODE0F_JCC(TME_RECODE_X86_COND_C)
	    << 8));
    thunk_bytes++;
  }
  thunk_bytes += 1 + sizeof(tme_int32_t);
  ((tme_int32_t *) thunk_bytes)[-1]
    = (TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic, chain_info)
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* if this is a chain conditional instruction: */
  if ((chain_info
       & (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
	  | TME_RECODE_CHAIN_INFO_CONDITIONAL))
      != TME_RECODE_CHAIN_INFO_UNCONDITIONAL) {

    /* for the alternate target, emit the unconditional jmp
       instruction to the chain fixup target alternate: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_JMP_RELz;
    thunk_bytes += 1 + sizeof(tme_int32_t);
    ((tme_int32_t *) thunk_bytes)[-1]
      = (TME_RECODE_X86_CHAIN_FIXUP_TARGET_ALTERNATE(ic, chain_info)
	 - tme_recode_build_to_thunk_off(ic, thunk_bytes));
  }

  /* check the size of the chain out: */
  assert ((thunk_bytes - thunk_bytes_start)
	  <= TME_RECODE_X86_CHAIN_OUT_SIZE_MAX);

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* if the given address is reachable with an effective address using
   an address in another register, this returns the signed 32-bit
   displacement, otherwise this returns zero: */
static inline tme_int32_t
_tme_recode_x86_chain_base_disp32(const tme_shared tme_uint8_t *base,
				  const tme_shared tme_uint8_t *pointer)
{
  signed long disp;

  disp = pointer - base;
  return (disp == (tme_int32_t) disp
	  ? disp
	  : 0);
}

/* if the given address is reachable with a PC-relative effective
   address, this returns the signed 32-bit displacement, otherwise
   this returns zero: */
static inline tme_int32_t
_tme_recode_x86_chain_ip_disp32(const struct tme_recode_ic *ic,
				const tme_uint8_t *thunk_bytes,
				const tme_shared tme_uint8_t *pointer)
{
  tme_recode_thunk_off_t thunk_off;
  const tme_uint8_t *ip;

  thunk_off = tme_recode_build_to_thunk_off(ic, thunk_bytes);
  ip = tme_recode_thunk_off_to_pointer(ic, thunk_off, const tme_uint8_t *);
  return (_tme_recode_x86_chain_base_disp32(ip, pointer));
}

/* this emits instructions for a chain in: */
static void
_tme_recode_x86_chain_in(struct tme_recode_ic *ic,
			 const struct tme_recode_insns_group *insns_group)
{
  tme_uint8_t *thunk_bytes;
  tme_uint8_t *thunk_bytes_start;
  const tme_shared tme_uint8_t *insns_group_src;
  unsigned int size_insns_group_src;
  tme_int32_t disp32;
  unsigned int rex;
  const tme_shared tme_uint8_t *insns_group_valid_byte;
  unsigned int size_insns_group_valid_byte;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* remember where these instructions started: */
  thunk_bytes_start = thunk_bytes;

  /* get the guest instructions source address: */
  insns_group_src = insns_group->tme_recode_insns_group_src;

  /* if this is an ia32 host, or if this is an x86-64 host and the
     guest instructions source address fits in a sign-extended 32
     bits: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
      || (((tme_int32_t)
	   (signed long)
	   insns_group_src)
	  == (signed long) insns_group_src)) {

    /* emit a cmpl $imm32, %reg or cmpq $imm32, %reg: */
    if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {
      thunk_bytes[0] = (tme_uint8_t) TME_RECODE_X86_INSN_CMP_Iz_REG_CHAIN_GUEST_SRC;
    }
    else {
      *((tme_uint16_t *) thunk_bytes) = TME_RECODE_X86_INSN_CMP_Iz_REG_CHAIN_GUEST_SRC;
    }
    *((tme_int32_t *) &thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1])
      = (signed long) insns_group_src;
    thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1 + sizeof(tme_int32_t);

    /* assume that this is an x86-64 host, and set the size of the
       instructions that check the guest instruction source address
       matches the instructions thunk: */
    size_insns_group_src = TME_RECODE_X86_64_CHAIN_IN_SIZE_CMPQ_IMM32_JNZ;
  }

  /* otherwise, this is an x86-64 host and the guest instructions
     source address doesn't fit in a sign-extended 32 bits: */
  else {

    /* if the guest instructions source address fits in 32 bits: */
    if (TME_RECODE_SIZE_FITS((unsigned long) insns_group_src,
			     TME_RECODE_SIZE_32)) {

      /* emit a movl $imm32, %reg: */
      thunk_bytes[0] = TME_RECODE_X86_INSN_MOVL_IMM32_REG_TLB_SCRATCH;
      *((tme_uint32_t *) &thunk_bytes[1]) = (unsigned long) insns_group_src;
      thunk_bytes += 1 + sizeof(tme_uint32_t);

      /* set the size of the instructions that check the guest
	 instruction source address matches the instructions thunk: */
      size_insns_group_src = TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVL_CMP_JNZ;
    }

    /* otherwise, if the guest instructions source address is within a
       signed 32-bit displacement of the %ip of an lea
       instruction: */
    else if ((disp32
	      = _tme_recode_x86_chain_ip_disp32(ic,
						(thunk_bytes
						 + 1 /* rex */
						 + 1 /* lea */
						 + 1 /* modR/M */
						 + sizeof(disp32)),
						insns_group_src))) {

      /* emit an lea disp32(%ip), %reg: */
      thunk_bytes[0] = (tme_uint8_t) TME_RECODE_X86_INSN_LEA_DISP32_IP_REG_TLB_SCRATCH;
      *((tme_uint16_t *) &thunk_bytes[1]) = (TME_RECODE_X86_INSN_LEA_DISP32_IP_REG_TLB_SCRATCH >> 8);
      *((tme_int32_t *) &thunk_bytes[3]) = disp32;
      thunk_bytes += 1 + 2 + sizeof(disp32);

      /* set the size of the instructions that check the guest
	 instruction source address matches the instructions thunk: */
      size_insns_group_src = TME_RECODE_X86_64_CHAIN_IN_SIZE_LEA_IP_CMP_JNZ;
    }

    /* otherwise, the guest instructions source address can't be
       generated: */
    else {

      /* emit a movq $imm64, %reg: */
      *((tme_uint16_t *) thunk_bytes) = TME_RECODE_X86_INSN_MOVQ_IMM64_REG_TLB_SCRATCH;
      *((unsigned long *) &thunk_bytes[2]) = (unsigned long) insns_group_src;
      thunk_bytes += 1 + 1 + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);

      /* set the size of the instructions that check the guest
	 instruction source address matches the instructions thunk: */
      size_insns_group_src = TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVQ_CMP_JNZ;
    }

    /* emit a cmpq %reg, %reg: */
    rex = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST,
				TME_RECODE_X86_REG_CHAIN_GUEST_SRC)
	   | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST,
				  TME_RECODE_X86_REG_TLB_SCRATCH));
    assert (rex != 0);
    thunk_bytes[0] = rex;
    *((tme_uint16_t *) &thunk_bytes[1])
      = ((TME_RECODE_X86_OPCODE_BINOP_CMP
	  + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv)
	 + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_CHAIN_GUEST_SRC),
					TME_RECODE_X86_REG(TME_RECODE_X86_REG_TLB_SCRATCH))
	    << 8));
    thunk_bytes += 3;
  }

  /* if the guest instructions source address doesn't match this
     instructions thunk, jump to the chain epilogue: */
  *((tme_uint16_t *) thunk_bytes)
    = (TME_RECODE_X86_OPCODE_ESC_0F
       + (TME_RECODE_X86_OPCODE0F_JCC(TME_RECODE_X86_COND_NOT | TME_RECODE_X86_COND_Z)
	  << 8));
  thunk_bytes += 2 + sizeof(tme_int32_t);
  ((tme_int32_t *) thunk_bytes)[-1]
    = (ic->tme_recode_x86_ic_chain_epilogue
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* get the guest instructions valid byte address: */
  insns_group_valid_byte = insns_group->tme_recode_insns_group_valid_byte;

  /* if this is an ia32 host: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

    /* emit the opcode and addr32 for a testb $imm8, addr32: */
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_OPCODE_GRP3_Eb
	 + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_NONE),
					TME_RECODE_X86_OPCODE_GRP3_TEST)
	    << 8));
    *((tme_uint32_t *) &thunk_bytes[2]) = (unsigned long) insns_group_valid_byte;
    thunk_bytes += 2 + sizeof(tme_uint32_t);

    /* silence uninitialized variable warnings: */
    size_insns_group_valid_byte = 0;
  }

  /* otherwise, this is an x86-64 host: */
  else {

    /* assume that we can emit a testb followed by a jz, and emit the
       first opcode byte for the testb: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP3_Eb;
    thunk_bytes++;

    /* if the guest instructions valid byte address fits in a
       sign-extended 32 bits: */
    if (((tme_int32_t)
	 (signed long)
	 insns_group_valid_byte)
	== (signed long) insns_group_valid_byte) {

      /* emit the modR/M, SIB, and disp32 for a testb $imm8, addr32: */
      *((tme_uint16_t *) thunk_bytes) = TME_RECODE_X86_INSN_TESTB_IMM8_ADDR32 >> 8;
      *((tme_int32_t *) &thunk_bytes[2]) = (signed long) insns_group_valid_byte;
      thunk_bytes += 2 + sizeof(tme_int32_t);

      /* set the size of the instructions that check that the guest
	 instructions are still valid: */
      size_insns_group_valid_byte = TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_ADDR32_JZ;
    }

    /* otherwise, if the guest instructions valid byte address is
       within a signed 32-bit displacement of the %ip of the testb
       instruction: */
    else if ((disp32
	      = _tme_recode_x86_chain_ip_disp32(ic,
						(thunk_bytes
						 + 1 /* testb */
						 + 1 /* modR/M */
						 + sizeof(disp32)
						 + 1 /* imm8 */),
						insns_group_valid_byte))) {

      /* emit the modR/M and disp32 for a testb $imm8, disp32(%ip): */
      thunk_bytes[0] = TME_RECODE_X86_INSN_TESTB_IMM8_DISP32_IP >> 8;
      *((tme_int32_t *) &thunk_bytes[1]) = disp32;
      thunk_bytes += 1 + sizeof(disp32);

      /* set the size of the instructions that check that the guest
	 instructions are still valid: */
      size_insns_group_valid_byte = TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_BASE_JZ;
    }

    /* otherwise, if the guest instructions valid byte address is
       within a signed 32-bit displacement of the guest instructions
       source address: */
    else if ((disp32
	      = _tme_recode_x86_chain_base_disp32(insns_group_src,
						  insns_group_valid_byte))) {

      /* emit the modR/M and disp32 for a testb $imm8, disp32(%reg): */
      thunk_bytes[0] = TME_RECODE_X86_INSN_TESTB_IMM8_DISP32_REG_CHAIN_GUEST_SRC >> 8;
      *((tme_int32_t *) &thunk_bytes[1]) = disp32;
      thunk_bytes += 1 + sizeof(disp32);

      /* set the size of the instructions that check that the guest
	 instructions are still valid: */
      size_insns_group_valid_byte = TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_BASE_JZ;
    }

    /* otherwise, the guest instructions valid byte address can't be
       generated: */
    else {

      /* emit a movq $imm64, %reg: */
      /* NB: thunk_bytes has already been advanced by one: */
      *((tme_uint16_t *) &thunk_bytes[0 - 1]) = TME_RECODE_X86_INSN_MOVQ_IMM64_REG_TLB_SCRATCH;
      *((unsigned long *) &thunk_bytes[2 - 1]) = (unsigned long) insns_group_valid_byte;
      thunk_bytes += -1 + 1 + 1 + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);

      /* emit the opcode and modR/M for a testb $imm8, (%reg): */
      *((tme_uint16_t *) thunk_bytes)
	= (TME_RECODE_X86_OPCODE_GRP3_Eb
	   + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_REG_TLB_SCRATCH),
					  TME_RECODE_X86_OPCODE_GRP3_TEST)
	      << 8));
      thunk_bytes += 2;

      /* set the size of the instructions that check that the guest
	 instructions are still valid: */
      size_insns_group_valid_byte = TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVQ_TESTB_JZ;
    }
  }

  /* emit the $imm8, and if the guest instructions aren't valid, jump
     to the chain epilogue: */
  thunk_bytes[0] = insns_group->tme_recode_insns_group_valid_mask;
  *((tme_uint16_t *) &thunk_bytes[1])
    = (TME_RECODE_X86_OPCODE_ESC_0F
       + (TME_RECODE_X86_OPCODE0F_JCC(TME_RECODE_X86_COND_Z)
	  << 8));
  thunk_bytes += 1 + 2 + sizeof(tme_int32_t);
  ((tme_int32_t *) thunk_bytes)[-1]
    = (ic->tme_recode_x86_ic_chain_epilogue
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* check the size of the chain in: */
  assert ((thunk_bytes - thunk_bytes_start)
	  <= TME_RECODE_X86_CHAIN_IN_SIZE_MAX);

  /* check the sizes of the instructions that we need to skip for a
     chain in near: */
  assert ((thunk_bytes - thunk_bytes_start)
	  == (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	      ? TME_RECODE_IA32_CHAIN_IN_SIZE_FAR
	      : (size_insns_group_src
		 + size_insns_group_valid_byte)));

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this makes the chain prologue for a new IC: */
static void
_tme_recode_x86_chain_prologue(struct tme_recode_ic *ic,
			       const struct tme_recode_chain *chain,
			       struct tme_recode_chain_thunk *chain_thunk)
{
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86_insns_thunk;
  unsigned long thunk_address0;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* set the chain prologue: */
  chain_thunk->tme_recode_x86_chain_thunk_prologue
    = tme_recode_thunk_off_to_pointer(ic,
				      tme_recode_build_to_thunk_off(ic, thunk_bytes),
				      void (*) _TME_P((struct tme_ic *, tme_recode_thunk_off_t)));

  /* push all callee-saved registers: */
  _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_BP);
  _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_B);
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(12));
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(13));
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(14));
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(15));
  }
  else {
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_SI);
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_DI);
  }

  /* the instructions thunk offset and address will be in the si register: */
  reg_x86_insns_thunk = TME_RECODE_X86_REG_SI;

  /* if this is an x86-64 host: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

    /* copy the struct tme_ic * argument into the ic register: */
    _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_DI, TME_RECODE_X86_REG_IC);

    /* the instructions thunk offset is already in the second argument register: */
    assert (reg_x86_insns_thunk == TME_RECODE_X86_REG_SI);
  }

  /* otherwise, this is an ia32 host: */
  else {

    /* load the struct tme_ic * argument from the stack: */
    /* NB: the magic 5 below is for the four callee-saved registers
       that we pushed above, plus the return address: */
    thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_EA_BASE_SIB),
						 TME_RECODE_X86_REG_IC);
    thunk_bytes[2] = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1);
    thunk_bytes[3] = (TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8) * 5);
    thunk_bytes += 4;

    /* load the instructions thunk offset from the stack: */
    /* NB: the magic 6 below is for the four callee-saved registers
       that we pushed above, plus the return address, plus the struct
       tme_ic * argument: */
    thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_EA_BASE_SIB),
						 reg_x86_insns_thunk);
    thunk_bytes[2] = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1);
    thunk_bytes[3] = (TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8) * 6);
    thunk_bytes += 4;
  }

  /* get the near address of the thunk at offset zero: */
  thunk_address0 = tme_recode_thunk_off_to_pointer(ic, 0, char *) - (char *) 0;

  /* if this is an ia32 host, or if this is an x86-64 host and the
     near address of the thunk at offset zero fits in 32 bits: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
      || thunk_address0 == (tme_uint32_t) thunk_address0) {

    /* add the near address of the thunk at offset zero to the
       instructions thunk offset: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Iz_Ev;
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_insns_thunk),
						 TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADD));
    *((tme_uint32_t *) &thunk_bytes[2]) = thunk_address0;
    thunk_bytes += 2 + sizeof(tme_uint32_t);
  }

  /* otherwise, this is an x86-64 host and the near address of the
     thunk at offset zero doesn't fit in 32 bits: */
  else {

    /* zero-extend the instructions thunk offset to 64 bits: */
    thunk_bytes[0]
      = (TME_RECODE_X86_OPCODE_BINOP_MOV
	 + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev);
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_insns_thunk),
				    TME_RECODE_X86_REG(reg_x86_insns_thunk));
    thunk_bytes += 2;

    /* load the near address of the thunk at offset zero into the TLB
       scratch register: */
    *((tme_uint16_t *) &thunk_bytes[0]) = TME_RECODE_X86_INSN_MOVQ_IMM64_REG_TLB_SCRATCH;
    *((unsigned long *) &thunk_bytes[2]) = thunk_address0;
    thunk_bytes += 2 + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);

    /* add the TLB scratch register to the instructions thunk
       offset: */
    _tme_recode_x86_emit_reg_binop(thunk_bytes,
				   TME_RECODE_X86_OPCODE_BINOP_ADD,
				   TME_RECODE_X86_REG_TLB_SCRATCH,
				   reg_x86_insns_thunk);
  }

  /* allocate one word on the stack for the recode flag bytes.  NB
     that on x86-64, this returns the stack pointer to 16-byte
     alignment as required by the ABI: */
  thunk_bytes
    = _tme_recode_x86_emit_adjust_sp(thunk_bytes,
				     (0 - TME_BIT(TME_RECODE_SIZE_HOST
						  - TME_RECODE_SIZE_8)));

  /* call the chain jump far unconditional subs: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_CALL_RELz;
  thunk_bytes += 1 + sizeof(tme_int32_t);
  ((tme_int32_t *) thunk_bytes)[-1]
    = (TME_RECODE_X86_CHAIN_SUBS(chain_thunk,
				 (TME_RECODE_CHAIN_INFO_JUMP
				  + TME_RECODE_CHAIN_INFO_FAR
				  + TME_RECODE_CHAIN_INFO_UNCONDITIONAL))
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* do the indirect jmp into the instructions thunk: */
  assert (TME_RECODE_X86_REX_B(0, reg_x86_insns_thunk) == 0);
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP5;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_insns_thunk),
					       TME_RECODE_X86_OPCODE_GRP5_JMP);
  thunk_bytes += 2;

  /* unknown indirect jmps are predicted to fallthrough; placing a UD2
     instruction after an indirect jmp can stop a processor from
     speculatively executing garbage fallthrough instructions: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;
  thunk_bytes[1] = TME_RECODE_X86_OPCODE0F_UD2;
  thunk_bytes += 2;

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* finish the chain prologue: */
  tme_recode_host_thunk_finish(ic);
}

/* this returns a chain thunk: */
const struct tme_recode_chain_thunk *
tme_recode_chain_thunk(struct tme_recode_ic *ic,
		       const struct tme_recode_chain *chain)
{
  struct tme_recode_chain_thunk *chain_thunk;

  /* if the chain fixup targets haven't been made yet: */
  if (TME_RECODE_X86_CHAIN_FIXUP_TARGET_ALTERNATE(ic,
						  (TME_RECODE_CHAIN_INFO_JUMP
						   + TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR))
      == 0) {

    /* make the chain fixup targets: */
    _tme_recode_x86_chain_fixup_targets(ic);
  }

  /* start the thunk for the chain subs: */
  if (!tme_recode_host_thunk_start(ic)) {
    abort();
  }
  chain_thunk = tme_new0(struct tme_recode_chain_thunk, 1);

  /* make the chain subs for chain calls and chain jumps: */
  _tme_recode_x86_chain_subs(ic,
			     chain,
			     chain_thunk,
			     TME_RECODE_CHAIN_INFO_CALL);
  
  /* make the chain subs for chain returns: */
  _tme_recode_x86_chain_subs(ic,
			     chain,
			     chain_thunk,
			     TME_RECODE_CHAIN_INFO_RETURN);

  /* finish the thunk for the chain subs: */
  tme_recode_host_thunk_finish(ic);

  /* start the thunk for the chain prologue: */
  if (!tme_recode_host_thunk_start(ic)) {
    abort();
  }

  /* make the chain prologue: */
  _tme_recode_x86_chain_prologue(ic,
				 chain,
				 chain_thunk);

  /* finish the thunk for the chain prologue: */
  tme_recode_host_thunk_finish(ic);

  return (chain_thunk);
}

/* this clears the return address stack: */
void
tme_recode_chain_ras_clear(const struct tme_recode_ic *recode_ic,
			   struct tme_ic *ic)
{
  tme_recode_ras_entry_t *_ras_entry;
  tme_uint32_t ras_size;
  tme_recode_ras_entry_t ras_entry;

  /* clear the return address stack: */
  ras_entry
    = TME_RECODE_X86_CHAIN_RETURN_ADDRESS(recode_ic,
					  recode_ic->tme_recode_x86_ic_chain_epilogue);
  _ras_entry
    = ((tme_recode_ras_entry_t *) 
       (((tme_uint8_t *) ic)
	+ recode_ic->tme_recode_ic_chain_ras_offset));
  ras_size = recode_ic->tme_recode_ic_chain_ras_size;
  do {
    _ras_entry[ras_size - 1] = ras_entry;
  } while (--ras_size);

  /* make sure that the return address stack pointer is valid: */
  assert (*((tme_uint32_t *)
	    (((tme_uint8_t *) ic)
	     + recode_ic->tme_recode_ic_chain_ras_pointer_offset))
	  < recode_ic->tme_recode_ic_chain_ras_size);
}

/* this fixes up a chain: */
tme_recode_thunk_off_t
tme_recode_chain_fixup(struct tme_recode_ic *ic,
		       tme_recode_thunk_off_t chain_fixup,
		       tme_uint32_t chain_info,
		       tme_recode_thunk_off_t insns_thunk_next,
		       tme_recode_thunk_off_t insns_thunk_return)
{
  tme_uint32_t return_imm32;
  tme_uint16_t insn_0_15_buffer;
  tme_uint32_t insn_0_15;
  const tme_uint8_t *thunk_bytes;
  tme_uint8_t opcode_buffer;
  tme_int32_t displacement;

  /* if this is a chain call: */
  if (chain_info & TME_RECODE_CHAIN_INFO_CALL) {

    /* there must be a return instructions thunk: */
    assert (insns_thunk_return != 0);

    /* a chain call emits:

       movl    $imm32, %TME_RECODE_X86_REG_CHAIN_RETURN_ADDRESS
       call    chain_subs_call_{near|far}_{unconditional|conditional}
       jmp/jnc chain_fixup_call_{near|far}

       on an ia32 host, the $imm32 is the absolute address of the
       instructions thunk to chain to.  on an x86-64 host, the $imm32
       is the offset of the instructions thunk to chain to. on both
       hosts, before fixup, the $imm32 indicates the chain epilogue.
       fix up the $imm32: */
    /* NB: chain_fixup points to the jump instruction.  the call
       instruction is a call rel32, which is five bytes long, and the
       $imm32 is immediately before that: */
    return_imm32 = TME_RECODE_X86_CHAIN_RETURN_ADDRESS(ic, insns_thunk_return);
    tme_recode_thunk_off_write(ic,
			       (chain_fixup
				- (5
				   + sizeof(tme_uint32_t))),
			       tme_uint32_t,
			       return_imm32);
  }

  /* if this is a chain near: */
  if ((chain_info
       & (TME_RECODE_CHAIN_INFO_NEAR
	  | TME_RECODE_CHAIN_INFO_FAR))
      == TME_RECODE_CHAIN_INFO_NEAR) {

    /* if this is an ia32 host: */
    if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

      /* advance the next instructions thunk offset past the chain
	 in far checks: */
      insns_thunk_next += TME_RECODE_IA32_CHAIN_IN_SIZE_FAR;
    }

    /* otherwise, this is an x86-64 host: */
    else {

      /* read the first two bytes of the first instruction of the
	 chain in.  this instruction starts checking that the guest
	 instruction source address matches the instructions thunk: */
      insn_0_15 = *tme_recode_thunk_off_read(ic, insns_thunk_next, tme_uint16_t, insn_0_15_buffer);

      /* if this instruction is a cmpq $imm32, %reg: */
      if (insn_0_15 == (tme_uint16_t) TME_RECODE_X86_INSN_CMP_Iz_REG_CHAIN_GUEST_SRC) {

	/* advance the next instructions thunk offset past the cmpq
	   $imm32, %reg; jnz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_CMPQ_IMM32_JNZ;
      }

      /* if this instruction is an lea disp32(%ip), %reg: */
      if (insn_0_15 == (tme_uint16_t) TME_RECODE_X86_INSN_LEA_DISP32_IP_REG_TLB_SCRATCH) {

	/* advance the next instructions thunk offset past the lea
	   disp32(%ip), %reg ; cmpq %reg, %reg ; jnz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_LEA_IP_CMP_JNZ;
      }

      /* if this instruction is a movq $imm64, %reg: */
      if (insn_0_15 == (tme_uint16_t) TME_RECODE_X86_INSN_MOVQ_IMM64_REG_TLB_SCRATCH) {

	/* advance the next instructions thunk offset past the movq
	   $imm64, %reg ; cmpq %reg, %reg ; jnz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVQ_CMP_JNZ;
      }

      /* if this instruction is a movl $imm32, %reg: */
      if (((tme_uint8_t) insn_0_15) == TME_RECODE_X86_INSN_MOVL_IMM32_REG_TLB_SCRATCH) {

	/* advance the next instructions thunk offset past the movl
	   $imm32, %reg ; cmpq %reg, %reg ; jnz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVL_CMP_JNZ;
      }

      /* read the first two bytes of the next instruction of the chain
	 in.  this instruction starts checking that the guest
	 instructions are still valid: */
      insn_0_15 = *tme_recode_thunk_off_read(ic, insns_thunk_next, tme_uint16_t, insn_0_15_buffer);

      /* if this instruction is a testb $imm8, addr32: */
      if (insn_0_15 == (tme_uint16_t) TME_RECODE_X86_INSN_TESTB_IMM8_ADDR32) {

	/* advance the next instructions thunk offset past the testb
	   $imm8, addr32 ; jz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_ADDR32_JZ;
      }

      /* if this instruction is a testb $imm8, disp32(%ip): */
      if (insn_0_15 == (tme_uint16_t) TME_RECODE_X86_INSN_TESTB_IMM8_DISP32_IP) {

	/* advance the next instructions thunk offset past the testb
	   $imm8, disp32(%ip) ; jz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_BASE_JZ;
      }

      /* if this instruction is a testb $imm8, disp32(%reg): */
      if (insn_0_15 == (tme_uint16_t) TME_RECODE_X86_INSN_TESTB_IMM8_DISP32_REG_CHAIN_GUEST_SRC) {

	/* advance the next instructions thunk offset past the testb
	   $imm8, disp32(%reg) ; jz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_TESTB_BASE_JZ;
      }

      /* if this instruction is a movq $imm64, %reg: */
      if (insn_0_15 == TME_RECODE_X86_INSN_MOVQ_IMM64_REG_TLB_SCRATCH) {

	/* advance the next instructions thunk offset past the movq
	   $imm64, %reg ; testb $imm8, (%reg) ; jz epilogue */
	insns_thunk_next += TME_RECODE_X86_64_CHAIN_IN_SIZE_MOVQ_TESTB_JZ;
      }
    }    
  }

  /* all chain fixup instructions are unconditional or conditional
     jump instructions with 32-bit displacements at their ends.
     unconditional jump instructions are five bytes, and conditional
     jump instructions are six bytes because they are escaped: */
     
  /* read the first opcode byte of the jump instruction: */
  thunk_bytes = tme_recode_thunk_off_read(ic, chain_fixup, tme_uint8_t, opcode_buffer);

  /* advance the chain fixup offset to the jump instruction displacement: */
  chain_fixup += (*thunk_bytes == TME_RECODE_X86_OPCODE_ESC_0F) + 1;

  /* rewrite the displacement for the instructions thunk: */
  displacement = insns_thunk_next - (chain_fixup + sizeof(tme_int32_t));
  tme_recode_thunk_off_write(ic, chain_fixup, tme_int32_t, displacement);

  /* return the next instructions thunk offset: */
  return (insns_thunk_next);
}

#ifdef TME_RECODE_DEBUG
#include <stdio.h>

/* this host function dumps a chain thunk: */
void
tme_recode_chain_thunk_dump(const struct tme_recode_ic *ic,
			    const struct tme_recode_chain_thunk *chain_thunk)
{
  tme_uint32_t chain_info;
  const char *s;
  tme_recode_thunk_off_t insns_thunk;

  printf("  x86 chain prologue: x/10i %p\n",
	 chain_thunk->tme_recode_x86_chain_thunk_prologue);
  for (chain_info = 0;
       chain_info <= (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
		      | TME_RECODE_CHAIN_INFO_CONDITIONAL
		      | TME_RECODE_CHAIN_INFO_NEAR
		      | TME_RECODE_CHAIN_INFO_FAR
		      | TME_RECODE_CHAIN_INFO_JUMP
		      | TME_RECODE_CHAIN_INFO_RETURN
		      | TME_RECODE_CHAIN_INFO_CALL);
       chain_info++) {
    switch (chain_info
	    & (TME_RECODE_CHAIN_INFO_JUMP
	       | TME_RECODE_CHAIN_INFO_RETURN
	       | TME_RECODE_CHAIN_INFO_CALL)) {
    case TME_RECODE_CHAIN_INFO_JUMP: s = "jump"; break;
    case TME_RECODE_CHAIN_INFO_RETURN:
      s = "return";
      if ((chain_info & TME_RECODE_CHAIN_INFO_FAR) == 0) {
	continue;
      }
      break;
    case TME_RECODE_CHAIN_INFO_CALL: s = "call"; break;
    default: continue;
    }
    insns_thunk = TME_RECODE_X86_CHAIN_SUBS(chain_thunk, chain_info);
    printf("  x86 chain %s %s %s subs: x/10i %p\n",
	   s,
	   (chain_info & TME_RECODE_CHAIN_INFO_FAR
	    ? "far"
	    : "near"),
	   (chain_info & TME_RECODE_CHAIN_INFO_CONDITIONAL
	    ? "conditional"
	    : "unconditional"),
	   tme_recode_thunk_off_to_pointer(ic,
					   insns_thunk,
					   void (*)(void)));
  }
  insns_thunk
    = TME_RECODE_X86_CHAIN_FIXUP_TARGET_ALTERNATE(ic,
						  (TME_RECODE_CHAIN_INFO_JUMP
						   + TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR));
  printf("  x86 chain fixup chain jump alternate near: x/10i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 insns_thunk,
					 void (*)(void)));
  insns_thunk
    = TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic,
					(TME_RECODE_CHAIN_INFO_JUMP
					 + TME_RECODE_CHAIN_INFO_NEAR));
  printf("  x86 chain fixup chain jump near, chain return alternate near: x/10i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 insns_thunk,
					 void (*)(void)));
  insns_thunk
    = TME_RECODE_X86_CHAIN_FIXUP_TARGET_ALTERNATE(ic,
						  (TME_RECODE_CHAIN_INFO_JUMP
						   + TME_RECODE_CHAIN_INFO_ALTERNATE_FAR));
  printf("  x86 chain fixup chain jump alternate far: x/3i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 insns_thunk,
					 void (*)(void)));
  insns_thunk
    = TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic,
					(TME_RECODE_CHAIN_INFO_JUMP
					 + TME_RECODE_CHAIN_INFO_FAR));
  printf("  x86 chain fixup chain jump far: x/2i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 insns_thunk,
					 void (*)(void)));
  insns_thunk
    = TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic,
					(TME_RECODE_CHAIN_INFO_CALL
					 + TME_RECODE_CHAIN_INFO_FAR));
  printf("  x86 chain fixup chain call far: x/2i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 insns_thunk,
					 void (*)(void)));
  insns_thunk
    = TME_RECODE_X86_CHAIN_FIXUP_TARGET(ic,
					(TME_RECODE_CHAIN_INFO_CALL
					 + TME_RECODE_CHAIN_INFO_NEAR));
  printf("  x86 chain fixup chain call near: x/2i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 insns_thunk,
					 void (*)(void)));
}

#endif /* TME_RECODE_DEBUG */
