/* $Id: recode-rws.c,v 1.2 2010/02/15 16:57:13 fredette Exp $ */

/* libtme/recode-rws.c - generic support for recode reads and writes: */

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
_TME_RCSID("$Id: recode-rws.c,v 1.2 2010/02/15 16:57:13 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* this returns a read/write thunk for a read or a write: */
const struct tme_recode_rw_thunk *
tme_recode_rw_thunk(struct tme_recode_ic *ic,
		    const struct tme_recode_rw *rw_template)
{
  struct tme_recode_rw *rw;
  struct tme_recode_rw_thunk *rw_thunk;
  const struct tme_recode_rw *rw_other;

  /* the register size can't be bigger than the guest register size,
     and if this is a write, it must match the memory size, otherwise
     it must be at least the memory size: */
  assert (rw_template->tme_recode_rw_reg_size
	  <= ic->tme_recode_ic_reg_size);
  assert (rw_template->tme_recode_rw_write
	  ? (rw_template->tme_recode_rw_reg_size
	     == rw_template->tme_recode_rw_memory_size)
	  : (rw_template->tme_recode_rw_reg_size
	     >= rw_template->tme_recode_rw_memory_size));

  /* check the address type: */
  tme_recode_address_type_check(ic, &rw_template->tme_recode_rw_address_type);

  /* the bus boundary must be a power of two: */
  /* NB: this is a value in bytes, not a TME_RECODE_SIZE_: */
  assert (rw_template->tme_recode_rw_bus_boundary > 0
	  && (rw_template->tme_recode_rw_bus_boundary
	      & (rw_template->tme_recode_rw_bus_boundary - 1)) == 0);

  /* allocate and fill the new read or write: */
  rw = tme_new0(struct tme_recode_rw, 1);
  *rw = *rw_template;

  /* assume that we won't be able to duplicate an existing read/write
     thunk: */
  rw_thunk = NULL;

  /* loop over the existing reads and writes: */
  for (rw_other = ic->tme_recode_ic_rws;
       rw_other != NULL;
       rw_other = rw_other->tme_recode_rw_next) {

    /* skip this existing read or write if its direction doesn't
       match: */
    if (!rw_other->tme_recode_rw_write
	!= !rw->tme_recode_rw_write) {
      continue;
    }

    /* skip this existing read or write if its address information
       doesn't match: */
    if (!tme_recode_address_type_compare(ic,
					 &rw->tme_recode_rw_address_type,
					 &rw_other->tme_recode_rw_address_type)) {
      continue;
    }

    /* skip this existing read or write if its memory size or
       endianness don't match: */
    if (rw_other->tme_recode_rw_memory_size
	!= rw->tme_recode_rw_memory_size) {
      continue;
    }
    if ((rw_other->tme_recode_rw_memory_size
	 > TME_RECODE_SIZE_8)
	&& (rw_other->tme_recode_rw_memory_endian
	    != rw->tme_recode_rw_memory_endian)) {
      continue;
    }

    /* skip this existing read or write if its bus boundary
       doesn't match: */
    if (rw_other->tme_recode_rw_bus_boundary
	!= rw->tme_recode_rw_bus_boundary) {
      continue;
    }

    /* skip this existing read or write if its assist function doesn't
       match: */
    if (rw_other->tme_recode_rw_write
	? (rw_other->tme_recode_rw_guest_func_write
	   != rw->tme_recode_rw_guest_func_write)
	: (rw_other->tme_recode_rw_guest_func_read
	   != rw->tme_recode_rw_guest_func_read)) {
      continue;
    }

    /* at this point, everything relevant in this existing read or
       write is known to be identical, except for the register size
       and possibly the memory signedness.  at least one of them must
       be different: */
    assert ((rw_other->tme_recode_rw_reg_size
	     != rw->tme_recode_rw_reg_size)
	    || (!rw->tme_recode_rw_write
		&& (rw_other->tme_recode_rw_reg_size
		    > rw_other->tme_recode_rw_memory_size)
		&& (!rw_other->tme_recode_rw_memory_signed
		    != !rw->tme_recode_rw_memory_signed)));

    /* if this is a read: */
    if (!rw->tme_recode_rw_write) {

      /* if we can duplicate this read/write thunk: */
      rw_thunk = tme_recode_host_rw_thunk_dup(ic, rw, rw_other);
      if (rw_thunk != NULL) {
	break;
      }
    }
  }

  /* if we have to, make a new read/write thunk: */
  if (rw_thunk == NULL) {
    rw_thunk = tme_recode_host_rw_thunk_new(ic, rw);
  }

  /* finish this read/write thunk: */
  rw_thunk->tme_recode_rw_thunk_address_size = rw->tme_recode_rw_address_type.tme_recode_address_type_size;
  rw_thunk->tme_recode_rw_thunk_reg_size = rw->tme_recode_rw_reg_size;
  rw_thunk->tme_recode_rw_thunk_write = !!rw->tme_recode_rw_write;
  rw->tme_recode_rw_thunk = rw_thunk;

  /* add the new read or write to the ic: */
  rw->tme_recode_rw_next = ic->tme_recode_ic_rws;
  ic->tme_recode_ic_rws = rw;

  /* update the initial thunk offset of the first variable thunk: */
  ic->tme_recode_ic_thunk_off_variable
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

  /* return the read/write thunk: */
  return (rw_thunk);
}

#endif /* TME_HAVE_RECODE */
