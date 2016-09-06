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
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
/* pthreads can support cooperative threads by setting the appropriate parameters */
#define TME_THREADS_PREEMPTIVE		(TRUE)

/* our errno convention: */
#define TME_EDEADLK		EDEADLK
#define TME_EBUSY		EBUSY
#define TME_THREADS_ERRNO(rc)	(rc)

#ifdef HAVE_PTHREAD_SETSCHEDPARAM
static int tme_thread_cooperative() {
  int policy;
  struct sched_param param;
  if(!pthread_getschedparam(pthread_self(), &policy, &param))
    return (policy == SCHED_FIFO);
  return FALSE;
}
#else
#define tme_thread_cooperative() FALSE
#endif

/* initializing and starting: */
#define _tme_threads_init() pthread_rwlock_init(&tme_rwlock_suspere, NULL)

/* thread suspension: */
extern pthread_rwlock_t tme_rwlock_suspere;
#define _tme_thread_suspended()	        pthread_rwlock_unlock(&tme_rwlock_suspere)
#define _tme_thread_resumed()	        pthread_rwlock_rdlock(&tme_rwlock_suspere)
#define tme_thread_suspend_others()	_tme_thread_suspended();if(!tme_thread_cooperative()) pthread_rwlock_wrlock(&tme_rwlock_suspere)
#define tme_thread_resume_others()	if(!tme_thread_cooperative()) pthread_rwlock_unlock(&tme_rwlock_suspere);_tme_thread_resumed()

/* if we want speed over lock debugging, we can compile very simple
   rwlock operations: */

typedef pthread_rwlock_t tme_rwlock_t;
#define tme_rwlock_init(l) pthread_rwlock_init(l, NULL)
#define tme_rwlock_destroy pthread_rwlock_destroy

static _tme_inline int tme_rwlock_lock _TME_P((tme_rwlock_t *l, int write)) { 
  int rc;

  _tme_thread_suspended();
  
  if (write)
    rc = pthread_rwlock_wrlock(l);
  else
    rc = pthread_rwlock_rdlock(l);

  _tme_thread_resumed();

  return rc;
}

#define tme_rwlock_rdlock(l) tme_rwlock_lock(l,0)
#define tme_rwlock_tryrdlock pthread_rwlock_tryrdlock
#define tme_rwlock_rdunlock pthread_rwlock_unlock
#define tme_rwlock_wrlock(l) tme_rwlock_lock(l,1)
#define tme_rwlock_trywrlock pthread_rwlock_trywrlock
#define tme_rwlock_wrunlock pthread_rwlock_unlock

#ifdef HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK

static _tme_inline int tme_rwlock_timedlock _TME_P((tme_rwlock_t *l, unsigned long sec, int write)) { 
  struct timespec now, timeout;
  int rc;
  
  _TME_TIME_SETV(timeout, sec, 0, tv_sec, tv_nsec);
  clock_gettime(CLOCK_REALTIME, &now);
  _TME_TIME_INC(timeout, now, tv_sec, tv_nsec);

  _tme_thread_suspended();
  
  if (write)
    rc = pthread_rwlock_timedwrlock(l, &timeout);
  else
    rc = pthread_rwlock_timedrdlock(l, &timeout);

  _tme_thread_resumed();

  return rc;
}

#define tme_rwlock_timedrdlock(l,sec) tme_rwlock_timedlock(l,sec,0)
#define tme_rwlock_timedwrlock(l,sec) tme_rwlock_timedlock(l,sec,1)
#else
#define tme_rwlock_timedrdlock(l,sec) tme_rwlock_rdlock(l)
#define tme_rwlock_timedwrlock(l,sec) tme_rwlock_wrlock(l)
#endif

/* mutexes. */
typedef pthread_mutex_t tme_mutex_t;
#define tme_mutex_init(m) pthread_mutex_init(m,NULL)
#define tme_mutex_destroy pthread_mutex_destroy

static _tme_inline int tme_mutex_lock _TME_P((tme_mutex_t *m)) { 
  int rc;

  _tme_thread_suspended();
  
  rc = pthread_mutex_lock(m);

  _tme_thread_resumed();

  return rc;
}

#define tme_mutex_trylock pthread_mutex_trylock
#define tme_mutex_unlock pthread_mutex_unlock

#ifdef HAVE_PTHREAD_MUTEX_TIMEDLOCK
static _tme_inline int tme_mutex_timedlock _TME_P((tme_mutex_t *m, unsigned long sec)) { 
  struct timespec now, timeout;
  int rc;
  
  _TME_TIME_SETV(timeout, sec, 0, tv_sec, tv_nsec);
  clock_gettime(CLOCK_REALTIME, &now);
  _TME_TIME_INC(timeout, now, tv_sec, tv_nsec);

  _tme_thread_suspended();
  
  rc = pthread_mutex_timedlock(m, &timeout);

  _tme_thread_resumed();

  return rc;
}
#else
#define tme_mutex_timedlock(m,t) pthread_mutex_trylock(m)
#endif

