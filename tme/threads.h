/* $Id: threads.h,v 1.10 2010/06/05 19:36:35 fredette Exp $ */

/* tme/threads.h - header file for threads: */

/*
 * Copyright (c) 2003 Matt Fredette
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

#ifndef _TME_THREADS_H
#define _TME_THREADS_H

#include <tme/common.h>
_TME_RCSID("$Id: threads.h,v 1.10 2010/06/05 19:36:35 fredette Exp $");

/* includes: */
#include <errno.h>
#include <sys/time.h>

/* setjmp/longjmp threading: */
#ifdef TME_THREADS_POSIX
#include "threads-posix.h"
#define tme_threads_glib_yield() do { } while (/* CONSTCOND */ 0)
#elif defined(TME_THREADS_GLIB)
#include "threads-glib.h"
#define tme_threads_glib_yield() do { } while (/* CONSTCOND */ 0)
#elif defined(TME_THREADS_SJLJ)
#include "threads-sjlj.h"
#endif

typedef void (*tme_threads_fn) _TME_P((void));
extern tme_threads_fn tme_threads_run;

static _tme_inline void tme_threads_init _TME_P((tme_threads_fn init, tme_threads_fn run)) {
  tme_threads_run = run;
  _tme_threads_init();
  if(init) (*init)();
}

#endif /* !_TME_THREADS_H */
