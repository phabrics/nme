/* $Id: rc-x86-regs.c,v 1.5 2010/02/15 22:23:15 fredette Exp $ */

/* libtme/host/x86/rc-x86-regs.c - x86 host recode register support: */

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

_TME_RCSID("$Id: rc-x86-regs.c,v 1.5 2010/02/15 22:23:15 fredette Exp $");

/* this maps a host register number to its x86 register number: */
/* NB: these are in a very deliberate order:

   the registers that are preserved across a C function call come
   first, from host register [0, TME_RECODE_REG_HOST_FREE_CALL)

   TME_RECODE_REG_HOST_SUBS_DST is used for the destination operand
   for subs.  it must be preserved across function calls, since subs
   may call guest C functions.  the hand-coded subs depend on the
   specific x86 register(s) used for this host register.

   TME_RECODE_REG_HOST_SUBS_SRC1 is used for the src1 operand for
   subs.  it does not need to be preserved across function calls.  the
   hand-coded subs depend on the specific x86 register(s) used for
   this host register.

   TME_RECODE_REG_HOST_SUBS_SRC0 is used for the src0 operand for subs
   that compute multiple condition codes and/or need to call a guest C
   function.  it must not be preserved across function calls.

   NB that the c register is really a scratch register, not available
   to the register allocator as either a host-size host register or as
   part of a double-host-size host register.  it is listed here since
   subs can use TME_RECODE_REG_HOST_SUBS_SRC0 as a double-host-size
   register anyways, and put the most-significant half of a value
   there, and also to make the cc thunk generator simpler.
*/
#define TME_RECODE_X86_REG_HOST_UNDEF		(TME_RECODE_REG_HOST_UNDEF + 1)
static const tme_uint8_t tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_UNDEF] = {
#if (TME_RECODE_SIZE_HOST <= TME_RECODE_SIZE_32)

#define TME_RECODE_X86_REG_HOST_SUBS_DST	TME_RECODE_REG_HOST(0)
  TME_RECODE_X86_REG_DI,	/* host reg 0 */
  TME_RECODE_X86_REG_SI,	/* host reg 1 */
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1	TME_RECODE_REG_HOST(2)
  TME_RECODE_X86_REG_BP,	/* host reg 2 */
#define TME_RECODE_X86_REG_HOST_FREE_CALL	TME_RECODE_REG_HOST(3)
  TME_RECODE_X86_REG_A,		/* host reg 3 */
#define TME_RECODE_X86_REG_HOST_SUBS_SRC0	TME_RECODE_REG_HOST(4)
  TME_RECODE_X86_REG_D,		/* host reg 4 */
  TME_RECODE_X86_REG_C,		/* not a true host register */

  /* this returns the host register number for an argument register.
     this is only valid for n == 1 and n == 2: */
#define TME_RECODE_X86_REG_HOST_ARG(n)		TME_RECODE_REG_HOST_UNDEF

#else  /* TME_RECODE_SIZE_HOST >= TME_RECODE_SIZE_32 */

  /* the x86-64 register layout is very careful:

     a cc thunk that needs to chain to a guest function can't use most
     of the argument registers, since they've already been loaded
     with guest function arguments.  however, it can use the nine
     register (in the max-guest-is-host-size case, guest function
     arguments will be in the di, si, dx, and cx registers, and in the
     max-guest-is-double-host-size case, guest function arguments will
     be in the di, si, dx, cx, and eight registers), and also the ten
     and eleven registers.

     a cc thunk that doesn't need to chain to a guest function can use
     the a, d, and c registers, which is good because they don't
     require a rex prefix when used with setcc. */
#define TME_RECODE_X86_REG_HOST_SUBS_DST	TME_RECODE_REG_HOST(0)
  TME_RECODE_X86_REG_N(12),	/* host reg 0 */
  TME_RECODE_X86_REG_N(13),	/* host reg 1 */
  TME_RECODE_X86_REG_N(14),	/* host reg 2 */
  TME_RECODE_X86_REG_N(15),	/* host reg 3 */
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1	TME_RECODE_REG_HOST(4)
  TME_RECODE_X86_REG_BP,	/* host reg 4 */
#define TME_RECODE_X86_REG_HOST_FREE_CALL	TME_RECODE_REG_HOST(5)
  TME_RECODE_X86_REG_A,		/* host reg 5 */
  TME_RECODE_X86_REG_N(10),	/* host reg 6 */
  TME_RECODE_X86_REG_N(11),	/* host reg 7 */
  TME_RECODE_X86_REG_DI,	/* host reg 8 */
  TME_RECODE_X86_REG_SI,	/* host reg 9 */
  TME_RECODE_X86_REG_N(8),	/* host reg 10 */
  TME_RECODE_X86_REG_N(9),	/* host reg 11 */
#define TME_RECODE_X86_REG_HOST_SUBS_SRC0	TME_RECODE_REG_HOST(12)
  TME_RECODE_X86_REG_D,		/* host reg 12 */
  TME_RECODE_X86_REG_C,		/* not a true host register */

  /* this returns the host register number for an argument register.
     this is only valid for n == 1 and n == 2: */
#define TME_RECODE_X86_REG_HOST_ARG(n)	\
  ((n) == 1				\
   ? TME_RECODE_REG_HOST(9)		\
   : TME_RECODE_REG_HOST(12))		\

#endif /* TME_RECODE_SIZE_HOST >= TME_RECODE_SIZE_32 */
};

