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
#if defined(TME_THREADS_FIBER) || defined(WIN32)
#include <tme/openvpn-setup.h>
#endif
#if defined(__EMSCRIPTEN__) && defined(TME_THREADS_POSIX)
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
#ifdef TME_THREADS_POSIX
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

// Set main thread loop iteration function & argument
void tme_threads_set_main(tme_threads_fn1 run, void *arg, tme_mutex_t *mutex, tme_time_t delay) {
  tme_threads.tme_threads_run = run;
  tme_threads.tme_threads_arg = arg;
  tme_threads.tme_threads_mutex = mutex;
  tme_threads.tme_threads_delay = delay;
  //  tme_cond_notify(&tme_cond_start,TRUE);
}

int tme_threads_init() {
  /* initialize the threading system: */
  tme_threads.tme_threads_run = tme_threads_main_iter;
  tme_threads.tme_threads_arg = 0;
  tme_threads.tme_threads_mutex = NULL;
  tme_threads.tme_threads_delay = TME_TIME_SET_SEC(10);
  _tme_threads_init();

  /* Synchronization primitive provided to allow sequential
     execution of pre-thread initialization code. It is used
     as the condition to start all threads. */

  tme_cond_init(&tme_cond_start);

#ifdef WIN32
  win32_stdin = tme_new0(struct tme_win32_handle, 1);
  win32_stdout = tme_new0(struct tme_win32_handle, 1);
  win32_stderr = tme_new0(struct tme_win32_handle, 1);
  win32_stdin->rw_handle.read =
    win32_stdin->handle = GetStdHandle(STD_INPUT_HANDLE);
  win32_stdout->rw_handle.write =
    win32_stdout->handle = GetStdHandle(STD_OUTPUT_HANDLE);
  win32_stderr->rw_handle.write =
    win32_stderr->handle = GetStdHandle(STD_ERROR_HANDLE);
  win32_stdin->reads.overlapped.hEvent =
    win32_stdout->writes.overlapped.hEvent =
    win32_stderr->reads.overlapped.hEvent = NULL;
#endif
  _tme_thread_resumed();
  return TME_OK;
}

void tme_threads_run(void) {
  _tme_thread_suspended();
  tme_thread_enter(tme_threads.tme_threads_mutex);
  
  /* Run the main loop */
#if defined(__EMSCRIPTEN__) && defined(TME_THREADS_POSIX)
  // Receives a function to call and some user data to provide it.
  emscripten_request_animation_frame_loop(tme_threads.tme_threads_run, tme_threads.tme_threads_arg);
#else
  if(tme_threads.tme_threads_run)
    for(;;) {
      if(tme_threads.tme_threads_delay)
	tme_thread_sleep_yield(tme_threads.tme_threads_delay, tme_threads.tme_threads_mutex);
      (*tme_threads.tme_threads_run)(tme_threads.tme_threads_arg);
    }
  else
    (*(tme_threads_fn)tme_threads.tme_threads_arg)();
#endif
  tme_thread_exit(tme_threads.tme_threads_mutex);
}

