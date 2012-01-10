/* $Id: rc-x86-tlb.c,v 1.1 2010/01/15 03:05:59 fredette Exp $ */

/* libtme/host/x86/rc-x86-tlb.c - x86 host recode TLB support: */

/*
 * Copyright (c) 2008, 2009 Matt Fredette
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

_TME_RCSID("$Id: rc-x86-tlb.c,v 1.1 2010/01/15 03:05:59 fredette Exp $");

/* macros: */

/* the x86 register for the TLB entry pointer for a TLB busy or unbusy: */
#define TME_RECODE_X86_REG_TLB			(TME_RECODE_X86_REG_D)

/* the x86 register for scratch values during a TLB busy: */
#define TME_RECODE_X86_REG_TLB_SCRATCH		(TME_RECODE_X86_REG_C)

/* types: */

/* a recode x86 TLB type: */
struct tme_recode_x86_tlb_type {

  /* the generic TLB type: */
  struct tme_recode_tlb_type tme_recode_tlb_type;

  /* the pointer in the thunk build memory of any assist jump for
     the most-significant half of a double-host-size guest address.
     at this assist jump, the guest address is unmodified: */
  tme_uint8_t *tme_recode_x86_tlb_type_assist_jmp_address_ok;

  /* the pointer in the thunk build memory of the main assist jump.
     at this assist jump, the (least-significant half of the) guest
     address has been exclusive-ORed with the (least-significant half
     of the) TLB page: */
  tme_uint8_t *tme_recode_x86_tlb_type_assist_jmp;
};

/* this returns the host register number for the address (on entry,
   the guest address, on exit, the host address) for a TLB busy: */
/* NB: for a double-host-size guest, the guest address is in the a:bp
   register pair, otherwise it's in the a register.  NB that we
   primarily deal with only a host-sized part: */
static inline unsigned long
_tme_recode_x86_tlb_reg_host_address(const struct tme_recode_ic *ic)
{
  assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_FREE_CALL]
	  == TME_RECODE_X86_REG_A);
  assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_FREE_CALL - 1]
	  == TME_RECODE_X86_REG_BP);
  return (TME_RECODE_X86_REG_HOST_FREE_CALL
	  - (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size) != 0));
}

/* this emits an instruction that references a TLB entry: */
static tme_uint8_t *
_tme_recode_x86_tlb_ref(tme_uint8_t *thunk_bytes,
			unsigned int size,
			unsigned int opcode,
			unsigned int reg_x86_tlb,
			unsigned int tlb_offset,
			tme_uint8_t opreg)
{
  unsigned int rex;

  /* emit any rex prefix: */
  rex = TME_RECODE_X86_REX_B(0, reg_x86_tlb);
  if (opcode != TME_RECODE_X86_OPCODE_GRP1_Ib_Eb
      && opcode != TME_RECODE_X86_OPCODE_MOV_Ib_Eb) {
    assert (size <= TME_RECODE_SIZE_HOST);
    rex |= TME_RECODE_X86_REX_R(size, opreg);
  }
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }

  /* emit the opcode: */
  thunk_bytes[0] = opcode;

  /* emit the ModR/M byte and an optional 8-bit displacement: */
  assert (tlb_offset < 0x80);
  thunk_bytes[1]
    = TME_RECODE_X86_MOD_OPREG_RM((tlb_offset
				   ? TME_RECODE_X86_MOD_RM_EA_DISP8(reg_x86_tlb)
				   : TME_RECODE_X86_MOD_RM_EA(reg_x86_tlb)),
				  TME_RECODE_X86_REG(opreg));
  thunk_bytes += 2;
  if (tlb_offset) {
    *(thunk_bytes++) = tlb_offset;
  }

  return (thunk_bytes);
}

