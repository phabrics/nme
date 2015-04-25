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

extern pthread_rwlock_t tme_rwlock_suspere;

/* initializing and starting: */
#define tme_threads_init() pthread_rwlock_init(&tme_rwlock_suspere, NULL)

/* thread suspension: */
#define _tme_thread_suspended()	        pthread_rwlock_unlock(&tme_rwlock_suspere)
#define _tme_thread_resumed()	        pthread_rwlock_rdlock(&tme_rwlock_suspere)
#define tme_thread_suspend_others()	_tme_thread_suspended();pthread_rwlock_wrlock(&tme_rwlock_suspere)
#define tme_thread_resume_others()	pthread_rwlock_unlock(&tme_rwlock_suspere);_tme_thread_resumed()
#define tme_thread_enter()	        _tme_thread_resumed()

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
  
  _TME_TIME_SETV(t, TME_TIME_SEC(*timeout), TME_TIME_GET_FRAC(*timeout) * 1000, tv_sec, tv_nsec);
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
static _tme_inline int tme_thread_yield _TME_P((void)) {
  int rc;
  
  _tme_thread_suspended();

  rc = pthread_yield();

  _tme_thread_resumed();

  return rc;
}

#define tme_thread_join(id) pthread_join(id,NULL)
#define tme_thread_exit() _tme_thread_suspended();pthread_exit(NULL)

/* sleeping: */
static _tme_inline int tme_thread_sleep_yield _TME_P((unsigned long sec, unsigned long usec)) { 
  struct timespec timeout;
  int rc;
  
  for (; usec >= 1000000; sec++, usec -= 1000000);

  _TME_TIME_SETV(timeout,sec, usec * 1000,tv_sec,tv_nsec);

  _tme_thread_suspended();

  rc = nanosleep(&timeout, NULL);

  _tme_thread_resumed();

  return rc;
}

/* I/O: */
static _tme_inline ssize_t tme_thread_read _TME_P((int fd, void *buf, size_t count)) {
  int rc;
  
  _tme_thread_suspended();

  rc = read(fd, buf, count);

  _tme_thread_resumed();

  return rc;
}

static _tme_inline ssize_t tme_thread_write _TME_P((int fd, const void *buf, size_t count)) {
  int rc;
  
  _tme_thread_suspended();

  rc = write(fd, buf, count);

  _tme_thread_resumed();

  return rc;
}

#define tme_thread_read_yield tme_thread_read
#define tme_thread_write_yield tme_thread_write
