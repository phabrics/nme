/* $Id: rc-x86-insns.c,v 1.7 2010/06/05 19:11:51 fredette Exp $ */

/* libtme/host/x86/rc-x86-insns.c - x86 host recode instructions support: */

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

_TME_RCSID("$Id: rc-x86-insns.c,v 1.7 2010/06/05 19:11:51 fredette Exp $");

/* this maps from recode opcode to x86 binary operation: */
#define TME_RECODE_X86_OPCODE_TO_BINOP(opcode) _tme_recode_x86_opcode_to_binop[(opcode) - TME_RECODE_OPCODE_SUB]
static const tme_uint8_t _tme_recode_x86_opcode_to_binop[] = {
#if TME_RECODE_OPCODE_SUB != 3
#error "TME_RECODE_OPCODE_ values changed, need to fix _tme_recode_x86_opcode_to_binop"
#endif
  TME_RECODE_X86_OPCODE_BINOP_SUB, /* TME_RECODE_OPCODE_SUB */
#if TME_RECODE_OPCODE_SUBC != 4
#error "TME_RECODE_OPCODE_ values changed, need to fix _tme_recode_x86_opcode_to_binop"
#endif
  TME_RECODE_X86_OPCODE_BINOP_SBB, /* TME_RECODE_OPCODE_SUBC */
#if TME_RECODE_OPCODE_AND != 5
#error "TME_RECODE_OPCODE_ values changed, need to fix _tme_recode_x86_opcode_to_binop"
#endif
  TME_RECODE_X86_OPCODE_BINOP_AND, /* TME_RECODE_OPCODE_AND */
#if TME_RECODE_OPCODE_OR != 6
#error "TME_RECODE_OPCODE_ values changed, need to fix _tme_recode_x86_opcode_to_binop"
#endif
  TME_RECODE_X86_OPCODE_BINOP_OR, /* TME_RECODE_OPCODE_OR */
#if TME_RECODE_OPCODE_XOR != 7
#error "TME_RECODE_OPCODE_ values changed, need to fix _tme_recode_x86_opcode_to_binop"
#endif
  TME_RECODE_X86_OPCODE_BINOP_XOR, /* TME_RECODE_OPCODE_XOR */
#if TME_RECODE_OPCODE_ADD != 8
#error "TME_RECODE_OPCODE_ values changed, need to fix _tme_recode_x86_opcode_to_binop"
#endif
  TME_RECODE_X86_OPCODE_BINOP_ADD, /* TME_RECODE_OPCODE_ADD */
#if TME_RECODE_OPCODE_ADDC != 9
#error "TME_RECODE_OPCODE_ values changed, need to fix _tme_recode_x86_opcode_to_binop"
#endif
  TME_RECODE_X86_OPCODE_BINOP_ADC, /* TME_RECODE_OPCODE_ADDC */
};

/* this maps from recode opcode to x86 shift group2 opcode: */
#define TME_RECODE_X86_OPCODE_TO_GRP2(opcode)	\
  ((((opcode)					\
     - TME_RECODE_OPCODE_SHLL)			\
    + TME_RECODE_X86_OPCODE_GRP2_SHL)		\
   + (((opcode)					\
       - TME_RECODE_OPCODE_SHLL)		\
      / 2))
#if TME_RECODE_X86_OPCODE_TO_GRP2(TME_RECODE_OPCODE_SHLL) != TME_RECODE_X86_OPCODE_GRP2_SHL
#error "TME_RECODE_OPCODE_ values changed"
#endif
#if TME_RECODE_X86_OPCODE_TO_GRP2(TME_RECODE_OPCODE_SHRL) != TME_RECODE_X86_OPCODE_GRP2_SHR
#error "TME_RECODE_OPCODE_ values changed"
#endif
#if TME_RECODE_X86_OPCODE_TO_GRP2(TME_RECODE_OPCODE_SHRA) != TME_RECODE_X86_OPCODE_GRP2_SAR
#error "TME_RECODE_OPCODE_ values changed"
#endif

/* this emits a call to a subs: */
static void
_tme_recode_x86_insn_subs(struct tme_recode_ic *ic,
			  const struct tme_recode_insn *insn)
{
  tme_uint8_t *thunk_bytes;

  /* force the destination operand to be in the same host register as
     the first source operand: */
  tme_recode_regs_dst_specific(ic,
			       insn,
			       0);

  /* a subs is allowed to destroy the a register, which must be the
     most-significant of the two host registers used for the subs
     second source operand.  if this is a double-host-size guest, we
     must free both the a register and the other least-significant
     host register, otherwise we only need to free the a register: */
#if (TME_RECODE_X86_REG_HOST_SUBS_SRC1 & 1) != 0
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
  assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1] == TME_RECODE_X86_REG_A);
  tme_recode_regs_host_free_specific(ic,
				     (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
				      ? TME_RECODE_X86_REG_HOST_SUBS_SRC1
				      : (TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1)));

  /* a subs is allowed to destroy the d and c registers, which must be
     the two host registers used for the subs first source operand.
     the d register must be a true host register, and the c register
     must not be a true host register.  if this isn't a
     double-host-size guest, we must free the d register.  we must
     always mark the c register as destroyed: */
#if (TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1) != TME_RECODE_REG_HOST_UNDEF
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
  assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 0] == TME_RECODE_X86_REG_D);
  assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1] == TME_RECODE_X86_REG_C);
  if (!TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {
    tme_recode_regs_host_free_specific(ic,
				       TME_RECODE_X86_REG_HOST_SUBS_SRC0);
  }
  ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* emit the call to the instruction's subs: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_CALL_RELz;
  thunk_bytes += 1 + sizeof(tme_uint32_t);
  ((tme_uint32_t *) thunk_bytes)[-1]
    = (insn->tme_recode_x86_insn_subs_thunk_off
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* finish this instruction: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this does a one's- or two's-complement negation of the destination
   operand: */
static void
_tme_recode_x86_insn_negate(struct tme_recode_ic *ic,
			    const struct tme_recode_insn *insn)
{
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86;
  unsigned int opcode;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the opcode for this instruction: */
  opcode = insn->tme_recode_insn_opcode;
  assert (opcode == TME_RECODE_OPCODE_SUB
	  || opcode == TME_RECODE_OPCODE_ANDN
	  || opcode == TME_RECODE_OPCODE_ORN
	  || opcode == TME_RECODE_OPCODE_XORN);

  /* get the (first) x86 register to negate: */
  reg_x86 = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst];

  /* emit a not %reg or neg %reg: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86);
  }
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0] = TME_RECODE_X86_OPCODE_GRP3_Ev;
#if TME_RECODE_OPCODE_SUB <= TME_RECODE_OPCODE_ANDN || TME_RECODE_OPCODE_SUB <= TME_RECODE_OPCODE_ORN || TME_RECODE_OPCODE_SUB <= TME_RECODE_OPCODE_XORN
#error "TME_RECODE_OPCODE_ values changed"
#endif
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
    = (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86), 0)
       + (opcode < TME_RECODE_OPCODE_SUB
	  ? TME_RECODE_X86_MOD_OPREG_RM(0, TME_RECODE_X86_OPCODE_GRP3_NOT)
	  : TME_RECODE_X86_MOD_OPREG_RM(0, TME_RECODE_X86_OPCODE_GRP3_NEG)));
  thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2;

  /* if this is a double-host-size operation: */
  if (TME_RECODE_SIZE_IS_DOUBLE_HOST(insn->tme_recode_insn_size)) {

    /* get the second x86 register to negate: */
    reg_x86 = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst + 1];

    /* if this is a two's complement negation: */
    if (opcode == TME_RECODE_OPCODE_SUB) {

      /* emit an adc $0, %reg: */
      if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
	thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86);
      }
      thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
      thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				      TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADC));
      thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2] = 0;
      thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 3;
    }

    /* emit a not %reg or a neg %reg: */
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
      thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86);
    }
    thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0] = TME_RECODE_X86_OPCODE_GRP3_Ev;
#if TME_RECODE_OPCODE_SUB <= TME_RECODE_OPCODE_ORN || TME_RECODE_OPCODE_SUB <= TME_RECODE_OPCODE_XORN
#error "TME_RECODE_OPCODE_ values changed"
#endif
    thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
      = (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86), 0)
	 + (opcode < TME_RECODE_OPCODE_SUB
	    ? TME_RECODE_X86_MOD_OPREG_RM(0, TME_RECODE_X86_OPCODE_GRP3_NOT)
	    : TME_RECODE_X86_MOD_OPREG_RM(0, TME_RECODE_X86_OPCODE_GRP3_NEG)));
    thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2;
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits a host-sized binary operation instruction with an
   immediate right source operand: */
