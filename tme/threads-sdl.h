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

#define TME_THREADS_PREEMPTIVE		(TRUE)

#define tme_thread_cooperative() FALSE

/* read/write locks. */
typedef SDL_RWLock *_tme_rwlock_t;

#define _tme_rwlock_init(l) ((l) = SDL_CreateRWLock())
#define _tme_rwlock_destroy SDL_DestroyRWLock
#define _tme_rwlock_rdlock SDL_LockRWLockForReading
#define _tme_rwlock_wrlock SDL_LockRWLockForWriting
#define _tme_rwlock_rdunlock SDL_UnlockRWLock
#define _tme_rwlock_wrunlock SDL_UnlockRWLock
#define _tme_rwlock_tryrdlock(l) (SDL_TryLockRWLockForReading(l) ? (TME_OK) : (TME_EBUSY))
#define _tme_rwlock_trywrlock(l) (SDL_TryLockRWLockForWriting(l) ? (TME_OK) : (TME_EBUSY))

/* mutexes. */
typedef struct tme_mutex {
  SDL_Mutex *mutex;
} tme_mutex_t;

#define tme_mutex_init(m) ((m)->mutex = SDL_CreateMutex())
#define tme_mutex_destroy(m) SDL_DestroyMutex((m)->mutex)
#define _tme_mutex_lock(m) SDL_LockMutex((m)->mutex)
#define tme_mutex_trylock(m) (SDL_TryLockMutex((m)->mutex) ? (TME_OK) : (TME_EBUSY))
#define tme_mutex_unlock(m) SDL_UnlockMutex((m)->mutex)

/* conditions: */
typedef struct tme_cond {
  SDL_Condition *cond;
} tme_cond_t;

#define tme_cond_init(c) ((c)->cond = SDL_CreateCondition())
#define tme_cond_destroy(c) SDL_CreateCondition((c)->cond)
#define tme_cond_wait(c,m) SDL_WaitCondition((c)->cond,(m)->mutex)
#define tme_cond_wait_until(c,m,t) SDL_WaitConditionTimeout((c)->cond,(m)->mutex,t)
#define tme_cond_notifyTRUE(c) SDL_BroadcastCondition((c)->cond)
#define tme_cond_notifyFALSE(c) SDL_SignalCondition((c)->cond)

/* deadlock sleeping: */
#define TME_THREAD_TIMEDLOCK		(0)
#define TME_THREAD_DEADLOCK_SLEEP	abort

typedef tme_int32_t tme_timeout_t;

#define tme_thread_get_timeout(sleep, timeout) ((*timeout) = TME_TIME_GET_MSEC(sleep))

/* threads: */
typedef int *_tme_thret;
typedef SDL_ThreadFunction tme_thread_t;
typedef SDL_Thread *tme_threadid_t;
typedef SDL_ThreadID _tme_threadid_t;
static _tme_inline void tme_thread_create _TME_P((tme_threadid_t *t, tme_thread_t f, void *a)) {
  *t = SDL_CreateThread(f,NULL,a);
}

static _tme_inline int tme_thread_join _TME_P((tme_threadid_t t)) {
  int r;
  SDL_WaitThread(t,&r);
  return r;
}
#define tme_thread_self SDL_GetCurrentThreadID

/* sleeping: */
#define tme_thread_sleep SDL_Delay

/* A default main iterator for use in the main thread loop */
static _tme_inline void tme_threads_main_iter _TME_P((void *usec)) {
  //  g_usleep((usec) ? (uintptr_t)usec : 1000000);
}

#define _tme_threads_main_iter(fn) if(fn) fn()
//#define tme_thread_get_time() tme_get_time()
