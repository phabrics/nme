/* $Id: threads-fiber.c,v 1.18 2010/06/05 19:10:28 fredette Exp $ */

/* libtme/threads-fiber.c - implementation of cooperative threads: */

/*
 * Copyright (c) 2003 Matt Fredette, 2014-2016 Ruben Agin
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

#include <tme/common.h>
_TME_RCSID("$Id: threads-fiber.c,v 1.18 2010/06/05 19:10:28 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <tme/events.h>
#include <stdlib.h>
#if defined(__EMSCRIPTEN__) && !defined(USE_SJLJ)
#include <emscripten/fiber.h>
#elif !defined(WIN32)
#define USE_SJLJ
#endif
#ifdef USE_SJLJ
#include <setjmp.h>
#endif

/* thread states: */
#define TME_FIBER_THREAD_STATE_BLOCKED		(1)
#define TME_FIBER_THREAD_STATE_RUNNABLE		(2)
#define TME_FIBER_THREAD_STATE_DISPATCHING	(3)
#define TME_NUM_EVENTS 1024
#define TME_NUM_EVFLAGS 3

/* types: */

/* a thread: */
struct tme_fiber_thread {

  const char *name;
  
  /* the all-threads list: */
  struct tme_fiber_thread *next;
  struct tme_fiber_thread **prev;

  /* the current state of the thread, and any state-related list that
     it is on: */
  int tme_fiber_thread_state;
  struct tme_fiber_thread *state_next;
  struct tme_fiber_thread **state_prev;

#if defined(__EMSCRIPTEN__) && !defined(USE_SJLJ)
  emscripten_fiber_t tme_fiber_context;
  char tme_fiber_asyncify_stack[1024];
  char tme_fiber_c_stack[4096]  __attribute__((aligned(16)));
#else
  /* the thread function: */
  tme_thread_t tme_fiber_thread_func;
  void *tme_fiber_thread_func_private;
#endif  

  /* any condition that this thread is waiting on: */
  tme_fiber_cond_t *tme_fiber_thread_cond;

  /* any events that this thread is waiting on: */
  struct tme_fiber_event_set *tme_fiber_thread_events;
  
  /* if nonzero, the amount of time that this thread is sleeping,
     followed by the time the sleep will timeout.  all threads with
     timeouts are kept on a sorted list: */
  tme_time_t tme_fiber_thread_sleep;
  tme_time_t tme_fiber_thread_timeout;
  struct tme_fiber_thread *timeout_next;
  struct tme_fiber_thread **timeout_prev;

  /* the last dispatch number for this thread: */
  tme_uint32_t tme_fiber_thread_dispatch_number;
};

/* event envelope */
struct tme_fiber_event {
  event_t event;
  unsigned int flags;
  void *arg;
};

/* a collection of events */
struct tme_fiber_event_set {
  struct event_set *es;
  int max_event;
  unsigned int flags;
  struct tme_fiber_event events[0];
};

/* thread blocked on an event */
struct tme_fiber_event_arg {
  tme_fiber_thread_t *thread;
  void *arg;
};

/* globals: */

/* the all-threads list: */
static tme_fiber_thread_t *tme_fiber_threads_all;

/* the timeout-threads list: */
static tme_fiber_thread_t *tme_fiber_threads_timeout;

/* the runnable-threads list: */
static tme_fiber_thread_t *tme_fiber_threads_runnable;

/* the dispatching-threads list: */
static tme_fiber_thread_t *tme_fiber_threads_dispatching;

/* the active thread: */
static tme_fiber_thread_t *tme_fiber_thread_active;

/* this is set if the active thread is exiting: */
static int tme_fiber_thread_exiting;

/* the fiber function interface to the platform-specific implementations: */

static inline void tme_fiber_convert(tme_fiber_thread_t *thread) {
#ifdef USE_SJLJ
  thread->tme_fiber_thread_func = NULL;
#elif defined(__EMSCRIPTEN__)
  emscripten_fiber_init_from_current_context(&thread->tme_fiber_context,
					     thread->tme_fiber_asyncify_stack,
					     sizeof(thread->tme_fiber_asyncify_stack));
#elif defined(WIN32)
  thread->tme_fiber_thread_func = ConvertThreadToFiber(NULL);
#endif
}

