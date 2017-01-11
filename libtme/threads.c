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

#ifdef WIN32

struct tme_win32_handle {
  HANDLE handle;
  struct overlapped_io reads;
  struct overlapped_io writes;
  struct rw_handle rw_handle;
};

tme_event_t win32_stdin;
tme_event_t win32_stdout;
tme_event_t win32_stderr;
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
#ifdef WIN32
    win32_stdin = tme_new0(struct tme_win32_handle, 1);
    win32_stdout = tme_new0(struct tme_win32_handle, 1);
    win32_stderr = tme_new0(struct tme_win32_handle, 1);
    win32_stdin->rw_handle.read = win32_stdin->reads.overlapped.hEvent =
      win32_stdin->handle = GetStdHandle(STD_INPUT_HANDLE);
    win32_stdout->rw_handle.write = win32_stdout->writes.overlapped.hEvent =
      win32_stdout->handle = GetStdHandle(STD_OUTPUT_HANDLE);
    win32_stderr->rw_handle.read = win32_stderr->reads.overlapped.hEvent =
      win32_stderr->handle = GetStdHandle(STD_ERROR_HANDLE);
#endif
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

#ifdef WIN32

int
tme_read_queue (tme_event_t hand, int maxsize)
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
			hand->handle,
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

	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Read immediate return [%d,%d]",
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
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Read queued [%d]",
		    (int) len);
	    }
	  else /* error occurred */
	    {
	      struct gc_arena gc = gc_new ();
	      ASSERT (SetEvent (hand->reads.overlapped.hEvent));
	      hand->reads.iostate = IOSTATE_IMMEDIATE_RETURN;
	      hand->reads.status = err;
	      hand->reads.size = 0;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Read error [%d] : %s",
		    (int) len,
		    strerror_win32 (status, &gc));
	      gc_free (&gc);
	    }
	}
    }
  return hand->reads.iostate;
}

int
tme_write_queue (tme_event_t hand, struct buffer *buf)
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
			 hand->handle,
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

	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Write immediate return [%d,%d]",
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
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Write queued [%d]",
		    BLEN (&hand->writes.buf));
	    }
	  else /* error occurred */
	    {
	      struct gc_arena gc = gc_new ();
	      ASSERT (SetEvent (hand->writes.overlapped.hEvent));
	      hand->writes.iostate = IOSTATE_IMMEDIATE_RETURN;
	      hand->writes.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Write error [%d] : %s",
		    BLEN (&hand->writes.buf),
		    strerror_win32 (err, &gc));
	      gc_free (&gc);
	    }
	}
    }
  return hand->writes.iostate;
}

tme_off_t tme_event_seek (HANDLE hand, struct overlapped_io *io, tme_off_t off, int where)
{
  LARGE_INTEGER pos;
  
  switch(where) {
  case TME_SEEK_SET:
    pos.QuadPart = 0;
    break;
  case TME_SEEK_CUR:
    pos.u.LowPart = io->overlapped.Offset;
    pos.u.HighPart = io->overlapped.OffsetHigh;
    break;
  case TME_SEEK_END:
    GetFileSizeEx(hand, &pos);
    break;
  }

  pos.QuadPart += off;
  io->overlapped.Offset = pos.u.LowPart;
  io->overlapped.OffsetHigh = pos.u.HighPart;
 
  return pos.QuadPart;
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
	  tme_event_seek(h, io, ret, TME_SEEK_CUR);
	  io->iostate = IOSTATE_INITIAL;
	  ASSERT (ResetEvent (io->overlapped.hEvent));
	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Completion success [%d]", ret);
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
	      msg (D_WIN32_IO | M_ERRNO, "WIN32 I/O: TME Completion error");
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
	  msg (D_WIN32_IO | M_ERRNO, "WIN32 I/O: TME Completion non-queued error");
	}
      else
	{
	  /* successful return for a non-queued operation */
	  if (buf)
	    *buf = io->buf;
	  ret = io->size;
	  tme_event_seek(h, io, ret, TME_SEEK_CUR);
	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Completion non-queued success [%d]", ret);
	}
      break;

    case IOSTATE_INITIAL: /* were we called without proper queueing? */
      SetLastError (ERROR_INVALID_FUNCTION);
      ret = -1;
      dmsg (D_WIN32_IO, "WIN32 I/O: TME Completion BAD STATE");
      break;

    default:
      ASSERT (0);
    }

  if (GetLastError() == ERROR_HANDLE_EOF)
    ret = 0;
  if (buf)
    buf->len = ret;
  return ret;
}

static _tme_inline int
tme_write_win32 (tme_event_t hand, struct buffer *buf)
{
  int err = 0;
  int status = 0;
  if (overlapped_io_active (&hand->writes))
    {
      status = tme_finalize (hand->handle, &hand->writes, NULL);
      if (status < 0)
	err = GetLastError ();
    }
  tme_write_queue (hand, buf);
  if (status < 0)
    {
      SetLastError (err);
      return status;
    }
  else
    return BLEN (buf);
}