/* this emits instructions that unbusy a TLB entry: */
static void
_tme_recode_x86_tlb_unbusy(struct tme_recode_ic *ic,
			   unsigned long tlb_offset_token)
{
  unsigned int reg_x86_tlb;
  tme_uint8_t *thunk_bytes;

  /* get the x86 register with the TLB entry pointer or token
     pointer: */
  reg_x86_tlb = TME_RECODE_X86_REG_TLB;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* unbusy the TLB entry: */
  thunk_bytes
    = _tme_recode_x86_tlb_ref(thunk_bytes,
			      TME_RECODE_SIZE_8,
			      TME_RECODE_X86_OPCODE_MOV_Ib_Eb,
			      reg_x86_tlb,
			      (tlb_offset_token
			       + ((unsigned long)
				  &((struct tme_token *) 0)->tme_token_busy)),
			      0 /* undefined */);
  *(thunk_bytes++) = 0;

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits instructions that find, busy, and check a TLB entry: */
static void
_tme_recode_x86_tlb_busy(struct tme_recode_ic *ic,
			 const struct tme_recode_address_type *address_type,
			 struct tme_recode_x86_tlb_type *x86_tlb_type)
{
  unsigned int reg_x86_address;
  struct tme_recode_insn insn_buffer;
  tme_uint8_t *thunk_bytes;
  unsigned int reg_x86_tlb;
  unsigned int reg_x86_scratch;
  unsigned int reg_x86_tlb_flags;
  unsigned int rex;
  tme_uint32_t address_mask_tlb_index_one;
  unsigned long tlb_factor;
  int shift_count;
  unsigned int opcode;
  unsigned int opreg;
  tme_uint32_t tlb_flags;

  /* assume that this is not a double-host-size guest: */
  x86_tlb_type->tme_recode_x86_tlb_type_assist_jmp_address_ok = (tme_uint8_t *) NULL;

  /* for a double-host-size guest, the guest address is in the a:bp
     register pair, otherwise it's in the a register.  NB that we
     primarily deal with only a host-sized part: */
  reg_x86_address
    = tme_recode_x86_reg_from_host[_tme_recode_x86_tlb_reg_host_address(ic)];

  /* if the guest address size is less than the host size: */
  if (address_type->tme_recode_address_type_size < TME_RECODE_SIZE_HOST) {

    /* zero- or sign-extend the guest address register to the host
       size: */
    insn_buffer.tme_recode_insn_opcode
      = (address_type->tme_recode_address_type_signed
	 ? TME_RECODE_OPCODE_EXTS
	 : TME_RECODE_OPCODE_EXTZ);
    insn_buffer.tme_recode_insn_operand_src[0] = _tme_recode_x86_tlb_reg_host_address(ic);
    insn_buffer.tme_recode_insn_operand_src[1] = address_type->tme_recode_address_type_size;
    insn_buffer.tme_recode_insn_operand_dst = insn_buffer.tme_recode_insn_operand_src[0];
    _tme_recode_x86_insn_ext(ic, &insn_buffer);
  }

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* we will hash the guest address into a TLB entry pointer in the
     d register: */
  reg_x86_tlb = TME_RECODE_X86_REG_TLB;

  /* we will use the c register for a multiply scratch register and
     the TLB flags: */
  reg_x86_scratch = TME_RECODE_X86_REG_TLB_SCRATCH;
  reg_x86_tlb_flags = reg_x86_scratch;

  /* copy the least significant 32 bits of the guest address into the
     TLB entry pointer register: */
  thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
  thunk_bytes[1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_address),
				  TME_RECODE_X86_REG(reg_x86_tlb));
  thunk_bytes += 2;

  /* mask the TLB entry pointer register with the TLB index address
     mask: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Iz_Ev;
  thunk_bytes[1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_tlb),
				  TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_AND));
  *((tme_uint32_t *) &thunk_bytes[2]) = address_type->tme_recode_address_type_mask_tlb_index;
  thunk_bytes += 2 + sizeof(tme_uint32_t);

  /* shift the TLB index in the TLB entry pointer register all the way
     down to the right, except for any factors of two in the size of a
     TLB entry: */
  address_mask_tlb_index_one
    = (address_type->tme_recode_address_type_mask_tlb_index
       & (0 - address_type->tme_recode_address_type_mask_tlb_index));
  tlb_factor = x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_sizeof;
  for (shift_count = _tme_recode_x86_ffs(address_mask_tlb_index_one);
       shift_count > 0 && (tlb_factor % 2) == 0;
       shift_count--) {
    tlb_factor /= 2;
  }
  if (shift_count > 0) {
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_tlb),
				    TME_RECODE_X86_OPCODE_GRP2_SHR);
    thunk_bytes[2] = shift_count;
    thunk_bytes += 3;
  }

  /* multiply the TLB index in the TLB entry pointer register by the
     remaining factors in the size of a TLB entry: */
  thunk_bytes
    = _tme_recode_x86_emit_mul_constant(thunk_bytes,
					TME_RECODE_SIZE_32,
					reg_x86_tlb,
					tlb_factor,
					reg_x86_scratch);

  /* add in the address of tlb zero to finish the TLB entry pointer
     register: */
  rex
    = (TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC)
       | TME_RECODE_X86_REX_X(reg_x86_tlb)
       | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86_tlb));
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_LEA;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_EA_BASE_SIB),
					       TME_RECODE_X86_REG(reg_x86_tlb));
  thunk_bytes[2] = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_IC, reg_x86_tlb, 1);
  *((tme_int32_t *) &thunk_bytes[3]) = address_type->tme_recode_address_type_tlb0_ic_offset;
  thunk_bytes += 3 + sizeof(tme_int32_t);

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* busy the TLB entry: */
  thunk_bytes
    = _tme_recode_x86_tlb_ref(thunk_bytes,
			      TME_RECODE_SIZE_8,
			      TME_RECODE_X86_OPCODE_MOV_Ib_Eb,
			      reg_x86_tlb,
			      (x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_token
			       + ((unsigned long)
				  &((struct tme_token *) 0)->tme_token_busy)),
			      0 /* undefined */);
  *(thunk_bytes++) = 1;

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */

  /* if this is a double-host-size guest: */
  if (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {

    /* compare the most-significant half of the guest address to the
       most-significant half of the TLB entry page: */

    /* assume that either the guest address size is the guest
       register size, or the guest address is sign-extended: */
    opcode
      = (TME_RECODE_X86_OPCODE_BINOP_CMP
	 + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    opreg
      = TME_RECODE_X86_REG(tme_recode_x86_reg_from_host
			   [_tme_recode_x86_tlb_reg_host_address(ic) + 1]);

    /* if the guest address size is less than the guest register size: */
    if (address_type->tme_recode_address_type_size < ic->tme_recode_ic_reg_size) {

      /* if the guest address is sign-extended: */
      if (address_type->tme_recode_address_type_signed) {

	/* sign-extend the guest address to double host size: */
	/* NB: the guest address has already been sign-extended to
	   host size by the _tme_recode_x86_insn_ext() call above: */
	_tme_recode_x86_emit_reg_binop(thunk_bytes,
				       TME_RECODE_X86_OPCODE_BINOP_MOV,
				       reg_x86_address,
				       opreg);
	_tme_recode_x86_emit_reg_binop(thunk_bytes,
				       TME_RECODE_X86_OPCODE_BINOP_ADD,
				       reg_x86_address,
				       opreg);
	_tme_recode_x86_emit_reg_binop(thunk_bytes,
				       TME_RECODE_X86_OPCODE_BINOP_SBB,
				       opreg,
				       opreg);
      }

      /* otherwise, the guest address is unsigned: */
      else {

	/* we can simply compare the most-significant half of the
	   TLB entry page to zero: */
	opcode = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
	opreg = TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_CMP);
      }
    }

    /* do the comparison: */
    thunk_bytes
      = _tme_recode_x86_tlb_ref(thunk_bytes,
				TME_RECODE_SIZE_HOST,
				opcode,
				reg_x86_tlb,
				(x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_page
				 + TME_BIT(TME_RECODE_SIZE_HOST
					   - TME_RECODE_SIZE_8)),
				opreg);
    if (opcode == TME_RECODE_X86_OPCODE_GRP1_Ib_Ev) {
      *(thunk_bytes++) = 0;
    }

    /* if the comparison fails, jump to the assist code: */
    x86_tlb_type->tme_recode_x86_tlb_type_assist_jmp_address_ok = thunk_bytes;
    thunk_bytes
      = _tme_recode_x86_emit_jmp(thunk_bytes,
				 TME_RECODE_X86_OPCODE_JCC(TME_RECODE_X86_COND_NOT
							   | TME_RECODE_X86_COND_Z),
				 (tme_uint8_t *) NULL);
  }

  /* get the guest fixed TLB flags for this operation: */
  tlb_flags = address_type->tme_recode_address_type_tlb_flags;

  /* in the TLB flags, set TME_RECODE_X86_TLB_FLAG_INVALID(ic), and
     also set all of the bits below it, down to bit zero.

     eventually, we will set the carry flag when a recode TLB entry is
     found valid, and use an add-with-carry instruction to clear all
     of these bits so they won't cause an assist (although this will
     carry out into the next TLB flag bit, which is unused and must
     always be clear in the TLB flags in TLB entries, otherwise it
     will cause an assist): */
  assert ((tlb_flags
	   & ((TME_RECODE_X86_TLB_FLAG_INVALID(ic) * 2)
	      + TME_RECODE_X86_TLB_FLAG_INVALID(ic)
	      + (TME_RECODE_X86_TLB_FLAG_INVALID(ic) - 1))) == 0);
  tlb_flags
    += (TME_RECODE_X86_TLB_FLAG_INVALID(ic)
	+ (TME_RECODE_X86_TLB_FLAG_INVALID(ic) - 1));

  /* if this address type includes a context: */
  if (address_type->tme_recode_address_type_context_ic_offset >= 0) {

    /* guest contexts can't be bigger than a host register: */
    assert (address_type->tme_recode_address_type_context_size <= TME_RECODE_SIZE_HOST);

    /* emit one of:
       movzb context(%tlb), %reg
       movzw context(%tlb), %reg
       movl context(%tlb), %reg
       movq context(%tlb), %reg

       where %reg is the TLB flags register:
    */
    rex = (TME_RECODE_X86_REX_B(0, reg_x86_tlb)
	   | TME_RECODE_X86_REX_R(TME_MAX(TME_RECODE_SIZE_32,
					  address_type->tme_recode_address_type_context_size),
				  reg_x86_tlb_flags));
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    if (address_type->tme_recode_address_type_context_size < TME_RECODE_SIZE_32) {
      *(thunk_bytes++) = TME_RECODE_X86_OPCODE_ESC_0F;
      thunk_bytes[0]
	= (address_type->tme_recode_address_type_context_size == TME_RECODE_SIZE_8
	   ? TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv
	   : TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv);
    }
    else {
      thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    }
    assert (x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_context < 0x80);
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(reg_x86_tlb),
				    TME_RECODE_X86_REG(reg_x86_tlb_flags));
    thunk_bytes[2] = x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_context;
    thunk_bytes += 3;

    /* the guest context register is a tme_bus_context_t, which must
       be at least a tme_uint32_t, because we read that much in the
       next instruction: */
    assert (sizeof(tme_bus_context_t) >= sizeof(tme_uint32_t));

    /* exclusive-or the guest context register into the TLB context in
       the TLB flags register: */
    rex = (TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC)
	   | TME_RECODE_X86_REX_R(TME_MAX(TME_RECODE_SIZE_32,
					  address_type->tme_recode_address_type_context_size),
				  reg_x86_tlb_flags));
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_XOR + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    thunk_bytes
      = _tme_recode_x86_emit_ic_modrm(&thunk_bytes[1],
				      address_type->tme_recode_address_type_context_ic_offset,
				      reg_x86_tlb_flags);

    /* set the carry flag if the context register doesn't match the
       TLB context, by negating the TLB flags register: */
    rex = TME_RECODE_X86_REX_B(TME_MAX(TME_RECODE_SIZE_32,
				       address_type->tme_recode_address_type_context_size),
			       reg_x86_tlb_flags);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP3_Ev;
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_tlb_flags),
						 TME_RECODE_X86_OPCODE_GRP3_NEG);
    thunk_bytes += 2;

    /* initialize the TLB flags register with the read/write flags,
       shifted left by one. we'll eventually rotate the register to
       the right by one: */
    rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, reg_x86_tlb_flags);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(reg_x86_tlb_flags);
    *((tme_uint32_t *) &thunk_bytes[1]) = (tlb_flags << 1);
    thunk_bytes += 1 + sizeof(tme_uint32_t);

    /* this add with carry will set bit zero of the read/write flags
       register if the context register doesn't match the TLB
       context: */
    rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, reg_x86_tlb_flags);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_tlb_flags),
				    TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADC));
    thunk_bytes[2] = 0;
    thunk_bytes += 3;

    /* rotate the read/write flags register to the right by one.  this
       will rotate bit zero around and up to
       TME_RECODE_RW_FLAG_CONTEXT_MISMATCH(ic), and put all of the
       other read/write flags in their correct positions: */
    rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, reg_x86_tlb_flags);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_1_Ev;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_tlb_flags),
				    TME_RECODE_X86_OPCODE_GRP2_ROR);
    thunk_bytes += 2;
  }

  /* make sure the token busy write completes before the token invalid
     read: */
  if (!TME_THREADS_COOPERATIVE) {
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;
    thunk_bytes[1] = TME_RECODE_X86_OPCODE0F_GRP15;
    thunk_bytes[2] = TME_RECODE_X86_OPCODE0F_GRP15_MFENCE;
    thunk_bytes += 3;
  }

  /* set the carry flag if this TLB entry is still valid: */
  thunk_bytes
    = _tme_recode_x86_tlb_ref(thunk_bytes,
			      TME_RECODE_SIZE_8,
			      TME_RECODE_X86_OPCODE_GRP1_Ib_Eb,
			      reg_x86_tlb,
			      (x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_token
			       + ((unsigned long)
				  &((struct tme_token *) 0)->tme_token_invalid)),
			      TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_CMP));
  *(thunk_bytes++) = 1;

  /* if this address type includes a context: */
  if (address_type->tme_recode_address_type_context_ic_offset < 0) {

    /* initialize the TLB flags register with the TLB flags: */
    rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, reg_x86_tlb_flags);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(reg_x86_tlb_flags);
    *((tme_uint32_t *) &thunk_bytes[1]) = tlb_flags;
    thunk_bytes += 1 + sizeof(tme_uint32_t);
  }

  /* add one to the read/write flags register if the TLB entry is
     still valid.  adding one will clear all of the
     (TME_RECODE_X86_TLB_FLAG_INVALID(ic) - 1) bits and
     TME_RECODE_X86_TLB_FLAG_INVALID(ic), so they won't cause an
     assist (but this will set the next bit, which is unused and must
     always be clear in the read/write flags in TLB entries so it
     doesn't cause an assist).

     not adding one will leave the
     (TME_RECODE_X86_TLB_FLAG_INVALID(ic) - 1) bits set, but this
     doesn't matter, because TME_RECODE_X86_TLB_FLAG_INVALID(ic) will
     still be set and will definitely cause an assist: */
  rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, reg_x86_tlb_flags);
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
  thunk_bytes[1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_tlb_flags),
				  TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_ADC));
  thunk_bytes[2] = 0;
  thunk_bytes += 3;

  /* exclusive-or the (least-significant half of the) guest address
     with the (least-significant half of the) TLB entry page, to
     convert the guest address into the TLB entry page offset: */
  thunk_bytes
    = _tme_recode_x86_tlb_ref(thunk_bytes,
			      TME_MIN(TME_RECODE_SIZE_HOST,
				      address_type->tme_recode_address_type_size),
			      (TME_RECODE_X86_OPCODE_BINOP_XOR
			       + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv),
			      reg_x86_tlb,
			      x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_page,
			      TME_RECODE_X86_REG(reg_x86_address));

  /* if this address type has TLB flags in the guest IC to and with
     the fixed TLB flags from the address type: */
  if (address_type->tme_recode_address_type_tlb_flags_ic_offset >= 0) {

    /* and in the read/write flags from the guest IC: */
    rex
      = (TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC)
	 | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, reg_x86_tlb_flags));
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_AND + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    thunk_bytes
      = _tme_recode_x86_emit_ic_modrm(&thunk_bytes[1],
				      address_type->tme_recode_address_type_tlb_flags_ic_offset,
				      reg_x86_tlb_flags);
  }

  /* and in the TLB flags from the TLB entry: */
  thunk_bytes
    = _tme_recode_x86_tlb_ref(thunk_bytes,
			      TME_RECODE_SIZE_32,
			      (TME_RECODE_X86_OPCODE_BINOP_AND
			       + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv),
			      reg_x86_tlb,
			      x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_flags,
			      TME_RECODE_X86_REG(reg_x86_tlb_flags));

  /* or the TLB entry page offset into the TLB flags register: */
  _tme_recode_x86_emit_reg_binop(thunk_bytes,
				 TME_RECODE_X86_OPCODE_BINOP_OR,
				 reg_x86_address,
				 reg_x86_tlb_flags);

  /* test if any TLB flags above the TLB page size are set, or if any
     TLB page offset bits are set that don't meet the access' minimum
     alignment: */
  /* NB that this will catch TLB flags that survived all of
     the mask ands, and also a TLB page mismatch, since the mismatch
     bits will be above the TLB page size: */
  rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_tlb_flags);
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP3_Ev;
  thunk_bytes[1]
    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_tlb_flags),
				  TME_RECODE_X86_OPCODE_GRP3_TEST);
  *((tme_int32_t *) &thunk_bytes[2])
    = ((0 - ic->tme_recode_ic_tlb_page_size)
       | (address_type->tme_recode_address_type_align_min - 1));
  thunk_bytes += 2 + sizeof(tme_int32_t);

  /* if the test fails, jump to the assist code: */
  x86_tlb_type->tme_recode_x86_tlb_type_assist_jmp = thunk_bytes;
  thunk_bytes
    = _tme_recode_x86_emit_jmp(thunk_bytes,
			       TME_RECODE_X86_OPCODE_JCC(TME_RECODE_X86_COND_NOT
							 | TME_RECODE_X86_COND_Z),
			       (tme_uint8_t *) NULL);

  /* add the TLB entry memory base to the TLB entry page offset, to
     make the host address: */
  thunk_bytes
    = _tme_recode_x86_tlb_ref(thunk_bytes,
			      TME_RECODE_SIZE_HOST,
			      (TME_RECODE_X86_OPCODE_BINOP_ADD
			       + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv),
			      reg_x86_tlb,
			      x86_tlb_type->tme_recode_tlb_type.tme_recode_tlb_type_offset_memory,
			      TME_RECODE_X86_REG(reg_x86_address));

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}