static void
_tme_recode_x86_insn_binop_imm(struct tme_recode_ic *ic,
			       const struct tme_recode_insn *insn)
{
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86_dst;
  tme_int32_t imm_32;
  unsigned int imm_size;

  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the x86 destination register: */
  reg_x86_dst = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst];

  /* emit any rex prefix: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_dst);
  }

  /* emit the modR/M byte for this instruction: */
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_dst),
				  TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_TO_BINOP(insn->tme_recode_insn_opcode)));

  /* emit the immediate for this instruction as a sign-extended 32 bits: */
  imm_32 = insn->tme_recode_insn_imm_guest;
  *((tme_int32_t *) &thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2]) = imm_32;

  /* see if this sign-extended 32-bit immediate can be a sign-extended
     eight bits instead: */
  imm_size
    = (imm_32 == (tme_int8_t) imm_32
       ? sizeof(tme_int8_t)
       : sizeof(tme_int32_t));

  /* emit the opcode for this instruction: */
  /* NB: we take advantage of the fact that the group one opcodes for
     an eight-bit immediate and a 32-bit immediate are separated by a
     distance of two, which is sizeof(tme_int32_t) / 2: */
#if (TME_RECODE_X86_OPCODE_GRP1_Ib_Ev - TME_RECODE_X86_OPCODE_GRP1_Iz_Ev) != 2
#error "TME_RECODE_X86_OPCODE_GRP1_ opcodes changed"
#endif
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
    = (TME_RECODE_X86_OPCODE_GRP1_Ib_Ev
       - (imm_size / 2));

  /* finish this instruction: */
  thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2 + imm_size;
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits a host-sized binary operation instruction with a
   register right source operand: */
static void
_tme_recode_x86_insn_binop(struct tme_recode_ic *ic,
			   const struct tme_recode_insn *insn)
{
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86_src;
  unsigned int reg_x86_dst;

  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the x86 source and destination registers: */
  reg_x86_src = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_src[1]];
  reg_x86_dst = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst];

  /* emit any rex prefix: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    thunk_bytes[0]
      = (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86_src)
	 | TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_dst));
  }

  /* emit the opcode for this instruction: */
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
    = (TME_RECODE_X86_OPCODE_TO_BINOP(insn->tme_recode_insn_opcode)
       + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev);

  /* emit the modR/M byte for this instruction: */
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_dst),
				  TME_RECODE_X86_REG(reg_x86_src));

  /* finish this instruction: */
  thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2;
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits a host-sized zero-extend or sign-extend instruction: */
static void
_tme_recode_x86_insn_ext(struct tme_recode_ic *ic,
			 const struct tme_recode_insn *insn)
{
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86_src;
  unsigned int reg_x86_dst;
  unsigned int extend;
  unsigned int rex;
  unsigned int mod_reg_rm;

  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the x86 source and destination registers: */
  reg_x86_src = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_src[0]];
  reg_x86_dst = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst];

  /* get the size of the source operand: */
  extend = insn->tme_recode_insn_operand_src[1];
  assert (extend >= TME_RECODE_SIZE_8
	  && extend < ic->tme_recode_ic_reg_size);

  /* assume that this is not an extension from eight bits, and start
     any rex prefix with the source register: */
  rex = TME_RECODE_X86_REX_B(0, reg_x86_src);

  /* if this is an extension from eight bits: */
  if (extend == TME_RECODE_SIZE_8) {

    /* if this is an ia32 host, and the source register has no
       eight-bit register encoding: */
    if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	&& reg_x86_src >= TME_RECODE_X86_REG_SP) {

      /* we destroy the c register: */
      ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

      /* copy the source register into the scratch c register: */
      _tme_recode_x86_emit_reg_copy(thunk_bytes, reg_x86_src, TME_RECODE_X86_REG_C);

      /* the source register is now the scratch c register: */
      reg_x86_src = TME_RECODE_X86_REG_C;
    }

    /* restart any rex prefix: */
    rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_8, reg_x86_src);
  }

  /* add the destination register to any rex prefix: */
  rex |= TME_RECODE_X86_REX_R(0, reg_x86_dst);

  /* make the modR/M byte for this instruction, shifted up to the
     second byte to make room for the opcode: */
  mod_reg_rm
    = (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_src),
				   TME_RECODE_X86_REG(reg_x86_dst))
       << 8);

  /* if this is an x86-64 host: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

    /* if this is a sign-extension: */
    if (insn->tme_recode_insn_opcode == TME_RECODE_OPCODE_EXTS) {

      /* sign extension is a 64-bit operation: */
      rex |= TME_RECODE_X86_REX_W(TME_RECODE_SIZE_HOST);
    }

    /* emit any rex prefix: */
    thunk_bytes[0] = rex;
    thunk_bytes += (rex != 0);
  }

  /* if this is an x86-64 host and this is an extension from 32
     bits: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
      && extend == TME_RECODE_SIZE_32) {

    /* emit either a movl %src, %dst or a movslq %src, %dst: */
    *((tme_uint16_t *) &thunk_bytes[0])
      = ((insn->tme_recode_insn_opcode == TME_RECODE_OPCODE_EXTS
	  ? TME_RECODE_X86_OPCODE_MOVS_El_Gv
	  : (TME_RECODE_X86_OPCODE_BINOP_MOV
	     + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv))
	 + mod_reg_rm);
    thunk_bytes += 2;
  }

  /* otherwise, either this is not an x86-64 host or this is an
     extension from 16 or 8 bits: */
  else {
    assert (extend == TME_RECODE_SIZE_16
	    || extend == TME_RECODE_SIZE_8);

    /* emit the two-byte opcode escape: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;

    /* emit either a movzb %src, %dst, or a movzw %src, %dst, or a
       movsb %src %dst, or a movsw %src, %dst: */
#if (TME_RECODE_X86_OPCODE0F_MOVS_Eb_Gv ^ TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv) != (TME_RECODE_X86_OPCODE0F_MOVS_Ew_Gv ^ TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv)
#error "TME_RECODE_X86_OPCODE0F_MOV values changed"
#endif
#if (TME_RECODE_OPCODE_EXTS - TME_RECODE_OPCODE_EXTZ) != 1
#error "TME_RECODE_OPCODE_ values changed"
#endif
#if (TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv ^ TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv) != (TME_RECODE_X86_OPCODE0F_MOVS_Ew_Gv ^ TME_RECODE_X86_OPCODE0F_MOVS_Eb_Gv)
#error "TME_RECODE_X86_OPCODE0F_MOV values changed"
#endif
#if (TME_RECODE_SIZE_16 - TME_RECODE_SIZE_8) != 1
#error "TME_RECODE_SIZE_ values changed"
#endif
    *((tme_uint16_t *) &thunk_bytes[1])
      = ((TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv
	  + ((extend
	      - TME_RECODE_SIZE_8)
	     * (TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv
		- TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv))
	  + ((insn->tme_recode_insn_opcode
	      - TME_RECODE_OPCODE_EXTZ)
	     * (TME_RECODE_X86_OPCODE0F_MOVS_Eb_Gv
		- TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv)))
	 + mod_reg_rm);
    thunk_bytes += 3;
  }

  /* finish this instruction: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits a host-sized shift-by-immediate instruction: */
static void
_tme_recode_x86_insn_shift_imm(struct tme_recode_ic *ic,
			       const struct tme_recode_insn *insn)
{
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86_dst;

  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the x86 destination register: */
  reg_x86_dst = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst];

  /* emit any rex prefix: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
    thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_dst);
  }

  /* emit the opcode for this instruction: */
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;

  /* emit the modR/M byte for this instruction: */
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_dst),
				  TME_RECODE_X86_OPCODE_TO_GRP2(insn->tme_recode_insn_opcode));

  /* emit the immediate shift count for this instruction: */
  thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2] = insn->tme_recode_insn_imm_uguest;

  /* finish this instruction: */
  thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 3;
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits a guest instruction: */
static void
_tme_recode_x86_insn_guest(struct tme_recode_ic *ic,
			   struct tme_recode_insn *insn)
{
  tme_uint8_t *thunk_bytes;
  unsigned long reg_host_free;

  /* since a guest function may fault and never return, we have to
     clean all dirty host registers so that all guest registers are
     correct in the guest ic state at the time of the fault: */
  tme_recode_regs_host_clean_all(ic);

  /* a guest function that takes an undefined first source operand,
     must take an undefined second source operand: */
  assert (insn->tme_recode_insn_operand_src[0] != TME_RECODE_OPERAND_UNDEF
	  || insn->tme_recode_insn_operand_src[1] == TME_RECODE_OPERAND_UNDEF);

  /* if this is an ia32 host: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

    /* if this guest function takes an undefined second source operand: */
    if (insn->tme_recode_insn_operand_src[1] == TME_RECODE_OPERAND_UNDEF) {

      /* if this guest function returns a register value, we need to
	 reserve the host register(s) for the return value, since we
	 won't reserve by loading the second source operand: */
      if (insn->tme_recode_insn_operand_dst != TME_RECODE_OPERAND_NULL) {
	tme_recode_regs_host_reserve(ic,
				     (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
				      ? (TME_RECODE_X86_REG_HOST_FREE_CALL - 1)
				      : TME_RECODE_X86_REG_HOST_FREE_CALL));
      }
    }

