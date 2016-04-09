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
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#include <setjmp.h>

/* if we don't have GLIB, fake a few definitions to keep things
   compiling: */
#ifdef HAVE_GLIB
#ifndef G_ENABLE_DEBUG
#define G_ENABLE_DEBUG (0)
#endif /* !G_ENABLE_DEBUG */
#include <glib.h>
#else  /* !HAVE_GLIB */
typedef int GIOCondition;
#define G_IO_IN		TME_BIT(0)
#define G_IO_OUT		TME_BIT(1)
#define G_IO_ERR	TME_BIT(2)
#endif /* !HAVE_GLIB */

/* thread states: */
#define TME_SJLJ_THREAD_STATE_BLOCKED		(1)
#define TME_SJLJ_THREAD_STATE_RUNNABLE		(2)
#define TME_SJLJ_THREAD_STATE_DISPATCHING	(3)

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

  /* true if thread is blocked on an fd: */
  int tme_sjlj_thread_fd_cond;
  
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
  GIOCondition tme_sjlj_fd_thread_conditions;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_read;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_write;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_except;
}

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

/* number of fds to select/poll on */
static int tme_sjlj_main_max_fd;

#ifdef HAVE_SELECT
/* the main loop fd sets: */
static fd_set tme_sjlj_main_fdset_read;
static fd_set tme_sjlj_main_fdset_write;
static fd_set tme_sjlj_main_fdset_except;
#endif

/* for each file descriptor, any threads blocked on it: */
static struct tme_sjlj_thread_fd tme_sjlj_fd_thread[FD_SETSIZE];

/* the dispatch number: */
static tme_uint32_t _tme_sjlj_thread_dispatch_number;

/* a reasonably current time: */
static tme_time_t _tme_sjlj_now;

/* if nonzero, the last dispatched thread ran for only a short time: */
int tme_sjlj_thread_short;

#ifdef HAVE_GLIB

/* nonzero iff we're using the glib main loop: */
static int tme_sjlj_using_glib;

/* for each file descriptor, the GLIB tag for the fd event source: */
static gint tme_sjlj_fd_tag[FD_SETSIZE];

/* this set iff the idle callback is set: */
static int tme_sjlj_idle_set;

/* any timeout source ID: */
static guint _tme_sjlj_glib_timeout_id;

/* any timeout time: */
static tme_time_t _tme_sjlj_glib_timeout;

#endif /* HAVE_GLIB */

/* this initializes the threads system: */
void
tme_sjlj_threads_init()
{
  int fd;

#ifdef HAVE_GLIB
  /* assume that we won't be using the GLIB main loop: */
  tme_sjlj_using_glib = FALSE;
  tme_sjlj_idle_set = FALSE;
#endif
  /* there are no threads: */
  tme_sjlj_threads_all = NULL;
  tme_sjlj_threads_timeout = NULL;
  tme_sjlj_threads_runnable = NULL;
  tme_sjlj_threads_dispatching = NULL;
  tme_sjlj_thread_active = NULL;
  tme_sjlj_thread_exiting = FALSE;

  /* no threads are waiting on any fds: */
  tme_sjlj_main_max_fd = -1;
#ifdef HAVE_SELECT
  FD_ZERO(&tme_sjlj_main_fdset_read);
  FD_ZERO(&tme_sjlj_main_fdset_write);
  FD_ZERO(&tme_sjlj_main_fdset_except);
  for (fd = 0; fd < FD_SETSIZE; fd++) {
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions = 0;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_read = NULL;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_write = NULL;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_except = NULL;
  }
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = -1;
#endif

  /* initialize the thread-blocked structure: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = NULL;
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
				 GIOCondition fd_conditions)
{
  struct tme_sjlj_thread *thread;
  struct tme_sjlj_thread_fd *fd_thread = &tme_sjlj_fd_thread[fd];
  
  /* loop over all set conditions: */
  for (fd_conditions &= fd_thread->tme_sjlj_fd_thread_conditions;
       fd_conditions != 0;
       fd_conditions &= (fd_conditions - 1)) {

    /* move the thread for this condition to the dispatching list: */
    thread = ((fd_conditions & G_IO_IN)
	      ? fd_thread->tme_sjlj_fd_thread_read
	      : (fd_conditions & G_IO_OUT)
	      ? fd_thread->tme_sjlj_fd_thread_write
	      : fd_thread->tme_sjlj_fd_thread_except);
    assert (thread != NULL);
    _tme_sjlj_change_state(thread, TME_SJLJ_THREAD_STATE_DISPATCHING);
  }
}