static inline void tme_fiber_create(tme_fiber_thread_t *thread,
				    tme_thread_t func,
				    void *func_private) {
#ifdef USE_SJLJ
  thread->tme_fiber_thread_func_private = func_private;
  thread->tme_fiber_thread_func = func;
#elif defined(__EMSCRIPTEN__)
  emscripten_fiber_init(&thread->tme_fiber_context,
			func,
			func_private,
			thread->tme_fiber_c_stack,
			sizeof(thread->tme_fiber_c_stack),
			thread->tme_fiber_asyncify_stack,
			sizeof(thread->tme_fiber_asyncify_stack));
#elif defined(WIN32)
  thread->tme_fiber_thread_func = CreateFiber(0,
					      func,
					      func_private);
#endif
}

#ifdef USE_SJLJ
/* this is a jmp_buf back to the dispatcher: */
static jmp_buf tme_fiber_dispatcher_jmp;
#define tme_fiber_switch(old_thread, new_thread)
#else
static inline void tme_fiber_switch(tme_fiber_thread_t *old_thread,
				    tme_fiber_thread_t *new_thread) {
#ifdef __EMSCRIPTEN__
  emscripten_fiber_swap(&old_thread->tme_fiber_context,
			&new_thread->tme_fiber_context);
#elif defined(WIN32)
  SwitchToFiber(new_thread->tme_fiber_thread_func);
#endif
}
#endif

/* the main loop events: */
static struct tme_fiber_event_set *tme_fiber_main_events;

/* this dummy thread structure is filled before a yield to represent
   what, if anything, the active thread is blocking on when it yields: */
static tme_fiber_thread_t tme_fiber_thread_blocked;

/* the dispatch number: */
static tme_uint32_t _tme_fiber_thread_dispatch_number;

/* a reasonably current time: */
static tme_time_t _tme_fiber_now;

/* if nonzero, the last dispatched thread ran for only a short time: */
int tme_fiber_thread_short;

/* this returns a reasonably current time: */
tme_time_t
tme_fiber_get_time()
{

  /* if we need to, call tme_fiber_get_time(): */
  if (__tme_predict_false(!tme_fiber_thread_short)) {
    _tme_fiber_now = tme_thread_time();
    tme_fiber_thread_short = TRUE;
  }

  /* return the reasonably current time: */
  return _tme_fiber_now;
}

/* this changes a thread's state: */
static void
_tme_fiber_change_state(tme_fiber_thread_t *thread, int state)
{
  tme_fiber_thread_t **_thread_prev;
  tme_fiber_thread_t *thread_next;

  /* the active thread is the only thread that can become blocked.
     the active thread cannot become runnable or dispatching: */
  assert (state == TME_FIBER_THREAD_STATE_BLOCKED
	  ? (thread->state_next == tme_fiber_thread_active)
	  : (thread != tme_fiber_thread_active));

  /* if the thread's current state is not BLOCKED: */
  _thread_prev = thread->state_prev;
  if (_thread_prev != NULL) {

    /* remove it from that list: */
    thread_next = thread->state_next;
    *_thread_prev = thread_next;
    if (thread_next != NULL) {
      thread_next->state_prev = _thread_prev;
    }

    /* this thread is now on no list: */
    thread->state_prev = NULL;
    thread->state_next = NULL;
  }

  /* if the thread's desired state is not BLOCKED: */
  if (state != TME_FIBER_THREAD_STATE_BLOCKED) {

    /* this thread must be runnable, or this thread must be
       dispatching before threads are being dispatched: */
    assert (state == TME_FIBER_THREAD_STATE_RUNNABLE
	    || (state == TME_FIBER_THREAD_STATE_DISPATCHING
		&& tme_fiber_thread_active == NULL));

    /* if threads are being dispatched, and this thread wasn't already
       in this dispatch: */
    if (tme_fiber_thread_active != NULL
	&& thread->tme_fiber_thread_dispatch_number != _tme_fiber_thread_dispatch_number) {

      /* add this thread to the dispatching list after the current
	 thread: */
      _thread_prev = &tme_fiber_thread_active->state_next;
    }

    /* otherwise, if this thread is dispatching: */
    else if (state == TME_FIBER_THREAD_STATE_DISPATCHING) {

      /* add this thread to the dispatching list at the head: */
      _thread_prev = &tme_fiber_threads_dispatching;
    }

    /* otherwise, this thread is runnable: */
    else {

      /* add this thread to the runnable list at the head: */
      _thread_prev = &tme_fiber_threads_runnable;
    }

    /* add this thread to the list: */
    thread_next = *_thread_prev;
    *_thread_prev = thread;
    thread->state_prev = _thread_prev;
    thread->state_next = thread_next;
    if (thread_next != NULL) {
      thread_next->state_prev = &thread->state_next;
    }

    /* all nonblocked threads appear to be runnable: */
    state = TME_FIBER_THREAD_STATE_RUNNABLE;
  }

  /* set the new state of the thread: */
  thread->tme_fiber_thread_state = state;
}

