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

#define tme_thread_cooperative() FALSE

/* read/write locks. */
typedef GRWLock _tme_rwlock_t;

#define _tme_rwlock_init g_rw_lock_init
#define _tme_rwlock_destroy g_rw_lock_clear
#define _tme_rwlock_rdlock g_rw_lock_reader_lock
#define _tme_rwlock_wrlock g_rw_lock_writer_lock
#define _tme_rwlock_rdunlock g_rw_lock_reader_unlock
#define _tme_rwlock_wrunlock g_rw_lock_writer_unlock
#define _tme_rwlock_tryrdlock(l) (g_rw_lock_reader_trylock(l) ? (TME_OK) : (TME_EBUSY))
#define _tme_rwlock_trywrlock(l) (g_rw_lock_writer_trylock(l) ? (TME_OK) : (TME_EBUSY))

/* mutexes. */
typedef GMutex tme_mutex_t;
#define tme_mutex_init g_mutex_init
#define tme_mutex_destroy g_mutex_clear
#define _tme_mutex_lock g_mutex_lock
#define tme_mutex_trylock(m) (g_mutex_trylock(m) ? (TME_OK) : (TME_EBUSY))
#define tme_mutex_unlock g_mutex_unlock

/* conditions: */
typedef GCond tme_cond_t;
#define tme_cond_init g_cond_init
#define tme_cond_destroy g_cond_clear
#define tme_cond_wait g_cond_wait
#define tme_cond_wait_until g_cond_wait_until
#define tme_cond_notifyTRUE g_cond_broadcast
#define tme_cond_notifyFALSE g_cond_signal

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(0)
#define TME_THREAD_DEADLOCK_SLEEP	abort

typedef tme_time_t tme_timeout_t;

#define tme_thread_get_timeout(sleep, timeout) ((*timeout) = TME_TIME_GET_USEC(sleep))

/* threads: */
typedef gpointer _tme_thret;
typedef GThreadFunc tme_thread_t;
typedef GThread *tme_threadid_t;
static _tme_inline void tme_thread_create _TME_P((tme_threadid_t *t, tme_thread_t f, void *a)) {
  *t = g_thread_new(NULL,f,a);
}
#define tme_thread_join g_thread_join
#define tme_thread_self g_thread_self

/* sleeping: */
#define tme_thread_sleep g_usleep

/* A default main iterator for use in the main thread loop */
static _tme_inline void tme_threads_main_iter _TME_P((void *usec)) {
  //  g_usleep((usec) ? (uintptr_t)usec : 1000000);
}

#define _tme_threads_main_iter(fn) if(fn) fn()
//#define tme_thread_get_time() tme_get_time()