/* this makes the timeout time: */
static void
_tme_sjlj_timeout_time(tme_time_t *timeout)
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
  TME_TIME_SETV(*timeout, secs, usecs);
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

#ifdef HAVE_GLIB

/* this handles a GLIB callback for a timeout: */
static gint
_tme_sjlj_glib_callback_timeout(gpointer callback_pointer)
{

  /* we were in GLIB for an unknown amount of time: */
  tme_thread_long();

  /* this GLIB timeout will soon be removed, so forget it: */
  _tme_sjlj_glib_timeout_id = 0;

  /* move all threads that have timed out to the dispatching list: */
  _tme_sjlj_threads_dispatching_timeout();

  /* dispatch: */
  tme_sjlj_dispatch(1);

  /* yield to GLIB: */
  tme_sjlj_threads_glib_yield(NULL);

  /* remove this timeout: */
  return (FALSE);

  /* unused: */
  callback_pointer = 0;
}

/* this handles a GLIB callback for a file descriptor: */
static gboolean
_tme_sjlj_glib_callback_fd(GIOChannel *chan,
			   GIOCondition fd_conditions,
			   gpointer callback_pointer)
{

  /* we were in GLIB for an unknown amount of time: */
  tme_thread_long();

  /* move all threads that match the conditions on this file
     descriptor to the dispatching list: */
  _tme_sjlj_threads_dispatching_fd(g_io_channel_unix_get_fd(chan), fd_conditions);

  /* dispatch: */
  tme_sjlj_dispatch(1);

  /* yield to GLIB: */
  tme_sjlj_threads_glib_yield(NULL);

  /* unused: */
  callback_pointer = 0;

  return TRUE;
}

/* this handles a GLIB callback for an idle: */
static gint
_tme_sjlj_glib_callback_idle(gpointer callback_pointer)
{

  /* we were in GLIB for an unknown amount of time: */
  tme_thread_long();

  /* move all runnable threads to the dispatching list: */
  _tme_sjlj_threads_dispatching_runnable();

  /* move all threads that have timed out to the dispatching list: */
  _tme_sjlj_threads_dispatching_timeout();

  /* dispatch: */
  tme_sjlj_dispatch(1);

  /* yield to GLIB: */
  tme_sjlj_threads_glib_yield(NULL);

  /* if there are no runnable threads: */
  if (tme_sjlj_threads_runnable == NULL) {

    /* remove this idle: */
    tme_sjlj_idle_set = FALSE;
    return (FALSE);
  }

  /* preserve this idle: */
  return (TRUE);

  /* unused: */
  callback_pointer = 0;
}

