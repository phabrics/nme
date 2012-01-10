/* $Id: rc-x86-flags.c,v 1.5 2010/02/15 22:21:48 fredette Exp $ */

/* libtme/host/x86/rc-x86-flags.c - x86 host recode flags support: */

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

_TME_RCSID("$Id: rc-x86-flags.c,v 1.5 2010/02/15 22:21:48 fredette Exp $");

/* this gives one of the x86 registers that can be used by an x86
   flags thunk: */
/* an x86-64 flags thunk that needs to call a guest function must use
   the 9, 10, and 11 registers.  on ia32 and on x86-64 otherwise, we
   must use the a, c, and d registers: */
#if (TME_RECODE_X86_REG_A + 1) != TME_RECODE_X86_REG_C || (TME_RECODE_X86_REG_C + 1) != TME_RECODE_X86_REG_D
#error "TME_RECODE_X86_REG_ values changed"
#endif
#define _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index)		\
  (((TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32				\
     || (flags_group)->tme_recode_flags_group_guest_func == NULL)	\
    ? TME_RECODE_X86_REG_A						\
    : TME_RECODE_X86_REG_N(9))						\
   + (flags_reg_index))
#define _TME_RECODE_X86_FLAGS_REG_COUNT   (3)

/* this adds a one-byte-opcode ModRM instruction to a thunk, that
   addresses the stacked host flags of a particular size: */
static tme_uint8_t *
_tme_recode_x86_flags_host_flags(const struct tme_recode_flags_group *flags_group,
				 tme_uint8_t *thunk_bytes,
				 tme_uint32_t larger_sizes,
				 tme_uint8_t rex,
				 tme_uint16_t opcode_opreg)
{
  unsigned long disp;

  /* the stacked host flags are always pushed in size order, largest
     last.  the least significant bit of larger_sizes must be set, and
     corresponds to the size of the flags that we want.  the remaining
     set bits are for the larger flag sizes that we must skip over.
     calculate the displacement to the flags we want, starting from
     the largest: */
  assert (larger_sizes & 1);
  larger_sizes -= 1;
  disp = 0;
  for (; larger_sizes != 0; larger_sizes &= (larger_sizes - 1)) {
    disp += TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
  }

  /* if there is a guest function: */
  if (flags_group->tme_recode_flags_group_guest_func != NULL) {

    /* there may be arguments for the guest function stacked before
       the host flags.  on ia32, all of the guest function arguments
       are stacked.  on x86-64, the destination operand argument is
       stacked, but only when double-host-size guests are
       supported: */
    disp
      += (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	  ? (sizeof(struct tme_ic *)
	     + sizeof(tme_recode_uguest_t)
	     + sizeof(tme_recode_uguest_t)
	     + sizeof(tme_recode_uguest_t))
	  : TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST
	  ? sizeof(tme_recode_uguest_t)
	  : 0);
  }

  /* emit the %sp-relative reference: */
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }
  *((tme_uint16_t *) &thunk_bytes[0])
    = (opcode_opreg
       | (TME_RECODE_X86_MOD_OPREG_RM((disp == 0
				       ? TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB)
				       : TME_RECODE_X86_MOD_RM_EA_DISP8(TME_RECODE_X86_EA_BASE_SIB)),
				      0) << 8));
  thunk_bytes[2] = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1);
  thunk_bytes += 3;
  if (disp != 0) {
    assert (disp < 0x80);
    *(thunk_bytes++) = disp;
  }

  /* done: */
  return (thunk_bytes);
}

/* this emits instructions to shift a destination register by some
   count, and then add in a register and a constant.  all of these
   three operations are optional: */
static tme_uint8_t *
_tme_recode_x86_flags_shift_add_add(const struct tme_recode_flags_group *flags_group,
				    tme_uint8_t *thunk_bytes,
				    unsigned int size,
				    unsigned int flags_reg_index_dest,
				    unsigned int dest_shift,
				    unsigned int flags_reg_index_addend,
				    tme_recode_uguest_t constant)
{
  unsigned int reg_x86_dst;
  tme_uint8_t rex;
  unsigned int reg_x86_base;
  unsigned int mod_rm;
  unsigned int size_constant;

  /* to keep things simple and instruction size down, we promote all
     operation sizes to at least 32 bits: */
  size = TME_MAX(size, TME_RECODE_SIZE_32);

  /* get the destination register: */
  reg_x86_dst = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_dest);

  /* if there is a destination shift, and it's more than three (more
     than a factor of eight) or if there is no constant and no addend
     register: */
  if (dest_shift != 0
      && (dest_shift > TME_RECODE_SIZE_8
	  || (constant == 0
	      && flags_reg_index_addend >= _TME_RECODE_X86_FLAGS_REG_COUNT))) {

    /* do all of the shifting with an shl instruction: */
    rex = TME_RECODE_X86_REX_R(size, reg_x86_dst);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
    thunk_bytes[1]
      = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_dst),
				    TME_RECODE_X86_OPCODE_GRP2_SHL);
    thunk_bytes[2] = dest_shift;
    thunk_bytes += 3;

    /* we don't need to shift the destination any more: */
    dest_shift = 0;
  }

  /* make the rex prefix for the lea.  if there is an addend register,
     that is the base register, otherwise if there is no destination
     shift then the destination register is also the base register,
     otherwise there is no base register.  if there is a destination
     shift, the destination register is also the index register,
     otherwise there is no index register: */
  rex
    = (TME_RECODE_X86_REX_R(size, reg_x86_dst)
       | (flags_reg_index_addend < _TME_RECODE_X86_FLAGS_REG_COUNT
	  ? TME_RECODE_X86_REX_B(0, _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_addend))
	  : dest_shift == 0
	  ? TME_RECODE_X86_REX_B(0, reg_x86_dst)
	  : 0)
       | (dest_shift != 0
	  ? TME_RECODE_X86_REX_X(reg_x86_dst)
	  : 0));

  /* if there is an addend register or a destination shift, the R/M
     base register indicates that there is a scale-index-base byte,
     otherwise the destination register is the R/M base register: */
  reg_x86_base
    = ((flags_reg_index_addend < _TME_RECODE_X86_FLAGS_REG_COUNT
	|| dest_shift != 0)
       ? TME_RECODE_X86_EA_BASE_SIB
       : reg_x86_dst);

  /* if there is no constant, we don't need a displacement: */
  if (constant == 0) {
    mod_rm = TME_RECODE_X86_MOD_RM_EA(reg_x86_base);
    size_constant = 0;

    /* if there is no scale-index-base byte, we don't need to emit the
       lea instruction at all: */
    if (reg_x86_base != TME_RECODE_X86_EA_BASE_SIB) {
      return (thunk_bytes);
    }
  }

  /* otherwise, if the constant fits into a sign-extended eight bits,
     we can use that displacement form: */
  else if (constant < 0x7f) {
    mod_rm = TME_RECODE_X86_MOD_RM_EA_DISP8(reg_x86_base);
    size_constant = sizeof(tme_uint8_t);
  }

  /* otherwise, we must use the 32-bit displacement form: */
  else {

    /* the constant must fit into 32 bits, unless the operation size is
       64 bits, in which case the constant must fit into 31 bits because
       the 32-bit displacement will be sign-extended to 64 bits: */
    assert ((constant >> 31) < (1 + (size <= TME_RECODE_SIZE_32)));
    mod_rm = TME_RECODE_X86_MOD_RM_EA_DISP32(reg_x86_base);
    size_constant = sizeof(tme_uint32_t);
  }

  /* emit the lea instruction: */
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_LEA;
  thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(mod_rm, TME_RECODE_X86_REG(reg_x86_dst));
  thunk_bytes += 2;
  if (reg_x86_base == TME_RECODE_X86_EA_BASE_SIB) {
    *(thunk_bytes++)
      = TME_RECODE_X86_SIB((flags_reg_index_addend < _TME_RECODE_X86_FLAGS_REG_COUNT
			    ? _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_addend)
			    : TME_RECODE_X86_SIB_BASE_NONE),
			   reg_x86_dst,
			   (1 << dest_shift));
  }
  memcpy(thunk_bytes, &constant, size_constant);
  thunk_bytes += size_constant;

  /* done: */
  return (thunk_bytes);
}

