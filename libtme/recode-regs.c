/* $Id: recode-regs.c,v 1.5 2010/06/05 19:09:05 fredette Exp $ */

/* libtme/recode-insn.c - generic recode instruction support: */

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
_TME_RCSID("$Id: recode-regs.c,v 1.5 2010/06/05 19:09:05 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* this frees all host registers, starting from the given host
   register: */
void
tme_recode_regs_host_free_many(struct tme_recode_ic *ic,
			       unsigned long reg_host)
{
  int freed_all;
  unsigned long reg_host_spacing;
  tme_uint32_t ruses;
  unsigned long reg_guest;
  tme_uint32_t reginfo_all;
  const tme_uint16_t *ruses_record;
  unsigned long ruses_record_next;

  /* remember if we're freeing all host registers: */
  freed_all = (reg_host == TME_RECODE_REG_HOST(0));

  reg_host_spacing
    = (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
       ? 2
       : 1);

  reg_host &= (0 - reg_host_spacing);

  /* loop over the host registers: */
  do {

    /* if this host register has a guest register: */
    ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host];
    assert (ruses != TME_RECODE_REG_RUSES_RESERVED);
    if (ruses != TME_RECODE_REG_RUSES_FREE) {

      /* get the guest register: */
      reg_guest = ic->tme_recode_ic_reg_host_to_reg_guest[reg_host];

      /* get all of the guest register's information: */
      reginfo_all = ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_all;

      /* the guest register must have valid tags for this host register: */
      assert ((reginfo_all & TME_RECODE_REGINFO_TAGS_VALID)
	      && TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all) == reg_host);

      /* write back the read-uses count for the guest register and
	 invalidate its tags: */
      ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses
	= ruses;

      /* this host register no longer has any guest register: */
      ic->tme_recode_ic_reg_host_to_ruses[reg_host]
	= TME_RECODE_REG_RUSES_FREE;

      /* if the host register is dirty: */
      if (TME_RECODE_REGINFO_TAGS_ARE_DIRTY(reginfo_all)) {

	/* store the host register: */
	tme_recode_host_reg_move(ic,
				 reg_guest,
				 reginfo_all);
      }
    }
  } while ((reg_host += reg_host_spacing) < TME_RECODE_REG_HOST_UNDEF);

  /* if all host registers are now free: */
  if (freed_all) {

    /* get as many initial read-uses records as we can: */
    ruses_record
      = (ic->tme_recode_ic_reg_guest_ruses_records
	 + ic->tme_recode_ic_reg_guest_ruses_record_next);
    for (;;) {

      /* if the next read-uses record isn't an initial read-uses
	 record, stop now: */
      reg_guest = *ruses_record;
      reg_guest -= TME_RECODE_REG_RUSES_RECORD_REG_GUEST(0);
      if (reg_guest
	  >= (TME_RECODE_REG_RUSES_RECORD_UNDEF
	      - TME_RECODE_REG_RUSES_RECORD_REG_GUEST(0))) {
	break;
      }

      /* get what must be the read-uses part of an initial read-uses record: */
      ruses = ruses_record[1];
      assert (ruses >= (TME_RECODE_REG_RUSES_FREE + 1)
	      && ruses < TME_RECODE_REG_RUSES_RESERVED);

      /* set this guest register's read-uses count: */
      ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses = ruses;

      /* advance: */
      ruses_record += 2;
    }

    /* get the index of the read-uses record that we stopped on: */
    ruses_record_next
      = (ruses_record
	 - ic->tme_recode_ic_reg_guest_ruses_records);

    /* the read-uses record that we stopped on is either a write
       read-uses record, a delimiter between groups of initial
       read-uses records, or the terminator at the end of the
       read-uses records.  we want to skip past a delimiter but not
       the terminator, which are both
       TME_RECODE_REG_RUSES_RECORD_UNDEF, so when we see that we have
       to make sure it's not the terminator: */
    if (reg_guest
	== (TME_RECODE_REG_RUSES_RECORD_UNDEF
	    - TME_RECODE_REG_RUSES_RECORD_REG_GUEST(0))) {
      ruses_record_next
	+= (ruses_record_next
	    < ic->tme_recode_ic_reg_guest_ruses_record_count);
    }

    /* set the index of the next read-uses record: */
    ic->tme_recode_ic_reg_guest_ruses_record_next = ruses_record_next;
  }
}