/* this loads or stores a guest register: */
void
tme_recode_host_reg_move(struct tme_recode_ic *ic,
			 tme_uint32_t reg_guest,
			 tme_uint32_t reginfo_all)
{
  tme_uint8_t *thunk_bytes;
  unsigned long window_i;
  unsigned int reg_x86;
  unsigned int reg_x86_other;
  unsigned int size;
  unsigned int rex;
  tme_uint32_t reg_guest_offset;
  unsigned int mod_reg_rm;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* if this is a windowed guest register: */
  if (TME_RECODE_REGINFO_TYPE_IS_WINDOW(reginfo_all)) {

    /* get the index of the window that this guest register is in: */
    window_i = TME_RECODE_REGINFO_TYPE_WHICH_WINDOW(reginfo_all);

    /* if the c register doesn't already hold this window's base: */
    if (ic->tme_recode_x86_ic_thunks_reg_guest_window_c != window_i) {

      /* if this is an x86-64 host: */
      if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

	/* emit a movslq ic_window_base_offset(%ic), %rcx: */
	*((tme_uint32_t *) &thunk_bytes[0])
	  = ((TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, TME_RECODE_X86_REG_C)
	      | TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC))
	     + (TME_RECODE_X86_OPCODE_MOVS_El_Gv
		<< 8)
	     + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_REG_IC),
					    TME_RECODE_X86_REG_C)
		<< 16));
	*((tme_int32_t *) &thunk_bytes[3]) = ic->tme_recode_ic_window_base_offsets[window_i];
	thunk_bytes += (1 + 1 + 1 + sizeof(tme_int32_t));
      }

      /* otherwise, this is an ia32 host: */
      else {

	/* emit a movl ic_window_base_offset(%ic), %ecx: */
	*((tme_uint16_t *) &thunk_bytes[0])
	  = ((TME_RECODE_X86_OPCODE_BINOP_MOV
	      + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv)
	     + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_REG_IC),
					    TME_RECODE_X86_REG_C)
		<< 8));
	*((tme_int32_t *) &thunk_bytes[2]) = ic->tme_recode_ic_window_base_offsets[window_i];
	thunk_bytes += (1 + 1 + sizeof(tme_int32_t));
      }

      /* the window base is now in the c register: */
      ic->tme_recode_x86_ic_thunks_reg_guest_window_c = window_i;
    }
  }

  /* if this is a load: */
  if (!TME_RECODE_REGINFO_TAGS_ARE_DIRTY(reginfo_all)) {

    /* if we may need it for a rex prefix, get the (first) x86
       register to load, otherwise just set reg_x86 to zero to
       initialize it: */
    reg_x86
      = (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
	 ? tme_recode_x86_reg_from_host[TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all)]
	 : 0);

    /* emit any rex prefix: */
    /* NB: since it's zero, for clarity we always use
       TME_RECODE_X86_REG_X(0, TME_RECODE_X86_REG_C), even when the
       guest register may not be windowed: */
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
      thunk_bytes[0]
	= (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86)
	   | TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC)
	   | TME_RECODE_X86_REX_X(TME_RECODE_X86_REG_C));
    }

    /* get the size to load: */
    size = TME_RECODE_REGINFO_TAGS_WHICH_VALID_SIZE(reginfo_all);
    assert (size >= TME_RECODE_SIZE_8 && size <= TME_RECODE_SIZE_GUEST_MAX);

    /* if this is a host-size or double-host-size load: */
    if (__tme_predict_true(size >= TME_RECODE_SIZE_HOST)) {

      /* emit the opcode part of a movl @ea, %reg or a movq @ea, %reg: */
      thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
	= (TME_RECODE_X86_OPCODE_BINOP_MOV
	   + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    }

    /* otherwise, if this is a 32-bit load on an x86-64 host: */
    else if (__tme_predict_true(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
				&& size == TME_RECODE_SIZE_32)) {

      /* emit the opcode part of a movslq @ea, %reg: */
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_MOVS_El_Gv;
    }

    /* otherwise, this is either an 8- or 16-bit load: */
    else {

      /* emit the opcode part of a movsbl @ea, %reg or a movswl @ea, %reg: */
      *((tme_uint16_t *) &thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0])
	= (size < TME_RECODE_SIZE_16
	   ? (TME_RECODE_X86_OPCODE_ESC_0F + (TME_RECODE_X86_OPCODE0F_MOVS_Eb_Gv << 8))
	   : (TME_RECODE_X86_OPCODE_ESC_0F + (TME_RECODE_X86_OPCODE0F_MOVS_Ew_Gv << 8)));
      thunk_bytes++;
    }

    /* advance thunk_bytes to the modR/M byte: */
    thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1;

    /* if we didn't get it above, now get the (first) x86 register to
       load: */
    if (TME_RECODE_SIZE_HOST <= TME_RECODE_SIZE_32) {
      reg_x86 = tme_recode_x86_reg_from_host[TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all)];
    }

    /* this is not an 8-bit store of a register that doesn't have an
     8-bit encoding: */
    reg_x86_other = TME_RECODE_X86_REG_A;
  }

  /* otherwise, this is a store: */
  else {

    /* get the size to store: */
    size = TME_RECODE_REGINFO_TAGS_WHICH_VALID_SIZE(reginfo_all);
    assert (size >= TME_RECODE_SIZE_8 && size <= TME_RECODE_SIZE_GUEST_MAX);

    /* get the (first) x86 register to store: */
    reg_x86 = tme_recode_x86_reg_from_host[TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all)];

    /* in case we have to use a different register to do the store,
       remember the original register: */
    reg_x86_other = reg_x86;

    /* if this is an ia32 host, and an 8-bit store of a register
       that doesn't have an 8-bit encoding: */
    if (TME_RECODE_SIZE_HOST <= TME_RECODE_SIZE_32
	&& size == TME_RECODE_SIZE_8
	&& reg_x86 >= TME_RECODE_X86_REG_SP) {

      /* swap this register with the d register: */
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_BINOP_XCHG + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev;
      thunk_bytes[1]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				      TME_RECODE_X86_REG_D);
      thunk_bytes += 2;

      /* we are now storing the d register: */
      reg_x86 = TME_RECODE_X86_REG_D;
    }

    /* if this is a 16-bit store: */
    if (__tme_predict_false(size == TME_RECODE_SIZE_16)) {

      /* emit the OPSIZ prefix: */
      thunk_bytes[0] = TME_RECODE_X86_PREFIX_OPSIZ;
      thunk_bytes++;
    }

    /* emit any rex prefix: */
    /* NB: since it's zero, for clarity we always use
       TME_RECODE_X86_REG_X(0, TME_RECODE_X86_REG_C), even when the
       guest register may not be windowed: */
    rex
      = (TME_RECODE_X86_REX_R(TME_MIN(size, TME_RECODE_SIZE_HOST), reg_x86)
	 | TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC)
	 | TME_RECODE_X86_REX_X(TME_RECODE_X86_REG_C));
    if (rex != 0) {
      thunk_bytes[0] = rex;
      thunk_bytes++;
    }

    /* emit the opcode part of a movb/movw/movl/movq %reg, @ea: */
    thunk_bytes[0]
      = (size >= TME_RECODE_SIZE_16
	 ? (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev)
	 : (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gb_Eb));
    thunk_bytes++;
  }

  /* get the offset of the guest register: */
  reg_guest_offset
    = (reg_guest
       << (ic->tme_recode_ic_reg_size
	   - TME_RECODE_SIZE_8));

  for (;;) {

    /* make the modR/M byte for this instruction, with a zero base
       register field: */
    mod_reg_rm
      = (TME_RECODE_X86_MOD_OPREG_RM(0, TME_RECODE_X86_REG(reg_x86))
	 + (reg_guest_offset < 0x80
	    ? TME_RECODE_X86_MOD_RM_EA_DISP8(0)
	    : TME_RECODE_X86_MOD_RM_EA_DISP32(0)));

    /* if this is a windowed guest register: */
    if (TME_RECODE_REGINFO_TYPE_IS_WINDOW(reginfo_all)) {

      /* emit the modR/M and scale-index-base bytes for this instruction: */
      thunk_bytes[0] = mod_reg_rm + TME_RECODE_X86_REG(TME_RECODE_X86_EA_BASE_SIB);
      thunk_bytes++;
      thunk_bytes[0] = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_IC, TME_RECODE_X86_REG_C, 1);
    }
    else {

      /* emit the modR/M byte for this instruction: */
      thunk_bytes[0] = mod_reg_rm + TME_RECODE_X86_REG(TME_RECODE_X86_REG_IC);
    }

    /* emit the displacement for this instruction: */
    *((tme_uint32_t *) &thunk_bytes[1]) = reg_guest_offset;
    thunk_bytes += sizeof(tme_uint8_t) + sizeof(tme_int8_t);
    if (reg_guest_offset >= 0x80) {
      thunk_bytes += (sizeof(tme_int32_t) - sizeof(tme_int8_t));
    }

    /* if this is a double-host-size move: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(size)) {
      
      /* get the second x86 register to move: */
      reg_x86 = tme_recode_x86_reg_from_host[TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all) + 1];

      /* emit any rex prefix: */
      /* NB: since it's zero, for clarity we always use
	 TME_RECODE_X86_REG_X(0, TME_RECODE_X86_REG_C), even when the
	 guest register may not be windowed: */
      if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {
	thunk_bytes[0]
	  = (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86)
	     | TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC)
	     | TME_RECODE_X86_REX_X(TME_RECODE_X86_REG_C));
      }

      /* emit the opcode part of a movl/movq @ea, %reg or a movl/movq %reg, @ea: */
      thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
	= (TME_RECODE_REGINFO_TAGS_ARE_DIRTY(reginfo_all)
	   ? (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev)
	   : (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv));

      /* advance thunk_bytes to the modR/M byte: */
      thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1;

      /* advance reg_guest_offset to the most-significant half of the guest register: */
      reg_guest_offset += TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);

      /* loop to emit the modR/M for this instruction: */
      size = TME_RECODE_SIZE_HOST;
      continue;
    }

    break;
  }

  /* if this is an ia32 host, and this was an 8-bit store of a register
     that doesn't have an 8-bit encoding: */
  if (TME_RECODE_SIZE_HOST <= TME_RECODE_SIZE_32
      && size == TME_RECODE_SIZE_8
      && reg_x86_other >= TME_RECODE_X86_REG_SP) {

    /* we swap the register with the d register above, and stored the
       d register.  swap the registers back: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_BINOP_XCHG + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_other),
				    TME_RECODE_X86_REG_D);
    thunk_bytes += 2;
  }
      
  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this copies a host register into another host register: */
void
tme_recode_host_reg_copy(struct tme_recode_ic *ic,
			 unsigned long reg_host_src,
			 unsigned long reg_host_dst)
{
  tme_uint8_t *thunk_bytes;
  unsigned int size;
  unsigned long reg_x86_src;
  unsigned long reg_x86_dst;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the size to move: */
  size = ic->tme_recode_ic_reg_size;

  /* get the x86 registers in the (first) copy: */
  reg_x86_src = tme_recode_x86_reg_from_host[reg_host_src];
  reg_x86_dst = tme_recode_x86_reg_from_host[reg_host_dst];

  for (;;) {

    /* if this is an x86-64 host: */
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

      /* emit the rex prefix: */
      thunk_bytes[0]
	= (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86_src)
	   | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86_dst));
    }

    /* emit the opcode part of a movl %src, %dst or a movq %src, %dst: */
    thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 0]
      = (TME_RECODE_X86_OPCODE_BINOP_MOV
	 + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);

    /* emit the modR/M for this instruction: */
    thunk_bytes[(TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_src),
				    TME_RECODE_X86_REG(reg_x86_dst));

    /* advance to the next instruction: */
    thunk_bytes += (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) + 2;

    /* if this is a double-host-size move: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(size)) {

      /* loop to do the second registers of the pairs: */
      reg_x86_src = tme_recode_x86_reg_from_host[reg_host_src + 1];
      reg_x86_dst = tme_recode_x86_reg_from_host[reg_host_dst + 1];
      size = TME_RECODE_SIZE_HOST;
      continue;
    }

    break;
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this zeroes a host register and returns the host register. if insn
   is non-NULL, it also reserves the host register: */
