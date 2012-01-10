/* $Id: rc-x86-conds.c,v 1.3 2009/09/07 15:10:10 fredette Exp $ */

/* libtme/host/x86/rc-x86-conds.c - x86 host recode conditions support: */

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

_TME_RCSID("$Id: rc-x86-conds.c,v 1.3 2009/09/07 15:10:10 fredette Exp $");

/* macros: */

/* the fields of a conditions simple value: */
#define TME_RECODE_X86_CONDS_SIMPLE_Z		(1 << 0)
#define TME_RECODE_X86_CONDS_SIMPLE_ENABLE	(1 << 1)
#define TME_RECODE_X86_CONDS_SIMPLE_FLAGS_MASK	((tme_uint32_t) 0xfffffffc)

/* this collects up to one host register's worth of guest flags into
   the c register: */
static unsigned int
_tme_recode_x86_conds_flags_collect(struct tme_recode_ic *ic,
				    tme_recode_uguest_t conds_group_flags_mask,
				    unsigned long disp,
				    unsigned int reg_x86_load)
{
  tme_uint8_t *thunk_bytes;
  unsigned int size;
  tme_uint8_t opcode;
  tme_uint8_t rex;
  unsigned int count_bits_other;
  unsigned int count_shift;
  unsigned int count_bits_flags;

  /* we must be collecting some flags, and they must fit into a host
     register: */
  assert (conds_group_flags_mask != 0);
  assert (TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_HOST));

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* we will do the collecting using either host-size instructions or,
     if the flags fit into 32 bits, 32-bit instructions: */
  size = TME_RECODE_SIZE_HOST;
  if (TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_32)) {
    size = TME_RECODE_SIZE_32;
  }

  /* assume that we will use a normal mov instruction to load the
     flags into the load register.  this will load all of that
     register with variable bits: */
  opcode = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
  count_bits_other = TME_BIT(size);

  /* if the flags fit into 16 bits: */
  if (TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_16)) {

    /* emit the opcode escape for a movzw or a movzb: */
    *(thunk_bytes++) = TME_RECODE_X86_OPCODE_ESC_0F;

    /* assume that we will use a movzw instruction to load the flags
       into the load register: */
    opcode = TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv;
    count_bits_other = TME_BIT(TME_RECODE_SIZE_16);

    /* if the flags fit into eight bits: */
    if (TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_8)) {

      /* we will use a movzb instruction to load the flags into the
	 load register: */
      opcode = TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv;
      count_bits_other = TME_BIT(TME_RECODE_SIZE_8);
    }
  }

  /* emit the (remainder of the) instruction to load the flags into
     the load register: */
  rex = (TME_RECODE_X86_REX_R(size, reg_x86_load)
	 + TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC));
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }
  thunk_bytes[0] = opcode;
  thunk_bytes
    = _tme_recode_x86_emit_ic_modrm(thunk_bytes + 1,
				    disp,
				    reg_x86_load);

  /* initially, we haven't collected any flags yet, and we don't have
     any previous flags to shift off: */
  count_bits_flags = 0;
  count_shift = 0;

  /* while we still have flags to collect: */
  for (; conds_group_flags_mask != 0; ) {

    /* shift trailing zeroes off of the flags mask, adding them to the
       shift count: */
    for (; (conds_group_flags_mask & 1) == 0; ) {
      count_shift++;
      conds_group_flags_mask >>= 1;
    }

    /* if we have to shift the load register to get to the next flags
       to collect: */
    if (count_shift > 0) {

      /* shift the load register to the right: */
      rex = TME_RECODE_X86_REX_R(size, reg_x86_load);
      if (rex != 0) {
	*(thunk_bytes++) = rex;
      }
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
      thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_load),
						   TME_RECODE_X86_OPCODE_GRP2_SHR);
      thunk_bytes[2] = count_shift;
      thunk_bytes += 3;
    }

    /* update the count of non-flags bits in the load register: */
    count_bits_other -= count_shift;

    /* reset the shift count: */
    count_shift = 0;

    /* shift trailing ones off of the flags mask, adding them to the
       shift count: */
    for (; (conds_group_flags_mask & 1) == 1; ) {
      count_shift++;
      conds_group_flags_mask >>= 1;
    }

    /* update the count of non-flags bits in the load register: */
    count_bits_other -= count_shift;

    /* update (in advance) the count of flags bits that we have
       collected in the c register, and make sure it's still within
       the maximum of 32 bits allowed by recode backends: */
    count_bits_flags += count_shift;
    assert (count_bits_flags <= (sizeof(tme_uint32_t) * 8));

    /* if the load register is not the c register: */
    if (reg_x86_load != TME_RECODE_X86_REG_C) {

      /* use the shrd instruction to shift the flags being collected
	 out of the load register and into the c register.  NB we do
	 this with 32-bit instructions because recode backends aren't
	 required to handle any condition that tests more than 32
	 flags: */
      rex
	= (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, reg_x86_load)
	   + TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, TME_RECODE_X86_REG_C));
      if (rex != 0) {
	*(thunk_bytes++) = rex;
      }
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;
      thunk_bytes[1] = TME_RECODE_X86_OPCODE0F_SHRD_Ib_Gv_Ev;
      thunk_bytes[2]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C),
				      reg_x86_load);
      thunk_bytes[3] = count_shift;
      thunk_bytes += 4;
    }

    /* otherwise, the load register is the c register: */
    else {

      /* all of the flags must be contiguous, so we can't have any
	 more to collect: */
      assert (conds_group_flags_mask == 0);

      /* if there are any non-flags bits left in %ecx: */
      if (count_bits_other > 0
	  && count_bits_flags < 32) {

	/* emit an and $imm8s32, %ecx or an and $imm32, %ecx to clear
	   those non-flags bits: */
	thunk_bytes[0]
	  = (count_bits_flags < 8
	     ? TME_RECODE_X86_OPCODE_GRP1_Ib_Ev
	     : TME_RECODE_X86_OPCODE_GRP1_Iz_Ev);
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C),
					TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_AND));
	*((tme_uint32_t *) &thunk_bytes[2]) = ((((tme_uint32_t) 1) << count_bits_flags) - 1);
	thunk_bytes += 2 + (count_bits_flags < 8 ? sizeof(tme_int8_t) : sizeof(tme_uint32_t));
      }
    }
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* return the number of flags collected: */
  return (count_bits_flags);
}