    /* otherwise, this guest function takes a defined second source operand: */
    else {

      /* if this is a double-host-size guest, load the second source
	 operand into %eax:%ebp, otherwise load it into %eax: */
#if (TME_RECODE_X86_REG_HOST_FREE_CALL & 1) != 1
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
      tme_recode_regs_src_specific(ic,
				   insn,
				   insn->tme_recode_insn_operand_src[1],
				   (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
				    ? (TME_RECODE_X86_REG_HOST_FREE_CALL - 1)
				    : TME_RECODE_X86_REG_HOST_FREE_CALL));
    }

    /* if this guest function takes a defined first source operand: */
    if (insn->tme_recode_insn_operand_src[0] != TME_RECODE_OPERAND_UNDEF) {

      /* load the first source operand into any host register: */
      insn->tme_recode_insn_operand_src[0]
	= tme_recode_regs_src_any(ic,
				  insn,
				  insn->tme_recode_insn_operand_src[0]);
    }

    /* if this guest function returns a register value: */
    if (insn->tme_recode_insn_operand_dst != TME_RECODE_OPERAND_NULL) {

      /* if this is a double-host-size guest, force the destination
	 operand to be in %eax:%ebp, otherwise force it to be directly
	 in the return value register (%eax): */
      tme_recode_regs_dst_specific(ic,
				   insn,
				   0);

      /* free all host registers that be will destroyed by the call,
	 except for the first one, which we are using for the
	 destination operand: */
      reg_host_free = TME_RECODE_X86_REG_HOST_FREE_CALL + 1;
    }

    /* otherwise, this guest function does not return a register value: */
    else {

      /* we must assume that this guest function modifies all guest
	 registers: */

      /* unreserve all registers: */
      tme_recode_regs_host_unreserve_all(ic);

      /* free all host registers: */
      reg_host_free = TME_RECODE_REG_HOST(0);
    }

    /* free host registers: */
    tme_recode_regs_host_free_many(ic, reg_host_free);

    /* the guest function call destroys the c register: */
    ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

    /* start more instructions: */
    tme_recode_x86_insns_start(ic, thunk_bytes);

    /* if this guest function takes two undefined source operands: */
    if (insn->tme_recode_insn_operand_src[0] == TME_RECODE_OPERAND_UNDEF) {

      /* push garbage source operands for the guest function: */
      thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, - (int) (sizeof(tme_recode_uguest_t) * 2));
    }

    /* otherwise, this guest function takes at least one defined
       source operand: */
    else {

      /* push the second source operand for the guest function.  NB that
	 if double-host-size guests are supported, but this isn't a
	 double-host-size guest, we use a garbage word on the stack as
	 the most-significant half of that argument (which is okay since
	 the guest functions are supposed to truncate their arguments to
	 the expected size): */
      if (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {
	_tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_A);
	_tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_BP);
      }
      else {
	if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {
	  thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, - (int) (sizeof(tme_recode_uguest_t) / 2));
	}
	_tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_A);
      }

      /* push the first source operand for the guest function.  if
	 double-host size guests are supported, assume that this is a
	 double-host-size guest and push the most-significant half of
	 the argument first.  if this isn't a double-host-size guest,
	 this will push some unknown register (which is okay since the
	 guest functions are supposed to truncate their arguments to the
	 expected size): */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {
	_tme_recode_x86_emit_reg_push(thunk_bytes,
				      tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_src[0] + 1]);
      }
      _tme_recode_x86_emit_reg_push(thunk_bytes,
				    tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_src[0]]);
    }

    /* emit the instruction to push the struct tme_ic * argument for
       the guest function, and then the call instruction to the guest
       function: */
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_OPCODE_PUSH_Gv(TME_RECODE_X86_REG_IC)
	 + (TME_RECODE_X86_OPCODE_CALL_RELz << 8));
    thunk_bytes += 2 + sizeof(tme_uint32_t);
    ((tme_int32_t *) thunk_bytes)[-1]
      = (tme_recode_function_to_thunk_off(ic, insn->tme_recode_insn_guest_func)
	 - tme_recode_build_to_thunk_off(ic, thunk_bytes));

    /* remove the guest function arguments from the stack: */
    thunk_bytes 
      = _tme_recode_x86_emit_adjust_sp(thunk_bytes,
				       (sizeof(struct tme_ic *)
					+ sizeof(tme_recode_uguest_t)
					+ sizeof(tme_recode_uguest_t)));
  }

  /* otherwise, this is an x86-64 host: */
  else {

    /* if this is a double-host-size guest: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {

      /* if this guest function takes an undefined first source operand: */
      if (insn->tme_recode_insn_operand_src[0] == TME_RECODE_OPERAND_UNDEF) {

	/* if this guest function returns a register value, we need to
	   reserve the host register(s) for the return value, since we
	   won't reserve it by loading the first source operand: */
	if (insn->tme_recode_insn_operand_dst != TME_RECODE_OPERAND_NULL) {
	  tme_recode_regs_host_reserve(ic,
				       TME_RECODE_X86_REG_HOST_FREE_CALL - 1);
	}
      }

      /* otherwise, this guest function takes a defined first source operand: */
      else {

	/* load the first source operand into the first host register
	   pair that is destroyed by the call: */
#if (TME_RECODE_X86_REG_HOST_FREE_CALL & 1) != 1
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
	tme_recode_regs_src_specific(ic,
				     insn,
				     insn->tme_recode_insn_operand_src[0],
				     TME_RECODE_X86_REG_HOST_FREE_CALL - 1);
      }

      /* if this guest function takes a defined second source operand: */
      if (insn->tme_recode_insn_operand_src[1] != TME_RECODE_OPERAND_UNDEF) {

	/* load the second source operand into the second host register
	   pair that is destroyed by the call: */
	tme_recode_regs_src_specific(ic,
				     insn,
				     insn->tme_recode_insn_operand_src[1],
				     TME_RECODE_X86_REG_HOST_FREE_CALL + 1);
      }

      /* if this guest function returns a register value: */
      if (insn->tme_recode_insn_operand_dst != TME_RECODE_OPERAND_NULL) {

	/* force the destination operand to be in %rax:%rbp: */
	tme_recode_regs_dst_specific(ic,
				     insn,
				     0);

	/* free all host registers that be will destroyed by the call,
	   except for the first one, which we are using for the
	   destination operand: */
	reg_host_free = TME_RECODE_X86_REG_HOST_FREE_CALL + 1;
      }

      /* otherwise, this guest function does not return a register value: */
      else {

	/* unreserve all registers: */
	tme_recode_regs_host_unreserve_all(ic);

	/* free all host registers: */
	reg_host_free = TME_RECODE_REG_HOST(0);
      }

      /* free host registers: */
      tme_recode_regs_host_free_many(ic, reg_host_free);

      /* the guest function call destroys the c register: */
      ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

      /* start more instructions: */
      tme_recode_x86_insns_start(ic, thunk_bytes);

      /* move the first and second source operands into the function
	 argument registers: */
      if (insn->tme_recode_insn_operand_src[0] != TME_RECODE_OPERAND_UNDEF) {
	_tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_BP, TME_RECODE_X86_REG_SI);
	_tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_A, TME_RECODE_X86_REG_D);
      }
      if (insn->tme_recode_insn_operand_src[1] != TME_RECODE_OPERAND_UNDEF) {
	_tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_N(10), TME_RECODE_X86_REG_C);
	_tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_N(11), TME_RECODE_X86_REG_N(8));
      }
    }

    /* otherwise, this is not a double-host-size guest: */
    else {

      /* if this guest function takes a defined first source operand: */
      if (insn->tme_recode_insn_operand_src[0] != TME_RECODE_OPERAND_UNDEF) {

	/* load the first source operand directly into its (first) argument
	   register (%rsi): */
	tme_recode_regs_src_specific(ic,
				     insn,
				     insn->tme_recode_insn_operand_src[0],
				     TME_RECODE_X86_REG_HOST_ARG(1));
      }

      /* if this guest function takes a defined second source operand: */
      if (insn->tme_recode_insn_operand_src[1] != TME_RECODE_OPERAND_UNDEF) {

	/* load the second source operand directly into %rdx.  if
	   double-host-size guests are not supported, this is the
	   correct argument register for the second source operand.
	   otherwise, double-host-size guests are supported, and %rdx is
	   the most-significant of the two first source operand argument
	   registers.  in that case, below we'll copy %rdx into the
	   first argument register for the second source operand, but
	   otherwise we'll leave %rdx loaded, which is okay since the
	   guest functions are supposed to truncate their arguments to
	   the expected size: */
	tme_recode_regs_src_specific(ic,
				     insn,
				     insn->tme_recode_insn_operand_src[1],
				     TME_RECODE_X86_REG_HOST_ARG(2));
      }

      /* if this guest function returns a register value: */
      if (insn->tme_recode_insn_operand_dst != TME_RECODE_OPERAND_NULL) {

	/* force the destination operand to be directly in the return
	   value register (%rax): */
	tme_recode_regs_host_reserve(ic,
				     TME_RECODE_X86_REG_HOST_FREE_CALL);
	tme_recode_regs_dst_specific(ic,
				     insn,
				     2);

	/* free all host registers that will be destroyed by the call,
	   except for the first one, which we are using for the
	   destination operand: */
	reg_host_free = TME_RECODE_X86_REG_HOST_FREE_CALL + 1;
      }

      /* otherwise, this guest function does not return a register value: */
      else {

	/* unreserve all registers: */
	tme_recode_regs_host_unreserve_all(ic);

	/* free all host registers: */
	reg_host_free = TME_RECODE_REG_HOST(0);
      }

      /* free host registers: */
      tme_recode_regs_host_free_many(ic, reg_host_free);

      /* the guest function call destroys the c register: */
      ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

      /* start more instructions: */
      tme_recode_x86_insns_start(ic, thunk_bytes);

      /* if double-host-size guests are supported: */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {

	/* if this guest function takes a defined second source operand: */
	if (insn->tme_recode_insn_operand_src[1] != TME_RECODE_OPERAND_UNDEF) {

	  /* copy the second source operand from %rdx into the second
	     source operand function argument registers.  NB that this
	     leaves the most-significant half of the argument (%r8)
	     undefined, which is okay since the guest functions are
	     supposed to truncate their arguments to the expected
	     size: */
	  _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_D, TME_RECODE_X86_REG_C);
	}
      }
    }

    /* emit a movq %ic, %rdi: */
    _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_IC, TME_RECODE_X86_REG_DI);

    /* we must assume that we can't reach the guest function from the
       instruction thunk with a 32-bit displacement.  emit a direct
       call to the guest function using %rax: */
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, TME_RECODE_X86_REG_A)
	 + (TME_RECODE_X86_OPCODE_MOV_Iv_Gv(TME_RECODE_X86_REG_A)
	    << 8));
    memcpy(thunk_bytes + 2,
	   &insn->tme_recode_insn_guest_func,
	   TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));
    thunk_bytes += 2 + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_OPCODE_GRP5
	 + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_A),
					TME_RECODE_X86_OPCODE_GRP5_CALL)
	    << 8));
    thunk_bytes += 2;
  }

  /* if the guest function returned a register value, and this is a
     double-host-size guest: */
  if (insn->tme_recode_insn_operand_dst != TME_RECODE_OPERAND_NULL
      && TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {

    /* move the guest function return value from d:a into ax:bp: */
    _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_A, TME_RECODE_X86_REG_BP);
    _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_D, TME_RECODE_X86_REG_A);
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits a read or a write instruction: */
static void
_tme_recode_x86_insn_rw(struct tme_recode_ic *ic,
			struct tme_recode_insn *insn)
{
  const struct tme_recode_rw_thunk *rw_thunk;
  unsigned int reg_host_address;
  tme_uint8_t *thunk_bytes;