/* this moves the runnable list to the dispatching list: */
static void
_tme_fiber_threads_dispatching_runnable(void)
{
  tme_fiber_thread_t *threads_dispatching;

  /* the dispatching list must be empty: */
  assert (tme_fiber_threads_dispatching == NULL);

  /* move the runnable list to the dispatching list: */
  threads_dispatching = tme_fiber_threads_runnable;
  tme_fiber_threads_runnable = NULL;
  tme_fiber_threads_dispatching = threads_dispatching;
  if (threads_dispatching != NULL) {
    threads_dispatching->state_prev = &tme_fiber_threads_dispatching;
  }
}

/* this moves all threads that have timed out to the dispatching list: */
static void
_tme_fiber_threads_dispatching_timeout(void)
{
  tme_time_t now;
  tme_fiber_thread_t *thread_timeout;

  /* get the current time: */
  now = tme_fiber_get_time();

  /* loop over the timeout list: */
  for (thread_timeout = tme_fiber_threads_timeout;
       thread_timeout != NULL;
       thread_timeout = thread_timeout->timeout_next) {

    /* if this timeout has not expired: */
    if (thread_timeout->tme_fiber_thread_timeout <= now)
      /* move this thread to the dispatching list: */
      _tme_fiber_change_state(thread_timeout, TME_FIBER_THREAD_STATE_DISPATCHING);
    else if(tme_fiber_thread_blocked.tme_fiber_thread_timeout <= now ||
	    thread_timeout->tme_fiber_thread_timeout <
	    tme_fiber_thread_blocked.tme_fiber_thread_timeout)
      /* set the minimum timeout to wait: */
      tme_fiber_thread_blocked.tme_fiber_thread_timeout =
	thread_timeout->tme_fiber_thread_timeout;
  }
}

/* this moves all threads with the given event flags to
   the dispatching list: */
static void
_tme_fiber_threads_dispatching_event(struct tme_fiber_event_arg *event_arg,
				    unsigned int flags)
{
  int i;
  
  for(i=0;i<TME_NUM_EVFLAGS;i++) {
    if(flags & (1<<i)) {
      _tme_fiber_change_state(event_arg->thread, TME_FIBER_THREAD_STATE_DISPATCHING);
    }
    event_arg++;
  }
}

/* this dispatches all dispatching threads: */
static void
tme_fiber_dispatch(volatile int passes)
{
  tme_fiber_thread_t * volatile thread;
  tme_fiber_thread_t **_thread_timeout_prev;
  tme_fiber_thread_t *thread_timeout_next;
  tme_fiber_thread_t *thread_other;
  volatile int _passes;
  int rc_one;

  /* dispatch the given number of passes over the dispatching threads: */
  for (_passes = passes; _passes-- > 0; ) {
    for (tme_fiber_thread_active = tme_fiber_threads_dispatching;
	 (thread = tme_fiber_thread_active) != NULL; ) {

      /* if this thread is on the timeout list: */
      _thread_timeout_prev = thread->timeout_prev;
      assert ((_thread_timeout_prev != NULL)
	      == (thread->tme_fiber_thread_sleep != 0));
      if (_thread_timeout_prev != NULL) {

	/* remove this thread from the timeout list: */
	thread_timeout_next = thread->timeout_next;
	*_thread_timeout_prev = thread_timeout_next;
	if (thread_timeout_next != NULL) {
	  thread_timeout_next->timeout_prev = _thread_timeout_prev;
	}

	/* this thread is no longer on the timeout list: */
	thread->timeout_prev = NULL;
	thread->timeout_next = NULL;
      }

      /* set the dispatch number on this thread: */
      thread->tme_fiber_thread_dispatch_number = _tme_fiber_thread_dispatch_number;
      
      /* when this active thread yields, we'll return here, where we
	 will continue the inner dispatching loop: */
#ifdef USE_SJLJ
      rc_one = setjmp(tme_fiber_dispatcher_jmp);
      if (rc_one) {
	continue;
      }

      /* run this thread.  if it happens to return, just call
         tme_fiber_exit(): */
      (*thread->tme_fiber_thread_func)(thread->tme_fiber_thread_func_private);
      //      tme_fiber_exit();
#else
      tme_fiber_switch(&tme_fiber_thread_blocked, thread);
#endif
    }
  }

  /* if there are still dispatching threads, move them en masse to the
     runnable list: */
  thread = tme_fiber_threads_dispatching;
  if (thread != NULL) {
    thread_other = tme_fiber_threads_runnable;
    thread->state_prev = &tme_fiber_threads_runnable;
    tme_fiber_threads_runnable = thread;
    tme_fiber_threads_dispatching = NULL;
    for (;; thread = thread->state_next) {
      if (thread->state_next == NULL) {
	thread->state_next = thread_other;
	if (thread_other != NULL) {
	  thread_other->state_prev = &thread->state_next;
	}
	break;
      }
    }
  }

  /* the next dispatch will use the next number: */
  _tme_fiber_thread_dispatch_number++;
}

