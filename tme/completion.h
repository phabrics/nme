/* $Id: completion.h,v 1.1 2009/02/28 16:48:11 fredette Exp $ */

/* tme/completion.h - header file for completions: */

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

#ifndef _TME_COMPLETION_H
#define _TME_COMPLETION_H

#include <tme/common.h>
_TME_RCSID("$Id: completion.h,v 1.1 2009/02/28 16:48:11 fredette Exp $");

/* includes: */
#include <tme/memory.h>

/* macros: */

/* this initializes a completion: */
#define tme_completion_init(completion)				\
  do {								\
    tme_memory_atomic_init_flag(&(completion)->tme_completion_valid, FALSE);\
  } while (/* CONSTCOND */ 0)

/* this returns nonzero if the completion is valid: */
/* NB: this does not make any barrier ordering its read of the valid
   flag with any reads of completion state protected by the flag.  it
   is up to callers to make these barriers: */
#define tme_completion_is_valid(completion)			\
  (__tme_predict_true(tme_memory_atomic_read_flag(&(completion)->tme_completion_valid)))

/* this validates a completion: */
/* NB: this does not necessarily make any barrier ordering its write
   of the valid flag with any writes of completion state protected by
   the flag.  it is up to the callers to guarantee that a barrier is
   made before validating a completion.

   a caller may be able to piggyback on the barrier made by one of its
   own tme_mutex_unlock().  tme_mutex_unlock() must make a barrier
   from the compiler's perspective (the compiler must not schedule
   instructions across it) and a barrier from the processor's
   perspective (the processor must not reorder earlier reads or writes
   across it).

   for example, an element with a central mutex could delay using
   tme_completion_validate() until after an unlock of that mutex: */
#define tme_completion_validate(completion)			\
  do {								\
    tme_memory_atomic_write_flag(&(completion)->tme_completion_valid, TRUE);\
  } while (/* CONSTCOND */ 0)

/* this invalidates a completion: */
#define tme_completion_invalidate(completion)			\
  do {								\
    tme_memory_atomic_write_flag(&(completion)->tme_completion_valid, FALSE);\
  } while (/* CONSTCOND */ 0)

/* types: */

/* a completion: */
struct tme_completion {

  /* if this is nonzero, the completion is valid: */
  tme_memory_atomic_flag_t tme_completion_valid;

  /* an error code: */
  int tme_completion_error;

  /* various scalar completion values: */
  union {
    tme_int8_t _tme_completion_scalar_u_int8;
#define tme_completion_scalar_int8 tme_completion_scalar._tme_completion_scalar_u_int8
    tme_uint8_t _tme_completion_scalar_u_uint8;
#define tme_completion_scalar_uint8 tme_completion_scalar._tme_completion_scalar_u_uint8
    tme_int16_t _tme_completion_scalar_u_int16;
#define tme_completion_scalar_int16 tme_completion_scalar._tme_completion_scalar_u_int16
    tme_uint16_t _tme_completion_scalar_u_uint16;
#define tme_completion_scalar_uint16 tme_completion_scalar._tme_completion_scalar_u_uint16
    tme_int32_t _tme_completion_scalar_u_int32;
#define tme_completion_scalar_int32 tme_completion_scalar._tme_completion_scalar_u_int32
    tme_uint32_t _tme_completion_scalar_u_uint32;
#define tme_completion_scalar_uint32 tme_completion_scalar._tme_completion_scalar_u_uint32
#ifdef TME_HAVE_INT64_T
    tme_int64_t _tme_completion_scalar_u_int64;
#define tme_completion_scalar_int64 tme_completion_scalar._tme_completion_scalar_u_int64
    tme_uint64_t _tme_completion_scalar_u_uint64;
#define tme_completion_scalar_uint64 tme_completion_scalar._tme_completion_scalar_u_uint64
#endif /* TME_HAVE_INT64_T */
  } tme_completion_scalar;
};

#endif /* !_TME_COMPLETION_H */