/* this updates one host-sized part of the guest flags: */
static tme_uint8_t *
_tme_recode_x86_flags_update(const struct tme_recode_flags_group *flags_group,
			     tme_uint8_t *thunk_bytes,
			     tme_recode_uguest_t flags_mask_update,
			     unsigned int flags_reg_index_accum,
			     tme_recode_uguest_t flag_accum_last,
			     unsigned int flags_reg_index_tmp)
{
  tme_recode_uguest_t flags_defined_host;
  tme_recode_uguest_t flags_set;
  tme_recode_uguest_t flags_clear;
  tme_recode_uguest_t flags_constant;
  unsigned int byte_shift;
  unsigned long disp;
  unsigned int size_flags_defined;
  unsigned int size_constant;
  unsigned int binop;
  unsigned int reg_x86;
  tme_uint8_t rex;
  
  /* get the mask of all defined flags that we are providing, that we
     are updating now.  if this is zero, return now: */
  flags_defined_host = tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_UNDEF);
  flags_defined_host &= flags_mask_update;
  if (flags_defined_host == 0) {
    return (thunk_bytes);
  }

  /* get the masks of flags that are always cleared or set, that we
     are updating now.  either or both of these may be zero: */
  flags_clear = tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_FALSE);
  flags_clear &= flags_mask_update;
  flags_set = tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_MOD_NOT | TME_RECODE_COND_FALSE);
  flags_set &= flags_mask_update;

  /* we only update whole bytes.  shift whole bytes of zero bits off
     of all guest flags masks, and get the displacement within the
     guest struct tme_ic of the first byte we are updating: */
  byte_shift = _tme_recode_x86_ffs(flags_defined_host) & (((unsigned int) 0) - 8);
  flags_defined_host >>= byte_shift;
  flags_clear >>= byte_shift;
  flags_set >>= byte_shift;
  flag_accum_last >>= byte_shift;
  disp
    = (tme_recode_flags_reg_offset(flags_group->tme_recode_flags_group_flags_reg_size,
				   flags_group->tme_recode_flags_group_flags_reg)
       + (((flags_mask_update & 1) == 0
	   ? TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8)
	   : 0)
	  + (byte_shift / 8)));

  /* get the log base two of the number of bytes in the guest flags
     register that we are updating: */
  size_flags_defined
    = ((flags_defined_host <= 0xff)
       ? TME_RECODE_SIZE_8
       : (flags_defined_host <= 0xffff)
       ? TME_RECODE_SIZE_16
       : (TME_RECODE_SIZE_32
	  + ((flags_defined_host >> 31) > 1)));

  /* to keep things simple (and to avoid updating eight bit register
     parts, which may decrease performance), we always use at least
     32-bit immediates with register operations: */
  size_constant = TME_MAX(size_flags_defined, TME_RECODE_SIZE_32);

  /* if the mask of all defined guest flags that we are providing,
     that we are updating now, is all flags that are always cleared,
     or all flags that are always set: */
  if (flags_defined_host == flags_clear
      || flags_defined_host == flags_set) {

    /* get the constant and binop for clearing or setting flags: */
    if (flags_defined_host == flags_clear) {
      flags_constant = ~flags_clear;
      binop = TME_RECODE_X86_OPCODE_BINOP_AND;
    }
    else {
      flags_constant = flags_set;
      binop = TME_RECODE_X86_OPCODE_BINOP_OR;
    }

    /* if this is a 64-bit operation, we have to load the constant
       into the temporary register and then do the binop from the
       temporary register to memory, since there aren't binop
       instructions with 64-bit immediates: */
    if (size_flags_defined > TME_RECODE_SIZE_32) {

      /* load the temporary register with the 64-bit immediate: */
      reg_x86 = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_tmp);
      thunk_bytes[0] = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, reg_x86);
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(reg_x86);
      memcpy(&thunk_bytes[2], &flags_constant, TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));
      thunk_bytes += 1 + 1 + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);

      /* update the guest flags: */
      thunk_bytes[0]
	= (TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC)
	   | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST, reg_x86));
      thunk_bytes[1] = (binop + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev);
      thunk_bytes
	= _tme_recode_x86_emit_ic_modrm(thunk_bytes + 2,
					disp,
					reg_x86);

      /* done: */
      return (thunk_bytes);
    }

    /* otherwise, emit the binop from the constant to memory: */
    if (size_flags_defined == TME_RECODE_SIZE_16) {
      *(thunk_bytes++) = TME_RECODE_X86_PREFIX_OPSIZ;
    }
#if TME_RECODE_X86_REG_IC >= 8
#error "TME_RECODE_X86_REG_IC changed"
#endif
    *(thunk_bytes++)
      = (size_flags_defined == TME_RECODE_SIZE_8
	 ? TME_RECODE_X86_OPCODE_GRP1_Ib_Eb
	 : TME_RECODE_X86_OPCODE_GRP1_Iz_Ev);
    thunk_bytes
      = _tme_recode_x86_emit_ic_modrm(thunk_bytes,
				      disp,
				      TME_RECODE_X86_OPCODE_GRP1_BINOP(binop));
    memcpy(thunk_bytes,
	   &flags_constant,
	   TME_BIT(size_flags_defined - TME_RECODE_SIZE_8));
    thunk_bytes += TME_BIT(size_flags_defined - TME_RECODE_SIZE_8);

    /* done: */
    return (thunk_bytes);
  }

  /* if we're updating all of the bits in a tme_uint8_t, tme_uint16_t,
     tme_uint32_t, or tme_uint64_t and we're not providing any
     variable guest flags, or if this is a 64-bit operation and the
     flags we're setting don't fit into 31 bits, we have to get the
     always-set flags into the register that (would have held any /
     does hold the) variable guest flags.  in the first case, this is
     an optimization, since the guest flags don't have to be read at
     all, and in the second case this is essential since an lea
     displacement is a sign-extended 32 bits: */
  if ((flags_defined_host == ((((unsigned long) 2) << (TME_BIT(size_flags_defined) - 1)) - 1)
       && flag_accum_last == 0)
      || (size_flags_defined > TME_RECODE_SIZE_32
	  && (flags_set >> 30) > 1)) {

    /* if there are any variable guest flags, load the always-set
       flags into the temporary register, otherwise load them directly
       into the register that would have held any variable guest
       flags: */
    reg_x86
      = _TME_RECODE_X86_FLAGS_REG(flags_group,
				  (flag_accum_last != 0
				   ? flags_reg_index_tmp
				   : flags_reg_index_accum));
    rex = TME_RECODE_X86_REX_B(size_constant, reg_x86);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(reg_x86);
    memcpy(&thunk_bytes[1], &flags_set, TME_BIT(size_constant - TME_RECODE_SIZE_8));
    thunk_bytes += 1 + TME_BIT(size_constant - TME_RECODE_SIZE_8);

    /* if there are any variable guest flags, shift the accumulation
       register as needed and add in the always-set flags in the
       temporary register: */
    if (flag_accum_last != 0) {
      thunk_bytes
	= _tme_recode_x86_flags_shift_add_add(flags_group,
					      thunk_bytes,
					      size_constant,
					      flags_reg_index_accum,
					      _tme_recode_x86_ffs(flag_accum_last),
					      flags_reg_index_tmp,
					      0);
    }

    /* we just combined the always-set flags with any variable guest
       flags.  from now on, there are no always-set flags and act like
       there are variable guest flags: */
    flag_accum_last = 1;
    flags_set = 0;
  }

  /* if we're updating all of the bits in a tme_uint8_t, tme_uint16_t,
     tme_uint32_t, or tme_uint64_t: */
  if (flags_defined_host == ((((unsigned long) 2) << (TME_BIT(size_flags_defined) - 1)) - 1)) {

    /* there must be some variable guest flags: */
    assert (flag_accum_last != 0);

    /* we don't have to read the guest flags at all to preserve any
       bits, so we don't have to add the temporary register to the
       variable guest flags below: */
    flags_reg_index_tmp = _TME_RECODE_X86_FLAGS_REG_COUNT;
  }

  /* otherwise, we're not updating all of the bits in a tme_uint8_t,
     tme_uint16_t, tme_uint32_t, or tme_uint64_t.  we have to read the
     guest flags, mask off the flags that we are updating, and
     preserve the rest: */
  else {

    /* if there are any variable guest flags, load the inverse of the
       mask of the guest flags that we are updating into the temporary
       register, otherwise load it directly into the register that
       would have held any variable guest flags: */
    flags_clear = ~flags_defined_host;
    reg_x86
      = _TME_RECODE_X86_FLAGS_REG(flags_group,
				  (flag_accum_last != 0
				   ? flags_reg_index_tmp
				   : flags_reg_index_accum));
    rex = TME_RECODE_X86_REX_B(size_constant, reg_x86);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(reg_x86);
    memcpy(&thunk_bytes[1], &flags_clear, TME_BIT(size_constant - TME_RECODE_SIZE_8));
    thunk_bytes += 1 + TME_BIT(size_constant - TME_RECODE_SIZE_8);

    /* binary-and the part of the guest flags register that we are
       updating into that same register: */
    if (size_flags_defined == TME_RECODE_SIZE_16) {
      *(thunk_bytes++) = TME_RECODE_X86_PREFIX_OPSIZ;
    }
    rex = TME_RECODE_X86_REX_B(size_flags_defined, reg_x86);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    *(thunk_bytes++)
      = (TME_RECODE_X86_OPCODE_BINOP_AND
	 + (size_flags_defined == TME_RECODE_SIZE_8
	    ? TME_RECODE_X86_OPCODE_BINOP_Eb_Gb
	    : TME_RECODE_X86_OPCODE_BINOP_Ev_Gv));
    thunk_bytes
      = _tme_recode_x86_emit_ic_modrm(thunk_bytes,
				      disp,
				      reg_x86);

    /* if there are no variable guest flags, we're only providing
       always-set and always-clear flags, which we now have in the
       register that would have held variable guest flags.  we don't
       have to add in the temporary register below: */
    if (flag_accum_last == 0) {
      flags_reg_index_tmp = _TME_RECODE_X86_FLAGS_REG_COUNT;
    }
  }

  /* shift the accumulation register as needed and add in any
     temporary register and any always-set guest flags: */
  thunk_bytes
    = _tme_recode_x86_flags_shift_add_add(flags_group,
					  thunk_bytes,
					  size_flags_defined,
					  flags_reg_index_accum,
					  _tme_recode_x86_ffs(flag_accum_last),
					  flags_reg_index_tmp,
					  flags_set);

  /* update the guest flags register: */
  reg_x86 = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_accum);
  if (size_flags_defined == TME_RECODE_SIZE_16) {
    *(thunk_bytes++) = TME_RECODE_X86_PREFIX_OPSIZ;
  }
  rex = (TME_RECODE_X86_REX_R(size_flags_defined, reg_x86)
	 + TME_RECODE_X86_REX_B(0, TME_RECODE_X86_REG_IC));
  if (rex != 0) {
    *(thunk_bytes++) = rex;
  }
  *(thunk_bytes++)
    = (TME_RECODE_X86_OPCODE_BINOP_MOV
       + (size_flags_defined == TME_RECODE_SIZE_8
	  ? TME_RECODE_X86_OPCODE_BINOP_Gb_Eb
	  : TME_RECODE_X86_OPCODE_BINOP_Gv_Ev));
  thunk_bytes
    = _tme_recode_x86_emit_ic_modrm(thunk_bytes,
				    disp,
				    reg_x86);

  /* done: */
  return (thunk_bytes);
}