/* this cleans all dirty host registers, freeing those that weren't
   dirty at the full guest register size: */
void
tme_recode_regs_host_clean_all(struct tme_recode_ic *ic)
{
  unsigned long reg_host;
  unsigned long reg_host_spacing;
  tme_uint32_t ruses_tags;
  unsigned long reg_guest;
  tme_uint32_t reginfo_all;

  reg_host = 0;

  reg_host_spacing
    = (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
       ? 2
       : 1);

  /* loop over the host registers: */
  do {

    /* if this host register has a guest register: */
    ruses_tags = ic->tme_recode_ic_reg_host_to_ruses[reg_host];
    assert (ruses_tags != TME_RECODE_REG_RUSES_RESERVED);
    if (ruses_tags != TME_RECODE_REG_RUSES_FREE) {

      /* get the guest register: */
      reg_guest = ic->tme_recode_ic_reg_host_to_reg_guest[reg_host];

      /* get all of the guest register's information: */
      reginfo_all = ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_all;

      /* the guest register must have valid tags for this host register: */
      assert ((reginfo_all & TME_RECODE_REGINFO_TAGS_VALID)
	      && TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all) == reg_host);

      /* if the host register is dirty: */
      if (TME_RECODE_REGINFO_TAGS_ARE_DIRTY(reginfo_all)) {

	/* update the tags for the guest register to mark it as clean,
	   without changing its valid size: */
	ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses
	  = (reginfo_all
	     + TME_RECODE_REGINFO_TAGS_CLEAN);

	/* store the host register: */
	tme_recode_host_reg_move(ic,
				 reg_guest,
				 reginfo_all);
      }
    }
  } while ((reg_host += reg_host_spacing) < TME_RECODE_REG_HOST_UNDEF);
}

/* this returns any best host register with the smallest read-uses
   count less than the given count: */
unsigned long
tme_recode_regs_host_best(const struct tme_recode_ic *ic,
			  tme_uint32_t ruses_best)
{
  unsigned long reg_host_spacing;
  unsigned long reg_host_other;
  unsigned long reg_host;
  tme_uint32_t ruses;

  reg_host_spacing
    = (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
       ? 2
       : 1);

  reg_host_other = (TME_RECODE_REG_HOST_UNDEF - reg_host_spacing) & (0 - reg_host_spacing);
  reg_host = TME_RECODE_REG_HOST_UNDEF;
  do {
    ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host_other];
    if (ruses < ruses_best) {
      reg_host = reg_host_other;
      ruses_best = ruses;
    }
  } while (((long) (reg_host_other -= reg_host_spacing)) >= 0);

  return (reg_host);
}

/* this frees and returns a specific host register: */
unsigned long
tme_recode_regs_host_free_specific(struct tme_recode_ic *ic,
				   unsigned long reg_host)
{
  tme_uint32_t ruses;
  unsigned long reg_guest;
  tme_uint32_t reginfo_all;

  /* if this host register has a guest register: */
  assert (reg_host < TME_RECODE_REG_HOST_UNDEF
	  && (!TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)
	      || (reg_host % 2) == 0));
  ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host];
  assert (ruses != TME_RECODE_REG_RUSES_RESERVED);
  if (ruses != TME_RECODE_REG_RUSES_FREE) {

    /* get the guest register: */
    reg_guest = ic->tme_recode_ic_reg_host_to_reg_guest[reg_host];

    /* get all of the guest register's information: */
    reginfo_all = ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_all;

    /* the guest register must have valid tags for this host register: */
    assert ((reginfo_all & TME_RECODE_REGINFO_TAGS_VALID)
	    && TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all) == reg_host);

    /* write back the read-uses count for the guest register and
       invalidate its tags: */
    ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses
      = ruses;

    /* this host register no longer has any guest register: */
    ic->tme_recode_ic_reg_host_to_ruses[reg_host]
	= TME_RECODE_REG_RUSES_FREE;

    /* if the host register is dirty: */
    if (TME_RECODE_REGINFO_TAGS_ARE_DIRTY(reginfo_all)) {

      /* store the host register: */
      tme_recode_host_reg_move(ic,
			       reg_guest,
			       reginfo_all);
    }
  }

  return (reg_host);
}

/* this frees and returns any host register: */
#define tme_recode_regs_host_free_any(ic) tme_recode_regs_host_free_specific(ic, tme_recode_regs_host_best(ic, TME_RECODE_REG_RUSES_RESERVED))