/* this yields to GLIB: */
void
tme_sjlj_threads_glib_yield(void *mainloop)
{
  struct tme_sjlj_thread *thread_timeout;
  tme_time_t timeout;
  unsigned long secs;
  tme_uint32_t msecs;

  /* if there are no runnable threads: */
  if (tme_sjlj_threads_runnable == NULL) {
    /* if there are no threads at all: */
    if (__tme_predict_false(tme_sjlj_threads_all == NULL)) {

      tme_sjlj_using_glib = FALSE;
  
      /* quit the GLIB main loop: */
      //      glib_main_quit();
      return;
    }

    /* if there is a GLIB timeout, but the timeout list is empty or the
       GLIB timeout isn't for the earliest timeout: */
    thread_timeout = tme_sjlj_threads_timeout;
    if (_tme_sjlj_glib_timeout_id != 0
	&& (thread_timeout == NULL
	    || !TME_TIME_EQ(_tme_sjlj_glib_timeout, thread_timeout->tme_sjlj_thread_timeout))) {
      /* remove the GLIB timeout: */
      g_source_remove(_tme_sjlj_glib_timeout_id);
      _tme_sjlj_glib_timeout_id = 0;
    }

    /* if the timeout list is not empty, but there is no GLIB timeout: */
    if (tme_sjlj_threads_timeout != NULL
	&& _tme_sjlj_glib_timeout_id == 0) {

      /* get the timeout: */
      _tme_sjlj_timeout_time(&timeout);

      /* if there are still no runnable threads: */
      /* NB: if the earliest timeout has already timed out,
	 _tme_sjlj_timeout_time() has already made the thread
	 runnable: */
      if (tme_sjlj_threads_runnable == NULL) {

	/* convert the timeout into milliseconds, and clip it at ten
	   seconds: */
	secs = TME_TIME_GET_SEC(timeout);
	msecs = (TME_TIME_GET_FRAC(timeout) + 999) / 1000;
	if (msecs == 1000) {
	  secs++;
	  msecs = 0;
	}
	msecs
	  = (secs >= 10
	     ? (10 * 1000)
	     : ((secs * 1000) + msecs));

	/* GLIB timeouts can expire up to one millisecond early, so we
	   always add one: */
	msecs++;

	/* add the timeout: */
	/* XXX we have to call g_timeout_add_full here, because
	   there are no glib_timeout_add_ functions that allow you to
	   specify the priority, and glib_timeout_add() uses
	   G_PRIORITY_DEFAULT, which means our (usually very
	   frequent) timeouts always win over glib's event handling,
	   meaning the glib windows never update: */
	_tme_sjlj_glib_timeout_id
	  = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE,
			       msecs,
			       _tme_sjlj_glib_callback_timeout,
			       NULL,
			       NULL);
	assert (_tme_sjlj_glib_timeout_id != 0);
	_tme_sjlj_glib_timeout = tme_sjlj_threads_timeout->tme_sjlj_thread_timeout;
      }
    }
  }

  /* if there are runnable threads: */
  if (tme_sjlj_threads_runnable != NULL) {

    /* if the idle callback isn't set */
    if (!tme_sjlj_idle_set) {

      /* set the idle callback: */
      g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
		      _tme_sjlj_glib_callback_idle,
		      NULL, NULL);
      tme_sjlj_idle_set = TRUE;
    }
  }

  if(tme_sjlj_using_glib) return;

  tme_sjlj_using_glib = TRUE;
  
  if(mainloop) (*(tme_threads_fn)mainloop)();
  
  glib_main();
}      

#endif /* HAVE_GLIB */