/* this returns a new host flags thunk: */
struct tme_recode_flags_thunk *
tme_recode_host_flags_thunk_new(struct tme_recode_ic *ic,
				const struct tme_recode_flags_thunk *flags_thunk_template,
				const struct tme_recode_flags_group *flags_group)
{
  struct tme_recode_flags_thunk *flags_thunk;
  tme_uint8_t *thunk_bytes;
  tme_recode_uguest_t flags_defined_host;
  tme_recode_uguest_t flags_variable_host;
  tme_uint32_t host_sizes;
  int stack_adjust;
  tme_uint32_t host_sizes_other;
  tme_recode_uguest_t flags_mask_0;
  tme_recode_uguest_t flags_mask_1;
  unsigned int size_accum;
  const struct tme_recode_flag *flag;
  tme_uint32_t cond;
  tme_uint32_t cond_mods;
  tme_recode_uguest_t factor;
  unsigned int subfactor;
  unsigned int flags_reg_index_next;
  unsigned int flags_reg_index_accum;
  unsigned int flags_reg_drain;
  tme_uint8_t rex;
  unsigned int x86_reg_0;
  unsigned int x86_reg_1;
  tme_recode_uguest_t flags_reg_index_to_flag[_TME_RECODE_X86_FLAGS_REG_COUNT];
  tme_uint32_t host_size_loaded;

  /* start the new flags thunk: */
  if (!tme_recode_host_thunk_start(ic)) {
    abort();
  }
  flags_thunk = tme_new(struct tme_recode_flags_thunk, 1);
  *flags_thunk = *flags_thunk_template;
  flags_thunk->tme_recode_x86_flags_thunk_subs_main
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

  /* start instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* get the mask of all guest flags that we are providing.  this may
     be zero: */
  flags_defined_host = tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_UNDEF);

  /* get the mask of variable guest flags that we are providing.  this
     may be zero: */
  flags_variable_host
    = (flags_defined_host
       & ~(tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_FALSE)
	   | tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_MOD_NOT | TME_RECODE_COND_FALSE)));

  /* get the mask of sizes for which we need to provide at least one
     variable flag in the given flags group.  this may be
     zero: */
  host_sizes = tme_recode_flags_group_sizes(flags_group, flags_variable_host);

  /* it's easiest to assume that we always need the instruction-size
     flags when we need any flags at all, since the instruction-size
     operation always happens last no matter what, and leaves its
     flags in the flags register: */
  if (host_sizes != 0) {
    host_sizes |= TME_BIT(flags_group->tme_recode_flags_group_insn_size);
  }

  /* when we need the double-host-size flags, we also need the
     host-size flags, because we need the host-size Z flag to complete
     the double-host-size Z flag: */
  if (host_sizes & TME_BIT(TME_RECODE_SIZE_HOST + 1)) {
    host_sizes |= TME_BIT(TME_RECODE_SIZE_HOST);
  }

  /* assume that we won't need to remove anything from the stack
     before returning to the insn thunk: */
  stack_adjust = 0;

  /* if there are flags from multiple sizes: */
  if (host_sizes & (host_sizes - 1)) {

    /* the flags for the instruction size are in the flags register on
       entry into the thunk, and the others are stacked in size order,
       with the flags for the smallest size farthest away from the
       stack pointer.  emit a pushf to save the instruction-size flags
       on the stack: */
    *(thunk_bytes++) = TME_RECODE_X86_OPCODE_PUSHF;

    /* before returning to the insn thunk, we will need to remove all
       of these flags from the stack: */
    for (host_sizes_other = host_sizes;
	 host_sizes_other != 0;
	 host_sizes_other &= (host_sizes_other - 1)) {
      stack_adjust += TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
    }
  }

  /* assume that the opcode-specific flags subs won't need to
     do any stack padding: */
  flags_thunk->tme_recode_x86_flags_thunk_stack_padding = 0;

  /* if there is a guest function: */
  flags_thunk->tme_recode_x86_flags_thunk_has_guest_func
    = (flags_group->tme_recode_flags_group_guest_func != NULL);
  if (flags_thunk->tme_recode_x86_flags_thunk_has_guest_func) {

    /* if this is an ia32 host: */
    if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

      /* push the destination operand for the guest function: */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST + 1]);
      }
      _tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST]);

      /* push the second source operand for the guest function: */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1]);
      }
      _tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1]);

      /* push the first source operand for the guest function: */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1]);
      }
      _tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0]);

      /* push the struct tme_ic * for the guest function: */
      _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_IC);

      /* before returning to the insn thunk, we will need to remove
	 all of these guest function arguments from the stack: */
      stack_adjust
	+= (sizeof(struct tme_ic *)
	    + sizeof(tme_recode_uguest_t)
	    + sizeof(tme_recode_uguest_t)
	    + sizeof(tme_recode_uguest_t));
    }

    /* otherwise, this is an x86-64 host: */
    else {

      /* copy the struct tme_ic * for the guest function: */
      _tme_recode_x86_emit_reg_copy(thunk_bytes,
				    TME_RECODE_X86_REG_IC,
				    TME_RECODE_X86_REG_DI);

      /* copy the (least-significant half of) the first source operand
	 for the guest function: */
      _tme_recode_x86_emit_reg_copy(thunk_bytes,
				    tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 0],
				    TME_RECODE_X86_REG_SI);

      /* if double-host-size guests are supported: */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {

	/* copy the most-significant half of the first source operand
	   for the guest function: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1],
				      TME_RECODE_X86_REG_D);

	/* copy the second source operand for the guest function: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 0],
				      TME_RECODE_X86_REG_C);
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1],
				      TME_RECODE_X86_REG_N(8));

	/* push the destination operand for the guest function: */
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST + 1]);
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST + 0]);

	/* before returning to the insn thunk, we will need to remove
	   the destination operand argument from the stack: */
	stack_adjust += sizeof(tme_recode_uguest_t);

	/* the x86-64 ABI requires that the stack pointer be 16-byte
	   aligned immediately before a call instruction.  inside an
	   insn thunk, the stack pointer is 16-byte aligned
	   immediately before the call to a subs, which means at the
	   beginning of the subs it is only 8-byte aligned (because of
	   the return address for the subs).

	   if our stack won't be 16-byte aligned before we call the
	   guest function, the subs that chained to us will need to
	   pad the stack with one extra word to get it aligned: */
	if ((TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8) /* the subs return address */
	     + stack_adjust)
	    % (TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8) * 2)) {
	  flags_thunk->tme_recode_x86_flags_thunk_stack_padding = TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
	}
      }

      /* otherwise, double-host-size guests are not supported: */
      else {

	/* copy the second source operand for the guest function: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1],
				      TME_RECODE_X86_REG_D);

	/* copy the destination operand for the guest function: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST],
				      TME_RECODE_X86_REG_C);

	/* NB that when double-host-size guests are not supported, we
	   will always chain to the guest function, so we don't have
	   to worry about the ABI stack alignment like we do above: */
      }
    }

    /* we will also have to remove any stack padding: */
    stack_adjust += flags_thunk->tme_recode_x86_flags_thunk_stack_padding;
  }

  /* if any double-host-size flag is needed: */
  if (host_sizes & TME_BIT(TME_RECODE_SIZE_HOST + 1)) {

    /* the instruction-size flags we pushed above are incomplete
       double-host-size flags, and there are also host-size flags on
       the stack.  load the least-significant 32 bits of the host-size
       flags from the stack into %eax: */
    thunk_bytes
      = _tme_recode_x86_flags_host_flags(flags_group,
					 thunk_bytes,
					 (host_sizes >> TME_RECODE_SIZE_HOST),
					 TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, TME_RECODE_X86_REG_A),
					 ((TME_RECODE_X86_OPCODE_BINOP_MOV
					   + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv)
					  + (TME_RECODE_X86_MOD_OPREG_RM(0, 
									 TME_RECODE_X86_REG(TME_RECODE_X86_REG_A))
					     << 8)));

    /* or a mask of all-bits-one, minus the Z flag, into %eax: */
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP1_Ib_Ev;
    thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_A), TME_RECODE_X86_OPCODE_GRP1_BINOP(TME_RECODE_X86_OPCODE_BINOP_OR));
