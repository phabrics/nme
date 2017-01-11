/* $Id: threads-sjlj.c,v 1.18 2010/06/05 19:10:28 fredette Exp $ */

/* libtme/threads-sjlj.c - implementation of setjmp/longjmp threads: */

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
_TME_RCSID("$Id: threads-sjlj.c,v 1.18 2010/06/05 19:10:28 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <stdlib.h>
#ifndef WIN32
#include <setjmp.h>
#endif

/* thread states: */
#define TME_SJLJ_THREAD_STATE_BLOCKED		(1)
#define TME_SJLJ_THREAD_STATE_RUNNABLE		(2)
#define TME_SJLJ_THREAD_STATE_DISPATCHING	(3)
#define TME_NUM_EVENTS 1024
#define TME_NUM_EVFLAGS 3

/* types: */

/* a thread: */
struct tme_sjlj_thread {

  /* the all-threads list: */
  struct tme_sjlj_thread *next;
  struct tme_sjlj_thread **prev;

  /* the current state of the thread, and any state-related list that
     it is on: */
  int tme_sjlj_thread_state;
  struct tme_sjlj_thread *state_next;
  struct tme_sjlj_thread **state_prev;

  /* the thread function: */
#ifndef WIN32
  void *tme_sjlj_thread_func_private;
#endif
  tme_thread_t tme_sjlj_thread_func;

  /* any condition that this thread is waiting on: */
  tme_cond_t *tme_sjlj_thread_cond;

  /* any events that this thread is waiting on: */
  struct tme_sjlj_event_set *tme_sjlj_thread_events;
  
  /* if nonzero, the amount of time that this thread is sleeping,
     followed by the time the sleep will timeout.  all threads with
     timeouts are kept on a sorted list: */
  tme_time_t tme_sjlj_thread_sleep;
  tme_time_t tme_sjlj_thread_timeout;
  struct tme_sjlj_thread *timeout_next;
  struct tme_sjlj_thread **timeout_prev;

  /* the last dispatch number for this thread: */
  tme_uint32_t tme_sjlj_thread_dispatch_number;
};

/* event envelope */
struct tme_sjlj_event {
  event_t event;
  unsigned int flags;
  void *arg;
};

/* a collection of events */
struct tme_sjlj_event_set {
  struct event_set *es;
  int max_event;
  unsigned int flags;
  struct tme_sjlj_event events[0];
};

/* thread blocked on an event */
struct tme_sjlj_event_arg {
  struct tme_sjlj_thread *thread;
  void *arg;
};

/* globals: */

/* the all-threads list: */
static struct tme_sjlj_thread *tme_sjlj_threads_all;

/* the timeout-threads list: */
static struct tme_sjlj_thread *tme_sjlj_threads_timeout;

/* the runnable-threads list: */
static struct tme_sjlj_thread *tme_sjlj_threads_runnable;

/* the dispatching-threads list: */
static struct tme_sjlj_thread *tme_sjlj_threads_dispatching;

/* the active thread: */
static struct tme_sjlj_thread *tme_sjlj_thread_active;

/* this is set if the active thread is exiting: */
static int tme_sjlj_thread_exiting;

#ifndef WIN32
/* this is a jmp_buf back to the dispatcher: */
static jmp_buf tme_sjlj_dispatcher_jmp;
#endif

/* the main loop events: */
static struct tme_sjlj_event_set *tme_sjlj_main_events;

/* this dummy thread structure is filled before a yield to represent
   what, if anything, the active thread is blocking on when it yields: */
static struct tme_sjlj_thread tme_sjlj_thread_blocked;

/* the dispatch number: */
static tme_uint32_t _tme_sjlj_thread_dispatch_number;

/* a reasonably current time: */
static tme_time_t _tme_sjlj_now;

/* if nonzero, the last dispatched thread ran for only a short time: */
int tme_sjlj_thread_short;

/* this initializes the threads system: */
void
tme_sjlj_threads_init()
{
  int num = TME_NUM_EVENTS;
  
  /* there are no threads: */
  tme_sjlj_threads_all = NULL;
  tme_sjlj_threads_timeout = NULL;
  tme_sjlj_threads_runnable = NULL;
  tme_sjlj_threads_dispatching = NULL;
  tme_sjlj_thread_active = NULL;
  tme_sjlj_thread_exiting = FALSE;

  /* no threads are waiting on any fds: */
  tme_sjlj_main_events = tme_sjlj_event_set_init(&num, 0);
  
  /* initialize the thread-blocked structure: */
#ifdef WIN32
  tme_sjlj_thread_blocked.tme_sjlj_thread_func = ConvertThreadToFiber(NULL);
#endif
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = NULL;
  tme_sjlj_thread_blocked.tme_sjlj_thread_events = NULL;
  TME_TIME_SETV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, 0, 0);
}

