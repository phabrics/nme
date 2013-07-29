/* $Id: recode-mmap.h,v 1.2 2008/07/01 23:57:30 fredette Exp $ */

/* libtme/host/recode-mmap.h - recode header for mmap hosts: */

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

_TME_RCSID("$Id: recode-mmap.h,v 1.2 2008/07/01 23:57:30 fredette Exp $");

/* macros: */

/* the maximum size of a single thunk, in bytes: */
#define TME_RECODE_HOST_THUNK_SIZE_MAX				(32768)

/* these are the mmap-specific members added to struct tme_recode_ic.
   this macro is added to TME_RECODE_HOST_IC at configure time: */
#define TME_RECODE_MMAP_IC						\
									\
  /* the start of the thunks memory: */					\
  tme_recode_host_insn_t *tme_recode_mmap_ic_thunks_start;		\

/* this converts a thunk build pointer into a thunk offset: */
#define tme_recode_build_to_thunk_off(ic, build)			\
  ((build)								\
   - (ic)->tme_recode_mmap_ic_thunks_start)

/* this converts a thunk offset into a thunk function pointer: */
#define tme_recode_thunk_off_to_pointer(ic, thunk_off, type)		\
  ((type)								\
   ((ic)->tme_recode_mmap_ic_thunks_start				\
    + (thunk_off)))

/* this converts a function pointer (not necessarily a thunk function
   pointer) into a thunk offset.  for a thunk function pointer
   returned by tme_recode_thunk_off_to_pointer(), this always
   succeeds.  for any other function, this may not succeed, and the
   caller must confirm that tme_recode_thunk_off_to_pointer() recovers
   the function pointer if there is a possibility that it might not
   (due to the limits of a tme_recode_thunk_off_t): */
#define tme_recode_function_to_thunk_off(ic, func)			\
  ((tme_recode_thunk_off_t)						\
   (((_tme_const tme_recode_host_insn_t *)				\
     (func))								\
    - (ic)->tme_recode_mmap_ic_thunks_start))

/* this reads part of a finished thunk: */
#define tme_recode_thunk_off_read(ic, thunk_off, type, lvalue)		\
  ((_tme_const type *) ((ic)->tme_recode_mmap_ic_thunks_start + (thunk_off)) + (0 && (&(lvalue) - 1)))

/* this writes part of a finished thunk: */
#define tme_recode_thunk_off_write(ic, thunk_off, type, rvalue)		\
  do {									\
    *((type *)								\
      ((ic)->tme_recode_mmap_ic_thunks_start				\
       + (thunk_off))) = rvalue;					\
  } while (/* CONSTCOND */ 0)