/* this reserves one host register, and returns the host register: */
unsigned long
tme_recode_regs_host_reserve(struct tme_recode_ic *ic,
			     unsigned long reg_host)
{
  unsigned long reserve_i;
  tme_uint32_t ruses;

  ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host];
  assert (ruses != TME_RECODE_REG_RUSES_RESERVED);

  ic->tme_recode_ic_reg_host_to_ruses[reg_host] = TME_RECODE_REG_RUSES_RESERVED;

  reserve_i = ic->tme_recode_ic_reg_host_reserve_next;
  assert (reserve_i < TME_ARRAY_ELS(ic->tme_recode_ic_reg_host_reserve));
  ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_reg_host = reg_host;
  ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_ruses = ruses;
  ic->tme_recode_ic_reg_host_reserve_next = reserve_i + 1;

  return (reg_host);
}

/* this unreserves all host registers: */
void
tme_recode_regs_host_unreserve_all(struct tme_recode_ic *ic)
{
  unsigned long reserve_i;
  unsigned long reg_host;

  for (reserve_i = ic->tme_recode_ic_reg_host_reserve_next;
       ((long) --reserve_i) >= 0;) {
    reg_host = ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_reg_host;
    ic->tme_recode_ic_reg_host_to_ruses[reg_host]
      = ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_ruses;
  }
  ic->tme_recode_ic_reg_host_reserve_next = 0;
}

/* this loads a source guest register into its host register, and
   reserves and returns the host register: */
static unsigned long
_tme_recode_regs_src_load(struct tme_recode_ic *ic,
			  const struct tme_recode_insn *insn,
			  unsigned long reg_guest)
{
  tme_uint32_t reginfo_all;
  unsigned long reg_host;
  tme_uint32_t ruses;
  unsigned long reserve_i;

  /* the guest register must already have a host register: */
  reginfo_all = ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_all;
  assert (reginfo_all & TME_RECODE_REGINFO_TAGS_VALID);
  
  /* if the host register is not valid at the instruction size: */
  if (__tme_predict_false(!TME_RECODE_REGINFO_TAGS_ARE_VALID_SIZE(reginfo_all,
								  insn->tme_recode_insn_size))) {

    /* if the host register is dirty: */
    if (TME_RECODE_REGINFO_TAGS_ARE_DIRTY(reginfo_all)) {

      /* store the host register: */
      tme_recode_host_reg_move(ic,
			       reg_guest,
			       reginfo_all);
    }

    /* get the host register: */
    reg_host = TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all);

    /* get the read-uses count for the guest register: */
    /* NB: this will be TME_RECODE_REG_RUSES_RESERVED if the host
       register is already reserved: */
    ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host];

    /* clear the valid size and clean information: */
    reginfo_all
      &= ~ (tme_uint32_t) (TME_RECODE_REGINFO_TAGS_VALID_SIZE_MASK
			   + TME_RECODE_REGINFO_TAGS_CLEAN);

    /* if the read-uses count for this guest register is more than one
       (which includes the case when the host register is already
       reserved), load the entire guest register, otherwise only load
       the size that this instruction needs: */
    /* NB: the only way that an already-reserved host register can be
       not valid at the instruction size is if the instruction size
       has changed since the host register was reserved.  this should
       be very rare, making the cost of loading the entire guest
       register in that case also rare: */
    reginfo_all
      += TME_RECODE_REGINFO_TAGS_VALID_SIZE(ruses > (TME_RECODE_REG_RUSES_FREE + 1)
					    ? ic->tme_recode_ic_reg_size
					    : insn->tme_recode_insn_size);

    /* set the clean information: */
    reginfo_all += TME_RECODE_REGINFO_TAGS_CLEAN;

    /* load the guest register: */
    tme_recode_host_reg_move(ic,
			     reg_guest,
			     reginfo_all);

    /* update the tags for the guest register: */
    ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses
      = reginfo_all;
  }

  /* get the host register: */
  reg_host = TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all);

  /* get the read-uses count for the guest register: */
  ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host];

  /* if this host register has already been reserved: */
  if (__tme_predict_false(ruses == TME_RECODE_REG_RUSES_RESERVED)) {

    /* in between two writes of a guest temporary register, the
       guest temporary register may be read at most once.  if a
       guest temporary register has a reserved host register, it
       has been read more than once: */
    assert (!(reginfo_all & TME_RECODE_REGINFO_TYPE_TEMPORARY));

    /* find the reservation for this host register: */
    reserve_i = ic->tme_recode_ic_reg_host_reserve_next;
    do {
      assert (reserve_i > 0);
      reserve_i--;
    } while (ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_reg_host
	     != reg_host);

    /* update the read-uses count for the guest register.  we can't
       let the read-uses count fall to TME_RECODE_REG_USES_FREE, since
       the host register might be dirty: */
    ruses = ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_ruses;
    if (ruses > (TME_RECODE_REG_RUSES_FREE + 1)) {
      ruses = ruses - 1;
    }
    ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_ruses = ruses;

    /* return the host register, which has already been reserved: */
    return (reg_host);
  }

  /* if the guest register is a temporary: */
  if (reginfo_all & TME_RECODE_REGINFO_TYPE_TEMPORARY) {

    /* write back a dummy read-uses count for the guest temporary
       register: */
    ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_tags_ruses
      = (TME_RECODE_REG_RUSES_FREE + 1);

    /* this host register no longer has any guest register: */
    ruses = TME_RECODE_REG_RUSES_FREE;
  }

  /* otherwise, the guest register is not a temporary: */
  else {

    /* update the read-uses count for the guest register.  we can't let
       the read-uses count fall to TME_RECODE_REG_RUSES_FREE, since the
       host register might be dirty: */
    if (ruses > (TME_RECODE_REG_RUSES_FREE + 1)) {
      ruses = ruses - 1;
    }
  }

  /* update the read-uses count for the host register: */
  ic->tme_recode_ic_reg_host_to_ruses[reg_host] = ruses;

  /* reserve and return the host register: */
  return (tme_recode_regs_host_reserve(ic, reg_host));
}

