/* $Id: recode-flags.c,v 1.2 2008/07/01 01:40:44 fredette Exp $ */

/* libtme/recode-flags.c - generic support for recode flags: */

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
_TME_RCSID("$Id: recode-flags.c,v 1.2 2008/07/01 01:40:44 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* this returns a flags thunk for a flags group: */
const struct tme_recode_flags_thunk *
tme_recode_flags_thunk(struct tme_recode_ic *ic,
		       const struct tme_recode_flags_group *flags_group_template)
{
  struct tme_recode_flags_group flags_group_buffer;
  struct tme_recode_flags_thunk flags_thunk_buffer;
  const struct tme_recode_flag *flag;
  unsigned int cond;
  tme_recode_uguest_t flag_flag;
  const struct tme_recode_flags_group *flag_group_other;
  const struct tme_recode_flag *flag_other;
  int flag_need;
  struct tme_recode_flags_group *flags_group;

  /* start a copy of the flags group: */
  memset(&flags_group_buffer, 0, sizeof(flags_group_buffer));
  flags_group_buffer.tme_recode_flags_group_insn_class = flags_group_template->tme_recode_flags_group_insn_class;
  flags_group_buffer.tme_recode_flags_group_insn_size = flags_group_template->tme_recode_flags_group_insn_size;
  flags_group_buffer.tme_recode_flags_group_flags = flags_group_template->tme_recode_flags_group_flags;
  flags_group_buffer.tme_recode_flags_group_flags_reg_size = flags_group_template->tme_recode_flags_group_flags_reg_size;
  flags_group_buffer.tme_recode_flags_group_flags_reg = flags_group_template->tme_recode_flags_group_flags_reg;

  /* assume that this group doesn't need the guest function: */
  flags_group_buffer.tme_recode_flags_group_guest_func = NULL;

  /* start a flags thunk: */
  memset(&flags_thunk_buffer, 0, sizeof(flags_thunk_buffer));
  flags_thunk_buffer.tme_recode_flags_thunk_flags_offset
    = tme_recode_flags_reg_offset(flags_group_template->tme_recode_flags_group_flags_reg_size,
				  flags_group_template->tme_recode_flags_group_flags_reg);

  /* loop over the flags in the group: */
  for (flag = flags_group_buffer.tme_recode_flags_group_flags;
       flag->tme_recode_flag_flag != 0;
       flag++) {

    /* the flag size can't be greater than the instruction size: */
    assert (flag->tme_recode_flag_size
	    <= flags_group_buffer.tme_recode_flags_group_insn_size);

    /* get this flag: */
    flag_flag = flag->tme_recode_flag_flag;

    /* the flag must be a single bit: */
    assert ((flag_flag & (flag_flag - 1)) == 0);

    /* the list of flags must be in decreasing order: */
    assert (flags_thunk_buffer.tme_recode_flags_thunk_flags_changed == 0
	    || flag_flag < flags_thunk_buffer.tme_recode_flags_thunk_flags_changed);

    /* add this flag to the mask of changed flags: */
    flags_thunk_buffer.tme_recode_flags_thunk_flags_changed |= flag_flag;

    /* get the unmodified condition: */
    cond = (flag->tme_recode_flag_cond & ~TME_RECODE_COND_MOD_NOT);

    /* if this isn't the undefined condition: */
    if (cond != TME_RECODE_COND_UNDEF) {

      /* add this flag to the mask of flags that are changed in a
	 defined way: */
      flags_thunk_buffer.tme_recode_flags_thunk_flags_defined |= flag_flag;

      /* if the guest needs to provide this flag: */
      if (cond != TME_RECODE_COND_FALSE
	  && TME_RECODE_FLAG_NEED(flags_group_buffer.tme_recode_flags_group_insn_class,
				  flags_group_buffer.tme_recode_flags_group_insn_size,
				  cond,
				  flag->tme_recode_flag_size)) {

	/* this flags group will use the guest function: */
	flags_group_buffer.tme_recode_flags_group_guest_func = flags_group_template->tme_recode_flags_group_guest_func;
      }
    }
  }

  /* loop over the existing flags groups: */
  for (flag_group_other = ic->tme_recode_ic_flags_groups;
       flag_group_other != NULL;
       flag_group_other = flag_group_other->tme_recode_flags_group_next) {

    /* skip this existing flags group if its flags register size or
       flags register number don't match: */
    if ((flag_group_other->tme_recode_flags_group_flags_reg_size
	 != flags_group_buffer.tme_recode_flags_group_flags_reg_size)
	|| (flag_group_other->tme_recode_flags_group_flags_reg
	    != flags_group_buffer.tme_recode_flags_group_flags_reg)) {
      continue;
    }

    /* skip this existing flags group if its instruction class or
       guest function don't match: */
    if ((flag_group_other->tme_recode_flags_group_insn_class
	 != flags_group_buffer.tme_recode_flags_group_insn_class)
	|| (flag_group_other->tme_recode_flags_group_guest_func
	    != flags_group_buffer.tme_recode_flags_group_guest_func)) {
      continue;
    }

    /* compare all of the flags in this existing group to the ones in
       the given group.  since we require group flags lists to be
       well-ordered, this is simple: */
    for (flag = flags_group_buffer.tme_recode_flags_group_flags,
	   flag_other = flag_group_other->tme_recode_flags_group_flags;
	 ;
	 flag++, flag_other++) {

      /* stop now if the flag isn't the same: */
      if (flag->tme_recode_flag_flag != flag_other->tme_recode_flag_flag) {
	break;
      }

      /* if we reached the ends of the lists, the two groups' flags
	 codes compare the same, and we can simply add the given flags
	 group to the existing flag group's thunk: */
      if (flag->tme_recode_flag_flag == 0) {

	/* add the given flags group to the existing flag group's thunk: */
	tme_recode_host_flags_thunk_add(ic, flag_group_other->tme_recode_flags_group_thunk, &flags_group_buffer);

	/* update the initial thunk offset of the first variable thunk: */
	ic->tme_recode_ic_thunk_off_variable
	  = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

	/* return the flag group's thunk: */
	return (flag_group_other->tme_recode_flags_group_thunk);
      }

      /* stop now if the condition isn't the same: */
      cond = flag->tme_recode_flag_cond;
      if (cond != flag_other->tme_recode_flag_cond) {
	break;
      }

      /* get the unmodified condition: */
      cond &= ~TME_RECODE_COND_MOD_NOT;

      /* skip the undefined condition: */
      if (cond == TME_RECODE_COND_UNDEF) {
	continue;
      }

      /* skip the false condition: */
      if (cond == TME_RECODE_COND_FALSE) {
	continue;
      }

      /* see if the guest needs to provide this flag: */
      flag_need
	= TME_RECODE_FLAG_NEED(flags_group_buffer.tme_recode_flags_group_insn_class,
			       flags_group_buffer.tme_recode_flags_group_insn_size,
			       cond,
			       flag->tme_recode_flag_size);

      /* if the guest needs to provide this flag in the given group,
	 but not in the existing group, or vice-versa, stop now: */
      if (!flag_need
	  != !TME_RECODE_FLAG_NEED(flag_group_other->tme_recode_flags_group_insn_class,
				   flag_group_other->tme_recode_flags_group_insn_size,
				   cond,
				   flag_other->tme_recode_flag_size)) {
	break;
      }

      /* if the guest doesn't need to provide this flag in either the
	 given group or the existing group, but the two flags aren't
	 compatible in the host, stop now: */
      if (!flag_need
	  && !TME_RECODE_FLAGS_COMPAT(&flags_group_buffer,
				      flag,
				      flag_group_other,
				      flag_other)) {
	break;
      }
    }
  }

  /* allocate and fill the new flags group: */
  flags_group = tme_new(struct tme_recode_flags_group, 1);
  *flags_group = flags_group_buffer;

  /* build the new flags thunk: */
  flags_group->tme_recode_flags_group_thunk = tme_recode_host_flags_thunk_new(ic, &flags_thunk_buffer, flags_group);

  /* add this new flags group to the list: */
  flags_group->tme_recode_flags_group_next = ic->tme_recode_ic_flags_groups;
  ic->tme_recode_ic_flags_groups = flags_group;

  /* update the initial thunk offset of the first variable thunk: */
  ic->tme_recode_ic_thunk_off_variable
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

  /* return the flags thunk: */
  return (flags_group->tme_recode_flags_group_thunk);
}

/* this returns a mask of flags that are changed in the same
   deterministic way (defined, set, clear) by all of the instructions
   in the group and that are provided by the host, optionally matching
   only a certain condition: */
tme_recode_uguest_t
tme_recode_flags_group_flags_defined_host(const struct tme_recode_flags_group *flags_group,
					  unsigned int cond_match)
{
  const struct tme_recode_flag *flag;
  unsigned int cond;
  tme_recode_uguest_t flags_defined_host;

  /* start with no flags: */
  flags_defined_host = 0;

  /* loop over the flags in the group: */
  for (flag = flags_group->tme_recode_flags_group_flags;
       flag->tme_recode_flag_flag != 0;
       flag++) {

    /* get the unmodified condition code: */
    cond = (flag->tme_recode_flag_cond & ~TME_RECODE_COND_MOD_NOT);

    /* if this is a deterministic condition code provided by the host: */
    if (cond != TME_RECODE_COND_UNDEF
	&& (cond == TME_RECODE_COND_FALSE
	    || !TME_RECODE_FLAG_NEED(flags_group->tme_recode_flags_group_insn_class,
				     flags_group->tme_recode_flags_group_insn_size,
				     cond,
				     flag->tme_recode_flag_size))) {

      /* if we aren't matching any particular condition, or if
	 this condition matches: */
      if (cond_match == TME_RECODE_COND_UNDEF
	  || flag->tme_recode_flag_cond == cond_match) {

	/* add this flag to the mask of flags that are provided by the
	   host: */
	flags_defined_host |= flag->tme_recode_flag_flag;
      }
    }
  }

  return (flags_defined_host);
}

/* this returns the mask of sizes for the given flags: */
tme_uint32_t
tme_recode_flags_group_sizes(const struct tme_recode_flags_group *flags_group, tme_recode_uguest_t flags)
{
  const struct tme_recode_flag *flag;
  unsigned int cond;
  tme_uint32_t sizes;

  /* start with no sizes: */
  sizes = 0;

  /* loop over the flags in the group: */
  for (flag = flags_group->tme_recode_flags_group_flags;
       flag->tme_recode_flag_flag != 0;
       flag++) {

    /* get the unmodified condition code: */
    cond = (flag->tme_recode_flag_cond & ~TME_RECODE_COND_MOD_NOT);

    /* if this is a deterministic variable condition code, and its
       flag is in the mask: */
    if (cond != TME_RECODE_COND_UNDEF
	&& cond != TME_RECODE_COND_FALSE
	&& (flag->tme_recode_flag_flag & flags)) {

      /* add this size to the mask: */
      sizes |= (1 << flag->tme_recode_flag_size);
    }
  }

  return (sizes);
}

/* this returns the offset of the first byte of a flags register: */
tme_uint32_t
tme_recode_flags_reg_offset(unsigned int flags_reg_size,
			    tme_uint32_t flags_reg)
{
  struct tme_ic *ic;
  char *flags;

  /* make a dummy struct tme_ic pointer: */
  ic = 0;

  /* dispatch on the size of the flags register to get its address in
     the dummy struct tme_ic: */
  switch (flags_reg_size) {
  default: assert(FALSE);
#ifdef TME_RECODE_SIZE_64
  case TME_RECODE_SIZE_64: flags = (char *) &ic->tme_ic_ireg_uint64(flags_reg); break;
#endif /* TME_RECODE_SIZE_64 */
  case TME_RECODE_SIZE_32: flags = (char *) &ic->tme_ic_ireg_uint32(flags_reg); break;
  case TME_RECODE_SIZE_16: flags = (char *) &ic->tme_ic_ireg_uint16(flags_reg); break;
  case TME_RECODE_SIZE_8: flags = (char *) &ic->tme_ic_ireg_uint8(flags_reg); break;
  }

  /* return the offset of the flags register: */
  return (flags - (char *) ic);
}

#ifdef TME_RECODE_DEBUG
#include <stdio.h>

/* this function dumps a flags thunk: */
void
tme_recode_flags_thunk_dump(const struct tme_recode_ic *ic,
			    const struct tme_recode_flags_thunk *flags_thunk)
{
  printf("  flags offset: 0x%x\n  flags changed: ",
	 (unsigned int) flags_thunk->tme_recode_flags_thunk_flags_offset);
  tme_recode_uguest_dump(flags_thunk->tme_recode_flags_thunk_flags_changed);
  printf("\n  flags defined: ");
  tme_recode_uguest_dump(flags_thunk->tme_recode_flags_thunk_flags_defined);
  printf("\n");
  tme_recode_host_flags_thunk_dump(ic, flags_thunk);
}
#endif /* TME_RECODE_DEBUG */

#endif /* TME_HAVE_RECODE */