/* this host function returns a new conditions thunk: */
struct tme_recode_conds_thunk *
tme_recode_host_conds_thunk_new(struct tme_recode_ic *ic,
				const struct tme_recode_conds_group *conds_group)
{
  struct tme_recode_conds_thunk *conds_thunk;
  unsigned int size_conds;
  tme_uint8_t *thunk_bytes;
  void *conds_array;
  tme_uint32_t conds_group_flags_index_max;
  tme_uint32_t conds_group_flags_index;
  tme_recode_uguest_t conds_group_flags;
  tme_uint32_t conds;
  unsigned int cond;
  tme_recode_uguest_t conds_group_flags_mask;
  unsigned int count_shift;
  int noncontiguous;
  unsigned int reg_x86_load;
  unsigned int count_bits_flags;
  tme_uint8_t opcode;
  tme_uint32_t bitwise_mask;
  tme_uint32_t simple;

  /* start the new conditions thunk: */
  if (!tme_recode_host_thunk_start(ic)) {
    abort();
  }
  assert (conds_group->tme_recode_conds_group_cond_count > 0);
  assert (conds_group->tme_recode_conds_group_cond_count <= 32);
  conds_thunk
    = ((struct tme_recode_conds_thunk *) 
       tme_malloc0(sizeof(struct tme_recode_conds_thunk)
		   + (sizeof(conds_thunk->tme_recode_x86_conds_thunk_simple[0])
		      * conds_group->tme_recode_conds_group_cond_count)));
  conds_thunk->tme_recode_x86_conds_thunk_subs
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);
  conds_thunk->tme_recode_conds_thunk_flags_offset
    = tme_recode_flags_reg_offset(conds_group->tme_recode_conds_group_flags_reg_size,
				  conds_group->tme_recode_conds_group_flags_reg);
  conds_thunk->tme_recode_conds_thunk_flags
    = conds_group->tme_recode_conds_group_flags;

  /* get the size of the word used to hold the conditions bitmap
     array.  we need one bit for each condition, and one recode
     conditions thunk may not handle more than 32 conditions: */
  size_conds
    = (conds_group->tme_recode_conds_group_cond_count <= 8
       ? TME_RECODE_SIZE_8
       : conds_group->tme_recode_conds_group_cond_count <= 16
       ? TME_RECODE_SIZE_16
       : TME_RECODE_SIZE_32);

  /* get the number of flags combinations tested by the conditions in
     this group: */
  conds_group_flags_index_max = tme_recode_conds_group_flags_index_max(conds_group);

  /* allocate the conditions bitmap array: */
  conds_array
    = tme_malloc(TME_BIT(size_conds - TME_RECODE_SIZE_8)
		 * (conds_group_flags_index_max + 1));

  /* loop over all of the combinations of flags tested by the
     conditions in this group: */
  conds_group_flags_index = 0;
  do {

    /* get this combination of flags: */
    conds_group_flags = tme_recode_conds_group_flags_from_index(conds_group, conds_group_flags_index);

    /* make the bitmap of conditions that are true for this
       combination of flags: */
    conds = 0;
    cond = 0;
    do {
      if ((*conds_group->tme_recode_conds_group_guest_func)
	  (conds_group_flags, cond)) {
	conds += (1 << cond);
      }
    } while (++cond < conds_group->tme_recode_conds_group_cond_count);

    /* store this bitmap: */
    switch (size_conds) {
    case TME_RECODE_SIZE_8: ((tme_uint8_t *) conds_array)[conds_group_flags_index] = conds; break;
    case TME_RECODE_SIZE_16: ((tme_uint16_t *) conds_array)[conds_group_flags_index] = conds; break;
    default: assert(FALSE);
    case TME_RECODE_SIZE_32: ((tme_uint32_t *) conds_array)[conds_group_flags_index] = conds; break;
    }

  } while (conds_group_flags_index++ != conds_group_flags_index_max);

  /* get the mask of flags tested by the conditions in this group, and
     the number of trailing zeroes in the mask: */
  conds_group_flags_mask = conds_group->tme_recode_conds_group_flags;
  count_shift = _tme_recode_x86_ffs(conds_group_flags_mask);

  /* see if the flags tested by the conditions in this group are all
     contiguous: */
  noncontiguous
    = (((conds_group_flags_mask
	 + (((tme_recode_uguest_t) 1) << count_shift))
	& conds_group_flags_mask) != 0);

  /* truncate the number of trailing zeroes in the mask to a multiple
     of eight, shift that many whole bytes off of the flags mask, and
     set the offset and size of the part of the flags register tested
     by this thunk: */
  count_shift &= 0 - (unsigned int) 8;
  conds_group_flags_mask >>= count_shift;
  conds_thunk->tme_recode_x86_conds_thunk_flags_offset
    = (conds_thunk->tme_recode_conds_thunk_flags_offset
       + (count_shift / 8));
  conds_thunk->tme_recode_x86_conds_thunk_flags_size
    = (TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_8)
       ? TME_RECODE_SIZE_8
       : TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_16)
       ? TME_RECODE_SIZE_16
       : TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_32)
       ? TME_RECODE_SIZE_32
       : TME_RECODE_SIZE_FITS(conds_group_flags_mask, TME_RECODE_SIZE_HOST)
       ? TME_RECODE_SIZE_HOST
       : TME_RECODE_SIZE_HOST + 1);

  /* we always collect the flags into the c register, which is always
     free.  assume that we can also use the c register to load the
     flags: */
  reg_x86_load = TME_RECODE_X86_REG_C;

  /* if the flags tested by the conditions in this group aren't
     contiguous, or if the flags are double-host-size: */
  if (noncontiguous
      || TME_RECODE_SIZE_IS_DOUBLE_HOST(conds_thunk->tme_recode_x86_conds_thunk_flags_size)) {

    /* push the d register, and use it to load the flags while we
       collect them into the c register: */
    reg_x86_load = TME_RECODE_X86_REG_D;
    tme_recode_x86_insns_start(ic, thunk_bytes);
    _tme_recode_x86_emit_reg_push(thunk_bytes, reg_x86_load);
    tme_recode_x86_insns_finish(ic, thunk_bytes);
  }

  /* load and collect flags from the (least-significant half of
     the) flags register tested by this thunk: */
  count_bits_flags
    = _tme_recode_x86_conds_flags_collect(ic,
					  (conds_group_flags_mask
					   & (TME_SHIFT(tme_recode_uguest_t,
							1,
							<<,
							TME_BIT(TME_RECODE_SIZE_HOST))
					      - 1)),
					  conds_thunk->tme_recode_x86_conds_thunk_flags_offset,
					  reg_x86_load);

  /* if the flags are double-host-size, load and collect flags from
     the most-significant half of the flags register tested by this
     thunk: */
  if (TME_RECODE_SIZE_IS_DOUBLE_HOST(conds_thunk->tme_recode_x86_conds_thunk_flags_size)) {
    count_bits_flags
      += _tme_recode_x86_conds_flags_collect(ic,
					     TME_SHIFT(tme_recode_uguest_t,
						       conds_group_flags_mask,
						       >>,
						       TME_BIT(TME_RECODE_SIZE_HOST)),
					     (conds_thunk->tme_recode_x86_conds_thunk_flags_offset
					      + TME_BIT(TME_RECODE_SIZE_HOST
							- TME_RECODE_SIZE_8)),
					     reg_x86_load);
  }

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* when we don't use the c register for loading, the flags get
     collected into the c register by shifting them in from the right,
     so we have to shift them down to start at bit zero: */
  if (reg_x86_load != TME_RECODE_X86_REG_C) {

    /* pop the register that we used for loading: */
    _tme_recode_x86_emit_reg_pop(thunk_bytes, reg_x86_load);

    /* if we collected less than 32 flags: */
    if (count_bits_flags < (sizeof(tme_uint32_t) * 8)) {

      /* shift the collected flags down: */
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
      thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C),
						   TME_RECODE_X86_OPCODE_GRP2_SHR);
      thunk_bytes[2] = (sizeof(tme_uint32_t) * 8) - count_bits_flags;
      thunk_bytes += 3;
    }
  }

  /* if this is an x86-64 host: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

    /* to keep things simple, we really want to limit conditions subs
       to the c register except in unusual cases.  on x86-64, this
       means that we have to add the 64-bit base of the conditions
       bitmap array to the c register from memory, after scaling the
       flags index in c by the size of the entries in that array: */

    /* dispatch on the size of the entries in the array: */
    switch (size_conds) {
    default: assert(FALSE);
    case TME_RECODE_SIZE_8:

      /* make sure that the most-significant 32 bits of %rcx are
	 zero: */
      thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev);
      thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C),
						   TME_RECODE_X86_REG_C);
      thunk_bytes += 2;
      break;

    case TME_RECODE_SIZE_16:

      /* shift %ecx by one: */
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_1_Ev;
      thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C),
						   TME_RECODE_X86_OPCODE_GRP2_SHL);
      thunk_bytes += 2;
      break;

    case TME_RECODE_SIZE_32:

      /* shift %ecx by two: */
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
      thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C),
						   TME_RECODE_X86_OPCODE_GRP2_SHL);
      thunk_bytes[2] = (TME_RECODE_SIZE_32 - TME_RECODE_SIZE_8);
      thunk_bytes += 3;
    }

    /* add the base of the conditions bitmap array to %rcx, from the
       (%rip-relative) value stored after the next movzbl/movzwl/movl
       and ret instructions: */
    thunk_bytes[0] = TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, TME_RECODE_X86_REG_C);
    thunk_bytes[1] = (TME_RECODE_X86_OPCODE_BINOP_ADD + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
    thunk_bytes[2] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_NONE),
						 TME_RECODE_X86_REG_C);
    *((tme_int32_t *) &thunk_bytes[3])
      = ((size_conds < TME_RECODE_SIZE_32)	/* TME_RECODE_X86_OPCODE_ESC_0F */
	 + 1					/* opcode */
	 + 1					/* ModR/M */
	 + 1);					/* TME_RECODE_X86_OPCODE_RET */
    thunk_bytes += 3 + sizeof(tme_int32_t);
  }

  /* emit the opcode(s) for a movzbl/movzwl/movl instruction,
     depending on the size of the entries in the conditions bitmap
     array: */
  opcode = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
  if (size_conds < TME_RECODE_SIZE_32) {
    *(thunk_bytes++) = TME_RECODE_X86_OPCODE_ESC_0F;
    opcode 
      = (size_conds == TME_RECODE_SIZE_16
	 ? TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv
	 : TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv);
  }
  thunk_bytes[0] = opcode;

  /* if this is an x86-64 host: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

    /* the complete address of the value we want to load is already in
       %rcx: */
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_REG_C),
						 TME_RECODE_X86_REG_C);
    thunk_bytes += 2;
  }

  /* otherwise, this is an ia32 host: */
  else {

    /* if the conditions bitmap array has eight-bit entries: */
    if (size_conds == TME_RECODE_SIZE_8) {

      /* emit a ModR/M byte for %ecx as the base with a 32-bit
	 displacement: */
      thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_REG_C),
						   TME_RECODE_X86_REG_C);
      thunk_bytes += 2;
    }

    /* otherwise, the conditions bitmap array has 16- or 32-bit entries: */
    else {

      /* emit a ModR/M byte and a scale-index-base byte for a scaled
	 %ecx index with a 32-bit displacement: */
      thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP32(TME_RECODE_X86_EA_BASE_SIB),
						   TME_RECODE_X86_REG_C);
      thunk_bytes[2] = TME_RECODE_X86_SIB(TME_RECODE_X86_SIB_BASE_NONE,
					  TME_RECODE_X86_REG_C,
					  TME_BIT(size_conds - TME_RECODE_SIZE_8));
      thunk_bytes += 3;
    }

    /* emit the 32-bit displacement to the conditions bitmap array: */
    *((tme_uint32_t *) thunk_bytes) = (unsigned long) conds_array;
    thunk_bytes += sizeof(tme_uint32_t);
  }

  /* return to the instruction thunk: */
  *(thunk_bytes++) = TME_RECODE_X86_OPCODE_RET;

  /* if this is an x86-64 host: */
  if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32) {

    /* store the 64-bit address of the conditions bitmap array at the
       end of the thunk: */
    memset(thunk_bytes, 0, TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));
    memcpy(thunk_bytes, &conds_array, sizeof(conds_array));
    thunk_bytes += TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* finish this conditions thunk: */
  tme_recode_host_thunk_finish(ic);

  /* loop over all of the conditions: */
  cond = 0;
  do {
    
    /* assume that this condition can't be tested simply: */
    simple = !TME_RECODE_X86_CONDS_SIMPLE_ENABLE;

    /* we can test this condition simply if there is a mask of flags
       that can be tested under OR or NOR to determine the condition's
       state, and those flags will fit into a simple condition
       information word: */
    bitwise_mask
      = tme_recode_conds_simple_mask(conds_group,
				     cond,
				     &conds_group_flags_mask);
    conds_group_flags_mask >>= count_shift;
    if ((bitwise_mask
	 & (TME_RECODE_BITWISE_OR
	    | TME_RECODE_BITWISE_NOR)) != 0
	&& (conds_group_flags_mask
	    <= TME_FIELD_MASK_EXTRACTU(TME_RECODE_X86_CONDS_SIMPLE_FLAGS_MASK,
				       TME_RECODE_X86_CONDS_SIMPLE_FLAGS_MASK))) {

      /* make the simple condition information word: */
      simple
	= (TME_RECODE_X86_CONDS_SIMPLE_ENABLE
	   | ((bitwise_mask & TME_RECODE_BITWISE_NOR)
	      ? TME_RECODE_X86_CONDS_SIMPLE_Z
	      : 0));
      TME_FIELD_MASK_DEPOSITU(simple,
			      TME_RECODE_X86_CONDS_SIMPLE_FLAGS_MASK,
			      conds_group_flags_mask);
    }

    /* set the simple condition information word for this condition: */
    conds_thunk->tme_recode_x86_conds_thunk_simple[cond] = simple;

  } while (++cond < conds_group->tme_recode_conds_group_cond_count);

  /* return the conditions thunk: */
  return (conds_thunk);
}