unsigned long
tme_recode_host_reg_zero(struct tme_recode_ic *ic,
			 const struct tme_recode_insn *insn,
			 unsigned long reg_host)
{
  tme_uint8_t *thunk_bytes;
  unsigned int size;
  unsigned long reg_x86;
  unsigned int rex;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the size to zero: */
  size = ic->tme_recode_ic_reg_size;

  /* get the x86 register to zero: */
  reg_x86 = tme_recode_x86_reg_from_host[reg_host];

  for (;;) {

    /* if this is an x86-64 host: */
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

      /* emit any rex prefix: */
      rex
	= (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, reg_x86)
	   | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, reg_x86));
      thunk_bytes[0] = rex;
      thunk_bytes += (rex > 0);
    }

    /* emit the opcode part of a xorl %reg, %reg: */
    thunk_bytes[0]
      = (TME_RECODE_X86_OPCODE_BINOP_XOR
	 + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);

    /* emit the modR/M for this instruction: */
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				    TME_RECODE_X86_REG(reg_x86));

    /* advance to the next instruction: */
    thunk_bytes += 2;

    /* if this is a double-host-size zero: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(size)) {

      /* loop to do the second register of the pair: */
      reg_x86 = tme_recode_x86_reg_from_host[reg_host + 1];
      size = TME_RECODE_SIZE_HOST;
      continue;
    }

    break;
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* return the host register, optionally reserving it: */
  return (insn == NULL
	  ? reg_host
	  : tme_recode_regs_host_reserve(ic, reg_host));
}

