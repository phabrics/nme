/* $Id: threads.h,v 1.10 2010/06/05 19:36:35 fredette Exp $ */

/* tme/threads-posix.h - header file for POSIX threads: */

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

#include <pthread.h>

/* setjmp/longjmp threads are cooperative: */
#define TME_THREADS_COOPERATIVE		(FALSE)

/* our errno convention: */
#define TME_EDEADLK		EDEADLK
#define TME_EBUSY		EBUSY
#define TME_THREADS_ERRNO(rc)	(rc)

/* initializing and starting: */
#define tme_threads_init() do { } while (/* CONSTCOND */ 0)

/* thread suspension: */
#define tme_thread_suspend_others()	do { } while (/* CONSTCOND */ 0)
#define tme_thread_resume_others()	do { } while (/* CONSTCOND */ 0)

/* if we want speed over lock debugging, we can compile very simple
   rwlock operations: */

typedef pthread_rwlock_t tme_rwlock_t;
#define tme_rwlock_init(l) pthread_rwlock_init(l, NULL)
#define tme_rwlock_destroy pthread_rwlock_destroy
#define tme_rwlock_rdlock pthread_rwlock_rdlock
#define tme_rwlock_tryrdlock pthread_rwlock_trylock
#define tme_rwlock_rdunlock pthread_rwlock_unlock
#define tme_rwlock_wrlock pthread_rwlock_wrlock
#define tme_rwlock_trywrlock pthread_rwlock_trywrlock
#define tme_rwlock_wrunlock pthread_rwlock_unlock

static inline int tme_rwlock_timedlock _TME_P((tme_rwlock_t *l, unsigned long sec, int write)) { 
  static tme_time_t now, timeout;

  TME_TIME_SETV(timeout, sec, 0);
  tme_get_time(&now);
  TME_TIME_INC(timeout, now);
  if (TME_TIME_GET_FRAC(timeout) >= 1000000) {
    TME_TIME_ADDV(timeout, 1, -1000000);
  }

  if (write)
    return pthread_rwlock_timedwrlock(l, &timeout);
  else
    return pthread_rwlock_timedrdlock(l, &timeout);
}

#define tme_rwlock_timedrdlock(l,sec) tme_rwlock_timedlock(l,sec,0)
#define tme_rwlock_timedwrlock(l,sec) tme_rwlock_timedlock(l,sec,1)

/* mutexes. */
typedef pthread_mutex_t tme_mutex_t;
#define tme_mutex_init(m) pthread_mutex_init(m,NULL)
#define tme_mutex_destroy pthread_mutex_destroy
#define tme_mutex_lock pthread_mutex_lock
#define tme_mutex_trylock pthread_mutex_trylock
#define tme_mutex_timedlock pthread_mutex_timedlock
#define tme_mutex_unlock pthread_mutex_unlock

/* conditions: */
typedef pthread_cond_t tme_cond_t;
#define tme_cond_init(c) pthread_cond_init(c,NULL)
#define tme_cond_destroy pthread_cond_destroy
#define tme_cond_wait_yield pthread_cond_wait

static inline int
tme_cond_sleep_yield _TME_P((tme_cond_t *cond, tme_mutex_t *mutex,
			     const tme_time_t *timeout)) {
  tme_time_t abstime;

  tme_get_time(&abstime);
  TME_TIME_INC(abstime, *timeout);
  if (TME_TIME_GET_FRAC(abstime) >= 1000000) {
    TME_TIME_ADDV(abstime, 1, -1000000);
  }

  return pthread_cond_timedwait(cond, mutex, &abstime);
}

#define tme_cond_notifyTRUE pthread_cond_broadcast
#define tme_cond_notifyFALSE pthread_cond_signal
#define tme_cond_notify(cond,bc) tme_cond_notify##bc(cond)

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(1000)
#define TME_THREAD_DEADLOCK_SLEEP	abort

/* threads: */
typedef void *(*tme_thread_t) _TME_P((void *));
typedef pthread_t tme_threadid_t;
#define tme_thread_create(t,f,a) pthread_create(t,NULL,f,a)
void tme_pthread_yield _TME_P((void));
#define tme_thread_yield() do { } while (/* CONSTCOND */ 0)
#define tme_thread_join(id) pthread_join(id,NULL)
#define tme_thread_exit pthread_exit(NULL)

/* sleeping: */
static inline int tme_thread_sleep_yield _TME_P((unsigned long sec, unsigned long usec)) { 
  static tme_time_t timeout;

  for (; usec >= 1000000; sec++, usec -= 1000000);

  TME_TIME_SETV(timeout, sec, usec);
  return nanosleep(&timeout, NULL); 
}

/* I/O: */
#define tme_thread_read read
#define tme_thread_write write
#define tme_thread_read_yield read
#define tme_thread_write_yield write