/* this emits instructions to define the recode carry flag: */
static void
_tme_recode_x86_conds_defc(struct tme_recode_ic *ic,
			   const struct tme_recode_insn *insn)
{
  const struct tme_recode_conds_thunk *conds_thunk;
  unsigned int cond;
  tme_uint32_t simple;
  unsigned int size;
  tme_uint8_t *thunk_bytes;
  unsigned int disp;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the conditions thunk and the condition from the
     instruction: */
  conds_thunk = insn->tme_recode_insn_conds_thunk;
  cond = insn->tme_recode_insn_operand_src[0];

  /* if this condition is simple: */
  simple = conds_thunk->tme_recode_x86_conds_thunk_simple[cond];
  if (__tme_predict_true(simple & TME_RECODE_X86_CONDS_SIMPLE_ENABLE)) {

    /* emit a testb/testw/testl instruction to test the condition's
       simple mask in the guest flags register: */
    size = conds_thunk->tme_recode_x86_conds_thunk_flags_size;
    assert (size <= TME_RECODE_SIZE_32);
    if (__tme_predict_false(size == TME_RECODE_SIZE_16)) {
      *thunk_bytes = TME_RECODE_X86_PREFIX_OPSIZ;
      thunk_bytes++;
    }
#if (TME_RECODE_X86_OPCODE_GRP3_Ev - 1) != TME_RECODE_X86_OPCODE_GRP3_Eb
#error "TME_RECODE_X86_OPCODE_ values changed"
#endif
    thunk_bytes[0]
      = (TME_RECODE_X86_OPCODE_GRP3_Ev
	 - ((size & 2) >> 1));
    thunk_bytes
      = _tme_recode_x86_emit_ic_modrm(thunk_bytes + 1,
				      conds_thunk->tme_recode_x86_conds_thunk_flags_offset,
				      TME_RECODE_X86_OPCODE_GRP3_TEST);
    *((tme_uint32_t *) thunk_bytes)
      = TME_FIELD_MASK_EXTRACTU(simple,
				TME_RECODE_X86_CONDS_SIMPLE_FLAGS_MASK);
    thunk_bytes += TME_BIT(size - TME_RECODE_SIZE_8);
  }

  /* otherwise, this condition is not simple: */
  else {

    /* the conditions subs destroys the c register: */
    ic->tme_recode_x86_ic_thunks_reg_guest_window_c = TME_RECODE_REG_GUEST_WINDOW_UNDEF;

    /* emit a call to the conditions subs: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_CALL_RELz;
    thunk_bytes += 1 + sizeof(tme_uint32_t);
    ((tme_uint32_t *) thunk_bytes)[-1]
      = (conds_thunk->tme_recode_x86_conds_thunk_subs
	 - tme_recode_build_to_thunk_off(ic, thunk_bytes));

    /* emit a testl instruction to test the condition in the mask
       returned in %ecx: */
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_OPCODE_GRP3_Ev
	 + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_C),
					TME_RECODE_X86_OPCODE_GRP3_TEST)
	    << 8));
    *((tme_uint32_t *) &thunk_bytes[2]) = ((tme_uint32_t) 1) << cond;
    thunk_bytes += 2 + sizeof(tme_uint32_t);

    /* if the test results in nonzero, the condition is true: */
    simple = !TME_RECODE_X86_CONDS_SIMPLE_Z;
  }

  /* if this instruction is negating the condition: */
  if (insn->tme_recode_insn_operand_src[1] < 0) {

    /* flip TME_RECODE_X86_CONDS_SIMPLE_Z: */
    simple = ~simple;
  }

  /* NB that a recode flag byte is zero if that flag is set, and one
     if it is clear, which seems backwards, but this allows us to set
     CF correctly by comparing a byte to 1.

     at this point, TME_RECODE_X86_CONDS_SIMPLE_Z is set if the
     condition is true if ZF is *set*, otherwise the condition is true
     if ZF is clear.  so we emit a setnz in the former case, otherwise
     we emit a setz: */