/* this is the main loop iteration function: */
int
tme_fiber_main_iter(void *unused)
{
  int fd;
  struct timeval timeout;
  int rc;
  struct event_set_return esr[64];
  
  /* make the select timeout: */

  /* if the timeout list is empty: */
  if (tme_fiber_threads_timeout == NULL) {

    /* assume that we will block in select indefinitely.  there must
       either be runnable threads (in which case we will not block
       at all in select), or we must be selecting on file
       descriptors: */
    timeout.tv_sec = BIG_TIMEOUT;
    timeout.tv_usec = 0;
    assert (tme_fiber_threads_runnable != NULL
	    || tme_fiber_main_events->max_event >= 0);
  }

  /* otherwise, the timeout list is not empty: */
  else if(tme_fiber_thread_blocked.tme_fiber_thread_timeout >
	  (now = tme_fiber_get_time())) {
    now = tme_fiber_thread_blocked.tme_fiber_thread_timeout - now;
    /* set the timeout time: */
    timeout.tv_sec = TME_TIME_GET_SEC(now);
    timeout.tv_usec = TME_TIME_GET_USEC(now % TME_FRAC_PER_SEC);
  }

  /* if there are runnable threads, make this a poll: */
  if (tme_fiber_threads_runnable != NULL) {
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
  }
  
  rc = (tme_fiber_main_events->max_event >= 0) ?
    (event_wait(tme_fiber_main_events->es, &timeout, esr, SIZE(esr))) :
    (tme_fiber_main_events->max_event);
  
  /* we were in select() for an unknown amount of time: */
  tme_thread_long();

  /* move all runnable threads to the dispatching list: */
  _tme_fiber_threads_dispatching_runnable();

  /* move all threads that have timed out to the dispatching list: */
  _tme_fiber_threads_dispatching_timeout();

  /* if some fds are ready, dispatch them: */
  if (rc > 0) {	
    for (fd = 0; fd < rc; ++fd) {
      const struct event_set_return *e = &esr[fd];
      
      if (e->rwflags != 0) {

	/* move all threads that match the conditions on this file
	   descriptor to the dispatching list: */
	_tme_fiber_threads_dispatching_event((struct tme_fiber_event_arg *)e->arg, e->rwflags);

      }
    }
  }
    
  /* dispatch: */
  tme_fiber_dispatch(1);

  return TME_OK;
}

/* this creates a new thread: */
tme_fiber_thread_t *
tme_fiber_new(const char *name, tme_thread_t func, void *func_private)
{
  tme_fiber_thread_t *thread;

  /* allocate a new thread and put it on the all-threads list: */
  thread = tme_new(tme_fiber_thread_t, 1);
  thread->name = tme_strdup(name);
  thread->prev = &tme_fiber_threads_all;
  thread->next = *thread->prev;
  *thread->prev = thread;
  if (thread->next != NULL) {
    thread->next->prev = &thread->next;
  }

  /* initialize the thread: */
  tme_fiber_create(thread, func, func_private);
  thread->tme_fiber_thread_cond = NULL;
  thread->tme_fiber_thread_events = NULL;
  thread->tme_fiber_thread_sleep = 0;
  thread->timeout_prev = NULL;

  /* make this thread runnable: */
  thread->tme_fiber_thread_state = TME_FIBER_THREAD_STATE_BLOCKED;
  thread->state_prev = NULL;
  thread->state_next = NULL;
  thread->tme_fiber_thread_dispatch_number = _tme_fiber_thread_dispatch_number - 1;
  _tme_fiber_change_state(thread,
			 TME_FIBER_THREAD_STATE_RUNNABLE);
  return thread;
}