/* this returns a reasonably current time: */
void
tme_sjlj_gettimeofday(tme_time_t *now)
{

  /* if we need to, call tme_get_time(): */
  if (__tme_predict_false(!tme_sjlj_thread_short)) {
    tme_get_time(&_tme_sjlj_now);
    tme_sjlj_thread_short = TRUE;
  }

  /* return the reasonably current time: */
  *now = _tme_sjlj_now;
}

/* this changes a thread's state: */
static void
_tme_sjlj_change_state(struct tme_sjlj_thread *thread, int state)
{
  struct tme_sjlj_thread **_thread_prev;
  struct tme_sjlj_thread *thread_next;

  /* the active thread is the only thread that can become blocked.
     the active thread cannot become runnable or dispatching: */
  assert (state == TME_SJLJ_THREAD_STATE_BLOCKED
	  ? (thread->state_next == tme_sjlj_thread_active)
	  : (thread != tme_sjlj_thread_active));

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
  if (state != TME_SJLJ_THREAD_STATE_BLOCKED) {

    /* this thread must be runnable, or this thread must be
       dispatching before threads are being dispatched: */
    assert (state == TME_SJLJ_THREAD_STATE_RUNNABLE
	    || (state == TME_SJLJ_THREAD_STATE_DISPATCHING
		&& tme_sjlj_thread_active == NULL));

    /* if threads are being dispatched, and this thread wasn't already
       in this dispatch: */
    if (tme_sjlj_thread_active != NULL
	&& thread->tme_sjlj_thread_dispatch_number != _tme_sjlj_thread_dispatch_number) {

      /* add this thread to the dispatching list after the current
	 thread: */
      _thread_prev = &tme_sjlj_thread_active->state_next;
    }

    /* otherwise, if this thread is dispatching: */
    else if (state == TME_SJLJ_THREAD_STATE_DISPATCHING) {

      /* add this thread to the dispatching list at the head: */
      _thread_prev = &tme_sjlj_threads_dispatching;
    }

    /* otherwise, this thread is runnable: */
    else {

      /* add this thread to the runnable list at the head: */
      _thread_prev = &tme_sjlj_threads_runnable;
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
    state = TME_SJLJ_THREAD_STATE_RUNNABLE;
  }

  /* set the new state of the thread: */
  thread->tme_sjlj_thread_state = state;
}

/* this moves the runnable list to the dispatching list: */
static void
_tme_sjlj_threads_dispatching_runnable(void)
{
  struct tme_sjlj_thread *threads_dispatching;

  /* the dispatching list must be empty: */
  assert (tme_sjlj_threads_dispatching == NULL);

  /* move the runnable list to the dispatching list: */
  threads_dispatching = tme_sjlj_threads_runnable;
  tme_sjlj_threads_runnable = NULL;
  tme_sjlj_threads_dispatching = threads_dispatching;
  if (threads_dispatching != NULL) {
    threads_dispatching->state_prev = &tme_sjlj_threads_dispatching;
  }
}

/* this moves all threads that have timed out to the dispatching list: */
static void
_tme_sjlj_threads_dispatching_timeout(void)
{
  tme_time_t now;
  struct tme_sjlj_thread *thread_timeout;

  /* get the current time: */
  tme_gettimeofday(&now);

  /* loop over the timeout list: */
  for (thread_timeout = tme_sjlj_threads_timeout;
       thread_timeout != NULL;
       thread_timeout = thread_timeout->timeout_next) {

    /* if this timeout has not expired: */
    if (TME_TIME_GT(thread_timeout->tme_sjlj_thread_timeout, now)) {
      break;
    }

    /* move this thread to the dispatching list: */
    _tme_sjlj_change_state(thread_timeout, TME_SJLJ_THREAD_STATE_DISPATCHING);
  }
}

/* this moves all threads with the given event flags to
   the dispatching list: */
static void
_tme_sjlj_threads_dispatching_event(struct tme_sjlj_event_arg *event_arg,
				    unsigned int flags)
{
  int i;
  
  for(i=0;i<TME_NUM_EVFLAGS;i++) {
    if(flags & (1<<i)) {
      _tme_sjlj_change_state(event_arg->thread, TME_SJLJ_THREAD_STATE_DISPATCHING);
    }
    event_arg++;
  }
}

/* this makes the timeout time: */
static void
_tme_sjlj_timeout_time(struct timeval *timeout)
{
  tme_time_t now;
  struct tme_sjlj_thread *thread_timeout;
  tme_int32_t usecs;
  unsigned long secs;
  unsigned long secs_other;

  /* get the current time: */
  tme_gettimeofday(&now);

  /* the timeout list must not be empty: */
  thread_timeout = tme_sjlj_threads_timeout;
  assert (thread_timeout != NULL);

  /* subtract the now microseconds from the timeout microseconds: */
  assert (TME_TIME_GET_FRAC(thread_timeout->tme_sjlj_thread_timeout) < 1000000);
  usecs = TME_TIME_GET_FRAC(thread_timeout->tme_sjlj_thread_timeout);
  assert (TME_TIME_GET_FRAC(now) < 1000000);
  usecs -= TME_TIME_GET_FRAC(now);

  /* make any seconds carry: */
  secs_other = (usecs < 0);
  if (usecs < 0) {
    usecs += 1000000;
  }

  /* if the earliest timeout has already timed out: */
  secs_other += TME_TIME_GET_SEC(now);
  secs = TME_TIME_GET_SEC(thread_timeout->tme_sjlj_thread_timeout);
  if (__tme_predict_false(secs_other > secs
			  || ((secs -= secs_other) == 0
			      && usecs == 0))) {

    /* this thread is runnable: */
    _tme_sjlj_change_state(thread_timeout, TME_SJLJ_THREAD_STATE_RUNNABLE);

    /* make this a poll: */
    secs = 0;
    usecs = 0;
  }

  /* return the timeout time: */
  timeout->tv_sec = secs;
  timeout->tv_usec = usecs;
}

/* this dispatches all dispatching threads: */
static void
tme_sjlj_dispatch(volatile int passes)
{
  struct tme_sjlj_thread * volatile thread;
  struct tme_sjlj_thread **_thread_timeout_prev;
  struct tme_sjlj_thread *thread_timeout_next;
  struct tme_sjlj_thread *thread_other;
  volatile int _passes;
  int rc_one;

  /* dispatch the given number of passes over the dispatching threads: */
  for (_passes = passes; _passes-- > 0; ) {
    for (tme_sjlj_thread_active = tme_sjlj_threads_dispatching;
	 (thread = tme_sjlj_thread_active) != NULL; ) {

      /* if this thread is on the timeout list: */
      _thread_timeout_prev = thread->timeout_prev;
      assert ((_thread_timeout_prev != NULL)
	      == (!TME_TIME_EQV(thread->tme_sjlj_thread_sleep, 0, 0)));
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
      thread->tme_sjlj_thread_dispatch_number = _tme_sjlj_thread_dispatch_number;
      
      /* when this active thread yields, we'll return here, where we
	 will continue the inner dispatching loop: */
#ifdef WIN32
      SwitchToFiber(thread->tme_sjlj_thread_func);
#else
      rc_one = setjmp(tme_sjlj_dispatcher_jmp);
      if (rc_one) {
	continue;
      }

      /* run this thread.  if it happens to return, just call
         tme_sjlj_exit(): */
      (*thread->tme_sjlj_thread_func)(thread->tme_sjlj_thread_func_private);
      tme_sjlj_exit();
#endif
    }
  }

  /* if there are still dispatching threads, move them en masse to the
     runnable list: */
  thread = tme_sjlj_threads_dispatching;
  if (thread != NULL) {
    thread_other = tme_sjlj_threads_runnable;
    thread->state_prev = &tme_sjlj_threads_runnable;
    tme_sjlj_threads_runnable = thread;
    tme_sjlj_threads_dispatching = NULL;
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
  _tme_sjlj_thread_dispatch_number++;
}

/* this is the main loop iteration function: */
int
tme_sjlj_threads_main_iter(void *event_check)
{
  int fd;
  struct timeval timeout;
  int rc;
  struct event_set_return esr[64];
  
  if(tme_sjlj_threads_all == NULL) return -1;

  if(event_check)
    (*(tme_threads_fn)event_check)();
  
  /* make the select timeout: */

  /* if the timeout list is empty: */
  if (tme_sjlj_threads_timeout == NULL) {

    /* assume that we will block in select indefinitely.  there must
       either be runnable threads (in which case we will not block
       at all in select), or we must be selecting on file
       descriptors: */
    timeout.tv_sec = BIG_TIMEOUT;
    timeout.tv_usec = 0;
    assert (tme_sjlj_threads_runnable != NULL
	    || tme_sjlj_main_events->max_event >= 0);
  }

  /* otherwise, the timeout list is not empty: */
  else {
    _tme_sjlj_timeout_time(&timeout);
  }

  /* if there are runnable threads, make this a poll: */
  if (tme_sjlj_threads_runnable != NULL) {
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
  }
  
  rc = (tme_sjlj_main_events->max_event >= 0) ?
    (event_wait(tme_sjlj_main_events->es, &timeout, esr, SIZE(esr))) :
    (tme_sjlj_main_events->max_event);
  
  /* we were in select() for an unknown amount of time: */
  tme_thread_long();

  /* move all runnable threads to the dispatching list: */
  _tme_sjlj_threads_dispatching_runnable();

  /* move all threads that have timed out to the dispatching list: */
  _tme_sjlj_threads_dispatching_timeout();

  /* if some fds are ready, dispatch them: */
  if (rc > 0) {	
    for (fd = 0; fd < rc; ++fd) {
      const struct event_set_return *e = &esr[fd];
      
      if (e->rwflags != 0) {

	/* move all threads that match the conditions on this file
	   descriptor to the dispatching list: */
	_tme_sjlj_threads_dispatching_event((struct tme_sjlj_event_arg *)e->arg, e->rwflags);

      }
    }
  }
    
  /* dispatch: */
  tme_sjlj_dispatch(1);

  return 0;
}

/* this creates a new thread: */
void
tme_sjlj_thread_create(tme_threadid_t *thr, tme_thread_t func, void *func_private)
{
  struct tme_sjlj_thread *thread;

  /* allocate a new thread and put it on the all-threads list: */
  thread = tme_new(struct tme_sjlj_thread, 1);
  *thr = thread;
  thread->prev = &tme_sjlj_threads_all;
  thread->next = *thread->prev;
  *thread->prev = thread;
  if (thread->next != NULL) {
    thread->next->prev = &thread->next;
  }

  /* initialize the thread: */
#ifdef WIN32
  thread->tme_sjlj_thread_func = CreateFiber(0, func, func_private);
#else
  thread->tme_sjlj_thread_func_private = func_private;
  thread->tme_sjlj_thread_func = func;
#endif
  thread->tme_sjlj_thread_cond = NULL;
  thread->tme_sjlj_thread_events = NULL;
  TME_TIME_SETV(thread->tme_sjlj_thread_sleep, 0, 0);
  thread->timeout_prev = NULL;

  /* make this thread runnable: */
  thread->tme_sjlj_thread_state = TME_SJLJ_THREAD_STATE_BLOCKED;
  thread->state_prev = NULL;
  thread->state_next = NULL;
  thread->tme_sjlj_thread_dispatch_number = _tme_sjlj_thread_dispatch_number - 1;
  _tme_sjlj_change_state(thread,
			 TME_SJLJ_THREAD_STATE_RUNNABLE);
}

/* this makes a thread wait on a condition: */
void
tme_sjlj_cond_wait_yield(tme_cond_t *cond, tme_mutex_t *mutex)
{

  /* unlock the mutex: */
  tme_mutex_unlock(mutex);

  /* remember that this thread is waiting on this condition: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = cond;

  /* yield: */
  tme_thread_yield();

  /* lock the mutex: */
  tme_mutex_lock(mutex);
}

/* this makes a thread sleep on a condition: */
void
tme_sjlj_cond_sleep_yield(tme_cond_t *cond, tme_mutex_t *mutex, const tme_time_t *sleep)
{

  /* remember that this thread is waiting on this condition: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = cond;

  /* sleep and yield: */
  tme_sjlj_sleep_yield(TME_TIME_GET_SEC(*sleep), TME_TIME_GET_FRAC(*sleep), mutex);
}

/* this notifies one or more threads waiting on a condition: */
void
tme_sjlj_cond_notify(tme_cond_t *cond, int broadcast)
{
  struct tme_sjlj_thread *thread;

  for (thread = tme_sjlj_threads_all;
       thread != NULL;
       thread = thread->next) {
    if (thread->tme_sjlj_thread_state == TME_SJLJ_THREAD_STATE_BLOCKED
	&& thread->tme_sjlj_thread_cond == cond) {
      
      /* this thread is runnable: */
      _tme_sjlj_change_state(thread,
			     TME_SJLJ_THREAD_STATE_RUNNABLE);

      /* if we're not broadcasting this notification, stop now: */
      if (!broadcast) {
	break;
      }
    }
  }
}
#if 0
/* this sleeps: */
void
tme_sjlj_sleep(unsigned long sec, unsigned long usec)
{
  tme_time_t then, now, timeout;
  int rc;
  
  /* the thread ran for an unknown amount of time: */
  tme_thread_long();

  /* get the wakeup time for the thread: */
  tme_gettimeofday(&then);
  for (; usec >= 1000000; sec++, usec -= 1000000);
  if (TME_TIME_INC_FRAC(then, usec) >= 1000000) {
    sec++;
    TME_TIME_INC_FRAC(then, -1000000);
  }
  TME_TIME_INC_SEC(then, sec);
  
  /* select for the sleep period: */
  for (;;) {

    /* calculate the current timeout: */
    tme_gettimeofday(&now);
    if (TME_TIME_GT(now, then)) {
      break;
    }
    timeout = then;
    if (TME_TIME_FRAC_LT(timeout, now)) {
      TME_TIME_ADDV(timeout, -1, 1000000);
    }
    TME_TIME_DEC(timeout, now);

    /* do the select.  select returns 0 iff the timeout expires, so we
       can skip another gettimeofday and loop: */
    rc = tme_select(-1, NULL, NULL, NULL, &timeout);
    tme_thread_long();
    if (rc == 0) {
      break;
    }

    /* loop to see if the timeout really expired: */
  }
}
#endif
/* this sleeps and yields: */
void
tme_sjlj_sleep_yield(unsigned long sec, unsigned long usec, tme_mutex_t *mutex)
{

  /* set the sleep interval: */
  for (; usec >= 1000000; ) {
    sec++;
    usec -= 1000000;
  }
  TME_TIME_SETV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, sec, usec);

  /* unlock the mutex: */
  if(mutex) tme_mutex_unlock(mutex);
  
  /* yield: */
  tme_thread_yield();

  /* lock the mutex: */
  if(mutex) tme_mutex_lock(mutex);
}

struct tme_sjlj_event_set *tme_sjlj_event_set_init(int *maxevents, unsigned int flags) {
  struct tme_sjlj_event_set *tes;
  struct event_set *es;
  es = event_set_init(maxevents, flags);
  tes = (struct tme_sjlj_event_set *) tme_malloc(sizeof(struct tme_sjlj_event_set) + sizeof(struct tme_sjlj_event) * (*maxevents));
  tes->es = es;
  tes->max_event = -1;
  tes->flags = flags;
  return tes;
}

void tme_sjlj_event_free(struct tme_sjlj_event_set *es) {
  if(es->es)
    event_free(es->es);
  tme_free(es);
}

void tme_sjlj_event_reset(struct tme_sjlj_event_set *es)
{
  if(es->es)
    event_reset(es->es);
  es->max_event = -1;
}

int tme_sjlj_event_del(struct tme_sjlj_event_set *es, event_t event)
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

int tme_sjlj_event_ctl(struct tme_sjlj_event_set *es, event_t event, unsigned int rwflags, void *arg) {
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
int
tme_sjlj_event_wait_yield(struct tme_sjlj_event_set *es, const struct timeval *timeout_in, struct event_set_return *out, int outlen, tme_mutex_t *mutex)
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
  if(mutex) tme_mutex_unlock(mutex);
  tme_sjlj_thread_blocked.tme_sjlj_thread_events = es;

  if (timeout_in != NULL) {
    TME_TIME_SETV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, timeout_in->tv_sec, timeout_in->tv_usec);
    for (; TME_TIME_GET_FRAC(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep) >= 1000000; ) {
      TME_TIME_ADDV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, 1, -1000000);
    }
  } else {
    TME_TIME_SETV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, BIG_TIMEOUT, 0);
  }

  /* yield: */
  tme_thread_yield();

  /* lock the mutex: */
  if(mutex) tme_mutex_lock(mutex);
  return event_wait(es->es, &timeout_out, out, outlen);
}

