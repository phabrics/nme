/* nmesh/nmesh-threads.c - nmesh threads initialization: */

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

/* includes: */
#include <tme/threads.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* globals: */
static struct tme_threads_t {
  tme_threads_fn1 tme_threads_run;
  void *tme_threads_arg;
  tme_mutex_t *tme_threads_mutex;
  tme_time_t tme_threads_delay;
} tme_threads;
static tme_cond_t tme_cond_start;
static tme_time_t tme_cond_delay = TME_TIME_SET_SEC(5);

// Set main thread loop iteration function & argument
void tme_threads_set_main(tme_threads_fn1 run, void *arg, tme_mutex_t *mutex, tme_time_t delay) {
  tme_threads.tme_threads_run = run;
  tme_threads.tme_threads_arg = arg;
  tme_threads.tme_threads_mutex = mutex;
  tme_threads.tme_threads_delay = delay;
  //  tme_cond_notify(&tme_cond_start,TRUE);
}

int nmesh_init(int mode) {
  /* initialize the threading system: */
  tme_threads.tme_threads_run = (mode) ? (tme_threads_main_iter) : (tme_fiber_main_iter);
  tme_threads.tme_threads_arg = 0;
  tme_threads.tme_threads_mutex = NULL;
  tme_threads.tme_threads_delay = (mode) ? (TME_TIME_SET_SEC(10)) : (0);
  tme_threads_init(mode);

  fprintf(stderr, "Using %s threads.\n", (mode) ? (TME_THREADS_NAME) : "fiber");
  
  /* Synchronization primitive provided to allow sequential
     execution of pre-thread initialization code. It is used
     as the condition to start all threads. */

  tme_cond_init(&tme_cond_start);

  _tme_thread_resumed();
  return TME_OK;
}

_tme_thret tme_threads_run(void) {
  _tme_thread_suspended();
  tme_thread_enter(tme_threads.tme_threads_mutex);
  
  /* Run the main loop */
#ifdef __EMSCRIPTEN__
  if(thread_mode)
    // Receives a function to call and some user data to provide it.
    emscripten_request_animation_frame_loop(tme_threads.tme_threads_run, tme_threads.tme_threads_arg);
  else
#else
  if(tme_threads.tme_threads_run)
    for(;;) {
      (*tme_threads.tme_threads_run)(tme_threads.tme_threads_arg);
      if(tme_threads.tme_threads_delay)
	tme_thread_sleep_yield(tme_threads.tme_threads_delay, tme_threads.tme_threads_mutex);
    }
  else
    (*(tme_threads_fn)tme_threads.tme_threads_arg)();
#endif
  tme_thread_exit(tme_threads.tme_threads_mutex);
}