/* this makes a thread wait on a condition: */
void
tme_fiber_cond_wait(tme_fiber_cond_t *cond, tme_fiber_mutex_t *mutex)
{

  /* unlock the mutex: */
  tme_fiber_mutex_unlock(mutex);

  /* remember that this thread is waiting on this condition: */
  tme_fiber_thread_blocked.tme_fiber_thread_cond = cond;

  /* yield: */
  tme_fiber_yield();

  /* lock the mutex: */
  tme_fiber_mutex_lock(mutex);
}

/* this makes a thread sleep on a condition: */
void
tme_fiber_cond_wait_until(tme_fiber_cond_t *cond, tme_fiber_mutex_t *mutex, const tme_time_t sleep)
{

  /* remember that this thread is waiting on this condition: */
  tme_fiber_thread_blocked.tme_fiber_thread_cond = cond;

  /* sleep and yield: */
  tme_thread_sleep_yield(sleep, mutex);
}

/* this notifies one or more threads waiting on a condition: */
void
tme_fiber_cond_notify(tme_fiber_cond_t *cond, int broadcast)
{
  tme_fiber_thread_t *thread;

  for (thread = tme_fiber_threads_all;
       thread != NULL;
       thread = thread->next) {
    if (thread->tme_fiber_thread_state == TME_FIBER_THREAD_STATE_BLOCKED
	&& thread->tme_fiber_thread_cond == cond) {
      
      /* this thread is runnable: */
      _tme_fiber_change_state(thread,
			     TME_FIBER_THREAD_STATE_RUNNABLE);

      /* if we're not broadcasting this notification, stop now: */
      if (!broadcast) {
	break;
      }
    }
  }
}

/* this sleeps and yields: */
void
tme_fiber_sleep(tme_time_t time)
{
  tme_fiber_thread_blocked.tme_fiber_thread_sleep = time;

  /* yield: */
  tme_fiber_yield();

}

/* the fiber event methods: */

static struct tme_fiber_event_set *tme_fiber_event_set_init(int *maxevents, unsigned int flags) {
  struct tme_fiber_event_set *tes;
  struct event_set *es;
  es = event_set_init(maxevents, flags);
  tes = (struct tme_fiber_event_set *) tme_malloc(sizeof(struct tme_fiber_event_set) + sizeof(struct tme_fiber_event) * (*maxevents));
  tes->es = es;
  tes->max_event = -1;
  tes->flags = flags;
  return tes;
}

static void tme_fiber_event_free(struct tme_fiber_event_set *es) {
  if(es->es)
    event_free(es->es);
  tme_free(es);
}

static void tme_fiber_event_reset(struct tme_fiber_event_set *es)
{
  if(es->es)
    event_reset(es->es);
  es->max_event = -1;
}

static int tme_fiber_event_del(struct tme_fiber_event_set *es, event_t event)
{
  int i, rc;

  if(es->es)
    event_del(es->es, event);
  for(i=0;i<=es->max_event;i++)
    if(es->events[i].event == event) break;
  if(i > es->max_event)
    return -1;
  es->events[i].event = UNDEFINED_EVENT;
  rc = i;
  while(i == es->max_event) {
    if(es->events[i--].event == UNDEFINED_EVENT)
      es->max_event--;
    if(i<0) break;
  }
  return rc;
}

static int tme_fiber_event_ctl(struct tme_fiber_event_set *es, event_t event, unsigned int rwflags, void *arg) {
  int i = -1, min_event;
  
  if(es->es)
    event_ctl(es->es, event, rwflags, arg);
  min_event = es->max_event + 1;
  if(!(es->flags & EVENT_METHOD_FAST))
    for(i=es->max_event;i>=0;i--) {
      if(es->events[i].event == event) break;
      if(es->events[i].event == UNDEFINED_EVENT)
	min_event = i;
    }
  if(i < 0) {
    i = min_event;
    es->events[i].event = event;
  }
  es->events[i].flags = rwflags;
  es->events[i].arg = arg;
  if(i > es->max_event)
    es->max_event++;
  return i;
}