  /* get the read/write thunk: */
  rw_thunk = insn->tme_recode_insn_rw_thunk;

  /* set the instruction size to the address size: */
  insn->tme_recode_insn_size = rw_thunk->tme_recode_rw_thunk_address_size;

  /* the guest address is often a value that has been calculated in a
     guest temporary register.  before we clean all dirty host
     registers, notify that the guest address will be the first source
     operand we load, to hopefully avoid a store of a dirty guest
     temporary register: */
  tme_recode_regs_src_notify(ic,
			     insn,
			     insn->tme_recode_insn_operand_src[0]);

  /* since a read or a write instruction may fault and never return,
     we have to clean all dirty host registers so that all guest
     registers are correct in the guest ic state at the time of the
     fault: */
  tme_recode_regs_host_clean_all(ic);

  /* if this is a double-host-size guest, load the guest address into
     the a:bp register pair, otherwise load it into the a register: */
#if (TME_RECODE_X86_REG_HOST_FREE_CALL & 1) != 1
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
  reg_host_address
    = (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
       ? (TME_RECODE_X86_REG_HOST_FREE_CALL - 1)
       : TME_RECODE_X86_REG_HOST_FREE_CALL);
  tme_recode_regs_src_specific(ic,
			       insn,
			       insn->tme_recode_insn_operand_src[0],
			       reg_host_address);

  /* set the instruction size to the register size: */
  insn->tme_recode_insn_size = rw_thunk->tme_recode_rw_thunk_reg_size;

  /* if this is a write: */
  if (rw_thunk->tme_recode_rw_thunk_write) {

    /* load the value to write into the first host register (pair): */
    tme_recode_regs_src_specific(ic,
				 insn,
				 insn->tme_recode_insn_operand_src[1],
				 TME_RECODE_REG_HOST(0));

    /* unreserve all registers: */
    tme_recode_regs_host_unreserve_all(ic);

    /* a write thunk destroys the guest address: */
    tme_recode_regs_host_free_specific(ic,
				       reg_host_address);
  }

  else {

    /* force the destination register into the same register(s)
       holding the guest address: */
    tme_recode_regs_dst_specific(ic,
				 insn,
				 0);
  }

