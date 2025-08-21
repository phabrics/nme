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

/* read/write locks. */
typedef GRWLock tme_thread_rwlock_t;

#define tme_thread_rwlock_init g_rw_lock_init
#define tme_thread_rwlock_destroy g_rw_lock_clear
#define tme_thread_rwlock_rdlock g_rw_lock_reader_lock
#define tme_thread_rwlock_wrlock g_rw_lock_writer_lock
#define tme_thread_rwlock_rdunlock g_rw_lock_reader_unlock
#define tme_thread_rwlock_wrunlock g_rw_lock_writer_unlock
#define tme_thread_rwlock_tryrdlock(l) (g_rw_lock_reader_trylock(l) ? (TME_OK) : (TME_EBUSY))
#define tme_thread_rwlock_trywrlock(l) (g_rw_lock_writer_trylock(l) ? (TME_OK) : (TME_EBUSY))

/* mutexes. */
typedef GMutex tme_thread_mutex_t;
#define tme_thread_mutex_init g_mutex_init
#define tme_thread_mutex_destroy g_mutex_clear
#define tme_thread_mutex_lock g_mutex_lock
#define tme_thread_mutex_trylock(m) (g_mutex_trylock(m) ? (TME_OK) : (TME_EBUSY))
#define tme_thread_mutex_unlock g_mutex_unlock

/* conditions: */
typedef GCond tme_thread_cond_t;
#define tme_thread_cond_init g_cond_init
#define tme_thread_cond_destroy g_cond_clear
#define tme_thread_cond_wait g_cond_wait
#define tme_thread_cond_wait_until(c,m,t) g_cond_wait_until(c,m,*t)
#define tme_thread_cond_notifyTRUE g_cond_broadcast
#define tme_thread_cond_notifyFALSE g_cond_signal

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(0)
#define TME_THREAD_DEADLOCK_SLEEP	abort

typedef tme_time_t tme_thread_time_t;

#define tme_thread_get_timeout(sleep, timeout) (*(timeout) = TME_TIME_GET_USEC(sleep))

/* threads: */
typedef gpointer _tme_thret;
typedef GThreadFunc tme_thread_t;
typedef GThread *tme_thread_threadid_t, *_tme_threadid_t;

#define tme_thread_make(t,n,f,a) ((t) = g_thread_new(n,f,a))
#define tme_thread_join g_thread_join
#define tme_thread_self g_thread_self
#define _tme_thread_yield() 

/* sleeping: */
#define tme_thread_sleep(t) g_usleep(*t)