/* this selects and yields: */
static int
tme_fiber_event_wait(struct tme_fiber_event_set *es, const struct timeval *timeout_in, struct event_set_return *out, int outlen, tme_mutex_t *mutex)
{
  struct timeval timeout_out;
  int rc;
  
  /* do a polling select: */
  timeout_out.tv_sec = 0;
  timeout_out.tv_usec = 0;
  rc = event_wait(es->es, &timeout_out, out, outlen);
  tme_thread_long();
  if(rc > 0 ||
     timeout_in != NULL &&
     timeout_in->tv_sec == 0 &&
     timeout_in->tv_usec == 0) {
    return (rc);
  }

  /* unlock the mutex: */
  if(mutex) tme_fiber_mutex_unlock(&mutex->fiber);
  tme_fiber_thread_blocked.tme_fiber_thread_events = es;

  tme_fiber_thread_blocked.tme_fiber_thread_sleep =
    (timeout_in != NULL) ?
    TME_TIME_SET_SEC(timeout_in->tv_sec) + TME_TIME_SET_USEC(timeout_in->tv_usec) :
    TME_TIME_SET_SEC(BIG_TIMEOUT);
  
  /* yield: */
  tme_fiber_yield();

  /* lock the mutex: */
  if(mutex) tme_fiber_mutex_lock(&mutex->fiber);
  return event_wait(es->es, &timeout_out, out, outlen);
}

static unsigned int
tme_fiber_tun_set (struct tuntap *tt,
		   struct tme_fiber_event_set *es,
		   unsigned int rwflags,
		   void *arg,
		   unsigned int *persistent)
{
  struct event_set *_es = es->es;
  unsigned int rc = tun_set(tt, _es, rwflags, (void*)0, NULL);

  es->es = 0;
  tme_fiber_event_ctl(es, tun_event_handle(tt), rwflags, 0);
  es->es = _es;
  return rc;
}

static unsigned int
tme_fiber_socket_set (struct link_socket *s,
		      struct tme_fiber_event_set *es,
		      unsigned int rwflags,
		      void *arg,
		      unsigned int *persistent)
{
  struct event_set *_es = es->es;
  unsigned int rc = socket_set(s, _es, rwflags, (void*)0, NULL);

  es->es = 0;
  tme_fiber_event_ctl(es, socket_event_handle(s), rwflags, 0);
  es->es = _es;
  return rc;
}

/* this initializes the threads system: */
void
tme_fiber_threads_init()
{
  int num = TME_NUM_EVENTS;
  
  /* there are no threads: */
  tme_fiber_threads_all = NULL;
  tme_fiber_threads_timeout = NULL;
  tme_fiber_threads_runnable = NULL;
  tme_fiber_threads_dispatching = NULL;
  tme_fiber_thread_active = NULL;
  tme_fiber_thread_exiting = FALSE;

  /* no threads are waiting on any fds: */
  tme_fiber_main_events = tme_fiber_event_set_init(&num, 0);
  
  /* initialize the thread-blocked structure: */
  tme_fiber_convert(&tme_fiber_thread_blocked);
  tme_fiber_thread_blocked.tme_fiber_thread_cond = NULL;
  tme_fiber_thread_blocked.tme_fiber_thread_events = NULL;
  tme_fiber_thread_blocked.tme_fiber_thread_sleep = 0;
  tme_fiber_thread_blocked.tme_fiber_thread_timeout = 0;

  /* initialize the runtime event callback handlers: */
  tme_event_set_init = tme_fiber_event_set_init;
  tme_event_free = tme_fiber_event_free;
  tme_event_reset = tme_fiber_event_reset;
  tme_event_del = tme_fiber_event_del;
  tme_event_ctl = tme_fiber_event_ctl;
  tme_event_wait = tme_fiber_event_wait;
  tme_tun_set = tme_fiber_tun_set;
  tme_socket_set = tme_fiber_socket_set;
}

/* this exits a thread: */
void
tme_fiber_exit(tme_fiber_mutex_t *mutex)
{
  
  /* mark that this thread is exiting: */
  tme_fiber_thread_exiting = TRUE;

  if(mutex)
    tme_fiber_mutex_unlock(mutex);

  /* yield: */
  tme_fiber_yield();
}
       
