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
static tme_threads_fn1 _tme_threads_run;
static void *_tme_threads_arg;
static int inited;
#ifdef TME_THREADS_POSIX
static pthread_rwlock_t tme_rwlock_start;
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
#elif defined(TME_THREADS_GLIB)
static GRWLock tme_rwlock_start;
GRWLock tme_rwlock_suspere;
#endif

void tme_threads_init(tme_threads_fn1 run, void *arg) {
  _tme_threads_run = run;
  _tme_threads_arg = arg;
  if(!inited) {
    _tme_threads_init();
    if(!tme_thread_cooperative()) {
#ifdef TME_THREADS_POSIX
      pthread_rwlock_init(&tme_rwlock_start, NULL);      
      pthread_rwlock_wrlock(&tme_rwlock_start);
#elif defined(TME_THREADS_GLIB)
      g_rw_lock_init(&tme_rwlock_start);
      g_rw_lock_writer_lock(&tme_rwlock_start);
#endif
    }
    _tme_thread_resumed();  
    inited=TRUE;
  }
}

void tme_threads_run(void) {
  if(!tme_thread_cooperative()) {
#ifdef TME_THREADS_POSIX
    pthread_rwlock_unlock(&tme_rwlock_start);
#elif defined(TME_THREADS_GLIB)
    g_rw_lock_writer_unlock(&tme_rwlock_start);  
#endif
  }
  _tme_thread_suspended();
  /* Run the main loop */
  for(;!(*_tme_threads_run)(_tme_threads_arg););
}

void tme_thread_enter(tme_mutex_t *mutex) {
  if(!tme_thread_cooperative()) {
#ifdef TME_THREADS_POSIX
    pthread_rwlock_rdlock(&tme_rwlock_start);
#elif defined(TME_THREADS_GLIB)
    g_rw_lock_reader_lock(&tme_rwlock_start);
#endif
  }
  _tme_thread_resumed();  
  if(mutex)
    tme_mutex_lock(mutex);
}
