/* tme/threads-sdl.h - header file for SDL3 threads: */

/*
 * Copyright (c) 2025 Ruben Agin
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
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_time.h>
#include <SDL3/SDL_timer.h>

#define TME_THREADS_PREEMPTIVE		(TRUE)

/* read/write locks. */
typedef SDL_RWLock *tme_thread_rwlock_t;

#define tme_thread_rwlock_init(l) (*(l) = SDL_CreateRWLock(),TME_OK)
#define tme_thread_rwlock_destroy(l) SDL_DestroyRWLock(*(l))
#define tme_thread_rwlock_rdlock(l) SDL_LockRWLockForReading(*(l))
#define tme_thread_rwlock_wrlock(l) SDL_LockRWLockForWriting(*(l))
#define tme_thread_rwlock_rdunlock(l) SDL_UnlockRWLock(*(l))
#define tme_thread_rwlock_wrunlock(l) SDL_UnlockRWLock(*(l))
#define tme_thread_rwlock_tryrdlock(l) (SDL_TryLockRWLockForReading(*(l)) ? (TME_OK) : (TME_EBUSY))
#define tme_thread_rwlock_trywrlock(l) (SDL_TryLockRWLockForWriting(*(l)) ? (TME_OK) : (TME_EBUSY))

/* mutexes. */
typedef SDL_Mutex *tme_thread_mutex_t;

#define tme_thread_mutex_init(m) (*(m) = SDL_CreateMutex(),TME_OK)
#define tme_thread_mutex_destroy(m) SDL_DestroyMutex(*(m))
#define tme_thread_mutex_lock(m) SDL_LockMutex(*(m))
#define tme_thread_mutex_trylock(m) (SDL_TryLockMutex(*(m)) ? (TME_OK) : (TME_EBUSY))
#define tme_thread_mutex_unlock(m) SDL_UnlockMutex(*(m))

/* conditions: */
typedef SDL_Condition *tme_thread_cond_t;

#define tme_thread_cond_init(c) (*(c) = SDL_CreateCondition(),TME_OK)
#define tme_thread_cond_destroy(c) SDL_CreateCondition(*(c))
#define tme_thread_cond_wait(c,m) SDL_WaitCondition(*(c),*(m))
#define tme_thread_cond_wait_until(c,m,t) SDL_WaitConditionTimeout(*(c),*(m),t)
#define tme_thread_cond_notifyTRUE(c) SDL_BroadcastCondition(*(c))
#define tme_thread_cond_notifyFALSE(c) SDL_SignalCondition(*(c))

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(0)
#define TME_THREAD_DEADLOCK_SLEEP	abort

/* time: */
#define TME_FRAC_PER_SEC SDL_NS_PER_SECOND

static _tme_inline tme_time_t tme_thread_time _TME_P((void)) {
  SDL_Time ticks;
  SDL_GetCurrentTime(&ticks);
  return ticks;
}

typedef tme_int32_t tme_thread_time_t;

/* ignore abs argument as we SDL API assumes relative time: */
#define tme_thread_get_timeout(sleep, timeout, abs) (*(timeout) = TME_TIME_GET_MSEC(sleep))

/* threads: */
typedef int *_tme_thret;
typedef SDL_ThreadFunction tme_thread_t;
typedef SDL_Thread *tme_thread_threadid_t;
typedef SDL_ThreadID _tme_threadid_t;

#define tme_thread_new(n,f,a) SDL_CreateThread(f,n,a)

static _tme_inline int tme_thread_join _TME_P((tme_thread_threadid_t t)) {
  int r;
  SDL_WaitThread(t,&r);
  return r;
}
#define tme_thread_self SDL_GetCurrentThreadID

/* sleeping: */
#define tme_thread_sleep SDL_Delay
