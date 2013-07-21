/* $Id: token.h,v 1.3 2010/06/05 19:37:27 fredette Exp $ */

/* tme/token.h - header file for token functions: */

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

#ifndef _TME_TOKEN_H
#define _TME_TOKEN_H

#include <tme/common.h>
_TME_RCSID("$Id: token.h,v 1.3 2010/06/05 19:37:27 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <tme/memory.h>

/* macros: */

/* this returns nonzero if a token is valid: */
#define tme_token_is_valid(token)					\
  (__tme_predict_true(!tme_memory_atomic_read_flag(&(token)->tme_token_invalid)))

/* this returns nonzero if a token is invalid: */
#define tme_token_is_invalid(token)					\
  (__tme_predict_false(tme_memory_atomic_read_flag(&(token)->tme_token_invalid)))

/* this internal macro busies or unbusies a token: */
#ifndef TME_NO_DEBUG_LOCKS
#define _tme_token_busy_change(token, x)				\
  do {									\
    assert ((!tme_memory_atomic_read_flag(&(token)->tme_token_busy))\
	    != !(x));							\
    tme_memory_atomic_write_flag(&(token)->tme_token_busy, x);		\
    (token)->_tme_token_busy_file = __FILE__;				\
    (token)->_tme_token_busy_line = __LINE__;				\
  } while (/* CONSTCOND */ 0)
#elif !TME_THREADS_COOPERATIVE
#define _tme_token_busy_change(token, x)				\
  tme_memory_atomic_write_flag(&(token)->tme_token_busy, x)
#else  /* TME_THREADS_COOPERATIVE && defined(TME_NO_DEBUG_LOCKS) */
#define _tme_token_busy_change(token, x)				\
  do { } while (0 && tme_memory_atomic_read_flag(&(token)->tme_token_invalid) && (x))
#endif /* TME_THREADS_COOPERATIVE && defined(TME_NO_DEBUG_LOCKS) */

/* this busies a token: */
/* NB: a token must be busied before its validity can be checked.
   tme_token_busy() automatically makes a write-before-read barrier on
   the token to order itself with a subsequent tme_token_is_valid() or
   tme_token_is_invalid(), and the caller generally won't need to make
   any additional barrier to order reads after a validity check
   (because tokens can only be asynchronously invalidated, not
   validated).  however, a caller may need to make write-before-write
   barriers to order any writes before a tme_token_unbusy(): */
#define tme_token_busy(token)						\
  do {									\
    _tme_token_busy_change(token, TRUE);				\
    tme_memory_barrier(token,						\
		       sizeof(*token),					\
		       TME_MEMORY_BARRIER_WRITE_BEFORE_READ);		\
  } while (/* CONSTCOND */ 0)

/* this unbusies a token: */
#define tme_token_unbusy(token)		_tme_token_busy_change(token, FALSE)

/* this invalidates a token without synchronizing: */
#define tme_token_invalidate_nosync(token)				\
  tme_memory_atomic_write_flag(&(token)->tme_token_invalid, TRUE)

/* types: */

/* a token: */
struct tme_token {

  /* if this is nonzero, the token has been invalidated: */
  tme_memory_atomic_flag_t tme_token_invalid;

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* if this is nonzero, the token is busy: */
  tme_memory_atomic_flag_t tme_token_busy;

  /* this mutex synchronizes tme_token_invalidate() and
     tme_token_invalid_clear(): */
  tme_mutex_t tme_token_invalid_mutex;

#ifndef TME_NO_DEBUG_LOCKS

  /* the file and line number of the last busier or unbusier: */
  _tme_const char *_tme_token_busy_file;
  unsigned long _tme_token_busy_line;

#endif /* !TME_NO_DEBUG_LOCKS */

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */
};

/* prototypes: */
void tme_token_init _TME_P((struct tme_token *));
void tme_token_invalidate _TME_P((struct tme_token *));
void tme_token_invalid_clear _TME_P((struct tme_token *));

/* this wrapper macro predicts that the token doesn't need to be
   cleared: */
#define tme_token_invalid_clear(token)		\
  do {						\
    if (tme_token_is_invalid(token)) {		\
      tme_token_invalid_clear(token);		\
    }						\
  } while (/* CONSTCOND */ 0)

#endif /* !_TME_TOKEN_H */