/* this is the main loop iteration function: */
int
tme_sjlj_threads_main_iter()
{
  int fd;
#ifdef HAVE_SELECT
  fd_set fdset_read_out;
  fd_set fdset_write_out;
  fd_set fdset_except_out;
#endif
  GIOCondition fd_conditions;
  tme_time_t timeout_buffer;
  tme_time_t *timeout;
  int rc;
  
  if(tme_sjlj_threads_all == NULL) return -1;

  /* if we have file descriptors to select on: */
  if (tme_sjlj_main_max_fd >= 0) {
#ifdef HAVE_SELECT
    /* make the fd sets: */
    fdset_read_out = tme_sjlj_main_fdset_read;
    fdset_write_out = tme_sjlj_main_fdset_write;
    fdset_except_out = tme_sjlj_main_fdset_except;
#endif
  }

  /* make the select timeout: */

  /* if the timeout list is empty: */
  if (tme_sjlj_threads_timeout == NULL) {

    /* assume that we will block in select indefinitely.  there must
       either be runnable threads (in which case we will not block
       at all in select), or we must be selecting on file
       descriptors: */
    timeout = NULL;
    assert (tme_sjlj_threads_runnable != NULL
	    || tme_sjlj_main_max_fd >= 0);
  }

  /* otherwise, the timeout list is not empty: */
  else {

    /* make the timeout: */
    _tme_sjlj_timeout_time(&timeout_buffer);
    timeout = &timeout_buffer;
  }

  /* if there are runnable threads, make this a poll: */
  if (tme_sjlj_threads_runnable != NULL) {
    TME_TIME_SETV(timeout_buffer, 0, 0);
    timeout = &timeout_buffer;
  }

#ifdef HAVE_SELECT
  /* do the select: */
  rc = tme_select(tme_sjlj_main_max_fd + 1,
		  &fdset_read_out,
		  &fdset_write_out,
		  &fdset_except_out,
		  timeout);
#endif
  
  /* we were in select() for an unknown amount of time: */
  tme_thread_long();

  /* move all runnable threads to the dispatching list: */
  _tme_sjlj_threads_dispatching_runnable();

  /* move all threads that have timed out to the dispatching list: */
  _tme_sjlj_threads_dispatching_timeout();

  /* if some fds are ready, dispatch them: */
  if (rc > 0) {	
    for (fd = tme_sjlj_main_max_fd; fd >= 0; fd--) {
#ifdef HAVE_SELECT
      fd_conditions = 0;
      if (FD_ISSET(fd, &fdset_read_out)) {
	fd_conditions |= G_IO_IN;
      }
      if (FD_ISSET(fd, &fdset_write_out)) {
	fd_conditions |= G_IO_OUT;
      }
      if (FD_ISSET(fd, &fdset_except_out)) {
	fd_conditions |= G_IO_ERR;
      }
#endif
      if (fd_conditions != 0) {

	/* move all threads that match the conditions on this file
	   descriptor to the dispatching list: */
	_tme_sjlj_threads_dispatching_fd(fd, fd_conditions);

	/* stop if there are no more file descriptors left in the
	   sets: */
	if (--rc == 0) {
	  break;
	}
      }
    }
  }
    
  /* dispatch: */
  tme_sjlj_dispatch(1);

  return 0;
}
#endif /* HAVE_SELECT */

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
#ifdef HAVE_SELECT
  thread->tme_sjlj_thread_max_fd = -1;
#endif
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

#if 0
int tme_sjlj_poll_yield(tme_pollfd_t *fds,
			tme_nfds_t nfds,
			tme_time_t *timeout_in) {
  tme_nfds_t fd;
  tme_time_t timeout_out;
  GIOChannel *chan;

  /* do a polling select: */
  TME_TIME_SETV(timeout_out, 0, 0);
  rc = tme_poll(fds, nfds, &timeout_out);
  tme_thread_long();
  if (rc != 0
      || (timeout_in != NULL
	  && TME_TIME_EQV(*timeout_in, 0, 0))) {
    return (rc);
  }

  for(fd = 0; fd < nfds; fd++) {
    /* it should be safe to remove this fd, even if we're
       currently in a callback for it.  if we happen to get a
       callback for it later anyways, _tme_sjlj_glib_callback_fd()
       will ignore it: */
    g_source_remove(tme_sjlj_fd_tag[fd]);
    chan = g_io_channel_unix_new(fd);
    tme_sjlj_fd_tag[fd] = 
      g_io_add_watch(chan,
		     fd_condition_new,
		     _tme_sjlj_glib_callback_fd,
		     NULL);
    g_io_channel_unref(chan);
  }
}

/* this reads, yielding if the fd is not ready: */
ssize_t
tme_sjlj_read_yield(int fd, void *data, size_t count)
{
  int rc;
  tme_pollfd_t fds[1];

  fds[0].fd = fd;
  fds[0].events = G_IO_IN;
  rc = tme_sjlj_poll_yield(fds, 1, NULL);
  if (rc != 1) {
    return (rc);
  }
  
  /* do the read: */
  return (read(fd, data, count));
}

/* this writes, yielding if the fd is not ready: */
ssize_t
tme_sjlj_write_yield(int fd, void *data, size_t count)
{
  int rc;
  tme_pollfd_t fds[1];

  fds[0].fd = fd;
  fds[0].events = G_IO_OUT;
  rc = tme_sjlj_poll_yield(fds, 1, NULL);
  if (rc != 1) {
    return (rc);
  }
  
  /* do the write: */
  return (write(fd, data, count));
}
#endif