/* this notifies about a source operand before a call to
   tme_recode_regs_src_any() or tme_recode_regs_src_specific().  calls
   to this function aren't needed for correctness, but they may enable
   optimizations.  if called, the next call to
   tme_recode_regs_src_any() or tme_recode_regs_src_specific() *must*
   be for the same source operand: */
void
tme_recode_regs_src_notify(struct tme_recode_ic *ic,
			   const struct tme_recode_insn *insn,
			   signed long operand_src)
{
  tme_uint32_t reginfo_all;

  /* get all of the source operand's register information.  NB that
     the non-register source operands also have entries in
     tme_recode_ic_reginfo[], as fixed registers, with invalid
     tags: */
  reginfo_all = ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_all;

  /* if the source operand is a guest register that has a host
     register: */
  if (reginfo_all & TME_RECODE_REGINFO_TAGS_VALID) {
    assert (operand_src >= TME_RECODE_REG_GUEST(0));

    /* if the source operand is a guest temporary register, and its
       host register is valid at the instruction size: */
    if ((reginfo_all & TME_RECODE_REGINFO_TYPE_TEMPORARY)
	&& TME_RECODE_REGINFO_TAGS_ARE_VALID_SIZE(reginfo_all,
						  insn->tme_recode_insn_size)) {

      /* force the host register to be clean: */
      /* NB: in most cases, the guest temporary register handling in
	 tme_recode_regs_src_specific() and
	 _tme_recode_regs_src_load() is enough to make sure that guest
	 temporary registers aren't unnecessarily stored.  this is
	 usually needed when an insn needs to call
	 tme_recode_regs_host_clean_all() before setting up its
	 registers.  tme_recode_regs_host_clean_all() must store any
	 dirty guest temporary registers: */
      /* NB: this forcing the host register to be clean is why the
	 next call to tme_recode_regs_src_any() or
	 tme_recode_regs_src_specific() *must* be for the same source
	 operand.  if it isn't, this host register might be reused for
	 that other source operand, with the unstored value of the
	 guest temporary register lost as a result: */
      reginfo_all |= TME_RECODE_REGINFO_TAGS_CLEAN;
    }
  }

  /* update the source operand's register information: */
  assert (operand_src >= TME_RECODE_REG_GUEST(0)
	  || reginfo_all == ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_all);
  ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_all = reginfo_all;
}

/* this loads a source operand into any host register, and reserves
   and returns the host register: */