  /* a read/write thunk is allowed to destroy the d and c registers,
     which must be the two host registers used for the subs first
     source operand.  the d register must be a true host register, and
     the c register must not be a true host register.  if this isn't a
     double-host-size guest, we must free the d register.  we must
     always mark the c register as destroyed: */
#if (TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1) != TME_RECODE_REG_HOST_UNDEF
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
  assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 0] == TME_RECODE_X86_REG_D);
  assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1] == TME_RECODE_X86_REG_C);
  if (!TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {
    tme_recode_regs_host_free_specific(ic,
				       TME_RECODE_X86_REG_HOST_SUBS_SRC0);
  }
  ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;
    
  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* emit the call to the instruction's subs: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_CALL_RELz;
  thunk_bytes += 1 + sizeof(tme_uint32_t);
  ((tme_uint32_t *) thunk_bytes)[-1]
    = (rw_thunk->tme_recode_x86_rw_thunk_subs
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* add any zero- or sign-extension instruction: */
  *((tme_uint32_t *) thunk_bytes) = rw_thunk->tme_recode_x86_rw_thunk_extend;
  thunk_bytes += rw_thunk->tme_recode_x86_rw_thunk_extend_size;

  /* finish this instruction: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this recodes one instruction: */
static void
_tme_recode_x86_insn_emit(struct tme_recode_ic *ic,
			  struct tme_recode_insn *insn)
{
  tme_uint32_t opcode_mask;
  const struct tme_recode_flags_thunk *flags_thunk;
  unsigned int size_saved;
  signed int operand_src;
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86_src;
  unsigned int reg_x86_dst;
  unsigned int rex;
  unsigned int mod_reg_rm;
  tme_uint8_t *thunk_bytes_other;
  unsigned int window_i;
  unsigned long relv;

  /* get the mask for this opcode: */
  opcode_mask = (1 << insn->tme_recode_insn_opcode);

  /* if this is an integer opcode: */
  if (__tme_predict_true(opcode_mask
			 & ((1 << TME_RECODE_OPCODES_INTEGER) - 1))) {

    /* if this instruction has a flags thunk: */
    flags_thunk = insn->tme_recode_insn_flags_thunk;
    if (__tme_predict_false(flags_thunk != NULL)) {

      /* if this is a zero- or sign-extension: */
      if (__tme_predict_false(opcode_mask
			      & ((1 << TME_RECODE_OPCODE_EXTZ)
				 | (1 << TME_RECODE_OPCODE_EXTS)))) {

	/* save the instruction size: */
	size_saved = insn->tme_recode_insn_size;

	/* temporarily set the instruction size to the size of the
	   source operand: */
	insn->tme_recode_insn_size = insn->tme_recode_insn_operand_src[1];

	/* set the condition code subs for this opcode at this size and
	   source operand size: */
#if (TME_RECODE_OPCODE_EXTS - TME_RECODE_OPCODE_EXTZ) != 1
#error "TME_RECODE_OPCODE_ values changed"
#endif
	insn->tme_recode_x86_insn_subs_thunk_off
	  = (flags_thunk->tme_recode_x86_flags_thunk_sizes
	     [size_saved - TME_RECODE_SIZE_8]
	     .tme_recode_x86_flags_thunk_size_subs_ext
	     [insn->tme_recode_insn_size - TME_RECODE_SIZE_8]
	     [insn->tme_recode_insn_opcode - TME_RECODE_OPCODE_EXTZ]);

	/* load the first source operand into the host register for a
	   subs first source operand and destination: */
	tme_recode_regs_src_specific(ic,
				     insn,
				     insn->tme_recode_insn_operand_src[0],
				     TME_RECODE_X86_REG_HOST_SUBS_DST);

	/* restore the instruction size: */
	insn->tme_recode_insn_size = size_saved;
      }

      /* otherwise, this is not a zero- or sign-extension: */
      else {

	/* set the condition code subs for this opcode at this size: */
	insn->tme_recode_x86_insn_subs_thunk_off
	  = (flags_thunk->tme_recode_x86_flags_thunk_sizes
	     [insn->tme_recode_insn_size - TME_RECODE_SIZE_8]
	     .tme_recode_x86_flags_thunk_size_subs[insn->tme_recode_insn_opcode]);

	/* load the first source operand into the host register for a
	   subs first source operand and destination: */
	tme_recode_regs_src_specific(ic,
				     insn,
				     insn->tme_recode_insn_operand_src[0],
				     TME_RECODE_X86_REG_HOST_SUBS_DST);

	/* load the second source operand into the host register for a
	   subs second source operand: */
	tme_recode_regs_src_specific(ic,
				     insn,
				     insn->tme_recode_insn_operand_src[1],
				     TME_RECODE_X86_REG_HOST_SUBS_SRC1);
      }

      /* if this is an x86-64 host, and this condition code thunk calls
	 a guest function: */
      if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
	  && flags_thunk->tme_recode_x86_flags_thunk_has_guest_func) {

	/* we need to free all host registers that can be destroyed by
	   a C function.  the first such register must be the a
	   register, and this is freed by _tme_recode_x86_insn_subs(),
	   so we only need to free the others: */
	assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_FREE_CALL] == TME_RECODE_X86_REG_A);
#if (TME_RECODE_X86_REG_HOST_FREE_CALL & 1) != 1
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
	tme_recode_regs_host_free_many(ic, TME_RECODE_X86_REG_HOST_FREE_CALL + 1);
      }

      /* emit a call to the condition code subs: */
      _tme_recode_x86_insn_subs(ic, insn);
      return;
    }
  }

  /* if the first source operand is zero: */
  operand_src = insn->tme_recode_insn_operand_src[0];
  if (operand_src == TME_RECODE_OPERAND_ZERO) {

    /* if this opcode is a copy or a negation when the first source
       operand is zero: */
    if (opcode_mask
	& ((1 << TME_RECODE_OPCODE_ORN)
	   | (1 << TME_RECODE_OPCODE_XORN)
	   | (1 << TME_RECODE_OPCODE_SUB)
	   | (1 << TME_RECODE_OPCODE_OR)
	   | (1 << TME_RECODE_OPCODE_XOR)
	   | (1 << TME_RECODE_OPCODE_ADD))) {

      /* load the second source operand and copy it into the
	 destination operand: */
      tme_recode_regs_src_any(ic,
			      insn,
			      insn->tme_recode_insn_operand_src[1]);
      insn->tme_recode_insn_operand_dst
        = tme_recode_regs_dst_specific(ic,
				       insn,
				       0);

      /* if this opcode is a negation: */
      if (opcode_mask
	  & ((1 << TME_RECODE_OPCODE_ORN)
	     | (1 << TME_RECODE_OPCODE_XORN)
	     | (1 << TME_RECODE_OPCODE_SUB))) {

	/* negate the destination operand: */
	_tme_recode_x86_insn_negate(ic, insn);
      }

      /* done: */
      return;
    }
  }

  /* otherwise, the first source operand is not zero: */

  /* if the first source operand is a guest register: */
  else if (operand_src >= TME_RECODE_REG_GUEST(0)) {

    /* if this opcode makes a zero when the two source operands are
       the same guest register, and the two source operands are the
       same guest register: */
    if ((opcode_mask
	 & ((1 << TME_RECODE_OPCODE_SUB)
	    | (1 << TME_RECODE_OPCODE_XOR)))
	&& (operand_src
	    == insn->tme_recode_insn_operand_src[1])) {
      
      /* zero the destination operand: */
      tme_recode_host_reg_zero(ic,
			       NULL,
			       tme_recode_regs_dst_any(ic,
						       insn));
      return;
    }
  }

  /* if this is a binary operation: */
  if (opcode_mask
      & ((1 << TME_RECODE_OPCODE_ANDN)
	 | (1 << TME_RECODE_OPCODE_ORN)
	 | (1 << TME_RECODE_OPCODE_XORN)
	 | (1 << TME_RECODE_OPCODE_SUB)
	 | (1 << TME_RECODE_OPCODE_SUBC)
	 | (1 << TME_RECODE_OPCODE_AND)
	 | (1 << TME_RECODE_OPCODE_OR)
	 | (1 << TME_RECODE_OPCODE_XOR)
	 | (1 << TME_RECODE_OPCODE_ADD)
	 | (1 << TME_RECODE_OPCODE_ADDC))) {

    /* if the right source operand is an immediate that we can emit
       directly in the instruction(s): */
    /* NB: on an ia32 host, we can emit all immediates directly in the
       instruction(s), even for a double-host-size instruction.  on an
       x86-64 host, we can only emit an immediate whose one or two
       host-sized parts can be sign-extended from 32-bit parts: */
    if (insn->tme_recode_insn_operand_src[1] == TME_RECODE_OPERAND_IMM
	&& (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	    || ((((signed long) insn->tme_recode_insn_imm_guest)
		 == (tme_int32_t) insn->tme_recode_insn_imm_guest)
		&& (!TME_RECODE_SIZE_IS_DOUBLE_HOST(insn->tme_recode_insn_size)
		    || (((signed long)
			 (insn->tme_recode_insn_imm_guest
			  >> ((sizeof(insn->tme_recode_insn_imm_guest) * 8) / 2)))
			== ((tme_int32_t)
			    (insn->tme_recode_insn_imm_guest
			     >> ((sizeof(insn->tme_recode_insn_imm_guest) * 8) / 2)))))))) {

      /* if this is a logical noncommutative instruction: */
      if (opcode_mask
	  & ((1 << TME_RECODE_OPCODE_ANDN)
	     | (1 << TME_RECODE_OPCODE_ORN)
	     | (1 << TME_RECODE_OPCODE_XORN))) {

	/* invert the immediate: */
	insn->tme_recode_insn_imm_uguest = ~insn->tme_recode_insn_imm_uguest;

	/* convert the opcode into its commutative version: */
	/* NB: we don't need to change opcode_mask, since the
	   only remaining tests are for additive operations: */
	insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_LOGICALN_TO_PLAIN(insn->tme_recode_insn_opcode);
      }

      /* load the left source operand: */
      insn->tme_recode_insn_operand_src[0]
	= tme_recode_regs_src_any(ic,
				  insn,
				  insn->tme_recode_insn_operand_src[0]);

      /* copy the left source operand into the destination operand: */
      insn->tme_recode_insn_operand_dst
	= tme_recode_regs_dst_specific(ic,
				       insn,
				       0);

      /* if this is an additive operation with the carry: */
      if (opcode_mask
	  & ((1 << TME_RECODE_OPCODE_ADDC)
	     | (1 << TME_RECODE_OPCODE_SUBC))) {

	/* emit the instruction to define CF with the recode carry flag: */
	_tme_recode_x86_conds_testc(ic, 0);
      }

      /* if this is a double-host-size instruction: */
      if (TME_RECODE_SIZE_IS_DOUBLE_HOST(insn->tme_recode_insn_size)) {

	/* emit the low-half host-size instruction: */
	_tme_recode_x86_insn_binop_imm(ic, insn);

	/* advance to the high halves of the immediate and
	   destination: */
	insn->tme_recode_insn_imm_uguest >>= (sizeof(insn->tme_recode_insn_imm_uguest) * 8) / 2;
	insn->tme_recode_insn_operand_dst++;

	/* a high-half additive operation must use the carry: */
	if (opcode_mask
	    & ((1 << TME_RECODE_OPCODE_ADD)
	       | (1 << TME_RECODE_OPCODE_SUB))) {
#if (TME_RECODE_OPCODE_SUBC - TME_RECODE_OPCODE_SUB) != (TME_RECODE_OPCODE_ADDC - TME_RECODE_OPCODE_ADD)
#error "TME_RECODE_OPCODE_ values changed"
#endif
	  insn->tme_recode_insn_opcode += (TME_RECODE_OPCODE_SUBC - TME_RECODE_OPCODE_SUB);
	}
      }	

      /* emit the host-size instruction: */
      _tme_recode_x86_insn_binop_imm(ic, insn);
      return;
    }

    /* if the second source operand is the same as the destination operand,
       and this operation is commutative: */
    operand_src = insn->tme_recode_insn_operand_src[1];
    if (operand_src == insn->tme_recode_insn_operand_dst
	&& (opcode_mask
	    & ((1 << TME_RECODE_OPCODE_AND)
	       | (1 << TME_RECODE_OPCODE_OR)
	       | (1 << TME_RECODE_OPCODE_XOR)
	       | (1 << TME_RECODE_OPCODE_ADD)
	       | (1 << TME_RECODE_OPCODE_ADDC)))) {

      /* switch the first and second source operands: */
      insn->tme_recode_insn_operand_src[1] = insn->tme_recode_insn_operand_src[0];
      insn->tme_recode_insn_operand_src[0] = operand_src;
    }

    /* load the left and right source operands: */
    insn->tme_recode_insn_operand_src[0]
      = tme_recode_regs_src_any(ic,
				insn,
				insn->tme_recode_insn_operand_src[0]);
    insn->tme_recode_insn_operand_src[1]
      = tme_recode_regs_src_any(ic,
				insn,
				insn->tme_recode_insn_operand_src[1]);

    /* if this is a logical noncommutative instruction: */
    if (opcode_mask
	& ((1 << TME_RECODE_OPCODE_ANDN)
	   | (1 << TME_RECODE_OPCODE_ORN)
	   | (1 << TME_RECODE_OPCODE_XORN))) {

      /* copy the right source operand into the destination operand: */
      insn->tme_recode_insn_operand_dst
	= tme_recode_regs_dst_specific(ic,
				       insn,
				       1);

      /* negate the destination operand: */
      _tme_recode_x86_insn_negate(ic, insn);

      /* move the left source operand into the right source operand: */
      insn->tme_recode_insn_operand_src[1] = insn->tme_recode_insn_operand_src[0];

      /* convert the opcode into its commutative version: */
      /* NB: we don't need to change opcode_mask, since the
	 only remaining tests are for additive operations: */
      insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_LOGICALN_TO_PLAIN(insn->tme_recode_insn_opcode);
    }

    /* otherwise, this is not a logical noncommutative instruction: */
    else {

      /* copy the left source operand into the destination operand: */
      insn->tme_recode_insn_operand_dst
	= tme_recode_regs_dst_specific(ic,
				       insn,
				       0);

      /* if this is an additive operation with the carry: */
      if (opcode_mask
	  & ((1 << TME_RECODE_OPCODE_ADDC)
	     | (1 << TME_RECODE_OPCODE_SUBC))) {

	/* emit the instruction to define CF with the recode carry flag: */
	_tme_recode_x86_conds_testc(ic, 0);
      }
    }

    /* if this is a double-host-size instruction: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(insn->tme_recode_insn_size)) {

      /* emit the low-half host-size instruction: */
      _tme_recode_x86_insn_binop(ic, insn);

      /* advance to the high halves of the source and destination: */
      insn->tme_recode_insn_operand_src[1]++;
      insn->tme_recode_insn_operand_dst++;

      /* a high-half additive operation must use the carry: */
      if (opcode_mask
	  & ((1 << TME_RECODE_OPCODE_ADD)
	     | (1 << TME_RECODE_OPCODE_SUB))) {
#if (TME_RECODE_OPCODE_SUBC - TME_RECODE_OPCODE_SUB) != (TME_RECODE_OPCODE_ADDC - TME_RECODE_OPCODE_ADD)
#error "TME_RECODE_OPCODE_ values changed"
#endif
	insn->tme_recode_insn_opcode += (TME_RECODE_OPCODE_SUBC - TME_RECODE_OPCODE_SUB);
      }
    }

    /* emit the host-size instruction: */
    _tme_recode_x86_insn_binop(ic, insn);
    return;
  }

  /* if this is a shift: */
  if (opcode_mask
      & ((1 << TME_RECODE_OPCODE_SHRA)
	 | (1 << TME_RECODE_OPCODE_SHRL)
	 | (1 << TME_RECODE_OPCODE_SHLL))) {

    /* if the shift count is an immediate, and this isn't a
       double-host-size instruction, and the shift count is less than
       the instruction size: */
    if (insn->tme_recode_insn_operand_src[1] == TME_RECODE_OPERAND_IMM
	&& !TME_RECODE_SIZE_IS_DOUBLE_HOST(insn->tme_recode_insn_size)
	&& insn->tme_recode_insn_imm_uguest < TME_BIT(insn->tme_recode_insn_size)) {

      /* load the source operand: */
      tme_recode_regs_src_any(ic,
			      insn,
			      insn->tme_recode_insn_operand_src[0]);

      /* copy the source operand into the destination operand: */
      insn->tme_recode_insn_operand_dst
	= tme_recode_regs_dst_specific(ic,
				       insn,
				       0);

      /* if this is less than a host-size instruction: */
      if (__tme_predict_false(insn->tme_recode_insn_size < TME_RECODE_SIZE_HOST)) {

	/* if this is a right shift: */
	if (opcode_mask != (1 << TME_RECODE_OPCODE_SHLL)) {

	  /* for a right shift, we have to make sure that the correct
	     bits get shifted in on the right, so we have to first
	     extend the value in the destination register to host
	     size.  for an arithmetic shift, we have to do a sign
	     extension, and for a logical shift we have to do a zero
	     extension: */

	  /* map TME_RECODE_OPCODE_SHRL to TME_RECODE_OPCODE_EXTZ, and
	     TME_RECODE_OPCODE_SHRA to TME_RECODE_OPCODE_EXTS: */
#if (TME_RECODE_OPCODE_SHRA - TME_RECODE_OPCODE_SHRL) != (TME_RECODE_OPCODE_EXTS - TME_RECODE_OPCODE_EXTZ)
#error "TME_RECODE_OPCODE_ values changed"
#endif
	  insn->tme_recode_insn_opcode
	    = ((insn->tme_recode_insn_opcode
		- TME_RECODE_OPCODE_SHRL)
	       + TME_RECODE_OPCODE_EXTZ);

	  /* zero- or sign-extend the destination operand: */
	  insn->tme_recode_insn_operand_src[0] = insn->tme_recode_insn_operand_dst;
	  insn->tme_recode_insn_operand_src[1] = insn->tme_recode_insn_size;
	  _tme_recode_x86_insn_ext(ic, insn);

	  /* restore the opcode: */
#if (TME_RECODE_OPCODE_SHRA - TME_RECODE_OPCODE_SHRL) != (TME_RECODE_OPCODE_EXTS - TME_RECODE_OPCODE_EXTZ)
#error "TME_RECODE_OPCODE_ values changed"
#endif
	  insn->tme_recode_insn_opcode
	    = ((insn->tme_recode_insn_opcode
		- TME_RECODE_OPCODE_EXTZ)
	       + TME_RECODE_OPCODE_SHRL);
	}
      }

      /* emit the host-size instruction: */
      _tme_recode_x86_insn_shift_imm(ic, insn);
      return;
    }

    /* load the source operand into the host register for a subs first
       source operand and destination: */
    tme_recode_regs_src_specific(ic,
				 insn,
				 insn->tme_recode_insn_operand_src[0],
				 TME_RECODE_X86_REG_HOST_SUBS_DST);

    /* load the shift count into the host register for a subs second
       source operand: */
    tme_recode_regs_src_specific(ic,
				 insn,
				 insn->tme_recode_insn_operand_src[1],
				 TME_RECODE_X86_REG_HOST_SUBS_SRC1);

    /* emit a call to the subs for this particular shift: */
#if (TME_RECODE_OPCODE_SHLL + 1) != TME_RECODE_OPCODE_SHRL || (TME_RECODE_OPCODE_SHRL + 1) != TME_RECODE_OPCODE_SHRA
#error "TME_RECODE_OPCODE_ values changed"
#endif
    insn->tme_recode_x86_insn_subs_thunk_off
      = (ic->tme_recode_x86_ic_subs_shift
	 [insn->tme_recode_insn_size - TME_RECODE_SIZE_8]
	 [insn->tme_recode_insn_opcode - TME_RECODE_OPCODE_SHLL]);
    _tme_recode_x86_insn_subs(ic, insn);
    return;
  }

  /* if this is a zero- or sign-extension: */
  else if (opcode_mask
	   & ((1 << TME_RECODE_OPCODE_EXTZ)
	      | (1 << TME_RECODE_OPCODE_EXTS))) {

    /* save the instruction size: */
    size_saved = insn->tme_recode_insn_size;

    /* temporarily set the instruction size to the size of the
       source operand: */
    insn->tme_recode_insn_size = insn->tme_recode_insn_operand_src[1];

    /* load the source operand: */
    insn->tme_recode_insn_operand_src[0]
      = tme_recode_regs_src_any(ic,
				insn,
				insn->tme_recode_insn_operand_src[0]);

    /* restore the instruction size: */
    insn->tme_recode_insn_size = size_saved;

    /* load the destination operand: */
    insn->tme_recode_insn_operand_dst
      = tme_recode_regs_dst_any(ic,
				insn);

    /* if this can't be a double-host-size instruction, or if this is
       not an extension from the host size: */
    if (TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_HOST
	|| insn->tme_recode_insn_operand_src[1] < TME_RECODE_SIZE_HOST) {

      /* emit the extension to host size: */
      _tme_recode_x86_insn_ext(ic, insn);
    }

    /* if this is a double-host-size instruction: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(insn->tme_recode_insn_size)) {

      /* start more instructions: */
      tme_recode_x86_insns_start(ic, thunk_bytes);

      /* get the x86 source register: */
      reg_x86_src = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_src[0] + 0];

      /* if this is an extension from host size: */
      if (insn->tme_recode_insn_operand_src[1] == TME_RECODE_SIZE_HOST) {

	/* get the x86 destination register: */
	reg_x86_dst = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst + 0];

	/* unless the host registers are the same, copy the least
	   significant half of the source operand into the least
	   significant half of the destination operand: */
	if (reg_x86_src != reg_x86_dst) {
	  _tme_recode_x86_emit_reg_copy(thunk_bytes, reg_x86_src, reg_x86_dst);
	}
      }	  

      /* get the x86 destination register: */
      reg_x86_dst = tme_recode_x86_reg_from_host[insn->tme_recode_insn_operand_dst + 1];

      /* if this is a sign-extension: */
      if (opcode_mask == (1 << TME_RECODE_OPCODE_EXTS)) {

	/* emit a mov %src, %dst; add %src, %dst: */
	if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
	  rex = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_src)
		 | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86_dst));
	  thunk_bytes[0] = rex;
	  thunk_bytes[3] = rex;
	}
	thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
	  = (TME_RECODE_X86_OPCODE_BINOP_MOV
	     + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
	thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2
		    + (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
	  = (TME_RECODE_X86_OPCODE_BINOP_ADD
	     + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
	mod_reg_rm
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_src),
					TME_RECODE_X86_REG(reg_x86_dst));
	thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1] = mod_reg_rm;
	thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2
		    + (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1] = mod_reg_rm;
	thunk_bytes += ((TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2) * 2;
      }

      /* emit a sbb %dst, %dst if this is a sign-extension,
	 or a sub %dst, %dst if this is a zero-extension: */
      if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
	thunk_bytes[0]
	  = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_dst)
	     | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86_dst));
      }
