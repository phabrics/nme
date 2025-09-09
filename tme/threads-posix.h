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

/* read/write locks. */
typedef pthread_rwlock_t tme_thread_rwlock_t;

#define tme_thread_rwlock_init(l) pthread_rwlock_init(l, NULL)
#define tme_thread_rwlock_destroy pthread_rwlock_destroy
#define tme_thread_rwlock_rdlock pthread_rwlock_rdlock
#define tme_thread_rwlock_wrlock pthread_rwlock_wrlock
#define tme_thread_rwlock_rdunlock pthread_rwlock_unlock
#define tme_thread_rwlock_wrunlock pthread_rwlock_unlock
#define tme_thread_rwlock_tryrdlock pthread_rwlock_tryrdlock
#define tme_thread_rwlock_trywrlock pthread_rwlock_trywrlock
#ifdef HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK
#define tme_thread_rwlock_timedrdlock pthread_rwlock_timedrdlock
#define tme_thread_rwlock_timedwrlock pthread_rwlock_timedwrlock
#endif

/* mutexes. */
typedef pthread_mutex_t tme_thread_mutex_t;
#define tme_thread_mutex_init(m) pthread_mutex_init(m,NULL)
#define tme_thread_mutex_destroy pthread_mutex_destroy
#define tme_thread_mutex_lock pthread_mutex_lock
#define tme_thread_mutex_trylock pthread_mutex_trylock
#define tme_thread_mutex_unlock pthread_mutex_unlock
#ifdef HAVE_PTHREAD_MUTEX_TIMEDLOCK
#define tme_thread_mutex_timedlock pthread_mutex_timedlock
#endif

/* conditions: */
typedef pthread_cond_t tme_thread_cond_t;
#define tme_thread_cond_init(c) pthread_cond_init(c,NULL)
#define tme_thread_cond_destroy pthread_cond_destroy
#define tme_thread_cond_wait pthread_cond_wait
#define tme_thread_cond_wait_until(c,m,t) pthread_cond_timedwait(c,m,&t)
#define tme_thread_cond_notifyTRUE pthread_cond_broadcast
#define tme_thread_cond_notifyFALSE pthread_cond_signal

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(1000)
#define TME_THREAD_DEADLOCK_SLEEP	abort

/* time: */
static _tme_inline tme_time_t tme_thread_time _TME_P((void)) {
#ifdef WIN32
  #define TME_FRAC_PER_SEC 10000000
  FILETIME filetime;
  ULARGE_INTEGER _time;
  GetSystemTimeAsFileTime(&filetime);
  _time.u.HighPart = filetime.dwHighDateTime;
  _time.u.LowPart = filetime.dwLowDateTime;
#ifdef TME_HAVE_INT64_T
  return _time.QuadPart;
#else
  return (_time.u.LowPart) | (_time.u.HighPart << 32);
#endif
#elif defined(USE_GETTIMEOFDAY) || !defined(_TME_HAVE_CLOCK_GETTIME)
#define TME_FRAC_PER_SEC 1000000
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return TME_TIME_SET_SEC(tv.tv_sec)
    +TME_TIME_SET_USEC(tv.tv_usec);
#else
#define TME_FRAC_PER_SEC 1000000000
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return TME_TIME_SET_SEC(ts.tv_sec)
    +TME_TIME_SET_NSEC(ts.tv_nsec);
#endif
}

typedef struct timespec tme_thread_time_t;

static _tme_inline void tme_thread_get_timeout(tme_time_t sleep, tme_thread_time_t *timeout, int abs) {
  if(abs) sleep += tme_thread_time();
  timeout->tv_sec = TME_TIME_GET_SEC(sleep);
  timeout->tv_nsec = TME_TIME_GET_NSEC(sleep % TME_FRAC_PER_SEC);
}

/* threads: */
typedef void *_tme_thret;
typedef _tme_thret (*tme_thread_t) _TME_P((void *));
typedef pthread_t _tme_threadid_t, tme_thread_threadid_t;

extern pthread_attr_t *attrp;

#ifdef HAVE_PTHREAD_SETSCHEDPARAM

static _tme_inline void tme_thread_set_defattr(pthread_attr_t *attr) {
  attrp=attr;
}

static _tme_inline pthread_attr_t *tme_thread_defattr() {
  return attrp;
}

static _tme_inline int _tme_thread_cooperative() {
  int policy;
  struct sched_param param;
  if(!pthread_getschedparam(pthread_self(), &policy, &param))
    return (policy == SCHED_FIFO);
  return FALSE;
}
#define tme_thread_cooperative() (!thread_mode || _tme_thread_cooperative())

#define _tme_thread_yield sched_yield

#else
#define tme_thread_set_defattr(attr)
#define tme_thread_defattr() NULL
#endif

static _tme_inline tme_thread_threadid_t
tme_thread_new _TME_P((const char *name, tme_thread_t func, void *arg)) {
  tme_thread_threadid_t thr;
  pthread_create(&thr,tme_thread_defattr(),func,arg);
#ifdef HAVE_PTHREAD_SETNAME_NP
  pthread_setname_np(thr,name);
#endif
  return thr;
}

#define tme_thread_join(id) pthread_join(id,NULL)
#define tme_thread_self pthread_self

/* sleeping: */
#define tme_thread_sleep(timeout) nanosleep(timeout, NULL)

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
