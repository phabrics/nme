/* libtme/threads.c - threads management: */

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

/* globals: */
static tme_threads_fn1 _tme_threads_run;
static void *_tme_threads_arg;
static int inited;
#ifdef TME_THREADS_SJLJ
/* the i/o events: */
tme_event_set_t *tme_events;
#endif
#ifdef TME_THREADS_POSIX
static pthread_rwlock_t tme_rwlock_start;
pthread_rwlock_t tme_rwlock_suspere;

#ifdef HAVE_PTHREAD_SETSCHEDPARAM
static pthread_attr_t *attrp;

void tme_thread_set_defattr(pthread_attr_t *attr) {
  attrp=attr;
}
pthread_attr_t *tme_thread_defattr() {
  return attrp;
}
#endif // HAVE_PTHREAD_SETSCHEDPARAM
#elif defined(TME_THREADS_GLIB)
static GRWLock tme_rwlock_start;
GRWLock tme_rwlock_suspere;
#endif

void tme_threads_init(tme_threads_fn1 run, void *arg) {
  _tme_threads_run = run;
  _tme_threads_arg = arg;
  if(!inited) {
    _tme_threads_init();
    if(!tme_thread_cooperative()) {
#ifdef TME_THREADS_POSIX
      pthread_rwlock_init(&tme_rwlock_start, NULL);      
      pthread_rwlock_wrlock(&tme_rwlock_start);
#elif defined(TME_THREADS_GLIB)
      g_rw_lock_init(&tme_rwlock_start);
      g_rw_lock_writer_lock(&tme_rwlock_start);
#endif
    }
    _tme_thread_resumed();  
    inited=TRUE;
  }
}

void tme_threads_run(void) {
  if(!tme_thread_cooperative()) {
#ifdef TME_THREADS_POSIX
    pthread_rwlock_unlock(&tme_rwlock_start);
#elif defined(TME_THREADS_GLIB)
    g_rw_lock_writer_unlock(&tme_rwlock_start);  
#endif
  }
#ifdef TME_THREADS_SJLJ
  int rc = 1;
  tme_events = tme_event_set_init(&rc, EVENT_METHOD_FAST);
#endif
  _tme_thread_suspended();
  /* Run the main loop */
  if(_tme_threads_run)
    for(;!(*_tme_threads_run)(_tme_threads_arg););
  else
    (*(tme_threads_fn)_tme_threads_arg)();
}

void tme_thread_enter(tme_mutex_t *mutex) {
  if(!tme_thread_cooperative()) {
#ifdef TME_THREADS_POSIX
    pthread_rwlock_rdlock(&tme_rwlock_start);
#elif defined(TME_THREADS_GLIB)
    g_rw_lock_reader_lock(&tme_rwlock_start);
#endif
  }
  _tme_thread_resumed();  
  if(mutex)
    tme_mutex_lock(mutex);
}

/* this reads or writes, yielding if the event is not ready: */
ssize_t
tme_event_yield(tme_event_t hand, void *data, size_t count, unsigned int rwflags, tme_mutex_t *mutex)
{
  int rc = 1;
  struct event_set_return esr;
#ifndef TME_THREADS_SJLJ
  tme_event_set_t *tme_events = tme_event_set_init(&rc, EVENT_METHOD_FAST);
#endif
  
  tme_event_reset(tme_events);
  tme_event_ctl(tme_events, tme_event_handle(hand), rwflags, 0);

#ifdef WIN32
  if (rwflags & EVENT_READ)
    tme_read_queue (hand, 0);
#endif

  rc = tme_event_wait_yield(tme_events, NULL, &esr, 1, mutex);

  /* do the i/o: */
  if(esr.rwflags & EVENT_WRITE)
    rc = tme_event_write(hand, data, count);  
  if(esr.rwflags & EVENT_READ)
    rc = tme_event_read(hand, data, count);
  return rc;
}

#ifdef WIN32

int
tme_read_queue (tme_win32_handle_t hand, int maxsize)
{
  if (hand->reads.iostate == IOSTATE_INITIAL)
    {
      DWORD len;
      BOOL status;
      int err;

      /* reset buf to its initial state */
      hand->reads.buf = hand->reads.buf_init;

      len = maxsize ? maxsize : BLEN (&hand->reads.buf);
      ASSERT (len <= BLEN (&hand->reads.buf));

      /* the overlapped read will signal this event on I/O completion */
      ASSERT (ResetEvent (hand->reads.overlapped.hEvent));

      status = ReadFile(
		      hand->hand,
		      BPTR (&hand->reads.buf),
		      len,
		      &hand->reads.size,
		      &hand->reads.overlapped
		      );

      if (status) /* operation completed immediately? */
	{
	  /* since we got an immediate return, we must signal the event object ourselves */
	  ASSERT (SetEvent (hand->reads.overlapped.hEvent));

	  hand->reads.iostate = IOSTATE_IMMEDIATE_RETURN;
	  hand->reads.status = 0;

	  dmsg (D_WIN32_IO, "WIN32 I/O: TAP Read immediate return [%d,%d]",
	       (int) len,
	       (int) hand->reads.size);	       
	}
      else
	{
	  err = GetLastError (); 
	  if (err == ERROR_IO_PENDING) /* operation queued? */
	    {
	      hand->reads.iostate = IOSTATE_QUEUED;
	      hand->reads.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TAP Read queued [%d]",
		   (int) len);
	    }
	  else /* error occurred */
	    {
	      struct gc_arena gc = gc_new ();
	      ASSERT (SetEvent (hand->reads.overlapped.hEvent));
	      hand->reads.iostate = IOSTATE_IMMEDIATE_RETURN;
	      hand->reads.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TAP Read error [%d] : %s",
		   (int) len,
		   strerror_win32 (status, &gc));
	      gc_free (&gc);
	    }
	}
    }
  return hand->reads.iostate;
}

