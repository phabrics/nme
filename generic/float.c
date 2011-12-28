/* $Id: float.c,v 1.2 2005/05/14 22:08:07 fredette Exp $ */

/* generic/float.c - generic native floating-point support: */

/*
 * Copyright (c) 2004 Matt Fredette
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
_TME_RCSID("$Id: float.c,v 1.2 2005/05/14 22:08:07 fredette Exp $");

/* includes: */
#include <tme/generic/float.h>
#include <signal.h>

/* the mutex protecting the native floating point: */
static tme_mutex_t _tme_float_mutex;

/* this is nonzero if the SIGFPE handler has been installed: */
/* XXX FIXME this assumes that a handler installed once in one thread
   will catch signals for all threads.  obviously, whether or not this
   is true depends on how the threads package deals with signals: */
static int _tme_float_sigfpe_handler_installed;

/* the current exceptions: */
static int _tme_float_exceptions;

/* the user's exception handler: */
static void (*_tme_float_exception_handler) _TME_P((int, void *));
static void *_tme_float_exception_handler_private;

/* update the current exceptions: */
static void
_tme_float_exceptions_update(int at_least_one)
{
  int exceptions_new;
#ifdef HAVE_FPGETSTICKY
  int sticky;
#endif /* HAVE_FPGETSTICKY */

  /* start with no new exceptions: */
  exceptions_new = 0;

  /* get the exception status: */
#ifdef HAVE_FPGETSTICKY
  sticky = fpgetsticky();
#define TME_FP_X_MAP(fp_x, float_x) if (sticky & (fp_x)) exceptions_new |= (float_x)
#ifdef FP_X_INV
  TME_FP_X_MAP(FP_X_INV, TME_FLOAT_EXCEPTION_INVALID);
#endif
#ifdef FP_X_DNML
  TME_FP_X_MAP(FP_X_DNML, TME_FLOAT_EXCEPTION_DENORMAL);
#endif
#ifdef FP_X_DZ
  TME_FP_X_MAP(FP_X_DZ, TME_FLOAT_EXCEPTION_DIVBYZERO);
#endif
#ifdef FP_X_OFL
  TME_FP_X_MAP(FP_X_OFL, TME_FLOAT_EXCEPTION_OVERFLOW);
#endif
#ifdef FP_X_UFL
  TME_FP_X_MAP(FP_X_UFL, TME_FLOAT_EXCEPTION_UNDERFLOW);
#endif
#ifdef FP_X_IMP
  TME_FP_X_MAP(FP_X_IMP, TME_FLOAT_EXCEPTION_INEXACT);
#endif
#ifdef FP_X_IOV
  TME_FP_X_MAP(FP_X_IOV, TME_FLOAT_EXCEPTION_OVERFLOW_INT);
#endif
#undef TME_FP_X_MAP
#endif /* HAVE_FPGETSTICKY */

  /* if we have no new exceptions, but we should have at least one,
     add in the generic exception: */
  if (exceptions_new == 0
      && at_least_one) {
    exceptions_new |= TME_FLOAT_EXCEPTION_GENERIC;
  }

  /* accumulate the new exceptions into the current exceptions: */
  _tme_float_exceptions |= exceptions_new;
  
  /* clear the exception status: */
#ifdef HAVE_FPSETSTICKY
  fpsetsticky(0);
#endif /* HAVE_FPSETSTICKY */
}

/* our SIGFPE handler: */
static RETSIGTYPE
_tme_float_sigfpe_handler(int unused)
{

  /* update the current exceptions: */
  _tme_float_exceptions_update(TRUE);

  /* call any user exception handler with the new exceptions: */
  if (_tme_float_exception_handler != NULL) {
    (*_tme_float_exception_handler)(_tme_float_exceptions, _tme_float_exception_handler_private);
  }
}  

/* this enters native floating-point operation: */
void
tme_float_enter(int rounding_mode, void (*exception_handler)(int, void *), void *exception_handler_private)
{

  /* lock the native floating-point mutex: */
  tme_mutex_lock(&_tme_float_mutex);

  /* set any user exception handler: */
  _tme_float_exception_handler = exception_handler;
  _tme_float_exception_handler_private = exception_handler_private;

  /* establish a signal handler: */
  /* XXX FIXME this assumes that a handler installed once in one thread
     will catch signals for all threads.  obviously, whether or not this
     is true depends on how the threads package deals with signals: */
  if (!_tme_float_sigfpe_handler_installed) {
    signal(SIGFPE, _tme_float_sigfpe_handler);
    _tme_float_sigfpe_handler_installed = TRUE;
  }
  
  /* set the rounding mode: */
#ifdef HAVE_FPSETROUND
  fpsetround(rounding_mode);
#endif /* HAVE_FPSETROUND */

  /* clear the exception status: */
  _tme_float_exceptions = 0;
#ifdef HAVE_FPSETSTICKY
  fpsetsticky(0);
#endif /* HAVE_FPSETSTICKY */

  /* unmask all exceptions: */
#ifdef HAVE_FPSETMASK
  fpsetmask(0
#ifdef FP_X_INV
  | FP_X_INV
#endif
#ifdef FP_X_DNML
  | FP_X_DNML
#endif
#ifdef FP_X_DZ
  | FP_X_DZ
#endif
#ifdef FP_X_OFL
  | FP_X_OFL
#endif
#ifdef FP_X_UFL
  | FP_X_UFL
#endif
#ifdef FP_X_IMP
  | FP_X_IMP
#endif
#ifdef FP_X_IOV
  | FP_X_IOV
#endif
	    );
#endif /* HAVE_FPSETMASK */
}

/* this returns the current native floating-point exceptions: */
int
tme_float_exceptions(void)
{
  /* update and return the current exceptions: */
  _tme_float_exceptions_update(FALSE);
  return (_tme_float_exceptions);
}  

/* this leaves native floating-point operation: */
int
tme_float_leave(void)
{
  int exceptions;

  /* get the final set of exceptions: */
  exceptions = tme_float_exceptions();

  /* clear any user exception handler: */
  _tme_float_exception_handler = NULL;

  /* unlock the native floating-point mutex: */
  tme_mutex_unlock(&_tme_float_mutex);

  /* return the final set of exceptions: */
  return (exceptions);
}

/* missing standard functions: */
#ifndef HAVE_ISINFF
int
isinff(float x)
{
  return (x > FLOAT_MAX_FLOAT
	  || x < (-FLOAT_MAX_FLOAT));
}
#endif /* !HAVE_ISINFF */

/* include the automatically-generated code: */
#include "float-auto.c"
