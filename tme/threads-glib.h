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

/* setjmp/longjmp threads are cooperative: */
#define TME_THREADS_COOPERATIVE		(FALSE)

/* our errno convention: */
#define TME_EDEADLK		EDEADLK
#define TME_EBUSY		EBUSY
#define TME_THREADS_ERRNO(rc)	(rc)

/* initializing and starting: */
#define tme_threads_init(x) do { } while (/* CONSTCOND */ 0)
#ifdef _TME_HAVE_GTK
void tme_threads_gtk_init _TME_P((void));
#endif
void tme_threads_run _TME_P((void));

/* thread suspension: */
#define tme_thread_suspend_others()	do { } while (/* CONSTCOND */ 0)
#define tme_thread_resume_others()	do { } while (/* CONSTCOND */ 0)

/* if we want speed over lock debugging, we can compile very simple
   rwlock operations: */

typedef GRWLock tme_rwlock_t;
#define tme_rwlock_init g_rw_lock_init
#define tme_rwlock_destroy g_rw_lock_clear
#define tme_rwlock_rdlock g_rw_lock_reader_lock
#define tme_rwlock_tryrdlock g_rw_lock_reader_trylock
#define tme_rwlock_rdunlock g_rw_lock_reader_unlock
#define tme_rwlock_wrlock g_rw_lock_writer_lock
#define tme_rwlock_trywrlock g_rw_lock_writer_trylock
#define tme_rwlock_wrunlock g_rw_lock_writer_unlock
/* for now, define as trylock (same as timedlock with 0 wait) */
#define tme_rwlock_timedrdlock g_rw_lock_reader_trylock
#define tme_rwlock_timedwrlock g_rw_lock_writer_trylock

/* mutexes. */
typedef GMutex tme_mutex_t;
#define tme_mutex_init g_mutex_init
#define tme_mutex_destroy g_mutex_clear
#define tme_mutex_lock g_mutex_lock
#define tme_mutex_trylock g_mutex_trylock
/* for now, define as trylock (same as timedlock with 0 wait) */
#define tme_mutex_timedlock g_mutex_trylock
#define tme_mutex_unlock g_mutex_unlock

/* conditions: */
typedef GCond tme_cond_t;
#define tme_cond_init g_cond_init
#define tme_cond_destroy g_cond_clear
#define tme_cond_wait_yield g_cond_wait
#define tme_cond_sleep_yield g_cond_wait_until
#define tme_cond_notify1 g_cond_broadcast
#define tme_cond_notify0 g_cond_signal
#define tme_cond_notify(cond,bc) tme_cond_notify##bc(cond)

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(0)
#define TME_THREAD_DEADLOCK_SLEEP	abort

/* threads: */
typedef void (*tme_thread_t) _TME_P((void *));
typedef GThread tme_threadid_t;
extern tme_threadid_t *tme_tid;
static inline void tme_thread_create _TME_P((tme_thread_t f, void *a)) {
  tme_tid = g_thread_new(NULL,f,a,NULL);
}
#define tme_thread_yield do { } while (/* CONSTCOND */ 0)
#define tme_thread_exit g_thread_exit(NULL)

/* sleeping: */
static inline void tme_thread_sleep_yield _TME_P((unsigned long sec, unsigned long usec)) { 
  for (; sec; sec--, usec += 1000000);

  g_usleep(usec);
}

/* I/O: */
#define tme_thread_read_yield read
#define tme_thread_write_yield write