#if (TME_RECODE_OPCODE_EXTS - TME_RECODE_OPCODE_EXTZ) != 1
#error "TME_RECODE_OPCODE_ values changed"
#endif
      thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
	= (TME_RECODE_X86_OPCODE_BINOP_SUB
	   + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv
	   - ((insn->tme_recode_insn_opcode
	       - TME_RECODE_OPCODE_EXTZ)
	      * (TME_RECODE_X86_OPCODE_BINOP_SUB
		 - TME_RECODE_X86_OPCODE_BINOP_SBB)));
      thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_dst),
				      TME_RECODE_X86_REG(reg_x86_dst));
      thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2;

      /* finish these instructions: */
      tme_recode_x86_insns_finish(ic, thunk_bytes);
    }
    return;
  }

  /* if this is a guest opcode: */
  else if (opcode_mask == (1 << TME_RECODE_OPCODE_GUEST)) {

    /* emit the call to the guest function: */
    _tme_recode_x86_insn_guest(ic, insn);
    return;
  }

  /* if this is a define-recode-carry opcode: */
  else if (opcode_mask == (1 << TME_RECODE_OPCODE_DEFC)) {

    /* emit the carry define: */
    _tme_recode_x86_conds_defc(ic, insn);
    return;
  }

  /* if this is an if opcode: */
  else if (opcode_mask == (1 << TME_RECODE_OPCODE_IF)) {

    /* clean all dirty host registers before we begin the if: */
    tme_recode_regs_host_clean_all(ic);

    /* emit the instruction to define CF with the desired recode flag: */
    _tme_recode_x86_conds_testc(ic, insn->tme_recode_insn_operand_src[0]);

    /* start more instructions: */
    tme_recode_x86_insns_start(ic, thunk_bytes);

    /* ifs can't be nested: */
    assert (ic->tme_recode_x86_ic_thunks_build_if == NULL
	    && ic->tme_recode_x86_ic_thunks_build_else == NULL);

    /* save the address of this if: */
    ic->tme_recode_x86_ic_thunks_build_if = thunk_bytes;
    
    /* a 32-bit displacement jnc instruction requires six bytes: */
    thunk_bytes += 2 + sizeof(tme_uint32_t);

    /* save any c register window index at the if jnc: */
    ic->tme_recode_x86_ic_thunks_reg_guest_window_c_if_jmp
      = ic->tme_recode_x86_ic_thunks_reg_guest_window_c;

    /* finish these instructions: */
    tme_recode_x86_insns_finish(ic, thunk_bytes);
  }

  /* if this is an else or an endif opcode: */
  else if (opcode_mask
	   & ((1 << TME_RECODE_OPCODE_ELSE)
	      | (1 << TME_RECODE_OPCODE_ENDIF))) {

    /* free all host registers: */
    tme_recode_regs_host_free_many(ic, TME_RECODE_REG_HOST(0));

    /* start more instructions: */
    tme_recode_x86_insns_start(ic, thunk_bytes);

    /* if we are in an if: */
    thunk_bytes_other = ic->tme_recode_x86_ic_thunks_build_if;
    if (thunk_bytes_other != NULL) {

      /* if this is an else: */
      if (opcode_mask == (1 << TME_RECODE_OPCODE_ELSE)) {

	/* save the address of this else: */
	ic->tme_recode_x86_ic_thunks_build_else = thunk_bytes;
    
	/* a 32-bit displacement jmp instruction requires five bytes: */
	thunk_bytes += 1 + sizeof(tme_uint32_t);

	/* switch any c register window index at the if jnc with any
	   window index at the end of the if body: */
	window_i = ic->tme_recode_x86_ic_thunks_reg_guest_window_c;
	ic->tme_recode_x86_ic_thunks_reg_guest_window_c
	  = ic->tme_recode_x86_ic_thunks_reg_guest_window_c_if_jmp;
	ic->tme_recode_x86_ic_thunks_reg_guest_window_c_if_jmp = window_i;

	/* finish these instructions: */
	tme_recode_x86_insns_finish(ic, thunk_bytes);
      }

      /* emit the jnc instruction at the if: */
      relv = thunk_bytes - (thunk_bytes_other + 1 + sizeof(tme_int8_t));
      if (relv < 0x7f) {
	thunk_bytes_other[0] = TME_RECODE_X86_OPCODE_JCC(TME_RECODE_X86_COND_NOT | TME_RECODE_X86_COND_C);
	thunk_bytes_other[1] = relv;
	*((tme_uint32_t *) &thunk_bytes_other[2]) = TME_RECODE_X86_NOP4;
      }
      else {
	relv = thunk_bytes - (thunk_bytes_other + 2 + sizeof(tme_int32_t));
	*((tme_uint16_t *) &thunk_bytes_other[0])
	  = (TME_RECODE_X86_OPCODE_ESC_0F
	     + (TME_RECODE_X86_OPCODE0F_JCC(TME_RECODE_X86_COND_NOT | TME_RECODE_X86_COND_C)
		<< 8));
	*((tme_uint32_t *) &thunk_bytes_other[2]) = relv;
      }

      /* clear the if: */
      ic->tme_recode_x86_ic_thunks_build_if = NULL;
    }

    /* otherwise, we must be in an else and this must be an endif: */
    else {
      thunk_bytes_other = ic->tme_recode_x86_ic_thunks_build_else;
      assert (thunk_bytes_other != NULL
	      && opcode_mask == (1 << TME_RECODE_OPCODE_ENDIF));

      /* emit the jmp instruction at the else: */
      relv = thunk_bytes - (thunk_bytes_other + 1 + sizeof(tme_int8_t));
      if (relv < 0x7f) {
	thunk_bytes_other[0] = TME_RECODE_X86_OPCODE_JMP_RELb;
	relv += (TME_RECODE_X86_NOP3 << 8);
      }
      else {
	relv = thunk_bytes - (thunk_bytes_other + 1 + sizeof(tme_int32_t));
	thunk_bytes_other[0] = TME_RECODE_X86_OPCODE_JMP_RELz;
      }
      *((tme_uint32_t *) &thunk_bytes_other[1]) = relv;
    }

    /* clear the else: */
    ic->tme_recode_x86_ic_thunks_build_else = NULL;

    /* if this is an endif: */
    if (opcode_mask == (1 << TME_RECODE_OPCODE_ENDIF)) {
 
      /* if there is no else body, and any c register window index at
	 the if jnc disagrees with the window index at the end of the
	 if body, or if there is an else body, and any c register
	 window index at the end of the if body disagrees with any c
	 register window index at the end of the else body: */
      if (ic->tme_recode_x86_ic_thunks_reg_guest_window_c_if_jmp
	  != ic->tme_recode_x86_ic_thunks_reg_guest_window_c) {

	/* at this point, there isn't a known window index in the c register: */
	ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;
      }
    }
  }

  /* if this is a read or a write instruction: */
  else if (opcode_mask == (1 << TME_RECODE_OPCODE_RW)) {

    /* emit the read or write instruction: */
    _tme_recode_x86_insn_rw(ic, insn);
  }

  /* otherwise, this is some unknown opcode: */
  else {
    assert(FALSE);
  }
}