int
tme_write_queue (tme_win32_handle_t hand, struct buffer *buf)
{
  if (hand->writes.iostate == IOSTATE_INITIAL)
    {
      BOOL status;
      int err;
 
      /* make a private copy of buf */
      hand->writes.buf = hand->writes.buf_init;
      hand->writes.buf.len = 0;
      ASSERT (buf_copy (&hand->writes.buf, buf));

      /* the overlapped write will signal this event on I/O completion */
      ASSERT (ResetEvent (hand->writes.overlapped.hEvent));

      status = WriteFile(
			hand->hand,
			BPTR (&hand->writes.buf),
			BLEN (&hand->writes.buf),
			&hand->writes.size,
			&hand->writes.overlapped
			);

      if (status) /* operation completed immediately? */
	{
	  hand->writes.iostate = IOSTATE_IMMEDIATE_RETURN;

	  /* since we got an immediate return, we must signal the event object ourselves */
	  ASSERT (SetEvent (hand->writes.overlapped.hEvent));

	  hand->writes.status = 0;

	  dmsg (D_WIN32_IO, "WIN32 I/O: TAP Write immediate return [%d,%d]",
	       BLEN (&hand->writes.buf),
	       (int) hand->writes.size);	       
	}
      else
	{
	  err = GetLastError (); 
	  if (err == ERROR_IO_PENDING) /* operation queued? */
	    {
	      hand->writes.iostate = IOSTATE_QUEUED;
	      hand->writes.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TAP Write queued [%d]",
		   BLEN (&hand->writes.buf));
	    }
	  else /* error occurred */
	    {
	      struct gc_arena gc = gc_new ();
	      ASSERT (SetEvent (hand->writes.overlapped.hEvent));
	      hand->writes.iostate = IOSTATE_IMMEDIATE_RETURN;
	      hand->writes.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TAP Write error [%d] : %s",
		   BLEN (&hand->writes.buf),
		   strerror_win32 (err, &gc));
	      gc_free (&gc);
	    }
	}
    }
  return hand->writes.iostate;
}

int
tme_finalize (
	      HANDLE h,
	      struct overlapped_io *io,
	      struct buffer *buf)
{
  int ret = -1;
  BOOL status;

  switch (io->iostate)
    {
    case IOSTATE_QUEUED:
      status = GetOverlappedResult(
				   h,
				   &io->overlapped,
				   &io->size,
				   FALSE
				   );
      if (status)
	{
	  /* successful return for a queued operation */
	  if (buf)
	    *buf = io->buf;
	  ret = io->size;
	  io->iostate = IOSTATE_INITIAL;
	  ASSERT (ResetEvent (io->overlapped.hEvent));
	  dmsg (D_WIN32_IO, "WIN32 I/O: TAP Completion success [%d]", ret);
	}
      else
	{
	  /* error during a queued operation */
	  ret = -1;
	  if (GetLastError() != ERROR_IO_INCOMPLETE)
	    {
	      /* if no error (i.e. just not finished yet),
		 then DON'T execute this code */
	      io->iostate = IOSTATE_INITIAL;
	      ASSERT (ResetEvent (io->overlapped.hEvent));
	      msg (D_WIN32_IO | M_ERRNO, "WIN32 I/O: TAP Completion error");
	    }
	}
      break;

    case IOSTATE_IMMEDIATE_RETURN:
      io->iostate = IOSTATE_INITIAL;
      ASSERT (ResetEvent (io->overlapped.hEvent));
      if (io->status)
	{
	  /* error return for a non-queued operation */
	  SetLastError (io->status);
	  ret = -1;
	  msg (D_WIN32_IO | M_ERRNO, "WIN32 I/O: TAP Completion non-queued error");
	}
      else
	{
	  /* successful return for a non-queued operation */
	  if (buf)
	    *buf = io->buf;
	  ret = io->size;
	  dmsg (D_WIN32_IO, "WIN32 I/O: TAP Completion non-queued success [%d]", ret);
	}
      break;

    case IOSTATE_INITIAL: /* were we called without proper queueing? */
      SetLastError (ERROR_INVALID_FUNCTION);
      ret = -1;
      dmsg (D_WIN32_IO, "WIN32 I/O: TAP Completion BAD STATE");
      break;

    default:
      ASSERT (0);
    }

  if (buf)
    buf->len = ret;
  return ret;
}

tme_win32_handle_t tme_win32_open(const char *path, int flags, int *fd) {
  tme_win32_handle_t hand;

  hand = tme_new0(tme_win32_handle_t, 1);
  hand->hand = NULL;
 /* manual reset event, initially set according to event_state */
  hand->reads.overlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (hand->reads.overlapped.hEvent == NULL)
    msg (M_ERR, "Error: overlapped_io_init: CreateEvent failed");
  hand->writes.overlapped.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL);
  if (hand->writes.overlapped.hEvent == NULL)
    msg (M_ERR, "Error: overlapped_io_init: CreateEvent failed");

  hand->rw_handle.read = hand->reads.overlapped.hEvent;
  hand->rw_handle.write = hand->writes.overlapped.hEvent;

  return hand;
}
#endif // WIN32
