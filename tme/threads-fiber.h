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

/* initializing and starting: */
void tme_fiber_threads_init _TME_P((void));
//#define _tme_threads_main_iter tme_fiber_threads_main_iter

//#define tme_thread_suspend_others()	do { } while (/* CONSTCOND */ 0)
//#define tme_thread_resume_others()	do { } while (/* CONSTCOND */ 0)
//#define _tme_thread_suspended()	do { } while (/* CONSTCOND */ 0)
//#define _tme_thread_resumed()	do { } while (/* CONSTCOND */ 0)

/* if we want speed over lock debugging, we can compile very simple
   rwlock operations: */
#ifdef TME_NO_DEBUG_LOCKS
typedef int tme_fiber_rwlock_t;
#define tme_fiber_rwlock_init(l) (*(l) = FALSE, TME_OK)
#define tme_fiber_rwlock_rdlock(l) (*(l) = TRUE, TME_OK)
#define tme_fiber_rwlock_tryrdlock(l) (*(l) ? TME_EBUSY : tme_fiber_rwlock_rdlock(l))
#define tme_fiber_rwlock_rdunlock(l) (*(l) = FALSE, TME_OK)
#else  /* !TME_NO_DEBUG_LOCKS */   

/* debugging rwlocks: */
typedef struct tme_fiber_rwlock {

  /* nonzero iff the lock is locked: */
  int _tme_fiber_rwlock_locked;

  /* the file and line number of the last locker or unlocker: */
  _tme_const char *_tme_fiber_rwlock_file;
  unsigned long _tme_fiber_rwlock_line;
} tme_fiber_rwlock_t;

/* lock operations: */
int tme_fiber_rwlock_init _TME_P((tme_fiber_rwlock_t *));
int tme_fiber_rwlock_lock _TME_P((tme_fiber_rwlock_t *, _tme_const char *, unsigned long, int));
int tme_fiber_rwlock_unlock _TME_P((tme_fiber_rwlock_t *, _tme_const char *, unsigned long));
#if defined(__FILE__) && defined(__LINE__)
#define tme_fiber_rwlock_rdlock(l) tme_fiber_rwlock_lock(l, __FILE__, __LINE__, FALSE)
#define tme_fiber_rwlock_tryrdlock(l) tme_fiber_rwlock_lock(l, __FILE__, __LINE__, TRUE)
#define tme_fiber_rwlock_rdunlock(l) tme_fiber_rwlock_unlock(l, __FILE__, __LINE__)
#else  /* !defined(__FILE__) || !defined(__LINE__) */
#define tme_fiber_rwlock_rdlock(l) tme_fiber_rwlock_lock(l, NULL, 0, FALSE)
#define tme_fiber_rwlock_tryrdlock(l) tme_fiber_rwlock_lock(l, NULL, 0, TRUE)
#define tme_fiber_rwlock_rdunlock(l) tme_fiber_rwlock_unlock(l, NULL, 0)
#endif /* !defined(__FILE__) || !defined(__LINE__) */

#endif /* TME_NO_DEBUG_LOCKS */

/* since our thread model doesn't allow recursive locking, write locking
   is always the same as read locking: */
#define tme_fiber_rwlock_wrlock tme_fiber_rwlock_rdlock
#define tme_fiber_rwlock_wrunlock tme_fiber_rwlock_rdunlock
#define tme_fiber_rwlock_trywrlock tme_fiber_rwlock_tryrdlock

/* with cooperative threads, it doesn't make any sense to wait for locks: */
#define tme_fiber_rwlock_timedrdlock(l, usec) tme_fiber_rwlock_tryrdlock(l)
#define tme_fiber_rwlock_timedwrlock(l, usec) tme_fiber_rwlock_trywrlock(l)

/* mutexes.  we use a read/write lock to represent a mutex, and always
   lock it for writing.  we do *not* allow recursive locking: */
#define tme_fiber_mutex_t tme_fiber_rwlock_t
#define tme_fiber_mutex_init tme_fiber_rwlock_init
#define tme_fiber_mutex_lock tme_fiber_rwlock_wrlock
#define tme_fiber_mutex_trylock tme_fiber_rwlock_trywrlock
#define tme_fiber_mutex_timedlock(t, usec) tme_fiber_mutex_trylock(t)
#define tme_fiber_mutex_unlock tme_fiber_rwlock_rdunlock

/* conditions: */
typedef int tme_fiber_cond_t;
#define tme_fiber_cond_init(c) (*(c)=0,TME_OK)
void tme_fiber_cond_wait _TME_P((tme_fiber_cond_t *, tme_fiber_mutex_t *));
void tme_fiber_cond_wait_until _TME_P((tme_fiber_cond_t *, tme_fiber_mutex_t *, const tme_time_t));
void tme_fiber_cond_notify _TME_P((tme_fiber_cond_t *, int));
#define tme_fiber_cond_notifyTRUE(c) tme_fiber_cond_notify(c, TRUE)
#define tme_fiber_cond_notifyFALSE(c) tme_fiber_cond_notify(c, FALSE)

/* deadlock sleeping: */
//#define TME_THREAD_TIMEDLOCK		(0)
//#define TME_THREAD_DEADLOCK_SLEEP	abort

/* threads: */
typedef struct tme_fiber_thread tme_fiber_thread_t;
void tme_fiber_main_iter _TME_P((void *unused));
tme_fiber_thread_t *tme_fiber_new _TME_P((const char *, tme_thread_t, void *));
void tme_fiber_yield _TME_P((void));
#define tme_fiber_join(id) do { } while (/* CONSTCOND */ 0)
void tme_fiber_exit _TME_P((tme_fiber_mutex_t *mutex));

  /* sleeping: */
void tme_fiber_sleep _TME_P((tme_time_t));

/* time: */
tme_time_t tme_fiber_get_time _TME_P((void));
extern int tme_fiber_thread_short;
#define tme_thread_long() do { tme_fiber_thread_short = FALSE; } while (/* CONSTCOND */ 0)