#if TME_RECODE_X86_CONDS_SIMPLE_Z != 1
#error "TME_RECODE_X86_CONDS_SIMPLE_Z value changed"
#endif
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;
  thunk_bytes[1]
    = TME_RECODE_X86_OPCODE0F_SETCC(TME_RECODE_X86_COND_Z
				    + ((simple
					& TME_RECODE_X86_CONDS_SIMPLE_Z)
				       * TME_RECODE_X86_COND_NOT));
  thunk_bytes += 2;
  disp = insn->tme_recode_insn_operand_dst;
  if (disp == 0) {
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB), 0)
	 + (TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1)
	    << 8));
    thunk_bytes += 2;
  }
  else {
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_EA_BASE_SIB), 0)
	 + (TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1)
	    << 8));
    thunk_bytes[2] = disp;
    thunk_bytes += 3;
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this emits an instruction that defines CF with the recode carry flag: */
static void
_tme_recode_x86_conds_testc(struct tme_recode_ic *ic,
			    unsigned int disp)
{
  tme_uint8_t *thunk_bytes;

  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* the recode carry flag byte at disp(%sp) is zero if the recode
     carry flag is set, and one if it is clear.  comparing this byte
     to one will set CF appropriately: */
  if (__tme_predict_true(disp == 0)) {
    *((tme_uint32_t *) &thunk_bytes[0])
      = (TME_RECODE_X86_OPCODE_GRP1_Ib_Eb
	 + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB),
					TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_CMP))
	    << 8)
	 + (TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1)
	    << 16)
	 + (1 << 24));
    thunk_bytes += 4;
  }
  else {
    *((tme_uint32_t *) &thunk_bytes[0])
      = (TME_RECODE_X86_OPCODE_GRP1_Ib_Eb
	 + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_EA_BASE_SIB),
					TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_CMP))
	    << 8)
	 + (TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1)
	    << 16));
    assert (disp < 0x80);
    thunk_bytes[3] = disp;
    thunk_bytes[4] = 1;
    thunk_bytes += 5;
  }

  /* finish this instruction: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

#ifdef TME_RECODE_DEBUG
#include <stdio.h>

/* this host function dumps a conditions thunk: */
void
tme_recode_host_conds_thunk_dump(const struct tme_recode_ic *ic,
				 const struct tme_recode_conds_thunk *conds_thunk,
				 const char * const *cond_names)
{
  unsigned int cond;
  tme_uint32_t simple;

  printf("  x86 conds subs: x/10i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 conds_thunk->tme_recode_x86_conds_thunk_subs,
					 void (*)(void)));
  printf("  x86 conds flags offset 0x%x bytes %u\n",
	 conds_thunk->tme_recode_x86_conds_thunk_flags_offset,
	 (1 << (conds_thunk->tme_recode_x86_conds_thunk_flags_size - TME_RECODE_SIZE_8)));
  for (cond = 0; cond_names[cond] != NULL; cond++) {
    printf("  x86 cond %2u (%s): ", cond, cond_names[cond]);

    /* if this condition is simple: */
    simple = conds_thunk->tme_recode_x86_conds_thunk_simple[cond];
    if (__tme_predict_true(simple & TME_RECODE_X86_CONDS_SIMPLE_ENABLE)) {
      printf("(flags & 0x%x) %c= 0\n",
	     TME_FIELD_MASK_EXTRACTU(simple,
				     TME_RECODE_X86_CONDS_SIMPLE_FLAGS_MASK),
	     (simple & TME_RECODE_X86_CONDS_SIMPLE_Z
	      ? '='
	      : '!'));
    }
    else {
      printf("subs\n");
    }
  }
}

#endif /* TME_RECODE_DEBUG */