/* this fills a host register with an immediate, and reserves and
   returns the host register: */
unsigned long
tme_recode_host_reg_imm(struct tme_recode_ic *ic,
			const struct tme_recode_insn *insn,
			unsigned long reg_host)
{
  tme_uint8_t *thunk_bytes;
  unsigned int size;
  unsigned long reg_x86;
  tme_recode_uguest_t imm;
  unsigned int size_part;
  unsigned int rex;

  /* get the immediate: */
  imm = insn->tme_recode_insn_imm_uguest;

  /* if the immediate is zero: */
  if (imm == 0) {

    /* zero this host register: */
    return (tme_recode_host_reg_zero(ic, insn, reg_host));
  }

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the size to fill: */
  size = insn->tme_recode_insn_size;

  /* get the x86 register to fill: */
  reg_x86 = tme_recode_x86_reg_from_host[reg_host];

  for (;;) {

    /* assume that we will move a host-sized immediate: */
    size_part = TME_RECODE_SIZE_HOST;

    /* if this is an x86-64 host: */
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

      /* if the most significant 32 bits of the immediate are zero, we
	 can move a 32-bit immediate instead of a 64-bit immediate: */
      size_part
	= (TME_RECODE_SIZE_32
	   + ((((unsigned long) imm)
	       & (((((unsigned long) 1)
		    << TME_BIT(TME_RECODE_SIZE_HOST - 1)) - 1)
		  << TME_BIT(TME_RECODE_SIZE_HOST - 1)))
	      != 0));

      /* emit any rex prefix: */
      rex = TME_RECODE_X86_REX_B(size_part, reg_x86);
      thunk_bytes[0] = rex;
      thunk_bytes += (rex > 0);
    }

    /* emit the opcode part of a movl $imm, %reg or movq $imm, %reg: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(reg_x86);

    /* emit the immediate for this instruction: */
    *((unsigned long *) &thunk_bytes[1]) = imm;

    /* advance to the next instruction: */
    thunk_bytes += 1 + TME_BIT(size_part - TME_RECODE_SIZE_8);

    /* if this is a double-host-size immediate: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(size)) {

      /* advance to the second register of the pair: */
      reg_x86 = tme_recode_x86_reg_from_host[reg_host + 1];
      imm >>= TME_BIT(TME_RECODE_SIZE_HOST) * (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST);

      /* if the immediate is zero: */
      if (imm == 0) {

	/* if this is an x86-64 host: */
	if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

	  /* emit any rex prefix: */
	  rex
	    = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, reg_x86)
	       | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, reg_x86));
	  thunk_bytes[0] = rex;
	  thunk_bytes += (rex > 0);
	}

	/* emit the opcode part of a xorl %reg, %reg: */
	thunk_bytes[0]
	  = (TME_RECODE_X86_OPCODE_BINOP_XOR
	     + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);

	/* emit the modR/M for this instruction: */
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
					TME_RECODE_X86_REG(reg_x86));

	/* advance to the next instruction: */
	thunk_bytes += 2;

	break;
      }

      /* loop to do the second register of the pair: */
      size = TME_RECODE_SIZE_HOST;
      continue;
    }

    break;
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* reserve and return the host register: */
  return (tme_recode_regs_host_reserve(ic, reg_host));
}
