/* $Id: recode-conds.c,v 1.2 2008/07/01 01:39:01 fredette Exp $ */

/* libtme/recode-conds.c - generic support for recode conditions: */

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
_TME_RCSID("$Id: recode-conds.c,v 1.2 2008/07/01 01:39:01 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* this returns a conditions thunk for a conditions group: */
const struct tme_recode_conds_thunk *
tme_recode_conds_thunk(struct tme_recode_ic *ic,
		       const struct tme_recode_conds_group *conds_group_template)
{
  const struct tme_recode_conds_group *conds_group_other;
  struct tme_recode_conds_group *conds_group;

  /* loop over the existing conditions groups: */
  for (conds_group_other = ic->tme_recode_ic_conds_groups;
       conds_group_other != NULL;
       conds_group_other = conds_group_other->tme_recode_conds_group_next) {

    /* skip this existing conditions group if its flags register
       size or index or flags don't match: */
    if (conds_group_other->tme_recode_conds_group_flags_reg_size
	!= conds_group_template->tme_recode_conds_group_flags_reg_size) {
      continue;
    }
    if (conds_group_other->tme_recode_conds_group_flags_reg
	!= conds_group_template->tme_recode_conds_group_flags_reg) {
      continue;
    }
    if (conds_group_other->tme_recode_conds_group_flags
	!= conds_group_template->tme_recode_conds_group_flags) {
      continue;
    }

    /* skip this existing conditions group if its conditions count or
       guest function don't match: */
    if (conds_group_other->tme_recode_conds_group_cond_count
	!= conds_group_template->tme_recode_conds_group_cond_count) {
      continue;
    }
    if (conds_group_other->tme_recode_conds_group_guest_func
	!= conds_group_template->tme_recode_conds_group_guest_func) {
      continue;
    }

    /* return the conditions thunk from the existing conditions
       group: */
    return (conds_group_other->tme_recode_conds_group_thunk);
  }

  /* allocate and fill the new conditions group: */
  conds_group = tme_new0(struct tme_recode_conds_group, 1);
  *conds_group = *conds_group_template;

  /* build the new conditions thunk: */
  conds_group->tme_recode_conds_group_thunk = tme_recode_host_conds_thunk_new(ic, conds_group);

  /* add this new conditions group to the ic: */
  conds_group->tme_recode_conds_group_next = ic->tme_recode_ic_conds_groups;
  ic->tme_recode_ic_conds_groups = conds_group;

  /* update the initial thunk offset of the first variable thunk: */
  ic->tme_recode_ic_thunk_off_variable
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

  /* return the conditions thunk: */
  return (conds_group->tme_recode_conds_group_thunk);
}

/* this generic function returns the maximum index for a conditions
   group's flags: */
tme_uint32_t
tme_recode_conds_group_flags_index_max(const struct tme_recode_conds_group *conds_group)
{
  tme_recode_uguest_t conds_group_flags_mask;
  unsigned int count_bits;

  /* count the number of bits in the condition group flags mask: */
  conds_group_flags_mask = conds_group->tme_recode_conds_group_flags;
  assert (conds_group_flags_mask != 0);
  for (count_bits = 0;
       conds_group_flags_mask != 0;
       count_bits++) {
    conds_group_flags_mask &= (conds_group_flags_mask - 1);
  }

  /* return the maximum condition group flags index: */
  assert (count_bits <= (sizeof(tme_uint32_t) * 8));
  return ((((tme_uint32_t) 2) << (count_bits - 1)) - 1);
}

/* this generic function returns the indexed combination of a
   conditions group's flags: */
tme_recode_uguest_t
tme_recode_conds_group_flags_from_index(const struct tme_recode_conds_group *conds_group,
				       tme_uint32_t conds_group_flags_index)
{
  tme_recode_uguest_t conds_group_flags_mask;
  tme_recode_uguest_t conds_group_flags;
  tme_recode_uguest_t conds_group_flag_next;

  /* make this combination of the flags: */
  conds_group_flags_mask = conds_group->tme_recode_conds_group_flags;
  conds_group_flags = 0;
  conds_group_flag_next = 1;
  do {
    if (conds_group_flags_mask & conds_group_flag_next) {
      if (conds_group_flags_index & 1) {
	conds_group_flags |= conds_group_flag_next;
      }
      conds_group_flags_index >>= 1;
    }
    conds_group_flag_next <<= 1;
  } while (conds_group_flags_index != 0);
  return (conds_group_flags);
}

/* this generic function returns a simple mask of a conditions group's
   flags and one or more bitwise operations that can be used with the
   mask to test the given condition.  it returns zero if the condition
   can't be tested with a simple mask: */
tme_uint32_t
tme_recode_conds_simple_mask(const struct tme_recode_conds_group *conds_group,
			     tme_uint32_t cond,
			     tme_recode_uguest_t *_conds_group_flags_mask)
{
  tme_recode_uguest_t conds_group_flags_same_for_mask[2];
  int conds_group_flags_same_for_defined[2];
  tme_uint32_t conds_group_flags_index;
  tme_recode_uguest_t conds_group_flags;
  int cond_true;
  tme_recode_uguest_t conds_group_flags_same_for[2];
  tme_recode_uguest_t conds_group_flags_mask;
  tme_uint32_t bitwise_mask;

  /* we haven't seen any combination of flags that make the condition
     either true or false yet: */
  conds_group_flags_same_for_mask[1] = conds_group->tme_recode_conds_group_flags;
  conds_group_flags_same_for_mask[0] = conds_group->tme_recode_conds_group_flags;
  conds_group_flags_same_for_defined[1] = FALSE;
  conds_group_flags_same_for_defined[0] = FALSE;

  /* loop over all combinations of the flags: */
  conds_group_flags_index = tme_recode_conds_group_flags_index_max(conds_group);
  do {
    conds_group_flags = tme_recode_conds_group_flags_from_index(conds_group, conds_group_flags_index);

    /* test if this combination of flags makes the condition true: */
    cond_true = ((*conds_group->tme_recode_conds_group_guest_func)(conds_group_flags, cond) != 0);

    /* if this is the first combination of flags that we have seen
       that makes the condition have this state: */
    if (!conds_group_flags_same_for_defined[cond_true]) {

      /* start tracking flags that always have the same values when
	 the condition has this state: */
      conds_group_flags_same_for[cond_true] = conds_group_flags;
      conds_group_flags_same_for_defined[cond_true] = TRUE;
    }

    /* otherwise, this is not the first combination of flags that
       we have seen that makes the condition have this state: */
    else {

      /* drop any flags that before had the same value when the
	 condition had this state, that now have a different value
	 when the condition has this same state: */
      conds_group_flags_same_for_mask[cond_true]
	&= ~(conds_group_flags_same_for[cond_true]
	     ^ conds_group_flags);
    }

  } while (conds_group_flags_index-- != 0);

  /* if we never saw a combination of flags that makes the condition
     true: */
  if (!conds_group_flags_same_for_defined[1]) {

    /* this condition is always false, which we can express with an
       all-bits-zero mask and either OR or AND: */
    *_conds_group_flags_mask = 0;
    return (TME_RECODE_BITWISE_OR | TME_RECODE_BITWISE_AND);
  }

  /* if we never saw a combination of flags that makes the condition
     false: */
  if (!conds_group_flags_same_for_defined[0]) {

    /* this condition is always true, which we can express with an
       all-bits-zero mask and either NOR or NAND: */
    *_conds_group_flags_mask = 0;
    return (TME_RECODE_BITWISE_NOR | TME_RECODE_BITWISE_NAND);
  }

  /* bits that are the same in all combinations of flags that make the
     condition both true and false, can't be in the mask of flags that
     determine the condition: */
  conds_group_flags_mask
    = ((conds_group_flags_same_for[1]
	^ ~conds_group_flags_same_for[0])
       & conds_group_flags_same_for_mask[1]
       & conds_group_flags_same_for_mask[0]);
  conds_group_flags_same_for_mask[1] &= ~conds_group_flags_mask;
  conds_group_flags_same_for_mask[0] &= ~conds_group_flags_mask;

  /* if there is any mask of flags that determine one state of the
     condition, the mask of flags that determine the other state must
     either be zero, or be exactly the same mask: */
  conds_group_flags_mask = conds_group_flags_same_for_mask[1];
  if (conds_group_flags_mask == 0) {
    conds_group_flags_mask = conds_group_flags_same_for_mask[0];
  }
  else {
    assert (conds_group_flags_same_for_mask[0] == 0
	    || conds_group_flags_same_for_mask[0] == conds_group_flags_mask);
  }

  /* loop over all combinations of the flags again, eliminating
     bitwise operations that don't give the correct condition state
     under the mask for all combinations: */
  bitwise_mask
    = (TME_RECODE_BITWISE_OR
       | TME_RECODE_BITWISE_NOR
       | TME_RECODE_BITWISE_AND
       | TME_RECODE_BITWISE_NAND);
  conds_group_flags_index = tme_recode_conds_group_flags_index_max(conds_group);
  do {
    conds_group_flags = tme_recode_conds_group_flags_from_index(conds_group, conds_group_flags_index);

    /* test if this combination of flags makes the condition true: */
    cond_true = ((*conds_group->tme_recode_conds_group_guest_func)(conds_group_flags, cond) != 0);

    /* mask off all other flags: */
    conds_group_flags &= conds_group_flags_mask;

    /* eliminate bitwise operations that don't give the correct
       condition state for the flags: */
    if (cond_true != (conds_group_flags != 0)) {
      bitwise_mask &= ~TME_RECODE_BITWISE_OR;
    }
    if (cond_true != (conds_group_flags == 0)) {
      bitwise_mask &= ~TME_RECODE_BITWISE_NOR;
    }
    if (cond_true != (conds_group_flags == conds_group_flags_mask)) {
      bitwise_mask &= ~TME_RECODE_BITWISE_AND;
    }
    if (cond_true != (conds_group_flags != conds_group_flags_mask)) {
      bitwise_mask &= ~TME_RECODE_BITWISE_NAND;
    }

  } while (conds_group_flags_index-- != 0);

  /* return any mask of flags and bitwise operations: */
  *_conds_group_flags_mask = conds_group_flags_mask;
  return (bitwise_mask);
}

#ifdef TME_RECODE_DEBUG
#include <stdio.h>

/* this function dumps a conditions thunk: */
void
tme_recode_conds_thunk_dump(const struct tme_recode_ic *ic,
			    const struct tme_recode_conds_thunk *conds_thunk,
			    const char * const *cond_names)
{
  printf("  flags offset: 0x%x\n  flags tested: ",
	 (unsigned int) conds_thunk->tme_recode_conds_thunk_flags_offset);
  tme_recode_uguest_dump(conds_thunk->tme_recode_conds_thunk_flags);
  printf("\n");
  tme_recode_host_conds_thunk_dump(ic, conds_thunk, cond_names);
}
#endif /* TME_RECODE_DEBUG */

#endif /* TME_HAVE_RECODE */