unsigned long
tme_recode_regs_src_any(struct tme_recode_ic *ic,
			const struct tme_recode_insn *insn,
			signed long operand_src)
{
  tme_uint32_t reginfo_all;
  unsigned long reg_host;

  /* get all of the source operand's register information.  NB that
     the non-register source operands also have entries in
     tme_recode_ic_reginfo[], as fixed registers, with invalid
     tags: */
  reginfo_all = ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_all;

  /* assume that this source operand has a host register: */
  reg_host = TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all);

  /* if this source operand does not have a host register: */
  if (!(reginfo_all & TME_RECODE_REGINFO_TAGS_VALID)) {

    /* free any host register: */
    reg_host = tme_recode_regs_host_free_any(ic);
  }

  /* if this source operand is a guest register: */
  if (operand_src >= TME_RECODE_REG_GUEST(0)) {

    /* if the guest register did not have a host register: */
    if (!(ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_all & TME_RECODE_REGINFO_TAGS_VALID)) {

      /* this host register now has this guest register: */
      ic->tme_recode_ic_reg_host_to_reg_guest[reg_host] = operand_src;
      ic->tme_recode_ic_reg_host_to_ruses[reg_host]
	= ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_tags_ruses;

      /* this guest register now has this host register: */
      ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_tags_ruses
	= (TME_RECODE_REGINFO_TAGS_VALID
	   + TME_RECODE_REGINFO_TAGS_REG_HOST(reg_host));
    }

    /* load this guest register into the host register, and reserve
       and return the host register: */
    return (_tme_recode_regs_src_load(ic,
				      insn,
				      operand_src));
  }

  /* if this source operand is an immediate: */
  if (operand_src == TME_RECODE_OPERAND_IMM) {

    /* fill this host register with the immediate, and reserve and
       return the host register: */
    return (tme_recode_host_reg_imm(ic, insn, reg_host));
  }

  /* this source operand must be a zero: */
  assert (operand_src == TME_RECODE_OPERAND_ZERO);

  /* zero this host register, and reserve and return the host register: */
  return (tme_recode_host_reg_zero(ic, insn, reg_host));
}

/* this loads a source operand into a specific host register, and
   reserves and returns that host register: */
unsigned long
tme_recode_regs_src_specific(struct tme_recode_ic *ic,
			     const struct tme_recode_insn *insn,
			     signed long operand_src,
			     unsigned long reg_host_specific)
{
  tme_uint32_t ruses;
  tme_uint32_t reginfo_all;
  unsigned long reg_host;

  /* if this specific host register has a guest register: */
  ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host_specific];
  assert (ruses != TME_RECODE_REG_RUSES_RESERVED);
  if (__tme_predict_true(ruses != TME_RECODE_REG_RUSES_FREE)) {

    /* if that guest register is the source operand: */
    if (ic->tme_recode_ic_reg_host_to_reg_guest[reg_host_specific] == operand_src) {

      /* reload this guest register into the host register if needed,
	 and reserve and return the host register: */
      return (_tme_recode_regs_src_load(ic,
					insn,
					operand_src));
    }

    /* free the specific host register: */
    tme_recode_regs_host_free_specific(ic, reg_host_specific);
  }

  /* if this source operand is a guest register: */
  if (operand_src >= 0) {

    /* get all of the guest register's information: */
    reginfo_all = ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_all;

    /* if the guest register has a host register: */
    if (reginfo_all & TME_RECODE_REGINFO_TAGS_VALID) {

      /* get the other host register: */
      reg_host = TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all);

      /* the other host register can't be the specific host register,
	 because we were supposed to catch that above: */
      assert(reg_host != reg_host_specific);

      /* if the other host register is valid at the instruction
	 size: */
      if (__tme_predict_true(TME_RECODE_REGINFO_TAGS_ARE_VALID_SIZE(reginfo_all,
								    insn->tme_recode_insn_size))) {

	/* if the guest register is a temporary: */
	if (reginfo_all & TME_RECODE_REGINFO_TYPE_TEMPORARY) {

	  /* in between two writes of a guest temporary register, the
	     guest temporary register may be read at most once.  if a
	     guest temporary register has a reserved host register, it
	     has been read more than once: */
	  assert (ic->tme_recode_ic_reg_host_to_ruses[reg_host] != TME_RECODE_REG_RUSES_RESERVED);

	  /* this host register no longer has any guest register: */
	  ic->tme_recode_ic_reg_host_to_ruses[reg_host] = TME_RECODE_REG_RUSES_FREE;

	  /* write back a dummy read-uses count for the guest
	     temporary register: */
	  ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_tags_ruses
	    = (TME_RECODE_REG_RUSES_FREE + 1);
	}
	
	/* copy the other host register into the specific host register: */
	tme_recode_host_reg_copy(ic,
				 reg_host,
				 reg_host_specific);

	/* reserve and return the specific host register: */
	return (tme_recode_regs_host_reserve(ic, reg_host_specific));
      }

      /* free the other host register: */
      tme_recode_regs_host_free_specific(ic, reg_host);

      /* get all of the guest register's information again: */
      reginfo_all = ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_all;
    }

    /* the specific host register now has this guest register: */
    ic->tme_recode_ic_reg_host_to_reg_guest[reg_host_specific] = operand_src;
    ic->tme_recode_ic_reg_host_to_ruses[reg_host_specific]
      = TME_RECODE_REGINFO_RUSES(reginfo_all);

    /* this guest register now has this specific host register: */
    ic->tme_recode_ic_reginfo[operand_src].tme_recode_reginfo_tags_ruses
      = (TME_RECODE_REGINFO_TAGS_VALID
	 + TME_RECODE_REGINFO_TAGS_REG_HOST(reg_host_specific));

    /* load this guest register into the specific host register, and
       reserve and return the specific host register: */
    return (_tme_recode_regs_src_load(ic,
				      insn,
				      operand_src));
  }

  /* if this source operand is an immediate: */
  if (operand_src == TME_RECODE_OPERAND_IMM) {

    /* fill this host register with the immediate, and reserve and
       return the host register: */
    return (tme_recode_host_reg_imm(ic, insn, reg_host_specific));
  }

  /* this source operand must be a zero: */
  assert (operand_src == TME_RECODE_OPERAND_ZERO);

  /* zero this host register, and reserve and return the host register: */
  return (tme_recode_host_reg_zero(ic, insn, reg_host_specific));
}

