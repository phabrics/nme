/* libtme/threads.c - threads management: */

/*
 * Copyright (c) 2015 Ruben Agin
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

/* includes: */
#include <tme/threads.h>

/* globals: */
static tme_threads_fn _tme_threads_run;
static int inited, using_glib, running;
static tme_cond_t thr_cond;

void tme_threads_init(tme_threads_fn init, tme_threads_fn run, int use_glib) {
  _tme_threads_run = run;
  using_glib = use_glib;
  if(!inited) {
    _tme_threads_init();
    tme_cond_init(&thr_cond);
    inited=TRUE;
  }
  if(init)
    (*init)();
}

void tme_threads_run(void) {
  tme_cond_notify(&thr_cond, TRUE);
  running=TRUE;
  _tme_thread_suspended();
#ifdef TME_THREADS_SJLJ
  tme_sjlj_threads_run(use_glib);
#endif
  if(_tme_threads_run) (*_tme_threads_run)();
  while(1) {
    usleep(1000000);
  }
}

void tme_thread_enter(tme_mutex_t *mutex) {
  _tme_thread_resumed();
  if(mutex) {
    tme_mutex_lock(mutex);
    if(!running) tme_cond_wait_yield(&thr_cond, mutex);
  }
}

#ifdef TME_THREADS_POSIX
pthread_rwlock_t tme_rwlock_suspere;

#ifdef HAVE_PTHREAD_SETSCHEDPARAM
static pthread_attr_t *attrp;

void tme_thread_set_defattr(pthread_attr_t *attr) {
  attrp=attr;
}
pthread_attr_t *tme_thread_defattr() {
  return attrp;
}
#endif // HAVE_PTHREAD_SETSCHEDPARAM
#endif