#if TME_RECODE_X86_FLAG_Z >= 0x80
#error "TME_RECODE_X86_FLAG_Z value changed"
#endif
    thunk_bytes[2] = ~TME_RECODE_X86_FLAG_Z;
    thunk_bytes += 3;

    /* and %eax into the least-significant 32 bits of the
       double-host-size flags on the stack: */
    thunk_bytes
      = _tme_recode_x86_flags_host_flags(flags_group,
					 thunk_bytes,
					 (host_sizes >> (TME_RECODE_SIZE_HOST + 1)),
					 TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, TME_RECODE_X86_REG_A),
					 ((TME_RECODE_X86_OPCODE_BINOP_AND
					   + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev)
					  + (TME_RECODE_X86_MOD_OPREG_RM(0,
									 TME_RECODE_X86_REG(TME_RECODE_X86_REG_A))
					     << 8)));

    /* initially, no host flags are loaded: */
    host_size_loaded = TME_RECODE_SIZE_1;
  }

  /* otherwise, no double-host-size flag is needed: */
  else {

    /* the flags from the instruction size are always in the flags
       register on entry into the thunk: */
    host_size_loaded = flags_group->tme_recode_flags_group_insn_size;
  }

  /* to keep things simple, only whole bytes are updated in the guest
     flags register.  with only host-size or smaller guests, this
     isn't so important since one host register will always cover the
     entire guest flags register, and most double-host-size guests
     will have their flags concentrated in their flags register so
     that one host flags register will also cover all of them, even when
     only updating whole bytes.  rarely, a double-host-size guest may
     have flags in so many bytes of its flags register that we have to
     split it in two halves, updated separately.  when we do this,
     flags_mask_1 is all-bits-one covering the most significant half
     of the guest flags register, otherwise flags_mask_1 is zero: */
  flags_mask_1
    = ((TME_SHIFT(tme_recode_uguest_t,
		  _tme_recode_x86_ffs_byte_shift(flags_defined_host),
		  >>,
		  TME_BIT(TME_RECODE_SIZE_HOST) - 1)
	> 1)
       * (0 - TME_SHIFT(tme_recode_uguest_t, 1, <<, TME_BIT(TME_RECODE_SIZE_HOST))));
  flags_mask_0 = ~flags_mask_1;

  /* if we aren't providing any variable guest flags: */
  if (flags_variable_host == 0) {

    /* we need to clear flags register zero, which will be used as the
       accumulated register for one or two
       _tme_recode_x86_flags_update() calls: */
    flags_reg_index_next = 1;
  }

  /* otherwise, if the whole-bytes span of defined flags in any
     host-sized part of the guest flags register is bigger than eight
     bits: */
  else if ((_tme_recode_x86_ffs_byte_shift(flags_defined_host & flags_mask_0)
	    | _tme_recode_x86_ffs_byte_shift(flags_defined_host & flags_mask_1))
	   > 0xff) {

    /* we need to clear all flags registers because setcc only sets
       the least significant byte of a register.  otherwise, the
       garbage in the other bytes would get accumulated into the
       results: */
    flags_reg_index_next = _TME_RECODE_X86_FLAGS_REG_COUNT;
  }

  /* otherwise, we are providing variable guest flags, and the
     whole-bytes span of defined guest flags in the one or two
     host-sized parts of the guest flags register fits into eight
     bits: */
  else {

    /* in this case, we don't need to clear any flags registers at
       all.  instead, we rely on setcc to clear the least significant
       eight bits of flags registers: */
    flags_reg_index_next = 0;
  }

  /* if there are any flags registers to clear: */
  if (flags_reg_index_next > 0) {

    /* if there are host flags initially loaded: */
    if (host_size_loaded != TME_RECODE_SIZE_1) {

      /* use only flags-preserving mov instructions: */

      /* mov an immediate zero into the first flags register: */
      x86_reg_0 = _TME_RECODE_X86_FLAGS_REG(flags_group, 0);
      rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, x86_reg_0);
      if (rex != 0) {
	*(thunk_bytes++) = rex;
      }
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOV_Iv_Gv(x86_reg_0);
      *((tme_uint32_t *) &thunk_bytes[1]) = 0;
      thunk_bytes += 1 + sizeof(tme_uint32_t);

      /* use register-to-register moves to copy that zero into any
	 other flags registers: */
      for (flags_reg_index_accum = 1;
	   flags_reg_index_accum < flags_reg_index_next;
	   flags_reg_index_accum++) {
	x86_reg_1 = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_accum);
	rex
	  = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, x86_reg_0)
	     | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, x86_reg_1));
	if (rex != 0) {
	  *(thunk_bytes++) = rex;
	}
	thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
	thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(x86_reg_0), TME_RECODE_X86_REG(x86_reg_1));
	thunk_bytes += 1 + 1;
      }
    }

    /* otherwise, no host flags are initially loaded: */
    else {

      /* use xor instructions to clear all of the registers: */
      for (flags_reg_index_accum = 0;
	   flags_reg_index_accum < flags_reg_index_next;
	   flags_reg_index_accum++) {
	x86_reg_0 = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_accum);
	rex
	  = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32, x86_reg_0)
	     | TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32, x86_reg_0));
	if (rex != 0) {
	  *(thunk_bytes++) = rex;
	}
	thunk_bytes[0] = (TME_RECODE_X86_OPCODE_BINOP_XOR + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
	thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(x86_reg_0), TME_RECODE_X86_REG(x86_reg_0));
	thunk_bytes += 2;
      }
    }
  }

  /* if the whole-bytes span of variable flags in any host-sized part
     of the guest flags register is bigger than 32 bits, we must
     accumulate using 64-bit math, otherwise we can use 32-bit
     math: */
  size_accum
    = (TME_RECODE_SIZE_32
       + (((_tme_recode_x86_ffs_byte_shift(flags_variable_host & flags_mask_0)
	    | _tme_recode_x86_ffs_byte_shift(flags_variable_host & flags_mask_1))
	   >> 31)
	  > 1));

  /* start accumulating the guest flags into the first flags register: */
  flags_reg_index_accum = 0;
  flags_reg_index_next = flags_reg_index_accum;
  memset(flags_reg_index_to_flag, 0, sizeof(flags_reg_index_to_flag));
  flags_reg_drain = _TME_RECODE_X86_FLAGS_REG_COUNT;

  /* loop over the flags: */
  flag = flags_group->tme_recode_flags_group_flags;
  for (;;) {

    /* if we're not at the end of the list of flags, but
       this is not a variable flag we're providing, skip
       it: */
    if (flag->tme_recode_flag_flag != 0
	&& (flag->tme_recode_flag_flag & flags_variable_host) == 0) {
      flag++;
      continue;
    }

    /* unless we haven't accumulated any flags yet: */
    if (flags_reg_index_to_flag[flags_reg_index_accum] != 0) {

      /* advance to the next register: */
      flags_reg_index_next++;
      if (flags_reg_index_next >= _TME_RECODE_X86_FLAGS_REG_COUNT) {
	flags_reg_index_next = 0;
      }

      /* always skip flags register zero, and the flags register we're
	 accumulating into (which can be nonzero when we need to
	 accumulate in two registers): */
      if (flags_reg_index_next == 0
	  || flags_reg_index_next == flags_reg_index_accum) {
	continue;
      }
    }

    /* if this next register is busy with a previous flag: */
    if (flags_reg_index_to_flag[flags_reg_index_next]) {

      /* if flags register zero has accumulated one or more flags from
	 the most significant half of a double-host-size guest flags
	 register, and the flag that is busy in the next register is
	 from the least significant half: */
      if ((flags_reg_index_to_flag[flags_reg_index_accum] & flags_mask_1)
	  && (flags_reg_index_to_flag[flags_reg_index_next] & flags_mask_0)) {

	/* this next register, that we originally wanted to store
	   another single flag into, is now actually the second flags
	   register that we will accumulate flags into, and already
	   has its first flag in it.  we loop to select another next
	   register for the next flag: */
	flags_reg_index_accum = flags_reg_index_next;
	continue;
      }

      /* otherwise, we need to shift the accumulating register up
	 and OR in the previous flag in this next register.  if the
	 factor between the last flag in the accumulating register
	 and this previous flag is less than or equal to eight, we
	 can do the shift and or in one lea instruction, otherwise
	 we need to shift the accumulating register by itself with
	 leas first: */
      for (factor = flags_reg_index_to_flag[flags_reg_index_accum] / flags_reg_index_to_flag[flags_reg_index_next];
	   factor > 1;
	   factor /= 8) {

	/* get the factor to multiply the index register (the
	   accumulating register) by, and the base to add to that,
	   which is none if this won't be our last shift: */
	if (factor > 8) {
	  subfactor = 8;
	  x86_reg_0 = TME_RECODE_X86_SIB_BASE_NONE;
	}
	else {
	  subfactor = factor;
	  x86_reg_0 = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_next);
	}

	/* emit the lea instruction to do the shift and add: */
	x86_reg_1 = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_accum);
	rex
	  = (TME_RECODE_X86_REX_W(size_accum)
	     | TME_RECODE_X86_REX_R(size_accum, x86_reg_1)
	     | TME_RECODE_X86_REX_X(x86_reg_1)
	     | TME_RECODE_X86_REX_B(size_accum, x86_reg_0));
	if (rex != 0) {
	  *(thunk_bytes++) = rex;
	}
	thunk_bytes[0] = TME_RECODE_X86_OPCODE_LEA;
	thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_EA_BASE_SIB, TME_RECODE_X86_REG(x86_reg_1));
	thunk_bytes[2] = TME_RECODE_X86_SIB(x86_reg_0, x86_reg_1, subfactor);
	thunk_bytes += 3;
      }

      /* update the last flag accumulated: */
      flags_reg_index_to_flag[flags_reg_index_accum] = flags_reg_index_to_flag[flags_reg_index_next];
      
      /* this next register is no longer busy: */
      flags_reg_index_to_flag[flags_reg_index_next] = 0;
    }

    /* if there are no more flags: */
    if (flag->tme_recode_flag_flag == 0) {

      /* keep looping until we've accumulated all of the flags still
	 in flags registers: */
      if (flags_reg_drain--) {
	continue;
      }

      /* otherwise, stop now: */
      break;
    }

    /* get the condition and remove any modifiers: */
    cond = flag->tme_recode_flag_cond;
    cond_mods = (cond & TME_RECODE_COND_MOD_NOT);
    cond -= cond_mods;

    /* if the x86 flags for this size aren't loaded: */
    assert (flag->tme_recode_flag_size >= TME_RECODE_SIZE_8);
    if (host_size_loaded == TME_RECODE_SIZE_1
	|| (cond != TME_RECODE_COND_PE
	    && host_size_loaded != flag->tme_recode_flag_size)) {

      /* push the flags for this size: */
      thunk_bytes
	= _tme_recode_x86_flags_host_flags(flags_group,
					   thunk_bytes,
					   (host_sizes >> flag->tme_recode_flag_size),
					   0,
					   (TME_RECODE_X86_OPCODE_GRP5
					    + (TME_RECODE_X86_MOD_OPREG_RM(0,
									   TME_RECODE_X86_OPCODE_GRP5_PUSH)
					       << 8)));

      /* popf the flags for this size: */
      *(thunk_bytes++) = TME_RECODE_X86_OPCODE_POPF;

      /* the x86 flags for this size are now loaded: */
      host_size_loaded = flag->tme_recode_flag_size;
    }

    /* emit the setcc instruction for this condition: */
    switch (cond) {
    default: assert(FALSE);
    case TME_RECODE_COND_N: cond = TME_RECODE_X86_COND_S; break;
    case TME_RECODE_COND_C: cond = TME_RECODE_X86_COND_C; break;
    case TME_RECODE_COND_V: cond = TME_RECODE_X86_COND_O; break;
    case TME_RECODE_COND_Z: cond = TME_RECODE_X86_COND_Z; break;
    case TME_RECODE_COND_PE: cond = TME_RECODE_X86_COND_PE; break;
    case TME_RECODE_COND_BE: cond = TME_RECODE_X86_COND_BE; break;
    case TME_RECODE_COND_LE: cond = TME_RECODE_X86_COND_LE; break;
    case TME_RECODE_COND_L: cond = TME_RECODE_X86_COND_L; break;
    }
    if (cond_mods & TME_RECODE_COND_MOD_NOT) {
      cond += TME_RECODE_X86_COND_NOT;
    }
    x86_reg_0 = _TME_RECODE_X86_FLAGS_REG(flags_group, flags_reg_index_next);
    rex = TME_RECODE_X86_REX_B(TME_RECODE_SIZE_8, x86_reg_0);
    if (rex != 0) {
      *(thunk_bytes++) = rex;
    }
    thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;
    thunk_bytes[1] = TME_RECODE_X86_OPCODE0F_SETCC(cond);
    thunk_bytes[2] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(x86_reg_0), 0);
    thunk_bytes += 3;

    /* this register now contains this flag: */
    flags_reg_index_to_flag[flags_reg_index_next] = flag->tme_recode_flag_flag;

    /* continue with the next flag: */
    flag++;
  }

  /* we always try to update at least one part of the guest flags
     register.  if we are providing variable flags, and they're all in
     the most-significant host-sized half of a double-host-size guest
     flags register, update that half of the guest flags register now,
     otherwise update the least-significant half (which may mean the
     whole guest flags register): */
  thunk_bytes
    = _tme_recode_x86_flags_update(flags_group,
				   thunk_bytes,
				   ((flags_reg_index_to_flag[flags_reg_index_accum]
				     & flags_mask_1)
				    ? flags_mask_1
				    : flags_mask_0),
				   flags_reg_index_accum,
				   flags_reg_index_to_flag[flags_reg_index_accum],
				   flags_reg_index_next);

  /* if we are providing variable flags and this is a double-host-size
     guest flags register and there are variable flags in both halves,
     we've only updated the least significant half above.  or, if all
     of the variable flags are in the most-significant half, we've
     only updated the most-significant half above.  we need to update
     the other half of the guest flags register: */
  if (flags_reg_index_accum != 0
      || (flags_reg_index_to_flag[flags_reg_index_accum]
	  & flags_mask_1)) {

    /* we've already updated the half of the guest flags register that
       was being accumulated in flags_reg_index_accum, so we can now free
       it up: */
    flags_reg_index_to_flag[flags_reg_index_accum] = 0;

    /* if we haven't already updated the half of the guest flags
       variable that was being accumulated in flags register zero, we'll
       do it now: */
    flags_reg_index_accum = 0;

    /* update the other half of the guest flags register: */
    thunk_bytes
      = _tme_recode_x86_flags_update(flags_group,
				     thunk_bytes,
				     ((flags_reg_index_to_flag[flags_reg_index_accum]
				       & flags_mask_1)
				      ? flags_mask_1
				      : flags_mask_0),
				     flags_reg_index_accum,
				     flags_reg_index_to_flag[flags_reg_index_accum],
				     flags_reg_index_next);
  }

  /* if there is a guest function: */
  if (flags_thunk->tme_recode_x86_flags_thunk_has_guest_func) {

    /* if this is an x86-64 host, and double-host-size guests are not
       supported, and we have things to remove from the stack: */
    if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
	&& TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_HOST
	&& stack_adjust != 0) {

      /* we can chain to the guest function, so adjust the stack
	 pointer now: */
      thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, stack_adjust);
      stack_adjust = 0;
    }

    /* jmp or call the guest function: */
    tme_recode_x86_insns_finish(ic, thunk_bytes);
    _tme_recode_x86_emit_transfer_func(ic,
				       (stack_adjust == 0
					? TME_RECODE_X86_OPCODE_JMP_RELz
					: TME_RECODE_X86_OPCODE_CALL_RELz),
				       ((void (*) _TME_P((void))) 
					flags_group->tme_recode_flags_group_guest_func));
    tme_recode_x86_insns_start(ic, thunk_bytes);
  }

  /* if we still have to adjust the stack, or if there is no guest
     function: */
  if (stack_adjust != 0
      || !flags_thunk->tme_recode_x86_flags_thunk_has_guest_func) {

    /* do any stack adjust: */
    if (stack_adjust != 0) {
      thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, stack_adjust);
    }

    /* return to the instruction thunk: */
    *(thunk_bytes++) = TME_RECODE_X86_OPCODE_RET;
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* finish this main flags subs thunk: */
  tme_recode_host_thunk_finish(ic);

  /* add this flags group to this new flags thunk: */
  return (tme_recode_host_flags_thunk_add(ic, flags_thunk, flags_group));
}

