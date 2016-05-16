/* $Id: threads-sjlj.c,v 1.18 2010/06/05 19:10:28 fredette Exp $ */

/* libtme/threads-sjlj.c - implementation of setjmp/longjmp threads: */

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

#include <tme/common.h>
_TME_RCSID("$Id: threads-sjlj.c,v 1.18 2010/06/05 19:10:28 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <setjmp.h>

/* thread states: */
#define TME_SJLJ_THREAD_STATE_BLOCKED		(1)
#define TME_SJLJ_THREAD_STATE_RUNNABLE		(2)
#define TME_SJLJ_THREAD_STATE_DISPATCHING	(3)
#define TME_MAX_FD 1023

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
  void *tme_sjlj_thread_func_private;
  tme_thread_t tme_sjlj_thread_func;

  /* any condition that this thread is waiting on: */
  tme_cond_t *tme_sjlj_thread_cond;

  /* range of fds that this thread is waiting on: */
  int tme_sjlj_thread_min_fd;
  int tme_sjlj_thread_max_fd;
  
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

/* thread(s) blocked on a file descriptor */
struct tme_sjlj_thread_fd {
  unsigned int tme_sjlj_fd_thread_conditions;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_read;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_write;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_except;
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

/* this dummy thread structure is filled before a yield to represent
   what, if anything, the active thread is blocking on when it yields: */
static struct tme_sjlj_thread tme_sjlj_thread_blocked;

/* this is set if the active thread is exiting: */
static int tme_sjlj_thread_exiting;

/* this is a jmp_buf back to the dispatcher: */
static jmp_buf tme_sjlj_dispatcher_jmp;

/* the main loop events: */
static struct event_set *tme_sjlj_main_events;

/* for each file descriptor, any threads blocked on it: */
static struct tme_sjlj_thread_fd tme_sjlj_fd_thread[TME_MAX_FD + 1];

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
  int fd = TME_MAX_FD + 1;
  int i;
  
  /* there are no threads: */
  tme_sjlj_threads_all = NULL;
  tme_sjlj_threads_timeout = NULL;
  tme_sjlj_threads_runnable = NULL;
  tme_sjlj_threads_dispatching = NULL;
  tme_sjlj_thread_active = NULL;
  tme_sjlj_thread_exiting = FALSE;

  /* no threads are waiting on any fds: */
  tme_sjlj_main_events = event_set_init(&fd, 0);
  
  for (i = 0; i < fd; i++) {
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions = 0;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_read = NULL;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_write = NULL;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_except = NULL;
  }

  /* initialize the thread-blocked structure: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = NULL;
  tme_sjlj_thread_blocked.tme_sjlj_thread_min_fd = TME_MAX_FD + 1;
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = -1;
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

/* this moves all threads with the given file descriptor conditions to
   the dispatching list: */
static void
_tme_sjlj_threads_dispatching_fd(int fd,
				 unsigned int fd_conditions)
{
  struct tme_sjlj_thread *thread;
  struct tme_sjlj_thread_fd *fd_thread = &tme_sjlj_fd_thread[fd];
  
  /* loop over all set conditions: */
  for (fd_conditions &= fd_thread->tme_sjlj_fd_thread_conditions;
       fd_conditions != 0;
       fd_conditions &= (fd_conditions - 1)) {

    /* move the thread for this condition to the dispatching list: */
    thread = ((fd_conditions & EVENT_READ)
	      ? fd_thread->tme_sjlj_fd_thread_read
	      : (fd_conditions & EVENT_WRITE)
	      ? fd_thread->tme_sjlj_fd_thread_write
	      : fd_thread->tme_sjlj_fd_thread_except);
    assert (thread != NULL);
    _tme_sjlj_change_state(thread, TME_SJLJ_THREAD_STATE_DISPATCHING);
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
  timeout->tv_val = secs;
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
      rc_one = setjmp(tme_sjlj_dispatcher_jmp);
      if (rc_one) {
	continue;
      }

      /* run this thread.  if it happens to return, just call
         tme_sjlj_exit(): */
      (*thread->tme_sjlj_thread_func)(thread->tme_sjlj_thread_func_private);
      tme_sjlj_exit();
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
  struct timeval timeout_buffer;
  struct timeval *timeout;
  int rc;
  struct event_set_return esr[64];
  
  if(tme_sjlj_threads_all == NULL) return -1;

  (*(tme_threads_fn)event_check)();
  
  /* make the select timeout: */

  /* if the timeout list is empty: */
  if (tme_sjlj_threads_timeout == NULL) {

    /* assume that we will block in select indefinitely.  there must
       either be runnable threads (in which case we will not block
       at all in select), or we must be selecting on file
       descriptors: */
    timeout = NULL;
    assert (tme_sjlj_threads_runnable != NULL);
	    //	    || tme_sjlj_main_max_fd >= 0);
  }

  /* otherwise, the timeout list is not empty: */
  else {
    timeout_buffer.tv_val = 0;
    timeout_buffer.tv_usec = 0;
    timeout = &timeout_buffer;

    if (tme_sjlj_threads_runnable == NULL)  
      /* if there are no runnable threads, make the timeout: */
      _tme_sjlj_timeout_time(timeout);
  }

  rc = event_wait(tme_sjlj_main_events, timeout, esr, SIZE(esr));
  
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
	_tme_sjlj_threads_dispatching_fd((int)e->arg, e->rwflags);

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
  thread->tme_sjlj_thread_func_private = func_private;
  thread->tme_sjlj_thread_func = func;
  thread->tme_sjlj_thread_cond = NULL;
  thread->tme_sjlj_thread_min_fd = TME_MAX_FD + 1;
  thread->tme_sjlj_thread_max_fd = -1;
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
}

/* this makes a thread sleep on a condition: */
void
tme_sjlj_cond_sleep_yield(tme_cond_t *cond, tme_mutex_t *mutex, const tme_time_t *sleep)
{

  /* unlock the mutex: */
  tme_mutex_unlock(mutex);

  /* remember that this thread is waiting on this condition: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = cond;

  /* sleep and yield: */
  tme_sjlj_sleep_yield(TME_TIME_GET_SEC(*sleep), TME_TIME_GET_FRAC(*sleep));
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
tme_sjlj_sleep_yield(unsigned long sec, unsigned long usec)
{

  /* set the sleep interval: */
  for (; usec >= 1000000; ) {
    sec++;
    usec -= 1000000;
  }
  TME_TIME_SETV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, sec, usec);

  /* yield: */
  tme_thread_yield();
}

tme_event_set_t *tme_sjlj_event_set_init(int *maxevents, unsigned int flags) {
  tme_event_set_t *tes;
  struct event_set *es;
  es = event_set_init(maxevents, flags);
  tes = (tme_event_set_t *) tme_malloc(sizeof(tme_event_set_t) + sizeof(tme_event_t) * (*maxevents));
  tes->es = es;
  tes->num_events = 0;
  return tes;
}

void tme_sjlj_event_ctl(struct tme_event_set *es, event_t event, unsigned int rwflags, void *arg) {
  event_ctl(es->es, event, rwflags, arg);
  es->events[es->num_events].fd = event;
  es->events[es->num_events++].flags = rwflags;
}

/* this selects and yields: */
int
tme_sjlj_event_wait_yield(struct tme_event_set *es, const struct timeval *timeout_in, struct event_set_return *out, int outlen)
{
  int fd_old;
  int fd_new;
  int min_fd, max_fd, fd;
  unsigned int fd_condition_old;
  unsigned int fd_condition_new;
  struct timeval timeout_out;
  struct tme_sjlj_thread *thread;
  int rc, i;

  /* do a polling select: */
  timeout_out.tv_val = 0;
  timeout_out.tv_usec = 0;
  rc = event_wait(es->es, &timeout_out, out, outlen);
  tme_thread_long();
  if(rc != 0 ||
     timeout_in != NULL &&
     timeout_in->tv_sec == 0 &&
     timeout_in->tv_usec == 0) {
    return (rc);
  }

  /* get the active thread: */
  thread = tme_sjlj_thread_active;

  min_fd = TME_MAX_FD + 1;
  max_fd = -1;
  
  /* we are yielding.  copy out the fds to merge with current events */
  for (i = 0; i <= es->num_events; i++) {
    fd = es->events[i].fd;
    if(fd < min_fd) min_fd = fd;
    if(fd > max_fd) max_fd = fd;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions = es->events[i].flags;
  }
  
  event_free(es);

  tme_sjlj_thread_blocked.tme_sjlj_thread_min_fd = min_fd;
  if(thread->tme_sjlj_thread_min_fd < min_fd)
    min_fd = thread->tme_sjlj_thread_min_fd;
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = max_fd;
  if(thread->tme_sjlj_thread_max_fd > max_fd)
    max_fd = thread->tme_sjlj_thread_max_fd;

  for (fd = min_fd; fd <= max_fd; fd++) {
    /* the old and new conditions on this fd start out empty: */
    fd_condition_old = 0;

    /* check the old fd sets: */
    if(tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_read == thread)
      fd_condition_old |= EVENT_READ;
    if(tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_write == thread)
      fd_condition_old |= EVENT_WRITE;
    if(tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_except == thread)
      fd_condition_old |= EVENT_UNDEF;
    
    /* check the new fd sets: */

    /* if the conditions haven't changed: */
    if (tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions == fd_condition_old)
      continue;

    /* if there is any blocking on this file descriptor, remove it: */
    /* remove this fd from our main loop's fd sets: */
    event_del(tme_sjlj_main_events, fd);
      
    fd_condition_new = 0;
    /* update the blocking by this thread on this file descriptor: */
#define UPDATE_FD_THREAD(fd_thread, condition)				\
    do {								\
      if (fd_condition_old & condition) {				\
	assert(tme_sjlj_fd_thread[fd].fd_thread == thread);		\
	tme_sjlj_fd_thread[fd].fd_thread = NULL;			\
      }									\
      if (tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions & condition) { \
	assert(tme_sjlj_fd_thread[fd].fd_thread == NULL);		\
	tme_sjlj_fd_thread[fd].fd_thread = thread;			\
      }									\
      /* get the conditions for all threads for this fd: */		\
      if(tme_sjlj_fd_thread[fd].fd_thread)				\
	fd_condition_new |= condition;					\
    } while	(/* CONSTCOND */ 0)
    UPDATE_FD_THREAD(tme_sjlj_fd_thread_read, EVENT_READ);
    UPDATE_FD_THREAD(tme_sjlj_fd_thread_write, EVENT_WRITE);
    UPDATE_FD_THREAD(tme_sjlj_fd_thread_except, EVENT_UNDEF);
#undef UPDATE_FD_THREAD    

    /* reset the new fd conditions for this thread: */
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions = 0;

    /* if there is any blocking on this file descriptor, add it: */
    if (fd_condition_new != 0) {
      /* add this fd to main loop's relevant fd sets: */
      event_ctl(tme_sjlj_main_events, fd, fd_condition_new, (void *)fd);
    }
  }

  if (timeout_in != NULL) {
    tme_sjlj_thread_blocked.tme_sjlj_thread_sleep = *timeout_in;
    for (; TME_TIME_GET_FRAC(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep) >= 1000000; ) {
      TME_TIME_ADDV(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep, 1, -1000000);
    }
  }

  /* yield: */
  tme_thread_yield();
  /* NOTREACHED */
  return (0);
}

/* this reads or writes, yielding if the fd is not ready: */
ssize_t
tme_sjlj_event_yield(int fd, void *data, size_t count, unsigned int rwflags)
{
  int rc = 1;
  struct tme_event_set *es = tme_event_set_init(&rc, EVENT_METHOD_FAST);
  
  tme_event_ctl(es, fd, rwflags, 0);

  rc = tme_event_wait_yield(es, NULL, NULL, 0);

  if (rc != 1) {
    return (rc);
  }

  /* do the read: */
  return ((event == EVENT_READ) ? (read(fd, data, count)) : (write(fd, data, count));
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

  /* see if this thread is blocked on any file descriptors: */
  if(tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd >= 0) {
    blocked = TRUE;
  }
  
  thread->tme_sjlj_thread_min_fd = tme_sjlj_thread_blocked.tme_sjlj_thread_min_fd;
  thread->tme_sjlj_thread_max_fd = tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd;

  tme_sjlj_thread_blocked.tme_sjlj_thread_min_fd = TME_MAX_FD + 1;
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = -1;
  
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
  longjmp(tme_sjlj_dispatcher_jmp, TRUE);
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