void _tme_thread_enter(tme_mutex_t *mutex) {
  _tme_thread_resumed();
  if(mutex) {
     tme_mutex_lock(mutex);
     //     tme_cond_sleep_yield(&tme_cond_start, mutex, tme_cond_delay);
  }
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

#ifdef TME_THREADS_FIBER
tme_off_t tme_thread_seek (tme_thread_handle_t hand, tme_off_t off, int where) {
  tme_event_seek(hand->handle, &hand->reads, off, where);
  return tme_event_seek(hand->handle, &hand->writes, off, where);
}
#endif

static _tme_inline int
tme_event_read (tme_event_t hand, void *data, int len)
{
  return (hand == TME_STD_HANDLE(stdin)) ?
    (tme_read(hand->handle, data, len)) :
    (tme_finalize (hand->handle, &hand->reads, NULL));
}

static _tme_inline int
tme_event_write (tme_event_t hand, void *data, int len)
{
  if(hand == TME_STD_HANDLE(stdout) || hand == TME_STD_HANDLE(stderr))
    return tme_write(hand->handle, data, len);
  else {
    struct buffer buf;
  
    CLEAR(buf);

    buf.data = data;
    buf.len = len;

    return tme_write_win32 (hand, &buf);
  }
}

#else // WIN32
#define tme_event_read tme_read
#define tme_event_write tme_write
#endif // !WIN32

#if defined(TME_THREADS_FIBER) || defined(WIN32)
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

#ifdef WIN32
  int i, key_event=FALSE;
  do {
    if (rwflags & EVENT_READ) {
      if (hand == TME_STD_HANDLE(stdin)) {
	INPUT_RECORD record[128];
	DWORD numRead;
	key_event=TRUE;
	if((rc = GetNumberOfConsoleInputEvents(hand->handle, &numRead)) &&
	   numRead>0 &&
	   (rc = PeekConsoleInput(hand->handle, record, 128, &numRead))) {
	  for(i=0;i<numRead;i++) {
	    if(record[i].EventType != KEY_EVENT) {
	      // don't care about other console events
	      continue;
	    }
      
	    if(!record[i].Event.KeyEvent.bKeyDown) {
	      // really only care about keydown
	      continue;
	    }
	    break;
	  }
	  if(i!=numRead) break;
	  rc = ReadConsoleInput(hand->handle, record, i, &numRead);
	}
	hand->reads.status = (rc) ? (rc) : (GetLastError());
	// if you're setup for ASCII, process this:
	//record.Event.KeyEvent.uChar.AsciiChar
      } else {
	if(!hand->writes.buf_init.data) {
	  hand->reads.buf_init.data = data;
	  hand->reads.buf_init.len = len;
	}
	tme_read_queue (hand, 0);
      }
    }
#endif
    tme_event_reset(tme_events);
    tme_event_ctl(tme_events, tme_event_handle(hand), rwflags, 0);
    rc = tme_event_wait(tme_events, NULL, &esr, 1, mutex);
#ifdef WIN32
  } while(key_event);
#endif
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
#endif // openvpn-dependent

#ifdef _TME_HAVE_ZLIB
struct tme_zlib_handle  *tme_zlib_open(const char *path, int flags) {
  struct tme_zlib_handle  *hand;
  gzFile handle;
  int fd = open(path, flags);
  char *mode;
  
  if(fd == -1)
    return NULL;
  
  switch(flags) {
  case TME_FILE_RO: mode="rb"; break;
  case TME_FILE_WO: mode="wb"; break;
  case TME_FILE_RW: mode="+"; break;
  default: mode="a"; break;
  }

  handle = gzdopen(fd, mode);

  if(handle == NULL)
    return NULL;
  
  hand = tme_new0(struct tme_zlib_handle, 1);
  hand->fd = fd;
  hand->handle = handle;
  return hand;
}

#ifdef TME_THREADS_FIBER
/* this reads or writes, yielding if the event is not ready: */
ssize_t
tme_zlib_yield(struct tme_zlib_handle  *hand, void *data, size_t len, unsigned int rwflags, tme_mutex_t *mutex, void **outbuf)
{
  int rc = 1;
  struct event_set_return esr;
  tme_event_set_t *tme_events = tme_event_set_init(&rc, EVENT_METHOD_FAST);

  tme_event_reset(tme_events);
  tme_event_ctl(tme_events, hand->fd, rwflags, 0);
  rc = tme_event_wait(tme_events, NULL, &esr, 1, mutex);
  tme_event_free(tme_events);
  
  /* do the i/o: */
  if(esr.rwflags & EVENT_WRITE)
    rc = gzwrite(hand->handle, data, len);
  if(esr.rwflags & EVENT_READ) {
    rc = gzread(hand->handle, data, len);
    if(outbuf)
      *outbuf = data;
  }
  return rc;
}

#endif // THREADS_FIBER
#endif // HAVE_ZLIB
