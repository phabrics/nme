/* $Id: token.c,v 1.1 2008/09/24 22:40:30 fredette Exp $ */

/* libtme/token.c - token functions: */

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
_TME_RCSID("$Id: token.c,v 1.1 2008/09/24 22:40:30 fredette Exp $");

/* includes: */
#include <tme/token.h>

/* this initializes a token: */
void
tme_token_init(struct tme_token *token)
{

  /* a constructed token is invalid: */
  tme_memory_atomic_init_flag(&token->tme_token_invalid, TRUE);

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* a constructed token is not busy: */
  tme_memory_atomic_init_flag(&token->tme_token_busy, FALSE);

  /* initialize the token's invalid mutex: */
  tme_mutex_init(&token->tme_token_invalid_mutex);

#ifndef TME_NO_DEBUG_LOCKS

  /* initialize the file and line number of the last busier or
     unbusier: */
  token->_tme_token_busy_file = NULL;
  token->_tme_token_busy_line = 0;

#endif /* !TME_NO_DEBUG_LOCKS */

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */
}

/* this invalidates a token: */
void
tme_token_invalidate(struct tme_token *token)
{

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* lock the token's invalid mutex: */
  tme_mutex_lock(&token->tme_token_invalid_mutex);

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */

  /* invalidate the token: */
  tme_memory_atomic_write_flag(&token->tme_token_invalid, TRUE);

#if TME_THREADS_COOPERATIVE
#ifndef TME_NO_DEBUG_LOCKS

  /* this token must not be busy: */
  assert (!tme_memory_atomic_read_flag(&token->tme_token_busy));

#endif /* !TME_NO_DEBUG_LOCKS */
#else  /* !TME_THREADS_COOPERATIVE */

  /* spin while the token is busy: */
  do { } while(tme_memory_atomic_read_flag(&token->tme_token_busy));

#endif /* !TME_THREADS_COOPERATIVE */

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* unlock the token's invalid mutex: */
  tme_mutex_unlock(&token->tme_token_invalid_mutex);

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */
}

/* this clears an invalid token: */
#undef tme_token_invalid_clear
void
tme_token_invalid_clear(struct tme_token *token)
{

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* the token must not be busy: */
  assert (!tme_memory_atomic_read_flag(&token->tme_token_busy));

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */

  /* if the token is valid, return now: */
  if (__tme_predict_false(!tme_memory_atomic_read_flag(&(token)->tme_token_invalid))) {
    return;
  }

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* lock the token's invalid mutex: */
  tme_mutex_lock(&token->tme_token_invalid_mutex);

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */

  /* mark the token as valid again: */
  tme_memory_atomic_write_flag(&token->tme_token_invalid, FALSE);

#if !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS)

  /* unlock the token's invalid mutex: */
  tme_mutex_unlock(&token->tme_token_invalid_mutex);

#endif /* !TME_THREADS_COOPERATIVE || !defined(TME_NO_DEBUG_LOCKS) */
}