/* conditions: */
typedef pthread_cond_t tme_cond_t;
#define tme_cond_init(c) pthread_cond_init(c,NULL)
#define tme_cond_destroy pthread_cond_destroy

static _tme_inline int
tme_cond_wait_yield _TME_P((tme_cond_t *cond, tme_mutex_t *mutex)) {
  int rc;
  
  _tme_thread_suspended();

  rc = pthread_cond_wait(cond, mutex);

  _tme_thread_resumed();

  return rc;
}

static _tme_inline int
tme_cond_sleep_yield _TME_P((tme_cond_t *cond, tme_mutex_t *mutex,
			     const tme_time_t *timeout)) {
  struct timespec abstime;
  struct timespec t;
  int rc;
  
  _TME_TIME_SETV(t, TME_TIME_GET_SEC(*timeout), TME_TIME_GET_FRAC(*timeout) * 1000, tv_sec, tv_nsec);
  clock_gettime(CLOCK_REALTIME, &abstime);
  _TME_TIME_INC(abstime, t, tv_sec, tv_nsec);
  if ((_TME_TIME_GET_FRAC(abstime,tv_nsec)/1000) >= 1000000) {
    _TME_TIME_ADDV(abstime, 1, -1000000 * 1000, tv_sec, tv_nsec);
  }

  _tme_thread_suspended();

  rc = pthread_cond_timedwait(cond, mutex, &abstime);

  _tme_thread_resumed();

  return rc;
}

#define tme_cond_notifyTRUE pthread_cond_broadcast
#define tme_cond_notifyFALSE pthread_cond_signal
#define tme_cond_notify(cond,bc) tme_cond_notify##bc(cond)

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(1000)
#define TME_THREAD_DEADLOCK_SLEEP	abort

/* threads: */
typedef void *_tme_thret;
typedef _tme_thret (*tme_thread_t) _TME_P((void *));
typedef pthread_t tme_threadid_t;
void tme_thread_set_defattr _TME_P((pthread_attr_t *attr));
pthread_attr_t *tme_thread_defattr _TME_P((void));
#define tme_thread_create(t,f,a) pthread_create(t,tme_thread_defattr(),f,a)
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
static _tme_inline void tme_thread_yield _TME_P((void)) {
  if(!tme_thread_cooperative()) return;
  
  _tme_thread_suspended();

  sched_yield();

  _tme_thread_resumed();
}
#else
#define tme_thread_yield() do { } while (/* CONSTCOND */ 0)
#endif

#define tme_thread_join(id) pthread_join(id,NULL)
#define tme_thread_exit() _tme_thread_suspended();return NULL

/* sleeping: */
static _tme_inline int tme_thread_sleep_yield _TME_P((unsigned long sec, unsigned long usec, tme_mutex_t *mutex)) { 
  struct timespec timeout;
  int rc;
  
  for (; usec >= 1000000; sec++, usec -= 1000000);

  _TME_TIME_SETV(timeout,sec, usec * 1000,tv_sec,tv_nsec);

  pthread_mutex_unlock(mutex);
  
  _tme_thread_suspended();

  rc = nanosleep(&timeout, NULL);

  pthread_mutex_lock(mutex);
  
  _tme_thread_resumed();

  return rc;
}

/* A default main iterator for use in the main thread loop */
static _tme_inline int tme_threads_main_iter _TME_P((void *usec)) {
  return usleep((usec) ? (unsigned long)usec : 1000000);
}

#define _tme_threads_main_iter(fn) fn()

#ifdef HAVE_CPUSET_CREATE
typedef cpuset_t *tme_cpuset_t;
typedef cpuid_t tme_cpuid_t;
#define tme_cpuset_init(c) c = cpuset_create()
#define tme_cpuset_destroy cpuset_destroy
#define tme_cpuset_zero cpuset_zero
#define tme_cpuset_set cpuset_set
#define tme_cpuset_clr cpuset_clr
#define tme_cpuset_isset cpuset_isset
#define tme_cpuset_size cpuset_size
#define tme_cpuset_ref(c) c
#elif defined(HAVE_CPUSET_T) || defined(HAVE_CPU_SET_T)
#ifdef HAVE_CPUSET_T
typedef cpuset_t tme_cpuset_t;
#else
typedef cpu_set_t tme_cpuset_t;
#endif
typedef int tme_cpuid_t;
#define tme_cpuset_init(c) CPU_ZERO(&c)
#define tme_cpuset_destroy(c) do { } while (/* CONSTCOND */ 0)
#define tme_cpuset_zero(c) CPU_ZERO(&c)
#define tme_cpuset_set(n,c) CPU_SET(n,&c)
#define tme_cpuset_clr(n,c) CPU_CLR(n,&c)
#define tme_cpuset_isset(n,c) CPU_ISSET(n,&c)
#define tme_cpuset_size(c) sizeof(tme_cpuset_t)
#define tme_cpuset_ref(c) &c
#endif
