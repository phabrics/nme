/* $Id: threads.h,v 1.10 2010/06/05 19:36:35 fredette Exp $ */

/* tme/threads-fiber.h - header file for cooperative threads: */

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

/* note that our locking model never allows recursive locking. */

/* setjmp/longjmp threads are cooperative: */
//#define TME_THREADS_PREEMPTIVE		(FALSE)

#define tme_thread_cooperative() TRUE

/* initializing and starting: */
void tme_fiber_threads_init _TME_P((void));
#define _tme_threads_init tme_fiber_threads_init
#define tme_threads_main_iter tme_fiber_threads_main_iter
#define _tme_threads_main_iter tme_fiber_threads_main_iter

#define tme_thread_suspend_others()	do { } while (/* CONSTCOND */ 0)
#define tme_thread_resume_others()	do { } while (/* CONSTCOND */ 0)
#define _tme_thread_suspended()	do { } while (/* CONSTCOND */ 0)
#define _tme_thread_resumed()	do { } while (/* CONSTCOND */ 0)

/* if we want speed over lock debugging, we can compile very simple
   rwlock operations: */
#ifdef TME_NO_DEBUG_LOCKS
typedef int _tme_rwlock_t;
#define _tme_rwlock_init(l) ((l) = FALSE, TME_OK)
#define _tme_rwlock_rdlock(l) ((l) = TRUE, TME_OK)
#define _tme_rwlock_tryrdlock(l) ((l) ? TME_EBUSY : _tme_rwlock_rdlock(l))
#define _tme_rwlock_rdunlock(l) ((l) = FALSE, TME_OK)
#else  /* !TME_NO_DEBUG_LOCKS */   

/* debugging rwlocks: */
typedef struct tme_fiber_rwlock {

  /* nonzero iff the lock is locked: */
  int _tme_fiber_rwlock_locked;

  /* the file and line number of the last locker or unlocker: */
  _tme_const char *_tme_fiber_rwlock_file;
  unsigned long _tme_fiber_rwlock_line;
} _tme_rwlock_t;

/* lock operations: */
int tme_fiber_rwlock_init _TME_P((struct tme_fiber_rwlock *));
int tme_fiber_rwlock_lock _TME_P((struct tme_fiber_rwlock *, _tme_const char *, unsigned long, int));
int tme_fiber_rwlock_unlock _TME_P((struct tme_fiber_rwlock *, _tme_const char *, unsigned long));
#define _tme_rwlock_init(l) tme_fiber_rwlock_init(&l)
#if defined(__FILE__) && defined(__LINE__)
#define _tme_rwlock_rdlock(l) tme_fiber_rwlock_lock(&l, __FILE__, __LINE__, FALSE)
#define _tme_rwlock_tryrdlock(l) tme_fiber_rwlock_lock(&l, __FILE__, __LINE__, TRUE)
#define _tme_rwlock_rdunlock(l) tme_fiber_rwlock_unlock(&l, __FILE__, __LINE__)
#else  /* !defined(__FILE__) || !defined(__LINE__) */
#define _tme_rwlock_rdlock(l) tme_fiber_rwlock_lock(&l, NULL, 0, FALSE)
#define _tme_rwlock_tryrdlock(l) tme_fiber_rwlock_lock(&l, NULL, 0, TRUE)
#define _tme_rwlock_rdunlock(l) tme_fiber_rwlock_unlock(&l, NULL, 0)
#endif /* !defined(__FILE__) || !defined(__LINE__) */

#endif /* TME_NO_DEBUG_LOCKS */

/* since our thread model doesn't allow recursive locking, write locking
   is always the same as read locking: */
#define _tme_rwlock_wrlock _tme_rwlock_rdlock
#define _tme_rwlock_wrunlock _tme_rwlock_rdunlock
#define _tme_rwlock_trywrlock _tme_rwlock_tryrdlock

/* with cooperative threads, it doesn't make any sense to wait for locks: */
#define _tme_rwlock_timedrdlock(l, usec) _tme_rwlock_tryrdlock(l)
#define _tme_rwlock_timedwrlock(l, usec) _tme_rwlock_trywrlock(l)

/* mutexes.  we use a read/write lock to represent a mutex, and always
   lock it for writing.  we do *not* allow recursive locking: */
#define tme_mutex_t _tme_rwlock_t
#define tme_mutex_init(l) _tme_rwlock_init(*l)
#define _tme_mutex_lock(l) _tme_rwlock_wrlock(*l)
#define tme_mutex_trylock(l) _tme_rwlock_trywrlock(*l)
//#define tme_mutex_timedlock(t, usec) tme_mutex_trylock(t)
#define tme_mutex_unlock(l) _tme_rwlock_rdunlock(*l)

/* conditions: */
typedef int tme_cond_t;
#define tme_cond_init(x) do { } while (/* CONSTCOND */ 0)
void tme_fiber_cond_wait_yield _TME_P((tme_cond_t *, tme_mutex_t *));
void tme_fiber_cond_sleep_yield _TME_P((tme_cond_t *, tme_mutex_t *, const tme_time_t));
void tme_fiber_cond_notify _TME_P((tme_cond_t *, int));
#define tme_cond_wait_yield tme_fiber_cond_wait_yield
#define tme_cond_sleep_yield tme_fiber_cond_sleep_yield
#define tme_cond_notify tme_fiber_cond_notify

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(0)
#define TME_THREAD_DEADLOCK_SLEEP	abort

/* threads: */
struct tme_fiber_thread;
typedef void _tme_thret;
typedef _tme_thret (*tme_thread_t) _TME_P((void *));
int tme_fiber_threads_main_iter _TME_P((void *unused));
typedef struct tme_fiber_thread *tme_threadid_t, *_tme_threadid_t;
void tme_fiber_thread_create _TME_P((tme_threadid_t *, tme_thread_t, void *));
#define tme_thread_create tme_fiber_thread_create
void tme_fiber_yield _TME_P((void));
#define _tme_thread_yield tme_fiber_yield
#define tme_thread_join(id) do { } while (/* CONSTCOND */ 0)
void tme_fiber_exit _TME_P((tme_mutex_t *mutex));
#define tme_thread_exit tme_fiber_exit

  /* sleeping: */
typedef tme_time_t tme_timeout_t;
#define tme_thread_get_timeout(sleep, timeout) ((*timeout) = (sleep))
void tme_fiber_sleep_yield _TME_P((tme_time_t));
#define tme_thread_sleep tme_fiber_sleep_yield

/* time: */
tme_time_t tme_fiber_get_time _TME_P((void));
//#define tme_thread_get_time() tme_fiber_get_time()
extern int tme_fiber_thread_short;
#define tme_thread_long() do { tme_fiber_thread_short = FALSE; } while (/* CONSTCOND */ 0)