/* this allocates any host register for the destination operand, and
   returns the host register: */
unsigned long
tme_recode_regs_dst_any(struct tme_recode_ic *ic,
			const struct tme_recode_insn *insn)
{
  signed long operand_dst;
  tme_uint32_t reginfo_all;
  unsigned long reg_host;
  unsigned long ruses_record_next;
  tme_uint32_t ruses;

  /* get the destination operand: */
  operand_dst = insn->tme_recode_insn_operand_dst;

  /* get all of the destination operand's register information.  NB
     that the non-register destination operands also have entries in
     tme_recode_ic_reginfo[], as fixed registers, with invalid
     tags: */
  reginfo_all = ic->tme_recode_ic_reginfo[operand_dst].tme_recode_reginfo_all;

  /* assume that this destination operand has a host register: */
  reg_host = TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all);

  /* if this destination operand is fixed or doesn't have a host
     register: */
  if ((reginfo_all
       & (TME_RECODE_REGINFO_TYPE_FIXED
	  + TME_RECODE_REGINFO_TAGS_VALID))
      != (!TME_RECODE_REGINFO_TYPE_FIXED
	  + TME_RECODE_REGINFO_TAGS_VALID)) {

    /* free any host register: */
    reg_host = tme_recode_regs_host_free_any(ic);
  }

  /* unreserve all registers: */
  tme_recode_regs_host_unreserve_all(ic);

  /* if the destination operand is not fixed: */
  if ((reginfo_all & TME_RECODE_REGINFO_TYPE_FIXED) == 0) {

    /* if the guest register has a host register: */
    if (reginfo_all & TME_RECODE_REGINFO_TAGS_VALID) {

      /* if the guest register has a dirty size greater than this
	 instruction's size: */
      if (TME_RECODE_REGINFO_TAGS_WHICH_DIRTY_SIZE(reginfo_all)
	  > insn->tme_recode_insn_size) {

	/* store the guest register: */
	tme_recode_host_reg_move(ic,
				 operand_dst,
				 reginfo_all);
      }
    }

    /* otherwise, the guest register did not have a host register: */
    else {

      /* this host register now has this guest register: */
      ic->tme_recode_ic_reg_host_to_reg_guest[reg_host] = operand_dst;
    }

    /* this guest register now has this host register, and is dirty at
       the instruction size: */
    ic->tme_recode_ic_reginfo[operand_dst].tme_recode_reginfo_tags_ruses
      = (TME_RECODE_REGINFO_TAGS_VALID
	 + TME_RECODE_REGINFO_TAGS_VALID_SIZE(insn->tme_recode_insn_size)
	 + !TME_RECODE_REGINFO_TAGS_CLEAN
	 + TME_RECODE_REGINFO_TAGS_REG_HOST(reg_host));

    /* get what should be a write read-uses record: */
    ruses_record_next = ic->tme_recode_ic_reg_guest_ruses_record_next;
    ruses = ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_next];
    ic->tme_recode_ic_reg_guest_ruses_record_next = ruses_record_next + 1;

    /* if this wasn't a write read-uses record: */
    if (__tme_predict_false(ruses >= TME_RECODE_REG_RUSES_RESERVED)) {

      /* this only happens when we ran out of space for read-uses
	 records.  back up the read-uses record index, and make up a
	 read-uses count.  this doesn't affect correctness, only
	 performance: */
      ic->tme_recode_ic_reg_guest_ruses_record_next--;
      ruses = (TME_RECODE_REG_RUSES_FREE + 1) + 2;
    }

    /* set the new read-uses count for this guest register: */
    ic->tme_recode_ic_reg_host_to_ruses[reg_host] = ruses;
  }

  /* return this host register: */
  return (reg_host);
}