/* this emits a jmp to chain to a subs, and finishes a thunk: */
static void
_tme_recode_x86_flags_thunk_chain(struct tme_recode_ic *ic,
				  tme_recode_thunk_off_t subs_next)
{
  tme_uint8_t *thunk_bytes;

  /* start another instruction: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* emit a jmp to the subs: */
  thunk_bytes[0] = TME_RECODE_X86_OPCODE_JMP_RELz;
  thunk_bytes += 1 + sizeof(tme_int32_t);
  ((tme_int32_t *) thunk_bytes)[-1]
    = (subs_next
       - tme_recode_build_to_thunk_off(ic, thunk_bytes));

  /* finish this instruction: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* finish this code thunk: */
  tme_recode_host_thunk_finish(ic);
}

/* this emits instructions to do a binop at multiple sizes: */
static void
_tme_recode_x86_flags_thunk_ops(struct tme_recode_flags_thunk *flags_thunk,
				unsigned int binop,
				unsigned int reg_host_src,
				tme_uint32_t host_sizes,
				struct tme_recode_ic *ic)
{
  tme_uint8_t *thunk_bytes;
  int stack_adjust;
  unsigned int size;
  unsigned int reg_x86_src;
  unsigned int reg_x86_dst;
  unsigned int reg_x86_op_src;
  unsigned int reg_x86_op_dst;
  tme_uint8_t rex;

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* do any stack padding needed for the host ABI: */
  stack_adjust = flags_thunk->tme_recode_x86_flags_thunk_stack_padding;
  if (stack_adjust != 0) {
    thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, stack_adjust);
  }

  /* we need to track the stack adjust here for binops that test the
     carry.  include the subs return address in the stack adjust: */
  stack_adjust += TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);

  /* loop over all of the sizes: */
  for (; host_sizes != 0; ) {

    /* get this size, and remove it from the mask: */
    size = _tme_recode_x86_ffs(host_sizes);
    host_sizes &= (host_sizes - 1);

    /* if this is a double-host size operation: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(size)) {

      /* force a host-size operation on the most-significant halves of
	 the host registers: */
      size = TME_RECODE_SIZE_HOST;
      reg_x86_src = tme_recode_x86_reg_from_host[reg_host_src + 1];
      reg_x86_dst = tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST + 1];

      /* the most-significant half of a double-host-size additive
	 operation uses the carry from the least-significant half: */
      if (binop == TME_RECODE_X86_OPCODE_BINOP_ADD) {
	binop = TME_RECODE_X86_OPCODE_BINOP_ADC;
      }
      else if (binop == TME_RECODE_X86_OPCODE_BINOP_SUB) {
	binop = TME_RECODE_X86_OPCODE_BINOP_SBB;
      }
    }

    /* otherwise, this is not a double-host-size operation: */
    else {

      /* operate on the (least-significant halves of) the host
	 registers: */
      reg_x86_src = tme_recode_x86_reg_from_host[reg_host_src];
      reg_x86_dst = tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST];

      /* if this is an additive instruction that needs the carry: */
      if (binop == TME_RECODE_X86_OPCODE_BINOP_ADC
	  || binop == TME_RECODE_X86_OPCODE_BINOP_SBB) {

	/* emit the instruction to define CF with the recode carry flag: */
	tme_recode_x86_insns_finish(ic, thunk_bytes);
	_tme_recode_x86_conds_testc(ic, stack_adjust);
	tme_recode_x86_insns_start(ic, thunk_bytes);
      }
    }

    /* if this is a test instruction, the source register is the same
       as the destination: */
    if (binop == TME_RECODE_X86_OPCODE_BINOP_TEST) {
      reg_x86_src = reg_x86_dst;
    }

    /* assume that we will be able to do the operation on the x86
       registers as loaded: */
    reg_x86_op_src = reg_x86_src;
    reg_x86_op_dst = reg_x86_dst;

    /* if this is an ia32 host, and this is an eight-bit operation: */
    if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	&& size == TME_RECODE_SIZE_8) {

      /* eight-bit operations aren't possible with the normal source
	 and destination x86 registers, since they don't have
	 eight-bit encodings: */

      /* if this is a test instruction: */
      if (binop == TME_RECODE_X86_OPCODE_BINOP_TEST) {

	/* emit a testl %reg, $0xff: */
	thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP3_Ev;
	thunk_bytes[1] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_op_dst),
						     TME_RECODE_X86_OPCODE_GRP3_TEST);
	*((tme_uint32_t *) &thunk_bytes[2]) = 0xff;
	thunk_bytes += 2 + sizeof(tme_uint32_t);

	/* suppress the binop instruction below: */
	size = TME_RECODE_SIZE_1;
      }

      /* otherwise, this is not a test instruction: */
      else {

	/* we need to get the (least-significant halves of the)
	   destination and second source operands into the two halves
	   of the first source operand host register: */
	reg_x86_op_src = tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1];
	reg_x86_op_dst = tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 0];

	/* mov the (least-significant half of the) second source
	   operand into place: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      reg_x86_src,
				      reg_x86_op_src);

	/* the (least significant half of the) destination operand has
	   already been copied to the (least significant half of the)
	   first source operand host register, unless there is no
	   guest function and this is the only operation size: */
	if (!flags_thunk->tme_recode_x86_flags_thunk_has_guest_func
	    && host_sizes == 0) {

	  /* otherwise, mov the (least-significant half of the)
	     destination operand into place: */
	  _tme_recode_x86_emit_reg_copy(thunk_bytes,
					reg_x86_dst,
					reg_x86_op_dst);
	}
      }
    }

    /* unless the binop has been suppressed: */
    if (size != TME_RECODE_SIZE_1) {

      /* if this is a 16-bit operation: */
      if (size == TME_RECODE_SIZE_16) {

	/* emit the OPSIZ prefix: */
	thunk_bytes[0] = TME_RECODE_X86_PREFIX_OPSIZ;
	thunk_bytes++;
      }

      /* emit any rex prefix: */
      /* NB: we must use the Gb_Eb/Gv_Ev forms here since those are the
	 only ones that the test instruction supports: */
      rex
	= (TME_RECODE_X86_REX_R(size, reg_x86_op_src)
	   | TME_RECODE_X86_REX_B(size, reg_x86_op_dst));
      if (rex != 0) {
	thunk_bytes[0] = rex;
	thunk_bytes++;
      }

      /* emit the opcode: */
      thunk_bytes[0]
	= (binop
	   + (size == TME_RECODE_SIZE_8
	      ? TME_RECODE_X86_OPCODE_BINOP_Gb_Eb
	      : TME_RECODE_X86_OPCODE_BINOP_Gv_Ev));
      thunk_bytes++;

      /* emit the mod R/M byte: */
      thunk_bytes[0] = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_op_dst),
						   TME_RECODE_X86_REG(reg_x86_op_src));
      thunk_bytes++;
    }

    /* if we have more sizes to do: */
    if (host_sizes != 0) {

      /* push these flags: */
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_PUSHF;
      thunk_bytes++;
      stack_adjust += TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
    }

    /* if this is an ia32 host, and this was an eight-bit operation: */
    if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	&& size == TME_RECODE_SIZE_8) {

      /* if double-host-size guests are supported, and the last size
	 is double-host-size, and there is a guest function: */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST
	  && (host_sizes & TME_BIT(TME_RECODE_SIZE_GUEST_MAX))
	  && flags_thunk->tme_recode_x86_flags_thunk_has_guest_func) {

	/* copy the most-significant half of the destination operand
	   back into the most-significant half of the first source
	   operand, to be passed to the guest function: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST + 1],
				      reg_x86_op_src);
      }

      /* if we have a guest function, or if we have at least two more
	 sizes to do: */
      if (flags_thunk->tme_recode_x86_flags_thunk_has_guest_func
	  || (host_sizes & (host_sizes - 1))) {

	/* copy the (least-significant half of) the destination
	   operand back into the (least-significant half of) the first
	   source operand, so it can be passed to the guest function
	   and/or used to reload the destination operand later.  if we
	   have no more sizes to do, this must be an xchg to put the
	   destination value in the destination operand: */
	thunk_bytes[0]
	  = ((host_sizes == 0
	      ? TME_RECODE_X86_OPCODE_BINOP_XCHG
	      : TME_RECODE_X86_OPCODE_BINOP_MOV)
	     + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev);
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86_op_dst),
					reg_x86_dst);
	thunk_bytes += 2;
      }

      /* otherwise, if we have no more sizes to do: */
      else if (host_sizes == 0) {

	/* copy the destination operand back into place: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      reg_x86_op_dst,
				      reg_x86_dst);
      }
    }

    /* otherwise, this is not an ia32 host or this was not
       an eight-bit operation: */
    else {

      /* if this was not a test instruction, and we have at least one
	 more size to do, and it isn't the double-host-size size: */
      if (binop != TME_RECODE_X86_OPCODE_BINOP_TEST
	  && host_sizes != 0
	  && (TME_RECODE_SIZE_GUEST_MAX == TME_RECODE_SIZE_HOST
	      || host_sizes != TME_BIT(TME_RECODE_SIZE_GUEST_MAX))) {

	/* reload the (least-significant half of the) destination
	   operand: */
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 0],
				      reg_x86_dst);
      }
    }
  }

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);
}

