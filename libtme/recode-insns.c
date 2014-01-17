/* $Id: recode-insns.c,v 1.4 2010/02/07 17:32:01 fredette Exp $ */

/* libtme/recode-insns.c - generic recode instruction support: */

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
_TME_RCSID("$Id: recode-insns.c,v 1.4 2010/02/07 17:32:01 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* macros: */

/* the undefined flags offset: */
#define TME_RECODE_FLAGS_OFFSET_UNDEF	(0 - (tme_uint32_t) 1)

/* this returns the thunk offset for a new instructions thunk.  it
   returns less than zero when thunks memory is exhausted and all
   instructions thunks are flushed: */
tme_recode_thunk_off_t
tme_recode_insns_thunk(struct tme_recode_ic *ic,
		       const struct tme_recode_insns_group *insns_group)
{
  signed long reg_guest;
  signed long ruses_record_tmp;
  signed long ruses_record_right;
  tme_uint32_t flags_offset;
  tme_recode_uguest_t flags_needed;
  tme_uint32_t flags_offset_else;
  tme_recode_uguest_t flags_needed_else;
  struct tme_recode_insn *insns;
  struct tme_recode_insn *insn;
  tme_uint32_t opcode_mask;
  signed long operand_i;
  tme_uint32_t ruses;
  const struct tme_recode_flags_thunk *flags_thunk;
  const struct tme_recode_conds_thunk *conds_thunk;

  /* initialize the mapping from host register to read-uses count for
     a cached guest register, to all host registers free: */
#if TME_RECODE_REG_RUSES_FREE != 0
#error "TME_RECODE_REG_RUSES_FREE changed"
#endif
  memset(ic->tme_recode_ic_reg_host_to_ruses,
	 TME_RECODE_REG_RUSES_FREE, 
	 sizeof(ic->tme_recode_ic_reg_host_to_ruses));

  /* there are no reserved registers: */
  ic->tme_recode_ic_reg_host_reserve_next = 0;

  /* initialize the read-uses counts for all guest registers.  this
     also marks the guest register tags as invalid: */
  reg_guest = TME_RECODE_REG_GUEST(ic->tme_recode_ic_reg_count - 1);
  do {
    ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses = (TME_RECODE_REG_RUSES_FREE + 1);
  } while (--reg_guest >= TME_RECODE_REG_GUEST(0));

  /* the largest guest register number must fit in
     [TME_RECODE_REG_RUSES_RECORD_REG_GUEST(0)..TME_RECODE_REG_RUSES_RECORD_UNDEF): */
  assert (ic->tme_recode_ic_reg_count
	  < (TME_RECODE_REG_RUSES_RECORD_UNDEF
	     - TME_RECODE_REG_RUSES_RECORD_REG_GUEST(0)));

  /* reset the read-uses records: */
  ruses_record_tmp = 0;
  ruses_record_right = ic->tme_recode_ic_reg_guest_ruses_record_count;
  assert (ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_right]
	  == TME_RECODE_REG_RUSES_RECORD_UNDEF);

  /* we haven't found any flags register yet: */
  flags_offset = TME_RECODE_FLAGS_OFFSET_UNDEF;
  flags_needed = 0;
  flags_offset_else = TME_RECODE_FLAGS_OFFSET_UNDEF;
  flags_needed_else = 0;

  /* loop over the instructions, from last to first: */
  insns = insns_group->tme_recode_insns_group_insns;
  insn = insns_group->tme_recode_insns_group_insns_end;
  do {
    insn--;

    /* get the bitmask for this instruction's opcode: */
    opcode_mask = (1 << insn->tme_recode_insn_opcode);

    /* if this is an else instruction, or an endif instruction, or a
       guest instruction with unknown destination registers: */
    if ((opcode_mask
	 & ((1 << TME_RECODE_OPCODE_ELSE)
	    | (1 << TME_RECODE_OPCODE_ENDIF)))
	|| ((opcode_mask & (1 << TME_RECODE_OPCODE_GUEST))
	    && insn->tme_recode_insn_operand_dst == TME_RECODE_OPERAND_NULL)) {

      /* if there are no guest register writes between this else,
	 endif, or guest instruction and the next else, endif, or
	 guest instruction: */
      if (ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_right]
	  >= TME_RECODE_REG_RUSES_RECORD_REG_GUEST(0)) {

	/* if we can, make a delimiter between any initial read-uses
	   records that we're about to make, and the initial read-uses
	   records that we previously made: */
	if (ruses_record_right > 0) {
	  ruses_record_right--;
	  ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_right]
	    = TME_RECODE_REG_RUSES_RECORD_UNDEF;
	}
      }

      /* loop over any temporary read-uses records: */
      for (; ruses_record_tmp > 0; ) {

	/* if this temporary read-uses record was overwritten by a
	   write read-uses record: */
	ruses_record_tmp--;
	if (ruses_record_tmp >= ruses_record_right) {

	  /* skip to the last temporary read-uses record that hasn't
	     been overwritten yet: */
	  ruses_record_tmp = ruses_record_right;
	  continue;
	}

	/* get the guest register: */
	reg_guest = ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_tmp];

	/* get this guest register's read-uses count: */
	ruses = ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses;

	/* if this guest register has a read use before any first
	   write after this else, endif, or guest instruction: */
	if (ruses > (TME_RECODE_REG_RUSES_FREE + 1)) {

	  /* reset this guest register's read-uses count: */
	  ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses = (TME_RECODE_REG_RUSES_FREE + 1);

	  /* if we can't make another initial read-uses record, stop now: */
	  if (ruses_record_right < 2) {
	    break;
	  }

	  /* make the initial read-uses record: */
	  ruses_record_right -= 2;
	  ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_right + 0]
	    = TME_RECODE_REG_RUSES_RECORD_REG_GUEST(reg_guest);
	  ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_right + 1] = ruses;
	}
      }
    }

    /* if this is an integer, or guest, or read/write instruction: */
    if (opcode_mask
	& (((1 << TME_RECODE_OPCODES_INTEGER) - 1)
	   | (1 << TME_RECODE_OPCODE_GUEST)
	   | (1 << TME_RECODE_OPCODE_RW))) {

      /* if the destination operand is a guest register: */
      reg_guest = insn->tme_recode_insn_operand_dst;
      if (reg_guest >= TME_RECODE_REG_GUEST(0)) {

	/* if this guest register is not fixed: */
	if ((ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_all
	     & TME_RECODE_REGINFO_TYPE_FIXED) == 0) {

	  /* get and reset this guest register's read-uses count: */
	  ruses = ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses;
	  ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses = (TME_RECODE_REG_RUSES_FREE + 1);

	  /* make a write read-uses record: */
	  if (ruses_record_right > 0) {
	    ruses_record_right--;
	  }
	  ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_right] = ruses;
	}
      }

      /* all integer and guest instructions can have a guest register
	 as the first source operand.  all of those instructions can
	 also have a guest register as the second source operand,
	 except for the zero- and sign-extension instructions, which
	 always have a TME_RECODE_SIZE for their second source
	 operand: */
      operand_i
	= ((opcode_mask
	    & ((1 << TME_RECODE_OPCODE_EXTZ)
	       | (1 << TME_RECODE_OPCODE_EXTS)))
	   == 0);

      /* loop over the source operands that can be guest registers: */
      do {

	/* if this source operand is a guest register: */
	reg_guest = insn->tme_recode_insn_operand_src[operand_i];
	if (reg_guest >= TME_RECODE_REG_GUEST(0)) {

	  /* get this guest register's current read-uses count: */
	  ruses = ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses;

	  /* if this guest register's current read-uses count is the
	     minimum: */
	  if (ruses == (TME_RECODE_REG_RUSES_FREE + 1)) {

	    /* if we can, make a temporary read-uses record for this
	       guest register: */
	    if (ruses_record_tmp < ruses_record_right) {
	      ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_tmp] = reg_guest;
	      ruses_record_tmp++;
	    }
	  }

	  /* increment the guest register's current read-uses count,
	     unless it would become TME_RECODE_REG_RUSES_RESERVED: */
	  if (__tme_predict_true(ruses < (TME_RECODE_REG_RUSES_RESERVED - 1))) {
	    ruses++;
	  }
	  ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses = ruses;
	}
      } while (--operand_i >= 0);
    }

    /* if this is an integer instruction: */
    if (opcode_mask & ((1 << TME_RECODE_OPCODES_INTEGER) - 1)) {

      /* if the second source operand is a zero, and the operation is
	 commutative: */
      if (insn->tme_recode_insn_operand_src[1] == TME_RECODE_OPERAND_ZERO) {
	if (opcode_mask
	    & ((1 << TME_RECODE_OPCODE_AND)
	       | (1 << TME_RECODE_OPCODE_OR)
	       | (1 << TME_RECODE_OPCODE_XOR)
	       | (1 << TME_RECODE_OPCODE_ADD)
	       | (1 << TME_RECODE_OPCODE_ADDC))) {

	  /* we have the convention of always putting a zero source
	     operand first whenever possible, so swap the first and
	     second source operands: */
	  insn->tme_recode_insn_operand_src[1] = insn->tme_recode_insn_operand_src[0];
	  insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
	}
      }

      /* if this integer instruction can change flags: */
      flags_thunk = insn->tme_recode_insn_flags_thunk;
      if (flags_thunk != NULL) {

	/* if this integer instruction changes flags in a different
	   flags register: */
	if (__tme_predict_false(flags_offset != flags_thunk->tme_recode_flags_thunk_flags_offset)) {

	  /* switch to this different flags register, and need all of
	     its flags: */
	  flags_offset = flags_thunk->tme_recode_flags_thunk_flags_offset;
	  flags_needed = 0 - (tme_recode_uguest_t) 1;
	}

	/* if this integer instruction doesn't define any of the flags
	   in this flags register needed by later instructions: */
	if ((flags_needed & flags_thunk->tme_recode_flags_thunk_flags_defined) == 0) {

	  /* this integer instruction doesn't need to define flags any
	     more: */
	  insn->tme_recode_insn_flags_thunk = NULL;
	}

	/* any earlier instruction that also defines flags in this
	   flags register, doesn't need to define any of the flags
	   that this instruction changes: */
	flags_needed &= ~flags_thunk->tme_recode_flags_thunk_flags_changed;
      }
    }

    /* if this is a defc instruction: */
    else if (opcode_mask == (1 << TME_RECODE_OPCODE_DEFC)) {

      /* get the conditions thunk for this instruction: */
      conds_thunk = insn->tme_recode_insn_conds_thunk;

      /* if this defc instruction tests flags in a different flags register: */
      if (__tme_predict_false(flags_offset != conds_thunk->tme_recode_conds_thunk_flags_offset)) {

	/* switch to this different flags register, and need all of
	   its flags: */
	flags_offset = conds_thunk->tme_recode_conds_thunk_flags_offset;
	flags_needed = 0 - (tme_recode_uguest_t) 1;
      }

      /* any earlier instruction that can define flags tested by this
	 defc instruction, needs to define those flags: */
      flags_needed |= conds_thunk->tme_recode_conds_thunk_flags;
    }

    /* if this is an endif instruction: */
    else if (opcode_mask == (1 << TME_RECODE_OPCODE_ENDIF)) {

      /* if we find an earlier else, as an optimization we can restore
	 the flags needed by the instructions after the endif, so save
	 them now: */
      flags_offset_else = flags_offset;
      flags_needed_else = flags_needed;
    }

    /* if this is an else instruction: */
    else if (opcode_mask == (1 << TME_RECODE_OPCODE_ELSE)) {

      /* restore the flags needed by the instructions after the endif: */
      flags_offset = flags_offset_else;
      flags_needed = flags_needed_else;
    }

    /* otherwise, this must be a guest, read/write, or if instruction: */
    else {
      assert (opcode_mask
	      & ((1 << TME_RECODE_OPCODE_GUEST)
		 | (1 << TME_RECODE_OPCODE_RW)
		 | (1 << TME_RECODE_OPCODE_IF)
		 ));

      /* since a guest function may fault and never return, we have to
	 make sure that all flags are correct in the guest ic state at
	 the time of the fault.

	 since a read/write instruction may fault and never return, we
	 have to make sure that all flags are correct in the guest ic
	 state at the time of the fault.

	 since an if body may not run, we have to make sure that all
	 flags are correct in the guest ic state at the time of the
	 if.

	 since a jump may never return, we have to make sure that all
	 flags are correct in the guest ic state at the time of the
	 jump.

	 in all of these cases, we need any earlier instruction that
	 can define flags to do so: */
      flags_needed = 0 - (tme_recode_uguest_t) 1;
    }

  } while (insn > insns);

  /* set the next read-uses record: */
  ic->tme_recode_ic_reg_guest_ruses_record_next = ruses_record_right;

  /* build the new instructions thunk: */
  return (tme_recode_host_insns_thunk_new(ic,
					  insns_group));
}

/* this invalidates all instructions thunks: */
void
tme_recode_insns_thunk_invalidate_all(struct tme_recode_ic *ic)
{

  /* invalidate all instructions thunks: */
  tme_recode_host_thunk_invalidate_all(ic, ic->tme_recode_ic_thunk_off_variable);
}

#endif /* TME_HAVE_RECODE */