/* this allocates a specific host register for the destination operand
   and returns the host register: */
unsigned long
tme_recode_regs_dst_specific(struct tme_recode_ic *ic,
			     const struct tme_recode_insn *insn,
			     unsigned long reserve_i)
{
  signed long operand_dst;
  tme_uint32_t reginfo_all;
  unsigned long reg_host;
  tme_uint32_t ruses_tags;
  tme_uint32_t ruses;
  unsigned long reg_host_specific;
  unsigned long reg_guest;
  unsigned long ruses_record_next;

  /* get the destination operand: */
  operand_dst = insn->tme_recode_insn_operand_dst;

  /* get all of the destination operand's register information.  NB
     that the non-register destination operands also have entries in
     tme_recode_ic_reginfo[], as fixed registers, with invalid
     tags: */
  reginfo_all = ic->tme_recode_ic_reginfo[operand_dst].tme_recode_reginfo_all;

  /* if this destination operand has a host register: */
  if (__tme_predict_true(reginfo_all & TME_RECODE_REGINFO_TAGS_VALID)) {

    /* get the host register: */
    reg_host = TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all);

    /* assume that this host register is reserved, and that we will
       rewrite the tags for this guest register so that it's neither
       dirty nor clean: */
    ruses_tags = TME_RECODE_REGINFO_TAGS_VALID + reg_host;

    /* if this host register is not reserved: */
    ruses = ic->tme_recode_ic_reg_host_to_ruses[reg_host];
    assert (ruses != TME_RECODE_REG_RUSES_FREE);
    if (ruses != TME_RECODE_REG_RUSES_RESERVED) {

      /* we will rewrite the tags for this guest register so that it
	 doesn't have a host register: */
      ruses_tags = ruses;

      /* this host register is free: */
      ic->tme_recode_ic_reg_host_to_ruses[reg_host] = TME_RECODE_REG_RUSES_FREE;
    }

    /* rewrite the tags for this guest register: */
    ic->tme_recode_ic_reginfo[operand_dst].tme_recode_reginfo_tags_ruses = ruses_tags;

    /* if the guest register has a dirty size greater than this
       instruction's size: */
    if (TME_RECODE_REGINFO_TAGS_WHICH_DIRTY_SIZE(reginfo_all)
	> insn->tme_recode_insn_size) {

      /* store the guest register: */
      tme_recode_host_reg_move(ic,
			       operand_dst,
			       reginfo_all);
    }
  }

  /* if this specific host register has a guest register that may
     still be read later: */
  assert (reserve_i < ic->tme_recode_ic_reg_host_reserve_next);
  ruses = ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_ruses;
  assert (ruses < TME_RECODE_REG_RUSES_RESERVED);
  if (ruses > (TME_RECODE_REG_RUSES_FREE + 1)) {

    /* get the specific host register: */
    reg_host_specific = ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_reg_host;

    /* if that guest register is not the destination operand: */
    if (ic->tme_recode_ic_reg_host_to_reg_guest[reg_host_specific] != operand_dst) {

      /* if there is any other host register that we can free up and
	 copy the specific host register into: */
      reg_host = tme_recode_regs_host_best(ic, ruses);
      if (reg_host != TME_RECODE_REG_HOST_UNDEF) {

	/* free this other host register: */
	reg_host = tme_recode_regs_host_free_specific(ic, reg_host);

	/* get the guest register we are copying: */
	reg_guest = ic->tme_recode_ic_reg_host_to_reg_guest[reg_host_specific];

	/* disconnect the guest register we are copying from the
	   specific host register, and connect it to this other host
	   register: */
	ic->tme_recode_ic_reginfo[reg_guest].tme_recode_reginfo_all
	  += (TME_RECODE_REGINFO_TAGS_REG_HOST(reg_host)
	      - TME_RECODE_REGINFO_TAGS_REG_HOST(reg_host_specific));
	ic->tme_recode_ic_reg_host_to_reg_guest[reg_host] = reg_guest;
	ic->tme_recode_ic_reg_host_to_ruses[reg_host]
	  = ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_ruses;

	/* arrange for the specific host register to be free after we
	   unreserve all registers: */
	ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_ruses = TME_RECODE_REG_RUSES_FREE;

	/* copy the specific host register into this other host register: */
	tme_recode_host_reg_copy(ic,
				 reg_host_specific,
				 reg_host);
      }
    }
  }

  /* unreserve all registers: */
  tme_recode_regs_host_unreserve_all(ic);

  /* get the specific host register: */
  reg_host_specific = ic->tme_recode_ic_reg_host_reserve[reserve_i].tme_reg_host_reserve_reg_host;

  /* free the specific host register: */
  reg_host_specific = tme_recode_regs_host_free_specific(ic, reg_host_specific);

  /* get the destination operand: */
  operand_dst = insn->tme_recode_insn_operand_dst;

  /* get all of the destination operand's register information.  NB
     that the non-register destination operands also have entries in
     tme_recode_ic_reginfo[], as fixed registers, with invalid
     tags: */
  reginfo_all = ic->tme_recode_ic_reginfo[operand_dst].tme_recode_reginfo_all;

  /* if the destination operand is not fixed: */
  if (__tme_predict_true((reginfo_all & TME_RECODE_REGINFO_TYPE_FIXED) == 0)) {

    /* if the guest register had a host register: */
    if (reginfo_all & TME_RECODE_REGINFO_TAGS_VALID) {

      /* get the host register: */
      reg_host = TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(reginfo_all);

      /* this guest register can't have the specific host register,
	 since we freed the specific host register above: */
      assert (reg_host != reg_host_specific);

      /* free this other host register: */
      ic->tme_recode_ic_reg_host_to_ruses[reg_host] = TME_RECODE_REG_RUSES_FREE;
    }

    /* get what should be a write read-uses record: */
    ruses_record_next = ic->tme_recode_ic_reg_guest_ruses_record_next;
    ruses = ic->tme_recode_ic_reg_guest_ruses_records[ruses_record_next];
    ic->tme_recode_ic_reg_guest_ruses_record_next = ruses_record_next + 1;

    /* if this wasn't a write read-uses record: */
    if (__tme_predict_false(ruses >= TME_RECODE_REG_RUSES_RESERVED)) {

      /* this only happens when we ran out of space for read-uses
	 records.  back up the read-uses record index, and make up a
	 read-uses count.  this doesn't affect correctness, only
	 performance: */
      ic->tme_recode_ic_reg_guest_ruses_record_next--;
      ruses = (TME_RECODE_REG_RUSES_FREE + 1) + 2;
    }

    ic->tme_recode_ic_reg_host_to_reg_guest[reg_host_specific] = operand_dst;
    ic->tme_recode_ic_reg_host_to_ruses[reg_host_specific] = ruses;

    /* this guest register now has this host register, and is dirty at
       the instruction size: */
    ic->tme_recode_ic_reginfo[operand_dst].tme_recode_reginfo_tags_ruses
      = (TME_RECODE_REGINFO_TAGS_VALID
	 + TME_RECODE_REGINFO_TAGS_VALID_SIZE(insn->tme_recode_insn_size)
	 + !TME_RECODE_REGINFO_TAGS_CLEAN
	 + TME_RECODE_REGINFO_TAGS_REG_HOST(reg_host_specific));
  }

  /* return this host register: */
  return (reg_host_specific);
}

#endif /* TME_HAVE_RECODE */