/* this adds another flags group to an existing flags thunk: */
struct tme_recode_flags_thunk *
tme_recode_host_flags_thunk_add(struct tme_recode_ic *ic,
				struct tme_recode_flags_thunk *flags_thunk,
				const struct tme_recode_flags_group *flags_group)
{
  tme_recode_uguest_t flags_variable_host;
  tme_uint32_t host_sizes;
  int multiple_ops;
  tme_recode_thunk_off_t *_subs;
  tme_recode_thunk_off_t subs_test;
  unsigned int size_other;
  unsigned int opcode;
  struct tme_recode_insn insn;
  struct tme_recode_insn insn_not;
  unsigned int class;
  tme_uint8_t *thunk_bytes;

  /* get the mask of variable guest flags that we are providing.  this
     may be zero: */
  flags_variable_host
    = (tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_UNDEF)
       & ~(tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_FALSE)
	   | tme_recode_flags_group_flags_defined_host(flags_group, TME_RECODE_COND_MOD_NOT | TME_RECODE_COND_FALSE)));

  /* get the mask of sizes for which we need to provide at least one
     variable flag in the given flags group.  this may
     be zero: */
  host_sizes = tme_recode_flags_group_sizes(flags_group, flags_variable_host);

  /* it's easiest to assume that we always need the instruction-size
     flags when we need any flags at all, since the instruction-size
     operation always happens last no matter what, and leaves its flags
     in the flags register: */
  if (host_sizes != 0) {
    host_sizes |= TME_BIT(flags_group->tme_recode_flags_group_insn_size);
  }

  /* when we need the double-host-size flags, we also need
     the host-size flags, because we need the host-size Z
     flag to complete the double-host-size Z flag: */
  if (host_sizes & TME_BIT(TME_RECODE_SIZE_HOST + 1)) {
    host_sizes |= TME_BIT(TME_RECODE_SIZE_HOST);
  }

  /* we have to do multiple operations if we need flags for multiple
     sizes, not counting the double-host-size: */
  multiple_ops
    = (((host_sizes
	 & (TME_BIT(TME_RECODE_SIZE_HOST + 1) - 1))
	& ((host_sizes
	    & (TME_BIT(TME_RECODE_SIZE_HOST + 1) - 1))
	   - 1))
       != 0);

  /* dispatch on the flags group instruction class: */
  switch (flags_group->tme_recode_flags_group_insn_class) {
  default: abort();

    /* these instruction classes never chain to the test subs: */
  case TME_RECODE_INSN_CLASS_ADDITIVE:
    subs_test = 0;
    break;

    /* these instruction classes always chain to the test subs: */
  case TME_RECODE_INSN_CLASS_SHIFT:
  case TME_RECODE_INSN_CLASS_EXT:
    subs_test = 1;
    break;

    /* these instruction classes chain to the test subs only if flags
       for multiple sizes are needed: */
  case TME_RECODE_INSN_CLASS_LOGICAL:
    subs_test = multiple_ops;
    break;
  }

  /* if this flags group chains to the test subs: */
  if (subs_test != 0) {
    
    /* if we haven't already created the test subs: */
    _subs
      = &(flags_thunk->tme_recode_x86_flags_thunk_sizes
	  [flags_group->tme_recode_flags_group_insn_size - TME_RECODE_SIZE_8]
	  .tme_recode_x86_flags_thunk_size_subs_test);
    subs_test = *_subs;
    if (subs_test == 0) {

      /* make the test subs, which chains to the main flags subs: */
      subs_test = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);
      assert (subs_test != 0);
      *_subs = subs_test;
      _tme_recode_x86_flags_thunk_ops(flags_thunk,
				      TME_RECODE_X86_OPCODE_BINOP_TEST,
				      TME_RECODE_X86_REG_HOST_SUBS_SRC1,
				      host_sizes,
				      ic);
      _tme_recode_x86_flags_thunk_chain(ic,
					flags_thunk->tme_recode_x86_flags_thunk_subs_main);
    }
  }

  /* loop over all sizes starting from eight bits: */
  for (size_other = TME_RECODE_SIZE_8;
       size_other <= TME_RECODE_SIZE_GUEST_MAX;
       size_other++) {

    /* loop over all integer opcodes: */
    for (opcode = 0;
	 opcode < TME_RECODE_OPCODES_INTEGER;
	 opcode++) {

      /* initialize the machine-independent parts of the recode IC
	 such that guest registers zero and one are loaded in the x86
	 subs destination operand and second source operand host
	 registers, respectively: */
      ic->tme_recode_ic_reg_host_reserve_next = 0;
      ic->tme_recode_ic_reg_host_to_ruses
	[TME_RECODE_X86_REG_HOST_SUBS_DST]
	= TME_RECODE_REGINFO_RUSES(1);
      ic->tme_recode_ic_reg_host_to_ruses
	[TME_RECODE_X86_REG_HOST_SUBS_SRC1]
	= TME_RECODE_REGINFO_RUSES(1);
      ic->tme_recode_ic_reg_host_to_reg_guest
	[TME_RECODE_X86_REG_HOST_SUBS_DST] = TME_RECODE_REG_GUEST(0);
      ic->tme_recode_ic_reg_host_to_reg_guest
	[TME_RECODE_X86_REG_HOST_SUBS_SRC1] = TME_RECODE_REG_GUEST(1);
      (ic->tme_recode_ic_reginfo
       + TME_RECODE_REG_GUEST(0))->tme_recode_reginfo_tags_ruses
	= (TME_RECODE_REGINFO_TAGS_VALID
	   | TME_RECODE_REGINFO_TAGS_VALID_SIZE(TME_RECODE_SIZE_GUEST_MAX)
	   | TME_RECODE_REGINFO_TAGS_CLEAN
	   | TME_RECODE_REGINFO_TAGS_REG_HOST(TME_RECODE_X86_REG_HOST_SUBS_DST));
      (ic->tme_recode_ic_reginfo
       + TME_RECODE_REG_GUEST(1))->tme_recode_reginfo_tags_ruses
	= (TME_RECODE_REGINFO_TAGS_VALID
	   | TME_RECODE_REGINFO_TAGS_VALID_SIZE(TME_RECODE_SIZE_GUEST_MAX)
	   | TME_RECODE_REGINFO_TAGS_CLEAN
	   | TME_RECODE_REGINFO_TAGS_REG_HOST(TME_RECODE_X86_REG_HOST_SUBS_SRC1));

      /* if this isn't a double-host-size guest, initialize the
	 machine-independent parts of the recode ic such that guest
	 register two is loaded in the a register, which can be
	 destroyed by a subs: */
      if (!TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {
#if (TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1) != TME_RECODE_X86_REG_HOST_FREE_CALL
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
	assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1] == TME_RECODE_X86_REG_A);
	ic->tme_recode_ic_reg_host_to_ruses
	  [TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1]
	  = TME_RECODE_REGINFO_RUSES(1);
	ic->tme_recode_ic_reg_host_to_reg_guest
	  [TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1] = TME_RECODE_REG_GUEST(2);
	(ic->tme_recode_ic_reginfo
	 + TME_RECODE_REG_GUEST(2))->tme_recode_reginfo_tags_ruses
	  = (TME_RECODE_REGINFO_TAGS_VALID
	     | TME_RECODE_REGINFO_TAGS_VALID_SIZE(TME_RECODE_SIZE_GUEST_MAX)
	     | TME_RECODE_REGINFO_TAGS_CLEAN
	     | TME_RECODE_REGINFO_TAGS_REG_HOST(TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1));
      }

      /* initialize the instruction, assuming that we'll emit for only
	 one size (double-host-size if this is a double-host-size flags
	 group, otherwise host-size), and assuming that this
	 instruction will operate with guest register zero on the left
	 and guest register one on the right, with the result going
	 back into guest register zero: */
      insn.tme_recode_insn_opcode = opcode;
      insn.tme_recode_insn_size
	= (TME_RECODE_SIZE_HOST
	   + (TME_RECODE_SIZE_IS_DOUBLE_HOST(flags_group->tme_recode_flags_group_insn_size) != 0));
      insn.tme_recode_insn_operand_src[0] = TME_RECODE_REG_GUEST(0);
      insn.tme_recode_insn_operand_src[1] = TME_RECODE_REG_GUEST(1);
      insn.tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(0);
      insn.tme_recode_insn_flags_thunk = NULL;

      /* assume that this opcode, for this flags group
	 instruction size, only has one subs: */
      _subs
	= &(flags_thunk->tme_recode_x86_flags_thunk_sizes
	    [flags_group->tme_recode_flags_group_insn_size - TME_RECODE_SIZE_8]
	    .tme_recode_x86_flags_thunk_size_subs[opcode]);

      /* dispatch on the opcode to get its instruction class and to
	 override any defaults: */
      switch (opcode) {
      default: assert(FALSE);
      case TME_RECODE_OPCODE_ANDN:
      case TME_RECODE_OPCODE_ORN:
      case TME_RECODE_OPCODE_XORN:
      case TME_RECODE_OPCODE_AND:
      case TME_RECODE_OPCODE_OR:
      case TME_RECODE_OPCODE_XOR:
	class = TME_RECODE_INSN_CLASS_LOGICAL;
	break;
      case TME_RECODE_OPCODE_SUB:
      case TME_RECODE_OPCODE_SUBC:
      case TME_RECODE_OPCODE_ADD:
      case TME_RECODE_OPCODE_ADDC:
	class = TME_RECODE_INSN_CLASS_ADDITIVE;
	break;
      case TME_RECODE_OPCODE_SHRL:
      case TME_RECODE_OPCODE_SHLL:
      case TME_RECODE_OPCODE_SHRA:
	class = TME_RECODE_INSN_CLASS_SHIFT;
	break;
      case TME_RECODE_OPCODE_EXTZ:
      case TME_RECODE_OPCODE_EXTS:
	class = TME_RECODE_INSN_CLASS_EXT;
#if (TME_RECODE_OPCODE_EXTS - TME_RECODE_OPCODE_EXTZ) != 1
#error "TME_RECODE_OPCODE_ values changed"
#endif
	_subs
	  = &(flags_thunk->tme_recode_x86_flags_thunk_sizes
	      [flags_group->tme_recode_flags_group_insn_size - TME_RECODE_SIZE_8]
	      .tme_recode_x86_flags_thunk_size_subs_ext
	      [size_other - TME_RECODE_SIZE_8]
	      [opcode - TME_RECODE_OPCODE_EXTZ]);
	insn.tme_recode_insn_imm_uguest = size_other;
	break;
      }

      /* skip this opcode if it's in the wrong class: */
      if (class != flags_group->tme_recode_flags_group_insn_class) {
	continue;
      }

      /* skip the other size if it's not the result size of the
	 instructions in this flags group, unless this is the
	 extension class, in which case we skip other sizes that are
	 greater than or equal to the result size: */
      if (class == TME_RECODE_INSN_CLASS_EXT
	  ? size_other >= flags_group->tme_recode_flags_group_insn_size
	  : size_other != flags_group->tme_recode_flags_group_insn_size) {
	continue;
      }

      /* skip this opcode if we've already created its subs: */
      if (*_subs != 0) {
	continue;
      }

      /* set the thunk offset of this subs: */
      *_subs = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);
      assert (*_subs != 0);

      /* if there is a guest function, or if this class doesn't chain
	 to the test subs and we have do to its instructions at
	 multiple sizes, not counting the double-host-size: */
      if (flags_thunk->tme_recode_x86_flags_thunk_has_guest_func
	  || (subs_test == 0
	      && multiple_ops)) {

	/* start more instructions: */
	tme_recode_x86_insns_start(ic, thunk_bytes);

	/* copy the first source operand, currently in the destination
	   operand host register, to the first source operand host
	   register: */
	if (TME_RECODE_SIZE_IS_DOUBLE_HOST(flags_group->tme_recode_flags_group_insn_size)) {
	  _tme_recode_x86_emit_reg_copy(thunk_bytes,
					tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST + 1],
					tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0 + 1]);
	}
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_DST],
				      tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC0]);

	/* finish these instructions: */
	tme_recode_x86_insns_finish(ic, thunk_bytes);
      }

      /* if this is a logical noncommutative instruction: */
      if ((1 << opcode)
	  & ((1 << TME_RECODE_OPCODE_ANDN)
	     | (1 << TME_RECODE_OPCODE_ORN)
	     | (1 << TME_RECODE_OPCODE_XORN))) {

	/* we have to invert the second source operand and convert
	   this instruction into its commutative version.  for speed,
	   we do at least a 32-bit not: */
	insn_not.tme_recode_insn_opcode = opcode;
	insn_not.tme_recode_insn_size = TME_MAX(insn.tme_recode_insn_size, TME_RECODE_SIZE_32);

	/* NB that the a register can't be allocated to any guest
	   register at this point, since it can be destroyed by a
	   subs.  since the a register is the second of a
	   double-host-size pair, if this is a double-host-size guest,
	   the first register of the pair is also free: */
#if (TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1) < TME_RECODE_X86_REG_HOST_FREE_CALL
#error "TME_RECODE_X86_REG_HOST_ values changed"
#endif
	assert (tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1] == TME_RECODE_X86_REG_A);

	/* if this is a double-host-size instruction: */
	if (TME_RECODE_SIZE_IS_DOUBLE_HOST(flags_group->tme_recode_flags_group_insn_size)) {

	  /* we invert the double-host-size second source operand in
	     place, since both host registers can't be allocated to
	     any guest register: */
	  insn_not.tme_recode_insn_operand_dst = TME_RECODE_X86_REG_HOST_SUBS_SRC1;
	}

	/* otherwise, this is not a double-host-size instruction: */
	else {

	  /* copy the second source operand into the free a register: */
	  tme_recode_x86_insns_start(ic, thunk_bytes);
	  _tme_recode_x86_emit_reg_copy(thunk_bytes,
					tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1],
					tme_recode_x86_reg_from_host[TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1]);
	  tme_recode_x86_insns_finish(ic, thunk_bytes);

	  /* we invert the host-size or smaller second source operand in
	     the a register: */
	  insn_not.tme_recode_insn_operand_dst = TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1;

	  /* rewrite the second source operand to be the guest
	     register we "allocated" to the a register: */
	  insn.tme_recode_insn_operand_src[1] = TME_RECODE_REG_GUEST(2);
	}

	/* invert the second source operand: */
	_tme_recode_x86_insn_negate(ic, &insn_not);

	/* rewrite the opcode to a plain logical: */
	insn.tme_recode_insn_opcode = TME_RECODE_OPCODE_LOGICALN_TO_PLAIN(opcode);
      }

      if (subs_test == 0) {

	/* make the binops subs.  NB that we have to load the second
	   source operand, since this is not necessarily
	   TME_RECODE_X86_REG_HOST_SUBS_SRC1: */
	_tme_recode_x86_flags_thunk_ops(flags_thunk,
					TME_RECODE_X86_OPCODE_TO_BINOP(insn.tme_recode_insn_opcode),
					tme_recode_regs_src_any(ic,
								&insn,
								insn.tme_recode_insn_operand_src[1]),
					host_sizes,
					ic);
      }

      else {

	/* emit an instruction normally: */
	_tme_recode_x86_insn_emit(ic, &insn);
      }

      /* if this is a double-host-size logical noncommutative
	 instruction and there is a guest function: */
      if (((1 << opcode)
	   & ((1 << TME_RECODE_OPCODE_ANDN)
	      | (1 << TME_RECODE_OPCODE_ORN)
	      | (1 << TME_RECODE_OPCODE_XORN)))
	  && TME_RECODE_SIZE_IS_DOUBLE_HOST(flags_group->tme_recode_flags_group_insn_size)
	  && flags_thunk->tme_recode_x86_flags_thunk_has_guest_func) {

	/* we have to undo the inversion of the double-host-size
	   second source operand in place, because we have to pass it
	   to the guest function: */

	/* if there isn't a test subs: */
	if (subs_test == 0) {

	  /* push the double-host-size flags: */
	  tme_recode_x86_insns_start(ic, thunk_bytes);
	  thunk_bytes[0] = TME_RECODE_X86_OPCODE_PUSHF;
	  thunk_bytes++;
	  tme_recode_x86_insns_finish(ic, thunk_bytes);
	}

	/* undo the inversion by redoing it: */
	_tme_recode_x86_insn_negate(ic, &insn_not);

	/* if there isn't a test subs: */
	if (subs_test == 0) {

	  /* pop the double-host-size flags: */
	  tme_recode_x86_insns_start(ic, thunk_bytes);
	  thunk_bytes[0] = TME_RECODE_X86_OPCODE_PUSHF;
	  thunk_bytes++;
	  tme_recode_x86_insns_finish(ic, thunk_bytes);
	}
      }

      /* chain to either the test subs or the main flags subs: */
      _tme_recode_x86_flags_thunk_chain(ic,
					(subs_test != 0
					 ? subs_test
					 : flags_thunk->tme_recode_x86_flags_thunk_subs_main));
    }
  }

  /* return the flags thunk: */
  return (flags_thunk);
}

