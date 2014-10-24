/* $Id: recode-mmap.c,v 1.2 2008/07/01 02:00:53 fredette Exp $ */

/* libtme/host/recode-mmap.c - recode code file for mmap() hosts: */

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
_TME_RCSID("$Id: recode-mmap.c,v 1.2 2008/07/01 02:00:53 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

/* this host function allocates memory for building and running thunks: */
void
tme_recode_host_thunks_alloc(struct tme_recode_ic *ic,
			     tme_recode_thunk_off_t size_run)
{
  int size_page;
  int fd;
  tme_recode_host_insn_t *memory;

  /* get the page size: */
  size_page = getpagesize();
  assert ((size_page % sizeof(tme_recode_host_insn_t)) == 0);

  /* make sure the run memory size is at least the minimum thunk size,
     and round the run memory size up to the page size: */
  size_run = TME_MAX(size_run, TME_RECODE_HOST_THUNK_SIZE_MAX);
  size_run += (size_page - 1);
  size_run -= (size_run % size_page);
  assert (size_run >= TME_MAX(size_page, TME_RECODE_HOST_THUNK_SIZE_MAX));

  /* if MAP_ANON isn't defined, assume that we have to mmap /dev/zero
     to map anonymous memory: */
#ifndef MAP_ANON
  fd = open("/dev/zero", O_RDWR, 0);
  if (fd < 0) {
    abort();
  }
#else  /* MAP_ANON */
  fd = -1;
#endif /* MAP_ANON */

  /* mmap memory that is readable, writable and executable: */
  memory
    = ((tme_recode_host_insn_t *)
       mmap((void *) 0,
	    size_run,
	    (PROT_READ
	     | PROT_WRITE
	     | PROT_EXEC),
	    (MAP_SHARED
#ifdef MAP_ANON
	     | MAP_ANON
#endif /* MAP_ANON */
	     ),
	    fd,
	    0));
  if (memory == (tme_recode_host_insn_t *) MAP_FAILED) {
    abort();
  }

  /* return the memory: */
  ic->tme_recode_mmap_ic_thunks_start = memory;
  ic->tme_recode_ic_thunk_build_next = memory;
  ic->tme_recode_ic_thunk_build_end = memory + (size_run / sizeof(tme_recode_host_insn_t));
}

/* this host function starts building a new thunk.  it returns nonzero
   if a thunk of TME_RECODE_HOST_THUNK_SIZE_MAX bytes can be built: */
int
tme_recode_host_thunk_start(struct tme_recode_ic *ic)
{
  return ((ic->tme_recode_ic_thunk_build_end
	   - ic->tme_recode_ic_thunk_build_next)
	  >= ((TME_RECODE_HOST_THUNK_SIZE_MAX
	       + sizeof(tme_recode_host_insn_t)
	       - 1)
	      / sizeof(tme_recode_host_insn_t)));
}

/* this host function finishes building a new thunk: */
void
tme_recode_host_thunk_finish(struct tme_recode_ic *ic)
{

  /* round the next build address up to the thunk alignment: */
  ic->tme_recode_ic_thunk_build_next
    = ((tme_recode_host_insn_t *)
       ((((unsigned long) ic->tme_recode_ic_thunk_build_next)
	 + TME_RECODE_HOST_THUNK_ALIGN
	 - 1)
	& (0 - (unsigned long) TME_RECODE_HOST_THUNK_ALIGN)));
}

/* this host function invalidates all thunks starting from the given
   thunk offset: */
void
tme_recode_host_thunk_invalidate_all(struct tme_recode_ic *ic,
				     tme_recode_thunk_off_t thunk_off)
{
  
  /* reset the build address: */
  ic->tme_recode_ic_thunk_build_next = ic->tme_recode_mmap_ic_thunks_start + thunk_off;
  tme_recode_host_thunk_finish(ic);
}

#endif /* TME_HAVE_RECODE */