/* this exits a thread: */
void
tme_sjlj_exit(void)
{
  
  /* mark that this thread is exiting: */
  tme_sjlj_thread_exiting = TRUE;

  /* yield: */
  tme_thread_yield();
}
       
/* this yields the current thread: */
void
tme_sjlj_yield(void)
{
  struct tme_sjlj_thread *thread;
  int blocked;
  struct tme_sjlj_thread **_thread_prev;
  struct tme_sjlj_thread *thread_other;
  struct tme_sjlj_event_arg *event_arg;
  struct tme_sjlj_event_set *es, *es2 = NULL;
  int i, j, changed;
  
  /* get the active thread: */
  thread = tme_sjlj_thread_active;

  /* the thread ran for an unknown amount of time: */
  tme_thread_long();

  /* assume that this thread is not blocked: */
  blocked = FALSE;

  /* see if this thread is blocked on a condition: */
  if (tme_sjlj_thread_blocked.tme_sjlj_thread_cond != NULL) {
    blocked = TRUE;
  }
  thread->tme_sjlj_thread_cond = tme_sjlj_thread_blocked.tme_sjlj_thread_cond;
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = NULL;

  /* see if this thread is blocked on any events: */
  es = tme_sjlj_thread_blocked.tme_sjlj_thread_events;

  if(es) {
    j = es->max_event + 1;
    es2 = tme_sjlj_event_set_init(&j, 0);
    for(i=0;i<=es->max_event;i++) {
      if(es->events[i].event == UNDEFINED_EVENT)
	continue;
      j = -1;
      changed = FALSE;
      if(thread->tme_sjlj_thread_events)
	j = tme_sjlj_event_del(thread->tme_sjlj_thread_events, es->events[i].event);
      if(j<0) {
	changed = TRUE;
	j = tme_sjlj_event_del(tme_sjlj_main_events, es->events[i].event);
	if(j<0)
	  j = tme_sjlj_event_ctl(tme_sjlj_main_events,
				 es->events[i].event,
				 es->events[i].flags,
				 tme_new0(struct tme_sjlj_event_arg, TME_NUM_EVFLAGS));
	else
	  tme_sjlj_main_events->events[j].event = es->events[i].event;
      } else {
	if(es->events[i].flags != thread->tme_sjlj_thread_events->events[j].flags) {
	  changed = TRUE;
	  tme_sjlj_main_events->events[(int)thread->tme_sjlj_thread_events->events[j].arg].flags
	    &= ~thread->tme_sjlj_thread_events->events[j].flags;
	}
	j = (int)thread->tme_sjlj_thread_events->events[j].arg;
      }
      tme_sjlj_event_ctl(es2,
			 es->events[i].event,
			 es->events[i].flags,
			 (void *)j);			      
      if(changed) {
	tme_sjlj_main_events->events[j].flags |= es->events[i].flags;
	event_ctl(tme_sjlj_main_events->es,
		  es->events[i].event,
		  tme_sjlj_main_events->events[j].flags,
		  tme_sjlj_main_events->events[j].arg);
	event_arg = (struct tme_sjlj_event_arg *)tme_sjlj_main_events->events[j].arg;
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
#ifndef WIN32
    tme_sjlj_event_free(es);
#endif
  }

  if(thread->tme_sjlj_thread_events) {
    for(i=0;i<=thread->tme_sjlj_thread_events->max_event;i++) {
      j = (int)thread->tme_sjlj_thread_events->events[i].arg;
      if(tme_sjlj_main_events->events[j].flags &= ~thread->tme_sjlj_thread_events->events[i].flags)
	event_ctl(tme_sjlj_main_events->es,
		  tme_sjlj_main_events->events[j].event,
		  tme_sjlj_main_events->events[j].flags,
		  tme_sjlj_main_events->events[j].arg);
      else {
	event_del(tme_sjlj_main_events->es,
		  tme_sjlj_main_events->events[j].event);
	tme_sjlj_main_events->events[j].event = UNDEFINED_EVENT;
	tme_free(tme_sjlj_main_events->events[j].arg);
	while(j == tme_sjlj_main_events->max_event) {
	  if(tme_sjlj_main_events->events[j--].event == UNDEFINED_EVENT)
	    tme_sjlj_main_events->max_event--;
	  if(j<0) break;
	}
      }
    }
    tme_sjlj_event_free(thread->tme_sjlj_thread_events);
  }
  
  thread->tme_sjlj_thread_events = es2;
  tme_sjlj_thread_blocked.tme_sjlj_thread_events = NULL;
  
  /* see if this thread is blocked for some amount of time: */
  if (!TME_TIME_EQV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, 0, 0)) {

    assert(TME_TIME_GET_FRAC(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep) < 1000000);
    blocked = TRUE;

    /* set the timeout for this thread: */
    tme_gettimeofday(&thread->tme_sjlj_thread_timeout);
    TME_TIME_INC(thread->tme_sjlj_thread_timeout, tme_sjlj_thread_blocked.tme_sjlj_thread_sleep);
    if (TME_TIME_GET_FRAC(thread->tme_sjlj_thread_timeout) >= 1000000) {
      TME_TIME_ADDV(thread->tme_sjlj_thread_timeout, 1, -1000000);
    }

    /* insert this thread into the timeout list: */
    assert (thread->timeout_prev == NULL);
    for (_thread_prev = &tme_sjlj_threads_timeout;
	 (thread_other = *_thread_prev) != NULL;
	 _thread_prev = &thread_other->timeout_next) {
      if (TME_TIME_GT(thread_other->tme_sjlj_thread_timeout,
		      thread->tme_sjlj_thread_timeout)) {
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
  thread->tme_sjlj_thread_sleep = tme_sjlj_thread_blocked.tme_sjlj_thread_sleep;
  TME_TIME_SETV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, 0, 0);

  /* if this thread is actually exiting, it must appear to be
     runnable, and it only isn't because it's exiting: */
  if (tme_sjlj_thread_exiting) {
    assert(!blocked);
    blocked = TRUE;
  }

  /* make any following thread on the runnable list the next active
     thread: */
  tme_sjlj_thread_active = thread->state_next;

  /* if this thread is blocked, move it to the blocked list: */
  if (blocked) {
    _tme_sjlj_change_state(thread, 
			   TME_SJLJ_THREAD_STATE_BLOCKED);

    /* if this thread is exiting: */
    if (tme_sjlj_thread_exiting) {

      /* remove this thread from the all-threads list: */
      *thread->prev = thread->next;
      if (thread->next != NULL) {
	thread->next->prev = thread->prev;
      }

      /* free this thread: */
      tme_free(thread);

      /* nothing is exiting any more: */
      tme_sjlj_thread_exiting = FALSE;
    }
  }

  /* jump back to the dispatcher: */
#ifdef WIN32
  SwitchToFiber(tme_sjlj_thread_blocked.tme_sjlj_thread_func);
#else
  longjmp(tme_sjlj_dispatcher_jmp, TRUE);
#endif
}

#ifndef TME_NO_DEBUG_LOCKS

/* lock operations: */
int
tme_sjlj_rwlock_init(struct tme_sjlj_rwlock *lock)
{
  /* initialize the lock: */
  lock->_tme_sjlj_rwlock_locked = FALSE;
  lock->_tme_sjlj_rwlock_file = NULL;
  lock->_tme_sjlj_rwlock_line = 0;
  return (TME_OK);
}
int
tme_sjlj_rwlock_lock(struct tme_sjlj_rwlock *lock, _tme_const char *file, unsigned long line, int try)
{
  
  /* if this lock is already locked: */
  if (lock->_tme_sjlj_rwlock_locked) {
    if (try) {
      return (TME_EDEADLK);
    }
    abort();
  }

  /* lock the lock: */
  lock->_tme_sjlj_rwlock_locked = TRUE;
  lock->_tme_sjlj_rwlock_file = file;
  lock->_tme_sjlj_rwlock_line = line;
  return (TME_OK);
}
int
tme_sjlj_rwlock_unlock(struct tme_sjlj_rwlock *lock, _tme_const char *file, unsigned long line)
{
  
  /* if this lock isn't locked: */
  if (!lock->_tme_sjlj_rwlock_locked) {
    abort();
  }

  /* unlock the lock: */
  lock->_tme_sjlj_rwlock_locked = FALSE;
  lock->_tme_sjlj_rwlock_file = file;
  lock->_tme_sjlj_rwlock_line = line;
  return (TME_OK);
}

#endif /* !TME_NO_DEBUG_LOCKS */