#ifdef TME_RECODE_DEBUG
#include <stdio.h>

/* this host function dumps a flags thunk: */
void
tme_recode_host_flags_thunk_dump(const struct tme_recode_ic *ic,
				 const struct tme_recode_flags_thunk *flags_thunk)
{
  unsigned int insn_size;
  unsigned int opcode;
  tme_recode_thunk_off_t thunk_off;
  unsigned int insn_size_other;

  printf("  x86 stack padding: %d\n", flags_thunk->tme_recode_x86_flags_thunk_stack_padding);
  printf("  x86 has guest func: %s\n",
	 (flags_thunk->tme_recode_x86_flags_thunk_has_guest_func
	  ? "yes"
	  : "no"));
  printf("  x86 main flags subs: x/15i %p\n",
	 tme_recode_thunk_off_to_pointer(ic,
					 flags_thunk->tme_recode_x86_flags_thunk_subs_main,
					 void (*)(void)));

  for (insn_size = TME_RECODE_SIZE_8;
       insn_size <= TME_RECODE_SIZE_GUEST_MAX;
       insn_size++) {
    for (opcode = 0;
	 opcode < TME_RECODE_OPCODES_INTEGER;
	 opcode++) {
      thunk_off = flags_thunk
	->tme_recode_x86_flags_thunk_sizes[insn_size - TME_RECODE_SIZE_8]
	.tme_recode_x86_flags_thunk_size_subs[opcode];
      if (thunk_off != 0) {
	printf("  x86 %s%u flags subs: x/4i %p\n",
	       tme_recode_opcode_dump(opcode),
	       (1 << insn_size),
	       tme_recode_thunk_off_to_pointer(ic,
					       thunk_off,
					       void (*)(void)));
      }
    }
#if (TME_RECODE_OPCODE_EXTZ + 1) != TME_RECODE_OPCODE_EXTS
#error "TME_RECODE_OPCODE_EXTS or TME_RECODE_OPCODE_EXTZ changed"
#endif
    for (opcode = TME_RECODE_OPCODE_EXTZ;
	 opcode <= TME_RECODE_OPCODE_EXTS;
	 opcode++) {
      for (insn_size_other = TME_RECODE_SIZE_8;
	   insn_size_other < insn_size;
	   insn_size_other++) {
	thunk_off = flags_thunk
	  ->tme_recode_x86_flags_thunk_sizes[insn_size - TME_RECODE_SIZE_8]
	  .tme_recode_x86_flags_thunk_size_subs_ext[insn_size_other - TME_RECODE_SIZE_8]
	  [opcode - TME_RECODE_OPCODE_EXTZ];
	if (thunk_off != 0) {
	  printf("  x86 ext%u%c%u flags subs: x/4i %p\n",
		 (1 << insn_size_other),
		 (opcode == TME_RECODE_OPCODE_EXTZ ? 'z' : 's'),
		 (1 << insn_size),
		 tme_recode_thunk_off_to_pointer(ic,
						 thunk_off,
						 void (*)(void)));
	}
      }
    }
  }
}

#endif /* TME_RECODE_DEBUG */