/* this invalidates an instructions thunk: */
void
tme_recode_insns_thunk_invalidate(struct tme_recode_ic *ic,
				  tme_recode_thunk_off_t insns_thunk)
{
  tme_uint8_t opcode_jmp_relz;
  tme_int32_t disp32;

  /* overwrite the first instruction of the thunk with a jmp to the
     chain epilogue: */
  opcode_jmp_relz = TME_RECODE_X86_OPCODE_JMP_RELz;
  disp32
    = (ic->tme_recode_x86_ic_chain_epilogue
       - (insns_thunk
	  + sizeof(opcode_jmp_relz)
	  + sizeof(disp32)));
  tme_recode_thunk_off_write(ic,
			     insns_thunk,
			     tme_uint8_t,
			     opcode_jmp_relz);
  tme_recode_thunk_off_write(ic,
			     (insns_thunk
			      + sizeof(opcode_jmp_relz)),
			     tme_int32_t,
			     disp32);
}

/* this checks the value of TME_RECODE_HOST_INSN_SIZE_MAX: */
static void
_tme_recode_x86_insn_size_max_check(void)
{
  tme_uint32_t insn_size_window_offset;
  tme_uint32_t insn_size_host_store;
  tme_uint32_t insn_size_host_copy;
  tme_uint32_t insn_size_host_load;
  tme_uint32_t insn_size;

  /* the largest insn recode for any guest should be that guest's
     worst-case TME_RECODE_OPCODE_GUEST insn, but exactly what that
     means depends on a lot of variables that we don't want to
     consider here.

     instead, this calculates an upper-bound for that worst-case
     TME_RECODE_OPCODE_GUEST insn, by including all possible x86
     instructions and x86 instruction features in a
     TME_RECODE_OPCODE_GUEST insn recode, even though it's impossible
     for all instructions and all features to be present at the same
     time: */

  /* size the x86 instruction to load one window offset: */
  insn_size_window_offset
    = ((TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) /* TME_RECODE_X86_REX */
       + 1		/* TME_RECODE_X86_OPCODE_MOVS_El_Gv or TME_RECODE_X86_OPCODE_BINOP_MOV */
       + 1		/* TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32) */
       + sizeof(tme_int32_t));

  /* size the x86 instruction(s) to store one host register at the
     worst size to a guest register window.  on an x86-64 host, the
     worst size is the 16-bit size, and we simply assume that all host
     registers need a REX prefix.  on an ia32, the worst size is the
     8-bit size, and we simply assume that all host registers don't
     have an 8-bit encoding: */
  insn_size_host_store
    = ((TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32)
       ? (1		/* TME_RECODE_X86_PREFIX_OPSIZ */
	  + 1		/* TME_RECODE_X86_REX */
	  + 1 		/* TME_RECODE_X86_OPCODE_BINOP_MOV */
	  + 1 		/* TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32) */
	  + 1 		/* TME_RECODE_X86_SIB */
	  + sizeof(tme_uint32_t))
       : (1		/* TME_RECODE_X86_OPCODE_BINOP_XCHG */
	  + 1		/* TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG) */
	  + 1 		/* TME_RECODE_X86_OPCODE_BINOP_MOV */
	  + 1 		/* TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32) */
	  + 1 		/* TME_RECODE_X86_SIB */
	  + sizeof(tme_uint32_t)
	  + 1		/* TME_RECODE_X86_OPCODE_BINOP_XCHG */
	  + 1));	/* TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG) */

  /* size the x86 instruction to copy one host register to another: */
  insn_size_host_copy
    = ((TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32)	/* TME_RECODE_X86_REX */
       + 1		/* TME_RECODE_X86_OPCODE_BINOP_MOV */
       + 1);		/* TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG) */

  /* size the x86 instruction to load one host register from a guest
     register window: */
  insn_size_host_load
    = ((TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32)	/* TME_RECODE_X86_REX */
       + 1 		/* TME_RECODE_X86_OPCODE_BINOP_MOV */
       + 1 		/* TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32) */
       + 1 		/* TME_RECODE_X86_SIB */
       + sizeof(tme_uint32_t));

  /* start with no instructions: */
  insn_size = 0;

  /* add in cleaning all TME_RECODE_REG_HOST_UNDEF host registers at
     the worst size, storing them to alternating guest register
     windows (requiring the c register to be loaded with a window
     offset before each store): */
  insn_size
    += ((insn_size_window_offset
	 + insn_size_host_store)
	* TME_RECODE_REG_HOST_UNDEF);

  /* add in the x86 instructions to load two double-host-size guest
     windowed registers for the two source operands, moving older
     guest register values to other host registers, and loading the
     source operands from alternating guest register windows: */
  insn_size
    += ((insn_size_host_copy
	 + insn_size_window_offset
	 + insn_size_host_load)
	* (2		/* double-host-size */
	   * 2));	/* two source operands */

  /* add in the x86 instructions to move an older guest register
     to other host registers, for the destination operand: */
  insn_size += (insn_size_host_copy * 2);	/* double-host-size */

  /* if this is an ia32 host: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

    /* add in the ia32 instructions for pushing the arguments for the
       guest function: */
    insn_size
      += (3		/* sub $imm8, %esp (double-host-size) */
	  + 1		/* TME_RECODE_X86_OPCODE_PUSH_Gv */
	  + 1		/* TME_RECODE_X86_OPCODE_PUSH_Gv (double-host-size) */
	  + 1		/* TME_RECODE_X86_OPCODE_PUSH_Gv */
	  + 1);		/* TME_RECODE_X86_OPCODE_PUSH_Gv (struct tme_ic *) */

    /* add in the ia32 instructions for calling the guest function: */
    insn_size
      += (1		/* TME_RECODE_X86_OPCODE_CALL_RELz */
	  + sizeof(tme_int32_t));

    /* add in the ia32 instructions for removing the guest function
       arguments from the stack: */
    insn_size += 3;	/* sub $imm8, %esp (double-host-size) */
  }

  /* otherwise, this is an x86-64 host: */
  else {

    /* add in the x86-64 instructions for moving host registers into
       arguments for the guest function: */
    insn_size
      += (insn_size_host_copy
	  * ((2		/* double-host-size */
	      * 2)	/* two source operands */
	     + 1));	/* struct tme_ic */

    /* add in the x86-64 instructions for calling the guest function: */
    insn_size
      += (1		/* TME_RECODE_X86_REX */
	  + 1		/* TME_RECODE_X86_OPCODE_MOV_Iv_Gv */
	  + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8)
	  + 1		/* TME_RECODE_X86_OPCODE_GRP5 */
	  + 1);		/* TME_RECODE_X86_MOD_OPREG_RM */	  
  }

  /* add in the x86 instructions to move a double-host-size guest
     function return value from d:a into ax:bp: */
  insn_size += (insn_size_host_copy * 2); /* double-host-size */

  /* since the worst-case TME_RECODE_OPCODE_GUEST insn is also the
     last insn in a thunk, add in the x86 instructions for cleaning
     the host register(s) holding the destination operand to a guest
     register window, and for jumping to the instructions thunk
     epilogue: */
  insn_size
    += (insn_size_window_offset
	+ (insn_size_host_store
	   * 2)			/* double-host-size */
	+ 1			/* TME_RECODE_X86_OPCODE_JMP_RELz */
	+ sizeof(tme_int32_t));

  /* check the value of TME_RECODE_HOST_INSN_SIZE_MAX: */
  if (insn_size > TME_RECODE_HOST_INSN_SIZE_MAX) {
    abort();
  }
}

