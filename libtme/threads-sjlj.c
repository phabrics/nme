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
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <setjmp.h>

/* if we don't have GTK, fake a few definitions to keep things
   compiling: */
#ifdef HAVE_GTK
#ifndef G_ENABLE_DEBUG
#define G_ENABLE_DEBUG (0)
#endif /* !G_ENABLE_DEBUG */
#include <gtk/gtk.h>
#else  /* !HAVE_GTK */
typedef int gint;
typedef int GdkInputCondition;
typedef void *gpointer;
#define GDK_INPUT_READ		TME_BIT(0)
#define GDK_INPUT_WRITE		TME_BIT(1)
#define GDK_INPUT_EXCEPTION	TME_BIT(2)
#endif /* !HAVE_GTK */

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

  /* the file descriptors that this thread is waiting on: */
  int tme_sjlj_thread_max_fd;
  fd_set tme_sjlj_thread_fdset_read;
  fd_set tme_sjlj_thread_fdset_write;
  fd_set tme_sjlj_thread_fdset_except;

  /* if nonzero, the amount of time that this thread is sleeping,
     followed by the time the sleep will timeout.  all threads with
     timeouts are kept on a sorted list: */
  struct timeval tme_sjlj_thread_sleep;
  struct timeval tme_sjlj_thread_timeout;
  struct tme_sjlj_thread *timeout_next;
  struct tme_sjlj_thread **timeout_prev;

  /* the last dispatch number for this thread: */
  tme_uint32_t tme_sjlj_thread_dispatch_number;
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

/* the main loop fd sets: */
static int tme_sjlj_main_max_fd;
static fd_set tme_sjlj_main_fdset_read;
static fd_set tme_sjlj_main_fdset_write;
static fd_set tme_sjlj_main_fdset_except;

/* for each file descriptor, any threads blocked on it: */
static struct {
  GdkInputCondition tme_sjlj_fd_thread_conditions;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_read;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_write;
  struct tme_sjlj_thread *tme_sjlj_fd_thread_except;
} tme_sjlj_fd_thread[FD_SETSIZE];

/* the dispatch number: */
static tme_uint32_t _tme_sjlj_thread_dispatch_number;

/* a reasonably current time: */
static struct timeval _tme_sjlj_now;

/* if nonzero, the last dispatched thread ran for only a short time: */
int tme_sjlj_thread_short;

#ifdef HAVE_GTK

/* nonzero iff we're using the gtk main loop: */
static int tme_sjlj_using_gtk;

/* for each file descriptor, the GTK tag for the fd event source: */
static gint tme_sjlj_fd_tag[FD_SETSIZE];

/* this set iff the idle callback is set: */
static int tme_sjlj_idle_set;

/* any timeout source ID: */
static guint _tme_sjlj_gtk_timeout_id;

/* any timeout time: */
static struct timeval _tme_sjlj_gtk_timeout;

#endif /* HAVE_GTK */