/* this yields the current thread: */
void
tme_fiber_yield(void)
{
  tme_fiber_thread_t *thread;
  int blocked;
  tme_fiber_thread_t **_thread_prev;
  tme_fiber_thread_t *thread_other;
  struct tme_fiber_event_arg *event_arg;
  struct tme_fiber_event_set *es, *es2 = NULL;
  int i, j, changed;
  
  /* get the active thread: */
  thread = tme_fiber_thread_active;

  /* the thread ran for an unknown amount of time: */
  tme_thread_long();

  /* assume that this thread is not blocked: */
  blocked = FALSE;

  /* see if this thread is blocked on a condition: */
  if (tme_fiber_thread_blocked.tme_fiber_thread_cond != NULL) {
    blocked = TRUE;
  }
  thread->tme_fiber_thread_cond = tme_fiber_thread_blocked.tme_fiber_thread_cond;
  tme_fiber_thread_blocked.tme_fiber_thread_cond = NULL;

  /* see if this thread is blocked on any events: */
  es = tme_fiber_thread_blocked.tme_fiber_thread_events;

  if(es) {
    j = es->max_event + 1;
    es2 = tme_fiber_event_set_init(&j, 0);
    for(i=0;i<=es->max_event;i++) {
      if(es->events[i].event == UNDEFINED_EVENT)
	continue;
      j = -1;
      changed = FALSE;
      if(thread->tme_fiber_thread_events)
	j = tme_fiber_event_del(thread->tme_fiber_thread_events, es->events[i].event);
      if(j<0) {
	changed = TRUE;
	j = tme_fiber_event_del(tme_fiber_main_events, es->events[i].event);
	if(j<0)
	  j = tme_fiber_event_ctl(tme_fiber_main_events,
				 es->events[i].event,
				 es->events[i].flags,
				 tme_new0(struct tme_fiber_event_arg, TME_NUM_EVFLAGS));
	else
	  tme_fiber_main_events->events[j].event = es->events[i].event;
      } else {
	if(es->events[i].flags != thread->tme_fiber_thread_events->events[j].flags) {
	  changed = TRUE;
	  tme_fiber_main_events->events[(uintptr_t)thread->tme_fiber_thread_events->events[j].arg].flags
	    &= ~thread->tme_fiber_thread_events->events[j].flags;
	}
	j = (uintptr_t)thread->tme_fiber_thread_events->events[j].arg;
      }
      tme_fiber_event_ctl(es2,
			  es->events[i].event,
			  es->events[i].flags,
			  (void *)(uintptr_t)j);			      
      if(changed) {
	tme_fiber_main_events->events[j].flags |= es->events[i].flags;
	event_ctl(tme_fiber_main_events->es,
		  es->events[i].event,
		  tme_fiber_main_events->events[j].flags,
		  tme_fiber_main_events->events[j].arg);
	event_arg = (struct tme_fiber_event_arg *)tme_fiber_main_events->events[j].arg;
	for(j=0;j<TME_NUM_EVFLAGS;j++) {
	  if(es->events[i].flags & (1<<j)) {
	    event_arg->thread = thread;
	    event_arg->arg = es->events[i].arg;
	  }
	  event_arg++;
	}
      }
    }
    blocked = i;
#ifdef USE_SJLJ
    tme_fiber_event_free(es);
#endif
  }

  if(thread->tme_fiber_thread_events) {
    for(i=0;i<=thread->tme_fiber_thread_events->max_event;i++) {
      j = (uintptr_t)thread->tme_fiber_thread_events->events[i].arg;
      if(tme_fiber_main_events->events[j].flags &= ~thread->tme_fiber_thread_events->events[i].flags)
	event_ctl(tme_fiber_main_events->es,
		  tme_fiber_main_events->events[j].event,
		  tme_fiber_main_events->events[j].flags,
		  tme_fiber_main_events->events[j].arg);
      else {
	event_del(tme_fiber_main_events->es,
		  tme_fiber_main_events->events[j].event);
	tme_fiber_main_events->events[j].event = UNDEFINED_EVENT;
	tme_free(tme_fiber_main_events->events[j].arg);
	while(j == tme_fiber_main_events->max_event) {
	  if(tme_fiber_main_events->events[j--].event == UNDEFINED_EVENT)
	    tme_fiber_main_events->max_event--;
	  if(j<0) break;
	}
      }
    }
    tme_fiber_event_free(thread->tme_fiber_thread_events);
  }
  
  thread->tme_fiber_thread_events = es2;
  tme_fiber_thread_blocked.tme_fiber_thread_events = NULL;
  
  /* see if this thread is blocked for some amount of time: */
  if (tme_fiber_thread_blocked.tme_fiber_thread_sleep != 0) {

    //    assert(TME_TIME_GET_FRAC(tme_fiber_thread_blocked.tme_fiber_thread_sleep) < 1000000);
    blocked = TRUE;

    /* set the timeout for this thread: */
    thread->tme_fiber_thread_timeout = tme_fiber_get_time()
      + tme_fiber_thread_blocked.tme_fiber_thread_sleep;
    
    /* insert this thread into the timeout list: */
    assert (thread->timeout_prev == NULL);
    for (_thread_prev = &tme_fiber_threads_timeout;
	 (thread_other = *_thread_prev) != NULL;
	 _thread_prev = &thread_other->timeout_next) {
      if (thread_other->tme_fiber_thread_timeout >
	  thread->tme_fiber_thread_timeout) {
	break;
      }
    }
    *_thread_prev = thread;
    thread->timeout_prev = _thread_prev;
    thread->timeout_next = thread_other;
    if (thread_other != NULL) {
      thread_other->timeout_prev = &thread->timeout_next;
    }
  }
  thread->tme_fiber_thread_sleep = tme_fiber_thread_blocked.tme_fiber_thread_sleep;
  tme_fiber_thread_blocked.tme_fiber_thread_sleep = 0;

  /* if this thread is actually exiting, it must appear to be
     runnable, and it only isn't because it's exiting: */
  if (tme_fiber_thread_exiting) {
    assert(!blocked);
    blocked = TRUE;
  }

  /* make any following thread on the runnable list the next active
     thread: */
  tme_fiber_thread_active = thread->state_next;

  /* if this thread is blocked, move it to the blocked list: */
  if (blocked) {
    _tme_fiber_change_state(thread, 
			   TME_FIBER_THREAD_STATE_BLOCKED);

    /* if this thread is exiting: */
    if (tme_fiber_thread_exiting) {

      /* remove this thread from the all-threads list: */
      *thread->prev = thread->next;
      if (thread->next != NULL) {
	thread->next->prev = thread->prev;
      }

      /* free this thread: */
      tme_free(thread);

      /* nothing is exiting any more: */
      tme_fiber_thread_exiting = FALSE;
    }
  }

  /* jump back to the dispatcher: */
#ifdef USE_SJLJ
  longjmp(tme_fiber_dispatcher_jmp, TRUE);
#else
  tme_fiber_switch(thread, &tme_fiber_thread_blocked);
#endif
}