/* this host function returns the thunk offset for a new instructions
   thunk.  it returns less than zero when thunks memory is exhausted
   and all instructions thunks are flushed: */
tme_recode_thunk_off_t
tme_recode_host_insns_thunk_new(struct tme_recode_ic *ic,
				const struct tme_recode_insns_group *insns_group)
{
  tme_uint8_t *thunk_bytes;
  tme_recode_thunk_off_t thunk_off;
  struct tme_recode_insn *insn;
  struct tme_recode_insn *insns_end;

  /* if we don't have enough room to make a maximum-sized thunk: */
  if (!tme_recode_host_thunk_start(ic)) {

    /* invalidate all instructions thunks: */
    tme_recode_insns_thunk_invalidate_all(ic);
    return (-1);
  }

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);
  thunk_off = tme_recode_build_to_thunk_off(ic, thunk_bytes);

  /* make the chain in: */
  _tme_recode_x86_chain_in(ic, insns_group);

  /* the c register doesn't hold the base offset of any guest register
     window: */
  ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

  /* make sure there isn't any leftover if/else insn state: */
  assert (ic->tme_recode_x86_ic_thunks_build_if == NULL
	  && ic->tme_recode_x86_ic_thunks_build_else == NULL);

  /* emit all of the instructions: */
  insn = insns_group->tme_recode_insns_group_insns;
  insns_end = insns_group->tme_recode_insns_group_insns_end;
  do {
    _tme_recode_x86_insn_emit(ic, insn);
  } while (++insn < insns_end);

  /* make sure there isn't any leftover if/else insn state: */
  assert (ic->tme_recode_x86_ic_thunks_build_if == NULL
	  && ic->tme_recode_x86_ic_thunks_build_else == NULL);

  /* free all host registers: */
  tme_recode_regs_host_free_many(ic, TME_RECODE_REG_HOST(0));

  /* make the chain out: */
  _tme_recode_x86_chain_out(ic, insns_group);

  /* finish and return this thunk: */
  tme_recode_host_thunk_finish(ic);
  return (thunk_off);
}
