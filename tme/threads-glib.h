/* $Id: threads.h,v 1.10 2010/06/05 19:36:35 fredette Exp $ */

/* tme/threads-glib.h - header file for GLib threads: */

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
#include <glib.h>

#define TME_THREADS_PREEMPTIVE		(TRUE)

/* our errno convention: */
#define TME_EDEADLK		EDEADLK
#define TME_EBUSY		EBUSY
#define TME_THREADS_ERRNO(rc)	(rc)

#define tme_thread_cooperative() FALSE

/* initializing and starting: */
#define _tme_threads_init() g_rw_lock_init(&tme_rwlock_suspere)

/* thread suspension: */
extern GRWLock tme_rwlock_suspere;
#define _tme_thread_suspended()	        g_rw_lock_reader_unlock(&tme_rwlock_suspere)
#define _tme_thread_resumed()	        g_rw_lock_reader_lock(&tme_rwlock_suspere)
#define tme_thread_suspend_others()	_tme_thread_suspended();g_rw_lock_writer_lock(&tme_rwlock_suspere)
#define tme_thread_resume_others()	g_rw_lock_writer_unlock(&tme_rwlock_suspere);_tme_thread_resumed()

/* if we want speed over lock debugging, we can compile very simple
   rwlock operations: */

typedef struct tme_rwlock {
  GRWLock lock;
  GThread *writer;
} tme_rwlock_t;

#define tme_rwlock_init(l) g_rw_lock_init(&(l)->lock)
#define tme_rwlock_destroy(l) g_rw_lock_clear(&(l)->lock)
#define tme_rwlock_tryrdlock(l) (g_rw_lock_reader_trylock(&(l)->lock) ? (TME_OK) : (TME_EBUSY))
#define tme_rwlock_rdunlock(l) g_rw_lock_reader_unlock(&(l)->lock)
static _tme_inline int tme_rwlock_rdlock _TME_P((tme_rwlock_t *l)) {
  if((l)->writer == g_thread_self())
    // simulates deadlock return when current thread has the write lock
    return TME_EDEADLK;

  _tme_thread_suspended();
  g_rw_lock_reader_lock(&(l)->lock);
  _tme_thread_resumed();
  
  /* TODO: insert some kind of timer to interrupt at the end of the timeout */
  return TME_OK;  
}
#define tme_rwlock_timedrdlock(l,t) tme_rwlock_rdlock(l)

static _tme_inline int tme_rwlock_wrlock _TME_P((tme_rwlock_t *l)) {
  if((l)->writer == g_thread_self())
    // simulates deadlock return when current thread has the write lock
    return TME_EDEADLK;

  _tme_thread_suspended();
  g_rw_lock_writer_lock(&(l)->lock);
  _tme_thread_resumed();
  (l)->writer = g_thread_self();
  return TME_OK;
}
static _tme_inline int tme_rwlock_trywrlock _TME_P((tme_rwlock_t *l)) {
  if(!g_rw_lock_writer_trylock(&(l)->lock)) return TME_EBUSY;
  (l)->writer = g_thread_self();
  return TME_OK;
}
static _tme_inline int tme_rwlock_wrunlock _TME_P((tme_rwlock_t *l)) {
  (l)->writer = 0;
  g_rw_lock_writer_unlock(&(l)->lock);
  return TME_OK;
}
#define tme_rwlock_timedwrlock(l,t) tme_rwlock_wrlock(l)

/* mutexes. */
typedef GMutex tme_mutex_t;
#define tme_mutex_init g_mutex_init
#define tme_mutex_destroy g_mutex_clear
static _tme_inline void tme_mutex_lock _TME_P((tme_mutex_t *m)) { 
  _tme_thread_suspended();
  
  g_mutex_lock(m);

  _tme_thread_resumed();
}

#define tme_mutex_trylock(m) (g_mutex_trylock(m) ? (TME_OK) : (TME_EBUSY))
/* for now, define as trylock (same as timedlock with 0 wait) */
#define tme_mutex_timedlock(m,t) g_mutex_trylock(m)
#define tme_mutex_unlock g_mutex_unlock

/* conditions: */
typedef GCond tme_cond_t;
#define tme_cond_init g_cond_init
#define tme_cond_destroy g_cond_clear
static _tme_inline void
tme_cond_wait_yield _TME_P((tme_cond_t *cond, tme_mutex_t *mutex)) {
  _tme_thread_suspended();

  g_cond_wait(cond, mutex);

  _tme_thread_resumed();
}

static _tme_inline int
tme_cond_sleep_yield _TME_P((tme_cond_t *cond, tme_mutex_t *mutex,
			     const tme_time_t *timeout)) {
  gint64 end_time;
  int rc;

  end_time =  TME_TIME_GET_FRAC(*timeout)
    + TME_TIME_GET_SEC(*timeout) * G_USEC_PER_SEC
    + g_get_monotonic_time();
  
  _tme_thread_suspended();

  rc = g_cond_wait_until(cond, mutex, end_time);

  _tme_thread_resumed();

  return rc;
}
#define tme_cond_notifyTRUE g_cond_broadcast
#define tme_cond_notifyFALSE g_cond_signal
#define tme_cond_notify(cond,bc) tme_cond_notify##bc(cond)

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(0)
#define TME_THREAD_DEADLOCK_SLEEP	abort

/* threads: */
typedef gpointer _tme_thret;
typedef GThreadFunc tme_thread_t;
typedef GThread *tme_threadid_t;
static _tme_inline void tme_thread_create _TME_P((tme_threadid_t *t, tme_thread_t f, void *a)) {
  *t = g_thread_new(NULL,f,a);
}
#define tme_thread_yield() do { } while (/* CONSTCOND */ 0)
#define tme_thread_join g_thread_join
#define tme_thread_exit() _tme_thread_suspended();return NULL

/* sleeping: */
static _tme_inline int tme_thread_sleep_yield _TME_P((unsigned long sec, unsigned long usec, tme_mutex_t *mutex)) { 
  g_mutex_unlock(mutex);
  
  _tme_thread_suspended();

  g_usleep(usec + sec * G_USEC_PER_SEC);
  
  g_mutex_lock(mutex);

  _tme_thread_resumed();

  return 0;
}

/* A default main iterator for use in the main thread loop */
static _tme_inline int tme_threads_main_iter _TME_P((void *usec)) {
  g_usleep((usec) ? (unsigned long)usec : 1000000);
  return 0;
}

#define _tme_threads_main_iter(fn) fn()