#ifndef TME_NO_DEBUG_LOCKS

/* lock operations: */
int
tme_fiber_rwlock_init(struct tme_fiber_rwlock *lock)
{
  /* initialize the lock: */
  lock->_tme_fiber_rwlock_locked = FALSE;
  lock->_tme_fiber_rwlock_file = NULL;
  lock->_tme_fiber_rwlock_line = 0;
  return (TME_OK);
}
int
tme_fiber_rwlock_lock(struct tme_fiber_rwlock *lock, _tme_const char *file, unsigned long line, int try)
{
  
  /* if this lock is already locked: */
  if (lock->_tme_fiber_rwlock_locked) {
    if (try) {
      return (TME_EDEADLK);
    }
    abort();
  }

  /* lock the lock: */
  lock->_tme_fiber_rwlock_locked = TRUE;
  lock->_tme_fiber_rwlock_file = file;
  lock->_tme_fiber_rwlock_line = line;
  return (TME_OK);
}
int
tme_fiber_rwlock_unlock(struct tme_fiber_rwlock *lock, _tme_const char *file, unsigned long line)
{
  
  /* if this lock isn't locked: */
  if (!lock->_tme_fiber_rwlock_locked) {
    abort();
  }

  /* unlock the lock: */
  lock->_tme_fiber_rwlock_locked = FALSE;
  lock->_tme_fiber_rwlock_file = file;
  lock->_tme_fiber_rwlock_line = line;
  return (TME_OK);
}

#endif /* !TME_NO_DEBUG_LOCKS */