tme_event_t tme_win32_open(const char *path, int flags, int attr, size_t size) {
  tme_event_t hand;
  HANDLE handle = CreateFile(path, flags, 0, 0, OPEN_EXISTING, attr, 0);

  if(handle == INVALID_HANDLE_VALUE)
    return NULL;
  
  hand = tme_new0(struct tme_win32_handle, 1);

  hand->handle = handle;
  
  /* manual reset event, initially set according to event_state */
  hand->reads.overlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if(hand->reads.overlapped.hEvent == NULL)
    msg (M_ERR, "Error: overlapped_io_init: CreateEvent failed");

  hand->writes.overlapped.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL);
  if (hand->writes.overlapped.hEvent == NULL)
    msg (M_ERR, "Error: overlapped_io_init: CreateEvent failed");
  
  hand->rw_handle.read = hand->reads.overlapped.hEvent;
  hand->rw_handle.write = hand->writes.overlapped.hEvent;

  if(size) {
    hand->reads.buf_init = alloc_buf(size);
    ASSERT (buf_init (&hand->reads.buf_init, 0));
    hand->reads.buf_init.len = size;
    ASSERT (buf_safe (&hand->reads.buf_init, 0));

    hand->writes.buf_init = alloc_buf(size);
    ASSERT (buf_init (&hand->writes.buf_init, 0));
    hand->writes.buf_init.len = size;
    ASSERT (buf_safe (&hand->writes.buf_init, 0));
  } 
  return hand;
}

void tme_win32_close(tme_event_t hand) {
  if (hand->handle != NULL)
    {
      dmsg (D_WIN32_IO_LOW, "Attempting CancelIO on TAP-Windows adapter");
      if (!CancelIo (hand->handle))
	msg (M_WARN | M_ERRNO, "Warning: CancelIO failed on TAP-Windows adapter");
    }

  dmsg (D_WIN32_IO_LOW, "Attempting close of overlapped read event on TAP-Windows adapter");

  if (hand->reads.overlapped.hEvent)
    {
      if (!CloseHandle (hand->reads.overlapped.hEvent))
	msg (M_WARN | M_ERRNO, "Warning: CloseHandle failed on overlapped I/O event object");
    }

  dmsg (D_WIN32_IO_LOW, "Attempting close of overlapped write event on TAP-Windows adapter");

  if (hand->writes.overlapped.hEvent)
    {
      if (!CloseHandle (hand->writes.overlapped.hEvent))
	msg (M_WARN | M_ERRNO, "Warning: CloseHandle failed on overlapped I/O event object");
    }

  if (hand->handle != NULL)
    {
      dmsg (D_WIN32_IO_LOW, "Attempting CloseHandle on TAP-Windows adapter");
      if (!CloseHandle (hand->handle))
	msg (M_WARN | M_ERRNO, "Warning: CloseHandle failed on TAP-Windows adapter");
    }
}

#ifdef TME_THREADS_SJLJ
tme_off_t tme_thread_seek (tme_thread_handle_t hand, tme_off_t off, int where) {
  tme_event_seek(hand->handle, &hand->reads, off, where);
  return tme_event_seek(hand->handle, &hand->writes, off, where);
}
#endif

static _tme_inline int
tme_event_read (tme_event_t hand, void *data, int len)
{
  return tme_finalize (hand->handle, &hand->reads, NULL);
}

static _tme_inline int
tme_event_write (tme_event_t hand, void *data, int len)
{
  struct buffer buf;

  CLEAR(buf);

  buf.data = data;
  buf.len = len;

  return tme_write_win32 (hand, &buf);
}

#else // WIN32
#define tme_event_read tme_read
#define tme_event_write tme_write
#endif // !WIN32

static _tme_inline event_t
tme_event_handle (const tme_event_t hand)
{
#ifdef WIN32
  return &hand->rw_handle;
#else
  return hand;
#endif
}

/* this reads or writes, yielding if the event is not ready: */
ssize_t
tme_event_yield(tme_event_t hand, void *data, size_t len, unsigned int rwflags, tme_mutex_t *mutex, void **outbuf)
{
  int rc = 1;
  struct event_set_return esr;
  tme_event_set_t *tme_events = tme_event_set_init(&rc, EVENT_METHOD_FAST);
  
  tme_event_reset(tme_events);
  tme_event_ctl(tme_events, tme_event_handle(hand), rwflags, 0);

#ifdef WIN32
  if (rwflags & EVENT_READ) {
    if(!hand->writes.buf_init.data) {
      hand->reads.buf_init.data = data;
      hand->reads.buf_init.len = len;
    }
    tme_read_queue (hand, 0);
  }
#endif

  rc = tme_event_wait_yield(tme_events, NULL, &esr, 1, mutex);
  tme_event_free(tme_events);
  
  /* do the i/o: */
  if(esr.rwflags & EVENT_WRITE)
    rc = tme_event_write(hand, data, len);  
  if(esr.rwflags & EVENT_READ) {
    rc = tme_event_read(hand, data, len);
#ifdef WIN32
    if(hand->writes.buf_init.data)
      if(outbuf)
	*outbuf = hand->reads.buf.data;
      else
	memcpy(data, hand->reads.buf.data, rc);
    else
#endif
      if(outbuf)
	*outbuf = data;
  }
  return rc;
}