/* this initializes the threads system: */
void
tme_sjlj_threads_init(void)
{
  int fd;

#ifdef HAVE_GTK
  /* assume that we won't be using the GTK main loop: */
  tme_sjlj_using_gtk = FALSE;
  tme_sjlj_idle_set = FALSE;
#endif /* HAVE_GTK */

  /* there are no threads: */
  tme_sjlj_threads_all = NULL;
  tme_sjlj_threads_timeout = NULL;
  tme_sjlj_threads_runnable = NULL;
  tme_sjlj_threads_dispatching = NULL;
  tme_sjlj_thread_active = NULL;
  tme_sjlj_thread_exiting = FALSE;

  /* no threads are waiting on any fds: */
  tme_sjlj_main_max_fd = -1;
  FD_ZERO(&tme_sjlj_main_fdset_read);
  FD_ZERO(&tme_sjlj_main_fdset_write);
  FD_ZERO(&tme_sjlj_main_fdset_except);
  for (fd = 0; fd < FD_SETSIZE; fd++) {
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions = 0;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_read = NULL;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_write = NULL;
    tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_except = NULL;
  }

  /* initialize the thread-blocked structure: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = NULL;
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = -1;
  tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_sec = 0;
  tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec = 0;
}

#ifdef HAVE_GTK
/* this initializes the threads system to use the GTK event loop: */
void
tme_sjlj_threads_gtk_init(void)
{
  char **argv;
  char *argv_buffer[3];
  int argc;

  /* if we've already initialized GTK: */
  if (tme_sjlj_using_gtk) {
    return;
  }

  /* conjure up an argv.  this is pretty bad: */
  argv = argv_buffer;
  argc = 0;
  argv[argc++] = "tmesh";
#if 1
  argv[argc++] = "--gtk-debug=signals";
#endif
  argv[argc] = NULL;
  gtk_init(&argc, &argv);
  
  /* we are now using GTK: */
  tme_sjlj_using_gtk = TRUE;
}
#endif /* HAVE_GTK */

/* this returns a reasonably current time: */
void
tme_sjlj_gettimeofday(struct timeval *now)
{

  /* if we need to, call gettimeofday(): */
  if (__tme_predict_false(!tme_sjlj_thread_short)) {
    gettimeofday(&_tme_sjlj_now, NULL);
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
  struct timeval now;
  struct tme_sjlj_thread *thread_timeout;

  /* get the current time: */
  tme_gettimeofday(&now);

  /* loop over the timeout list: */
  for (thread_timeout = tme_sjlj_threads_timeout;
       thread_timeout != NULL;
       thread_timeout = thread_timeout->timeout_next) {

    /* if this timeout has not expired: */
    if (thread_timeout->tme_sjlj_thread_timeout.tv_sec > now.tv_sec
	|| (thread_timeout->tme_sjlj_thread_timeout.tv_sec == now.tv_sec
	    && thread_timeout->tme_sjlj_thread_timeout.tv_usec > now.tv_usec)) {
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
				 GdkInputCondition fd_conditions)
{
  struct tme_sjlj_thread *thread;

  /* loop over all set conditions: */
  for (fd_conditions &= tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions;
       fd_conditions != 0;
       fd_conditions &= (fd_conditions - 1)) {

    /* move the thread for this condition to the dispatching list: */
    thread = ((fd_conditions & GDK_INPUT_READ)
	      ? tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_read
	      : (fd_conditions & GDK_INPUT_WRITE)
	      ? tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_write
	      : tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_except);
    assert (thread != NULL);
    _tme_sjlj_change_state(thread, TME_SJLJ_THREAD_STATE_DISPATCHING);
  }
}

/* this makes the timeout time: */
static void
_tme_sjlj_timeout_time(struct timeval *timeout)
{
  struct timeval now;
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
  assert (thread_timeout->tme_sjlj_thread_timeout.tv_usec < 1000000);
  usecs = thread_timeout->tme_sjlj_thread_timeout.tv_usec;
  assert (now.tv_usec < 1000000);
  usecs -= now.tv_usec;

  /* make any seconds carry: */
  secs_other = (usecs < 0);
  if (usecs < 0) {
    usecs += 1000000;
  }

  /* if the earliest timeout has already timed out: */
  secs_other += now.tv_sec;
  secs = thread_timeout->tme_sjlj_thread_timeout.tv_sec;
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
  int rc_one;

  /* dispatch the given number of passes over the dispatching threads: */
  for (; passes-- > 0; ) {
    for (tme_sjlj_thread_active = tme_sjlj_threads_dispatching;
	 (thread = tme_sjlj_thread_active) != NULL; ) {

      /* if this thread is on the timeout list: */
      _thread_timeout_prev = thread->timeout_prev;
      assert ((_thread_timeout_prev != NULL)
	      == (thread->tme_sjlj_thread_sleep.tv_sec != 0
		  || thread->tme_sjlj_thread_sleep.tv_usec != 0));
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

#ifdef HAVE_GTK

/* this handles a GTK callback for a timeout: */
static gint
_tme_sjlj_gtk_callback_timeout(gpointer callback_pointer)
{

  /* we were in GTK for an unknown amount of time: */
  tme_thread_long();

  /* this GTK timeout will soon be removed, so forget it: */
  _tme_sjlj_gtk_timeout_id = 0;

  /* move all threads that have timed out to the dispatching list: */
  _tme_sjlj_threads_dispatching_timeout();

  /* dispatch: */
  tme_sjlj_dispatch(1);

  /* yield to GTK: */
  tme_threads_gtk_yield();

  /* remove this timeout: */
  return (FALSE);

  /* unused: */
  callback_pointer = 0;
}

/* this handles a GTK callback for a file descriptor: */
static void
_tme_sjlj_gtk_callback_fd(gpointer callback_pointer,
			  gint fd,
			  GdkInputCondition fd_conditions)
{

  /* we were in GTK for an unknown amount of time: */
  tme_thread_long();

  /* move all threads that match the conditions on this file
     descriptor to the dispatching list: */
  _tme_sjlj_threads_dispatching_fd(fd, fd_conditions);

  /* dispatch: */
  tme_sjlj_dispatch(1);

  /* yield to GTK: */
  tme_threads_gtk_yield();

  /* unused: */
  callback_pointer = 0;
}

/* this handles a GTK callback for an idle: */
static gint
_tme_sjlj_gtk_callback_idle(gpointer callback_pointer)
{

  /* we were in GTK for an unknown amount of time: */
  tme_thread_long();

  /* move all runnable threads to the dispatching list: */
  _tme_sjlj_threads_dispatching_runnable();

  /* move all threads that have timed out to the dispatching list: */
  _tme_sjlj_threads_dispatching_timeout();

  /* dispatch: */
  tme_sjlj_dispatch(1);

  /* yield to GTK: */
  tme_threads_gtk_yield();

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

/* this yields to GTK: */
void
tme_sjlj_threads_gtk_yield(void)
{
  struct tme_sjlj_thread *thread_timeout;
  struct timeval timeout;
  unsigned long secs;
  tme_uint32_t msecs;

  /* if there are no runnable threads: */
  if (tme_sjlj_threads_runnable == NULL) {

    /* if there are no threads at all: */
    if (__tme_predict_false(tme_sjlj_threads_all == NULL)) {

      /* quit the GTK main loop: */
      gtk_main_quit();
      return;
    }

    /* if there is a GTK timeout, but the timeout list is empty or the
       GTK timeout isn't for the earliest timeout: */
    thread_timeout = tme_sjlj_threads_timeout;
    if (_tme_sjlj_gtk_timeout_id != 0
	&& (thread_timeout == NULL
	    || _tme_sjlj_gtk_timeout.tv_sec != thread_timeout->tme_sjlj_thread_timeout.tv_sec
	    || _tme_sjlj_gtk_timeout.tv_usec != thread_timeout->tme_sjlj_thread_timeout.tv_usec)) {

      /* remove the GTK timeout: */
      g_source_remove(_tme_sjlj_gtk_timeout_id);
      _tme_sjlj_gtk_timeout_id = 0;
    }

    /* if the timeout list is not empty, but there is no GTK timeout: */
    if (tme_sjlj_threads_timeout != NULL
	&& _tme_sjlj_gtk_timeout_id == 0) {

      /* get the timeout: */
      _tme_sjlj_timeout_time(&timeout);

      /* if there are still no runnable threads: */
      /* NB: if the earliest timeout has already timed out,
	 _tme_sjlj_timeout_time() has already made the thread
	 runnable: */
      if (tme_sjlj_threads_runnable == NULL) {

	/* convert the timeout into milliseconds, and clip it at ten
	   seconds: */
	secs = timeout.tv_sec;
	msecs = (timeout.tv_usec + 999) / 1000;
	if (msecs == 1000) {
	  secs++;
	  msecs = 0;
	}
	msecs
	  = (secs >= 10
	     ? (10 * 1000)
	     : ((secs * 1000) + msecs));

	/* GTK timeouts can expire up to one millisecond early, so we
	   always add one: */
	msecs++;

	/* add the timeout: */
	/* XXX we have to call g_timeout_add_full here, because
	   there are no gtk_timeout_add_ functions that allow you to
	   specify the priority, and gtk_timeout_add() uses
	   G_PRIORITY_DEFAULT, which means our (usually very
	   frequent) timeouts always win over gtk's event handling,
	   meaning the gtk windows never update: */
	_tme_sjlj_gtk_timeout_id
	  = g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE,
			       msecs,
			       _tme_sjlj_gtk_callback_timeout,
			       NULL,
			       NULL);
	assert (_tme_sjlj_gtk_timeout_id != 0);
	_tme_sjlj_gtk_timeout = tme_sjlj_threads_timeout->tme_sjlj_thread_timeout;
      }
    }
  }

  /* if there are runnable threads: */
  if (tme_sjlj_threads_runnable != NULL) {

    /* if the idle callback isn't set */
    if (!tme_sjlj_idle_set) {

      /* set the idle callback: */
      gtk_idle_add_priority(G_PRIORITY_DEFAULT_IDLE,
			    _tme_sjlj_gtk_callback_idle,
			    NULL);
      tme_sjlj_idle_set = TRUE;
    }
  }
}      

#endif /* HAVE_GTK */
      
/* this starts the threads dispatching: */
void
tme_sjlj_threads_run(void)
{
  int fd;
  fd_set fdset_read_out;
  fd_set fdset_write_out;
  fd_set fdset_except_out;
  GdkInputCondition fd_conditions;
  struct timeval timeout_buffer;
  struct timeval *timeout;
  int rc;
  
#ifdef HAVE_GTK
  /* if we're using the GTK main loop, yield to GTK and
     call gtk_main(): */
  if (tme_sjlj_using_gtk) {
    tme_threads_gtk_yield();
    gtk_main();
    return;
  }
#endif /* HAVE_GTK */

  /* otherwise, we have to use our own main loop: */
  
  /* loop while we have threads: */
  for (; tme_sjlj_threads_all != NULL; ) {

    /* if we have file descriptors to select on: */
    if (tme_sjlj_main_max_fd >= 0) {

      /* make the fd sets: */
      fdset_read_out = tme_sjlj_main_fdset_read;
      fdset_write_out = tme_sjlj_main_fdset_write;
      fdset_except_out = tme_sjlj_main_fdset_except;
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
      timeout_buffer.tv_sec = 0;
      timeout_buffer.tv_usec = 0;
      timeout = &timeout_buffer;
    }

    /* do the select: */
    rc = select(tme_sjlj_main_max_fd + 1,
		&fdset_read_out,
		&fdset_write_out,
		&fdset_except_out,
		timeout);

    /* we were in select() for an unknown amount of time: */
    tme_thread_long();

    /* move all runnable threads to the dispatching list: */
    _tme_sjlj_threads_dispatching_runnable();

    /* move all threads that have timed out to the dispatching list: */
    _tme_sjlj_threads_dispatching_timeout();

    /* if some fds are ready, dispatch them: */
    if (rc > 0) {	
      for (fd = tme_sjlj_main_max_fd; fd >= 0; fd--) {
	fd_conditions = 0;
	if (FD_ISSET(fd, &fdset_read_out)) {
	  fd_conditions |= GDK_INPUT_READ;
	}
	if (FD_ISSET(fd, &fdset_write_out)) {
	  fd_conditions |= GDK_INPUT_WRITE;
	}
	if (FD_ISSET(fd, &fdset_except_out)) {
	  fd_conditions |= GDK_INPUT_EXCEPTION;
	}
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
  }

  /* all threads have exited: */
}

/* this creates a new thread: */
void
tme_sjlj_thread_create(tme_thread_t func, void *func_private)
{
  struct tme_sjlj_thread *thread;

  /* allocate a new thread and put it on the all-threads list: */
  thread = tme_new(struct tme_sjlj_thread, 1);
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
  thread->tme_sjlj_thread_max_fd = -1;
  thread->tme_sjlj_thread_sleep.tv_sec = 0;
  thread->tme_sjlj_thread_sleep.tv_usec = 0;
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
tme_sjlj_cond_sleep_yield(tme_cond_t *cond, tme_mutex_t *mutex, const struct timeval *sleep)
{

  /* unlock the mutex: */
  tme_mutex_unlock(mutex);

  /* remember that this thread is waiting on this condition: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_cond = cond;

  /* sleep and yield: */
  tme_sjlj_sleep_yield(sleep->tv_sec, sleep->tv_usec);
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
       
/* this yields the current thread: */
void
tme_sjlj_yield(void)
{
  struct tme_sjlj_thread *thread;
  int blocked;
  int max_fd_old;
  int max_fd_new;
  int max_fd, fd;
  GdkInputCondition fd_condition_old;
  GdkInputCondition fd_condition_new;
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
  max_fd_old = thread->tme_sjlj_thread_max_fd;
  max_fd_new = tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd;
  max_fd = TME_MAX(max_fd_old, max_fd_new);
  for (fd = 0; fd <= max_fd; fd++) {

    /* the old and new conditions on this fd start out empty: */
    fd_condition_old = 0;
    fd_condition_new = 0;

    /* check the old fd sets: */
    if (fd <= max_fd_old) {
#define CHECK_FD_SET(fd_set, condition)	\
do {					\
  if (FD_ISSET(fd, &thread->fd_set)) {	\
    fd_condition_old |= condition;	\
  }					\
} while (/* CONSTCOND */ 0)
      CHECK_FD_SET(tme_sjlj_thread_fdset_read, GDK_INPUT_READ);
      CHECK_FD_SET(tme_sjlj_thread_fdset_write, GDK_INPUT_WRITE);
      CHECK_FD_SET(tme_sjlj_thread_fdset_except, GDK_INPUT_EXCEPTION);
#undef CHECK_FD_SET
    }

    /* check the new fd sets: */
    if (fd <= max_fd_new) {
#define CHECK_FD_SET(fd_set, condition)			\
do {							\
  if (FD_ISSET(fd, &tme_sjlj_thread_blocked.fd_set)) {	\
    fd_condition_new |= condition;			\
    FD_SET(fd, &thread->fd_set);			\
  }							\
  else {						\
    FD_CLR(fd, &thread->fd_set);			\
  }							\
} while (/* CONSTCOND */ 0)
      CHECK_FD_SET(tme_sjlj_thread_fdset_read, GDK_INPUT_READ);
      CHECK_FD_SET(tme_sjlj_thread_fdset_write, GDK_INPUT_WRITE);
      CHECK_FD_SET(tme_sjlj_thread_fdset_except, GDK_INPUT_EXCEPTION);
#undef CHECK_FD_SET
    }

    /* if this thread is blocked on this file descriptor: */
    if (fd_condition_new != 0) {
      blocked = TRUE;
    }

    /* if the conditions have changed: */
    if (fd_condition_new != fd_condition_old) {

      /* if there is any blocking on this file descriptor, remove it: */
      if (tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions != 0) {

#ifdef HAVE_GTK
	if (tme_sjlj_using_gtk) {

	  /* it should be safe to remove this fd, even if we're
	     currently in a callback for it.  if we happen to get a
	     callback for it later anyways, _tme_sjlj_gtk_callback_fd()
	     will ignore it: */
	  gdk_input_remove(tme_sjlj_fd_tag[fd]);
	}
	else
#endif /* HAVE_GTK */
	  {

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
      }

      /* update the blocking by this thread on this file descriptor: */
      assert ((tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions
	       & fd_condition_old)
	      == fd_condition_old);
      tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions
	= ((tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions
	    & ~fd_condition_old)
	   | fd_condition_new);
#define UPDATE_FD_THREAD(fd_thread, condition)		\
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
      UPDATE_FD_THREAD(tme_sjlj_fd_thread_read, GDK_INPUT_READ);
      UPDATE_FD_THREAD(tme_sjlj_fd_thread_write, GDK_INPUT_WRITE);
      UPDATE_FD_THREAD(tme_sjlj_fd_thread_except, GDK_INPUT_EXCEPTION);
#undef UPDATE_FD_THREAD    

      /* get the conditions for all threads for this fd: */
      fd_condition_new = tme_sjlj_fd_thread[fd].tme_sjlj_fd_thread_conditions;

      /* if there is any blocking on this file descriptor, add it: */
      if (fd_condition_new != 0) {

#ifdef HAVE_GTK
	if (tme_sjlj_using_gtk) {
	  tme_sjlj_fd_tag[fd] = 
	    gdk_input_add(fd,
			  fd_condition_new,
			  _tme_sjlj_gtk_callback_fd,
			  NULL);
	}
	else
#endif /* HAVE_GTK */
	  {

	    /* add this fd to main loop's relevant fd sets: */
	    if (fd_condition_new & GDK_INPUT_READ) {
	      FD_SET(fd, &tme_sjlj_main_fdset_read);
	    }
	    if (fd_condition_new & GDK_INPUT_WRITE) {
	      FD_SET(fd, &tme_sjlj_main_fdset_write);
	    }
	    if (fd_condition_new & GDK_INPUT_EXCEPTION) {
	      FD_SET(fd, &tme_sjlj_main_fdset_except);
	    }
	    if (fd > tme_sjlj_main_max_fd) {
	      tme_sjlj_main_max_fd = fd;
	    }
	  }
      }
    }
  }
  thread->tme_sjlj_thread_max_fd = max_fd_new;
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = -1;

  /* see if this thread is blocked for some amount of time: */
  if (tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_sec != 0
      || tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec != 0) {

    assert(tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec < 1000000);
    blocked = TRUE;

    /* set the timeout for this thread: */
    tme_gettimeofday(&thread->tme_sjlj_thread_timeout);
    thread->tme_sjlj_thread_timeout.tv_sec
      += tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_sec;
    thread->tme_sjlj_thread_timeout.tv_usec
      += tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec;
    if (thread->tme_sjlj_thread_timeout.tv_usec >= 1000000) {
      thread->tme_sjlj_thread_timeout.tv_sec++;
      thread->tme_sjlj_thread_timeout.tv_usec -= 1000000;
    }

    /* insert this thread into the timeout list: */
    assert (thread->timeout_prev == NULL);
    for (_thread_prev = &tme_sjlj_threads_timeout;
	 (thread_other = *_thread_prev) != NULL;
	 _thread_prev = &thread_other->timeout_next) {
      if ((thread_other->tme_sjlj_thread_timeout.tv_sec
	   > thread->tme_sjlj_thread_timeout.tv_sec)
	  || ((thread_other->tme_sjlj_thread_timeout.tv_sec
	       == thread->tme_sjlj_thread_timeout.tv_sec)
	      && (thread_other->tme_sjlj_thread_timeout.tv_usec
		  >= thread->tme_sjlj_thread_timeout.tv_usec))) {
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
  tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_sec = 0;
  tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec = 0;

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

/* this sleeps: */
void
tme_sjlj_sleep(unsigned long sec, unsigned long usec)
{
  struct timeval then, now, timeout;
  int rc;
  
  /* the thread ran for an unknown amount of time: */
  tme_thread_long();

  /* get the wakeup time for the thread: */
  tme_gettimeofday(&then);
  for (; usec >= 1000000; sec++, usec -= 1000000);
  if ((then.tv_usec += usec) >= 1000000) {
    sec++;
    then.tv_usec -= 1000000;
  }
  then.tv_sec += sec;
  
  /* select for the sleep period: */
  for (;;) {

    /* calculate the current timeout: */
    tme_gettimeofday(&now);
    if ((now.tv_sec > then.tv_sec)
	|| (now.tv_sec == then.tv_sec
	    && now.tv_usec >= then.tv_usec)) {
      break;
    }
    timeout = then;
    if (timeout.tv_usec < now.tv_usec) {
      timeout.tv_sec--;
      timeout.tv_usec += 1000000;
    }
    timeout.tv_sec -= now.tv_sec;
    timeout.tv_usec -= now.tv_usec;

    /* do the select.  select returns 0 iff the timeout expires, so we
       can skip another gettimeofday and loop: */
    rc = select(-1, NULL, NULL, NULL, &timeout);
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
  tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_sec = sec;
  tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec = usec;

  /* yield: */
  tme_thread_yield();
}

/* this selects and yields: */
int
tme_sjlj_select_yield(int nfds,
		      fd_set *fdset_read_in,
		      fd_set *fdset_write_in,
		      fd_set *fdset_except_in,
		      struct timeval *timeout_in)
{
  struct timeval timeout_out;
  int rc;

  /* we can't deal if there are more than FD_SETSIZE fds: */
  assert(nfds <= FD_SETSIZE);

  /* in case we end up yielding, we need to save the original
     descriptor sets: */
  if (fdset_read_in != NULL) {
    tme_sjlj_thread_blocked.tme_sjlj_thread_fdset_read = *fdset_read_in;
  }
  if (fdset_write_in != NULL) {
    tme_sjlj_thread_blocked.tme_sjlj_thread_fdset_write = *fdset_write_in;
  }
  if (fdset_except_in != NULL) {
    tme_sjlj_thread_blocked.tme_sjlj_thread_fdset_except = *fdset_except_in;
  }

  /* do a polling select: */
  timeout_out.tv_sec = timeout_out.tv_usec = 0;
  rc = select(nfds, fdset_read_in, fdset_write_in, fdset_except_in, &timeout_out);
  tme_thread_long();
  if (rc != 0
      || (timeout_in != NULL
	  && timeout_in->tv_sec == 0
	  && timeout_in->tv_usec == 0)) {
    return (rc);
  }

  /* we are yielding.  zero any unused descriptor sets and set the
     timeout time: */
  tme_sjlj_thread_blocked.tme_sjlj_thread_max_fd = nfds - 1;
  if (fdset_read_in == NULL) {
    FD_ZERO(&tme_sjlj_thread_blocked.tme_sjlj_thread_fdset_read);
  }
  if (fdset_write_in == NULL) {
    FD_ZERO(&tme_sjlj_thread_blocked.tme_sjlj_thread_fdset_write);
  }
  if (fdset_except_in == NULL) {
    FD_ZERO(&tme_sjlj_thread_blocked.tme_sjlj_thread_fdset_except);
  }
  if (timeout_in != NULL) {
    tme_sjlj_thread_blocked.tme_sjlj_thread_sleep = *timeout_in;
    for (; tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec >= 1000000; ) {
      tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_sec++;
      tme_sjlj_thread_blocked.tme_sjlj_thread_sleep.tv_usec -= 1000000;
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
  fd_set fdset_read_in;
  int rc;

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
  fd_set fdset_write_in;
  int rc;

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

/* this exits a thread: */
void
tme_sjlj_exit(void)
{
  
  /* mark that this thread is exiting: */
  tme_sjlj_thread_exiting = TRUE;

  /* yield: */
  tme_thread_yield();
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