#ifdef HAVE_SELECT
/* this selects and yields: */
int
tme_sjlj_select_yield(int nfds,
		      fd_set *fdset_read_in,
		      fd_set *fdset_write_in,
		      fd_set *fdset_except_in,
		      tme_time_t *timeout_in)
{
  int max_fd_old;
  int max_fd_new;
  int max_fd, fd;
  GIOCondition fd_condition_old;
  GIOCondition fd_condition_new;
  tme_time_t timeout_out;
  int rc;

  /* we can't deal if there are more than FD_SETSIZE fds: */
  assert(nfds <= FD_SETSIZE);

  /* in case we end up yielding, we need to save the original
     descriptor sets: */
  if (fdset_read_in != NULL) {
    fdset_read = *fdset_read_in;
  }
  if (fdset_write_in != NULL) {
    fdset_write = *fdset_write_in;
  }
  if (fdset_except_in != NULL) {
    fdset_except = *fdset_except_in;
  }

  /* do a polling select: */
  TME_TIME_SETV(timeout_out, 0, 0);
  rc = tme_select(nfds, fdset_read_in, fdset_write_in, fdset_except_in, &timeout_out);
  tme_thread_long();
  if (rc != 0
      || (timeout_in != NULL
	  && TME_TIME_EQV(*timeout_in, 0, 0))) {
    return (rc);
  }

  /* we are yielding.  zero any unused descriptor sets and set the
     timeout time: */

  if (fdset_read_in == NULL) {
    FD_ZERO(&fdset_read);
  }
  if (fdset_write_in == NULL) {
    FD_ZERO(&fdset_write);
  }
  if (fdset_except_in == NULL) {
    FD_ZERO(&fdset_except);
  }

  max_fd_old = thread->tme_sjlj_thread_max_fd;
  max_fd_new = nfds - 1;
  max_fd = TME_MAX(max_fd_old, max_fd_new);
  for (fd = 0; fd <= max_fd; fd++) {

    /* the old and new conditions on this fd start out empty: */
    fd_condition_old = 0;
    fd_condition_new = 0;

    /* check the old fd sets: */
    if (fd <= max_fd_old) {
#define CHECK_FD_SET(fd_set, condition)		\
      do {					\
	if (FD_ISSET(fd, &thread->fd_set)) {	\
	  fd_condition_old |= condition;	\
	}					\
      } while (/* CONSTCOND */ 0)
      CHECK_FD_SET(tme_sjlj_thread_fdset_read, G_IO_IN);
      CHECK_FD_SET(tme_sjlj_thread_fdset_write, G_IO_OUT);
      CHECK_FD_SET(tme_sjlj_thread_fdset_except, G_IO_ERR);
#undef CHECK_FD_SET
    }
    
    /* check the new fd sets: */
    if (fd <= max_fd_new) {
#define CHECK_FD_SET(fd_set, condition)				\
      do {							\
	if (FD_ISSET(fd, &fd_set)) {	\
	  fd_condition_new |= condition;			\
	  FD_SET(fd, &thread->fd_set);				\
	}							\
	else {							\
	  FD_CLR(fd, &thread->fd_set);				\
	}							\
      } while (/* CONSTCOND */ 0)
      CHECK_FD_SET(fdset_read, G_IO_IN);
      CHECK_FD_SET(fdset_write, G_IO_OUT);
      CHECK_FD_SET(fdset_except, G_IO_ERR);
#undef CHECK_FD_SET
    }

    /* if this thread is blocked on this file descriptor: */
    tme_sjlj_thread_blocked.tme_sjlj_thread_fd_cond |= fd_condition_new;
    
    /* if the conditions have changed: */
    if (fd_condition_new != fd_condition_old) {

      /* if there is any blocking on this file descriptor, remove it: */
      if (tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions != 0) {
	/* remove this fd from our main loop's fd sets: */
	assert(fd <= tme_sjlj_main_max_fd);

	FD_CLR(fd, &tme_sjlj_main_fdset_read);
	FD_CLR(fd, &tme_sjlj_main_fdset_write);
	FD_CLR(fd, &tme_sjlj_main_fdset_except);
	if (fd == tme_sjlj_main_max_fd) {
	  for (; --tme_sjlj_main_max_fd > 0; ) {
	    if (tme_sjlj_fd_thread[tme_sjlj_main_max_fd].tme_sjlj_fd_thread_conditions != 0) {
	      break;
	    }
	  }
	}
      }

      /* update the blocking by this thread on this file descriptor: */
      assert ((tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions
	       & fd_condition_old)
	      == fd_condition_old);
      tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions
	= ((tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions
	    & ~fd_condition_old)
	   | fd_condition_new);
#define UPDATE_FD_THREAD(fd_thread, condition)			\
      do {							\
	if (fd_condition_old & condition) {			\
	  assert(tme_sjlj_fd_thread[fd].fd_thread == thread);	\
	  tme_sjlj_fd_thread[fd].fd_thread = NULL;		\
	}							\
	if (fd_condition_new & condition) {			\
	  assert(tme_sjlj_fd_thread[fd].fd_thread == NULL);	\
	  tme_sjlj_fd_thread[fd].fd_thread = thread;		\
	}							\
      } while	(/* CONSTCOND */ 0)
      UPDATE_FD_THREAD(tme_sjlj_fd_thread_read, G_IO_IN);
      UPDATE_FD_THREAD(tme_sjlj_fd_thread_write, G_IO_OUT);
      UPDATE_FD_THREAD(tme_sjlj_fd_thread_except, G_IO_ERR);
#undef UPDATE_FD_THREAD    

      /* get the conditions for all threads for this fd: */
      fd_condition_new = tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions;

      /* if there is any blocking on this file descriptor, add it: */
      if (fd_condition_new != 0) {
	/* add this fd to main loop's relevant fd sets: */
	if (fd_condition_new & G_IO_IN) {
	  FD_SET(fd, &tme_sjlj_main_fdset_read);
	}
	if (fd_condition_new & G_IO_OUT) {
	  FD_SET(fd, &tme_sjlj_main_fdset_write);
	}
	if (fd_condition_new & G_IO_ERR) {
	  FD_SET(fd, &tme_sjlj_main_fdset_except);
	}
	if (fd > tme_sjlj_main_max_fd) {
	  tme_sjlj_main_max_fd = fd;
	}
      }
    }
  }
  thread->tme_sjlj_thread_max_fd = max_fd_new;
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = -1;

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

/* this reads, yielding if the fd is not ready: */
ssize_t
tme_sjlj_read_yield(int fd, void *data, size_t count)
{
  int rc;
  fd_set fdset_read_in;

  /* select on the fd for reading: */
  FD_ZERO(&fdset_read_in);
  FD_SET(fd, &fdset_read_in);
  rc = tme_sjlj_select_yield(fd + 1,
			     &fdset_read_in,
			     NULL,
			     NULL,
			     NULL);
  if (rc != 1) {
    return (rc);
  }

  /* do the read: */
  return (read(fd, data, count));
}

/* this writes, yielding if the fd is not ready: */
ssize_t
tme_sjlj_write_yield(int fd, void *data, size_t count)
{
  int rc;
  fd_set fdset_write_in;

  /* select on the fd for writing: */
  FD_ZERO(&fdset_write_in);
  FD_SET(fd, &fdset_write_in);
  rc = tme_sjlj_select_yield(fd + 1,
			     NULL,
			     &fdset_write_in,
			     NULL,
			     NULL);
  if (rc != 1) {
    return (rc);
  }

  /* do the write: */
  return (write(fd, data, count));
}
#endif // HAVE_SELECT

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
  if(tme_sjlj_thread_blocked.tme_sjlj_thread_fd_cond != 0) {
    blocked = TRUE;
  }
  
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
